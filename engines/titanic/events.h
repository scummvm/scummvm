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
#include "common/stack.h"
#include "support/rect.h"

namespace Titanic {

#define GAME_FRAME_RATE 30
#define GAME_FRAME_TIME (1000 / GAME_FRAME_RATE)
#define DOUBLE_CLICK_TIME 100

enum SpecialButtons {
	MK_LBUTTON = 1, MK_RBUTTON = 2, MK_SHIFT = 4, MK_CONTROL = 8,
	MK_MBUTTON = 0x10
};

class TitanicEngine;

/**
 * A base class for windows that can receive event messages
 */
class CEventTarget {
public:
	virtual ~CEventTarget() {}

	/**
	 * Called to handle any regular updates the game requires
	 */
	virtual void onIdle() {}

	/**
	 * Mouse/key event handlers
	 */
	virtual void mouseMove(const Point &mousePos) {}
	virtual void leftButtonDown(const Point &mousePos) {}
	virtual void leftButtonUp(const Point &mousePos) {}
	virtual void leftButtonDoubleClick(const Point &mousePos) {}
	virtual void middleButtonDown(const Point &mousePos) {}
	virtual void middleButtonUp(const Point &mousePos) {}
	virtual void middleButtonDoubleClick(const Point &mousePos) {}
	virtual void mouseWheel(const Point &mousePos, bool wheelUp) {}
	virtual void keyDown(Common::KeyState keyState) {}
	virtual void keyUp(Common::KeyState keyState) {}
};

/**
 * An event target used for waiting for a mouse or keypress
 */
class CPressTarget : public CEventTarget {
public:
	bool _pressed;
public:
	CPressTarget() : _pressed(false) {}
	~CPressTarget() override {}
	void leftButtonDown(const Point &mousePos) override { _pressed = true; }
	void middleButtonDown(const Point &mousePos) override { _pressed = true; }
	void keyDown(Common::KeyState keyState) override {
		if (keyState.ascii)
			_pressed = true;
	}
};

class Events {
private:
	TitanicEngine *_vm;
	Common::Stack<CEventTarget *> _eventTargets;
	uint32 _frameCounter;
	uint32 _priorFrameTime;
	uint _totalFrames;
	Common::Point _mousePos;
	uint _specialButtons;

	/**
	 * Check whether it's time to display the next screen frame
	 */
	bool checkForNextFrameCounter();

	/**
	 * Return the currently active event target
	 */
	CEventTarget *eventTarget() const {
		return _eventTargets.top();
	}

	/**
	 * Handles setting/resettings special buttons on key up/down
	 */
	void handleKbdSpecial(Common::KeyState keyState);
public:
	Events(TitanicEngine *vm);
	~Events() {}

	/**
	 * Adds a new event target to the top of the list. It will get
	 * all events generated until such time as another is pushed on
	 * top of it, or the removeTarget method is called
	 */
	void addTarget(CEventTarget *target) {
		_eventTargets.push(target);
	}

	/**
	 * Removes the currently active event target
	 */
	void removeTarget() {
		_eventTargets.pop();
	}

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
	 * Return the current game ticks
	 */
	uint32 getTicksCount() const;

	/**
	 * Get the total number of playtime frames/ticks
	 */
	uint32 getTotalPlayTicks() const;

	/**
	 * Set the total number of frames/ticks played
	 */
	void setTotalPlayTicks(uint frames);

	/**
	 * Sleep for a specified period of time
	 */
	void sleep(uint time);

	/**
	 * Wait for a mouse or keypress
	 */
	bool waitForPress(uint expiry);

	/**
	 * Get the mouse position
	 */
	Common::Point getMousePos() const { return _mousePos; }

	/**
	 * Sets the mouse position
	 */
	void setMousePos(const Common::Point &pt);

	/*
	 * Return whether a given special key is currently pressed
	 */
	bool isSpecialPressed(SpecialButtons btn) const {
		return (_specialButtons & btn) != 0;
	}

	/**
	 * Returns the bitset of the currently pressed special buttons
	 */
	uint getSpecialButtons() const { return _specialButtons; }
};

} // End of namespace Titanic

#endif /* TITANIC_EVENTS_H */
