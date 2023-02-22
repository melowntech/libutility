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
#include <sys/types.h>
#include <fcntl.h>

#include <ctime>
#include <system_error>

#include <boost/utility/in_place_factory.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/crc.hpp>

#include "utility/unistd_compat.hpp"
#include "dbglog/dbglog.hpp"

#include "binaryio.hpp"
#include "zip.hpp"
#include "enum-io.hpp"
#include "uri.hpp"
#include "filedes.hpp"
#include "scopedguard.hpp"
#include "typeinfo.hpp"
#include "raise.hpp"

/*
Format documentation (Library of Congress preserved version):
https://www.loc.gov/preservation/digital/formats/digformatspecs/APPNOTE%2820120901%29_Version_6.3.3.txt

Some useful stuff:

version made by (2 bytes):
     0 - MS-DOS and OS/2 (FAT / VFAT / FAT32 file systems)
     1 - Amiga                     2 - OpenVMS
     3 - UNIX                      4 - VM/CMS
     5 - Atari ST                  6 - OS/2 H.P.F.S.
     7 - Macintosh                 8 - Z-System
     9 - CP/M                     10 - Windows NTFS
    11 - MVS (OS/390 - Z/OS)      12 - VSE
    13 - Acorn Risc               14 - VFAT
    15 - alternate MVS            16 - BeOS
    17 - Tandem                   18 - OS/400
    19 - OS X (Darwin)            20 thru 255 - unused

version needed to extract (2 bytes)
    1.0 - Default value
    1.1 - File is a volume label
    2.0 - File is a folder (directory)
    2.0 - File is compressed using Deflate compression
    2.0 - File is encrypted using traditional PKWARE encryption
    2.1 - File is compressed using Deflate64(tm)
    2.5 - File is compressed using PKWARE DCL Implode
    2.7 - File is a patch data set
    4.5 - File uses ZIP64 format extensions
    4.6 - File is compressed using BZIP2 compression*
    5.0 - File is encrypted using DES
    5.0 - File is encrypted using 3DES
    5.0 - File is encrypted using original RC2 encryption
    5.0 - File is encrypted using RC4 encryption
    5.1 - File is encrypted using AES encryption
    5.1 - File is encrypted using corrected RC2 encryption**
    5.2 - File is encrypted using corrected RC2-64 encryption**
    6.1 - File is encrypted using non-OAEP key wrapping***
    6.2 - Central directory encryption
    6.3 - File is compressed using LZMA
    6.3 - File is compressed using PPMd+
    6.3 - File is encrypted using Blowfish
    6.3 - File is encrypted using Twofish


MS-DOS time/date formats:
https://msdn.microsoft.com/en-us/library/windows/desktop/ms724247.aspx

MS-DOS date format:
    Bits   Description
    0-4    Day of the month (1–31)
    5-8    Month (1 = January, 2 = February, and so on)
    9-15   Year offset from 1980 (add 1980 to get actual year)

MS-DOS time format:
    0-4    Second divided by 2
    5-10   Minute (0–59)
    11-15   Hour (0–23 on a 24-hour clock)
 */


namespace bin = utility::binaryio;
namespace fs = boost::filesystem;
namespace bio = boost::iostreams;

