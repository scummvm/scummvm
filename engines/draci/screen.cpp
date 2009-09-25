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

/**
 * @brief Sets the first numEntries of palette to zero
 * @param numEntries The number of entries to set to zero (from start)
 */
void Screen::setPaletteEmpty(unsigned int numEntries) {
	for (unsigned int i = 0; i < 4 * numEntries; ++i) {
		_palette[i] = 0;
	}

	_vm->_system->setPalette(_palette, 0, numEntries);
}	

/**
 * @brief Sets a part of the palette
 * @param data Pointer to a buffer containing new palette data
 *		  start Index of the colour where replacement should start
 *		  num Number of colours to replace 
 */
void Screen::setPalette(const byte *data, uint16 start, uint16 num) {

	Common::MemoryReadStream pal(data, 3 * kNumColours);
	pal.seek(start * 4);

	// Copy the palette	
	for (uint16 i = start; i < start + num; ++i) {
		_palette[i * 4] = pal.readByte();
		_palette[i * 4 + 1] = pal.readByte();
		_palette[i * 4 + 2] = pal.readByte();
		_palette[i * 4 + 3] = 0;
	}

	// TODO: Investigate why this is needed
	// Shift the palette two bits to the left to make it brighter
	for (unsigned int i = 0; i < 4 * kNumColours; ++i) {
		_palette[i] <<= 2;
	}

	_vm->_system->setPalette(_palette, start, num);
}

/**
 * @brief Copies the current memory screen buffer to the real screen
 */
void Screen::copyToScreen() {
	const Common::List<Common::Rect> *dirtyRects = _surface->getDirtyRects();
	Common::List<Common::Rect>::const_iterator it;
	
	// If a full update is needed, update the whole screen	
	if (_surface->needsFullUpdate()) {
		byte *ptr = (byte *)_surface->getBasePtr(0, 0);

		_vm->_system->copyRectToScreen(ptr, kScreenWidth, 
			0, 0, kScreenWidth, kScreenHeight);
	} else {
	
		// Otherwise, update only the dirty rectangles
	
		for (it = dirtyRects->begin(); it != dirtyRects->end(); ++it) {
			
			// Pointer to the upper left corner of the rectangle
			byte *ptr = (byte *)_surface->getBasePtr(it->left, it->top);

			_vm->_system->copyRectToScreen(ptr, kScreenWidth, 
				it->left, it->top, it->width(), it->height());
		}
	}
	
	// Call the "real" updateScreen and mark the surface clean
	_vm->_system->updateScreen();
	_surface->markClean();
}

/**
 * @brief Clears the screen
 *
 * Clears the screen and marks the whole screen dirty.
 */
void Screen::clearScreen() {
	byte *ptr = (byte *)_surface->getBasePtr(0, 0);

	_surface->markDirty();

	memset(ptr, 0, kScreenWidth * kScreenHeight);
}

/**
 * @brief Fills the screen with the specified colour
 * @param colour The colour the screen should be filled with
 *
 * Fills the screen with the specified colour and marks the whole screen dirty.
 */
void Screen::fillScreen(uint8 colour) {
	_surface->fill(colour);
	_surface->markDirty();
}

/**
 * @brief Draws a rectangle on the screen
 * @param r Which rectangle to draw
 *		  colour The colour of the rectangle
 */
void Screen::drawRect(Common::Rect r, uint8 colour) {

	// Clip the rectangle to screen size
	r.clip(_surface->w, _surface->h);

	// If the whole rectangle is outside the screen, return
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

/**
 * @brief Fetches the current palette
 * @return A byte pointer to the current palette
 */
byte *Screen::getPalette() const {
	return _palette;
}

/**
 * @brief Fetches the current surface
 * @return A pointer to the current surface
 */
Draci::Surface *Screen::getSurface() {
	return _surface;
}

} // End of namespace Draci


