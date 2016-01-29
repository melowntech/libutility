#ifndef utility_format_hpp_included_
#define utility_format_hpp_included_

#include <utility>

#include <boost/format.hpp>

namespace utility {

namespace detail {

inline void format(boost::format&) {}

template <typename T, typename ...Args>
inline void format(boost::format &fmt, T &&arg, Args &&...rest)
{
    fmt % arg;
    return format(fmt, std::forward<Args>(rest)...);
}

} // namespace detail

template <typename ...Args>
inline std::string format(const std::string &message, Args &&...args)
{
    boost::format fmt(message);
    detail::format(fmt, std::forward<Args>(args)...);
    return str(fmt);
}

} // namespace utility

#endif // utility_format_hpp_included_
