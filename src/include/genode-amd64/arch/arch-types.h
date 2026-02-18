#include <base/fixed_stdint.h>


typedef genode_int8_t int8_t;
typedef genode_uint8_t u_int8_t;
typedef genode_int16_t int16_t;
typedef genode_uint16_t u_int16_t;
typedef genode_int32_t int32_t;
typedef genode_uint32_t u_int32_t;
typedef genode_int64_t int64_t;
typedef genode_uint64_t u_int64_t;

typedef void *vmem_ptr_t;
typedef unsigned long addr_t;

// physical mem address
typedef u_int64_t physaddr_t;
// linear mem address
typedef u_int64_t linaddr_t;

// typedef	u_int32_t		uintptr_t;
typedef int64_t ptrdiff_t;
typedef u_int64_t register_t;


typedef unsigned intsize_t;
typedef long ssize_t;
typedef unsigned long size_t;

// XXX : Not sure shy we have these. Made them long
// typedef int			off_t;
// typedef int			_off_t;
typedef long off_t;
typedef long _off_t;

/* Integer types capable of holding object pointers */

typedef int64_t intptr_t;
typedef u_int64_t uintptr_t;
