/**
 * @file utility/limits.hpp
 * @author Vaclav Blazek <vaclav.blazek@melown.com>
 */

#ifndef utility_limits_hpp_included_
#define utility_limits_hpp_included_

namespace utility {

/** Sets unlimited core dump size.
 *
 *  Unlimited means:
 *      * privileged user: sets both soft and hard limit to RLIM_INFINITY
 *      * non-privileged user: sets soft limit to the value of hard limit
 *
 *  Returns false on failure.
 */
bool unlimitedCoredump();

} // namespace utility

#endif // utility_limits_hpp_included_
