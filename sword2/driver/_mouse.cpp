/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#include "common/stdafx.h"
#include "common/system.h"
#include "common/stream.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/mouse.h"

namespace Sword2 {

// This is the maximum mouse cursor size in the SDL backend
#define MAX_MOUSE_W     80
#define MAX_MOUSE_H     80

#define MOUSEFLASHFRAME 6

void Mouse::decompressMouse(byte *decomp, byte *comp, uint8 frame, int width, int height, int pitch, int xOff, int yOff) {
	int32 size = width * height;
	int32 i = 0;
	int x = 0;
	int y = 0;

	comp = comp + READ_LE_UINT32(comp + frame * 4) - MOUSE_ANIM_HEADER_SIZE;

	while (i < size) {
		if (*comp > 183) {
			decomp[(y + yOff) * pitch + x + xOff] = *comp++;
			if (++x >= width) {
				x = 0;
				y++;
			}
			i++;
		} else {
			x += *comp;
			while (x >= width) {
				y++;
				x -= width;
			}
			i += *comp++;
		}
	}
}

void Mouse::drawMouse() {
	byte mouseData[MAX_MOUSE_W * MAX_MOUSE_H];

	if (!_mouseAnim.data && !_luggageAnim.data)
		return;

	// When an object is used in the game, the mouse cursor should be a
	// combination of a standard mouse cursor and a luggage cursor.
	//
	// However, judging by the original code luggage cursors can also
	// appear on their own. I have no idea which cases though.

	uint16 mouse_width = 0;
	uint16 mouse_height = 0;
	uint16 hotspot_x = 0;
	uint16 hotspot_y = 0;
	int deltaX = 0;
	int deltaY = 0;

	if (_mouseAnim.data) {
		hotspot_x = _mouseAnim.xHotSpot;
		hotspot_y = _mouseAnim.yHotSpot;
		mouse_width = _mouseAnim.mousew;
		mouse_height = _mouseAnim.mouseh;
	}

	if (_luggageAnim.data) {
		if (!_mouseAnim.data) {
			hotspot_x = _luggageAnim.xHotSpot;
			hotspot_y = _luggageAnim.yHotSpot;
		}
		if (_luggageAnim.mousew > mouse_width)
			mouse_width = _luggageAnim.mousew;
		if (_luggageAnim.mouseh > mouse_height)
			mouse_height = _luggageAnim.mouseh;
	}

	if (_mouseAnim.data && _luggageAnim.data) {
		deltaX = _mouseAnim.xHotSpot - _luggageAnim.xHotSpot;
		deltaY = _mouseAnim.yHotSpot - _luggageAnim.yHotSpot;
	}

	assert(deltaX >= 0);
	assert(deltaY >= 0);

	// HACK for maximum cursor size. (The SDL backend imposes this
	// restriction)

	if (mouse_width + deltaX > MAX_MOUSE_W)
		deltaX = 80 - mouse_width;
	if (mouse_height + deltaY > MAX_MOUSE_H)
		deltaY = 80 - mouse_height;

	mouse_width += deltaX;
	mouse_height += deltaY;

	if ((uint32)(mouse_width * mouse_height) > sizeof(mouseData)) {
		warning("Mouse cursor too large");
		return;
	}

	memset(mouseData, 0, mouse_width * mouse_height);

	if (_luggageAnim.data)
		decompressMouse(mouseData, _luggageAnim.data, 0,
			_luggageAnim.mousew, _luggageAnim.mouseh,
			mouse_width, deltaX, deltaY);

	if (_mouseAnim.data)
		decompressMouse(mouseData, _mouseAnim.data, _mouseFrame,
			_mouseAnim.mousew, _mouseAnim.mouseh, mouse_width);

	_vm->_system->setMouseCursor(mouseData, mouse_width, mouse_height, hotspot_x, hotspot_y, 0);
}

/**
 * Animates the current mouse pointer
 */

int32 Mouse::animateMouse() {
	uint8 prevMouseFrame = _mouseFrame;

	if (!_mouseAnim.data)
		return RDERR_UNKNOWN;

	if (++_mouseFrame == _mouseAnim.noAnimFrames)
		_mouseFrame = MOUSEFLASHFRAME;

	if (_mouseFrame != prevMouseFrame)
		drawMouse();

	return RD_OK;
}

/**
 * Sets the mouse cursor animation.
 * @param ma a pointer to the animation data, or NULL to clear the current one
 * @param size the size of the mouse animation data
 * @param mouseFlash RDMOUSE_FLASH or RDMOUSE_NOFLASH, depending on whether
 * or not there is a lead-in animation
 */

int32 Mouse::setMouseAnim(byte *ma, int32 size, int32 mouseFlash) {
	free(_mouseAnim.data);
	_mouseAnim.data = NULL;

	if (ma)	{
		if (mouseFlash == RDMOUSE_FLASH)
			_mouseFrame = 0;
		else
			_mouseFrame = MOUSEFLASHFRAME;

		Common::MemoryReadStream readS(ma, size);

		_mouseAnim.runTimeComp = readS.readByte();
		_mouseAnim.noAnimFrames = readS.readByte();
		_mouseAnim.xHotSpot = readS.readSByte();
		_mouseAnim.yHotSpot = readS.readSByte();
		_mouseAnim.mousew = readS.readByte();
		_mouseAnim.mouseh = readS.readByte();

		_mouseAnim.data = (byte *)malloc(size - MOUSE_ANIM_HEADER_SIZE);
		if (!_mouseAnim.data)
			return RDERR_OUTOFMEMORY;

		readS.read(_mouseAnim.data, size - MOUSE_ANIM_HEADER_SIZE);

		animateMouse();
		drawMouse();

		_vm->_system->showMouse(true);
	} else {
		if (_luggageAnim.data)
			drawMouse();
		else
			_vm->_system->showMouse(false);
	}

	return RD_OK;
}

/**
 * Sets the "luggage" animation to accompany the mouse animation. Luggage
 * sprites are of the same format as mouse sprites.
 * @param ma a pointer to the animation data, or NULL to clear the current one
 * @param size the size of the animation data
 */

int32 Mouse::setLuggageAnim(byte *ma, int32 size) {
	free(_luggageAnim.data);
	_luggageAnim.data = NULL;

	if (ma)	{
		Common::MemoryReadStream readS(ma, size);

		_luggageAnim.runTimeComp = readS.readByte();
		_luggageAnim.noAnimFrames = readS.readByte();
		_luggageAnim.xHotSpot = readS.readSByte();
		_luggageAnim.yHotSpot = readS.readSByte();
		_luggageAnim.mousew = readS.readByte();
		_luggageAnim.mouseh = readS.readByte();

		_luggageAnim.data = (byte *)malloc(size - MOUSE_ANIM_HEADER_SIZE);
		if (!_luggageAnim.data)
			return RDERR_OUTOFMEMORY;

		readS.read(_luggageAnim.data, size - MOUSE_ANIM_HEADER_SIZE);

		animateMouse();
		drawMouse();

		_vm->_system->showMouse(true);
	} else {
		if (_mouseAnim.data)
			drawMouse();
		else
			_vm->_system->showMouse(false);
	}

	return RD_OK;
}

} // End of namespace Sword2
