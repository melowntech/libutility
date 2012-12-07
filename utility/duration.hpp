#ifndef utility_duration_hpp_included_
#define utility_duration_hpp_included_

#include <chrono>

class DurationMeter {
public:
    typedef std::chrono::monotonic_clock clock_t;

    DurationMeter()
        : start_(clock_t::now()), last_(start_)
    {
    }

    clock_t::duration duration() {
        last_ = clock_t::now();
        return (last_ - start_);
    }

    clock_t::duration fromLast() {
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

#endif // utility_duration_hpp_included_
