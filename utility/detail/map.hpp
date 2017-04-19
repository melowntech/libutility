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
#ifndef shared_utility_detail_map_hpp_included_
#define shared_utility_detail_map_hpp_included_

#include <cstdlib>
#include <utility>
#include <type_traits>
#include <boost/range.hpp>
#include <boost/format.hpp>
#include <boost/utility/result_of.hpp>

#include "dbglog/dbglog.hpp"

namespace utility { namespace detail {

template <typename CallResult>
class Result {
public:
    void set(const std::exception_ptr &exc) {
        exc_ = exc;
        result_.reset();
    }

    void set(CallResult &&result) {
        exc_ = 0x0;
        result_ = result;
    }

    const CallResult& get() const {
        if (result_) {
            return *result_;
        }
        std::rethrow_exception(exc_);
    }

    CallResult& get() {
        if (result_) {
            return *result_;
        }
        std::rethrow_exception(exc_);
    }

    const CallResult& operator*() const { return get(); }

    CallResult& operator*() { return get(); }

    const CallResult* operator->() const { return &get(); }

    CallResult* operator->() { return &get(); }

    bool valid() const {
        return result_ != boost::none;
    }

private:
    std::exception_ptr exc_;
    boost::optional<CallResult> result_;
};

template <typename Sequence, typename Callable, typename... Args>
class map_helper {
public:
    typedef typename Sequence::value_type value_type;

//    typedef typename std::result_of
    typedef typename boost::result_of
        <Callable(const value_type&, Args&&...)>::type CallResult;

    typedef detail::Result<CallResult> Result;
    typedef std::vector<Result> ResultList;
    typedef boost::sub_range<const Sequence> SequenceSubRange;

    map_helper() {}

    template <typename... Args2>
    void operator()(const SequenceSubRange &values, const std::string &name
                    , size_t index, size_t count
                    , typename ResultList::iterator result
                    , const Callable &callable, Args2&& ...args)
        const
    {
        for (const auto &value : values) {
            dbglog::thread_id(str(boost::format("%s [%d/%d]")
                                  % name % (index + 1) % count));

            try {
                result->set(callable(value, std::forward<Args2>(args)...));
            } catch(...) {
                result->set(std::current_exception());
            }

            ++result;
            ++index;
        }
    }
};

} } // namespace utility::detail

#endif // shared_utility_detail_map_hpp_included_
