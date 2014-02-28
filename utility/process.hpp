#ifndef utility_process_hpp_included_
#define utility_process_hpp_included_

#include <cstdio>
#include <string>
#include <vector>
#include <utility>
#include <functional>
#include <stdexcept>
#include <system_error>
#include <iosfwd>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <boost/any.hpp>

namespace utility {

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
        boost::optional<std::string> format;
    };

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
        : dst(DstArg()), dstType(DstType::arg)
        , src(&is), srcType(SrcType::istream) {}
    RedirectFile(std::ostream &os)
        : dst(DstArg()), dstType(DstType::arg)
        , src(&os), srcType(SrcType::ostream) {}

    RedirectFile(const std::string &format, std::istream &is)
        : dst(DstArg(format)), dstType(DstType::arg)
        , src(&is), srcType(SrcType::istream) {}
    RedirectFile(const std::string &format, std::ostream &os)
        : dst(DstArg(format)), dstType(DstType::arg)
        , src(&os), srcType(SrcType::ostream) {}

    boost::any dst;
    enum class DstType { fd, arg };
    DstType dstType;

    boost::any src;
    enum class SrcType { fd, path, istream, ostream };
    SrcType srcType;
};

struct Stdin : RedirectFile {
    explicit Stdin(int fd) : RedirectFile(STDIN_FILENO, fd) {}
    explicit Stdin(const boost::filesystem::path &path)
        : RedirectFile(STDIN_FILENO, path, false) {}
    explicit Stdin(std::istream &is) : RedirectFile(STDIN_FILENO, is) {}
};

struct Stdout : RedirectFile {
    explicit Stdout(int fd) : RedirectFile(STDOUT_FILENO, fd) {}
    explicit Stdout(const boost::filesystem::path &path)
        : RedirectFile(STDOUT_FILENO, path, true) {}
    explicit Stdout(std::ostream &os) : RedirectFile(STDOUT_FILENO, os) {}
};

struct Stderr : RedirectFile {
    explicit Stderr(int fd) : RedirectFile(STDERR_FILENO, fd) {}
    explicit Stderr(const boost::filesystem::path &path)
        : RedirectFile(STDERR_FILENO, path, true) {}
    explicit Stderr(std::ostream &os) : RedirectFile(STDERR_FILENO, os) {}
};

struct InStream : RedirectFile {
    explicit InStream(std::istream &is) : RedirectFile(is) {}
    explicit InStream(const std::string &format, std::istream &is)
        : RedirectFile(format, is) {}
};

struct OutStream : RedirectFile {
    explicit OutStream(std::ostream &os) : RedirectFile(os) {}
    explicit OutStream(const std::string &format, std::ostream &os)
        : RedirectFile(format, os) {}
};

struct SetEnv {
    std::string name;
    std::string value;
    explicit SetEnv(const std::string &name, const std::string value = "")
        : name(name), value(value)
    {}
};

struct UnsetEnv {
    std::string name;
    explicit UnsetEnv(const std::string &name) : name(name) {}
};

/** Execute child process and wait for its completion.
 *
 *  Argument can be one of:
 *     Stdin/Stdout/Stderr: fd/path/stream to redirect stdin/stdout/stderr
 *                          to/from
 *     InStream/OutStream: stream input/output
 *     SetEnv: sets environmental variable
 *     UnsetEnv: unsets environmental variable
 *     other: positional argument to exec (converted to string
 *            via boost::lexical_cast)
 *
 *  First positional argument is interpreted as path to binary to execute.
 */
template <typename ...Args>
int system(const std::string &program, Args &&...args);

int spawn(const std::function<int ()> &func);

} // namespace utility

#include "./detail/process.hpp"

// implementation

namespace utility {

template <typename ...Args>
inline int system(const std::string &program, Args &&...args)
{
    detail::Context ctx;
    detail::systemBuildArgs(ctx, std::forward<Args>(args)...);
    return detail::systemImpl(program, ctx);
}

/** Call function in new process.
 *  Convenient wrapper around fork(2)
 */
inline int spawn(const std::function<int ()> &func)
{
    return detail::spawnImpl(func);
}

} // namespace utility

#endif // utility_process_hpp_included_
