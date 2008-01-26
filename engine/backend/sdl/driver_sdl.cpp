/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "debug.h"
#include "driver_sdl.h"

// NOTE: This is not a complete driver, it needs to be subclassed
//       to provide rendering functionality.


// Fake SDLK_* values for joystick and mouse events
enum {
	SDLK_JOY1_B1 = SDLK_LAST,
	SDLK_JOY1_B2,
	SDLK_JOY1_B3,
	SDLK_JOY1_B4,
	SDLK_JOY1_B5,
	SDLK_JOY1_B6,
	SDLK_JOY1_B7,
	SDLK_JOY1_B8,
	SDLK_JOY1_B9,
	SDLK_JOY1_B10,
	SDLK_JOY1_HLEFT,
	SDLK_JOY1_HUP,
	SDLK_JOY1_HRIGHT,
	SDLK_JOY1_HDOWN,
	SDLK_JOY2_B1,
	SDLK_JOY2_B2,
	SDLK_JOY2_B3,
	SDLK_JOY2_B4,
	SDLK_JOY2_B5,
	SDLK_JOY2_B6,
	SDLK_JOY2_B7,
	SDLK_JOY2_B8,
	SDLK_JOY2_B9,
	SDLK_JOY2_B10,
	SDLK_JOY2_HLEFT,
	SDLK_JOY2_HUP,
	SDLK_JOY2_HRIGHT,
	SDLK_JOY2_HDOWN,
	SDLK_MOUSE_B1,
	SDLK_MOUSE_B2,
	SDLK_MOUSE_B3,
	SDLK_MOUSE_B4,
	SDLK_AXIS_JOY1_X,
	SDLK_AXIS_JOY1_Y,
	SDLK_AXIS_JOY1_Z,
	SDLK_AXIS_JOY1_R,
	SDLK_AXIS_JOY1_U,
	SDLK_AXIS_JOY1_V,
	SDLK_AXIS_JOY2_X,
	SDLK_AXIS_JOY2_Y,
	SDLK_AXIS_JOY2_Z,
	SDLK_AXIS_JOY2_R,
	SDLK_AXIS_JOY2_U,
	SDLK_AXIS_JOY2_V,
	SDLK_AXIS_MOUSE_X,
	SDLK_AXIS_MOUSE_Y,
	SDLK_AXIS_MOUSE_Z,
	SDLK_EXTRA_LAST
};

// Entries in the system.controls table

