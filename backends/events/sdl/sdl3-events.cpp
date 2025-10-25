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

#if defined(USE_IMGUI)
#include "backends/imgui/backends/imgui_impl_sdl3.h"
#endif

#define GAMECONTROLLERDB_FILE "gamecontrollerdb.txt"

static uint32 convUTF8ToUTF32(const char *src) {
	if (!src || src[0] == 0)
		return 0;

	Common::U32String u32(src);
	return u32[0];
}

void SdlEventSource::loadGameControllerMappingFile() {
	bool loaded = false;
	if (ConfMan.hasKey("controller_map_db")) {
		Common::FSNode file = Common::FSNode(ConfMan.getPath("controller_map_db"));
		if (file.exists()) {
			if (!SDL_AddGamepadMappingsFromFile(file.getPath().toString(Common::Path::kNativeSeparator).c_str()))
				error("File %s not valid: %s", file.getPath().toString(Common::Path::kNativeSeparator).c_str(), SDL_GetError());
			else {
				loaded = true;
				debug("Game controller DB file loaded: %s", file.getPath().toString(Common::Path::kNativeSeparator).c_str());
			}
		} else
			warning("Game controller DB file not found: %s", file.getPath().toString(Common::Path::kNativeSeparator).c_str());
	}
	if (!loaded && ConfMan.hasKey("extrapath")) {
		Common::FSNode dir = Common::FSNode(ConfMan.getPath("extrapath"));
		Common::FSNode file = dir.getChild(GAMECONTROLLERDB_FILE);
		if (file.exists()) {
			if (!SDL_AddGamepadMappingsFromFile(file.getPath().toString(Common::Path::kNativeSeparator).c_str()))
				error("File %s not valid: %s", file.getPath().toString(Common::Path::kNativeSeparator).c_str(), SDL_GetError());
			else
				debug("Game controller DB file loaded: %s", file.getPath().toString(Common::Path::kNativeSeparator).c_str());
		}
	}
}

SdlEventSource::SdlEventSource()
	: EventSource(), _scrollLock(false), _joystick(nullptr), _lastScreenID(0), _graphicsManager(nullptr), _queuedFakeMouseMove(false),
	  _lastHatPosition(SDL_HAT_CENTERED), _mouseX(0), _mouseY(0), _engineRunning(false)
	  , _queuedFakeKeyUp(false), _fakeKeyUp(), _controller(nullptr) {
	int joystick_num = ConfMan.getInt("joystick_num");
	if (joystick_num >= 0) {
		// Initialize SDL joystick subsystem
		if (!SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
			warning("Could not initialize SDL joystick: %s", SDL_GetError());
			return;
		}

		if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
			warning("Could not initialize SDL game controller: %s", SDL_GetError());
			return;
		}
		loadGameControllerMappingFile();

		openJoystick(joystick_num);
	}

	// ensure that touch doesn't create double-events
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
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
			if (mod & (SDL_KMOD_CTRL | SDL_KMOD_ALT)) {
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

	if (key >= Common::KEYCODE_F1 && key <= Common::KEYCODE_F9) {
		return key - Common::KEYCODE_F1 + Common::ASCII_F1;
	} else if (key >= Common::KEYCODE_KP0 && key <= Common::KEYCODE_KP9) {
		if ((mod & SDL_KMOD_NUM) == 0)
			return 0; // In case Num-Lock is NOT enabled, return 0 for ascii, so that directional keys on numpad work
		return key - Common::KEYCODE_KP0 + '0';
	} else if (key >= Common::KEYCODE_UP && key <= Common::KEYCODE_PAGEDOWN) {
		return key;
	} else if (unicode) {
		// Return unicode in case it's still set and wasn't filtered.
		return unicode;
	} else if (key >= 'a' && key <= 'z' && (mod & SDL_KMOD_SHIFT)) {
		return key & ~0x20;
	} else if (key >= Common::KEYCODE_NUMLOCK && key < Common::KEYCODE_LAST) {
		return 0;
	} else {
		return key;
	}
}

