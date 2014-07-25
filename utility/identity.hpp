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
};

void setEffectivePersona(const Identity &persona);

struct NamedIdentity {
    std::string username;
    std::string groupname;

    Identity resolve();
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

} // namespace utility

#endif // utility_identity_hpp_included_
