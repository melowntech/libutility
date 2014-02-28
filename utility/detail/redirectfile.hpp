#ifndef utility_detail_redirectfile_hpp_included_
#define utility_detail_redirectfile_hpp_included_

#include <string>
#include <vector>
#include <utility>
#include <iosfwd>
#include <boost/filesystem/path.hpp>
#include <boost/any.hpp>

namespace utility { namespace detail {

class SystemContext;

struct RedirectFile {
    struct SrcPath {
        SrcPath(const boost::filesystem::path &path, bool out)
            : path(path), out(out) {}
        boost::filesystem::path path;
        bool out;
    };

    struct DstArg {
        DstArg() {}
        DstArg(const std::string &format) : format(format) {}
        std::string format;
    };

    boost::any dst;
    enum class DstType { fd, arg, none };
    DstType dstType;

    boost::any src;
    enum class SrcType { fd, path, istream, ostream };
    SrcType srcType;

    RedirectFile() {}
    RedirectFile(int dst, int src)
        : dst(dst), dstType(DstType::fd)
        , src(src), srcType(SrcType::fd) {}
    RedirectFile(int dst, const boost::filesystem::path &path, bool out)
        : dst(dst), dstType(DstType::fd)
        , src(SrcPath(path, out)), srcType(SrcType::path) {}
    RedirectFile(int dst, std::istream &is)
        : dst(dst), dstType(DstType::fd)
        , src(&is), srcType(SrcType::istream) {}
    RedirectFile(int dst, std::ostream &os)
        : dst(dst), dstType(DstType::fd)
        , src(&os), srcType(SrcType::ostream) {}

    RedirectFile(std::istream &is)
        : dst(), dstType(DstType::none), src(&is), srcType(SrcType::istream) {}
    RedirectFile(std::ostream &os)
        : dst(), dstType(DstType::none), src(&os), srcType(SrcType::ostream) {}

    RedirectFile(const std::string &format, std::istream &is)
        : dst(DstArg(format)), dstType(DstType::arg)
        , src(&is), srcType(SrcType::istream) {}
    RedirectFile(const std::string &format, std::ostream &os)
        : dst(DstArg(format)), dstType(DstType::arg)
        , src(&os), srcType(SrcType::ostream) {}
};

} } // namespace utility::detail

#endif // utility_detail_redirectfile_hpp_included_
