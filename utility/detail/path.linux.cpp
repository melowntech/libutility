#include <boost/filesystem.hpp>

#include "dbglog/dbglog.hpp"

#include "../path.hpp"

namespace utility {

boost::optional<boost::filesystem::path> exePath()
{
    // on linux, exe is pointed by /proc/self/exe
    return boost::filesystem::read_symlink("/proc/self/exe");
}

} // namespace utility
