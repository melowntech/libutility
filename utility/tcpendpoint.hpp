#ifndef utility_tcpendpoint_hpp_included_
#define utility_tcpendpoint_hpp_included_

#include <boost/asio/ip/tcp.hpp>

namespace utility {

struct TcpEndpoint {
    TcpEndpoint() = default;

    TcpEndpoint(unsigned short portNum)
        : value(boost::asio::ip::tcp::v4(), portNum) {}

    TcpEndpoint(const std::string &def);

    TcpEndpoint(const boost::asio::ip::tcp::endpoint &value) : value(value) {}

    operator const boost::asio::ip::tcp::endpoint&() { return value; }

    boost::asio::ip::tcp::endpoint value;
};

} // namespace utility

#endif // utility_tcpendpoint_hpp_included_
