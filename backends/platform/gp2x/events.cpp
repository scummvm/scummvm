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

/*
 * GP2X: Common::Event Handling.
 *
 */

#include "backends/platform/gp2x/gp2x-common.h"
#include "backends/platform/gp2x/gp2x-hw.h"
#include "backends/keymapper/keymapper.h"
#include "common/util.h"
#include "common/events.h"

// FIXME move joystick defines out and replace with confile file options
// we should really allow users to map any key to a joystick button using the keymapper.
#define JOY_DEADZONE 2200

#define JOY_XAXIS 0
#define JOY_YAXIS 1

/* GP2X Wiz: Main Joystick Mappings */
enum {
	GP2X_BUTTON_UP			= 0,
	GP2X_BUTTON_UPLEFT		= 1,
	GP2X_BUTTON_LEFT		= 2,
	GP2X_BUTTON_DOWNLEFT	= 3,
	GP2X_BUTTON_DOWN		= 4,
	GP2X_BUTTON_DOWNRIGHT	= 5,
	GP2X_BUTTON_RIGHT		= 6,
	GP2X_BUTTON_UPRIGHT		= 7,
	GP2X_BUTTON_START		= 8,
	GP2X_BUTTON_SELECT		= 9,
	GP2X_BUTTON_L			= 10,
	GP2X_BUTTON_R			= 11,
	GP2X_BUTTON_A			= 12,
	GP2X_BUTTON_B			= 13,
	GP2X_BUTTON_X			= 14,
	GP2X_BUTTON_Y			= 15,
	GP2X_BUTTON_VOLUP		= 16,
	GP2X_BUTTON_VOLDOWN		= 17,
	GP2X_BUTTON_CLICK		= 18
};

static int mapKey(SDLKey key, SDLMod mod, Uint16 unicode) {
	if (key >= SDLK_F1 && key <= SDLK_F9) {
		return key - SDLK_F1 + Common::ASCII_F1;
	} else if (key >= SDLK_KP0 && key <= SDLK_KP9) {
		return key - SDLK_KP0 + '0';
	} else if (key >= SDLK_UP && key <= SDLK_PAGEDOWN) {
		return key;
	} else if (unicode) {
		return unicode;
	} else if (key >= 'a' && key <= 'z' && (mod & KMOD_SHIFT)) {
		return key & ~0x20;
	} else if (key >= SDLK_NUMLOCK && key <= SDLK_EURO) {
		return 0;
	}
	return key;
}

void OSystem_GP2X::fillMouseEvent(Common::Event &event, int x, int y) {
	event.mouse.x = x;
	event.mouse.y = y;

	// Update the "keyboard mouse" coords
	_km.x = x;
	_km.y = y;

	// Adjust for the screen scaling
	if (!_overlayVisible) {
		event.mouse.x /= _videoMode.scaleFactor;
		event.mouse.y /= _videoMode.scaleFactor;
		if (_videoMode.aspectRatioCorrection)
			event.mouse.y = aspect2Real(event.mouse.y);
	}
}

void OSystem_GP2X::handleKbdMouse() {
	uint32 curTime = getMillis();
	if (curTime >= _km.last_time + _km.delay_time) {
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
				if (curTime > _km.x_down_time + _km.delay_time * 12) {
					if (_km.x_vel > 0)
						_km.x_vel++;
					else
						_km.x_vel--;
				} else if (curTime > _km.x_down_time + _km.delay_time * 8) {
					if (_km.x_vel > 0)
						_km.x_vel = 5;
					else
						_km.x_vel = -5;
				}
			}
			if (_km.y_down_count) {
				if (curTime > _km.y_down_time + _km.delay_time * 12) {
					if (_km.y_vel > 0)
						_km.y_vel++;
					else
						_km.y_vel--;
				} else if (curTime > _km.y_down_time + _km.delay_time * 8) {
					if (_km.y_vel > 0)
						_km.y_vel = 5;
					else
						_km.y_vel = -5;
				}
			}

			_km.x += _km.x_vel;
			_km.y += _km.y_vel;

			if (_km.x < 0) {
				_km.x = 0;
				_km.x_vel = -1;
				_km.x_down_count = 1;
			} else if (_km.x > _km.x_max) {
				_km.x = _km.x_max;
				_km.x_vel = 1;
				_km.x_down_count = 1;
			}

			if (_km.y < 0) {
				_km.y = 0;
				_km.y_vel = -1;
				_km.y_down_count = 1;
			} else if (_km.y > _km.y_max) {
				_km.y = _km.y_max;
				_km.y_vel = 1;
				_km.y_down_count = 1;
			}

			SDL_WarpMouse((Uint16)_km.x, (Uint16)_km.y);
		}
	}
}

