/**
 * Copyright (c) 2019 Melown Technologies SE
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

#include "dbglog/dbglog.hpp"

#include "./eventcounter.hpp"

namespace utility {

EventCounter::Counts EventCounter::standardTimes{5, 60, 300};

EventCounter::EventCounter(int size)
    : size_(size), slots_(size)
{}

void EventCounter::event(std::size_t count)
{
    const auto now(std::time(nullptr));
    const int index(now % size_);

    std::lock_guard<std::mutex> lock(mutex_);

    auto &current(slots_[index]);
    if (current.when != now) {
        current.count = count;
        current.when = now;
    } else {
        current.count += count;
    }
}

void EventCounter::eventMax(std::size_t count)
{
    const auto now(std::time(nullptr));
    const int index(now % size_);

    std::lock_guard<std::mutex> lock(mutex_);

    auto &current(slots_[index]);
    if (current.when != now) {
        current.count = count;
        current.when = now;
    } else if (count > current.count) {
        current.count =count;
    }
}

template <typename F>
std::size_t EventCounter::processBlock(std::size_t count, const F &f) const
{
    // limit to count to the number of slots, ignore current slot
    if ((count + 1) >= size_) {
        count = size_ - 1;
    }

    const auto now(std::time(nullptr));
    std::time_t start(now - count - 1);
    std::size_t index(start % size_);

    std::lock_guard<std::mutex> lock(mutex_);
    {
        for (auto time(start); time < now; ++time, ++index) {
            // handle index overflow
            if (index >= size_) { index = 0; }

            const auto &slot(slots_[index]);
            if (slot.when == time) {
                f(slot.count);
            }
        }
    }

    return count;
}

double EventCounter::average(std::size_t count) const
{
    double total(.0);

    count = processBlock
        (count, [&total](std::size_t value) { total += value; });

    return total / count;
}

std::size_t EventCounter::max(std::size_t count) const
{
    std::size_t max(0);

    processBlock
        (count, [&max](std::size_t value) {
            if (value > max) { max = value; }
        });

    return max;
}

std::size_t EventCounter::total(std::size_t count) const
{
    std::size_t total(.0);

    count = processBlock
        (count, [&total](std::size_t value) { total += value; });

    return total;
}

std::tuple<double, std::size_t> EventCounter::averageAndMax(std::size_t count)
    const
{
    double total(.0);
    std::size_t max(0);

    count = processBlock
        (count, [&total, &max](std::size_t value) {
            total += value;
            if (value > max) { max = value; }
        });

    return std::tuple<double, std::size_t>(total / count, max);
}

void EventCounter::average(std::ostream &os, const std::string &name
                           , const Counts &counts) const
{
    for (auto count : counts) {
        os << name << "avg." << count << '=' << average(count) << '\n';
    }
}

void EventCounter::total(std::ostream &os, const std::string &name
                           , const Counts &counts) const
{
    for (auto count : counts) {
        os << name << "total." << count << '=' << total(count) << '\n';
    }
}

void EventCounter::max(std::ostream &os, const std::string &name
                       , const Counts &counts) const
{
    for (auto count : counts) {
        os << name << "max." << count << '=' << max(count) << '\n';
    }
}

void EventCounter::averageAndMax(std::ostream &os, const std::string &name
                                 , const Counts &counts) const
{
    for (auto count : counts) {
        const auto am(averageAndMax(count));
        os << name << "avg." << count << '=' << std::get<0>(am) << '\n';
        os << name << "max." << count << '=' << std::get<1>(am) << '\n';
    }
}

} // namespace utility
