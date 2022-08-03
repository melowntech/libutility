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
        std::string log = "Wrong number of multi_value entries: " + 
                          std::to_string(value.size()) + 
                          " vs expected " + std::to_string(size);
        throw po::validation_error
            (po::validation_error::invalid_option_value, log, name);
    }
    return value;
}

} } // namespace utility::detail

#endif // utility_detail_multivalues_hpp_included_
