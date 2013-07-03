#include <stdexcept>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_as.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include "tcpendpoint.hpp"
#include "tcpendpoint-io.hpp"
#include "detail/iface.hpp"

namespace ip = boost::asio::ip;

namespace utility {

TcpEndpoint::TcpEndpoint(const std::string &def)
    : value(parseEndpoint(def).value)
{}

TcpEndpoint parseEndpoint(const std::string &endpoint)
{
    using boost::spirit::qi::char_;
    using boost::spirit::qi::ushort_;
    using boost::spirit::qi::digit;
    using boost::spirit::qi::xdigit;
    using boost::spirit::qi::alnum;
    using boost::spirit::qi::parse;
    using boost::spirit::qi::phrase_parse;
    using boost::spirit::qi::lexeme;
    using boost::spirit::qi::_1;
    using boost::spirit::ascii::space;
    using boost::spirit::as_string;
    using boost::phoenix::ref;

    enum format { invalid, full, port_only };

    std::string ipv4_v;
    std::string ipv6_v;
    std::string iface4_v;
    std::string iface6_v;
    unsigned short port_v;
    format type(invalid);

    auto iface(+alnum);
    auto ipv4(+(digit | char_('.')));
    auto ipv6(+(xdigit | char_(':') | char_('.')));
    auto ipv6_lit(char_('[')
                  >> as_string[ipv6][boost::phoenix::ref(ipv6_v) = _1]
                  >> char_(']'));
    auto iface6_lit(char_('[')
                  >> as_string[iface][boost::phoenix::ref(iface6_v) = _1]
                  >> char_(']'));
    auto ip(as_string[ipv4][boost::phoenix::ref(ipv4_v) = _1]
            | ipv6_lit
            | as_string[iface][boost::phoenix::ref(iface4_v) = _1]
            | iface6_lit
            );
    auto port(ushort_[boost::phoenix::ref(port_v) = _1]);
    auto grammar
        (lexeme[(ip >> ':' >> port)[boost::phoenix::ref(type) = full]
                || (-char_(':') >> port)
                [boost::phoenix::ref(type) = port_only]]);

    auto first(endpoint.begin());
    auto last(endpoint.end());
    if (!phrase_parse(first, last, grammar, space) || (first != last)) {
        throw std::runtime_error("Not an endpoint: <" + endpoint + ">");
    }

    switch (type) {
    case invalid:
        throw std::runtime_error
            ("Endpoint parsing failed: <" + endpoint + ">");

    case full:
        if (!ipv4_v.empty()) {
            return { ip::tcp::endpoint
                    (ip::address_v4::from_string(ipv4_v), port_v)
                    };
        } else if (!ipv6_v.empty()) {
            return { ip::tcp::endpoint
                    (ip::address_v6::from_string(ipv6_v), port_v)
                    };
        } else if (!iface4_v.empty()) {
            return { detail::tcpEndpointForIface
                    (ip::tcp::v4(), iface4_v, port_v) };
        } else if (!iface6_v.empty()) {
            return { detail::tcpEndpointForIface
                    (ip::tcp::v6(), iface6_v, port_v) };
        }
        break;

    case port_only:
        return { ip::tcp::endpoint(ip::address_v4(), port_v) };
    }

    throw std::runtime_error
        ("Endpoint parsing failed: <" + endpoint + ">");
}

} // namespace utility
