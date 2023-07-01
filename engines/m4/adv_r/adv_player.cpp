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

#include "m4/adv_r/adv_player.h"
#include "m4/adv_r/adv_walk.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/gui/gui_vmng_core.h"
#include "m4/globals.h"

namespace M4 {

bool player_said(const char *w0, const char *w1, const char *w2) {
	const char *ptrs[3] = { w0, w1, w2 };

	for (int i = 0; i < 3; i++) {
		if (ptrs[i])
			if ((scumm_strnicmp(_G(player).noun, ptrs[i], MAX_PLYR_STRING_LEN))
				&& (scumm_strnicmp(_G(player).object, ptrs[i], MAX_PLYR_STRING_LEN))
				&& (scumm_strnicmp(_G(player).verb, ptrs[i], MAX_PLYR_STRING_LEN)))
				return false;
	}

	return true;
}

bool player_said_any(const char *w0, const char *w1, const char *w2,
		const char *w3, const char *w4, const char *w5, const char *w6,
		const char *w7, const char *w8, const char *w9) {
	const char *ptrs[10] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9 };

	for (int i = 0; i < 10; i++) {
		if (ptrs[i]) {
			if (!scumm_strnicmp(_G(player).noun, ptrs[i], MAX_PLYR_STRING_LEN))
				return true;
			if (!scumm_strnicmp(_G(player).object, ptrs[i], MAX_PLYR_STRING_LEN))
				return true;
			if (!scumm_strnicmp(_G(player).verb, ptrs[i], MAX_PLYR_STRING_LEN))
				return true;
		}
	}

	return false;
}


void player_inform_walker_new_scale(int32 frontY, int32 backY, int32 frontS, int32 backS) {
	_G(globals)[GLB_MIN_Y] = backY << 16;
	_G(globals)[GLB_MAX_Y] = frontY << 16;
	_G(globals)[GLB_MIN_SCALE] = FixedDiv(backS << 16, 100 << 16);
	_G(globals)[GLB_MAX_SCALE] = FixedDiv(frontS << 16, 100 << 16);
	if (_G(globals)[GLB_MIN_Y] == _G(globals)[GLB_MAX_Y])
		_G(globals)[GLB_SCALER] = 0;
	else
		_G(globals)[GLB_SCALER] = FixedDiv(_G(globals)[GLB_MAX_SCALE] - _G(globals)[GLB_MIN_SCALE], _G(globals)[GLB_MAX_Y] - _G(globals)[GLB_MIN_Y]);
}


// this routine must also load shadow animations

bool player_load_series(const char *walkerName, const char *shadowName, bool load_palette) {
	int i;
	int32 thatRoomCode;
	char assetPath[MAX_FILENAME_SIZE];
	char *tempPtr;

	//LOAD WALKER
	db_rmlst_get_asset_room_path(walkerName, assetPath, &thatRoomCode);
	tempPtr = strrchr(assetPath, '.');
	if (!tempPtr)
		return false;

	tempPtr--;
	if ((*tempPtr < '0') || (*tempPtr > '9'))
		return false;

	for (i = 1; i <= 5; i++) {
		*tempPtr = (char)((int32)'0' + i);
		//env_get_path(fullPath, thatRoomCode, assetPath);
		AddWSAssetCELS(assetPath, i - 1, load_palette ? _G(master_palette) : nullptr);
	}

	//LOAD WALKER SHADOW
	db_rmlst_get_asset_room_path(shadowName, assetPath, &thatRoomCode);
	tempPtr = strrchr(assetPath, '.');
	if (!tempPtr)
		return false;

	tempPtr--;
	if ((*tempPtr < '0') || (*tempPtr > '9'))
		return false;

	for (i = 1; i <= 5; i++) {
		*tempPtr = (char)((int32)'0' + i);
		AddWSAssetCELS(assetPath, i + 4, nullptr);
	}

	SendWSMessage(0, 0, nullptr, 6, nullptr, 1);		// Hash 6 is the shadow machine
	return true;
}

void player_first_walk(int32 x1, int32 y1, int32 /*f1*/, int32 x2, int32 y2, int32 /*f2*/, bool /*enable_commands_at_destination*/) {
	player_demand_location(x1, y1);
	_G(player).ready_to_walk = true;
	_G(player).need_to_walk = true;
	_G(player).walk_x = x2;
	_G(player).walk_y = y2;
}

void player_set_defaults() {
	_G(player).walker_visible = false;
	_G(player).disable_hyperwalk = false;
	_G(player).walker_loads_first = true;
	_G(player).walker_reload_palette = true;
	_G(player).walker_in_this_scene = true;
}

bool player_commands_allowed() {
	return _G(player).comm_allowed;
}

PlayerInfo *player_update_info(machine *myWalker, PlayerInfo *player_info) {
	if (!myWalker)
		return nullptr;

	ws_get_walker_info(myWalker, &player_info->x, &player_info->y,
		&player_info->scale, &player_info->depth, &player_info->facing);

	int32 status;
	ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
	player_info->camera_x = game_buff_ptr->x1;
	player_info->camera_y = game_buff_ptr->y1;
	return player_info;
}

} // End of namespace M4
