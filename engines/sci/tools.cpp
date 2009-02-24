/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/scummsys.h"

#ifdef WIN32
#  include <windows.h>
#  include <errno.h>
#  include <mmsystem.h>
#  include <sys/timeb.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <direct.h>
#  undef ARRAYSIZE
#endif

#include "common/util.h"
#include "common/str.h"

#include "sci/include/engine.h"
#include "sci/engine/kernel.h"

namespace Sci {

#ifndef _MSC_VER
#  include <sys/time.h>
#endif

int script_debug_flag = 0; // Defaulting to running mode
int sci_debug_flags = 0; // Special flags

#ifndef con_file
#	define con_file 0
#endif

int sci_ffs(int _mask) {
	int retval = 0;

	if (!_mask)
		return 0;
	retval++;
	while (!(_mask & 1)) {
		retval++;
		_mask >>= 1;
	}

	return retval;
}

//******************* Debug functions *******************

// Functions for internal macro use
void _SCIkvprintf(FILE *file, const char *format, va_list args);

void _SCIkvprintf(FILE *file, const char *format, va_list args) {
	vfprintf(file, format, args);
	if (con_file) vfprintf(con_file, format, args);
}


void _SCIkwarn(EngineState *s, const char *file, int line, int area, const char *format, ...) {
	va_list args;

	if (area == SCIkERROR_NR)
		fprintf(stderr, "ERROR: ");
	else
		fprintf(stderr, "Warning: ");

	va_start(args, format);
	_SCIkvprintf(stderr, format, args);
	va_end(args);
	fflush(NULL);

	if (sci_debug_flags & _DEBUG_FLAG_BREAK_ON_WARNINGS) script_debug_flag = 1;
}

void _SCIkdebug(EngineState *s, const char *file, int line, int area, const char *format, ...) {
	va_list args;

	if (s->debug_mode & (1 << area)) {
		fprintf(stdout, " kernel: (%s L%d): ", file, line);
		va_start(args, format);
		_SCIkvprintf(stdout, format, args);
		va_end(args);
		fflush(NULL);
	}
}

void _SCIGNUkdebug(const char *funcname, EngineState *s, const char *file, int line, int area, const char *format, ...) {
	va_list xargs;
	int error = ((area == SCIkWARNING_NR) || (area == SCIkERROR_NR));

	if (error || (s->debug_mode & (1 << area))) { // Is debugging enabled for this area?

		fprintf(stderr, "FSCI: ");

		if (area == SCIkERROR_NR)
			fprintf(stderr, "ERROR in %s ", funcname);
		else if (area == SCIkWARNING_NR)
			fprintf(stderr, "%s: Warning ", funcname);
		else
			fprintf(stderr, funcname);

		fprintf(stderr, "(%s L%d): ", file, line);

		va_start(xargs, format);
		_SCIkvprintf(stderr, format, xargs);
		va_end(xargs);

	}
}


#ifndef _MSC_VER
void sci_gettime(long *seconds, long *useconds) {
	struct timeval tv;

	assert(!gettimeofday(&tv, NULL));
	*seconds = tv.tv_sec;
	*useconds = tv.tv_usec;
}
#elif defined (WIN32)

/*WARNING(Incorrect)*/
/* Warning: This function only retrieves the amount of mseconds since the start of
** the Win32 kernel; it does /not/ provide the number of seconds since the epoch!
** There are no known cases where this causes problems, though.  */
void sci_gettime(long *seconds, long *useconds) {
	DWORD tm;

	if (TIMERR_NOERROR != timeBeginPeriod(1)) {
		fprintf(stderr, "timeBeginPeriod(1) failed in sci_gettime\n");
	}

	tm = timeGetTime();

	if (TIMERR_NOERROR != timeEndPeriod(1)) {
		fprintf(stderr, "timeEndPeriod(1) failed in sci_gettime\n");
	}

	*seconds = tm / 1000;
	*useconds = (tm % 1000) * 1000;
}
#else
#  error "You need to provide a microsecond resolution sci_gettime implementation for your platform!"
#endif


void sci_get_current_time(GTimeVal *val) {
	long foo, bar;
	sci_gettime(&foo, &bar);
	val->tv_sec = foo;
	val->tv_usec = bar;
}

} // End of namespace Sci
