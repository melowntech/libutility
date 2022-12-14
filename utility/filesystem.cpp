/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <sys/types.h>
#include <sys/stat.h>

#include <cerrno>
#include <system_error>

#include <boost/filesystem/operations.hpp>

#include "dbglog/dbglog.hpp"

#include "filesystem.hpp"
#include "streams.hpp"
#include "path.hpp"

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

std::vector<fs::path> scanDir(const fs::path &root)
{
    std::vector<fs::path> results;

    for (fs::recursive_directory_iterator
             iroot(root, fs::symlink_option::recurse)
             , eroot;
         iroot != eroot; ++iroot)
    {
        const auto status(iroot->status());
        if (status.type() == fs::file_type::directory_file) { continue; }

        const auto &path(iroot->path());
        const auto local(cutPathPrefix(path, root));

        results.emplace_back(local);
    }

    return results;
}

std::map<std::string, fs::path> id2path(const std::vector<fs::path> &localPaths)
    std::map<std::string, fs::path> result;
    for (const auto & local : localPaths) {
        const auto id((local.parent_path() / local.stem()).generic_string());
        result[id] = local;
    }
    return result;
}

} // namespace utility
