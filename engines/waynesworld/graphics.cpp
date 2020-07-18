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

#include "waynesworld/graphics.h"
#include "common/file.h"
#include "common/rect.h"
#include "common/system.h"

namespace WaynesWorld {

// WWSurface

WWSurface::WWSurface(int width, int height) {
	create(width, height, Graphics::PixelFormat::createFormatCLUT8());
}

WWSurface::WWSurface(const Graphics::Surface *sourceSurface) {
	copyFrom(*sourceSurface);
}

WWSurface::~WWSurface() {
	free();
}

void WWSurface::drawSurfaceIntern(const Graphics::Surface *surface, int x, int y, bool transparent) {
	int width, height, skipX = 0, skipY = 0;

	if (x >= w || y >= h)
		return;

	height = surface->h;
	if (y < 0) {
		height += y;
		if (height <= 0)
			return;
		skipY = -y;
		y = 0;
	}
	if (y + height > h)
		height = h - y;

	width = surface->w;
	if (x < 0) {
		width += x;
		if (width <= 0)
			return;
		skipX = -x;
		x = 0;
	}
	if (x + width >= w)
		width = w - x;

	// debug(6, "drawSurfaceIntern() (%d, %d, %d, %d); skipX: %d; skipY: %d", x, y, width, height, skipX, skipY);

	if (transparent) {
		for (int yc = 0; yc < height; ++yc) {
			const byte *source = (const byte*)surface->getBasePtr(skipX, skipY + yc);
			byte *dest = (byte*)getBasePtr(x, y + yc);
			for (int xc = 0; xc < width; ++xc) {
				byte value = *source++;
				if (value != 0) {
					*dest = value;
				}
				++dest;
			}
		}
	} else {
		for (int yc = 0; yc < height; ++yc) {
			const byte *source = (const byte*)surface->getBasePtr(skipX, skipY + yc);
			byte *dest = (byte*)getBasePtr(x, y + yc);
			memcpy(dest, source, width);
		}
	}
}

void WWSurface::drawSurface(const Graphics::Surface *surface, int x, int y) {
	drawSurfaceIntern(surface, x, y, false);
}

void WWSurface::drawSurfaceTransparent(const Graphics::Surface *surface, int x, int y) {
	drawSurfaceIntern(surface, x, y, true);
}

void WWSurface::scaleSurface(const Graphics::Surface *surface) {
	const int xIncr = surface->w / w;
	const int xIncrErr = surface->w % w;
	const int yIncr = (surface->h / h) * surface->pitch;
	const int yIncrErr = surface->h % h;
	int errY = 0;
	const byte *source = (const byte*)surface->getBasePtr(0, 0);
	for (int yc = 0; yc < h; yc++) {
		const byte *sourceRow = source;
		byte *destRow = (byte*)getBasePtr(0, yc);
		int errX = 0;
		for (int xc = 0; xc < w; xc++) {
			*destRow++ = *sourceRow;
			sourceRow += xIncr;
			errX += xIncrErr;
			if (errX >= w) {
				errX -= w;
				sourceRow++;
			}
		}
		source += yIncr;
		errY += yIncrErr;
		if (errY >= h) {
			errY -= h;
			source += surface->pitch;
		}
	}
}

void WWSurface::frameRect(int x1, int y1, int x2, int y2, byte color) {
	Graphics::Surface::frameRect(Common::Rect(x1, y1, x2, y2), color);
}

void WWSurface::borderSquare(int x, int y, int length, byte frameColor, byte fillColor) {
	Graphics::Surface::frameRect(Common::Rect(x, y, x + length, y + length), frameColor);
}

void WWSurface::fillRect(int x1, int y1, int x2, int y2, byte color) {
	Graphics::Surface::fillRect(Common::Rect(x1, y1, x2, y2), color);
}

void WWSurface::clear(byte color) {
	Graphics::Surface::fillRect(Common::Rect(w, h), color);
}

// GFTFont

GFTFont::GFTFont() : _charTable(nullptr), _fontData(nullptr) {
}

GFTFont::~GFTFont() {
	delete[] _charTable;
	delete[] _fontData;
}

void GFTFont::loadFromFile(const char *filename) {
	Common::File fd;
	if (!fd.open(filename))
		error("GFTFont::loadFromFile() Could not open %s", filename);
	fd.seek(0x24);
	_firstChar = fd.readUint16LE();
	_lastChar = fd.readUint16LE();
	uint16 charCount = _lastChar - _firstChar + 2;
	fd.seek(0x48);
	uint32 charTableOfs = fd.readUint32LE();
	uint32 charDataOfs = fd.readUint32LE();
	_formWidth = fd.readSint16LE();
	_formHeight = fd.readSint16LE();
	fd.seek(charTableOfs);
	_charTable = new uint16[charCount];
	for (uint16 i = 0; i < charCount; i++)
		_charTable[i] = fd.readUint16LE();
	fd.seek(charDataOfs);
	_fontData = new byte[_formWidth * 8 * _formHeight];
    for (int y = 0; y < _formHeight; y++) {
        int x = 0;
        for (int formPos = 0; formPos < _formWidth; formPos++) {
            byte charByte = fd.readByte();
            for (int bitNum = 0; bitNum < 8; bitNum++) {
                _fontData[x + y * _formWidth * 8] = ((charByte & (1 << (7 - bitNum))) != 0) ? 1 : 0;
                x++;
            }
        }
    }
}

void GFTFont::drawText(Graphics::Surface *surface, const char *text, int x, int y, byte color) {
	while (*text) {
		byte ch = (byte)*text++;
		x += drawChar(surface, ch, x, y, color);
	}
}

void GFTFont::drawWrappedText(Graphics::Surface *surface, const char *text, int x, int y, int maxWidth, byte color) {
    const char *textP = text;
    const char *lineP = text, *lastSpaceP = nullptr;
	int textX = x;
    int lineWidth = 0;
    while (*textP) {
        if (textP[1] == 32 || textP[1] == 0)
            lastSpaceP = textP + 1;
        int charWidth = getCharWidth(*textP);
        if (lineWidth + charWidth > maxWidth || textP[1] == 0) {
			for (const char *p = lineP; p < lastSpaceP; p++) {
				x += drawChar(surface, (byte)*p, x, y, color);
			}
            if (textP[1] == 0)
                break;
            lineP = lastSpaceP + 1;
            textP = lastSpaceP + 1;
            lineWidth = 0;
			x = textX;
            y += _formHeight;
        } else {
            lineWidth += charWidth;
            textP++;
        }
    }
}

int GFTFont::drawChar(Graphics::Surface *surface, byte ch, int x, int y, byte color) {
	if (ch < _firstChar || ch > _lastChar)
		return 0;
	const uint charIndex = ch - _firstChar;
	byte *charData = _fontData + _charTable[charIndex];
	int charWidth = getCharWidth(ch);
	for (int yc = 0; yc < _formHeight; yc++) {
		byte *destRow = (byte*)surface->getBasePtr(x, y + yc);
		for (int xc = 0; xc < charWidth; xc++) {
			if (charData[xc] != 0)
				*destRow = color;
			destRow++;
		}
		charData += _formWidth * 8;
	}
	return charWidth;
}

int GFTFont::getTextWidth(const char *text) const {
	int textWidth = 0;
	while (*text) {
		textWidth += getCharWidth(*text++);
	}
	return textWidth;
}

int GFTFont::getCharWidth(byte ch) const {
	if (ch < _firstChar || ch > _lastChar)
		return 0;
	const uint charIndex = ch - _firstChar;
	return _charTable[charIndex + 1] - _charTable[charIndex];
}

// Screen

Screen::Screen() {
	_surface = new WWSurface(320, 200);
}

Screen::~Screen() {
	delete _surface;
}

void Screen::drawSurface(const Graphics::Surface *surface, int x, int y) {
	_surface->drawSurface(surface, x, y);
	updateScreen();
}

void Screen::drawSurfaceTransparent(const Graphics::Surface *surface, int x, int y) {
	_surface->drawSurfaceTransparent(surface, x, y);
	updateScreen();
}

void Screen::frameRect(int x1, int y1, int x2, int y2, byte color) {
	_surface->frameRect(x1, y1, x2, y2, color);
	updateScreen();
}

void Screen::borderSquare(int x, int y, int length, byte frameColor, byte fillColor) {
	_surface->borderSquare(x, y, length, frameColor, fillColor);
	updateScreen();
}

void Screen::fillRect(int x1, int y1, int x2, int y2, byte color) {
	_surface->fillRect(x1, y1, x2, y2, color);
	updateScreen();
}

void Screen::clear(byte color) {
	_surface->clear(color);
	updateScreen();
}

void Screen::drawText(GFTFont *font, const char *text, int x, int y, byte color) {
	font->drawText(_surface, text, x, y, color);
	updateScreen();
}

void Screen::drawWrappedText(GFTFont *font, const char *text, int x, int y, int maxWidth, byte color) {
	font->drawWrappedText(_surface, text, x, y, maxWidth, color);
	updateScreen();
}

void Screen::updateScreen() {
	// TODO Use dirty rectangles or similar
	g_system->copyRectToScreen(_surface->getPixels(), _surface->pitch, 0, 0, _surface->w, _surface->h);
	g_system->updateScreen();
}

} // End of namespace WaynesWorld
