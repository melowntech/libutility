#ifndef SHARED_UTILITY_FUTURE_HPP_INCLUDED_
#define SHARED_UTILITY_FUTURE_HPP_INCLUDED_

#include <future>

#include "gccversion.hpp"

namespace utility {

#ifndef UTILITY_HAS_NO_STD_FUTURE_STATUS
inline bool isFutureReady(const std::future_status &status)
{
    return (status == std::future_status::ready);
}
#else
inline bool isFutureReady(bool res)
{
    return res;
}
#endif

} // namespace utility

#endif // SHARED_UTILITY_FUTURE_HPP_INCLUDED_