static const Driver::ControlDescriptor controls[] = {
	{ "KEY_ESCAPE", SDLK_ESCAPE },
	{ "KEY_1", SDLK_1 },
	{ "KEY_2", SDLK_2 },
	{ "KEY_3", SDLK_3 },
	{ "KEY_4", SDLK_4 },
	{ "KEY_5", SDLK_5 },
	{ "KEY_6", SDLK_6 },
	{ "KEY_7", SDLK_7 },
	{ "KEY_8", SDLK_8 },
	{ "KEY_9", SDLK_9 },
	{ "KEY_0", SDLK_0 },
	{ "KEY_MINUS", SDLK_MINUS },
	{ "KEY_EQUALS", SDLK_EQUALS },
	{ "KEY_BACK", SDLK_BACKSPACE },
	{ "KEY_TAB", SDLK_TAB },
	{ "KEY_Q", SDLK_q },
	{ "KEY_W", SDLK_w },
	{ "KEY_E", SDLK_e },
	{ "KEY_R", SDLK_r },
	{ "KEY_T", SDLK_t },
	{ "KEY_Y", SDLK_y },
	{ "KEY_U", SDLK_u },
	{ "KEY_I", SDLK_i },
	{ "KEY_O", SDLK_o },
	{ "KEY_P", SDLK_p },
	{ "KEY_LBRACKET", SDLK_LEFTBRACKET },
	{ "KEY_RBRACKET", SDLK_RIGHTBRACKET },
	{ "KEY_RETURN", SDLK_RETURN },
	{ "KEY_LCONTROL", SDLK_LCTRL },
	{ "KEY_A", SDLK_a },
	{ "KEY_S", SDLK_s },
	{ "KEY_D", SDLK_d },
	{ "KEY_F", SDLK_f },
	{ "KEY_G", SDLK_g },
	{ "KEY_H", SDLK_h },
	{ "KEY_J", SDLK_j },
	{ "KEY_K", SDLK_k },
	{ "KEY_L", SDLK_l },
	{ "KEY_SEMICOLON", SDLK_SEMICOLON },
	{ "KEY_APOSTROPHE", SDLK_QUOTE },
	{ "KEY_GRAVE", SDLK_BACKQUOTE },
	{ "KEY_LSHIFT", SDLK_LSHIFT },
	{ "KEY_BACKSLASH", SDLK_BACKSLASH },
	{ "KEY_Z", SDLK_z },
	{ "KEY_X", SDLK_x },
	{ "KEY_C", SDLK_c },
	{ "KEY_V", SDLK_v },
	{ "KEY_B", SDLK_b },
	{ "KEY_N", SDLK_n },
	{ "KEY_M", SDLK_m },
	{ "KEY_COMMA", SDLK_COMMA },
	{ "KEY_PERIOD", SDLK_PERIOD },
	{ "KEY_SLASH", SDLK_SLASH },
	{ "KEY_RSHIFT", SDLK_RSHIFT },
	{ "KEY_MULTIPLY", SDLK_KP_MULTIPLY },
	{ "KEY_LMENU", SDLK_LALT },
	{ "KEY_SPACE", SDLK_SPACE },
	{ "KEY_CAPITAL", SDLK_CAPSLOCK },
	{ "KEY_F1", SDLK_F1 },
	{ "KEY_F2", SDLK_F2 },
	{ "KEY_F3", SDLK_F3 },
	{ "KEY_F4", SDLK_F4 },
	{ "KEY_F5", SDLK_F5 },
	{ "KEY_F6", SDLK_F6 },
	{ "KEY_F7", SDLK_F7 },
	{ "KEY_F8", SDLK_F8 },
	{ "KEY_F9", SDLK_F9 },
	{ "KEY_F10", SDLK_F10 },
	{ "KEY_NUMLOCK", SDLK_NUMLOCK },
	{ "KEY_SCROLL", SDLK_SCROLLOCK },
	{ "KEY_NUMPAD7", SDLK_KP7 },
	{ "KEY_NUMPAD8", SDLK_KP8 },
	{ "KEY_NUMPAD9", SDLK_KP9 },
	{ "KEY_SUBTRACT", SDLK_KP_MINUS },
	{ "KEY_NUMPAD4", SDLK_KP4 },
	{ "KEY_NUMPAD5", SDLK_KP5 },
	{ "KEY_NUMPAD6", SDLK_KP6 },
	{ "KEY_ADD", SDLK_KP_PLUS },
	{ "KEY_NUMPAD1", SDLK_KP1 },
	{ "KEY_NUMPAD2", SDLK_KP2 },
	{ "KEY_NUMPAD3", SDLK_KP3 },
	{ "KEY_NUMPAD0", SDLK_KP0 },
	{ "KEY_DECIMAL", SDLK_KP_PERIOD },
	{ "KEY_F11", SDLK_F11 },
	{ "KEY_F12", SDLK_F12 },
	{ "KEY_F13", SDLK_F13 },
	{ "KEY_F14", SDLK_F14 },
	{ "KEY_F15", SDLK_F15 },
	{ "KEY_STOP", SDLK_BREAK },
	{ "KEY_NUMPADENTER", SDLK_KP_ENTER },
	{ "KEY_RCONTROL", SDLK_RCTRL },
	{ "KEY_DIVIDE", SDLK_KP_DIVIDE },
	{ "KEY_SYSRQ", SDLK_SYSREQ },
	{ "KEY_RMENU", SDLK_RALT },
	{ "KEY_HOME", SDLK_HOME },
	{ "KEY_UP", SDLK_UP },
	{ "KEY_PRIOR", SDLK_PAGEUP },
	{ "KEY_LEFT", SDLK_LEFT },
	{ "KEY_RIGHT", SDLK_RIGHT },
	{ "KEY_END", SDLK_END },
	{ "KEY_DOWN", SDLK_DOWN },
	{ "KEY_NEXT", SDLK_PAGEDOWN },
	{ "KEY_INSERT", SDLK_INSERT },
	{ "KEY_DELETE", SDLK_DELETE },
	{ "KEY_LWIN", SDLK_LSUPER },
	{ "KEY_RWIN", SDLK_RSUPER },
	{ "KEY_APPS", SDLK_MENU },
	{ "KEY_JOY1_B1", SDLK_JOY1_B1 },
	{ "KEY_JOY1_B2", SDLK_JOY1_B2 },
	{ "KEY_JOY1_B3", SDLK_JOY1_B3 },
	{ "KEY_JOY1_B4", SDLK_JOY1_B4 },
	{ "KEY_JOY1_B5", SDLK_JOY1_B5 },
	{ "KEY_JOY1_B6", SDLK_JOY1_B6 },
	{ "KEY_JOY1_B7", SDLK_JOY1_B7 },
	{ "KEY_JOY1_B8", SDLK_JOY1_B8 },
	{ "KEY_JOY1_B9", SDLK_JOY1_B9 },
	{ "KEY_JOY1_B10", SDLK_JOY1_B10 },
	{ "KEY_JOY1_HLEFT", SDLK_JOY1_HLEFT },
	{ "KEY_JOY1_HUP", SDLK_JOY1_HUP },
	{ "KEY_JOY1_HRIGHT", SDLK_JOY1_HRIGHT },
	{ "KEY_JOY1_HDOWN", SDLK_JOY1_HDOWN },
	{ "KEY_JOY2_B1", SDLK_JOY2_B1 },
	{ "KEY_JOY2_B2", SDLK_JOY2_B2 },
	{ "KEY_JOY2_B3", SDLK_JOY2_B3 },
	{ "KEY_JOY2_B4", SDLK_JOY2_B4 },
	{ "KEY_JOY2_B5", SDLK_JOY2_B5 },
	{ "KEY_JOY2_B6", SDLK_JOY2_B6 },
	{ "KEY_JOY2_B7", SDLK_JOY2_B7 },
	{ "KEY_JOY2_B8", SDLK_JOY2_B8 },
	{ "KEY_JOY2_B9", SDLK_JOY2_B9 },
	{ "KEY_JOY2_B10", SDLK_JOY2_B10 },
	{ "KEY_JOY2_HLEFT", SDLK_JOY2_HLEFT },
	{ "KEY_JOY2_HUP", SDLK_JOY2_HUP },
	{ "KEY_JOY2_HRIGHT", SDLK_JOY2_HRIGHT },
	{ "KEY_JOY2_HDOWN", SDLK_JOY2_HDOWN },
	{ "KEY_MOUSE_B1", SDLK_MOUSE_B1 },
	{ "KEY_MOUSE_B2", SDLK_MOUSE_B2 },
	{ "KEY_MOUSE_B3", SDLK_MOUSE_B3 },
	{ "KEY_MOUSE_B4", SDLK_MOUSE_B4 },
	{ "AXIS_JOY1_X", SDLK_AXIS_JOY1_X },
	{ "AXIS_JOY1_Y", SDLK_AXIS_JOY1_Y },
	{ "AXIS_JOY1_Z", SDLK_AXIS_JOY1_Z },
	{ "AXIS_JOY1_R", SDLK_AXIS_JOY1_R },
	{ "AXIS_JOY1_U", SDLK_AXIS_JOY1_U },
	{ "AXIS_JOY1_V", SDLK_AXIS_JOY1_V },
	{ "AXIS_JOY2_X", SDLK_AXIS_JOY2_X },
	{ "AXIS_JOY2_Y", SDLK_AXIS_JOY2_Y },
	{ "AXIS_JOY2_Z", SDLK_AXIS_JOY2_Z },
	{ "AXIS_JOY2_R", SDLK_AXIS_JOY2_R },
	{ "AXIS_JOY2_U", SDLK_AXIS_JOY2_U },
	{ "AXIS_JOY2_V", SDLK_AXIS_JOY2_V },
	{ "AXIS_MOUSE_X", SDLK_AXIS_MOUSE_X },
	{ "AXIS_MOUSE_Y", SDLK_AXIS_MOUSE_Y },
	{ "AXIS_MOUSE_Z", SDLK_AXIS_MOUSE_Z },
	{ NULL, 0 }
};

