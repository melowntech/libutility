/**
 * Copyright (c) 2023 Melown Technologies SE
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

#pragma once

#include <vector>
#include <iostream>
#include <memory>
#include <chrono>

#include "json/json.hpp"

namespace utility {
class TraceTimer
{
    // TODO: use utc_clock since C++20
    using Clock = std::chrono::system_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    public:
        TraceTimer(const std::string _name): name(_name)
        {
            records.emplace_back(name, true);
        }

        ~TraceTimer()
        {
            stop();
        }

        void stop()
        {
            if (stopped)
                return;
            else
                stopped = true;

            records.emplace_back(name, false);
        }

        static void dump()
        {
            nlohmann::json array;
            for (const TraceRecord& record : records) {
                const auto ts
                    = std::chrono::duration_cast<std::chrono::microseconds>(
                          record.time.time_since_epoch())
                          .count();
                array.push_back({
                    { "name", record.name},
                    { "ts", ts },
                    { "ph", record.isStart ? "B" : "E" }
                     });
            }

            nlohmann::json json;
            json["traceEvents"] = array;

            std::ofstream f("trace.json");
            f << json;
        }

    private:
        struct TraceRecord
        {
            TraceRecord(const std::string &_name, bool _isStart): name(_name), isStart(_isStart)
            {
                time = Clock::now();
            }

            std::string name;
            TimePoint time;
            bool isStart;
        };

        static inline std::vector<TraceRecord> records;

        std::string name;
        bool stopped = false;
};

} // namespace utility
