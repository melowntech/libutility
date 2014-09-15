#include <boost/filesystem/operations.hpp>

#include "filesystem.hpp"
#include "streams.hpp"

namespace fs = boost::filesystem;

namespace utility {

void copyTree(const fs::path &from, const fs::path &to)
{
    auto s(symlink_status(from));

    if (is_symlink(s)) {
        copy_symlink(from, to);
    } else if (is_directory(s)) {
        copy_directory(from, to);
    } else if (is_regular_file(s)) {
        detail::copy_file(from, to, false);
    } else {
        // wtf?
    }

    // copy directory contents
    if (!is_directory(s)) { return; }

    for (fs::directory_iterator ifrom(from), efrom; ifrom != efrom; ) {
        auto file(ifrom->path());
        copyTree(file, to / file.filename());
        ++ifrom;
    }
}

void copyTree(const fs::path &from, const fs::path &to
              , boost::system::error_code &ec)
{
    ec.clear();

    auto s(symlink_status(from, ec));
    if (ec) { return; }

    if (is_symlink(s)) {
        copy_symlink(from, to, ec);
    } else if (is_directory(s)) {
        copy_directory(from, to, ec);
    } else if (is_regular_file(s)) {
        detail::copy_file(from, to, false, ec);
    } else {
        // wtf?
    }
    if (ec) { return; }

    // copy directory contents
    if (!is_directory(s)) { return; }

    for (fs::directory_iterator ifrom(from), efrom; ifrom != efrom; ) {
        auto file(ifrom->path());
        copyTree(file, to / file.filename(), ec);
        if (ec) { return; }

        ifrom.increment(ec);
        if (ec) { return; }
    }
}

void processFile( std::istream &is
                , std::ostream &os
                , const LineProcessor &lineProcessor)
{
    std::string line;

    std::size_t index(0);
    bool useProcessor(true);

    while (getline(is, line)) {
        std::pair<std::string, LineProcessorResult> result;

        if (useProcessor) {
            result = lineProcessor(line, index);
        } else {
            result = std::make_pair(line, LineProcessorResult::next);
        }

        ++index;
        os << result.first << "\n";

        if (result.second == LineProcessorResult::stop) {
            break;
        } else if (result.second == LineProcessorResult::pass) {
            //os << is.rdbuf(); // throws for some reason
            useProcessor = true;
        }
    }

}

void processFile( const boost::filesystem::path &from
                , const boost::filesystem::path &to
                , bool overwrite
                , const LineProcessor &lineProcessor)
{
    boost::system::error_code ec;
    fs::exists(to, ec);

    if (!overwrite && !ec) {
        throw std::runtime_error("Attempt to overwrite existing file.");
    }

    std::ifstream f;
    f.exceptions(std::ios::badbit | std::ios::failbit);
    f.open(from.string(), std::ios_base::in);
    f.exceptions(std::ios::badbit);

    utility::ofstreambuf of;
    of.exceptions(std::ios::badbit | std::ios::failbit);
    of.open(to.string(), std::ios_base::out | std::ios_base::trunc);

    processFile(f, of, lineProcessor);

    of.close();
}

} // namespace utility
