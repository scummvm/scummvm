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

namespace Gargoyle {

void Screen::fill(const byte *rgb) {
	uint color = format.RGBToColor(rgb[0], rgb[1], rgb[2]);
	clear(color);
}

void Screen::fillRect(uint x, uint y, uint w, uint h, const byte *rgb) {
	uint color = format.RGBToColor(rgb[0], rgb[1], rgb[2]);
	Graphics::Screen::fillRect(Common::Rect(x, y, x + w, y + h), color);
}

int Screen::drawString(int x, int y, int fidx, const byte *rgb, const char *s, int n, int spw) {
	// TODO
	return 0;
}

int Screen::drawStringUni(int x, int y, int fidx, const byte *rgb, const uint32 *s, int n, int spw) {
	// TODO
	return 0;
}

int Screen::stringWidth(int fidx, const char *s, int n, int spw) {
	// TODO
	return 0;
}

int Screen::stringWidthUni(int fidx, const uint32 *s, int n, int spw) {
	// TODO
	return 0;
}

void Screen::drawCaret(const Common::Point &pos) {
	// TODO
}

} // End of namespace Gargoyle
