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
#include <unistd.h>
#include <fcntl.h>

#include <system_error>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/zlib.hpp>

#include "dbglog/dbglog.hpp"

#include "./binaryio.hpp"
#include "./zip.hpp"
#include "./enum-io.hpp"

namespace bin = utility::binaryio;
namespace fs = boost::filesystem;
namespace bio = boost::iostreams;

namespace utility { namespace zip {

namespace {

constexpr std::uint32_t LOCAL_HEADER_SIGNATURE = 0x04034b50;
constexpr std::uint32_t CENTRAL_DIRECTORY_FILE_HEADER_SIGNATURE = 0x02014b50;
constexpr std::uint32_t END_OF_CENTRAL_DIRECTORY_SIGNATURE = 0x06054b50;
constexpr std::uint32_t END_OF_CENTRAL_DIRECTORY64_SIGNATURE = 0x06064b50;
constexpr std::uint32_t END_OF_CENTRAL_DIRECTORY64_LOCATOR_SIGNATURE = 0x07064b50;

constexpr std::uint16_t Tag64 = 0x0001;

/** Should be used only for uint16 and uint32
 */
template <typename T>
bool invalid(T value)
{
    return value == std::numeric_limits<T>::max();
}

void readVector(std::istream &is, std::vector<char> &vec, std::size_t size)
{
    vec.assign(size, 0);
    is.read(&vec[0], size);
}

void readString(std::istream &is, std::string &str, std::size_t size)
{
    std::vector<char> tmp;
    readVector(is, tmp, size);
    str.assign(tmp.begin(), tmp.end());
}

void checkSignature(const std::string &what, std::istream &in
                    , const std::uint32_t expect)
{
    const auto signature(bin::read<std::uint32_t>(in));
    if (signature != expect) {
        LOGTHROW(err1, BadSignature)
            << "Error reading " << what << ": invalid signature;"
            << " expected 0x" << std::setfill('0')
            << std::hex << std::setw(8) << expect
            << " got 0x"
            << std::hex << std::setw(8) << signature
            << ".";
    }
}

const auto DeflateParams([]() -> bio::zlib_params {
        bio::zlib_params params;
        params.noheader = true;
        return params;
    }());

enum class CompressionMethod : std::uint16_t {
    store = 0
    , shrink = 1
    , reduce1 = 2
    , reduce2 = 3
    , reduce3 = 4
    , reduce4 = 5
    , implode = 6
    , deflate = 8
    , deflate64 = 9
    , pkwareImplode = 10
    , bzip2 = 12
    , lzma = 14
    , terse = 18
    , lz77 = 19
    , wavpack = 97
    , ppmd = 98
};

UTILITY_GENERATE_ENUM_IO(CompressionMethod,
                         ((store))
                         ((shrink))
                         ((reduce1))
                         ((reduce2))
                         ((reduce3))
                         ((reduce4))
                         ((implode))
                         ((deflate))
                         ((deflate64))
                         ((pkwareImplode))
                         ((bzip2))
                         ((lzma))
                         ((terse))
                         ((lz77))
                         ((wavpack))
                         ((ppmd))
                         )

const std::size_t
localFileHeaderSize(sizeof(std::uint32_t)
                    + sizeof(std::uint16_t) // versionNeeded
                    + sizeof(std::uint16_t) // flag)
                    + sizeof(std::uint16_t) // compressionMethod)
                    + sizeof(std::uint16_t) // modificationTime
                    + sizeof(std::uint16_t) // modificationDate
                    + sizeof(std::uint32_t) // crc32
                    + sizeof(std::uint32_t) // compressedSize
                    + sizeof(std::uint32_t) // uncompressedSize
                    + sizeof(std::uint16_t)
                    + sizeof(std::uint16_t)
                    );

const std::size_t maxFilenameSize(std::numeric_limits<std::uint16_t>::max());
const std::size_t maxExtraSize(std::numeric_limits<std::uint16_t>::max());

struct MinimalFileHeader {
    std::uint16_t compressionMethod;
    std::uint64_t compressedSize;
    std::uint64_t uncompressedSize;
    std::uint16_t filenameSize;
    std::uint16_t fileExtraSize;

    MinimalFileHeader()
        : compressionMethod(), compressedSize(), uncompressedSize()
        , filenameSize(), fileExtraSize()
    {}

    void read(std::istream &in);

    std::size_t size() const {
        return localFileHeaderSize + filenameSize + fileExtraSize;
    }
};

struct CentralDirectoryFileHeader {
    std::uint16_t versionNeeded;
    std::uint16_t flag;
    std::uint16_t compressionMethod;
    std::uint16_t modificationTime;
    std::uint16_t modificationDate;
    std::uint32_t crc32;
    std::uint64_t compressedSize;
    std::uint64_t uncompressedSize;
    std::string filename;
    std::uint16_t versionMadeBy;
    std::uint32_t diskNumberStart;
    std::uint16_t internalFileAttributes;
    std::uint32_t externalFileAttributes;
    std::uint64_t fileOffset;

