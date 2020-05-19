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

// Translation table for UTF16->Win1250 (Polish encoding)
// Covers characters 0x80-0xFF.
// '0x0' means end of list, 0x1 means 'unused'
static const uint16 UTF16toWin1250[] = {
	0x20ac, 0x0001, 0x201a, 0x0001, 0x201e, 0x2026, 0x2020, 0x2021,	// 0x80
	0x0001, 0x2030, 0x0160, 0x2039, 0x015a, 0x0164, 0x017d, 0x0179,
	0x0001, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,	// 0x90
	0x0001, 0x2122, 0x0161, 0x203a, 0x015b, 0x0165, 0x017e, 0x017a,
	0x00a0, 0x02c7, 0x02d8, 0x0141, 0x00a4, 0x0104, 0x00a6, 0x00a7,	// 0xa0
	0x00a8, 0x00a9, 0x015e, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x017b,
	0x00b0, 0x00b1, 0x02db, 0x0142, 0x00b4, 0x00b5, 0x00b6, 0x00b7,	// 0xb0
	0x00b8, 0x0105, 0x015f, 0x00bb, 0x013d, 0x02dd, 0x013e, 0x017c,
	0x0154, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0139, 0x0106, 0x00c7,	// 0xc0
	0x010c, 0x00c9, 0x0118, 0x00cb, 0x011a, 0x00cd, 0x00ce, 0x010e,
	0x0110, 0x0143, 0x0147, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x00d7,	// 0xd0
	0x0158, 0x016e, 0x00da, 0x0170, 0x00dc, 0x00dd, 0x0162, 0x00df,
	0x0155, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x013a, 0x0107, 0x00e7,	// 0xe0
	0x010d, 0x00e9, 0x0119, 0x00eb, 0x011b, 0x00ed, 0x00ee, 0x010f,
	0x0111, 0x0144, 0x0148, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x00f7,	// 0xf0
	0x0159, 0x016f, 0x00fa, 0x0171, 0x00fc, 0x00fd, 0x0163, 0x02d9,
	0
};

// Table for transferring Alt+<Key> into Polish symbols
// Note, that 'A' and 'S' are swapped with lowercase. This is due to the fact
// that we use Alt+a for aspect ratio correction and Alt+s for screenshot saving
// and we do that in the SDL backend. There are no words starting with 'A' and
// very few starting with S, thus, this is a viable compromise
static const byte PolishAltifytoWin1250[] = {
	'e', 0xea, 'o', 0xf3, 'a', 0xa5, 's', 0x8c, 'l', 0xb3, 'z', 0xbf, 'x', 0x9f, 'c', 0xe6, 'n', 0xF1,
	'E', 0xca, 'O', 0xd3, 'A', 0xb9, 'S', 0x9c, 'L', 0xa3, 'Z', 0xaf, 'X', 0x8f, 'C', 0xc6, 'N', 0xD1,
	0
};

static const SciKeyConversion keyMappings[] = {
	{ Common::KEYCODE_UP          , kSciKeyUp       , kSciKeyUp       },
	{ Common::KEYCODE_DOWN        , kSciKeyDown     , kSciKeyDown     },
	{ Common::KEYCODE_RIGHT       , kSciKeyRight    , kSciKeyRight    },
	{ Common::KEYCODE_LEFT        , kSciKeyLeft     , kSciKeyLeft     },
	{ Common::KEYCODE_INSERT      , kSciKeyInsert   , kSciKeyInsert   },
	{ Common::KEYCODE_HOME        , kSciKeyHome     , kSciKeyHome     },
	{ Common::KEYCODE_END         , kSciKeyEnd      , kSciKeyEnd      },
	{ Common::KEYCODE_PAGEUP      , kSciKeyPageUp   , kSciKeyPageUp   },
	{ Common::KEYCODE_PAGEDOWN    , kSciKeyPageDown , kSciKeyPageDown },
	{ Common::KEYCODE_DELETE      , kSciKeyDelete   , kSciKeyDelete   },
	{ Common::KEYCODE_KP0         , kSciKeyInsert   , '0'             },
	{ Common::KEYCODE_KP1         , kSciKeyEnd      , '1'             },
	{ Common::KEYCODE_KP2         , kSciKeyDown     , '2'             },
	{ Common::KEYCODE_KP3         , kSciKeyPageDown , '3'             },
	{ Common::KEYCODE_KP4         , kSciKeyLeft     , '4'             },
	{ Common::KEYCODE_KP5         , kSciKeyCenter   , '5'             },
	{ Common::KEYCODE_KP6         , kSciKeyRight    , '6'             },
	{ Common::KEYCODE_KP7         , kSciKeyHome     , '7'             },
	{ Common::KEYCODE_KP8         , kSciKeyUp       , '8'             },
	{ Common::KEYCODE_KP9         , kSciKeyPageUp   , '9'             },
	{ Common::KEYCODE_KP_PERIOD   , kSciKeyDelete   , '.'             },
	{ Common::KEYCODE_KP_ENTER    , kSciKeyEnter    , kSciKeyEnter    },
	{ Common::KEYCODE_KP_PLUS     , '+'             , '+'             },
	{ Common::KEYCODE_KP_MINUS    , '-'             , '-'             },
	{ Common::KEYCODE_KP_MULTIPLY , '*'             , '*'             },
	{ Common::KEYCODE_KP_DIVIDE   , '/'             , '/'             }
};

