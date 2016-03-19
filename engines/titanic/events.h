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

#ifndef TITANIC_EVENTS_H
#define TITANIC_EVENTS_H

#include "common/scummsys.h"
#include "common/events.h"

namespace Titanic {

#define GAME_FRAME_RATE 30
#define GAME_FRAME_TIME (1000 / GAME_FRAME_RATE)
#define DOUBLE_CLICK_TIME 100

enum SpecialButtons { 
	MK_LBUTTON = 1, MK_RBUTTON = 2, MK_SHIFT = 4, MK_CONTROL = 8, 
	MK_MBUTTON = 0x10
};

class TitanicEngine;

class Events {
private:
	TitanicEngine *_vm;
	uint32 _frameCounter;
	uint32 _priorFrameTime;
	uint32 _priorLeftDownTime;
	uint32 _priorMiddleDownTime;
	uint32 _priorRightDownTime;
	Common::Point _mousePos;
	int _specialButtons;

	/**
	 * Check whether it's time to display the next screen frame
	 */
	bool checkForNextFrameCounter();

	void mouseMove();
	void leftButtonDown();
	void leftButtonUp();
	void leftButtonDoubleClick();
	void middleButtonDown();
	void middleButtonUp();
	void middleButtonDoubleClick();
	void rightButtonDown();
	void rightButtonUp();
	void rightButtonDoubleClick();
	void charPress(char c);
	void keyDown(Common::KeyState keyState);
public:
	Events(TitanicEngine *vm);
	~Events() {}

	/**
	 * Check for any pending events
	 */
	void pollEvents();

	/**
	 * Poll for events and introduce a small delay, to allow the system to
	 * yield to other running programs
	 */
	void pollEventsAndWait();

	/**
	 * Return the current game frame number
	 */
	uint32 getFrameCounter() const { return _frameCounter; }

	/**
	 * Get the elapsed playtime
	 */
	uint32 getTicksCount() const;
};

} // End of namespace Titanic

#endif /* TITANIC_EVENTS_H */
