#include <sys/types.h>
#include <ifaddrs.h>

#include <iostream>

#include <memory>
#include <stdexcept>
#include <system_error>

#include "iface.hpp"

namespace utility { namespace detail {

namespace ip = boost::asio::ip;

ip::tcp::endpoint
tcpEndpointForIface(const ip::tcp &protocol
                    , const std::string &iface
                    , unsigned short portNum)
{
    int family;
    switch (protocol.family()) {
    case PF_INET: family = AF_INET; break;
    case PF_INET6: family = AF_INET6; break;
    default:
        throw std::runtime_error("Unsupported protocol family.");
    }

    struct ::ifaddrs *ifa;
    if (-1 == ::getifaddrs(&ifa)) {
        std::system_error e(errno, std::system_category(), "getifaddrs");
        throw e;
    }
    std::shared_ptr<struct ::ifaddrs> guard(ifa, [](struct ::ifaddrs *ifa) {
            freeifaddrs(ifa);
        });

    for (; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_name != iface) { continue; }
        if (ifa->ifa_addr->sa_family == family) {
            break;
        }
    }

    if (!ifa) {
        throw std::runtime_error("Interface <" + iface + "> not found.");
    }

    switch (protocol.family()) {
    case PF_INET:
        return { ip::address_v4
                (ntohl(reinterpret_cast<struct sockaddr_in*>
                       (ifa->ifa_addr)->sin_addr.s_addr))
                , portNum };

    case PF_INET6: {
        auto s6(reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_addr));
        ip::address_v6::bytes_type a;
        for (int i = 0; i < 16; ++i) { a[i] = s6->sin6_addr.s6_addr[i]; }

        return { ip::address_v6(a, s6->sin6_scope_id), portNum };
    } }

    throw std::runtime_error("Unsupported protocol family.");
}

} } // namespace utility::detail

