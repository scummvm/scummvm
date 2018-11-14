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

#ifndef GLK_DRAW_H
#define GLK_DRAW_H

#include "graphics/screen.h"
#include "glk/fonts.h"

namespace Glk {

enum CaretShape {
	SMALL_DOT = 0, FAT_DOT = 1, THIN_LINE = 2, FAT_LINE = 3, BLOCK = 4
};

/**
 * Screen surface class
 */
class Screen : public Graphics::Screen, public Fonts {
public:
	/**
	 * Constructor
	 */
	Screen() : Graphics::Screen(), Fonts(this) {}

	/**
	 * Fills the screen with a given rgb color
	 */
	void fill(const byte *rgb);

	/**
	 * Fill a given area of the screen with an rgb color
	 */
	void fillRect(const Rect &box, const byte *rgb);

	/**
	 * Draws the text input caret at the given position
	 * @remarks     The position specifies the caret's bottom-left corner,
	 *      and the X position is in multiples of GLI_SUBPIX
	 */
	void drawCaret(const Point &pos);
};

} // End of namespace Glk

#endif
