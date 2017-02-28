#include <unistd.h>

#include <system_error>

#include "./raise.hpp"
#include "./substream.hpp"

namespace utility { namespace io {

std::streampos SubStreamDevice::seek(boost::iostreams::stream_offset off
                                     , std::ios_base::seekdir way)
{
    std::int64_t newPos(0);

    switch (way) {
    case std::ios_base::beg:
        newPos = fd_.start + off;
        break;

    case std::ios_base::end:
        newPos = fd_.end + off;
        break;

    case std::ios_base::cur:
        newPos = pos_ + off;
        break;

    default: // shut up compiler!
        break;
    };

    if (newPos < std::int64_t(fd_.start)) {
        pos_ = fd_.start;
    } else if (newPos > std::int64_t(fd_.end)) {
        pos_ = fd_.end;
    } else {
        pos_ = newPos;
    }

    return (pos_ - fd_.start);
}

std::streamsize
SubStreamDevice::read_impl(char *data, std::streamsize size
                           , boost::iostreams::stream_offset pos)
{
    // trim if out of range
    auto end(fd_.end);
    if (size > std::streamsize(end - pos)) {
        size = end - pos;
    }

    if (!size) { return size; }

    auto bytes(::pread(fd_.fd, data, size, pos));
    if (-1 == bytes) {
        std::system_error e
            (errno, std::system_category()
             , utility::formatError
             ("Unable to read from tilar file %s.", path_));
        throw e;
    }
    return bytes;
}

} } // namespace utility::io