struct MouseEventConversion {
	Common::EventType commonType;
	SciEventType sciType;
};

static const MouseEventConversion mouseEventMappings[] = {
	{ Common::EVENT_LBUTTONDOWN , kSciEventMousePress   },
	{ Common::EVENT_RBUTTONDOWN , kSciEventMousePress   },
	{ Common::EVENT_MBUTTONDOWN , kSciEventMousePress   },
	{ Common::EVENT_LBUTTONUP   , kSciEventMouseRelease },
	{ Common::EVENT_RBUTTONUP   , kSciEventMouseRelease },
	{ Common::EVENT_MBUTTONUP   , kSciEventMouseRelease }
};

EventManager::EventManager(bool fontIsExtended) :
	_fontIsExtended(fontIsExtended)
#ifdef ENABLE_SCI32
	, _hotRectanglesActive(false)
#endif
	{}

EventManager::~EventManager() {
}

/**
 * Calculates the IBM keyboard alt-key scancode of a printable character.
 */
static int altify(char ch) {
	if (g_sci->getLanguage() == Common::PL_POL) {
		const byte *p = PolishAltifytoWin1250;

		while (*p) {
			if ((byte)ch == *p)
				return *(p + 1);

			p += 2;
		}
	}

	const char c = toupper(ch);

	for (int row = 0; row < ARRAYSIZE(scancodeAltifyRows); ++row) {
		const char *keys = scancodeAltifyRows[row].keys;
		int offset = scancodeAltifyRows[row].offset;

		while (*keys) {
			if (*keys == c)
				return offset << 8;

			++offset;
			++keys;
		}
	}

	return ch;
}

