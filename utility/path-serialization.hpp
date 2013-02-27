/**
 * @file detail/path.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Boost.Filesystem path support
 */

#ifndef utility_path_serialization_hpp_included_
#define utility_path_serialization_hpp_included_

#include <boost/filesystem/path.hpp>
#include <boost/serialization/split_free.hpp>

namespace boost { namespace serialization {

template<class Archive>
inline void save(Archive &ar, const boost::filesystem::path &p
                     , unsigned int version)
{
    (void) version;
    ar & p.string();
}

template<class Archive>
inline void load(Archive &ar, boost::filesystem::path &p
                 , unsigned int version)
{
    (void) version;

    std::string raw;
    ar & raw;
    p = raw;
}

} } // namespace boost::serialization;

BOOST_SERIALIZATION_SPLIT_FREE(boost::filesystem::path)

#endif // utility_path_serialization_hpp_included_
