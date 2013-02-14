/**
 * @file detail/path.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Boost.Filesystem path support
 */

#ifndef utility_path_hpp_included_
#define utility_path_hpp_included_

#include <utility>
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

boost::filesystem::path
inline addExtension(const boost::filesystem::path &path
                    , const boost::filesystem::path &ext)
{
    return path.parent_path() / (path.filename().string() + ext.string());
}

} // namespace utility

#endif // utility_path_hpp_included_
