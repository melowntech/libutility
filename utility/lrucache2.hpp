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
/**
 * @file lrucache2.hpp
 * @author Jakub Cerveny <jakub.cerveny@melown.com>
 *
 * Multi-threaded LRU cache.
 */

#ifndef utility_lrucache2_hpp_included_
#define utility_lrucache2_hpp_included_

#include <list>
#include <unordered_map>
#include <mutex>

#include <boost/noncopyable.hpp>

#include "dbglog/dbglog.hpp"

namespace utility {

/** Multi-threaded LRU cache implementation. Compared to the simpler LruCache
 *  class, this version has proper load locking and is suitable for items that
 *  are costly to load. Other threads may continue to use the cache while items
 *  are being loaded.
 */
template<typename Key, typename Value, typename CostType = std::size_t>
class LruCache2 : boost::noncopyable
{
public:
    typedef std::shared_ptr<Value> value_pointer;

    LruCache2(CostType maxCost)
        : maxCost_(maxCost), totalCost_()
        , missCnt_(0), hitCnt_(0)
    {}

    /** Get an item from the cache (identified by 'key'). If the item is not
     *  in the cache, the supplied loading function is called first. The loading
     *  function takes 'key' and returns a tuple (ptr, size), where ptr is a
     *  shared pointer to 'Value' and size is the cost of the item:
     *
     *    std::tuple<std::shared_ptr<Value>, CostType> loadFunc(Key);
     */
    template<typename LoadFunc>
    value_pointer get(const Key &key, LoadFunc loadFunc);

    /** Set a limit on the total cost of items in the cache.
     */
    void setMaxCost(CostType maxCost) { maxCost_ = maxCost; }

    /** Removes as many LRU elements as needed to get total cost under 'limit'.
     *  Returns the number of items removed.
     */
    std::size_t trim(CostType limit) {
        std::unique_lock<std::mutex> mainLock(mainMutex_);
        return trimImpl(limit);
    }

    /** Return total cost of items in the cache.
     */
    CostType totalCost() { return totalCost_; }

    ~LruCache2() {
        LOG(info2) << "Cache hit count: " << hitCnt_
                   << ", miss count: " << missCnt_;
    }

protected:
    struct Item : boost::noncopyable
    {
        Key key;
        value_pointer ptr;
        CostType cost;

        bool loading;
        std::mutex loadMutex;

        Item(const Key &key) : key(key), cost(), loading(true) {}
    };

    std::list<Item> itemList_;

    typedef decltype(itemList_.begin()) list_iterator;
    std::unordered_map<Key, list_iterator> itemMap_;

    CostType maxCost_;
    CostType totalCost_;
    long long missCnt_, hitCnt_;

    std::mutex mainMutex_;

    std::size_t trimImpl(CostType limit);
};


// implementation

template<typename Key, typename Value, typename CostType>
template<typename LoadFunc>
typename LruCache2<Key, Value, CostType>::value_pointer
LruCache2<Key, Value, CostType>::get(const Key &key, LoadFunc loadFunc)
{
    std::unique_lock<std::mutex> mainLock(mainMutex_);

    auto it = itemMap_.find(key);
    if (it != itemMap_.end())
    {
        // item already in cache, move it to the end of the list
        itemList_.splice(itemList_.end(), itemList_, it->second);

        Item &item = *(it->second);
        if (!item.loading)
        {
            LOG(info1) << "Cache hit on key <" << key << ">.";
            hitCnt_++;
            return item.ptr;
        }

        // the item is loading, wait and try again
        mainLock.unlock();
        {
            LOG(info1) << "Waiting while key <"  << key << "> is loading.";
            std::lock_guard<std::mutex> loadLock(item.loadMutex);
        }
        return get(key, loadFunc);
    }

    LOG(info1) << "Cache miss on key <" << key << ">.";
    missCnt_++;

    // create a new cache entry
    itemList_.emplace_back(key);
    itemMap_[key] = --itemList_.end();

    Item &item = itemList_.back();
    item.loading = true;

    // lock the item and unlock the cache
    std::lock_guard<std::mutex> loadLock(item.loadMutex);
    mainLock.unlock();

    // load the item
    LOG(info1) << "Loading cache item <" << key << ">.";
    std::tie(item.ptr, item.cost) = loadFunc(key);

    mainLock.lock();
    totalCost_ += item.cost;

    // free LRU items if necessary
    trimImpl(maxCost_);

    item.loading = false;
    return item.ptr;
}


template<typename Key, typename Value, typename CostType>
std::size_t LruCache2<Key, Value, CostType>::trimImpl(CostType limit)
{
    std::size_t ndeleted = 0;

    auto it = itemList_.begin();
    while (it != itemList_.end() && totalCost_ > limit)
    {
        if (it->loading) {
            it++;
            continue;
        }

        LOG(info1) << "Deleting cache item <" << it->key << ">.";
        totalCost_ -= it->cost;
        itemMap_.erase(it->key);
        it = itemList_.erase(it);

        ndeleted++;
    }

    LOG(info1) << "Cache size is " << totalCost_ << " in " << itemList_.size()
               << " items (just deleted " << ndeleted << " items).";

    return ndeleted;
}

} // namespace utility

#endif // utility_lrucache2_hpp_included_
