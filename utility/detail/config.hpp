/**
 * @file detail/config.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Simple interface to config file: implementation
 */

#ifndef utility_detail_config_hpp_included_
#define utility_detail_config_hpp_included_

#include <boost/filesystem/path.hpp>
#include <boost/program_options.hpp>

namespace utility { namespace detail {

void readConfig(const boost::filesystem::path &file
                , const boost::program_options::options_description &od
                , boost::program_options::variables_map &vm);

} } // namespace utility::detail

#endif // utility_detail_config_hpp_included_


