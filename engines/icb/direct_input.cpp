/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_rccommon.h"

#if defined (SDL_BACKEND)
#include <SDL_joystick.h>
#endif

#include "engines/icb/p4_generic.h"
#include "engines/icb/debug.h"
#include "engines/icb/p4.h"
#include "engines/icb/player.h"
#include "engines/icb/direct_input.h"
#include "engines/icb/cluster_manager_pc.h"

#include "common/textconsole.h"
#include "common/events.h"
#include "common/keyboard.h"

namespace ICB {

uint32 dinputTime = 0;

// Initialise global joystick variables
// This is given an initial value during init_direct_input() but can be
// used as a switch to alter joystick behaviour (analogue/digital/off)
int currentJoystick = NO_JOYSTICK;
int attachedJoystick = NO_JOYSTICK;

// Initialised to unset values
// For mapping purposes
uint8 up_joy = 0xFF;
uint8 down_joy = 0xFF;
uint8 left_joy = 0xFF;
uint8 right_joy = 0xFF;
uint8 sidestep_button = 0xFF;
uint8 run_button = 0xFF;
uint8 crouch_button = 0xFF;
uint8 interact_button = 0xFF;
uint8 arm_button = 0xFF;
uint8 fire_button = 0xFF;
uint8 inventory_button = 0xFF;
uint8 remora_button = 0xFF;
uint8 pause_button = 0xFF;

// These are the structures used to hold the data we are polling for joysticks
#define MAX_AXES 8

typedef struct {
	bool8 held_down; // For polling
	char name[12];   // For the options screen configuration

} BUTTON;

typedef struct {
	int32 val;     // For polling
	char name[12]; // For the options screen configuration

} HALFAXIS;

int g_noButtons = 0;        // Available number of buttons
int g_noAxes = 0;           // Available number of axes
BUTTON g_buttons[32];       // Button data (max 32)
bool8 g_buttonsRepeats[32]; // Button repeat data (max 32)
HALFAXIS g_axes[MAX_AXES];  // Axes data (max 8 * 2)

// Fixed defines for mapping
#define AXIS_1_POS 0
#define AXIS_1_NEG 1
#define AXIS_2_POS 2
#define AXIS_2_NEG 3
#define AXIS_3_POS 4
#define AXIS_3_NEG 5
#define AXIS_4_POS 6
#define AXIS_4_NEG 7

#define JOY_B1 1
#define JOY_B2 2
#define JOY_B3 4
#define JOY_B4 8

typedef struct {
	uint32 posX;
	uint32 posY;
	uint32 buttons;
} JoyInfo;

class NTJoystickHandler {
      private:
	int joyThere[16]; // 1  if joystick is plugged in, 0 otherwise
#if defined (SDL_BACKEND)
	SDL_Joystick *joy[16];
#endif
	JoyInfo cached_joyinfo[16];

      public:
	NTJoystickHandler(void);
	~NTJoystickHandler(void);

	bool8 isValid;

	void Init(void);

