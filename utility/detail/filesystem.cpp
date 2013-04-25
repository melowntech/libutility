#include <boost/filesystem/operations.hpp>

#include "filesystem.hpp"

namespace utility { namespace detail {

void copy_file(const boost::filesystem::path &from
               , const boost::filesystem::path &to)
{
    boost::filesystem::copy_file(from, to);
}

} } // namespace utility::detail
