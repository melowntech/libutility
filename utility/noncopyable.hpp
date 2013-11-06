#ifndef utility_noncopyable_hpp_included_
#define utility_noncopyable_hpp_included_

namespace utility {

struct Noncopyable {
    Noncopyable() {}

    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;
    Noncopyable(Noncopyable&&) = default;
    Noncopyable& operator=(Noncopyable&&) = default;
};

} // namespace utility

#endif // utility_noncopyable_hpp_included_
