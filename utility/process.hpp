#ifndef utility_exec_hpp_included_
#define utility_exec_hpp_included_

#include <string>
#include <vector>
#include <utility>

#include <boost/lexical_cast.hpp>

namespace utility {

namespace detail {

inline void systemBuildArgs(std::vector<std::string>&) {}

template <typename T, typename ...Args>
inline void systemBuildArgs(std::vector<std::string> &argv
                           , T &&arg, Args &&...rest)
{
    argv.push_back(boost::lexical_cast<std::string>(arg));
    return detail::systemBuildArgs(argv, std::forward<Args>(rest)...);
}

/** Equivalent of system(3); does fork/exec and waits for result.
 */
int systemImpl(const std::string &program
               , const std::vector<std::string> &args);

} // namespace detail

template <typename ...Args>
inline int system(const std::string &program, Args &&...args)
{
    std::vector<std::string> argv;
    detail::systemBuildArgs(argv, std::forward<Args>(args)...);
    return detail::systemImpl(program, argv);
}


} // namespace utility


#endif // utility_exec_hpp_included_