// numupper provides conversion between number keys and their "upper case"
const char numupper[] = {')', '!', '@', '#', '$', '%', '^', '&', '*', '('};

static int mapKey(SDLKey key, SDLMod mod, Uint16 unicode)
{
	if (key >= SDLK_KP0 && key <= SDLK_KP9) {
		return key - SDLK_KP0 + '0';
	} else if (unicode) {
		return unicode;
	} else if (key >= 'a' && key <= 'z' && mod & KMOD_SHIFT) {
		return key & ~0x20;
	} else if (key >= SDLK_0 && key <= SDLK_9 && mod & KMOD_SHIFT) {
		return numupper[key - SDLK_0];
	} else if (key > 127) {
		return 0;
	}
	return key;
}

static byte SDLModToDriverKeyFlags(SDLMod mod) {
	byte b = 0;
	if (mod & KMOD_SHIFT)
		b |= Driver::KBD_SHIFT;
	if (mod & KMOD_ALT)
		b |= Driver::KBD_ALT;
	if (mod & KMOD_CTRL)
		b |= Driver::KBD_CTRL;

	return b;
}

char *DriverSDL::getVideoDeviceName() {
	return "SDL Video Device";
}

const Driver::ControlDescriptor *DriverSDL::listControls() {
	return controls;
}

