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

#include "glk/alan3/readline.h"

#include "glk/alan3/sysdep.h"
#include "glk/alan3/output.h"
#include "glk/alan3/term.h"
#include "glk/alan3/exe.h"
#include "glk/alan3/save.h"
#include "glk/alan3/location.h"

#include "glk/alan3/options.h"
#include "glk/alan3/alan3.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/resources.h"

namespace Glk {
namespace Alan3 {

#define LINELENGTH 1000

/*======================================================================

  readline()

  Read a line from the user, with history and editing

*/

/* TODO - length of user buffer should be used */
bool readline(char buffer[]) {
	event_t event;
	static bool readingCommands = FALSE;
	static frefid_t commandFileRef;
	static strid_t commandFile;

	if (readingCommands) {
		if (g_vm->glk_get_line_stream(commandFile, buffer, 255) == 0) {
			g_vm->glk_stream_close(commandFile, NULL);
			readingCommands = FALSE;
		} else {
			g_vm->glk_set_style(style_Input);
			printf(buffer);
			g_vm->glk_set_style(style_Normal);
		}
	} else {
		g_vm->glk_request_line_event(glkMainWin, buffer, 255, 0);
		/* FIXME: buffer size should be infallible: all existing calls use 256 or
		   80 character buffers, except parse which uses LISTLEN (currently 100)
		*/
		do {
			g_vm->glk_select(&event);
			switch (event.type) {
			case evtype_Arrange:
				statusline();
				break;

			default:
				break;
			}
		} while (event.type != evtype_LineInput);
		if (buffer[0] == '@') {
			buffer[event.val1] = 0;
			commandFileRef = g_vm->glk_fileref_create_by_name(fileusage_InputRecord + fileusage_TextMode, &buffer[1], 0);
			commandFile = g_vm->glk_stream_open_file(commandFileRef, filemode_Read, 0);
			if (commandFile != NULL)
				if (g_vm->glk_get_line_stream(commandFile, buffer, 255) != 0) {
					readingCommands = TRUE;
					g_vm->glk_set_style(style_Input);
					printf(buffer);
					g_vm->glk_set_style(style_Normal);
				}
		} else
			buffer[event.val1] = 0;
	}
	return TRUE;
}

} // End of namespace Alan3
} // End of namespace Glk
