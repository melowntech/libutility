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
#include <sys/time.h>

#include "../time.hpp"

namespace utility {

std::string formatDateTime(const std::time_t t, bool gmt)
{
    std::tm tm;
    if (gmt) {
        ::gmtime_r(&t, &tm);
    } else {
        ::localtime_r(&t, &tm);
    }
    char buf[128];
    ::strftime(buf, sizeof(buf) - 1, "%Y-%m-%d %T", &tm);
    return buf;
}

std::pair<std::uint64_t, std::uint64_t> currentTime()
{
    timeval now;
    ::gettimeofday(&now, 0x0);
    return { now.tv_sec, now. tv_usec };
}

std::uint64_t usecFromEpoch()
{
    timeval now;
    ::gettimeofday(&now, 0x0);
    return std::uint64_t(now.tv_sec) * 1000000 + now.tv_usec;
}

} // namespace utility

