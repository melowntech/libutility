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

inline boost::filesystem::path
replaceOrAddExtension(const boost::filesystem::path &path
                      , const boost::filesystem::path &ext)
{
    if (!path.has_extension()) {
        // no extension -> add it
        return addExtension(path, "." + ext.string());
    }
    auto p(path);
    return p.replace_extension(ext);
}

inline boost::filesystem::path
addFilenameSuffix(const boost::filesystem::path &path
                  , const std::string &suffix)
{
    return path.parent_path() /
           (path.stem().string() + suffix + path.extension().string());
}

inline bool isPathPrefix(const boost::filesystem::path &path
                         , const boost::filesystem::path &prefix)
{
    return (std::mismatch(path.begin(), path.end(), prefix.begin()).second
            == prefix.end());
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

} // namespace utility

#endif // utility_path_hpp_included_
