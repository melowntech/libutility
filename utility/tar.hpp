#ifndef utility_tar_hpp_included_
#define utility_tar_hpp_included_

#include <ctime>
#include <array>

#include <boost/filesystem/path.hpp>

#include "./filedes.hpp"

namespace utility { namespace tar {

enum class Type { invalid, ustar, posix };

struct Header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];

    Type type() const;

    bool valid() const { return type() != Type::invalid; }

    std::size_t getSize() const;

    std::size_t getBlocks() const {
        return (getSize() + 511UL) / 512UL;
    }

    std::size_t getBlocksBytes() const {
        return getBlocks() * 512UL;
    }

    boost::filesystem::path getPath() const;

    bool isFile() const;

    std::time_t getTime() const;
};

union Block {
    std::array<char, 512> raw;
    Header header;
};

class Reader {
public:
    Reader() : cursor_() {}

    Reader(const boost::filesystem::path &path);

    void seek(std::size_t blocks);

    void advance(std::size_t blocks);

    void skip(const Header &header);

    /** Reads one block from current file position.
     *  Returns false on EOF.
     *  Throws system_error on error.
     */
    bool read(Block &block);

    std::size_t cursor() const { return cursor_; }

    typedef std::vector<char> Data;

    /** Reads given amount of bytes starting at given block number.
     */
    Data readData(std::size_t block, std::size_t size);

private:
    const boost::filesystem::path path_;
    utility::Filedes fd_;

    std::size_t cursor_;
};

inline void Reader::skip(const Header &header)
{
    advance(header.getBlocks());
}

} } // namespace utility::tar

#endif // utility_tar_hpp_included_
