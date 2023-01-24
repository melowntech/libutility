/**
 * Copyright (c) 2019 Melown Technologies SE
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

/** Various C++ version-related stuff. May introduce code into std namespace for
 *  older standard versions.
 */

#ifndef utility_cppversion_hpp_included_
#define utility_cppversion_hpp_included_

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <functional>

#include "gccversion.hpp"

/** Custom std::make_unique implementation when not available (before C++14)
 *
 * Adopted from the original Stephan T. Lavavej's proposal (N3656
 *     http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3656.htm
 * ) that becomne part of the ISO/IEC 14882:2014 standard (aka C++14)
 *
 * NB: Code injected into the std namespace!
 */
#ifndef __cpp_lib_make_unique

namespace std {

template<typename T> struct _Unique_if {
    typedef unique_ptr<T> _Single_object;
};

template<typename T> struct _Unique_if<T[]> {
    typedef unique_ptr<T[]> _Unknown_bound;
};

template<typename T, size_t N> struct _Unique_if<T[N]> {
    typedef void _Known_bound;
};

template<typename T, typename... Args>
typename _Unique_if<T>::_Single_object make_unique(Args&&... args) {
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename T>
typename _Unique_if<T>::_Unknown_bound make_unique(size_t n) {
    typedef typename remove_extent<T>::type U;
    return unique_ptr<T>(new U[n]());
}

template<typename T, typename... Args>
typename _Unique_if<T>::_Known_bound make_unique(Args&&...) = delete;

} // namespace std

#endif // __cpp_lib_make_unique

#endif // utility_cppversion_hpp_included_
