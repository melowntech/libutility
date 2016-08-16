#include <fnmatch.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <cstdlib>
#include <cerrno>
#include <system_error>

#include <boost/filesystem.hpp>

#include "dbglog/dbglog.hpp"

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

boost::filesystem::path homeDir()
{
    // first, try ${HOME}
    if (const char *home = ::getenv("HOME")) { return home; }

    // no ${HOME} -> consult user database

    struct ::passwd pwd, *dummy;
    auto buflen(::sysconf(_SC_GETPW_R_SIZE_MAX));
    std::unique_ptr<char[]> buf(new char[buflen]);

    if (-1 == ::getpwuid_r(::getuid(), &pwd, buf.get(), buflen, &dummy)) {
        std::system_error e(errno, std::system_category());
        LOG(err3) << "Cannot determine home directory (getpwuid_r failed): <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    return pwd.pw_dir;
}

} // namespace utility
