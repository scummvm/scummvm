
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

#include "common/algorithm.h"
#include "m4/adv_r/adv_hotspot.h"
#include "m4/m4_types.h"

namespace M4 {

#define MAX_SCENES 180
#define MAXRAILNODES 32
#define PATH_END     0xffff
#define MAX_PLYR_STRING_LEN     40

#define STR_FADEPAL "fade palette"
#define STR_RAILNODE "rail node"
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

enum {
	UNKNOWN_OBJECT = 997,
	BACKPACK = 998,
	NOWHERE = 999
};
#define GLOBAL_SCENE     999

enum KernelTriggerType {
	KT_PARSE = 1,
	KT_DAEMON, KT_PREPARSE, KT_EXPIRE, KT_LOOP
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

	int16 depth_table[16];							// Player sprite depth table
	int32 numRailNodes = 0;							// # of rails

	SceneDef() {
		Common::fill(depth_table, depth_table + 16, 0);
	}
};

} // namespace M4

#endif
