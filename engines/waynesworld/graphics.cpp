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

void WWSurface::fillSquare(int x, int y, int length, byte color) {
	Graphics::Surface::fillRect(Common::Rect(x, y, x + length, y + length), color);
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
    const char *lineStartP = text, *lastSpaceP = nullptr;
	int textX = x;
    int lineWidth = 0;
    while (*textP) {
        if (textP > text && textP[-1] == 32)
            lastSpaceP = textP - 1;
        int charWidth = getCharWidth(*textP);
        if (lineWidth + charWidth > maxWidth) {
			const char *lineEndP = lastSpaceP ? lastSpaceP : textP + 1;
			for (const char *p = lineStartP; p < lineEndP; p++) {
				x += drawChar(surface, (byte)*p, x, y, color);
			}
            lineStartP = lastSpaceP + 1;
            textP = lastSpaceP + 1;
            lineWidth = 0;
			x = textX;
            y += _formHeight;
        } else if (textP[1] == 0) {
			const char *lineEndP = textP + 1;
			for (const char *p = lineStartP; p < lineEndP; p++) {
				x += drawChar(surface, (byte)*p, x, y, color);
			}
			break;
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

void Screen::fillSquare(int x, int y, int length, byte color) {
	_surface->fillSquare(x, y, length, color);
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

// ScreenEffect

// TODO Currently effects are quite slow because the screen is updated for each block.
// Add beginUpdate/endUpdate to Screen class and only update each X blocks.

ScreenEffect::ScreenEffect(WaynesWorldEngine *vm, Graphics::Surface *surface, int x, int y, int grainWidth, int grainHeight)
	: _vm(vm), _surface(surface), _x(x), _y(y), _grainWidth(grainWidth), _grainHeight(grainHeight) {
    _blockCountW = _surface->w / _grainWidth + (_surface->w % _grainWidth > 0 ? 1 : 0);
	_blockCountH = _surface->h / _grainHeight + (_surface->h % _grainHeight > 0 ? 1 : 0);
}

void ScreenEffect::drawSpiralEffect() {
	int middleBlockW = _blockCountW / 2 + (_blockCountW % 2 > 0 ? 1 : 0);
	int middleBlockH = _blockCountH / 2 + (_blockCountH % 2 > 0 ? 1 : 0);
	int startBlock = (middleBlockW < middleBlockH ? middleBlockW : middleBlockH) - 1;
	int sideLenW = _blockCountW - startBlock;
	int sideLenH = _blockCountH - startBlock;
	while (startBlock >= 0 && !_vm->shouldQuit()) {
		int blockX, blockY;
		blockX = startBlock;
		blockY = startBlock;
		while (++blockX < sideLenW) {
			drawBlock(blockX, blockY);
		}
		blockX = sideLenW - 1;
		blockY = startBlock;
		while (++blockY < sideLenH) {
			drawBlock(blockX, blockY);
		}
		blockX = sideLenW - 1;
		blockY = sideLenH - 1;
		while (--blockX >= startBlock) {
			drawBlock(blockX, blockY);
		}
		blockX = startBlock;
		blockY = sideLenH;
		while (--blockY >= startBlock) {
			drawBlock(blockX, blockY);
		}
		sideLenW++;
		sideLenH++;
		startBlock--;
	}
}

void ScreenEffect::drawRandomEffect() {
	uint bitCountW = getBitCount(_blockCountW);
	uint bitCountH = getBitCount(_blockCountH);
	uint bitCount = bitCountW + bitCountH;
	uint mask = (1 << bitCountW) - 1;
	uint rvalue = getSeed(bitCount), value = 1;
	do {
		int blockX = value & mask;
		int blockY = value >> bitCountW;
		drawBlock(blockX, blockY);
		if (value & 1) {
			value >>= 1;
			value ^= rvalue;
		} else {
			value >>= 1;
		}
	} while (value != 1 && !_vm->shouldQuit());
	drawBlock(0, 0);
}

void ScreenEffect::drawBlock(int blockX, int blockY) {
	if (blockX < _blockCountW && blockY < _blockCountH) {
		int sourceLeft = blockX * _grainWidth, sourceTop = blockY * _grainHeight;
		int sourceRight = sourceLeft + _grainWidth, sourceBottom = sourceTop + _grainHeight;
		sourceRight = MIN<int>(_surface->w, sourceRight);
		sourceBottom = MIN<int>(_surface->h, sourceBottom);
		Common::Rect r(sourceLeft, sourceTop, sourceRight, sourceBottom);
		Graphics::Surface blockSurface = _surface->getSubArea(r);
		_vm->_screen->drawSurface(&blockSurface, _x + sourceLeft, _y + sourceTop);
		_vm->updateEvents();
	}
}

uint ScreenEffect::getBitCount(int value) const {
	int bitCount = 0;
	while ((value >> bitCount) != 0) {
		bitCount++;
	}
	return bitCount;
}

uint ScreenEffect::getSeed(uint bitCount) const {
	// Only used values are implemented
	switch (bitCount) {
	case 13:
		return 0x3500;
	case 15:
		return 0xB400;
	case 16:
		return 0x12000;
	default:
		return 1;
	}
}

} // End of namespace WaynesWorld
