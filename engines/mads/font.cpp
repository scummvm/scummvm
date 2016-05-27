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

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/compression.h"
#include "mads/font.h"
#include "mads/msurface.h"

namespace MADS {

MADSEngine *Font::_vm;

Common::HashMap<Common::String, Font *> *Font::_fonts;

uint8 Font::_fontColors[4];

void Font::init(MADSEngine *vm) {
	_vm = vm;
	_fontColors[0] = 0xFF;
	_fontColors[1] = 0xF;
	_fontColors[2] = 7;
	_fontColors[3] = 8;

	_fonts = new Common::HashMap<Common::String, Font *>();
}

void Font::deinit() {
	Common::HashMap<Common::String, Font *>::iterator i;
	for (i = _fonts->begin(); i != _fonts->end(); ++i)
		delete (*i)._value;

	delete _fonts;
}

Font *Font::getFont(const Common::String &fontName) {
	if (_fonts->contains(fontName)) {
		return _fonts->getVal(fontName);
	} else {
		Font *font = new Font(fontName);
		_fonts->setVal(fontName, font);
		return font;
	}
}

Font::Font() {
	_charWidths = nullptr;
	_charOffs = nullptr;
	_charData = nullptr;
	setFont(FONT_INTERFACE);
}

Font::Font(const Common::String &filename) {
	_charWidths = nullptr;
	_charOffs = nullptr;
	_charData = nullptr;
	setFont(filename);
}

Font::~Font() {
	delete[] _charWidths;
	delete[] _charOffs;
	delete[] _charData;
}

void Font::setFont(const Common::String &filename) {
	if (!_filename.empty() && (filename == _filename))
		// Already using specified font, so don't bother reloading
		return;

	_filename = filename;

	Common::String resName = filename;
	if (!resName.hasSuffix(".FF"))
		resName += ".FF";

	MadsPack fontData(resName, _vm);
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

void Font::setColors(uint8 v1, uint8 v2, uint8 v3, uint8 v4) {
	_fontColors[0] = v1;
	_fontColors[1] = v2;
	_fontColors[2] = v3;
	_fontColors[3] = v4;
}

void Font::setColorMode(SelectionMode mode) {
	switch (mode) {
	case SELMODE_UNSELECTED:
		setColors(0xFF, 4, 4, 0);
		break;
	case SELMODE_HIGHLIGHTED:
		setColors(0xFF, 5, 5, 0);
		break;
	case SELMODE_SELECTED:
		setColors(0xFF, 6, 6, 0);
		break;
	default:
		break;
	}
}

int Font::writeString(BaseSurface *surface, const Common::String &msg, const Common::Point &pt,
		int spaceWidth, int width) {
	int xEnd;
	if (width > 0)
		xEnd = MIN((int)surface->w, pt.x + width);
	else
		xEnd = surface->w;

	int x = pt.x;
	int y = pt.y;

	int skipY = 0;
	if (y < 0) {
		skipY = -y;
		y = 0;
	}

	int height = MAX(0, _maxHeight - skipY);
	if (height == 0)
		return x;

	int bottom = y + height - 1;
	if (bottom > surface->h - 1) {
		height -= MIN(height, bottom - (surface->h - 1));
	}

	if (height <= 0)
		return x;

	int xPos = x;

	const char *text = msg.c_str();
	while (*text) {
		char theChar = (*text++) & 0x7F;
		int charWidth = _charWidths[(byte)theChar];

		if (charWidth > 0) {
			if (xPos + charWidth > xEnd)
				return xPos;

			Graphics::Surface dest = surface->getSubArea(
				Common::Rect(xPos, y, xPos + charWidth, y + height));
			uint8 *charData = &_charData[_charOffs[(byte)theChar]];
			int bpp = getBpp(charWidth);

			if (skipY != 0)
				charData += bpp * skipY;

			for (int i = 0; i < height; i++) {
				byte *destPtr = (byte *)dest.getBasePtr(0, i);

				for (int j = 0; j < bpp; j++) {
					if (*charData & 0xc0)
						*destPtr = _fontColors[(*charData & 0xc0) >> 6];
					destPtr++;
					if (*charData & 0x30)
						*destPtr = _fontColors[(*charData & 0x30) >> 4];
					destPtr++;
					if (*charData & 0x0C)
						*destPtr = _fontColors[(*charData & 0x0C) >> 2];
					destPtr++;
					if (*charData & 0x03)
						*destPtr = _fontColors[*charData & 0x03];
					destPtr++;
					charData++;
				}
			}
		}

		xPos += charWidth + spaceWidth;
	}

	return xPos;
}

int Font::getWidth(const Common::String &msg, int spaceWidth) {
	int width = 0;
	const char *text = msg.c_str();

	if (msg.size() > 0) {
		while (*text)
			width += _charWidths[*text++ & 0x7F] + spaceWidth;
		width -= spaceWidth;
	}

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

} // End of namespace MADS
