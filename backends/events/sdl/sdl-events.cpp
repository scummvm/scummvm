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

#include "common/scummsys.h"

#if defined(SDL_BACKEND)

#include "backends/events/sdl/sdl-events.h"
#include "backends/platform/sdl/sdl.h"
#include "backends/graphics/graphics.h"
#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/fs.h"
#include "engines/engine.h"
#include "gui/gui-manager.h"

// FIXME move joystick defines out and replace with confile file options
// we should really allow users to map any key to a joystick button

// #define JOY_INVERT_Y
#define JOY_XAXIS 0
#define JOY_YAXIS 1
// buttons
#define JOY_BUT_LMOUSE 0
#define JOY_BUT_RMOUSE 2
#define JOY_BUT_ESCAPE 3
#define JOY_BUT_PERIOD 1
#define JOY_BUT_SPACE 4
#define JOY_BUT_F5 5
#ifdef ENABLE_VKEYBD
#define JOY_BUT_VKEYBOARD 7
#endif


#if SDL_VERSION_ATLEAST(2, 0, 0)
#define GAMECONTROLLERDB_FILE "gamecontrollerdb.txt"

static uint32 convUTF8ToUTF32(const char *src) {
	uint32 utf32 = 0;

	char *dst = SDL_iconv_string(
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	                             "UTF-32BE",
#else
	                             "UTF-32LE",
#endif
                                 "UTF-8", src, SDL_strlen(src) + 1);

	if (dst) {
		utf32 = *((uint32 *)dst);
		SDL_free(dst);
	}

	return utf32;
}

void SdlEventSource::loadGameControllerMappingFile() {
	bool loaded = false;
	if (ConfMan.hasKey("controller_map_db")) {
		Common::FSNode file = Common::FSNode(ConfMan.get("controller_map_db"));
		if (file.exists()) {
			if (SDL_GameControllerAddMappingsFromFile(file.getPath().c_str()) < 0)
				error("File %s not valid: %s", file.getPath().c_str(), SDL_GetError());	
			else {
				loaded = true;
				debug("Game controller DB file loaded: %s", file.getPath().c_str());
			}
		} else
			warning("Game controller DB file not found: %s", file.getPath().c_str());
	}
	if (!loaded && ConfMan.hasKey("extrapath")) {
		Common::FSNode dir = Common::FSNode(ConfMan.get("extrapath"));
		Common::FSNode file = dir.getChild(GAMECONTROLLERDB_FILE);
		if (file.exists()) {
			if (SDL_GameControllerAddMappingsFromFile(file.getPath().c_str()) < 0)
				error("File %s not valid: %s", file.getPath().c_str(), SDL_GetError());	
			else
				debug("Game controller DB file loaded: %s", file.getPath().c_str());
		}
	}
}
#endif

SdlEventSource::SdlEventSource()
    : EventSource(), _scrollLock(false), _joystick(0), _lastScreenID(0), _graphicsManager(0), _queuedFakeMouseMove(false), _lastHatPosition(SDL_HAT_CENTERED)
#if SDL_VERSION_ATLEAST(2, 0, 0)
      , _queuedFakeKeyUp(false), _fakeKeyUp(), _controller(nullptr)
#endif
      {
	// Reset mouse state
	memset(&_km, 0, sizeof(_km));

	int joystick_num = ConfMan.getInt("joystick_num");
	if (joystick_num >= 0) {
		// Initialize SDL joystick subsystem
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1) {
			error("Could not initialize SDL: %s", SDL_GetError());
		}

#if SDL_VERSION_ATLEAST(2, 0, 0)
		if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) == -1) {
			error("Could not initialize SDL: %s", SDL_GetError());
		}
		loadGameControllerMappingFile();
#endif

		openJoystick(joystick_num);
	}
}

SdlEventSource::~SdlEventSource() {
	closeJoystick();
}

int SdlEventSource::mapKey(SDLKey sdlKey, SDLMod mod, Uint16 unicode) {
	Common::KeyCode key = SDLToOSystemKeycode(sdlKey);

	// Keep unicode in case it's regular ASCII text or in case we didn't get a valid keycode
	//
	// We need to use unicode in those cases, simply because SDL1.x passes us non-layout-adjusted keycodes.
	// So unicode is the only way to get layout-adjusted keys.
	if (unicode < 0x20) {
		// don't use unicode, in case it's control characters
		unicode = 0;
	} else {
		// Use unicode, in case keycode is invalid.
		// Umlauts and others will set KEYCODE_INVALID on SDL2, so in such a case always keep unicode.
		if (key != Common::KEYCODE_INVALID) {
			// keycode is valid, check further also depending on modifiers
			if (mod & (KMOD_CTRL | KMOD_ALT)) {
				// Ctrl and/or Alt is active
				//
				// We need to restrict unicode to only up to 0x7E, because on macOS the option/alt key will switch to
				// an alternate keyboard, which will cause us to receive Unicode characters for some keys, which are outside
				// of the ASCII range (e.g. alt-x will get us U+2248). We need to return 'x' for alt-x, so using unicode
				// in that case would break alt-shortcuts.
				if (unicode > 0x7E)
					unicode = 0; // do not allow any characters above 0x7E
			} else {
				// We must not restrict as much as when Ctrl/Alt-modifiers are active, otherwise
				// we wouldn't let umlauts through for SDL1. For SDL1 umlauts may set for example KEYCODE_QUOTE, KEYCODE_MINUS, etc.
				if (unicode > 0xFF)
					unicode = 0; // do not allow any characters above 0xFF
			}
		}
	}

	// Attention:
	// When using SDL1.x, we will get scancodes via sdlKey, that are raw scancodes, so NOT adjusted to keyboard layout/
	// mapping. So for example for certain locales, we will get KEYCODE_y, when 'z' is pressed and so on.
	// When using SDL2.x however, we will get scancodes based on the keyboard layout.

	if (key >= Common::KEYCODE_F1 && key <= Common::KEYCODE_F9) {
		return key - Common::KEYCODE_F1 + Common::ASCII_F1;
	} else if (key >= Common::KEYCODE_KP0 && key <= Common::KEYCODE_KP9) {
		// WORKAROUND:  Disable this change for AmigaOS4 as it is breaking numpad usage ("fighting") on that platform.
		// This fixes bug #10558.
		// The actual issue here is that the SCUMM engine uses ASCII codes instead of keycodes for input.
		// See also the relevant FIXME in SCUMM's input.cpp.
		#ifndef __amigaos4__
			if ((mod & KMOD_NUM) == 0)
				return 0; // In case Num-Lock is NOT enabled, return 0 for ascii, so that directional keys on numpad work
		#endif
		return key - Common::KEYCODE_KP0 + '0';
	} else if (key >= Common::KEYCODE_UP && key <= Common::KEYCODE_PAGEDOWN) {
		return key;
	} else if (unicode) {
		// Return unicode in case it's still set and wasn't filtered.
		return unicode;
	} else if (key >= 'a' && key <= 'z' && (mod & KMOD_SHIFT)) {
		return key & ~0x20;
	} else if (key >= Common::KEYCODE_NUMLOCK && key <= Common::KEYCODE_EURO) {
		return 0;
	} else {
		return key;
	}
}

bool SdlEventSource::processMouseEvent(Common::Event &event, int x, int y) {
	event.mouse.x = x;
	event.mouse.y = y;

	if (_graphicsManager) {
		return _graphicsManager->notifyMousePosition(event.mouse);
	}

	return true;
}

