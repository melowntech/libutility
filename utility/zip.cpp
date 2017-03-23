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

namespace bin = utility::binaryio;
namespace fs = boost::filesystem;
namespace bio = boost::iostreams;

namespace utility { namespace zip {

namespace {

constexpr std::uint32_t LOCAL_HEADER_SIGNATURE = 0x04034b50;
constexpr std::uint32_t CENTRAL_DIRECTORY_FILE_HEADER_SIGNATURE = 0x02014b50;
constexpr std::uint32_t END_OF_CENTRAL_DIRECTORY_SIGNATURE = 0x06054b50;

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


struct LocalFileHeader {
    std::uint16_t versionNeeded;
    std::uint16_t flag;
    std::uint16_t compressionMethod;
    std::uint16_t modificationTime;
    std::uint16_t modificationDate;
    std::uint32_t crc32;
    std::uint32_t compressedSize;
    std::uint32_t uncompressedSize;

    std::string filename;
    std::vector<char> fileExtra; // unparsed!

    typedef std::vector<LocalFileHeader> list;

    static LocalFileHeader read(std::istream &in);

    std::size_t size() const;

    std::size_t totalSize() const;

    static std::size_t baseSize();
};

struct MinimalFileHeader {
    std::uint16_t compressionMethod;
    std::uint32_t compressedSize;
    std::uint32_t uncompressedSize;
    std::uint16_t filenameSize;
    std::uint16_t fileExtraSize;

    MinimalFileHeader()
        : compressionMethod(), compressedSize(), uncompressedSize()
        , filenameSize(), fileExtraSize()
    {}

    void read(std::istream &in);

    std::size_t size() const {
        return LocalFileHeader::baseSize() + filenameSize + fileExtraSize;
    }
};

struct CentralDirectoryFileHeader : LocalFileHeader {
    std::uint16_t versionMadeBy;
    std::uint16_t diskNumberStart;
    std::uint16_t internalFileAttributes;
    std::uint32_t externalFileAttributes;
    std::uint32_t fileOffset;

    std::vector<char> fileComment;

    typedef std::vector<CentralDirectoryFileHeader> list;

    CentralDirectoryFileHeader(const LocalFileHeader &lh)
        : LocalFileHeader(lh)
        , versionMadeBy(lh.versionNeeded)
        , diskNumberStart(0)
        , internalFileAttributes(0)
        , externalFileAttributes(0)
        , fileOffset()
    {}

    CentralDirectoryFileHeader()
        : LocalFileHeader()
        , versionMadeBy(0)
        , diskNumberStart(0)
        , internalFileAttributes(0)
        , externalFileAttributes(0)
        , fileOffset()
    {}

    static CentralDirectoryFileHeader read(std::istream &in);
};

struct EndOfCentralDirectoryRecord {
    std::uint16_t numberOfThisDisk;
    std::uint16_t diskWhereCentralDirectoryStarts;
    std::uint16_t numberOfCentralDirectoryRecordsOnThisDisk;
    std::uint16_t totalNumberOfCentralDirectoryRecords;
    std::uint32_t sizeOfCentralDirectory;
    std::uint32_t centralDirectoryOffset;
    std::vector<char> comment;

    EndOfCentralDirectoryRecord()
        : numberOfThisDisk(0)
        , diskWhereCentralDirectoryStarts(0)
        , numberOfCentralDirectoryRecordsOnThisDisk(0)
        , totalNumberOfCentralDirectoryRecords(0)
        , sizeOfCentralDirectory(0)
        , centralDirectoryOffset(0)
    {}

