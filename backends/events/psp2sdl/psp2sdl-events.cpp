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

#if defined(PSP2)

#include <psp2/kernel/processmgr.h>
#include "backends/platform/sdl/psp2/psp2.h"
#include "backends/events/psp2sdl/psp2sdl-events.h"
#include "backends/platform/sdl/sdl.h"
#include "engines/engine.h"

#include "common/util.h"
#include "common/events.h"
#include "common/config-manager.h"

#include "math.h"

#define JOY_DEADZONE 2000
#define JOY_ANALOG
#define JOY_XAXIS 0
#define JOY_YAXIS 1
#define JOY_XAXISR 2
#define JOY_YAXISR 3

enum {
	BTN_LEFT		= 7,
	BTN_DOWN		= 6,
	BTN_RIGHT		= 9,
	BTN_UP			= 8,

	BTN_START		= 11,
	BTN_SELECT		= 10,

	BTN_SQUARE		= 3,
	BTN_CROSS		= 2,
	BTN_CIRCLE		= 1,
	BTN_TRIANGLE	= 0,

	BTN_R1			= 5,
	BTN_L1			= 4
};

bool PSP2EventSource::handleJoyButtonDown(SDL_Event &ev, Common::Event &event) {

	event.kbd.flags = 0;

	switch (ev.jbutton.button) {
// Dpad
	case BTN_LEFT: // Left (+R_trigger: Up+Left)
		if (!_km.modifier) {
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = Common::KEYCODE_KP4;
			event.kbd.ascii = mapKey(SDLK_KP4, (SDLMod) ev.key.keysym.mod, 0);
		} else {
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = Common::KEYCODE_KP7;
			event.kbd.ascii = mapKey(SDLK_KP7, (SDLMod) ev.key.keysym.mod, 0);
		}
		break;
	case BTN_RIGHT: // Right (+R_trigger: Down+Right)
		if (!_km.modifier) {
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = Common::KEYCODE_KP6;
			event.kbd.ascii = mapKey(SDLK_KP6, (SDLMod) ev.key.keysym.mod, 0);
		} else {
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = Common::KEYCODE_KP3;
			event.kbd.ascii = mapKey(SDLK_KP3, (SDLMod) ev.key.keysym.mod, 0);
		}
		break;
	case BTN_UP: // Up (+R_trigger: Up+Right)
		if (!_km.modifier) {
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = Common::KEYCODE_KP8;
			event.kbd.ascii = mapKey(SDLK_KP8, (SDLMod) ev.key.keysym.mod, 0);
		} else {
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = Common::KEYCODE_KP9;
			event.kbd.ascii = mapKey(SDLK_KP9, (SDLMod) ev.key.keysym.mod, 0);
		}
		break;
	case BTN_DOWN: // Down (+R_trigger: Down+Left)
		if (!_km.modifier) {
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = Common::KEYCODE_KP2;
			event.kbd.ascii = mapKey(SDLK_KP2, (SDLMod) ev.key.keysym.mod, 0);
		} else {
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = Common::KEYCODE_KP1;
			event.kbd.ascii = mapKey(SDLK_KP1, (SDLMod) ev.key.keysym.mod, 0);
		}
		break;
// Buttons
	case BTN_CROSS: // Left mouse button
		event.type = Common::EVENT_LBUTTONDOWN;
		processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER);
		break;
	case BTN_CIRCLE: // Right mouse button
		event.type = Common::EVENT_RBUTTONDOWN;
		processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER);
		break;
	case BTN_TRIANGLE: // Escape (+R_trigger: Return)
		if (!_km.modifier) {
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = mapKey(SDLK_ESCAPE, (SDLMod) ev.key.keysym.mod, 0);
		} else {
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = Common::KEYCODE_RETURN;
			event.kbd.ascii = mapKey(SDLK_RETURN, (SDLMod) ev.key.keysym.mod, 0);
		}
		break;
	case BTN_SQUARE: // Period (+R_trigger: Space)
		if (!_km.modifier) {
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = Common::KEYCODE_PERIOD;
			event.kbd.ascii = mapKey(SDLK_PERIOD, (SDLMod) ev.key.keysym.mod, 0);
		} else {
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = Common::KEYCODE_SPACE;
			event.kbd.ascii = mapKey(SDLK_SPACE, (SDLMod) ev.key.keysym.mod, 0);
		}
		break;
	case BTN_L1: // Game menu
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_F5;
		event.kbd.ascii = mapKey(SDLK_F5, (SDLMod) ev.key.keysym.mod, 0);
		break;
	case BTN_R1: // Modifier + Shift
		_km.modifier=true; // slow mouse
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_INVALID;
		event.kbd.ascii = 0;
		event.kbd.flags = Common::KBD_SHIFT;
		break;
	case BTN_START: // ScummVM in game menu
		event.type = Common::EVENT_MAINMENU;
		break;
	case BTN_SELECT: // Virtual keyboard (+R_trigger: Predictive Input Dialog)
		if (!_km.modifier) {
#ifdef ENABLE_VKEYBD
			event.type = Common::EVENT_VIRTUAL_KEYBOARD;
#endif
		} else {
			event.type = Common::EVENT_PREDICTIVE_DIALOG;
		}
		break;
	}
	return true;
}

