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
/**
 * \desc: Zip file support
 * \file: zip.hpp
 * \author: Vaclav Blazek <vaclav.blazek@citationtech.net>
 */

#ifndef utility_zip_hpp_included_
#define utility_zip_hpp_included_

#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include "./filedes.hpp"
#include "./substream.hpp"

namespace utility { namespace zip {

struct Error : std::runtime_error {
    Error(const std::string &e) : std::runtime_error(e) {}
};

struct BadSignature : Error {
    BadSignature(const std::string &e) : Error(e) {}
};

struct PluggedFile {
    boost::filesystem::path path;
    std::size_t uncompressedSize;

    PluggedFile(boost::filesystem::path path, std::size_t uncompressedSize)
        : path(path), uncompressedSize(uncompressedSize)
    {}
};

class Reader {
public:
    Reader(const boost::filesystem::path &path);

    /** File record.
     */
    struct Record {
        typedef std::vector<Record> list;

        std::size_t index;
        boost::filesystem::path path;
        std::size_t headerStart;

        Record(std::size_t index, const boost::filesystem::path &path
               , std::size_t headerStart)
            : index(index), path(path), headerStart(headerStart)
        {}
    };

    const Record::list& files() const { return records_; }

    /** Plug decompressing stream for file at given index at the end of the
     *  filtering_istream.
     */
    PluggedFile plug(std::size_t index
                     , boost::iostreams::filtering_istream &fis) const;

private:
    boost::filesystem::path path_;

    /** Open file descriptor.
     */
    Filedes fd_;

    /** Total lenght of file
     */
    std::size_t fileLength_;

    /** List of records.
     */
    Record::list records_;
};

} } // namespace utility::zip

#endif // utility_zip_hpp_included_
