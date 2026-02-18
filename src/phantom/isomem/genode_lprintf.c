#include <debug_ext.h>
#include <ph_io.h>
#include <stdarg.h>
// // #include <stdio.h>

void lprintf(char const *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	ph_vprintf(fmt, ap);
	va_end(ap);
}
