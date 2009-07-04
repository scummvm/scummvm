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

#include "sci/tools.h"
#include "sci/engine/state.h"

#include "sci/sci.h"	// For _console only
#include "sci/console.h"	// For _console only

namespace Sci {

int sci_ffs(int bits) {
	if (!bits)
		return 0;

	int retval = 1;

	while (!(bits & 1)) {
		retval++;
		bits >>= 1;
	}

	return retval;
}

bool g_redirect_sciprintf_to_gui = false;

void sciprintf(const char *fmt, ...) {
	va_list argp;

	assert(fmt);

	// First determine how big a buffer we need
	va_start(argp, fmt);
	int bufsize = vsnprintf(0, 0, fmt, argp);
	assert(bufsize >= 0);
	va_end(argp);

	// Allocate buffer for the full printed string
	char *buf = (char *)malloc(bufsize + 1);
	assert(buf);

	// Print everything according to fmt into buf
	va_start(argp, fmt); // reset argp
	int bufsize2 = vsnprintf(buf, bufsize + 1, fmt, argp);
	assert(bufsize == bufsize2);
	va_end(argp);

	// Display the result suitably
	if (g_redirect_sciprintf_to_gui)
		((SciEngine *)g_engine)->getSciDebugger()->DebugPrintf("%s", buf);
	printf("%s", buf);

	free(buf);
}

} // End of namespace Sci
