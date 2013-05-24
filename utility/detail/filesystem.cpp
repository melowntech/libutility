#include <boost/filesystem/operations.hpp>

#include "filesystem.hpp"

namespace fs = boost::filesystem;

namespace utility { namespace detail {

void copy_file(const boost::filesystem::path &from
               , const boost::filesystem::path &to
               , bool overwrite)
{
   copy_file(from, to, (overwrite ? fs::copy_option::overwrite_if_exists
                        : fs::copy_option::fail_if_exists));
}

void copy_file(const boost::filesystem::path &from
               , const boost::filesystem::path &to
               , bool overwrite
               , boost::system::error_code& ec)
{
   copy_file(from, to, (overwrite ? fs::copy_option::overwrite_if_exists
                        : fs::copy_option::fail_if_exists), ec);
}

} } // namespace utility::detail
