
#include <stdarg.h>

#include "engine/tinygl/zgl.h"

void gl_fatal_error(const char *format, ...) {
	va_list ap;

	va_start(ap, format);

	fprintf(stderr, "TinyGL: fatal error: ");
	vfprintf(stderr, format,ap);
	fprintf(stderr, "\n");
	exit(1);

	va_end(ap);
}
