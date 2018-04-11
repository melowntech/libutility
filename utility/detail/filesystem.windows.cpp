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

#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <system_error>

#include "dbglog/dbglog.hpp"

#include "../filesystem.hpp"
#include "../raise.hpp"

namespace utility {

FileId FileId::from(const boost::filesystem::path &path)
{
    struct ::_stat s;

    if (-1 == ::_stat(path.string().c_str(), &s)) {
        std::system_error e
            (errno, std::system_category()
             , utility::formatError("Cannot stat file %s.", path));
        LOG(err1) << e.what();
        throw e;
    }

    return FileId(s.st_dev, s.st_ino);
}

FileStat FileStat::from(const boost::filesystem::path &path)
{
    struct ::_stat s;

    if (-1 == ::_stat(path.string().c_str(), &s)) {
        std::system_error e
            (errno, std::system_category()
             , utility::formatError("Cannot stat file %s.", path));
        LOG(err1) << e.what();
        throw e;
    }

    return FileStat(s.st_mtime, s.st_size, FileId(s.st_dev, s.st_ino));
}

FileStat FileStat::from(const boost::filesystem::path &path, std::nothrow_t)
{
    struct ::_stat s;

    if (-1 == ::_stat(path.string().c_str(), &s)) {
        return FileStat(-1, 0, FileId(0, 0));
    }

    return FileStat(s.st_mtime, s.st_size, FileId(s.st_dev, s.st_ino));
}

FileStat FileStat::from(int fd)
{
    struct ::_stat s;

    if (-1 == ::_fstat(fd, &s)) {
        std::system_error e
            (errno, std::system_category()
             , utility::formatError("Cannot stat fd %d.", fd));
        LOG(err1) << e.what();
        throw e;
    }

    return FileStat(s.st_mtime, s.st_size, FileId(s.st_dev, s.st_ino));
}

FileStat FileStat::from(int fd, std::nothrow_t)
{
    struct ::_stat s;

    if (-1 == ::_fstat(fd, &s)) {
        return FileStat(-1, 0, FileId(0, 0));
    }

    return FileStat(s.st_mtime, s.st_size, FileId(s.st_dev, s.st_ino));
}

std::time_t lastModified(const boost::filesystem::path &path)
{
    struct ::_stat buf;
    if (-1 == ::_stat(path.string().c_str(), &buf)) {
        std::system_error e
            (errno, std::system_category()
             , utility::formatError("Cannot stat file %s.", path));
        LOG(err1) << e.what();
        throw e;
    }
    return buf.st_mtime;
}

std::size_t fileSize(const boost::filesystem::path &path)
{
    struct ::_stat buf;
    if (-1 == ::_stat(path.string().c_str(), &buf)) {
        std::system_error e
            (errno, std::system_category()
             , utility::formatError("Cannot stat file %s.", path));
        LOG(err1) << e.what();
        throw e;
    }
    return buf.st_size;
}

} // namespace utility
