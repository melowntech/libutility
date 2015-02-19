#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <tar.h>

#include <system_error>

#include "dbglog/dbglog.hpp"

#include "./tar.hpp"

namespace utility { namespace tar {

namespace fs = boost::filesystem;

namespace {

template <std::size_t size>
std::uint64_t parse(const char(&v)[size])
{
    char *e(nullptr);
    auto r(std::strtoul(v, &e, 8));
    return r;
}

template <std::size_t size>
std::string getString(const char(&v)[size])
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
    if (!std::memcmp(TMAGIC, magic, TMAGLEN)
        && !std::memcmp(TVERSION, version, TVERSLEN))
    {
        return Type::ustar;
    }

    if (!std::memcmp("ustar  ", magic, 8)) {
        return Type::posix;
    }

    return Type::invalid;
}

std::size_t Header::getSize() const {
    return parse(size);
}


fs::path Header::getPath() const {
    auto p(getString(prefix));
    auto n(getString(name));
    if (p.empty()) { return n; }
    return fs::path(p) / n;
}

bool Header::isFile() const {
    return (*typeflag == REGTYPE) || (*typeflag == AREGTYPE);
}

Reader::Reader(const fs::path &path)
    : path_(path), fd_(::open(path.string().c_str(), O_RDONLY))
    , cursor_(0)
{
    if (fd_ == -1) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot open tar file " << path_ << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }
}

void Reader::seek(std::size_t blocks)
{
    auto res(::lseek(fd_, blocks * 512, SEEK_SET));
    if (res == -1) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot seek in tar file " << path_ << ": <"
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
        LOG(err2) << "Cannot seek in tar file " << path_ << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    cursor_ += blocks;
}

bool Reader::read(Block &block)
{
    auto bytes(TEMP_FAILURE_RETRY(::read(fd_, block.raw.data(), 512)));
    if (bytes == -1) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot read from tar file " << path_ << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    if (!bytes) { return false; }

    if (bytes != 512) {
        LOGTHROW(err2, std::runtime_error)
            << "Short read from tar file " << path_ << ".";
    }

    ++cursor_;
    return true;
}

Reader::Data Reader::readData(std::size_t block, std::size_t size)
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
            LOG(err2) << "Cannot read from tar file " << path_ << ": <"
                      << e.code() << ", " << e.what() << ">.";
            throw e;
        }

        size -= bytes;
        p += bytes;
    }

    if (size) {
        LOGTHROW(err2, std::runtime_error)
            << "Too few data in " << path_ << " at position "
            << block << ".";
    }

    return data;
}

} } // namespace utility::tar
