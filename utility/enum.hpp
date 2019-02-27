/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * @file utility/enum.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 */

#ifndef utility_enum_hpp_included_
#define utility_enum_hpp_included_

#include <type_traits>
#include <cstdint>

#include "gccversion.hpp"

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

    GENERATE(signed char);
    GENERATE(unsigned char);

#if USHRT_MAX > UCHAR_MAX
    GENERATE(signed short);
    GENERATE(unsigned short);
#endif

#if UINT_MAX > USHRT_MAX
    GENERATE(signed int);
    GENERATE(unsigned int);
#endif

#if ULONG_MAX > UINT_MAX
    GENERATE(signed long);
    GENERATE(unsigned long);
#endif

#if ULLONG_MAX > ULONG_MAX
    GENERATE(signed long long);
    GENERATE(unsigned long long);
#endif

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

/** Tells whether t is one of args.
 */
template <typename T, typename ...Args>
bool in(const T &t, Args &&...args);

namespace detail {

template <typename T, typename U, typename ...Args>
bool oneOf(const T &t, const U &u)
{
    return (t == u);
}

template <typename T, typename U, typename ...Args>
bool oneOf(const T &t, const U &u, Args &&...args)
{
    if (t == u) { return true; }
    return detail::oneOf(t, std::forward<Args>(args)...);
}

} // detail

template <typename T, typename ...Args>
bool in(const T &t, Args &&...args)
{
    return detail::oneOf(t, std::forward<Args>(args)...);
}

} // namespace utility

#endif // utility_enum_hpp_included_
