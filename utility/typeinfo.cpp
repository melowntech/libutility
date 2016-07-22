#include "./typeinfo.hpp"

#ifdef __GNUG__
// gnu

#include <cstdlib>
#include <memory>
#include <cxxabi.h>

namespace utility {

std::string demangleTypeName(const char *name)
{

    int status(0);

    std::unique_ptr<char, void(*)(void*)> res
        (abi::__cxa_demangle(name, nullptr, nullptr, &status)
         , std::free);

    return status ? name : res.get();
}

} // namespace utility

#else

// non-gnu

namespace utility {

std::string demangleTypeName(const char *name)
{
    return name;
}

} // namespace utility

#endif
