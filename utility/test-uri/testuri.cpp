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

#include "dbglog/dbglog.hpp"
#include "utility/uri.hpp"

void log(const utility::Uri &uri)
{
    auto c(uri.components());
    LOG(info4) << "    str(uri): " << str(uri);
    LOG(info4) << "    uri.scheme: " << c.scheme;
    LOG(info4) << "    uri.user: " << c.user;
    LOG(info4) << "    uri.password: " << c.password;
    LOG(info4) << "    uri.host: " << c.host;
    LOG(info4) << "    uri.port: " << c.port;
    LOG(info4) << "    uri.path: " << c.path;
    LOG(info4) << "    uri.search: " << c.search;
    LOG(info4) << "    uri.fragment: " << c.fragment;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        LOG(fatal) << "Missing parameters.";
        return EXIT_FAILURE;
    }

    utility::Uri base(argv[1]);
    LOG(info4) << "Base:";
    log(base);

    if (argc < 3) { return EXIT_SUCCESS; }

    utility::Uri relative(argv[2]);
    LOG(info4) << "Relative:";
    log(relative);

    auto resolved(base + relative);
    LOG(info4) << "Resolved:";
    log(resolved);

    return EXIT_SUCCESS;
}
