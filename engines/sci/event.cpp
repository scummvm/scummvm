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

#include "common/system.h"
#include "common/events.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/console.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"

namespace Sci {

#define SCANCODE_ROWS_NR 3

SciEvent::SciEvent() {
}

SciEvent::~SciEvent() {
}

struct scancode_row {
	int offset;
	const char *keys;
} scancode_rows[SCANCODE_ROWS_NR] = {
	{0x10, "QWERTYUIOP[]"},
	{0x1e, "ASDFGHJKL;'\\"},
	{0x2c, "ZXCVBNM,./"}
};

int SciEvent::controlify(int ch) {
	if ((ch >= 97) && (ch <= 121)) {
		ch -= 96; // 'a' -> 0x01, etc.
	}
	return ch;
}

int SciEvent::altify (int ch) {
	// Calculates a PC keyboard scancode from a character */
	int row;
	int c = toupper((char)ch);

	for (row = 0; row < SCANCODE_ROWS_NR; row++) {
		const char *keys = scancode_rows[row].keys;
		int offset = scancode_rows[row].offset;

		while (*keys) {
			if (*keys == c)
				return offset << 8;

			offset++;
			keys++;
		}
	}

	return ch;
}

int SciEvent::shiftify (int c) {
	char shifted_numbers[] = ")!@#$%^&*(";

	if (c < 256) {
		c = toupper((char)c);

		if (c >= 'A' && c <= 'Z')
			return c;

		if (c >= '0' && c <= '9')
			return shifted_numbers[c-'0'];

		switch (c) {
		case SCI_KEY_TAB:
			return SCI_KEY_SHIFT_TAB;
		case ']':
			return '}';
		case '[':
			return '{';
		case '`':
			return '~';
		case '-':
			return '_';
		case '=':
			return '+';
		case ';':
			return ':';
		case '\'':
			return '"';
		case '\\':
			return '|';
		case ',':
			return '<';
		case '.':
			return '>';
		case '/':
			return '?';
		default:
			return c; // No match
		}
	}

	if (c >= SCI_KEY_F1 && c <= SCI_KEY_F10)
		return c + 25;

	return c;
}

int SciEvent::numlockify (int c) {
	switch (c) {
	case SCI_KEY_DELETE:
		return '.';
	case SCI_KEY_INSERT:
		return '0';
	case SCI_KEY_END:
		return '1';
	case SCI_KEY_DOWN:
		return '2';
	case SCI_KEY_PGDOWN:
		return '3';
	case SCI_KEY_LEFT:
		return '4';
	case SCI_KEY_CENTER:
		return '5';
	case SCI_KEY_RIGHT:
		return '6';
	case SCI_KEY_HOME:
		return '7';
	case SCI_KEY_UP:
		return '8';
	case SCI_KEY_PGUP:
		return '9';
	default:
		return c; // Unchanged
	}
}

sciEvent SciEvent::getFromScummVM() {
	static int _modifierStates = 0;	// FIXME: Avoid non-const global vars
	sciEvent input = { SCI_EVENT_NONE, 0, 0, 0 };

	Common::EventManager *em = g_system->getEventManager();
	Common::Event ev;

	bool found = em->pollEvent(ev);
	Common::Point p = ev.mouse;

	// Don't generate events for mouse movement
	while (found && ev.type == Common::EVENT_MOUSEMOVE) {
		found = em->pollEvent(ev);
	}

	if (found && !ev.synthetic && ev.type != Common::EVENT_MOUSEMOVE) {
		int modifiers = em->getModifierState();

		// We add the modifier key status to buckybits
		// SDL sends a keydown event if a modifier key is turned on and a keyup event if it's off
		//
		// FIXME: This code is semi-bogus. It only records the modifier key being *pressed*.
		// It does not track correctly whether capslock etc. is active. To do that, we
		// would have to record the fact that the modifier was pressed in global var,
		// and also watch for Common::EVENT_KEYUP events.
		// But this is still not quite good enough, because not all events might
		// pass through here (e.g. the GUI might be running with its own event loop).
		//
		// The best solution likely would be to add code to the EventManager class
		// for tracking which keys are pressed and which are not...
		if (ev.type == Common::EVENT_KEYDOWN || ev.type == Common::EVENT_KEYUP) {
			switch (ev.kbd.keycode) {
			case Common::KEYCODE_CAPSLOCK:
				if (ev.type == Common::EVENT_KEYDOWN) {
					_modifierStates |= SCI_KEYMOD_CAPSLOCK;
				} else {
					_modifierStates &= ~SCI_KEYMOD_CAPSLOCK;
				}
				break;
			case Common::KEYCODE_NUMLOCK:
				if (ev.type == Common::EVENT_KEYDOWN) {
					_modifierStates |= SCI_KEYMOD_NUMLOCK;
				} else {
					_modifierStates &= ~SCI_KEYMOD_NUMLOCK;
				}
				break;
			case Common::KEYCODE_SCROLLOCK:
				if (ev.type == Common::EVENT_KEYDOWN) {
					_modifierStates |= SCI_KEYMOD_SCRLOCK;
				} else {
					_modifierStates &= ~SCI_KEYMOD_SCRLOCK;
				}
				break;
			default:
				break;
			}
		}
		//TODO: SCI_EVM_INSERT

		input.modifiers =
		    ((modifiers & Common::KBD_ALT) ? SCI_KEYMOD_ALT : 0) |
		    ((modifiers & Common::KBD_CTRL) ? SCI_KEYMOD_CTRL : 0) |
		    ((modifiers & Common::KBD_SHIFT) ? SCI_KEYMOD_LSHIFT | SCI_KEYMOD_RSHIFT : 0) |
			_modifierStates;

		switch (ev.type) {
			// Keyboard events
		case Common::EVENT_KEYDOWN:
			input.data = ev.kbd.keycode;
			input.character = ev.kbd.ascii;

			// Debug console
			if (ev.kbd.flags == Common::KBD_CTRL && ev.kbd.keycode == Common::KEYCODE_d) {
				// Open debug console
				Console *con = ((Sci::SciEngine*)g_engine)->getSciDebugger();
				con->attach();

				// Clear keyboard event
				input.type = SCI_EVENT_NONE;
				input.character = 0;
				input.data = 0;
				input.modifiers = 0;

				return input;
			}

			if (!(input.data & 0xFF00)) {
				// Directly accept most common keys without conversion
				input.type = SCI_EVENT_KEYBOARD;
				if (input.data == Common::KEYCODE_TAB) {
					// Tab
					input.type = SCI_EVENT_KEYBOARD;
					input.data = SCI_KEY_TAB;
					if (input.modifiers & (SCI_KEYMOD_LSHIFT | SCI_KEYMOD_RSHIFT))
						input.character = SCI_KEY_SHIFT_TAB;
					else
						input.character = SCI_KEY_TAB;
				}
			} else if ((input.data >= Common::KEYCODE_F1) && input.data <= Common::KEYCODE_F10) {
				// F1-F10
				input.type = SCI_EVENT_KEYBOARD;
				// SCI_K_F1 == 59 << 8
				// SCI_K_SHIFT_F1 == 84 << 8
				input.data = SCI_KEY_F1 + ((input.data - Common::KEYCODE_F1)<<8);
				if (input.modifiers & (SCI_KEYMOD_LSHIFT | SCI_KEYMOD_RSHIFT))
					input.character = input.data + 25;
				else
					input.character = input.data;
			} else {
				// Special keys that need conversion
				input.type = SCI_EVENT_KEYBOARD;
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_UP:
					input.data = SCI_KEY_UP;
					break;
				case Common::KEYCODE_DOWN:
					input.data = SCI_KEY_DOWN;
					break;
				case Common::KEYCODE_RIGHT:
					input.data = SCI_KEY_RIGHT;
					break;
				case Common::KEYCODE_LEFT:
					input.data = SCI_KEY_LEFT;
					break;
				case Common::KEYCODE_INSERT:
					input.data = SCI_KEY_INSERT;
					break;
				case Common::KEYCODE_HOME:
					input.data = SCI_KEY_HOME;
					break;
				case Common::KEYCODE_END:
					input.data = SCI_KEY_END;
					break;
				case Common::KEYCODE_PAGEUP:
					input.data = SCI_KEY_PGUP;
					break;
				case Common::KEYCODE_PAGEDOWN:
					input.data = SCI_KEY_PGDOWN;
					break;
				case Common::KEYCODE_DELETE:
					input.data = SCI_KEY_DELETE;
					break;
				// Keypad keys
				case Common::KEYCODE_KP8:	// up
					if (!(_modifierStates & SCI_KEYMOD_NUMLOCK))
						input.data = SCI_KEY_UP;
					break;
				case Common::KEYCODE_KP2:	// down
					if (!(_modifierStates & SCI_KEYMOD_NUMLOCK))
						input.data = SCI_KEY_DOWN;
					break;
				case Common::KEYCODE_KP6:	// right
					if (!(_modifierStates & SCI_KEYMOD_NUMLOCK))
						input.data = SCI_KEY_RIGHT;
					break;
				case Common::KEYCODE_KP4:	// left
					if (!(_modifierStates & SCI_KEYMOD_NUMLOCK))
						input.data = SCI_KEY_LEFT;
					break;
				case Common::KEYCODE_KP5:	// center
					if (!(_modifierStates & SCI_KEYMOD_NUMLOCK))
						input.data = SCI_KEY_CENTER;
					break;
				default:
					input.type = SCI_EVENT_NONE;
					break;
				}
				input.character = input.data;
			}
			break;

			// Mouse events
		case Common::EVENT_LBUTTONDOWN:
			input.type = SCI_EVENT_MOUSE_PRESS;
			input.data = 1;
			break;
		case Common::EVENT_RBUTTONDOWN:
			input.type = SCI_EVENT_MOUSE_PRESS;
			input.data = 2;
			break;
		case Common::EVENT_LBUTTONUP:
			input.type = SCI_EVENT_MOUSE_RELEASE;
			input.data = 1;
			break;
		case Common::EVENT_RBUTTONUP:
			input.type = SCI_EVENT_MOUSE_RELEASE;
			input.data = 2;
			break;

			// Misc events
		case Common::EVENT_QUIT:
			input.type = SCI_EVENT_QUIT;
			break;

		default:
			break;
		}
	}

