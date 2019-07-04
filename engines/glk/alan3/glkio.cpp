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

#include "glk/alan3/glkio.h"
#include "glk/alan3/acode.h"
#include "glk/alan3/current.h"
#include "glk/alan3/instance.h"
#include "glk/alan3/options.h"
#include "glk/alan3/output.h"

namespace Glk {
namespace Alan3 {

winid_t glkMainWin;
winid_t glkStatusWin;
bool onStatusLine;

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

void showImage(int image, int align) {
	uint ecode;

	if ((g_vm->glk_gestalt(gestalt_Graphics, 0) == 1) &&
		(g_vm->glk_gestalt(gestalt_DrawImage, wintype_TextBuffer) == 1)) {
		g_vm->glk_window_flow_break(glkMainWin);
		printf("\n");
		ecode = g_vm->glk_image_draw(glkMainWin, image, imagealign_MarginLeft, 0);
		(void)ecode;
	}
}

void playSound(int sound) {
#ifdef GLK_MODULE_SOUND
	static schanid_t soundChannel = NULL;

	if (g_vm->glk_gestalt(gestalt_Sound, 0) == 1) {
		if (soundChannel == NULL)
			soundChannel = g_vm->glk_schannel_create(0);
		if (soundChannel != NULL) {
			g_vm->glk_schannel_stop(soundChannel);
			(void)g_vm->glk_schannel_play(soundChannel, sound);
		}
	}
#endif
}

void setStyle(int style) {
	switch (style) {
	case NORMAL_STYLE:
		g_vm->glk_set_style(style_Normal);
		break;
	case EMPHASIZED_STYLE:
		g_vm->glk_set_style(style_Emphasized);
		break;
	case PREFORMATTED_STYLE:
		g_vm->glk_set_style(style_Preformatted);
		break;
	case ALERT_STYLE:
		g_vm->glk_set_style(style_Alert);
		break;
	case QUOTE_STYLE:
		g_vm->glk_set_style(style_BlockQuote);
		break;
	}
}

void statusline(CONTEXT) {
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
	CALL1(sayInstance, where(HERO, /*TRUE*/ TRANSITIVE))

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


/*======================================================================

  readline()

  Read a line from the user, with history and editing

*/

/* TODO - length of user buffer should be used */
bool readline(CONTEXT, char *buffer, size_t maxLen) {
	event_t event;
	static bool readingCommands = FALSE;
	static frefid_t commandFileRef;
	static strid_t commandFile;

	if (readingCommands) {
		if (g_vm->glk_get_line_stream(commandFile, buffer, maxLen) == 0) {
			g_vm->glk_stream_close(commandFile, NULL);
			readingCommands = FALSE;
		} else {
			g_vm->glk_set_style(style_Input);
			printf(buffer);
			g_vm->glk_set_style(style_Normal);
		}
	} else {
		g_vm->glk_request_line_event(glkMainWin, buffer, maxLen, 0);

		do {
			g_vm->glk_select(&event);
			if (g_vm->shouldQuit())
				LONG_JUMP0

			switch (event.type) {
			case evtype_Arrange:
				R0CALL0(statusline)
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
				if (g_vm->glk_get_line_stream(commandFile, buffer, maxLen) != 0) {
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
