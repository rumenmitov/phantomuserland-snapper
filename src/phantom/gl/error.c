#include "zgl.h"

#include <ph_os.h>
#include <stdarg.h>

void gl_fatal_error(char *format, ...)  //__attribute__((noreturn))
{
	va_list ap;

	va_start(ap, format);

	// fprintf(stderr,"TinyGL: fatal error: ");
	// vfprintf(stderr,format,ap);
	// fprintf(stderr,"\n");

	ph_vprintf(format, ap);

	exit(1);

	va_end(ap);
}
