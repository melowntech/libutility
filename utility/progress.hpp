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
/** \file progress.hpp
 *
 * Simple progress information.
 */

#ifndef utility_progress_hpp_included_
#define utility_progress_hpp_included_

#include <atomic>
#include <thread>
#include <mutex>
#include <cstddef>
#include <boost/rational.hpp>

namespace utility {

class Progress {
public:
    typedef boost::rational<std::size_t> ratio_t;

    Progress(std::size_t total) : total_(total), value_(), reportedValue_() {}

    Progress& operator++() { ++value_; return *this; }

    Progress& operator+=(std::size_t inc) { value_ += inc; return *this; }

    std::size_t total() const { return total_; }

    std::size_t value() const { return value_; }

    double percentage() const { return (100. * value_) / total_; }

    ratio_t ratio() const { return { value_, total_ }; }

    void report(const ratio_t &threshold
                , const std::string &name = std::string());
    
    void reportRational(const std::string &name = std::string());

    void incrementAndReport(const ratio_t &threshold
                            , const std::string &name = std::string());

    bool full() const { return value_ >= total_; }

private:
    std::size_t total_;
    std::size_t value_;
    std::size_t reportedValue_;
};

/** Thread-safe variant
 */
namespace ts {

class Progress {
public:
    typedef boost::rational<std::size_t> Ratio;

    Progress(const std::string &name, std::size_t total
             , const Ratio &reportTreshold = Ratio(1, 100))
        : name_(name), total_(total), reportTreshold_(reportTreshold)
        , value_(0), nextReportValue_(calculateNextReportValue(0))
    {}

    Progress& operator++() { report(++value_); return *this; }

    Progress& operator+=(std::size_t inc) {
        report(value_ += inc); return *this;
    }

    std::size_t total() const { return total_; }

    std::size_t value() const { return value_; }

    double percentage() const { return (100. * value_) / total_; }

    Ratio ratio() const { return { value_, total_ }; }

private:
    std::size_t calculateNextReportValue(std::size_t v) {
        auto next(boost::rational_cast<std::size_t>
                  (total_ * (Ratio(v, total_) + reportTreshold_)));
        // add one because we can get same value for really little treshold
        return next + 1;
    }

    void report(std::size_t rv);

    const std::string name_;
    const std::size_t total_;
    const Ratio reportTreshold_;

    std::atomic<std::size_t> value_;
    std::atomic<std::size_t> nextReportValue_;
    std::mutex mutex_;
};

} // namespace ts

} // namespace utility

#endif // utility_progress_hpp_included_
