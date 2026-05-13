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
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/room113.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */

	int16 side_of_room;     /* indicates if Pid on left or right side of room */
	int16 stairs_id[5];     /* Handles for stairs hotspots */

	int16 seal_frame;       /* animation frame being held for seal stuff */
	int16 seal_action;      /* Type of action to run for seal animation */
	int16 anim_0_running;

	int16 clicked_on;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

static Scratch scratch;


/* ========================= Sprites ========================= */

#define fx_pid_descends      1  /* rm113b2 */
#define fx_rare_coin         2  /* rm113p0 */
#define fx_emerald           3  /* rm113p1 */
#define fx_ward              4  /* rm113y  */    
#define fx_take_coin         5  /* pddrl_9 */
#define fx_take_emerald      6  /* rm113g  */  
#define fx_trap_door         7  /* rm113x  */
#define fx_dead_king         8  /* rm113a  */
#define fx_down_stairs       9  /* rm113b1 */
#define fx_pid_floats        10  /* rm113z0 */
#define fx_dead_pid          11  /* rm113z1 */


/* ======================== Triggers ========================= */

#define ROOM_113_RESTART     70
#define ROOM_113_PID_APPEARS 80


/* walk points */
#define START_X_ROOM_114     127
#define START_Y_ROOM_114     73
#define START_X_ROOM_117     227
#define START_Y_ROOM_117     118
#define START_X_ROOM_110     254
#define START_Y_ROOM_110     117

#define WALK_TO_STAIRS_X     127
#define WALK_TO_STAIRS_Y     73

/* cursor points */
#define FLOOR_WITH_TRAP_X    203
#define FLOOR_WITH_TRAP_Y    103

/* absolute series points */
#define DEAD_KING_X          255
#define DEAD_KING_Y          123

#define SEAL_TREAD           0
#define SEAL_LEAVE           1
#define SEAL_GET_OUT_FAR     2
#define SEAL_GET_OUT_NEAR    3

#define OUT_OF_WATER_FAR_X   122
#define OUT_OF_WATER_FAR_Y   81

#define OUT_OF_WATER_NEAR_X  183
#define OUT_OF_WATER_NEAR_Y  130

#define INTO_SEAL_NEAR_X     213
#define INTO_SEAL_NEAR_Y     127

#define INTO_SEAL_FAR_X      107
#define INTO_SEAL_FAR_Y      81

#define LEFT_SIDE            0
#define RIGHT_SIDE           1 

#define SEAL                 0
#define HUMAN                1

#define RIVER_X              132
#define RIVER_Y              94



void room_113_alter_hotspots(int state) {
	int count;

	if (state == SEAL) {
		kernel_flip_hotspot(words_river, true);

		for (count = 0; count < room_num_spots; count++) {

			if (room_spots[count].vocab == words_river) {
				if (room_spots[count].verb != words_swim_up) {
					room_spots[count].verb = words_swim_down;
				}
			}
			if (room_spots[count].vocab == words_cave_floor) {
				room_spots[count].vocab = words_shore;
				room_spots[count].verb = words_swim_to;
			}
			if (room_spots[count].vocab == words_grate) {
				room_spots[count].verb = words_swim_to;
			}
			if (room_spots[count].vocab == words_trap_door) {
				room_spots[count].cursor_number = 1;
			}
			if ((room_spots[count].vocab == words_cave_wall) || (room_spots[count].vocab == words_stairs) ||
				(room_spots[count].vocab == words_trap_door) || (room_spots[count].vocab == words_ward) ||
				(room_spots[count].vocab == words_rare_coin) || (room_spots[count].vocab == words_emerald)) {
				room_spots[count].verb = words_swim_towards;
			}
		}

	} else {

		kernel_flip_hotspot(words_river, false);
		kernel_flip_hotspot_loc(words_river, true, RIVER_X, RIVER_Y);

		for (count = 0; count < room_num_spots; count++) {

			if (room_spots[count].vocab == words_river) {
				if (room_spots[count].verb != words_swim_up && room_spots[count].cursor_number != 5) {
					room_spots[count].verb = words_walk_to;
				}
			}
			if (room_spots[count].vocab == words_shore) {
				room_spots[count].verb = words_walk_across;
				room_spots[count].vocab = words_cave_floor;
			}
			if (room_spots[count].vocab == words_stairs) {
				room_spots[count].verb = words_walk_down;
			}
			if (room_spots[count].vocab == words_trap_door) {
				room_spots[count].cursor_number = 3;
			}
			if (room_spots[count].vocab == words_trap_door) {
				room_spots[count].verb = words_walk_across;
			}
			if (room_spots[count].vocab == words_trap_door || room_spots[count].vocab == words_grate ||
				room_spots[count].vocab == words_ward || room_spots[count].vocab == words_emerald ||
				room_spots[count].vocab == words_rare_coin || room_spots[count].vocab == words_cave_wall) {
				room_spots[count].verb = words_walk_to;
			}
		}
	}
}

