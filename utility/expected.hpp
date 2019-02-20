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
#include <future>

#include <system_error>

#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>

#include "./errorcode.hpp"

namespace utility {

struct ExpectedInPlace {};

struct ExpectedAsSink {};

template <typename T>
struct ExpectedTraits {
    typedef T value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;

    typedef boost::optional<T> ValueHolder;
    typedef pointer GetPointer;
    typedef const_pointer ConstGetPointer;

    reference asReference(ValueHolder &value) { return *value; }
    const_reference asReference(const ValueHolder &value) const {
        return *value;
    }

    GetPointer getPointer(ValueHolder &value) { return &*value; }
    ConstGetPointer getPointer(const ValueHolder &value) const {
        return &*value;
    }

    ConstGetPointer NoConstGetPointer() const { return nullptr; }
    GetPointer NoGetPointer() { return nullptr; }

    template <typename ...Args>
    auto inplace(Args &&...args)
        -> decltype(boost::in_place(std::forward<Args>(args)...))
    {
        return boost::in_place(std::forward<Args>(args)...);
    }
};

template <typename T>
struct ExpectedTraits<std::shared_ptr<T>> {
    typedef std::shared_ptr<T> value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type pointer;
    // shared_ptr mimics pointer -> no const
    typedef value_type const_pointer;
    typedef value_type ValueHolder;
    typedef reference GetPointer;
    typedef const_reference ConstGetPointer;

    reference asReference(ValueHolder &value) { return value; }
    const_reference asReference(const ValueHolder &value) const {
        return value;
    }

    GetPointer getPointer(ValueHolder &value) { return value; }
    ConstGetPointer getPointer(const ValueHolder &value) const {
        return value;
    }

    ConstGetPointer NoConstGetPointer() const { return nullptr_; }
    GetPointer NoGetPointer() { return nullptr_; }

    template <typename ...Args>
    value_type inplace(Args &&...args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

private:
    /** Dummy empty shared pointer that is used as a reference to empty shared
     *  pointer.
     */
    value_type nullptr_;
};

/** Wrapper arround expected value or exception or error_code. Can be used in
 ** callbacks to pass value and signal error in one variable.
 */
template <typename T, typename Traits = ExpectedTraits<T>>
class Expected : private Traits {
public:
    typedef Traits traits_type;

    typedef typename Traits::value_type value_type;
    typedef typename Traits::reference reference;
    typedef typename Traits::const_reference const_reference;
    typedef typename Traits::pointer pointer;
    typedef typename Traits::const_pointer const_pointer;

    typedef typename Traits::GetPointer GetPointer;
    typedef typename Traits::ConstGetPointer ConstGetPointer;

    Expected() : value_() {}
    Expected(const value_type &value) : value_(value) {}
    Expected(const std::exception &exc): exc_(std::make_exception_ptr(exc)) {}
    Expected(const std::exception_ptr &exc) : exc_(exc) {}
    Expected(const std::error_code &ec) : ec_(ec) {}

    template <typename ...Args> Expected(ExpectedInPlace, Args &&...args)
        : value_(Traits::inplace(std::forward<Args>(args)...))
    {}

    /** Build value in place.
     */
    template <typename ...Args> reference emplace(Args &&...args);

    /** Set value, unset exception and error code.
     */
    Expected<T, Traits>& set(const_reference value);

    /** Set exception, unset value and errorcode.
     */
    Expected<T, Traits>& set(const std::exception_ptr &exc);

    /** Set error code, unset value and exception.
     */
    Expected<T, Traits>& set(const std::error_code &ec);

    /** Checks whether we have valid value.
     */
    operator bool() const { return static_cast<bool>(value_); }

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

    /** If exception or error code is set then sink(*this) is called and true is
     *  returned. Otherwise returns false.
     */
    template <typename ErrorSink>
    bool forwardError(ErrorSink &sink, const ExpectedAsSink&) const;

    /** Combines forwardError(sink) and get().
     *  Returns nullptr if exception is set.
     */
    template <typename ErrorSink>
    ConstGetPointer get(ErrorSink &sink) const;

    /** Combines forwardError(sink) and get().
     *  Returns nullptr if exception is set.
     */
    template <typename ErrorSink>
    GetPointer get(ErrorSink &sink);

    /** Combines forwardError(sink, ExpectedAsSink) and get().
     *  Returns nullptr if exception is set.
     */
    template <typename ErrorSink>
    ConstGetPointer get(ErrorSink &sink, const ExpectedAsSink&) const;

    /** Combines forwardError(sink, ExpectedAsSink) and get().
     *  Returns nullptr if exception is set.
     */
    template <typename ErrorSink>
    GetPointer get(ErrorSink &sink, const ExpectedAsSink&);

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

