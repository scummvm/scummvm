/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

SdlEventSource::SdlEventSource()
	: EventSource(), _scrollLock(false), _joystick(nullptr), _lastScreenID(0), _graphicsManager(nullptr), _queuedFakeMouseMove(false),
	  _lastHatPosition(SDL_HAT_CENTERED), _mouseX(0), _mouseY(0), _engineRunning(false)
	  {
	int joystick_num = ConfMan.getInt("joystick_num");
	if (joystick_num >= 0) {
		// Initialize SDL joystick subsystem
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1) {
			warning("Could not initialize SDL joystick: %s", SDL_GetError());
			return;
		}

		openJoystick(joystick_num);
	}
}

int SdlEventSource::mapKey(SDL_Keycode sdlKey, SDL_Keymod mod, Uint16 unicode) {
	Common::KeyCode key = SDLToOSystemKeycode(sdlKey);

	// Keep unicode in case it's regular ASCII text, Hebrew or in case we didn't get a valid keycode
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
				// We allow Hebrew characters
				if (unicode >= 0x05D0 && unicode <= 0x05EA)
					return unicode;

				// Cyrillic
				if (unicode >= 0x0400 && unicode <= 0x045F)
					return unicode;

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
		if ((mod & KMOD_NUM) == 0)
			return 0; // In case Num-Lock is NOT enabled, return 0 for ascii, so that directional keys on numpad work
		return key - Common::KEYCODE_KP0 + '0';
	} else if (key >= Common::KEYCODE_UP && key <= Common::KEYCODE_PAGEDOWN) {
		return key;
	} else if (unicode) {
		// Return unicode in case it's still set and wasn't filtered.
		return unicode;
	} else if (key >= 'a' && key <= 'z' && (mod & KMOD_SHIFT)) {
		return key & ~0x20;
	} else if (key >= Common::KEYCODE_NUMLOCK && key < Common::KEYCODE_LAST) {
		return 0;
	} else {
		return key;
	}
}

void SdlEventSource::SDLModToOSystemKeyFlags(SDL_Keymod mod, Common::Event &event) {

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

Common::KeyCode SdlEventSource::SDLToOSystemKeycode(const SDL_Keycode key) {
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
	case SDLK_CAPSLOCK: return Common::KEYCODE_CAPSLOCK;
	case SDLK_RSHIFT: return Common::KEYCODE_RSHIFT;
	case SDLK_LSHIFT: return Common::KEYCODE_LSHIFT;
	case SDLK_RCTRL: return Common::KEYCODE_RCTRL;
	case SDLK_LCTRL: return Common::KEYCODE_LCTRL;
	case SDLK_RALT: return Common::KEYCODE_RALT;
	case SDLK_LALT: return Common::KEYCODE_LALT;
	case SDLK_MODE: return Common::KEYCODE_MODE;
	case SDLK_HELP: return Common::KEYCODE_HELP;
	case SDLK_SYSREQ: return Common::KEYCODE_SYSREQ;
	case SDLK_MENU: return Common::KEYCODE_MENU;
	case SDLK_POWER: return Common::KEYCODE_POWER;
#if SDL_VERSION_ATLEAST(1, 2, 3)
	case SDLK_UNDO: return Common::KEYCODE_UNDO;
#endif
	case SDLK_SCROLLOCK: return Common::KEYCODE_SCROLLOCK;
	case SDLK_NUMLOCK: return Common::KEYCODE_NUMLOCK;
	case SDLK_LSUPER: return Common::KEYCODE_LSUPER;
	case SDLK_RSUPER: return Common::KEYCODE_RSUPER;
	case SDLK_PRINT: return Common::KEYCODE_PRINT;
	case SDLK_COMPOSE: return Common::KEYCODE_COMPOSE;
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
	case SDLK_WORLD_16: return Common::KEYCODE_TILDE;
	case SDLK_BREAK: return Common::KEYCODE_BREAK;
	case SDLK_LMETA: return Common::KEYCODE_LMETA;
	case SDLK_RMETA: return Common::KEYCODE_RMETA;
	case SDLK_EURO: return Common::KEYCODE_EURO;
	default: return Common::KEYCODE_INVALID;
	}
}

