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

#define JOY_DEADZONE 3200
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
	case BTN_CROSS: // Left mouse button
		event.type = Common::EVENT_LBUTTONDOWN;
		processMouseEvent(event, _km.x, _km.y);
		break;
	case BTN_CIRCLE: // Right mouse button
		event.type = Common::EVENT_RBUTTONDOWN;
		processMouseEvent(event, _km.x, _km.y);
		break;
	case BTN_TRIANGLE: // Game menu
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_F5;
		event.kbd.ascii = mapKey(SDLK_F5, (SDLMod) ev.key.keysym.mod, 0);
		break;
	case BTN_SELECT: // Virtual keyboard
#ifdef ENABLE_VKEYBD
		event.type = Common::EVENT_VIRTUAL_KEYBOARD;
#endif
		break;
	case BTN_SQUARE: // Escape
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_ESCAPE;
		event.kbd.ascii = mapKey(SDLK_ESCAPE, (SDLMod) ev.key.keysym.mod, 0);
		break;
	case BTN_L1: // Predictive input dialog
		event.type = Common::EVENT_PREDICTIVE_DIALOG;
		break;
	case BTN_START: // ScummVM in game menu
		event.type = Common::EVENT_MAINMENU;
		break;
	}
	return true;
}

bool PSP2EventSource::handleJoyButtonUp(SDL_Event &ev, Common::Event &event) {

	event.kbd.flags = 0;

	switch (ev.jbutton.button) {
	case BTN_CROSS: // Left mouse button
		event.type = Common::EVENT_LBUTTONUP;
		processMouseEvent(event, _km.x, _km.y);
		break;
	case BTN_CIRCLE: // Right mouse button
		event.type = Common::EVENT_RBUTTONUP;
		processMouseEvent(event, _km.x, _km.y);
		break;
	case BTN_TRIANGLE: // Game menu
		event.type = Common::EVENT_KEYUP;
		event.kbd.keycode = Common::KEYCODE_F5;
		event.kbd.ascii = mapKey(SDLK_F5, (SDLMod) ev.key.keysym.mod, 0);
		break;
	case BTN_SELECT: // Virtual keyboard
		// Handled in key down
		break;
	case BTN_SQUARE: // Escape
		event.type = Common::EVENT_KEYUP;
		event.kbd.keycode = Common::KEYCODE_ESCAPE;
		event.kbd.ascii = mapKey(SDLK_ESCAPE, (SDLMod) ev.key.keysym.mod, 0);
		break;
	}
	return true;
}

bool PSP2EventSource::handleJoyAxisMotion(SDL_Event &ev, Common::Event &event) {

	int axis = ev.jaxis.value;
	
	if (axis > JOY_DEADZONE) {
		axis -= JOY_DEADZONE;
		event.type = Common::EVENT_MOUSEMOVE;
	} else if (axis < -JOY_DEADZONE) {
		axis += JOY_DEADZONE;
		event.type = Common::EVENT_MOUSEMOVE;
	} else
		axis = 0;

	if (ev.jaxis.axis == JOY_XAXIS) {
		_km.x_vel = axis / 2000;
		_km.x_down_count = 0;
	} else if (ev.jaxis.axis == JOY_YAXIS) {
		axis = -axis;
		_km.y_vel = -axis / 2000;
		_km.y_down_count = 0;
	}

	processMouseEvent(event, _km.x, _km.y);

	return true;
}

void PSP2EventSource::preprocessEvents(SDL_Event *event) {
	
	// prevent suspend (scummvm games contains a lot of cutscenes..)
	sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
	sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_OLED_OFF);
}

#endif
