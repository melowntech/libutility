/**
 * @file detail/multivalue.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Config multivalue.
 */

#ifndef utility_detail_multivalues_hpp_included_
#define utility_detail_multivalues_hpp_included_

#include <string>
#include <boost/program_options.hpp>

/** This module adds support for C++ iomultivalue.
 */
namespace utility { namespace detail {
namespace po = boost::program_options;

template <typename T>
const std::vector<T>
inline get_multi_value(const po::variables_map &map, const char *name
                       , typename std::vector<T>::size_type size)
{
    const auto &v(map[name]);
    if (v.empty()) {
        throw po::required_option(name);
    }

    const auto &value(v.as<std::vector<T> >());

    if (value.size() != size) {
        // TODO: replace "xxx" with size difference description
        throw po::validation_error
            (po::validation_error::invalid_option_value, "xxx", name);
    }
    return value;
}

} } // namespace utility::detail

#endif // utility_detail_multivalues_hpp_included_
