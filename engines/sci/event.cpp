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
#include "common/file.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/console.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"

namespace Sci {

EventManager::EventManager(bool fontIsExtended) : _fontIsExtended(fontIsExtended), _modifierStates(0) {

	if (getSciVersion() >= SCI_VERSION_1_MIDDLE) {
		_usesNewKeyboardDirectionType = true;
	} else if (getSciVersion() <= SCI_VERSION_01) {
		_usesNewKeyboardDirectionType = false;
	} else {
		// they changed this somewhere inbetween SCI1EGA/EARLY
		_usesNewKeyboardDirectionType = false;

		// We are looking if script 933 exists, that one has the PseudoMouse class in it that handles it
		//  The good thing is that PseudoMouse seems to only exists in games that use the new method
		if (g_sci->getResMan()->testResource(ResourceId(kResourceTypeScript, 933)))
			_usesNewKeyboardDirectionType = true;
		// Checking the keyboard driver size in here would also be a valid method, but the driver is only available
		//  in PC versions of the game
	}
}

EventManager::~EventManager() {
}

bool EventManager::getUsesNewKeyboardDirectionType() {
	return _usesNewKeyboardDirectionType;
}

struct ScancodeRow {
	int offset;
	const char *keys;
};

const ScancodeRow s_scancodeRows[] = {
	{ 0x10, "QWERTYUIOP[]"  },
	{ 0x1e, "ASDFGHJKL;'\\" },
	{ 0x2c, "ZXCVBNM,./"    }
};

static int altify(int ch) {
	// Calculates a PC keyboard scancode from a character */
	int row;
	int c = toupper((char)ch);

	for (row = 0; row < ARRAYSIZE(s_scancodeRows); row++) {
		const char *keys = s_scancodeRows[row].keys;
		int offset = s_scancodeRows[row].offset;

		while (*keys) {
			if (*keys == c)
				return offset << 8;

			offset++;
			keys++;
		}
	}

	return ch;
}

const byte codepagemap_88591toDOS[0x80] = {
	 '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?', // 0x8x
	 '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?', // 0x9x
	 '?', 0xad, 0x9b, 0x9c,  '?', 0x9d,  '?', 0x9e,  '?',  '?', 0xa6, 0xae, 0xaa,  '?',  '?',  '?', // 0xAx
	 '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?', 0xa7, 0xaf, 0xac, 0xab,  '?', 0xa8, // 0xBx
	 '?',  '?',  '?',  '?', 0x8e, 0x8f, 0x92, 0x80,  '?', 0x90,  '?',  '?',  '?',  '?',  '?',  '?', // 0xCx
	 '?', 0xa5,  '?',  '?',  '?',  '?', 0x99,  '?',  '?',  '?',  '?',  '?', 0x9a,  '?',  '?', 0xe1, // 0xDx
	0x85, 0xa0, 0x83,  '?', 0x84, 0x86, 0x91, 0x87, 0x8a, 0x82, 0x88, 0x89, 0x8d, 0xa1, 0x8c, 0x8b, // 0xEx
	 '?', 0xa4, 0x95, 0xa2, 0x93,  '?', 0x94,  '?',  '?', 0x97, 0xa3, 0x96, 0x81,  '?',  '?', 0x98  // 0xFx
};

struct SciKeyConversion {
	Common::KeyCode scummVMKey;
	int sciKeyNumlockOff;
	int sciKeyNumlockOn;
};

const SciKeyConversion keyMappings[] = {
	{ Common::KEYCODE_UP          , SCI_KEY_UP     , SCI_KEY_UP     },
	{ Common::KEYCODE_DOWN        , SCI_KEY_DOWN   , SCI_KEY_DOWN   },
	{ Common::KEYCODE_RIGHT       , SCI_KEY_RIGHT  , SCI_KEY_RIGHT  },
	{ Common::KEYCODE_LEFT        , SCI_KEY_LEFT   , SCI_KEY_LEFT   },
	{ Common::KEYCODE_INSERT      , SCI_KEY_INSERT , SCI_KEY_INSERT },
	{ Common::KEYCODE_HOME        , SCI_KEY_HOME   , SCI_KEY_HOME   },
	{ Common::KEYCODE_END         , SCI_KEY_END    , SCI_KEY_END    },
	{ Common::KEYCODE_PAGEUP      , SCI_KEY_PGUP   , SCI_KEY_PGUP   },
	{ Common::KEYCODE_PAGEDOWN    , SCI_KEY_PGDOWN , SCI_KEY_PGDOWN },
	{ Common::KEYCODE_DELETE      , SCI_KEY_DELETE , SCI_KEY_DELETE },
	// Keypad
	{ Common::KEYCODE_KP0         , SCI_KEY_INSERT , '0'            },
	{ Common::KEYCODE_KP1         , SCI_KEY_END    , '1'            },
	{ Common::KEYCODE_KP2         , SCI_KEY_DOWN   , '2'            },
	{ Common::KEYCODE_KP3         , SCI_KEY_PGDOWN , '3'            },
	{ Common::KEYCODE_KP4         , SCI_KEY_LEFT   , '4'            },
	{ Common::KEYCODE_KP5         , SCI_KEY_CENTER , '5'            },
	{ Common::KEYCODE_KP6         , SCI_KEY_RIGHT  , '6'            },
	{ Common::KEYCODE_KP7         , SCI_KEY_HOME   , '7'            },
	{ Common::KEYCODE_KP8         , SCI_KEY_UP     , '8'            },
	{ Common::KEYCODE_KP9         , SCI_KEY_PGUP   , '9'            },
	{ Common::KEYCODE_KP_PERIOD   , SCI_KEY_DELETE , '.'            },
	{ Common::KEYCODE_KP_ENTER    , SCI_KEY_ENTER  , SCI_KEY_ENTER  },
	{ Common::KEYCODE_KP_PLUS     , '+'            , '+'            },
	{ Common::KEYCODE_KP_MINUS    , '-'            , '-'            },
	{ Common::KEYCODE_KP_MULTIPLY , '*'            , '*'            },
	{ Common::KEYCODE_KP_DIVIDE   , '/'            , '/'            },
};

SciEvent EventManager::getScummVMEvent() {
	SciEvent input = { SCI_EVENT_NONE, 0, 0, 0 };

	Common::EventManager *em = g_system->getEventManager();
	Common::Event ev;

	bool found = em->pollEvent(ev);
	Common::Point p = ev.mouse;

	// Don't generate events for mouse movement
	while (found && ev.type == Common::EVENT_MOUSEMOVE) {
		found = em->pollEvent(ev);
	}

	if (found && ev.type != Common::EVENT_MOUSEMOVE) {
		int modifiers = em->getModifierState();
		bool numlockOn = (ev.kbd.flags & Common::KBD_NUM);

		// We add the modifier key status to buckybits
		//TODO: SCI_EVM_INSERT

		input.modifiers =
		    ((modifiers & Common::KBD_ALT) ? SCI_KEYMOD_ALT : 0) |
		    ((modifiers & Common::KBD_CTRL) ? SCI_KEYMOD_CTRL : 0) |
		    ((modifiers & Common::KBD_SHIFT) ? SCI_KEYMOD_LSHIFT | SCI_KEYMOD_RSHIFT : 0) |
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
					// We get a 8859-1 character, we need dos (cp850/437) character for multilingual sci01 games
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
				if (input.data == Common::KEYCODE_DELETE) {
					// Delete key
					input.type = SCI_EVENT_KEYBOARD;
					input.data = input.character = SCI_KEY_DELETE;
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
				for (int i = 0; i < ARRAYSIZE(keyMappings); i++) {
					if (keyMappings[i].scummVMKey == ev.kbd.keycode) {
						input.data = numlockOn ? keyMappings[i].sciKeyNumlockOn : keyMappings[i].sciKeyNumlockOff;
						break;
					}
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
		case Common::EVENT_MBUTTONDOWN:
			input.type = SCI_EVENT_MOUSE_PRESS;
			input.data = 3;
			break;
		case Common::EVENT_LBUTTONUP:
			input.type = SCI_EVENT_MOUSE_RELEASE;
			input.data = 1;
			break;
		case Common::EVENT_RBUTTONUP:
			input.type = SCI_EVENT_MOUSE_RELEASE;
			input.data = 2;
			break;
		case Common::EVENT_MBUTTONUP:
			input.type = SCI_EVENT_MOUSE_RELEASE;
			input.data = 3;
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

void EventManager::updateScreen() {
	// Update the screen here, since it's called very often.
	// Throttle the screen update rate to 60fps.
	EngineState *s = g_sci->getEngineState();
	if (g_system->getMillis() - s->_screenUpdateTime >= 1000 / 60) {
		g_system->updateScreen();
		s->_screenUpdateTime = g_system->getMillis();
		// Throttle the checking of shouldQuit() to 60fps as well, since 
		// Engine::shouldQuit() invokes 2 virtual functions
		// (EventManager::shouldQuit() and EventManager::shouldRTL()),
		// which is very expensive to invoke constantly without any
		// throttling at all.
		if (g_engine->shouldQuit())
			s->abortScriptProcessing = kAbortQuitGame;
	}
}

SciEvent EventManager::getSciEvent(unsigned int mask) {
	//sci_event_t error_event = { SCI_EVT_ERROR, 0, 0, 0 };
	SciEvent event = { 0, 0, 0, 0 };

	EventManager::updateScreen();

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
