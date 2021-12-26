/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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

bool8 keyboard_buf_scancodes[Common::KEYCODE_LAST];
bool8 repeats_scancodes[Common::KEYCODE_LAST];

void Init_direct_input() {
	SetDefaultKeys();
}

void setKeyState(Common::KeyCode key, bool pressed) {
	if (key >= Common::KEYCODE_LAST)
		return;

	keyboard_buf_scancodes[key] = pressed;
}

uint32 Get_DI_key_press() {
	for (uint32 i = 0; i < Common::KEYCODE_LAST; i++) {
		if (Read_DI_once_keys(i)) {
			return i;
		}
	}

	return 0;
}

void Clear_DI_key_buffer() {
	for (uint32 i = 0; i < Common::KEYCODE_LAST; i++) {
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

	if (key >= Common::KEYCODE_LAST)
		return FALSE8;

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

	if (key >= Common::KEYCODE_LAST)
		return FALSE8;

	// set repeat
	repeats_scancodes[key] = keyboard_buf_scancodes[key];

	return (repeats_scancodes[key]);
}

bool8 DI_key_waiting() {
	for (uint32 i = 0; i < Common::KEYCODE_LAST; i++) {
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
	remora_key = Common::KEYCODE_r;
	crouch_key = Common::KEYCODE_x;
	sidestep_key = Common::KEYCODE_LSHIFT;
	run_key = Common::KEYCODE_z;
	up_key = Common::KEYCODE_UP;
	down_key = Common::KEYCODE_DOWN;
	left_key = Common::KEYCODE_LEFT;
	right_key = Common::KEYCODE_RIGHT;
	pause_key = Common::KEYCODE_ESCAPE;
}

} // End of namespace ICB