void SdlEventSource::updateKbdMouse() {
	uint32 curTime = g_system->getMillis(true);
	if (curTime < _km.last_time + _km.delay_time) {
		return;
	}

	_km.last_time = curTime;
	if (_km.x_down_count == 1) {
		_km.x_down_time = curTime;
		_km.x_down_count = 2;
	}
	if (_km.y_down_count == 1) {
		_km.y_down_time = curTime;
		_km.y_down_count = 2;
	}

	if (_km.x_vel || _km.y_vel) {
		if (_km.x_down_count) {
			if (curTime > _km.x_down_time + 300) {
				if (_km.x_vel > 0)
					_km.x_vel += MULTIPLIER;
				else
					_km.x_vel -= MULTIPLIER;
			} else if (curTime > _km.x_down_time + 200) {
				if (_km.x_vel > 0)
					_km.x_vel = 5 * MULTIPLIER;
				else
					_km.x_vel = -5 * MULTIPLIER;
			}
		}
		if (_km.y_down_count) {
			if (curTime > _km.y_down_time + 300) {
				if (_km.y_vel > 0)
					_km.y_vel += MULTIPLIER;
				else
					_km.y_vel -= MULTIPLIER;
			} else if (curTime > _km.y_down_time + 200) {
				if (_km.y_vel > 0)
					_km.y_vel = 5 * MULTIPLIER;
				else
					_km.y_vel = -5 * MULTIPLIER;
			}
		}

		int16 speedFactor = computeJoystickMouseSpeedFactor();

		// - The modifier key makes the mouse movement slower
		// - The extra factor "delay/speedFactor" ensures velocities
		// are independent of the kbdMouse update rate
		// - all velocities were originally chosen
		// at a delay of 25, so that is the reference used here
		// - note: operator order is important to avoid overflow
		if (_km.modifier) {
			_km.x += ((_km.x_vel / 10) * ((int16)_km.delay_time)) / speedFactor;
			_km.y += ((_km.y_vel / 10) * ((int16)_km.delay_time)) / speedFactor;
		} else {
			_km.x += (_km.x_vel * ((int16)_km.delay_time)) / speedFactor;
			_km.y += (_km.y_vel * ((int16)_km.delay_time)) / speedFactor;
		}

		if (_km.x < 0) {
			_km.x = 0;
			_km.x_vel = -1 * MULTIPLIER;
			_km.x_down_count = 1;
		} else if (_km.x > _km.x_max * MULTIPLIER) {
			_km.x = _km.x_max * MULTIPLIER;
			_km.x_vel = 1 * MULTIPLIER;
			_km.x_down_count = 1;
		}

		if (_km.y < 0) {
			_km.y = 0;
			_km.y_vel = -1 * MULTIPLIER;
			_km.y_down_count = 1;
		} else if (_km.y > _km.y_max * MULTIPLIER) {
			_km.y = _km.y_max * MULTIPLIER;
			_km.y_vel = 1 * MULTIPLIER;
			_km.y_down_count = 1;
		}
	}
}

bool SdlEventSource::handleKbdMouse(Common::Event &event) {
	int32 oldKmX = _km.x;
	int32 oldKmY = _km.y;

	updateKbdMouse();

	if (_km.x != oldKmX || _km.y != oldKmY) {
		if (_graphicsManager) {
			_graphicsManager->getWindow()->warpMouseInWindow((Uint16)(_km.x / MULTIPLIER), (Uint16)(_km.y / MULTIPLIER));
		}

		event.type = Common::EVENT_MOUSEMOVE;
		return processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER);
	}

	return false;
}

int16 SdlEventSource::computeJoystickMouseSpeedFactor() const {
	int16 speedFactor;

	switch (ConfMan.getInt("kbdmouse_speed")) {
	// 0.25 keyboard pointer speed
	case 0:
		speedFactor = 100;
		break;
	// 0.5 speed
	case 1:
		speedFactor = 50;
		break;
	// 0.75 speed
	case 2:
		speedFactor = 33;
		break;
	// 1.0 speed
	case 3:
		speedFactor = 25;
		break;
	// 1.25 speed
	case 4:
		speedFactor = 20;
		break;
	// 1.5 speed
	case 5:
		speedFactor = 17;
		break;
	// 1.75 speed
	case 6:
		speedFactor = 14;
		break;
	// 2.0 speed
	case 7:
		speedFactor = 12;
		break;
	default:
		speedFactor = 25;
	}

	// Scale the mouse cursor speed with the display size so moving across
	// the screen takes a reasonable amount of time at higher resolutions.
	return speedFactor * 480 / _km.y_max;
}

void SdlEventSource::SDLModToOSystemKeyFlags(SDLMod mod, Common::Event &event) {

	event.kbd.flags = 0;

	if (mod & KMOD_SHIFT)
		event.kbd.flags |= Common::KBD_SHIFT;
	if (mod & KMOD_ALT)
		event.kbd.flags |= Common::KBD_ALT;
	if (mod & KMOD_CTRL)
		event.kbd.flags |= Common::KBD_CTRL;
	if (mod & KMOD_META)
		event.kbd.flags |= Common::KBD_META;

	// Sticky flags
	if (mod & KMOD_NUM)
		event.kbd.flags |= Common::KBD_NUM;
	if (mod & KMOD_CAPS)
		event.kbd.flags |= Common::KBD_CAPS;
}

