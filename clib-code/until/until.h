#ifndef _UNTIL_H_
#define _UNTIL_H_

#include <stddef.h>

#ifndef WD_MIN
#define WD_MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef WD_MAX
#define WD_MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef WD_CONSTRAIN
#define WD_CONSTRAIN(x, a, b) WD_MAX(a, WD_MIN(b, x))
#endif

#ifndef WD_LINEAR_MAP
#define WD_LINEAR_MAP(x, x1, x2, y1, y2) WD_CONSTRAIN(((y1) + ((x) - (x1)) * ((y2) - (y1)) / ((x2) - (x1))), y1, y2)
#endif

#ifndef WD_BIT
#define WD_BIT(n) (1U << (n))
#endif

#ifndef WD_BIT_SET
#define WD_BIT_SET(x, n) ((x) |= WD_BIT(n))
#endif

#ifndef WD_BIT_CLEAR
#define WD_BIT_CLEAR(x, n) ((x) &= ~WD_BIT(n))
#endif

#ifndef WD_BIT_TOGGLE
#define WD_BIT_TOGGLE(x, n) ((x) ^= WD_BIT(n))
#endif

#ifndef WD_BIT_READ
#define WD_BIT_READ(x, n) (((x) >> (n)) & 1U)
#endif

#ifndef WD_REG_SET
#define WD_REG_SET(x, cmark, smark) ((x) = ((x) & ~(cmark)) | (smark))
#endif

#ifndef WD_OFFSETOF
#define WD_OFFSETOF(type, member) offsetof(type, member)
#endif

#ifndef WD_CONTAINER_OF
#define WD_CONTAINER_OF(ptr, type, member) ((type *)((char *)(ptr) - WD_OFFSETOF(type, member)))
#endif

#ifndef WD_STRINGIFY_IMPL
#define WD_STRINGIFY_IMPL(x) #x
#endif

#ifndef WD_STRINGIFY
#define WD_STRINGIFY(x) WD_STRINGIFY_IMPL(x)
#endif

#ifndef WD_CONCAT_IMPL
#define WD_CONCAT_IMPL(a, b) a##b
#endif

#ifndef WD_CONCAT
#define WD_CONCAT(a, b) WD_CONCAT_IMPL(a, b)
#endif

#ifndef WD_ASSERT
#define WD_ASSERT(expr)                                                                                        \
    do                                                                                                          \
    {                                                                                                           \
        if (!(expr))                                                                                            \
        {                                                                                                       \
            while (1)                                                                                           \
            {                                                                                                   \
            }                                                                                                   \
        }                                                                                                       \
    } while (0)
#endif

#if defined(__GNUC__) || defined(__clang__)
#ifndef WD_WEAK
#define WD_WEAK         __attribute__((weak))
#endif
#ifndef WD_UNUSED
#define WD_UNUSED       __attribute__((unused))
#endif
#ifndef WD_PACKED
#define WD_PACKED       __attribute__((packed))
#endif
#ifndef WD_INLINE
#define WD_INLINE       static inline __attribute__((always_inline))
#endif
#ifndef WD_NORETURN
#define WD_NORETURN     __attribute__((noreturn))
#endif
#ifndef WD_DEPRECATED
#define WD_DEPRECATED   __attribute__((deprecated))
#endif
#ifndef WD_ALIGNED
#define WD_ALIGNED(n)   __attribute__((aligned(n)))
#endif
#ifndef WD_SECTION
#define WD_SECTION(s)   __attribute__((section(s)))
#endif
#ifndef WD_LIKELY
#define WD_LIKELY(x)    __builtin_expect(!!(x), 1)
#endif
#ifndef WD_UNLIKELY
#define WD_UNLIKELY(x)  __builtin_expect(!!(x), 0)
#endif
#elif defined(__CC_ARM)
#ifndef WD_WEAK
#define WD_WEAK         __attribute__((weak))
#endif
#ifndef WD_UNUSED
#define WD_UNUSED       __attribute__((unused))
#endif
#ifndef WD_PACKED
#define WD_PACKED       __attribute__((packed))
#endif
#ifndef WD_INLINE
#define WD_INLINE       static __forceinline
#endif
#ifndef WD_NORETURN
#define WD_NORETURN     __attribute__((noreturn))
#endif
#ifndef WD_DEPRECATED
#define WD_DEPRECATED   __attribute__((deprecated))
#endif
#ifndef WD_ALIGNED
#define WD_ALIGNED(n)   __attribute__((aligned(n)))
#endif
#ifndef WD_SECTION
#define WD_SECTION(s)   __attribute__((section(s)))
#endif
#ifndef WD_LIKELY
#define WD_LIKELY(x)    (x)
#endif
#ifndef WD_UNLIKELY
#define WD_UNLIKELY(x)  (x)
#endif
#elif defined(__ICCARM__)
#ifndef WD_WEAK
#define WD_WEAK         __weak
#endif
#ifndef WD_UNUSED
#define WD_UNUSED
#endif
#ifndef WD_PACKED
#define WD_PACKED       __packed
#endif
#ifndef WD_INLINE
#define WD_INLINE       static inline
#endif
#ifndef WD_NORETURN
#define WD_NORETURN     __noreturn
#endif
#ifndef WD_DEPRECATED
#define WD_DEPRECATED
#endif
#ifndef WD_ALIGNED
#define WD_ALIGNED(n)   _Pragma(WD_STRINGIFY(data_alignment=n))
#endif
#ifndef WD_SECTION
#define WD_SECTION(s)
#endif
#ifndef WD_LIKELY
#define WD_LIKELY(x)    (x)
#endif
#ifndef WD_UNLIKELY
#define WD_UNLIKELY(x)  (x)
#endif
#elif defined(_MSC_VER)
#ifndef WD_WEAK
#define WD_WEAK         __declspec(selectany)
#endif
#ifndef WD_UNUSED
#define WD_UNUSED
#endif
#ifndef WD_PACKED
#define WD_PACKED
#endif
#ifndef WD_INLINE
#define WD_INLINE       __forceinline
#endif
#ifndef WD_NORETURN
#define WD_NORETURN     __declspec(noreturn)
#endif
#ifndef WD_DEPRECATED
#define WD_DEPRECATED   __declspec(deprecated)
#endif
#ifndef WD_ALIGNED
#define WD_ALIGNED(n)   __declspec(align(n))
#endif
#ifndef WD_SECTION
#define WD_SECTION(s)   __declspec(allocate(s))
#endif
#ifndef WD_LIKELY
#define WD_LIKELY(x)    (x)
#endif
#ifndef WD_UNLIKELY
#define WD_UNLIKELY(x)  (x)
#endif
#else
#ifndef WD_WEAK
#define WD_WEAK
#endif
#ifndef WD_UNUSED
#define WD_UNUSED
#endif
#ifndef WD_PACKED
#define WD_PACKED
#endif
#ifndef WD_INLINE
#define WD_INLINE       static inline
#endif
#ifndef WD_NORETURN
#define WD_NORETURN
#endif
#ifndef WD_DEPRECATED
#define WD_DEPRECATED
#endif
#ifndef WD_ALIGNED
#define WD_ALIGNED(n)
#endif
#ifndef WD_SECTION
#define WD_SECTION(s)
#endif
#ifndef WD_LIKELY
#define WD_LIKELY(x)    (x)
#endif
#ifndef WD_UNLIKELY
#define WD_UNLIKELY(x)  (x)
#endif
#endif
#endif
