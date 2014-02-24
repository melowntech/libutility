#ifndef SHARED_UTILITY_GCCVERSION_HPP_INCLUDED_
#define SHARED_UTILITY_GCCVERSION_HPP_INCLUDED_

#ifdef __GNUC__
#    define GCC_VERSION \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#    define GCC_VERSION 0
#endif

#if GCC_VERSION && (GCC_VERSION < 40700)
#    define UTILITY_HAS_NO_STD_FUTURE_STATUS
#endif

// pre-4.7 gcc has no override/final modifier
#if GCC_VERSION && (GCC_VERSION < 40700)
#    define UTILITY_OVERRIDE
#    define UTILITY_FINAL
#else
#    define UTILITY_OVERRIDE override
#    define UTILITY_FINAL final
#endif

#ifdef __GNUC__
#define UTILITY_FUNCTION_ERROR(message) \
    __attribute__((error(message)))
#else
#define UTILITY_FUNCTION_ERROR(message) \
    ; static_assert(false, "error message allowed only in GCC")
#endif

#endif // SHARED_UTILITY_GCCVERSION_HPP_INCLUDED_
