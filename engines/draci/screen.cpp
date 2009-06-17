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
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stream.h"

#include "draci/draci.h"
#include "draci/screen.h"

namespace Draci {

Screen::Screen(DraciEngine *vm) : _vm(vm) {
	_surface = new Graphics::Surface();
	_surface->create(_vm->_screenWidth, _vm->_screenHeight, 1);
	this->clearScreen();
	_palette = new byte[4 * 256];
	setPaletteEmpty(256);
}

Screen::~Screen() {
	_surface->free();
	delete[] _surface;
	delete[] _palette;
}

void Screen::setPaletteEmpty(unsigned int numEntries) {
	for (unsigned int i = 0; i < numEntries * 4; ++i) {
		_palette[i] = 0;
	}

	_vm->_system->setPalette(_palette, 0, numEntries);
	_vm->_system->updateScreen();
}	

void Screen::setPalette(byte *data, uint16 start, uint16 num) {

	Common::MemoryReadStream pal(data, 256 * 3);
	pal.seek(start * 4);

	// Copy the palette	
	for (unsigned int i = start; i < start + num; ++i) {
		_palette[i * 4] = pal.readByte();
		_palette[i * 4 + 1] = pal.readByte();
		_palette[i * 4 + 2] = pal.readByte();
		_palette[i * 4 + 3] = 0;
	}

	// TODO: Investigate why this is needed
	// Shift the palette one bit to the left to make it brighter
	for (unsigned int i = 0; i < 4 * 256; ++i) {
		_palette[i] <<= 2;
	}

	_vm->_system->setPalette(_palette, start, num);
	_vm->_system->updateScreen(); 
}

void Screen::copyToScreen() const {
	byte *ptr = (byte *)_surface->getBasePtr(0, 0);

	_vm->_system->copyRectToScreen(ptr, _vm->_screenWidth, 0, 0, 
		_vm->_screenWidth, _vm->_screenHeight);

	_vm->_system->updateScreen();
}


void Screen::clearScreen() const {
	byte *ptr = (byte *)_surface->getBasePtr(0, 0);

	memset(ptr, 0, _vm->_screenWidth * _vm->_screenHeight);
}

void Screen::drawSprite(const Sprite &s) const { 
	byte *dst = (byte *)_surface->getBasePtr(s._x, s._y);
	byte *src = s._data;	

	for (unsigned int i = 0; i < s._height; ++i) {
		for(unsigned int j = 0; j < s._width; ++j) {
			dst[j] = *src++;
		}
		
		dst += _surface->pitch;
	}
}

void Screen::fillScreen(uint16 colour) const {
	byte *ptr = (byte *)_surface->getBasePtr(0, 0);
	
	memset(ptr, colour, _vm->_screenWidth * _vm->_screenHeight);
}

byte *Screen::getPalette() const {
	return _palette;
}

Graphics::Surface *Screen::getSurface() {
	return _surface;
}

} // End of namespace Draci


