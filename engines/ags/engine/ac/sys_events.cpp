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

#include "core/platform.h"
#include "ac/common.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/keycode.h"
#include "ac/mouse.h"
#include "ac/sys_events.h"
#include "device/mousew32.h"
#include "platform/base/agsplatformdriver.h"
#include "ac/timer.h"

using namespace AGS::Shared;
using namespace AGS::Engine;

extern GameSetupStruct game;
extern GameState play;

extern volatile unsigned long globalTimerCounter;
extern int pluginSimulatedClick;
extern int displayed_room;
extern char check_dynamic_sprites_at_exit;

extern void domouse(int str);
extern int mgetbutton();
extern int misbuttondown(int buno);

int mouse_z_was = 0;

int ags_kbhit() {
	return keypressed();
}

int ags_iskeypressed(int keycode) {
	if (keycode >= 0 && keycode < __allegro_KEY_MAX) {
		return key[keycode] != 0;
	}
	return 0;
}

int ags_misbuttondown(int but) {
	return misbuttondown(but);
}

int ags_mgetbutton() {
	int result;

	if (pluginSimulatedClick > NONE) {
		result = pluginSimulatedClick;
		pluginSimulatedClick = NONE;
	} else {
		result = mgetbutton();
	}
	return result;
}

void ags_domouse(int what) {
	// do mouse is "update the mouse x,y and also the cursor position", unless DOMOUSE_NOCURSOR is set.
	if (what == DOMOUSE_NOCURSOR)
		mgetgraphpos();
	else
		domouse(what);
}

int ags_check_mouse_wheel() {
	int result = 0;
	if ((mouse_z != mouse_z_was) && (game.options[OPT_MOUSEWHEEL] != 0)) {
		if (mouse_z > mouse_z_was)
			result = 1;
		else
			result = -1;
		mouse_z_was = mouse_z;
	}
	return result;
}

