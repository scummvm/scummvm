#include "ps2debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <sio.h>

void sioprintf(const char *zFormat, ...) {
	va_list ap;
	char resStr[2048];

	va_start(ap,zFormat);
	vsnprintf(resStr, 2048, zFormat, ap);
	va_end(ap);

	char *pos = resStr;
	while (*pos) {
		if (*pos == '\n') {
			// SIO terminal needs explicit CRLF
			sio_putc('\r');
			sio_putc('\n');
		} else
			sio_putc(*pos);
		pos++;
	}
}

