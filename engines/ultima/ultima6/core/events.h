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

#ifndef ULTIMA6_CORE_EVENTS_H
#define ULTIMA6_CORE_EVENTS_H

#include "common/events.h"

namespace Ultima {
namespace Ultima6 {

enum MouseButton { BUTTON_NONE = 0, BUTTON_LEFT = 1, BUTTON_RIGHT = 2, BUTTON_MIDDLE = 3 };

#define BUTTON_MASK(MB) (1 << ((int)(MB) - 1))

/**
 * Joystick actions mapped to dummy unused keycode values
 */
const Common::KeyCode FIRST_JOY = (Common::KeyCode)400;
const Common::KeyCode JOY_UP = FIRST_JOY;               // PS d-pad when analog is disabled. left stick when enabled
const Common::KeyCode JOY_DOWN = (Common::KeyCode)(FIRST_JOY + 1);
const Common::KeyCode JOY_LEFT = (Common::KeyCode)(FIRST_JOY + 2);
const Common::KeyCode JOY_RIGHT = (Common::KeyCode)(FIRST_JOY + 3);
const Common::KeyCode JOY_RIGHTUP = (Common::KeyCode)(FIRST_JOY + 4);
const Common::KeyCode JOY_RIGHTDOWN = (Common::KeyCode)(FIRST_JOY + 5);
const Common::KeyCode JOY_LEFTUP = (Common::KeyCode)(FIRST_JOY + 6);
const Common::KeyCode JOY_LEFTDOWN = (Common::KeyCode)(FIRST_JOY + 7);
const Common::KeyCode JOY_UP2 = (Common::KeyCode)(FIRST_JOY + 8); // PS right stick when analog is enabled
const Common::KeyCode JOY_DOWN2 = (Common::KeyCode)(FIRST_JOY + 9);
const Common::KeyCode JOY_LEFT2 = (Common::KeyCode)(FIRST_JOY + 10);
const Common::KeyCode JOY_RIGHT2 = (Common::KeyCode)(FIRST_JOY + 11);
const Common::KeyCode JOY_RIGHTUP2 = (Common::KeyCode)(FIRST_JOY + 12);
const Common::KeyCode JOY_RIGHTDOWN2 = (Common::KeyCode)(FIRST_JOY + 13);
const Common::KeyCode JOY_LEFTUP2 = (Common::KeyCode)(FIRST_JOY + 14);
const Common::KeyCode JOY_LEFTDOWN2 = (Common::KeyCode)(FIRST_JOY + 15);
const Common::KeyCode JOY_UP3 = (Common::KeyCode)(FIRST_JOY + 16);
const Common::KeyCode JOY_DOWN3 = (Common::KeyCode)(FIRST_JOY + 17);
const Common::KeyCode JOY_LEFT3 = (Common::KeyCode)(FIRST_JOY + 18);
const Common::KeyCode JOY_RIGHT3 = (Common::KeyCode)(FIRST_JOY + 19);
const Common::KeyCode JOY_RIGHTUP3 = (Common::KeyCode)(FIRST_JOY + 20);
const Common::KeyCode JOY_RIGHTDOWN3 = (Common::KeyCode)(FIRST_JOY + 21);
const Common::KeyCode JOY_LEFTUP3 = (Common::KeyCode)(FIRST_JOY + 22);
const Common::KeyCode JOY_LEFTDOWN3 = (Common::KeyCode)(FIRST_JOY + 23);
const Common::KeyCode JOY_UP4 = (Common::KeyCode)(FIRST_JOY + 24);
const Common::KeyCode JOY_DOWN4 = (Common::KeyCode)(FIRST_JOY + 25);
const Common::KeyCode JOY_LEFT4 = (Common::KeyCode)(FIRST_JOY + 26);
const Common::KeyCode JOY_RIGHT4 = (Common::KeyCode)(FIRST_JOY + 27);
const Common::KeyCode JOY_RIGHTUP4 = (Common::KeyCode)(FIRST_JOY + 28);
const Common::KeyCode JOY_RIGHTDOWN4 = (Common::KeyCode)(FIRST_JOY + 29);
const Common::KeyCode JOY_LEFTUP4 = (Common::KeyCode)(FIRST_JOY + 30);
const Common::KeyCode JOY_LEFTDOWN4 = (Common::KeyCode)(FIRST_JOY + 31);
const Common::KeyCode JOY_HAT_UP = (Common::KeyCode)(FIRST_JOY + 32); // PS d-pad when analog is enabled
const Common::KeyCode JOY_HAT_DOWN = (Common::KeyCode)(FIRST_JOY + 33);
const Common::KeyCode JOY_HAT_LEFT = (Common::KeyCode)(FIRST_JOY + 34);
const Common::KeyCode JOY_HAT_RIGHT = (Common::KeyCode)(FIRST_JOY + 35);
const Common::KeyCode JOY_HAT_RIGHTUP = (Common::KeyCode)(FIRST_JOY + 36);
const Common::KeyCode JOY_HAT_RIGHTDOWN = (Common::KeyCode)(FIRST_JOY + 37);
const Common::KeyCode JOY_HAT_LEFTUP = (Common::KeyCode)(FIRST_JOY + 38);
const Common::KeyCode JOY_HAT_LEFTDOWN = (Common::KeyCode)(FIRST_JOY + 39);
const Common::KeyCode JOY0 = (Common::KeyCode)(FIRST_JOY + 40); // PS triangle
const Common::KeyCode JOY1 = (Common::KeyCode)(FIRST_JOY + 41); // PS circle
const Common::KeyCode JOY2 = (Common::KeyCode)(FIRST_JOY + 42); // PS x
const Common::KeyCode JOY3 = (Common::KeyCode)(FIRST_JOY + 43); // PS square
const Common::KeyCode JOY4 = (Common::KeyCode)(FIRST_JOY + 44); // PS L2
const Common::KeyCode JOY5 = (Common::KeyCode)(FIRST_JOY + 45); // PS R2
const Common::KeyCode JOY6 = (Common::KeyCode)(FIRST_JOY + 46); // PS L1
const Common::KeyCode JOY7 = (Common::KeyCode)(FIRST_JOY + 47); // PS R1
const Common::KeyCode JOY8 = (Common::KeyCode)(FIRST_JOY + 48); // PS select
const Common::KeyCode JOY9 = (Common::KeyCode)(FIRST_JOY + 49); // PS start
const Common::KeyCode JOY10 = (Common::KeyCode)(FIRST_JOY + 50); // PS L3 (analog must be enabled)
const Common::KeyCode JOY11 = (Common::KeyCode)(FIRST_JOY + 51); // PS R3 (analog must be enabled)
const Common::KeyCode JOY12 = (Common::KeyCode)(FIRST_JOY + 52);
const Common::KeyCode JOY13 = (Common::KeyCode)(FIRST_JOY + 53);
const Common::KeyCode JOY14 = (Common::KeyCode)(FIRST_JOY + 54);
const Common::KeyCode JOY15 = (Common::KeyCode)(FIRST_JOY + 55);
const Common::KeyCode JOY16 = (Common::KeyCode)(FIRST_JOY + 56);
const Common::KeyCode JOY17 = (Common::KeyCode)(FIRST_JOY + 57);
const Common::KeyCode JOY18 = (Common::KeyCode)(FIRST_JOY + 58);
const Common::KeyCode JOY19 = (Common::KeyCode)(FIRST_JOY + 59);

/**
 * Main system events manager
 */
class Events {
private:
	static Events *g_events;
	uint8  _buttonsDown;
	Common::Point _mousePos;

	/**
	 * Sets whether a given button is depressed
	 */
	void setButtonDown(MouseButton button, bool isDown);
public:
	Events();
	~Events();

	/**
	 * Gets a reference to the events manager
	 */
	static Events *get() { return g_events; }

	/**
	 * Returns true if a given mouse button is pressed
	 */
	inline bool isButtonDown(MouseButton button) const {
		return (_buttonsDown & BUTTON_MASK(button)) != 0;
	}

	/**
	 * Returns true if any mouse button is pressed
	 */
	bool isButtonDown() const {
		return isButtonDown(BUTTON_LEFT) || isButtonDown(BUTTON_RIGHT) || isButtonDown(BUTTON_MIDDLE);
	}

	/**
	 * Returns the mouse buttons states
	 */
	byte getButtonState() const { return _buttonsDown; }

	/**
	 * Returns the mouse position
	 */
	Common::Point getMousePos() const { return _mousePos; }

	/**
	 * Poll for any pending events
	 */
	bool pollEvent(Common::Event &event);
};

extern bool isMouseDownEvent(Common::EventType type);

extern bool isMouseUpEvent(Common::EventType type);

extern MouseButton whichButton(Common::EventType type);

extern bool shouldQuit();

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
