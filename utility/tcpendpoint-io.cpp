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

#include <stdexcept>

#include <boost/lexical_cast.hpp>

#include "tcpendpoint.hpp"
#include "tcpendpoint-io.hpp"
#include "detail/iface.hpp"

namespace ip = boost::asio::ip;

namespace utility {

TcpEndpoint::TcpEndpoint(const std::string &def)
    : value(parseTcpEndpoint(def).value)
{}

namespace detail {

TcpEndpoint parseIpv4(const std::string &input, const std::string &host
                      , short port)
{
    // try to parse ipv4 first
    try {
        return {ip::tcp::endpoint
                (ip::address_v4::from_string(host), port)};
    } catch (const std::exception&) {}

    // try interface as last resort
    try {
        return { detail::tcpEndpointForIface
                (ip::tcp::v4(), host, port) };
    } catch (const std::exception&) {}

    throw std::runtime_error
        ("Not an endpoint: <" + input + ">: " + host
         + " is neither valid IPv4 address nor any local interface.");
}

TcpEndpoint parseIpv6(const std::string &input, const std::string &host
                      , short port)
{
    // try to parse ipv6 first
    try {
        return {ip::tcp::endpoint
                (ip::address_v6::from_string(host), port)};
    } catch (const std::exception&) {}

    // try interface as last resort
    try {
        return { detail::tcpEndpointForIface
                (ip::tcp::v6(), host, port) };
    } catch (const std::exception&) {}

    throw std::runtime_error
        ("Not an endpoint: <" + input + ">: " + host
         + " is neither valid IPv6 address nor any local interface.");
}

} // namespace detail

TcpEndpoint parseTcpEndpoint(const std::string &input) {
    auto colon(input.rfind(':'));

    const auto host
        ((colon == std::string::npos) ? std::string()
         : input.substr(0, colon));

    const auto sport
        ((colon == std::string::npos) ? input
         : input.substr(colon + 1, std::string::npos));

    if (sport.empty()) {
        // no port!
        throw std::runtime_error
            ("Not an endpoint: <" + input + ">: no port specified.");
    }

    auto port(boost::lexical_cast<unsigned long>(sport));

    if (port > 65535) {
        throw std::runtime_error
            ("Not an endpoint: <" + input + ">: invalid port value.");
    }
    auto sPort(static_cast<unsigned short int>(port));

    if (host.empty() || (host == "*")) {
        // just port
        return {ip::tcp::endpoint(ip::address_v4(), sPort)};
    }

    // we have host specification here
    if (host.front() == '[') {
        if (host.back() != ']') {
            throw std::runtime_error
                ("Not an endpoint: <" + input + ">: missing closing ']'.");
        }
        return detail::parseIpv6(input, host.substr(1, host.size() - 2)
                                 , sPort);
    }

    return detail::parseIpv4(input, host, sPort);
}

} // namespace utility
