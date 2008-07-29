
#include "engine/tinygl/zgl.h"

#ifndef TINYGL_DEBUG
//#define TINYGL_DEBUG
#endif

// Use this function to output messages when something unexpected
// happens (which might be an indication of an error). *Don't* use it
// when there's internal errors in the code - these should be handled
// by asserts.
#ifndef TINYGL_DEBUG
void tgl_warning(const char *, ...) { }
#else
void tgl_warning(const char *format, ...) {
	va_list args;
	va_start(args, format);
	fprintf(stderr, "*WARNING* ");
	vfprintf(stderr, format, args);
	va_end(args);
}
#endif

// This function should be used for debug output only.
#ifndef TINYGL_DEBUG
void tgl_trace(const char *, ...) { }
#else
void tgl_trace(const char *format, ...) {
	va_list args;
	va_start(args, format);
	fprintf(stderr, "*DEBUG* ");
	vfprintf(stderr, format, args);
	va_end(args);
}
#endif

// Use this function to output info about things in the code which
// should be fixed (missing handling of special cases, important
// features not implemented, known bugs/buglets, ...).
#ifndef TINYGL_DEBUG
void tgl_fixme(const char *, ...) { }
#else
void tgl_fixme(const char *format, ...) {
	va_list args;
	va_start(args, format);
	fprintf(stderr, "*FIXME* ");
	vfprintf(stderr, format, args);
	va_end(args);
}
#endif

