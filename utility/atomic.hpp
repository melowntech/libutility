/**
 *  @file atomic.hpp
 *  @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 *  Extra atomic support
 *
 *  2014-07-10 (vasek) created
 */

#ifndef utility_atomic_hpp_included_
#define utility_atomic_hpp_included_

#include <atomic>
#include <cstdint>

namespace utility {

namespace detail {
    template <int size> struct SizeToType;

    template <> struct SizeToType<1> { typedef std::uint8_t type; };
    template <> struct SizeToType<2> { typedef std::uint16_t type; };
    template <> struct SizeToType<4> { typedef std::uint32_t type; };
    template <> struct SizeToType<8> { typedef std::uint64_t type; };

    template <typename T> struct BitRepr {
        typedef typename SizeToType<sizeof(T)>::type type;
    };
} // namespace detail

template <typename T>
class AtomicReal {
public:
    /** type exists only if T is a floating_type
     */
    typedef typename std::enable_if<std::is_floating_point<T>::value, T>::type
        value_type;
    typedef typename detail::BitRepr<T>::type RawType;

    AtomicReal() : repr_(asRaw(0)) {}
    AtomicReal(value_type init) : repr_(asRaw(init)) {}

    AtomicReal& operator=(value_type value) {
        repr_ = asRaw(value);
        return *this;
    }

    operator value_type() const { return asT(repr_); }

    value_type load(std::memory_order order = std::memory_order_seq_cst)
        const
    {
        return asT(repr_.load(order));
    }

    value_type load(std::memory_order order = std::memory_order_seq_cst)
        const volatile
    {
        return asT(repr_.load(order));
    }

    void store(value_type value
               , std::memory_order order = std::memory_order_seq_cst)
    {
        repr_.store(asRaw(value), order);
    }

    void store(value_type value
               , std::memory_order order = std::memory_order_seq_cst)
        volatile
    {
        repr_.store(asRaw(value), order);
    }

    bool compare_exchange_weak(value_type& expected, value_type desired
                               , std::memory_order success
                               , std::memory_order failure)
    {
        return repr_.compare_exchange_weak(asRawRef(expected), asRaw(desired)
                                           , success, failure);
    }

    bool compare_exchange_weak(value_type& expected, value_type desired
                               , std::memory_order success
                               , std::memory_order failure)
        volatile
    {
        return repr_.compare_exchange_weak(asRawRef(expected), asRaw(desired)
                                           , success, failure);
    }

    bool compare_exchange_weak(value_type& expected, value_type desired
                               , std::memory_order order =
                               std::memory_order_seq_cst)
    {
        return repr_.compare_exchange_weak(asRawRef(expected), asRaw(desired)
                                           , order);
    }

    bool compare_exchange_weak(value_type& expected, value_type desired
                               , std::memory_order order =
                               std::memory_order_seq_cst)
        volatile
    {
        return repr_.compare_exchange_weak(asRawRef(expected), asRaw(desired)
                                           , order);
    }

    bool compare_exchange_strong(value_type& expected, value_type desired
                                 , std::memory_order success
                                 , std::memory_order failure)
    {
        return repr_.compare_exchange_strong(asRawRef(expected), asRaw(desired)
                                             , success, failure);
    }

    bool compare_exchange_strong(value_type& expected, value_type desired
                                 , std::memory_order success
                                 , std::memory_order failure)
        volatile
    {
        return repr_.compare_exchange_strong(asRawRef(expected), asRaw(desired)
                                             , success, failure);
    }

    bool compare_exchange_strong(value_type& expected, value_type desired
                                 , std::memory_order order =
                                 std::memory_order_seq_cst)
    {
        return repr_.compare_exchange_strong(asRawRef(expected), asRaw(desired)
                                             , order);
    }

    bool compare_exchange_strong(value_type& expected, value_type desired
                                 , std::memory_order order =
                                 std::memory_order_seq_cst)
        volatile
    {
        return repr_.compare_exchange_strong(asRawRef(expected), asRaw(desired)
                                             , order);
    }

    value_type operator+=(value_type add) {
        auto oldValue(load());
        auto newValue(oldValue + add);
        while (!compare_exchange_weak(oldValue, newValue)) {
            newValue = oldValue + add;
        }
        return newValue;
    }

    value_type operator-=(value_type add) {
        auto oldValue(load());
        auto newValue(oldValue - add);
        while (!compare_exchange_weak(oldValue, newValue)) {
            newValue = oldValue - add;
        }
        return newValue;
    }

    value_type operator++() {
        auto oldValue(load());
        auto newValue(oldValue + 1);
        while (!compare_exchange_weak(oldValue, newValue)) {
            newValue = oldValue + 1;
        }
        return newValue;
    }

    value_type operator++(int) {
        auto oldValue(load());
        auto newValue(oldValue + 1);
        while (!compare_exchange_weak(oldValue, newValue)) {
            newValue = oldValue + 1;
        }
        return oldValue;
    }

    value_type operator--() {
        auto oldValue(load());
        auto newValue(oldValue - 1);
        while (!compare_exchange_weak(oldValue, newValue)) {
            newValue = oldValue - 1;
        }
        return newValue;
    }

    value_type operator--(int) {
        auto oldValue(load());
        auto newValue(oldValue - 1);
        while (!compare_exchange_weak(oldValue, newValue)) {
            newValue = oldValue - 1;
        }
        return oldValue;
    }

private:
    inline static value_type asT(const RawType raw) {
        const void *tmp(&raw);
        return *static_cast<const value_type*>(tmp);
    }

    inline static value_type asTRef(RawType &raw) {
        void *tmp(&raw);
        return *static_cast<value_type*>(tmp);
    }

    inline static RawType asRaw(const value_type value) {
        const void *tmp(&value);
        return *static_cast<const RawType*>(tmp);
    }

    inline static RawType& asRawRef(value_type& value) {
        void *tmp(&value);
        return *static_cast<RawType*>(tmp);
    }

    std::atomic<RawType> repr_;
};

typedef AtomicReal<float> AtomicFloat;
typedef AtomicReal<double> AtomicDouble;

} // namespace utility

#endif // utility_atomic_hpp_included_
