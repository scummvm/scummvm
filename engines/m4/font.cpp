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

#include "m4/font.h"
#include "m4/m4.h"
#include "m4/compression.h"

namespace M4 {

FontManager::~FontManager() {
	for (uint i = 0; i < _entries.size(); ++i)
		delete _entries[i];
	_entries.clear();
}

Font *FontManager::getFont(const char *filename) {
	// Append an extension if the filename doesn't already have one
	char buffer[20];
	strncpy(buffer, filename, 19);
	if (!strchr(buffer, '.'))
		strcat(buffer, ".ff");

	// Check if the font is already loaded
	for (uint i = 0; i < _entries.size(); ++i) {
		if (!strcmp(_entries[i]->_filename, buffer))
			return _entries[i];
	}

	Font *f = new Font(_vm, buffer);
	_entries.push_back(f);
	return f;
}

void FontManager::setFont(const char *filename) {
	_currentFont = getFont(filename);
}

//--------------------------------------------------------------------------

Font::Font(MadsM4Engine *vm, const char *filename) : _vm(vm) {
	_sysFont = true;
	strncpy(_filename, filename, 19);
	_filename[19] = '\0';

	//TODO: System font
	_fontColors[0] = _vm->_palette->BLACK;
	_fontColors[1] = _vm->_palette->WHITE;
	_fontColors[2] = _vm->_palette->BLACK;
	_fontColors[3] = _vm->_palette->DARK_GRAY;

	_sysFont = false;

	if (_vm->isM4())
		setFontM4(filename);
	else
		setFontMads(filename);
}

void Font::setFontM4(const char *filename) {
	Common::SeekableReadStream *fontFile = _vm->res()->openFile(filename);

	if (fontFile->readUint32LE() != MKTAG('F','O','N','T')) {
		debugCN(kDebugGraphics, "Font::Font: FONT tag expected\n");
		return;
	}

	_maxHeight = fontFile->readByte();
	_maxWidth = fontFile->readByte();
	uint32 fontSize = fontFile->readUint32LE();

	//debugCN(kDebugGraphics, "Font::Font: _maxWidth = %d, _maxHeight = %d, fontSize = %d\n", _maxWidth, _maxHeight, fontSize);

	if (fontFile->readUint32LE() != MKTAG('W','I','D','T')) {
		debugCN(kDebugGraphics, "Font::Font: WIDT tag expected\n");
		return;
	}

	_charWidths = new uint8[256];
	fontFile->read(_charWidths, 256);

	if (fontFile->readUint32LE() != MKTAG('O','F','F','S')) {
		debugCN(kDebugGraphics, "Font::Font: OFFS tag expected\n");
		return;
	}

	_charOffs = new uint16[256];

	for (int i = 0; i < 256; i++)
		_charOffs[i] = fontFile->readUint16LE();

	if (fontFile->readUint32LE() != MKTAG('P','I','X','S')) {
		debugCN(kDebugGraphics, "Font::Font: PIXS tag expected\n");
		return;
	}

	_charData = new uint8[fontSize];
	fontFile->read(_charData, fontSize);

	_vm->res()->toss(filename);
}

void Font::setFontMads(const char *filename) {
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

	uint32 startOffs = 2 + 128 + 256;
	uint32 fontSize = fontFile->size() - startOffs;

	// Char data is shifted by 1
	_charOffs[0] = 0;
	for (int i = 1; i < 128; i++)
		_charOffs[i] = fontFile->readUint16LE() - startOffs;
	fontFile->readUint16LE();	// remainder

	_charData = new uint8[fontSize];
	fontFile->read(_charData, fontSize);

	delete fontFile;
}

Font::~Font() {
	if (!_sysFont) {
		delete[] _charWidths;
		delete[] _charOffs;
		delete[] _charData;
	}
}

void Font::setColor(uint8 color) {
	if (_sysFont)
		_fontColors[1] = color;
	else
		_fontColors[3] = color;
}

void Font::setColors(uint8 col1, uint8 col2, uint8 col3) {
	if (_sysFont)
		_fontColors[1] = col3;
	else {
		_fontColors[0] = 0xFF;
		_fontColors[1] = col1;
		_fontColors[2] = col2;
		_fontColors[3] = col3;
	}
}

int32 Font::write(M4Surface *surface, const char *text, int x, int y, int width, int spaceWidth, uint8 colors[]) {

	/*TODO
	if (custom_ascii_converter) {			 // if there is a function to convert the extended ASCII characters
		custom_ascii_converter(out_string);	 // call it with the string
	}
	*/

	int right;
	if (width > 0)
		right = MIN(surface->width(), x + width + 1);
	else
		right = surface->width();

	x++;
	y++;

	int skipY = 0;
	if (y < 0) {
		skipY = -y;
		y = 0;
	}

	int height = MAX(0, _maxHeight - skipY);
	if (height == 0)
		return x;

	int bottom = y + height - 1;
	if (bottom > surface->height() - 1) {
		height -= MIN(height, bottom - (surface->height() - 1));
	}

	if (height <= 0)
		return x;

	uint8 *destPtr = (uint8*)surface->getBasePtr(x, y);
	uint8 *oldDestPtr = destPtr;

	int xPos = x;

	while (*text) {

		unsigned char theChar = (*text++) & 0x7F;
		int charWidth = _charWidths[theChar];

		if (charWidth > 0) {

			if (xPos + charWidth > right)
				return xPos;

			uint8 *charData = &_charData[_charOffs[theChar]];
			int bpp = charWidth / 4 + 1;

			if (!_vm->isM4()) {
				if (charWidth > 12)
					bpp = 4;
				else if (charWidth > 8)
					bpp = 3;
				else if (charWidth > 4)
					bpp = 2;
				else
					bpp = 1;
			}

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

				destPtr += surface->width() - bpp * 4;

			}

			destPtr = oldDestPtr + charWidth + spaceWidth;
			oldDestPtr = destPtr;

		}

		xPos += charWidth + spaceWidth;

	}

	surface->freeData();
	return xPos;

}

int32 Font::getWidth(const char *text, int spaceWidth) {
	/*
	if (custom_ascii_converter) {			 // if there is a function to convert the extended ASCII characters
		custom_ascii_converter(out_string);	 // call it with the string
	}
	*/
	int width = -spaceWidth;	// Accomodate final character not needing spacing
	while (*text)
		width += _charWidths[*text++ & 0x7F] + spaceWidth;
	return width;
}

} // End of namespace M4