Common::KeyCode SdlEventSource::SDLToOSystemKeycode(const SDLKey key) {
	switch (key) {
	case SDLK_BACKSPACE: return Common::KEYCODE_BACKSPACE;
	case SDLK_TAB: return Common::KEYCODE_TAB;
	case SDLK_CLEAR: return Common::KEYCODE_CLEAR;
	case SDLK_RETURN: return Common::KEYCODE_RETURN;
	case SDLK_PAUSE: return Common::KEYCODE_PAUSE;
	case SDLK_ESCAPE: return Common::KEYCODE_ESCAPE;
	case SDLK_SPACE: return Common::KEYCODE_SPACE;
	case SDLK_EXCLAIM: return Common::KEYCODE_EXCLAIM;
	case SDLK_QUOTEDBL: return Common::KEYCODE_QUOTEDBL;
	case SDLK_HASH: return Common::KEYCODE_HASH;
	case SDLK_DOLLAR: return Common::KEYCODE_DOLLAR;
	case SDLK_AMPERSAND: return Common::KEYCODE_AMPERSAND;
	case SDLK_QUOTE: return Common::KEYCODE_QUOTE;
	case SDLK_LEFTPAREN: return Common::KEYCODE_LEFTPAREN;
	case SDLK_RIGHTPAREN: return Common::KEYCODE_RIGHTPAREN;
	case SDLK_ASTERISK: return Common::KEYCODE_ASTERISK;
	case SDLK_PLUS: return Common::KEYCODE_PLUS;
	case SDLK_COMMA: return Common::KEYCODE_COMMA;
	case SDLK_MINUS: return Common::KEYCODE_MINUS;
	case SDLK_PERIOD: return Common::KEYCODE_PERIOD;
	case SDLK_SLASH: return Common::KEYCODE_SLASH;
	case SDLK_0: return Common::KEYCODE_0;
	case SDLK_1: return Common::KEYCODE_1;
	case SDLK_2: return Common::KEYCODE_2;
	case SDLK_3: return Common::KEYCODE_3;
	case SDLK_4: return Common::KEYCODE_4;
	case SDLK_5: return Common::KEYCODE_5;
	case SDLK_6: return Common::KEYCODE_6;
	case SDLK_7: return Common::KEYCODE_7;
	case SDLK_8: return Common::KEYCODE_8;
	case SDLK_9: return Common::KEYCODE_9;
	case SDLK_COLON: return Common::KEYCODE_COLON;
	case SDLK_SEMICOLON: return Common::KEYCODE_SEMICOLON;
	case SDLK_LESS: return Common::KEYCODE_LESS;
	case SDLK_EQUALS: return Common::KEYCODE_EQUALS;
	case SDLK_GREATER: return Common::KEYCODE_GREATER;
	case SDLK_QUESTION: return Common::KEYCODE_QUESTION;
	case SDLK_AT: return Common::KEYCODE_AT;
	case SDLK_LEFTBRACKET: return Common::KEYCODE_LEFTBRACKET;
	case SDLK_BACKSLASH: return Common::KEYCODE_BACKSLASH;
	case SDLK_RIGHTBRACKET: return Common::KEYCODE_RIGHTBRACKET;
	case SDLK_CARET: return Common::KEYCODE_CARET;
	case SDLK_UNDERSCORE: return Common::KEYCODE_UNDERSCORE;
	case SDLK_BACKQUOTE: return Common::KEYCODE_BACKQUOTE;
	case SDLK_a: return Common::KEYCODE_a;
	case SDLK_b: return Common::KEYCODE_b;
	case SDLK_c: return Common::KEYCODE_c;
	case SDLK_d: return Common::KEYCODE_d;
	case SDLK_e: return Common::KEYCODE_e;
	case SDLK_f: return Common::KEYCODE_f;
	case SDLK_g: return Common::KEYCODE_g;
	case SDLK_h: return Common::KEYCODE_h;
	case SDLK_i: return Common::KEYCODE_i;
	case SDLK_j: return Common::KEYCODE_j;
	case SDLK_k: return Common::KEYCODE_k;
	case SDLK_l: return Common::KEYCODE_l;
	case SDLK_m: return Common::KEYCODE_m;
	case SDLK_n: return Common::KEYCODE_n;
	case SDLK_o: return Common::KEYCODE_o;
	case SDLK_p: return Common::KEYCODE_p;
	case SDLK_q: return Common::KEYCODE_q;
	case SDLK_r: return Common::KEYCODE_r;
	case SDLK_s: return Common::KEYCODE_s;
	case SDLK_t: return Common::KEYCODE_t;
	case SDLK_u: return Common::KEYCODE_u;
	case SDLK_v: return Common::KEYCODE_v;
	case SDLK_w: return Common::KEYCODE_w;
	case SDLK_x: return Common::KEYCODE_x;
	case SDLK_y: return Common::KEYCODE_y;
	case SDLK_z: return Common::KEYCODE_z;
	case SDLK_DELETE: return Common::KEYCODE_DELETE;
#if SDL_VERSION_ATLEAST(2, 0, 0)
	case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_GRAVE): return Common::KEYCODE_TILDE;
#else
	case SDLK_WORLD_16: return Common::KEYCODE_TILDE;
#endif
	case SDLK_KP0: return Common::KEYCODE_KP0;
	case SDLK_KP1: return Common::KEYCODE_KP1;
	case SDLK_KP2: return Common::KEYCODE_KP2;
	case SDLK_KP3: return Common::KEYCODE_KP3;
	case SDLK_KP4: return Common::KEYCODE_KP4;
	case SDLK_KP5: return Common::KEYCODE_KP5;
	case SDLK_KP6: return Common::KEYCODE_KP6;
	case SDLK_KP7: return Common::KEYCODE_KP7;
	case SDLK_KP8: return Common::KEYCODE_KP8;
	case SDLK_KP9: return Common::KEYCODE_KP9;
	case SDLK_KP_PERIOD: return Common::KEYCODE_KP_PERIOD;
	case SDLK_KP_DIVIDE: return Common::KEYCODE_KP_DIVIDE;
	case SDLK_KP_MULTIPLY: return Common::KEYCODE_KP_MULTIPLY;
	case SDLK_KP_MINUS: return Common::KEYCODE_KP_MINUS;
	case SDLK_KP_PLUS: return Common::KEYCODE_KP_PLUS;
	case SDLK_KP_ENTER: return Common::KEYCODE_KP_ENTER;
	case SDLK_KP_EQUALS: return Common::KEYCODE_KP_EQUALS;
	case SDLK_UP: return Common::KEYCODE_UP;
	case SDLK_DOWN: return Common::KEYCODE_DOWN;
	case SDLK_RIGHT: return Common::KEYCODE_RIGHT;
	case SDLK_LEFT: return Common::KEYCODE_LEFT;
	case SDLK_INSERT: return Common::KEYCODE_INSERT;
	case SDLK_HOME: return Common::KEYCODE_HOME;
	case SDLK_END: return Common::KEYCODE_END;
	case SDLK_PAGEUP: return Common::KEYCODE_PAGEUP;
	case SDLK_PAGEDOWN: return Common::KEYCODE_PAGEDOWN;
	case SDLK_F1: return Common::KEYCODE_F1;
	case SDLK_F2: return Common::KEYCODE_F2;
	case SDLK_F3: return Common::KEYCODE_F3;
	case SDLK_F4: return Common::KEYCODE_F4;
	case SDLK_F5: return Common::KEYCODE_F5;
	case SDLK_F6: return Common::KEYCODE_F6;
	case SDLK_F7: return Common::KEYCODE_F7;
	case SDLK_F8: return Common::KEYCODE_F8;
	case SDLK_F9: return Common::KEYCODE_F9;
	case SDLK_F10: return Common::KEYCODE_F10;
	case SDLK_F11: return Common::KEYCODE_F11;
	case SDLK_F12: return Common::KEYCODE_F12;
	case SDLK_F13: return Common::KEYCODE_F13;
	case SDLK_F14: return Common::KEYCODE_F14;
	case SDLK_F15: return Common::KEYCODE_F15;
	case SDLK_NUMLOCK: return Common::KEYCODE_NUMLOCK;
	case SDLK_CAPSLOCK: return Common::KEYCODE_CAPSLOCK;
	case SDLK_SCROLLOCK: return Common::KEYCODE_SCROLLOCK;
	case SDLK_RSHIFT: return Common::KEYCODE_RSHIFT;
	case SDLK_LSHIFT: return Common::KEYCODE_LSHIFT;
	case SDLK_RCTRL: return Common::KEYCODE_RCTRL;
	case SDLK_LCTRL: return Common::KEYCODE_LCTRL;
	case SDLK_RALT: return Common::KEYCODE_RALT;
	case SDLK_LALT: return Common::KEYCODE_LALT;
	case SDLK_LSUPER: return Common::KEYCODE_LSUPER;
	case SDLK_RSUPER: return Common::KEYCODE_RSUPER;
	case SDLK_MODE: return Common::KEYCODE_MODE;
	case SDLK_COMPOSE: return Common::KEYCODE_COMPOSE;
	case SDLK_HELP: return Common::KEYCODE_HELP;
	case SDLK_PRINT: return Common::KEYCODE_PRINT;
	case SDLK_SYSREQ: return Common::KEYCODE_SYSREQ;
#if !SDL_VERSION_ATLEAST(2, 0, 0)
	case SDLK_BREAK: return Common::KEYCODE_BREAK;
#endif
	case SDLK_MENU: return Common::KEYCODE_MENU;
	case SDLK_POWER: return Common::KEYCODE_POWER;
	case SDLK_UNDO: return Common::KEYCODE_UNDO;
	default: return Common::KEYCODE_INVALID;
	}
}