void SdlEventSource::SDLModToOSystemKeyFlags(SDL_Keymod mod, Common::Event &event) {

	event.kbd.flags = 0;

	if (mod & SDL_KMOD_GUI)
		event.kbd.flags |= Common::KBD_META;
	if (mod & SDL_KMOD_SHIFT)
		event.kbd.flags |= Common::KBD_SHIFT;
	if (mod & SDL_KMOD_ALT)
		event.kbd.flags |= Common::KBD_ALT;
	if (mod & SDL_KMOD_CTRL)
		event.kbd.flags |= Common::KBD_CTRL;

	// Sticky flags
	if (mod & SDL_KMOD_NUM)
		event.kbd.flags |= Common::KBD_NUM;
	if (mod & SDL_KMOD_CAPS)
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
	case SDLK_DBLAPOSTROPHE: return Common::KEYCODE_QUOTEDBL;
	case SDLK_HASH: return Common::KEYCODE_HASH;
	case SDLK_DOLLAR: return Common::KEYCODE_DOLLAR;
	case SDLK_AMPERSAND: return Common::KEYCODE_AMPERSAND;
	case SDLK_APOSTROPHE: return Common::KEYCODE_QUOTE;
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
	case SDLK_GRAVE: return Common::KEYCODE_BACKQUOTE;
	case SDLK_A: return Common::KEYCODE_a;
	case SDLK_B: return Common::KEYCODE_b;
	case SDLK_C: return Common::KEYCODE_c;
	case SDLK_D: return Common::KEYCODE_d;
	case SDLK_E: return Common::KEYCODE_e;
	case SDLK_F: return Common::KEYCODE_f;
	case SDLK_G: return Common::KEYCODE_g;
	case SDLK_H: return Common::KEYCODE_h;
	case SDLK_I: return Common::KEYCODE_i;
	case SDLK_J: return Common::KEYCODE_j;
	case SDLK_K: return Common::KEYCODE_k;
	case SDLK_L: return Common::KEYCODE_l;
	case SDLK_M: return Common::KEYCODE_m;
	case SDLK_N: return Common::KEYCODE_n;
	case SDLK_O: return Common::KEYCODE_o;
	case SDLK_P: return Common::KEYCODE_p;
	case SDLK_Q: return Common::KEYCODE_q;
	case SDLK_R: return Common::KEYCODE_r;
	case SDLK_S: return Common::KEYCODE_s;
	case SDLK_T: return Common::KEYCODE_t;
	case SDLK_U: return Common::KEYCODE_u;
	case SDLK_V: return Common::KEYCODE_v;
	case SDLK_W: return Common::KEYCODE_w;
	case SDLK_X: return Common::KEYCODE_x;
	case SDLK_Y: return Common::KEYCODE_y;
	case SDLK_Z: return Common::KEYCODE_z;
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
	case SDLK_UNDO: return Common::KEYCODE_UNDO;
	case SDLK_SCROLLLOCK: return Common::KEYCODE_SCROLLOCK;
	case SDLK_NUMLOCKCLEAR: return Common::KEYCODE_NUMLOCK;
	case SDLK_LGUI: return Common::KEYCODE_LSUPER;
	case SDLK_RGUI: return Common::KEYCODE_RSUPER;
	case SDLK_PRINTSCREEN: return Common::KEYCODE_PRINT;
	case SDLK_APPLICATION: return Common::KEYCODE_COMPOSE;
	case SDLK_KP_0: return Common::KEYCODE_KP0;
	case SDLK_KP_1: return Common::KEYCODE_KP1;
	case SDLK_KP_2: return Common::KEYCODE_KP2;
	case SDLK_KP_3: return Common::KEYCODE_KP3;
	case SDLK_KP_4: return Common::KEYCODE_KP4;
	case SDLK_KP_5: return Common::KEYCODE_KP5;
	case SDLK_KP_6: return Common::KEYCODE_KP6;
	case SDLK_KP_7: return Common::KEYCODE_KP7;
	case SDLK_KP_8: return Common::KEYCODE_KP8;
	case SDLK_KP_9: return Common::KEYCODE_KP9;
	case SDLK_PERCENT: return Common::KEYCODE_PERCENT;
	case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_GRAVE): return Common::KEYCODE_TILDE;
	case SDLK_F16: return Common::KEYCODE_F16;
	case SDLK_F17: return Common::KEYCODE_F17;
	case SDLK_F18: return Common::KEYCODE_F18;
	case SDLK_SLEEP: return Common::KEYCODE_SLEEP;
	case SDLK_VOLUMEUP: return Common::KEYCODE_VOLUMEUP;
	case SDLK_VOLUMEDOWN: return Common::KEYCODE_VOLUMEDOWN;
	case SDLK_MEDIA_EJECT: return Common::KEYCODE_EJECT;
	case SDLK_MEDIA_NEXT_TRACK: return Common::KEYCODE_AUDIONEXT;
	case SDLK_MEDIA_PREVIOUS_TRACK: return Common::KEYCODE_AUDIOPREV;
	case SDLK_MEDIA_STOP: return Common::KEYCODE_AUDIOSTOP;
	case SDLK_MEDIA_PLAY: return Common::KEYCODE_AUDIOPLAYPAUSE;
	case SDLK_MUTE: return Common::KEYCODE_AUDIOMUTE;
	case SDLK_CUT: return Common::KEYCODE_CUT;
	case SDLK_COPY: return Common::KEYCODE_COPY;
	case SDLK_PASTE: return Common::KEYCODE_PASTE;
	case SDLK_SELECT: return Common::KEYCODE_SELECT;
	case SDLK_CANCEL: return Common::KEYCODE_CANCEL;
	case SDLK_AC_SEARCH: return Common::KEYCODE_AC_SEARCH;
	case SDLK_AC_HOME: return Common::KEYCODE_AC_HOME;
	case SDLK_AC_BACK: return Common::KEYCODE_AC_BACK;
	case SDLK_AC_FORWARD: return Common::KEYCODE_AC_FORWARD;
	case SDLK_AC_STOP: return Common::KEYCODE_AC_STOP;
	case SDLK_AC_REFRESH: return Common::KEYCODE_AC_REFRESH;
	case SDLK_AC_BOOKMARKS: return Common::KEYCODE_AC_BOOKMARKS;
	case SDLK_MEDIA_REWIND: return Common::KEYCODE_AUDIOREWIND;
	case SDLK_MEDIA_FAST_FORWARD: return Common::KEYCODE_AUDIOFASTFORWARD;
	default: return Common::KEYCODE_INVALID;
	}
}

