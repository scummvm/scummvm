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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/freescape/freescape.h"
#include "engines/freescape/font.h"

namespace Freescape {

Font::Font() {
	_backgroundColor = 0;
	_secondaryColor = 0;
	_kerningOffset = 0;
	_charWidth = 0;
	_chars.clear();
}

Font::Font(Common::Array<Graphics::ManagedSurface *> &chars) {
	_chars = chars;
	_backgroundColor = 0;
	_secondaryColor = 0;
	_kerningOffset = 0;
	_charWidth = 8;
}

Font::~Font() {
	/*for (Graphics::ManagedSurface *surface : _chars) {
		surface->free();
		delete surface;
	}*/
}

int Font::getCharWidth(uint32 chr) const {
	return _charWidth;
}

int Font::getMaxCharWidth() const {
	return getCharWidth(0);
}

int Font::getFontHeight() const {
	return _chars[0]->h + 1;
}

void Font::setSecondaryColor(uint32 color) {
	_secondaryColor = color;
}

void Font::setBackground(uint32 color) {
	_backgroundColor = color;
}

void Font::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	assert(chr >= 32);
	chr -= 32;

	Graphics::ManagedSurface surface = Graphics::ManagedSurface();
	surface.copyFrom(*_chars[chr]);

	uint8 rb, gb, bb;
	uint8 rp, gp, bp;
	uint8 rs, gs, bs;

	dst->format.colorToRGB(color, rp, gp, bp);
	dst->format.colorToRGB(_secondaryColor, rs, gs, bs);
	dst->format.colorToRGB(_backgroundColor, rb, gb, bb);

	byte palette[3][3] = {
		{ rb, gb, bb },
		{ rp, gp, bp },
		{ rs, gs, bs },
	};

	if (surface.format != dst->format)
		surface.convertToInPlace(dst->format, (byte *)palette, 3);

	if (_backgroundColor == dst->format.ARGBToColor(0x00, 0x00, 0x00, 0x00))
		dst->copyRectToSurfaceWithKey(surface, x, y, Common::Rect(0, 0, MIN(int(surface.w), _charWidth), surface.h), dst->format.ARGBToColor(0xFF, 0x00, 0x00, 0x00));
	else
		dst->copyRectToSurface(surface, x, y, Common::Rect(0, 0, MIN(int(surface.w), _charWidth), surface.h));

	surface.free();
}

Common::Array<Graphics::ManagedSurface *> FreescapeEngine::getChars(Common::SeekableReadStream *file, int offset, int charsNumber) {
	byte *fontBuffer = (byte *)malloc(6 * charsNumber);
	file->seek(offset);
	file->read(fontBuffer, 6 * charsNumber);

	Common::BitArray font;
	font.set_size(48 * charsNumber);
	font.set_bits(fontBuffer);

	Common::Array<Graphics::ManagedSurface *> chars;

	int sizeX = 8;
	int sizeY = 6;
	int additional = isEclipse() ? 0 : 1;

	for (int c = 0; c < charsNumber - 1; c++) {
		int position = sizeX * sizeY * c;

		Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
		surface->create(_renderMode == Common::kRenderHercG ? 16 : 8, sizeY, Graphics::PixelFormat::createFormatCLUT8());
		for (int j = 0; j < sizeY; j++) {
			for (int i = 0; i < sizeX; i++) {
				if (font.get(position + additional + j * 8 + i)) {
					if (_renderMode != Common::kRenderHercG) {
						surface->setPixel(7 - i, j, 1);
					} else {
						surface->setPixel(2 * (7 - i), j, 1);
						surface->setPixel(2 * (7 - i) + 1, j, 1);
					}
				}
			}
		}
		chars.push_back(surface);
	}
	return chars;
}

Common::Array<Graphics::ManagedSurface *> FreescapeEngine::getCharsAmigaAtari(Common::SeekableReadStream *file, int offset, int charsNumber) {

	file->seek(offset);
	int fontSize = 4654;
	byte *fontBuffer = (byte *)malloc(fontSize);
	file->read(fontBuffer, fontSize);

	Common::BitArray font;
	font.set_size(8 * fontSize);
	font.set_bits(fontBuffer);

	Common::Array<Graphics::ManagedSurface *> chars;

	int sizeX = 8;
	int sizeY = 8;
	int additional = isEclipse() ? 0 : 1;
	int m1 = isDriller() ? 33 : 16;
	int m2 = isDriller() ? 32 : 16;

	for (int c = 0; c < charsNumber - 1; c++) {
		int position = 8 * (m1*c + 1);
		Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
		surface->create(sizeX, sizeY, Graphics::PixelFormat::createFormatCLUT8());
		for (int j = 0; j < sizeY; j++) {
			for (int i = 0; i < sizeX; i++) {
				if (font.get(position + additional + j * m2 + i + 7))
					surface->setPixel(7 - i, j, 2);
				else if (font.get(position + j * m2 + i)) {
					surface->setPixel(7 - i, j, 1);
				} else
					surface->setPixel(7 - i, j, 0);
			}
		}
		chars.push_back(surface);
	}
	return chars;
}

void FreescapeEngine::drawStringInSurface(const Common::String &str, int x, int y, uint32 fontColor, uint32 backColor, Graphics::Surface *surface, int offset) {
	Common::String ustr = str;
	ustr.toUppercase();
	_font.setBackground(backColor);
	_font.drawString(surface, ustr, x, y, _screenW, fontColor);
}

void FreescapeEngine::drawStringInSurface(const Common::String &str, int x, int y, uint32 primaryColor, uint32 secondaryColor, uint32 backColor, Graphics::Surface *surface, int offset) {
	Common::String ustr = str;
	ustr.toUppercase();
	_font.setBackground(backColor);
	_font.setSecondaryColor(secondaryColor);
	_font.drawString(surface, ustr, x, y, _screenW, primaryColor);
}


} // End of namespace Freescape
