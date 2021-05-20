/**
 * Copyright (c) 2021 Melown Technologies SE
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

#ifndef utility_release_memory_hpp_included_
#define utility_release_memory_hpp_included_

#include <vector>

namespace {

namespace __impl {

// enable namespace-scope ADL
using std::swap;

template<typename T>
struct swappable : std::is_same<decltype(swap(std::declval<T&>(),
                                              std::declval<T&>())), void>::type {};

} // namespace __impl

} // namespace


namespace utility {

template <typename T,
          typename Allocator,
          typename std::enable_if_t<std::is_nothrow_move_constructible<T>{}, int> = 0>
inline void shrink_to_fit(std::vector<T, Allocator>& vec)
{
    std::vector<T, Allocator> resized(std::make_move_iterator(std::begin(vec)),
                                      std::make_move_iterator(std::end(vec)));
    resized.swap(vec);
}


template <typename T,
          typename Allocator,
          typename std::enable_if_t<!std::is_nothrow_move_constructible<T>{}, int> = 0>
inline void shrink_to_fit(std::vector<T, Allocator>& vec)
{
    std::vector<T, Allocator> resized(vec);
    resized.swap(vec);
}


// releases memory occupied by the contents of a std::vector using swap-idiom
template <typename T, typename Allocator>
inline void release(std::vector<T, Allocator>& vec)
{
    std::vector<T, Allocator>().swap(vec);
}


// releases memory occupied using swap-idiom
// C++17: replace with std::is_swappable
template <typename T,
          typename std::enable_if_t<std::is_default_constructible<T>{}
                   && __impl::swappable<T>{}, int> = 0>
inline void release(T& container)
{
    T tmp;
    using std::swap;
    swap(container, tmp);
}


} // namespace utility

#endif // utility_release_memory_hpp_included_