void SdlEventSource::preprocessFingerDown(SDL_Event *event) {
	// front (1) or back (2) panel
	SDL_TouchID port = event->tfinger.touchID;
	// id (for multitouch)
	SDL_FingerID id = event->tfinger.fingerID;

	int x = _mouseX;
	int y = _mouseY;

	if (!isTouchPortTouchpadMode(port)) {
		convertTouchXYToGameXY(event->tfinger.x, event->tfinger.y, &x, &y);
	}

	// make sure each finger is not reported down multiple times
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_touchPanels[port]._finger[i].id == id) {
			_touchPanels[port]._finger[i].id = 0;
		}
	}

	// we need the timestamps to decide later if the user performed a short tap (click)
	// or a long tap (drag)
	// we also need the last coordinates for each finger to keep track of dragging
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_touchPanels[port]._finger[i].id == 0) {
			_touchPanels[port]._finger[i].id = id;
			_touchPanels[port]._finger[i].timeLastDown = SDL_NS_TO_MS(event->tfinger.timestamp);
			_touchPanels[port]._finger[i].lastDownX = event->tfinger.x;
			_touchPanels[port]._finger[i].lastDownY = event->tfinger.y;
			_touchPanels[port]._finger[i].lastX = x;
			_touchPanels[port]._finger[i].lastY = y;
			break;
		}
	}
}

void SdlEventSource::finishSimulatedMouseClicks() {
	for (auto &panel : _touchPanels) {
		for (int i = 0; i < 2; i++) {
			if (panel._value._simulatedClickStartTime[i] != 0) {
				Uint32 currentTime = SDL_GetTicks();
				if (currentTime - panel._value._simulatedClickStartTime[i] >= SIMULATED_CLICK_DURATION) {
					int simulatedButton;
					if (i == 0) {
						simulatedButton = SDL_BUTTON_LEFT;
					} else {
						simulatedButton = SDL_BUTTON_RIGHT;
					}
					SDL_Event ev;
					ev.type = SDL_EVENT_MOUSE_BUTTON_UP;
					ev.button.button = simulatedButton;
					ev.button.x = _mouseX;
					ev.button.y = _mouseY;
					SDL_PushEvent(&ev);

					panel._value._simulatedClickStartTime[i] = 0;
				}
			}
		}
	}
}

