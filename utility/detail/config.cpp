#include <string>
#include <fstream>

#include "dbglog/dbglog.hpp"

#include "config.hpp"

namespace utility { namespace detail {

void readConfig(const boost::filesystem::path &file
                , const boost::program_options::options_description &od
                , boost::program_options::variables_map &vm)
{
    std::ifstream f;
    f.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        f.open(file.native());
        f.exceptions(std::ifstream::badbit);
        store(parse_config_file(f, od), vm);
        notify(vm);
    } catch(const std::ios_base::failure &e) {
        LOGTHROW(err3, std::runtime_error)
            << "Cannot read config file <" << file << ">: " << e.what();
    }
    f.close();
}

} } // namespace utility::detail