bool SdlEventSource::pollEvent(Common::Event &event) {

#if SDL_VERSION_ATLEAST(2, 0, 0)
	// In case we still need to send a key up event for a key down from a
	// TEXTINPUT event we do this immediately.
	if (_queuedFakeKeyUp) {
		event = _fakeKeyUp;
		_queuedFakeKeyUp = false;
		return true;
	}
#endif

	// If the screen changed, send an Common::EVENT_SCREEN_CHANGED
	int screenID = ((OSystem_SDL *)g_system)->getGraphicsManager()->getScreenChangeID();
	if (screenID != _lastScreenID) {
		_lastScreenID = screenID;
		event.type = Common::EVENT_SCREEN_CHANGED;
		return true;
	}

	if (_queuedFakeMouseMove) {
		event = _fakeMouseMove;
		_queuedFakeMouseMove = false;
		return true;
	}

	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		preprocessEvents(&ev);
		if (dispatchSDLEvent(ev, event))
			return true;
	}

	// Handle mouse control via analog joystick and keyboard
	if (handleKbdMouse(event)) {
		return true;
	}

	return false;
}

bool SdlEventSource::dispatchSDLEvent(SDL_Event &ev, Common::Event &event) {
	switch (ev.type) {
	case SDL_KEYDOWN:
		return handleKeyDown(ev, event);
	case SDL_KEYUP:
		return handleKeyUp(ev, event);
	case SDL_MOUSEMOTION:
		return handleMouseMotion(ev, event);
	case SDL_MOUSEBUTTONDOWN:
		return handleMouseButtonDown(ev, event);
	case SDL_MOUSEBUTTONUP:
		return handleMouseButtonUp(ev, event);
	case SDL_SYSWMEVENT:
		return handleSysWMEvent(ev, event);

#if SDL_VERSION_ATLEAST(2, 0, 0)
	case SDL_MOUSEWHEEL: {
		Sint32 yDir = ev.wheel.y;
		// HACK: It seems we want the mouse coordinates supplied
		// with a mouse wheel event. However, SDL2 does not supply
		// these, thus we use whatever we got last time. It seems
		// these are always stored in _km.x, _km.y.
		if (!processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER)) {
			return false;
		}
		if (yDir < 0) {
			event.type = Common::EVENT_WHEELDOWN;
			return true;
		} else if (yDir > 0) {
			event.type = Common::EVENT_WHEELUP;
			return true;
		} else {
			return false;
		}
		}

	case SDL_TEXTINPUT: {
		// When we get a TEXTINPUT event it means we got some user input for
		// which no KEYDOWN exists. SDL 1.2 introduces a "fake" key down+up
		// in such cases. We will do the same to mimic it's behavior.
		event.type = Common::EVENT_KEYDOWN;

		event.kbd = Common::KeyState(Common::KEYCODE_INVALID, convUTF8ToUTF32(ev.text.text), 0);

		SDLModToOSystemKeyFlags(SDL_GetModState(), event);
		// Set the scroll lock sticky flag
		if (_scrollLock)
			event.kbd.flags |= Common::KBD_SCRL;

		// Fake a key up when we have a proper ascii value.
		_queuedFakeKeyUp = (event.kbd.ascii != 0);
		_fakeKeyUp = event;
		_fakeKeyUp.type = Common::EVENT_KEYUP;

		return _queuedFakeKeyUp;
		}

	case SDL_WINDOWEVENT:
		switch (ev.window.event) {
		case SDL_WINDOWEVENT_EXPOSED:
			if (_graphicsManager)
				_graphicsManager->notifyVideoExpose();
			return false;

		// SDL2 documentation indicate that SDL_WINDOWEVENT_SIZE_CHANGED is sent either as a result
		// of the size being changed by an external event (for example the user resizing the window
		// or going fullscreen) or a call to the SDL API (for example SDL_SetWindowSize). On the
		// other hand SDL_WINDOWEVENT_RESIZED is only sent for resize resulting from an external event,
		// and is always preceded by a SDL_WINDOWEVENT_SIZE_CHANGED event.
		// We need to handle the programmatic resize as well so that the graphics manager always know
		// the current size. See comments in SdlWindow::createOrUpdateWindow for details of one case
		// where we need to call SDL_SetWindowSize and we need the resulting event to be processed.
		// However if the documentation is correct we can ignore SDL_WINDOWEVENT_RESIZED since when we
		// get one we should always get a SDL_WINDOWEVENT_SIZE_CHANGED as well.
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		//case SDL_WINDOWEVENT_RESIZED:
			return handleResizeEvent(event, ev.window.data1, ev.window.data2);

		default:
			return false;
		}

	case SDL_JOYDEVICEADDED:
		return handleJoystickAdded(ev.jdevice);

	case SDL_JOYDEVICEREMOVED:
		return handleJoystickRemoved(ev.jdevice);

	case SDL_DROPFILE:
		event.type = Common::EVENT_DROP_FILE;
		event.path = Common::String(ev.drop.file);
		SDL_free(ev.drop.file);
		return true;

	case SDL_CLIPBOARDUPDATE:
		event.type = Common::EVENT_CLIPBOARD_UPDATE;
		return true;
#else
	case SDL_VIDEOEXPOSE:
		if (_graphicsManager)
			_graphicsManager->notifyVideoExpose();
		return false;

	case SDL_VIDEORESIZE:
		return handleResizeEvent(event, ev.resize.w, ev.resize.h);
#endif

	case SDL_QUIT:
		event.type = Common::EVENT_QUIT;
		return true;

	}

	if (_joystick) {
		switch (ev.type) {
		case SDL_JOYBUTTONDOWN:
			return handleJoyButtonDown(ev, event);
		case SDL_JOYBUTTONUP:
			return handleJoyButtonUp(ev, event);
		case SDL_JOYAXISMOTION:
			return handleJoyAxisMotion(ev, event);
		case SDL_JOYHATMOTION:
			return handleJoyHatMotion(ev, event);
		}
	}

#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (_controller) {
		switch (ev.type) {
		case SDL_CONTROLLERBUTTONDOWN:
			return handleControllerButton(ev, event, false);
		case SDL_CONTROLLERBUTTONUP:
			return handleControllerButton(ev, event, true);
		case SDL_CONTROLLERAXISMOTION:
			return handleControllerAxisMotion(ev, event);
		}
	}
#endif

	return false;
}


