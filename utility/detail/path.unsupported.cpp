#include "../path.hpp"

namespace utility {

boost::optional<boost::filesystem::path> exePath()
{
    // unable to find out
    return boost::none;
}

} // namespace utility
