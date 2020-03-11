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

#ifndef COMMON_EVENTS_H
#define COMMON_EVENTS_H

#include "common/keyboard.h"
#include "common/queue.h"
#include "common/rect.h"
#include "common/noncopyable.h"

#include "common/list.h"
#include "common/singleton.h"

namespace Common {

/**
 * The types of events backends may generate.
 * @see Event
 *
 * @todo Merge EVENT_LBUTTONDOWN, EVENT_RBUTTONDOWN and EVENT_WHEELDOWN;
 *       likewise EVENT_LBUTTONUP, EVENT_RBUTTONUP, EVENT_WHEELUP.
 *       To do that, we just have to add a field to the Event which
 *       indicates which button was pressed.
 */
enum EventType {
	EVENT_INVALID = 0,
	/** A key was pressed, details in Event::kbd. */
	EVENT_KEYDOWN = 1,
	/** A key was released, details in Event::kbd. */
	EVENT_KEYUP = 2,
	/** The mouse moved, details in Event::mouse. */
	EVENT_MOUSEMOVE = 3,
	EVENT_LBUTTONDOWN = 4,
	EVENT_LBUTTONUP = 5,
	EVENT_RBUTTONDOWN = 6,
	EVENT_RBUTTONUP = 7,
	EVENT_WHEELUP = 8,
	EVENT_WHEELDOWN = 9,
	EVENT_MBUTTONDOWN = 13,
	EVENT_MBUTTONUP = 14,

	EVENT_MAINMENU = 15,
	EVENT_RTL = 16,
	EVENT_MUTE = 17,

	EVENT_QUIT = 10,
	EVENT_SCREEN_CHANGED = 11,

	/** The input devices have changed, input related configuration needs to be re-applied */
	EVENT_INPUT_CHANGED  = 35,

	/**
	 * The backend requests the agi engine's predictive dialog to be shown.
	 * TODO: Fingolfin suggests that it would be of better value to expand
	 * on this notion by generalizing its use. For example the backend could
	 * use events to ask for the save game dialog or to pause the engine.
	 * An associated enumerated type can accomplish this.
	 **/
	EVENT_PREDICTIVE_DIALOG = 12,

	EVENT_CUSTOM_BACKEND_ACTION_START = 18,
	EVENT_CUSTOM_BACKEND_ACTION_END   = 19,
	EVENT_CUSTOM_BACKEND_ACTION_AXIS  = 34,
	EVENT_CUSTOM_ENGINE_ACTION_START  = 20,
	EVENT_CUSTOM_ENGINE_ACTION_END    = 21,

#ifdef ENABLE_VKEYBD
	EVENT_VIRTUAL_KEYBOARD = 22,
#endif

	EVENT_DROP_FILE = 23,

	EVENT_JOYAXIS_MOTION = 24,
	EVENT_JOYBUTTON_DOWN = 25,
	EVENT_JOYBUTTON_UP = 26,

	EVENT_CLIPBOARD_UPDATE = 27,

	EVENT_CUSTOM_BACKEND_HARDWARE = 28,
	EVENT_DEBUGGER = 29,

	/**
	 * Additional mouse events, details in Event::mouse.
	 *
	 * Note that X1 and X2 are usually back and forward, however
	 * this can't be guaranteed on all platforms.
	 */
	EVENT_X1BUTTONDOWN = 30,
	EVENT_X1BUTTONUP = 31,
	EVENT_X2BUTTONDOWN = 32,
	EVENT_X2BUTTONUP = 33
};

const int16 JOYAXIS_MIN = -32768;
const int16 JOYAXIS_MAX = 32767;

/**
 * Data structure for joystick events
 */
struct JoystickState {
	/** The axis for EVENT_JOYAXIS_MOTION events */
	byte axis;
	/** The new axis position for EVENT_JOYAXIS_MOTION events */
	int16 position;
	/**
	 * The button index for EVENT_JOYBUTTON_DOWN/UP events
	 *
	 * Some of the button indices match well-known game controller
	 * buttons. See JoystickButton.
	 */
	uint8 button;

