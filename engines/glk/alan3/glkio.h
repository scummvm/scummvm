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
#include "glk/alan3/jumps.h"

namespace Glk {
namespace Alan3 {

class GlkIO : public GlkAPI {
private:
	winid_t glkMainWin;
	winid_t glkStatusWin;
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

	void print(const char *, ...);

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
		if (glk_gestalt(gestalt_Graphics, 0) == 1)
			glk_window_flow_break(glkMainWin);
	}
};

extern GlkIO *g_io;

#undef printf
#define printf g_io->print

} // End of namespace Alan3
} // End of namespace Glk

#endif
