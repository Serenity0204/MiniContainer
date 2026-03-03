#include "../includes/container.hpp"

int main(int argc, char* argv[])
{
    // Check if the required arguments are provided
    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " ROOTFS CGROUP_NAME CMD [ARGS...]\n";
        return 1;
    }

    // get rootfs and cgroup name from command line arguments
    std::string rootfs = argv[1];
    std::string cgname = argv[2];

    std::vector<std::string> cmd;

    // get command and arguments from command line arguments
    for (int i = 3; i < argc; i++)
    {
        cmd.push_back(argv[i]);
    }

    // create and run the container
    Container c(rootfs, cmd, cgname, 50, 256);
    c.run();

    return 0;
}