bool SdlEventSource::pollEvent(Common::Event &event) {
	// If the screen changed, send an Common::EVENT_SCREEN_CHANGED
	int screenID = g_system->getScreenChangeID();
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
	case SDL_VIDEOEXPOSE:
		if (_graphicsManager) {
			_graphicsManager->notifyVideoExpose();
		}
		return false;

	case SDL_VIDEORESIZE:
		return handleResizeEvent(event, ev.resize.w, ev.resize.h);
	case SDL_QUIT:
		event.type = Common::EVENT_QUIT;
		return true;

	default:
		break;
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
		default:
			break;
		}
	}

	return false;
}


bool SdlEventSource::handleKeyDown(SDL_Event &ev, Common::Event &event) {

	SDLModToOSystemKeyFlags(SDL_GetModState(), event);

	SDL_Keycode sdlKeycode = obtainKeycode(ev.key.keysym);
	Common::KeyCode key = SDLToOSystemKeycode(sdlKeycode);

	// Handle scroll lock as a key modifier
	if (key == Common::KEYCODE_SCROLLOCK)
		_scrollLock = !_scrollLock;

	if (_scrollLock)
		event.kbd.flags |= Common::KBD_SCRL;

	if (remapKey(ev, event))
		return true;

	event.type = Common::EVENT_KEYDOWN;
	event.kbd.keycode = key;

	SDL_Keymod mod = (SDL_Keymod)ev.key.keysym.mod;
#if defined(__amigaos4__)
	// On AmigaOS, SDL always reports numlock as off. However, we get KEYCODE_KP# only when
	// it is on, and get different keycodes (for example KEYCODE_PAGEDOWN) when it is off.
	if (event.kbd.keycode >= Common::KEYCODE_KP0 && event.kbd.keycode <= Common::KEYCODE_KP9) {
		event.kbd.flags |= Common::KBD_NUM;
		mod = SDL_Keymod(mod | KMOD_NUM);
	}
#endif
	event.kbd.ascii = mapKey(sdlKeycode, mod, ev.key.keysym.unicode);

	return true;
}

bool SdlEventSource::handleKeyUp(SDL_Event &ev, Common::Event &event) {
	if (remapKey(ev, event))
		return true;

	SDLModToOSystemKeyFlags(SDL_GetModState(), event);

	SDL_Keycode sdlKeycode = obtainKeycode(ev.key.keysym);

	// Set the scroll lock sticky flag
	if (_scrollLock)
		event.kbd.flags |= Common::KBD_SCRL;

	event.type = Common::EVENT_KEYUP;
	event.kbd.keycode = SDLToOSystemKeycode(sdlKeycode);

	SDL_Keymod mod = (SDL_Keymod)ev.key.keysym.mod;
#if defined(__amigaos4__)
	// On AmigaOS, SDL always reports numlock as off. However, we get KEYCODE_KP# only when
	// it is on, and get different keycodes (for example KEYCODE_PAGEDOWN) when it is off.
	if (event.kbd.keycode >= Common::KEYCODE_KP0 && event.kbd.keycode <= Common::KEYCODE_KP9) {
		event.kbd.flags |= Common::KBD_NUM;
		mod = SDL_Keymod(mod | KMOD_NUM);
	}
#endif
	event.kbd.ascii = mapKey(sdlKeycode, mod, 0);

	return true;
}

void SdlEventSource::openJoystick(int joystickIndex) {
	if (SDL_NumJoysticks() > joystickIndex) {
			_joystick = SDL_JoystickOpen(joystickIndex);
			debug("Using joystick: %s",
				  SDL_JoystickName(joystickIndex)
			);
	} else {
		debug(5, "Invalid joystick: %d", joystickIndex);
	}
}

void SdlEventSource::closeJoystick() {
	if (_joystick) {
		SDL_JoystickClose(_joystick);
		_joystick = nullptr;
	}
}

bool SdlEventSource::isJoystickConnected() const {
	return _joystick;
}

SDL_Keycode SdlEventSource::obtainKeycode(const SDL_Keysym keySym) {
#ifdef WIN32
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
				return (SDL_Keycode)(SDLK_a + (ch - 'A'));
			} else {
				return (SDL_Keycode)ch;
			}
		}
	}
#endif

	return keySym.sym;
}

#endif
