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

#ifndef ULTIMA4_CONTROLLERS_CONTROLLER_H
#define ULTIMA4_CONTROLLERS_CONTROLLER_H

#include "ultima/ultima4/meta_engine.h"

namespace Ultima {
namespace Ultima4 {

/**
 * A generic controller base class.  Controllers are classes that
 * contain the logic for responding to external events (e.g. keyboard,
 * mouse, timers).
 */
class Controller {
public:
	Controller(int timerInterval = 1);
	virtual ~Controller();

	/* methods for interacting with event manager */
	virtual bool isCombatController() const {
		return false;
	}

	/**
	 * The event manager will call this method to notify the active
	 * controller that a key has been pressed.  The key will be passed on
	 * to the virtual keyPressed method.
	 */
	bool notifyKeyPressed(int key);

	/**
	 * The event manager will call this method to notify that
	 * the left button was clicked
	 */
	bool notifyMousePress(const Common::Point &mousePos);

	int getTimerInterval();

	/**
	 * A simple adapter to make a timer callback into a controller method
	 * call.
	 */
	static void timerCallback(void *data);

	/** control methods subclasses may want to override */

	/**
	 * Called when a controller is made active
	 */
	virtual void setActive();

	/**
	 * Key was pressed
	 */
	virtual bool keyPressed(int key) {
		return false;
	}

	/**
	 * Mouse button was pressed
	 */
	virtual bool mousePressed(const Common::Point &mousePos) {
		return false;
	}

	/**
	 * Handles keybinder actions
	 */
	virtual void keybinder(KeybindingAction action) {}

	/**
	 * The default timerFired handler for a controller.  By default,
	 * timers are ignored, but subclasses can override this method and it
	 * will be called every <interval> 1/4 seconds.
	 */
	virtual void timerFired();

	/**
	 * Returns true if game should quit
	 */
	bool shouldQuit() const;
private:
	int _timerInterval;
};

// helper functions for the waitable controller; they just avoid
// having eventhandler dependencies in this header file
void Controller_startWait();
void Controller_endWait();

/**
 * Class template for controllers that can be "waited for".
 * Subclasses should set the value variable and call doneWaiting when
 * the controller has completed.
 */
template<class T>
class WaitableController : public Controller {
private:
	bool _exitWhenDone;
	T _defaultValue;
protected:
	T _value;
	void doneWaiting() {
		if (_exitWhenDone)
			Controller_endWait();
	}
public:
	WaitableController(T defaultValue) : _defaultValue(defaultValue),
		_value(defaultValue), _exitWhenDone(false) {}

	virtual T getValue() {
		return shouldQuit() ? _defaultValue : _value;
	}

	virtual T waitFor() {
		_exitWhenDone = true;
		Controller_startWait();
		return getValue();
	}

	/**
	 * Mouse button was pressed
	 */
	virtual bool mousePressed(const Common::Point &mousePos) {
		// Treat mouse clicks as an abort
		doneWaiting();
		_value = _defaultValue;
		return true;
	}
};

class TurnCompleter {
public:
	virtual ~TurnCompleter() {
	}
	virtual void finishTurn() = 0;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
