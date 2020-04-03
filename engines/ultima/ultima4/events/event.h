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

#ifndef ULTIMA4_EVENT_H
#define ULTIMA4_EVENT_H

#include "common/events.h"
#include "common/list.h"
#include "common/str.h"
#include "ultima/ultima4/events/controller.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

#define eventHandler (EventHandler::getInstance())

#define U4_UP           '['
#define U4_DOWN         '/'
#define U4_LEFT         ';'
#define U4_RIGHT        '\''
#define U4_BACKSPACE    8
#define U4_TAB          9
#define U4_SPACE        ' '
#define U4_ESC          27
#define U4_ENTER        13
#define U4_ALT          128
#define U4_KEYPAD_ENTER 271
#define U4_META         323
#define U4_FKEY         282
#define U4_RIGHT_SHIFT  303
#define U4_LEFT_SHIFT   304
#define U4_RIGHT_CTRL   305
#define U4_LEFT_CTRL    306
#define U4_RIGHT_ALT    307
#define U4_LEFT_ALT     308
#define U4_RIGHT_META   309
#define U4_LEFT_META    310

extern int eventTimerGranularity;

struct MouseArea;
class EventHandler;
class TextView;

/**
 * A class for handling keystrokes.
 */
class KeyHandler {
public:
	virtual ~KeyHandler() {}

	/* Typedefs */
	typedef bool (*Callback)(int, void *);

	/** Additional information to be passed as data param for read buffer key handler */
	typedef struct ReadBuffer {
		int (*_handleBuffer)(Common::String *);
		Common::String *_buffer;
		int _bufferLen;
		int _screenX, _screenY;
	} ReadBuffer;

	/** Additional information to be passed as data param for get choice key handler */
	typedef struct GetChoice {
		Common::String _choices;
		int (*_handleChoice)(int);
	} GetChoice;

	/* Constructors */
	KeyHandler(Callback func, void *data = NULL, bool asyncronous = true);

	/* Static functions */
	static int setKeyRepeat(int delay, int interval);

	/**
	 * Handles any and all keystrokes.
	 * Generally used to exit the application, switch applications,
	 * minimize, maximize, etc.
	 */
	static bool globalHandler(int key);

	/* Static default key handler functions */
	/**
	 * A default key handler that should be valid everywhere
	 */
	static bool defaultHandler(int key, void *data);

	/**
	 * A key handler that ignores keypresses
	 */
	static bool ignoreKeys(int key, void *data);

	/* Operators */
	bool operator==(Callback cb) const;

	/* Member functions */
	/**
	 * Handles a keypress.
	 * First it makes sure the key combination is not ignored
	 * by the current key handler. Then, it passes the keypress
	 * through the global key handler. If the global handler
	 * does not process the keystroke, then the key handler
	 * handles it itself by calling its handler callback function.
	 */
	bool handle(int key);

	/**
	 * Returns true if the key or key combination is always ignored by xu4
	 */
	virtual bool isKeyIgnored(int key);

protected:
	Callback _handler;
	bool _async;
	void *_data;
};

/**
 * A controller that wraps a keyhander function.  Keyhandlers are
 * deprecated -- please use a controller instead.
 */
class KeyHandlerController : public Controller {
public:
	KeyHandlerController(KeyHandler *handler);
	~KeyHandlerController();

	virtual bool keyPressed(int key);
	KeyHandler *getKeyHandler();

private:
	KeyHandler *_handler;
};

/**
 * A controller to read a Common::String, terminated by the enter key.
 */
