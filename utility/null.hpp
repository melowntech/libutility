/**
 * @file utility/null.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Workaroud for nullptr overload GCC bug.
 */

#ifndef utility_null_hpp_included_
#define utility_null_hpp_included_

namespace utility {

struct Null {
    template <typename T> operator T*() const { return 0x0; }
    template <typename T> operator const T*() const { return 0x0; }
};

const Null null;

} // namespace utility

#endif // utility_null_hpp_included_
