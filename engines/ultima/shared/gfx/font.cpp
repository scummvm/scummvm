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

#include "ultima/shared/gfx/font.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

Font::Font(const byte *data, size_t startingChar, size_t charCount) :
	_data(data), _startingChar(startingChar), _endingChar(startingChar + charCount - 1) {}

int Font::writeString(Graphics::ManagedSurface &surface, const Common::String &msg, Point &pt,
		byte color, byte bgColor) {
	int total = 0;
	int xs = pt.x;

	for (const char *msgP = msg.c_str(); *msgP; ++msgP, total += 8) {
		if (*msgP == '\n') {
			// Move to next line
			pt.x = xs;
			pt.y += lineHeight();
		} else {
			// Write out character
			writeChar(surface, (unsigned char)*msgP, pt, color, bgColor);
		}
	}
	
	return total;
}

void Font::writeChar(Graphics::ManagedSurface &surface, unsigned char c, Point &pt,
		byte color, byte bgColor) {
	assert(c >= _startingChar && c <= _endingChar);
	const byte *charP = _data + (c - _startingChar) * 8;
	Graphics::Surface s = surface.getSubArea(Common::Rect(pt.x, pt.y, pt.x + 8, pt.y + 8));

	for (int y = 0; y < 8; ++y) {
		byte *lineP = (byte *)s.getBasePtr(0, y);
		byte lineData = charP[y];

		for (int x = 0; x < 8; ++x, lineData <<= 1, ++lineP) {
			*lineP = (lineData & 0x80) ? color : bgColor;
		}
	}

	pt.x += charWidth(c);
}

uint Font::charWidth(char c) const {
	return 8;
}

uint Font::lineHeight() const {
	return 8;
}

uint Font::stringWidth(const Common::String &msg) const {
	return msg.size() * 8;
}

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima
