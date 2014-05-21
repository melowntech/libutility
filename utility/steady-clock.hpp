#ifndef utility_steady_clock_hpp_included_
#define utility_steady_clock_hpp_included_

#include <chrono>

namespace utility {

#if defined(__GNUC__) && (__GNUC__ == 4) && (__GNUC_MINOR__ < 7) && !defined(__APPLE__)
    // gcc 4.6, pre standard
    typedef std::chrono::monotonic_clock steady_clock;
#else
    // conforming to standard
    typedef std::chrono::steady_clock steady_clock;
#endif

} // namespace utility

#endif // utility_steady_clock_hpp_included_
