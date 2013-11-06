#ifndef utility_raise_hpp_included_
#define utility_raise_hpp_included_

#include <utility>

#include <boost/format.hpp>

namespace utility {

namespace detail {

inline void formatException(boost::format&) {}

template <typename T, typename ...Args>
inline void formatException(boost::format &format, T &&arg, Args &&...rest)
{
    format % arg;
    return detail::formatException(format, std::forward<Args>(rest)...);
}

} // namespace detail

template <typename Exception, typename ...Args>
void raise(const std::string &message, Args &&...args)
{
    boost::format format(message);
    detail::formatException(format, std::forward<Args>(args)...);
    throw Exception(str(format));
}

} // namespace utility

#endif // utility_raise_hpp_included_