SciEvent EventManager::getScummVMEvent() {
#ifdef ENABLE_SCI32
	SciEvent input   = { kSciEventNone, kSciKeyModNone, 0, Common::Point(), Common::Point(), -1 };
	SciEvent noEvent = { kSciEventNone, kSciKeyModNone, 0, Common::Point(), Common::Point(), -1 };
#else
	SciEvent input   = { kSciEventNone, kSciKeyModNone, 0, Common::Point() };
	SciEvent noEvent = { kSciEventNone, kSciKeyModNone, 0, Common::Point() };
#endif

	Common::EventManager *em = g_system->getEventManager();
	Common::Event ev;

	// SCI does not generate separate events for mouse movement (it puts the
	// current mouse position on every event, including non-mouse events), so
	// skip past all mousemove events in the event queue
	bool found;
	do {
		found = em->pollEvent(ev);
	} while (found && ev.type == Common::EVENT_MOUSEMOVE);

	Common::Point mousePos = em->getMousePos();

#if ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		const GfxFrameout *gfxFrameout = g_sci->_gfxFrameout;

		// This will clamp `mousePos` according to the restricted zone,
		// so any cursor or screen item associated with the mouse position
		// does not bounce when it hits the edge (or ignore the edge)
		g_sci->_gfxCursor32->deviceMoved(mousePos);

		Common::Point mousePosSci = mousePos;
		mulru(mousePosSci, Ratio(gfxFrameout->getScriptWidth(), gfxFrameout->getScreenWidth()), Ratio(gfxFrameout->getScriptHeight(), gfxFrameout->getScreenHeight()));
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
		if (modifiers & Common::KBD_ALT)
			noEvent.modifiers |= kSciKeyModAlt;
		if (modifiers & Common::KBD_CTRL)
			noEvent.modifiers |= kSciKeyModCtrl;
		if (modifiers & Common::KBD_SHIFT)
			noEvent.modifiers |= kSciKeyModShift;

		return noEvent;
	}
	if (ev.type == Common::EVENT_QUIT || ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
		input.type = kSciEventQuit;
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

	// Caps lock and scroll lock are not handled here because we already
	// handle upper case keys elsewhere, and scroll lock doesn't seem to
	// ever be used
	input.modifiers = kSciKeyModNone;
	if (scummVMKeyFlags & Common::KBD_ALT)
		input.modifiers |= kSciKeyModAlt;
	if (scummVMKeyFlags & Common::KBD_CTRL)
		input.modifiers |= kSciKeyModCtrl;
	if (scummVMKeyFlags & Common::KBD_SHIFT)
		input.modifiers |= kSciKeyModShift;

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
				input.modifiers |= kSciKeyModShift; // this value was hardcoded in the mouse interrupt handler
				break;
			case Common::EVENT_MBUTTONDOWN: // middle button
			case Common::EVENT_MBUTTONUP:
				input.modifiers |= kSciKeyModCtrl; // this value was hardcoded in the mouse interrupt handler
				break;
			default:
				break;
			}
			return input;
		}
	}

	// Handle keyboard events for the rest of the function
	if (ev.type != Common::EVENT_KEYDOWN && ev.type != Common::EVENT_KEYUP) {
		return noEvent;
	}

	// The IBM keyboard driver prior to SCI1.1 only sent keydown events to the
	// interpreter
	if (ev.type != Common::EVENT_KEYDOWN && getSciVersion() < SCI_VERSION_1_1) {
		return noEvent;
	}

	const Common::KeyCode scummVMKeycode = ev.kbd.keycode;

	input.character = ev.kbd.ascii;

	if (scummVMKeycode >= Common::KEYCODE_KP0 && scummVMKeycode <= Common::KEYCODE_KP9 && !(scummVMKeyFlags & Common::KBD_NUM)) {
		// TODO: Leaky abstractions from SDL should not be handled in game
		// engines!
		// SDL may provide a character value for numpad keys even if numlock is
		// turned off, but arrow/navigation keys won't get mapped below if a
		// character value is provided
		input.character = 0;
	}

	if (input.character && input.character <= 0xFF) {
		// Extended characters need to be converted to the old to DOS CP850/437
		// character sets for multilingual games
		if (input.character >= 0x80 && input.character <= 0xFF) {
			// SSCI accepted all input scan codes, regardless of locale, and
			// just didn't display any characters that were missing from fonts
			// used by text input controls. We intentionally filter them out
			// entirely for non-multilingual games here instead, so we can have
			// better error detection for bugs in the text controls
			if (!_fontIsExtended) {
				return noEvent;
			}

			input.character = codePageMap88591ToDOS[input.character & 0x7f];
		}

		if (scummVMKeycode == Common::KEYCODE_TAB) {
			input.character = kSciKeyTab;
			if (scummVMKeyFlags & Common::KBD_SHIFT)
				input.character = kSciKeyShiftTab;
		}

		if (scummVMKeycode == Common::KEYCODE_DELETE)
			input.character = kSciKeyDelete;
	} else if (scummVMKeycode >= Common::KEYCODE_F1 && scummVMKeycode <= Common::KEYCODE_F10) {
		if (scummVMKeyFlags & Common::KBD_SHIFT)
			input.character = kSciKeyShiftF1 + ((scummVMKeycode - Common::KEYCODE_F1) << 8);
		else
			input.character = kSciKeyF1 + ((scummVMKeycode - Common::KEYCODE_F1) << 8);
	} else {
		// Arrow keys, numpad keys, etc.
		for (int i = 0; i < ARRAYSIZE(keyMappings); i++) {
			if (keyMappings[i].scummVMKey == scummVMKeycode) {
				const bool numlockOn = (ev.kbd.flags & Common::KBD_NUM);
				input.character = numlockOn ? keyMappings[i].sciKeyNumlockOn : keyMappings[i].sciKeyNumlockOff;
				break;
			}
		}

		if (g_sci->getLanguage() == Common::RU_RUS) {
			// Convert UTF16 to CP866
			if (input.character >= 0x400 && input.character <= 0x4ff) {
				if (input.character >= 0x440)
					input.character = input.character - 0x410 + 0xb0;
				else
					input.character = input.character - 0x410 + 0x80;
			}
		} else if (g_sci->getLanguage() == Common::PL_POL) {
			for (int i = 0; UTF16toWin1250[i]; i++)
				if (UTF16toWin1250[i] == input.character) {
					input.character = 0x80 + i;
					break;
				}
		} else if (g_sci->getLanguage() == Common::HE_ISR) {
			if (input.character >= 0x05d0 && input.character <= 0x05ea)
				// convert to WIN-1255
				input.character = input.character - 0x05d0 + 0xe0;
		}
	}

	// TODO: Leaky abstractions from SDL should not be handled in game engines!
	// When Ctrl and Alt are pressed together with a printable key, SDL1 on
	// Linux will give us a control character instead of the printable
	// character we need to convert to an alt scancode
	if ((scummVMKeyFlags & Common::KBD_ALT) && input.character > 0 && input.character < 27)
		input.character += 96; // 0x01 -> 'a'

	if (scummVMKeyFlags & Common::KBD_ALT) {
		input.character = altify(input.character & 0xFF);
	} else if ((scummVMKeyFlags & Common::KBD_NON_STICKY) == Common::KBD_CTRL && input.character >= 'a' && input.character <= 'z') {
		// In SSCI, Ctrl+<key> generates ASCII control characters, but the
		// backends usually give us a printable character + Ctrl flag, so
		// convert this combo back into what is expected by game scripts
		input.character -= 96;
	}

	// In SCI1.1, if only a modifier key is pressed, the IBM keyboard driver
	// sends an event the same as if a key had been released
	if (getSciVersion() != SCI_VERSION_1_1 && !input.character) {
		return noEvent;
	} else if (!input.character || ev.type == Common::EVENT_KEYUP) {
		input.type = kSciEventKeyUp;

		// SCI32 includes the released key character code in keyup messages, but
		// the IBM keyboard driver in SCI1.1 sends a special character value
		// instead. This is necessary to prevent at least Island of Dr Brain
		// from processing keyup events as though they were keydown events in
		// the word search puzzle
		if (getSciVersion() == SCI_VERSION_1_1) {
			input.character = 0x8000;
		}
	} else {
		input.type = kSciEventKeyDown;
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
		// (EventManager::shouldQuit() and EventManager::shouldReturnToLauncher()),
		// which is very expensive to invoke constantly without any
		// throttling at all.
		if (g_engine->shouldQuit())
			s->abortScriptProcessing = kAbortQuitGame;
	}
}

