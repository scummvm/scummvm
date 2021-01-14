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

#include "ags/shared/core/platform.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/keycode.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/device/mousew32.h"
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/engine/ac/timer.h"
#include "ags/ags.h"

namespace AGS3 {

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
	if (keycode >= 0 && keycode < Common::KEYCODE_LAST) {
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
	const Common::KeyState keyState = readkey();
	int gott;

	// I believe we rely on a lot of keys being converted to ASCII, which is why
	// the complete scan code list is not here.

	switch (keyState.keycode) {
	case Common::KEYCODE_F1:
		gott = eAGSKeyCodeF1;
		break;
	case Common::KEYCODE_F2:
		gott = eAGSKeyCodeF2;
		break;
	case Common::KEYCODE_F3:
		gott = eAGSKeyCodeF3;
		break;
	case Common::KEYCODE_F4:
		gott = eAGSKeyCodeF4;
		break;
	case Common::KEYCODE_F5:
		gott = eAGSKeyCodeF5;
		break;
	case Common::KEYCODE_F6:
		gott = eAGSKeyCodeF6;
		break;
	case Common::KEYCODE_F7:
		gott = eAGSKeyCodeF7;
		break;
	case Common::KEYCODE_F8:
		gott = eAGSKeyCodeF8;
		break;
	case Common::KEYCODE_F9:
		gott = eAGSKeyCodeF9;
		break;
	case Common::KEYCODE_F10:
		gott = eAGSKeyCodeF10;
		break;
	case Common::KEYCODE_F11:
		gott = eAGSKeyCodeF11;
		break;
	case Common::KEYCODE_F12:
		gott = eAGSKeyCodeF12;
		break;

	case Common::KEYCODE_INSERT:
	case Common::KEYCODE_KP0:
		gott = eAGSKeyCodeInsert;
		break;
	case Common::KEYCODE_DELETE:
	case Common::KEYCODE_KP_PERIOD:
		gott = eAGSKeyCodeDelete;
		break;
	case Common::KEYCODE_HOME:
	case Common::KEYCODE_KP7:
		gott = eAGSKeyCodeHome;
		break;
	case Common::KEYCODE_END:
	case Common::KEYCODE_KP1:
		gott = eAGSKeyCodeEnd;
		break;
	case Common::KEYCODE_PAGEUP:
	case Common::KEYCODE_KP9:
		gott = eAGSKeyCodePageUp;
		break;
	case Common::KEYCODE_PAGEDOWN:
	case Common::KEYCODE_KP3:
		gott = eAGSKeyCodePageDown;
		break;
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_KP4:
		gott = eAGSKeyCodeLeftArrow;
		break;
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_KP6:
		gott = eAGSKeyCodeRightArrow;
		break;
	case Common::KEYCODE_UP:
	case Common::KEYCODE_KP8:
		gott = eAGSKeyCodeUpArrow;
		break;
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP2:
		gott = eAGSKeyCodeDownArrow;
		break;

	default:
		if (keyState.flags & (Common::KBD_ALT | Common::KBD_CTRL) &&
			keyState.keycode >= Common::KEYCODE_a && keyState.keycode <= Common::KEYCODE_z)
			// no meaningful mappings
			// this is how we accidentally got the alt-key mappings
			gott = AGS_EXT_KEY_SHIFT + (keyState.keycode - Common::KEYCODE_a) + 1;
		else
			gott = keyState.ascii;
		break;
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
	while (!SHOULD_QUIT && ags_kbhit())
		ags_getch();

	while (!SHOULD_QUIT && mgetbutton() != NONE) {
	}
}

void ags_wait_until_keypress() {
	while (!SHOULD_QUIT && !ags_kbhit()) {
		platform->YieldCPU();
	}

	ags_getch();
}

} // namespace AGS3
