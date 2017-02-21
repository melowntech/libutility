#include <sys/time.h>
#include <sys/resource.h>

#include <system_error>

#include "dbglog/dbglog.hpp"

#include "./limits.hpp"

namespace utility {

bool unlimitedCoredump()
{
    // first, try to set unlimited value
    struct rlimit limit;
    limit.rlim_max = limit.rlim_cur = RLIM_INFINITY;
    if (0 == ::setrlimit(RLIMIT_CORE, &limit)) {
        // fine, we can do it
        return true;
    }

    // damn, we do not have sufficient privileges
    // fetch current settings
    if (-1 == ::getrlimit(RLIMIT_CORE, &limit)) {
        std::system_error e(errno, std::system_category());
        LOG(err1) << "Cannot get core rlimit: <"
                  << e.code() << ", " << e.what() << ">.";
        return false;
    }

    // set soft limit to hard limit
    limit.rlim_cur = limit.rlim_max;

    // and store again
    if (-1 == ::setrlimit(RLIMIT_CORE, &limit)) {
        std::system_error e(errno, std::system_category());
        LOG(err1) << "Cannot set core rlimit to {"
                  << limit.rlim_cur << ", " << limit.rlim_max << "}: "
                  << e.code() << ", " << e.what() << ">.";
        return false;
    }

    return true;
}

} // namespace utility
