#include "map.hpp"
#include "has_member.hpp"
#include "duration.hpp"

namespace utility {
    boost::thread_specific_ptr<std::map<std::string, TimeMetrics::Stopwatch>> 
            TimeMetrics::watches_;
} // namespace utility
