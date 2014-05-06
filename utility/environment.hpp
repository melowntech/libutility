#ifndef utility_environment_hpp_included_
#define utility_environment_hpp_included_

#include <map>

#include <boost/optional.hpp>

namespace utility {

typedef std::map<std::string, boost::optional<std::string> > Environment;

void apply(const Environment &environ);

} // namespace utility

#endif // utility_environment_hpp_included_
