
#define DEBUG_MSG_PREFIX "vm.unixhal"
#include <debug_ext.h>
#define debug_level_flow  10
#define debug_level_error 10
#define debug_level_info  10

#include "event.h"

#include <stdarg.h>
#include <threads.h>

// pvm_memcheck
#include <vm/alloc.h>

void panic(const char *fmt, ...)
{
	va_list vl;

	// CI: this word is being watched by CI scripts. Do not change -- or change CI
	// appropriately
	ph_printf("\nPanic: ");
	va_start(vl, fmt);
	ph_vprintf(fmt, vl);
	va_end(vl);

	// save_mem(mem, size);
	//  getchar();
	//  CI: this word is being watched by CI scripts. Do not change -- or change CI
	//  appropriately
	ph_printf("\nPress Enter from memcheck...");
	pvm_memcheck();
	// ph_printf("\nPress Enter...");	getchar();
	//  exit(1);
	//  XXX : Add some real panic code here
	hal_sleep_msec(1000000);
}


void phantom_wakeup_after_msec(long msec)
{
	hal_sleep_msec(msec);
}

// time_t time(time_t *);

// //time_t fast_time(void)
// long fast_time(void)
// {
//     return time(0);
// }

extern int sleep(int);