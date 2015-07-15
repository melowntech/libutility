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
    return path.parent_path() / (path.filename().string() + ext.string());
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
    auto e(ext.string());
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
    return dir / (stem.string() + e);
}

inline boost::filesystem::path
addFilenameSuffix(const boost::filesystem::path &path
                  , const std::string &suffix)
{
    return path.parent_path() /
           (path.stem().string() + suffix + path.extension().string());
}

inline boost::filesystem::path toLower(const boost::filesystem::path &path)
{
    return boost::algorithm::to_lower_copy(path.string());
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

/** Filename matching.
 */
namespace FileMatch {
    enum {
        icase = 0x1   //!< case-insensitive matching
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

} // namespace utility

#endif // utility_path_hpp_included_
