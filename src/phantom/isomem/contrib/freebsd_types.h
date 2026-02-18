// XXX : Use with care. This header should not be included in Phantom headers!
// It is used only in several files in phantom/isomem/contrib requiring missing types

// #include <stdint.h>
#include <phantom_types.h>

typedef u_int64_t uintmax_t;
typedef int64_t intmax_t;
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

typedef u_int64_t u_quad_t; /* quads (deprecated) */
typedef int64_t quad_t;