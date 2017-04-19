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
#ifndef shared_utility_algorithm_hpp_included_
#define shared_utility_algorithm_hpp_included_

#include <algorithm>

namespace utility { namespace array {

/** Calls unary function op for every item in 2-dimensional array in row major
 *  order. (immutable version)
 */
template <typename T, int rows, int cols, typename UnaryFunction>
UnaryFunction for_each(const T(&data)[rows][cols], UnaryFunction op);

/** Calls unary function op for every item in 2-dimensional array in row major
 *  order. (mutable version)
 */
template <typename T, int rows, int cols, typename UnaryFunction>
UnaryFunction for_each(T(&data)[rows][cols], UnaryFunction op);



// implementation

template <typename T, int rows, int cols, typename UnaryFunction>
inline UnaryFunction for_each(const T(&data)[rows][cols], UnaryFunction op)
{
    return std::for_each(&data[0][0], &data[rows - 1][cols], op);
}

template <typename T, int rows, int cols, typename UnaryFunction>
inline UnaryFunction for_each(T(&data)[rows][cols], UnaryFunction op)
{
    return std::for_each(&data[0][0], &data[rows - 1][cols], op);
}

} } // namespace utility::array

#endif // shared_utility_algorithm_hpp_included_
