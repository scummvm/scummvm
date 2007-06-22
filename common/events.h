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
 *       likewise EVENT_LBUTTONUP, EVENT_RBUTTONUP, EVENT_WHEELUP.
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
	EVENT_SCREEN_CHANGED = 11,
	/**
	 * The backend requests the agi engine's predictive dialog to be shown.
	 * TODO: Fingolfin suggests that it would be of better value to expand
	 * on this notion by generalizing its use. For example the backend could
	 * use events to ask for the save game dialog or to pause the engine.
	 * An associated enumerated type can accomplish this.
	 **/
	EVENT_PREDICTIVE_DIALOG = 12
};

enum KeyCode {
	KEYCODE_BACKSPACE		= 8,
	KEYCODE_TAB		= 9,
	KEYCODE_CLEAR		= 12,
	KEYCODE_RETURN		= 13,
	KEYCODE_PAUSE		= 19,
	KEYCODE_ESCAPE		= 27,
	KEYCODE_SPACE		= 32,
	KEYCODE_EXCLAIM		= 33,
	KEYCODE_QUOTEDBL		= 34,
	KEYCODE_HASH		= 35,
	KEYCODE_DOLLAR		= 36,
	KEYCODE_AMPERSAND		= 38,
	KEYCODE_QUOTE		= 39,
	KEYCODE_LEFTPAREN		= 40,
	KEYCODE_RIGHTPAREN		= 41,
	KEYCODE_ASTERISK		= 42,
	KEYCODE_PLUS		= 43,
	KEYCODE_COMMA		= 44,
	KEYCODE_MINUS		= 45,
	KEYCODE_PERIOD		= 46,
	KEYCODE_SLASH		= 47,
	KEYCODE_0			= 48,
	KEYCODE_1			= 49,
	KEYCODE_2			= 50,
	KEYCODE_3			= 51,
	KEYCODE_4			= 52,
	KEYCODE_5			= 53,
	KEYCODE_6			= 54,
	KEYCODE_7			= 55,
	KEYCODE_8			= 56,
	KEYCODE_9			= 57,
	KEYCODE_COLON		= 58,
	KEYCODE_SEMICOLON		= 59,
	KEYCODE_LESS		= 60,
	KEYCODE_EQUALS		= 61,
	KEYCODE_GREATER		= 62,
	KEYCODE_QUESTION		= 63,
	KEYCODE_AT			= 64,

	KEYCODE_LEFTBRACKET	= 91,
	KEYCODE_BACKSLASH		= 92,
	KEYCODE_RIGHTBRACKET	= 93,
	KEYCODE_CARET		= 94,
	KEYCODE_UNDERSCORE		= 95,
	KEYCODE_BACKQUOTE		= 96,
	KEYCODE_a			= 97,
	KEYCODE_b			= 98,
	KEYCODE_c			= 99,
	KEYCODE_d			= 100,
	KEYCODE_e			= 101,
	KEYCODE_f			= 102,
	KEYCODE_g			= 103,
	KEYCODE_h			= 104,
	KEYCODE_i			= 105,
	KEYCODE_j			= 106,
	KEYCODE_k			= 107,
	KEYCODE_l			= 108,
	KEYCODE_m			= 109,
	KEYCODE_n			= 110,
	KEYCODE_o			= 111,
	KEYCODE_p			= 112,
	KEYCODE_q			= 113,
	KEYCODE_r			= 114,
	KEYCODE_s			= 115,
	KEYCODE_t			= 116,
	KEYCODE_u			= 117,
	KEYCODE_v			= 118,
	KEYCODE_w			= 119,
	KEYCODE_x			= 120,
	KEYCODE_y			= 121,
	KEYCODE_z			= 122,
	KEYCODE_DELETE		= 127,
	
	// Numeric keypad
	KEYCODE_KP0		= 256,
	KEYCODE_KP1		= 257,
	KEYCODE_KP2		= 258,
	KEYCODE_KP3		= 259,
	KEYCODE_KP4		= 260,
	KEYCODE_KP5		= 261,
	KEYCODE_KP6		= 262,
	KEYCODE_KP7		= 263,
	KEYCODE_KP8		= 264,
	KEYCODE_KP9		= 265,
	KEYCODE_KP_PERIOD		= 266,
	KEYCODE_KP_DIVIDE		= 267,
	KEYCODE_KP_MULTIPLY	= 268,
	KEYCODE_KP_MINUS		= 269,
	KEYCODE_KP_PLUS		= 270,
	KEYCODE_KP_ENTER		= 271,
	KEYCODE_KP_EQUALS		= 272,

	// Arrows + Home/End pad
	KEYCODE_UP			= 273,
	KEYCODE_DOWN		= 274,
	KEYCODE_RIGHT		= 275,
	KEYCODE_LEFT		= 276,
	KEYCODE_INSERT		= 277,
	KEYCODE_HOME		= 278,
	KEYCODE_END		= 279,
	KEYCODE_PAGEUP		= 280,
	KEYCODE_PAGEDOWN		= 281,

	// Function keys
	KEYCODE_F1			= 282,
	KEYCODE_F2			= 283,
	KEYCODE_F3			= 284,
	KEYCODE_F4			= 285,
	KEYCODE_F5			= 286,
	KEYCODE_F6			= 287,
	KEYCODE_F7			= 288,
	KEYCODE_F8			= 289,
	KEYCODE_F9			= 290,
	KEYCODE_F10		= 291,
	KEYCODE_F11		= 292,
	KEYCODE_F12		= 293,
	KEYCODE_F13		= 294,
	KEYCODE_F14		= 295,
	KEYCODE_F15		= 296,

	// Key state modifier keys
	KEYCODE_NUMLOCK		= 300,
	KEYCODE_CAPSLOCK		= 301,
	KEYCODE_SCROLLOCK		= 302,
	KEYCODE_RSHIFT		= 303,
	KEYCODE_LSHIFT		= 304,
	KEYCODE_RCTRL		= 305,
	KEYCODE_LCTRL		= 306,
	KEYCODE_RALT		= 307,
	KEYCODE_LALT		= 308,
	KEYCODE_RMETA		= 309,
	KEYCODE_LMETA		= 310,
	KEYCODE_LSUPER		= 311,		// Left "Windows" key
	KEYCODE_RSUPER		= 312,		// Right "Windows" key
	KEYCODE_MODE		= 313,		// "Alt Gr" key
	KEYCODE_COMPOSE		= 314,		// Multi-key compose key

	// Miscellaneous function keys 
	KEYCODE_HELP		= 315,
	KEYCODE_PRINT		= 316,
	KEYCODE_SYSREQ		= 317,
	KEYCODE_BREAK		= 318,
	KEYCODE_MENU		= 319,
	KEYCODE_POWER		= 320,		// Power Macintosh power key
	KEYCODE_EURO		= 321,		// Some european keyboards
	KEYCODE_UNDO		= 322		// Atari keyboard has Undo
};

/**
 * List of fake 'ascii' values used in keyboard events.
 * The values here are based on what certain SCUMM games require
 * in their scripts.
 * @todo Get rid of this, and instead enforce that engines use the
 * keycode instead to handle these.
 */
enum {
	ASCII_F1 = 315,
	ASCII_F2 = 316,
	ASCII_F3 = 317,
	ASCII_F4 = 318,
	ASCII_F5 = 319,
	ASCII_F6 = 320,
	ASCII_F7 = 321,
	ASCII_F8 = 322,
	ASCII_F9 = 323
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
		KeyCode keycode;
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