bool PSP2EventSource::handleJoyButtonUp(SDL_Event &ev, Common::Event &event) {

	event.kbd.flags = 0;

	switch (ev.jbutton.button) {
// Dpad
	case BTN_LEFT: // Left (+R_trigger: Up+Left)
		if (!_km.modifier) {
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = Common::KEYCODE_KP4;
			event.kbd.ascii = mapKey(SDLK_KP4, (SDLMod) ev.key.keysym.mod, 0);
		} else {
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = Common::KEYCODE_KP7;
			event.kbd.ascii = mapKey(SDLK_KP7, (SDLMod) ev.key.keysym.mod, 0);
		}
		break;
	case BTN_RIGHT: // Right (+R_trigger: Down+Right)
		if (!_km.modifier) {
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = Common::KEYCODE_KP6;
			event.kbd.ascii = mapKey(SDLK_KP6, (SDLMod) ev.key.keysym.mod, 0);
		} else {
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = Common::KEYCODE_KP3;
			event.kbd.ascii = mapKey(SDLK_KP3, (SDLMod) ev.key.keysym.mod, 0);
		}
		break;
	case BTN_UP: // Up (+R_trigger: Up+Right)
		if (!_km.modifier) {
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = Common::KEYCODE_KP8;
			event.kbd.ascii = mapKey(SDLK_KP8, (SDLMod) ev.key.keysym.mod, 0);
		} else {
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = Common::KEYCODE_KP9;
			event.kbd.ascii = mapKey(SDLK_KP9, (SDLMod) ev.key.keysym.mod, 0);
		}
		break;
	case BTN_DOWN: // Down (+R_trigger: Down+Left)
		if (!_km.modifier) {
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = Common::KEYCODE_KP2;
			event.kbd.ascii = mapKey(SDLK_KP2, (SDLMod) ev.key.keysym.mod, 0);
		} else {
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = Common::KEYCODE_KP1;
			event.kbd.ascii = mapKey(SDLK_KP1, (SDLMod) ev.key.keysym.mod, 0);
		}
		break;
// Buttons
	case BTN_CROSS: // Left mouse button
		event.type = Common::EVENT_LBUTTONUP;
		processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER);
		break;
	case BTN_CIRCLE: // Right mouse button
		event.type = Common::EVENT_RBUTTONUP;
		processMouseEvent(event, _km.x / MULTIPLIER, _km.y / MULTIPLIER);
		break;
	case BTN_TRIANGLE: // Escape (+R_trigger: Return)
		if (!_km.modifier) {
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = mapKey(SDLK_ESCAPE, (SDLMod) ev.key.keysym.mod, 0);
		} else {
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = Common::KEYCODE_RETURN;
			event.kbd.ascii = mapKey(SDLK_RETURN, (SDLMod) ev.key.keysym.mod, 0);
		}
		break;
	case BTN_SQUARE: // Period (+R_trigger: Space)
		if (!_km.modifier) {
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = Common::KEYCODE_PERIOD;
			event.kbd.ascii = mapKey(SDLK_PERIOD, (SDLMod) ev.key.keysym.mod, 0);
		} else {
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = Common::KEYCODE_SPACE;
			event.kbd.ascii = mapKey(SDLK_SPACE, (SDLMod) ev.key.keysym.mod, 0);
		}
		break;
	case BTN_L1: // Game menu
		event.type = Common::EVENT_KEYUP;
		event.kbd.keycode = Common::KEYCODE_F5;
		event.kbd.ascii = mapKey(SDLK_F5, (SDLMod) ev.key.keysym.mod, 0);
		break;
	case BTN_R1: // Modifier + SHIFT Key
		_km.modifier = false; // slow mouse
		event.type = Common::EVENT_KEYUP;
		event.kbd.keycode = Common::KEYCODE_INVALID;
		event.kbd.ascii = 0;
		event.kbd.flags = 0;
		break;
	case BTN_START: // ScummVM in game menu
		// Handled in key down
		break;
	case BTN_SELECT: // Virtual keyboard (+R_trigger: Predictive Input Dialog)
		// Handled in key down
		break;
	}
	return true;
}

bool PSP2EventSource::handleJoyAxisMotion(SDL_Event &ev, Common::Event &event) {

	int axis = ev.jaxis.value;
	
	// conversion factor between keyboard mouse and joy axis value
	int vel_to_axis = (1500 / MULTIPLIER);

	if (ev.jaxis.axis == JOY_XAXIS) {
		_km.joy_x = axis;
	} else if (ev.jaxis.axis == JOY_YAXIS) {
		axis = -axis;
		_km.joy_y = -axis;
	}
	
	// radial and scaled deadzone

	float analogX = (float)_km.joy_x;
	float analogY = (float)_km.joy_y;
	float deadZone = (float)JOY_DEADZONE;
	if (g_system->hasFeature(OSystem::kFeatureJoystickDeadzone))
		deadZone = (float)ConfMan.getInt("joystick_deadzone") * 1000.0f;
	float scalingFactor = 1.0f;
	float magnitude = 0.0f;

	magnitude = sqrt(analogX * analogX + analogY * analogY);

	if (magnitude >= deadZone) {
		_km.x_down_count = 0;
		_km.y_down_count = 0;
		scalingFactor = 1.0f / magnitude * (magnitude - deadZone) / (32769.0f - deadZone);
		_km.x_vel = (int16)(analogX * scalingFactor * 32768.0f / (float) vel_to_axis);
		_km.y_vel = (int16)(analogY * scalingFactor * 32768.0f / (float) vel_to_axis);
	} else {
		_km.x_vel = 0;
		_km.y_vel = 0;
	}

	return false;
}

void PSP2EventSource::preprocessEvents(SDL_Event *event) {

	// prevent suspend (scummvm games contains a lot of cutscenes..)
	sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
	sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_OLED_OFF);
}

#endif
