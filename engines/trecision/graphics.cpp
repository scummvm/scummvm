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
	_linearMode = true;
	_locked = false;
	_pitch = 0;
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
	if (!_locked) {
		Graphics::Surface *surface = g_system->lockScreen();
		_screenPtr = (uint16 *)surface->getPixels();
		_pitch = surface->pitch;

		_locked = true;
	}
}

/*------------------------------------------------
					unlock
--------------------------------------------------*/
void GraphicsManager::unlock() {
	if (_locked) {
		g_system->unlockScreen();
		_locked = false;
		_pitch = 0;
		_screenPtr = nullptr;
	}
}

/*------------------------------------------------
					clearScreen
--------------------------------------------------*/
void GraphicsManager::clearScreen() {
	g_system->fillScreen(0);
}

/*------------------------------------------------
	vCopy - Execute fast copy to video 
--------------------------------------------------*/
void GraphicsManager::vCopy(uint32 Sco, uint16 *Src, uint32 Len) {
	lock();
	if ((_screenPtr == nullptr) || (Len == 0))
		return;

	if (_linearMode && ((_pitch == 0) || (_pitch == SCREENLEN * 2))) {
		MCopy(_screenPtr + Sco, Src, Len);
		return;
	}

	int32 x1 = Sco % SCREENLEN;
	int32 y1 = Sco / SCREENLEN;

	uint32 endSco = Sco + Len;
	int32 y2 = endSco / SCREENLEN;

	uint32 srcSco = 0;
	uint32 copyNow = MIN<uint32>(Len, SCREENLEN - x1);

	MCopy(_screenPtr + y1 * (_pitch / 2) + x1, Src + srcSco, copyNow);
	srcSco += copyNow;
	Len -= copyNow;

	for (int32 i = (y1 + 1); i <= (y2 - 1); i++) {
		copyNow = SCREENLEN;
		MCopy(_screenPtr + i * (_pitch / 2), Src + srcSco, copyNow);
		srcSco += copyNow;
		Len -= copyNow;
	}

	if (Len > 0) {
		copyNow = Len;
		MCopy(_screenPtr + y2 * (_pitch / 2), Src + srcSco, copyNow);
	}
}

/*------------------------------------------------
					BCopy
--------------------------------------------------*/
void GraphicsManager::BCopy(uint32 Sco, uint8 *Src, uint32 Len) {
	_vm->_graphicsMgr->lock();
	if ((_screenPtr == nullptr) || (Len == 0))
		return;

	if (_vm->_graphicsMgr->_linearMode && ((_pitch == 0) || (_pitch == SCREENLEN * 2))) {
		byte2word(_screenPtr + Sco, Src, _vm->_newData, Len);
		return;
	}

	int32 x1 = Sco % SCREENLEN;
	int32 y1 = Sco / SCREENLEN;

	uint32 EndSco = Sco + Len;

	int32 y2 = EndSco / SCREENLEN;

	uint32 SrcSco = 0;

	uint32 CopyNow = MIN<uint32>(Len, SCREENLEN - x1);

	byte2word(_screenPtr + y1 * (_pitch / 2) + x1, Src + SrcSco, _vm->_newData, CopyNow);
	SrcSco += CopyNow;
	Len -= CopyNow;

	for (int32 i = (y1 + 1); i <= (y2 - 1); i++) {
		CopyNow = SCREENLEN;
		byte2word(_screenPtr + i * (_pitch / 2), Src + SrcSco, _vm->_newData, CopyNow);
		SrcSco += CopyNow;
		Len -= CopyNow;
	}

	if (Len > 0) {
		CopyNow = Len;
		byte2word(_screenPtr + y2 * (_pitch / 2), Src + SrcSco, _vm->_newData, CopyNow);
	}
}
/*------------------------------------------------
					DCopy
--------------------------------------------------*/
void GraphicsManager::DCopy(uint32 Sco, uint8 *Src, uint32 Len) {
	lock();
	if ((_screenPtr == nullptr) || (Len == 0))
		return;

	if (_vm->_graphicsMgr->_linearMode && ((_pitch == 0) || (_pitch == SCREENLEN * 2))) {
		byte2long(_screenPtr + Sco, Src, _vm->_newData2, Len / 2);
		return;
	}

	int32 x1 = Sco % SCREENLEN;
	int32 y1 = Sco / SCREENLEN;

	uint32 EndSco = Sco + Len;

	int32 y2 = EndSco / SCREENLEN;

	uint32 SrcSco = 0;

	uint32 CopyNow = MIN<uint32>(Len, SCREENLEN - x1);

	byte2long(_screenPtr + y1 * (_pitch / 2) + x1, Src + SrcSco, _vm->_newData2, CopyNow / 2);
	SrcSco += CopyNow;
	Len -= CopyNow;

	for (int32 i = (y1 + 1); i <= (y2 - 1); i++) {
		CopyNow = SCREENLEN;
		byte2long(_screenPtr + i * (_pitch / 2), Src + SrcSco, _vm->_newData2, CopyNow / 2);
		SrcSco += CopyNow;
		Len -= CopyNow;
	}

	if (Len > 0) {
		CopyNow = Len;
		byte2long(_screenPtr + y2 * (_pitch / 2), Src + SrcSco, _vm->_newData2, CopyNow / 2);
	}
}

/*------------------------------------------------
					showScreen
--------------------------------------------------*/
void GraphicsManager::showScreen(int px, int py, int dx, int dy) {
	for (int a = 0; a < dy; a++) {
		vCopy(px + (py + a) * VirtualPageLen + VideoScrollPageDx,
		      _vm->_video2 + px + (py + a) * CurRoomMaxX + CurScrollPageDx,
		      dx);
	}

	unlock();
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
		uint16 t = p[a];
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

/*------------------------------------------------
					VPix
--------------------------------------------------*/
void GraphicsManager::VPix(int16 x, int16 y, uint16 col) {
	if (_screenPtr == nullptr)
		return;

	if (_linearMode && ((_pitch == 0) || (_pitch == SCREENLEN * 2))) {
		uint32 a = ((uint32)x + MAXX * (uint32)y);
		if ((x >= 0) && (y >= 0) && (x < CurRoomMaxX) && (y < MAXY))
			_screenPtr[a] = col;
		return;
	}

	if ((x >= 0) && (y >= 0) && (x < CurRoomMaxX) && (y < MAXY))
		_screenPtr[x + y * (_pitch / 2)] = col;
}

} // end of namespace
