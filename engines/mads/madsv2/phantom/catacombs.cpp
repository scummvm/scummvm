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

#include "mads/madsv2/phantom/catacombs.h"
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/error.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {

/* Fun stuff for the catacombs */

#define NORTH   0
#define EAST    1
#define SOUTH   2
#define WEST    3

#define DIRECTION_MASK          0x0003
#define FLAGS_MASK              0x00fc

#define CATACOMBS_TO_309        -2
#define CATACOMBS_TO_409a       -3
#define CATACOMBS_TO_409b       -4
#define CATACOMBS_TO_501        -5


struct Catacombs {
	int16 room_id;		/* Room ID of catacomb part.                 */
	int8 exit[4];		/* Catacomb room # if exit in direction      */
	int8 from[4];		/* When exit, will enter from this direction */
	int16 misc;			/* Miscellaneous bit data                    */
};

Catacombs *catacombs = NULL;

Catacombs easy_catacombs[32] = {
	{ 401, { -1, 1, 2, 6 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK },
	{ 404, { 10, 11, 3, 0 }, { SOUTH, WEST, NORTH, EAST }, MAZE_PUDDLE },
	{ 404, { 0, 3, 4, CATACOMBS_TO_309 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BLOCK },
	{ 401, { 1, 14, 5, 2 }, { SOUTH, WEST, NORTH, EAST }, MAZE_POT },
	{ 453, { 2, 4, -1, 4 }, { SOUTH, WEST, NORTH, EAST }, MAZE_DRAIN },
	{ 403, { 3, 6, -1, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK | MAZE_PLANK },
	{ 406, { -1, 0, -1, 5 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	{ 453, { -1, 8, -1, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK },
	/* 8 */
	{ 406, { -1, 9, -1, 7 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	{ 401, { 1, -1, 10, 8 }, { SOUTH, WEST, NORTH, EAST }, MAZE_RAT_NEST | MAZE_SKULL },
	{ 408, { 9, -1, 1, -1 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	{ 453, { 12, -1, -1, 1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK | MAZE_STONE },
	/* 12 */
	{ 408, { 13, -1, 11, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK },
	{ 401, { 13, 20, 12, 13 }, { WEST, WEST, NORTH, NORTH }, MAZE_BRICK },
	{ 453, { 16, 15, -1, 3 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK | MAZE_RAT_NEST },
	{ 456, { -1, -1, -1, 14 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	/* 16 */
	{ 404, { -1, 17, 14, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_WEB | MAZE_POT },
	{ 401, { 18, -1, 19, 16 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK },
	{ 408, { -1, -1, 17, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK },
	{ 403, { 17, -1, -1, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_HOLE | MAZE_WEB },
	/* 20 */
	{ 403, { 21, 22, -1, 13 }, { SOUTH, WEST, NORTH, EAST }, MAZE_WEB | MAZE_SKULL },
	{ 404, { -1, -1, 20, -1 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	{ 406, { -1, 23, -1, 20 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	{ 404, { 24, 23, 23, 22 }, { SOUTH, SOUTH, EAST, EAST }, MAZE_RAT_NEST | MAZE_BRICK },
	/* 24 */
	{ 401, { -1, 1, 23, 25 }, { SOUTH, EAST, NORTH, EAST }, MAZE_PUDDLE | MAZE_POT | MAZE_BRICK },
	{ 407, { 29, 24, 28, 26 }, { WEST, WEST, EAST, EAST }, 0 },
	{ 401, { 27, 25, 23, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_SKULL },
	{ 404, { -1, 28, 26, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_WEB | MAZE_FALLEN_BLOCK },
	/* 28 */
	{ 456, { -1, 25, -1, 27 }, { SOUTH, SOUTH, NORTH, EAST }, 0 },
	{ 406, { -1, 30, -1, 25 }, { SOUTH, WEST, NORTH, NORTH }, 0 },
	{ 453, { CATACOMBS_TO_409a, 30, -1, 29 }, { SOUTH, WEST, NORTH, EAST },
		MAZE_STONE | MAZE_RAT_NEST | MAZE_WEB },
	{ 408, { CATACOMBS_TO_501, -1, CATACOMBS_TO_409b, -1 }, { SOUTH, WEST, NORTH, EAST },
		MAZE_WEB | MAZE_BRICK }
};

Catacombs hard_catacombs[62] = {
	{ 401, { -1, 1, 2, 6 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK },
	{ 404, { 10, 11, 3, 0 }, { SOUTH, WEST, NORTH, EAST }, MAZE_PUDDLE },
	{ 404, { 0, 3, 4, CATACOMBS_TO_309 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BLOCK },
	{ 401, { 1, 20, 5, 2 }, { SOUTH, NORTH, NORTH, EAST }, MAZE_POT },
	{ 453, { 2, 4, -1, 4 }, { SOUTH, WEST, NORTH, EAST }, MAZE_DRAIN },
	{ 403, { 3, 6, -1, 4 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK | MAZE_PLANK },
	{ 406, { -1, 0, -1, 5 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	{ 453, { -1, 8, -1, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK },
	/* 8 */
	{ 406, { -1, 9, -1, 7 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	{ 401, { 1, -1, 10, 8 }, { NORTH, WEST, NORTH, EAST }, MAZE_RAT_NEST | MAZE_SKULL },
	{ 408, { 9, -1, 1, -1 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	{ 453, { 12, -1, -1, 1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK | MAZE_STONE },
	/* 12 */
	{ 408, { 13, -1, 11, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK },
	{ 401, { 13, 21, 12, 13 }, { WEST, WEST, NORTH, NORTH }, MAZE_BRICK },
	{ 453, { 16, 15, -1, 20 }, { SOUTH, WEST, NORTH, SOUTH }, MAZE_RAT_NEST | MAZE_BRICK },
	{ 456, { -1, -1, -1, 14 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	/* 16 */
	{ 404, { -1, 17, 14, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_WEB | MAZE_POT },
	{ 401, { 18, -1, 19, 16 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK },
	{ 408, { -1, -1, 17, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK },
	{ 403, { 17, -1, -1, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_HOLE | MAZE_WEB },
	/* 20 */
	{ 408, { 3, -1, 14, -1 }, { EAST, WEST, WEST, EAST }, 0 },
	{ 404, { 9, 30, 22, 13 }, { NORTH, WEST, NORTH, EAST }, MAZE_RAT_NEST },
	{ 403, { 21, 23, -1, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_HOLE | MAZE_WEB },
	{ 401, { -1, -1, 24, 22 }, { SOUTH, WEST, WEST, EAST }, MAZE_BRICK },
	/* 24 */
	{ 406, { -1, 26, -1, 23 }, { SOUTH, NORTH, NORTH, SOUTH }, 0 },
	{ 407, { 36, 33, 35, 34 }, { WEST, WEST, EAST, EAST }, 0 },
	{ 453, { 24, 27, -1, -1 }, { EAST, NORTH, NORTH, EAST }, MAZE_BRICK },
	{ 403, { 26, -1, -1, 28 }, { EAST, WEST, NORTH, NORTH }, MAZE_BRICK | MAZE_SKULL },
	/* 28 */
	{ 404, { 27, 28, 28, 29 }, { WEST, SOUTH, EAST, SOUTH }, 0 },
	{ 408, { -1, -1, 28, -1 }, { SOUTH, WEST, WEST, EAST }, MAZE_BRICK },
	{ 406, { -1, 31, -1, 21 }, { SOUTH, NORTH, NORTH, EAST }, 0 },
	{ 401, { 30, 33, 1, -1 }, { EAST, SOUTH, EAST, EAST }, MAZE_PUDDLE | MAZE_POT },
	/* 32 */
	{ 456, { -1, 31, -1, 33 }, { SOUTH, EAST, NORTH, NORTH }, 0 },
	{ 404, { 32, -1, 31, 25 }, { WEST, WEST, EAST, EAST }, 0 },
	{ 401, { 46, 25, 31, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_SKULL },
	{ 401, { -1, 25, 41, -1 }, { SOUTH, SOUTH, EAST, EAST }, MAZE_BRICK | MAZE_POT },
	/* 36 */
	{ 406, { -1, 37, -1, 25 }, { SOUTH, WEST, NORTH, NORTH }, 0 },
	{ 453, { CATACOMBS_TO_409a, 37, -1, 36 }, { SOUTH, WEST, NORTH, EAST }, MAZE_STONE | MAZE_RAT_NEST | MAZE_WEB },
	{ 408, { 57, -1, 54, -1 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	{ 408, { 40, -1, CATACOMBS_TO_409b, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK | MAZE_WEB },
	/* 40 */
	{ 404, { 40, 40, 39, 53 }, { EAST, NORTH, NORTH, EAST }, MAZE_BLOCK | MAZE_FALLEN_BLOCK },
	{ 456, { -1, 35, -1, 42 }, { SOUTH, SOUTH, NORTH, SOUTH }, 0 },
	{ 408, { 43, -1, 41, -1 }, { EAST, WEST, WEST, EAST }, MAZE_BRICK },
	{ 406, { -1, 42, -1, 61 }, { SOUTH, NORTH, NORTH, EAST }, 0 },
	/* 44 */
	{ 403, { 58, 45, -1, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK | MAZE_RAT_NEST },
	{ 401, { 34, -1, 46, 44 }, { NORTH, WEST, NORTH, EAST }, MAZE_RAT_NEST | MAZE_BRICK },
	{ 404, { 45, -1, 34, 47 }, { SOUTH, WEST, NORTH, EAST }, MAZE_WEB | MAZE_FALLEN_BLOCK },
	{ 406, { -1, 46, -1, 48 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	/* 48 */
	{ 403, { 49, 47, -1, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK | MAZE_SKULL | MAZE_WEB },
	{ 408, { 50, -1, 48, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK },
	{ 408, { 51, -1, 49, -1 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	{ 408, { 52, -1, 50, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK },
	/* 52 */
	{ 408, { -1, -1, 51, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK },
	{ 406, { -1, 40, -1, 54 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	{ 403, { 38, 53, -1, 55 }, { SOUTH, WEST, NORTH, EAST }, MAZE_SKULL },
	{ 453, { 56, 54, -1, -1 }, { SOUTH, WEST, NORTH, EAST }, MAZE_BRICK | MAZE_WEB },
	/* 56 */
	{ 401, { 56, CATACOMBS_TO_501, 55, 56 }, { WEST, WEST, NORTH, NORTH }, MAZE_BRICK | MAZE_SKULL },
	{ 404, { -1, 57, 38, 57 }, { SOUTH, WEST, NORTH, EAST }, MAZE_POT | MAZE_BLOCK },
	{ 404, { 59, 59, 44, 60 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	{ 404, { 59, 60, 59, 58 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	/* 60 */
	{ 404, { 61, 58, 59, 59 }, { SOUTH, WEST, NORTH, EAST }, 0 },
	{ 404, { 34, 43, 60, 44 }, { NORTH, WEST, NORTH, EAST }, 0 }
};

void global_catacombs_setup() {
	if (game.difficulty == HARD_MODE) {
		catacombs = hard_catacombs;
		global[catacombs_309_from] = 3;
		global[catacombs_409a] = 37;
		global[catacombs_409a_from] = 0;
		global[catacombs_409b] = 39;
		global[catacombs_309] = 2;
		global[catacombs_409b_from] = 2;
		global[catacombs_501] = 56;
		global[catacombs_501_from] = 1;
	} else {
		catacombs = easy_catacombs;
		global[catacombs_309_from] = 3;
		global[catacombs_409a] = 30;
		global[catacombs_309] = 2;
		global[catacombs_409b_from] = 2;
		global[catacombs_409b] = 31;
		global[catacombs_501] = 31;
		global[catacombs_409a_from] = 0;
		global[catacombs_501_from] = 0;
	}
}

static void global_catacombs_new_room(int catacomb_node, int from) {
	int newRoom = 0;

	global[catacombs_next_room] = catacomb_node;
	global[catacombs_from] = from & DIRECTION_MASK;
	global[catacombs_flag] = from & FLAGS_MASK;

	if (catacomb_node >= 0) {
		newRoom = catacombs[catacomb_node].room_id;
		global[catacombs_misc] = catacombs[catacomb_node].misc;
	} else {
		switch (catacomb_node) {
		case CATACOMBS_TO_409a:
		case CATACOMBS_TO_409b:
			newRoom = 409;
			break;

		case CATACOMBS_TO_501:
			newRoom = 501;
			break;

		case CATACOMBS_TO_309:
			newRoom = 309;
			break;

		default:
			error_report(ERROR_KERNEL_NO_ROOM, ERROR, MODULE_UNKNOWN, new_room, 0);
			break;
		}
	}

	if (kernel.trigger_setup_mode == KERNEL_TRIGGER_PREPARSE) {
		player.walk_off_edge_to_room = newRoom;
	} else {
		new_room = newRoom;
		kernel.force_restart = true;
	}
}

void global_catacombs_init() {
	global_catacombs_setup();
	global[catacombs_room] = global[catacombs_next_room];
}

void global_enter_catacombs(int special) {
	global_catacombs_setup();

	switch (room_id) {
	case 409:
		if (special)
			global_catacombs_new_room(global[catacombs_409b], global[catacombs_409b_from]);
		else
			global_catacombs_new_room(global[catacombs_409a], global[catacombs_409a_from]);
		break;

	case 501:
		global_catacombs_new_room(global[catacombs_501], global[catacombs_501_from]);
		break;

	default:
		global_catacombs_new_room(global[catacombs_309], global[catacombs_309_from]);
		break;
	}
}

int global_catacombs_exit(int exit) {
	return catacombs[global[catacombs_room]].exit[exit];
}

void global_catacombs_move(int exit) {
	int from = catacombs[global[catacombs_room]].from[exit];
	int node = catacombs[global[catacombs_room]].exit[exit];
	global_catacombs_new_room(node, from);
}

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
