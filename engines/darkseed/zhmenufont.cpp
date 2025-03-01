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

#include "darkseed/darkseed.h"
#include "darkseed/zhmenufont.h"

namespace Darkseed {

ZhMenuFont::ZhMenuFont() {
	load();
}

void ZhMenuFont::load() {
	Common::File fontData;
	if (!fontData.open("zhmenufont_game.dat")) {
		error("Error: failed to open zhmenufont_game.dat");
	}
	_glyphs.resize(10);

	for (int i = 0; i < 10; i++) {
		_glyphs[i].charIdx = fontData.readUint16BE();
		fontData.read(_glyphs[i].pixels, 72);
	}

	fontData.close();
}

const ZhMenuFontGlyph *ZhMenuFont::getGlyph(uint32 chr) const {
	for (auto &glyph : _glyphs) {
		if (glyph.charIdx == chr) {
			return &glyph;
		}
	}
	return nullptr;
}

int ZhMenuFont::getFontHeight() const {
	return 24;
}

int ZhMenuFont::getMaxCharWidth() const {
	return 24;
}

int ZhMenuFont::getCharWidth(uint32 chr) const {
	auto glyph = getGlyph(chr);
	return glyph ? getMaxCharWidth() : 8;
}

void ZhMenuFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	auto glyph = getGlyph(chr);
	if (glyph) {
		byte *ptr = (byte *)g_engine->_screen->getBasePtr(x, y);
		int srcPixel = 0;
		int sByteOffset = 0;
		for (int sy = 0; sy < getFontHeight(); sy++) {
			for (int sx = 0; sx < getMaxCharWidth(); sx++) {
				if (glyph->pixels[srcPixel] & 1 << (7 - sByteOffset)) {
					*ptr = (uint8)color;
				}
				sByteOffset++;
				if (sByteOffset == 8) {
					sByteOffset = 0;
					srcPixel++;
				}
				ptr++;
			}
			ptr -= getMaxCharWidth();
			ptr += g_engine->_screen->pitch;
		}
	}
}

} // namespace Darkseed