    /** Gets value/exceptions and stores it into promise. Promise can have
     *  different (but compatible type)/
     *  Returns true if value was set.
     */
    template <typename ValueType>
    bool get(std::promise<ValueType> &promise) const;

private:
    std::exception_ptr exc_;
    std::error_code ec_;
    typename Traits::ValueHolder value_;
};

template <typename T, typename Traits>
template <typename ...Args>
typename Expected<T, Traits>::reference Expected<T, Traits>::emplace(Args &&...args)
{
    value_ = boost::in_place(std::forward<Args>(args)...);
    exc_ = {};
    ec_ = {};
    return *this;
}

template <typename T, typename Traits>
Expected<T, Traits>& Expected<T, Traits>::set(const std::exception_ptr &exc)
{
    value_.reset();
    exc_ = exc;
    ec_ = {};
    return *this;
}

template <typename T, typename Traits>
Expected<T, Traits>& Expected<T, Traits>::set(const std::error_code &ec)
{
    value_.reset();
    exc_ = {};
    ec_ = ec;
    return *this;
}

template <typename T, typename Traits>
Expected<T, Traits>& Expected<T, Traits>::set(const_reference value)
{
    value_ = value;
    exc_ = {};
    ec_ = {};
    return *this;
}

// inlines

template <typename T, typename Traits>
typename Expected<T, Traits>::const_reference Expected<T, Traits>::get() const {
    if (exc_) { std::rethrow_exception(exc_); }
    if (ec_) { throwErrorCode(ec_); }
    if (value_) { return Traits::asReference(value_); }
    throw std::logic_error("Expected unset");
}

template <typename T, typename Traits>
typename Expected<T, Traits>::reference Expected<T, Traits>::get() {
    if (exc_) { std::rethrow_exception(exc_); }
    if (ec_) { throwErrorCode(ec_); }
    if (value_) { return Traits::asReference(value_); }
    throw std::logic_error("Expected value unset");
}

template <typename T, typename Traits>
template <typename ErrorSink>
typename Expected<T, Traits>::ConstGetPointer
Expected<T, Traits>::get(ErrorSink &sink) const
{
    if (exc_) { sink(exc_); return Traits::NoConstGetPointer(); }
    if (ec_) { sink(ec_); return Traits::NoConstGetPointer(); }

    if (!value_) {
        sink(std::make_exception_ptr
             (std::logic_error("Expected value unset")));
        return Traits::NoConstGetPointer();
    }

    return Traits::getPointer(value_);
}

template <typename T, typename Traits>
template <typename ErrorSink>
typename Expected<T, Traits>::GetPointer
Expected<T, Traits>::get(ErrorSink &sink)
{
    if (exc_) { sink(exc_); return Traits::NoGetPointer(); }
    if (ec_) { sink(ec_); return Traits::NoGetPointer(); }

    if (!value_) {
        sink(std::make_exception_ptr
             (std::logic_error("Expected value unset")));
        return Traits::NoGetPointer();
    }

    return Traits::getPointer(value_);
}

template <typename T, typename Traits>
template <typename ErrorSink>
typename Expected<T, Traits>::ConstGetPointer
Expected<T, Traits>::get(ErrorSink &sink, const ExpectedAsSink&) const
{
    if (exc_ || ec_) { sink(*this); return Traits::NoConstGetPointer(); }

    if (!value_) {
        sink(std::make_exception_ptr
             (std::logic_error("Expected value unset")));
        return Traits::NoConstGetPointer();
    }

    return Traits::getPointer(value_);
}

template <typename T, typename Traits>
template <typename ErrorSink>
typename Expected<T, Traits>::GetPointer
Expected<T, Traits>::get(ErrorSink &sink, const ExpectedAsSink&)
{
    if (exc_ || ec_) { sink(*this); return Traits::NoGetPointer(); }

    if (!value_) {
        sink(std::make_exception_ptr
             (std::logic_error("Expected value unset")));
        return Traits::NoGetPointer();
    }

    return Traits::getPointer(value_);
}

template <typename T, typename Traits>
template <typename ErrorSink>
bool Expected<T, Traits>::get(reference &out, ErrorSink &sink) const
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

template <typename T, typename Traits>
template <typename ErrorSink>
bool Expected<T, Traits>::get(reference &out, ErrorSink &sink)
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

template <typename T, typename Traits>
template <typename ErrorSink>
bool Expected<T, Traits>::forwardError(ErrorSink &sink) const
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

template <typename T, typename Traits>
template <typename ErrorSink>
bool Expected<T, Traits>::forwardError(ErrorSink &sink, const ExpectedAsSink&)
    const
{
    if (exc_ || ec_) { sink(*this); return true; }

    if (!value_) {
        sink(std::make_exception_ptr
             (std::logic_error("Expected value unset")));
        return true;
    }

    return false;
}

template <typename T, typename Traits>
template <typename ValueType>
bool Expected<T, Traits>::get(std::promise<ValueType> &promise) const
{
    if (exc_) { promise.set_exception(exc_); return true; }
    if (ec_) {
        promise.set_exception(makeErrorCodeException(ec_));
        return false;
    }

    if (!value_) {
        promise.set_exception(std::make_exception_ptr
                              (std::logic_error("Expected value unset")));
        return false;
    }

    promise.set_value(*value_);
    return true;
}

} // namespace utility

#endif // utility_value_hpp_included_