static byte SDLModToOSystemKeyFlags(SDLMod mod) {
	byte b = 0;
	if (mod & KMOD_SHIFT)
		b |= Common::KBD_SHIFT;
	if (mod & KMOD_ALT)
		b |= Common::KBD_ALT;
	if (mod & KMOD_CTRL)
		b |= Common::KBD_CTRL;

	return b;
}

void OSystem_GP2X::moveStick() {
	bool stickBtn[32];

	memcpy(stickBtn, _stickBtn, sizeof(stickBtn));

	if ((stickBtn[0])||(stickBtn[2])||(stickBtn[4])||(stickBtn[6]))
		stickBtn[1] = stickBtn[3] = stickBtn[5] = stickBtn[7] = 0;

	if ((stickBtn[1])||(stickBtn[2])||(stickBtn[3])){
		if (_km.x_down_count!=2){
			_km.x_vel = -1;
			_km.x_down_count = 1;
		}else
			_km.x_vel = -4;
	} else if ((stickBtn[5])||(stickBtn[6])||(stickBtn[7])){
		if (_km.x_down_count!=2){
			_km.x_vel = 1;
			_km.x_down_count = 1;
		} else
			_km.x_vel = 4;
	} else {
		_km.x_vel = 0;
		_km.x_down_count = 0;
	}


	if ((stickBtn[0])||(stickBtn[1])||(stickBtn[7])){
		if (_km.y_down_count!=2){
			_km.y_vel = -1;
			_km.y_down_count = 1;
		}else
			_km.y_vel = -4;
	} else if ((stickBtn[3])||(stickBtn[4])||(stickBtn[5])){
		if (_km.y_down_count!=2){
			_km.y_vel = 1;
			_km.y_down_count = 1;
		} else
			_km.y_vel = 4;
	} else {
		_km.y_vel = 0;
		_km.y_down_count = 0;
	}
}

/* Quick default button states for modifiers. */
int GP2X_BUTTON_STATE_L					=	false;

