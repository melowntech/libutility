#ifndef utility_guarded_call_hpp_included_
#define utility_guarded_call_hpp_included_

namespace utility {

template <typename Prepare, typename Call, typename Finish>
auto guardedCall(Prepare prepare, Call call, Finish finish)
    -> decltype(call())
{
    struct Guard {
        Finish finish;
        Guard(Finish finish) : finish(finish) {}
        ~Guard() { finish(); }
    } guard(finish);
    prepare();
    return call();
}

} // namespace utility

#endif // utility_guarded_call_hpp_included_