int DriverSDL::getNumControls() {
	return SDLK_EXTRA_LAST;
}

bool DriverSDL::controlIsAxis(int num) {
	return num >= SDLK_AXIS_JOY1_X && num <= SDLK_AXIS_MOUSE_Z;
}

float DriverSDL::getControlAxis(int num) {
	return 0;
}

bool DriverSDL::getControlState(int num) {
	if (num >= SDLK_JOY1_B1 && num <= SDLK_MOUSE_B4)
		return false;
	else {
		uint8 *keystate = SDL_GetKeyState(NULL);
		return keystate[num] != 0;
	}
}

bool DriverSDL::pollEvent(Event &event) {
	SDL_Event ev;
	byte b = 0;

	while(SDL_PollEvent(&ev)) {

		if (ev.type == SDL_KEYDOWN &&
		    (ev.key.keysym.sym == SDLK_RETURN ||
		     ev.key.keysym.sym == SDLK_KP_ENTER) &&
		    (ev.key.keysym.mod & KMOD_ALT)) {
			toggleFullscreenMode();
		}

		switch(ev.type) {
		case SDL_KEYDOWN:{
			event.kbd.flags = SDLModToDriverKeyFlags(SDL_GetModState());

			// Alt-Return and Alt-Enter toggle full screen mode
			if (b == KBD_ALT && (ev.key.keysym.sym == SDLK_RETURN
			                  || ev.key.keysym.sym == SDLK_KP_ENTER)) {
				toggleFullscreenMode();
				break;
			}

			event.type = EVENT_KEYDOWN;
			event.kbd.num = ev.key.keysym.sym;
			event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);

			return true;
		}
		case SDL_KEYUP: {

			event.type = EVENT_KEYUP;
			event.kbd.num = ev.key.keysym.sym;
			event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);
			b = event.kbd.flags = SDLModToDriverKeyFlags(SDL_GetModState());

			return true;
		}
		case SDL_VIDEOEXPOSE:
			event.type = EVENT_REFRESH;
			return true;

		case SDL_QUIT:
			event.type = EVENT_QUIT;
			return true;
		}
	}
	return false;
}

uint32 DriverSDL::getMillis() {
	return SDL_GetTicks();
}

void DriverSDL::delayMillis(uint msecs) {
	SDL_Delay(msecs);
}

void DriverSDL::setTimerCallback(TimerProc callback, int timer) {
	SDL_SetTimer(timer, (SDL_TimerCallback) callback);
}

MutexRef DriverSDL::createMutex() {
	return (MutexRef)SDL_CreateMutex();
}

void DriverSDL::lockMutex(MutexRef mutex) {
	SDL_mutexP((SDL_mutex *)mutex);
}

void DriverSDL::unlockMutex(MutexRef mutex) {
	SDL_mutexV((SDL_mutex *)mutex);
}

void DriverSDL::deleteMutex(MutexRef mutex) {
	SDL_DestroyMutex((SDL_mutex *)mutex);
}

bool DriverSDL::setSoundCallback(SoundProc proc, void *param) {
	SDL_AudioSpec desired;

	memset(&desired, 0, sizeof(desired));

	desired.freq = _samplesPerSec;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = 2048;
	desired.callback = proc;
	desired.userdata = param;

	if (SDL_OpenAudio(&desired, NULL) != 0) {
		return false;
	}

	SDL_PauseAudio(0);
	return true;
}

void DriverSDL::clearSoundCallback() {
	SDL_CloseAudio();
}

int DriverSDL::getOutputSampleRate() const {
	return _samplesPerSec;
}

/* This function sends the SDL signal to
 * go ahead and exit the game
 */
void DriverSDL::quit() {
	SDL_Event event;
	
	event.type = SDL_QUIT;
	if (SDL_PushEvent(&event) != 0)
		error("Unable to push exit event!");
}
