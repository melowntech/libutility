#include <boost/filesystem/operations.hpp>

#include "filesystem.hpp"

namespace fs = boost::filesystem;

namespace utility {

void copyTree(const fs::path &from, const fs::path &to, bool overwrite)
{
    auto s(symlink_status(from));

    create_directories(to.parent_path());

    if (is_symlink(s)) {
        copy_symlink(from, to);
    } else if (is_directory(s)) {
        copy_directory(from, to);
    } else if (is_regular_file(s)) {
        detail::copy_file(from, to, overwrite);
    } else {
        // wtf?
    }

    // copy directory contents
    if (!is_directory(s)) { return; }

    for (fs::directory_iterator ifrom(from), efrom; ifrom != efrom; ) {
        auto file(*ifrom);
        copyTree(from / file, to / file, overwrite);
        ++ifrom;
    }
}

void copyTree(const fs::path &from, const fs::path &to
              , bool overwrite, boost::system::error_code &ec)
{
    ec.clear();

    auto s(symlink_status(from, ec));
    if (ec) { return; }

    create_directories(to.parent_path(), ec);
    if (ec) { return; }

    if (is_symlink(s)) {
        copy_symlink(from, to, ec);
    } else if (is_directory(s)) {
        copy_directory(from, to, ec);
    } else if (is_regular_file(s)) {
        detail::copy_file(from, to, overwrite, ec);
    } else {
        // wtf?
    }
    if (ec) { return; }

    // copy directory contents
    if (!is_directory(s)) { return; }

    for (fs::directory_iterator ifrom(from), efrom; ifrom != efrom; ) {
        auto file(*ifrom);
        copyTree(from / file, to / file, overwrite, ec);
        if (ec) { return; }

        ifrom.increment(ec);
        if (ec) { return; }
    }
}

} // namespace utility