static void room_113_init() {
	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
	}

	/* Load sprite series */

	if (global[player_persona] == PLAYER_IS_PID) {
		if (global[statue_is_on_stairway_114]) {
			ss[fx_down_stairs] = kernel_load_series(kernel_name('b', 1), false);
		} else {
			ss[fx_pid_descends] = kernel_load_series(kernel_name('b', 2), false);
		}
	}

	if (object_is_here(rare_coin)) {
		ss[fx_rare_coin] = kernel_load_series(kernel_name('p', 0), false);
		if (global[player_persona] == PLAYER_IS_PID) {
			ss[fx_take_coin] = kernel_load_series("*PDDRL_9", false);
		}
		seq[fx_rare_coin] = kernel_seq_stamp(ss[fx_rare_coin], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_rare_coin], 6);
	} else {
		kernel_flip_hotspot(words_rare_coin, false);
	}

	if (global[ward_status] == WARD_PRESENT) {
		ss[fx_ward] = kernel_load_series(kernel_name('y', -1), false);
		seq[fx_ward] = kernel_seq_stamp(ss[fx_ward], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_ward], 12);
		kernel_flip_hotspot(words_emerald, false);
	} else {
		kernel_flip_hotspot(words_ward, false);
	}


	if (object_is_here(emerald)) {
		ss[fx_emerald] = kernel_load_series(kernel_name('p', 1), false);
		if (global[player_persona] == PLAYER_IS_PID) {
			ss[fx_take_emerald] = kernel_load_series(kernel_name('g', -1), false);
		}
		if (global[ward_status] == WARD_GONE) {
			seq[fx_emerald] = kernel_seq_stamp(ss[fx_emerald], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_emerald], 12);
		}
	} else {
		kernel_flip_hotspot(words_emerald, false);
	}


	/* if statue on stairs in rm114 show stairs hotspots else show trap_door */

	if (global[statue_is_on_stairway_114]) {
		kernel_flip_hotspot(words_trap_door, false);

	} else {
		kernel_flip_hotspot(words_trap_door, true);
		kernel_flip_hotspot(words_stairs, false);
		ss[fx_trap_door] = kernel_load_series(kernel_name('x', -1), false);
		seq[fx_trap_door] = kernel_seq_stamp(ss[fx_trap_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_trap_door], 14);
	}

	if (local->anim_0_running) {
		aa[0] = kernel_run_animation(kernel_name('s', 1), 0);
		player.walker_visible = false;
		local->seal_action = SEAL_TREAD;
		kernel_reset_animation(aa[0], 187);
		/* player is a seal */

	} else if (previous_room == 114) {       /* Player comes from Stair Trap */
		player.x = START_X_ROOM_114;
		player.y = START_Y_ROOM_114;
		player.facing = FACING_SOUTHEAST;
		local->side_of_room = LEFT_SIDE;

	} else if (previous_room == 117) {  /* Player comes from Swimming Cutaway */
		aa[0] = kernel_run_animation(kernel_name('s', 1), 0);
		player.walker_visible = false;
		player.commands_allowed = false;
		local->seal_action = SEAL_TREAD;
		local->anim_0_running = true;
		kernel_reset_animation(aa[0], 84);

	} else if (previous_room != KERNEL_RESTORING_GAME) {  /* Player comes from Way Station rm110 */
		local->side_of_room = RIGHT_SIDE;
		player.commands_allowed = false;
		if (global[player_persona] == PLAYER_IS_PID) {
			player.walker_visible = false;
			if (!player_has(magic_belt)) {
				sound_play(N_PlayerDies);
				ss[fx_dead_pid] = kernel_load_series(kernel_name('z', 1), false);
				seq[fx_dead_pid] = kernel_seq_stamp(ss[fx_dead_pid], false, KERNEL_FIRST);
				kernel_seq_loc(seq[fx_dead_pid], DEAD_KING_X, DEAD_KING_Y);
				kernel_seq_depth(seq[fx_dead_pid], 5);
				kernel_timing_trigger(TWO_SECONDS, ROOM_113_RESTART);

			} else {
				ss[fx_pid_floats] = kernel_load_series(kernel_name('z', 0), false);
				seq[fx_pid_floats] = kernel_seq_forward(ss[fx_pid_floats], false,
					8, 0, 0, 1);
				kernel_seq_depth(seq[fx_pid_floats], 8);
				kernel_seq_range(seq[fx_pid_floats], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_pid_floats],
					KERNEL_TRIGGER_EXPIRE, 0, ROOM_113_PID_APPEARS);
			}

		} else {
			sound_play(N_PlayerDies);
			ss[fx_dead_king] = kernel_load_series(kernel_name('a', -1), false);
			seq[fx_dead_king] = kernel_seq_stamp(ss[fx_dead_king], false, KERNEL_FIRST);
			kernel_seq_loc(seq[fx_dead_king], DEAD_KING_X, DEAD_KING_Y);
			kernel_seq_depth(seq[fx_dead_king], 5);
			kernel_seq_scale(seq[fx_dead_king], 85);
			kernel_timing_trigger(TWO_SECONDS, ROOM_113_RESTART);
		}
	}

	if (local->anim_0_running) {
		room_113_alter_hotspots(SEAL);
	} else {
		room_113_alter_hotspots(HUMAN);
	}

	section_1_music();
}

