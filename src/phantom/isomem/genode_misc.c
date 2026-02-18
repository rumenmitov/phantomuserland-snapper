#include "genode_misc.h"
// #include "stdio.h"

#include <ph_io.h>

void _stub_print_func(const char *func, const char *file, int line)
{
	ph_printf("STUB: %s\n", func);
}
