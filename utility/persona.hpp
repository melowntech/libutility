#ifndef utility_persona_hpp_included_
#define utility_persona_hpp_included_

#include "./identity.hpp"

namespace utility {

/** Process persona.
 */
struct Persona {
    /** Persona at before switch
     */
    utility::Identity start;

    /** Persona after switch
     */
    utility::Identity running;

};

/** Switch persona to given username:groupname.
 *
 *  When switching uid, supplementary groups are used as well
 *
 *  \param username switch uid to username's gid (if non-null)
 *  \param groupname switch gid to groupname's gid (if non-null)
 *  \param privilegesRegainable allow return to original persona
 */
Persona switchPersona(const char *username, const char *groupname
                      , bool privilegesRegainable = false);

} // namespace utility

#endif // utility_persona_hpp_included_
