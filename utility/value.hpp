/**
 * @file utility/value.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 */

#ifndef utility_value_hpp_included_
#define utility_value_hpp_included_

#include <exception>
#include <stdexcept>

#include <boost/optional.hpp>

namespace utility {

/** Wrapper arround value or exception. Can be used in callbacks to pass value
 ** and signal error in one variable.
 */
template <typename T>
class Value {
public:
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T* pointer;
    typedef const T* const_pointer;

    Value() : value_() {}
    Value(const value_type &value) : value_(value) {}
    Value(const std::exception &exc): exc_(std::make_exception_ptr(exc)) {}
    Value(const std::exception_ptr &exc) : exc_(exc) {}

    /** Returns value or thows exception if set.
     */
    const_reference get() const;

    /** Returns value or thows exception if set.
     */
    reference get();

    /** If exception is set then sink(exc) is called and true is returned.
     *  Otherwise returns false.
     */
    template <typename ErrorSink>
    bool forwardException(ErrorSink &sink) const;

    /** Combines forwardException(sink) and get().
     *  Returns nullptr if exception is set.
     */
    template <typename ErrorSink>
    const_pointer get(ErrorSink &sink) const;

    /** Combines forwardException(sink) and get().
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
    boost::optional<value_type> value_;
};

template <typename T, typename ErrorSink>
std::shared_ptr<T> get(const Value<std::shared_ptr<T>> &value, ErrorSink &sink)
{
    std::shared_ptr<T> out;
    value.get(out, sink);
    return out;
}

template <typename T, typename ErrorSink>
std::shared_ptr<T> get(Value<std::shared_ptr<T>> &value, ErrorSink &sink)
{
    std::shared_ptr<T> out;
    value.get(out, sink);
    return out;
}

// inlines

template <typename T>
typename Value<T>::const_reference Value<T>::get() const {
    if (exc_) { std::rethrow_exception(exc_); }
    if (value_) { return *value_; }
    throw std::logic_error("Value unset");
}

template <typename T>
typename Value<T>::reference Value<T>::get() {
    if (exc_) { std::rethrow_exception(exc_); }
    if (value_) { return *value_; }
    throw std::logic_error("Value unset");
}

template <typename T>
template <typename ErrorSink>
typename Value<T>::const_pointer Value<T>::get(ErrorSink &sink) const
{
    if (exc_) {
        sink(exc_);
        return nullptr;
    }

    if (!value_) {
        sink(std::make_exception_ptr(std::logic_error("Value unset")));
        return nullptr;
    }

    return &*value_;
}

template <typename T>
template <typename ErrorSink>
typename Value<T>::pointer Value<T>::get(ErrorSink &sink)
{
    if (exc_) {
        sink(exc_);
        return nullptr;
    }

    if (!value_) {
        sink(std::make_exception_ptr(std::logic_error("Value unset")));
        return nullptr;
    }

    return &*value_;
}

template <typename T>
template <typename ErrorSink>
bool Value<T>::get(reference &out, ErrorSink &sink) const
{
    if (exc_) {
        sink(exc_);
        return false;
    }

    if (!value_) {
        sink(std::make_exception_ptr(std::logic_error("Value unset")));
        return false;
    }

    out = *value_;
    return true;
}

template <typename T>
template <typename ErrorSink>
bool Value<T>::get(reference &out, ErrorSink &sink)
{
    if (exc_) {
        sink(exc_);
        return false;
    }

    if (!value_) {
        sink(std::make_exception_ptr(std::logic_error("Value unset")));
        return false;
    }

    out = *value_;
    return true;
}

template <typename T>
template <typename ErrorSink>
bool Value<T>::forwardException(ErrorSink &sink) const
{
    if (exc_) {
        sink(exc_);
        return true;
    }

    if (!value_) {
        sink(std::make_exception_ptr(std::logic_error("Value unset")));
        return true;
    }

    return false;
}

} // namespace utility

#endif // utility_value_hpp_included_
