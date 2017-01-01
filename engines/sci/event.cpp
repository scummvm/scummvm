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

#include "common/system.h"
#include "common/events.h"
#include "common/file.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/console.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#ifdef ENABLE_SCI32
#include "sci/graphics/cursor32.h"
#include "sci/graphics/frameout.h"
#endif
#include "sci/graphics/screen.h"

namespace Sci {

struct ScancodeRow {
	int offset;
	const char *keys;
};

static const ScancodeRow scancodeAltifyRows[] = {
	{ 0x10, "QWERTYUIOP[]"  },
	{ 0x1e, "ASDFGHJKL;'\\" },
	{ 0x2c, "ZXCVBNM,./"    }
};

static const byte codePageMap88591ToDOS[0x80] = {
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

static const SciKeyConversion keyMappings[] = {
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

struct MouseEventConversion {
	Common::EventType commonType;
	short sciType;
};

static const MouseEventConversion mouseEventMappings[] = {
	{ Common::EVENT_LBUTTONDOWN,   SCI_EVENT_MOUSE_PRESS },
	{ Common::EVENT_RBUTTONDOWN,   SCI_EVENT_MOUSE_PRESS },
	{ Common::EVENT_MBUTTONDOWN,   SCI_EVENT_MOUSE_PRESS },
	{   Common::EVENT_LBUTTONUP, SCI_EVENT_MOUSE_RELEASE },
	{   Common::EVENT_RBUTTONUP, SCI_EVENT_MOUSE_RELEASE },
	{   Common::EVENT_MBUTTONUP, SCI_EVENT_MOUSE_RELEASE }
};

EventManager::EventManager(bool fontIsExtended) :
	_fontIsExtended(fontIsExtended)
#ifdef ENABLE_SCI32
	, _hotRectanglesActive(false)
#endif
	{}

EventManager::~EventManager() {
}

static int altify(int ch) {
	// Calculates a PC keyboard scancode from a character */
	int row;
	int c = toupper((char)ch);

	for (row = 0; row < ARRAYSIZE(scancodeAltifyRows); row++) {
		const char *keys = scancodeAltifyRows[row].keys;
		int offset = scancodeAltifyRows[row].offset;

		while (*keys) {
			if (*keys == c)
				return offset << 8;

			offset++;
			keys++;
		}
	}

	return ch;
}

SciEvent EventManager::getScummVMEvent() {
#ifdef ENABLE_SCI32
	SciEvent input = { SCI_EVENT_NONE, 0, 0, Common::Point(), Common::Point(), -1 };
	SciEvent noEvent = { SCI_EVENT_NONE, 0, 0, Common::Point(), Common::Point(), -1 };
#else
	SciEvent input = { SCI_EVENT_NONE, 0, 0, Common::Point() };
	SciEvent noEvent = { SCI_EVENT_NONE, 0, 0, Common::Point() };
#endif

	Common::EventManager *em = g_system->getEventManager();
	Common::Event ev;

	bool found = em->pollEvent(ev);

	// Don't generate events for mouse movement
	while (found && ev.type == Common::EVENT_MOUSEMOVE)
		found = em->pollEvent(ev);

	// Save the mouse position
	//
	// We call getMousePos of the event manager here, since we also want to
	// store the mouse position in case of keyboard events, which do not feature
	// any mouse position information itself.
	// This should be safe, since the mouse position in the event manager should
	// only be updated when a mouse related event has been taken from the queue
	// via pollEvent.
	// We also adjust the position based on the scaling of the screen.
	Common::Point mousePos = em->getMousePos();

#if ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		const Buffer &screen = g_sci->_gfxFrameout->getCurrentBuffer();

		// This will clamp `mousePos` according to the restricted zone,
		// so any cursor or screen item associated with the mouse position
		// does not bounce when it hits the edge (or ignore the edge)
		g_sci->_gfxCursor32->deviceMoved(mousePos);

		Common::Point mousePosSci = mousePos;
		mulru(mousePosSci, Ratio(screen.scriptWidth, screen.screenWidth), Ratio(screen.scriptHeight, screen.screenHeight));
		noEvent.mousePosSci = input.mousePosSci = mousePosSci;

		if (_hotRectanglesActive) {
			checkHotRectangles(mousePosSci);
		}
	} else {
#endif
		g_sci->_gfxScreen->adjustBackUpscaledCoordinates(mousePos.y, mousePos.x);
#if ENABLE_SCI32
	}
#endif

	noEvent.mousePos = input.mousePos = mousePos;

	if (!found || ev.type == Common::EVENT_MOUSEMOVE) {
		int modifiers = em->getModifierState();
		noEvent.modifiers =
			((modifiers & Common::KBD_ALT) ? SCI_KEYMOD_ALT : 0) |
			((modifiers & Common::KBD_CTRL) ? SCI_KEYMOD_CTRL : 0) |
			((modifiers & Common::KBD_SHIFT) ? SCI_KEYMOD_LSHIFT | SCI_KEYMOD_RSHIFT : 0);

		return noEvent;
	}
	if (ev.type == Common::EVENT_QUIT || ev.type == Common::EVENT_RTL) {
		input.type = SCI_EVENT_QUIT;
		return input;
	}

	int scummVMKeyFlags;

	switch (ev.type) {
	case Common::EVENT_KEYDOWN:
	case Common::EVENT_KEYUP:
		// Use keyboard modifiers directly in case this is a keyboard event
		scummVMKeyFlags = ev.kbd.flags;
		break;
	default:
		// Otherwise get them from EventManager
		scummVMKeyFlags = em->getModifierState();
		break;
	}

	input.modifiers =
		((scummVMKeyFlags & Common::KBD_ALT) ? SCI_KEYMOD_ALT : 0) |
		((scummVMKeyFlags & Common::KBD_CTRL) ? SCI_KEYMOD_CTRL : 0) |
		((scummVMKeyFlags & Common::KBD_SHIFT) ? SCI_KEYMOD_LSHIFT | SCI_KEYMOD_RSHIFT : 0);
		// Caps lock and Scroll lock have been removed, cause we already handle upper
		// case keys and Scroll lock doesn't seem to be used anywhere
		//((ourModifiers & Common::KBD_CAPS) ? SCI_KEYMOD_CAPSLOCK : 0) |
		//((ourModifiers & Common::KBD_SCRL) ? SCI_KEYMOD_SCRLOCK : 0) |

	// Handle mouse events
	for (int i = 0; i < ARRAYSIZE(mouseEventMappings); i++) {
		if (mouseEventMappings[i].commonType == ev.type) {
			input.type = mouseEventMappings[i].sciType;
			// Sierra passed keyboard modifiers for mouse events, too.

			// Sierra also set certain modifiers within their mouse interrupt handler
			// This whole thing was probably meant for people using a mouse, that only featured 1 button
			// So the user was able to press Ctrl and click the mouse button to create a right click.
			switch (ev.type) {
			case Common::EVENT_RBUTTONDOWN: // right button
			case Common::EVENT_RBUTTONUP:
				input.modifiers |= (SCI_KEYMOD_RSHIFT | SCI_KEYMOD_LSHIFT); // this value was hardcoded in the mouse interrupt handler
				break;
			case Common::EVENT_MBUTTONDOWN: // middle button
			case Common::EVENT_MBUTTONUP:
				input.modifiers |= SCI_KEYMOD_CTRL; // this value was hardcoded in the mouse interrupt handler
				break;
			default:
				break;
			}
			return input;
		}
	}

	// If we reached here, make sure that it's a keydown event
	if (ev.type != Common::EVENT_KEYDOWN)
		return noEvent;

	// Check for Control-Shift-D (debug console)
	if (ev.kbd.hasFlags(Common::KBD_CTRL | Common::KBD_SHIFT) && ev.kbd.keycode == Common::KEYCODE_d) {
		// Open debug console
		Console *con = g_sci->getSciDebugger();
		con->attach();
		return noEvent;
	}

	// Process keyboard events

	bool numlockOn = (ev.kbd.flags & Common::KBD_NUM);

	Common::KeyCode scummVMKeycode = ev.kbd.keycode;

	input.character = ev.kbd.ascii;
	input.type = SCI_EVENT_KEYBOARD;

	if (scummVMKeycode >= Common::KEYCODE_KP0 && scummVMKeycode <= Common::KEYCODE_KP9) {
		if (!(scummVMKeyFlags & Common::KBD_NUM)) {
			// HACK: Num-Lock not enabled
			// We shouldn't get a valid ascii code in these cases. We fix it here, so that cursor keys
			// on the numpad work properly.
			input.character = 0;
		}
	}

	if ((input.character) && (input.character <= 0xFF)) {
		// Directly accept most common keys without conversion
		if ((input.character >= 0x80) && (input.character <= 0xFF)) {
			// If there is no extended font, we will just clear the
			// current event.
			// Sierra SCI actually accepted those characters, but
			// didn't display them inside text edit controls because
			// the characters were missing inside the font(s).
			// We filter them out for non-multilingual games because
			// of that.
			if (!_fontIsExtended)
				return noEvent;
			// Convert 8859-1 characters to DOS (cp850/437) for
			// multilingual SCI01 games
			input.character = codePageMap88591ToDOS[input.character & 0x7f];
		}
		if (scummVMKeycode == Common::KEYCODE_TAB) {
			input.character = SCI_KEY_TAB;
			if (scummVMKeyFlags & Common::KBD_SHIFT)
				input.character = SCI_KEY_SHIFT_TAB;
		}
		if (scummVMKeycode == Common::KEYCODE_DELETE)
			input.character = SCI_KEY_DELETE;
	} else if ((scummVMKeycode >= Common::KEYCODE_F1) && scummVMKeycode <= Common::KEYCODE_F10) {
		// SCI_K_F1 == 59 << 8
		// SCI_K_SHIFT_F1 == 84 << 8
		if (!(scummVMKeyFlags & Common::KBD_SHIFT))
			input.character = SCI_KEY_F1 + ((scummVMKeycode - Common::KEYCODE_F1)<<8);
		else
			input.character = SCI_KEY_SHIFT_F1 + ((scummVMKeycode - Common::KEYCODE_F1)<<8);
	} else {
		// Special keys that need conversion
		for (int i = 0; i < ARRAYSIZE(keyMappings); i++) {
			if (keyMappings[i].scummVMKey == scummVMKeycode) {
				input.character = numlockOn ? keyMappings[i].sciKeyNumlockOn : keyMappings[i].sciKeyNumlockOff;
				break;
			}
		}
	}

	// When Ctrl AND Alt are pressed together with a regular key, Linux will give us control-key, Windows will give
	//  us the actual key. My opinion is that windows is right, because under DOS the keys worked the same, anyway
	//  we support the other case as well
	if ((scummVMKeyFlags & Common::KBD_ALT) && input.character > 0 && input.character < 27)
		input.character += 96; // 0x01 -> 'a'

	// Scancodify if appropriate
	if (scummVMKeyFlags & Common::KBD_ALT)
		input.character = altify(input.character);
	if (getSciVersion() <= SCI_VERSION_1_MIDDLE && (scummVMKeyFlags & Common::KBD_CTRL) && input.character > 0 && input.character < 27)
		input.character += 96; // 0x01 -> 'a'
#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2 && (scummVMKeyFlags & Common::KBD_CTRL) && input.character == 'c') {
		input.character = SCI_KEY_ETX;
	}
#endif

	// If no actual key was pressed (e.g. if only a modifier key was pressed),
	// ignore the event
	if (!input.character)
		return noEvent;

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

SciEvent EventManager::getSciEvent(uint32 mask) {
#ifdef ENABLE_SCI32
	SciEvent event = { SCI_EVENT_NONE, 0, 0, Common::Point(), Common::Point(), -1 };
#else
	SciEvent event = { SCI_EVENT_NONE, 0, 0, Common::Point() };
#endif

	if (getSciVersion() < SCI_VERSION_2) {
		updateScreen();
	}

	// Get all queued events from graphics driver
	do {
		event = getScummVMEvent();
		if (event.type != SCI_EVENT_NONE)
			_events.push_back(event);
	} while (event.type != SCI_EVENT_NONE);

	// Search for matching event in queue
	Common::List<SciEvent>::iterator iter = _events.begin();
	while (iter != _events.end() && !(iter->type & mask))
		++iter;

	if (iter != _events.end()) {
		// Event found
		event = *iter;

		// If not peeking at the queue, remove the event
		if (!(mask & SCI_EVENT_PEEK))
			_events.erase(iter);
	} else {
		// No event found: we must return a SCI_EVT_NONE event.

		// Because event.type is SCI_EVT_NONE already here,
		// there is no need to change it.
	}

	return event;
}

#ifdef ENABLE_SCI32
void EventManager::setHotRectanglesActive(const bool active) {
	_hotRectanglesActive = active;
}

void EventManager::setHotRectangles(const Common::Array<Common::Rect> &rects) {
	_hotRects = rects;
	_activeRectIndex = -1;
}

void EventManager::checkHotRectangles(const Common::Point &mousePosition) {
	int lastActiveRectIndex = _activeRectIndex;
	_activeRectIndex = -1;

	for (int16 i = 0; i < (int16)_hotRects.size(); ++i) {
		if (_hotRects[i].contains(mousePosition)) {
			_activeRectIndex = i;
			if (i != lastActiveRectIndex) {
				SciEvent hotRectEvent;
				hotRectEvent.type = SCI_EVENT_HOT_RECTANGLE;
				hotRectEvent.hotRectangleIndex = i;
				_events.push_front(hotRectEvent);
				break;
			}

			lastActiveRectIndex = _activeRectIndex;
		}
	}

	if (lastActiveRectIndex != _activeRectIndex && lastActiveRectIndex != -1) {
		_activeRectIndex = -1;
		SciEvent hotRectEvent;
		hotRectEvent.type = SCI_EVENT_HOT_RECTANGLE;
		hotRectEvent.hotRectangleIndex = -1;
		_events.push_front(hotRectEvent);
	}
}
#endif

} // End of namespace Sci
