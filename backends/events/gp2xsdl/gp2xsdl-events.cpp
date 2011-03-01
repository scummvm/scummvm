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

#include "common/scummsys.h"

#if defined(GP2X)

#include "backends/events/gp2xsdl/gp2xsdl-events.h"
#if defined(GP2X)
#include "backends/platform/gp2x/gp2x-hw.h"
#include "backends/graphics/gp2xsdl/gp2xsdl-graphics.h"
#else
#include "backends/platform/gp2xwiz/gp2xwiz-hw.h"
#endif

#include "backends/platform/sdl/sdl.h"

// FIXME move joystick defines out and replace with confile file options
// we should really allow users to map any key to a joystick button using the keymapper.
#define JOY_DEADZONE 2200

#define JOY_XAXIS 0
#define JOY_YAXIS 1

/* Quick default button states for modifiers. */
int BUTTON_STATE_L					=	false;

enum {
	/* DPAD/Stick */
	BUTTON_UP			= 0,
	BUTTON_UPLEFT		= 1,
	BUTTON_LEFT			= 2,
	BUTTON_DOWNLEFT		= 3,
	BUTTON_DOWN			= 4,
	BUTTON_DOWNRIGHT	= 5,
	BUTTON_RIGHT		= 6,
	BUTTON_UPRIGHT		= 7,
	/* Joystick Buttons */
	BUTTON_MENU			= 8,	// Start on F100 GP2X
	BUTTON_SELECT		= 9,
	BUTTON_L			= 10,
	BUTTON_R			= 11,
	BUTTON_A			= 12,
	BUTTON_B			= 13,
	BUTTON_X			= 14,
	BUTTON_Y			= 15,
	BUTTON_VOLUP		= 16,
	BUTTON_VOLDOWN		= 17,
	BUTTON_CLICK		= 18
};

enum {
	/* Unused Joystick Buttons on the GP2X */
	BUTTON_HOME			= 51,
	BUTTON_HOLD			= 52,
	BUTTON_HELP			= 53,
	BUTTON_HELP2		= 54
};

enum {
	/* Touchscreen TapMode */
	TAPMODE_LEFT		= 0,
	TAPMODE_RIGHT		= 1,
	TAPMODE_HOVER		= 2
};

GP2XSdlEventSource::GP2XSdlEventSource()
	: _buttonStateL(false){

}

void GP2XSdlEventSource::SDLModToOSystemKeyFlags(SDLMod mod, Common::Event &event) {
	event.kbd.flags = 0;

	if (mod & KMOD_SHIFT)
		event.kbd.flags |= Common::KBD_SHIFT;
	if (mod & KMOD_ALT)
		event.kbd.flags |= Common::KBD_ALT;
	if (mod & KMOD_CTRL)
		event.kbd.flags |= Common::KBD_CTRL;

	// Sticky flags
	if (mod & KMOD_NUM)
		event.kbd.flags |= Common::KBD_NUM;
	if (mod & KMOD_CAPS)
		event.kbd.flags |= Common::KBD_CAPS;
}

void GP2XSdlEventSource::moveStick() {
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

/* GP2X Input mappings.
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

GP2X_BUTTON_CLICK           Left Mouse Click (GP2X only)
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
GP2X_BUTTON_L &	GP2X_BUTTON_Y				Toggles setZoomOnMouse() for larger then 320*240 games to scale to the point + raduis. (GP2X only)
*/

bool GP2XSdlEventSource::handleKeyDown(SDL_Event &ev, Common::Event &event) {
	SDLModToOSystemKeyFlags(SDL_GetModState(), event);

	if (remapKey(ev, event))
		return true;

	event.type = Common::EVENT_KEYDOWN;
	event.kbd.keycode = (Common::KeyCode)ev.key.keysym.sym;
	event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);

	return true;
}

