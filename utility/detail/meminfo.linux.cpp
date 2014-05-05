#include <cerrno>
#include <memory>
#include <system_error>

#include <sys/sysinfo.h>

#include "dbglog/dbglog.hpp"

#include "../meminfo.hpp"

namespace utility {

MemInfo meminfo()
{
    struct ::sysinfo si;

    if (-1 == ::sysinfo(&si)) {
        std::system_error e(errno, std::system_category());
        LOG(err1) << "sysinfo call failed: <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    return {
        {
            std::size_t(si.totalram)
            , std::size_t(si.freeram)
            , std::size_t(si.bufferram)
        }, {
            std::size_t(si.totalswap)
            , std::size_t(si.freeswap)
            , 0
        }
    };
}

} // namespace utility