    static EndOfCentralDirectoryRecord read(std::istream &in);
};

void MinimalFileHeader::read(std::istream &in)
{
    checkSignature("local file header", in, LOCAL_HEADER_SIGNATURE);
    bin::read<std::uint16_t>(in);
    bin::read<std::uint16_t>(in);
    bin::read(in, compressionMethod);
    bin::read<std::uint16_t>(in);
    bin::read<std::uint16_t>(in);
    bin::read<std::uint32_t>(in);
    bin::read(in, compressedSize);
    bin::read(in, uncompressedSize);
    bin::read(in, filenameSize);
    bin::read(in, fileExtraSize);
}

LocalFileHeader LocalFileHeader::read(std::istream &in)
{
    checkSignature("local file header", in, LOCAL_HEADER_SIGNATURE);

    LocalFileHeader h;

    bin::read(in, h.versionNeeded);
    bin::read(in, h.flag);
    bin::read(in, h.compressionMethod);
    bin::read(in, h.modificationTime);
    bin::read(in, h.modificationDate);
    bin::read(in, h.crc32);
    bin::read(in, h.compressedSize);
    bin::read(in, h.uncompressedSize);

    const auto sizeFilename(bin::read<std::uint16_t>(in));
    const auto sizeFileExtra(bin::read<std::uint16_t>(in));

    // read filename
    readString(in, h.filename, sizeFilename);
    readVector(in, h.fileExtra, sizeFileExtra);

    return h;
}

std::size_t LocalFileHeader::baseSize()
{
    // construct base size of header
    return (sizeof(std::uint32_t)
            + sizeof(LocalFileHeader::versionNeeded)
            + sizeof(LocalFileHeader::flag)
            + sizeof(LocalFileHeader::compressionMethod)
            + sizeof(LocalFileHeader::modificationTime)
            + sizeof(LocalFileHeader::modificationDate)
            + sizeof(LocalFileHeader::crc32)
            + sizeof(LocalFileHeader::compressedSize)
            + sizeof(LocalFileHeader::uncompressedSize)
            + sizeof(std::uint16_t)
            + sizeof(std::uint16_t));
}

std::size_t LocalFileHeader::size() const
{
    // construct size of header
    return (baseSize()
            + filename.size()
            + fileExtra.size());
}

std::size_t LocalFileHeader::totalSize() const
{
    return size() + compressedSize;
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
    bin::read(in, h.compressedSize);
    bin::read(in, h.uncompressedSize);

    const auto filenameSize(bin::read<std::uint16_t>(in));
    const auto fileExtraSize(bin::read<std::uint16_t>(in));
    const auto fileCommentSize(bin::read<std::uint16_t>(in));

    bin::read(in, h.diskNumberStart);
    bin::read(in, h.internalFileAttributes);
    bin::read(in, h.externalFileAttributes);
    bin::read(in, h.fileOffset);

    readString(in, h.filename, filenameSize);
    readVector(in, h.fileExtra, fileExtraSize);
    readVector(in, h.fileComment, fileCommentSize);

    return h;
}

EndOfCentralDirectoryRecord EndOfCentralDirectoryRecord::read(std::istream &in)
{
    checkSignature("end of central directory", in
                   , END_OF_CENTRAL_DIRECTORY_SIGNATURE);

    EndOfCentralDirectoryRecord r;

    bin::read(in, r.numberOfThisDisk);
    bin::read(in, r.diskWhereCentralDirectoryStarts);
    bin::read(in, r.numberOfCentralDirectoryRecordsOnThisDisk);
    bin::read(in, r.totalNumberOfCentralDirectoryRecords);
    bin::read(in, r.sizeOfCentralDirectory);
    bin::read(in, r.centralDirectoryOffset);

    const auto fileCommentSize(bin::read<std::uint16_t>(in));
    readVector(in, r.comment, fileCommentSize);

    return r;
}

} // namespace

Reader::Reader(const fs::path &path)
    : path_(path), fd_(::open(path.string().c_str(), O_RDONLY), path)
{
    if (!fd_) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot open zip file " << fd_.path() << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    struct ::stat st;
    if (-1 == ::fstat(fd_, &st)) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot stat zip file " << fd_.path() << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    std::size_t fsize(st.st_size);

    try {
        boost::iostreams::stream<utility::io::SubStreamDevice> f
            (utility::io::SubStreamDevice
             (path_, utility::io::SubStreamDevice::Filedes
              { int(fd_), std::size_t(0), fsize }), 512);
        f.exceptions(std::ios::badbit | std::ios::failbit);

        // read at most 1 KB block from the end of file
        auto bsize = (fsize > 1024) ? 1024 : fsize;
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
            << "Cannot process zip file " << path << ": " << e.what() << ".";
    }
}

RawFile Reader::rawfile(std::size_t index) const
{
    if (index >= records_.size())  {
        LOGTHROW(err2, Error)
            << "Invalid file index " << index << " in zip file "
            << path_ << ".";
    }

    const auto &record(records_[index]);

    boost::iostreams::stream<utility::io::SubStreamDevice> hf
        (utility::io::SubStreamDevice
         (path_, utility::io::SubStreamDevice::Filedes
          { int(fd_), std::size_t(record.headerStart)
            , std::size_t(record.headerStart) + LocalFileHeader::baseSize() })
         , 512);

    MinimalFileHeader header;
    header.read(hf);
    std::size_t fileStart(record.headerStart + header.size());
    std::size_t fileEnd(fileStart + header.compressedSize);

    return RawFile(static_cast<CompressionMethod>(header.compressionMethod)
                   , header.uncompressedSize
                   , utility::io::SubStreamDevice
                   (record.path, utility::io::SubStreamDevice::Filedes
                    { int(fd_), fileStart, fileEnd }));
}



PluggedFile Reader::plug(std::size_t index
                         , boost::iostreams::filtering_istream &fis)
    const
{
    const auto raw(rawfile(index));
    switch (raw.compressionMethod) {
    case CompressionMethod::store:
        // nothing more
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
            << "Unsupported compression method for file "
            << raw.device.path() << " in zip file "
            << path_ << ".";
    }

    fis.push(raw.device);
    return raw;
}

} } // namespace utility::zip
