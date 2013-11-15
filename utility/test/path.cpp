#include <string>

#include <boost/test/unit_test.hpp>

#include "utility/path.hpp"

#include "dbglog/dbglog.hpp"

namespace {

void replaceOrAddExtension(const boost::filesystem::path &path
                           , const boost::filesystem::path &ext)
{
    auto npath(utility::replaceOrAddExtension(path, ext));
    LOG(info4) << path << " + " << ext << " -> " << npath;
}

} // namespace

BOOST_AUTO_TEST_CASE(utility_path_extension)
{
    BOOST_TEST_MESSAGE("* Testing utility/path.");

    namespace fs = boost::filesystem;

    replaceOrAddExtension("/usr/local/bin/script", "exe");
    replaceOrAddExtension("/usr/local/bin/script", ".exe");
    replaceOrAddExtension("/usr/local/bin/script.exe", "sh");
    replaceOrAddExtension("/usr/local/bin/script.exe", ".sh");
    replaceOrAddExtension("/usr/local/bin/script.exe", "tar.gz");
    replaceOrAddExtension("/usr/local/bin/script.exe", ".tar.gz");
    replaceOrAddExtension("/usr/local/bin/.hidden", "sh");
    replaceOrAddExtension("/usr/local/bin/.hidden", ".sh");
    replaceOrAddExtension("/usr/local/bin/.hidden.sh", "exe");
    replaceOrAddExtension("/usr/local/bin/.hidden.sh", ".exe");
    replaceOrAddExtension("/usr/local/bin/archive.tar.gz", ".bzip2");
    replaceOrAddExtension("/usr/local/bin/archive.tar.gz", "");
}
