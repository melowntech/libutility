/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
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
