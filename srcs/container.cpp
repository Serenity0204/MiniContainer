#include "../includes/container.hpp"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sched.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

Container::Container(const std::string& rootfs, const std::vector<std::string>& cmd, const std::string& cgname, int cpu_limit, long mem_limit)
    : rootfs(rootfs), cmd(cmd), cgroupName(cgname), cpuQuotaPct(cpu_limit), memoryLimitMB(mem_limit)
{
    // Allocate stack for the child process
    childStack = new char[STACK_SIZE];
}

Container::~Container()
{
    delete[] childStack;
}

void Container::run()
{
    // Create child process with isolated namespaces
    // CLONE_NEWUTS: Isolate UTS namespace (hostname, domain name)
    // CLONE_NEWPID: Isolate PID namespace (process IDs)
    // CLONE_NEWNS: Isolate mount namespace (filesystem mounts)
    // CLONE_NEWIPC: Isolate System V IPC and POSIX message queues
    // SIGCHLD: Send SIGCHLD signal to parent when child exits
    // this: Pass the current Container instance to the child function
    pid_t pid = clone(&Container::childFuncWrapper, childStack + STACK_SIZE, CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWIPC | SIGCHLD, this);

    // if clone fails
    if (pid == -1)
    {
        std::cerr << "Failed to create child process: " << strerror(errno) << std::endl;
        return;
    }

    // Setup cgroup limits
    setupCgroup(pid);

    // wait for the child process to finish
    waitpid(pid, nullptr, 0);
}

int Container::childFunc()
{
    // Make mounts private
    // MS_REC:
    // MS_PRIVATE:
    // if fails,
    if (mount(nullptr, "/", nullptr, MS_REC | MS_PRIVATE, nullptr) < 0)
    {
        perror("mount(MS_PRIVATE)");
        return -1;
    }

    // set hostname for the container
    // if fails, print error messages
    if (sethostname("container", 9) < 0)
    {
        perror("sethostname");
    }

    // change rootfs
    if (chroot(rootfs.c_str()) < 0)
    {
        perror("chroot");
        return -1;
    }

    // change working directory to root of the new rootfs
    if (chdir("/") < 0)
    {
        perror("chdir");
        return -1;
    }

    // Create /proc directory inside the container root filesystem
    // 0555: READ and EXECUTE permissions for everyone, no WRITE permissions
    mkdir("/proc", 0555);

    // mount proc inside the container namespace
    // if fails, print error message and returns -1
    if (mount("proc", "/proc", "proc", 0, nullptr) < 0)
    {
        perror("mount proc");
        return -1;
    }

    std::vector<char*> argv;
    // Convert command arguments to char* array for execvp
    for (const auto& arg : cmd)
    {
        // const_cast is used here to convert the const char* returned by c_str() to char* required by execvp
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    // Null-terminate the arguments array for execvp
    argv.push_back(nullptr);

    // Execute the specified command inside the container
    execvp(argv[0], argv.data());

    // If execvp returns, it means there was an error
    std::cerr << "Failed to execute command: " << strerror(errno) << std::endl;
    return -1;
}

void Container::setupCgroup(pid_t pid)
{
    // Create cgroup directory for the container
    const std::string cgroupBasePath = "/sys/fs/cgroup/" + cgroupName;

    // Create the cgroup directory if it doesn't exist
    // 0755: READ, WRITE, EXECUTE permissons for owner
    //       READ, EXECUTE permissions for group and others
    // if fails, print error messages and return -1
    if (mkdir(cgroupBasePath.c_str(), 0755) == -1 && errno != EEXIST)
    {
        std::cerr << "Failed to create cgroup directory: "
                  << strerror(errno) << std::endl;
        return;
    }

    // Configure CPU limit using cgroup v2 interface.
    // cpu.max format: "<quota> <period>" (in microseconds)
    // quota = allowed CPU time within one period.
    // Example: "50000 100000" → 50% of one CPU core.
    int period = 100000;
    int quota = (cpuQuotaPct * period) / 100;
    writeFile(cgroupBasePath + "/cpu.max", std::to_string(quota) + " " + std::to_string(period));

    // Set memory
    writeFile(cgroupBasePath + "/memory.max", std::to_string(memoryLimitMB * 1024L * 1024L));
    // Add pid
    writeFile(cgroupBasePath + "/cgroup.procs", std::to_string(pid));
}

void Container::writeFile(const std::string& path, const std::string& content)
{
    // Open the file for writing
    // O_WRONLY: Open for writing only
    // O_CLOEXEC: Automatically close the file descriptor on exec() calls
    int fd = open(path.c_str(), O_WRONLY | O_CLOEXEC);

    // if open fails
    if (fd == -1)
    {
        std::cerr << "Failed to open file " << path << ": " << strerror(errno) << std::endl;
        return;
    }

    // Write the content to the file
    // if fails, print error messages
    if (write(fd, content.c_str(), content.size()) < 0)
    {
        std::cerr << "Write failed: " << strerror(errno) << std::endl;
    }

    // Close the file descriptor
    close(fd);
}

int Container::childFuncWrapper(void* arg)
{
    // Cast the argument back to a Container pointer
    Container* container = static_cast<Container*>(arg);
    // Call the child function to set up the container environment and execute the command
    return container->childFunc();
}