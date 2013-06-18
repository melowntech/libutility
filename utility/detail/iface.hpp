#ifndef utility_detail_iface_hpp_included_
#define utility_detail_iface_hpp_included_

#include <string>

#include <boost/asio/ip/tcp.hpp>

namespace utility { namespace detail {

boost::asio::ip::tcp::endpoint
tcpEndpointForIface(const boost::asio::ip::tcp &protocol
                    , const std::string &iface
                    , unsigned short portNum);

} } // namespace utility::detail

#endif // utility_detail_iface_hpp_included_
