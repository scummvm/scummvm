/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "graphics/fonts/dosfont.h"
#include "graphics/surface.h"

#include "tot/tot.h"
#include "tot/font/biosfont.h"


namespace Tot {

BiosFont::BiosFont() { }

int BiosFont::getFontHeight() const {
	return 8;
}

int BiosFont::getMaxCharWidth() const {
	return 8;
}

int BiosFont::getCharWidth(uint32 chr) const {
	return 8;
}

void BiosFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	drawBiosFontGlyph(chr, x, y, color);
}

void BiosFont::drawBiosFontGlyph(uint8 chr, int x, int y, uint8 color) const {
	byte *ptr = (byte *)g_engine->_screen->getBasePtr(x, y);
	int srcPixel = chr * 8;
	for (int sy = 0; sy < 8; sy++) {
		for (int sx = 0; sx < 8; sx++) {
			if (Graphics::DosFont::fontData_PCBIOS[srcPixel] & 1 << (7 - sx)) {
				*ptr = color;
			}
			ptr++;
		}
		srcPixel++;
		ptr -= 8;
		ptr += (g_engine->_screen->pitch);
	}
}

} // namespace Darkseed
