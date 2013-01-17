#ifndef utility_small_map_serialization_hpp_included_
#define utility_small_map_serialization_hpp_included_

#include <boost/serialization/vector.hpp>

#include "small_map.hpp"

namespace utility {

template<typename Key, typename T, typename Compare, typename Allocator>
template<typename Archive>
inline void small_map<Key, T, Compare, Allocator>
::serialize(Archive &ar, const unsigned int version)
{
    return boost::serialization::serialize(ar, storage_, version);
}

} // namespace utility

 #endif // utility_small_map_serialization_hpp_included_
