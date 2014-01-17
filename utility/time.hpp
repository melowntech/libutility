#ifndef utility_time_hpp_included_
#define utility_time_hpp_included_

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

} // namespace utility

#endif // utility_time_hpp_included_
