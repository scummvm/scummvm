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

#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section4.h"
#include "mads/madsv2/dragonsphere/rooms/room402.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[20];
	int16 sequence[20];
	int16 animation[4];
	int16 moving;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

static Scratch scratch;

#define fx_red_rocks            1
#define fx_distant_dune         2
#define fx_medium_dune          3
#define fx_small_dune           4
#define fx_dune_rocks           5
#define fx_large_dune           6
#define fx_bush                 7
#define fx_large_dune_cactus    8
#define fx_large_round_dune     9
#define fx_grassy_dune          10
#define fx_medium_dunes         11
#define fx_large_tent           12
#define fx_small_tent           13
#define fx_tumbleweed           14
#define fx_00                   15
#define fx_disp_6               16
#define fx_disp_9               17
#define fx_disp_3               18

#define RED_ROCKS_X             45
#define RED_ROCKS_Y             155
#define BUSH_X                  289
#define BUSH_Y                  155
#define MEDIUM_DUNE_X           270
#define MEDIUM_DUNE_Y           155
#define DUNE_ROCKS_X            227
#define DUNE_ROCKS_Y            155
#define LARGE_DUNE_CACTUS_X     279
#define LARGE_DUNE_CACTUS_Y     155
#define LARGE_ROUND_DUNE_X      300
#define LARGE_ROUND_DUNE_Y      155
#define MEDIUM_DUNES_X          270
#define MEDIUM_DUNES_Y          155

#define GRASSY_DUNE_X           29
#define GRASSY_DUNE_Y           131
#define LARGE_DUNE_X            32
#define LARGE_DUNE_Y            126
#define SMALL_DUNE_X            230
#define SMALL_DUNE_Y            133
#define DISTANT_DUNE_X          280
#define DISTANT_DUNE_Y          128

#define LARGE_TENT_X            70
#define LARGE_TENT_Y            130
#define SMALL_TENT_X            217
#define SMALL_TENT_Y            127
#define TUMBLEWEED_X            108
#define TUMBLEWEED_Y            149

#define FROM_NORTH_X            169
#define FROM_NORTH_Y            130
#define FROM_SOUTH_X            161
#define FROM_SOUTH_Y            143
#define FROM_EAST_X_1           330
#define FROM_EAST_Y_1           143
#define FROM_EAST_X_2           302
#define FROM_EAST_Y_2           143
#define FROM_WEST_X_1           -15
#define FROM_WEST_Y_1           143
#define FROM_WEST_X_2           15
#define FROM_WEST_Y_2           143


