#include <string>

#include <boost/test/unit_test.hpp>

#include "utility/path.hpp"

#include "dbglog/dbglog.hpp"

BOOST_AUTO_TEST_CASE(utility_path_extension)
{
    BOOST_TEST_MESSAGE("* Testing utility/path.");

    namespace fs = boost::filesystem;

    fs::path base("/usr/local/bin/script");
    auto exe1(utility::addExtension(base, ".exe"));
    auto exe2(utility::replaceOrAddExtension(base, "exe"));
    auto sh1(utility::replaceOrAddExtension(exe1, "sh"));

    LOG(info4) << "base: " << base;
    LOG(info4) << "added .exe: " << exe1;
    LOG(info4) << "added/replaced .exe: " << exe2;
    LOG(info4) << "added/replaced .exe -> .sh: " << sh1;
}