SciEvent EventManager::getSciEvent(SciEventType mask) {
#ifdef ENABLE_SCI32
	SciEvent event = { kSciEventNone, kSciKeyModNone, 0, Common::Point(), Common::Point(), -1 };
#else
	SciEvent event = { kSciEventNone, kSciKeyModNone, 0, Common::Point() };
#endif

	if (getSciVersion() < SCI_VERSION_2) {
		updateScreen();
	}

	// Get all queued events from graphics driver
	do {
		event = getScummVMEvent();
		if (event.type != kSciEventNone)
			_events.push_back(event);
	} while (event.type != kSciEventNone);

	// Search for matching event in queue
	Common::List<SciEvent>::iterator iter = _events.begin();
	while (iter != _events.end() && !(iter->type & mask))
		++iter;

	if (iter != _events.end()) {
		// Event found
		event = *iter;

		// If not peeking at the queue, remove the event
		if (!(mask & kSciEventPeek))
			_events.erase(iter);
	} else {
		// No event found: we must return a kSciEventNone event.

		// Because event.type is kSciEventNone already here,
		// there is no need to change it.
	}

	return event;
}

void EventManager::flushEvents() {
	Common::EventManager *em = g_system->getEventManager();
	Common::Event event;
	while (em->pollEvent(event)) {}
	_events.clear();
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
				hotRectEvent.type = kSciEventHotRectangle;
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
		hotRectEvent.type = kSciEventHotRectangle;
		hotRectEvent.hotRectangleIndex = -1;
		_events.push_front(hotRectEvent);
	}
}
#endif

} // End of namespace Sci
