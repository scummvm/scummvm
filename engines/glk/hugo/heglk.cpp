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

#include "glk/hugo/hugo.h"

namespace Glk {
namespace Hugo {

void Hugo::hugo_init_screen() {
	// Open the main window...
	mainwin = currentwin = glk_window_open(0, 0, 0, wintype_TextBuffer, 1);
	assert(mainwin);

	// ...and set it up for default output
	glk_set_window(mainwin);

	// By setting the width and height so high, we're basically forcing the Glk library
	// to deal with text-wrapping and page ends
	SCREENWIDTH = 0x7fff;
	SCREENHEIGHT = 0x7fff;
	FIXEDCHARWIDTH = 1;
	FIXEDLINEHEIGHT = 1;

	hugo_settextwindow(1, 1,
		SCREENWIDTH/FIXEDCHARWIDTH, SCREENHEIGHT/FIXEDLINEHEIGHT);
}

void Hugo::hugo_cleanup_screen() {
}

void Hugo::hugo_settextwindow(int left, int top, int right, int bottom) {
	// TODO
}

} // End of namespace Hugo
} // End of namespace Glk
