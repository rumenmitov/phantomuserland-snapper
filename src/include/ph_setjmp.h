#ifndef PHANTOM_SETJMP
#define PHANTOM_SETJMP

#include <kernel/config.h>

#if USE_LIBC_SETJMP != 0

#include <libc/setjmp.h>

/*
	These function have to defined using macros since
	they work with execution context
*/

// usual setjmp
#define ph_setjmp(jmp_buf) setjmp(jmp_buf)
// usual longjmp
#define ph_longjmp(jmp_buf, int) longjmp(jmp_buf, int)


#else

#warning Using unimplemented setjmp

#define _JBLEN 1
typedef int jmp_buf[_JBLEN];

int ph_setjmp(jmp_buf);
void ph_longjmp(jmp_buf, int);
#endif  // USE_LIBC_SETJMP


#endif  // PHANTOM_SETJMP
