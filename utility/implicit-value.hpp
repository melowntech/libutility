#ifndef utility_implicit_value_hpp_included_
#define utility_implicit_value_hpp_included_

#include <boost/program_options.hpp>

/** Misbehaving implicit value workaround for Boost 1.59-1.64
 */

namespace utility {

#if (BOOST_VERSION >= 105900) && (BOOST_VERSION < 106500)

/** Special typed-value to overcome problem with implicit_value on boost
 *  v. [1.57, 1.64) where explicit value to option with implicit value must be
 *  passed in one token, i.e. via --option=value because --option value is
 *  treated as implicit value and positional argument.
 */
template <typename T>
struct greedy_implicit_value : public boost::program_options::typed_value<T>
{
    greedy_implicit_value(T *value, const T &implicit)
        : boost::program_options::typed_value<T>(value)
    {
        boost::program_options::typed_value<T>::implicit_value(implicit);
    }

    bool adjacent_tokens_only() const override { return false; }
    unsigned max_tokens() const override { return 1; }
};

template <typename T>
boost::program_options::typed_value<T>*
implicit_value(T *value, const T &implicit)
{
    return new greedy_implicit_value<T>(value, implicit);
}

#else

template <typename T>
boost::program_options::typed_value<T>*
implicit_value(T *value, const T &implicit)
{
    auto tv(new boost::program_options::typed_value<T>(value));
    tv->implicit_value(implicit);
    return tv;
}

#endif

} // utility

#endif // utility_implicit_value_hpp_included_
