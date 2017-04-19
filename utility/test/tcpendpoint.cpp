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
#include <string>

#include <boost/test/unit_test.hpp>

#include "../tcpendpoint-io.hpp"

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
