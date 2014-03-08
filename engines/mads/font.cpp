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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/compression.h"
#include "mads/font.h"
#include "mads/msurface.h"

namespace MADS {

Font::Font(MADSEngine *vm) : _vm(vm) {
	_sysFont = true;
	/*
	_fontColors[0] = _vm->_palette->BLACK;
	_fontColors[1] = _vm->_palette->WHITE;
	_fontColors[2] = _vm->_palette->BLACK;
	_fontColors[3] = _vm->_palette->DARK_GRAY;
	*/
}

Font::~Font() {
	if (!_sysFont) {
		delete[] _charWidths;
		delete[] _charOffs;
		delete[] _charData;
	}
}

void Font::setFont(const Common::String &filename) {
	if (!_filename.empty() && (filename == _filename))
		// Already using specified font, so don't bother reloading
		return;

	_sysFont = false;
	_filename = filename;

	MadsPack fontData(filename, _vm);
	Common::SeekableReadStream *fontFile = fontData.getItemStream(0);

	_maxHeight = fontFile->readByte();
	_maxWidth = fontFile->readByte();

	_charWidths = new uint8[128];
	// Char data is shifted by 1
	_charWidths[0] = 0;
	fontFile->read(_charWidths + 1, 127);
	fontFile->readByte();	// remainder

	_charOffs = new uint16[128];

	uint startOffs = 2 + 128 + 256;
	uint fontSize = fontFile->size() - startOffs;

	// Char data is shifted by 1
	_charOffs[0] = 0;
	for (int i = 1; i < 128; i++)
		_charOffs[i] = fontFile->readUint16LE() - startOffs;
	fontFile->readUint16LE();	// remainder

	_charData = new uint8[fontSize];
	fontFile->read(_charData, fontSize);

	delete fontFile;
}

void Font::setColor(uint8 color) {
	if (_sysFont)
		_fontColors[1] = color;
	else 
		_fontColors[3] = color;		
}

void Font::setColors(uint8 v1, uint8 v2, uint8 v3, uint8 v4) {
	_fontColors[0] = v1;
	_fontColors[1] = v2;
	_fontColors[2] = v3;
}

int Font::write(MSurface *surface, const Common::String &msg, const Common::Point &pt, int width, int spaceWidth, uint8 colors[]) {

	/*TODO
	if (custom_ascii_converter) {			 // if there is a function to convert the extended ASCII characters
		custom_ascii_converter(out_string);	 // call it with the string
	}
	*/

	if (width > 0)
		width = MIN(surface->getWidth(), pt.x + width);
	else
		width = surface->getWidth();

	int x = pt.x + 1;
	int y = pt.y + 1;
	
	int skipY = 0;
	if (y < 0) {
		skipY = -y;
		y = 0;
	}

	int height = MAX(0, _maxHeight - skipY);
	if (height == 0)
		return x;

	int bottom = y + height - 1;
	if (bottom > surface->getHeight() - 1) {
		height -= MIN(height, bottom - (surface->getHeight() - 1));
	}

	if (height <= 0)
		return x;

	byte *destPtr = surface->getBasePtr(x, y);
	uint8 *oldDestPtr = destPtr;

	int xPos = x;

	const char *text = msg.c_str();
	while (*text) {
		char theChar = (*text++) & 0x7F;
		int charWidth = _charWidths[(byte)theChar];
		
		if (charWidth > 0) {

			if (xPos + charWidth >= width)
				return xPos;

			uint8 *charData = &_charData[_charOffs[(byte)theChar]];
			int bpp = getBpp(charWidth);

			if (skipY != 0)
				charData += bpp * skipY;

			for (int i = 0; i < height; i++) {
				for (int j = 0; j < bpp; j++) {
					if (*charData & 0xc0)
						*destPtr = colors[(*charData & 0xc0) >> 6];
					destPtr++;
					if (*charData & 0x30)
						*destPtr = colors[(*charData & 0x30) >> 4];
					destPtr++;
					if (*charData & 0x0C)
						*destPtr = colors[(*charData & 0x0C) >> 2];
					destPtr++;
					if (*charData & 0x03)
						*destPtr = colors[*charData & 0x03];
					destPtr++;
					charData++;
				}

				destPtr += surface->getWidth() - bpp * 4;

			}

			destPtr = oldDestPtr + charWidth + spaceWidth;
			oldDestPtr = destPtr;

		}

		xPos += charWidth + spaceWidth;
		
	}

	return xPos;

}

int Font::getWidth(const Common::String &msg, int spaceWidth) {
	int width = 0;
	const char *text = msg.c_str();

	while (*text)
		width += _charWidths[*text++ & 0x7F] + spaceWidth;
	return width;
}

int Font::getBpp(int charWidth) {
	if (charWidth > 12)
		return 4;
	else if (charWidth > 8)
		return 3;
	else if (charWidth > 4)
		return 2;
	else
		return 1;
}

Font *Font::getFont(const Common::String &fontName) {
	Font *font = new Font(_vm);
	font->setFont(fontName);
	return font;
}

} // End of namespace MADS