bool OSystem_GP2X::pollEvent(Common::Event &event) {
	SDL_Event ev;
	int axis;
	byte b = 0;

	handleKbdMouse();

	// If the screen mode changed, send an Common::EVENT_SCREEN_CHANGED
	if (_modeChanged) {
		_modeChanged = false;
		event.type = Common::EVENT_SCREEN_CHANGED;
		return true;
	}

	// GP2X Input mappings.

	/*
	Single Button

	Movement:

	GP2X_BUTTON_UP              Cursor Up
	GP2X_BUTTON_DOWN            Cursor Down
	GP2X_BUTTON_LEFT            Cursor Left
	GP2X_BUTTON_RIGHT           Cursor Right

	GP2X_BUTTON_UPLEFT          Cursor Up Left
	GP2X_BUTTON_UPRIGHT         Cursor Up Right
	GP2X_BUTTON_DOWNLEFT        Cursor Down Left
	GP2X_BUTTON_DOWNRIGHT       Cursor Down Right

	Button Emulation:

	GP2X_BUTTON_CLICK           Left Mouse Click
	GP2X_BUTTON_A				. (Period)
	GP2X_BUTTON_B               Left Mouse Click
	GP2X_BUTTON_Y               Space Bar
	GP2X_BUTTON_X               Right Mouse Click
	GP2X_BUTTON_L				Combo Modifier (Left Trigger)
	GP2X_BUTTON_R               Return (Right Trigger)
	GP2X_BUTTON_START           F5 (Game Menu)
	GP2X_BUTTON_SELECT          Escape
	GP2X_BUTTON_VOLUP           /dev/mixer Global Volume Up
	GP2X_BUTTON_VOLDOWN         /dev/mixer Global Volume Down

	Combos:

	GP2X_BUTTON_VOLUP &	GP2X_BUTTON_VOLDOWN		0 (For Monkey 2 CP) or Virtual Keyboard if enabled
	GP2X_BUTTON_L &	GP2X_BUTTON_SELECT			Common::EVENT_QUIT (Calls Sync() to make sure SD is flushed)
	GP2X_BUTTON_L &	GP2X_BUTTON_Y				Toggles setZoomOnMouse() for larger then 320*240 games to scale to the point + raduis.
	GP2X_BUTTON_L &	GP2X_BUTTON_START			Common::EVENT_MAINMENU (ScummVM Global Main Menu)
	GP2X_BUTTON_L &	GP2X_BUTTON_A				Common::EVENT_PREDICTIVE_DIALOG for predictive text entry box (AGI games)
	*/

	while (SDL_PollEvent(&ev)) {

		switch (ev.type) {
		case SDL_KEYDOWN:{
			b = event.kbd.flags = SDLModToOSystemKeyFlags(SDL_GetModState());

			const bool event_complete = remapKey(ev,event);

			if (event_complete)
				return true;

			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = (Common::KeyCode)ev.key.keysym.sym;
			event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);

			return true;
			}
		case SDL_KEYUP:
			{
			const bool event_complete = remapKey(ev,event);

			if (event_complete)
				return true;

			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = (Common::KeyCode)ev.key.keysym.sym;
			event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);
			b = event.kbd.flags = SDLModToOSystemKeyFlags(SDL_GetModState());

			// Ctrl-Alt-<key> will change the GFX mode
			if ((b & (Common::KBD_CTRL|Common::KBD_ALT)) == (Common::KBD_CTRL|Common::KBD_ALT)) {
				// Swallow these key up events
				break;
			}

			return true;
			}
		case SDL_MOUSEMOTION:
			event.type = Common::EVENT_MOUSEMOVE;
			fillMouseEvent(event, ev.motion.x, ev.motion.y);

			setMousePos(event.mouse.x, event.mouse.y);
			return true;

		case SDL_MOUSEBUTTONDOWN:
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
				break;

			fillMouseEvent(event, ev.button.x, ev.button.y);

			return true;

		case SDL_MOUSEBUTTONUP:
			if (ev.button.button == SDL_BUTTON_LEFT)
				event.type = Common::EVENT_LBUTTONUP;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				event.type = Common::EVENT_RBUTTONUP;
#if defined(SDL_BUTTON_MIDDLE)
			else if (ev.button.button == SDL_BUTTON_MIDDLE)
				event.type = Common::EVENT_MBUTTONUP;
#endif
			else
				break;
			fillMouseEvent(event, ev.button.x, ev.button.y);

			return true;

		// GP2X Button mapings. Main code

		case SDL_JOYBUTTONDOWN:
			_stickBtn[ev.jbutton.button] = 1;
			if (ev.jbutton.button == GP2X_BUTTON_B) {
				event.type = Common::EVENT_LBUTTONDOWN;
				fillMouseEvent(event, _km.x, _km.y);
			} else if (ev.jbutton.button == GP2X_BUTTON_CLICK) {
				event.type = Common::EVENT_LBUTTONDOWN;
				fillMouseEvent(event, _km.x, _km.y);
			} else if (ev.jbutton.button == GP2X_BUTTON_X) {
				event.type = Common::EVENT_RBUTTONDOWN;
				fillMouseEvent(event, _km.x, _km.y);
			} else if (_stickBtn[GP2X_BUTTON_L] && (ev.jbutton.button == GP2X_BUTTON_SELECT)) {
				event.type = Common::EVENT_QUIT;
			} else if (ev.jbutton.button < 8) {
				moveStick();
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);
			} else {
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.flags = 0;
				switch (ev.jbutton.button) {
					case GP2X_BUTTON_L:
						GP2X_BUTTON_STATE_L = true;
						break;
					case GP2X_BUTTON_R:
						if (GP2X_BUTTON_STATE_L == true) {
#ifdef ENABLE_VKEYBD
							event.kbd.keycode = Common::KEYCODE_F7;
							event.kbd.ascii = mapKey(SDLK_F7, ev.key.keysym.mod, 0);
#else
							event.kbd.keycode = Common::KEYCODE_0;
							event.kbd.ascii = mapKey(SDLK_0, ev.key.keysym.mod, 0);
#endif
						} else {
							event.kbd.keycode = Common::KEYCODE_RETURN;
							event.kbd.ascii = mapKey(SDLK_RETURN, ev.key.keysym.mod, 0);
						}
						break;
					case GP2X_BUTTON_SELECT:
						if (GP2X_BUTTON_STATE_L == true) {
							event.type = Common::EVENT_QUIT;
						} else {
							event.kbd.keycode = Common::KEYCODE_ESCAPE;
							event.kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
						}
						break;
					case GP2X_BUTTON_A:
						if (GP2X_BUTTON_STATE_L == true) {
							event.type = Common::EVENT_PREDICTIVE_DIALOG;
						} else {
						event.kbd.keycode = Common::KEYCODE_PERIOD;
						event.kbd.ascii = mapKey(SDLK_PERIOD, ev.key.keysym.mod, 0);
						}
						break;
					case GP2X_BUTTON_Y:
						if (GP2X_BUTTON_STATE_L == true) {
							setZoomOnMouse();
						} else {
							event.kbd.keycode = Common::KEYCODE_SPACE;
							event.kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
						}
						break;
					case GP2X_BUTTON_START:
						if (GP2X_BUTTON_STATE_L == true) {
							event.type = Common::EVENT_MAINMENU;
						} else {
							event.kbd.keycode = Common::KEYCODE_F5;
							event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
						}
						break;
					case GP2X_BUTTON_VOLUP:
						GP2X_HW::mixerMoveVolume(2);
						if (GP2X_HW::volumeLevel == 100) {
							displayMessageOnOSD("Maximum Volume");
						} else {
							displayMessageOnOSD("Increasing Volume");
						}
						break;

					case GP2X_BUTTON_VOLDOWN:
						GP2X_HW::mixerMoveVolume(1);
						if (GP2X_HW::volumeLevel == 0) {
							displayMessageOnOSD("Minimal Volume");
						} else {
							displayMessageOnOSD("Decreasing Volume");
						}
						break;
				}
			}
			return true;

		case SDL_JOYBUTTONUP:
			_stickBtn[ev.jbutton.button] = 0;
			if (ev.jbutton.button == GP2X_BUTTON_B) {
				event.type = Common::EVENT_LBUTTONUP;
				fillMouseEvent(event, _km.x, _km.y);
			} else if (ev.jbutton.button == GP2X_BUTTON_CLICK) {
				event.type = Common::EVENT_LBUTTONUP;
				fillMouseEvent(event, _km.x, _km.y);
			} else if (ev.jbutton.button == GP2X_BUTTON_X) {
				event.type = Common::EVENT_RBUTTONUP;
				fillMouseEvent(event, _km.x, _km.y);
			} else if (ev.jbutton.button < 8) {
				moveStick();
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);
			} else {
				event.type = Common::EVENT_KEYUP;
				event.kbd.flags = 0;
				switch (ev.jbutton.button) {
					case GP2X_BUTTON_SELECT:
						event.kbd.keycode = Common::KEYCODE_ESCAPE;
						event.kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
						break;
					case GP2X_BUTTON_A:
						event.kbd.keycode = Common::KEYCODE_PERIOD;
						event.kbd.ascii = mapKey(SDLK_PERIOD, ev.key.keysym.mod, 0);
						break;
					case GP2X_BUTTON_Y:
						event.kbd.keycode = Common::KEYCODE_SPACE;
						event.kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
						break;
					case GP2X_BUTTON_START:
						if (GP2X_BUTTON_STATE_L == true) {
							event.type = Common::EVENT_MAINMENU;
						} else {
							event.kbd.keycode = Common::KEYCODE_F5;
							event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
						}
						break;
					case GP2X_BUTTON_L:
						GP2X_BUTTON_STATE_L = false;
						break;
					case GP2X_BUTTON_R:
						if (GP2X_BUTTON_STATE_L == true) {
#ifdef ENABLE_VKEYBD
							event.kbd.keycode = Common::KEYCODE_F7;
							event.kbd.ascii = mapKey(SDLK_F7, ev.key.keysym.mod, 0);
#else
							event.kbd.keycode = Common::KEYCODE_0;
							event.kbd.ascii = mapKey(SDLK_0, ev.key.keysym.mod, 0);
#endif
						} else {
							event.kbd.keycode = Common::KEYCODE_RETURN;
							event.kbd.ascii = mapKey(SDLK_RETURN, ev.key.keysym.mod, 0);
						}
						break;
					case GP2X_BUTTON_VOLUP:
						break;
					case GP2X_BUTTON_VOLDOWN:
						break;
				}
			}
			return true;

		case SDL_JOYAXISMOTION:
			axis = ev.jaxis.value;
			if ( axis > JOY_DEADZONE) {
				axis -= JOY_DEADZONE;
				event.type = Common::EVENT_MOUSEMOVE;
			} else if ( axis < -JOY_DEADZONE ) {
				axis += JOY_DEADZONE;
				event.type = Common::EVENT_MOUSEMOVE;
			} else
				axis = 0;

			if ( ev.jaxis.axis == JOY_XAXIS) {
				if (axis != 0) {
					_km.x_vel = (axis > 0) ? 1:-1;
					_km.x_down_count = 1;
				} else {
					_km.x_vel = 0;
					_km.x_down_count = 0;
				}

			} else if (ev.jaxis.axis == JOY_YAXIS) {
#ifndef JOY_INVERT_Y
				axis = -axis;
#endif
#ifdef JOY_ANALOG
				_km.y_vel = -axis / 2000;
				_km.y_down_count = 0;
#else
				if (axis != 0) {
					_km.y_vel = (-axis > 0) ? 1: -1;
					_km.y_down_count = 1;
				} else {
					_km.y_vel = 0;
					_km.y_down_count = 0;
				}
#endif
			}

			fillMouseEvent(event, _km.x, _km.y);

			return true;

		case SDL_VIDEOEXPOSE:
			_forceFull = true;
			break;

		case SDL_QUIT:
			event.type = Common::EVENT_QUIT;
			return true;
		}
	}
	return false;
}

