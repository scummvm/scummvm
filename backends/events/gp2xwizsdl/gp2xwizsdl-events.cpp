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

#ifdef GP2XWIZ

#include "backends/events/gp2xwizsdl/gp2xwizsdl-events.h"
#include "backends/platform/gp2xwiz/gp2xwiz-hw.h"
#include "backends/platform/gp2xwiz/gp2xwiz-sdl.h"

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
	GP2X_BUTTON_MENU		= 8,
	GP2X_BUTTON_SELECT		= 9,
	GP2X_BUTTON_L			= 10,
	GP2X_BUTTON_R			= 11,
	GP2X_BUTTON_A			= 12,
	GP2X_BUTTON_B			= 13,
	GP2X_BUTTON_X			= 14,
	GP2X_BUTTON_Y			= 15,
	GP2X_BUTTON_VOLUP		= 16,
	GP2X_BUTTON_VOLDOWN		= 17
};

GP2XWIZSdlEventManager::GP2XWIZSdlEventManager(Common::EventSource *boss)
	:
	_buttonStateL(false),
	SdlEventManager(boss) {

}

void GP2XWIZSdlEventManager::SDLModToOSystemKeyFlags(SDLMod mod, Common::Event &event) {
	event.kbd.flags = 0;

	if (mod & KMOD_SHIFT)
		event.kbd.flags |= Common::KBD_SHIFT;
	if (mod & KMOD_ALT)
		event.kbd.flags |= Common::KBD_ALT;
	if (mod & KMOD_CTRL)
		event.kbd.flags |= Common::KBD_CTRL;
}

void GP2XWIZSdlEventManager::moveStick() {
	bool stickBtn[32];

	memcpy(stickBtn, _stickBtn, sizeof(stickBtn));

	if ((stickBtn[0])||(stickBtn[2])||(stickBtn[4])||(stickBtn[6]))
		stickBtn[1] = stickBtn[3] = stickBtn[5] = stickBtn[7] = 0;

	if ((stickBtn[1])||(stickBtn[2])||(stickBtn[3])) {
		if (_km.x_down_count!=2) {
			_km.x_vel = -1;
			_km.x_down_count = 1;
		} else
			_km.x_vel = -4;
	} else if ((stickBtn[5])||(stickBtn[6])||(stickBtn[7])) {
		if (_km.x_down_count!=2) {
			_km.x_vel = 1;
			_km.x_down_count = 1;
		} else
			_km.x_vel = 4;
	} else {
		_km.x_vel = 0;
		_km.x_down_count = 0;
	}

	if ((stickBtn[0])||(stickBtn[1])||(stickBtn[7])) {
		if (_km.y_down_count!=2) {
			_km.y_vel = -1;
			_km.y_down_count = 1;
		} else
			_km.y_vel = -4;
	} else if ((stickBtn[3])||(stickBtn[4])||(stickBtn[5])) {
		if (_km.y_down_count!=2) {
			_km.y_vel = 1;
			_km.y_down_count = 1;
		} else
			_km.y_vel = 4;
	} else {
		_km.y_vel = 0;
		_km.y_down_count = 0;
	}
}

/* GP2X Wiz Input mappings.
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

GP2X_BUTTON_A				. (Period)
GP2X_BUTTON_B               Left Mouse Click
GP2X_BUTTON_Y               Space Bar
GP2X_BUTTON_X               Right Mouse Click
GP2X_BUTTON_L				Combo Modifier (Left Trigger)
GP2X_BUTTON_R               Return (Right Trigger)
GP2X_BUTTON_MENU			F5 (Game Menu)
GP2X_BUTTON_SELECT          Escape
GP2X_BUTTON_VOLUP           /dev/mixer Global Volume Up
GP2X_BUTTON_VOLDOWN         /dev/mixer Global Volume Down

Combos:

GP2X_BUTTON_VOLUP &	GP2X_BUTTON_VOLDOWN		0 (For Monkey 2 CP) or Virtual Keyboard if enabled
GP2X_BUTTON_L &	GP2X_BUTTON_SELECT			Common::EVENT_QUIT (Calls Sync() to make sure SD is flushed)
GP2X_BUTTON_L &	GP2X_BUTTON_MENU			Common::EVENT_MAINMENU (ScummVM Global Main Menu)
GP2X_BUTTON_L &	GP2X_BUTTON_A				Common::EVENT_PREDICTIVE_DIALOG for predictive text entry box (AGI games)
*/

