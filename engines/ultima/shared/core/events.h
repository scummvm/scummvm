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

#ifndef ULTIMA_SHARED_EVENTS_H
#define ULTIMA_SHARED_EVENTS_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/queue.h"

namespace Ultima {
namespace Shared {

#define GAME_FRAME_RATE (1000 / 50)
#define GAME_FRAME_TIME 50
#define SCREEN_UPDATE_TIME 10
#define BUTTON_MASK(MB) (1 << ((int)(MB) - 1))

enum MouseButton {
	BUTTON_NONE = 0,
	BUTTON_LEFT = 1,
	BUTTON_RIGHT = 2,
	BUTTON_MIDDLE = 3,
	MOUSE_LAST
};

class UltimaEngine;

class EventsManager {
private:
	uint32 _frameCounter;
	uint32 _priorFrameCounterTime;
	uint32 _lastAutosaveTime;
	uint32 _gameCounter;
	uint32 _playTime;
private:
	/**
	 * Sets whether a given button is depressed
	 */
	void setButtonDown(MouseButton button, bool isDown);
protected:
	/**
	 * Handles moving to the next game frame
	 */
	virtual void nextFrame();
public:
	Common::Point _mousePos;
	uint8  _buttonsDown;
public:
	EventsManager();
	virtual ~EventsManager();

	/*
	 * Set the cursor
	 */
	virtual void setCursor(int cursorId) {}

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

	/**
	 * Polls the ScummVM backend for any pending events
	 */
	virtual bool pollEvent(Common::Event &event);

	/**
	 * Gets the current total ticks
	 */
	uint32 getTicks() { return _frameCounter; }

	/**
	 * Gets the total overall play time
	 */
	uint32 playTime() const { return _playTime; }
	
	/**
	 * Sets the current play time
	 */
	void setPlayTime(uint32 time) { _playTime = time; }


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
	byte getButtonState() const {
		return _buttonsDown;
	}

	/**
	 * Return the mouse position
	 */
	Common::Point getMousePos() const {
		return _mousePos;
	}
};

extern bool isMouseDownEvent(Common::EventType type);

extern bool isMouseUpEvent(Common::EventType type);

extern MouseButton whichButton(Common::EventType type);

} // End of namespace Shared
} // End of namespace Ultima

#endif
