#ifndef shared_utility_thread_hpp_included_
#define shared_utility_thread_hpp_included_

#include <string>

namespace utility { namespace thread {

/** Sets current thread's name to give name.
 */
void setName(const std::string &name);

/** Appends given name to current thread's name.
 */
void appendName(const std::string &name);

} } // namespace utility::thread

#endif // shared_utility_thread_hpp_included_
