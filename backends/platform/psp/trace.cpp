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

#define TRACE_C
#include <pspkernel.h>
#include <pspdebug.h>
#include "backends/platform/psp/trace.h"


int psp_debug_indent = 0;

void PSPDebugTrace(bool alsoToScreen, const char *format, ...) {
	va_list	opt;
	char		buffer[2048];
	int			bufsz;
	FILE *fd = 0;

	va_start(opt, format);
	bufsz = vsnprintf(buffer, (size_t) sizeof(buffer), format, opt);
	va_end(opt);

	//fd = fopen("MS0:/SCUMMTRACE.TXT", "ab");
	fd = fopen("SCUMMTRACE.TXT", "ab");

	if (fd == 0)
		return;

	fwrite(buffer, 1, bufsz, fd);
	fclose(fd);

	if (alsoToScreen)
		fprintf(stderr, buffer);
}
