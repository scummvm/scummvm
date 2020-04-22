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

#ifndef ULTIMA4_EVENTS_EVENT_H
#define ULTIMA4_EVENTS_EVENT_H

#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/events/timed_event_mgr.h"
#include "ultima/ultima4/controllers/key_handler_controller.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/shared/std/containers.h"
#include "common/events.h"
#include "common/list.h"
#include "common/str.h"

namespace Ultima {
namespace Ultima4 {

#define eventHandler (EventHandler::getInstance())

#if defined(IOS)
#ifndef __OBJC__
typedef void *TimedManagerHelper;
typedef void *UIEvent;
#else
@class TimedManagerHelper;
@class UIEvent;
#endif
#endif

typedef void(*updateScreenCallback)();
/**
 * A class for handling game events.
 */
class EventHandler {
	typedef Common::List<const MouseArea *> MouseAreaList;
private:
	static EventHandler *_instance;
private:
	void handleMouseMotionEvent(const Common::Event &event);
	void handleMouseButtonDownEvent(const Common::Event &event, Controller *controller, updateScreenCallback updateScreen);
	void handleKeyDownEvent(const Common::Event &event, Controller *controller, updateScreenCallback updateScreen);
protected:
	static bool _controllerDone;
	static bool _ended;
	TimedEventMgr _timer;
	Std::vector<Controller *> _controllers;
	MouseAreaList _mouseAreaSets;
	updateScreenCallback _updateScreen;
public:
	/**
	 * Constructor
	 */
	EventHandler();

	/* Static functions */
	static EventHandler *getInstance();

	/**
	 * Delays program execution for the specified number of milliseconds.
	 * This doesn't actually stop events, but it stops the user from interacting
	 * While some important event happens (e.g., getting hit by a cannon ball or a spell effect).
	 */
	static void sleep(uint usec);

	/**
	 * Waits a given number of milliseconds before continuing
	 */
	static void wait_msecs(uint msecs);

	/**
	 * Waits a given number of game cycles before continuing
	 */
	static void wait_cycles(uint cycles);

	static void setControllerDone(bool exit = true);

	/**
	 * Returns the current value of the global exit flag
	 */
	static bool getControllerDone();
	static void end();

	/* Member functions */
	TimedEventMgr *getTimer();

	/* Event functions */
	void run();
	void setScreenUpdate(void (*updateScreen)(void));
#if defined(IOS)
	void handleEvent(UIEvent *);
	static void controllerStopped_helper();
	updateScreenCallback screenCallback() {
		return updateScreen;
	}
#endif

	/* Controller functions */
	Controller *pushController(Controller *c);
	Controller *popController();
	Controller *getController() const;
	void setController(Controller *c);

	/* Key handler functions */
	/**
	 * Adds a key handler to the stack.
	 */
	void pushKeyHandler(KeyHandler kh);

	/**
	 * Pops a key handler off the stack.
	 * Returns a pointer to the resulting key handler after
	 * the current handler is popped.
	 */
	void popKeyHandler();

	/**
	 * Returns a pointer to the current key handler.
	 * Returns nullptr if there is no key handler.
	 */
	KeyHandler *getKeyHandler() const;

	/**
	 * Eliminates all key handlers and begins stack with new handler.
	 * This pops all key handlers off the stack and adds
	 * the key handler provided to the stack, making it the
	 * only key handler left. Use this function only if you
	 * are sure the key handlers in the stack are disposable.
	 */
	void setKeyHandler(KeyHandler kh);

	/* Mouse area functions */
	void pushMouseAreaSet(const MouseArea *mouseAreas);
	void popMouseAreaSet();

	/**
	 * Get the currently active mouse area set off the top of the stack.
	 */
	const MouseArea *getMouseAreaSet() const;

	const MouseArea *mouseAreaForPoint(int x, int y);
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
