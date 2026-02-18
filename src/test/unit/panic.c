
#include <stdarg.h>


void panic(const char *fmt, ...)
{
	va_list ap;

	// CI: this word is being watched by CI scripts. Do not change -- or change CI
	// appropriately
	ph_printf("Panic: ");

	va_start(ap, fmt);
	ph_vprintf(fmt, ap);
	va_end(ap);

	exit(33);
}
