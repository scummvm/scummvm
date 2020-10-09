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

#ifndef XEEN_EVENTS_H
#define XEEN_EVENTS_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/queue.h"
#include "xeen/sprites.h"

namespace Xeen {

#define GAME_FRAME_RATE (1000 / 50)
#define GAME_FRAME_TIME 50
#define SCREEN_UPDATE_TIME 10
#define MAX_PENDING_EVENTS 5

class XeenEngine;

struct PendingEvent {
	Common::KeyState _keyState;
	bool _leftButton;
	bool _rightButton;

	PendingEvent() : _leftButton(false), _rightButton(false) {}
	PendingEvent(const Common::KeyState &keyState) : _keyState(keyState), _leftButton(false), _rightButton(false) {}
	PendingEvent(bool leftButton, bool rightButton) : _leftButton(leftButton), _rightButton(rightButton) {}

	/**
	 * Returns true if a keyboard event is pending
	 */
	bool isKeyboard() const { return _keyState.keycode != Common::KEYCODE_INVALID; }

	/**
	 * Returns ture if a mouse button event is pending
	 */
	bool isMouse() const { return _leftButton || _rightButton; }
};

class EventsManager {
private:
	XeenEngine *_vm;
	uint32 _frameCounter;
	uint32 _priorFrameCounterTime;
	uint32 _priorScreenRefreshTime;
	uint32 _gameCounter;
	uint32 _gameCounters[6];
	uint32 _playTime;
	Common::Queue<PendingEvent> _pendingEvents;
	SpriteResource _sprites;
	bool _mousePressed;

	/**
	 * Handles moving to the next game frame
	 */
	void nextFrame();

	/**
	 * Returns whether the keypress is a modifier key like Ctrl or Alt
	 */
	bool isModifierKey(const Common::KeyCode &keycode) const;
public:
	Common::Point _mousePos;
public:
	EventsManager(XeenEngine *vm);
	~EventsManager();

	/*
	 * Set the cursor
	 */
	void setCursor(int cursorId);

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
	void pollEvents();

	/**
	 * Polls for events, and wait a slight delay. This ensures the game doesn't use up 100% of the CPU
	 */
	void pollEventsAndWait();

	/**
	 * Clears all pending events
	 */
	void clearEvents();

	/**
	 * Waits for a mouse press to be released
	 */
	void debounceMouse();

	/**
	 * Adds a keyboard event to the queue
	 */
	void addEvent(const Common::KeyState &keyState);

	/**
	 * Adds a mouse button event to the queue
	 */
	void addEvent(bool leftButton, bool rightButton);

	/**
	 * Returns the next pending key/mouse press, if any
	 */
	bool getEvent(PendingEvent &pe);

	/**
	 * Returns true if a key or mouse event is pending
	 */
	bool isEventPending() const { return !_pendingEvents.empty(); }

	/**
	 * Returns true if a key or mouse press is pending
	 */
	bool isKeyMousePressed();

	void updateGameCounter() { _gameCounter = _frameCounter; }
	void timeMark1() { _gameCounters[1] = _frameCounter; }
	void timeMark2() { _gameCounters[2] = _frameCounter; }
	void timeMark3() { _gameCounters[3] = _frameCounter; }
	void timeMark4() { _gameCounters[4] = _frameCounter; }
	void timeMark5() { _gameCounters[5] = _frameCounter; }

	uint32 timeElapsed() const { return _frameCounter - _gameCounter; }
	uint32 timeElapsed1() const { return _frameCounter - _gameCounters[1]; }
	uint32 timeElapsed2() const { return _frameCounter - _gameCounters[2]; }
	uint32 timeElapsed3() const { return _frameCounter - _gameCounters[3]; }
	uint32 timeElapsed4() const { return _frameCounter - _gameCounters[4]; }
	uint32 timeElapsed5() const { return _frameCounter - _gameCounters[5]; }
	uint32 getTicks() { return _frameCounter; }
	uint32 playTime() const { return _playTime; }
	void setPlayTime(uint32 time) { _playTime = time; }

	/**
	 * Waits for a given number of frames
	 * @param numFrames			Number of frames to wait
	 * @param interruptable		If set, aborts if the mouse or a key is pressed
	 * @returns		True if the wait was aborted
	 */
	bool wait(uint numFrames, bool interruptable = true);

	/**
	 * Pause for a set amount
	 */
	void ipause(uint amount);

	/**
	 * Pauses a set amount past the previous call to timeMark5
	 */
	void ipause5(uint amount);

	/**
	 * Waits for a key or mouse press, animating the 3d view in the background
	 */
	void waitForPressAnimated();

	/**
	 * Waits for a key or mouse press
	 */
	void waitForPress();
};

} // End of namespace Xeen

#endif /* XEEN_EVENTS_H */
