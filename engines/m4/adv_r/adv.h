
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

#ifndef M4_ADV_R_ADV_H
#define M4_ADV_R_ADV_H

#include "m4/m4_types.h"

namespace M4 {

#define MAX_SCENES 180
#define GLB_SHARED_VARS 512
#define MAXRAILNODES 32
#define PATH_END     0xffff
#define MAX_PLYR_STRING_LEN     40

#define STR_FADEPAL "fade palette"
#define STR_RAILNODE "rail node"
#define STR_HOT_SPOT "hot spot"
#define STR_PATH_NODE "path node"

enum {
	INSTALL_SOUND_DRIVERS    =    1,
	INSTALL_PLAYER_BEEN_INIT =    2,
	INSTALL_RAIL_SYSTEM      =    4,
	INSTALL_INVENTORY_SYSTEM =    8,
	INSTALL_INVERSE_PALETTE  = 0x10,
	INSTALL_MININUM          =    0,
	INSTALL_ALL              = 0x1f
};

struct HotSpotRec {
	int32 ul_x = 0, ul_y = 0, lr_x = 0, lr_y = 0;	// Hotspot screen coordinates
	int32 feet_x = 0, feet_y = 0;			// Walk-to target for player
	int8 facing = 0;						// Direction player should face
	bool active = false;					// Flag if hotspot is active
	char cursor_number = 0;					// Mouse cursor number
	char syntax = 0;						// Word syntax
	int32 vocabID = 0, verbID = 0;			// ids of name and verb
	char *vocab = nullptr;					// Vocabulary name of hotspot
	char *verb = nullptr;					// Vocabulary default verb name
	char *prep = nullptr;					// Preposition
	char *sprite = nullptr;					// Sprite name
	int16 hash = 0;							// woodscript sprite hash (runtime only)
	HotSpotRec *next = nullptr;
};

struct pathNode {
	pathNode *next = nullptr;
	byte nodeID = 0;
};

struct noWalkRect {
	noWalkRect *next = nullptr;
	noWalkRect *prev = nullptr;
	int32 x1 = 0, y1 = 0, x2 = 0, y2 = 0;
	int32 alternateWalkToNode = 0;
	int32 walkAroundNode1 = 0;
	int32 walkAroundNode2 = 0;
	int32 walkAroundNode3 = 0;
	int32 walkAroundNode4 = 0;
};

struct SceneDef {
	char art_base[MAX_FILENAME_SIZE] = { 0 };
	char picture_base[MAX_FILENAME_SIZE] = { 0 };

	int32 num_hotspots = 0;								// # of hotspots
	HotSpotRec *hotspots = nullptr;
	int32 num_parallax = 0;
	HotSpotRec *parallax = nullptr;
	int32 num_props = 0;
	HotSpotRec *props = nullptr;

	int32 front_y = 400, back_y = 100;				// Player scaling baselines
	int32 front_scale = 100, back_scale = 85;		// Player scaling factors

	int16 depth_table[16] = { 0 };					// Player sprite depth table
	int32 numRailNodes = 0;							// # of rails
};

struct Player {
	int32 x = 0, y = 0;			// Player's current screen location
	int32 facing = 0;			// Player's current directional facing

	char verb[MAX_PLYR_STRING_LEN] = { 0 };
	char noun[MAX_PLYR_STRING_LEN] = { 0 };
	char prep[MAX_PLYR_STRING_LEN] = { 0 };
	char object[MAX_PLYR_STRING_LEN] = { 0 };
	char ws_asset_name[32] = { 0 };
	char ws_shadow_name[32] = { 0 };
	int16 walker_type = 0;			// Type of walker (ripley, mei_chin, safari, etc.)
	int16 shadow_type = 0;			// Type of walker (ripley shadow, candleman shadow, etc.)

	// If he walks off edge, this holds the number of the room he goes to
	int32 walk_off_edge_to_room;	// Player should walk off edge unless told otherwise

	// if false player won't walk - totally aborts a walk
	bool need_to_walk = false;		// Player needs to walk somewhere

	// if FALSE player won't walk yet - walk suspended until TRUE
	bool ready_to_walk = false;		// Player is ready to perform that walk

	bool waiting_for_walk = false;

	// Default TRUE for every room
	// Flag if accepting player input
	bool comm_allowed = true;

	// Means a parser command is ready. When apps programmer finishes
	// parsing command, the programmer must set this to FALSE. If this
	// doesn't get set, the command falls through to the error handling code.
	int32 command_ready = 0;

	// Default TRUE, if set to FALSE walker disappears instantly, and vv
	bool walker_visible = false;	// Flag if player's sprite is visible

	// if TRUE, then apps programmer must display look messages
	bool look_around = false;		// Flag for special "look around" command

	// Tested by the apps programmer to check if player has been here before
	bool been_here_before = false;

	// Walker sprites dumped on switching scenes if TRUE in room preload code
	bool walker_reload_palette = false;
	bool disable_hyperwalk = false;
	bool walker_loads_first = false;
	bool walker_in_this_scene = false;

	int32 walker_trigger = false;

	int32 walk_x = 0, walk_y = 0;	// where to walk to when player.ready_to_walk
	int32 walk_facing = 0;

	int32 click_x = 0, click_y = 0;
};

} // namespace M4

#endif
