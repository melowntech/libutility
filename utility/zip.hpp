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
