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

    if (host.empty() || (host == "*")) {
        // just port
        return {ip::tcp::endpoint(ip::address_v4(), port)};
    }

    // we have host specification here
    if (host.front() == '[') {
        if (host.back() != ']') {
            throw std::runtime_error
                ("Not an endpoint: <" + input + ">: missing closing ']'.");
        }
        return detail::parseIpv6(input, host.substr(1, host.size() - 2)
                                 , port);
    }

    return detail::parseIpv4(input, host, port);
}

} // namespace utility
