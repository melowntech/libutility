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
#ifndef utility_substream_hpp_included_
#define utility_substream_hpp_included_

#include <memory>

#include <boost/filesystem/path.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>

namespace utility { namespace io {

/** Input device for part of file. Can be pointed at any point.
 */
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
