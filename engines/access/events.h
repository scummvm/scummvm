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

#ifndef ACCESS_EVENTS_H
#define ACCESS_EVENTS_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/stack.h"

namespace Access {

enum CursorType { 
	CURSOR_NONE = -1,
	CURSOR_0 = 0, CURSOR_1, CURSOR_2, CURSOR_3, CURSOR_EYE, CURSOR_HAND, 
	CURSOR_GET, CURSOR_CLIMB, CURSOR_TALK, CURSOR_HELP
};

#define GAME_FRAME_RATE 50
#define GAME_FRAME_TIME (1000 / GAME_FRAME_RATE)

class AccessEngine;

class EventsManager {
private:
	AccessEngine *_vm;
	uint32 _frameCounter;
	uint32 _priorFrameTime;

	void checkForNextFrameCounter();
public:
	CursorType _cursorId;
	bool _leftButton;
	Common::Point _mousePos;
public:
	/**
	 * Constructor
	 */
	EventsManager(AccessEngine *vm);

	/**
	 * Destructor
	 */
	~EventsManager();

	/**
	 * Sets the cursor
	 */
	void setCursor(CursorType cursorId);

	/**
	 * Return the current cursor Id
	 */
	CursorType getCursor() const { return _cursorId; }

	/**
	 * Show the mouse cursor
	 */
	void showCursor();

	/**
	 * Hide the mouse cursor
	 */
	void hideCursor();

	/**
	 * Returns if the mouse cursor is visible
	 */
	bool isCursorVisible();

	void pollEvents();
};

} // End of namespace Access

#endif /* ACCESS_EVENTS_H */
