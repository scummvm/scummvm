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

#include "glk/alan3/current.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/instance.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/output.h"
#include "glk/alan3/options.h"
#include "glk/alan3/sysdep.h"

namespace Glk {
namespace Alan3 {

/* PUBLIC DATA */
bool onStatusLine = FALSE; /* To know if where printing the status line or not */

/*======================================================================

  getPageSize()

  Try to get the current page size from the system, else use the ones
  from the header.

 */
void getPageSize(void) {
	pageLength = 0;
	pageWidth = 0;
}

/*======================================================================*/
void statusline(void) {
	uint32 glkWidth;
	char line[100];
	int pcol = col;

	if (!statusLineOption) return;
	if (glkStatusWin == NULL)
		return;

	g_vm->glk_set_window(glkStatusWin);
	g_vm->glk_window_clear(glkStatusWin);
	g_vm->glk_window_get_size(glkStatusWin, &glkWidth, NULL);

	onStatusLine = TRUE;
	col = 1;
	g_vm->glk_window_move_cursor(glkStatusWin, 1, 0);
	sayInstance(where(HERO, /*TRUE*/ TRANSITIVE));

	// TODO Add status message1  & 2 as author customizable messages
	if (header->maximumScore > 0)
		sprintf(line, "Score %d(%d)/%d moves", current.score, (int)header->maximumScore, current.tick);
	else
		sprintf(line, "%d moves", current.tick);
	g_vm->glk_window_move_cursor(glkStatusWin, glkWidth - strlen(line) - 1, 0);
	g_vm->glk_put_string(line);
	needSpace = FALSE;

	col = pcol;
	onStatusLine = FALSE;

	g_vm->glk_set_window(glkMainWin);
}

} // End of namespace Alan3
} // End of namespace Glk
