#ifndef utility_duration_hpp_included_
#define utility_duration_hpp_included_

#include <chrono>
#include <boost/thread.hpp>

namespace utility {

class DurationMeter {
public:
    typedef std::chrono::monotonic_clock clock_t;
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

private:
    clock_t::time_point start_;
    clock_t::time_point last_;
};

typedef DurationMeter::duration_type Duration;


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
private:
    static boost::thread_specific_ptr<std::map<std::string, Stopwatch>> watches_;
};

#ifdef UTILITY_TIMERS
    #define UTILITY_TIMEMETRICS_START_COUNTER(NAME)\
        utility::TimeMetrics::startCounter(NAME)

    #define UTILITY_TIMEMETRICS_STOP_COUNTER_AND_PRINT_TOTAL(NAME)\
        LOG(info2) << utility::TimeMetrics::stopCounterAndPrintTotal(NAME);
#else
    #define UTILITY_TIMEMETRICS_START_COUNTER(NAME)          
    #define UTILITY_TIMEMETRICS_STOP_COUNTER_AND_PRINT_TOTAL(NAME)
#endif



} // namespace utility

#endif // utility_duration_hpp_included_