static void handle_animation_seal() {
	int seal_reset_frame;

	if (kernel_anim[aa[0]].frame != local->seal_frame) {
		local->seal_frame = kernel_anim[aa[0]].frame;
		seal_reset_frame = -1;

		switch (local->seal_frame) {
		case 37:  /* end of Pid getting out of water far side */
			room_113_alter_hotspots(HUMAN);
			player_demand_location(OUT_OF_WATER_FAR_X, OUT_OF_WATER_FAR_Y);
			player_demand_facing(FACING_NORTHWEST);
			global[player_is_seal] = false;
			sound_play(N_UnderGroundMus);
			player.walker_visible = true;
			kernel_abort_animation(aa[0]);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			seal_reset_frame = -1;
			local->side_of_room = LEFT_SIDE;
			player.commands_allowed = true;
			local->anim_0_running = false;
			break;

		case 72:  /* end of Pid getting out of water near side */
			room_113_alter_hotspots(HUMAN);
			player_demand_location(OUT_OF_WATER_NEAR_X, OUT_OF_WATER_NEAR_Y);
			player_demand_facing(FACING_SOUTHEAST);
			player.walker_visible = true;
			global[player_is_seal] = false;
			sound_play(N_UnderGroundMus);
			kernel_abort_animation(aa[0]);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			seal_reset_frame = -1;
			local->side_of_room = RIGHT_SIDE;
			player.commands_allowed = true;
			local->anim_0_running = false;
			break;

		case 101:
		case 136:
		case 177:
			player.commands_allowed = true;
			break;

		case 102: /* end Pid coming from room 117 */
		case 137: /* end Pid on far side to seal  */
		case 178: /* end Pid on near side to seal */
		case 186: /* end of 360  */
		case 187: /* tread water */
		case 188: /* tread water */
		case 189: /* tread water */
		case 190: /* tread water */
		case 191: /* tread water */
		case 192: /* tread water */
		case 193: /* tread water */
		case 194: /* tread water */
		case 195: /* tread water */
			switch (local->seal_action) {
			case SEAL_TREAD:
				seal_reset_frame = imath_random(186, 194);
				if (imath_random(1, 60) == 1) {
					seal_reset_frame = 178;  /* do a 360 */
				}
				break;

			case SEAL_LEAVE:
				seal_reset_frame = 72; /* seal swim down stream */
				break;

			case SEAL_GET_OUT_FAR:
				seal_reset_frame = 0; /* seal get out of water on far side */
				break;

			case SEAL_GET_OUT_NEAR:
				seal_reset_frame = 37; /* seal get out of water on near side */
				break;
			}
			break;

		case 83: /* end of swimming off screen */
			new_room = 117;
			break;
		}

		if (seal_reset_frame >= 0) {
			kernel_reset_animation(aa[0], seal_reset_frame);
			local->seal_frame = seal_reset_frame;
		}
	}
}

