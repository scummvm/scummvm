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
	_surface = new Surface(kScreenWidth, kScreenHeight);
	_palette = new byte[4 * kNumColours];
	setPaletteEmpty();
	this->clearScreen();
}

Screen::~Screen() {
	delete _surface;
	delete[] _palette;
}

void Screen::setPaletteEmpty(unsigned int numEntries) {
	for (unsigned int i = 0; i < 4 * numEntries; ++i) {
		_palette[i] = 0;
	}

	_vm->_system->setPalette(_palette, 0, numEntries);
	copyToScreen();
}	

void Screen::setPalette(byte *data, uint16 start, uint16 num) {

	Common::MemoryReadStream pal(data, 3 * kNumColours);
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
	for (unsigned int i = 0; i < 4 * kNumColours; ++i) {
		_palette[i] <<= 2;
	}

	_vm->_system->setPalette(_palette, start, num);
	copyToScreen();
}

void Screen::copyToScreen() const {
	Common::List<Common::Rect> *dirtyRects = _surface->getDirtyRects();
	Common::List<Common::Rect>::iterator it;
	
	if (_surface->needsFullUpdate()) {
		byte *ptr = (byte *)_surface->getBasePtr(0, 0);

		_vm->_system->copyRectToScreen(ptr, kScreenWidth, 
			0, 0, kScreenWidth, kScreenHeight);
	} else {
		for (it = dirtyRects->begin(); it != dirtyRects->end(); ++it) {
			byte *ptr = (byte *)_surface->getBasePtr(it->left, it->top);

			_vm->_system->copyRectToScreen(ptr, kScreenWidth, 
				it->left, it->top, it->width(), it->height());
		}
	}
	
	_vm->_system->updateScreen();
	_surface->markClean();
}


void Screen::clearScreen() const {
	byte *ptr = (byte *)_surface->getBasePtr(0, 0);

	_surface->markDirty();

	memset(ptr, 0, kScreenWidth * kScreenHeight);
}

void Screen::fillScreen(uint16 colour) const {
	byte *ptr = (byte *)_surface->getBasePtr(0, 0);

	_surface->markDirty();

	memset(ptr, colour, kScreenWidth * kScreenHeight);
}

void Screen::drawRect(Common::Rect &r, uint8 colour) {

	r.clip(_surface->w, _surface->h);

	if (r.isEmpty())
		return;

	byte *ptr = (byte *)_surface->getBasePtr(r.left, r.top);

	for (uint16 i = 0; i < r.width(); ++i) {
		for (uint16 j = 0; j < r.height(); ++j) {
			ptr[j * kScreenWidth + i] = colour;
		}
	}

	_surface->markDirtyRect(r);
}

byte *Screen::getPalette() const {
	return _palette;
}

Draci::Surface *Screen::getSurface() {
	return _surface;
}

} // End of namespace Draci


