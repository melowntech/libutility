#ifndef utility_tar_hpp_included_
#define utility_tar_hpp_included_

#include <ctime>
#include <array>

#include <boost/filesystem/path.hpp>

#include "./filedes.hpp"

namespace utility { namespace tar {

enum class Type { invalid, ustar, posix };

struct Block {
    std::array<char, 512> data;
};

struct Header : Block {
    char* name() { return Block::data.data(); }
    const char* name() const { return Block::data.data(); }

    char* mode() { return Block::data.data() + 100; }
    const char* mode() const { return Block::data.data() + 100; }

    char* uid() { return Block::data.data() + 108; }
    const char* uid() const { return Block::data.data() + 108; }

    char* gid() { return Block::data.data() + 116; }
    const char* gid() const { return Block::data.data() + 116; }

    char* size() { return Block::data.data() + 124; }
    const char* size() const { return Block::data.data() + 124; }

    char* mtime() { return Block::data.data() + 136; }
    const char* mtime() const { return Block::data.data() + 136; }

    char* chksum() { return Block::data.data() + 148; }
    const char* chksum() const { return Block::data.data() + 148; }

    char* typeflag() { return Block::data.data() + 156; }
    const char* typeflag() const { return Block::data.data() + 156; }

    char* linkname() { return Block::data.data() + 157; }
    const char* linkname() const { return Block::data.data() + 157; }

    char* magic() { return Block::data.data() + 257; }
    const char* magic() const { return Block::data.data() + 257; }

    char* version() { return Block::data.data() + 263; }
    const char* version() const { return Block::data.data() + 263; }

    char* uname() { return Block::data.data() + 265; }
    const char* uname() const { return Block::data.data() + 265; }

    char* gname() { return Block::data.data() + 297; }
    const char* gname() const { return Block::data.data() + 297; }

    char* devmajor() { return Block::data.data() + 329; }
    const char* devmajor() const { return Block::data.data() + 329; }

    char* devminor() { return Block::data.data() + 337; }
    const char* devminor() const { return Block::data.data() + 337; }

    char* prefix() { return Block::data.data() + 345; }
    const char* prefix() const { return Block::data.data() + 345; }

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

typedef std::vector<char> Data;

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

    std::size_t cursorByte() const { return cursor_ * 512; }

    /** Reads given amount of bytes starting at given block number.
     */
    Data readData(std::size_t block, std::size_t size);

    /** Simple structure for file descriptor and block start/end.
     */
    struct Filedes {
        int fd;
        std::size_t start;
        std::size_t end;
    };

    /** Return file descriptor for block index and size.
     */
    Filedes filedes(std::size_t block, std::size_t size);

    int filedes() const { return fd_; }

    /** File info.
     */
    struct File {
        typedef std::vector<File> list;

        boost::filesystem::path path;
        std::size_t start;
        std::size_t size;
        std::size_t end() const { return start + size; }

        File(boost::filesystem::path path, std::size_t start
             , std::size_t size)
            : path(path), start(start), size(size)
        {}
    };

    File::list files();

    const boost::filesystem::path& path() const { return path_; }

private:
    boost::filesystem::path path_;

    utility::Filedes fd_;

    std::size_t cursor_;
};

class Writer {
public:
    enum Flags {
        truncate = 0x01
        , create = 0x02
        , exclusive = 0x04
    };

    Writer(const boost::filesystem::path &path
           , int flags = Flags::create | Flags::exclusive | Flags::truncate);

    void write(const Header &header, std::size_t block, std::size_t size);

    void write(const Header &header, const boost::filesystem::path &file);

    /** Write file terminator
     */
    void end();

private:
    boost::filesystem::path path_;

    utility::Filedes fd_;
};

// inlines

inline void Reader::skip(const Header &header)
{
    advance(header.getBlocks());
}

} } // namespace utility::tar

#endif // utility_tar_hpp_included_
