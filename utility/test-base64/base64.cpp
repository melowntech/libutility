#include <cstdlib>
#include <iostream>

#include "utility/base64.hpp"

std::string readStdin()
{
    std::string data;

    char buf[1024];
    do {
        std::cin.read(buf, sizeof(buf));
        data.append(buf, buf + std::cin.gcount());
    } while (!std::cin.eof());
    return data;
}

int encode()
{
    auto data(readStdin());
    std::cout << utility::base64::encode(data) << std::endl;
    return EXIT_SUCCESS;
}

int decode()
{
    auto data(readStdin());
    std::cout << utility::base64::decode(data) << std::flush;
    return EXIT_SUCCESS;
}

int usage(const char *name)
{
    std::cerr << "usage: \n"
              << "    " << name << " encode\n"
              << "    " << name << " decode\n"
              << std::endl;
    return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
    if (argc != 2) { return usage(argv[0]); }
    const std::string mode(argv[1]);
    if (mode == "encode") {
        return encode();
    } else if (mode == "decode") {
        return decode();
    }

    return usage(argv[0]);
}

