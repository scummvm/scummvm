
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

#ifndef M4_ADV_R_ADV_PLAYER_H
#define M4_ADV_R_ADV_PLAYER_H

#include "common/serializer.h"
#include "m4/m4_types.h"
#include "m4/wscript/ws_machine.h"

namespace M4 {

#define MAX_PLYR_STRING_LEN     40

struct Player {
	int32 x = 0, y = 0;			// Player's current screen location
	int32 facing = 0;			// Player's current directional facing

	char verb[MAX_PLYR_STRING_LEN] = { 0 };
	char noun[MAX_PLYR_STRING_LEN] = { 0 };
	char prep[MAX_PLYR_STRING_LEN] = { 0 };
	char object[MAX_PLYR_STRING_LEN];
	char ws_asset_name[32] = { 0 };		// Name of the walker sprite series holder
	char ws_shadow_name[32] = { 0 };	// Name of the walker sprite series shadow holder
	int16 walker_type = 0;		// Type of walker (ripley, mei_chin, safari, etc.)
	int16 shadow_type = 0;		// Type of walker (ripley shadow, candleman shadow, etc.)

	// If he walks off edge, this holds the number of the room he goes to
	int32 walk_off_edge_to_room = 0;	// Player should walk off edge unless told otherwise

	// If False player won't walk - totally aborts a walk
	bool need_to_walk = false;			// Player needs to walk somewhere

	// If False player won't walk yet - walk suspended until TRUE
	bool ready_to_walk = false;			// Player is ready to perform that walk

	bool waiting_for_walk = false;

	// Default True for every room
	// Flag if accepting player input
	bool comm_allowed = false;

	// Means a parser command is ready. When apps programmer finishes
	// parsing command, the programmer must set this to FALSE. If this
	// doesn't get set, the command falls through to the error handling code.
	int32 command_ready = 0;

	// Default True, if set to FALSE walker disappears instantly, and vv
	bool walker_visible = false;		// Flag if player's sprite is visible

	// If True, then apps programmer must display look messages
	bool look_around = false;			// Flag for special "look around" command

	// Tested by the apps programmer to check if player has been here before
	bool been_here_before = false;

	// Walker sprites dumped on switching scenes if TRUE in room preload code
	bool walker_reload_palette = false;
	bool disable_hyperwalk = false;
	bool walker_loads_first = false;
	bool walker_in_this_scene = false;

	int32 walker_trigger = 0;

	int32 walk_x = 0, walk_y = 0;		// Where to walk to when player.ready_to_walk
	int32 walk_facing = 0;

	int32 click_x = 0, click_y = 0;

	void syncGame(Common::Serializer &s);
};

struct PlayerInfo {
	int32 x = 0, y = 0, facing = 0;
	int32 scale = 0, depth = 0;
	int32 camera_x = 0, camera_y = 0;

	void syncGame(Common::Serializer &s);
};

bool player_said(const char *w0, const char *w1 = nullptr, const char *w2 = nullptr);
bool player_said_any(const char *w0, const char *w1 = nullptr, const char *w2 = nullptr,
	const char *w3 = nullptr, const char *w4 = nullptr, const char *w5 = nullptr, const char *w6 = nullptr,
	const char *w7 = nullptr, const char *w8 = nullptr, const char *w9 = nullptr);

void player_inform_walker_new_scale(int32 frontY, int32 backY, int32 frontS, int32 backS);

bool player_load_series(const char *walkerName, const char *shadowName, bool load_palette);
void player_first_walk(int32 x1, int32 y1, int32 f1, int32 x2, int32 y2, int32 f2, bool enable_commands_at_destination);
void player_set_defaults();
void player_noun_becomes_verb(int32 spriteNum);

void player_hotspot_walk_override(int32 x, int32 y, int32 facing = -1, int32 trigger = -1);
void player_hotspot_walk_override_just_face(int32 facing, int32 trigger = -1);

bool player_commands_allowed();
PlayerInfo *player_update_info(machine *myWalker, PlayerInfo *player_info);
PlayerInfo *player_update_info();
void adv_kill_digi_between_rooms(bool true_or_false);

void player_set_facing_hotspot(int trigger = -1);
void player_set_facing_at(int x, int y, int trigger = -1);
int calc_facing(int x, int y);

} // End of namespace M4

#endif