static void room_402_init() {
	int id;
	int count;
	int count2 = global[fire_holes] - 1;
	int skip   = 0;

	int front_right = false;
	int front_left  = false;
	int back_left   = false;
	int back_right  = false;

	if (global[player_persona] == PLAYER_IS_PID) {
		global[perform_displacements] = true;
	}

	ss[fx_bush]              = -1;
	ss[fx_red_rocks]         = -1;
	ss[fx_medium_dune]       = -1;
	ss[fx_large_dune_cactus] = -1;
	ss[fx_medium_dunes]      = -1;
	ss[fx_00]                = -1;

	local->moving  = false;
	ss[fx_disp_3]  = kernel_load_series("*KG4DIS3", false);
	ss[fx_disp_6]  = kernel_load_series("*KG4DIS6", false);
	ss[fx_disp_9]  = kernel_load_series("*KG4DIS9", false);

	/* put down RIGHT foreground sprites */

	for (count = 0; count < 77; count++) {
		++skip;
		if (skip == 7)
			skip = 1;
		++count2;
		if (count2 == 78)
			count2 = 1;

		if (count2 == global[desert_room]) {
			switch (skip) {
			case 1:
				ss[fx_medium_dune]  = kernel_load_series("*DUNE07", false);
				seq[fx_medium_dune] = kernel_seq_stamp(ss[fx_medium_dune], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_medium_dune], 1);
				kernel_seq_loc(seq[fx_medium_dune], MEDIUM_DUNE_X, MEDIUM_DUNE_Y);
				break;

			case 2:
				ss[fx_dune_rocks]  = kernel_load_series("*DUNE09", false);
				seq[fx_dune_rocks] = kernel_seq_stamp(ss[fx_dune_rocks], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_dune_rocks], 1);
				kernel_seq_loc(seq[fx_dune_rocks], DUNE_ROCKS_X, DUNE_ROCKS_Y);
				break;

			case 3:
				ss[fx_bush]  = kernel_load_series("*DUNE11", false);
				seq[fx_bush] = kernel_seq_stamp(ss[fx_bush], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_bush], 1);
				kernel_seq_loc(seq[fx_bush], BUSH_X, BUSH_Y);
				break;

			case 4:
				ss[fx_large_dune_cactus]  = kernel_load_series("*DUNE12", false);
				seq[fx_large_dune_cactus] = kernel_seq_stamp(ss[fx_large_dune_cactus], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_large_dune_cactus], 1);
				kernel_seq_loc(seq[fx_large_dune_cactus], LARGE_DUNE_CACTUS_X, LARGE_DUNE_CACTUS_Y);
				break;

			case 5:
				ss[fx_large_round_dune]  = kernel_load_series("*DUNE13", false);
				seq[fx_large_round_dune] = kernel_seq_stamp(ss[fx_large_round_dune], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_large_round_dune], 1);
				kernel_seq_loc(seq[fx_large_round_dune], LARGE_ROUND_DUNE_X, LARGE_ROUND_DUNE_Y);
				break;

			case 6:
				ss[fx_medium_dunes]  = kernel_load_series("*DUNE15", false);
				seq[fx_medium_dunes] = kernel_seq_stamp(ss[fx_medium_dunes], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_medium_dunes], 1);
				kernel_seq_loc(seq[fx_medium_dunes], MEDIUM_DUNES_X, MEDIUM_DUNES_Y);
				break;
			}

			if (skip <= 6)
				front_right = true;
		}
	}

	/* put down LEFT foreground sprites */

	count2 = 29;
	skip   = 0;

	for (count = 0; count < 77; count++) {
		++skip;
		if (skip == 9)
			skip = 1;
		++count2;
		if (count2 == 78)
			count2 = 1;

		if (count2 == global[desert_room]) {
			switch (skip) {
			case 1:
				if (ss[fx_red_rocks] == -1) {
					ss[fx_red_rocks]  = kernel_load_series("*DUNE01", false);
					seq[fx_red_rocks] = kernel_seq_stamp(ss[fx_red_rocks], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_red_rocks], 1);
					kernel_seq_loc(seq[fx_red_rocks], RED_ROCKS_X, RED_ROCKS_Y);
				}
				break;

			case 2:
				if (ss[fx_bush] == -1) {
					ss[fx_bush]  = kernel_load_series("*DUNE11", false);
					seq[fx_bush] = kernel_seq_stamp(ss[fx_bush], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_bush], 1);
					kernel_seq_loc(seq[fx_bush], 0, BUSH_Y);
				}
				break;

			case 3:
				if (ss[fx_medium_dune] == -1) {
					ss[fx_medium_dune]  = kernel_load_series("*DUNE07", false);
					seq[fx_medium_dune] = kernel_seq_stamp(ss[fx_medium_dune], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_medium_dune], 1);
					kernel_seq_loc(seq[fx_medium_dune], 0, MEDIUM_DUNE_Y);
				}
				break;

			case 4:
				if (ss[fx_large_dune_cactus] == -1) {
					ss[fx_large_dune_cactus]  = kernel_load_series("*DUNE12", false);
					seq[fx_large_dune_cactus] = kernel_seq_stamp(ss[fx_large_dune_cactus], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_large_dune_cactus], 1);
					kernel_seq_loc(seq[fx_large_dune_cactus], 0, LARGE_DUNE_CACTUS_Y);
				}
				break;

			case 5:
				if (ss[fx_medium_dunes] == -1) {
					ss[fx_medium_dunes]  = kernel_load_series("*DUNE15", false);
					seq[fx_medium_dunes] = kernel_seq_stamp(ss[fx_medium_dunes], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_medium_dunes], 1);
					kernel_seq_loc(seq[fx_medium_dunes], 0, MEDIUM_DUNES_Y);
				}
				break;

			case 6:
				if (ss[fx_00] == -1) {
					ss[fx_00]  = kernel_load_series("*DUNE00", false);
					seq[fx_00] = kernel_seq_stamp(ss[fx_00], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_00], 1);
					kernel_seq_loc(seq[fx_00], 0, MEDIUM_DUNES_Y);
				}
				break;
			}

			if (skip <= 6)
				front_left = true;
		}
	}

	/* put down LEFT background sprites */

	count2 = global[oasis] - 1;
	skip   = 0;

	for (count = 0; count < 77; count++) {
		++skip; if (skip == 5) skip = 1;
		++count2; if (count2 == 78) count2 = 1;

		if (count2 == global[desert_room]) {
			switch (skip) {
			case 1:
				ss[fx_large_dune]  = kernel_load_series("*DUNE10", false);
				seq[fx_large_dune] = kernel_seq_stamp(ss[fx_large_dune], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_large_dune], 14);
				kernel_seq_loc(seq[fx_large_dune], LARGE_DUNE_X, LARGE_DUNE_Y);
				break;

			case 2:
				ss[fx_grassy_dune]  = kernel_load_series("*DUNE14", false);
				seq[fx_grassy_dune] = kernel_seq_stamp(ss[fx_grassy_dune], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_grassy_dune], 14);
				kernel_seq_loc(seq[fx_grassy_dune], GRASSY_DUNE_X, GRASSY_DUNE_Y);
				break;
			}

			if (skip <= 2)
				back_left = true;
		}
	}

	/* put down RIGHT background sprites */

	count2 = global[oasis] - 1;
	skip   = 0;

	for (count = 0; count < 77; count++) {
		++skip;
		if (skip == 4)
			skip = 1;
		++count2;
		if (count2 == 78)
			count2 = 1;

		if (count2 == global[desert_room]) {

			switch (skip) {
			case 1:
				if (front_right && !back_left) {
					ss[fx_distant_dune]  = kernel_load_series("*DUNE02", false);
					seq[fx_distant_dune] = kernel_seq_stamp(ss[fx_distant_dune], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_distant_dune], 14);
					kernel_seq_loc(seq[fx_distant_dune], 40, DISTANT_DUNE_Y);

				} else if (front_left && !back_right) {
					ss[fx_distant_dune]  = kernel_load_series("*DUNE02", false);
					seq[fx_distant_dune] = kernel_seq_stamp(ss[fx_distant_dune], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_distant_dune], 14);
					kernel_seq_loc(seq[fx_distant_dune], DISTANT_DUNE_X, DISTANT_DUNE_Y);
				}
				break;
			}

			if (skip == 1)
				back_right = true;
		}
	}

	if (!player.been_here_before) {
		aa[0] = kernel_run_animation(kernel_name('l', 1), 0);
		id    = kernel_add_dynamic(words_lizard, words_look_at, SYNTAX_SINGULAR, KERNEL_NONE,
		                           0, 0, 0, 0);
		kernel_dynamic_hot[id].prep = PREP_ON;

		kernel_dynamic_anim(id, aa[0], 0);
		kernel_dynamic_anim(id, aa[0], 1);
	}


	if (previous_room != KERNEL_RESTORING_GAME) {
		switch (global[from_direction]) {
		case FROM_NORTH:
			player.x      = FROM_NORTH_X;
			player.y      = FROM_NORTH_Y;
			player.facing = FACING_SOUTH;
			break;

		case FROM_SOUTH:
			player.x      = FROM_SOUTH_X;
			player.y      = FROM_SOUTH_Y;
			player.facing = FACING_NORTH;
			break;

		case FROM_EAST:
			player_first_walk(FROM_EAST_X_1, FROM_EAST_Y_1, FACING_WEST,
			                  FROM_EAST_X_2, FROM_EAST_Y_2, FACING_WEST, true);
			break;

		case FROM_WEST:
			player_first_walk(FROM_WEST_X_1, FROM_WEST_Y_1, FACING_EAST,
			                  FROM_WEST_X_2, FROM_WEST_Y_2, FACING_EAST, true);
			break;
		}
	}

	section_4_music();
}