bool SdlEventSource::preprocessFingerUp(SDL_Event *event, Common::Event *ev) {
	// front (1) or back (2) panel
	SDL_TouchID port = event->tfinger.touchID;
	// id (for multitouch)
	SDL_FingerID id = event->tfinger.fingerID;

	// find out how many fingers were down before this event
	int numFingersDown = 0;
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_touchPanels[port]._finger[i].id != 0) {
			numFingersDown++;
		}
	}

	int x = _mouseX;
	int y = _mouseY;

	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_touchPanels[port]._finger[i].id == id) {
			_touchPanels[port]._finger[i].id = 0;
			if (!_touchPanels[port]._multiFingerDragging) {
				if ((SDL_NS_TO_MS(event->tfinger.timestamp) - _touchPanels[port]._finger[i].timeLastDown) <= MAX_TAP_TIME && !_touchPanels[port]._tapMade) {
					// short (<MAX_TAP_TIME ms) tap is interpreted as right/left mouse click depending on # fingers already down
					// but only if the finger hasn't moved since it was pressed down by more than MAX_TAP_MOTION_DISTANCE pixels
					Common::Point touchscreenSize = getTouchscreenSize();
					float xrel = ((event->tfinger.x * (float) touchscreenSize.x) - (_touchPanels[port]._finger[i].lastDownX * (float) touchscreenSize.x));
					float yrel = ((event->tfinger.y * (float) touchscreenSize.y) - (_touchPanels[port]._finger[i].lastDownY * (float) touchscreenSize.y));
					float maxRSquared = (float) (MAX_TAP_MOTION_DISTANCE * MAX_TAP_MOTION_DISTANCE);
					if ((xrel * xrel + yrel * yrel) < maxRSquared) {
						if (numFingersDown == 3) {
							_touchPanels[port]._tapMade = true;
							ev->type = Common::EVENT_VIRTUAL_KEYBOARD;
							return true;
						} else if (numFingersDown == 2 || numFingersDown == 1) {
							Uint8 simulatedButton = 0;
							if (numFingersDown == 2) {
								simulatedButton = SDL_BUTTON_RIGHT;
								// need to raise the button later
								_touchPanels[port]._simulatedClickStartTime[1] = SDL_NS_TO_MS(event->tfinger.timestamp);
								_touchPanels[port]._tapMade = true;
							} else if (numFingersDown == 1) {
								simulatedButton = SDL_BUTTON_LEFT;
								// need to raise the button later
								_touchPanels[port]._simulatedClickStartTime[0] = SDL_NS_TO_MS(event->tfinger.timestamp);
								if (!isTouchPortTouchpadMode(port)) {
									convertTouchXYToGameXY(event->tfinger.x, event->tfinger.y, &x, &y);
								}
							}

							event->type = SDL_EVENT_MOUSE_BUTTON_DOWN;
							event->button.button = simulatedButton;
							event->button.x = x;
							event->button.y = y;
						}
					}
				}
			} else if (numFingersDown == 1) {
				// when dragging, and the last finger is lifted, the drag is over
				if (!isTouchPortTouchpadMode(port)) {
					convertTouchXYToGameXY(event->tfinger.x, event->tfinger.y, &x, &y);
				}
				Uint8 simulatedButton = 0;
				if (_touchPanels[port]._multiFingerDragging == DRAG_THREE_FINGER)
					simulatedButton = SDL_BUTTON_RIGHT;
				else {
					simulatedButton = SDL_BUTTON_LEFT;
				}
				event->type = SDL_EVENT_MOUSE_BUTTON_UP;
				event->button.button = simulatedButton;
				event->button.x = x;
				event->button.y = y;
				_touchPanels[port]._multiFingerDragging = DRAG_NONE;
			}
		}
	}

	if (numFingersDown == 1) {
		_touchPanels[port]._tapMade = false;
	}

	return false;
}