bool GP2XWIZSdlEventManager::handleKeyDown(SDL_Event &ev, Common::Event &event) {
	SDLModToOSystemKeyFlags(SDL_GetModState(), event);

	if (remapKey(ev, event))
		return true;

	event.type = Common::EVENT_KEYDOWN;
	event.kbd.keycode = (Common::KeyCode)ev.key.keysym.sym;
	event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);

	return true;
}

bool GP2XWIZSdlEventManager::handleKeyUp(SDL_Event &ev, Common::Event &event) {
	if (remapKey(ev, event))
		return true;

	event.type = Common::EVENT_KEYUP;
	event.kbd.keycode = (Common::KeyCode)ev.key.keysym.sym;
	event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);
	SDLModToOSystemKeyFlags(SDL_GetModState(), event);

	// Ctrl-Alt-<key> will change the GFX mode
	if ((event.kbd.flags & (Common::KBD_CTRL | Common::KBD_ALT)) == (Common::KBD_CTRL | Common::KBD_ALT)) {
		// Swallow these key up events
		return false;
	}

	return true;
}

bool GP2XWIZSdlEventManager::handleJoyButtonDown(SDL_Event &ev, Common::Event &event) {
	_stickBtn[ev.jbutton.button] = 1;
	if (ev.jbutton.button == GP2X_BUTTON_B) {
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
				_buttonStateL = true;
				break;
			case GP2X_BUTTON_R:
				if (_buttonStateL == true) {
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
				if (_buttonStateL == true) {
					event.type = Common::EVENT_QUIT;
				} else {
					event.kbd.keycode = Common::KEYCODE_ESCAPE;
					event.kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
				}
				break;
			case GP2X_BUTTON_A:
				if (_buttonStateL == true) {
					event.type = Common::EVENT_PREDICTIVE_DIALOG;
				} else {
				event.kbd.keycode = Common::KEYCODE_PERIOD;
				event.kbd.ascii = mapKey(SDLK_PERIOD, ev.key.keysym.mod, 0);
				}
				break;
			case GP2X_BUTTON_Y:
				event.kbd.keycode = Common::KEYCODE_SPACE;
				event.kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
				break;
			case GP2X_BUTTON_MENU:
				if (_buttonStateL == true) {
					event.type = Common::EVENT_MAINMENU;
				} else {
					event.kbd.keycode = Common::KEYCODE_F5;
					event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
				}
				break;
			case GP2X_BUTTON_VOLUP:
				WIZ_HW::mixerMoveVolume(2);
				if (WIZ_HW::volumeLevel == 100) {
					g_system->displayMessageOnOSD("Maximum Volume");
				} else {
					g_system->displayMessageOnOSD("Increasing Volume");
				}
				break;

			case GP2X_BUTTON_VOLDOWN:
				WIZ_HW::mixerMoveVolume(1);
				if (WIZ_HW::volumeLevel == 0) {
					g_system->displayMessageOnOSD("Minimal Volume");
				} else {
					g_system->displayMessageOnOSD("Decreasing Volume");
				}
				break;
		}
	}
	return true;
}

bool GP2XWIZSdlEventManager::handleJoyButtonUp(SDL_Event &ev, Common::Event &event) {
	_stickBtn[ev.jbutton.button] = 0;
	if (ev.jbutton.button == GP2X_BUTTON_B) {
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
			case GP2X_BUTTON_MENU:
				if (_buttonStateL == true) {
					event.type = Common::EVENT_MAINMENU;
				} else {
					event.kbd.keycode = Common::KEYCODE_F5;
					event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
				}
				break;
			case GP2X_BUTTON_L:
				_buttonStateL = false;
				break;
			case GP2X_BUTTON_R:
				if (_buttonStateL == true) {
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
}

#endif