	JoystickState() : axis(0), position(0), button(0) {}
};

/**
 *  The list of named buttons available from a joystick
 */
enum JoystickButton {
	JOYSTICK_BUTTON_A,
	JOYSTICK_BUTTON_B,
	JOYSTICK_BUTTON_X,
	JOYSTICK_BUTTON_Y,
	JOYSTICK_BUTTON_BACK,
	JOYSTICK_BUTTON_GUIDE,
	JOYSTICK_BUTTON_START,
	JOYSTICK_BUTTON_LEFT_STICK,
	JOYSTICK_BUTTON_RIGHT_STICK,
	JOYSTICK_BUTTON_LEFT_SHOULDER,
	JOYSTICK_BUTTON_RIGHT_SHOULDER,
	JOYSTICK_BUTTON_DPAD_UP,
	JOYSTICK_BUTTON_DPAD_DOWN,
	JOYSTICK_BUTTON_DPAD_LEFT,
	JOYSTICK_BUTTON_DPAD_RIGHT
};

/**
 *  The list of named axes available from a joystick
 */
enum JoystickAxis {
	JOYSTICK_AXIS_LEFT_STICK_X,
	JOYSTICK_AXIS_LEFT_STICK_Y,
	JOYSTICK_AXIS_RIGHT_STICK_X,
	JOYSTICK_AXIS_RIGHT_STICK_Y,
	JOYSTICK_AXIS_LEFT_TRIGGER,
	JOYSTICK_AXIS_RIGHT_TRIGGER
};

/**
 *  The list named buttons available from a mouse
 */
enum MouseButton {
	MOUSE_BUTTON_LEFT   = 0,
	MOUSE_BUTTON_RIGHT  = 1,
	MOUSE_BUTTON_MIDDLE = 2,
	MOUSE_WHEEL_UP      = 3,
	MOUSE_WHEEL_DOWN    = 4,
	MOUSE_BUTTON_X1     = 5,
	MOUSE_BUTTON_X2     = 6
};

typedef uint32 CustomEventType;

/**
 * Data structure for an event. A pointer to an instance of Event
 * can be passed to pollEvent.
 */
struct Event {

	/** The type of the event. */
	EventType type;

	/**
	 * True if this is a key down repeat event.
	 *
	 * Only valid for EVENT_KEYDOWN events.
	 */
	bool kbdRepeat;

	/**
	  * Keyboard data; only valid for keyboard events (EVENT_KEYDOWN and
	  * EVENT_KEYUP). For all other event types, content is undefined.
	  */
	KeyState kbd;

	/**
	 * The mouse coordinates, in virtual screen coordinates. Only valid
	 * for mouse events.
	 * Virtual screen coordinates means: the coordinate system of the
	 * screen area as defined by the most recent call to initSize().
	 */
	Point mouse;

	CustomEventType customType;

	/* The path of the file or directory dragged to the ScummVM window */
	Common::String path;

	/**
	 * Joystick data; only valid for joystick events (EVENT_JOYAXIS_MOTION,
	 * EVENT_JOYBUTTON_DOWN and EVENT_JOYBUTTON_UP).
	 */
	JoystickState joystick;

	Event() : type(EVENT_INVALID), kbdRepeat(false), customType(0) {
	}
};

/**
 * Determinates whether an event is a mouse event
 *
 * Mouse events have valid mouse coordinates
 */
bool isMouseEvent(const Event &event);

/**
 * A source of Events.
 *
 * An example for this is OSystem, it provides events created by the system
 * and or user.
 */
class EventSource {
public:
	virtual ~EventSource();

	/**
	 * Queries a event from the source.
	 *
	 * @param	event 	a reference to the event struct, where the event should be stored.
	 * @return	true if an event was polled, false otherwise.
	 */
	virtual bool pollEvent(Event &event) = 0;

	/**
	 * Checks whether events from this source are allowed to be mapped.
	 *
	 * Possible event sources not allowing mapping are: the event recorder/player and/or
	 * the EventManager, which allows user events to be pushed.
	 *
	 * By default we allow mapping for every event source.
	 */
	virtual bool allowMapping() const { return true; }
};

/**
 * An artificial event source. This is class is used as an event source, which is
 * made up by client specific events.
 *
 * Example usage cases for this are the Keymapper or the DefaultEventManager.
 */
class ArtificialEventSource : public EventSource {
protected:
	Queue<Event> _artificialEventQueue;
public:
	void addEvent(const Event &ev) {
		_artificialEventQueue.push(ev);
	}

	bool pollEvent(Event &ev) {
	if (!_artificialEventQueue.empty()) {
			ev = _artificialEventQueue.pop();
			return true;
		} else {
			return false;
		}
	}

	/**
	 * By default an artificial event source prevents its events
	 * from being mapped.
	 */
	virtual bool allowMapping() const { return false; }
};

/**
 * Object which catches and processes Events.
 *
 * An example for this is the Engine object, it is catching events and processing them.
 */
class EventObserver {
public:
	virtual ~EventObserver();

	/**
	 * Notifies the observer of an incoming event.
	 *
	 * An observer is supposed to eat the event, with returning true, when
	 * it wants to prevent other observers from receiving the event.
	 * A usage example here is the keymapper:
	 * If it processes an Event, it should 'eat' it and create a new
	 * event, which the EventDispatcher will then catch.
	 *
	 * @param   event   the event, which is incoming.
	 * @return  true if the event should not be passed to other observers,
	 *          false otherwise.
	 */
	virtual bool notifyEvent(const Event &event) = 0;

	/**
	 * Notifies the observer of pollEvent() query.
	 */
	virtual void notifyPoll() { }
};

/**
 * A event mapper, which will map events to others.
 *
 * An example for this is the Keymapper.
 */
class EventMapper {
public:
	virtual ~EventMapper();