bool SdlEventSource::handleKeyDown(SDL_Event &ev, Common::Event &event) {

	SDLModToOSystemKeyFlags(SDL_GetModState(), event);

	SDLKey sdlKeycode = obtainKeycode(ev.key.keysym);

	// Handle scroll lock as a key modifier
	if (sdlKeycode == SDLK_SCROLLOCK)
		_scrollLock = !_scrollLock;

	if (_scrollLock)
		event.kbd.flags |= Common::KBD_SCRL;

	// Ctrl-m toggles mouse capture
	if (event.kbd.hasFlags(Common::KBD_CTRL) && sdlKeycode == 'm') {
		if (_graphicsManager) {
			_graphicsManager->getWindow()->toggleMouseGrab();
		}
		return false;
	}

#if defined(MACOSX)
	// On Macintosh, Cmd-Q quits
	if ((ev.key.keysym.mod & KMOD_META) && sdlKeycode == 'q') {
		event.type = Common::EVENT_QUIT;
		return true;
	}
#elif defined(POSIX)
	// On other *nix systems, Control-Q quits
	if ((ev.key.keysym.mod & KMOD_CTRL) && sdlKeycode == 'q') {
		event.type = Common::EVENT_QUIT;
		return true;
	}
#else
	// Ctrl-z quits
	if ((event.kbd.hasFlags(Common::KBD_CTRL) && sdlKeycode == 'z')) {
		event.type = Common::EVENT_QUIT;
		return true;
	}

	#ifdef WIN32
	// On Windows, also use the default Alt-F4 quit combination
	if ((ev.key.keysym.mod & KMOD_ALT) && sdlKeycode == SDLK_F4) {
		event.type = Common::EVENT_QUIT;
		return true;
	}
	#endif
#endif

	// Ctrl-u toggles mute
	if ((ev.key.keysym.mod & KMOD_CTRL) && sdlKeycode == 'u') {
		event.type = Common::EVENT_MUTE;
		return true;
	}

	if (remapKey(ev, event))
		return true;

	event.type = Common::EVENT_KEYDOWN;
	event.kbd.keycode = SDLToOSystemKeycode(sdlKeycode);
	event.kbd.ascii = mapKey(sdlKeycode, (SDLMod)ev.key.keysym.mod, obtainUnicode(ev.key.keysym));

#if SDL_VERSION_ATLEAST(2, 0, 0)
	event.kbdRepeat = ev.key.repeat;
#endif

	return true;
}

bool SdlEventSource::handleKeyUp(SDL_Event &ev, Common::Event &event) {
	if (remapKey(ev, event))
		return true;

	SDLKey sdlKeycode = obtainKeycode(ev.key.keysym);
	SDLMod mod = SDL_GetModState();

	// Check if this is an event handled by handleKeyDown(), and stop if it is

	// Check if the Ctrl key is down, so that we can trap cases where the
	// user has the Ctrl key down, and has just released a special key
	if (mod & KMOD_CTRL) {
		if (sdlKeycode == 'm' ||	// Ctrl-m toggles mouse capture
#if defined(MACOSX)
			// Meta - Q, handled below
#elif defined(POSIX)
			sdlKeycode == 'q' ||	// On other *nix systems, Control-Q quits
#else
			sdlKeycode == 'z' ||	// Ctrl-z quit
#endif
			sdlKeycode == 'u')	// Ctrl-u toggles mute
			return false;
	}

	// Same for other keys (Meta and Alt)
#if defined(MACOSX)
	if ((mod & KMOD_META) && sdlKeycode == 'q')
		return false;	// On Macintosh, Cmd-Q quits
#endif

	// If we reached here, this isn't an event handled by handleKeyDown(), thus
	// continue normally

	event.type = Common::EVENT_KEYUP;
	event.kbd.keycode = SDLToOSystemKeycode(sdlKeycode);
	event.kbd.ascii = mapKey(sdlKeycode, (SDLMod)ev.key.keysym.mod, 0);

	// Ctrl-Alt-<key> will change the GFX mode
	SDLModToOSystemKeyFlags(mod, event);

	// Set the scroll lock sticky flag
	if (_scrollLock)
		event.kbd.flags |= Common::KBD_SCRL;

	return true;
}

bool SdlEventSource::handleMouseMotion(SDL_Event &ev, Common::Event &event) {
	event.type = Common::EVENT_MOUSEMOVE;

	// update KbdMouse
	_km.x = ev.motion.x * MULTIPLIER;
	_km.y = ev.motion.y * MULTIPLIER;

	return processMouseEvent(event, ev.motion.x, ev.motion.y);
}

bool SdlEventSource::handleMouseButtonDown(SDL_Event &ev, Common::Event &event) {
	if (ev.button.button == SDL_BUTTON_LEFT)
		event.type = Common::EVENT_LBUTTONDOWN;
	else if (ev.button.button == SDL_BUTTON_RIGHT)
		event.type = Common::EVENT_RBUTTONDOWN;
#if defined(SDL_BUTTON_WHEELUP) && defined(SDL_BUTTON_WHEELDOWN)
	else if (ev.button.button == SDL_BUTTON_WHEELUP)
		event.type = Common::EVENT_WHEELUP;
	else if (ev.button.button == SDL_BUTTON_WHEELDOWN)
		event.type = Common::EVENT_WHEELDOWN;
#endif
#if defined(SDL_BUTTON_MIDDLE)
	else if (ev.button.button == SDL_BUTTON_MIDDLE)
		event.type = Common::EVENT_MBUTTONDOWN;
#endif
	else
		return false;

	// update KbdMouse
	_km.x = ev.button.x * MULTIPLIER;
	_km.y = ev.button.y * MULTIPLIER;

	return processMouseEvent(event, ev.button.x, ev.button.y);
}

bool SdlEventSource::handleMouseButtonUp(SDL_Event &ev, Common::Event &event) {
	if (ev.button.button == SDL_BUTTON_LEFT)
		event.type = Common::EVENT_LBUTTONUP;
	else if (ev.button.button == SDL_BUTTON_RIGHT)
		event.type = Common::EVENT_RBUTTONUP;
#if defined(SDL_BUTTON_MIDDLE)
	else if (ev.button.button == SDL_BUTTON_MIDDLE)
		event.type = Common::EVENT_MBUTTONUP;
#endif
	else
		return false;

	// update KbdMouse
	_km.x = ev.button.x * MULTIPLIER;
	_km.y = ev.button.y * MULTIPLIER;

	return processMouseEvent(event, ev.button.x, ev.button.y);
}

bool SdlEventSource::handleSysWMEvent(SDL_Event &ev, Common::Event &event) {
	return false;
}

void SdlEventSource::openJoystick(int joystickIndex) {
	if (SDL_NumJoysticks() > joystickIndex) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
		if (SDL_IsGameController(joystickIndex)) {
			_controller = SDL_GameControllerOpen(joystickIndex);
			debug("Using game controller: %s", SDL_GameControllerName(_controller));
		} else
#endif
		{
			_joystick = SDL_JoystickOpen(joystickIndex);
			debug("Using joystick: %s",
#if SDL_VERSION_ATLEAST(2, 0, 0)
                  SDL_JoystickName(_joystick)
#else
                  SDL_JoystickName(joystickIndex)
#endif
			);
		}
	} else {
		warning("Invalid joystick: %d", joystickIndex);
	}
}

void SdlEventSource::closeJoystick() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (_controller) {
		SDL_GameControllerClose(_controller);
		_controller = nullptr;
	}
#endif
	if (_joystick) {
		SDL_JoystickClose(_joystick);
		_joystick = nullptr;
	}
}

bool SdlEventSource::shouldGenerateMouseEvents() {
	// Engine doesn't support joystick -> emulate mouse events
	if (g_engine && !g_engine->hasFeature(Engine::kSupportsJoystick)) {
		return true;
	}
	if (g_gui.isActive()) {
		return true;
	}
	return false;
}

