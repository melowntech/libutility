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

#include <cstdlib>
#include <cerrno>
#include <system_error>
#include <winapifamily.h>
#include <Windows.h>

#include <boost/filesystem.hpp>

#include "dbglog/dbglog.hpp"

#include "path.hpp"

namespace fs = boost::filesystem;

namespace utility {

bool match(const std::string&, const boost::filesystem::path&, int)
{
    LOGTHROW(err3, std::runtime_error)
        << "utility::match unavailable on Windows. TODO: implement me.";
    return false;
}

boost::filesystem::path homeDir()
{
    // measure
    size_t size;
    if (auto err = ::getenv_s(&size, nullptr, 0, "USERPROFILE")) {
        std::system_error e(err, std::system_category());
        LOG(err3) << "Cannot determine home directory (getenv_s failed): <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    std::vector<char> buf(size, 0);
    ::getenv_s(&size, buf.data(), size, "USERPROFILE");
    return buf.data();
}

boost::optional<boost::filesystem::path> exePath()
{
#if WINAPI_PARTITION_DESKTOP
    std::vector<char> buf(MAX_PATH + 1);
    if (!::GetModuleFileName(nullptr, buf.data(), DWORD(buf.size()))) {
        std::system_error e(::GetLastError(), std::generic_category());
        LOG(err3)
            << "Cannot determine exe file path "
            "(GetModuleFileName failed): <"
            << e.code() << ", " << e.what() << ">.";
    }
    return boost::filesystem::path(buf.data());
#else
    return {};
#endif
}

} // namespace utility
