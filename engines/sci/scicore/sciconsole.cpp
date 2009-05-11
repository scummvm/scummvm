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

/* First part of the console implmentation: VM independent stuff */
/* Remember, it doesn't have to be fast. */

#include "sci/sci_memory.h"
#include "sci/engine/state.h"
#include "sci/scicore/sciconsole.h"

#include "sci/sci.h"	// For _console only
#include "sci/console.h"	// For _console only

namespace Sci {

#ifdef SCI_CONSOLE

static void (*_con_pixmap_callback)(gfx_pixmap_t *) = NULL;

bool g_redirect_sciprintf_to_gui = false;

int sciprintf(const char *fmt, ...) {
	va_list argp;

	assert(fmt);

	// First determine how big a buffer we need
	va_start(argp, fmt);
	int bufsize = vsnprintf(0, 0, fmt, argp);
	assert(bufsize >= 0);
	va_end(argp);

	// Allocate buffer for the full printed string
	char *buf = (char *)sci_malloc(bufsize + 1);
	assert(buf);

	// Print everything according to fmt into buf
	va_start(argp, fmt); // reset argp
	int bufsize2 = vsnprintf(buf, bufsize + 1, fmt, argp);
	assert(bufsize == bufsize2);
	va_end(argp);

	// Display the result suitably
	if (g_redirect_sciprintf_to_gui)
		((SciEngine *)g_engine)->_console->DebugPrintf("%s", buf);
	printf("%s", buf);

	free(buf);

	return 1;
}

void con_set_pixmap_callback(void(*callback)(gfx_pixmap_t *)) {
	_con_pixmap_callback = callback;
}

int con_can_handle_pixmaps() {
	return _con_pixmap_callback != NULL;
}

int con_insert_pixmap(gfx_pixmap_t *pixmap) {
	if (_con_pixmap_callback)
		_con_pixmap_callback(pixmap);
	else
		return 1;
	return 0;
}

#endif // SCI_CONSOLE

} // End of namespace Sci