class ReadStringController : public WaitableController<Common::String> {
public:
	/**
	 * @param maxlen the maximum length of the Common::String
	 * @param screenX the screen column where to begin input
	 * @param screenY the screen row where to begin input
	 * @param accepted_chars a Common::String characters to be accepted for input
	 */
	ReadStringController(int maxlen, int screenX, int screenY, const Common::String &accepted_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 \n\r\010");
	ReadStringController(int maxlen, TextView *view, const Common::String &accepted_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 \n\r\010");
	virtual bool keyPressed(int key);

	static Common::String get(int maxlen, int screenX, int screenY, EventHandler *eh = NULL);
	static Common::String get(int maxlen, TextView *view, EventHandler *eh = NULL);
#ifdef IOS
	void setValue(const Common::String &utf8StringValue) {
		value = utf8StringValue;
	}
#endif

protected:
	int _maxLen, _screenX, _screenY;
	TextView *_view;
	Common::String _accepted;
};

/**
 * A controller to read a integer, terminated by the enter key.
 * Non-numeric keys are ignored.
 */
class ReadIntController : public ReadStringController {
public:
	ReadIntController(int maxlen, int screenX, int screenY);

	static int get(int maxlen, int screenX, int screenY, EventHandler *eh = NULL);
	int getInt() const;
};

/**
 * A controller to read a single key from a provided list.
 */
class ReadChoiceController : public WaitableController<int> {
public:
	ReadChoiceController(const Common::String &choices);
	virtual bool keyPressed(int key);

	static char get(const Common::String &choices, EventHandler *eh = NULL);

protected:
	Common::String _choices;
};

/**
 * A controller to read a direction enter with the arrow keys.
 */
class ReadDirController : public WaitableController<Direction> {
public:
	ReadDirController();
	virtual bool keyPressed(int key);
};

/**
 * A controller to pause for a given length of time, ignoring all
 * keyboard input.
 */
class WaitController : public Controller {
public:
	WaitController(unsigned int cycles);
	virtual bool keyPressed(int key);
	virtual void timerFired();

	void wait();
	void setCycles(int c);

private:
	unsigned int _cycles;
	unsigned int _current;
};

/**
 * A class for handling timed events.
 */
class TimedEvent {
public:
	/* Typedefs */
	typedef Common::List<TimedEvent *> List;
	typedef void (*Callback)(void *);

	/* Constructors */
	TimedEvent(Callback callback, int interval, void *data = NULL);

	/* Member functions */
	Callback getCallback() const;
	void *getData();

	/**
	 * Advances the timed event forward a tick.
	 * When (current >= interval), then it executes its callback function.
	 */
	void tick();

	/* Properties */
protected:
	Callback _callback;
	void *_data;
	int _interval;
	int _current;
};

#if defined(IOS)
#ifndef __OBJC__
typedef void *TimedManagerHelper;
typedef void *UIEvent;
#else
@class TimedManagerHelper;
@class UIEvent;
#endif
#endif


/**
 * A class for managing timed events
 */
class TimedEventMgr {
public:
	/* Typedefs */
	typedef TimedEvent::List List;

	/* Constructors */
	/**
	 * Constructs a timed event manager object.
	 * Adds a timer callback to the SDL subsystem, which
	 * will drive all of the timed events that this object
	 * controls.
	 */
	TimedEventMgr(int baseInterval);

	/**
	 * Destructs a timed event manager object.
	 * It removes the callback timer and un-initializes the
	 * SDL subsystem if there are no other active TimedEventMgr
	 * objects.
	 */
	~TimedEventMgr();

	/* Static functions */
	/**
	 * Adds an SDL timer event to the message queue.
	 */
	static unsigned int callback(unsigned int interval, void *param);

	/* Member functions */

	/**
	 * Returns true if the event queue is locked (in use)
	 */
	bool isLocked() const;

	/**
	 * Adds a timed event to the event queue.
	 */
	void add(TimedEvent::Callback theCallback, int interval, void *data = NULL);

	/**
	 * Removes a timed event from the event queue.
	 */
	List::iterator remove(List::iterator i);
	void remove(TimedEvent *event);
	void remove(TimedEvent::Callback theCallback, void *data = NULL);

	/**
	 * Runs each of the callback functions of the TimedEvents associated with this manager.
	 */
	void tick();
	void stop();
	void start();

	/**
	 * Re-initializes the timer manager to a new timer granularity
	 */
	void reset(unsigned int interval);     /**< Re-initializes the event manager to a new base interval */
#if defined(IOS)
	bool hasActiveTimer() const;
#endif

private:
	void lock();                /**< Locks the event list */
	void unlock();              /**< Unlocks the event list */

	/* Properties */
protected:
	/* Static properties */
	static unsigned int _instances;

	void *_id;
	int _baseInterval;
	bool _locked;
	List _events;
	List _deferredRemovals;
#if defined(IOS)
	TimedManagerHelper *m_helper;
#endif
};

typedef void(*updateScreenCallback)(void);
/**
 * A class for handling game events.
 */
class EventHandler {
public:
	/* Typedefs */
	typedef Common::List<const MouseArea *> MouseAreaList;

	/* Constructors */
	/**
	 * Constructs an event handler object.
	 */
	EventHandler();

	/* Static functions */
	static EventHandler *getInstance();

	/**
	 * Delays program execution for the specified number of milliseconds.
	 * This doesn't actually stop events, but it stops the user from interacting
	 * While some important event happens (e.g., getting hit by a cannon ball or a spell effect).
	 */
	static void sleep(unsigned int usec);

	/**
	 * Waits a given number of milliseconds before continuing
	 */
	static void wait_msecs(unsigned int msecs);

	/**
	 * Waits a given number of game cycles before continuing
	 */
	static void wait_cycles(unsigned int cycles);

	static void setControllerDone(bool exit = true);
	static bool getControllerDone();
	static void end();

	/**
	 * Returns true if the queue is empty of events that match 'mask'.
	 */
	static bool timerQueueEmpty();

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
	 * Returns NULL if there is no key handler.
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

protected:
	static bool _controllerDone;
	static bool _ended;
	TimedEventMgr _timer;
	Std::vector<Controller *> _controllers;
	MouseAreaList _mouseAreaSets;
	updateScreenCallback _updateScreen;

private:
	static EventHandler *_instance;
	uint32 _lastTickTime;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
