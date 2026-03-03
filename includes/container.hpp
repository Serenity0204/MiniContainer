#pragma once
#include <iostream>
#include <vector>

/**
 * @brief A simple container implementation using Linux
 * namespaces and cgroups to isolate and limit resources for a process.
 *
 * This class provides lightweight container functionality,
 * allowing you to run a command in an isolated environment with specified CPU and memory limits.
 *
 */
class Container
{
public:
    /**
     * @brief  Construct a new Container instance.
     *
     * @param rootfs Path to the container root filesystem
     * @param cmd Command and arguments to execute inside the container.
     * @param cgname       Name of the cgroup used to limit resources.
     * @param cpu_limit    CPU usage limit (percentage, e.g., 50 means 50% of one CPU core).
     *  @param mem_limit    Memory limit in MB.
     */
    Container(const std::string& rootfs, const std::vector<std::string>& cmd, const std::string& cgname, int cpu_limit, long mem_limit);

    /**
     * @brief Destroy container instance
     *
     */
    ~Container();

    /**
     * @brief Run() creates a new child process using clone() with multiple
     * namespace isolation flags.
     *
     * The parent process:
     *   - Configures cgroup resource limits
     *   - Waits for the child process to exit
     *
     * The child process:
     *   - Runs inside isolated namespaces
     *   - Sets up filesystem and hostname
     *   - Executes the target command
     */
    void run();

private:
    // Stack size for the child process
    static constexpr int STACK_SIZE = 1024 * 1024;

    // Child process stack
    char* childStack;

    // Root filesystem path for the container.
    std::string rootfs;

    // Command and arguments to execute inside the container.
    std::vector<std::string> cmd;
    //  Name of the cgroup used for resource limitation.
    std::string cgroupName;
    // CPU quota limit as a percentage of a single core.
    int cpuQuotaPct;
    // Memory limit in megabytes.
    long memoryLimitMB;

    /**
     * @brief Entry function for the child process created by clone().
     *
     *Steps:
     * 1. Make mount propagation private to avoid affecting the host.
     * 2. Set container hostname (UTS namespace).
     * 3. Change root filesystem using chroot().
     * 4. Mount /proc for process visibility.
     * 5. Replace the process image with execvp().
     *
     * @return int Exit status of the executed command.
     */
    int childFunc();

    /**
     * @brief Configure cgroup resource limits for the given process.
     *
     * CPU control:
     *   cpu.max format: "<quota> <period>"
     *   Example: "50000 100000" → 50% CPU
     *
     * Memory control:
     *   memory.max format: "<bytes>"
     *
     * This function adds specified PID to the cgroup and applies CPU and memory
     * limits according to cpuQuotaPct and memoryLimitMB.
     *
     * @param pid process ID of the child process
     */
    void setupCgroup(pid_t pid);

    /**
     * @brief Write content to a file.
     *
     * This utility function is used to write configuration values to
     * cgroup files for setting resource limits.
     *
     * @param path Path to the file to write to.
     * @param content Content to write into the file.
     */
    void writeFile(const std::string& path, const std::string& content);

    /**
     * @brief Static wrapper function for childFunc to be used with clone().
     *
     * @param arg Pointer to the Container instance (passed as void* for clone compatibility).
     * @return int Exit status of the child process execution.
     */
    static int childFuncWrapper(void* arg);
};