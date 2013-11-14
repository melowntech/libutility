#ifndef utility_filesystem_hpp_included_
#define utility_filesystem_hpp_included_

#include <boost/filesystem/path.hpp>

#include "detail/filesystem.hpp"

namespace utility {

void copy_file(const boost::filesystem::path &from
               , const boost::filesystem::path &to
               , bool overwrite);

void copy_file(const boost::filesystem::path &from
               , const boost::filesystem::path &to
               , bool overwrite
               , boost::system::error_code &ec);

void copyTree(const boost::filesystem::path &from
              , const boost::filesystem::path &to
              , bool overwrite);

void copyTree(const boost::filesystem::path &from
              , const boost::filesystem::path &to
              , bool overwrite
              , boost::system::error_code &ec);

} // namespace utility

// impelemtation

inline void utility::copy_file(const boost::filesystem::path &from
                               , const boost::filesystem::path &to
                               , bool overwrite)
{
    return utility::detail::copy_file(from, to, overwrite);
}

inline void utility::copy_file(const boost::filesystem::path &from
                               , const boost::filesystem::path &to
                               , bool overwrite
                               , boost::system::error_code &ec)
{
    return utility::detail::copy_file(from, to, overwrite, ec);
}

#endif // utility_filesystem_hpp_included_
