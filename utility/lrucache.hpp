#ifndef utility_lrucache_hpp_included_
#define utility_lrucache_hpp_included_

#include <memory>
#include <iostream>
#include <mutex>

#include <boost/noncopyable.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>

namespace utility {

template <typename Value> struct LruCacheTraits;

template <typename T> inline const T& reference() {
    return *static_cast<const T*>(nullptr);
}

template <typename Value>
class LruCache : boost::noncopyable {
public:
    typedef std::shared_ptr<Value> value_pointer;

    /** Determine key type via traits
     */
    typedef typename std::remove_reference
    <decltype(LruCacheTraits<Value>::key(reference<Value>()))>
    ::type key_type;

    /** Determine cost type via traits
     */
    typedef typename std::remove_reference
    <decltype(LruCacheTraits<Value>::cost(reference<Value>()))>
    ::type cost_type;

    LruCache() : priorityGenerator_() {}

    void insert(const value_pointer &v);

    value_pointer get(const key_type &key);

    std::size_t trim(cost_type limit);

private:
    typedef std::uint64_t priority_type;

    typedef LruCacheTraits<Value> Traits;

    /** Cache entry.
     */
    struct Entry {
        value_pointer value;
        key_type key;
        priority_type priority;

        template <typename T, typename E>
        std::basic_ostream<T,E>& dump(std::basic_ostream<T,E> &os) const
        {
            return os << "value=" << value.get() << ", key=" << key
                      << ", priority=" << priority;
        }
    };

    struct KeyIdx;
    struct PriorityIdx;

    typedef boost::multi_index_container<
        Entry // what is held by the container
        , boost::multi_index::indexed_by<
              // first index: ordered-unique ~ std::map
              boost::multi_index::ordered_unique
              <boost::multi_index::tag<KeyIdx> // accessible by KeyIdx
               // key extractor:
               , BOOST_MULTI_INDEX_MEMBER(Entry, key_type, key)>

              // second index: ordered-unique ~ std::map
              , boost::multi_index::ordered_unique
              <boost::multi_index::tag<PriorityIdx> // accessible by PriorityIdx
               // key extractor:
               , BOOST_MULTI_INDEX_MEMBER(Entry, priority_type, priority)>
              >
        > Container;

    Container container_; // all is stored here
    priority_type priorityGenerator_; // this generates unique priority

    cost_type totalCost_; // total cost of all values in the key

    std::mutex mutex_;
};

template <typename Value>
void LruCache<Value>::insert(const value_pointer &v)
{
    std::unique_lock<std::mutex> lock(mutex_);

    container_.insert(Entry{v, Traits::key(*v), priorityGenerator_});
    ++priorityGenerator_;
    totalCost_ += Traits::cost(*v);
}

template <typename Value>
typename LruCache<Value>::value_pointer
LruCache<Value>::get(const key_type &key)
{
    std::unique_lock<std::mutex> lock(mutex_);

    // get key index
    auto &keys(container_.template get<KeyIdx>());

    // try to find entry by key
    auto fkeys(keys.find(key));
    if (fkeys == keys.end()) {
        // not found -> nullptr
        return {};
    }

    std::cout << "found entry ";
    fkeys->dump(std::cout);
    std::cout << std::endl;

    // FIXME: this may throw when entry cannot be modified (it removes the
    // entry completely)
    container_.modify
        (fkeys, [&](Entry &e) { e.priority = priorityGenerator_; });

    ++priorityGenerator_;

    return fkeys->value;
}

template <typename Value>
std::size_t LruCache<Value>::trim(cost_type limit)
{
    std::unique_lock<std::mutex> lock(mutex_);

    std::size_t removed(0);

    // get priority index
    auto &priorities(container_.template get<PriorityIdx>());
    for (auto ipriorities(priorities.begin())
             , epriorities(priorities.end())
             ; (totalCost_ > limit) && (ipriorities != epriorities); )
    {
        std::cout << "Removing: ";
        ipriorities->dump(std::cout);
        std::cout << " because total " << totalCost_ << " > "
                  << limit << std::endl;

        auto cost(Traits::cost(*ipriorities->value));
        ipriorities = priorities.erase(ipriorities);
        totalCost_ -= cost;
        ++removed;
    }

    return removed;
}

} // namespace utility

#endif // utility_lrucache_hpp_included_
