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
 */

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#ifndef TONY_INPUT_H
#define TONY_INPUT_H

#include "common/events.h"
#include "tony/utils.h"

namespace Tony {

class RMInput {
private:
	Common::Event _event;

	// Mouse related fields
	RMPoint _mousePos;
	bool _clampMouse;
	bool _leftButton, _rightButton;
	bool _leftClickMouse, _leftReleaseMouse, _rightClickMouse, _rightReleaseMouse;

	// Keyboard related fields
	bool _keyDown[350];
private:
	// Deinitialize DirectInput
	void DIClose(void);

public:
	RMInput();
	~RMInput();

	// Class initialisation
	void init(/*uint32 hInst*/);

	// Closes the class
	void close(void);

	// Polling (must be performed once per frame)
	void poll(void);

	// Reading of the mouse
	RMPoint mousePos() {
		return _mousePos;
	}

	// Current status of the mouse buttons
	bool mouseLeft();
	bool mouseRight();

	// Events of mouse clicks
	bool mouseLeftClicked() {
		return _leftClickMouse;
	}
	bool mouseRightClicked() {
		return _rightClickMouse;
	}
	bool mouseBothClicked() {
		return _leftClickMouse && _rightClickMouse;
	}
	bool mouseLeftReleased() {
		return _leftReleaseMouse;
	}
	bool mouseRightReleased() {
		return _rightReleaseMouse;
	}
	bool mouseBothReleased() {
		return _leftReleaseMouse && _rightReleaseMouse;
	}

	// Returns true if the given key is pressed
	bool getAsyncKeyState(Common::KeyCode kc);
};

} // End of namespace Tony

#endif
