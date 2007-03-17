/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2007 The ScummVM project
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

#ifndef COMMON_EVENTS_H
#define COMMON_EVENTS_H

#include "common/rect.h"
#include "common/system.h"

namespace Common {

/**
 * The EventManager provides user input events to the client code.
 * In addition, it keeps track of the state of various input devices,
 * like keys, mouse position and buttons.
 */
class EventManager {
public:
	EventManager() {}
	virtual ~EventManager() {}
	
	enum {
		LBUTTON = 1 << 0,
		RBUTTON = 1 << 1
	};

	/**
	 * Get the next event in the event queue.
	 * @param event	point to an Event struct, which will be filled with the event data.
	 * @return true if an event was retrieved.
	 */
	virtual bool pollEvent(OSystem::Event &event) = 0;


	/** Return the current key state */
	virtual Common::Point getMousePos() const = 0;
	
	/**
	 * Return a bitmask with the button states:
	 * - bit 0: left button up=1, down=0
	 * - bit 1: right button up=1, down=0
	 */
	virtual int getButtonState() const = 0;
	
	/** Get a bitmask with the current modifier state */
	virtual int getModifierState() const = 0;

	/**
	 * Should the application terminate? Set to true if we
	 * received an EVENT_QUIT.
	 */
	virtual int shouldQuit() const = 0;
	
	// Optional: check whether a given key is currently pressed ????
	//virtual bool isKeyPressed(int keycode) = 0;

	// TODO: Keyboard repeat support?
	
	// TODO: Consider removing OSystem::getScreenChangeID and
	// replacing it by a generic getScreenChangeID method here
};

}

#endif
