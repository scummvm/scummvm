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
 * GPH: Device Specific Event Handling.
 *
 */

#include "backends/platform/gp2xwiz/gp2xwiz-sdl.h"
#include "backends/platform/gp2xwiz/gp2xwiz-hw.h"
#include "graphics/scaler/aspect.h"

#include "common/util.h"
#include "common/events.h"

#define JOY_DEADZONE 2200

#define JOY_XAXIS 0
#define JOY_YAXIS 1

/* Quick default button states for modifiers. */
int BUTTON_STATE_L					=	false;

#if defined(CAANOO)

	/* Caanoo: Main Joystick Button Mappings */
	/* 		   The Caanoo has an analogue stick so no digital DPAD */
	enum {
		/* Joystick Buttons */
		BUTTON_A			= 0,
		BUTTON_X			= 1,
		BUTTON_B			= 2,
		BUTTON_Y			= 3,
		BUTTON_L			= 4,
		BUTTON_R			= 5,
		BUTTON_HOME			= 6,	// Home
		BUTTON_HOLD			= 7,	// Hold (on Power)
		BUTTON_HELP			= 8,	// Help I
		BUTTON_HELP2		= 9,	// Help II
		BUTTON_CLICK		= 10	// Stick Click
	};

	enum {
		/* Unused Joystick Buttons on the Caanoo */
		BUTTON_VOLUP		= 51,
		BUTTON_VOLDOWN		= 52,
		BUTTON_UP			= 53,
		BUTTON_UPLEFT		= 54,
		BUTTON_LEFT			= 55,
		BUTTON_DOWNLEFT		= 56,
		BUTTON_DOWN			= 57,
		BUTTON_DOWNRIGHT	= 58,
		BUTTON_RIGHT		= 59,
		BUTTON_UPRIGHT		= 60,
		BUTTON_MENU			= 61,
		BUTTON_SELECT		= 62
	};

#else

	/* Wiz: Main Joystick Mappings */
	enum {
		/* DPAD */
		BUTTON_UP			= 0,
		BUTTON_UPLEFT		= 1,
		BUTTON_LEFT			= 2,
		BUTTON_DOWNLEFT		= 3,
		BUTTON_DOWN			= 4,
		BUTTON_DOWNRIGHT	= 5,
		BUTTON_RIGHT		= 6,
		BUTTON_UPRIGHT		= 7,
		/* Joystick Buttons */
		BUTTON_MENU			= 8,
		BUTTON_SELECT		= 9,
		BUTTON_L			= 10,
		BUTTON_R			= 11,
		BUTTON_A			= 12,
		BUTTON_B			= 13,
		BUTTON_X			= 14,
		BUTTON_Y			= 15,
		BUTTON_VOLUP		= 16,
		BUTTON_VOLDOWN		= 17
	};

	enum {
		/* Unused Joystick Buttons on the Wiz */
		BUTTON_HOME			= 51,
		BUTTON_HOLD			= 52,
		BUTTON_CLICK		= 53,
		BUTTON_HELP			= 54,
		BUTTON_HELP2		= 55
	};

#endif