int SdlEventSource::mapSDLJoystickButtonToOSystem(Uint8 sdlButton) {
	Common::JoystickButton osystemButtons[] = {
	    Common::JOYSTICK_BUTTON_A,
	    Common::JOYSTICK_BUTTON_B,
	    Common::JOYSTICK_BUTTON_X,
	    Common::JOYSTICK_BUTTON_Y,
	    Common::JOYSTICK_BUTTON_LEFT_SHOULDER,
	    Common::JOYSTICK_BUTTON_RIGHT_SHOULDER,
	    Common::JOYSTICK_BUTTON_BACK,
	    Common::JOYSTICK_BUTTON_START,
	    Common::JOYSTICK_BUTTON_LEFT_STICK,
	    Common::JOYSTICK_BUTTON_RIGHT_STICK
	};

	if (sdlButton >= ARRAYSIZE(osystemButtons)) {
		return -1;
	}

	return osystemButtons[sdlButton];
}

bool SdlEventSource::handleJoyButtonDown(SDL_Event &ev, Common::Event &event) {
	if (!shouldGenerateMouseEvents()) {
		event.type = Common::EVENT_JOYBUTTON_DOWN;
		event.joystick.button = mapSDLJoystickButtonToOSystem(ev.jbutton.button);
		return true;
	}

	if (ev.jbutton.button == JOY_BUT_LMOUSE) {
		event.type = Common::EVENT_LBUTTONDOWN;
		return processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER);
	} else if (ev.jbutton.button == JOY_BUT_RMOUSE) {
		event.type = Common::EVENT_RBUTTONDOWN;
		return processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER);
	} else {
		event.type = Common::EVENT_KEYDOWN;
		switch (ev.jbutton.button) {
		case JOY_BUT_ESCAPE:
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = mapKey(SDLK_ESCAPE, (SDLMod)ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_PERIOD:
			event.kbd.keycode = Common::KEYCODE_PERIOD;
			event.kbd.ascii = mapKey(SDLK_PERIOD, (SDLMod)ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_SPACE:
			event.kbd.keycode = Common::KEYCODE_SPACE;
			event.kbd.ascii = mapKey(SDLK_SPACE, (SDLMod)ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_F5:
			event.kbd.keycode = Common::KEYCODE_F5;
			event.kbd.ascii = mapKey(SDLK_F5, (SDLMod)ev.key.keysym.mod, 0);
			break;
#ifdef ENABLE_VKEYBD
		case JOY_BUT_VKEYBOARD: // Toggles virtual keyboard
			event.type = Common::EVENT_VIRTUAL_KEYBOARD;
			break;
#endif
		}
		return true;
	}
}

bool SdlEventSource::handleJoyButtonUp(SDL_Event &ev, Common::Event &event) {
	if (!shouldGenerateMouseEvents()) {
		event.type = Common::EVENT_JOYBUTTON_UP;
		event.joystick.button = mapSDLJoystickButtonToOSystem(ev.jbutton.button);
		return true;
	}

	if (ev.jbutton.button == JOY_BUT_LMOUSE) {
		event.type = Common::EVENT_LBUTTONUP;
		return processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER);
	} else if (ev.jbutton.button == JOY_BUT_RMOUSE) {
		event.type = Common::EVENT_RBUTTONUP;
		return processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER);
	} else {
		event.type = Common::EVENT_KEYUP;
		switch (ev.jbutton.button) {
		case JOY_BUT_ESCAPE:
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = mapKey(SDLK_ESCAPE, (SDLMod)ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_PERIOD:
			event.kbd.keycode = Common::KEYCODE_PERIOD;
			event.kbd.ascii = mapKey(SDLK_PERIOD, (SDLMod)ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_SPACE:
			event.kbd.keycode = Common::KEYCODE_SPACE;
			event.kbd.ascii = mapKey(SDLK_SPACE, (SDLMod)ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_F5:
			event.kbd.keycode = Common::KEYCODE_F5;
			event.kbd.ascii = mapKey(SDLK_F5, (SDLMod)ev.key.keysym.mod, 0);
			break;
#ifdef ENABLE_VKEYBD
		case JOY_BUT_VKEYBOARD: // Toggles virtual keyboard
			// Handled in key down
			break;
#endif
		}
		return true;
	}
}

bool SdlEventSource::handleJoyAxisMotion(SDL_Event &ev, Common::Event &event) {
	// TODO: move handleAxisToMouseMotion to Common?
#if 0
	if (!shouldGenerateMouseEvents()) {
		event.type = Common::EVENT_JOYAXIS_MOTION;
		event.joystick.axis = ev.jaxis.axis;
		event.joystick.position = ev.jaxis.value;
		return true;
	}
#endif

	if (ev.jaxis.axis == JOY_XAXIS) {
		_km.joy_x = ev.jaxis.value;
		return handleAxisToMouseMotion(_km.joy_x, _km.joy_y);
	} else if (ev.jaxis.axis == JOY_YAXIS) {
		_km.joy_y = ev.jaxis.value;
		return handleAxisToMouseMotion(_km.joy_x, _km.joy_y);
	}

	return false;
}

#define HANDLE_HAT_UP(new, old, mask, joybutton) \
	if ((old & mask) && !(new & mask)) { \
		event.joystick.button = joybutton; \
		g_system->getEventManager()->pushEvent(event); \
	}

#define HANDLE_HAT_DOWN(new, old, mask, joybutton) \
	if ((new & mask) && !(old & mask)) { \
		event.joystick.button = joybutton; \
		g_system->getEventManager()->pushEvent(event); \
	}

bool SdlEventSource::handleJoyHatMotion(SDL_Event &ev, Common::Event &event) {
	if (shouldGenerateMouseEvents())
		return false;

	event.type = Common::EVENT_JOYBUTTON_UP;
	HANDLE_HAT_UP(ev.jhat.value, _lastHatPosition, SDL_HAT_UP, Common::JOYSTICK_BUTTON_DPAD_UP)
	HANDLE_HAT_UP(ev.jhat.value, _lastHatPosition, SDL_HAT_DOWN, Common::JOYSTICK_BUTTON_DPAD_DOWN)
	HANDLE_HAT_UP(ev.jhat.value, _lastHatPosition, SDL_HAT_LEFT, Common::JOYSTICK_BUTTON_DPAD_LEFT)
	HANDLE_HAT_UP(ev.jhat.value, _lastHatPosition, SDL_HAT_RIGHT, Common::JOYSTICK_BUTTON_DPAD_RIGHT)

	event.type = Common::EVENT_JOYBUTTON_DOWN;
	HANDLE_HAT_DOWN(ev.jhat.value, _lastHatPosition, SDL_HAT_UP, Common::JOYSTICK_BUTTON_DPAD_UP)
	HANDLE_HAT_DOWN(ev.jhat.value, _lastHatPosition, SDL_HAT_DOWN, Common::JOYSTICK_BUTTON_DPAD_DOWN)
	HANDLE_HAT_DOWN(ev.jhat.value, _lastHatPosition, SDL_HAT_LEFT, Common::JOYSTICK_BUTTON_DPAD_LEFT)
	HANDLE_HAT_DOWN(ev.jhat.value, _lastHatPosition, SDL_HAT_RIGHT, Common::JOYSTICK_BUTTON_DPAD_RIGHT)

	_lastHatPosition = ev.jhat.value;

	return false;
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
bool SdlEventSource::handleJoystickAdded(const SDL_JoyDeviceEvent &device) {
	debug(5, "SdlEventSource: Received joystick added event for index '%d'", device.which);

	int joystick_num = ConfMan.getInt("joystick_num");
	if (joystick_num == device.which) {
		debug(5, "SdlEventSource: Newly added joystick with index '%d' matches 'joysticky_num', trying to use it", device.which);

		closeJoystick();
		openJoystick(joystick_num);
	}

	return false;
}

bool SdlEventSource::handleJoystickRemoved(const SDL_JoyDeviceEvent &device) {
	debug(5, "SdlEventSource: Received joystick removed event for instance id '%d'", device.which);

	SDL_Joystick *joystick;
	if (_controller) {
		joystick = SDL_GameControllerGetJoystick(_controller);
	} else {
		joystick = _joystick;
	}

	if (!joystick) {
		return false;
	}

	if (SDL_JoystickInstanceID(joystick) == device.which) {
		debug(5, "SdlEventSource: Newly removed joystick with instance id '%d' matches currently used joystick, closing current joystick", device.which);

		closeJoystick();
	}

	return false;
}

int SdlEventSource::mapSDLControllerButtonToOSystem(Uint8 sdlButton) {
	Common::JoystickButton osystemButtons[] = {
	    Common::JOYSTICK_BUTTON_A,
	    Common::JOYSTICK_BUTTON_B,
	    Common::JOYSTICK_BUTTON_X,
	    Common::JOYSTICK_BUTTON_Y,
	    Common::JOYSTICK_BUTTON_BACK,
	    Common::JOYSTICK_BUTTON_GUIDE,
	    Common::JOYSTICK_BUTTON_START,
	    Common::JOYSTICK_BUTTON_LEFT_STICK,
	    Common::JOYSTICK_BUTTON_RIGHT_STICK,
	    Common::JOYSTICK_BUTTON_LEFT_SHOULDER,
	    Common::JOYSTICK_BUTTON_RIGHT_SHOULDER,
	    Common::JOYSTICK_BUTTON_DPAD_UP,
	    Common::JOYSTICK_BUTTON_DPAD_DOWN,
	    Common::JOYSTICK_BUTTON_DPAD_LEFT,
	    Common::JOYSTICK_BUTTON_DPAD_RIGHT
	};

	if (sdlButton >= ARRAYSIZE(osystemButtons)) {
		return -1;
	}

	return osystemButtons[sdlButton];
}

bool SdlEventSource::handleControllerButton(const SDL_Event &ev, Common::Event &event, bool buttonUp) {
	using namespace Common;

	struct ControllerEventMapping {
		EventType normalType;
		KeyState normalKeystate;
		EventType modifierType;
		KeyState modifierKeystate;
	};

	static const ControllerEventMapping mapping[] = {
			// SDL_CONTROLLER_BUTTON_A: Left mouse button
			{ EVENT_LBUTTONDOWN, KeyState(), EVENT_LBUTTONDOWN, KeyState() },
			// SDL_CONTROLLER_BUTTON_B: Right mouse button
			{ EVENT_RBUTTONDOWN, KeyState(), EVENT_RBUTTONDOWN, KeyState() },
			// SDL_CONTROLLER_BUTTON_X: Period (+R_trigger: Space)
			{ EVENT_KEYDOWN, KeyState(KEYCODE_PERIOD, '.'), EVENT_KEYDOWN, KeyState(KEYCODE_SPACE, ASCII_SPACE) },
			// SDL_CONTROLLER_BUTTON_Y: Escape (+R_trigger: Return)
			{ EVENT_KEYDOWN, KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE), EVENT_KEYDOWN, KeyState(KEYCODE_RETURN, ASCII_RETURN) },
			// SDL_CONTROLLER_BUTTON_BACK: Virtual keyboard (+R_trigger: Predictive Input Dialog)
#ifdef ENABLE_VKEYBD
			{ EVENT_VIRTUAL_KEYBOARD, KeyState(), EVENT_PREDICTIVE_DIALOG, KeyState() },
#else
			{ EVENT_INVALID, KeyState(), EVENT_PREDICTIVE_DIALOG, KeyState() },
#endif
			// SDL_CONTROLLER_BUTTON_GUIDE: Unmapped
			{ EVENT_INVALID, KeyState(), EVENT_INVALID, KeyState() },
			// SDL_CONTROLLER_BUTTON_START: ScummVM in game menu
			{ EVENT_MAINMENU, KeyState(), EVENT_MAINMENU, KeyState() },
			// SDL_CONTROLLER_BUTTON_LEFTSTICK: Unmapped
			{ EVENT_INVALID, KeyState(), EVENT_INVALID, KeyState() },
			// SDL_CONTROLLER_BUTTON_RIGHTSTICK: Unmapped
			{ EVENT_INVALID, KeyState(), EVENT_INVALID, KeyState() },
			// SDL_CONTROLLER_BUTTON_LEFTSHOULDER: Game menu
			{ EVENT_KEYDOWN, KeyState(KEYCODE_F5, ASCII_F5), EVENT_KEYDOWN, KeyState(KEYCODE_F5, ASCII_F5) },
			// SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: Modifier + Shift
			{ EVENT_KEYDOWN, KeyState(KEYCODE_INVALID, 0, KBD_SHIFT), EVENT_KEYDOWN, KeyState(KEYCODE_INVALID, 0, 0) },
			// SDL_CONTROLLER_BUTTON_DPAD_UP: Up (+R_trigger: Up+Right)
			{ EVENT_KEYDOWN, KeyState(KEYCODE_KP8, 0), EVENT_KEYDOWN, KeyState(KEYCODE_KP9, 0) },
			// SDL_CONTROLLER_BUTTON_DPAD_DOWN: Down (+R_trigger: Down+Left)
			{ EVENT_KEYDOWN, KeyState(KEYCODE_KP2, 0), EVENT_KEYDOWN, KeyState(KEYCODE_KP1, 0) },
			// SDL_CONTROLLER_BUTTON_DPAD_LEFT: Left (+R_trigger: Up+Left)
			{ EVENT_KEYDOWN, KeyState(KEYCODE_KP4, 0), EVENT_KEYDOWN, KeyState(KEYCODE_KP7, 0) },
			// SDL_CONTROLLER_BUTTON_DPAD_RIGHT: Right (+R_trigger: Down+Right)
			{ EVENT_KEYDOWN, KeyState(KEYCODE_KP6, 0), EVENT_KEYDOWN, KeyState(KEYCODE_KP3, 0) }
	};

	if (!shouldGenerateMouseEvents()) {
		event.type = buttonUp ? Common::EVENT_JOYBUTTON_UP : Common::EVENT_JOYBUTTON_DOWN;
		event.joystick.button = mapSDLControllerButtonToOSystem(ev.cbutton.button);
		if (event.joystick.button == -1)
				return false;

		return true;
	}

	if (ev.cbutton.button > SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
		warning("Unknown SDL controller button: '%d'", ev.cbutton.button);
		return false;
	}

	if (!_km.modifier) {
		event.type = mapping[ev.cbutton.button].normalType;
		event.kbd = mapping[ev.cbutton.button].normalKeystate;
	} else {
		event.type = mapping[ev.cbutton.button].modifierType;
		event.kbd = mapping[ev.cbutton.button].modifierKeystate;
	}

	// Setting the mouse speed modifier after filling the event structure above
	// ensures that the shift key events are correctly handled
	if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
		// Right shoulder is the modifier button that makes the mouse go slower
		// and allows access to an extended layout while pressed.
		_km.modifier = !buttonUp;
	}

	if (event.type == EVENT_LBUTTONDOWN || event.type == EVENT_RBUTTONDOWN) {
		processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER);
	}

	if (buttonUp) {
		// The event mapping table is for button down events. If we received a button up event,
		// transform the event type to the corresponding up type.
		if (event.type == EVENT_KEYDOWN) {
			event.type = EVENT_KEYUP;
		} else if (event.type == EVENT_LBUTTONDOWN) {
			event.type = EVENT_LBUTTONUP;
		} else if (event.type == EVENT_RBUTTONDOWN) {
			event.type = EVENT_RBUTTONUP;
		} else {
			// Handled in key down
			event.type = EVENT_INVALID;
		}
	}

	return event.type != EVENT_INVALID;
}

bool SdlEventSource::handleControllerAxisMotion(const SDL_Event &ev, Common::Event &event) {
	// TODO: move handleAxisToMouseMotion to Common?
#if 0
	if (!shouldGenerateMouseEvents()) {
		event.type = Common::EVENT_JOYAXIS_MOTION;
		event.joystick.axis = ev.caxis.axis;
		event.joystick.position = ev.caxis.value;
		return true;
	}
#endif

	if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
		_km.joy_x = ev.caxis.value;
		return handleAxisToMouseMotion(_km.joy_x, _km.joy_y);
	} else if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
		_km.joy_y = ev.caxis.value;
		return handleAxisToMouseMotion(_km.joy_x, _km.joy_y);
	}

	return false;
}
#endif

