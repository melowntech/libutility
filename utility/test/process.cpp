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
                                , utility::Stream(input)
                                , utility::Stdout(output)
                                )
                );
    BOOST_CHECK(output.str() == data);
}
