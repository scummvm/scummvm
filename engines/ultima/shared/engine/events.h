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

#ifndef ULTIMA_SHARED_ENGINE_EVENTS_H
#define ULTIMA_SHARED_ENGINE_EVENTS_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/stack.h"
#include "gui/debugger.h"
#include "graphics/screen.h"
#include "ultima/shared/core/rect.h"

namespace Ultima {
namespace Shared {

#define GAME_FRAME_RATE (1000 / 50)
#define GAME_FRAME_TIME 50
#define SCREEN_UPDATE_TIME 10
#define BUTTON_MASK(MB) (1 << ((int)(MB) - 1))
#define DOUBLE_CLICK_TIME 100

enum MouseButton {
	BUTTON_NONE = 0,
	BUTTON_LEFT = 1,
	BUTTON_RIGHT = 2,
	BUTTON_MIDDLE = 3,
	MOUSE_LAST
};

enum SpecialButtons {
	MK_LBUTTON = 1, MK_RBUTTON = 2, MK_MBUTTON = 4, MK_SHIFT = 8, MK_CONTROL = 0x10
};

/**
 * A base class for windows that can receive event messages
 */
class EventTarget {
public:
	virtual ~EventTarget() {
	}

	/**
	 * Called to handle any regular updates the game requires
	 */
	virtual void onIdle() {
	}

	/**
	 * Mouse/key event handlers
	 */
	virtual void mouseMove(const Common::Point &mousePos) {
	}
	virtual void leftButtonDown(const Common::Point &mousePos) {
	}
	virtual void leftButtonUp(const Common::Point &mousePos) {
	}
	virtual void leftButtonDoubleClick(const Common::Point &mousePos) {
	}
	virtual void middleButtonDown(const Common::Point &mousePos) {
	}
	virtual void middleButtonUp(const Common::Point &mousePos) {
	}
	virtual void middleButtonDoubleClick(const Common::Point &mousePos) {
	}
	virtual void rightButtonDown(const Common::Point &mousePos) {
	}
	virtual void rightButtonUp(const Common::Point &mousePos) {
	}
	virtual void mouseWheel(const Common::Point &mousePos, bool wheelUp) {
	}
	virtual void keyDown(Common::KeyState keyState) {
	}
	virtual void keyUp(Common::KeyState keyState) {
	}
};

/**
 * An eent target used for waiting for a mouse or keypress
 */
class CPressTarget : public EventTarget {
public:
	bool _pressed;
public:
	CPressTarget() : _pressed(false) {
	}
	~CPressTarget() override {
	}
	void leftButtonDown(const Common::Point &mousePos) override {
		_pressed = true;
	}
	void middleButtonDown(const Common::Point &mousePos) override {
		_pressed = true;
	}
	void rightButtonDown(const Common::Point &mousePos) override {
		_pressed = true;
	}
	void keyDown(Common::KeyState keyState) override {
		_pressed = true;
	}
};

/**
 * Abstract interface for engine functionality the events manager needs to access
 */
class EventsCallback {
public:
	/**
	 * Destructor
	 */
	virtual ~EventsCallback() {}

	/**
	 * Get the screen
	 */
	virtual Graphics::Screen *getScreen() const {
		return nullptr;
	}
};

class EventsManager {
private:
	EventsCallback *_callback;
	Common::Stack<EventTarget *> _eventTargets;
	uint32 _frameCounter;
	uint32 _priorFrameTime;
	uint32 _priorFrameCounterTime;
	uint32 _gameCounter;
	uint32 _playTime;
	Point _mousePos;
	uint _specialButtons;
	uint8  _buttonsDown;

	/**
	 * Check whether it's time to display the next screen frame
	 */
	bool checkForNextFrameCounter();

	/**
	 * Return the currently active event target
	 */
	EventTarget *eventTarget() const {
		return _eventTargets.top();
	}

	/**
	 * Handles setting/resettings special buttons on key up/down
	 */
	void handleKbdSpecial(Common::KeyState keyState);

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
	EventsManager(EventsCallback *callback);
	virtual ~EventsManager() {}

	/**
	 * Adds a new event target to the top of the list. It will get
	 * all events generated until such time as another is pushed on
	 * top of it, or the removeTarget method is called
	 */
	void addTarget(EventTarget *target) {
		_eventTargets.push(target);
	}

	/**
	 * Removes the currently active event target
	 */
	void removeTarget() {
		_eventTargets.pop();
	}

	/**
	 * Polls the ScummVM backend for any pending events, passing out the event, if any
	 */

	virtual bool pollEvent(Common::Event &event);

	/**
	 * Checks for any pending events. This differs from pollEvent, in that the event manager will dispatch
	 * all pending events to the currently registered active event target, rather than simply returning a
	 * single event like pollEvent does
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
	uint32 getFrameCounter() const {
		return _frameCounter;
	}

	/**
	 * Get the elapsed playtime
	 */
	uint32 getTicksCount() const;

	/**
	 * Sleep for a specified period of time
	 */
	void sleep(uint time);

	/**
	 * Wait for a mouse or keypress
	 */
	bool waitForPress(uint expiry);

	/**
	 * Sets the mouse position
	 */
	void setMousePos(const Point &pt);

	/*
	 * Return whether a given special key is currently pressed
	 */
	bool isSpecialPressed(SpecialButtons btn) const {
		return (_specialButtons & btn) != 0;
	}

	/**
	 * Returns the bitset of the currently pressed special buttons
	 */
	uint getSpecialButtons() const {
		return _specialButtons;
	}

	/*
	 * Set the cursor
	 */
	virtual void setCursor(int cursorId) {
	}

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
	 * Gets the current total ticks
	 */
	uint32 getTicks() {
		return _frameCounter;
	}

	/**
	 * Gets the total overall play time
	 */
	uint32 playTime() const {
		return _playTime;
	}

	/**
	 * Sets the current play time
	 */
	void setPlayTime(uint32 time) {
		_playTime = time;
	}

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