    std::vector<char> fileExtra;
    std::vector<char> fileComment;

    typedef std::vector<CentralDirectoryFileHeader> list;

    CentralDirectoryFileHeader()
        : versionMadeBy()
        , diskNumberStart()
        , internalFileAttributes()
        , externalFileAttributes()
        , fileOffset()
    {}

    static CentralDirectoryFileHeader read(std::istream &in);
};

struct EndOfCentralDirectoryRecord {
    std::uint32_t numberOfThisDisk;
    std::uint32_t diskWhereCentralDirectoryStarts;
    std::uint64_t numberOfCentralDirectoryRecordsOnThisDisk;
    std::uint64_t totalNumberOfCentralDirectoryRecords;
    std::uint64_t sizeOfCentralDirectory;
    std::uint64_t centralDirectoryOffset;
    std::vector<char> comment;

    EndOfCentralDirectoryRecord()
        : numberOfThisDisk(0)
        , diskWhereCentralDirectoryStarts(0)
        , numberOfCentralDirectoryRecordsOnThisDisk(0)
        , totalNumberOfCentralDirectoryRecords(0)
        , sizeOfCentralDirectory(0)
        , centralDirectoryOffset(0)
    {}

    bool has64locator() const {
        return (invalid<std::uint16_t>(numberOfThisDisk)
                || invalid<std::uint16_t>(diskWhereCentralDirectoryStarts)
                || invalid<std::uint16_t>
                (numberOfCentralDirectoryRecordsOnThisDisk)
                || invalid<std::uint32_t>(totalNumberOfCentralDirectoryRecords)
                || invalid<std::uint32_t>(sizeOfCentralDirectory)
                || invalid<std::uint32_t>(centralDirectoryOffset));
    }

    static EndOfCentralDirectoryRecord read(std::istream &in);
    static EndOfCentralDirectoryRecord read64(std::istream &in);
};

struct EndOfCentralDirectory64Locator {
    std::uint32_t numberOfThisDisk;
    std::uint64_t endOfCentralDirectoryOffset;
    std::uint32_t totalNumberOfDisks;

    EndOfCentralDirectory64Locator()
        : numberOfThisDisk()
        , endOfCentralDirectoryOffset()
        , totalNumberOfDisks()
    {}

    static EndOfCentralDirectory64Locator read(std::istream &in);

