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
#ifndef utility_duration_hpp_included_
#define utility_duration_hpp_included_

#include <chrono>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#include "steady-clock.hpp"

namespace utility {

class DurationMeter {
public:
    typedef utility::steady_clock clock_t;

    typedef clock_t::duration duration_type;

    DurationMeter()
        : start_(clock_t::now()), last_(start_)
    {
    }

    duration_type duration() {
        last_ = clock_t::now();
        return (last_ - start_);
    }

    duration_type fromLast() {
        clock_t::time_point last(last_);
        last_ = clock_t::now();
        return (last_ - last);
    }

    void update() {
        last_ = clock_t::now();
    }

    void reset() {
        last_ = start_ = clock_t::now();
    }

private:
    clock_t::time_point start_;
    clock_t::time_point last_;
};

typedef DurationMeter::duration_type Duration;

/** Measure duration of operation `op`.
 */
template <typename Op>
Duration measureDuration(const Op &op)
{
    DurationMeter dm;
    op();
    return dm.duration();
}

//static class for counter registering .. NOT threadsafe
class TimeMetrics {
public:
    typedef std::pair<DurationMeter, Duration> Stopwatch;
    //registers counter upon the first call
    static void startCounter(std::string name){
        if(!watches_.get()){
            watches_.reset(new std::map<std::string, Stopwatch>());
        }
        (*watches_)[name].first.update();
    }
    
    static std::string stopCounterAndPrintTotal(std::string name){
        if(!watches_.get()){
            watches_.reset(new std::map<std::string, Stopwatch>());
        }
        if (watches_->find(name) == watches_->end()){
            return "TimeMetrics: counter "+name+" not found!";
        }else{
            (*watches_)[name].second += (*watches_)[name].first.fromLast();
            return "TimeMetrics: "+name+" total: "
                    +boost::lexical_cast<std::string>(
                        (*watches_)[name].second.count());
        }
    }
    static void resetCounter(std::string name){
        if (watches_->find(name) == watches_->end()){
            return;
        }
        watches_->erase(name);
    }
private:
    static boost::thread_specific_ptr<std::map<std::string, Stopwatch>> watches_;
};

#ifdef UTILITY_TIMERS
    #define UTILITY_TIMEMETRICS_START_COUNTER(NAME)\
        utility::TimeMetrics::startCounter(NAME)

    #define UTILITY_TIMEMETRICS_RESET_COUNTER(NAME)\
        utility::TimeMetrics::resetCounter(NAME)

    #define UTILITY_TIMEMETRICS_STOP_COUNTER_AND_PRINT_TOTAL(NAME)\
        LOG(info2) << utility::TimeMetrics::stopCounterAndPrintTotal(NAME);
#else
    #define UTILITY_TIMEMETRICS_START_COUNTER(NAME) 
    #define UTILITY_TIMEMETRICS_RESET_COUNTER(NAME)   
    #define UTILITY_TIMEMETRICS_STOP_COUNTER_AND_PRINT_TOTAL(NAME)
#endif



} // namespace utility

#endif // utility_duration_hpp_included_
