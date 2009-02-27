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

#include "sci/engine/kdebug.h"
#include "sci/engine/state.h"

namespace Sci {

int script_debug_flag = 0; // Defaulting to running mode
int sci_debug_flags = 0; // Special flags

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

} // End of namespace Sci