void SdlEventSource::preprocessFingerMotion(SDL_Event *event) {
	// front (1) or back (2) panel
	SDL_TouchID port = event->tfinger.touchID;
	// id (for multitouch)
	SDL_FingerID id = event->tfinger.fingerID;

	// find out how many fingers were down before this event
	int numFingersDown = 0;
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_touchPanels[port]._finger[i].id != 0) {
			numFingersDown++;
		}
	}

	if (numFingersDown >= 1) {
		int x = _mouseX;
		int y = _mouseY;
		int xMax = _graphicsManager->getWindowWidth() - 1;
		int yMax = _graphicsManager->getWindowHeight() - 1;

		if (!isTouchPortTouchpadMode(port)) {
			convertTouchXYToGameXY(event->tfinger.x, event->tfinger.y, &x, &y);
		} else {
			// for relative mode, use the pointer speed setting
			const int kbdMouseSpeed = CLIP<int>(ConfMan.getInt("kbdmouse_speed"), 0, 7);
			float speedFactor = (kbdMouseSpeed + 1) * 0.25;

			// convert touch events to relative mouse pointer events
			// track sub-pixel relative finger motion using the FINGER_SUBPIXEL_MULTIPLIER
			_touchPanels[port]._hiresDX += (event->tfinger.dx * 1.25 * speedFactor * xMax * FINGER_SUBPIXEL_MULTIPLIER);
			_touchPanels[port]._hiresDY += (event->tfinger.dy * 1.25 * speedFactor * yMax * FINGER_SUBPIXEL_MULTIPLIER);
			int xRel = _touchPanels[port]._hiresDX / FINGER_SUBPIXEL_MULTIPLIER;
			int yRel = _touchPanels[port]._hiresDY / FINGER_SUBPIXEL_MULTIPLIER;
			x = _mouseX + xRel;
			y = _mouseY + yRel;
			_touchPanels[port]._hiresDX %= FINGER_SUBPIXEL_MULTIPLIER;
			_touchPanels[port]._hiresDY %= FINGER_SUBPIXEL_MULTIPLIER;
		}

		x = CLIP(x, 0, xMax);
		y = CLIP(y, 0, yMax);

		// update the current finger's coordinates so we can track it later
		for (int i = 0; i < MAX_NUM_FINGERS; i++) {
			if (_touchPanels[port]._finger[i].id == id) {
				_touchPanels[port]._finger[i].lastX = x;
				_touchPanels[port]._finger[i].lastY = y;
			}
		}

		// If we are starting a multi-finger drag, start holding down the mouse button
		if (numFingersDown >= 2) {
			if (!_touchPanels[port]._multiFingerDragging) {
				// only start a multi-finger drag if at least two fingers have been down long enough
				int numFingersDownLong = 0;
				for (int i = 0; i < MAX_NUM_FINGERS; i++) {
					if (_touchPanels[port]._finger[i].id != 0) {
						if (SDL_NS_TO_MS(event->tfinger.timestamp) - _touchPanels[port]._finger[i].timeLastDown > MAX_TAP_TIME) {
							numFingersDownLong++;
						}
					}
				}
				if (numFingersDownLong >= 2) {
					// starting drag, so push mouse down at current location (back)
					// or location of "oldest" finger (front)
					int mouseDownX = _mouseX;
					int mouseDownY = _mouseY;
					if (!isTouchPortTouchpadMode(port)) {
						for (int i = 0; i < MAX_NUM_FINGERS; i++) {
							if (_touchPanels[port]._finger[i].id == id) {
								Uint32 earliestTime = _touchPanels[port]._finger[i].timeLastDown;
								for (int j = 0; j < MAX_NUM_FINGERS; j++) {
									if (_touchPanels[port]._finger[j].id != 0 && (i != j) ) {
										if (_touchPanels[port]._finger[j].timeLastDown < earliestTime) {
											mouseDownX = _touchPanels[port]._finger[j].lastX;
											mouseDownY = _touchPanels[port]._finger[j].lastY;
											earliestTime = _touchPanels[port]._finger[j].timeLastDown;
										}
									}
								}
								break;
							}
						}
					}
					Uint8 simulatedButton = 0;
					if (numFingersDownLong == 2) {
						simulatedButton = SDL_BUTTON_LEFT;
						_touchPanels[port]._multiFingerDragging = DRAG_TWO_FINGER;
					} else {
						simulatedButton = SDL_BUTTON_RIGHT;
						_touchPanels[port]._multiFingerDragging = DRAG_THREE_FINGER;
					}
					SDL_Event ev;
					ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
					ev.button.button = simulatedButton;
					ev.button.x = mouseDownX;
					ev.button.y = mouseDownY;
					SDL_PushEvent(&ev);
				}
			}
		}

		//check if this is the "oldest" finger down (or the only finger down), otherwise it will not affect mouse motion
		bool updatePointer = true;
		if (numFingersDown > 1) {
			for (int i = 0; i < MAX_NUM_FINGERS; i++) {
				if (_touchPanels[port]._finger[i].id == id) {
					for (int j = 0; j < MAX_NUM_FINGERS; j++) {
						if (_touchPanels[port]._finger[j].id != 0 && (i != j) ) {
							if (_touchPanels[port]._finger[j].timeLastDown < _touchPanels[port]._finger[i].timeLastDown) {
								updatePointer = false;
							}
						}
					}
				}
			}
		}
		if (updatePointer) {
			event->type = SDL_EVENT_MOUSE_MOTION;
			event->motion.x = x;
			event->motion.y = y;
		}
	}
}