    static std::size_t size() {
        return (sizeof(std::uint32_t) // signature
                + sizeof(numberOfThisDisk)
                + sizeof(endOfCentralDirectoryOffset)
                + sizeof(totalNumberOfDisks)
                );
    }
};

void readExtra64(std::istream &in, int extraSize
                 , std::uint64_t &uncompressedSize
                 , std::uint64_t &compressedSize
                 , std::uint64_t *fileOffset = nullptr
                 , std::uint32_t *diskNumberStart = nullptr)
{
    const auto headerSize(2 * sizeof(std::uint16_t));

    // end seek to
    auto skip(in.tellg());
    skip += extraSize;

    // read stuff from extra data
    while (extraSize > 0) {
        const auto tag(bin::read<std::uint16_t>(in));
        const auto size(bin::read<std::uint16_t>(in));
        extraSize -= (headerSize + size);

        if (tag != Tag64) {
            // skip
            in.seekg(size, std::ios_base::cur);
            continue;
        }

        // 64bit info -> read

        // read only fields that are marked invalid
        if (invalid<std::uint32_t>(uncompressedSize)) {
            bin::read(in, uncompressedSize);
        }

        if (invalid<std::uint32_t>(compressedSize)) {
            bin::read(in, compressedSize);
        }

        if (fileOffset && invalid<std::uint32_t>(*fileOffset)) {
            bin::read(in, *fileOffset);
        }

        if (diskNumberStart && invalid<std::uint16_t>(*diskNumberStart)) {
            bin::read(in, *diskNumberStart);
        }

        // no need to continue
        break;
    }

    // seek to proper location regardless where we ended while parsing data
    in.seekg(skip, std::ios_base::beg);
}

void MinimalFileHeader::read(std::istream &in)
{
    checkSignature("local file header", in, LOCAL_HEADER_SIGNATURE);
    bin::read<std::uint16_t>(in); // version needed to extract
    bin::read<std::uint16_t>(in); // general purpose bit flag
    bin::read(in, compressionMethod);
    bin::read<std::uint16_t>(in); // last modification time
    bin::read<std::uint16_t>(in); // last modification date
    bin::read<std::uint32_t>(in); // crc-32
    compressedSize = bin::read<std::uint32_t>(in);
    uncompressedSize = bin::read<std::uint32_t>(in);
    bin::read(in, filenameSize);
    bin::read(in, fileExtraSize);

    // skip filename
    in.seekg(filenameSize, std::ios_base::cur);

    // read extra info for 64 bit version (if any)
    readExtra64(in, fileExtraSize, compressedSize, uncompressedSize);
}

CentralDirectoryFileHeader CentralDirectoryFileHeader::read(std::istream &in)
{
    checkSignature("central directory file header", in
                   , CENTRAL_DIRECTORY_FILE_HEADER_SIGNATURE);

    CentralDirectoryFileHeader h;

    bin::read(in, h.versionMadeBy);
    bin::read(in, h.versionNeeded);
    bin::read(in, h.flag);
    bin::read(in, h.compressionMethod);
    bin::read(in, h.modificationTime);
    bin::read(in, h.modificationDate);
    bin::read(in, h.crc32);
    h.compressedSize = bin::read<std::uint32_t>(in);
    h.uncompressedSize = bin::read<std::uint32_t>(in);

    const auto filenameSize(bin::read<std::uint16_t>(in));
    const auto fileExtraSize(bin::read<std::uint16_t>(in));
    const auto fileCommentSize(bin::read<std::uint16_t>(in));

    h.diskNumberStart = bin::read<std::uint16_t>(in);
    bin::read(in, h.internalFileAttributes);
    bin::read(in, h.externalFileAttributes);
    h.fileOffset = bin::read<std::uint32_t>(in);

    readString(in, h.filename, filenameSize);
    readExtra64(in, fileExtraSize, h.compressedSize, h.uncompressedSize
                , &h.fileOffset, &h.diskNumberStart);
    readVector(in, h.fileComment, fileCommentSize);

    return h;
}

EndOfCentralDirectoryRecord EndOfCentralDirectoryRecord::read(std::istream &in)
{
    checkSignature("end of central directory", in
                   , END_OF_CENTRAL_DIRECTORY_SIGNATURE);

    EndOfCentralDirectoryRecord r;

    r.numberOfThisDisk = bin::read<std::uint16_t>(in);
    r.diskWhereCentralDirectoryStarts = bin::read<std::uint16_t>(in);
    r.numberOfCentralDirectoryRecordsOnThisDisk = bin::read<std::uint16_t>(in);
    r.totalNumberOfCentralDirectoryRecords = bin::read<std::uint16_t>(in);
    r.sizeOfCentralDirectory = bin::read<std::uint32_t>(in);
    r.centralDirectoryOffset = bin::read<std::uint32_t>(in);

    const auto fileCommentSize(bin::read<std::uint16_t>(in));
    readVector(in, r.comment, fileCommentSize);

    return r;
}

EndOfCentralDirectoryRecord
EndOfCentralDirectoryRecord::read64(std::istream &in)
{
    checkSignature("end of central directory (64)", in
                   , END_OF_CENTRAL_DIRECTORY64_SIGNATURE);

    // size of this record, ignore
    bin::read<std::uint64_t>(in);
    // version made by, ignroe
    bin::read<std::uint16_t>(in);
    // version needed, ignore
    bin::read<std::uint16_t>(in);

    EndOfCentralDirectoryRecord r;

    r.numberOfThisDisk = bin::read<std::uint32_t>(in);
    r.diskWhereCentralDirectoryStarts = bin::read<std::uint32_t>(in);
    r.numberOfCentralDirectoryRecordsOnThisDisk = bin::read<std::uint64_t>(in);
    r.totalNumberOfCentralDirectoryRecords = bin::read<std::uint64_t>(in);
    r.sizeOfCentralDirectory = bin::read<std::uint64_t>(in);
    r.centralDirectoryOffset = bin::read<std::uint64_t>(in);

    return r;
}

EndOfCentralDirectory64Locator
EndOfCentralDirectory64Locator::read(std::istream &in)
{
    checkSignature("end of central directory 64 locator", in
                   , END_OF_CENTRAL_DIRECTORY64_LOCATOR_SIGNATURE);

    EndOfCentralDirectory64Locator r;

    bin::read(in, r.numberOfThisDisk);
    bin::read(in, r.endOfCentralDirectoryOffset);
    bin::read(in, r.totalNumberOfDisks);

    return r;
}

Filedes openFile(const fs::path &path)
{
    Filedes fd(::open(path.string().c_str(), O_RDONLY), path);
    if (!fd) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot open zip file " << path << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }
    return fd;
}

std::size_t fileSize(const Filedes &fd)
{
    struct ::stat st;
    if (-1 == ::fstat(fd, &st)) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot stat zip file " << fd.path() << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }
    return st.st_size;
}

} // namespace

Reader::Reader(const fs::path &path)
    : path_(path), fd_(openFile(path))
    , fileLength_(fileSize(fd_))
{
    try {
        boost::iostreams::stream<utility::io::SubStreamDevice> f
            (utility::io::SubStreamDevice
             (path_, utility::io::SubStreamDevice::Filedes
              { int(fd_), std::size_t(0), fileLength_}), 512);
        f.exceptions(std::ios::badbit | std::ios::failbit);

        // read at most 1 KB block from the end of file
        auto bsize = (fileLength_ > 1024) ? 1024 : fileLength_;
        f.seekg(-bsize, std::ios_base::end);

        // read block
        std::vector<char> block(bsize, 0);
        f.read(&block[0], bsize);

        int off = -1;
        for (std::size_t i(sizeof(END_OF_CENTRAL_DIRECTORY_SIGNATURE));
             i < bsize + 1; ++i)
        {
            // real index in the file
            auto ii(bsize - i);
            if (!std::memcmp(&END_OF_CENTRAL_DIRECTORY_SIGNATURE, &block[ii]
                             , sizeof(END_OF_CENTRAL_DIRECTORY_SIGNATURE)))
            {
                off = i;
                break;
            }
        }

        if (off < 0) {
            LOGTHROW(err2, Error)
                << "Cannot find end of central directory signature "
                "in zip file " << path_ << ".";
        }

        f.seekg(-off, std::ios_base::end);
        auto eocd(EndOfCentralDirectoryRecord::read(f));

        if (eocd.has64locator()) {
            // seek to locator and read
            f.seekg(-off - EndOfCentralDirectory64Locator::size()
                    , std::ios_base::end);
            const auto locator(EndOfCentralDirectory64Locator::read(f));
            f.seekg(locator.endOfCentralDirectoryOffset, std::ios_base::beg);
            eocd = EndOfCentralDirectoryRecord::read64(f);
        }

        // seek to first central directory record
        f.seekg(eocd.centralDirectoryOffset, std::ios_base::beg);

        // read central directory
        for (std::size_t e(eocd.numberOfCentralDirectoryRecordsOnThisDisk)
                 , i(0); i != e; ++i)
        {
            auto cdfh(CentralDirectoryFileHeader::read(f));

            if (cdfh.diskNumberStart != eocd.numberOfThisDisk) {
                LOGTHROW(err2, Error)
                    << "Unsupported format in zip file " << path_
                    << ": multiple discs encountered.";
            }

            records_.emplace_back(i, cdfh.filename, cdfh.fileOffset);
        }
    } catch (const std::ios_base::failure &e) {
        LOGTHROW(err2, Error)
            << "Cannot process the zip file " << path << ": " << e.what()
            << ".";
    }
}

PluggedFile Reader::plug(std::size_t index
                         , boost::iostreams::filtering_istream &fis)
    const
{
    if (index >= records_.size())  {
        LOGTHROW(err2, Error)
            << "Invalid file index " << index << " in zip file "
            << path_ << ".";
    }

    // grab record
    const auto &record(records_[index]);

    // load minimal version of files header
    MinimalFileHeader header;
    {
        std::size_t headerStart(record.headerStart);
        std::size_t headerEnd(headerStart + localFileHeaderSize
                              + maxFilenameSize + maxExtraSize);
        if (headerEnd > fileLength_) { headerEnd = fileLength_; }

        boost::iostreams::stream<utility::io::SubStreamDevice> hf
            (utility::io::SubStreamDevice
             (path_, utility::io::SubStreamDevice::Filedes
              { int(fd_), headerStart, headerEnd }), 512);

        header.read(hf);
    }

    // add decompressor based on compression method
    switch (const auto cm
            = static_cast<CompressionMethod>(header.compressionMethod))
    {
    case CompressionMethod::store:
        // not compressed, no decompressor needed
        break;

    case CompressionMethod::bzip2:
        fis.push(bio::bzip2_decompressor());
        break;

    case CompressionMethod::deflate:
    case CompressionMethod::deflate64:
        fis.push(bio::zlib_decompressor(DeflateParams));
        break;

    default:
        LOGTHROW(err2, Error)
            << "Unsupported compression method <" << cm << "> for file "
            << record.path << " in the zip file "
            << path_ << ".";
    }

    const std::size_t fileStart(record.headerStart + header.size());
    const std::size_t fileEnd(fileStart + header.compressedSize);

    // and finally push the device for the underlying compressed file
    fis.push(utility::io::SubStreamDevice
             (record.path, utility::io::SubStreamDevice::Filedes
              { int(fd_), fileStart, fileEnd }));

    return PluggedFile(record.path, header.uncompressedSize);
}

} } // namespace utility::zip
