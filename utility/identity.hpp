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
#ifndef utility_identity_hpp_included_
#define utility_identity_hpp_included_

#include <sys/types.h>

#include <iostream>
#include <string>
#include <stdexcept>


namespace utility {

struct Identity {
    ::uid_t uid;
    ::gid_t gid;

    Identity() : uid(-1), gid(-1) {}

    /** Loads current effective persona of process.
     */
    void loadEffectivePersona();

    static Identity effectivePersona();

    bool hasValidUid() const;

    bool hasValidGid() const;
};

void setEffectivePersona(const Identity &persona);

void setRealPersona(const Identity &persona);

class ScopedPersona {
public:
    ScopedPersona(const Identity &ep);
    ~ScopedPersona();

private:
    Identity saved_;
};

struct NamedIdentity {
    std::string username;
    std::string groupname;

    Identity resolve() const;
};

// inlines

template<typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os, const Identity &p)
{
    return os << p.uid << ":" << p.gid;
}

template<typename CharT, typename Traits>
inline std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &is, NamedIdentity &p)
{
    std::getline(is, p.username, ':');
    is >> p.groupname;
    return is;
}

template<typename CharT, typename Traits>
inline std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &is, Identity &p)
{
    NamedIdentity np;
    is >> np;
    try {
        p = np.resolve();
    } catch (std::runtime_error) {
        is.setstate(std::ios::failbit);
    }
    return is;
}

inline bool Identity::hasValidUid() const
{
    const Identity invalid;
    return uid != invalid.uid;
}

inline bool Identity::hasValidGid() const
{
    const Identity invalid;
    return gid != invalid.gid;
}

} // namespace utility

#endif // utility_identity_hpp_included_
