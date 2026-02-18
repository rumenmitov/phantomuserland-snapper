#include <compat/shorttype-def.h>  // for u16 etc. ??

// From limits.h in old libc
#define CHAR_BIT 8

// From Genode's dde_linux header (kernel.h)
#define INT_MAX ((int)(~0U >> 1))
// #define UINT_MAX  (~0U)
// freetype needed 32 bit integer type. It looks like there was no options for it here
#define UINT_MAX  (0xffffffffu)
#define INT_MIN   (-INT_MAX - 1)
#define USHRT_MAX ((u16)(~0U))
#define LONG_MAX  ((long)(~0UL >> 1))
#define SHRT_MAX  ((s16)(USHRT_MAX >> 1))
#define SHRT_MIN  ((s16)(-SHRT_MAX - 1))
#define ULONG_MAX (~0UL)

#define SIZE_MAX (ULONG_MAX)

// from / for wamr libc thing

/* Minimum of signed integral types.  */
#define INT8_MIN  (-128)
#define INT16_MIN (-32767 - 1)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN (-__INT64_C(9223372036854775807) - 1)
/* Maximum of signed integral types.  */
#define INT8_MAX  (127)
#define INT16_MAX (32767)
#define INT32_MAX (2147483647)
#define INT64_MAX (__INT64_C(9223372036854775807))

/* Maximum of unsigned integral types.  */
#define UINT8_MAX  (255)
#define UINT16_MAX (65535)
#define UINT32_MAX (4294967295U)
#define UINT64_MAX (__UINT64_C(18446744073709551615))

#define CHAR_MAX  127
#define SCHAR_MAX 127
#define UCHAR_MAX 255

#define LLONG_MAX  INT64_MAX
#define ULLONG_MAX UINT64_MAX

#define UINTPTR_MAX UINT64_MAX