namespace utility { namespace zip {

/** Attributes
 */
#ifndef S_IXUSR // windows
#define S_IXUSR _S_IEXEC
#define S_IXGRP _S_IEXEC
#endif
const std::uint32_t Attributes::regular =
    ((S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP) << 16);
const std::uint32_t Attributes::directory =
    ((S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP) << 16);
const std::uint32_t Attributes::executable =
    ((S_IFREG | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP) << 16);

std::uint32_t Attributes::fromFileStatus(std::uint32_t status)
{
    return (status << 16);
}

std::uint32_t Attributes::fromFile(const boost::filesystem::path &path)
{
    struct ::stat s;

    if (-1 == ::stat(path.string().c_str(), &s)) {
        std::system_error e
            (errno, std::system_category()
             , formatError("Cannot stat file %s.", path));
        LOG(err1) << e.what();
        throw e;
    }

    return fromFileStatus(s.st_mode);
}

EmbedFlag Embed;

namespace detail {

constexpr std::uint32_t LOCAL_HEADER_SIGNATURE = 0x04034b50;
constexpr std::uint32_t CENTRAL_DIRECTORY_FILE_HEADER_SIGNATURE = 0x02014b50;
constexpr std::uint32_t END_OF_CENTRAL_DIRECTORY_SIGNATURE = 0x06054b50;
constexpr std::uint32_t END_OF_CENTRAL_DIRECTORY64_SIGNATURE = 0x06064b50;
constexpr std::uint32_t END_OF_CENTRAL_DIRECTORY64_LOCATOR_SIGNATURE = 0x07064b50;


/** 64-bit support + bzip2
 */
constexpr std::uint16_t VERSION_NEEDED = 46;

/** UNIX (3) + version 6.3 (3f = 63)
 */
constexpr std::uint16_t VERSION_MADE_BY = 0x33f;

constexpr std::uint16_t Tag64 = 0x0001u;

constexpr std::uint16_t FlagDataDescriptor = 1u << 3;

/** Should be used only for uint16 and uint32
 */
template <typename T> bool invalid(T value)
{
    return value == std::numeric_limits<T>::max();
}

template <typename T> constexpr T invalid()
{
    return std::numeric_limits<T>::max();
}

std::pair<std::uint16_t, std::uint16_t>
msDateTime(std::time_t timestamp = std::time(nullptr))
{
    std::tm lt;
    if (!::localtime_r(&timestamp, &lt)) {
        LOGTHROW(err1, Error)
            << "Cannot convert timestamp <" << timestamp << "> to local time.";
    }

    std::pair<std::uint16_t, std::uint16_t> res;

    // date
    res.first
        = ((lt.tm_mday & 0x1f) // 0-4: Day of the month (1–31)
           | ((lt.tm_mon + 1) & 0xf) << 5 // 5-8: Month (1 = January, ...)
           | ((lt.tm_year - 80) & 0x7f) << 9 // 9-15: Year offset from 1980
           );

    res.second
        = (((lt.tm_sec >> 1) & 0x1f) // 0-4: Second divided by 2
           | ((lt.tm_min) & 0x3f) << 5 // 5-10   Minute (0–59)
           | ((lt.tm_hour) & 0x1f) << 11 // 11-15: Hour (0–23)
           );

    return res;
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

const auto InflateParams([]() -> bio::zlib_params {
        bio::zlib_params params;
        params.method = bio::zlib::deflated;
        params.level = 9;
        params.window_bits = 15;
        params.mem_level = 8;
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
        : versionNeeded(), flag(), compressionMethod()
        , modificationTime(), modificationDate()
        , crc32(), compressedSize(), uncompressedSize()
        , versionMadeBy(), diskNumberStart()
        , internalFileAttributes()
        , externalFileAttributes()
        , fileOffset()
    {}

    MinimalFileHeader minimal() const {
        MinimalFileHeader mh;
        mh.flag = flag;
        mh.compressionMethod = compressionMethod;
        mh.compressedSize = compressedSize;
        mh.uncompressedSize = uncompressedSize;
        mh.filenameSize = filename.size();
        mh.fileExtraSize = fileExtra.size();
        return mh;
    }

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

    void write64(std::ostream &os) const;
    void write64Marker(std::ostream &os);
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

    static void writeSimple(std::ostream &os, std::uint64_t eocd);

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

inline std::size_t MinimalFileHeader::size() const
{
    return localFileHeaderSize + filenameSize + fileExtraSize;
}

void MinimalFileHeader::read(std::istream &in)
{
    checkSignature("local file header", in, LOCAL_HEADER_SIGNATURE);
    bin::read<std::uint16_t>(in); // version needed to extract
    flag = bin::read<std::uint16_t>(in); // general purpose bit flag
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

inline void updateHeader(MinimalFileHeader &header
                         , const MinimalFileHeader &other)
{
    if (!(header.flag & FlagDataDescriptor)) { return; }

    header.compressedSize = other.compressedSize;
    header.uncompressedSize = other.uncompressedSize;
    // CRC not used
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

    LOG(debug) << "Reading end of central directory record.";
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

void EndOfCentralDirectoryRecord::write64(std::ostream &os) const
{
    const auto size(sizeof(std::uint16_t) // version made by
                    + sizeof(std::uint16_t) // version needed to extract
                    + sizeof(std::uint32_t) // number of this disk

                    /* number of the disk with the start of the central
                       directory */
                    + sizeof(std::uint32_t)

                    /* total number of entries in the central directory
                       on this disk */
                    + sizeof(std::uint64_t)

                    /* total number of entries in the central directory */
                    + sizeof(std::uint64_t)

                    + sizeof(std::uint64_t) // size of the central directory

                    /* offset of start of central directory with respect to
                       the starting disk number */
                    + sizeof(std::uint64_t));

    // signature
    bin::write(os, END_OF_CENTRAL_DIRECTORY64_SIGNATURE);

    // size of this record
    bin::write(os, std::uint64_t(size));

    // versions
    bin::write(os, std::uint16_t(VERSION_MADE_BY));
    bin::write(os, std::uint16_t(VERSION_NEEDED));

    // disk info
    bin::write(os, std::uint32_t(numberOfThisDisk));
    bin::write(os, std::uint32_t(diskWhereCentralDirectoryStarts));

    // record info
    bin::write(os, std::uint64_t(numberOfCentralDirectoryRecordsOnThisDisk));
    bin::write(os, std::uint64_t(totalNumberOfCentralDirectoryRecords));

    // central directory
    bin::write(os, std::uint64_t(sizeOfCentralDirectory));
    bin::write(os, std::uint64_t(centralDirectoryOffset));
}

void EndOfCentralDirectoryRecord::write64Marker(std::ostream &os)
{
    bin::write(os, END_OF_CENTRAL_DIRECTORY_SIGNATURE);
    bin::write(os, std::uint16_t(0));
    bin::write(os, std::uint16_t(0));
    bin::write(os, invalid<std::uint16_t>());
    bin::write(os, invalid<std::uint16_t>());
    bin::write(os, invalid<std::uint32_t>());
    bin::write(os, invalid<std::uint32_t>());
    bin::write(os, std::uint16_t(0));
}

EndOfCentralDirectory64Locator
EndOfCentralDirectory64Locator::read(std::istream &in)
{
    checkSignature("end of central directory 64 locator", in
                   , END_OF_CENTRAL_DIRECTORY64_LOCATOR_SIGNATURE);

    LOG(debug) << "Reading end of central directory locator.";
    EndOfCentralDirectory64Locator r;

    bin::read(in, r.numberOfThisDisk);
    bin::read(in, r.endOfCentralDirectoryOffset);
    bin::read(in, r.totalNumberOfDisks);

    return r;
}

void EndOfCentralDirectory64Locator::writeSimple(std::ostream &os
                                                 , std::uint64_t eocd)
{
    bin::write(os, END_OF_CENTRAL_DIRECTORY64_LOCATOR_SIGNATURE);
    bin::write(os, std::uint32_t(0)); // numberOfThisDisk
    bin::write(os, std::uint64_t(eocd)); // endOfCentralDirectoryOffset
    bin::write(os, std::uint32_t(1)); // totalNumberOfDisks
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

fs::path sanitize(std::string original, bool enabled)
{
    if (!enabled) { return original; }

    // \ -> /
    for (auto &c : original) { if (c == '\\') { c= '/'; } }

    // remove any (double-)dot occurrence and force start from root
    return Uri::joinAndRemoveDotSegments("/", original);
}

template <typename Stream>
int findCentralDirectory(Stream &f, std::size_t fileLength)
{
    // read at most 1 KB block from the end of file
    auto bsize = (fileLength > 1024) ? 1024 : fileLength;
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

    return off;
}

class CentralDirectoryReader {
public:
    CentralDirectoryReader(const Filedes &fd)
        : path_(fd.path()), fileLength_(fileSize(fd))
        , f_(utility::io::SubStreamDevice
             (path_, utility::io::SubStreamDevice::Filedes
              { int(fd), std::size_t(0), fileLength_}), 512)
    {}

    CentralDirectoryReader(const Filedes &fd, std::size_t fileLength)
        : path_(fd.path()), fileLength_(fileLength)
        , f_(utility::io::SubStreamDevice
             (path_, utility::io::SubStreamDevice::Filedes
              { int(fd), std::size_t(0), fileLength_}), 512)
    {}

    bool open(bool nothrow = false) {
        f_.exceptions(std::ios::badbit | std::ios::failbit);

        auto off(findCentralDirectory(f_, fileLength_));

        if (off < 0) {
            if (nothrow) { return false; }
            LOGTHROW(err2, Error)
                << "Cannot find end of central directory signature "
                "in zip file " << path_ << ".";
        }

        f_.seekg(-off, std::ios_base::end);
        eocd_ = EndOfCentralDirectoryRecord::read(f_);

        if (eocd_.has64locator()) {
            // seek to locator and read
            f_.seekg(-off - EndOfCentralDirectory64Locator::size()
                    , std::ios_base::end);
            const auto locator(EndOfCentralDirectory64Locator::read(f_));
            f_.seekg(locator.endOfCentralDirectoryOffset, std::ios_base::beg);
            eocd_ = EndOfCentralDirectoryRecord::read64(f_);
        }

        return true;
    }

    template <typename Callback>
    void read(const Callback &callback
              , std::size_t limit = std::numeric_limits<std::size_t>::max())
    {
        const auto recordCount
            ((eocd_.numberOfCentralDirectoryRecordsOnThisDisk > limit)
             ? limit
             : eocd_.numberOfCentralDirectoryRecordsOnThisDisk);

        LOG(debug) << "Reading " << recordCount << " records out of "
                   << eocd_.numberOfCentralDirectoryRecordsOnThisDisk
                   << " from ZIP archive " << path_ << ".";

        // seek to first central directory record
        f_.seekg(eocd_.centralDirectoryOffset, std::ios_base::beg);

        // read central directory
        for (std::size_t i(0); i != recordCount; ++i) {
            auto cdfh(CentralDirectoryFileHeader::read(f_));

            if (cdfh.diskNumberStart != eocd_.numberOfThisDisk) {
                LOGTHROW(err2, Error)
                    << "Unsupported format in zip file " << path_
                    << ": multiple discs encountered.";
            }

            callback(i, cdfh);
        }
    }

private:
    const fs::path path_;
    const std::size_t fileLength_;
    boost::iostreams::stream<utility::io::SubStreamDevice> f_;
    EndOfCentralDirectoryRecord eocd_;
};

} // namespace detail

// pull in everything from detail namespace above
using namespace detail;

bool Reader::check(const boost::filesystem::path &path)
{
    try {
        Filedes fd(openFile(path));
        const auto length(fileSize(fd));

        boost::iostreams::stream<utility::io::SubStreamDevice> f
            (utility::io::SubStreamDevice
             (path, utility::io::SubStreamDevice::Filedes
              { int(fd), std::size_t(0), length}), 512);
        f.exceptions(std::ios::badbit | std::ios::failbit);

        return (findCentralDirectory(f, length) >= 0);
    } catch (...) {}

    return false;
}

Reader::Reader(const fs::path &path, std::size_t limit, bool sanitizePaths)
    : path_(path), fd_(openFile(path))
    , fileLength_(fileSize(fd_))
{
    try {
        // open and read central directory
        CentralDirectoryReader cdr(fd_, fileLength_);
        cdr.open();

        cdr.read([&](int i, const CentralDirectoryFileHeader &cdfh) -> void
        {
            records_.emplace_back(i, sanitize(cdfh.filename, sanitizePaths)
                                  , cdfh.fileOffset, cdfh.minimal());
        }, limit);
    } catch (const std::ios_base::failure &e) {
        LOGTHROW(err2, Error)
            << "Cannot process the zip file " << path << ": " << e.what()
            << ".";
    }
}

std::size_t Reader::find(const boost::filesystem::path &path) const
{
    auto frecords(std::find_if(records_.begin(), records_.end()
                               , [&](const Record &r) {
                                   return r.path == path;
                               }));
    if (frecords == records_.end()) {
        LOGTHROW(err2, Error)
            << "File " << path << "not found in zip file "
            << path_ << ".";
    }

    return (&*frecords - records_.data());
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
        updateHeader(header, record.header);
    }

    bool seekable(false);
    std::size_t safetyPadding(0);

    // add decompressor based on compression method
    switch (const auto cm
            = static_cast<CompressionMethod>(header.compressionMethod))
    {
    case CompressionMethod::store:
        // not compressed, no decompressor needed
        seekable = true;
        break;

    case CompressionMethod::bzip2:
        fis.push(bio::bzip2_decompressor());
        break;

    case CompressionMethod::deflate:
    case CompressionMethod::deflate64:
        fis.push(bio::zlib_decompressor(DeflateParams));
        safetyPadding = 16;
        break;

    default:
        LOGTHROW(err2, Error)
            << "Unsupported compression method <" << cm << "> for file "
            << record.path << " in the zip file "
            << path_ << ".";
    }

    const std::size_t fileStart(record.headerStart + header.size());
    const std::size_t fileEnd(fileStart + header.compressedSize
                              + safetyPadding);

    // and finally push the device for the underlying compressed file
    fis.push(utility::io::SubStreamDevice
             (record.path, utility::io::SubStreamDevice::Filedes
              { int(fd_), fileStart, fileEnd}));

    return PluggedFile(record.path, header.uncompressedSize, seekable);
}

struct Writer::Detail : public std::enable_shared_from_this<Detail>
{
    typedef std::shared_ptr<Detail> pointer;

    Detail(const boost::filesystem::path &path, bool overwrite);
    Detail(const boost::filesystem::path &path, const EmbedFlag&);

    ~Detail() {
        if (fd) {
            LOG(warn2) << "ZIP file " << fd.path() << " was not flushed. "
                       << "Use close() member function.";
        }
    }

    OStream::pointer ostream(const boost::filesystem::path &path
                             , Compression compression
                             , const FilterInit &filterInit);

    void close();

    void begin(const std::string &name);

    struct FileEntry {
        fs::path name;
        CompressionMethod compressionMethod;
        std::size_t compressedSize;
        std::size_t uncompressedSize;
        std::uint32_t crc32;
        std::uint32_t attributes = Attributes::regular;

        FileEntry(const fs::path &name, CompressionMethod compressionMethod)
            : name(name), compressionMethod(compressionMethod)
            , compressedSize(), uncompressedSize(), crc32()
        {}
    };

    // TODO: pass record info
    void commit(const FileEntry &fileEntry);

    void rollback();

    void seekTo(std::size_t off) {
        auto res(::lseek(fd, off, SEEK_SET));
        if (res == -1) {
            std::system_error e(errno, std::system_category());
            LOG(err2) << "Cannot seek in a ZIP file " << fd.path() << ": <"
                      << e.code() << ", " << e.what() << ">.";
            throw e;
        }
    }

    ::off_t seekEnd() {
        auto res(::lseek(fd, 0, SEEK_END));
        if (res == -1) {
            std::system_error e(errno, std::system_category());
            LOG(err2) << "Cannot seek in a ZIP file " << fd.path() << ": <"
                      << e.code() << ", " << e.what() << ">.";
            throw e;
        }
        return res;
    }

    ::off_t whereAmI() {
        auto res(::lseek(fd, 0, SEEK_CUR));
        if (res == -1) {
            std::system_error e(errno, std::system_category());
            LOG(err2) << "Cannot seek in a ZIP file " << fd.path() << ": <"
                      << e.code() << ", " << e.what() << ">.";
            throw e;
        }
        return res;
    }

    void advance(std::size_t bytes) {
        // ensure we are at current end
        auto end(seekEnd());

        // truncated to inflated size
        auto res(::ftruncate(fd, end + bytes));
        if (res == -1) {
            std::system_error e(errno, std::system_category());
            LOG(err2) << "Cannot truncate a ZIP file " << fd.path() << ": <"
                      << e.code() << ", " << e.what() << ">.";
            throw e;
        }

        // and seek at the new end
        seekEnd();
    }

    Filedes fd;

    ::off_t tx;

    CentralDirectoryFileHeader::list directory;
};

namespace {

enum class OpenMode {
    failIfExists, overwrite, append
};

int openFlags(OpenMode mode)
{
    switch (mode) {
    case OpenMode::overwrite:
        return (O_WRONLY | O_CREAT | O_TRUNC);

    case OpenMode::failIfExists:
        return (O_WRONLY | O_CREAT | O_EXCL);

    case OpenMode::append:
        return (O_RDWR | O_CREAT);
    }

    throw; // never reached
}

OpenMode openMode(bool overwrite)
{
    return overwrite ? OpenMode::overwrite : OpenMode::failIfExists;
}

Filedes openFile(const boost::filesystem::path &path, OpenMode openMode)
{
    Filedes fd(::open(path.string().c_str(), openFlags(openMode)
                      , (S_IRUSR | S_IWUSR | S_IRGRP)), path);

    if (!fd) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot create a ZIP file " << path << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    return fd;
}

} // namespace

Writer::Detail::Detail(const boost::filesystem::path &path, bool overwrite)
    : fd(openFile(path, openMode(overwrite))), tx(-1)
{
}

Writer::Detail::Detail(const boost::filesystem::path &path, const EmbedFlag&)
    : fd(openFile(path, OpenMode::append)), tx(-1)
{
    CentralDirectoryReader cdr(fd);
    if (!cdr.open(true)) {
        // not a ZIP, just append
        seekEnd();
        return;
    }

    try {
        cdr.read([&](int, const CentralDirectoryFileHeader &cdfh) -> void
        {
            directory.push_back(cdfh);
        });
    } catch (const std::ios_base::failure &e) {
        LOGTHROW(err2, Error)
            << "Cannot process the zip file " << path << ": " << e.what()
            << ".";
    }

    seekEnd();
}

CompressionMethod compressionMethod(Compression compression)
{
    switch (compression) {
    case Compression::store: return CompressionMethod::store;
    case Compression::deflate: return CompressionMethod::deflate;
    case Compression::bzip2: return CompressionMethod::bzip2;
    }

    LOGTHROW(err2, Error)
        << "Unsupported compression value (" << static_cast<int>(compression)
        << ").";
    throw;
}

class CounterFilter : public bio::multichar_output_filter
{
public:
    explicit CounterFilter() : count_() {}

    template<typename Sink>
    std::streamsize write(Sink &sink, const char_type *s, std::streamsize n) {
        auto result(bio::write(sink, s, n));
        count_ += result;
        return result;
    }

    std::size_t count() const { return count_; }

private:
    std::size_t count_;
};

class Crc32Filter : public bio::multichar_output_filter
{
public:
    explicit Crc32Filter() : crc32_() {}

    template<typename Sink>
    std::streamsize write(Sink &sink, const char_type *s, std::streamsize n) {
        auto result(bio::write(sink, s, n));
        crc32_.process_bytes(s, result);
        return result;
    }

    std::uint32_t checksum() const { return crc32_.checksum(); }

private:
    boost::crc_32_type crc32_;
};

class ZipStream : public Writer::OStream
{
public:
    ZipStream(Writer::Detail::pointer detail, const fs::path &name
              , Compression compression, const Writer::FilterInit &filterInit)
        : detail_(std::move(detail))
        , fileEntry_(name, compressionMethod(compression))
    {
        detail_->begin(fileEntry_.name.generic_string());
        open_ = true;

        if (filterInit) { filterInit(fos_); }

        // compute crc32 for uncompressed file
        fos_.push(boost::ref(crc32_));

        switch (compression) {
        case Compression::store:
            // do not touch
            break;

        case Compression::deflate:
            // measure uncompressed size
            uncompressedSize_ = boost::in_place();
            fos_.push(boost::ref(*uncompressedSize_));
            // compress
            fos_.push(bio::zlib_compressor(InflateParams));
            break;

        case Compression::bzip2:
            // measure uncompressed size
            uncompressedSize_ = boost::in_place();
            fos_.push(boost::ref(*uncompressedSize_));
            // compress
            fos_.push(bio::bzip2_compressor());
            break;
        }

        // measure compressed size
        fos_.push(boost::ref(compressedSize_));
        // sink to file
        fos_.push(bio::file_descriptor_sink
                   (detail_->fd.get()
                    , bio::file_descriptor_flags::never_close_handle));
    }

    virtual ~ZipStream() {
        try {
            close();
        } catch (const std::exception &e) {
            LOG(warn2) << "Uncaught exception in ZipStream flush: <"
                       << e.what() << ">.";
        } catch (...) {
            LOG(warn2) << "Unknowmn uncaught exception in ZipStream flush.";
        }
    }

    virtual std::ostream& get() { return fos_; }

    virtual Statistics close() {
        if (!open_) { return {}; }
        open_ = false;

        // close output file
        bio::close(fos_);

        fileEntry_.compressedSize = compressedSize_.count();
        fileEntry_.uncompressedSize
            = (uncompressedSize_
               ? uncompressedSize_->count() : fileEntry_.compressedSize);
        fileEntry_.crc32 = crc32_.checksum();

        detail_->commit(fileEntry_);

        return Statistics(fileEntry_.compressedSize
                          , fileEntry_.uncompressedSize);
    }

    virtual void setFileAttributes(std::uint32_t attributes) {
        if (!open_) {
            LOGTHROW(err2, Error)
                << "Zip stream for file " << fileEntry_.name
                << " is already closed. Cannot udpate.";
        }

        fileEntry_.attributes = attributes;
    }

private:
    Writer::Detail::pointer detail_;
    Writer::Detail::FileEntry fileEntry_;

    bool open_;
    bio::filtering_ostream fos_;

    CounterFilter compressedSize_;
    boost::optional<CounterFilter> uncompressedSize_;
    Crc32Filter crc32_;
};

const auto extra64SizeLocalHeader
(2 * sizeof(std::uint16_t) // tag + size
 + 2 * sizeof(std::uint64_t) // (un)compressedSize
 );

void writeLocalHeader(std::ostream &os, const CentralDirectoryFileHeader &fh)
{
    // signature
    bin::write(os, LOCAL_HEADER_SIGNATURE);

    // fixed fields
    bin::write(os, std::uint16_t(fh.versionNeeded));
    bin::write(os, std::uint16_t(fh.flag));
    bin::write(os, std::uint16_t(fh.compressionMethod));
    bin::write(os, std::uint16_t(fh.modificationTime));
    bin::write(os, std::uint16_t(fh.modificationDate));

    bin::write(os, std::uint32_t(fh.crc32));

    // lengths, mark them as invalid -> redirect to extra 64 field
    bin::write(os, invalid<std::uint32_t>()); // cs
    bin::write(os, invalid<std::uint32_t>()); // uncs

    bin::write(os, std::uint16_t(fh.filename.size()));
    bin::write(os, std::uint16_t(fh.fileExtra.size()
                                 + extra64SizeLocalHeader));
    bin::write(os, fh.filename.data(), fh.filename.size());

    // write extra 64
    // header
    bin::write(os, std::uint16_t(Tag64));
    bin::write(os, std::uint16_t(2 * sizeof(std::uint64_t)));
    // body
    bin::write(os, std::uint64_t(fh.compressedSize));
    bin::write(os, std::uint64_t(fh.uncompressedSize));

    // other extra
    bin::write(os, fh.fileExtra.data(), fh.fileExtra.size());
}

/** Helper type for handling short/long versions of some ZIP-related size
 */
template <typename Short, typename Long>
struct ShortValue {
    typedef Short short_type;
    typedef Long long_type;

    static constexpr short_type limit = invalid<Short>();

    /** Is value inside short range?
     */
    bool inLimit;

    /** Short version of value or invalid marker.
     */
    short_type shortValue;

    /** Original (long version) value.
     */
    long_type value;

    ShortValue(long_type v)
        : inLimit(v < limit)
        , shortValue(inLimit ? short_type(v) : limit)
        , value(v)
    {}

    /** How many bytes do this element occupy in the extra field?
     */
    std::size_t extraSize() const { return inLimit ? 0 : sizeof(long_type); }

    /** Write short version (always).
     */
    void writeShort(std::ostream &os) const {
        bin::write(os, shortValue);
    }

    /** Write long version (only when too large to fit short version)
     */
    void writeLong(std::ostream &os) const {
        if (!inLimit) {
            bin::write(os, value);
        }
    }
};

typedef ShortValue<std::uint32_t, std::uint64_t> ShortValue64;

void writeCentralHeader(std::ostream &os, const CentralDirectoryFileHeader &fh)
{
    const ShortValue64 compressedSize(fh.compressedSize);
    const ShortValue64 uncompressedSize(fh.uncompressedSize);
    const ShortValue64 fileOffset(fh.fileOffset);
    const auto tag64Size(compressedSize.extraSize()
                         + uncompressedSize.extraSize()
                         + fileOffset.extraSize());
    const auto extraSize(tag64Size
                         ? tag64Size + 2 * sizeof(std::uint16_t)
                         : 0);

    bin::write(os, CENTRAL_DIRECTORY_FILE_HEADER_SIGNATURE);

    // fixed fields
    bin::write(os, std::uint16_t(fh.versionMadeBy));
    bin::write(os, std::uint16_t(fh.versionNeeded));
    bin::write(os, std::uint16_t(fh.flag));

    bin::write(os, std::uint16_t(fh.compressionMethod));
    bin::write(os, std::uint16_t(fh.modificationTime));
    bin::write(os, std::uint16_t(fh.modificationDate));

    bin::write(os, std::uint32_t(fh.crc32));

    compressedSize.writeShort(os);
    uncompressedSize.writeShort(os);

    // variable sizes
    bin::write(os, std::uint16_t(fh.filename.size()));
    bin::write(os, std::uint16_t(fh.fileExtra.size() + extraSize));
    bin::write(os, std::uint16_t(fh.fileComment.size()));

    bin::write(os, std::uint16_t(fh.diskNumberStart));
    bin::write(os, std::uint16_t(fh.internalFileAttributes));

    bin::write(os, std::uint32_t(fh.externalFileAttributes));
    fileOffset.writeShort(os);

    // variable sized fields
    bin::write(os, fh.filename.data(), fh.filename.size());

    if (extraSize) {
        // write extra 64
        // header
        bin::write(os, std::uint16_t(Tag64));
        bin::write(os, std::uint16_t(tag64Size));

        // body
        compressedSize.writeLong(os);
        uncompressedSize.writeLong(os);
        fileOffset.writeLong(os);
    }

    // extra + comment
    bin::write(os, fh.fileExtra.data(), fh.fileExtra.size());
    bin::write(os, fh.fileComment.data(), fh.fileComment.size());
}

Writer::OStream::pointer
Writer::Detail::ostream(const boost::filesystem::path &path
                        , Compression compression
                        , const FilterInit &filterInit)
{
    if (!fd) {
        LOGTHROW(err2, Error)
            << "ZIP archive " << fd.path() << " is closed.";
    }

    auto os(std::make_shared<ZipStream>
            (shared_from_this(), path, compression, filterInit));
    os->get().exceptions(std::ios::badbit | std::ios::failbit);
    return os;
}

void Writer::Detail::begin(const std::string &name)
{
    if (tx >= 0) {
        LOGTHROW(err2, Error)
            << "Cannot write more than one file to a ZIP archive at once.";
    }
    tx = whereAmI();

    // make room for header
    advance(localFileHeaderSize + extra64SizeLocalHeader + name.size());
}

void Writer::Detail::commit(const FileEntry &fe)
{
    if (tx < 0) {
        LOGTHROW(err2, Error)
            << "Not inside a transaction.";
    }

    CentralDirectoryFileHeader fh;

    fh.versionNeeded = VERSION_NEEDED;
    fh.versionMadeBy = VERSION_MADE_BY;
    fh.compressionMethod = static_cast<decltype(fh.compressionMethod)>
        (fe.compressionMethod);
    std::tie(fh.modificationDate, fh.modificationTime) = msDateTime();
    fh.crc32 = fe.crc32;
    fh.compressedSize = fe.compressedSize;
    fh.uncompressedSize = fe.uncompressedSize;
    fh.externalFileAttributes = fe.attributes;
    fh.filename = fe.name.generic_string();
    fh.fileOffset = tx;

    try {
        // write a file header
        seekTo(tx);
        {
            bio::stream<bio::file_descriptor_sink> os
                (fd.get(), bio::file_descriptor_flags::never_close_handle);
            os.exceptions(std::ios::badbit | std::ios::failbit);
            writeLocalHeader(os, fh);
            bio::close(os);
        }
        seekEnd();
    } catch (const std::exception &e) {
        LOG(err2) << "Cannot commit a ZIP file " << fd.path()
                  << "; rolling back.";
        rollback();
        throw;
    }

    // try to find a record with the same path
    auto fdirectory(std::find_if(directory.begin(), directory.end()
                                 , [&](const CentralDirectoryFileHeader &h)
                                 {
                                     return (h.filename == fh.filename);
                                 }));

    if (fdirectory == directory.end()) {
        // not found, append
        directory.push_back(fh);
    } else {
        // found, replace
        *fdirectory = fh;
    }

    tx = -1;
}

void Writer::Detail::rollback()
{
    if (tx < 0) {
        LOGTHROW(err2, Error)
            << "Not inside a transaction.";
    }

    auto res(::ftruncate(fd, tx));
    if (res == -1) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot truncate a ZIP file " << fd.path() << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }
    seekEnd();

    tx = -1;
}

void Writer::Detail::close()
{
    if (!fd) { return; }

    // this ensures fd is closed even if tail write fails
    ScopedGuard guard([&]() -> void
    {
        try { fd.close(); } catch (...) {}
    });


    // write file tail
    {
        // central directory
        const auto cdOff(seekEnd());

        {
            bio::stream<bio::file_descriptor_sink> os
                (fd.get(), bio::file_descriptor_flags::never_close_handle);
            os.exceptions(std::ios::badbit | std::ios::failbit);

            // write central directory headers
            for (const auto &fh : directory) {
                writeCentralHeader(os, fh);
            }
        }

        // end of central directory
        const auto eocdOff(seekEnd());

        bio::stream<bio::file_descriptor_sink> os
            (fd.get(), bio::file_descriptor_flags::never_close_handle);
        os.exceptions(std::ios::badbit | std::ios::failbit);

        // write 64bit end of central directory record
        EndOfCentralDirectoryRecord eocd;
        eocd.numberOfThisDisk = 0;
        eocd.diskWhereCentralDirectoryStarts = 0;
        eocd.numberOfCentralDirectoryRecordsOnThisDisk = directory.size();
        eocd.totalNumberOfCentralDirectoryRecords = directory.size();
        eocd.sizeOfCentralDirectory = (eocdOff - cdOff);
        eocd.centralDirectoryOffset = cdOff;
        eocd.write64(os);

        // write 64bit end of central directory locator
        EndOfCentralDirectory64Locator::writeSimple(os, eocdOff);

        // write end of central directory record
        eocd.write64Marker(os);

        bio::close(os);
    }
}

Writer::Writer(const boost::filesystem::path &path, bool overwrite)
    : detail_(std::make_shared<Detail>(path, overwrite))
{}

Writer::Writer(const boost::filesystem::path &path, const EmbedFlag &embed)
    : detail_(std::make_shared<Detail>(path, embed))
{
}

Writer::~Writer() {}

void Writer::close()
{
    return detail_->close();
}

Writer::OStream::pointer Writer::ostream(const boost::filesystem::path &path
                                         , Compression compression
                                         , const FilterInit &filterInit)
{
    return detail_->ostream(path, compression, filterInit);
}

} } // namespace utility::zip
