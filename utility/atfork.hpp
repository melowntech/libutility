#ifndef shared_utility_atfork_hpp_included_
#define shared_utility_atfork_hpp_included_

#include <functional>
#include <vector>

namespace utility {

class AtFork {
public:
    enum Event { prepare, parent, child };
    typedef std::function<void(Event)> Callback;

    static void add(const void *id, const Callback &cb);
    static void remove(const void *id);
    static void run(Event event);

private:
    AtFork();

    struct Entry {
        const void *id;
        Callback cb;

        Entry(const void *id, Callback cb) : id(id), cb(cb) {}

        Entry() : id(), cb() {}
    };

    static std::vector<Entry> entries_;
    static AtFork init_;
};

} // namespace utility

#endif // shared_utility_atfork_hpp_included_
