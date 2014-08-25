#ifndef utility_detail_process_hpp_included_
#define utility_detail_process_hpp_included_

#include <vector>
#include <map>
#include <utility>

#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

#include "../process.hpp"
#include "../environment.hpp"

namespace utility { namespace detail {

struct SystemContext {
    typedef std::vector<boost::optional<std::string> > Argv;
    typedef std::vector<RedirectFile> Redirects;
    typedef std::map<int, int> PlaceHolders;

    Argv argv;
    Redirects redirects;
    PlaceHolders placeHolders;

    Environment environ;

    boost::optional<boost::filesystem::path> cwd;

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
    void apply(const Stream &arg) { add(arg); }
    void apply(const SetEnv &arg) { environ[arg.name] = arg.value; }
    void apply(const UnsetEnv &arg) { environ[arg.name] = boost::none; }
    void apply(const ChangeCwd &arg) { cwd = arg.wd; }
    void apply(const std::string &arg) { argv.push_back(arg); }

    template <typename T>
    void apply(const T &arg) {
        argv.push_back(boost::lexical_cast<std::string>(arg));
    }

    void setFdPath(int redirectIdx, const RedirectFile::DstArg &arg, int fd);
};

inline void systemBuildArgs(SystemContext&) {}

template <typename T, typename ...Args>
inline void systemBuildArgs(SystemContext &ctx, T &&arg, Args &&...rest)
{
    // ctx.argv.push_back(boost::lexical_cast<std::string>(arg));
    ctx.apply(arg);
    return detail::systemBuildArgs(ctx, std::forward<Args>(rest)...);
}

/** Equivalent of system(3); does fork/exec and waits for result.
 */
int systemImpl(const std::string &program, SystemContext ctx);

int spawnImpl(const std::function<int ()> &func);

} } // namespace utility::detail

#endif // utility_detail_process_hpp_included_
