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

#ifndef utility_udpendpoint_io_hpp_included_
#define utility_udpendpoint_io_hpp_included_

#include <string>

#include "udpendpoint.hpp"

namespace utility {

UdpEndpoint parseUdpEndpoint(const std::string &listen);

template<typename CharT, typename Traits>
std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &is, UdpEndpoint &e);

template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os, const UdpEndpoint &e);

// inlines

template<typename CharT, typename Traits>
inline std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &is, UdpEndpoint &e)
{
    std::string str;
    is >> str;
    e.value = parseUdpEndpoint(str);
    return is;
}

template<typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os, const UdpEndpoint &e)
{
    return os << e.value;
}

struct UdpEndpointPrettyPrint {
    const UdpEndpoint &endpoint;

    UdpEndpointPrettyPrint(const UdpEndpoint &endpoint)
        : endpoint(endpoint)
    {}
};

template<typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os
           , const UdpEndpointPrettyPrint &e)
{
    const auto &value(e.endpoint.value);

    if (value.address().is_loopback()) {
        return os << "localhost:" << value.port();
    }

    if (value.address().is_unspecified()) {
        return os << "localhost:" << value.port();
    }

    return os << value;
}

} // namespace utility

#endif // utility_udpendpoint_io_hpp_included_
