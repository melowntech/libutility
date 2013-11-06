#ifndef utility_process_hpp_included_
#define utility_process_hpp_included_

#include <string>
#include <vector>
#include <utility>
#include <functional>
#include <stdexcept>
#include <system_error>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

namespace utility {

struct InFile {
    int fd;
    InFile(int fd) : fd(fd) {}
};

struct OutFile {
    int fd;
    OutFile(int fd) : fd(fd) {}
};

struct ErrFile {
    int fd;
    ErrFile(int fd) : fd(fd) {}
};

namespace detail {

struct Context {
    std::vector<std::string> argv;
    boost::optional<InFile> inFile;
    boost::optional<OutFile> outFile;
    boost::optional<ErrFile> errFile;

    void set(const InFile &f) {
        if (inFile) {
            throw std::runtime_error("Input file already specified.");
        }
        inFile = f;
    }

    void set(const OutFile &f) {
        if (outFile) {
            throw std::runtime_error("Output file already specified.");
        }
        outFile = f;
    }

    void set(const ErrFile &f) {
        if (errFile) {
            throw std::runtime_error("Error file already specified.");
        }
        errFile = f;
    }
};

inline void systemBuildArgs(Context&) {}

inline void systemBuildArgs(Context &ctx, InFile &&f)
{
    ctx.set(f);
}

inline void systemBuildArgs(Context &ctx, OutFile &&f)
{
    ctx.set(f);
}

inline void systemBuildArgs(Context &ctx, ErrFile &&f)
{
    ctx.set(f);
}

template <typename ...Args>
inline void systemBuildArgs(Context &ctx, InFile &&f, Args &&...rest)
{
    ctx.set(f);
    return detail::systemBuildArgs(ctx, std::forward<Args>(rest)...);
}

template <typename ...Args>
inline void systemBuildArgs(Context &ctx, OutFile &&f, Args &&...rest)
{
    ctx.set(f);
    return detail::systemBuildArgs(ctx, std::forward<Args>(rest)...);
}

template <typename ...Args>
inline void systemBuildArgs(Context &ctx, ErrFile &&f, Args &&...rest)
{
    ctx.set(f);
    return detail::systemBuildArgs(ctx, std::forward<Args>(rest)...);
}

template <typename T, typename ...Args>
inline void systemBuildArgs(Context &ctx, T &&arg, Args &&...rest)
{
    ctx.argv.push_back(boost::lexical_cast<std::string>(arg));
    return detail::systemBuildArgs(ctx, std::forward<Args>(rest)...);
}

/** Equivalent of system(3); does fork/exec and waits for result.
 */
int systemImpl(const std::string &program, const Context &ctx);

int spawnImpl(const std::function<int ()> &func);

} // namespace detail

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
