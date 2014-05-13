/**
 * @file utility/enum.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 */

#ifndef utility_enum_hpp_included_
#define utility_enum_hpp_included_

#include <type_traits>
#include <cstdint>

#include "./gccversion.hpp"

namespace utility {
#if GCC_VERSION && (GCC_VERSION >= 40700)

// Proper C++11 way to go:
template <typename T>
inline typename std::underlying_type<T>::type to_underlying(T e) {
    return static_cast<typename std::underlying_type<T>::type>(e);
}

#else
// Need to do manually
namespace detail {
    template <std::size_t, bool> struct UnderlyingType;

#define GENERATE(TYPE)                                                  \
    template <>                                                         \
    struct UnderlyingType<sizeof(TYPE), std::is_signed<TYPE>::value >   \
    { typedef TYPE type; }

    GENERATE(std::int8_t);
    GENERATE(std::uint8_t);

    GENERATE(std::int16_t);
    GENERATE(std::uint16_t);

    GENERATE(std::int32_t);
    GENERATE(std::uint32_t);

    GENERATE(std::int64_t);
    GENERATE(std::uint64_t);

#undef GENERATE
}

template <typename T>
struct underlying_type {
    typedef typename detail::UnderlyingType
        <sizeof(T), std::is_signed<T>::value>::type type;
};


template <typename T>
inline typename underlying_type<T>::type to_underlying(T e) {
    return static_cast<typename underlying_type<T>::type>(e);
}
#endif

} // namespace utility

#endif // utility_enum_hpp_included_
