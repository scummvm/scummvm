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

#include "trecision/graphics.h"
#include "trecision/trecision.h"

#include "common/system.h"
#include "engines/util.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

namespace Trecision {

const Graphics::PixelFormat GraphicsManager::kImageFormat(2, 5, 5, 5, 0, 10, 5, 0, 0); // RGB555

GraphicsManager::GraphicsManager(TrecisionEngine *vm) : _vm(vm) {
}

GraphicsManager::~GraphicsManager() {
	_background.free();
	_smkBackground.free();
}

bool GraphicsManager::initScreen() {
	const Graphics::PixelFormat *bestFormat = &kImageFormat;

	// Find a 16-bit format, currently we don't support other color depths
	Common::List<Graphics::PixelFormat> formats = g_system->getSupportedFormats();
	for (Common::List<Graphics::PixelFormat>::const_iterator i = formats.begin(); i != formats.end(); ++i) {
		if (i->bytesPerPixel == 2) {
			bestFormat = &*i;
			break;
		}
	}
	initGraphics(MAXX, MAXY, bestFormat);

	_screenFormat = g_system->getScreenFormat();
	if (_screenFormat.bytesPerPixel != 2)
		return false;
	_bitMask[0] = _screenFormat.rMax() << _screenFormat.rShift;
	_bitMask[1] = _screenFormat.gMax() << _screenFormat.gShift;
	_bitMask[2] = _screenFormat.bMax() << _screenFormat.bShift;

	clearScreen();

	_background.create(MAXX, MAXY, _screenFormat);
	_smkBackground.create(MAXX, AREA, _screenFormat);

	return true;
}

void GraphicsManager::clearScreen() {
	g_system->fillScreen(0);
}

void GraphicsManager::copyToScreenBuffer(Graphics::Surface *surface, int x, int y) {
	for (int curY = 0; curY < surface->h; curY++) {
		memcpy(_vm->_screenBuffer + x + (y + curY) * MAXX, surface->getBasePtr(0, curY), surface->pitch);
	}
}

void GraphicsManager::blitToScreenBuffer(Graphics::Surface *surface, int x, int y, uint16 mask, bool useSmkBg) {
	for (int curY = 0; curY < surface->h; curY++) {
		for (int curX = 0; curX < surface->w; curX++) {
			const int destX = x + curX;
			const int destY = y + curY;
			uint16 *dest = _vm->_screenBuffer + destX + destY * MAXX;
			const uint16 pixel = (uint16)surface->getPixel(curX, curY);
			if (pixel != mask) {
				*dest = pixel;
			} else if (useSmkBg) {
				*dest = (uint16)_smkBackground.getPixel(destX, destY - TOP);
			}
		}
	}
}

void GraphicsManager::copyToScreen(int x, int y, int w, int h) {
	g_system->copyRectToScreen(
		_vm->_screenBuffer + x + y * MAXX,
		MAXX * 2, x, y, w, h
	);
}

void GraphicsManager::setSmkBackground() {
	_smkBackground.copyFrom(_background);
}

void GraphicsManager::resetScreenBuffer() {
	memset(_vm->_screenBuffer, 0, TOP * MAXX * 2);
	memcpy(_vm->_screenBuffer + TOP * MAXX, _background.getPixels(), _background.pitch * AREA);
}

uint16 *GraphicsManager::getBackgroundPtr() {
	return (uint16 *)_background.getPixels();
}

void GraphicsManager::loadBackground(Common::SeekableReadStream *stream, uint16 width, uint16 height) {
	Graphics::Surface img;
	img.create(width, height, kImageFormat);

	uint16 *buffer = new uint16[width * height];
	for (int i = 0; i < width * height; ++i)
		buffer[i] = stream->readUint16LE();

	memcpy(img.getPixels(), buffer, img.pitch * img.h);
	img.convertToInPlace(_screenFormat);
	_background.copyFrom(img);
	delete[] buffer;
	img.free();
}

void GraphicsManager::putPixel(int x, int y, uint16 color) {
	if (x > 0 && x < MAXX && y > 60 && y < 420) {
		_vm->_screenBuffer[x + MAXX * y] = color;
		_background.setPixel(x, y - 60, color);
		_smkBackground.setPixel(x, y - 60, color);
	}
}

void GraphicsManager::drawLine(int x1, int y1, int x2, int y2, uint16 color) {
	int deltaX = ABS(x2 - x1);
	int deltaY = ABS(y2 - y1);

	int x = x1;
	int y = y1;

	const int incX = (x1 < x2) ? 1 : -1;
	const int incY = (y1 < y2) ? 1 : -1;

	if (deltaX < deltaY) {
		int d = (deltaX << 1) - deltaY;
		const int delta = (deltaX - deltaY) << 1;
		const int numCycles = deltaY + 1;

		for (int cycle = 0; cycle < numCycles; cycle++) {
			if (x >= 0 && x < MAXX && y >= 0 && y < MAXY)
				putPixel(x, y, color);

			if (d < 0)
				d += (deltaX << 1);
			else {
				d += delta;
				x += incX;
			}
			y += incY;
		}
	} else {
		int d = (deltaY << 1) - deltaX;
		const int delta = (deltaY - deltaX) << 1;
		const int numCycles = deltaX + 1;

		for (int cycle = 0; cycle < numCycles; cycle++) {
			if (x >= 0 && x < MAXX && y >= 0 && y < MAXY)
				putPixel(x, y, color);
			if (d < 0)
				d += (deltaY << 1);
			else {
				d += delta;
				y += incY;
			}
			x += incX;
		}
	}
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

void GraphicsManager::NlDissolve(uint8 val) {
	uint16 CenterX = MAXX / 2;
	uint16 CenterY = MAXY / 2;

	int lastv = 9000;

	uint32 sv = _vm->readTime();
	uint32 cv = _vm->readTime();
	
	while (sv + val > cv) {
		_vm->checkSystem();
		if (lastv + cv < sv + val)
			continue;

		lastv = (sv - cv) + val;

		float a = (float)(((CenterX + 200) / val) * lastv);
		float b = (float)((CenterY / val) * lastv);

		float x = 0.0f;
		float y = b;

		if (CenterY - (int)y > TOP)
			memset(_vm->_screenBuffer + TOP*MAXX, 0, (CenterY - (int)y - TOP) * MAXX * 2);
		if ((AREA + TOP) > CenterY + (int)y)
			memset(_vm->_screenBuffer + (CenterY + (int)y) * MAXX, 0, (AREA + TOP - (CenterY + (int)y)) * MAXX * 2);

		float d1 = b * b - a * a * b + a * a / 4.0f;
		while (a * a * (y - 0.5f) > b * b * (x + 1.0f)) {
			if (d1 < 0.0f)
				d1 += b * b * (2.0f * x + 3.0f);
			else {
				d1 += b * b * (2.0f * x + 3.0f) + a * a * (-2.0f * y + 2.0f);
				y -= 1.0f;
			}
			x += 1.0f;

			int rightX = CenterX + (int)x;
			if (rightX < MAXX) {
				memset(_vm->_screenBuffer + rightX + (CenterY + (int)y) * MAXX, 0, (MAXX - rightX) * 2);
				memset(_vm->_screenBuffer + rightX + (CenterY - (int)y) * MAXX, 0, (MAXX - rightX) * 2);
			}
			int leftX = CenterX - (int)x;
			if (leftX > 0) {
				memset(_vm->_screenBuffer + (CenterY + (int)y) * MAXX, 0, leftX * 2);
				memset(_vm->_screenBuffer + (CenterY - (int)y) * MAXX, 0, leftX * 2);
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

			int rightX = CenterX + (int)x;
			if (rightX < MAXX) {
				memset(_vm->_screenBuffer + rightX + (CenterY + (int)y) * MAXX, 0, (MAXX - rightX) * 2);
				memset(_vm->_screenBuffer + rightX + (CenterY - (int)y) * MAXX, 0, (MAXX - rightX) * 2);
			}
			int leftX = CenterX - (int)x;
			if (leftX > 0) {
				memset(_vm->_screenBuffer + (CenterY + (int)y) * MAXX, 0, leftX * 2);
				memset(_vm->_screenBuffer + (CenterY - (int)y) * MAXX, 0, leftX * 2);
			}
		}

		copyToScreen(0, 0, MAXX, MAXY);
		cv = _vm->readTime();
	}

	clearScreen();
}

void GraphicsManager::DrawObj(SDObj d) {
	if (d.l.left > MAXX || d.l.top > MAXX || d.l.right > MAXX || d.l.bottom > MAXX)
		return;

	const uint16 *buf = d.objIndex >= 0 ? _vm->_objPointers[d.objIndex] : getBackgroundPtr();
	if (d.drawMask) {
		uint8 *mask = _vm->_maskPointers[d.objIndex];

		for (uint16 b = d.rect.top; b < d.rect.bottom; b++) {
			uint16 Sco = 0;
			uint16 c = 0;
			while (Sco < d.rect.width()) {
				if (c == 0) { // jump
					Sco += *mask;
					mask++;

					c = 1;
				} else { // copy
					uint16 maskOffset = *mask;

					if ((maskOffset != 0) && (b >= (d.rect.top + d.l.top)) && (b < (d.rect.top + d.l.bottom))) {
						if ((Sco >= d.l.left) && ((Sco + maskOffset) < d.l.right))
							memcpy(_vm->_screenBuffer + (b * MAXX) + Sco + d.rect.left, buf, maskOffset * 2);

						else if ((Sco < d.l.left) && ((Sco + maskOffset) < d.l.right) && ((Sco + maskOffset) >= d.l.left))
							memcpy(_vm->_screenBuffer + (b * MAXX) + d.l.left + d.rect.left, buf + d.l.left - Sco, (maskOffset + Sco - d.l.left) * 2);

						else if ((Sco >= d.l.left) && ((Sco + maskOffset) >= d.l.right) && (Sco < d.l.right))
							memcpy(_vm->_screenBuffer + (b * MAXX) + Sco + d.rect.left, buf, (d.l.right - Sco) * 2);

						else if ((Sco < d.l.left) && ((Sco + maskOffset) >= d.l.right))
							memcpy(_vm->_screenBuffer + (b * MAXX) + d.l.left + d.rect.left, buf + d.l.left - Sco, (d.l.right - d.l.left) * 2);
					}
					Sco += *mask;
					buf += *mask++;
					c = 0;
				}
			}
		}
	} else {
		for (uint16 b = d.l.top; b < d.l.bottom; b++) {
			memcpy(_vm->_screenBuffer + (d.rect.top + b) * MAXX + (d.rect.left + d.l.left),
				   buf + (b * d.rect.width()) + d.l.left, d.l.width() * 2);
		}
	}
}

} // end of namespace
