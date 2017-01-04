#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#include <sys/types.h>

#include <system_error>

#include "dbglog/dbglog.hpp"

#include "./persona.hpp"

namespace utility {

Persona switchPersona(const char *username, const char *groupname
                      , bool privilegesRegainable)
{
    // choose proper uid/gid setter
    auto &uidSetter(privilegesRegainable ? ::seteuid : ::setuid);
    auto &gidSetter(privilegesRegainable ? ::setegid : ::setgid);

    bool switchUid(false);
    bool switchGid(false);
    Persona persona;
    persona.start.loadEffectivePersona();
    persona.running = persona.start;

    if (!username && !groupname) { return persona; }

    auto log([](const char *s) { return s ? s : "-"; });

    LOG(info3)
        << "Trying to run under " << log(username)
        << ":" << log(groupname) << ".";
    if (username) {
        auto pwd(::getpwnam(username));
        if (!pwd) {
            LOGTHROW(err3, std::runtime_error)
                << "There is no user <" << username
                << "> present on the system.";
            throw;
        }

        // get uid and gid
        persona.running.uid = pwd->pw_uid;
        persona.running.gid = pwd->pw_gid;
        switchUid = switchGid = true;
    }

    if (groupname) {
        auto gr(::getgrnam(groupname));
        if (!gr) {
            LOGTHROW(err3, std::runtime_error)
                << "There is no group <" << groupname
                << "> present on the system.";
            throw;
        }
        persona.running.gid = gr->gr_gid;
        switchGid = true;
    }

    // change log file owner to uid/gid before persona change
    dbglog::log_file_owner(persona.running.uid, persona.running.gid);

    // TODO: check whether we do not run under root!

    if (switchGid) {
        LOG(info3) << "Switching to gid <" << persona.running.gid << ">.";
        if (-1 == gidSetter(persona.running.gid)) {
            std::system_error e(errno, std::system_category());
            LOG(fatal)
                << "Cannot switch to gid <" << persona.running.gid << ">: "
                << "<" << e.code() << ", " << e.what() << ">.";
            throw e;
        }
    }

    // set supplementary groups, only when switching user and current user is
    // root
    if (switchUid) {
        LOG(info3)
            << "Setting supplementary groups for user <"
            << username << ">.";
        if (-1 == ::initgroups(username, persona.running.gid)) {
            if (errno != EPERM) {
                std::system_error e(errno, std::system_category());
                LOG(fatal)
                    << "Cannot initialize supplementary groups for user <"
                    << username << ">: <" << e.code()
                    << ", " << e.what() << ">.";
                throw e;
            } else {
                LOG(warn2)
                    << "Insufficient privilege to set supplementary groups.";
            }
        }

        LOG(info3) << "Switching to uid <" << persona.running.uid << ">.";
        if (-1 == uidSetter(persona.running.uid)) {
            std::system_error e(errno, std::system_category());
            LOG(fatal)
                << "Cannot switch to uid <" << persona.running.uid << ">: "
                << "<" << e.code() << ", " << e.what() << ">.";
            throw e;
        }
    }

    LOG(info3)
        << "Run under " << log(username) << ":" << log(groupname) << ".";

    return persona;
}

} // namespace utility