static void room_402_daemon() {
	int temp;

	if (player.walker_visible && (player.commands_allowed || (conv_control.running >= 0)) && !player.walking &&
	   (player.facing == player.turn_to_facing) && !local->moving && global[player_persona] == PLAYER_IS_KING) {

		switch (player.facing) {
		case FACING_EAST:
		case FACING_WEST:
			if (imath_random(1, 500) == 1) {
				if (imath_random(1, 2) == 1) {
					player.walker_visible = false;
					local->moving         = true;
					if (player.facing == FACING_EAST) {
						seq[fx_disp_6] = kernel_seq_forward(ss[fx_disp_6], false, 6, 0, 0, 1);
					} else {
						seq[fx_disp_6] = kernel_seq_forward(ss[fx_disp_6], true, 6, 0, 0, 1);
					}
					kernel_seq_player(seq[fx_disp_6], true);
					kernel_seq_range(seq[fx_disp_6], 8, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_disp_6], KERNEL_TRIGGER_EXPIRE, 0, 1);

				} else {
					player.walker_visible = false;
					local->moving         = true;
					if (player.facing == FACING_EAST) {
						seq[fx_disp_6] = kernel_seq_forward(ss[fx_disp_6], false, 6, 0, 0, 1);
					} else {
						seq[fx_disp_6] = kernel_seq_forward(ss[fx_disp_6], true, 6, 0, 0, 1);
					}
					kernel_seq_player(seq[fx_disp_6], true);
					kernel_seq_range(seq[fx_disp_6], 1, 7);
					kernel_seq_trigger(seq[fx_disp_6], KERNEL_TRIGGER_EXPIRE, 0, 2);
				}
			}
			break;

		case FACING_SOUTHEAST:
		case FACING_SOUTHWEST:
			if (imath_random(1, 500) == 1) {
				if (imath_random(1, 2) == 1) {
					player.walker_visible = false;
					local->moving         = true;
					if (player.facing == FACING_SOUTHEAST) {
						seq[fx_disp_3] = kernel_seq_forward(ss[fx_disp_3], false, 6, 0, 0, 1);
					} else {
						seq[fx_disp_3] = kernel_seq_forward(ss[fx_disp_3], true, 6, 0, 0, 1);
					}
					kernel_seq_player(seq[fx_disp_3], true);
					kernel_seq_range(seq[fx_disp_3], 1, 17);
					kernel_seq_trigger(seq[fx_disp_3], KERNEL_TRIGGER_EXPIRE, 0, 1);

				} else {
					player.walker_visible = false;
					local->moving         = true;
					if (player.facing == FACING_SOUTHEAST) {
						seq[fx_disp_3] = kernel_seq_forward(ss[fx_disp_3], false, 6, 0, 0, 1);
					} else {
						seq[fx_disp_3] = kernel_seq_forward(ss[fx_disp_3], true, 6, 0, 0, 1);
					}
					kernel_seq_player(seq[fx_disp_3], true);
					kernel_seq_range(seq[fx_disp_3], 18, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_disp_3], KERNEL_TRIGGER_EXPIRE, 0, 4);
				}
			}
			break;


		case FACING_NORTHWEST:
		case FACING_NORTHEAST:
			if (imath_random(1, 500) == 1) {
				player.walker_visible = false;
				local->moving         = true;
				if (player.facing == FACING_NORTHEAST) {
					seq[fx_disp_9] = kernel_seq_forward(ss[fx_disp_9], false, 6, 0, 0, 1);
				} else {
					seq[fx_disp_9] = kernel_seq_forward(ss[fx_disp_9], true, 6, 0, 0, 1);
				}
				kernel_seq_player(seq[fx_disp_9], true);
				kernel_seq_range(seq[fx_disp_9], 1, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_disp_9], KERNEL_TRIGGER_EXPIRE, 0, 1);
			}
			break;
		}
	}

	if (local->moving) switch (kernel.trigger) {
	case 1:
		player.walker_visible = true;
		local->moving         = false;
		if (player.facing == FACING_WEST || player.facing == FACING_EAST) {
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_disp_6]);

		} else if (player.facing == FACING_NORTHWEST || player.facing == FACING_NORTHEAST) {
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_disp_9]);

		} else if (player.facing == FACING_SOUTHWEST || player.facing == FACING_SOUTHEAST) {
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_disp_3]);
		}
		break;

	case 2:
		temp = seq[fx_disp_6];
		if (player.facing == FACING_EAST) {
			seq[fx_disp_6] = kernel_seq_stamp(ss[fx_disp_6], false, 7);
		} else {
			seq[fx_disp_6] = kernel_seq_stamp(ss[fx_disp_6], true, 7);
		}
		kernel_seq_player(seq[fx_disp_6], false);
		kernel_timing_trigger(imath_random(30, 150), 3);
		kernel_synch(KERNEL_SERIES, seq[fx_disp_6], KERNEL_SERIES, temp);
		break;

	case 3:
		kernel_seq_delete(seq[fx_disp_6]);
		if (player.facing == FACING_EAST) {
			seq[fx_disp_6] = kernel_seq_backward(ss[fx_disp_6], false, 6, 0, 0, 1);
		} else {
			seq[fx_disp_6] = kernel_seq_backward(ss[fx_disp_6], true, 6, 0, 0, 1);
		}
		kernel_seq_player(seq[fx_disp_6], false);
		kernel_seq_range(seq[fx_disp_6], 1, 7);
		kernel_seq_trigger(seq[fx_disp_6], KERNEL_TRIGGER_EXPIRE, 0, 1);
		break;

	case 4:
		temp = seq[fx_disp_3];
		if (player.facing == FACING_SOUTHEAST) {
			seq[fx_disp_3] = kernel_seq_stamp(ss[fx_disp_3], false, KERNEL_LAST);
		} else {
			seq[fx_disp_3] = kernel_seq_stamp(ss[fx_disp_3], true, KERNEL_LAST);
		}
		kernel_seq_player(seq[fx_disp_3], false);
		kernel_timing_trigger(imath_random(30, 150), 5);
		kernel_synch(KERNEL_SERIES, seq[fx_disp_3], KERNEL_SERIES, temp);
		break;

	case 5:
		kernel_seq_delete(seq[fx_disp_3]);
		if (player.facing == FACING_SOUTHEAST) {
			seq[fx_disp_3] = kernel_seq_backward(ss[fx_disp_3], false, 6, 0, 0, 1);
		} else {
			seq[fx_disp_3] = kernel_seq_backward(ss[fx_disp_3], true, 6, 0, 0, 1);
		}
		kernel_seq_player(seq[fx_disp_3], false);
		kernel_seq_range(seq[fx_disp_3], 18, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_disp_3], KERNEL_TRIGGER_EXPIRE, 0, 1);
		break;
	}
}