bool GP2XSdlEventSource::handleJoyButtonDown(SDL_Event &ev, Common::Event &event) {

	_stickBtn[ev.jbutton.button] = 1;
	event.kbd.flags = 0;

	switch (ev.jbutton.button) {
	case BUTTON_UP:
	case BUTTON_UPLEFT:
	case BUTTON_LEFT:
	case BUTTON_DOWNLEFT:
	case BUTTON_DOWN:
	case BUTTON_DOWNRIGHT:
	case BUTTON_RIGHT:
	case BUTTON_UPRIGHT:
		moveStick();
		event.type = Common::EVENT_MOUSEMOVE;
		fillMouseEvent(event, _km.x, _km.y);
		break;
	case BUTTON_B:
	case BUTTON_CLICK:
		if (BUTTON_STATE_L == true) {
			((GP2XSdlGraphicsManager *)((OSystem_SDL *)g_system)->getGraphicsManager())->toggleZoomOnMouse();
			fillMouseEvent(event, _km.x, _km.y);
		} else {
			event.type = Common::EVENT_LBUTTONDOWN;
			fillMouseEvent(event, _km.x, _km.y);
		}
		break;
	case BUTTON_X:
		event.type = Common::EVENT_RBUTTONDOWN;
		fillMouseEvent(event, _km.x, _km.y);
		break;
	case BUTTON_L:
		BUTTON_STATE_L = true;
		break;
	case BUTTON_R:
		event.type = Common::EVENT_KEYDOWN;
		if (BUTTON_STATE_L == true) {
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
	case BUTTON_SELECT:
	case BUTTON_HOME:
		event.type = Common::EVENT_KEYDOWN;
		if (BUTTON_STATE_L == true) {
			event.type = Common::EVENT_QUIT;
		} else {
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
		}
		break;
	case BUTTON_A:
		event.type = Common::EVENT_KEYDOWN;
		if (BUTTON_STATE_L == true) {
			event.type = Common::EVENT_PREDICTIVE_DIALOG;
		} else {
		event.kbd.keycode = Common::KEYCODE_PERIOD;
		event.kbd.ascii = mapKey(SDLK_PERIOD, ev.key.keysym.mod, 0);
		}
		break;
	case BUTTON_Y:
		event.type = Common::EVENT_KEYDOWN;
		if (BUTTON_STATE_L == true) {
			GPH::ToggleTapMode();
			if (GPH::tapmodeLevel == TAPMODE_LEFT) {
				g_system->displayMessageOnOSD("Touchscreen 'Tap Mode' - Left Click");
			} else if (GPH::tapmodeLevel == TAPMODE_RIGHT) {
				g_system->displayMessageOnOSD("Touchscreen 'Tap Mode' - Right Click");
			} else if (GPH::tapmodeLevel == TAPMODE_HOVER) {
				g_system->displayMessageOnOSD("Touchscreen 'Tap Mode' - Hover (No Click)");
 			}
		} else {
			event.kbd.keycode = Common::KEYCODE_SPACE;
			event.kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
		}
		break;
	case BUTTON_MENU:
	case BUTTON_HELP:
		event.type = Common::EVENT_KEYDOWN;
		if (BUTTON_STATE_L == true) {
			event.type = Common::EVENT_MAINMENU;
		} else {
			event.kbd.keycode = Common::KEYCODE_F5;
			event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
		}
		break;
	case BUTTON_VOLUP:
		GP2X_HW::mixerMoveVolume(2);
		if (GP2X_HW::volumeLevel == 100) {
			g_system->displayMessageOnOSD("Maximum Volume");
		} else {
			g_system->displayMessageOnOSD("Increasing Volume");
		}
		break;

	case BUTTON_VOLDOWN:
		GP2X_HW::mixerMoveVolume(1);
		if (GP2X_HW::volumeLevel == 0) {
			g_system->displayMessageOnOSD("Minimal Volume");
		} else {
			g_system->displayMessageOnOSD("Decreasing Volume");
		}
		break;
	case BUTTON_HOLD:
		event.type = Common::EVENT_QUIT;
		break;
	case BUTTON_HELP2:
		GPH::ToggleTapMode();
		if (GPH::tapmodeLevel == TAPMODE_LEFT) {
			g_system->displayMessageOnOSD("Touchscreen 'Tap Mode': Left Click");
		} else if (GPH::tapmodeLevel == TAPMODE_RIGHT) {
			g_system->displayMessageOnOSD("Touchscreen 'Tap Mode': Right Click");
		} else if (GPH::tapmodeLevel == TAPMODE_HOVER) {
			g_system->displayMessageOnOSD("Touchscreen 'Tap Mode': Hover (No Click)");
		}
		break;
	}
	return true;
}

bool GP2XSdlEventSource::handleJoyButtonUp(SDL_Event &ev, Common::Event &event) {
	_stickBtn[ev.jbutton.button] = 0;
	event.kbd.flags = 0;

	switch (ev.jbutton.button) {
	case BUTTON_UP:
	case BUTTON_UPLEFT:
	case BUTTON_LEFT:
	case BUTTON_DOWNLEFT:
	case BUTTON_DOWN:
	case BUTTON_DOWNRIGHT:
	case BUTTON_RIGHT:
	case BUTTON_UPRIGHT:
		moveStick();
		event.type = Common::EVENT_MOUSEMOVE;
		fillMouseEvent(event, _km.x, _km.y);
		break;
	case BUTTON_B:
	case BUTTON_CLICK:
		if (BUTTON_STATE_L == true) {
			break;
		} else {
			event.type = Common::EVENT_LBUTTONUP;
			fillMouseEvent(event, _km.x, _km.y);
		}
		break;
	case BUTTON_X:
		event.type = Common::EVENT_RBUTTONUP;
		fillMouseEvent(event, _km.x, _km.y);
		break;
	case BUTTON_L:
		BUTTON_STATE_L = false;
		break;
	case BUTTON_SELECT:
	case BUTTON_HOME:
		event.type = Common::EVENT_KEYUP;
		event.kbd.keycode = Common::KEYCODE_ESCAPE;
		event.kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
		break;
	case BUTTON_A:
		event.type = Common::EVENT_KEYUP;
		event.kbd.keycode = Common::KEYCODE_PERIOD;
		event.kbd.ascii = mapKey(SDLK_PERIOD, ev.key.keysym.mod, 0);
		break;
	case BUTTON_Y:
		event.type = Common::EVENT_KEYUP;
		event.kbd.keycode = Common::KEYCODE_SPACE;
		event.kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
		break;
	case BUTTON_MENU:
	case BUTTON_HELP:
		event.type = Common::EVENT_KEYUP;
		if (BUTTON_STATE_L == true) {
			event.type = Common::EVENT_MAINMENU;
		} else {
			event.kbd.keycode = Common::KEYCODE_F5;
			event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
		}
		break;
	case BUTTON_R:
		event.type = Common::EVENT_KEYUP;
		if (BUTTON_STATE_L == true) {
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
	case BUTTON_VOLUP:
		break;
	case BUTTON_VOLDOWN:
		break;
	case BUTTON_HOLD:
		break;
	case BUTTON_HELP2:
		break;
	}
	return true;
}

bool GP2XSdlEventSource::handleJoyAxisMotion(SDL_Event &ev, Common::Event &event) {
	int axis = ev.jaxis.value;
	if ( axis > JOY_DEADZONE) {
		axis -= JOY_DEADZONE;
		event.type = Common::EVENT_MOUSEMOVE;
	} else if ( axis < -JOY_DEADZONE ) {
		axis += JOY_DEADZONE;
		event.type = Common::EVENT_MOUSEMOVE;
	} else
		axis = 0;

	if ( ev.jaxis.axis == JOY_XAXIS) {
#ifdef JOY_ANALOG
		_km.x_vel = axis/2000;
		_km.x_down_count = 0;
#else
		if (axis != 0) {
			_km.x_vel = (axis > 0) ? 1:-1;
			_km.x_down_count = 1;
		} else {
			_km.x_vel = 0;
			_km.x_down_count = 0;
		}
#endif

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
}

bool GP2XSdlEventSource::remapKey(SDL_Event &ev, Common::Event &event) {
	return false;
}


#endif
