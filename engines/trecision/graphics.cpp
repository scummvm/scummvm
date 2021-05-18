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

#include "common/file.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "trecision/defines.h"
#include "trecision/graphics.h"
#include "trecision/trecision.h"

namespace Trecision {

const Graphics::PixelFormat GraphicsManager::kImageFormat(2, 5, 5, 5, 0, 10, 5, 0, 0); // RGB555

GraphicsManager::GraphicsManager(TrecisionEngine *vm) : _vm(vm), _font(nullptr) {
}

GraphicsManager::~GraphicsManager() {
	_screenBuffer.free();
	_background.free();
	_smkBackground.free();
	_leftInventoryArrow.free();
	_rightInventoryArrow.free();
	_inventoryIcons.free();
	_saveSlotThumbnails.free();
	delete[] _font;
}

bool GraphicsManager::init() {
	const Graphics::PixelFormat *bestFormat = &kImageFormat;

	// Find a 16-bit format, currently we don't support other color depths
	Common::List<Graphics::PixelFormat> formats = g_system->getSupportedFormats();
	bool found = false;
	for (Common::List<Graphics::PixelFormat>::const_iterator i = formats.begin(); i != formats.end(); ++i) {
		if (i->bytesPerPixel == 2) {
			bestFormat = &*i;
			found = true;
			break;
		}
	}

	if (!found)
		return false;
	
	initGraphics(MAXX, MAXY, bestFormat);

	_screenFormat = g_system->getScreenFormat();
	if (_screenFormat.bytesPerPixel != 2)
		return false;
	_bitMask[0] = _screenFormat.rMax() << _screenFormat.rShift;
	_bitMask[1] = _screenFormat.gMax() << _screenFormat.gShift;
	_bitMask[2] = _screenFormat.bMax() << _screenFormat.bShift;

	clearScreen();

	_screenBuffer.create(MAXX, MAXY, _screenFormat);
	_background.create(MAXX, MAXY, _screenFormat);
	_smkBackground.create(MAXX, AREA, _screenFormat);
	_saveSlotThumbnails.create(READICON * ICONDX, ICONDY, _screenFormat);

	loadData();

	return true;
}

void GraphicsManager::clearScreen() {
	g_system->fillScreen(0);
}

void GraphicsManager::copyToScreenBuffer(Graphics::Surface *surface, int x, int y) {
	for (int curY = 0; curY < surface->h; curY++) {
		// NOTE: We use surface width for the pitch so that memcpy works
		// correcly with surfaces from getSubArea()
		memcpy(_screenBuffer.getBasePtr(x, y + curY), surface->getBasePtr(0, curY), surface->w * 2);
	}
}

void GraphicsManager::blitToScreenBuffer(Graphics::Surface *surface, int x, int y, uint16 mask, bool useSmkBg) {
	for (int curY = 0; curY < surface->h; curY++) {
		for (int curX = 0; curX < surface->w; curX++) {
			const int destX = x + curX;
			const int destY = y + curY;
			const uint16 pixel = (uint16)surface->getPixel(curX, curY);
			if (pixel != mask) {
				_screenBuffer.setPixel(destX, destY, pixel);
			} else if (useSmkBg) {
				_screenBuffer.setPixel(destX, destY, _smkBackground.getPixel(destX, destY - TOP));
			}
		}
	}
}

void GraphicsManager::copyToScreen(int x, int y, int w, int h) {
	g_system->copyRectToScreen(
		_screenBuffer.getBasePtr(x, y),
		MAXX * 2, x, y, w, h
	);
}

uint16 *GraphicsManager::getScreenBufferPtr() {
	return (uint16 *)_screenBuffer.getPixels();
}

void GraphicsManager::readSurface(Common::SeekableReadStream *stream, Graphics::Surface *surface, uint16 width, uint16 height, uint16 count) {
	surface->create(width * count, height, kImageFormat);

	for (uint16 i = 0; i < count; i++) {
		for (uint16 y = 0; y < height; y++) {
			for (uint16 x = 0; x < width; x++) {
				surface->setPixel(width * i + x, y, stream->readUint16LE());
			}
		}
	}

	surface->convertToInPlace(_screenFormat);
}

void GraphicsManager::loadBackground(Common::SeekableReadStream *stream, uint16 width, uint16 height) {
	readSurface(stream, &_background, width, height);
	_smkBackground.copyFrom(_background);
	memcpy(_screenBuffer.getBasePtr(0, TOP), _background.getPixels(), _background.pitch * _background.h);
}

void GraphicsManager::loadData() {
	Common::SeekableReadStream *arrowsDataFile = _vm->_dataFile.createReadStreamForMember("frecc.bm");
	// The data file contains images for deactivated arrows, which aren't used. Skip them.
	arrowsDataFile->skip(ICONMARGDX * ICONDY * 2 * 3);
	readSurface(arrowsDataFile, &_leftInventoryArrow, ICONMARGSX, ICONDY);
	readSurface(arrowsDataFile, &_rightInventoryArrow, ICONMARGSX, ICONDY);
	delete arrowsDataFile;

	Common::SeekableReadStream *iconsDataFile = _vm->_dataFile.createReadStreamForMember("icone.bm");
	readSurface(iconsDataFile, &_inventoryIcons, ICONDX, ICONDY, READICON);
	delete iconsDataFile;

	_font = _vm->readData("nlfont.fnt");
}

void GraphicsManager::setSaveSlotThumbnail(byte iconSlot, Graphics::Surface *thumbnail) {
	thumbnail->convertToInPlace(_screenFormat);
	Graphics::Surface *scaled = thumbnail->scale(ICONDX, ICONDY);

	thumbnail->free();
	delete thumbnail;

	for (uint16 y = 0; y < ICONDY; y++) {
		memcpy(_saveSlotThumbnails.getBasePtr(ICONDX * iconSlot, y), scaled->getBasePtr(0, y), ICONDX * 2);
	}

	scaled->free();
	delete scaled;
}

void GraphicsManager::drawLeftInventoryArrow(byte startLine) {
	Graphics::Surface arrow = _leftInventoryArrow.getSubArea(Common::Rect(
		0, startLine, _leftInventoryArrow.w, _leftInventoryArrow.h
	));
	copyToScreenBuffer(&arrow, 0, FIRSTLINE);
}

void GraphicsManager::drawRightInventoryArrow(byte startLine) {
	Graphics::Surface arrow = _rightInventoryArrow.getSubArea(Common::Rect(
		0, startLine, _rightInventoryArrow.w, _rightInventoryArrow.h
	));
	copyToScreenBuffer(&arrow, MAXX - ICONMARGDX, FIRSTLINE);
}

void GraphicsManager::drawInventoryIcon(byte iconIndex, byte iconSlot, byte startLine) {
	Graphics::Surface icon = _inventoryIcons.getSubArea(Common::Rect(
		iconIndex * ICONDX,
		startLine,
		iconIndex * ICONDX + ICONDX,
		_inventoryIcons.h
	));
	copyToScreenBuffer(&icon, iconSlot * ICONDX + ICONMARGSX, FIRSTLINE);
}

void GraphicsManager::drawSaveSlotThumbnail(byte iconIndex, byte iconSlot, byte startLine) {
	Graphics::Surface icon = _saveSlotThumbnails.getSubArea(Common::Rect(
		iconIndex * ICONDX,
		startLine,
		iconIndex * ICONDX + ICONDX,
		_saveSlotThumbnails.h
	));
	copyToScreenBuffer(&icon, iconSlot * ICONDX + ICONMARGSX, FIRSTLINE);
}

void GraphicsManager::clearScreenBuffer() {
	_screenBuffer.fillRect(Common::Rect(0, 0, MAXX, MAXY), 0);
}

void GraphicsManager::clearScreenBufferTop() {
	// Clears lines 0 - 60
	_screenBuffer.fillRect(Common::Rect(0, 0, MAXX, TOP), 0);
}

void GraphicsManager::clearScreenBufferInventory() {
	// Clears lines 420 - 480
	_screenBuffer.fillRect(Common::Rect(0, FIRSTLINE, MAXX, MAXY), 0);
}

void GraphicsManager::clearScreenBufferSaveSlotDescriptions() {
	// Clears lines 470 - 480
	_screenBuffer.fillRect(Common::Rect(0, FIRSTLINE + ICONDY + 10, MAXX, MAXY), 0);
}

uint16 GraphicsManager::palTo16bit(uint8 r, uint8 g, uint8 b) const {
	return (uint16)_screenFormat.RGBToColor(r, g, b);
}

void GraphicsManager::updatePixelFormat(uint16 *p, uint32 len) const {
	if (_screenFormat == kImageFormat)
		return;

	uint8 r, g, b;
	for (uint32 a = 0; a < len; a++) {
		const uint16 t = p[a];
		kImageFormat.colorToRGB(t, r, g, b);
		p[a] = _screenFormat.RGBToColor(r, g, b);
	}
}

/*------------------------------------------------
					Shadow Pixel
				(dark) 0..8 (light)
--------------------------------------------------*/
uint16 GraphicsManager::shadow(uint32 val, uint8 num) {
	return ((((val & _bitMask[2]) * num >> 7) & _bitMask[2]) |
			(((val & _bitMask[1]) * num >> 7) & _bitMask[1]) |
			(((val & _bitMask[0]) * num >> 7) & _bitMask[0]));
}

void GraphicsManager::pixelAliasing(uint16 x, uint16 y) {
	int px1 = _screenBuffer.getPixel(x - 1, y);
	int px2 = _screenBuffer.getPixel(x, y);

	_screenBuffer.setPixel(x - 1, y, aliasing(px1, px2, 6));      // 75% 25%
	_screenBuffer.setPixel(x, y, aliasing(px1, px2, 2));            // 25% 75%
}

/*------------------------------------------------
					Aliasing Pixel
--------------------------------------------------*/
uint16 GraphicsManager::aliasing(uint32 val1, uint32 val2, uint8 num) {
	// 0:   0% val1 100% val2
	// 1:  12% val1  87% val2
	// 2:  25% val1  75% val2
	// 3:  37% val1  62% val2
	// 4:  50% val1  50% val2
	// 5:  62% val1  37% val2
	// 6:  75% val1  25% val2
	// 7:  87% val1  12% val2
	// 8: 100% val1   0% val2

	return (((((val1 & _bitMask[2]) * num + (val2 & _bitMask[2]) * (8 - num)) >> 3) & _bitMask[2]) |
			((((val1 & _bitMask[1]) * num + (val2 & _bitMask[1]) * (8 - num)) >> 3) & _bitMask[1]) |
			((((val1 & _bitMask[0]) * num + (val2 & _bitMask[0]) * (8 - num)) >> 3) & _bitMask[0]));
}

void GraphicsManager::dissolve(uint8 val) {
	uint16 centerX = MAXX / 2;
	uint16 centerY = MAXY / 2;

	int lastv = 9000;

	uint32 sv = _vm->readTime();
	uint32 cv = _vm->readTime();
	
	while (sv + val > cv) {
		_vm->checkSystem();
		if (lastv + cv < sv + val)
			continue;

		lastv = (sv - cv) + val;

		const float a = (float)(((centerX + 200) / val) * lastv);
		const float b = (float)((centerY / val) * lastv);

		float x = 0.0f;
		float y = b;

		if (centerY - (int)y > TOP)
			memset(_screenBuffer.getBasePtr(0, TOP), 0, (centerY - (int)y - TOP) * MAXX * 2);
		if ((AREA + TOP) > centerY + (int)y)
			memset(_screenBuffer.getBasePtr(0, centerY + (int)y), 0, (AREA + TOP - (centerY + (int)y)) * MAXX * 2);

		float d1 = b * b - a * a * b + a * a / 4.0f;
		while (a * a * (y - 0.5f) > b * b * (x + 1.0f)) {
			if (d1 < 0.0f)
				d1 += b * b * (2.0f * x + 3.0f);
			else {
				d1 += b * b * (2.0f * x + 3.0f) + a * a * (-2.0f * y + 2.0f);
				y -= 1.0f;
			}
			x += 1.0f;

			int rightX = centerX + (int)x;
			if (rightX < MAXX) {
				memset(_screenBuffer.getBasePtr(rightX, centerY + (int)y), 0, (MAXX - rightX) * 2);
				memset(_screenBuffer.getBasePtr(rightX, centerY - (int)y), 0, (MAXX - rightX) * 2);
			}
			int leftX = centerX - (int)x;
			if (leftX > 0) {
				memset(_screenBuffer.getBasePtr(0, centerY + (int)y), 0, leftX * 2);
				memset(_screenBuffer.getBasePtr(0, centerY - (int)y), 0, leftX * 2);
			}
		}

		float d2 = b * b * (x + 0.5f) * (x + 0.5f) + a * a * (y - 1.0f) * (y - 1.0f) - a * a * b * b;
		while (y > 0.0f) {
			if (d2 < 0.0f) {
				d2 += b * b * (2.0f * x + 2.0f) + a * a * (-2.0f * y + 3.0f);
				x += 1.0f;
			} else
				d2 += a * a * (-2.0f * y + 3.0f);
			y -= 1.0f;

			int rightX = centerX + (int)x;
			if (rightX < MAXX) {
				memset(_screenBuffer.getBasePtr(rightX, centerY + (int)y), 0, (MAXX - rightX) * 2);
				memset(_screenBuffer.getBasePtr(rightX, centerY - (int)y), 0, (MAXX - rightX) * 2);
			}
			int leftX = centerX - (int)x;
			if (leftX > 0) {
				memset(_screenBuffer.getBasePtr(0, centerY + (int)y), 0, leftX * 2);
				memset(_screenBuffer.getBasePtr(0, centerY - (int)y), 0, leftX * 2);
			}
		}

		copyToScreen(0, 0, MAXX, MAXY);
		cv = _vm->readTime();
	}

	clearScreen();
}

void GraphicsManager::drawObj(SDObj d) {
	if (d.l.left > MAXX || d.l.top > MAXX || d.l.right > MAXX || d.l.bottom > MAXX)
		return;

	// If we have a valid object, draw it, otherwise erase it
	// by using the background buffer
	const uint16 *buf = d.objIndex >= 0 ? _vm->_objPointers[d.objIndex] : (uint16 *)_background.getPixels();
	if (d.drawMask) {
		uint8 *mask = _vm->_maskPointers[d.objIndex];

		for (uint16 b = d.rect.top; b < d.rect.bottom; b++) {
			uint16 sco = 0;
			uint16 c = 0;
			while (sco < d.rect.width()) {
				if (c == 0) { // jump
					sco += *mask;
					mask++;

					c = 1;
				} else { // copy
					const uint16 maskOffset = *mask;

					if (maskOffset != 0 && b >= d.rect.top + d.l.top && b < d.rect.top + d.l.bottom) {
						if (sco >= d.l.left && sco + maskOffset < d.l.right)
							memcpy(_screenBuffer.getBasePtr(sco + d.rect.left, b), buf, maskOffset * 2);

						else if (sco < d.l.left && sco + maskOffset < d.l.right && sco + maskOffset >= d.l.left)
							memcpy(_screenBuffer.getBasePtr(d.l.left + d.rect.left, b), buf + d.l.left - sco, (maskOffset + sco - d.l.left) * 2);

						else if (sco >= d.l.left && sco + maskOffset >= d.l.right && sco < d.l.right)
							memcpy(_screenBuffer.getBasePtr(sco + d.rect.left, b), buf, (d.l.right - sco) * 2);

						else if (sco < d.l.left && sco + maskOffset >= d.l.right)
							memcpy(_screenBuffer.getBasePtr(d.l.left + d.rect.left, b), buf + d.l.left - sco, (d.l.right - d.l.left) * 2);
					}
					sco += *mask;
					buf += *mask++;
					c = 0;
				}
			}
		}
	} else {
		for (uint16 b = d.l.top; b < d.l.bottom; b++) {
			memcpy(_screenBuffer.getBasePtr(d.rect.left + d.l.left, d.rect.top + b),
				   buf + (b * d.rect.width()) + d.l.left, d.l.width() * 2);
		}
	}
}

void GraphicsManager::eraseObj(SDObj d) {
	_screenBuffer.fillRect(Common::Rect(d.l.left, d.l.top + TOP, d.l.right, d.l.bottom + TOP), 0);
}

uint16 GraphicsManager::getCharWidth(byte character) {
	return _font[character * 3 + 2];
}

void GraphicsManager::drawChar(byte curChar, uint16 sColor, uint16 tColor, uint16 line, Common::Rect rect, Common::Rect subtitleRect, uint16 inc, Graphics::Surface *externalSurface) {
	const uint16 charOffset = _font[curChar * 3] + (uint16)(_font[curChar * 3 + 1] << 8);
	uint16 fontDataOffset = 768;
	const uint16 charWidth = getCharWidth(curChar);

	for (uint16 y = line * CARHEI; y < (line + 1) * CARHEI; y++) {
		uint16 curPos = 0;
		uint16 curColor = sColor;

		while (curPos <= charWidth - 1) {
			if (y >= subtitleRect.top && y < subtitleRect.bottom) {
				if (curColor != MASKCOL && (_font[charOffset + fontDataOffset])) {
					const uint16 charLeft = inc + curPos;
					const uint16 charRight = charLeft + _font[charOffset + fontDataOffset];
					drawCharPixel(
						y,
						charLeft,
						charRight,
						rect,
						subtitleRect,
						curColor,
						externalSurface
					);
				}
			}

			curPos += _font[charOffset + fontDataOffset];
			fontDataOffset++;

			if (curColor == sColor)
				curColor = 0;
			else if (curColor == 0)
				curColor = tColor;
			else if (curColor == tColor)
				curColor = sColor;
		}
	}
}

void GraphicsManager::drawCharPixel(uint16 y, uint16 charLeft, uint16 charRight, Common::Rect rect, Common::Rect subtitleRect, uint16 color, Graphics::Surface *externalSurface) {
	Graphics::Surface *surface = externalSurface ? externalSurface : &_screenBuffer;
	uint16 *dst1 = (uint16 *)surface->getBasePtr(rect.left + charLeft, rect.top + y);
	uint16 *dst2 = (uint16 *)surface->getBasePtr(rect.left + subtitleRect.left, rect.top + y);
	uint16 *dst = nullptr;
	uint16 size = 0;

	if (charLeft >= subtitleRect.left && charRight < subtitleRect.right) {
		dst = dst1;
		size = charRight - charLeft;
	} else if (charLeft < subtitleRect.left && charRight < subtitleRect.right && charRight > subtitleRect.left) {
		dst = dst2;
		size = charRight - subtitleRect.left;
	} else if (charLeft >= subtitleRect.left && charRight >= subtitleRect.right && subtitleRect.right > charLeft) {
		dst = dst1;
		size = subtitleRect.right - charLeft;
	} else if (charLeft < subtitleRect.left && charRight >= subtitleRect.right && subtitleRect.right > charLeft) {
		dst = dst2;
		size = subtitleRect.right - subtitleRect.left;
	}

	if (dst && size > 0) {
		uint16 *d = dst;
		for (uint32 i = 0; i < size; i++)
			*d++ = color;
	}
}

void GraphicsManager::initCursor() {
	const int cw = 21, ch = 21;
	const int cx = 10, cy = 10;
	uint16 cursor[cw * ch];
	memset(cursor, 0, ARRAYSIZE(cursor) * 2);

	const uint16 cursorColor = palTo16bit(255, 255, 255);

	for (int i = 0; i < cw; i++) {
		if (i >= 8 && i <= 12 && i != 10)
			continue;
		cursor[cx * cw + i] = cursorColor; // horizontal
		cursor[cx + cw * i] = cursorColor; // vertical
	}

	CursorMan.pushCursor(cursor, cw, ch, cx, cy, 0, false, &_screenFormat);
}

void GraphicsManager::showCursor() {
	CursorMan.showMouse(true);
}

void GraphicsManager::hideCursor() {
	CursorMan.showMouse(false);
}

bool GraphicsManager::isCursorVisible() {
	return CursorMan.isVisible();
}

void GraphicsManager::showDemoPic() {
	Common::File file;
	if (file.open("EndPic.bm")) {
		readSurface(&file, &_screenBuffer, MAXX, MAXY);
		copyToScreen(0, 0, MAXX, MAXY);
		g_system->updateScreen();

		_vm->freeKey();
		_vm->_mouseLeftBtn = _vm->_mouseRightBtn = false;
		_vm->waitKey();
	}
}

} // End of namespace Trecision
