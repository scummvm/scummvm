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

#include "glk/tads/tads.h"
#include "common/config-manager.h"
#include "common/translation.h"

namespace Glk {
namespace TADS {

TADS::TADS(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc) {
	/*
	 * GLK Initialization
	 */

	// Open the story window
	story_win = glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	if (!story_win)
		error("fatal: could not open window!\n");

	// get default colors for main window
	if (!glk_style_measure(story_win, style_Normal, stylehint_TextColor, &mainfg))
		mainfg = 0;

	if (!glk_style_measure(story_win, style_Normal, stylehint_BackColor, &mainbg))
		mainbg = 0;

	// get default colors for status window
	status_win = glk_window_open(story_win, winmethod_Above | winmethod_Fixed, 1,
		wintype_TextGrid, 0);

	if (!glk_style_measure(status_win, style_Normal, stylehint_TextColor, &statusfg))
		statusfg = 0;

	if (!glk_style_measure(status_win, style_Normal, stylehint_BackColor, &statusbg))
		statusbg = 0;

	// close status window; reopened on request
	glk_window_close(status_win, 0);
	status_win = nullptr;

	glk_set_window(story_win);
}

Common::Error TADS::loadGameData(strid_t file) {
	// TODO
	return Common::kNoError;
}

Common::Error TADS::saveGameData(strid_t file, const Common::String &desc) {
	// TODO
	return Common::kNoError;
}

} // End of namespace TADS
} // End of namespace Glk
