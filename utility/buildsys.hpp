/**
 * @file buildsys.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Gets constants from build system.
 */

#ifndef utility_buildsys_hpp_included_
#define utility_buildsys_hpp_included_

namespace utility { namespace buildsys {

#ifndef BUILD_TARGET_NAME
#    define BUILD_TARGET_NAME "unknown"
#endif
constexpr char TargetName[] = BUILD_TARGET_NAME;

#ifndef BUILD_TARGET_VERSION
#    define BUILD_TARGET_VERSION "unknown"
#endif
constexpr char TargetVersion[] = BUILD_TARGET_VERSION;

#ifndef BUILDSYS_INSTALL_PREFIX
#    define BUILDSYS_INSTALL_PREFIX "/usr/local"
#endif
constexpr char InstallPrefix[] = BUILDSYS_INSTALL_PREFIX;

#ifndef BUILDSYS_HOSTNAME
#    define  BUILDSYS_HOSTNAME "unknown"
#endif
constexpr char Hostname[] = BUILDSYS_HOSTNAME;

#ifdef BUILDSYS_CUSTOMER_BUILD
#    ifndef BUILDSYS_CUSTOMER
#        define BUILDSYS_CUSTOMER "none"
#    endif
#else
#    define BUILDSYS_CUSTOMER "none"
#endif
constexpr char Customer[] = BUILDSYS_CUSTOMER;

} } // namespace utility::buildsys

#endif // utility_buildsys_hpp_included_
