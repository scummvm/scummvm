/* Copyright (C) 1994-2004 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "sword2/sword2.h"

namespace Sword2 {

#define MOUSEFLASHFRAME 6

/**
 * Logs the mouse button event passed in buttons. The button events were
 * originaly defined as RD_LEFTBUTTONDOWN, RD_LEFTBUTTONUP, RD_RIGHTBUTTONDOWN
 * and RD_RIGHTBUTTONUP. ScummVM adds RD_WHEELDOWN and RD_WHEELUP.
 */

void Input::logMouseEvent(uint16 buttons) {
	// We need to leave the one, which is the current event, alone!
	if (_mouseBacklog == MAX_MOUSE_EVENTS - 1)
		return;

	_mouseLog[(_mouseBacklog + _mouseLogPos) % MAX_MOUSE_EVENTS].buttons = buttons;
	_mouseBacklog++;
}

bool Input::checkForMouseEvents(void) {
	return _mouseBacklog != 0;
}

/**
 * Get the next pending mouse event.
 * @return a pointer to the mouse event, or NULL of there is none
 */

MouseEvent *Input::mouseEvent(void) {
	MouseEvent *me;

	if (_mouseBacklog) {
		me = &_mouseLog[_mouseLogPos];
		if (++_mouseLogPos == MAX_MOUSE_EVENTS)
			_mouseLogPos = 0;

		_mouseBacklog--;
		return me;
	}

	return NULL;
}

void Graphics::resetRenderEngine(void) {
	_parallaxScrollX = 0;
	_parallaxScrollY = 0;
	_scrollX = 0;
	_scrollY = 0;
}

// FIXME: The original code used 0 for transparency, while our backend uses
// 0xFF. That means that parts of the mouse cursor that weren't meant to be
// transparent may be now.

void Graphics::decompressMouse(uint8 *decomp, uint8 *comp, int width, int height, int pitch, int xOff, int yOff) {
	int32 size = width * height;
	int32 i = 0;
	int x = 0;
	int y = 0;

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

void Graphics::drawMouse(void) {
	if (!_mouseAnim && !_luggageAnim)
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

	if (_mouseAnim) {
		hotspot_x = _mouseAnim->xHotSpot;
		hotspot_y = _mouseAnim->yHotSpot;
		mouse_width = _mouseAnim->mousew;
		mouse_height = _mouseAnim->mouseh;
	}

	if (_luggageAnim) {
		if (!_mouseAnim) {
			hotspot_x = _luggageAnim->xHotSpot;
			hotspot_y = _luggageAnim->yHotSpot;
		}
		if (_luggageAnim->mousew > mouse_width)
			mouse_width = _luggageAnim->mousew;
		if (_luggageAnim->mouseh > mouse_height)
			mouse_height = _luggageAnim->mouseh;
	}

	if (_mouseAnim && _luggageAnim) {
		deltaX = _mouseAnim->xHotSpot - _luggageAnim->xHotSpot;
		deltaY = _mouseAnim->yHotSpot - _luggageAnim->yHotSpot;
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

	if ((uint32) (mouse_width * mouse_height) > sizeof(_mouseData)) {
		warning("Mouse cursor too large");
		return;
	}

	memset(_mouseData, 0xFF, mouse_width * mouse_height);

	if (_luggageAnim)
		decompressMouse(_mouseData, (uint8 *) _luggageAnim + READ_LE_UINT32(_luggageOffset), _luggageAnim->mousew,
				_luggageAnim->mouseh, mouse_width, deltaX, deltaY);

	if (_mouseAnim)
		decompressMouse(_mouseData, _mouseSprite, _mouseAnim->mousew, _mouseAnim->mouseh, mouse_width);

	_vm->_system->set_mouse_cursor(_mouseData, mouse_width, mouse_height, hotspot_x, hotspot_y);
}

/**
 * Animates the current mouse pointer
 */

int32 Graphics::animateMouse(void) {
	uint8 prevMouseFrame = _mouseFrame;

	if (!_mouseAnim)
		return RDERR_UNKNOWN;

	if (++_mouseFrame == _mouseAnim->noAnimFrames)
		_mouseFrame = MOUSEFLASHFRAME;

	_mouseSprite = (uint8 *) _mouseAnim + READ_LE_UINT32(_mouseOffsets + _mouseFrame);

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

int32 Graphics::setMouseAnim(uint8 *ma, int32 size, int32 mouseFlash) {
	if (_mouseAnim) {
		free(_mouseAnim);
		_mouseAnim = NULL;
	}

	if (ma)	{
		if (mouseFlash == RDMOUSE_FLASH)
			_mouseFrame = 0;
		else
			_mouseFrame = MOUSEFLASHFRAME;

		_mouseAnim = (MouseAnim *) malloc(size);
		if (!_mouseAnim)
			return RDERR_OUTOFMEMORY;

		memcpy((uint8 *) _mouseAnim, ma, size);
		_mouseOffsets = (int32 *) ((uint8 *) _mouseAnim + sizeof(MouseAnim));

		animateMouse();
		drawMouse();

		_vm->_system->show_mouse(true);
	} else {
		if (_luggageAnim)
			drawMouse();
		else
			_vm->_system->show_mouse(false);
	}

	return RD_OK;
}

/**
 * Sets the "luggage" animation to accompany the mouse animation. Luggage
 * sprites are of the same format as mouse sprites.
 * @param ma a pointer to the animation data, or NULL to clear the current one
 * @param size the size of the animation data
 */

int32 Graphics::setLuggageAnim(uint8 *ma, int32 size) {
	if (_luggageAnim) {
		free(_luggageAnim);
		_luggageAnim = NULL;
	}

	if (ma)	{
		_luggageAnim = (MouseAnim *) malloc(size);
		if (!_luggageAnim)
			return RDERR_OUTOFMEMORY;

		memcpy((uint8 *) _luggageAnim, ma, size);
		_luggageOffset = (int32 *) ((uint8 *) _luggageAnim + sizeof(MouseAnim));

		animateMouse();
		drawMouse();

		_vm->_system->show_mouse(true);
	} else {
		if (_mouseAnim)
			drawMouse();
		else
			_vm->_system->show_mouse(false);
	}

	return RD_OK;
}

} // End of namespace Sword2
