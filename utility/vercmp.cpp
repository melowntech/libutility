/**
 * Copyright (c) 2022 Melown Technologies SE
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

#include <cctype>

#include "vercmp.hpp"

namespace utility {

namespace {

inline int characterOrder(int c) {
    if (std::isdigit(c)) { return 0; }
    if (std::isalpha(c)) { return c; }
    if (c) { return c + 256; }
    return 0;
}

} // namespace

int versionCompare(const std::string &a, const std::string &b)
{
    const char *pa(a.data());
    const char *pb(b.data());

    while (*pa || *pb) {
        int diff = 0;

        // compare non-digits
        while ((*pa && !std::isdigit(*pa)) || (*pb && !std::isdigit(*pb))) {
            int aco = characterOrder(*pa);
            int bco = characterOrder(*pb);

            if (aco != bco) { return aco - bco; }
            ++pa; ++pb;
        }

        // consume leading zeroes
        while (*pa == '0') { ++pa; }
        while (*pb == '0') { ++pb; }

        // compare numbers
        while (std::isdigit(*pa) && std::isdigit(*pb)) {
            if (!diff) { diff = *pa - *pb; }
            ++pa; ++pb;
        }

        if (std::isdigit(*pa)) { return 1; }
        if (std::isdigit(*pb)) { return -1; }
        if (diff) { return diff; }
    }

    return 0;
}

} // namespace utility
