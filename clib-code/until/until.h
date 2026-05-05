#ifndef _UNTIL_H_
#define _UNTIL_H_

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef constrain
#define constrain(x, a, b) max(a, min(b, x))
#endif

#ifndef linear_map
#define linear_map(x, x1, x2, y1, y2) constrain(((y1) + ((x) - (x1)) * ((y2) - (y1)) / ((x2) - (x1))), y1, y2)
#endif

#ifndef BIT
#define BIT(n) (1U << (n))
#endif

#ifndef SET_BIT
#define SET_BIT(x, n) ((x) |= BIT(n))
#endif

#ifndef CLEAR_BIT
#define CLEAR_BIT(x, n) ((x) &= ~BIT(n))
#endif

#ifndef TOGGLE_BIT
#define TOGGLE_BIT(x, n) ((x) ^= BIT(n))
#endif

#ifndef READ_BIT
#define READ_BIT(x, n) (((x) >> (n)) & 1U)
#endif

#ifndef REG_SET
#define REG_SET(x, cmark, smark) ((x) = ((x) & ~(cmark)) | (smark))
#endif

#ifndef OFFSETOF
#define OFFSETOF(type, member) ((unsigned long)&(((type *)0)->member))
#endif

#ifndef CONTAINER_OF
#define CONTAINER_OF(ptr, type, member) ((type *)((char *)(ptr) - OFFSETOF(type, member)))
#endif

#ifndef _STRINGIFY
#define _STRINGIFY(x) #x
#endif

#ifndef STRINGIFY
#define STRINGIFY(x) _STRINGIFY(x)
#endif

#ifndef _CONCAT
#define _CONCAT(a, b) a##b
#endif

#ifndef CONCAT
#define CONCAT(a, b) _CONCAT(a, b)
#endif

#ifndef ASSERT
#define ASSERT(expr)                                                                                           \
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
#ifndef WEAK
#define WEAK         __attribute__((weak))
#endif
#ifndef UNUSED
#define UNUSED       __attribute__((unused))
#endif
#ifndef PACKED
#define PACKED       __attribute__((packed))
#endif
#ifndef INLINE
#define INLINE       static inline __attribute__((always_inline))
#endif
#ifndef NORETURN
#define NORETURN     __attribute__((noreturn))
#endif
#ifndef DEPRECATED
#define DEPRECATED   __attribute__((deprecated))
#endif
#ifndef ALIGNED
#define ALIGNED(n)   __attribute__((aligned(n)))
#endif
#ifndef SECTION
#define SECTION(s)   __attribute__((section(s)))
#endif
#ifndef LIKELY
#define LIKELY(x)    __builtin_expect(!!(x), 1)
#endif
#ifndef UNLIKELY
#define UNLIKELY(x)  __builtin_expect(!!(x), 0)
#endif
#elif defined(__CC_ARM)
#ifndef WEAK
#define WEAK         __attribute__((weak))
#endif
#ifndef UNUSED
#define UNUSED       __attribute__((unused))
#endif
#ifndef PACKED
#define PACKED       __attribute__((packed))
#endif
#ifndef INLINE
#define INLINE       static __forceinline
#endif
#ifndef NORETURN
#define NORETURN     __attribute__((noreturn))
#endif
#ifndef DEPRECATED
#define DEPRECATED   __attribute__((deprecated))
#endif
#ifndef ALIGNED
#define ALIGNED(n)   __attribute__((aligned(n)))
#endif
#ifndef SECTION
#define SECTION(s)   __attribute__((section(s)))
#endif
#ifndef LIKELY
#define LIKELY(x)    (x)
#endif
#ifndef UNLIKELY
#define UNLIKELY(x)  (x)
#endif
#elif defined(__ICCARM__)
#ifndef WEAK
#define WEAK         __weak
#endif
#ifndef UNUSED
#define UNUSED
#endif
#ifndef PACKED
#define PACKED       __packed
#endif
#ifndef INLINE
#define INLINE       static inline
#endif
#ifndef NORETURN
#define NORETURN     __noreturn
#endif
#ifndef DEPRECATED
#define DEPRECATED
#endif
#ifndef ALIGNED
#define ALIGNED(n)   _Pragma(STRINGIFY(data_alignment=n))
#endif
#ifndef SECTION
#define SECTION(s)
#endif
#ifndef LIKELY
#define LIKELY(x)    (x)
#endif
#ifndef UNLIKELY
#define UNLIKELY(x)  (x)
#endif
#elif defined(_MSC_VER)
#ifndef WEAK
#define WEAK         __declspec(selectany)
#endif
#ifndef UNUSED
#define UNUSED
#endif
#ifndef PACKED
#define PACKED
#endif
#ifndef INLINE
#define INLINE       __forceinline
#endif
#ifndef NORETURN
#define NORETURN     __declspec(noreturn)
#endif
#ifndef DEPRECATED
#define DEPRECATED   __declspec(deprecated)
#endif
#ifndef ALIGNED
#define ALIGNED(n)   __declspec(align(n))
#endif
#ifndef SECTION
#define SECTION(s)   __declspec(allocate(s))
#endif
#ifndef LIKELY
#define LIKELY(x)    (x)
#endif
#ifndef UNLIKELY
#define UNLIKELY(x)  (x)
#endif
#else
#ifndef WEAK
#define WEAK
#endif
#ifndef UNUSED
#define UNUSED
#endif
#ifndef PACKED
#define PACKED
#endif
#ifndef INLINE
#define INLINE       static inline
#endif
#ifndef NORETURN
#define NORETURN
#endif
#ifndef DEPRECATED
#define DEPRECATED
#endif
#ifndef ALIGNED
#define ALIGNED(n)
#endif
#ifndef SECTION
#define SECTION(s)
#endif
#ifndef LIKELY
#define LIKELY(x)    (x)
#endif
#ifndef UNLIKELY
#define UNLIKELY(x)  (x)
#endif
#endif
#endif
