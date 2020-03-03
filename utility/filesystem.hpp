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

#ifndef utility_filesystem_hpp_included_
#define utility_filesystem_hpp_included_

#include <new>
#include <ctime>
#include <map>

#include <boost/filesystem/path.hpp>

#include "detail/filesystem.hpp"

namespace utility {

enum class LineProcessorResult { next, pass, stop };

typedef std::function< std::pair<std::string, LineProcessorResult>
        (const std::string &line, std::size_t lineIndex) > LineProcessor;

void copy_file(const boost::filesystem::path &from
               , const boost::filesystem::path &to
               , bool overwrite);

void copy_file(const boost::filesystem::path &from
               , const boost::filesystem::path &to
               , bool overwrite
               , boost::system::error_code &ec);

void processFile( const boost::filesystem::path &from
                , const boost::filesystem::path &to
                , bool overwrite
                , const LineProcessor &lineProcessor);

void copyTree(const boost::filesystem::path &from
              , const boost::filesystem::path &to);

void copyTree(const boost::filesystem::path &from
              , const boost::filesystem::path &to
              , boost::system::error_code &ec);

std::time_t lastModified(const boost::filesystem::path &path);

std::size_t fileSize(const boost::filesystem::path &path);

std::map<std::string, boost::filesystem::path> scanDir(
    const boost::filesystem::path& root);

/** Generalized file ID. Wrapper around file devide/inode. Using uint64 to
 *  capture every possibility.
 */
struct FileId {
    std::uint64_t dev;
    std::uint64_t id;

    FileId(std::uint64_t dev, std::uint64_t id) : dev(dev), id(id) {}

    bool operator<(const FileId &fid) const;

    bool operator==(const FileId &fid) const;

    bool operator!=(const FileId &fid) const;

    static FileId from(const boost::filesystem::path &path);
};

/** Generalized file statistics.
 */
struct FileStat {
    std::time_t modified;
    std::size_t size;
    FileId id;

    FileStat(std::time_t modified, std::size_t size, const FileId &id)
        : modified(modified), size(size), id(id)
    {}

    bool changed(const FileStat &fs) const;

    static FileStat from(const boost::filesystem::path &path);
    static FileStat from(const boost::filesystem::path &path
                         , std::nothrow_t);
    static FileStat from(int fd);
    static FileStat from(int fd, std::nothrow_t);
};

// impelemtation

inline bool FileId::operator<(const FileId &fid) const {
    if (dev < fid.dev) { return true; }
    if (fid.dev < dev) { return false; }
    return id < fid.id;
}

inline bool FileId::operator==(const FileId &fid) const {
    return (dev == fid.dev) && (id == fid.id);
}

inline bool FileId::operator!=(const FileId &fid) const {
    return (dev != fid.dev) || (id != fid.id);
}

inline bool FileStat::changed(const FileStat &fs) const
{
    return ((modified != fs.modified)
            || (size != fs.size)
            || (id != fs.id));
}

inline void copy_file(const boost::filesystem::path &from
                      , const boost::filesystem::path &to
                      , bool overwrite)
{
    return detail::copy_file(from, to, overwrite);
}

inline void copy_file(const boost::filesystem::path &from
                      , const boost::filesystem::path &to
                      , bool overwrite
                      , boost::system::error_code &ec)
{
    return detail::copy_file(from, to, overwrite, ec);
}

} // namespace utility

#endif // utility_filesystem_hpp_included_