bool SdlEventSource::pollEvent(Common::Event &event) {
	finishSimulatedMouseClicks();

	// In case we still need to send a key up event for a key down from a
	// TEXTINPUT event we do this immediately.
	if (_queuedFakeKeyUp) {
		event = _fakeKeyUp;
		_queuedFakeKeyUp = false;
		return true;
	}

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

		// Supported touch gestures:
		// left mouse click: single finger short tap
		// right mouse click: second finger short tap while first finger is still down
		// pointer motion: single finger drag
		if (ev.type == SDL_EVENT_FINGER_DOWN || ev.type == SDL_EVENT_FINGER_UP || ev.type == SDL_EVENT_FINGER_MOTION) {
			// front (0) or back (1) panel
			SDL_TouchID port = ev.tfinger.touchID;
			// touchpad_mouse_mode off: use only front panel for direct touch control of pointer
			// touchpad_mouse_mode on: also enable rear touch with indirect touch control
			// where the finger can be somewhere else than the pointer and still move it
			if (isTouchPortActive(port)) {
				switch (ev.type) {
				case SDL_EVENT_FINGER_DOWN:
					preprocessFingerDown(&ev);
					break;
				case SDL_EVENT_FINGER_UP:
					if (preprocessFingerUp(&ev, &event))
						return true;
					break;
				case SDL_EVENT_FINGER_MOTION:
					preprocessFingerMotion(&ev);
					break;
				}
			}
		}

#if defined(USE_IMGUI)
		ImGui_ImplSDL3_ProcessEvent(&ev);
		ImGuiIO &io = ImGui::GetIO();
		if (io.WantTextInput || io.WantCaptureMouse)
			continue;
#endif
		if (dispatchSDLEvent(ev, event))
			return true;
	}

	return false;
}

