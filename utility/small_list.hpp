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
#ifndef utility_small_list_hpp_included_
#define utility_small_list_hpp_included_

#include <array>
#include <forward_list>
#include <boost/variant.hpp>

namespace utility {

/**
 * std::forward_list-like container that stores small number of elements in-place,
 * thus avoiding heap allocation if the size of the container does not exceed N.
 */
template<typename T, int N>
class small_list
{
    typedef std::array<T, N> static_type;
    typedef std::forward_list<T> dynamic_type;

    /// \todo could be further optimized by using std::aligned_union directly,
    /// since we do not need to store the data type
    boost::variant<static_type, dynamic_type> data_;
    int size_ = 0;

public:
    small_list() = default;

    void insert(const T& value)
    {
        if (size_ < N) {
            boost::get<static_type>(data_)[size_] = value;
        } else {
            if (size_ == N) {
                make_dynamic();
            }
            boost::get<dynamic_type>(data_).push_front(value);
        }
        size_++;
    }

    uint32_t size() const { return size_; }

    bool is_dynamic() const { return size_ > N; }

    template<typename TFunc>
    void for_each(const TFunc& func) const
    {
        if (size_ <= N) {
            const static_type& static_data = boost::get<static_type>(data_);
            for (int i = size_ - 1; i >= 0; --i) {
                func(static_data[i]);
            }
        } else {
            for (const T& value : boost::get<dynamic_type>(data_)) {
                func(value);
            }
        }
    }

private:
    void make_dynamic()
    {
        static_type values = boost::get<static_type>(data_);
        data_ = dynamic_type();
        for (int i = 0; i < size_; ++i) {
            boost::get<dynamic_type>(data_).push_front(values[i]);
        }
    }
};

} // namespace utility

#endif // utility_small_list_hpp_included_
