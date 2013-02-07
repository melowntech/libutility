/**
 * @file config.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Simple interface to config file.
 */

#ifndef utility_config_hpp_included_
#define utility_config_hpp_included_

#include <string>

#include "detail/config.hpp"

namespace utility {

template <typename Configurable>
void readConfig(const boost::filesystem::path &file
                , Configurable &config
                , const std::string &section = "");

// inline functions implementation

template <typename Configurable>
inline void readConfig(const boost::filesystem::path &file
                       , Configurable &config, const std::string &section)
{
    boost::program_options::options_description od;
    config.configuration(section, od);

    boost::program_options::variables_map vm;
    detail::readConfig(file, od, vm);

    config.configure(vm);
}

} // namespace utility

#endif // utility_config_hpp_included_
