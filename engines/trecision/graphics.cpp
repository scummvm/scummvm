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
	_screenPtr = nullptr;
}

GraphicsManager::~GraphicsManager() {}

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
	return true;
}

/*------------------------------------------------
					lock
--------------------------------------------------*/
void GraphicsManager::lock() {
	if (!_screenPtr) {
		Graphics::Surface *surface = g_system->lockScreen();
		_screenPtr = (uint16 *)surface->getPixels();
	}
}

/*------------------------------------------------
					unlock
--------------------------------------------------*/
void GraphicsManager::unlock() {
	if (_screenPtr) {
		g_system->unlockScreen();
		_screenPtr = nullptr;
	}
}

/*------------------------------------------------
					clearScreen
--------------------------------------------------*/
void GraphicsManager::clearScreen() {
	g_system->fillScreen(0);
}

#if USE_DIRTY_RECTS
/*------------------------------------------------
	vCopy - Execute fast copy to video 
--------------------------------------------------*/
void GraphicsManager::vCopy(uint32 Sco, uint16 *Src, uint32 Len) {
	if (_screenPtr == nullptr || Len == 0)
		return;

	memcpy(_screenPtr + Sco, Src, Len * 2);
}
#endif

void GraphicsManager::copyToScreen(int px, int py, int dx, int dy) {
	g_system->copyRectToScreen(
		_vm->_screenBuffer + px + py * SCREENLEN,
		MAXX * 2, px, py, dx, dy
	);
}

/* ------------------------------------------------
					palTo16bit
 --------------------------------------------------*/
uint16 GraphicsManager::palTo16bit(uint8 r, uint8 g, uint8 b) const {
	return (uint16)_screenFormat.RGBToColor(r, g, b);
}

/* ------------------------------------------------
				updatePixelFormat
 --------------------------------------------------*/
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

/* ------------------------------------------------
				restorePixelFormat
 --------------------------------------------------*/
uint16 GraphicsManager::restorePixelFormat(uint16 t) const {
	if (_screenFormat == kImageFormat)
		return t;

	uint8 r, g, b;
	_screenFormat.colorToRGB(t, r, g, b);
	return (uint16)kImageFormat.RGBToColor(r, g, b);
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

} // end of namespace
