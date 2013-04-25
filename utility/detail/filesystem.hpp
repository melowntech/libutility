#ifndef utility_detail_filesystem_hpp_included_
#define utility_detail_filesystem_hpp_included_

#include <boost/filesystem/path.hpp>

namespace utility { namespace detail {

void copy_file(const boost::filesystem::path &from
               , const boost::filesystem::path &to);

} } // namespace utility::detail

#endif // utility_detail_filesystem_hpp_included_