bool SdlEventSource::handleAxisToMouseMotion(int16 xAxis, int16 yAxis) {
#ifdef JOY_INVERT_Y
	yAxis = -yAxis;
#endif

	// conversion factor between keyboard mouse and joy axis value
	int vel_to_axis = (1500 / MULTIPLIER);

	// radial and scaled deadzone

	float analogX = (float)xAxis;
	float analogY = (float)yAxis;
	float deadZone = (float)ConfMan.getInt("joystick_deadzone") * 1000.0f;

	float magnitude = sqrt(analogX * analogX + analogY * analogY);

	if (magnitude >= deadZone) {
		_km.x_down_count = 0;
		_km.y_down_count = 0;
		float scalingFactor = 1.0f / magnitude * (magnitude - deadZone) / (32769.0f - deadZone);
		_km.x_vel = (int16)(analogX * scalingFactor * 32768.0f / vel_to_axis);
		_km.y_vel = (int16)(analogY * scalingFactor * 32768.0f / vel_to_axis);
	} else {
		_km.x_vel = 0;
		_km.y_vel = 0;
	}

	return false;
}

bool SdlEventSource::remapKey(SDL_Event &ev, Common::Event &event) {
	return false;
}

void SdlEventSource::resetKeyboardEmulation(int16 x_max, int16 y_max) {
	_km.x_max = x_max;
	_km.y_max = y_max;
	_km.delay_time = 12;
	_km.last_time = 0;
	_km.modifier = false;
	_km.joy_x = 0;
	_km.joy_y = 0;
}

