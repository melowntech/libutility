#include <stdexcept>

#include "iface.hpp"

namespace utility { namespace detail {

boost::asio::ip::tcp::endpoint
tcpEndpointForIface(const boost::asio::ip::tcp&
                    , const std::string&
                    , unsigned short)
{
    throw std::runtime_error
        ("Network interface querying not supported on this platform");
}

} } // namespace utility::detail