static void room_113_daemon(void) {
	if (local->anim_0_running) {
		handle_animation_seal();
	}

	if (kernel.trigger == ROOM_113_RESTART) {
		if (game.difficulty == EASY_MODE) {
			text_show(11336);
		} else {
			text_show(45);
		}
		new_room = 110;
	}

	if (kernel.trigger == ROOM_113_PID_APPEARS) {
		player_demand_facing(FACING_SOUTH);
		player.x = START_X_ROOM_110;
		player.y = START_Y_ROOM_110;
		player.commands_allowed = true;
		player.walker_visible = true;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_pid_floats]);
	}
}

static void room_113_pre_parser() {
	if (inter_point_y >= 100 || inter_point_x > 221) {
		local->clicked_on = RIGHT_SIDE;
	} else {
		local->clicked_on = LEFT_SIDE;
	}

	if (local->side_of_room == RIGHT_SIDE && !local->anim_0_running) {
		if (local->clicked_on == LEFT_SIDE) {
			player.need_to_walk = false;
			if (player_said_1(walk_to) || player_said_1(walk_across)) {
				if (!player_said_1(river)) {
					text_show(11304);
					player_cancel_command();
				}
			}
		}
	}

	if (local->side_of_room == LEFT_SIDE && !local->anim_0_running) {
		if (local->clicked_on == RIGHT_SIDE) {
			player.need_to_walk = false;
			if (player_said_1(walk_to) || player_said_1(walk_across)) {
				if (!player_said_1(river)) {
					text_show(11304);
					player_cancel_command();
				}
			}
		}
	}

	if (player_said_1(shift_into_seal) && !local->anim_0_running) {
		if (local->side_of_room == RIGHT_SIDE) {
			player_walk(INTO_SEAL_NEAR_X, INTO_SEAL_NEAR_Y, FACING_NORTHWEST);
		} else {
			player_walk(INTO_SEAL_FAR_X, INTO_SEAL_FAR_Y, FACING_SOUTHEAST);
		}
	}

	if (local->anim_0_running) {  /* player is Seal */
		player.need_to_walk = false;
	}
}

