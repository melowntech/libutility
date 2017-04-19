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
#ifndef utility_time_hpp_included_
#define utility_time_hpp_included_

#include <cstdint>
#include <utility>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include "steady-clock.hpp"

namespace utility {

template <typename TimeDuration>
std::string formatDuration(const TimeDuration &d)
{
    using namespace std::chrono;

    std::ostringstream os;
    os << std::setw(2) << std::setfill('0')
       << duration_cast<hours>(d).count() << ':'
       << std::setw(2) << std::setfill('0')
       << (duration_cast<minutes>(d).count() % 60) << ':'
       << std::setw(2) << std::setfill('0')
       << (duration_cast<seconds>(d).count() % 60) << '.'
       << std::setw(6) << std::setfill('0')
       << (duration_cast<microseconds>(d).count() % 1000000);

    return os.str();
}

std::string formatDateTime(const std::time_t t, bool gmt = false);

std::pair<std::uint64_t, std::uint64_t> currentTime();

std::uint64_t usecFromEpoch();

} // namespace utility

#endif // utility_time_hpp_included_
