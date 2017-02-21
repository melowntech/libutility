#ifndef utility_small_map_hpp_included_
#define utility_small_map_hpp_included_

#include <vector>
#include <utility>
#include <initializer_list>
#include <algorithm>

namespace utility {

template<typename Key, typename T
         , typename Compare = std::less<Key>
         , typename Allocator = std::allocator<std::pair<Key, T> > >
class small_map {
public:
    typedef std::pair<Key, T> value_type;
    typedef Key key_type ;
    typedef T mapped_type;
    typedef Allocator allocator_type;
    typedef std::vector<value_type, allocator_type> storage_type;
    typedef typename storage_type::size_type size_type;
    typedef typename storage_type::difference_type difference_type;
    typedef typename storage_type::reference reference;
    typedef typename storage_type::const_reference const_reference;
    typedef typename storage_type::pointer pointer;

    small_map() {}

    small_map(std::initializer_list<value_type> ilist) {
        insert(ilist.begin(), ilist.end());
    }

    allocator_type get_allocator() const { return storage_.get_allocator(); }

    typedef typename storage_type::iterator iterator;
    typedef typename storage_type::const_iterator const_iterator;
    typedef typename storage_type::reverse_iterator reverse_iterator;
    typedef typename storage_type::const_reverse_iterator
        const_reverse_iterator;

    iterator begin() { return storage_.begin(); }
    const_iterator begin() const { return storage_.begin(); }
    const_iterator cbegin() const { return storage_.cbegin(); }

    iterator end() { return storage_.end(); }
    const_iterator end() const { return storage_.end(); }
    const_iterator cend() const { return storage_.cend(); }

    reverse_iterator rbegin() { return storage_.rbegin(); }
    const_reverse_iterator rbegin() const { return storage_.rbegin(); }
    const_reverse_iterator crbegin() const { return storage_.crbegin(); }

    reverse_iterator rend() { return storage_.rend(); }
    const_reverse_iterator rend() const { return storage_.rend(); }
    const_reverse_iterator crend() const { return storage_.crend(); }

    bool empty() const { return storage_.empty(); }
    size_type size() const { return storage_.size(); }
    size_type max_size() const { return storage_.max_size(); }

    void clear() { return storage_.clear(); }

    void swap(small_map &other) {
        storage_.swap(other.storage_);
    }

    iterator erase(const_iterator position) {
        return storage_.erase(position);
    }

    iterator erase(const_iterator first, const_iterator last) {
        return storage_.erase(first, last);
    }

    size_type erase(const key_type &key) {
        auto f(find(key));
        if (f == storage_.end()) {
            return 0;
        }
        storage_.erase(f);
        return 1;
    }

    size_type count(const key_type & key) {
        return find(key) != storage_.end();
    }

    struct KeyCompare {
        bool operator()(const value_type &l, const key_type &r) {
            return Compare()(l.first, r);
        }

        bool operator()(const key_type &l, const value_type &r) {
            return Compare()(l, r.first);
        }
    };

    iterator find(const Key &key) {
        auto e(storage_.end());
        auto f(std::lower_bound(storage_.begin(), e, key, KeyCompare()));
        return ((f != e) && (f->first != key)) ? e : f;
    }

    const_iterator find(const Key &key) const {
        auto e(storage_.end());
        auto f(std::lower_bound(storage_.begin(), e, key, KeyCompare()));
        return ((f != e) && (f->first != key)) ? e : f;
    }

    T& operator[](const Key& key) {
        auto e(storage_.end());
        auto f(std::lower_bound(storage_.begin(), e, key, KeyCompare()));
        if ((f != e) && (f->first == key)) {
            // found
            return f->second;
        }

        // insert at iterator and return
        return storage_.insert(f, value_type(key, T()))->second;
    }

    std::pair<iterator, bool> insert(const value_type &value) {
        auto e(storage_.end());
        auto f(std::lower_bound(storage_.begin(), e, value.first
                                , KeyCompare()));

        if ((f != e) && (f->first == value.first)) {
            return { f, false };
        }

        return { storage_.insert(f, value), true };
    }

    std::pair<iterator, bool> insert(value_type &&value) {
        auto e(storage_.end());
        auto f(std::lower_bound(storage_.begin(), e, value.first
                                , KeyCompare()));

        if ((f != e) && (f->first == value.first)) {
            return { f, false };
        }

        return { storage_.insert(f, std::move(value)), true };
    }

    template<typename InputIt>
    void insert(InputIt first, InputIt last) {
        for (; first != last; ++first) {
            insert(*first);
        }
    }

    void insert(std::initializer_list<value_type> ilist) {
        insert(ilist.begin(), ilist.end());
    }

    /** Serialization support; implemented in small_map_serialization.hpp
     */
    template<typename Archive>
    void serialize(Archive &ar, const unsigned int version);

    /** Present only for deserialization.
     *  It is up to the caller to ensure data are sorted.
     */
    storage_type& storage() { return storage_; }

private:
    storage_type storage_;
};

template<typename CharT, typename Traits
         , typename A, typename B, typename C, typename D>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os
           , const small_map<A, B, C, D> &m)
{
    os << "{\n";
    for (const auto &t : m) {
        os << "    " << t.first << " -> " << t.second << "\n";
    }
    return os << "}\n";
}

} // namespace utility

#endif // utility_small_map_hpp_included_