enum {
	/* Touchscreen TapMode */
	TAPMODE_LEFT		= 0,
	TAPMODE_RIGHT		= 1,
	TAPMODE_HOVER		= 2
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


void OSystem_GPH::fillMouseEvent(Common::Event &event, int x, int y) {
	if (_videoMode.mode == GFX_HALF && !_overlayVisible){
		event.mouse.x = x*2;
		event.mouse.y = y*2;
	} else {
		event.mouse.x = x;
		event.mouse.y = y;
	}

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


void OSystem_GPH::moveStick() {
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

/* Custom handleMouseButtonDown/handleMouseButtonUp to deal with 'Tap Mode' for the touchscreen */

bool OSystem_GPH::handleMouseButtonDown(SDL_Event &ev, Common::Event &event) {
	if (ev.button.button == SDL_BUTTON_LEFT){
		if (BUTTON_STATE_L == true) /* BUTTON_STATE_L = Left Trigger Held, force Right Click */
			event.type = Common::EVENT_RBUTTONDOWN;
		else if (GPH::tapmodeLevel == TAPMODE_LEFT) /* TAPMODE_LEFT = Left Click Tap Mode */
			event.type = Common::EVENT_LBUTTONDOWN;
		else if (GPH::tapmodeLevel == TAPMODE_RIGHT) /* TAPMODE_RIGHT = Right Click Tap Mode */
			event.type = Common::EVENT_RBUTTONDOWN;
		else if (GPH::tapmodeLevel == TAPMODE_HOVER) /* TAPMODE_HOVER = Hover (No Click) Tap Mode */
			event.type = Common::EVENT_MOUSEMOVE;
		else
			event.type = Common::EVENT_LBUTTONDOWN; /* For normal mice etc. */
	}
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

	fillMouseEvent(event, ev.button.x, ev.button.y);

	return true;
}

bool OSystem_GPH::handleMouseButtonUp(SDL_Event &ev, Common::Event &event) {
	if (ev.button.button == SDL_BUTTON_LEFT){
		if (BUTTON_STATE_L == true) /* BUTTON_STATE_L = Left Trigger Held, force Right Click */
			event.type = Common::EVENT_RBUTTONUP;
		else if (GPH::tapmodeLevel == TAPMODE_LEFT) /* TAPMODE_LEFT = Left Click Tap Mode */
			event.type = Common::EVENT_LBUTTONUP;
		else if (GPH::tapmodeLevel == TAPMODE_RIGHT) /* TAPMODE_RIGHT = Right Click Tap Mode */
			event.type = Common::EVENT_RBUTTONUP;
		else if (GPH::tapmodeLevel == TAPMODE_HOVER) /* TAPMODE_HOVER = Hover (No Click) Tap Mode */
			event.type = Common::EVENT_MOUSEMOVE;
		else
			event.type = Common::EVENT_LBUTTONUP; /* For normal mice etc. */

	}
	else if (ev.button.button == SDL_BUTTON_RIGHT)
		event.type = Common::EVENT_RBUTTONUP;
#if defined(SDL_BUTTON_MIDDLE)
	else if (ev.button.button == SDL_BUTTON_MIDDLE)
		event.type = Common::EVENT_MBUTTONUP;
#endif
	else
		return false;

	fillMouseEvent(event, ev.button.x, ev.button.y);

	return true;
}

/* Custom handleJoyButtonDown/handleJoyButtonUp to deal with the joystick buttons on GPH devices */

bool OSystem_GPH::handleJoyButtonDown(SDL_Event &ev, Common::Event &event) {
			_stickBtn[ev.jbutton.button] = 1;
			if (ev.jbutton.button == BUTTON_B) {
				event.type = Common::EVENT_LBUTTONDOWN;
				fillMouseEvent(event, _km.x, _km.y);
			} else if (ev.jbutton.button == BUTTON_CLICK) {
				event.type = Common::EVENT_LBUTTONDOWN;
				fillMouseEvent(event, _km.x, _km.y);
			} else if (ev.jbutton.button == BUTTON_X) {
				event.type = Common::EVENT_RBUTTONDOWN;
				fillMouseEvent(event, _km.x, _km.y);
			} else if (_stickBtn[BUTTON_L] && (ev.jbutton.button == BUTTON_SELECT)) {
				event.type = Common::EVENT_QUIT;
#if !defined (CAANOO)
			} else if (ev.jbutton.button < 8) {
				moveStick();
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);
#endif
			} else {
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.flags = 0;
				switch (ev.jbutton.button) {
					case BUTTON_L:
						BUTTON_STATE_L = true;
						break;
					case BUTTON_R:
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
					case BUTTON_HELP2:
						if (BUTTON_STATE_L == true) {
							event.type = Common::EVENT_QUIT;
						} else {
							event.kbd.keycode = Common::KEYCODE_ESCAPE;
							event.kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
						}
						break;
					case BUTTON_A:
						if (BUTTON_STATE_L == true) {
							event.type = Common::EVENT_PREDICTIVE_DIALOG;
						} else {
						event.kbd.keycode = Common::KEYCODE_PERIOD;
						event.kbd.ascii = mapKey(SDLK_PERIOD, ev.key.keysym.mod, 0);
						}
						break;
					case BUTTON_Y:
						if (BUTTON_STATE_L == true) {
							GPH::ToggleTapMode();
							if (GPH::tapmodeLevel == TAPMODE_LEFT) {
								displayMessageOnOSD("Touchscreen 'Tap Mode': Left Click");
							} else if (GPH::tapmodeLevel == TAPMODE_RIGHT) {
								displayMessageOnOSD("Touchscreen 'Tap Mode': Right Click");
							} else if (GPH::tapmodeLevel == TAPMODE_HOVER) {
							displayMessageOnOSD("Touchscreen 'Tap Mode': Hover (No Click)");
							}
						} else {
							event.kbd.keycode = Common::KEYCODE_SPACE;
							event.kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
						}
						break;
					case BUTTON_MENU:
					case BUTTON_HELP:
						if (BUTTON_STATE_L == true) {
							event.type = Common::EVENT_MAINMENU;
						} else {
							event.kbd.keycode = Common::KEYCODE_F5;
							event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
						}
						break;
					case BUTTON_VOLUP:
						WIZ_HW::mixerMoveVolume(2);
						if (WIZ_HW::volumeLevel == 100) {
							displayMessageOnOSD("Maximum Volume");
						} else {
							displayMessageOnOSD("Increasing Volume");
						}
						break;
					case BUTTON_VOLDOWN:
						WIZ_HW::mixerMoveVolume(1);
						if (WIZ_HW::volumeLevel == 0) {
							displayMessageOnOSD("Minimal Volume");
						} else {
							displayMessageOnOSD("Decreasing Volume");
						}
						break;
					case BUTTON_HOLD:
						event.type = Common::EVENT_QUIT;
 						break;
					case BUTTON_HOME:
						GPH::ToggleTapMode();
						if (GPH::tapmodeLevel == TAPMODE_LEFT) {
							displayMessageOnOSD("Touchscreen 'Tap Mode': Left Click");
						} else if (GPH::tapmodeLevel == TAPMODE_RIGHT) {
							displayMessageOnOSD("Touchscreen 'Tap Mode': Right Click");
						} else if (GPH::tapmodeLevel == TAPMODE_HOVER) {
							displayMessageOnOSD("Touchscreen 'Tap Mode': Hover (No Click)");
						}
 						break;
				}
			}
			return true;
}

bool OSystem_GPH::handleJoyButtonUp(SDL_Event &ev, Common::Event &event) {
			_stickBtn[ev.jbutton.button] = 0;
			if (ev.jbutton.button == BUTTON_B) {
				event.type = Common::EVENT_LBUTTONUP;
				fillMouseEvent(event, _km.x, _km.y);
			} else if (ev.jbutton.button == BUTTON_CLICK) {
				event.type = Common::EVENT_LBUTTONUP;
				fillMouseEvent(event, _km.x, _km.y);
			} else if (ev.jbutton.button == BUTTON_X) {
				event.type = Common::EVENT_RBUTTONUP;
				fillMouseEvent(event, _km.x, _km.y);
#if !defined (CAANOO)
			} else if (ev.jbutton.button < 8) {
				moveStick();
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);
#endif
			} else {
				event.type = Common::EVENT_KEYUP;
				event.kbd.flags = 0;
				switch (ev.jbutton.button) {
					case BUTTON_SELECT:
					case BUTTON_HELP2:
						event.kbd.keycode = Common::KEYCODE_ESCAPE;
						event.kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
						break;
					case BUTTON_A:
						event.kbd.keycode = Common::KEYCODE_PERIOD;
						event.kbd.ascii = mapKey(SDLK_PERIOD, ev.key.keysym.mod, 0);
						break;
					case BUTTON_Y:
						event.kbd.keycode = Common::KEYCODE_SPACE;
						event.kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
						break;
					case BUTTON_MENU:
					case BUTTON_HELP:
						if (BUTTON_STATE_L == true) {
							event.type = Common::EVENT_MAINMENU;
						} else {
							event.kbd.keycode = Common::KEYCODE_F5;
							event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
						}
						break;
					case BUTTON_L:
						BUTTON_STATE_L = false;
						break;
					case BUTTON_R:
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
					case BUTTON_HOME:
						break;
				}
			}
			return true;
}

bool OSystem_GPH::remapKey(SDL_Event &ev,Common::Event &event) {
	return false;
}
