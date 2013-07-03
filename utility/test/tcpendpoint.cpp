#include <cstdlib>

#include <boost/test/unit_test.hpp>

#include "utility/tcpendpoint.hpp"
#include "utility/tcpendpoint-io.hpp"

#include "dbglog/dbglog.hpp"

BOOST_AUTO_TEST_CASE(utility_tcpendpoint_1)
{
    BOOST_TEST_MESSAGE("* Testing utility/tcpendpoint.");

    utility::TcpEndpoint("1234");
    utility::TcpEndpoint(":1234");
    utility::TcpEndpoint("0.0.0.0:1234");
}
