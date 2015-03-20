#include <cerrno>
#include <system_error>

#include <sys/time.h>
#include <sys/resource.h>

#include "dbglog/dbglog.hpp"

#include "../rlimit.hpp"

namespace utility {

std::size_t maxOpenFiles()
{
    struct rlimit rlim;
    if (-1 == ::getrlimit(RLIMIT_NOFILE, &rlim)) {
        std::system_error e
            (errno, std::system_category()
             , "Failed to get RLIMIT_NOFILE resource limit.");
        LOG(err1) << e.what();
        throw e;
    }

    return rlim.rlim_cur;
}

} // namespace utility

