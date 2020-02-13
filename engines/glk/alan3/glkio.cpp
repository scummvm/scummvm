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
#include "common/config-manager.h"

namespace Glk {
namespace Alan3 {

GlkIO *g_io;

GlkIO::GlkIO(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		glkMainWin(nullptr), glkStatusWin(nullptr), onStatusLine(false), _saveSlot(-1),
		_soundChannel(nullptr) {
	g_io = this;
}

bool GlkIO::initialize() {
	// first, open a window for error output
	glkMainWin = glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	if (glkMainWin == nullptr)
		return false;

	glk_stylehint_set(wintype_TextGrid, style_User1, stylehint_ReverseColor, 1);
	glkStatusWin = glk_window_open(glkMainWin, winmethod_Above |
		winmethod_Fixed, 1, wintype_TextGrid, 0);
	glk_set_window(glkMainWin);

	// Check for a save being loaded directly from the launcher
	_saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;

	return true;
}

void GlkIO::print(const char *fmt, ...) {
	// If there's a savegame being loaded from the launcher, ignore any text out
	if (_saveSlot != -1)
		return;

	va_list argp;
	va_start(argp, fmt);
	Common::String str = Common::String::vformat(fmt, argp);
	va_end(argp);

	if (glkMainWin) {
		glk_put_string(str.c_str());
	} else {
		// assume stdio is available in this case only
		warning("%s", str.c_str());
	}
}

void GlkIO::showImage(int image, int align) {
	uint ecode;
	if (_saveSlot != -1)
		return;

	if ((glk_gestalt(gestalt_Graphics, 0) == 1) &&
		(glk_gestalt(gestalt_DrawImage, wintype_TextBuffer) == 1)) {
		glk_window_flow_break(glkMainWin);
		printf("\n");
		ecode = glk_image_draw(glkMainWin, image, imagealign_MarginLeft, 0);
		(void)ecode;
	}
}

void GlkIO::playSound(int sound) {
	if (_saveSlot != -1)
		return;

#ifdef GLK_MODULE_SOUND
	if (glk_gestalt(gestalt_Sound, 0) == 1) {
		if (_soundChannel == nullptr)
			_soundChannel = glk_schannel_create(0);
		if (_soundChannel) {
			glk_schannel_stop(_soundChannel);
			(void)glk_schannel_play(_soundChannel, sound);
		}
	}
#endif
}

void GlkIO::setStyle(int style) {
	switch (style) {
	case NORMAL_STYLE:
		glk_set_style(style_Normal);
		break;
	case EMPHASIZED_STYLE:
		glk_set_style(style_Emphasized);
		break;
	case PREFORMATTED_STYLE:
		glk_set_style(style_Preformatted);
		break;
	case ALERT_STYLE:
		glk_set_style(style_Alert);
		break;
	case QUOTE_STYLE:
		glk_set_style(style_BlockQuote);
		break;
	default:
		break;
	}
}

void GlkIO::statusLine(CONTEXT) {
	uint glkWidth;
	char line[100];
	int pcol = col;

	if (!statusLineOption || _saveSlot != -1 || glkStatusWin == nullptr)
		return;

	glk_set_window(glkStatusWin);
	glk_window_clear(glkStatusWin);
	glk_window_get_size(glkStatusWin, &glkWidth, NULL);

	onStatusLine = TRUE;
	col = 1;
	glk_window_move_cursor(glkStatusWin, 1, 0);
	CALL1(sayInstance, where(HERO, /*TRUE*/ TRANSITIVE))

		// TODO Add status message1  & 2 as author customizable messages
		if (header->maximumScore > 0)
			sprintf(line, "Score %d(%d)/%d moves", current.score, (int)header->maximumScore, current.tick);
		else
			sprintf(line, "%d moves", current.tick);
	glk_window_move_cursor(glkStatusWin, glkWidth - strlen(line) - 1, 0);
	glk_put_string(line);
	needSpace = FALSE;

	col = pcol;
	onStatusLine = FALSE;

	glk_set_window(glkMainWin);
}


/*======================================================================

  readline()

  Read a line from the user, with history and editing

*/

/* TODO - length of user buffer should be used */
bool GlkIO::readLine(CONTEXT, char *buffer, size_t maxLen) {
	event_t event;
	static bool readingCommands = FALSE;
	static frefid_t commandFileRef;
	static strid_t commandFile;

	if (_saveSlot != -1) {
		// Return a "restore" command
		forcePrint("> ");
		forcePrint("restore\n");
		strcpy(buffer, "restore");

	} else if (readingCommands) {
		if (glk_get_line_stream(commandFile, buffer, maxLen) == 0) {
			glk_stream_close(commandFile, NULL);
			readingCommands = FALSE;
		} else {
			glk_set_style(style_Input);
			printf(buffer);
			glk_set_style(style_Normal);
		}

	} else {
		glk_request_line_event(glkMainWin, buffer, maxLen, 0);

		do {
			glk_select(&event);
			if (shouldQuit())
				LONG_JUMP0

			switch (event.type) {
			case evtype_Arrange:
				R0CALL0(g_io->statusLine)
				break;

			default:
				break;
			}
		} while (event.type != evtype_LineInput);
		if (buffer[0] == '@') {
			buffer[event.val1] = 0;
			commandFileRef = glk_fileref_create_by_name(fileusage_InputRecord + fileusage_TextMode, &buffer[1], 0);
			commandFile = glk_stream_open_file(commandFileRef, filemode_Read, 0);
			if (commandFile != NULL)
				if (glk_get_line_stream(commandFile, buffer, maxLen) != 0) {
					readingCommands = TRUE;
					glk_set_style(style_Input);
					printf(buffer);
					glk_set_style(style_Normal);
				}
		} else
			buffer[event.val1] = 0;
	}
	return TRUE;
}

Common::Error GlkIO::loadGame() {
	if (_saveSlot != -1) {
		int saveSlot = _saveSlot;
		_saveSlot = -1;
		return loadGameState(saveSlot);
	} else {
		return GlkAPI::loadGame();
	}
}

} // End of namespace Alan3
} // End of namespace Glk
