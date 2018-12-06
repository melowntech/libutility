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

#ifndef utility_stl_helpers_hpp_included_
#define utility_stl_helpers_hpp_included_

#include <vector>
#include <utility>
#include <numeric>

namespace utility {

template <typename T, typename ...Args>
T& append(std::vector<T> &vector, Args &&...args)
{
    vector.emplace_back(std::forward<Args>(args)...);
    return vector.back();
}

/** Joins containers of data convertible to T into a flat vector of T.
 *
 *  \param containers collection of containers of data convertible to T
 *  \return std::vector<T> of all elements from containers
 */
template <typename T, typename Containers>
std::vector<T> flatten(const Containers &containers);

// inlines

template <typename T, typename Containers>
std::vector<T> flatten(const Containers &containers)
{
    // single container from container
    typedef typename Containers::value_type Container;

    std::vector<T> out;

    // make room for all output elements
    out.resize(std::accumulate
               (containers.begin(), containers.end(), std::size_t(0)
                , [](std::size_t count, const Container &c)
                {
                    return count + c.size();
                }));

    // join all the data into single vector
    for (const auto &c : containers) {
        out.insert(out.end(), c.begin(), c.end());
    }

    return out;
}

} // namespace utility

#endif // utility_stl_helpers_hpp_included_
