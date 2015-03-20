#include <stdexcept>

#include "dbglog/dbglog.hpp"

#include "../rlimit.hpp"

namespace utility {

std::size_t maxOpenFiles()
{
    LOGTHROW(err1, std::runtime_error)
        << "No support for utility::maxOpenFiles on this platform.";
    return 0;
}

} // namespace utility
