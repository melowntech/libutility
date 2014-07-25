#include <pwd.h>
#include <grp.h>

#include <system_error>

#include <boost/lexical_cast.hpp>

#include "dbglog/dbglog.hpp"

#include "./identity.hpp"

namespace utility {

void Identity::loadEffectivePersona()
{
    uid = geteuid();
    gid = getegid();
}

Identity Identity::effectivePersona()
{
    Identity i;
    i.loadEffectivePersona();
    return i;
}

Identity NamedIdentity::resolve()
{
    Identity p;
    try {
        p.uid = boost::lexical_cast< ::uid_t>(username);
    } catch (boost::bad_lexical_cast) {
        auto pwd(::getpwnam(username.c_str()));
        if (!pwd) {
            LOGTHROW(err1, std::runtime_error)
                << "There is no user <" << username
                << "> present on the system.";
        }
        p.uid = pwd->pw_uid;
    }

    try {
        p.gid = boost::lexical_cast< ::gid_t>(groupname);
    } catch (boost::bad_lexical_cast) {
        auto gr(::getgrnam(groupname.c_str()));
        if (!gr) {
            LOGTHROW(err1, std::runtime_error)
                << "There is no group <" << groupname
                << "> present on the system.";
        }
        p.gid = gr->gr_gid;
    }

    return p;
}

void setEffectivePersona(const Identity &persona)
{
    if (setegid(persona.gid) == -1) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot change effective gid to " << persona.gid << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    if (seteuid(persona.uid) == -1) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot change effective uid to " << persona.uid << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }
}

} // namespace utility
