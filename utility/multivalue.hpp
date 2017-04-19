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

template <typename T, typename C>
inline void process_multi_value(const detail::po::variables_map &map
                                , const char *name
                                , std::vector<C> &output, T C::*destination)
{
    const auto &input(detail::get_multi_value<T>(map, name, output.size()));

    auto ioutput(output.begin());

    for (const auto &value : input) {
        ((*ioutput++).*destination) = value;
    }
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

template <typename T, typename U, typename C, typename D, typename O
          , typename Output>
inline void process_multi_value(const detail::po::variables_map &map
                                , const char *name
                                , Output &output, T C::*owner
                                , U D::*destination)
{
    const auto &input(detail::get_multi_value<U>(map, name, output.size()));

    auto ioutput(output.begin());

    for (const auto &value : input) {
        (((*ioutput++).*owner).*destination) = value;
    }
}

// support for helper parser

template <typename Helper, typename T, typename C>
inline void
process_multi_value(const detail::po::variables_map &map
                    , const char *name
                    , std::vector<C> &output
                    , T C::*destination)
{
    const auto &input
        (detail::get_multi_value<Helper>(map, name, output.size()));

    auto ioutput(output.begin());

    for (const auto &value : input) {
        ((*ioutput++).*destination) = value;
    }
}

} // namespace utility

#endif // utility_multivalues_hpp_included_
