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

#include <common/system.h>

#include "engines/util.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "trecision/trecision.h"

namespace Trecision {
GraphicsManager::GraphicsManager(TrecisionEngine *vm) : _vm(vm) {
	const Graphics::PixelFormat kVideoFormat(2, 5, 6, 5, 0, 11, 5, 0, 0); // RGB565
	_linearMode = true;
	_locked = false;
	_pitch = 0;
	_screenPtr = nullptr;
	
	initGraphics(MAXX, MAXY, &kVideoFormat);

	clearScreen();

	_screenFormat = g_system->getScreenFormat();
	_vm->getColorMask(_screenFormat);
}

GraphicsManager::~GraphicsManager() {}

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
	lock();
	if (_screenPtr == nullptr)
		return;
	if ((_pitch == 0) || (_pitch == SCREENLEN * 2))
		longset(_screenPtr, 0x00000000, 320 * 480);
	else
		longset(_screenPtr, 0x00000000, (_pitch / 4) * 480);
	unlock();
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
uint16 GraphicsManager::palTo16bit(uint8 r, uint8 g, uint8 b) {
	return (uint16)_screenFormat.RGBToColor(r, g, b);
}

/* ------------------------------------------------
				updatePixelFormat
 --------------------------------------------------*/
void GraphicsManager::updatePixelFormat(uint16 *p, uint32 len) {
	uint8 r, g, b;

	for (int a = 0; a < len; a++) {
		uint16 t = p[a];
		color2RGB(t, &r, &g, &b);
		p[a] = _screenFormat.RGBToColor(r, g, b);
	}
}

/* ------------------------------------------------
				restorePixelFormat
 --------------------------------------------------*/
uint16 GraphicsManager::restorePixelFormat(uint16 t) {
	uint8 r, g, b;
	_screenFormat.colorToRGB(t, r, g, b);
	return (RGB2Color(r, g, b));
}

/*------------------------------------------------
  color2RGB - Returns a 8bit RGB from a high color
--------------------------------------------------*/
void GraphicsManager::color2RGB(uint16 a, uint8 *r, uint8 *g, uint8 *b) {
	*r = (uint8)(((uint16)((uint16)a >> 10L) & 0x1F) << 3);
	*g = (uint8)(((uint16)((uint16)a >> 5L) & 0x1F) << 3);
	*b = (uint8)(((uint16)((uint16)a) & 0x1F) << 3);
}

/*------------------------------------------------
  ColorRGB - Returns a high color from a 8bit RGB
--------------------------------------------------*/
uint16 GraphicsManager::RGB2Color(uint8 r, uint8 g, uint8 b) {
	r >>= 3;
	g >>= 3;
	b >>= 3;

	uint16 a = (uint16)(((uint16)(b & 0x1F) & 0x1F) +
	                  ((uint16)((uint16)(g & 0x1F) << 5L) & 0x3E0) +
	                  ((uint16)((uint16)(r & 0x1F) << 10L) & 0x7C00));

	return a;
}

} // end of namespace
