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
#ifndef utility_tcpendpoint_hpp_included_
#define utility_tcpendpoint_hpp_included_

#ifdef _WIN32
#  include <SDKDDKVer.h>
#endif

#include <boost/asio/ip/tcp.hpp>

namespace utility {

struct TcpEndpoint {
    enum class ParseFlags : unsigned int {
        default_ = 0x00
        , allowResolve = 0x01
    };

    TcpEndpoint() = default;

    TcpEndpoint(unsigned short portNum)
        : value(boost::asio::ip::tcp::v4(), portNum) {}

    TcpEndpoint(const std::string &def
                , ParseFlags flags = ParseFlags::default_);

    TcpEndpoint(const boost::asio::ip::tcp::endpoint &value) : value(value) {}

    operator const boost::asio::ip::tcp::endpoint&() { return value; }

    boost::asio::ip::tcp::endpoint value;
};

// inlines

inline constexpr auto
operator+(TcpEndpoint::ParseFlags f)
{
    using Type = std::underlying_type<TcpEndpoint::ParseFlags>::type;
    return static_cast<Type>(f);
}

inline constexpr TcpEndpoint::ParseFlags
operator&(TcpEndpoint::ParseFlags l, TcpEndpoint::ParseFlags r)
{
    using Type = std::underlying_type<TcpEndpoint::ParseFlags>::type;
    return static_cast<TcpEndpoint::ParseFlags>
        (static_cast<Type>(l) & static_cast<Type>(r));
}

inline constexpr TcpEndpoint::ParseFlags
operator|(TcpEndpoint::ParseFlags l, TcpEndpoint::ParseFlags r)
{
    using Type = std::underlying_type<TcpEndpoint::ParseFlags>::type;
    return static_cast<TcpEndpoint::ParseFlags>
        (static_cast<Type>(l) | static_cast<Type>(r));
}

inline constexpr TcpEndpoint::ParseFlags&
operator|=(TcpEndpoint::ParseFlags& l, TcpEndpoint::ParseFlags r)
{
    using Type = std::underlying_type<TcpEndpoint::ParseFlags>::type;
    reinterpret_cast<Type&>(l) |= static_cast<Type>(r);
    return l;
}

} // namespace utility

#endif // utility_tcpendpoint_hpp_included_
