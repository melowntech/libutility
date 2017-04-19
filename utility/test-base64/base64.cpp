/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
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

