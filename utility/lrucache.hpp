/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef utility_lrucache_hpp_included_
#define utility_lrucache_hpp_included_

#include <memory>
#include <iostream>
#include <mutex>

#include <boost/noncopyable.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#include "dbglog/dbglog.hpp"
#include "utility/streams.hpp"

namespace utility {

/** Value traits for LruCache.
 *
 * Should contain 2 static functions:
 *     key_ key(const Value &v)
 *     cost_type cost(const Value &v)
 *
 */
template <typename Value> struct LruCacheTraits;

namespace detail {

template <typename T> inline const T& reference() {
    return *static_cast<const T*>(nullptr);
}

} // namespace detail

/** LRU Cache for arbitrary values.
 *  Values are held by share pointer.
 *  There must be LruCacheTraits specialization for Value:
 *      template <typename>
 *      struct LruCacheTraits<Value> {
 *          static key_type key(const Value &);
 *          static cost_type cost(const Value &);
 *      }
 *
 *  All operations on cache are thread safe.
 */
template <typename Value, typename TraitsType = LruCacheTraits<Value> >
class LruCache : boost::noncopyable {
public:
    typedef TraitsType Traits;
    typedef std::shared_ptr<Value> value_pointer;

    /** Determine key type via traits
     */
    typedef typename std::remove_reference
    <decltype(Traits::key(detail::reference<Value>()))>
    ::type key_type;

    /** Determine cost type via traits
     */
    typedef typename std::remove_reference
    <decltype(Traits::cost(detail::reference<Value>()))>
    ::type cost_type;

    /** Create cache.
     */
    LruCache() : priorityGenerator_() {}

    /** Insert new element to cache. Returns false on conflict.
     */
    bool insert(const value_pointer &v);

    /** Returns element under key or nullptr if not found.
     *  Element is moved to to head of priority queue.
     */
    value_pointer get(const key_type &key);

    /** Removes as many as needed elements from priority queue tail until total
     * cost of all items in the cache is not greated than provided limit.
     */
    std::size_t trim(cost_type limit);

    /** Return total cost of items in cache 
     */
    cost_type totalCost() { return totalCost_; };

private:
    typedef std::uint64_t priority_type;

    /** Cache entry.
     */
    struct Entry {
        value_pointer value;
        key_type key;
        priority_type priority;

        template <typename T, typename E>
        std::basic_ostream<T,E>& dump(std::basic_ostream<T,E> &os
                                      , const std::string&) const
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

template <typename Value, typename TraitsType>
bool LruCache<Value, TraitsType>::insert(const value_pointer &v)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (!container_.insert(Entry{v, Traits::key(*v), priorityGenerator_})
        .second)
    {
        // key conflict!
        return false;
    }
    ++priorityGenerator_;
    totalCost_ += Traits::cost(*v);
    return true;
}

template <typename Value, typename TraitsType>
typename LruCache<Value, TraitsType>::value_pointer
LruCache<Value, TraitsType>::get(const key_type &key)
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

    LOG(debug) << "lru-cache: found entry " << utility::dump(*fkeys);

    // modify entry (this may fail!)
    keys.modify
        (fkeys, [&](Entry &e) { e.priority = priorityGenerator_; });

    ++priorityGenerator_;

    return fkeys->value;
}

template <typename Value, typename TraitsType>
std::size_t LruCache<Value, TraitsType>::trim(cost_type limit)
{
    std::unique_lock<std::mutex> lock(mutex_);

    std::size_t removed(0);

    // get priority index
    auto &priorities(container_.template get<PriorityIdx>());
    for (auto ipriorities(priorities.begin())
             , epriorities(priorities.end())
             ; (totalCost_ > limit) && (ipriorities != epriorities); )
    {
        LOG(debug) << "lru-cache: removing " << utility::dump(*ipriorities)
                   << " because total " << totalCost_ << " > "
                   << limit;

        // remove entry and update total cost
        auto cost(Traits::cost(*ipriorities->value));
        ipriorities = priorities.erase(ipriorities);
        totalCost_ -= cost;
        ++removed;
    }

    return removed;
}

} // namespace utility

#endif // utility_lrucache_hpp_included_
