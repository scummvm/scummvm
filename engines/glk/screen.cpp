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

#include "glk/screen.h"
#include "glk/conf.h"

namespace Gargoyle {

void Screen::fill(const byte *rgb) {
	uint color = format.RGBToColor(rgb[0], rgb[1], rgb[2]);
	clear(color);
}

void Screen::fillRect(const Rect &box, const byte *rgb) {
	uint color = format.RGBToColor(rgb[0], rgb[1], rgb[2]);
	Graphics::Screen::fillRect(box, color);
}

void Screen::drawCaret(const Point &pos) {
	const byte *rgb = g_conf->_caretColor;
	uint color = format.RGBToColor(rgb[0], rgb[1], rgb[2]);
	int x = pos.x / GLI_SUBPIX, y = pos.y;

	switch (g_conf->_caretShape) {
	case SMALL_DOT:
		hLine(x + 0, y + 1, x + 0, color);
		hLine(x - 1, y + 2, x + 1, color);
		hLine(x - 2, y + 3, x + 2, color);
		break;

	case FAT_DOT:
		hLine(x + 0, y + 1, x + 0, color);
		hLine(x - 1, y + 2, x + 1, color);
		hLine(x - 2, y + 3, x + 2, color);
		hLine(x - 3, y + 4, x + 3, color);
		break;

	case THIN_LINE:
		vLine(x, y - g_conf->_baseLine + 1, y - 1, color);
		break;

	case FAT_LINE:
		Graphics::Screen::fillRect(Rect(x, y - g_conf->_baseLine + 1, x + 1,  y - 1), color);
		break;

	default:
		// BLOCK
		Graphics::Screen::fillRect(Rect(x, y - g_conf->_baseLine + 1, x + g_conf->_cellW, y - 1), color);
		break;
	}
}

} // End of namespace Gargoyle
