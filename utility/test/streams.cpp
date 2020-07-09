/**
 * Copyright (c) 2020 Melown Technologies SE
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

#include <cstdlib>
#include <string>
#include <sstream>

#include <boost/test/unit_test.hpp>
#include <boost/optional.hpp>

#include "../streams.hpp"

#include "dbglog/dbglog.hpp"

BOOST_AUTO_TEST_CASE(utility_streams_printFirst_1)
{
    BOOST_TEST_MESSAGE("* Testing print first");

    boost::optional<std::string> emptyString;
    boost::optional<std::string> validString("valid");
    boost::optional<double> validDouble(1);

    {
        std::ostringstream output;
        output << utility::printFirst(emptyString, "NULL");
        LOG(info4) << "empty string: <" << output.str() << ">";
        BOOST_CHECK(output.str() == "NULL");
    }

    {
        std::ostringstream output;
        output << utility::printFirst(emptyString, validString, "NULL");
        LOG(info4) << "valid string: <" << output.str() << ">";
        BOOST_CHECK(output.str() == validString);
    }
}
