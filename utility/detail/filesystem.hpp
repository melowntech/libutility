#ifndef utility_detail_filesystem_hpp_included_
#define utility_detail_filesystem_hpp_included_

#include <boost/filesystem.hpp>

namespace utility { namespace detail {

void copy_file(const boost::filesystem::path &from
               , const boost::filesystem::path &to
               , bool overwrite);

void copy_file(const boost::filesystem::path &from
               , const boost::filesystem::path &to
               , bool overwrite
               , boost::system::error_code& ec);

} } // namespace utility::detail

#endif // utility_detail_filesystem_hpp_included_
