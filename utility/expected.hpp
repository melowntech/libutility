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
 * @file utility/value.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 */

#ifndef utility_expected_hpp_included_
#define utility_expected_hpp_included_

#include <utility>
#include <exception>
#include <stdexcept>

#include <system_error>

#include <boost/optional.hpp>

#include "./errorcode.hpp"

namespace utility {

struct ExpectedInPlace {};

/** Wrapper arround expected value or exception or error_code. Can be used in
 ** callbacks to pass value and signal error in one variable.
 */
template <typename T>
class Expected {
public:
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T* pointer;
    typedef const T* const_pointer;

    Expected() : value_() {}
    Expected(const value_type &value) : value_(value) {}
    Expected(const std::exception &exc): exc_(std::make_exception_ptr(exc)) {}
    Expected(const std::exception_ptr &exc) : exc_(exc) {}
    Expected(const std::error_code &ec) : ec_(ec) {}

    template <typename ...Args> Expected(ExpectedInPlace, Args &&...args)
        : value_(boost::in_place(std::forward<Args>(args)...)) {}

    /** Build value in place.
     */
    template <typename ...Args> reference emplace(Args &&...args);

    /** Set value, unset exception and error code.
     */
    reference set(const_reference &value);

    /** Set exception, unset value and errorcode.
     */
    reference set(const std::exception_ptr &exc);

    /** Set error code, unset value and exception.
     */
    reference set(const std::error_code &ec);

    /** Returns value or thows exception if set.
     */
    const_reference get() const;

    /** Returns value or thows exception if set.
     */
    reference get();

    /** If exception is set then sink(exc) is called and true is returned.
     * If error_code is set then sink(ec) is called and true is returned.
     *  Otherwise returns false.
     */
    template <typename ErrorSink>
    bool forwardError(ErrorSink &sink) const;

    /** Combines forwardError(sink) and get().
     *  Returns nullptr if exception is set.
     */
    template <typename ErrorSink>
    const_pointer get(ErrorSink &sink) const;

    /** Combines forwardError(sink) and get().
     *  Returns nullptr if exception is set.
     */
    template <typename ErrorSink>
    pointer get(ErrorSink &sink);

    /** Calls sink(exc) if set or copies value into out.
     *  Returns true if value was set.
     */
    template <typename ErrorSink>
    bool get(reference &out, ErrorSink &sink) const;

    /** Calls sink(exc) if set or copies value into out.
     *  Returns true if value was set.
     */
    template <typename ErrorSink>
    bool get(reference &out, ErrorSink &sink);

private:
    std::exception_ptr exc_;
    std::error_code ec_;
    boost::optional<value_type> value_;
};

template <typename T>
template <typename ...Args>
typename Expected<T>::reference Expected<T>::emplace(Args &&...args)
{
    value_ = boost::in_place(std::forward<Args>(args)...);
    exc_ = {};
    ec_ = {};
    return *this;
}

template <typename T>
typename Expected<T>::reference Expected<T>::set(const std::exception_ptr &exc)
{
    value_.reset();
    exc_ = exc;
    ec_ = {};
    return *this;
}

template <typename T>
typename Expected<T>::reference Expected<T>::set(const std::error_code &ec)
{
    value_.reset();
    exc_ = {};
    ec_ = ec;
    return *this;
}

template <typename T>
typename Expected<T>::reference Expected<T>::set(const_reference value)
{
    value_ = value;
    exc_ = {};
    ec_ = {};
    return *this;
}

template <typename T, typename ErrorSink>
std::shared_ptr<T> get(const Expected<std::shared_ptr<T>> &value
                       , ErrorSink &sink)
{
    std::shared_ptr<T> out;
    value.get(out, sink);
    return out;
}

template <typename T, typename ErrorSink>
std::shared_ptr<T> get(Expected<std::shared_ptr<T>> &value, ErrorSink &sink)
{
    std::shared_ptr<T> out;
    value.get(out, sink);
    return out;
}

// inlines

template <typename T>
typename Expected<T>::const_reference Expected<T>::get() const {
    if (exc_) { std::rethrow_exception(exc_); }
    if (ec_) { throwErrorCode(ec_); }
    if (value_) { return *value_; }
    throw std::logic_error("Expected unset");
}

template <typename T>
typename Expected<T>::reference Expected<T>::get() {
    if (exc_) { std::rethrow_exception(exc_); }
    if (ec_) { throwErrorCode(ec_); }
    if (value_) { return *value_; }
    throw std::logic_error("Expected value unset");
}

template <typename T>
template <typename ErrorSink>
typename Expected<T>::const_pointer Expected<T>::get(ErrorSink &sink) const
{
    if (exc_) { sink(exc_); return nullptr; }
    if (ec_) { sink(ec_); return nullptr; }

    if (!value_) {
        sink(std::make_exception_ptr
             (std::logic_error("Expected value unset")));
        return nullptr;
    }

    return &*value_;
}

template <typename T>
template <typename ErrorSink>
typename Expected<T>::pointer Expected<T>::get(ErrorSink &sink)
{
    if (exc_) { sink(exc_); return nullptr; }
    if (ec_) { sink(ec_); return nullptr; }

    if (!value_) {
        sink(std::make_exception_ptr
             (std::logic_error("Expected value unset")));
        return nullptr;
    }

    return &*value_;
}

template <typename T>
template <typename ErrorSink>
bool Expected<T>::get(reference &out, ErrorSink &sink) const
{
    if (exc_) { sink(exc_); return false; }
    if (ec_) { sink(ec_); return false; }

    if (!value_) {
        sink(std::make_exception_ptr
             (std::logic_error("Expected value unset")));
        return false;
    }

    out = *value_;
    return true;
}

template <typename T>
template <typename ErrorSink>
bool Expected<T>::get(reference &out, ErrorSink &sink)
{
    if (exc_) { sink(exc_); return false; }
    if (ec_) { sink(ec_); return false; }

    if (!value_) {
        sink(std::make_exception_ptr
             (std::logic_error("Expected value unset")));
        return false;
    }

    out = *value_;
    return true;
}

template <typename T>
template <typename ErrorSink>
bool Expected<T>::forwardError(ErrorSink &sink) const
{
    if (exc_) { sink(exc_); return true; }
    if (ec_) { sink(ec_); return true; }

    if (!value_) {
        sink(std::make_exception_ptr
             (std::logic_error("Expected value unset")));
        return true;
    }

    return false;
}

} // namespace utility

#endif // utility_value_hpp_included_
