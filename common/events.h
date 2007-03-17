/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2007 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#ifndef COMMON_EVENTS_H
#define COMMON_EVENTS_H

#include "common/rect.h"
#include "common/system.h"
#include "common/noncopyable.h"

namespace Common {

/**
 * The types of events backends may generate.
 * @see Event
 *
 * @todo Merge EVENT_LBUTTONDOWN, EVENT_RBUTTONDOWN and EVENT_WHEELDOWN;
 *       likewiese EVENT_LBUTTONUP, EVENT_RBUTTONUP, EVENT_WHEELUP.
 *       To do that, we just have to add a field to the Event which
 *       indicates which button was pressed.
 */
enum EventType {
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

	EVENT_QUIT = 10,
	EVENT_SCREEN_CHANGED = 11
};

/**
 * Keyboard modifier flags, used for Event::kbd::flags.
 */
enum {
	KBD_CTRL  = 1 << 0,
	KBD_ALT   = 1 << 1,
	KBD_SHIFT = 1 << 2
};

/**
 * Data structure for an event. A pointer to an instance of Event
 * can be passed to pollEvent.
 * @todo Rework/document this structure. It should be made 100% clear which
 *       field is valid for which event type.
 *       Implementation wise, we might want to use the classic
 *       union-of-structs trick. It goes roughly like this:
 *       struct BasicEvent {
 *       	EventType type;
 *       };
 *       struct MouseMovedEvent : BasicEvent {
 *       	Common::Point pos;
 *       };
 *       struct MouseButtonEvent : MouseMovedEvent {
 *       	int button;
 *       };
 *       struct KeyEvent : BasicEvent {
 *       	...
 *       };
 *       ...
 *       union Event {
 *          EventType type;
 *       	MouseMovedEvent mouse;
 *       	MouseButtonEvent button;
 *       	KeyEvent key;
 *       	...
 *       };
 */
struct Event {
	/** The type of the event. */
	EventType type;
	/** Flag to indicate if the event is real or synthetic. E.g. keyboard
	  * repeat events are synthetic.
	  */
	bool synthetic;
	/**
	  * Keyboard data; only valid for keyboard events (EVENT_KEYDOWN and
	  * EVENT_KEYUP). For all other event types, content is undefined.
	  */
	struct {
		/**
		 * Abstract key code (will be the same for any given key regardless
		 * of modifiers being held at the same time.
		 * For example, this is the same for both 'A' and Shift-'A'.
		 * @todo Document which values are to be used for non-ASCII keys
		 * like F1-F10. For now, let's just say that our primary backend
		 * is the SDL one, and it uses the values SDL uses... so until
		 * we fix this, your best bet is to get a copy of SDL_keysym.h
		 * and look at that, if you want to find out a key code.
		 */
		int keycode;
		/**
		 * ASCII-value of the pressed key (if any).
		 * This depends on modifiers, i.e. pressing the 'A' key results in
		 * different values here depending on the status of shift, alt and
		 * caps lock.
		 * For the function keys F1-F9, values of 315-323 are used.
		 */
		uint16 ascii;
		/**
		 * Status of the modifier keys. Bits are set in this for each
		 * pressed modifier
		 * @see KBD_CTRL, KBD_ALT, KBD_SHIFT
		 */
		byte flags;
	} kbd;
	/**
	 * The mouse coordinates, in virtual screen coordinates. Only valid
	 * for mouse events.
	 * Virtual screen coordinates means: the coordinate system of the
	 * screen area as defined by the most recent call to initSize().
	 */
	Common::Point mouse;
};


/**
 * The EventManager provides user input events to the client code.
 * In addition, it keeps track of the state of various input devices,
 * like keys, mouse position and buttons.
 */
class EventManager : NonCopyable {
public:
	EventManager() {}
	virtual ~EventManager() {}
	
	enum {
		LBUTTON = 1 << 0,
		RBUTTON = 1 << 1
	};

	/**
	 * Get the next event in the event queue.
	 * @param event	point to an Event struct, which will be filled with the event data.
	 * @return true if an event was retrieved.
	 */
	virtual bool pollEvent(Common::Event &event) = 0;


	/** Return the current key state */
	virtual Common::Point getMousePos() const = 0;
	
	/**
	 * Return a bitmask with the button states:
	 * - bit 0: left button up=1, down=0
	 * - bit 1: right button up=1, down=0
	 */
	virtual int getButtonState() const = 0;
	
	/** Get a bitmask with the current modifier state */
	virtual int getModifierState() const = 0;

	/**
	 * Should the application terminate? Set to true if we
	 * received an EVENT_QUIT.
	 */
	virtual int shouldQuit() const = 0;
	
	// Optional: check whether a given key is currently pressed ????
	//virtual bool isKeyPressed(int keycode) = 0;

	// TODO: Keyboard repeat support?
	
	// TODO: Consider removing OSystem::getScreenChangeID and
	// replacing it by a generic getScreenChangeID method here
};

} // End of namespace Common

#endif
