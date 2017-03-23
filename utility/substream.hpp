#ifndef utility_substream_hpp_included_
#define utility_substream_hpp_included_

#include <memory>

#include <boost/filesystem/path.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>

namespace utility { namespace io {

struct SubStreamDevice {
    typedef char char_type;
    struct category : boost::iostreams::device_tag
                    , boost::iostreams::input_seekable {};

    /** Simple structure for file descriptor and block start/end.
     */
    struct Filedes {
        int fd;
        std::size_t start;
        std::size_t end;
    };

    SubStreamDevice(const boost::filesystem::path &path
                    , const Filedes &fd)
        : path_(path), fd_(fd), pos_(fd.start)
    {}

    std::streampos seek(boost::iostreams::stream_offset off
                        , std::ios_base::seekdir way);

    std::streamsize read(char *data, std::streamsize size
                         , boost::iostreams::stream_offset pos)
    {
        // read data from given position
        auto bytes(read_impl(data, size, pos + fd_.start));
        // update position after read block
        pos_ = fd_.start + pos + bytes;
        return bytes;
    }

    std::streamsize read(char *data, std::streamsize size) {
        auto bytes(read_impl(data, size, pos_));
        pos_ += bytes;
        return bytes;
    }

    const boost::filesystem::path& path() const { return path_; }

private:
    std::streamsize read_impl(char *data, std::streamsize size
                              , boost::iostreams::stream_offset pos);

    boost::filesystem::path path_;
    Filedes fd_;
    boost::iostreams::stream_offset pos_;
};

} } // namespace utility::io

#endif // utility_tar_substream_included_
