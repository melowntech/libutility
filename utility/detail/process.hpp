#ifndef utility_detail_process_hpp_included_
#define utility_detail_process_hpp_included_

#include <vector>
#include <map>
#include <utility>

#include <boost/optional.hpp>

#include "../process.hpp"

namespace utility { namespace detail {

struct Context {
    typedef std::vector<boost::optional<std::string> > Argv;
    typedef std::vector<RedirectFile> Redirects;
    typedef std::map<int, int> PlaceHolders;

    Argv argv;
    Redirects redirects;
    PlaceHolders placeHolders;

    typedef std::map<std::string, boost::optional<std::string> > Environ;
    Environ environ;

    void add(const RedirectFile &arg) {
        // TODO: check for duplicity

        placeHolders[redirects.size()] = argv.size();

        redirects.push_back(arg);

        // placeholder for redirect argument (if any)
        argv.push_back(boost::none);
    }

    void apply(const Stdin &arg) { add(arg); }
    void apply(const Stderr &arg) { add(arg); }
    void apply(const Stdout &arg) { add(arg); }
    void apply(const InStream &arg) { add(arg); }
    void apply(const OutStream &arg) { add(arg); }

    void apply(const SetEnv &arg) {
        environ[arg.name] = arg.value;
    }

    void apply(const UnsetEnv &arg) {
        environ[arg.name] = boost::none;
    }

    void apply(const std::string &arg) {
        argv.push_back(arg);
    }

    template <typename T>
    void apply(const T &arg) {
        argv.push_back(boost::lexical_cast<std::string>(arg));
    }

    void setFdPath(int redirectIdx, const RedirectFile::DstArg &arg, int fd);
};

inline void systemBuildArgs(Context&) {}
#if 0
template <typename ...Args>
inline void systemBuildArgs(Context &ctx, Stdin &&f, Args &&...rest)
{
    ctx.set(f);
    return detail::systemBuildArgs(ctx, std::forward<Args>(rest)...);
}

template <typename ...Args>
inline void systemBuildArgs(Context &ctx, Stdout &&f, Args &&...rest)
{
    ctx.set(f);
    return detail::systemBuildArgs(ctx, std::forward<Args>(rest)...);
}

template <typename ...Args>
inline void systemBuildArgs(Context &ctx, Stderr &&f, Args &&...rest)
{
    ctx.set(f);
    return detail::systemBuildArgs(ctx, std::forward<Args>(rest)...);
}

template <typename ...Args>
inline void systemBuildArgs(Context &ctx, InStream &&f, Args &&...rest)
{
    ctx.set(f);
    return detail::systemBuildArgs(ctx, std::forward<Args>(rest)...);
}

template <typename ...Args>
inline void systemBuildArgs(Context &ctx, OutStream &&f, Args &&...rest)
{
    ctx.set(f);
    return detail::systemBuildArgs(ctx, std::forward<Args>(rest)...);
}

template <typename ...Args>
inline void systemBuildArgs(Context &ctx, SetEnv &&s, Args &&...rest)
{
    ctx.apply(s);
    return detail::systemBuildArgs(ctx, std::forward<Args>(rest)...);
}

template <typename ...Args>
inline void systemBuildArgs(Context &ctx, UnsetEnv &&s, Args &&...rest)
{
    ctx.apply(s);
    return detail::systemBuildArgs(ctx, std::forward<Args>(rest)...);
}
#endif

template <typename T, typename ...Args>
inline void systemBuildArgs(Context &ctx, T &&arg, Args &&...rest)
{
    // ctx.argv.push_back(boost::lexical_cast<std::string>(arg));
    ctx.apply(arg);
    return detail::systemBuildArgs(ctx, std::forward<Args>(rest)...);
}

/** Equivalent of system(3); does fork/exec and waits for result.
 */
int systemImpl(const std::string &program, Context ctx);

int spawnImpl(const std::function<int ()> &func);

} } // namespace utility::detail

#endif // utility_detail_process_hpp_included_
