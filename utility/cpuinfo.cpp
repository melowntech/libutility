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

#include <system_error>

#ifdef __linux__
#  include <sched.h>
#endif

#include <boost/thread.hpp>

#include "dbglog/dbglog.hpp"

#include "cpuinfo.hpp"

namespace utility {

namespace {

std::size_t boostThreadCpuCount()
{
    // TODO: use sched_getaffinity instead
    auto hc(boost::thread::hardware_concurrency());
    if (!hc) { return 1; }
    return hc;
}

} // namespace

#ifdef __linux__

std::size_t cpuCount()
{
    cpu_set_t set;
    auto res(::sched_getaffinity(0, sizeof(set), &set));
    if (res == -1) {
        std::system_error e(errno, std::system_category());
        LOG(warn1)
            << "Unable to get CPU count using scheduler affinity ("
            << e.code() << ", " << e.what()
            << "), reverting to boost::thread.";
        return boostThreadCpuCount();
    }
    return CPU_COUNT(&set);
}

#else // __linux__

std::size_t cpuCount()
{
    return boostThreadCpuCount();
}

#endif // __linux__

} // namespace utility
