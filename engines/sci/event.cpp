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

EventManager::EventManager(bool fontIsExtended) : _fontIsExtended(fontIsExtended) {
}

EventManager::~EventManager() {
}

static const struct scancode_row {
	int offset;
	const char *keys;
} scancode_rows[SCANCODE_ROWS_NR] = {
	{0x10, "QWERTYUIOP[]"},
	{0x1e, "ASDFGHJKL;'\\"},
	{0x2c, "ZXCVBNM,./"}
};

static int altify(int ch) {
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

/*
static int numlockify(int c) {
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
*/

static const byte codepagemap_88591toDOS[0x80] = {
	 '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?', // 0x8x
	 '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?', // 0x9x
	 '?', 0xad, 0x9b, 0x9c,  '?', 0x9d,  '?', 0x9e,  '?',  '?', 0xa6, 0xae, 0xaa,  '?',  '?',  '?', // 0xAx
	 '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?', 0xa7, 0xaf, 0xac, 0xab,  '?', 0xa8, // 0xBx
	 '?',  '?',  '?',  '?', 0x8e, 0x8f, 0x92, 0x80,  '?', 0x90,  '?',  '?',  '?',  '?',  '?',  '?', // 0xCx
	 '?', 0xa5,  '?',  '?',  '?',  '?', 0x99,  '?',  '?',  '?',  '?',  '?', 0x9a,  '?',  '?', 0xe1, // 0xDx
	0x85, 0xa0, 0x83,  '?', 0x84, 0x86, 0x91, 0x87, 0x8a, 0x82, 0x88, 0x89, 0x8d, 0xa1, 0x8c, 0x8b, // 0xEx
	 '?', 0xa4, 0x95, 0xa2, 0x93,  '?', 0x94,  '?',  '?', 0x97, 0xa3, 0x96, 0x81,  '?',  '?', 0x98  // 0xFx
};

SciEvent EventManager::getScummVMEvent() {
	static int _modifierStates = 0;	// FIXME: Avoid non-const global vars
	SciEvent input = { SCI_EVENT_NONE, 0, 0, 0 };

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
		//TODO: SCI_EVM_INSERT

		input.modifiers =
		    ((modifiers & Common::KBD_ALT) ? SCI_KEYMOD_ALT : 0) |
		    ((modifiers & Common::KBD_CTRL) ? SCI_KEYMOD_CTRL : 0) |
		    ((modifiers & Common::KBD_SHIFT) ? SCI_KEYMOD_LSHIFT | SCI_KEYMOD_RSHIFT : 0) |
		    ((ev.kbd.flags & Common::KBD_NUM) ? SCI_KEYMOD_NUMLOCK : 0) |
		    ((ev.kbd.flags & Common::KBD_CAPS) ? SCI_KEYMOD_CAPSLOCK : 0) |
			((ev.kbd.flags & Common::KBD_SCRL) ? SCI_KEYMOD_SCRLOCK : 0) |
			_modifierStates;

		switch (ev.type) {
			// Keyboard events
		case Common::EVENT_KEYDOWN:
			input.data = ev.kbd.keycode;
			input.character = ev.kbd.ascii;

			// Debug console
			if (ev.kbd.hasFlags(Common::KBD_CTRL) && ev.kbd.keycode == Common::KEYCODE_d) {
				// Open debug console
				Console *con = g_sci->getSciDebugger();
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
				if ((input.character >= 0x80) && (input.character <= 0xFF)) {
					// If there is no extended font, we will just clear the current event
					//  Sierra SCI actually accepted those characters, but didn't display them inside textedit-controls
					//  because the characters were missing inside the font(s)
					//  We filter them out for non-multilingual games because of that
					if (!_fontIsExtended) {
						input.type = SCI_EVENT_NONE;
						input.character = 0;
						input.data = 0;
						input.modifiers = 0;
						return input;
					}
					// we get 8859-1 character, we need dos (cp850/437) character for multilingual sci01 games
					// TODO: check, if we get 8859-1 on all platforms
					// Currently checked: Windows XP (works), Wii w/ USB keyboard (works), Mac OS X (works)
					//  Ubuntu (works)
					input.character = codepagemap_88591toDOS[input.character & 0x7f];
				}
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
					input.character = input.data + 0x1900;
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
				case Common::KEYCODE_KP7:	// home
					if (!(_modifierStates & SCI_KEYMOD_NUMLOCK))
						input.data = SCI_KEY_HOME;
					break;
				case Common::KEYCODE_KP9:	// pageup
					if (!(_modifierStates & SCI_KEYMOD_NUMLOCK))
						input.data = SCI_KEY_PGUP;
					break;
				case Common::KEYCODE_KP1:	// end
					if (!(_modifierStates & SCI_KEYMOD_NUMLOCK))
						input.data = SCI_KEY_END;
					break;
				case Common::KEYCODE_KP3:	// pagedown
					if (!(_modifierStates & SCI_KEYMOD_NUMLOCK))
						input.data = SCI_KEY_PGDOWN;
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

SciEvent EventManager::getSciEvent(unsigned int mask) {
	//sci_event_t error_event = { SCI_EVT_ERROR, 0, 0, 0 };
	SciEvent event = { 0, 0, 0, 0 };

	// Update the screen here, since it's called very often.
	// Throttle the screen update rate to 60fps.
	if (g_system->getMillis() - g_sci->getEngineState()->_screenUpdateTime >= 1000 / 60) {
		g_system->updateScreen();
		g_sci->getEngineState()->_screenUpdateTime = g_system->getMillis();
	}

	// Get all queued events from graphics driver
	do {
		event = getScummVMEvent();
		if (event.type != SCI_EVENT_NONE)
			_events.push_back(event);
	} while (event.type != SCI_EVENT_NONE);

	// Search for matching event in queue
	Common::List<SciEvent>::iterator iter = _events.begin();
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

		// When Ctrl AND Alt are pressed together with a regular key, Linux will give us control-key, Windows will give
		//  us the actual key. My opinion is that windows is right, because under DOS the keys worked the same, anyway
		//  we support the other case as well
		if (event.modifiers & SCI_KEYMOD_ALT) {
			if (event.character < 27)
				event.character += 96; // 0x01 -> 'a'
		}

		if (getSciVersion() <= SCI_VERSION_1_MIDDLE) {
			// TODO: find out if altify is also not needed for sci1late+, couldnt find any game that uses those keys
			// Scancodify if appropriate
			if (event.modifiers & SCI_KEYMOD_ALT) {
				event.character = altify(event.character);
			} else if (event.modifiers & SCI_KEYMOD_CTRL) {
				if (event.character < 27)
					event.character += 96; // 0x01 -> 'a'
			}
		}

		// Numlockify if appropriate
		//if (event.modifiers & SCI_KEYMOD_NUMLOCK)
		//	event.data = numlockify(event.data);
		// TODO: dont know yet if this can get dumped as well
	}

	return event;
}

void SciEngine::sleep(uint32 msecs) {
	uint32 time;
	const uint32 wakeup_time = g_system->getMillis() + msecs;

	while (true) {
		// let backend process events and update the screen
		_eventMan->getSciEvent(SCI_EVENT_PEEK);
		time = g_system->getMillis();
		if (time + 10 < wakeup_time) {
			g_system->delayMillis(10);
		} else {
			if (time < wakeup_time)
				g_system->delayMillis(wakeup_time - time);
			break;
		}

	}
}


} // End of namespace Sci