int ags_getch() {
	const int read_key_value = readkey();
	int gott = read_key_value;
	const int scancode = ((gott >> 8) & 0x00ff);
	const int ascii = (gott & 0x00ff);

	bool is_extended = (ascii == EXTENDED_KEY_CODE);
	// On macos, the extended keycode is the ascii character '?' or '\0' if alt-key
	// so check it's not actually the character '?'
#if AGS_PLATFORM_OS_MACOS && ! AGS_PLATFORM_OS_IOS
	is_extended = is_extended || ((ascii == EXTENDED_KEY_CODE_MACOS) && (scancode != __allegro_KEY_SLASH));
#endif

	/*  char message[200];
	sprintf(message, "Scancode: %04X", gott);
	Debug::Printf(message);*/

	/*if ((scancode >= KEY_0_PAD) && (scancode <= KEY_9_PAD)) {
	// fix numeric pad keys if numlock is off (allegro 4.2 changed this behaviour)
	if ((key_shifts & KB_NUMLOCK_FLAG) == 0)
	gott = (gott & 0xff00) | EXTENDED_KEY_CODE;
	}*/

	if (gott == READKEY_CODE_ALT_TAB) {
		// Alt+Tab, it gets stuck down unless we do this
		gott = AGS_KEYCODE_ALT_TAB;
	}
#if AGS_PLATFORM_OS_MACOS
	else if (scancode == __allegro_KEY_BACKSPACE) {
		gott = eAGSKeyCodeBackspace;
	}
#endif
	else if (is_extended) {

		// I believe we rely on a lot of keys being converted to ASCII, which is why
		// the complete scan code list is not here.

		switch (scancode) {
		case __allegro_KEY_F1 :
			gott = eAGSKeyCodeF1 ;
			break;
		case __allegro_KEY_F2 :
			gott = eAGSKeyCodeF2 ;
			break;
		case __allegro_KEY_F3 :
			gott = eAGSKeyCodeF3 ;
			break;
		case __allegro_KEY_F4 :
			gott = eAGSKeyCodeF4 ;
			break;
		case __allegro_KEY_F5 :
			gott = eAGSKeyCodeF5 ;
			break;
		case __allegro_KEY_F6 :
			gott = eAGSKeyCodeF6 ;
			break;
		case __allegro_KEY_F7 :
			gott = eAGSKeyCodeF7 ;
			break;
		case __allegro_KEY_F8 :
			gott = eAGSKeyCodeF8 ;
			break;
		case __allegro_KEY_F9 :
			gott = eAGSKeyCodeF9 ;
			break;
		case __allegro_KEY_F10 :
			gott = eAGSKeyCodeF10 ;
			break;
		case __allegro_KEY_F11 :
			gott = eAGSKeyCodeF11 ;
			break;
		case __allegro_KEY_F12 :
			gott = eAGSKeyCodeF12 ;
			break;

		case __allegro_KEY_INSERT :
			gott = eAGSKeyCodeInsert ;
			break;
		case __allegro_KEY_DEL :
			gott = eAGSKeyCodeDelete ;
			break;
		case __allegro_KEY_HOME :
			gott = eAGSKeyCodeHome ;
			break;
		case __allegro_KEY_END :
			gott = eAGSKeyCodeEnd ;
			break;
		case __allegro_KEY_PGUP :
			gott = eAGSKeyCodePageUp ;
			break;
		case __allegro_KEY_PGDN :
			gott = eAGSKeyCodePageDown ;
			break;
		case __allegro_KEY_LEFT :
			gott = eAGSKeyCodeLeftArrow ;
			break;
		case __allegro_KEY_RIGHT :
			gott = eAGSKeyCodeRightArrow ;
			break;
		case __allegro_KEY_UP :
			gott = eAGSKeyCodeUpArrow ;
			break;
		case __allegro_KEY_DOWN :
			gott = eAGSKeyCodeDownArrow ;
			break;

		case __allegro_KEY_0_PAD :
			gott = eAGSKeyCodeInsert ;
			break;
		case __allegro_KEY_1_PAD :
			gott = eAGSKeyCodeEnd ;
			break;
		case __allegro_KEY_2_PAD :
			gott = eAGSKeyCodeDownArrow ;
			break;
		case __allegro_KEY_3_PAD :
			gott = eAGSKeyCodePageDown ;
			break;
		case __allegro_KEY_4_PAD :
			gott = eAGSKeyCodeLeftArrow ;
			break;
		case __allegro_KEY_5_PAD :
			gott = eAGSKeyCodeNumPad5 ;
			break;
		case __allegro_KEY_6_PAD :
			gott = eAGSKeyCodeRightArrow ;
			break;
		case __allegro_KEY_7_PAD :
			gott = eAGSKeyCodeHome ;
			break;
		case __allegro_KEY_8_PAD :
			gott = eAGSKeyCodeUpArrow ;
			break;
		case __allegro_KEY_9_PAD :
			gott = eAGSKeyCodePageUp ;
			break;
		case __allegro_KEY_DEL_PAD :
			gott = eAGSKeyCodeDelete ;
			break;

		default:
			// no meaningful mappings
			// this is how we accidentally got the alt-key mappings
			gott = scancode + AGS_EXT_KEY_SHIFT;
		}
	} else {
		// this includes ascii characters and ctrl-A-Z
		gott = ascii;
	}

	// Alt+X, abort (but only once game is loaded)
	if ((gott == play.abort_key) && (displayed_room >= 0)) {
		check_dynamic_sprites_at_exit = 0;
		quit("!|");
	}

	//sprintf(message, "Keypress: %d", gott);
	//Debug::Printf(message);

	return gott;
}

void ags_clear_input_buffer() {
	while (ags_kbhit()) ags_getch();
	while (mgetbutton() != NONE);
}

void ags_wait_until_keypress() {
	while (!ags_kbhit()) {
		platform->YieldCPU();
	}
	ags_getch();
}
