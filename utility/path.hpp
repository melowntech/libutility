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

/**
 * @file detail/path.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Boost.Filesystem path support
 */

#ifndef utility_path_hpp_included_
#define utility_path_hpp_included_

#include <utility>
#include <algorithm>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/optional.hpp>

#include "utility/gccversion.hpp"

#include "detail/path.hpp"

namespace utility {

/** Join any number of Boost.Filesystem paths to one path by operator/
 */
template <typename ...Paths>
boost::filesystem::path joinPaths(Paths &&...tail);

boost::filesystem::path
addExtension(const boost::filesystem::path &path
             , const boost::filesystem::path &ext);


// implementation

template <typename ...Paths>
inline boost::filesystem::path joinPaths(Paths &&...tail)
{
    boost::filesystem::path res;
    detail::joinPaths(res, std::forward<Paths>(tail)...);
    return res;
}

inline boost::filesystem::path
addExtension(const boost::filesystem::path &path
                    , const boost::filesystem::path &ext)
{
    return path.parent_path() / (path.filename().generic_string() + ext.generic_string());
}

/** Replaces file's extension with new one.
 *  If file doesn't have an extentsion new extension is appended.
 *  Leading dot in `ext` is optional.
 *  Leading dot is never considered as a start of an extension.
 *  Empty `ext` causes extension removal.
 */
inline boost::filesystem::path
replaceOrAddExtension(const boost::filesystem::path &path
                      , const boost::filesystem::path &ext)
{
    auto e(ext.generic_string());
    if (!e.empty() && (e[0] != '.')) {
        // no leading dot: add
        e = "." + e;
    }

    if (!path.has_extension()) {
        // no extension -> add it
        return addExtension(path, e);
    }
    auto dir(path.parent_path());
    auto stem(path.stem());
    if (stem.empty()) { stem = path.filename(); }
    return dir / (stem.generic_string() + e);
}

inline boost::filesystem::path
addFilenameSuffix(const boost::filesystem::path &path
                  , const std::string &suffix)
{
    return path.parent_path() /
           (path.stem().generic_string() + suffix + path.extension().generic_string());
}

inline boost::filesystem::path toLower(const boost::filesystem::path &path)
{
    return boost::algorithm::to_lower_copy(path.generic_string());
}

constexpr int ExactMatch = 2;

inline int isPathPrefix(const boost::filesystem::path &path
                        , const boost::filesystem::path &prefix)
{
    auto mismatch(std::mismatch(path.begin(), path.end(), prefix.begin()));

    if (mismatch.second != prefix.end()) { return false; }
    return (mismatch.first == path.end()) ? ExactMatch : true;
}

/** Cut prefix from path. Prerequisity: isPathPrefix(path, prefix) == true
 */
inline boost::filesystem::path
cutPathPrefix(const boost::filesystem::path &path
              , const boost::filesystem::path &prefix)
{
    boost::filesystem::path res;
    for (auto i(std::mismatch(path.begin(), path.end(), prefix.begin()).first)
             , e(path.end()); i != e; ++i)
    {
        res /= *i;
    }
    return res;
}


/**
 * Cut prefix when isPathPrefix(path, prefix) == true, otherwise returns
 * unchanged path
 */
inline boost::filesystem::path
    cutPathPrefixIfPossible(const boost::filesystem::path& path,
                            const boost::filesystem::path& prefix)
{
    if (isPathPrefix(path, prefix)) { return cutPathPrefix(path, prefix); }
    return path;
}

/** Filename matching.
 */
namespace FileMatch {
    enum {
        icase = 0x1   //!< case-insensitive matching
        , pathname = 0x2   //!< match '/' only against '/'
    };
}

/** Match path againts glob pattern.
 */
bool match(const std::string &globPattern
           , const boost::filesystem::path &path
           , int flags = 0x0);

/** Returns index-th component of path.
 *  Remember that 0-th component of absolute path is '/' on UNIX!
 *
 * \param path path to analyze
 * \param index index of path to return
 * \return index-th component or boost::none if index is out of bounds
 */
inline boost::optional<boost::filesystem::path>
pathComponent(const boost::filesystem::path &path
              , unsigned int index)
{
    for (const auto &c : path) {
        if (!index--) {
            return c;
        }
    }
    return boost::none;
}

inline std::size_t
numberOfPathComponents(const boost::filesystem::path &path)
{
    return std::distance(path.begin(), path.end());
}

/** Returns path to this executable, if known.
 */
boost::optional<boost::filesystem::path> exePath();

/** Returns home directory.
 */
boost::filesystem::path homeDir();

/** Path/id sanitizer options.
 */
struct SanitizerOptions {
    /** Converts any script into Latin.
     */
    bool latinize;

    /** Convert result to lower case.
     */
    bool lowercase;

    /** Replace sequence of non-alphanum characters with dash
     */
    bool dashNonAlphanum;

    /** Removes accents.
     */
    bool removeAccents;

    /** Replace all space separators by single space.
     */
    bool singleSpace;

    SanitizerOptions(bool lowercase = false)
        : latinize(true)
        , lowercase(lowercase)
        , dashNonAlphanum(true)
        , removeAccents(true)
        , singleSpace(false)
    {}
};

boost::filesystem::path
sanitizePath(const boost::filesystem::path &path
             , const SanitizerOptions &options = SanitizerOptions())
#ifndef UTILITY_HAS_ICU
    UTILITY_FUNCTION_ERROR("Path sanitization is available only when compiled "
                           "with libicu.")
#endif
    ;

std::string sanitizeId(const std::string &id
                       , const SanitizerOptions &options = SanitizerOptions())
#ifndef UTILITY_HAS_ICU
    UTILITY_FUNCTION_ERROR("Id sanitization is available only when compiled "
                           "with libicu.")
#endif
    ;

// Calls either path.lexically_relative(base) or uses own implementation based
// on Boost version
boost::filesystem::path
lexically_relative(const boost::filesystem::path &path
                   , const boost::filesystem::path &base);

void rename(const boost::filesystem::path& oldPath,
            const boost::filesystem::path& newPath);

} // namespace utility

#endif // utility_path_hpp_included_
