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
#include <boost/test/unit_test.hpp>

#include "../small_list.hpp"

#include "dbglog/dbglog.hpp"

namespace {
    template<typename T, int N>
    std::vector<T> listToVector(const utility::small_list<T, N>& list)
    {
        std::vector<T> result;
        list.for_each([&result](T value) { result.push_back(value); });
        return result;
    }

    template <typename T, int N>
    bool compare(const utility::small_list<T, N>& actual,
                 const std::vector<T>& expected) {
        std::vector<T> values = listToVector(actual);
        if (values.size() != expected.size()) {
            return false;
        }
        for (std::size_t i = 0; i < expected.size(); ++i) {
            if (values[i] != expected[i]) {
                return false;
            }
        }
        return true;
    }
} // namespace

BOOST_AUTO_TEST_CASE(utility_small_list)
{
    BOOST_TEST_MESSAGE("* Testing utility/small_list.");

    utility::small_list<int, 2> list;
    BOOST_CHECK(list.size() == 0);
    BOOST_CHECK(compare(list, {}));

    list.insert(1);
    BOOST_CHECK(compare(list, { 1 }));
    BOOST_CHECK(!list.is_dynamic());

    list.insert(2);
    BOOST_CHECK(compare(list, { 2, 1 }));
    BOOST_CHECK(!list.is_dynamic());

    list.insert(3);
    BOOST_CHECK(compare(list, { 3, 2, 1 }));
    BOOST_CHECK(list.is_dynamic());

    list.insert(4);
    list.insert(5);
    list.insert(6);
    BOOST_CHECK(compare(list, { 6, 5, 4, 3, 2, 1 }));

    list = {};
    BOOST_CHECK(list.size() == 0);
}
