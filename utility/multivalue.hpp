/**
 * @file multivalue.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Config multivalue.
 */

#ifndef utility_multivalues_hpp_included_
#define utility_multivalues_hpp_included_

#include "detail/multivalue.hpp"

/** This module adds support for Boost.Program_options multivalue.
 */
namespace utility {

template <typename T>
inline detail::po::value_semantic* multi_value();

template <typename T, typename C>
void process_multi_value(const detail::po::variables_map &map, const char *name
                         , std::vector<C> &output, T C::*destination);


template <typename T, typename U, typename C, typename D, typename O>
void process_multi_value(const detail::po::variables_map &map, const char *name
                         , std::vector<O> &output, T C::*owner
                         , U D::*destination);

// inline function implementation

template <typename T>
inline detail::po::value_semantic* multi_value()
{
    return detail::po::value<std::vector<T> >();
}

template <typename T, typename C, typename Output>
inline void process_multi_value(const detail::po::variables_map &map
                                , const char *name
                                , Output &output, T C::*destination)
{
    const auto &input(detail::get_multi_value<T>(map, name, output.size()));

    auto ioutput(output.begin());

    for (const auto &value : input) {
        ((*ioutput++).*destination) = value;
    }
}

template <typename T, typename U, typename C, typename D, typename O>
inline void process_multi_value(const detail::po::variables_map &map
                                , const char *name
                                , std::vector<O> &output, T C::*owner
                                , U D::*destination)
{
    const auto &input(detail::get_multi_value<U>(map, name, output.size()));

    auto ioutput(output.begin());

    for (const auto &value : input) {
        (((*ioutput++).*owner).*destination) = value;
    }
}

} // namespace utility

#endif // utility_multivalues_hpp_included_
