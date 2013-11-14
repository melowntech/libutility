#include <cstdlib>
#include <iostream>

#include "utility/filesystem.hpp"

int main(int argc, char *argv[])
{
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " from to" << std::endl;
        return EXIT_FAILURE;
    }
    try {
        utility::copyTree(argv[1], argv[2]);
    } catch (const std::exception &e) {
        std::cerr << "Failed: " << e.what() << std::endl;
    }
}
