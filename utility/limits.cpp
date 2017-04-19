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
#include <sys/resource.h>

#include <system_error>

#include "dbglog/dbglog.hpp"

#include "./limits.hpp"

namespace utility {

bool unlimitedCoredump()
{
    // first, try to set unlimited value
    struct rlimit limit;
    limit.rlim_max = limit.rlim_cur = RLIM_INFINITY;
    if (0 == ::setrlimit(RLIMIT_CORE, &limit)) {
        // fine, we can do it
        return true;
    }

    // damn, we do not have sufficient privileges
    // fetch current settings
    if (-1 == ::getrlimit(RLIMIT_CORE, &limit)) {
        std::system_error e(errno, std::system_category());
        LOG(err1) << "Cannot get core rlimit: <"
                  << e.code() << ", " << e.what() << ">.";
        return false;
    }

    // set soft limit to hard limit
    limit.rlim_cur = limit.rlim_max;

    // and store again
    if (-1 == ::setrlimit(RLIMIT_CORE, &limit)) {
        std::system_error e(errno, std::system_category());
        LOG(err1) << "Cannot set core rlimit to {"
                  << limit.rlim_cur << ", " << limit.rlim_max << "}: "
                  << e.code() << ", " << e.what() << ">.";
        return false;
    }

    return true;
}

} // namespace utility
