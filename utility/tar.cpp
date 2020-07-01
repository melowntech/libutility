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
#include <sys/stat.h>
#include <fcntl.h>

#include <system_error>

#include "utility/unistd_compat.hpp"
#include "dbglog/dbglog.hpp"

#include "tar.hpp"


#ifdef _WIN32
 // taken from https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/tar.h.html
#define REGTYPE '0'
#define AREGTYPE '\0'
#define TMAGIC "ustar"
#define TMAGLEN 6
#define TVERSION "00"
#define TVERSLEN 2
#else
#include <tar.h>
#endif


namespace utility { namespace tar {

namespace fs = boost::filesystem;

namespace {

std::uint64_t parse(const char *v)
{
    char *e(nullptr);
    auto r(std::strtoul(v, &e, 8));
    return r;
}

template <std::size_t size>
std::string getString(const char *v)
{
    if (v[size - 1]) {
        // no NUL terminator
        return { v, v + size };
    }
    // with terminator
    return { v };
}

} // namespace

Type Header::type() const
{
    if (!std::memcmp(TMAGIC, magic(), TMAGLEN)
        && !std::memcmp(TVERSION, version(), TVERSLEN))
    {
        return Type::ustar;
    }

    if (!std::memcmp("ustar  ", magic(), 8)) {
        return Type::posix;
    }

    return Type::invalid;
}

std::size_t Header::getSize() const {
    return parse(size());
}

std::time_t Header::getTime() const {
    return parse(mtime());
}

fs::path Header::getPath() const {
    auto p(getString<155>(prefix()));
    auto n(getString<100>(name()));
    if (p.empty()) { return n; }
    return fs::path(p) / n;
}

bool Header::isFile() const {
    return (*typeflag() == REGTYPE) || (*typeflag() == AREGTYPE);
}

Reader::Reader(const fs::path &path)
    : path_(path), fd_(::open(path.string().c_str(), O_RDONLY), path)
    , cursor_(0)
{
    if (!fd_) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot open tar file " << fd_.path() << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }
}

void Reader::seek(std::size_t blocks)
{
    auto res(::lseek(fd_, blocks * 512, SEEK_SET));
    if (res == -1) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot seek in tar file " << fd_.path() << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    cursor_ = blocks;
}

void Reader::advance(std::size_t blocks)
{
    auto res(::lseek(fd_, blocks * 512, SEEK_CUR));
    if (res == -1) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot seek in tar file " << fd_.path() << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    cursor_ += blocks;
}

bool Reader::read(Block &block)
{
    auto bytes(TEMP_FAILURE_RETRY
               (::read(fd_, block.data.data(), block.data.size())));
    if (bytes == -1) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot read from tar file " << fd_.path() << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    if (!bytes) { return false; }

    if (bytes != 512) {
        LOGTHROW(err2, std::runtime_error)
            << "Short read from tar file " << fd_.path() << ".";
    }

    ++cursor_;
    return true;
}

Data Reader::readData(std::size_t block, std::size_t size)
{
    seek(block);

    Data data(size, 0);
    char *p(data.data());

    while (size) {
        auto bytes(TEMP_FAILURE_RETRY(::read(fd_, p, size)));
        if (!bytes) {
            break;
        }

        if (bytes == -1) {
            std::system_error e(errno, std::system_category());
            LOG(err2) << "Cannot read from tar file " << fd_.path() << ": <"
                      << e.code() << ", " << e.what() << ">.";
            throw e;
        }

        size -= bytes;
        p += bytes;
    }

    if (size) {
        LOGTHROW(err2, std::runtime_error)
            << "Too few data in " << fd_.path() << " at position "
            << block << ".";
    }

    return data;
}

Reader::Filedes Reader::filedes(std::size_t block, std::size_t size)
{
    return { fd_.get(), std::size_t(block * 512)
            , std::size_t(block * 512) + size };
}

/** Build index.
 */
Reader::File::list Reader::files(std::size_t limit)
{
    // rewind
    seek(0);

    File::list files;

    for (Header header; read(header); ) {
        if (!header.valid()) {
            continue;
        }

        if (header.isFile()) {
            std::size_t start(cursorByte());
            files.emplace_back(header.getPath()
                               , start, header.getSize());

            // apply limit
            if (files.size() >= limit) { break; }
        }

        // skip file/whatever content
        skip(header);
    }

    return files;
}

#if 0
// TODO: implement me
namespace {

int flagsToOpenFlags(int inflags)
{
    flags = O_WRONLY | O_APPEND;
    if (inflags & Writer::Flags::truncate) { flags |= O_TRUNC; }
    if (inflags & Writer::Flags::create) { flags |= O_CREAT; }
    if (inflags & Writer::Flags::exclusive) { flags |= O_EXCL; }
}

} // namespace

Writer::Writer(const boost::filesystem::path &path, int flags)
    : path_(path), fd_(::open(path.string().c_str(), flagsToOpenFlags(flags))
                       , path)
{
    if (!fd_) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot create/overwrite/append tar file "
                  << fd_.path() << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    // TODO: handle 
}

void Writer::write(const Header &header, std::size_t block, std::size_t size)
{
    
}

void Writer::write(const Header &header, const boost::filesystem &file)
{
    
}
#endif

} } // namespace utility::tar
