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
/**
 * @file buildsys.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Gets constants from build system.
 */

#ifndef utility_buildsys_hpp_included_
#define utility_buildsys_hpp_included_

#include "boost/filesystem/path.hpp"

namespace utility { namespace buildsys {

/** Updates path to be under installation prefix.
 */
boost::filesystem::path installPath(const boost::filesystem::path &p);

// Fix missing macros
#ifndef BUILD_TARGET_NAME
#    define BUILD_TARGET_NAME "unknown"
#endif

#ifndef BUILD_TARGET_VERSION
#    define BUILD_TARGET_VERSION "unknown"
#endif

#ifndef BUILDSYS_INSTALL_PREFIX
#    define BUILDSYS_INSTALL_PREFIX "/usr/local"
#endif

#ifndef BUILDSYS_HOSTNAME
#    define BUILDSYS_HOSTNAME "unknown"
#endif

#ifdef BUILDSYS_CUSTOMER_BUILD
#    ifndef BUILDSYS_CUSTOMER
#        define BUILDSYS_CUSTOMER "none"
#    endif
#else
#    define BUILDSYS_CUSTOMER "none"
#endif

/** Target name.
 */
constexpr char TargetName[] = BUILD_TARGET_NAME;

/** Target version.
 */
constexpr char TargetVersion[] = BUILD_TARGET_VERSION;

/** Target installation prefix.
 */
constexpr char InstallPrefix[] = BUILDSYS_INSTALL_PREFIX;

/** Hostname this build is being compiled at.
 */
constexpr char Hostname[] = BUILDSYS_HOSTNAME;

/** Customer this build is compiled for.
 */
constexpr char Customer[] = BUILDSYS_CUSTOMER;



// inline stuff

inline boost::filesystem::path installPath(const boost::filesystem::path &p)
{
    return InstallPrefix / p;
}

} } // namespace utility::buildsys

#endif // utility_buildsys_hpp_included_
