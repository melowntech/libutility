/**
 * Copyright (c) 2021 Melown Technologies SE
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

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <memory>
#include <cstring>
#include <climits>

#include "dbglog/dbglog.hpp"

#include "../hostname.hpp"

namespace utility {

std::string hostname()
{
    char hn[HOST_NAME_MAX + 1];
    hn[HOST_NAME_MAX] = '\0';
    ::gethostname(hn, sizeof(hn));
    return std::string(hn);
}

std::string fqdn()
{
    const auto hn(hostname());

    struct ::addrinfo hints;
    ::memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_CANONNAME;

    struct ::addrinfo *res = nullptr;
    auto ret(::getaddrinfo(hn.c_str(), nullptr, &hints, &res));
    std::shared_ptr<struct ::addrinfo> guard(res, [](auto *ai)
    {
        if (ai) { ::freeaddrinfo(ai); }
    });

    if (ret != 0) {
        LOGTHROW(err1, std::runtime_error)
            << "Unable to get FQDN for localhost (" << hn
            << "): " << ::gai_strerror(ret);
    }

    return std::string(res->ai_canonname);
}

} // namespace utility
