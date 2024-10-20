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
#include "m4/vars.h"

namespace M4 {

void Player::syncGame(Common::Serializer &s) {
	s.skip(12);
	s.syncBytes((byte *)verb, MAX_PLYR_STRING_LEN);
	s.syncBytes((byte *)noun, MAX_PLYR_STRING_LEN);
	s.syncBytes((byte *)object, MAX_PLYR_STRING_LEN);
	s.syncBytes((byte *)prep, MAX_PLYR_STRING_LEN);
	s.skip(64);

	s.syncAsSint16LE(walker_type);
	s.syncAsSint16LE(shadow_type);
	s.skip(4);
	s.syncAsByte(need_to_walk);
	s.syncAsByte(ready_to_walk);
	s.syncAsByte(waiting_for_walk);
	s.syncAsByte(comm_allowed);
	s.syncAsUint32LE(command_ready);
	s.syncAsByte(walker_visible);
	s.skip(1);
	s.syncAsByte(been_here_before);
	s.syncAsByte(walker_reload_palette);
	s.syncAsByte(disable_hyperwalk);
	s.syncAsByte(walker_loads_first);
	s.syncAsByte(walker_in_this_scene);
	s.syncAsSint32LE(walker_trigger);
	s.syncAsSint32LE(walk_x);
	s.syncAsSint32LE(walk_y);
	s.syncAsSint32LE(walk_facing);
	s.syncAsSint32LE(click_x);
	s.syncAsSint32LE(click_y);
}

void Player::resetWalk() {
	need_to_walk = false;
	ready_to_walk = true;
	waiting_for_walk = false;
}

void PlayerInfo::syncGame(Common::Serializer &s) {
	s.syncAsSint32LE(x);
	s.syncAsSint32LE(y);
	s.syncAsSint32LE(facing);
	s.syncAsSint32LE(scale);
	s.syncAsSint32LE(depth);
	s.syncAsSint32LE(camera_x);
	s.syncAsSint32LE(camera_y);
}


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


// This routine must also load shadow animations

bool player_load_series(const char *walkerName, const char *shadowName, bool load_palette) {
	int i;
	int32 thatRoomCode;
	char assetPath[MAX_FILENAME_SIZE];
	char *tempPtr;

	// Load walker
	db_rmlst_get_asset_room_path(walkerName, assetPath, &thatRoomCode);
	tempPtr = strrchr(assetPath, '.');
	if (!tempPtr)
		return false;

	tempPtr--;
	if ((*tempPtr < '0') || (*tempPtr > '9'))
		return false;

	for (i = 1; i <= 5; i++) {
		*tempPtr = (char)((int32)'0' + i);
		AddWSAssetCELS(assetPath, i - 1, load_palette ? _G(master_palette) : nullptr);
	}

	// Load walker shadow
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

	sendWSMessage(0, 0, nullptr, 6, nullptr, 1);		// Hash 6 is the shadow machine
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

PlayerInfo *player_update_info() {
	return player_update_info(_G(my_walker), &_G(player_info));
}

void player_set_facing_hotspot(int trigger) {
	player_set_facing_at(_G(player).click_x, _G(player).click_y, trigger);
}

void player_set_facing_at(int x, int y, int trigger) {
	player_hotspot_walk_override_just_face(calc_facing(x, y), trigger);
}

int calc_facing(int x, int y) {
	player_update_info(_G(my_walker), &_G(player_info));

	if (!x) {
		return -_G(player_info).y < -y;
	} else {
		double slope = (double)(y - _G(player_info).y) / (double)(x - _G(player_info).x);
		term_message("click (%d,%d)  player (%d,%d)  slope = %f",
			x, -y, _G(player_info).x, -_G(player_info).y);

		if (_G(player_info).x < x) {
			if (slope >= 1.25)
				return 1;
			else if (slope >= 0.1)
				return 2;
			else if (slope >= -0.1)
				return 3;
			else if (slope >= -0.4)
				return 4;
			else
				return 5;
		} else {
			if (slope >= 0.4)
				return 7;
			else if (slope >= 0.1)
				return 8;
			else if (slope >= -0.1)
				return 9;
			else if (slope >= -1.25)
				return 10;
			else
				return 11;
		}
	}
}

} // End of namespace M4
