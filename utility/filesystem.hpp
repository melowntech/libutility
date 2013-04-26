#ifndef utility_filesystem_hpp_included_
#define utility_filesystem_hpp_included_

#include <boost/filesystem/path.hpp>

#include "detail/filesystem.hpp"

namespace utility {

void copy_file(const boost::filesystem::path &from
               , const boost::filesystem::path &to);

} // namespace utility

// impelemtation

inline void utility::copy_file(const boost::filesystem::path &from
                               , const boost::filesystem::path &to
                               , bool rewrite = false)
{
    return utility::detail::copy_file(from, to, overwrite);
}


#endif // utility_filesystem_hpp_included_
