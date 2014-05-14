/**
 * @file utility/null.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Workaroud for nullptr overload GCC bug.
 */

#ifndef utility_null_hpp_included_
#define utility_null_hpp_included_

namespace utility {

class Null {
public:
    template <typename T> operator T*() const { return 0x0; }
    template<class C, class T> inline operator T C::*() const { return 0x0; }

private:
    void operator&() const = delete;
};

const Null null;

} // namespace utility

#endif // utility_null_hpp_included_
