#include <string>

#include <boost/test/unit_test.hpp>

#include "utility/tcpendpoint-io.hpp"

#include "dbglog/dbglog.hpp"

BOOST_AUTO_TEST_CASE(utility_tcpendpoint_1)
{
    BOOST_TEST_MESSAGE("* Testing utility/tcpendpoint.");

    LOG(info4) << utility::TcpEndpoint("100");
    LOG(info4) << utility::TcpEndpoint(":100");
    LOG(info4) << utility::TcpEndpoint("*:100");
    LOG(info4) << utility::TcpEndpoint("192.168.1.1:100");
    LOG(info4) << utility::TcpEndpoint("[::]:200");
    LOG(info4) << utility::TcpEndpoint("[::1]:200");
    LOG(info4) << utility::TcpEndpoint("eth0:200");
    LOG(info4) << utility::TcpEndpoint("[eth0]:500");
}