static void room_402_pre_parser() {
	if (local->moving) {
		switch (player.facing) {
		case FACING_EAST:
		case FACING_WEST:
			kernel_seq_delete(seq[fx_disp_6]);
			player.walker_visible = true;
			break;

		case FACING_NORTHEAST:
		case FACING_NORTHWEST:
			kernel_seq_delete(seq[fx_disp_9]);
			player.walker_visible = true;
			break;

		case FACING_SOUTHEAST:
		case FACING_SOUTHWEST:
			kernel_seq_delete(seq[fx_disp_3]);
			player.walker_visible = true;
			break;
		}
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		local->moving = false;
	}

	if (player_said_1(cross)) {
		if (player_has_been_in_room(405) && player_said_1(desert_to_west)) {
			global[from_direction]       = FROM_EAST;
			player.walk_off_edge_to_room = 405;

		} else if (player_has_been_in_room(405) && player_said_1(desert_to_east)) {
			global[pre_room]             = 401;
			player.walk_off_edge_to_room = 120;

		} else if (player_said_1(desert_to_east) && global[desert_room] % 7 == 0) {
			global[pre_room]             = 401;
			player.walk_off_edge_to_room = 120;

		} else if (player_said_1(desert_to_east) || player_said_1(desert_to_west)) {
			player.walk_off_edge_to_room = 400;
		}
	}
}

