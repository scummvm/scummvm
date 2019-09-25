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

#ifndef GLK_ALAN3_GLKIO
#define GLK_ALAN3_GLKIO

#include "glk/glk_api.h"
#include "glk/windows.h"
#include "glk/jumps.h"

namespace Glk {
namespace Alan3 {

class GlkIO : public GlkAPI {
private:
	winid_t glkMainWin;
	winid_t glkStatusWin;
	schanid_t _soundChannel;
	int _saveSlot;
public:
	bool onStatusLine;
protected:
	/**
	 * Does initialization
	 */
	bool initialize();
public:
	/**
	 * Constructor
	 */
	GlkIO(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Print a string to the window
	 */
	void print(const char *fmt, ...);

	/**
	 * Outputs a string to the window, even during startup
	 */
	void forcePrint(const char *str) {
		glk_put_string(str);
	}

	void showImage(int image, int align);

	void playSound(int sound);

	void setStyle(int style);

	void statusLine(CONTEXT);

	bool readLine(CONTEXT, char *usrBuf, size_t maxLen);

	void clear() {
		glk_window_clear(glkMainWin);
	}

	void flowBreak() {
		/* Make a new paragraph, i.e one empty line (one or two newlines). */
		if (_saveSlot == -1 && glk_gestalt(gestalt_Graphics, 0) == 1)
			glk_window_flow_break(glkMainWin);
	}

	/**
	 * If a savegame was selected to be loaded from the launcher, then load it.
	 * Otherwise, prompt the user for a savegame to load, and then load it
	 */
	Common::Error loadGame();
};

extern GlkIO *g_io;

#undef printf
#define printf g_io->print

} // End of namespace Alan3
} // End of namespace Glk

#endif
