#include <iostream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "dbglog/dbglog.hpp"
#include "utility/binaryio.hpp"

/** Helper stuff to load files in either boost::archive::binary_iarchive or
 *  custom format. Useful when converting Boost.Serialization binary format.
 */
namespace utility { namespace archiveloader {

template <std::size_t size>
bool checkMagic(std::istream &f, const char (&expectedMagic)[size])
{
    char magic[size];
    binaryio::read(f, magic);

    return !std::memcmp(magic, expectedMagic, size);
}

template <std::size_t size>
bool checkMagic(std::istream &f, const std::array<char, size> &expectedMagic)
{
    char magic[size];
    binaryio::read(f, magic);

    return !std::memcmp(magic, expectedMagic.data(), size);
}

/** Check for two alternative magic numbers if the same size.
 *  Returns 0 if not matched, 1 if expected magic matched and 2 if alternative
 *  magic matched
 */
template <std::size_t size>
std::size_t checkMagic(std::istream &f
                       , const std::array<char, size> &expectedMagic
                       , const std::array<char, size> &altMagic)
{
    char magic[size];
    binaryio::read(f, magic);

    if (!std::memcmp(magic, expectedMagic.data(), size)) { return 1; }
    if (!std::memcmp(magic, altMagic.data(), size)) { return 2; }

    return false;
}

template <typename BadFileFormat>
std::uint16_t
checkVersion(std::istream &f, const boost::filesystem::path &filename
             , std::uint16_t expectedMinVersion
             , std::uint16_t expectedMaxVersion
             , const char *type)
{
    // load and check version
    std::uint16_t version;
    binaryio::read(f, version);

    if (version < expectedMinVersion) {
        LOGTHROW(err1, BadFileFormat)
            << "Invalid version in serialized " << type
            << " in file " << filename << ".";
    }

    if (version > expectedMaxVersion) {
        LOGTHROW(err1, BadFileFormat)
            << "Invalid version in serialized " << type
            << " in file " << filename << ".";
    }

    return version;
}

template <typename BadFileFormat>
boost::archive::binary_iarchive&
checkVersion(const boost::filesystem::path &filename
             , boost::archive::binary_iarchive &ia
             , unsigned int expectedVersion)
{
    unsigned int version;
    ia & version;
    if (expectedVersion != version) {
        LOGTHROW(err1, BadFileFormat)
            << "Wrong pointcloud data version: "
            << " expected " << expectedVersion << ", got " << version
            << " (file: " << filename << ").";
    }
    return ia;
}

/**
 * Loads archive content.
 * Detects format and calls one of callbacks:
 *
 * for boost archive:
 *     callbackArchive(boost::archive::binary_iarchive &archive)
 * for custom format:
 *     callbackStream(std::istream &stream)
 *
 */
template <typename CallbackArchive, typename CallbackStream, std::size_t size>
void loadArchive(const boost::filesystem::path &filename
                 , std::istream &ifs
                 , const char (&expectedMagic)[size]
                 , CallbackArchive callbackArchive
                 , CallbackStream callbackStream)
{
    try {
        // try gzipped format
        {
            // try new format
            boost::iostreams::filtering_istream filter;
            filter.push(boost::iostreams::gzip_decompressor());
            filter.push(ifs);

            if (checkMagic(filter, expectedMagic)) {
                // new format

                // call callback
                callbackStream(filter);
                return;
            }

            // not new format
        }

        // rewind stream
        ifs.seekg(0);

        boost::iostreams::filtering_istream filter;
        filter.push(boost::iostreams::gzip_decompressor());
        filter.push(ifs);

        // try old boost archive
        boost::archive::binary_iarchive ia(filter);
        callbackArchive(ia);
    } catch (const boost::iostreams::gzip_error&) {
        // it must be ungzipped old boost archive
        LOG(warn1) << "File " << filename << " probably not gzipped, "
                   << "try old plain format.";
        ifs.seekg(0);
        boost::archive::binary_iarchive ia(ifs);
        callbackArchive(ia);
    }
}

} } // namespace utility::archiveloader
