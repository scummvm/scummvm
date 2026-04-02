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

#include "waynesworld/graphics.h"
#include "common/file.h"
#include "common/rect.h"
#include "common/system.h"

namespace WaynesWorld {

void drawSurfaceIntern(Graphics::Surface *destSurface, const Graphics::Surface *surface, int x, int y, bool transparent) {
	int skipX = 0, skipY = 0;

	if (x >= destSurface->w || y >= destSurface->h)
		return;

	int height = surface->h;
	if (y < 0) {
		height += y;
		if (height <= 0)
			return;
		skipY = -y;
		y = 0;
	}
	if (y + height > destSurface->h)
		height = destSurface->h - y;

	int width = surface->w;
	if (x < 0) {
		width += x;
		if (width <= 0)
			return;
		skipX = -x;
		x = 0;
	}
	if (x + width >= destSurface->w)
		width = destSurface->w - x;

	// debug(6, "drawSurfaceIntern() (%d, %d, %d, %d); skipX: %d; skipY: %d", x, y, width, height, skipX, skipY);

	if (transparent) {
		for (int yc = 0; yc < height; ++yc) {
			const byte *source = (const byte*)surface->getBasePtr(skipX, skipY + yc);
			byte *dest = (byte*)destSurface->getBasePtr(x, y + yc);
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
			byte *dest = (byte*)destSurface->getBasePtr(x, y + yc);
			memcpy(dest, source, width);
		}
	}
}

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

void WWSurface::drawSurface(const Graphics::Surface *surface, int x, int y) {
	drawSurfaceIntern(this, surface, x, y, false);
}

void WWSurface::drawSurfaceTransparent(const Graphics::Surface *surface, int x, int y) {
	drawSurfaceIntern(this, surface, x, y, true);
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
	const uint16 charCount = _lastChar - _firstChar + 2;
	fd.seek(0x48);
	const uint32 charTableOfs = fd.readUint32LE();
	const uint32 charDataOfs = fd.readUint32LE();
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
			const byte charByte = fd.readByte();
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
	const int textX = x;
	int lineWidth = 0;
	while (*textP) {
		if (textP > text && textP[-1] == 32)
			lastSpaceP = textP - 1;
		const int charWidth = getCharWidth(*textP);
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
	const int charWidth = getCharWidth(ch);
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
}

Screen::~Screen() {
	delete _screenCopy;
}

void Screen::beginUpdate() {
	if (_lockCtr == 0) {
		_vgaSurface = g_system->lockScreen();
	}
	_lockCtr++;
}

void Screen::endUpdate() {
	if (_lockCtr > 0) {
		--_lockCtr;
		if (_lockCtr == 0) {
			g_system->unlockScreen();
			_vgaSurface = nullptr;
			g_system->updateScreen();
		}
	}
}

void Screen::drawSurface(const Graphics::Surface *surface, int x, int y) {
	if (!surface) {
		warning("drawSurface - Trying to draw on a null surface!");
		return;
	}

	beginUpdate();
	drawSurfaceIntern(_vgaSurface, surface, x, y, false);
	endUpdate();
}

void Screen::drawSurfaceTransparent(const Graphics::Surface *surface, int x, int y) {
	if (!surface) {
		warning("drawSurfaceTransparent - Trying to draw on a null surface!");
		return;
	}

	beginUpdate();
	drawSurfaceIntern(_vgaSurface, surface, x, y, true);
	endUpdate();
}

void Screen::frameRect(int x1, int y1, int x2, int y2, byte color) {
	beginUpdate();
	_vgaSurface->frameRect(Common::Rect(x1, y1, x2, y2), color);
	endUpdate();
}

void Screen::fillSquare(int x, int y, int length, byte color) {
	beginUpdate();
	_vgaSurface->fillRect(Common::Rect(x, y, x + length, y + length), color);
	endUpdate();
}

void Screen::fillRect(int x1, int y1, int x2, int y2, byte color) {
	beginUpdate();
	_vgaSurface->fillRect(Common::Rect(x1, y1, x2, y2), color);
	endUpdate();
}

void Screen::clear(byte color) {
	beginUpdate();
	_vgaSurface->fillRect(Common::Rect(_vgaSurface->w, _vgaSurface->h), color);
	endUpdate();
}

void Screen::drawText(GFTFont *font, const char *text, int x, int y, byte color) {
	beginUpdate();
	font->drawText(_vgaSurface, text, x, y, color);
	endUpdate();
}

void Screen::drawWrappedText(GFTFont *font, const char *text, int x, int y, int maxWidth, byte color) {
	beginUpdate();
	font->drawWrappedText(_vgaSurface, text, x, y, maxWidth, color);
	endUpdate();
}

void Screen::saveScreenshot() {
	beginUpdate();
	delete _screenCopy;
	_screenCopy = new Graphics::Surface();
	_screenCopy->copyFrom(*_vgaSurface);
	endUpdate();
}

// ScreenEffect

ScreenEffect::ScreenEffect(WaynesWorldEngine *vm, Graphics::Surface *surface, int x, int y, int grainWidth, int grainHeight)
	: _vm(vm), _surface(surface), _x(x), _y(y), _grainWidth(grainWidth), _grainHeight(grainHeight), _blockCtr(0) {
	_blockCountW = _surface->w / _grainWidth + (_surface->w % _grainWidth > 0 ? 1 : 0);
	_blockCountH = _surface->h / _grainHeight + (_surface->h % _grainHeight > 0 ? 1 : 0);
	const int blockCount = _blockCountW * _blockCountH;
	const int duration = blockCount / 5; // Approximate time this effect should take in ms
	_timePerSlice = 50; // Time after which the screen should be updated
	_blocksPerSlice = blockCount / (duration / _timePerSlice);
}

void ScreenEffect::drawSpiralEffect() {
	int middleBlockW = _blockCountW / 2 + (_blockCountW % 2 > 0 ? 1 : 0);
	int middleBlockH = _blockCountH / 2 + (_blockCountH % 2 > 0 ? 1 : 0);
	int startBlock = (middleBlockW < middleBlockH ? middleBlockW : middleBlockH) - 1;
	int sideLenW = _blockCountW - startBlock;
	int sideLenH = _blockCountH - startBlock;
	_totalSliceTicks = g_system->getMillis();
	_vm->_screen->beginUpdate();
	while (startBlock >= 0 && !_vm->shouldQuit()) {
		int blockX = startBlock;
		int blockY = startBlock;
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
	_vm->_screen->endUpdate();
}

void ScreenEffect::drawRandomEffect() {
	const uint bitCountW = getBitCount(_blockCountW);
	const uint bitCountH = getBitCount(_blockCountH);
	const uint bitCount = bitCountW + bitCountH;
	const uint mask = (1 << bitCountW) - 1;
	const uint rvalue = getSeed(bitCount);
	uint value = 1;
	_totalSliceTicks = g_system->getMillis();
	_vm->_screen->beginUpdate();
	do {
		const int blockX = value & mask;
		const int blockY = value >> bitCountW;
		drawBlock(blockX, blockY);
		if (value & 1) {
			value >>= 1;
			value ^= rvalue;
		} else {
			value >>= 1;
		}
	} while (value != 1 && !_vm->shouldQuit());
	drawBlock(0, 0);
	_vm->_screen->endUpdate();
}

void ScreenEffect::drawBlock(int blockX, int blockY) {
	if (blockX < _blockCountW && blockY < _blockCountH) {
		const int sourceLeft = blockX * _grainWidth;
		const int sourceTop = blockY * _grainHeight;
		int sourceRight = sourceLeft + _grainWidth, sourceBottom = sourceTop + _grainHeight;
		sourceRight = MIN<int>(_surface->w, sourceRight);
		sourceBottom = MIN<int>(_surface->h, sourceBottom);
		Common::Rect r(sourceLeft, sourceTop, sourceRight, sourceBottom);
		Graphics::Surface blockSurface = _surface->getSubArea(r);
		_vm->_screen->drawSurface(&blockSurface, _x + sourceLeft, _y + sourceTop);
		if (++_blockCtr == _blocksPerSlice) {
			_vm->_screen->endUpdate();
			const uint32 currTicks = g_system->getMillis();
			_blockCtr = 0;
			_totalSliceTicks += _timePerSlice;
			// Check if the system is faster than the current slice ticks
			// and wait for the difference.
			if (currTicks < _totalSliceTicks) {
				const uint32 waitTicks = _totalSliceTicks - currTicks;
				_vm->waitMillis(waitTicks);
			} else {
				_vm->updateEvents();
			}
			_vm->_screen->beginUpdate();
		}
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
