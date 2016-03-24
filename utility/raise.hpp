#ifndef utility_raise_hpp_included_
#define utility_raise_hpp_included_

#include <utility>

#include <boost/format.hpp>

#include "./format.hpp"

namespace utility {

template <typename ...Args>
inline std::string formatError(const std::string &message, Args &&...args)
{
    // forward to generic formatting function
    return format(message, std::forward<Args>(args)...);
}

template <typename Exception, typename ...Args>
inline Exception makeError(const std::string &message, Args &&...args)
{
    return Exception(formatError(message, std::forward<Args>(args)...));
}

template <typename Exception, typename ...Args>
inline void raise(const std::string &message, Args &&...args)
{
    throw Exception(formatError(message, std::forward<Args>(args)...));
}

} // namespace utility

#endif // utility_raise_hpp_included_
