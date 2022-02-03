#include "uncaught-exception.hpp"

namespace utility {

bool uncaught_exception() noexcept
{
#if __cplusplus >= 201703L
    return std::uncaught_exceptions();
#else
    return std::uncaught_exception();
#endif
}

} // utility

