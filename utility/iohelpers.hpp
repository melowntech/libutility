#ifndef utility_iohelpers_hpp_included_
#define utility_iohelpers_hpp_included_

#include <boost/filesystem/path.hpp>

#include "streams.hpp"

namespace utility {

template<typename Dumpable>
void dumpToFile(const Dumpable &object, const boost::filesystem::path &path)
{
    utility::ofstreambuf f;
    f.exceptions(std::ios::badbit | std::ios::failbit);
    f.open(path.native(), std::ios_base::out | std::ios_base::trunc);
    object.dump(f);
    f.close();
}

template<typename Loadable>
void loadFromFile(Loadable &object, const boost::filesystem::path &path)
{
    utility::ifstreambuf f;
    f.exceptions(std::ios::badbit | std::ios::failbit);
    f.open(path.native(), std::ios_base::in);
    object.load(f);
    f.close();
}

} // namespace utility

#endif // utility_iohelpers_hpp_included_
