/**
 * @file detail/path.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Boost.Filesystem path support
 */

#ifndef utility_detail_path_hpp_included_
#define utility_detail_path_hpp_included_

#include <utility>
#include <boost/filesystem/path.hpp>

namespace utility { namespace detail {

template <typename TailType>
inline void joinPaths(boost::filesystem::path &res
                      , const TailType &tail)
{
    res /= tail;
}

template <typename HeadType, typename ...Paths>
inline void joinPaths(boost::filesystem::path &res, const HeadType &head
                      , Paths &&...tail)
{
    res /= head;
    detail::joinPaths(res, std::forward<Paths>(tail)...);
}

} } // namespace utility::detail

#endif // utility_detail_path_hpp_included_
