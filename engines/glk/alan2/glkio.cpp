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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/glk.h"
#include "glk/alan2/alan2.h"
#include "glk/alan2/glkio.h"
#include "glk/alan2/main.h"

namespace Glk {
namespace Alan2 {

winid_t glkMainWin;
winid_t glkStatusWin;

void glkio_printf(const char *fmt, ...) {
	// If there's a savegame being loaded from the launcher, ignore any text out
	if (g_vm->_saveSlot != -1)
		return;

	va_list argp;
	va_start(argp, fmt);
	if (glkMainWin) {
		char buf[1024]; /* FIXME: buf size should be foolproof */
		vsprintf(buf, fmt, argp);
		g_vm->glk_put_string(buf);
	} else {
		// assume stdio is available in this case only
		Common::String str = Common::String::vformat(fmt, argp);
		warning(fmt, argp);
	}

	va_end(argp);
}

/*======================================================================

  readline()

  Read a line from the user, with history and editing

  */

  /* 4f - length of user buffer should be used */
Boolean readline(char usrbuf[]) {
	if (g_vm->_pendingLook) {
		g_vm->_pendingLook = false;
		glkio_printf("look\n");
		strcpy(usrbuf, "look");
	} else {
		event_t event;
		g_vm->glk_request_line_event(glkMainWin, usrbuf, 255, 0);

		/* FIXME: buffer size should be infallible: all existing calls use 256 or
		   80 character buffers, except parse which uses LISTLEN (currently 100)
		 */
		do {
			g_vm->glk_select(&event);
			if (evtype_Arrange == event.type)
				statusline();
			if (g_vm->shouldQuit())
				return false;

		} while (event.type != evtype_LineInput);

		usrbuf[event.val1] = 0;
	}

	return TRUE;
}


} // End of namespace Alan2
} // End of namespace Glk

