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
 * @file detail/path.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Boost.Filesystem path support
 */

#ifndef utility_path_serialization_hpp_included_
#define utility_path_serialization_hpp_included_

#include <boost/filesystem/path.hpp>
#include <boost/serialization/split_free.hpp>

namespace boost { namespace serialization {

template<class Archive>
inline void save(Archive &ar, const boost::filesystem::path &p
                     , unsigned int version)
{
    (void) version;
    ar & p.generic_string();
}

template<class Archive>
inline void load(Archive &ar, boost::filesystem::path &p
                 , unsigned int version)
{
    (void) version;

    std::string raw;
    ar & raw;
    p = raw;
}

} } // namespace boost::serialization;

BOOST_SERIALIZATION_SPLIT_FREE(boost::filesystem::path)

#endif // utility_path_serialization_hpp_included_
