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
#include <cstdlib>
#include <string>
#include <sstream>

#include <boost/test/unit_test.hpp>

#include "../process.hpp"

#include "dbglog/dbglog.hpp"

namespace {
    const std::string data(R"RAW(Lorem ipsum dolor sit amet, consectetur adipisicing
elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi
ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit
in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint
occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim
id est laborum.)RAW");

    class MaskSaver {
    public:
        MaskSaver(unsigned mask)
            : mask_(dbglog::get_mask())
        {
            dbglog::set_mask(mask);
        }
        ~MaskSaver() { dbglog::set_mask(mask_); }
    private:
        unsigned int mask_;
    };
}



BOOST_AUTO_TEST_CASE(utility_process_system_1)
{
    MaskSaver guard(dbglog::level::default_ | dbglog::level::info2);

    BOOST_TEST_MESSAGE("* Testing cat < istream > ostream");

    std::istringstream input(data);
    std::ostringstream output;

    BOOST_CHECK(EXIT_SUCCESS ==
                utility::system("cat"
                                , utility::Stdin(input)
                                , utility::Stdout(output)
                                )
                );
    BOOST_CHECK(output.str() == data);
}

BOOST_AUTO_TEST_CASE(utility_process_system_2)
{
    MaskSaver guard(dbglog::level::default_ | dbglog::level::info2);

    BOOST_TEST_MESSAGE("* Testing cat istream > ostream");
    std::istringstream input(data);
    std::ostringstream output;

    BOOST_CHECK(EXIT_SUCCESS ==
                utility::system("cat"
                                , utility::Stream("%s", input)
                                , utility::Stdout(output)
                                )
                );
    BOOST_CHECK(output.str() == data);
}