void SdlEventSource::fakeWarpMouse(const int x, const int y) {
	_queuedFakeMouseMove = true;
	_fakeMouseMove.type = Common::EVENT_MOUSEMOVE;
	_fakeMouseMove.mouse = Common::Point(x, y);
}

bool SdlEventSource::handleResizeEvent(Common::Event &event, int w, int h) {
	if (_graphicsManager) {
		_graphicsManager->notifyResize(w, h);

		// If the screen changed, send an Common::EVENT_SCREEN_CHANGED
		int screenID = ((OSystem_SDL *)g_system)->getGraphicsManager()->getScreenChangeID();
		if (screenID != _lastScreenID) {
			_lastScreenID = screenID;
			event.type = Common::EVENT_SCREEN_CHANGED;
			return true;
		}
	}

	return false;
}

SDLKey SdlEventSource::obtainKeycode(const SDL_keysym keySym) {
#if !SDL_VERSION_ATLEAST(2, 0, 0) && defined(WIN32) && !defined(_WIN32_WCE)
	// WORKAROUND: SDL 1.2 on Windows does not use the user configured keyboard layout,
	// resulting in "keySym.sym" values to always be those expected for an US keyboard.
	// For example, SDL returns SDLK_Q when pressing the 'A' key on an AZERTY keyboard.
	// This defeats the purpose of keycodes which is to be able to refer to a key without
	// knowing where it is physically located.
	// We work around this issue by querying the currently active Windows keyboard layout
	// using the scancode provided by SDL.

	if (keySym.sym >= SDLK_0 && keySym.sym <= SDLK_9) {
		// The keycode returned by SDL is kept for the number keys.
		// Querying the keyboard layout for those would return the base key values
		// for AZERTY keyboards, which are not numbers. For example, SDLK_1 would
		// map to SDLK_AMPERSAND. This is theoretically correct but practically unhelpful,
		// because it makes it impossible to handle key combinations such as "ctrl-1".
		return keySym.sym;
	}

	int vk = MapVirtualKey(keySym.scancode, MAPVK_VSC_TO_VK);
	if (vk) {
		int ch = (MapVirtualKey(vk, MAPVK_VK_TO_CHAR) & 0x7FFF);
		// The top bit of the result of MapVirtualKey with MAPVK_VSC_TO_VK signals
		// a dead key was pressed. In that case we keep the value of the accent alone.
		if (ch) {
			if (ch >= 'A' && ch <= 'Z') {
				// Windows returns uppercase ASCII whereas SDL expects lowercase
				return (SDLKey)(SDLK_a + (ch - 'A'));
			} else {
				return (SDLKey)ch;
			}
		}
	}
#endif

	return keySym.sym;
}

uint32 SdlEventSource::obtainUnicode(const SDL_keysym keySym) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_Event events[2];

	// Update the event queue here to give SDL a chance to insert TEXTINPUT
	// events for KEYDOWN events. Otherwise we have a high chance that on
	// Windows the TEXTINPUT event is not in the event queue at this point.
	// In this case we will get two events with ascii values due to mapKey
	// and dispatchSDLEvent. This results in nasty double input of characters
	// in the GUI.
	//
	// FIXME: This is all a bit fragile because in mapKey we derive the ascii
	// value from the key code if no unicode value is given. This is legacy
	// behavior and should be removed anyway. If that is removed, we might not
	// even need to do this peeking here but instead can rely on the
	// SDL_TEXTINPUT case in dispatchSDLEvent to introduce keydown/keyup with
	// proper ASCII values (but with KEYCODE_INVALID as keycode).
	SDL_PumpEvents();

	// In SDL2, the unicode field has been removed from the keysym struct.
	// Instead a SDL_TEXTINPUT event is generated on key combinations that
	// generates unicode.
	// Here we peek into the event queue for the event to see if it exists.
	int n = SDL_PeepEvents(events, 2, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_TEXTINPUT);
	// Make sure that the TEXTINPUT event belongs to this KEYDOWN
	// event and not another pending one.
	if ((n > 0 && events[0].type == SDL_TEXTINPUT)
	    || (n > 1 && events[0].type != SDL_KEYDOWN && events[1].type == SDL_TEXTINPUT)) {
		// Remove the text input event we associate with the key press. This
		// makes sure we never get any SDL_TEXTINPUT events which do "belong"
		// to SDL_KEYDOWN events.
		n = SDL_PeepEvents(events, 1, SDL_GETEVENT, SDL_TEXTINPUT, SDL_TEXTINPUT);
		// This is basically a paranoia safety check because we know there
		// must be a text input event in the queue.
		if (n > 0) {
			return convUTF8ToUTF32(events[0].text.text);
		} else {
			return 0;
		}
	} else {
		return 0;
	}
#else
	return keySym.unicode;
#endif
}

#endif
