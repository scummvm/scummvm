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
 */

#include "common/system.h"
#include "engines/engine.h"

#include "asylum/screen.h"

namespace Asylum {

Screen::Screen(OSystem *system){
	initGraphics(SCREEN_WIDTH, SCREEN_DEPTH, true);

	_system        = system;
	_updatePalette = false;
	_fullRefresh   = false;

	_frontBuf = (uint8 *)malloc(sizeof(uint8)*SCREEN_WIDTH * SCREEN_DEPTH);
	_backBuf  = (uint8 *)malloc(sizeof(uint8)*SCREEN_WIDTH * SCREEN_DEPTH);
}

Screen::~Screen(){
	if(_frontBuf){
		free(_frontBuf);
	}
	if(_backBuf){
		free(_backBuf);
	}
}

void Screen::setFrontBuffer(int32 x, int32 y, int32 width, int32 height, uint8 *buffer){
	copyBuffer(x, y, width, height, buffer, _frontBuf);
}

void Screen::setBackBuffer(int32 x, int32 y, int32 width, int32 height, uint8 *buffer){
	copyBuffer(x, y, width, height, buffer, _backBuf);
}

void Screen::copyBuffer(int32 x, int32 y, int32 width, int32 height, uint8 *src, uint8 *dst){
	copyBuffer(x, y, 0, 0, width, height, src, dst);
}

void Screen::copyBuffer(int32 xs, int32 ys, int32 xd, int32 yd, int32 width, int32 height, uint8 *src, uint8 *dst){
	copyBuffer(xs, ys, 0, 0, width, height, src, dst, SCREEN_WIDTH);
}

void Screen::copyBuffer(int32 xs, int32 ys, int32 xd, int32 yd, int32 width, int32 height, uint8 *src, uint8 *dst, int32 dstWidth ){
	src += xs + ys * SCREEN_WIDTH;
	dst += xd + yd * dstWidth; // destination width

	while(height--){
		memcpy(dst, src, width);
		src += SCREEN_WIDTH;
		dst += dstWidth;
	}
}

void Screen::clearScreen(){
	memset(_frontBuf, 0, sizeof(uint8)*SCREEN_WIDTH * SCREEN_DEPTH);
	updateScreen();
}

void Screen::updateScreen(){
	updateScreen(0, 0, SCREEN_WIDTH, SCREEN_DEPTH);
}

void Screen::updateScreen(int32 x, int32 y, int32 width, int32 height){
	_system->copyRectToScreen(_frontBuf, SCREEN_WIDTH, x, y, width, height);
	_system->updateScreen();
}

void Screen::setPalette(uint8 *palette){
	uint8 *p = _currentPalette;

	for (int i = 0; i < PAL_SIZE; i++) {
		*p++ = *palette++ * BRIGHTNESS;
		*p++ = *palette++ * BRIGHTNESS;
		*p++ = *palette++ * BRIGHTNESS;
		*p++ = 0;
	}

	//setGammaLevel(0.9);

	_system->setPalette(_currentPalette, 0, PAL_SIZE);
}

// FIXME: improve gamma correction
void Screen::setGammaLevel(double gamma){
	uint8 LUT[768];
	for(int g=0;g<768;++g)
		LUT[g] = (unsigned char) (255.f * pow((double)g/255.0, gamma));
	for(int i=0;i<768;++i)
		_currentPalette[i] = LUT[ _currentPalette[i] ];
}

void Screen::drawLine(int32 x0, int32 y0, int32 x1, int32 y1, uint8 colour){
}

} // end of namespace Asylum
