#ifndef utility_tcpendpoint_io_hpp_included_
#define utility_tcpendpoint_io_hpp_included_

#include <string>

#include "tcpendpoint.hpp"

namespace utility {

TcpEndpoint parseEndpoint(const std::string &listen);

template<typename CharT, typename Traits>
std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &is, TcpEndpoint &e);

template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os, const TcpEndpoint &e);

// inlines

template<typename CharT, typename Traits>
inline std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &is, TcpEndpoint &e)
{
    std::string str;
    is >> str;
    e.value = parseEndpoint(str);
    return is;
}

template<typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os, const TcpEndpoint &e)
{
    os << e.value;
    return os;
}

} // namespace utility

#endif // utility_tcpendpoint_io_hpp_included_
