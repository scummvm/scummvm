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

#include "gargoyle/screen.h"
#include "gargoyle/conf.h"

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

	switch (g_conf->_caretShape) {
	case SMALL_DOT:
		hLine(pos.x + 0, pos.y + 1, pos.x + 0, color);
		hLine(pos.x - 1, pos.y + 2, pos.x + 1, color);
		hLine(pos.x - 2, pos.y + 3, pos.x + 2, color);
		break;

	case FAT_DOT:
		hLine(pos.x + 0, pos.y + 1, pos.x + 0, color);
		hLine(pos.x - 1, pos.y + 2, pos.x + 1, color);
		hLine(pos.x - 2, pos.y + 3, pos.x + 2, color);
		hLine(pos.x - 3, pos.y + 4, pos.x + 3, color);
		break;

	case THIN_LINE:
		vLine(pos.x, pos.y - g_conf->_baseLine + 1, pos.y - 1, color);
		break;

	case FAT_LINE:
		Graphics::Screen::fillRect(Rect(pos.x, pos.y - g_conf->_baseLine + 1, pos.x + 1,  pos.y - 1), color);
		break;

	default:
		// BLOCK
		Graphics::Screen::fillRect(Rect(pos.x, pos.y - g_conf->_baseLine + 1, pos.x + g_conf->_cellW, pos.y - 1), color);
		break;
	}
}

} // End of namespace Gargoyle
