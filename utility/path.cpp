#include <fnmatch.h>

#include <boost/filesystem.hpp>

#include "path.hpp"

namespace fs = boost::filesystem;

namespace utility {

bool match(const std::string &globPattern
           , const boost::filesystem::path &path
           , int flags)
{
    int fnFlags(0x0);
    if (flags & FileMatch::icase) {
        fnFlags |= FNM_CASEFOLD;
    }

    auto res(::fnmatch(globPattern.c_str(), path.string().c_str(), fnFlags));

    if (!res) { return true; }
    if (res != FNM_NOMATCH) {
        boost::system::error_code ec(boost::system::errc::invalid_argument
                                     , boost::system::system_category());
        throw fs::filesystem_error("Error matching with pattern " + globPattern
                                   , path, ec);
    }

    return false;
}

} // namespace utility
