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

#include "glk/alan2/alan2.h"
#include "glk/alan2/types.h"
#include "glk/alan2/glkio.h"
#include "glk/alan2/main.h"
#include "glk/alan2/readline.h"

namespace Glk {
namespace Alan2 {

/*======================================================================

  readline()

  Read a line from the user, with history and editing

  */

/* 4f - length of user buffer should be used */
Boolean readline(char usrbuf[]) {
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
	return TRUE;
}

} // End of namespace Alan2
} // End of namespace Glk

