#ifndef utility_pre_main_hpp_included_
#define utility_pre_main_hpp_included_

#include <functional>

namespace utility {

struct PreMain {
    PreMain(const std::function<void()> &code) { code(); }
};

} // namespace utility

#endif // utility_pre_main_hpp_included_
