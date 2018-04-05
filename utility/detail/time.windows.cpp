/**
 * Copyright (c) 2018 Melown Technologies SE
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

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <time.h>

#include <cstdio>

#include "../time.hpp"

namespace utility {

std::string formatDateTime(const std::time_t t, bool gmt)
{
    struct tm tm;

    if (gmt) {
        ::gmtime_s(&tm, &t);
    } else {
        ::localtime_s(&tm, &t);
    }

    char buf[128];
    ::strftime(buf, sizeof(buf) - 1, "%Y-%m-%d %T", &tm);
    return buf;
}

namespace {
const std::int64_t unixTimeStart(0x019DB1DED53E8000); // epoch start
const std::int64_t ticksPerSecond = 10000000; // a tick is 100ns}
const std::int64_t ticksPerMicrosecond = 10; // a tick is 100ns}
} // namespace

std::pair<std::uint64_t, std::uint64_t> currentTime()
{
    FILETIME now;
    ::GetSystemTimeAsFileTime(&now);

    LARGE_INTEGER li;
    li.LowPart  = now.dwLowDateTime;
    li.HighPart = now.dwHighDateTime;

    const auto diff(li.QuadPart - unixTimeStart);

    return { diff / ticksPerSecond
            , (diff % ticksPerSecond) / ticksPerMicrosecond };
}

std::uint64_t usecFromEpoch()
{
    FILETIME now;
    ::GetSystemTimeAsFileTime(&now);

    LARGE_INTEGER li;
    li.LowPart  = now.dwLowDateTime;
    li.HighPart = now.dwHighDateTime;
    const auto diff(li.QuadPart - unixTimeStart);

    return diff / ticksPerMicrosecond;
}

} // namespace utility

