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

    Value(const value_type &value) : value_(value) {}
    Value(const std::exception &exc): exc_(std::make_exception_ptr(exc)) {}
    Value(const std::exception_ptr &exc) : exc_(exc) {}

    const value_type& get() const;
    value_type& get();

private:
    std::exception_ptr exc_;
    boost::optional<value_type> value_;
};

// inlines

template <typename T> const T& Value<T>::get() const {
    if (exc_) { std::rethrow_exception(exc_); }
    if (value_) { return *value_; }
    throw std::logic_error("Unset value.");
}

template <typename T> T& Value<T>::get() {
    if (exc_) { std::rethrow_exception(exc_); }
    if (value_) { return *value_; }
    throw std::logic_error("Unset value.");
}

} // namespace utility

#endif // utility_value_hpp_included_