	return input;
}

sciEvent SciEvent::get(unsigned int mask) {
	//sci_event_t error_event = { SCI_EVT_ERROR, 0, 0, 0 };
	sciEvent event = { 0, 0, 0, 0 };

	// TODO: we need to call SciGuiCursor::refreshPosition() before each screen update to limit the mouse cursor position

	// Update the screen here, since it's called very often
	g_system->updateScreen();

	// Get all queued events from graphics driver
	do {
		event = getFromScummVM();
		if (event.type != SCI_EVENT_NONE)
			_events.push_back(event);
	} while (event.type != SCI_EVENT_NONE);

	// Search for matching event in queue
	Common::List<sciEvent>::iterator iter = _events.begin();
	while (iter != _events.end() && !((*iter).type & mask))
		++iter;

	if (iter != _events.end()) {
		// Event found
		event = *iter;

		// If not peeking at the queue, remove the event
		if (!(mask & SCI_EVENT_PEEK)) {
			_events.erase(iter);
		}
	} else {
		// No event found: we must return a SCI_EVT_NONE event.

		// Because event.type is SCI_EVT_NONE already here,
		// there is no need to change it.
	}

	if (event.type == SCI_EVENT_KEYBOARD) {
		// Do we still have to translate the key?

		event.character = event.data;

		// TODO: Remove this as soon as ScummVM handles Ctrl-Alt-X to us
		if ((event.modifiers == SCI_KEYMOD_CTRL) && (event.character = 'x'))
			event.modifiers |= SCI_KEYMOD_ALT;

		// Scancodify if appropriate
		if (event.modifiers & SCI_KEYMOD_ALT) {
			event.character = altify(event.character);
		} else if (event.modifiers & SCI_KEYMOD_CTRL) {
			event.character = controlify(event.character);
		}

		// Shift if appropriate
		else if (((event.modifiers & (SCI_KEYMOD_RSHIFT | SCI_KEYMOD_LSHIFT)) && !(event.modifiers & SCI_KEYMOD_CAPSLOCK))
		         || (!(event.modifiers & (SCI_KEYMOD_RSHIFT | SCI_KEYMOD_LSHIFT)) && (event.modifiers & SCI_KEYMOD_CAPSLOCK)))
			event.character = shiftify(event.character);

		// Numlockify if appropriate
		else if (event.modifiers & SCI_KEYMOD_NUMLOCK)
			event.data = numlockify(event.data);
	}

	return event;
}

} // End of namespace Sci