static void room_113_parser() {
	int count;
	int ok = false;

	if (player.look_around) {
		text_show(11301);
		goto handled;
	}

	if (local->anim_0_running) {  /* player is Seal */
		if (player_said_2(swim_down, river)) {
			for (count = 0; count < room_num_spots; count++) {
				if (room_spots[count].verb == words_swim_down) {
					if (room_spots[count].cursor_number == 5) {
						if ((inter_point_x >= room_spots[count].ul_x) &&
							(inter_point_x <= room_spots[count].lr_x) &&
							(inter_point_y >= room_spots[count].ul_y) &&
							(inter_point_y <= room_spots[count].lr_y)) {
							ok = true;
						}
					}
				}
			}

			if (!ok) {
				text_show(11332);
				goto handled;
			}

			if (player_has_been_in_room(117)) {
				text_show(11317);
			} else {
				local->seal_action = SEAL_LEAVE;
				player.commands_allowed = false;
			}
			goto handled;
		}

		if (player_said_1(revert)) {
			if (local->side_of_room == RIGHT_SIDE) {
				local->seal_action = SEAL_GET_OUT_NEAR;
			} else {
				local->seal_action = SEAL_GET_OUT_FAR;
			}
			goto handled;
		}

		if (player_said_2(swim_up, river)) {
			text_show(11331);
			goto handled;
		}

		if (player_said_2(swim_to, grate)) {
			text_show(11330);
			goto handled;
		}

		if (player_said_1(swim_towards) || player_said_1(swim_to) ||
			(player_said_2(take, emerald) && !player_has(emerald)) ||
			(player_said_2(take, rare_coin) && !player_has(rare_coin))) {
			if (local->clicked_on == RIGHT_SIDE) {
				local->seal_action = SEAL_GET_OUT_NEAR;
			} else {
				local->seal_action = SEAL_GET_OUT_FAR;
			}
			player.commands_allowed = false;
			goto handled;
		}
	}

	if (player_said_2(walk_to, trap_door) || player_said_2(swim_towards, trap_door)) {
		if (local->side_of_room == LEFT_SIDE) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				kernel_seq_delete(seq[fx_trap_door]);
				seq[fx_pid_descends] = kernel_seq_forward(ss[fx_pid_descends],
					false, 8, 0, 0, 1);
				kernel_seq_depth(seq[fx_pid_descends], 14);
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_pid_descends]);
				kernel_seq_trigger(seq[fx_pid_descends],
					KERNEL_TRIGGER_EXPIRE, 0, 1);
				sound_play(N_OldMachinery);
				break;

			case 1:
				new_room = 114;
				break;
			}
		}
		goto handled;
	}

	if (player_said_2(walk_down, stairs) || player_said_2(swim_towards, stairs)) {
		if (local->side_of_room == LEFT_SIDE) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				seq[fx_down_stairs] = kernel_seq_forward(ss[fx_down_stairs],
					false, 7, 0, 0, 1);
				kernel_seq_depth(seq[fx_down_stairs], 11);
				kernel_synch(KERNEL_SERIES, seq[fx_down_stairs], KERNEL_PLAYER, 0);
				kernel_seq_trigger(seq[fx_down_stairs],
					KERNEL_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				new_room = 114;
				break;
			}

		} else {
			text_show(11304);
		}
		goto handled;
	}


	if (player_said_2(take, rare_coin)) {
		if (kernel.trigger || !player_has(rare_coin)) {
			if (local->side_of_room == RIGHT_SIDE) {
				switch (kernel.trigger) {
				case 0:
					player.commands_allowed = false;
					player.walker_visible = false;
					seq[fx_take_coin] = kernel_seq_pingpong(ss[fx_take_coin],
						false, 6, 0, 0, 2);
					kernel_seq_player(seq[fx_take_coin], true);
					kernel_seq_trigger(seq[fx_take_coin],
						KERNEL_TRIGGER_SPRITE, 5, 1);
					kernel_seq_trigger(seq[fx_take_coin],
						KERNEL_TRIGGER_EXPIRE, 0, 2);
					break;

				case 1:
					/* sound_queue (N_PickUpObject006);*/
					kernel_seq_delete(seq[fx_rare_coin]);
					kernel_flip_hotspot(words_rare_coin, false);
					sound_play(N_TakeObjectSnd);
					inter_give_to_player(rare_coin);
					global[player_score] += 5;
					object_examine(rare_coin, 11318, 0);
					break;

				case 2:
					kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_coin]);
					player.walker_visible = true;
					player.commands_allowed = true;
					break;
				}

			} else {
				text_show(11304);
			}
			goto handled;
		}
	}

	if (player_said_2(invoke_power_of, crystal_ball)) {
		if (global[ward_status] == WARD_PRESENT) {
			if (local->anim_0_running) {
				text_show(32);
				goto handled;

			} else if (local->side_of_room == LEFT_SIDE) {
				switch (kernel.trigger) {
				case 0:
					sound_play(N_InvokeCrystalBall);
					text_show(970);
					player.commands_allowed = false;
					kernel_seq_delete(seq[fx_ward]);
					kernel_flip_hotspot(words_ward, false);
					kernel_flip_hotspot(words_emerald, true);
					seq[fx_emerald] = kernel_seq_stamp(ss[fx_emerald], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_emerald], 12);
					kernel_timing_trigger(TENTH_SECOND, 1);
					global[player_score] += 5;
					goto handled;
					break;

				case 1:
					global[ward_status] = WARD_GONE;
					global[crystal_ball_dead] = true;
					inter_move_object(crystal_ball, NOWHERE);
					player.commands_allowed = true;
					text_show(11320);
					goto handled;
					break;
				}
			} else {
				text_show(11327);
				goto handled;
			}
		}
	}

	if (player_said_2(take, emerald)) {
		if (kernel.trigger || !player_has(emerald)) {
			if (local->side_of_room == LEFT_SIDE) {
				switch (kernel.trigger) {
				case 0:
					player.commands_allowed = false;
					player.walker_visible = false;
					seq[fx_take_emerald] = kernel_seq_pingpong(ss[fx_take_emerald],
						false, 6, 0, 0, 2);
					kernel_seq_range(seq[fx_take_emerald], 1, 2);
					kernel_seq_player(seq[fx_take_emerald], true);
					kernel_seq_trigger(seq[fx_take_emerald],
						KERNEL_TRIGGER_SPRITE, 2, 1);
					kernel_seq_trigger(seq[fx_take_emerald],
						KERNEL_TRIGGER_EXPIRE, 0, 2);
					break;

				case 1:
					/* sound_queue (N_PickUpObject006);*/
					kernel_seq_delete(seq[fx_emerald]);
					kernel_flip_hotspot(words_emerald, false);
					sound_play(N_TakeObjectSnd);
					inter_give_to_player(emerald);
					global[player_score] += 5;
					object_examine(emerald, 11319, 0);
					break;

				case 2:
					kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_emerald]);
					player.walker_visible = true;
					player.commands_allowed = true;
					break;
				}
			} else {
				text_show(11304);
			}
			goto handled;
		}
	}

	if (player_said_1(shift_into_seal)) {
		if (local->anim_0_running) {
			text_show(11325);

		} else {
			if (!(global[player_score_flags] & SCORE_SHIFT_INTO_SEAL_113)) {
				global[player_score_flags] = global[player_score_flags] | SCORE_SHIFT_INTO_SEAL_113;
				global[player_score] += 2;
			}

			global[player_is_seal] = true;
			sound_play(N_SealMus);
			local->anim_0_running = true;
			aa[0] = kernel_run_animation(kernel_name('s', 1), 0);
			if (local->side_of_room == LEFT_SIDE) {
				kernel_reset_animation(aa[0], 103);
			} else {
				kernel_reset_animation(aa[0], 138);
			}
			player.commands_allowed = false;
			player.walker_visible = false;
			local->seal_action = SEAL_TREAD;
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
			room_113_alter_hotspots(SEAL);
		}
		goto handled;
	}

	if ((player_said_1(open) || player_said_1(pull)) &&
		player_said_1(trap_door)) {
		text_show(11306);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(cave_floor) || player_said_1(shore)) {
			text_show(11302);
			goto handled;
		}

		if (player_said_1(trap_door)) {
			text_show(11303);
			goto handled;
		}

		if (player_said_1(river)) {
			if (local->anim_0_running) {
				text_show(11316);
			} else if (global[player_persona] == PLAYER_IS_PID) {
				text_show(11307);
			} else {
				text_show(11316);
			}
			goto handled;
		}

		if (player_said_1(cave_wall)) {
			text_show(11308);
			goto handled;
		}

		if (player_said_1(grate)) {
			text_show(11310);
			goto handled;
		}

		if (player_said_1(ward)) {
			text_show(11312);
			goto handled;
		}


		if (player_said_1(rare_coin)) {
			if (object_is_here(rare_coin)) {
				text_show(11315);
				goto handled;
			}
		}

		if (player_said_1(emerald)) {
			if (object_is_here(emerald)) {
				text_show(11314);
				goto handled;
			}
		}

		if (player_said_1(stairs)) {
			if (global[statue_is_on_stairway_114] == STAIRWAY_STATUE) {
				text_show(11326);
			} else {
				text_show(11305);
			}
			goto handled;
		}
	}

	if ((player_said_1(look) || player_said_1(look_at) || player_said_1(take)) &&
		player_said_1(bucket)) {
		text_show(11309);
		goto handled;
	}

	if ((player_said_1(push) || player_said_1(pull)) &&
		player_said_1(grate)) {
		text_show(11311);
		goto handled;
	}

	if (player_said_1(ward)) {
		if (local->side_of_room == RIGHT_SIDE && !local->anim_0_running) {
			text_show(11304);
			goto handled;

		} else if (local->side_of_room == LEFT_SIDE && !local->anim_0_running) {
			if (player_said_1(push) ||
				player_said_1(open) ||
				player_said_1(pull)) {
				text_show(11313);
				goto handled;
			}
		}
	}

	if (player_said_1(shift_into_bear)) {
		text_show(11321);
		goto handled;
	}

	if (player_said_1(shift_into_snake)) {
		text_show(11322);
		goto handled;
	}

	if (player_said_1(swim_to) || player_said_1(swim_towards) ||
		player_said_1(swim_down) || player_said_1(swim_up)) {
		goto handled;
	}

	if (player_said_2(gaze_into, crystal_ball)) {
		if (local->anim_0_running) {
			text_show(32);
		} else {
			text_show(11323);
		}
		goto handled;
	}

	if (local->anim_0_running) {  /* player is Seal */
		if (!player_said_1(look) && !player_said_1(take) && !player_said_1(push) &&
			!player_said_1(open) && !player_said_1(put) && !player_said_1(talk_to) &&
			!player_said_1(give) && !player_said_1(pull) && !player_said_1(close) &&
			!player_said_1(throw) && !player_said_1(swim_to) && !player_said_1(swim_towards)) {
			text_show(32);
			goto handled;
		}
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_113_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.side_of_room);
	for (int16 &v : scratch.stairs_id) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.seal_frame);
	s.syncAsSint16LE(scratch.seal_action);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.clicked_on);
}

void room_113_preload() {
	room_init_code_pointer = room_113_init;
	room_pre_parser_code_pointer = room_113_pre_parser;
	room_parser_code_pointer = room_113_parser;
	room_daemon_code_pointer = room_113_daemon;

	if (kernel.teleported_in) {
		inter_give_to_player(crystal_ball);
		inter_give_to_player(shifter_ring);
		inter_give_to_player(magic_belt);
		global[player_persona] = PLAYER_IS_PID;
	}

	section_1_walker();
	section_1_interface();

	if (global[statue_is_on_stairway_114]) {
		kernel_initial_variant = 1;
	}

	vocab_make_active(words_swim_down);
	vocab_make_active(words_swim_up);
	vocab_make_active(words_shore);
	vocab_make_active(words_swim_to);
	vocab_make_active(words_swim_towards);
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
