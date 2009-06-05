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

	_system = system;
	_updatePalette = false;
	_fullRefresh = false;

	_frontBuf = (uint8 *)malloc(sizeof(uint8)*SCREEN_WIDTH * SCREEN_DEPTH);
	_backBuf = (uint8 *)malloc(sizeof(uint8)*SCREEN_WIDTH * SCREEN_DEPTH);
}

Screen::~Screen(){
	if(_frontBuf){
		free(_frontBuf);
	}
	if(_backBuf){
		free(_backBuf);
	}
}

void Screen::setFrontBuffer(int x, int y, int width, int height, uint8 *buffer){
	copyBuffer(x, y, width, height, _frontBuf, buffer);
}

void Screen::setBackBuffer(int x, int y, int width, int height, uint8 *buffer){
	copyBuffer(x, y, width, height, _backBuf, buffer);
}

void Screen::copyBuffer(int x, int y, int width, int height, uint8 *src, uint8 *dst){
	copyBuffer(x, y, 0, 0, width, height, src, dst);
}

void Screen::copyBuffer(int xs, int ys, int xd, int yd, int width, int height, uint8 *src, uint8 *dst){
	
}


void Screen::clearScreen(){
	memset(_frontBuf, 0, sizeof(uint8)*SCREEN_WIDTH * SCREEN_DEPTH);
	updateScreen();
}

void Screen::updateScreen(){
	updateScreen(0, 0, SCREEN_WIDTH, SCREEN_DEPTH);
}

void Screen::updateScreen(int x, int y, int width, int height){
	_system->copyRectToScreen(_frontBuf, SCREEN_WIDTH, x, y, width, height);
	_system->updateScreen();
}

void Screen::setPalette(uint8 *palette){
	uint8 *p = _currentPalette;

	for (int i = 0; i < PAL_SIZE; i++) {
		*p++ = *palette++;
		*p++ = *palette++;
		*p++ = *palette++;
		*p++ = 0;
	}

	_system->setPalette(_currentPalette, 0, PAL_SIZE);
}

void Screen::drawLine(int x0, int y0, int x1, int y1, uint8 colour){
}

} // end of namespace Asylum
