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
 * Main system events manager
 */
class Events {
private:
	static Events *g_events;
	uint8  _buttonsDown;

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
