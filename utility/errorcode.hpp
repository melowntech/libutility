/** Generic HTTP code-related stuff
 */

#ifndef utility_errorcode_hpp_included_
#define utility_errorcode_hpp_included_

#include <system_error>
#include <future>

#include "./httpcode.hpp"

namespace utility {

inline void throwErrorCode(const std::error_code &ec) noexcept(false)
{
    const auto &cat(ec.category());
    if (cat == std::system_category()) {
        throw std::system_error(ec);
    }
    if (cat == std::future_category()) {
        throw std::future_error(ec);
    }
    if (cat == httpCodeCategory()) {
        throw HttpError(ec);
    }

    // unknown category, use system error
    throw std::system_error(ec);
}

} // namespace std

#endif // utility_errorcode_hpp_included_

