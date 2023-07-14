/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "m4/adv_r/adv_control.h"
#include "m4/adv_r/adv_interface.h"
#include "m4/core/errors.h"
#include "m4/mem/memman.h"
#include "m4/wscript/ws_machine.h"
#include "m4/vars.h"

namespace M4 {

bool kernel_section_startup() {
	_G(game).previous_section = _G(game).section_id;
	_G(game).section_id = _G(game).new_section;

	return true;
}

void player_set_commands_allowed(bool t_or_f) {
	_G(set_commands_allowed_since_last_checked) = true;
	_G(player).comm_allowed = t_or_f;

	if (t_or_f) {
		// OK to do something
		mouse_set_sprite(kArrowCursor);
		intr_cancel_sentence();
		track_hotspots_refresh();

	} else {
		// Hour glass
		_GI().showWaitCursor();
	}
}

void game_pause(bool flag) {
	if (flag) {
		_G(kernel).pause = true;
		PauseEngines();
	} else {
		_G(kernel).pause = false;
		UnpauseEngines();
	}
}

void player_hotspot_walk_override(int32 x, int32 y, int32 facing, int32 trigger) {
	_G(player).walk_x = x;
	_G(player).walk_y = y;
	_G(player).walk_facing = facing;
	_G(player).walker_trigger = trigger;
	_G(player).ready_to_walk = true;
	_G(player).need_to_walk = true;
}

void player_hotspot_walk_override_just_face(int32 facing, int32 trigger) {
	player_update_info(_G(my_walker), &_G(player_info));
	player_hotspot_walk_override(_G(player_info).x, _G(player_info).y, facing, trigger);
}

void adv_kill_digi_between_rooms(bool true_or_false) {
	_G(shut_down_digi_tracks_between_rooms) = true_or_false;
}


} // End of namespace M4
