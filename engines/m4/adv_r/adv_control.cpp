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
#include "m4/core/mouse.h"
#include "m4/gui/hotkeys.h"
#include "m4/gui/gui_vmng.h"
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
		pauseEngines();
	} else {
		_G(kernel).pause = false;
		unpauseEngines();
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

bool this_is_a_walkcode(int32 x, int32 y) {
	if (!_G(screenCodeBuff))
		return false;

	Buffer *walkCodes;
	byte *ptr;
	bool result;

	walkCodes = _G(screenCodeBuff)->get_buffer();
	if (!walkCodes)
		return false;

	// Verify params
	if (x < 0 || y < 0 || x >= walkCodes->w || y >= walkCodes->h)
		return false;

	ptr = gr_buffer_pointer(walkCodes, x, y);
	result = ((*ptr) & 0x10) ? true : false;

	_G(screenCodeBuff)->release();
	return result;
}

int32 get_screen_depth(int32 x, int32 y) {
	Buffer *walkCodes;
	byte *ptr;
	int32 myDepth;

	if (!_G(screenCodeBuff))
		return 0;

	walkCodes = _G(screenCodeBuff)->get_buffer();
	if (!walkCodes) {
		return 0;
	}

	// Verify params
	if (x < 0 || y < 0 || x >= walkCodes->w || y >= walkCodes->h) {
		return -1;
	}

	ptr = gr_buffer_pointer(walkCodes, x, y);
	myDepth = (*ptr) & 0x0f;

	_G(screenCodeBuff)->release();
	return myDepth;
}

int32 get_screen_color(int32 x, int32 y) {
	Buffer *game_buff;
	byte *ptr;
	int32 myColor;

	game_buff = _G(gameDrawBuff)->get_buffer();
	if (!game_buff) {
		return -1;
	}

	//verify params
	if (x < 0 || y < 0 || x >= game_buff->w || y >= game_buff->h) {
		return -1;
	}

	ptr = gr_buffer_pointer(game_buff, x, y);
	myColor = *ptr;

	_G(gameDrawBuff)->release();
	return myColor;
}

void update_mouse_pos_dialog() {
	int32 status;

	ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
	assert(game_buff_ptr);

	if (_G(my_walker) != nullptr) {
		if (!_G(my_walker)->myAnim8)
			error_show(FL, 'W:-(');
		player_get_info();
	}

	char tempStr1[MAX_STRING_LEN], tempStr2[MAX_STRING_LEN];

	Common::sprintf_s(tempStr1, "%d  From: %d", _G(game).room_id, _G(game).previous_room);
	Dialog_Change_Item_Prompt(_G(mousePosDialog), tempStr1, nullptr, 1);

	int32 xxx = _G(MouseState).CursorColumn;
	int32 yyy = _G(MouseState).CursorRow;

	int32 scrnDepth = get_screen_depth(xxx - game_buff_ptr->x1, yyy - game_buff_ptr->y1);
	int32 palColor = get_screen_color(xxx - game_buff_ptr->x1, yyy - game_buff_ptr->y1);

	if (this_is_a_walkcode(xxx - game_buff_ptr->x1, yyy - game_buff_ptr->y1)) {
		Common::sprintf_s(tempStr1, "WC %d, %d  PAL: %d", xxx, yyy, palColor);
		Common::sprintf_s(tempStr2, "WC %d, %d  D: %d", xxx - game_buff_ptr->x1, yyy - game_buff_ptr->y1, scrnDepth);
	} else {
		Common::sprintf_s(tempStr1, "   %d, %d  PAL: %d", xxx, yyy, palColor);
		Common::sprintf_s(tempStr2, "   %d, %d  D: %d", xxx - game_buff_ptr->x1, yyy - game_buff_ptr->y1, scrnDepth);
	}
	Dialog_Change_Item_Prompt(_G(mousePosDialog), tempStr1, nullptr, 2);
	Dialog_Change_Item_Prompt(_G(mousePosDialog), tempStr2, nullptr, 3);

	if (this_is_a_walkcode(_G(player_info).x, _G(player_info).y)) {
		Common::sprintf_s(tempStr1, "WC %d, %d", _G(player_info).x + game_buff_ptr->x1, _G(player_info).y + game_buff_ptr->y1);
		Common::sprintf_s(tempStr2, "WC %d, %d", _G(player_info).x, _G(player_info).y);
	} else {
		Common::sprintf_s(tempStr1, "  %d, %d", _G(player_info).x + game_buff_ptr->x1, _G(player_info).y + game_buff_ptr->y1);
		Common::sprintf_s(tempStr2, "  %d, %d", _G(player_info).x, _G(player_info).y);
	}
	Dialog_Change_Item_Prompt(_G(mousePosDialog), tempStr1, nullptr, 4);
	Dialog_Change_Item_Prompt(_G(mousePosDialog), tempStr2, nullptr, 5);

	Common::sprintf_s(tempStr1, "%d", _G(player_info).scale);
	Dialog_Change_Item_Prompt(_G(mousePosDialog), tempStr1, nullptr, 6);

	Common::sprintf_s(tempStr1, "%x", _G(player_info).depth);
	Dialog_Change_Item_Prompt(_G(mousePosDialog), tempStr1, nullptr, 7);

	Common::sprintf_s(tempStr1, "%d, %d", game_buff_ptr->x1, game_buff_ptr->y1);
	Dialog_Change_Item_Prompt(_G(mousePosDialog), tempStr1, nullptr, 8);

	Common::sprintf_s(tempStr1, "%d", _G(player_info).facing);
	Dialog_Change_Item_Prompt(_G(mousePosDialog), tempStr1, nullptr, 10);
}

} // End of namespace M4