bool SdlEventSource::dispatchSDLEvent(SDL_Event &ev, Common::Event &event) {
	switch (ev.type) {
	case SDL_EVENT_KEY_DOWN:
		return handleKeyDown(ev, event);
	case SDL_EVENT_KEY_UP:
		return handleKeyUp(ev, event);
	case SDL_EVENT_MOUSE_MOTION:
		return handleMouseMotion(ev, event);
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
		return handleMouseButtonDown(ev, event);
	case SDL_EVENT_MOUSE_BUTTON_UP:
		return handleMouseButtonUp(ev, event);

	case SDL_EVENT_MOUSE_WHEEL: {
		Sint32 yDir = ev.wheel.y;
		// We want the mouse coordinates supplied with a mouse wheel event.
		// However, SDL2 does not supply these, thus we use whatever we got
		// last time.
		if (!processMouseEvent(event, _mouseX, _mouseY)) {
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

	case SDL_EVENT_TEXT_INPUT: {
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

		case SDL_EVENT_WINDOW_EXPOSED:
			if (_graphicsManager) {
				_graphicsManager->notifyVideoExpose();
			}
			return false;

		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			return handleResizeEvent(event, ev.window.data1, ev.window.data2);

		case SDL_EVENT_WINDOW_FOCUS_GAINED: {
			// When we gain focus, we to update whether the display can turn off
			// dependingif a game isn't running or not
			event.type = Common::EVENT_FOCUS_GAINED;
			if (_engineRunning) {
				SDL_DisableScreenSaver();
			} else {
				SDL_EnableScreenSaver();
			}
			return true;
		}

		case SDL_EVENT_WINDOW_FOCUS_LOST: {
			// Always allow the display to turn off if ScummVM is out of focus
			event.type = Common::EVENT_FOCUS_LOST;
			SDL_EnableScreenSaver();
			return true;
		}

	case SDL_EVENT_JOYSTICK_ADDED:
		return handleJoystickAdded(ev.jdevice, event);

	case SDL_EVENT_JOYSTICK_REMOVED:
		return handleJoystickRemoved(ev.jdevice, event);

	case SDL_EVENT_DROP_FILE:
		event.type = Common::EVENT_DROP_FILE;
		event.path = Common::Path(ev.drop.data, Common::Path::kNativeSeparator);
		return true;

	case SDL_EVENT_CLIPBOARD_UPDATE:
		event.type = Common::EVENT_CLIPBOARD_UPDATE;
		return true;

	case SDL_EVENT_QUIT:
		event.type = Common::EVENT_QUIT;
		return true;

	default:
		break;
	}

	if (_joystick) {
		switch (ev.type) {
		case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
			return handleJoyButtonDown(ev, event);
		case SDL_EVENT_JOYSTICK_BUTTON_UP:
			return handleJoyButtonUp(ev, event);
		case SDL_EVENT_JOYSTICK_AXIS_MOTION:
			return handleJoyAxisMotion(ev, event);
		case SDL_EVENT_JOYSTICK_HAT_MOTION:
			return handleJoyHatMotion(ev, event);
		default:
			break;
		}
	}

	if (_controller) {
		switch (ev.type) {
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			return handleControllerButton(ev, event, false);
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
			return handleControllerButton(ev, event, true);
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
			return handleControllerAxisMotion(ev, event);
		default:
			break;
		}
	}
	return false;
}


bool SdlEventSource::handleKeyDown(SDL_Event &ev, Common::Event &event) {

	SDLModToOSystemKeyFlags(SDL_GetModState(), event);

	SDL_Keycode sdlKeycode = ev.key.key;
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

	SDL_Keymod mod = ev.key.mod;
#if defined(__amigaos4__)
	// On AmigaOS, SDL always reports numlock as off. However, we get KEYCODE_KP# only when
	// it is on, and get different keycodes (for example KEYCODE_PAGEDOWN) when it is off.
	if (event.kbd.keycode >= Common::KEYCODE_KP0 && event.kbd.keycode <= Common::KEYCODE_KP9) {
		event.kbd.flags |= Common::KBD_NUM;
		mod = SDL_Keymod(mod | KMOD_NUM);
	}
#endif
	event.kbd.ascii = mapKey(sdlKeycode, mod, obtainUnicode(ev.key));

	event.kbdRepeat = ev.key.repeat;

	return true;
}

bool SdlEventSource::handleKeyUp(SDL_Event &ev, Common::Event &event) {
	if (remapKey(ev, event))
		return true;

	SDLModToOSystemKeyFlags(SDL_GetModState(), event);

	SDL_Keycode sdlKeycode = ev.key.key;

	// Set the scroll lock sticky flag
	if (_scrollLock)
		event.kbd.flags |= Common::KBD_SCRL;

	event.type = Common::EVENT_KEYUP;
	event.kbd.keycode = SDLToOSystemKeycode(sdlKeycode);

	SDL_Keymod mod = ev.key.mod;

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
	int numJoysticks = 0;
	auto joystickIds = SDL_GetJoysticks(&numJoysticks);
	if (!joystickIds) {
		warning("Failed to get connected joysticks: %s", SDL_GetError());
	} else if (numJoysticks > joystickIndex) {
		auto joystickId = joystickIds[joystickIndex];
		if (SDL_IsGamepad(joystickId)) {
			_controller = SDL_OpenGamepad(joystickId);
			debug("Using game controller: %s", SDL_GetGamepadName(_controller));
		} else {
			_joystick = SDL_OpenJoystick(joystickId);
			debug("Using joystick: %s", SDL_GetJoystickName(_joystick));
		}
	} else {
		debug(5, "Invalid joystick: %d", joystickIndex);
	}
	SDL_free(joystickIds);
}

void SdlEventSource::closeJoystick() {
	if (_controller) {
		SDL_CloseGamepad(_controller);
		_controller = nullptr;
	}
	if (_joystick) {
		SDL_CloseJoystick(_joystick);
		_joystick = nullptr;
	}
}

bool SdlEventSource::handleJoystickAdded(const SDL_JoyDeviceEvent &device, Common::Event &event) {
	debug(5, "SdlEventSource: Received joystick added event for index '%d'", device.which);

	int joystick_num = ConfMan.getInt("joystick_num");
	if (joystick_num != device.which) {
		return false;
	}

	debug(5, "SdlEventSource: Newly added joystick with index '%d' matches 'joysticky_num', trying to use it", device.which);

	closeJoystick();
	openJoystick(joystick_num);

	event.type = Common::EVENT_INPUT_CHANGED;
	return true;
}

bool SdlEventSource::handleJoystickRemoved(const SDL_JoyDeviceEvent &device, Common::Event &event) {
	debug(5, "SdlEventSource: Received joystick removed event for instance id '%d'", device.which);

	SDL_Joystick *joystick;
	if (_controller) {
		joystick = SDL_GetGamepadJoystick(_controller);
	} else {
		joystick = _joystick;
	}

	if (!joystick) {
		return false;
	}

	if (SDL_GetJoystickID(joystick) != device.which) {
		return false;
	}

	debug(5, "SdlEventSource: Newly removed joystick with instance id '%d' matches currently used joystick, closing current joystick", device.which);

	closeJoystick();

	event.type = Common::EVENT_INPUT_CHANGED;
	return true;
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
#if SDL_VERSION_ATLEAST(3, 0, 0)
	int button = mapSDLControllerButtonToOSystem(ev.gbutton.button);
#else
	int button = mapSDLControllerButtonToOSystem(ev.cbutton.button);
#endif

	if (button < 0)
		return false;

	event.type = buttonUp ? Common::EVENT_JOYBUTTON_UP : Common::EVENT_JOYBUTTON_DOWN;
	event.joystick.button = button;

	return true;
}

bool SdlEventSource::handleControllerAxisMotion(const SDL_Event &ev, Common::Event &event) {
	event.joystick.axis = ev.gaxis.axis;
	event.joystick.position = ev.gaxis.value;
	event.type = Common::EVENT_JOYAXIS_MOTION;

	return true;
}

bool SdlEventSource::isJoystickConnected() const {
	return _joystick || _controller;
}

uint32 SdlEventSource::obtainUnicode(const SDL_KeyboardEvent &key) {
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
	int n = SDL_PeepEvents(events, 2, SDL_PEEKEVENT, SDL_EVENT_KEY_DOWN, SDL_EVENT_TEXT_INPUT);
	// Make sure that the TEXTINPUT event belongs to this KEYDOWN
	// event and not another pending one.
	if ((n > 0 && events[0].type == SDL_EVENT_TEXT_INPUT)
	    || (n > 1 && events[0].type != SDL_EVENT_KEY_DOWN && events[1].type == SDL_EVENT_TEXT_INPUT)) {
		// Remove the text input event we associate with the key press. This
		// makes sure we never get any SDL_TEXTINPUT events which do "belong"
		// to SDL_KEYDOWN events.
		n = SDL_PeepEvents(events, 1, SDL_GETEVENT, SDL_EVENT_TEXT_INPUT, SDL_EVENT_TEXT_INPUT);
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
}

#endif // SDL_BACKEND
