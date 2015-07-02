#ifndef utility_filesystem_hpp_included_
#define utility_filesystem_hpp_included_

#include <boost/filesystem/path.hpp>

#include "detail/filesystem.hpp"

namespace utility {

enum class LineProcessorResult { next, pass, stop };

typedef std::function< std::pair<std::string, LineProcessorResult>
		(const std::string &line, std::size_t lineIndex) > LineProcessor;

void copy_file(const boost::filesystem::path &from
               , const boost::filesystem::path &to
               , bool overwrite);

void copy_file(const boost::filesystem::path &from
               , const boost::filesystem::path &to
               , bool overwrite
               , boost::system::error_code &ec);

void processFile( const boost::filesystem::path &from
                , const boost::filesystem::path &to
                , bool overwrite
                , const LineProcessor &lineProcessor);

void copyTree(const boost::filesystem::path &from
              , const boost::filesystem::path &to);

void copyTree(const boost::filesystem::path &from
              , const boost::filesystem::path &to
              , boost::system::error_code &ec);

std::time_t lastModified(const boost::filesystem::path &path);

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