bool OSystem_GP2X::remapKey(const SDL_Event &ev,Common::Event &event) {
	return false;
}

void OSystem_GP2X::setupKeymapper() {
#ifdef ENABLE_KEYMAPPER
	using namespace Common;
	Keymapper *mapper = getEventManager()->getKeymapper();

	HardwareKeySet *keySet = new HardwareKeySet();
	keySet->addHardwareKey(new HardwareKey( "a", KeyState(KEYCODE_a), "a", kActionKeyType ));
	keySet->addHardwareKey(new HardwareKey( "s", KeyState(KEYCODE_s), "s", kActionKeyType ));
	keySet->addHardwareKey(new HardwareKey( "d", KeyState(KEYCODE_d), "d", kActionKeyType ));
	keySet->addHardwareKey(new HardwareKey( "f", KeyState(KEYCODE_f), "f", kActionKeyType ));
	keySet->addHardwareKey(new HardwareKey( "n", KeyState(KEYCODE_n), "n (vk)", kTriggerLeftKeyType, kVirtualKeyboardActionType ));
	keySet->addHardwareKey(new HardwareKey( "m", KeyState(KEYCODE_m), "m (remap)", kTriggerRightKeyType, kKeyRemapActionType ));
	keySet->addHardwareKey(new HardwareKey( "[", KeyState(KEYCODE_LEFTBRACKET), "[ (select)", kSelectKeyType ));
	keySet->addHardwareKey(new HardwareKey( "]", KeyState(KEYCODE_RIGHTBRACKET), "] (start)", kStartKeyType ));
	mapper->registerHardwareKeySet(keySet);

	Keymap *globalMap = new Keymap("global");
	Keymap *guiMap = new Keymap("gui");
	Action *act;
	Event evt ;

	act = new Action(globalMap, "MENU", "Menu", kGenericActionType, kSelectKeyType);
	act->addKeyEvent(KeyState(KEYCODE_F5, ASCII_F5, 0));

	act = new Action(globalMap, "SKCT", "Skip", kGenericActionType, kActionKeyType);
	act->addKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE, 0));

	act = new Action(globalMap, "PAUS", "Pause", kGenericActionType, kStartKeyType);
	act->addKeyEvent(KeyState(KEYCODE_SPACE, ' ', 0));

	act = new Action(globalMap, "SKLI", "Skip line", kGenericActionType, kActionKeyType);
	act->addKeyEvent(KeyState(KEYCODE_PERIOD, '.', 0));

	act = new Action(globalMap, "VIRT", "Display keyboard", kVirtualKeyboardActionType);
	act->addKeyEvent(KeyState(KEYCODE_F6, ASCII_F6, 0));

	act = new Action(globalMap, "REMP", "Remap keys", kKeyRemapActionType);
	act->addKeyEvent(KeyState(KEYCODE_F7, ASCII_F7, 0));

	mapper->addGlobalKeymap(globalMap);

	act = new Action(guiMap, "CLOS", "Close", kGenericActionType, kStartKeyType);
	act->addKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE, 0));

	act = new Action(guiMap, "CLIK", "Mouse click");
	act->addLeftClickEvent();

	act = new Action(guiMap, "VIRT", "Display keyboard", kVirtualKeyboardActionType);
	act->addKeyEvent(KeyState(KEYCODE_F6, ASCII_F6, 0));

	act = new Action(guiMap, "REMP", "Remap keys", kKeyRemapActionType);
	act->addKeyEvent(KeyState(KEYCODE_F7, ASCII_F7, 0));

	mapper->addGlobalKeymap(guiMap);

	mapper->pushKeymap("global");
#endif
}