	// HW Query functions: determine whether or not a device exists.
	// Return 0 if no such device, > 0 (depending on device) if it does.
	int QueryJoystick(void); // 0=none, else number of joysticks
	// Input functions: anyone wishing for input needs to call these:
	int GetJoystick(int joynum, JoyInfo *joypos);
	void UpdateJoystick(void);
};

NTJoystickHandler::NTJoystickHandler(void) {
	memset(joyThere, 0, sizeof(int) * 16);

	isValid = FALSE8;
}

NTJoystickHandler::~NTJoystickHandler(void) {
	for (int x = 0; x < 16; x++) {
		if (joyThere[x]) {
			// FIXME: By the time we're in this destructor, SDL has already been terminated.
			// SDL_JoystickClose(joy[x]);
		}
	}
}

void NTJoystickHandler::Init() {
	for (int x = 0; x < QueryJoystick(); x++) {
#if defined (SDL_BACKEND)
		SDL_Joystick *j = SDL_JoystickOpen(x);
		if (j) {
			// there *is* a joystick 'x' installed!
			++joyThere[x];
			joy[x] = j;
		}
#endif
	}
	UpdateJoystick();
}

int NTJoystickHandler::QueryJoystick(void) {
	// if no joystick(s), returns 0 else number of joysticks attached.
	return
#if defined (SDL_BACKEND)
	(SDL_NumJoysticks());
#else
	0;
#endif
}

int normalize(int val, int minval, int maxval) {
	// error detection:
	if ((maxval - minval) == 0) {
		return (0);
	}

	// zero-base:
	val -= minval;

	// normalize to 0..255:
	val = (255L * val) / (maxval - minval);

	return (val);
}

void NTJoystickHandler::UpdateJoystick() {
#ifdef ENABLE_JOYSTICK
	for (int x = 0; x < 16; x++) {
		if (joyThere[x] && SDL_JoystickGetAttached(joy[x])) {
			cached_joyinfo[x].posX = SDL_JoystickGetAxis(joy[x], 0);
			cached_joyinfo[x].posY = SDL_JoystickGetAxis(joy[x], 1);
			cached_joyinfo[x].buttons = 0;
			if (SDL_JoystickGetButton(joy[x], 0)) {
				cached_joyinfo[x].buttons |= JOY_B1;
			}
			if (SDL_JoystickGetButton(joy[x], 1)) {
				cached_joyinfo[x].buttons |= JOY_B2;
			}
			if (SDL_JoystickGetButton(joy[x], 2)) {
				cached_joyinfo[x].buttons |= JOY_B3;
			}
			if (SDL_JoystickGetButton(joy[x], 3)) {
				cached_joyinfo[x].buttons |= JOY_B4;
			}
		}
	}
#endif
}

int NTJoystickHandler::GetJoystick(int joynum, JoyInfo *joypos) {
	if ((joynum >= 16) || (joynum <= 0) || !joyThere[joynum - 1])
		return (0);

	memcpy(joypos, &(cached_joyinfo[joynum - 1]), sizeof(JoyInfo));

	// normalize the joypos to -127, 0, 127 scale....
	joypos->posX = normalize(joypos->posX, -32768, 32768);
	joypos->posY = normalize(joypos->posY, -32768, 32768);

	// To overcome UINT type within JOYINFO struct
	int32 ntX = joypos->posX - 127;
	int32 ntY = joypos->posY - 127;

	g_axes[AXIS_1_POS].val = (ntX < 0) ? 0 : ntX;
	g_axes[AXIS_1_NEG].val = (ntX >= 0) ? 0 : ntX;
	g_axes[AXIS_2_POS].val = (ntY < 0) ? 0 : ntY;
	g_axes[AXIS_2_NEG].val = (ntY >= 0) ? 0 : ntY;

	return (1);
}

NTJoystickHandler ntJoystick;

bool PollNTJoystick() {
	ntJoystick.UpdateJoystick();

	// State structre
	JoyInfo inf;

	// Get current state
	if (ntJoystick.GetJoystick(1, &inf) == 0) {
		ntJoystick.isValid = FALSE8;
		attachedJoystick = NO_JOYSTICK;
		Zdebug("GetJoystick failed");

		return false;
	}

	// Buttons

	if (inf.buttons & JOY_B1)
		g_buttons[0].held_down = TRUE8;
	else
		g_buttons[0].held_down = FALSE8;

	if (inf.buttons & JOY_B2)
		g_buttons[1].held_down = TRUE8;
	else
		g_buttons[1].held_down = FALSE8;

	if (inf.buttons & JOY_B3)
		g_buttons[2].held_down = TRUE8;
	else
		g_buttons[2].held_down = FALSE8;

	if (inf.buttons & JOY_B4)
		g_buttons[3].held_down = TRUE8;
	else
		g_buttons[3].held_down = FALSE8;

	return true;
}

bool8 keyboard_buf_scancodes[512]; // SDL_NUM_SCANCODES
bool8 repeats_scancodes[512];      // SDL_NUM_SCANCODES

void Init_direct_input() {
	uint32 k;

	// Zero everything out
	for (k = 0; k < 32; k++) {
		g_buttons[k].held_down = FALSE8;
		memset(g_buttons[k].name, 0, 12);
		g_buttonsRepeats[k] = FALSE8;
	}
	for (k = 0; k < MAX_AXES; k++) {
		g_axes[k].val = 0;
		memset(g_axes[k].name, 0, 12);
	}

	// Default axes to use (overriden by ini file config)
	up_joy = AXIS_2_NEG;
	down_joy = AXIS_2_POS;
	left_joy = AXIS_1_NEG;
	right_joy = AXIS_1_POS;

	// Initialize joystick(s)
	ntJoystick.Init();

	if (ntJoystick.QueryJoystick() == 0) {
		ntJoystick.isValid = FALSE8;

		Zdebug("Joystick not installed");

		currentJoystick = NO_JOYSTICK;
	} else {
		ntJoystick.isValid = TRUE8;

		// For development give these defaults
		g_noButtons = 4;
		strcpy(g_buttons[0].name, "Button 1");
		strcpy(g_buttons[1].name, "Button 2");
		strcpy(g_buttons[2].name, "Button 3");
		strcpy(g_buttons[3].name, "Button 4");
		g_noAxes = 2;
		strcpy(g_axes[AXIS_1_POS].name, "+ve X-Axis");
		strcpy(g_axes[AXIS_1_NEG].name, "-ve X-Axis");
		strcpy(g_axes[AXIS_2_POS].name, "+ve Y-Axis");
		strcpy(g_axes[AXIS_2_NEG].name, "-ve Y-Axis");

		attachedJoystick = ANALOGUE_JOYSTICK;
		currentJoystick = ANALOGUE_JOYSTICK;
	}

	SetDefaultKeys();

	// Ready to accept all input if we're on NT or not !!!
}

void setKeyState(Common::KeyCode key, bool pressed) { keyboard_buf_scancodes[key] = pressed; }

void Poll_direct_input() {
	//warning("TODO: Fix Poll_direct_input");
#if 0
	int keys = 0;
	const Uint8 *key_state = SDL_GetKeyboardState(&keys);
	for (uint32 i = 0; i < keys; i++) {
		keyboard_buf_scancodes[i] = key_state[i] ? TRUE8 : FALSE8;
	}

	// Already set to NULL if no pad attached so don't worry
	if (ntJoystick.isValid) {
		if (!PollNTJoystick()) {
			ntJoystick.isValid = FALSE8;
			currentJoystick = NO_JOYSTICK;
			Zdebug("PollNTJoystick failed");
		}
	}
#endif
}

uint32 Get_DI_key_press() {
	for (uint32 i = 0; i < 512; i++) {
		if (Read_DI_once_keys(i)) {
			return i;
		}
	}

	return 0;
}

void Clear_DI_key_buffer() {
	for (uint32 i = 0; i < 512; i++) {
		repeats_scancodes[i] = FALSE8;
		keyboard_buf_scancodes[i] = FALSE8;
	}
}

bool8 Read_DI_once_keys(uint32 key) {
	// in
	//      key = keycode

	// out
	//      0 not pressed down currently
	//      1 pressed down

	// set repeat
	if (keyboard_buf_scancodes[key] && (repeats_scancodes[key]))
		return (0); // key is still pressed so return 0

	repeats_scancodes[key] = keyboard_buf_scancodes[key];

	return (repeats_scancodes[key]);
}

bool8 Read_DI_keys(uint32 key) {
	// in
	//      key = keycode

	// out
	//      0 not pressed down currently
	//      1 pressed down

	// set repeat
	repeats_scancodes[key] = keyboard_buf_scancodes[key];

	return (repeats_scancodes[key]);
}

bool8 DI_key_waiting() {
	for (uint32 i = 0; i < 512; i++) {
		if (keyboard_buf_scancodes[i])
			return TRUE8;
	}

	return FALSE8;
}

void SetDefaultKeys() {
	fire_key = Common::KEYCODE_SPACE;
	interact_key = Common::KEYCODE_LCTRL;
	inventory_key = Common::KEYCODE_RETURN;
	arm_key = Common::KEYCODE_LALT;
	remora_key = Common::KEYCODE_DELETE;
	crouch_key = Common::KEYCODE_x;
	sidestep_key = Common::KEYCODE_LSHIFT;
	run_key = Common::KEYCODE_z;
	up_key = Common::KEYCODE_UP;
	down_key = Common::KEYCODE_DOWN;
	left_key = Common::KEYCODE_LEFT;
	right_key = Common::KEYCODE_RIGHT;
	pause_key = Common::KEYCODE_ESCAPE;
}

void UnsetKeys() {
	// Unset all
	fire_key = 0;
	interact_key = 0;
	inventory_key = 0;
	arm_key = 0;
	remora_key = 0;
	crouch_key = 0;
	sidestep_key = 0;
	run_key = 0;
	up_key = 0;
	down_key = 0;
	left_key = 0;
	right_key = 0;
	pause_key = Common::KEYCODE_ESCAPE; // So we can always bring up the options
}

const char *GetKeyName(uint32 key) {
	TLANGUAGE tl = g_theClusterManager->GetLanguage();

	switch (tl) {
	case T_FRENCH: // French key names
		switch (key) {
		case Common::KEYCODE_ESCAPE:
			return "Echap";
		case Common::KEYCODE_1:
			return "1";
		case Common::KEYCODE_2:
			return "2";
		case Common::KEYCODE_3:
			return "3";
		case Common::KEYCODE_4:
			return "4";
		case Common::KEYCODE_5:
			return "5";
		case Common::KEYCODE_6:
			return "6";
		case Common::KEYCODE_7:
			return "7";
		case Common::KEYCODE_8:
			return "8";
		case Common::KEYCODE_9:
			return "9";
		case Common::KEYCODE_0:
			return "0";
		case Common::KEYCODE_MINUS:
			return "Moins";
		case Common::KEYCODE_EQUALS:
			return "Egal";
		case Common::KEYCODE_BACKSPACE:
			return "Retour arri" "\xe8" "re";
		case Common::KEYCODE_TAB:
			return "Tabulation";
		case Common::KEYCODE_q:
			return "Q";
		case Common::KEYCODE_w:
			return "W";
		case Common::KEYCODE_e:
			return "E";
		case Common::KEYCODE_r:
			return "R";
		case Common::KEYCODE_t:
			return "T";
		case Common::KEYCODE_y:
			return "Y";
		case Common::KEYCODE_u:
			return "U";
		case Common::KEYCODE_i:
			return "I";
		case Common::KEYCODE_o:
			return "O";
		case Common::KEYCODE_p:
			return "P";
		case Common::KEYCODE_LEFTBRACKET:
			return "Parenth" "\xe8" "se g";
		case Common::KEYCODE_RIGHTBRACKET:
			return "Parenth" "\xe8" "se d";
		case Common::KEYCODE_RETURN:
			return "Entr" "\xe9" "e";
		case Common::KEYCODE_LCTRL:
			return "Ctrl gauche";
		case Common::KEYCODE_a:
			return "A";
		case Common::KEYCODE_s:
			return "S";
		case Common::KEYCODE_d:
			return "D";
		case Common::KEYCODE_f:
			return "F";
		case Common::KEYCODE_g:
			return "G";
		case Common::KEYCODE_h:
			return "H";
		case Common::KEYCODE_j:
			return "J";
		case Common::KEYCODE_k:
			return "K";
		case Common::KEYCODE_l:
			return "L";
		case Common::KEYCODE_SEMICOLON:
			return "Point-virgule";
		case Common::KEYCODE_QUOTE:
			return "Apostrophe";
		case Common::KEYCODE_BACKQUOTE:
			return "Carr" "\xe9";
		case Common::KEYCODE_LSHIFT:
			return "Maj gauche";
		case Common::KEYCODE_BACKSLASH:
			return "Di" "\xe8" "se";
		case Common::KEYCODE_z:
			return "Z";
		case Common::KEYCODE_x:
			return "X";
		case Common::KEYCODE_c:
			return "C";
		case Common::KEYCODE_v:
			return "V";
		case Common::KEYCODE_b:
			return "B";
		case Common::KEYCODE_n:
			return "N";
		case Common::KEYCODE_m:
			return "M";
		case Common::KEYCODE_COMMA:
			return "Virgule";
		case Common::KEYCODE_PERIOD:
			return "Point";
		case Common::KEYCODE_SLASH:
			return "Barre oblique";
		case Common::KEYCODE_RSHIFT:
			return "Maj droite";
		case Common::KEYCODE_KP_MULTIPLY:
			return "Multiplier num.";
		case Common::KEYCODE_SPACE:
			return "Espace";
		case Common::KEYCODE_CAPSLOCK:
			return "Verr. num.";
		case Common::KEYCODE_F1:
			return "F1";
		case Common::KEYCODE_F2:
			return "F2";
		case Common::KEYCODE_F3:
			return "F3";
		case Common::KEYCODE_F4:
			return "F4";
		case Common::KEYCODE_F5:
			return "F5";
		case Common::KEYCODE_F6:
			return "F6";
		case Common::KEYCODE_F7:
			return "F7";
		case Common::KEYCODE_F8:
			return "F8";
		case Common::KEYCODE_F9:
			return "F9";
		case Common::KEYCODE_F10:
			return "F10";
		case Common::KEYCODE_F11:
			return "F11";
		case Common::KEYCODE_F12:
			return "F12";
		case Common::KEYCODE_KP7:
			return "Num 7";
		case Common::KEYCODE_KP8:
			return "Num 8";
		case Common::KEYCODE_KP9:
			return "Num 9";
		case Common::KEYCODE_KP_MINUS:
			return "Soustraire num";
		case Common::KEYCODE_KP4:
			return "Num 4";
		case Common::KEYCODE_KP5:
			return "Num 5";
		case Common::KEYCODE_KP6:
			return "Num 6";
		case Common::KEYCODE_KP_PLUS:
			return "Ajouter num.";
		case Common::KEYCODE_KP1:
			return "Num 1";
		case Common::KEYCODE_KP2:
			return "Num 2";
		case Common::KEYCODE_KP3:
			return "Num 3";
		case Common::KEYCODE_KP0:
			return "Num 0";
		case Common::KEYCODE_KP_PERIOD:
			return "D" "\xe9" "cimal num.";
		case Common::KEYCODE_KP_ENTER:
			return "Entr";
		case Common::KEYCODE_RCTRL:
			return "Ctrl droite";
		case Common::KEYCODE_KP_DIVIDE:
			return "Diviser num";
		case Common::KEYCODE_HOME:
			return "Origine";
		case Common::KEYCODE_UP:
			return "Fl" "\xe8" "che haut";
		case Common::KEYCODE_PAGEUP:
			return "Pg pr" "\xe9" "c.";
		case Common::KEYCODE_LEFT:
			return "Fl" "\xe8" "che gauche";
		case Common::KEYCODE_RIGHT:
			return "Fl" "\xe8" "che droite";
		case Common::KEYCODE_END:
			return "Fin";
		case Common::KEYCODE_DOWN:
			return "Fl" "\xe8" "che bas";
		case Common::KEYCODE_PAGEDOWN:
			return "Pg Suiv.";
		case Common::KEYCODE_INSERT:
			return "Inser";
		case Common::KEYCODE_DELETE:
			return "Suppr";
		case Common::KEYCODE_LALT:
			return "Alt gauche";
		case Common::KEYCODE_RALT:
			return "Alt droite";

		default:
			return NULL;
		}
		return NULL;

	case T_GERMAN: // German key names
		switch (key) {
		case Common::KEYCODE_ESCAPE:
			return "ESC";
		case Common::KEYCODE_1:
			return "1";
		case Common::KEYCODE_2:
			return "2";
		case Common::KEYCODE_3:
			return "3";
		case Common::KEYCODE_4:
			return "4";
		case Common::KEYCODE_5:
			return "5";
		case Common::KEYCODE_6:
			return "6";
		case Common::KEYCODE_7:
			return "7";
		case Common::KEYCODE_8:
			return "8";
		case Common::KEYCODE_9:
			return "9";
		case Common::KEYCODE_0:
			return "0";
		case Common::KEYCODE_MINUS:
			return "Minus";
		case Common::KEYCODE_EQUALS:
			return "Gleich";
		case Common::KEYCODE_BACKSPACE:
			return "Rucktaste";
		case Common::KEYCODE_TAB:
			return "Tab";
		case Common::KEYCODE_q:
			return "Q";
		case Common::KEYCODE_w:
			return "W";
		case Common::KEYCODE_e:
			return "E";
		case Common::KEYCODE_r:
			return "R";
		case Common::KEYCODE_t:
			return "T";
		case Common::KEYCODE_y:
			return "Z";
		case Common::KEYCODE_u:
			return "U";
		case Common::KEYCODE_i:
			return "I";
		case Common::KEYCODE_o:
			return "O";
		case Common::KEYCODE_p:
			return "P";
		case Common::KEYCODE_LEFTBRACKET:
			return "Klammer L";
		case Common::KEYCODE_RIGHTBRACKET:
			return "Klammer R";
		case Common::KEYCODE_RETURN:
			return "Eingabe";
		case Common::KEYCODE_LCTRL:
			return "Strg links";
		case Common::KEYCODE_a:
			return "A";
		case Common::KEYCODE_s:
			return "S";
		case Common::KEYCODE_d:
			return "D";
		case Common::KEYCODE_f:
			return "F";
		case Common::KEYCODE_g:
			return "G";
		case Common::KEYCODE_h:
			return "H";
		case Common::KEYCODE_j:
			return "J";
		case Common::KEYCODE_k:
			return "K";
		case Common::KEYCODE_l:
			return "L";
		case Common::KEYCODE_SEMICOLON:
			return "Semikolon";
		case Common::KEYCODE_QUOTE:
			return "Apostroph";
		case Common::KEYCODE_BACKQUOTE:
			return "Akzent";
		case Common::KEYCODE_LSHIFT:
			return "Umschalt L";
		case Common::KEYCODE_BACKSLASH:
			return "Raute";
		case Common::KEYCODE_z:
			return "Y";
		case Common::KEYCODE_x:
			return "X";
		case Common::KEYCODE_c:
			return "C";
		case Common::KEYCODE_v:
			return "V";
		case Common::KEYCODE_b:
			return "B";
		case Common::KEYCODE_n:
			return "N";
		case Common::KEYCODE_m:
			return "M";
		case Common::KEYCODE_COMMA:
			return "Komma";
		case Common::KEYCODE_PERIOD:
			return "Punkt";
		case Common::KEYCODE_SLASH:
			return "Schragstrich";
		case Common::KEYCODE_RSHIFT:
			return "Umschalt R";
		case Common::KEYCODE_KP_MULTIPLY:
			return "Num Mal";
		case Common::KEYCODE_SPACE:
			return "Leertaste";
		case Common::KEYCODE_CAPSLOCK:
			return "Feststelltaste";
		case Common::KEYCODE_F1:
			return "F1";
		case Common::KEYCODE_F2:
			return "F2";
		case Common::KEYCODE_F3:
			return "F3";
		case Common::KEYCODE_F4:
			return "F4";
		case Common::KEYCODE_F5:
			return "F5";
		case Common::KEYCODE_F6:
			return "F6";
		case Common::KEYCODE_F7:
			return "F7";
		case Common::KEYCODE_F8:
			return "F8";
		case Common::KEYCODE_F9:
			return "F9";
		case Common::KEYCODE_F10:
			return "F10";
		case Common::KEYCODE_F11:
			return "F11";
		case Common::KEYCODE_F12:
			return "F12";
		case Common::KEYCODE_KP7:
			return "Num 7";
		case Common::KEYCODE_KP8:
			return "Num 8";
		case Common::KEYCODE_KP9:
			return "Num 9";
		case Common::KEYCODE_KP_MINUS:
			return "Num Minus";
		case Common::KEYCODE_KP4:
			return "Num 4";
		case Common::KEYCODE_KP5:
			return "Num 5";
		case Common::KEYCODE_KP6:
			return "Num 6";
		case Common::KEYCODE_KP_PLUS:
			return "Num Plus";
		case Common::KEYCODE_KP1:
			return "Num 1";
		case Common::KEYCODE_KP2:
			return "Num 2";
		case Common::KEYCODE_KP3:
			return "Num 3";
		case Common::KEYCODE_KP0:
			return "Num 0";
		case Common::KEYCODE_KP_PERIOD:
			return "Num Dezimal";
		case Common::KEYCODE_KP_ENTER:
			return "Eingabe";
		case Common::KEYCODE_RCTRL:
			return "Strg rechts";
		case Common::KEYCODE_KP_DIVIDE:
			return "Num Geteilt";
		case Common::KEYCODE_HOME:
			return "Pos1";
		case Common::KEYCODE_UP:
			return "Pfeil hoch";
		case Common::KEYCODE_PAGEUP:
			return "Bild hoch";
		case Common::KEYCODE_LEFT:
			return "Pfeil links";
		case Common::KEYCODE_RIGHT:
			return "Pfeil rechts";
		case Common::KEYCODE_END:
			return "Ende";
		case Common::KEYCODE_DOWN:
			return "Pfeil runter";
		case Common::KEYCODE_PAGEDOWN:
			return "Bild runter";
		case Common::KEYCODE_INSERT:
			return "Einf.";
		case Common::KEYCODE_DELETE:
			return "Entf";
		case Common::KEYCODE_LALT:
			return "Alt links";
		case Common::KEYCODE_RALT:
			return "Alt rechts";

		default:
			return NULL;
		}
		return NULL;

	case T_ITALIAN: // Italian key names
		switch (key) {
		case Common::KEYCODE_ESCAPE:
			return "esci";
		case Common::KEYCODE_1:
			return "1";
		case Common::KEYCODE_2:
			return "2";
		case Common::KEYCODE_3:
			return "3";
		case Common::KEYCODE_4:
			return "4";
		case Common::KEYCODE_5:
			return "5";
		case Common::KEYCODE_6:
			return "6";
		case Common::KEYCODE_7:
			return "7";
		case Common::KEYCODE_8:
			return "8";
		case Common::KEYCODE_9:
			return "9";
		case Common::KEYCODE_0:
			return "0";
		case Common::KEYCODE_MINUS:
			return "meno";
		case Common::KEYCODE_EQUALS:
			return "uguale";
		case Common::KEYCODE_BACKSPACE:
			return "spazio indietro";
		case Common::KEYCODE_TAB:
			return "Tab";
		case Common::KEYCODE_q:
			return "Q";
		case Common::KEYCODE_w:
			return "W";
		case Common::KEYCODE_e:
			return "E";
		case Common::KEYCODE_r:
			return "R";
		case Common::KEYCODE_t:
			return "T";
		case Common::KEYCODE_y:
			return "Y";
		case Common::KEYCODE_u:
			return "U";
		case Common::KEYCODE_i:
			return "I";
		case Common::KEYCODE_o:
			return "O";
		case Common::KEYCODE_p:
			return "P";
		case Common::KEYCODE_LEFTBRACKET:
			return "parentesi sinistra";
		case Common::KEYCODE_RIGHTBRACKET:
			return "parentesi destra";
		case Common::KEYCODE_RETURN:
			return "torna indietro";
		case Common::KEYCODE_LCTRL:
			return "control a sinistra";
		case Common::KEYCODE_a:
			return "A";
		case Common::KEYCODE_s:
			return "S";
		case Common::KEYCODE_d:
			return "D";
		case Common::KEYCODE_f:
			return "F";
		case Common::KEYCODE_g:
			return "G";
		case Common::KEYCODE_h:
			return "H";
		case Common::KEYCODE_j:
			return "J";
		case Common::KEYCODE_k:
			return "K";
		case Common::KEYCODE_l:
			return "L";
		case Common::KEYCODE_SEMICOLON:
			return "punto e virgola";
		case Common::KEYCODE_QUOTE:
			return "apostrofo";
		case Common::KEYCODE_BACKQUOTE:
			return "slash sinistra"; // ANDYB: TOO LONG "barra obliqua a sinistra"
		case Common::KEYCODE_LSHIFT:
			return "maiuscolo sinistro";
		case Common::KEYCODE_BACKSLASH:
			return "cancelletto";
		case Common::KEYCODE_z:
			return "Z";
		case Common::KEYCODE_x:
			return "X";
		case Common::KEYCODE_c:
			return "C";
		case Common::KEYCODE_v:
			return "V";
		case Common::KEYCODE_b:
			return "B";
		case Common::KEYCODE_n:
			return "N";
		case Common::KEYCODE_m:
			return "M";
		case Common::KEYCODE_COMMA:
			return "virgola";
		case Common::KEYCODE_PERIOD:
			return "punto";
		case Common::KEYCODE_SLASH:
			return "segno di frazione";
		case Common::KEYCODE_RSHIFT:
			return "maiuscolo destro";
		case Common::KEYCODE_KP_MULTIPLY:
			return "moltiplicazione";
		case Common::KEYCODE_SPACE:
			return "spazio";
		case Common::KEYCODE_CAPSLOCK:
			return "lucchetto";
		case Common::KEYCODE_F1:
			return "F1";
		case Common::KEYCODE_F2:
			return "F2";
		case Common::KEYCODE_F3:
			return "F3";
		case Common::KEYCODE_F4:
			return "F4";
		case Common::KEYCODE_F5:
			return "F5";
		case Common::KEYCODE_F6:
			return "F6";
		case Common::KEYCODE_F7:
			return "F7";
		case Common::KEYCODE_F8:
			return "F8";
		case Common::KEYCODE_F9:
			return "F9";
		case Common::KEYCODE_F10:
			return "F10";
		case Common::KEYCODE_F11:
			return "F11";
		case Common::KEYCODE_F12:
			return "F12";
		case Common::KEYCODE_KP7:
			return "Num 7";
		case Common::KEYCODE_KP8:
			return "Num 8";
		case Common::KEYCODE_KP9:
			return "Num 9";
		case Common::KEYCODE_KP_MINUS:
			return "sottrazione";
		case Common::KEYCODE_KP4:
			return "Num 4";
		case Common::KEYCODE_KP5:
			return "Num 5";
		case Common::KEYCODE_KP6:
			return "Num 6";
		case Common::KEYCODE_KP_PLUS:
			return "addizione";
		case Common::KEYCODE_KP1:
			return "Num 1";
		case Common::KEYCODE_KP2:
			return "Num 2";
		case Common::KEYCODE_KP3:
			return "Num 3";
		case Common::KEYCODE_KP0:
			return "Num 0";
		case Common::KEYCODE_KP_PERIOD:
			return "numero decimale";
		case Common::KEYCODE_KP_ENTER:
			return "invio";
		case Common::KEYCODE_RCTRL:
			return "control a destra";
		case Common::KEYCODE_KP_DIVIDE:
			return "divisione";
		case Common::KEYCODE_HOME:
			return "Home";
		case Common::KEYCODE_UP:
			return "freccia in su";
		case Common::KEYCODE_PAGEUP:
			return "pagina in su";
		case Common::KEYCODE_LEFT:
			return "freccia a sinistra";
		case Common::KEYCODE_RIGHT:
			return "freccia a destra";
		case Common::KEYCODE_END:
			return "fine";
		case Common::KEYCODE_DOWN:
			return "freccia in gi" "\xf9";
		case Common::KEYCODE_PAGEDOWN:
			return "pagina in gi" "\xf9";
		case Common::KEYCODE_INSERT:
			return "inserisci";
		case Common::KEYCODE_DELETE:
			return "cancella";
		case Common::KEYCODE_LALT:
			return "alt a sinistra";
		case Common::KEYCODE_RALT:
			return "alt a destra";

		default:
			return NULL;
		}
		return NULL;

	case T_SPANISH: // Spanish key names
		switch (key) {
		case Common::KEYCODE_ESCAPE:
			return "Escape";
		case Common::KEYCODE_1:
			return "1";
		case Common::KEYCODE_2:
			return "2";
		case Common::KEYCODE_3:
			return "3";
		case Common::KEYCODE_4:
			return "4";
		case Common::KEYCODE_5:
			return "5";
		case Common::KEYCODE_6:
			return "6";
		case Common::KEYCODE_7:
			return "7";
		case Common::KEYCODE_8:
			return "8";
		case Common::KEYCODE_9:
			return "9";
		case Common::KEYCODE_0:
			return "0";
		case Common::KEYCODE_MINUS:
			return "Ap" "\xf3" "strofo";
		case Common::KEYCODE_EQUALS:
			return "Admiraci" "\xf3" "n";
		case Common::KEYCODE_BACKSPACE:
			return "Retroceso";
		case Common::KEYCODE_TAB:
			return "Tabulador";
		case Common::KEYCODE_q:
			return "Q";
		case Common::KEYCODE_w:
			return "W";
		case Common::KEYCODE_e:
			return "E";
		case Common::KEYCODE_r:
			return "R";
		case Common::KEYCODE_t:
			return "T";
		case Common::KEYCODE_y:
			return "Y";
		case Common::KEYCODE_u:
			return "U";
		case Common::KEYCODE_i:
			return "I";
		case Common::KEYCODE_o:
			return "O";
		case Common::KEYCODE_p:
			return "P";
		case Common::KEYCODE_LEFTBRACKET:
			return "Suma";
		case Common::KEYCODE_RIGHTBRACKET:
			return "Tilde";
		case Common::KEYCODE_RETURN:
			return "Intro";
		case Common::KEYCODE_LCTRL:
			return "Control Izquierdo";
		case Common::KEYCODE_a:
			return "A";
		case Common::KEYCODE_s:
			return "S";
		case Common::KEYCODE_d:
			return "D";
		case Common::KEYCODE_f:
			return "F";
		case Common::KEYCODE_g:
			return "G";
		case Common::KEYCODE_h:
			return "H";
		case Common::KEYCODE_j:
			return "J";
		case Common::KEYCODE_k:
			return "K";
		case Common::KEYCODE_l:
			return "L";
		case Common::KEYCODE_SEMICOLON:
			return "\xd1";
		case Common::KEYCODE_QUOTE:
			return "Tilde Invertida";
		case Common::KEYCODE_BACKQUOTE:
			return "O Altilla";
		case Common::KEYCODE_LSHIFT:
			return "May\xfasculas Izquierda";
		case Common::KEYCODE_BACKSLASH:
			return "Barra Invertida";
		case Common::KEYCODE_z:
			return "Z";
		case Common::KEYCODE_x:
			return "X";
		case Common::KEYCODE_c:
			return "C";
		case Common::KEYCODE_v:
			return "V";
		case Common::KEYCODE_b:
			return "B";
		case Common::KEYCODE_n:
			return "N";
		case Common::KEYCODE_m:
			return "M";
		case Common::KEYCODE_COMMA:
			return "Coma";
		case Common::KEYCODE_PERIOD:
			return "Punto";
		case Common::KEYCODE_SLASH:
			return "Menos";
		case Common::KEYCODE_RSHIFT:
			return "May\xfasculas Derecha";
		case Common::KEYCODE_KP_MULTIPLY:
			return "Asterisco";
		case Common::KEYCODE_SPACE:
			return "Barra Espaciadora";
		case Common::KEYCODE_CAPSLOCK:
			return "Bloqueo May\xfasculas";
		case Common::KEYCODE_F1:
			return "F1";
		case Common::KEYCODE_F2:
			return "F2";
		case Common::KEYCODE_F3:
			return "F3";
		case Common::KEYCODE_F4:
			return "F4";
		case Common::KEYCODE_F5:
			return "F5";
		case Common::KEYCODE_F6:
			return "F6";
		case Common::KEYCODE_F7:
			return "F7";
		case Common::KEYCODE_F8:
			return "F8";
		case Common::KEYCODE_F9:
			return "F9";
		case Common::KEYCODE_F10:
			return "F10";
		case Common::KEYCODE_F11:
			return "F11";
		case Common::KEYCODE_F12:
			return "F12";
		case Common::KEYCODE_KP7:
			return "Num 7";
		case Common::KEYCODE_KP8:
			return "Num 8";
		case Common::KEYCODE_KP9:
			return "Num 9";
		case Common::KEYCODE_KP_MINUS:
			return "Menos";
		case Common::KEYCODE_KP4:
			return "Num 4";
		case Common::KEYCODE_KP5:
			return "Num 5";
		case Common::KEYCODE_KP6:
			return "Num 6";
		case Common::KEYCODE_KP_PLUS:
			return "M\xe1s";
		case Common::KEYCODE_KP1:
			return "Num 1";
		case Common::KEYCODE_KP2:
			return "Num 2";
		case Common::KEYCODE_KP3:
			return "Num 3";
		case Common::KEYCODE_KP0:
			return "Num 0";
		case Common::KEYCODE_KP_PERIOD:
			return "Punto";
		case Common::KEYCODE_KP_ENTER:
			return "Intro";
		case Common::KEYCODE_RCTRL:
			return "Control Derecho";
		case Common::KEYCODE_KP_DIVIDE:
			return "Dividir";
		case Common::KEYCODE_HOME:
			return "Inicio";
		case Common::KEYCODE_UP:
			return "Flecha Arriba";
		case Common::KEYCODE_PAGEUP:
			return "Avance P\xe1gina";
		case Common::KEYCODE_LEFT:
			return "Flecha Izquierda";
		case Common::KEYCODE_RIGHT:
			return "Flecha Derecha";
		case Common::KEYCODE_END:
			return "Fin";
		case Common::KEYCODE_DOWN:
			return "Flecha Abajo";
		case Common::KEYCODE_PAGEDOWN:
			return "Retroceder P" "\xe1" "gina";
		case Common::KEYCODE_INSERT:
			return "Insertar";
		case Common::KEYCODE_DELETE:
			return "Suprimir";
		case Common::KEYCODE_LALT:
			return "Alt Izquierdo";
		case Common::KEYCODE_RALT:
			return "Alt Derecho";

		default:
			return NULL;
		}
		return NULL;

	case T_RUSSIAN: // Russian key names
		switch (key) {
		case Common::KEYCODE_ESCAPE:
			return "Escape";
		case Common::KEYCODE_1:
			return "1";
		case Common::KEYCODE_2:
			return "2";
		case Common::KEYCODE_3:
			return "3";
		case Common::KEYCODE_4:
			return "4";
		case Common::KEYCODE_5:
			return "5";
		case Common::KEYCODE_6:
			return "6";
		case Common::KEYCODE_7:
			return "7";
		case Common::KEYCODE_8:
			return "8";
		case Common::KEYCODE_9:
			return "9";
		case Common::KEYCODE_0:
			return "0";
		case Common::KEYCODE_MINUS:
			return "\xCC\xE8\xED\xF3\xF1";
		case Common::KEYCODE_EQUALS:
			return "\xD0\xE0\xE2\xED\xEE";
		case Common::KEYCODE_BACKSPACE:
			return "Backspace";
		case Common::KEYCODE_TAB:
			return "Tab";
		case Common::KEYCODE_q:
			return "Q";
		case Common::KEYCODE_w:
			return "W";
		case Common::KEYCODE_e:
			return "E";
		case Common::KEYCODE_r:
			return "R";
		case Common::KEYCODE_t:
			return "T";
		case Common::KEYCODE_y:
			return "Y";
		case Common::KEYCODE_u:
			return "U";
		case Common::KEYCODE_i:
			return "I";
		case Common::KEYCODE_o:
			return "O";
		case Common::KEYCODE_p:
			return "P";
		case Common::KEYCODE_LEFTBRACKET:
			return "\xCB.\xEA\xE2.\xF1\xEA\xEE\xE1.";
		case Common::KEYCODE_RIGHTBRACKET:
			return "\xCF.\xEA\xE2.c\xEA\xEE\xE1.";
		case Common::KEYCODE_RETURN:
			return "\xC2\xE2\xEE\xE4";
		case Common::KEYCODE_LCTRL:
			return "\xCB\xE5\xE2\xFB\xE9 Ctrl";
		case Common::KEYCODE_a:
			return "A";
		case Common::KEYCODE_s:
			return "S";
		case Common::KEYCODE_d:
			return "D";
		case Common::KEYCODE_f:
			return "F";
		case Common::KEYCODE_g:
			return "G";
		case Common::KEYCODE_h:
			return "H";
		case Common::KEYCODE_j:
			return "J";
		case Common::KEYCODE_k:
			return "K";
		case Common::KEYCODE_l:
			return "L";
		case Common::KEYCODE_SEMICOLON:
			return "\xD2\xEE\xF7\xEA\xE0 \xF1 \xE7\xE0\xEF\xFF\xF2\xEE\xE9";
		case Common::KEYCODE_QUOTE:
			return "\xC0\xEF\xEE\xF1\xF2\xF0\xEE\xF4";
		case Common::KEYCODE_BACKQUOTE:
			return "\xD3\xE4\xE0\xF0\xE5\xED\xE8\xE5";
		case Common::KEYCODE_LSHIFT:
			return "\xCB\xE5\xE2\xFB\xE9 Shift";
		case Common::KEYCODE_BACKSLASH:
			return "\xD0\xE5\xF8\xE5\xF2\xEA\xE0";
		case Common::KEYCODE_z:
			return "Z";
		case Common::KEYCODE_x:
			return "X";
		case Common::KEYCODE_c:
			return "C";
		case Common::KEYCODE_v:
			return "V";
		case Common::KEYCODE_b:
			return "B";
		case Common::KEYCODE_n:
			return "N";
		case Common::KEYCODE_m:
			return "M";
		case Common::KEYCODE_COMMA:
			return "\xC7\xE0\xEF\xFF\xF2\xE0\xFF";
		case Common::KEYCODE_PERIOD:
			return "\xD2\xEE\xF7\xEA\xE0";
		case Common::KEYCODE_SLASH:
			return "\xD1\xEB\xFD\xF8";
		case Common::KEYCODE_RSHIFT:
			return "\xCF\xF0\xE0\xE2\xFB\xE9 Shift";
		case Common::KEYCODE_KP_MULTIPLY:
			return "\xC7\xE2.";
		case Common::KEYCODE_SPACE:
			return "\xCF\xF0\xEE\xE1\xE5\xEB";
		case Common::KEYCODE_CAPSLOCK:
			return "Caps Lock";
		case Common::KEYCODE_F1:
			return "F1";
		case Common::KEYCODE_F2:
			return "F2";
		case Common::KEYCODE_F3:
			return "F3";
		case Common::KEYCODE_F4:
			return "F4";
		case Common::KEYCODE_F5:
			return "F5";
		case Common::KEYCODE_F6:
			return "F6";
		case Common::KEYCODE_F7:
			return "F7";
		case Common::KEYCODE_F8:
			return "F8";
		case Common::KEYCODE_F9:
			return "F9";
		case Common::KEYCODE_F10:
			return "F10";
		case Common::KEYCODE_F11:
			return "F11";
		case Common::KEYCODE_F12:
			return "F12";
		case Common::KEYCODE_KP7:
			return "Num 7";
		case Common::KEYCODE_KP8:
			return "Num 8";
		case Common::KEYCODE_KP9:
			return "Num 9";
		case Common::KEYCODE_KP_MINUS:
			return "\xCC\xE8\xED\xF3\xF1 \xED\xE0 \xE4\xEE\xEF. \xEA\xEB ";
		case Common::KEYCODE_KP4:
			return "Num 4";
		case Common::KEYCODE_KP5:
			return "Num 5";
		case Common::KEYCODE_KP6:
			return "Num 6";
		case Common::KEYCODE_KP_PLUS:
			return "\xCF\xEB\xFE\xF1 \xED\xE0 \xE4\xEE\xEF. \xEA\xEB ";
		case Common::KEYCODE_KP1:
			return "Num 1";
		case Common::KEYCODE_KP2:
			return "Num 2";
		case Common::KEYCODE_KP3:
			return "Num 3";
		case Common::KEYCODE_KP0:
			return "Num 0";
		case Common::KEYCODE_KP_PERIOD:
			return "\xD2\xF7\xEA.";
		case Common::KEYCODE_KP_ENTER:
			return "Enter";
		case Common::KEYCODE_RCTRL:
			return "\xCF\xF0\xE0\xE2\xFB\xE9 Ctrl";
		case Common::KEYCODE_KP_DIVIDE:
			return "\xD1\xEB\xFD\xF8 \xED\xE0 \xE4\xEE\xEF. \xEA\xEB";
		case Common::KEYCODE_HOME:
			return "Home";
		case Common::KEYCODE_UP:
			return "\xD1\xF2\xF0\xE5\xEB\xEA\xE0 \xE2\xE2\xE5\xF0\xF5";
		case Common::KEYCODE_PAGEUP:
			return "Page Up";
		case Common::KEYCODE_LEFT:
			return "\xD1\xF2\xF0\xE5\xEB\xEA\xE0 \xE2\xEB\xE5\xE2\xEE";
		case Common::KEYCODE_RIGHT:
			return "\xD1\xF2\xF0\xE5\xEB\xEA\xE0 \xE2\xEF\xF0\xE0\xE2\xEE";
		case Common::KEYCODE_END:
			return "End";
		case Common::KEYCODE_DOWN:
			return "\xD1\xF2\xF0\xE5\xEB\xEA\xE0 \xE2\xED\xE8\xE7";
		case Common::KEYCODE_PAGEDOWN:
			return "Page Down";
		case Common::KEYCODE_INSERT:
			return "Insert";
		case Common::KEYCODE_DELETE:
			return "Delete";
		case Common::KEYCODE_LALT:
			return "\xCB\xE5\xE2\xFB\xE9 Alt";
		case Common::KEYCODE_RALT:
			return "\xCF\xF0\xE0\xE2\xFB\xE9 Alt";

		default:
			return NULL;
		}
		return NULL;

	case T_POLISH: // Polish key names
		switch (key) {
		case Common::KEYCODE_ESCAPE:
			return "Escape";
		case Common::KEYCODE_1:
			return "1";
		case Common::KEYCODE_2:
			return "2";
		case Common::KEYCODE_3:
			return "3";
		case Common::KEYCODE_4:
			return "4";
		case Common::KEYCODE_5:
			return "5";
		case Common::KEYCODE_6:
			return "6";
		case Common::KEYCODE_7:
			return "7";
		case Common::KEYCODE_8:
			return "8";
		case Common::KEYCODE_9:
			return "9";
		case Common::KEYCODE_0:
			return "0";
		case Common::KEYCODE_MINUS:
			return "Minus";
		case Common::KEYCODE_EQUALS:
			return "R\xF3wna sie";
		case Common::KEYCODE_BACKSPACE:
			return "Wstecz";
		case Common::KEYCODE_TAB:
			return "Tabulator";
		case Common::KEYCODE_q:
			return "Q";
		case Common::KEYCODE_w:
			return "W";
		case Common::KEYCODE_e:
			return "E";
		case Common::KEYCODE_r:
			return "R";
		case Common::KEYCODE_t:
			return "T";
		case Common::KEYCODE_y:
			return "Y";
		case Common::KEYCODE_u:
			return "U";
		case Common::KEYCODE_i:
			return "I";
		case Common::KEYCODE_o:
			return "O";
		case Common::KEYCODE_p:
			return "P";
		case Common::KEYCODE_LEFTBRACKET:
			return "Lewy nawias";
		case Common::KEYCODE_RIGHTBRACKET:
			return "Prawy nawias";
		case Common::KEYCODE_RETURN:
			return "Enter";
		case Common::KEYCODE_LCTRL:
			return "Lewy Ctrl";
		case Common::KEYCODE_a:
			return "A";
		case Common::KEYCODE_s:
			return "S";
		case Common::KEYCODE_d:
			return "D";
		case Common::KEYCODE_f:
			return "F";
		case Common::KEYCODE_g:
			return "G";
		case Common::KEYCODE_h:
			return "H";
		case Common::KEYCODE_j:
			return "J";
		case Common::KEYCODE_k:
			return "K";
		case Common::KEYCODE_l:
			return "L";
		case Common::KEYCODE_SEMICOLON:
			return "Przecinek";
		case Common::KEYCODE_QUOTE:
			return "Apostrof";
		case Common::KEYCODE_BACKQUOTE:
			return "Apostrof1";
		case Common::KEYCODE_LSHIFT:
			return "Lewy shift";
		case Common::KEYCODE_BACKSLASH:
			return "Krzyzyk";
		case Common::KEYCODE_z:
			return "Z";
		case Common::KEYCODE_x:
			return "X";
		case Common::KEYCODE_c:
			return "C";
		case Common::KEYCODE_v:
			return "V";
		case Common::KEYCODE_b:
			return "B";
		case Common::KEYCODE_n:
			return "N";
		case Common::KEYCODE_m:
			return "M";
		case Common::KEYCODE_COMMA:
			return "Precinek";
		case Common::KEYCODE_PERIOD:
			return "Kropka";
		case Common::KEYCODE_SLASH:
			return "Ukosnik";
		case Common::KEYCODE_RSHIFT:
			return "Prawy shift";
		case Common::KEYCODE_KP_MULTIPLY:
			return "Razy (klaw.num.)";
		case Common::KEYCODE_SPACE:
			return "Spacja";
		case Common::KEYCODE_CAPSLOCK:
			return "Caps Lock";
		case Common::KEYCODE_F1:
			return "F1";
		case Common::KEYCODE_F2:
			return "F2";
		case Common::KEYCODE_F3:
			return "F3";
		case Common::KEYCODE_F4:
			return "F4";
		case Common::KEYCODE_F5:
			return "F5";
		case Common::KEYCODE_F6:
			return "F6";
		case Common::KEYCODE_F7:
			return "F7";
		case Common::KEYCODE_F8:
			return "F8";
		case Common::KEYCODE_F9:
			return "F9";
		case Common::KEYCODE_F10:
			return "F10";
		case Common::KEYCODE_F11:
			return "F11";
		case Common::KEYCODE_F12:
			return "F12";
		case Common::KEYCODE_KP7:
			return "Num 7";
		case Common::KEYCODE_KP8:
			return "Num 8";
		case Common::KEYCODE_KP9:
			return "Num 9";
		case Common::KEYCODE_KP_MINUS:
			return "Minus (klaw. num.)";
		case Common::KEYCODE_KP4:
			return "Num 4";
		case Common::KEYCODE_KP5:
			return "Num 5";
		case Common::KEYCODE_KP6:
			return "Num 6";
		case Common::KEYCODE_KP_PLUS:
			return "Plus (klaw num.)";
		case Common::KEYCODE_KP1:
			return "Num 1";
		case Common::KEYCODE_KP2:
			return "Num 2";
		case Common::KEYCODE_KP3:
			return "Num 3";
		case Common::KEYCODE_KP0:
			return "Num 0";
		case Common::KEYCODE_KP_PERIOD:
			return "Kropka (klaw. num.)";
		case Common::KEYCODE_KP_ENTER:
			return "Numeryczny Enter";
		case Common::KEYCODE_RCTRL:
			return "Prawy Ctrl";
		case Common::KEYCODE_KP_DIVIDE:
			return "Podziel (klaw. num.)";
		case Common::KEYCODE_HOME:
			return "Home";
		case Common::KEYCODE_UP:
			return "Kursor do g\xF3ry";
		case Common::KEYCODE_PAGEUP:
			return "Page Up";
		case Common::KEYCODE_LEFT:
			return "Kursor w lewo";
		case Common::KEYCODE_RIGHT:
			return "Kursor w prawo";
		case Common::KEYCODE_END:
			return "End";
		case Common::KEYCODE_DOWN:
			return "Kursor do dolu";
		case Common::KEYCODE_PAGEDOWN:
			return "Page Down";
		case Common::KEYCODE_INSERT:
			return "Insert";
		case Common::KEYCODE_DELETE:
			return "Delete";
		case Common::KEYCODE_LALT:
			return "Lewy Alt";
		case Common::KEYCODE_RALT:
			return "Prawy Alt";

		default:
			return NULL;
		}
		return NULL;

	default: // English key names
		switch (key) {
		case Common::KEYCODE_ESCAPE:
			return "Escape";
		case Common::KEYCODE_1:
			return "1";
		case Common::KEYCODE_2:
			return "2";
		case Common::KEYCODE_3:
			return "3";
		case Common::KEYCODE_4:
			return "4";
		case Common::KEYCODE_5:
			return "5";
		case Common::KEYCODE_6:
			return "6";
		case Common::KEYCODE_7:
			return "7";
		case Common::KEYCODE_8:
			return "8";
		case Common::KEYCODE_9:
			return "9";
		case Common::KEYCODE_0:
			return "0";
		case Common::KEYCODE_MINUS:
			return "Minus";
		case Common::KEYCODE_EQUALS:
			return "Equals";
		case Common::KEYCODE_BACKSPACE:
			return "Backspace";
		case Common::KEYCODE_TAB:
			return "Tab";
		case Common::KEYCODE_q:
			return "Q";
		case Common::KEYCODE_w:
			return "W";
		case Common::KEYCODE_e:
			return "E";
		case Common::KEYCODE_r:
			return "R";
		case Common::KEYCODE_t:
			return "T";
		case Common::KEYCODE_y:
			return "Y";
		case Common::KEYCODE_u:
			return "U";
		case Common::KEYCODE_i:
			return "I";
		case Common::KEYCODE_o:
			return "O";
		case Common::KEYCODE_p:
			return "P";
		case Common::KEYCODE_LEFTBRACKET:
			return "Left Bracket";
		case Common::KEYCODE_RIGHTBRACKET:
			return "Right Bracket";
		case Common::KEYCODE_RETURN:
			return "Return";
		case Common::KEYCODE_LCTRL:
			return "Left Control";
		case Common::KEYCODE_a:
			return "A";
		case Common::KEYCODE_s:
			return "S";
		case Common::KEYCODE_d:
			return "D";
		case Common::KEYCODE_f:
			return "F";
		case Common::KEYCODE_g:
			return "G";
		case Common::KEYCODE_h:
			return "H";
		case Common::KEYCODE_j:
			return "J";
		case Common::KEYCODE_k:
			return "K";
		case Common::KEYCODE_l:
			return "L";
		case Common::KEYCODE_SEMICOLON:
			return "Semicolon";
		case Common::KEYCODE_QUOTE:
			return "Apostrophe";
		case Common::KEYCODE_BACKQUOTE:
			return "Grave";
		case Common::KEYCODE_LSHIFT:
			return "Left Shift";
		case Common::KEYCODE_BACKSLASH:
			return "Hash";
		case Common::KEYCODE_z:
			return "Z";
		case Common::KEYCODE_x:
			return "X";
		case Common::KEYCODE_c:
			return "C";
		case Common::KEYCODE_v:
			return "V";
		case Common::KEYCODE_b:
			return "B";
		case Common::KEYCODE_n:
			return "N";
		case Common::KEYCODE_m:
			return "M";
		case Common::KEYCODE_COMMA:
			return "Comma";
		case Common::KEYCODE_PERIOD:
			return "Full Stop";
		case Common::KEYCODE_SLASH:
			return "Slash";
		case Common::KEYCODE_RSHIFT:
			return "Right Shift";
		case Common::KEYCODE_KP_MULTIPLY:
			return "Num Multiply";
		case Common::KEYCODE_SPACE:
			return "Spacebar";
		case Common::KEYCODE_CAPSLOCK:
			return "Caps Lock";
		case Common::KEYCODE_F1:
			return "F1";
		case Common::KEYCODE_F2:
			return "F2";
		case Common::KEYCODE_F3:
			return "F3";
		case Common::KEYCODE_F4:
			return "F4";
		case Common::KEYCODE_F5:
			return "F5";
		case Common::KEYCODE_F6:
			return "F6";
		case Common::KEYCODE_F7:
			return "F7";
		case Common::KEYCODE_F8:
			return "F8";
		case Common::KEYCODE_F9:
			return "F9";
		case Common::KEYCODE_F10:
			return "F10";
		case Common::KEYCODE_F11:
			return "F11";
		case Common::KEYCODE_F12:
			return "F12";
		case Common::KEYCODE_KP7:
			return "Num 7";
		case Common::KEYCODE_KP8:
			return "Num 8";
		case Common::KEYCODE_KP9:
			return "Num 9";
		case Common::KEYCODE_KP_MINUS:
			return "Num Subtract";
		case Common::KEYCODE_KP4:
			return "Num 4";
		case Common::KEYCODE_KP5:
			return "Num 5";
		case Common::KEYCODE_KP6:
			return "Num 6";
		case Common::KEYCODE_KP_PLUS:
			return "Num Add";
		case Common::KEYCODE_KP1:
			return "Num 1";
		case Common::KEYCODE_KP2:
			return "Num 2";
		case Common::KEYCODE_KP3:
			return "Num 3";
		case Common::KEYCODE_KP0:
			return "Num 0";
		case Common::KEYCODE_KP_PERIOD:
			return "Num Decimal";
		case Common::KEYCODE_KP_ENTER:
			return "Enter";
		case Common::KEYCODE_RCTRL:
			return "Right Control";
		case Common::KEYCODE_KP_DIVIDE:
			return "Num Divide";
		case Common::KEYCODE_HOME:
			return "Home";
		case Common::KEYCODE_UP:
			return "Up Arrow";
		case Common::KEYCODE_PAGEUP:
			return "Page Up";
		case Common::KEYCODE_LEFT:
			return "Left Arrow";
		case Common::KEYCODE_RIGHT:
			return "Right Arrow";
		case Common::KEYCODE_END:
			return "End";
		case Common::KEYCODE_DOWN:
			return "Down Arrow";
		case Common::KEYCODE_PAGEDOWN:
			return "Page Down";
		case Common::KEYCODE_INSERT:
			return "Insert";
		case Common::KEYCODE_DELETE:
			return "Delete";
		case Common::KEYCODE_LALT:
			return "Left Alt";
		case Common::KEYCODE_RALT:
			return "Right Alt";

		default:
			return NULL;
		}
		return NULL;
	}
}

const char *GetJoystickName() {
	const char *name = NULL;

	if (ntJoystick.isValid) {
		name = "Joystick";
	}

	return name;
}

const char *GetButtonName(uint8 part) {
	// Unset
	if (part == 0xFF)
		return NULL;

	// Only have 32 buttons maximum
	if (part < 0 || part >= g_noButtons)
		return NULL;

	return g_buttons[part].name;
}

const char *GetAxisName(uint8 part) {
	char *name = NULL;

	if (part != 0xFF) {
		if (strcmp(g_axes[part].name, "") != 0)
			name = g_axes[part].name;
	}

	return name;
}

int32 Read_Joystick(uint8 part) {
	// Unset value
	if (part == 0xFF)
		return 0;

	if (!ntJoystick.isValid) {
		return 0;
	}

	// Axes we are configured to use
	int32 upVal = (up_joy == 0xFF) ? 0 : g_axes[up_joy].val;
	int32 downVal = (down_joy == 0xFF) ? 0 : g_axes[down_joy].val;
	int32 leftVal = (left_joy == 0xFF) ? 0 : g_axes[left_joy].val;
	int32 rightVal = (right_joy == 0xFF) ? 0 : g_axes[right_joy].val;

	// Ensure we report axes data with consistent sign
	if (upVal > 0)
		upVal *= -1;
	if (downVal < 0)
		downVal *= -1;
	if (leftVal > 0)
		leftVal *= -1;
	if (rightVal < 0)
		rightVal *= -1;

	// Are we returning a digital response
	if (currentJoystick == DIGITAL_JOYSTICK) {
		// Windows98 and NT (both sytem methods modify the same data)

		// Map analogue axis values to digital extremes
		if (part == X_AXIS) {
			if (rightVal > 20)
				return JOY_RIGHT;
			else if (leftVal < -20)
				return JOY_LEFT;
			else
				return 0;
		} else if (part == Y_AXIS) {
			if (downVal > 20)
				return JOY_DOWN;
			else if (upVal < -20)
				return JOY_UP;
			else
				return 0;
		} else {
			g_buttonsRepeats[part] = g_buttons[part].held_down;
			return g_buttons[part].held_down;
		}
	} else
	    // We want axis ranges please
	    if (currentJoystick == ANALOGUE_JOYSTICK) {
		// Get analogue axis values
		if (part == X_AXIS) {
			if ((rightVal > 20) || (leftVal < -20))
				return (leftVal + rightVal);
			else
				return 0;
		} else if (part == Y_AXIS) {
			if ((downVal > 20) || (upVal < -20))
				return (downVal + upVal);
			else
				return 0;
		} else {
			g_buttonsRepeats[part] = g_buttons[part].held_down;
			return g_buttons[part].held_down;
		}
	} else
		return 0;
}

int32 Read_Joystick_once(uint8 part) {
	// Fuck axis nonsense off
	if (part == X_AXIS || part == Y_AXIS)
		return Read_Joystick(part);

	// Unset value
	if (part == 0xFF)
		return 0;

	if (!ntJoystick.isValid) {
		return 0;
	}

	if (currentJoystick == ANALOGUE_JOYSTICK || currentJoystick == DIGITAL_JOYSTICK) {
		// Button still pressed so return false
		if (g_buttons[part].held_down && g_buttonsRepeats[part])
			return 0;

		// Set repeat
		g_buttonsRepeats[part] = g_buttons[part].held_down;

		// Return correct value
		return g_buttons[part].held_down;
	}

	return 0;
}

void SetDefaultJoystick() {
	SetDefaultKeys();

	up_joy = AXIS_2_NEG;
	down_joy = AXIS_2_POS;
	left_joy = AXIS_1_NEG;
	right_joy = AXIS_1_POS;

	// Can only really assume two buttons
	sidestep_button = 0xFF;
	run_button = 0xFF;
	crouch_button = 0xFF;
	interact_button = 0;
	arm_button = 0xFF;
	fire_button = 1;
	inventory_button = 0xFF;
	remora_button = 0xFF;
	pause_button = 0xFF;
}

void UnsetJoystickConfig() {
	// Unset all
	up_joy = 0xFF;
	down_joy = 0xFF;
	left_joy = 0xFF;
	right_joy = 0xFF;
	sidestep_button = 0xFF;
	run_button = 0xFF;
	crouch_button = 0xFF;
	interact_button = 0xFF;
	arm_button = 0xFF;
	fire_button = 0xFF;
	inventory_button = 0xFF;
	remora_button = 0xFF;
	pause_button = 0xFF;
}

uint8 GetJoystickButtonPress() {
	for (int32 i = 0; i < g_noButtons; i++) {
		if (Read_Joystick((uint8)i)) {
			return (uint8)i;
		}
	}

	// Perhaps we need to loop over axis settings here for directional config too

	return 0xFF;
}

uint8 GetJoystickAxisPress() {
	// Check for the first non-zero positioned axis
	for (int32 i = 0; i < MAX_AXES; i++) {
		// Ignore meaningless data if the axis has no name
		if (strcmp(g_axes[i].name, "") == 0)
			continue;

		// Return axis index
		if (g_axes[i].val > 20 || g_axes[i].val < -20)
			return (uint8)i;
	}

	return 0xFF;
}

} // End of namespace ICB