static void room_402_parser() {
	int count;
	int count2 = global[oasis] - 1;
	int roomNum = 400;

	if (player_said_1(cross)) {
		if (player_said_1(desert_to_north) || player_said_1(desert_to_south)) {
			if (player_said_1(desert_to_north)) {
				global[desert_room] = global[desert_room] - 7;
				global[from_direction] = FROM_SOUTH;

			} else if (player_said_1(desert_to_south)) {
				global[desert_room] += 7;
				global[from_direction] = FROM_NORTH;
			}

			for (count = 0; count < 77; count++) {
				++roomNum; if (roomNum == 404) roomNum = 401;
				++count2; if (count2 == 78) count2 = 1;

				if (count2 == global[desert_room]) {
					if (global[desert_room] == 42)                 roomNum = 401;
					if (global[desert_room] == global[oasis])      roomNum = 454;
					if (global[desert_room] == global[fire_holes]) roomNum = 412;
					goto over;
				}
			}

over:

			++global[desert_counter];
			if (player_has_been_in_room(405)) {
				global[from_direction] = FROM_EAST;
				new_room               = 405;

			} else if (global[desert_counter] == 6 && !player_has_been_in_room(405)) {
				new_room = 404;

			} else if (room_id == roomNum) {
				kernel.force_restart = true;

			} else {
				new_room = roomNum;
			}
			goto handled;
		}
	}

	if (player.look_around) {
		text_show(40201);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(desert_to_north) ||
		    player_said_1(desert_to_south) ||
		    player_said_1(desert_to_east)  ||
		    player_said_1(desert_to_west)) {
			text_show(40202);
			goto handled;
		}

		if (player_said_1(sky)) {
			text_show(40203);
			goto handled;
		}

		if (player_said_1(lizard)) {
			text_show(40205);
			goto handled;
		}
	}

	if (player_said_2(take, desert)) {
		text_show(40204);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_402_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.moving);
}

void room_402_preload() {
	room_init_code_pointer = room_402_init;
	room_pre_parser_code_pointer = room_402_pre_parser;
	room_parser_code_pointer = room_402_parser;
	room_daemon_code_pointer = room_402_daemon;

	section_4_walker();
	section_4_interface();

	vocab_make_active(words_lizard);
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
