/**
 * @file expect.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Defined expect(precond, message, message-args) function that throws
 * std::logic_error with formatted message using boost::format when precondition
 * yields false.
 */

#ifndef utility_expect_hpp_included_
#define utility_expect_hpp_included_

#include <stdexcept>

#include "./raise.hpp"

namespace utility {

/** Throws std::logic_error with formatted message using boost::format when
 * precondition yields false. Otherwise does nothing.
 */
template <typename ...Args>
inline void expect(bool precondition, const std::string &message
                   , Args &&...args)
{
    if (!precondition) {
        raise<std::logic_error>(message, std::forward<Args>(args)...);
    }
}

} // namespace utility

#endif // utility_expect_hpp_included_
