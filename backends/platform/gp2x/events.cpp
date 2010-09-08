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
#include "common/util.h"
#include "common/events.h"
#include "graphics/scaler/aspect.h"	// for aspect2Real

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

static void SDLModToOSystemKeyFlags(SDLMod mod, Common::Event &event) {

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

bool OSystem_GP2X::pollEvent(Common::Event &event) {
	SDL_Event ev;
	ev.type = SDL_NOEVENT;

	handleKbdMouse();

	// If the screen mode changed, send an Common::EVENT_SCREEN_CHANGED
	if (_modeChanged) {
		_modeChanged = false;
		event.type = Common::EVENT_SCREEN_CHANGED;
		return true;
	}

	while (SDL_PollEvent(&ev)) {
		preprocessEvents(&ev);
		if (dispatchSDLEvent(ev, event))
			return true;
	}
	return false;
}

bool OSystem_GP2X::dispatchSDLEvent(SDL_Event &ev, Common::Event &event) {
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
	case SDL_JOYBUTTONDOWN:
		return handleJoyButtonDown(ev, event);
	case SDL_JOYBUTTONUP:
		return handleJoyButtonUp(ev, event);
	case SDL_JOYAXISMOTION:
		return handleJoyAxisMotion(ev, event);

	case SDL_VIDEOEXPOSE:
		_forceFull = true;
		break;

	case SDL_QUIT:
		event.type = Common::EVENT_QUIT;
		return true;

	}

	return false;
}

bool OSystem_GP2X::handleKeyDown(SDL_Event &ev, Common::Event &event) {

	SDLModToOSystemKeyFlags(SDL_GetModState(), event);

	// Handle scroll lock as a key modifier
	if (ev.key.keysym.sym == SDLK_SCROLLOCK)
		_scrollLock = !_scrollLock;

	if (_scrollLock)
		event.kbd.flags |= Common::KBD_SCRL;

	// Alt-Return and Alt-Enter toggle full screen mode
	if (event.kbd.hasFlags(Common::KBD_ALT) && (ev.key.keysym.sym == SDLK_RETURN || ev.key.keysym.sym == SDLK_KP_ENTER)) {
		beginGFXTransaction();
			setFullscreenMode(!_videoMode.fullscreen);
		endGFXTransaction();
#ifdef USE_OSD
		if (_videoMode.fullscreen)
			displayMessageOnOSD("Fullscreen mode");
		else
			displayMessageOnOSD("Windowed mode");
#endif

		return false;
	}

	// Alt-S: Create a screenshot
	if (event.kbd.hasFlags(Common::KBD_ALT) && ev.key.keysym.sym == 's') {
		char filename[20];

		for (int n = 0;; n++) {
			SDL_RWops *file;

			sprintf(filename, "scummvm%05d.bmp", n);
			file = SDL_RWFromFile(filename, "r");
			if (!file)
				break;
			SDL_RWclose(file);
		}
		if (saveScreenshot(filename))
			printf("Saved '%s'\n", filename);
		else
			printf("Could not save screenshot!\n");
		return false;
	}

	// Ctrl-m toggles mouse capture
	if (event.kbd.hasFlags(Common::KBD_CTRL) && ev.key.keysym.sym == 'm') {
		toggleMouseGrab();
		return false;
	}

	if ((ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.sym == 'q') {
		event.type = Common::EVENT_QUIT;
		return true;
	}

	if ((ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.sym == 'u') {
		event.type = Common::EVENT_MUTE;
		return true;
	}

	// Ctrl-Alt-<key> will change the GFX mode
	if ((event.kbd.flags & (Common::KBD_CTRL|Common::KBD_ALT)) == (Common::KBD_CTRL|Common::KBD_ALT)) {
		if (handleScalerHotkeys(ev.key))
			return false;
	}

	if (remapKey(ev, event))
		return true;

	event.type = Common::EVENT_KEYDOWN;
	event.kbd.keycode = (Common::KeyCode)ev.key.keysym.sym;
	event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);

	return true;
}

bool OSystem_GP2X::handleKeyUp(SDL_Event &ev, Common::Event &event) {
	if (remapKey(ev, event))
		return true;

	event.type = Common::EVENT_KEYUP;
	event.kbd.keycode = (Common::KeyCode)ev.key.keysym.sym;
	event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);

	// Ctrl-Alt-<key> will change the GFX mode
	SDLModToOSystemKeyFlags(SDL_GetModState(), event);

	// Set the scroll lock sticky flag
	if (_scrollLock)
		event.kbd.flags |= Common::KBD_SCRL;

	if (isScalerHotkey(event))
		// Swallow these key up events
		return false;

	return true;
}

bool OSystem_GP2X::handleMouseMotion(SDL_Event &ev, Common::Event &event) {
	event.type = Common::EVENT_MOUSEMOVE;
	fillMouseEvent(event, ev.motion.x, ev.motion.y);

	setMousePos(event.mouse.x, event.mouse.y);
	return true;
}

/* Custom handleMouseButtonDown/handleMouseButtonUp to deal with 'Tap Mode' for the touchscreen */

bool OSystem_GP2X::handleMouseButtonDown(SDL_Event &ev, Common::Event &event) {
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

bool OSystem_GP2X::handleMouseButtonUp(SDL_Event &ev, Common::Event &event) {
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

bool OSystem_GP2X::handleJoyButtonDown(SDL_Event &ev, Common::Event &event) {

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
			setZoomOnMouse();
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
				displayMessageOnOSD("Touchscreen 'Tap Mode' - Left Click");
			} else if (GPH::tapmodeLevel == TAPMODE_RIGHT) {
				displayMessageOnOSD("Touchscreen 'Tap Mode' - Right Click");
			} else if (GPH::tapmodeLevel == TAPMODE_HOVER) {
				displayMessageOnOSD("Touchscreen 'Tap Mode' - Hover (No Click)");
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
			displayMessageOnOSD("Maximum Volume");
		} else {
			displayMessageOnOSD("Increasing Volume");
		}
		break;

	case BUTTON_VOLDOWN:
		GP2X_HW::mixerMoveVolume(1);
		if (GP2X_HW::volumeLevel == 0) {
			displayMessageOnOSD("Minimal Volume");
		} else {
			displayMessageOnOSD("Decreasing Volume");
		}
		break;
	case BUTTON_HOLD:
		event.type = Common::EVENT_QUIT;
		break;
	case BUTTON_HELP2:
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
	return true;
}

bool OSystem_GP2X::handleJoyButtonUp(SDL_Event &ev, Common::Event &event) {
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

bool OSystem_GP2X::handleJoyAxisMotion(SDL_Event &ev, Common::Event &event) {
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


bool OSystem_GP2X::remapKey(SDL_Event &ev, Common::Event &event) {
	return false;
}
