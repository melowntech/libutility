#include "dbglog/dbglog.hpp"

namespace utility {

MemInfo meminfo()
{
    LOGTHROW(err2, std::runtime_error)
        << "Function meminfo unsupported on this platform.";
    throw;
}

} // namespace utility