	/**
	 * Map an incoming event to one or more action events
	 */
	virtual List<Event> mapEvent(const Event &ev) = 0;
};

/**
 * Dispatches events from various sources to various observers.
 *
 * EventDispatcher is using a priority based approach. Observers
 * with higher priority will be notified before observers with
 * lower priority. Because of the possibility that oberservers
 * might 'eat' events, not all observers might be notified.
 *
 * Another speciality is the support for a event mapper, which
 * will catch events and create new events out of them. This
 * mapper will be processed before an event is sent to the
 * observers.
 */
class EventDispatcher {
public:
	EventDispatcher();
	~EventDispatcher();

	/**
	 * Tries to catch events from the registered event
	 * sources and dispatch them to the observers.
	 *
	 * This dispatches *all* events the sources offer.
	 */
	void dispatch();

	/**
	 * Clear all events currently in the event queue.
	 * The cleared events are not dispatched and are simply discarded.
	 */
	void clearEvents();

	/**
	 * Registers an event mapper with the dispatcher.
	 */
	void registerMapper(EventMapper *mapper);

	/**
	 * Registers a new EventSource with the Dispatcher.
	 */
	void registerSource(EventSource *source, bool autoFree);

	/**
	 * Unregisters a EventSource.
	 *
	 * This takes the "autoFree" flag passed to registerSource into account.
	 */
	void unregisterSource(EventSource *source);

	/**
	 * Registers a new EventObserver with the Dispatcher.
	 *
	 * @param listenPolls if set, then all pollEvent() calls are passed to observer
	 *                    currently it is used by keyMapper
	 */
	void registerObserver(EventObserver *obs, uint priority, bool autoFree, bool listenPolls = false);

	/**
	 * Unregisters a EventObserver.
	 *
	 * This takes the "autoFree" flag passed to registerObserver into account.
	 */
	void unregisterObserver(EventObserver *obs);
private:
	EventMapper *_mapper;

	struct Entry {
		bool autoFree;
	};

	struct SourceEntry : public Entry {
		EventSource *source;
	};

	List<SourceEntry> _sources;

	struct ObserverEntry : public Entry {
		uint priority;
		EventObserver *observer;
		bool poll;
	};

	List<ObserverEntry> _observers;

	void dispatchEvent(const Event &event);
	void dispatchPoll();
};

class Keymap;
class Keymapper;

/**
 * The EventManager provides user input events to the client code.
 * In addition, it keeps track of the state of various input devices,
 * like keys, mouse position and buttons.
 */
class EventManager : NonCopyable {
public:
	virtual ~EventManager();

	enum {
		LBUTTON = 1 << MOUSE_BUTTON_LEFT,
		RBUTTON = 1 << MOUSE_BUTTON_RIGHT
	};


	/**
	 * Initialize the event manager.
	 * @note	called after graphics system has been set up
	 */
	virtual void init() {}

	/**
	 * Get the next event in the event queue.
	 * @param event	point to an Event struct, which will be filled with the event data.
	 * @return true if an event was retrieved.
	 */
	virtual bool pollEvent(Event &event) = 0;

	/**
	 * Pushes a "fake" event into the event queue
	 */
	virtual void pushEvent(const Event &event) = 0;

	/**
	 * Purges all unprocessed mouse events already in the event queue.
	 */
	virtual void purgeMouseEvents() = 0;

	/** Return the current mouse position */
	virtual Point getMousePos() const = 0;

	/**
	 * Return a bitmask with the button states:
	 * - bit 0: left button up=0, down=1
	 * - bit 1: right button up=0, down=1
	 */
	virtual int getButtonState() const = 0;

	/** Get a bitmask with the current modifier state */
	virtual int getModifierState() const = 0;

	/**
	 * Should the application terminate? Set to true if we
	 * received an EVENT_QUIT.
	 */
	virtual int shouldQuit() const = 0;

	/**
	 * Should we return to the launcher?
	 */
	virtual int shouldRTL() const = 0;

	/**
	 * Reset the "return to launcher" flag (as returned shouldRTL()) to false.
	 * Used when we have returned to the launcher.
	 */
	virtual void resetRTL() = 0;
#ifdef FORCE_RTL
	virtual void resetQuit() = 0;
#endif
	// Optional: check whether a given key is currently pressed ????
	//virtual bool isKeyPressed(int keycode) = 0;

	// TODO: Keyboard repeat support?

	// TODO: Consider removing OSystem::getScreenChangeID and
	// replacing it by a generic getScreenChangeID method here

	virtual Keymapper *getKeymapper() = 0;
	virtual Keymap *getGlobalKeymap() = 0;

	enum {
		/**
		 * Priority of the event manager, for now it's lowest since it eats
		 * *all* events, we might to change that in the future though.
		 */
		kEventManPriority = 0,
		/**
		 * Priority of the event recorder. It has to go after event manager
		 * in order to record events generated by it
		 */
		kEventRecorderPriority = 1,
		/**
		 * Priority of the remap dialog. It has to go first to capture all
		 * the events before they are consumed by other observers.
		 */
		kEventRemapperPriority = 999
	};

	/**
	 * Returns the underlying EventDispatcher.
	 */
	EventDispatcher *getEventDispatcher() { return &_dispatcher; }

protected:
	EventDispatcher _dispatcher;
};

} // End of namespace Common

#endif
