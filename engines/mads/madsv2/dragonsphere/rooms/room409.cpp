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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section4.h"
#include "mads/madsv2/dragonsphere/rooms/room409.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];
	int16 facing;
	int16 new_x;
	int16 new_y;
	int16 old_x;
	int16 old_y;
	int16 old_facing;
	int16 on_shore;
	int16 standing_on;
	int16 pid_frame;
	int16 pid_action;
	int16 anim_0_running;
	int16 pid_1_frame;
	int16 anim_1_running;
	int16 move_counter;
	int16 fire;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_n                    0
#define fx_ew                   1
#define fx_s                    2
#define fx_death_n              3
#define fx_death_ew             4
#define fx_death_s              5
#define fx_fire                 6

#define ROOM_409_DONE_ENTER_FROM_509 60
#define ROOM_409_JUMP                65
#define ROOM_409_END_OF_JUMP         67
#define ROOM_409_TURN                70
#define ROOM_409_END_OF_TURN         75
#define ROOM_409_WHOA_SOUTH          80
#define ROOM_409_WHOA_NORTH          85
#define ROOM_409_WAIT_FOR_END_SLIDE  90
#define ROOM_409_SOUND               120
#define ROOM_409_END_DEATH           125

#define BASE_X                  78
#define BASE_Y                  18

#define ENTER_408_X_BOTTOM      43
#define ENTER_408_Y_BOTTOM      47

#define ENTER_410_X_BOTTOM      253
#define ENTER_410_Y_BOTTOM      105

#define NEITHER                 0
#define TOP_LEFT                1
#define BOTTOM_LEFT             2
#define TOP_RIGHT               3
#define BOTTOM_RIGHT            4

#define PID_SLIDE               0
#define PID_LEAVE               1
#define PID_FREEZE              2
#define PID_SLIDE_JUMP          3

#define FIRE                    60

static Scratch scratch;


static void which_pillar_409(int *it, int *new_x, int *new_y, int *adjoining) {
	int offset;

	if (inter_point_x < 99) {
		offset = 0;
	} else if (inter_point_x < 130) {
		offset = 1;
	} else if (inter_point_x < 168) {
		offset = 2;
	} else if (inter_point_x < 201) {
		offset = 3;
	} else {
		offset = 4;
	}

	if (inter_point_y < 31) {
		*it    = 1 + offset;
		*new_y = BASE_Y;
	} else if (inter_point_y < 61) {
		*it    = 6 + offset;
		*new_y = BASE_Y + 29;
	} else if (inter_point_y < 91) {
		*it    = 11 + offset;
		*new_y = BASE_Y + 58;
	} else if (inter_point_y < 121) {
		*it    = 16 + offset;
		*new_y = BASE_Y + 87;
	} else {
		*it    = 21 + offset;
		*new_y = BASE_Y + 116;
	}

	*new_x = BASE_X + (35 * offset);

	if (((*adjoining >= 7) && (*adjoining <= 9)) ||
	    ((*adjoining >= 12) && (*adjoining <= 14)) ||
	    ((*adjoining >= 17) && (*adjoining <= 19))) {
		if ((*it == *adjoining + 1) || (*it == *adjoining - 1) ||
		    (*it == *adjoining + 5) || (*it == *adjoining - 5)) {
			*adjoining = true;
		} else {
			*adjoining = false;
		}

	} else if ((*adjoining >= 2) && (*adjoining <= 4)) {
		if ((*it == *adjoining + 5) || (*it == *adjoining + 1) || (*it == *adjoining - 1)) {
			*adjoining = true;
		} else {
			*adjoining = false;
		}

	} else if ((*adjoining >= 22) && (*adjoining <= 24)) {
		if ((*it == *adjoining - 5) || (*it == *adjoining + 1) || (*it == *adjoining - 1)) {
			*adjoining = true;
		} else {
			*adjoining = false;
		}

	} else if ((*adjoining == 6) || (*adjoining == 11) || (*adjoining == 16)) {
		if ((*it == *adjoining - 5) || (*it == *adjoining + 5) || (*it == *adjoining + 1)) {
			*adjoining = true;
		} else {
			*adjoining = false;
		}

	} else if ((*adjoining == 10) || (*adjoining == 15) || (*adjoining == 20)) {
		if ((*it == *adjoining - 5) || (*it == *adjoining + 5) || (*it == *adjoining - 1)) {
			*adjoining = true;
		} else {
			*adjoining = false;
		}

	} else if (*adjoining == 1) {
		if ((*it == 2) || (*it == 6)) {
			*adjoining = true;
		} else {
			*adjoining = false;
		}

	} else if (*adjoining == 5) {
		if ((*it == 4) || (*it == 10)) {
			*adjoining = true;
		} else {
			*adjoining = false;
		}

	} else if (*adjoining == 21) {
		if ((*it == 22) || (*it == 16)) {
			*adjoining = true;
		} else {
			*adjoining = false;
		}
	}
}

static void handle_animation_pid() {
	int pid_reset_frame;
	int no_commands = false;

	if (kernel_anim[aa[0]].frame != local->pid_frame) {
		local->pid_frame = kernel_anim[aa[0]].frame;
		pid_reset_frame = -1;

		if (local->pid_action == PID_SLIDE_JUMP) {
			no_commands = true;
		}

		switch (local->pid_frame) {
		case 176:
		case 220:
			global[no_load_walker] = false;
			new_room               = 408;
			break;

		case 101:
		case 133:
		case 70:
		case 35:
			if (local->pid_frame == 101 || local->pid_frame == 133) {
				local->pid_action = PID_FREEZE;
			}

			switch (local->pid_action) {
			case PID_FREEZE:
				if (!no_commands) {
					player.commands_allowed = true;
				}
				if (local->on_shore == TOP_LEFT) {
					pid_reset_frame = 34;
				} else {
					pid_reset_frame = 69;
				}
				break;

			case PID_SLIDE:
			case PID_SLIDE_JUMP:
				player.commands_allowed = false;
				if (local->on_shore == TOP_LEFT) {
					pid_reset_frame = 70;
					local->on_shore = BOTTOM_LEFT;
					if (local->pid_action == PID_SLIDE) {
						local->new_y = ENTER_408_Y_BOTTOM;
					}
				} else {
					pid_reset_frame = 101;
					local->on_shore = TOP_LEFT;
					if (local->pid_action == PID_SLIDE) {
						local->new_y = ENTER_408_Y_BOTTOM - 29;
					}
				}
				break;

			case PID_LEAVE:
				player.commands_allowed = false;
				if (local->on_shore == TOP_LEFT) {
					pid_reset_frame = 133;
				} else {
					pid_reset_frame = 176;
				}
				break;
			}
			break;
		}

		if (pid_reset_frame >= 0) {
			kernel_reset_animation(aa[0], pid_reset_frame);
			local->pid_frame = pid_reset_frame;
		}
	}
}

static void handle_animation_pid_1() {
	int pid_1_reset_frame;
	int no_commands = false;

	if (kernel_anim[aa[1]].frame != local->pid_1_frame) {
		local->pid_1_frame = kernel_anim[aa[1]].frame;
		pid_1_reset_frame = -1;

		if (local->pid_action == PID_SLIDE_JUMP) {
			no_commands = true;
		}

		switch (local->pid_1_frame) {
		case 205:
		case 259:
			global[no_load_walker] = false;
			new_room               = 410;
			break;

		case 152:
		case 123:
		case 48:
		case 95:
			if (local->pid_1_frame == 123 || local->pid_1_frame == 152) {
				local->pid_action = PID_FREEZE;
			}

			switch (local->pid_action) {
			case PID_FREEZE:
				if (!no_commands) {
					player.commands_allowed = true;
				}
				if (local->on_shore == TOP_RIGHT) {
					pid_1_reset_frame = 47;
				} else {
					pid_1_reset_frame = 94;
				}
				break;

			case PID_SLIDE:
			case PID_SLIDE_JUMP:
				player.commands_allowed = false;
				if (local->on_shore == TOP_RIGHT) {
					pid_1_reset_frame = 123;
					local->on_shore   = BOTTOM_RIGHT;
					if (local->pid_action == PID_SLIDE) {
						local->new_y = ENTER_410_Y_BOTTOM;
					}
				} else {
					pid_1_reset_frame = 95;
					local->on_shore   = TOP_RIGHT;
					if (local->pid_action == PID_SLIDE) {
						local->new_y = ENTER_410_Y_BOTTOM - 29;
					}
				}
				break;

			case PID_LEAVE:
				player.commands_allowed = false;
				if (local->on_shore == TOP_RIGHT) {
					pid_1_reset_frame = 205;
				} else {
					pid_1_reset_frame = 152;
				}
				break;
			}
			break;
		}

		if (pid_1_reset_frame >= 0) {
			kernel_reset_animation(aa[1], pid_1_reset_frame);
			local->pid_1_frame = pid_1_reset_frame;
		}
	}
}

static void room_409_init() {
	global[perform_displacements] = true;

	local->fire = 0;

	global[grid_position] = 1;
	global[grid_position + 1] = 2;
	global[grid_position + 2] = 7;
	global[grid_position + 3] = 6;
	global[grid_position + 4] = 11;
	global[grid_position + 5] = 16;
	global[grid_position + 6] = 17;
	global[grid_position + 7] = 22;
	global[grid_position + 8] = 23;
	global[grid_position + 9] = 18;
	global[grid_position + 10] = 13;
	global[grid_position + 11] = 8;
	global[grid_position + 12] = 3;
	global[grid_position + 13] = 4;
	global[grid_position + 14] = 5;
	global[grid_position + 15] = 10;
	global[grid_position + 16] = 9;
	global[grid_position + 17] = 14;
	global[grid_position + 18] = 19;
	global[grid_position + 19] = 24;
	global[grid_position + 20] = 25;
	global[grid_position + 21] = 20;
	global[grid_position + 22] = 15;

	if (kernel.teleported_in) {
		global[player_persona] = PLAYER_IS_PID;
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->standing_on = 0;
		local->move_counter = 0;
	}

	ss[fx_n] = kernel_load_series("*RM510B0", false);
	ss[fx_s] = kernel_load_series("*RM510B3", false);
	ss[fx_ew] = kernel_load_series("*RM510B1", false);
	ss[fx_death_n] = kernel_load_series(kernel_name('b', 3), false);
	ss[fx_death_ew] = kernel_load_series(kernel_name('b', 4), false);
	ss[fx_death_s] = kernel_load_series(kernel_name('b', 5), false);
	ss[fx_fire] = kernel_load_series(kernel_name('x', 0), false);

	player.walker_visible = false;

	if (previous_room == KERNEL_RESTORING_GAME) {
		if (local->standing_on) {
			switch (local->facing) {
			case FACING_NORTH:
				seq[fx_n] = kernel_seq_stamp(ss[fx_n], false, 13);
				kernel_seq_loc(seq[fx_n], local->new_x - 1, local->new_y + 11);
				kernel_seq_depth(seq[fx_n], 1);
				break;

			case FACING_SOUTH:
				seq[fx_s] = kernel_seq_stamp(ss[fx_s], true, 12);
				kernel_seq_loc(seq[fx_s], local->new_x + 3, local->new_y + 36);
				kernel_seq_depth(seq[fx_s], 1);
				break;

			case FACING_EAST:
				seq[fx_ew] = kernel_seq_stamp(ss[fx_ew], true, 13);
				kernel_seq_loc(seq[fx_ew], local->new_x + 2, local->new_y + 19);
				kernel_seq_depth(seq[fx_ew], 1);
				break;

			case FACING_WEST:
				seq[fx_ew] = kernel_seq_stamp(ss[fx_ew], false, 13);
				kernel_seq_loc(seq[fx_ew], local->new_x + 2, local->new_y + 19);
				kernel_seq_depth(seq[fx_ew], 1);
				break;
			}
		} else {
			if (local->on_shore == TOP_LEFT) {
				aa[0] = kernel_run_animation(kernel_name('l', -1), 0);
				kernel_reset_animation(aa[0], 35);
			} else if (local->on_shore == BOTTOM_LEFT) {
				aa[0] = kernel_run_animation(kernel_name('l', -1), 0);
				kernel_reset_animation(aa[0], 70);
			} else if (local->on_shore == TOP_RIGHT) {
				aa[1] = kernel_run_animation(kernel_name('r', -1), 0);
				kernel_reset_animation(aa[0], 95);
			} else if (local->on_shore == BOTTOM_RIGHT) {
				aa[1] = kernel_run_animation(kernel_name('r', -1), 0);
				kernel_reset_animation(aa[0], 48);
			}
		}

	} else if (previous_room == 409) {
		if (global[move_direction_409]) {
			aa[0] = kernel_run_animation(kernel_name('l', -1), 0);
			kernel_reset_animation(aa[0], 70);
			local->anim_0_running = true;
			local->on_shore = BOTTOM_LEFT;
			local->pid_action = PID_FREEZE;
			local->facing = FACING_EAST;
			local->new_x = ENTER_408_X_BOTTOM;
			local->new_y = ENTER_408_Y_BOTTOM;
		} else {
			aa[1] = kernel_run_animation(kernel_name('r', -1), 0);
			kernel_reset_animation(aa[0], 48);
			local->anim_1_running = true;
			local->on_shore = BOTTOM_RIGHT;
			local->pid_action = PID_FREEZE;
			local->facing = FACING_WEST;
			local->new_x = ENTER_410_X_BOTTOM;
			local->new_y = ENTER_410_Y_BOTTOM;
		}

	} else if (previous_room == 410) {
		aa[1] = kernel_run_animation(kernel_name('r', -1), 0);
		kernel_reset_animation(aa[1], 49);
		player.commands_allowed = false;
		local->anim_1_running = true;
		local->pid_action = PID_FREEZE;
		local->on_shore = BOTTOM_RIGHT;
		local->facing = FACING_WEST;
		local->new_x = ENTER_410_X_BOTTOM;
		local->new_y = ENTER_410_Y_BOTTOM;

	} else if (previous_room == 408 || previous_room != KERNEL_RESTORING_GAME) {
		aa[0] = kernel_run_animation(kernel_name('l', -1), 0);
		kernel_reset_animation(aa[0], 36);
		player.commands_allowed = false;
		local->anim_0_running = true;
		local->pid_action = PID_FREEZE;
		local->on_shore = BOTTOM_LEFT;
		local->facing = FACING_EAST;
		local->new_x = ENTER_408_X_BOTTOM;
		local->new_y = ENTER_408_Y_BOTTOM;
	}

	section_4_music();
}

static void room_409_daemon() {
	int x, y;

	if (local->anim_0_running) {
		handle_animation_pid();
	}

	if (local->anim_1_running) {
		handle_animation_pid_1();
	}

	if (kernel.trigger == ROOM_409_SOUND) {
		sound_play(N_JumpThwang);
	}

	if (imath_random(1, 100) == 1 && local->fire == 0) {
		local->fire = imath_random(1, 25);
		while (local->standing_on == local->fire) {
			local->fire = imath_random(1, 25);
		}
		seq[fx_fire] = kernel_seq_forward(ss[fx_fire], false, 7, 0, 0, 1);
		kernel_seq_depth(seq[fx_fire], 1);
		kernel_seq_range(seq[fx_fire], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_fire], KERNEL_TRIGGER_EXPIRE, 0, FIRE);

		if (local->fire <= 5)       y = BASE_Y + 10;
		else if (local->fire <= 10) y = BASE_Y + 39;
		else if (local->fire <= 15) y = BASE_Y + 68;
		else if (local->fire <= 20) y = BASE_Y + 97;
		else                        y = BASE_Y + 126;

		switch (local->fire) {
		case 1: case 6:  case 11: case 16: case 21: x = BASE_X + 1;   break;
		case 2: case 7:  case 12: case 17: case 22: x = BASE_X + 36;  break;
		case 3: case 8:  case 13: case 18: case 23: x = BASE_X + 71;  break;
		case 4: case 9:  case 14: case 19: case 24: x = BASE_X + 106; break;
		default:                                     x = BASE_X + 141; break;
		}
		kernel_seq_loc(seq[fx_fire], x, y);
	}

	if (kernel.trigger == FIRE) {
		local->fire = 0;
	}
}

static void room_409_pre_parser() {
	player.need_to_walk = false;
}

static void room_409_parser() {
	int temp;
	int selected;
	int new_x;
	int new_y;
	int adjoining;

	if (kernel.trigger == ROOM_409_SOUND) {
		sound_play(N_StepOnFloatingDisk);
		player.command_ready = false;
		return;
	}

	if (kernel.trigger == ROOM_409_END_DEATH) {
		sound_play(N_PlayerDies);
		if (game.difficulty == EASY_MODE) {
			text_show(40914);
		} else {
			text_show(45);
		}
		kernel.force_restart = true;
		player.command_ready = false;
		return;
	}

	switch (kernel.trigger) {
	case ROOM_409_JUMP:
		if (local->on_shore) {
			if (local->anim_0_running) {
				kernel_abort_animation(aa[0]);
				local->anim_0_running = false;
			} else {
				kernel_abort_animation(aa[1]);
				local->anim_1_running = false;
			}
			player.commands_allowed = false;

		} else {
			switch (local->facing) {
			case FACING_NORTH:
				kernel_seq_delete(seq[fx_n]);
				break;

			case FACING_SOUTH:
				kernel_seq_delete(seq[fx_s]);
				break;

			case FACING_EAST:
			case FACING_WEST:
				kernel_seq_delete(seq[fx_ew]);
				break;
			}
		}

		switch (local->facing) {
		case FACING_NORTH:
			seq[fx_n] = kernel_seq_forward(ss[fx_n], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_n], 1);
			kernel_seq_range(seq[fx_n], 15, 34);
			kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_END_OF_JUMP);
			kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_SPRITE, 30, ROOM_409_SOUND);
			kernel_seq_loc(seq[fx_n], local->new_x, local->new_y + 40);
			break;

		case FACING_SOUTH:
			seq[fx_s] = kernel_seq_forward(ss[fx_s], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_s], 1);
			kernel_seq_range(seq[fx_s], 15, 33);
			kernel_seq_trigger(seq[fx_s], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_END_OF_JUMP);
			kernel_seq_trigger(seq[fx_s], KERNEL_TRIGGER_SPRITE, 29, ROOM_409_SOUND);
			kernel_seq_loc(seq[fx_s], local->new_x, local->new_y + 7);
			break;

		case FACING_EAST:
			seq[fx_ew] = kernel_seq_forward(ss[fx_ew], true, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_ew], 1);
			kernel_seq_range(seq[fx_ew], 16, 34);
			kernel_seq_trigger(seq[fx_ew], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_END_OF_JUMP);
			kernel_seq_trigger(seq[fx_ew], KERNEL_TRIGGER_SPRITE, 30, ROOM_409_SOUND);
			if (local->on_shore == TOP_LEFT) {
				kernel_seq_loc(seq[fx_ew], ENTER_408_X_BOTTOM, ENTER_408_Y_BOTTOM - 11);
				kernel_synch(KERNEL_SERIES, seq[fx_ew], KERNEL_NOW, 0);
				local->on_shore = NEITHER;
			} else if (local->on_shore == BOTTOM_LEFT) {
				kernel_seq_loc(seq[fx_ew], ENTER_408_X_BOTTOM + 2, ENTER_408_Y_BOTTOM + 17);
				kernel_synch(KERNEL_SERIES, seq[fx_ew], KERNEL_NOW, 0);
				local->on_shore = NEITHER;
			} else if (local->on_shore == NEITHER) {
				kernel_seq_loc(seq[fx_ew], local->old_x + 2, local->new_y + 19);
			}
			break;

		case FACING_WEST:
			seq[fx_ew] = kernel_seq_forward(ss[fx_ew], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_ew], 1);
			kernel_seq_range(seq[fx_ew], 16, 34);
			kernel_seq_trigger(seq[fx_ew], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_END_OF_JUMP);
			kernel_seq_trigger(seq[fx_ew], KERNEL_TRIGGER_SPRITE, 30, ROOM_409_SOUND);
			if (local->on_shore == TOP_RIGHT) {
				kernel_seq_loc(seq[fx_ew], ENTER_410_X_BOTTOM, ENTER_410_Y_BOTTOM - 11);
				kernel_synch(KERNEL_SERIES, seq[fx_ew], KERNEL_NOW, 0);
				local->on_shore = NEITHER;
			} else if (local->on_shore == BOTTOM_RIGHT) {
				kernel_seq_loc(seq[fx_ew], ENTER_410_X_BOTTOM + 2, ENTER_410_Y_BOTTOM + 17);
				kernel_synch(KERNEL_SERIES, seq[fx_ew], KERNEL_NOW, 0);
				local->on_shore = NEITHER;
			} else if (local->on_shore == NEITHER) {
				kernel_seq_loc(seq[fx_ew], local->old_x + 2, local->new_y + 19);
			}
			break;
		}
		player.command_ready = false;
		return;

	case ROOM_409_END_OF_JUMP:

		seq[fx_death_n]  = -1;
		seq[fx_death_s]  = -1;
		seq[fx_death_ew] = -1;

		if (local->standing_on < 400) {

			if ((global[move_direction_409] && global[grid_position + local->move_counter] != local->standing_on) ||
			    (!global[move_direction_409] && global[global[max_grid_value] - local->move_counter] != local->standing_on)) {

				sound_play(N_FlameBurst);
				switch (local->facing) {
				case FACING_NORTH:
					seq[fx_death_n] = kernel_seq_forward(ss[fx_death_n], false, 6, 0, 0, 1);
					kernel_seq_depth(seq[fx_death_n], 1);
					kernel_seq_range(seq[fx_death_n], KERNEL_FIRST, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_death_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_END_DEATH);
					kernel_seq_loc(seq[fx_death_n], local->new_x - 1, local->new_y + 17);
					break;

				case FACING_SOUTH:
					seq[fx_death_s] = kernel_seq_forward(ss[fx_death_s], false, 6, 0, 0, 1);
					kernel_seq_depth(seq[fx_death_s], 1);
					kernel_seq_range(seq[fx_death_s], KERNEL_FIRST, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_death_s], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_END_DEATH);
					kernel_seq_loc(seq[fx_death_s], local->new_x + 3, local->new_y + 29);
					break;

				case FACING_EAST:
				case FACING_WEST:
					if (local->facing == FACING_EAST) {
						seq[fx_death_ew] = kernel_seq_forward(ss[fx_death_ew], false, 6, 0, 0, 1);
						kernel_seq_loc(seq[fx_death_ew], local->new_x + 7, local->new_y + 29);
					} else {
						seq[fx_death_ew] = kernel_seq_forward(ss[fx_death_ew], true, 6, 0, 0, 1);
						kernel_seq_loc(seq[fx_death_ew], local->new_x - 3, local->new_y + 30);
					}
					kernel_seq_depth(seq[fx_death_ew], 1);
					kernel_seq_range(seq[fx_death_ew], KERNEL_FIRST, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_death_ew], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_END_DEATH);
					break;
				}
			}
		}

		++local->move_counter;

		if (seq[fx_death_n] == -1 && seq[fx_death_s] == -1 && seq[fx_death_ew] == -1) {

			switch (local->facing) {

			case FACING_NORTH:
				temp      = seq[fx_n];
				seq[fx_n] = kernel_seq_stamp(ss[fx_n], false, 13);
				kernel_seq_loc(seq[fx_n], local->new_x - 1, local->new_y + 11);
				kernel_seq_depth(seq[fx_n], 2);
				kernel_synch(KERNEL_SERIES, seq[fx_n], KERNEL_SERIES, temp);
				local->facing   = FACING_NORTH;
				local->on_shore = NEITHER;
				if (imath_random(1, 7) == 1) {
					kernel_timing_trigger(1, ROOM_409_WHOA_NORTH);
				} else {
					player.commands_allowed = true;
				}
				break;

			case FACING_SOUTH:
				temp      = seq[fx_s];
				seq[fx_s] = kernel_seq_stamp(ss[fx_s], true, 12);
				kernel_seq_loc(seq[fx_s], local->new_x + 3, local->new_y + 36);
				kernel_seq_depth(seq[fx_s], 2);
				kernel_synch(KERNEL_SERIES, seq[fx_s], KERNEL_SERIES, temp);
				local->facing   = FACING_SOUTH;
				local->on_shore = NEITHER;
				if (imath_random(1, 7) == 1) {
					kernel_timing_trigger(1, ROOM_409_WHOA_SOUTH);
				} else {
					player.commands_allowed = true;
				}
				break;

			case FACING_EAST:
				temp       = seq[fx_ew];
				if (local->standing_on == 410) {
					aa[1] = kernel_run_animation(kernel_name('r', -1), 0);
					kernel_synch(KERNEL_ANIM, aa[1], KERNEL_SERIES, temp);
					if (local->new_y == ENTER_410_Y_BOTTOM) {
						kernel_reset_animation(aa[1], 161);
					} else {
						kernel_reset_animation(aa[1], 214);
					}
					local->anim_1_running = true;
				} else {
					seq[fx_ew] = kernel_seq_stamp(ss[fx_ew], true, 13);
					kernel_seq_loc(seq[fx_ew], local->new_x + 2, local->new_y + 19);
					kernel_seq_depth(seq[fx_ew], 2);
					kernel_synch(KERNEL_SERIES, seq[fx_ew], KERNEL_SERIES, temp);
					player.commands_allowed = true;
					local->facing           = FACING_EAST;
					local->on_shore         = NEITHER;
				}
				if (!(global[player_score_flags] & SCORE_CROSS_SKY)) {
					global[player_score_flags] = global[player_score_flags] | SCORE_CROSS_SKY;
					global[player_score] += 2;
				}
				break;

			case FACING_WEST:
				temp       = seq[fx_ew];
				if (local->standing_on == 408) {
					aa[0] = kernel_run_animation(kernel_name('l', -1), 0);
					kernel_synch(KERNEL_ANIM, aa[0], KERNEL_SERIES, temp);
					if (local->new_y == ENTER_408_Y_BOTTOM) {
						kernel_reset_animation(aa[0], 183);
					} else {
						kernel_reset_animation(aa[0], 141);
					}
					local->anim_0_running = true;
				} else {
					seq[fx_ew] = kernel_seq_stamp(ss[fx_ew], false, 13);
					kernel_seq_loc(seq[fx_ew], local->new_x + 2, local->new_y + 19);
					kernel_seq_depth(seq[fx_ew], 2);
					kernel_synch(KERNEL_SERIES, seq[fx_ew], KERNEL_SERIES, temp);
					player.commands_allowed = true;
					local->facing           = FACING_WEST;
					local->on_shore         = NEITHER;
				}
				break;
			}
		}
		player.command_ready = false;
		return;

	case ROOM_409_TURN:
		switch (local->old_facing) {

		case FACING_NORTH:
			kernel_seq_delete(seq[fx_n]);
			switch (local->facing) {
			case FACING_EAST:
				seq[fx_n] = kernel_seq_forward(ss[fx_n], true, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 36, 39);
				kernel_seq_loc(seq[fx_n], local->old_x + 2, local->old_y + 11);
				break;
			case FACING_WEST:
				seq[fx_n] = kernel_seq_forward(ss[fx_n], false, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 36, 39);
				kernel_seq_loc(seq[fx_n], local->old_x - 1, local->old_y + 11);
				break;
			case FACING_SOUTH:
				seq[fx_n] = kernel_seq_forward(ss[fx_n], false, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 36, 43);
				kernel_seq_loc(seq[fx_n], local->old_x - 1, local->old_y + 11);
				break;
			}
			kernel_seq_depth(seq[fx_n], 1);
			kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_END_OF_TURN);
			break;

		case FACING_SOUTH:
			kernel_seq_delete(seq[fx_s]);
			switch (local->facing) {
			case FACING_EAST:
				seq[fx_n] = kernel_seq_backward(ss[fx_n], true, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 40, 43);
				kernel_seq_loc(seq[fx_n], local->old_x + 2, local->old_y + 11);
				break;
			case FACING_WEST:
				seq[fx_n] = kernel_seq_backward(ss[fx_n], false, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 40, 43);
				kernel_seq_loc(seq[fx_n], local->old_x - 1, local->old_y + 11);
				break;
			case FACING_NORTH:
				seq[fx_n] = kernel_seq_backward(ss[fx_n], false, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 36, 43);
				kernel_seq_loc(seq[fx_n], local->old_x - 1, local->old_y + 11);
				break;
			}
			kernel_seq_depth(seq[fx_n], 1);
			kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_END_OF_TURN);
			break;

		case FACING_WEST:
			kernel_seq_delete(seq[fx_ew]);
			switch (local->facing) {
			case FACING_EAST:
				seq[fx_n] = kernel_seq_backward(ss[fx_n], false, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 36, 39);
				kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_TURN + 2);
				kernel_seq_loc(seq[fx_n], local->old_x + 2, local->old_y + 11);
				break;
			case FACING_NORTH:
				seq[fx_n] = kernel_seq_backward(ss[fx_n], false, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 36, 39);
				kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_END_OF_TURN);
				kernel_seq_loc(seq[fx_n], local->old_x + 1, local->old_y + 11);
				break;
			case FACING_SOUTH:
				seq[fx_n] = kernel_seq_forward(ss[fx_n], false, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 40, 43);
				kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_END_OF_TURN);
				kernel_seq_loc(seq[fx_n], local->old_x - 1, local->old_y + 11);
				break;
			}
			kernel_seq_depth(seq[fx_n], 1);
			break;

		case FACING_EAST:
			kernel_seq_delete(seq[fx_ew]);
			switch (local->facing) {
			case FACING_NORTH:
				seq[fx_n] = kernel_seq_backward(ss[fx_n], true, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 36, 39);
				kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_END_OF_TURN);
				kernel_seq_loc(seq[fx_n], local->old_x + 2, local->old_y + 11);
				break;
			case FACING_WEST:
				seq[fx_n] = kernel_seq_backward(ss[fx_n], true, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 36, 39);
				kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_TURN + 1);
				kernel_seq_loc(seq[fx_n], local->old_x + 2, local->old_y + 11);
				break;
			case FACING_SOUTH:
				seq[fx_n] = kernel_seq_forward(ss[fx_n], true, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 40, 43);
				kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_END_OF_TURN);
				kernel_seq_loc(seq[fx_n], local->old_x + 2, local->old_y + 11);
				break;
			}
			kernel_seq_depth(seq[fx_n], 1);
			break;
		}
		player.command_ready = false;
		return;

	case ROOM_409_TURN + 1:
		seq[fx_n] = kernel_seq_forward(ss[fx_n], false, 7, 0, 0, 1);
		kernel_seq_range(seq[fx_n], 36, 39);
		kernel_seq_depth(seq[fx_n], 1);
		kernel_seq_loc(seq[fx_n], local->old_x + 2, local->old_y + 11);
		kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_END_OF_TURN);
		player.command_ready = false;
		return;

	case ROOM_409_TURN + 2:
		seq[fx_n] = kernel_seq_forward(ss[fx_n], true, 7, 0, 0, 1);
		kernel_seq_range(seq[fx_n], 36, 39);
		kernel_seq_depth(seq[fx_n], 1);
		kernel_seq_loc(seq[fx_n], local->old_x + 2, local->old_y + 11);
		kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_END_OF_TURN);
		player.command_ready = false;
		return;

	case ROOM_409_END_OF_TURN:
		switch (local->facing) {
		case FACING_NORTH:
			temp      = seq[fx_n];
			seq[fx_n] = kernel_seq_stamp(ss[fx_n], false, 13);
			kernel_seq_loc(seq[fx_n], local->old_x - 1, local->old_y + 11);
			kernel_seq_depth(seq[fx_n], 1);
			kernel_synch(KERNEL_SERIES, seq[fx_n], KERNEL_SERIES, temp);
			break;
		case FACING_SOUTH:
			temp      = seq[fx_n];
			seq[fx_s] = kernel_seq_stamp(ss[fx_s], true, 12);
			kernel_seq_loc(seq[fx_s], local->old_x + 3, local->old_y + 36);
			kernel_seq_depth(seq[fx_s], 1);
			kernel_synch(KERNEL_SERIES, seq[fx_s], KERNEL_SERIES, temp);
			break;
		case FACING_EAST:
			temp       = seq[fx_n];
			seq[fx_ew] = kernel_seq_stamp(ss[fx_ew], true, 13);
			kernel_seq_loc(seq[fx_ew], local->old_x + 2, local->old_y + 19);
			kernel_seq_depth(seq[fx_ew], 1);
			kernel_synch(KERNEL_SERIES, seq[fx_ew], KERNEL_SERIES, temp);
			break;
		case FACING_WEST:
			temp       = seq[fx_n];
			seq[fx_ew] = kernel_seq_stamp(ss[fx_ew], false, 13);
			kernel_seq_loc(seq[fx_ew], local->old_x + 2, local->old_y + 19);
			kernel_seq_depth(seq[fx_ew], 1);
			kernel_synch(KERNEL_SERIES, seq[fx_ew], KERNEL_SERIES, temp);
			break;
		}
		kernel_timing_trigger(4, ROOM_409_JUMP);
		player.command_ready = false;
		return;

	case ROOM_409_WHOA_SOUTH:
		sound_play(N_004CryOfDismay);
		kernel_seq_delete(seq[fx_s]);
		seq[fx_s] = kernel_seq_forward(ss[fx_s], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_s], 1);
		kernel_seq_range(seq[fx_s], 35, 54);
		kernel_seq_loc(seq[fx_s], local->new_x + 1, local->new_y + 37);
		kernel_seq_trigger(seq[fx_s], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_WHOA_SOUTH + 1);
		player.command_ready = false;
		return;

	case ROOM_409_WHOA_SOUTH + 1:
		temp      = seq[fx_s];
		seq[fx_s] = kernel_seq_stamp(ss[fx_s], true, 12);
		kernel_seq_loc(seq[fx_s], local->new_x + 3, local->new_y + 36);
		kernel_seq_depth(seq[fx_s], 1);
		kernel_synch(KERNEL_SERIES, seq[fx_s], KERNEL_SERIES, temp);
		player.commands_allowed = true;
		player.command_ready = false;
		return;

	case ROOM_409_WHOA_NORTH:
		sound_play(N_004CryOfDismay);
		kernel_seq_delete(seq[fx_n]);
		seq[fx_n] = kernel_seq_forward(ss[fx_n], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_n], 1);
		kernel_seq_range(seq[fx_n], 44, 63);
		kernel_seq_loc(seq[fx_n], local->new_x - 1, local->new_y + 11);
		kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_WHOA_NORTH + 1);
		player.command_ready = false;
		return;

	case ROOM_409_WHOA_NORTH + 1:
		temp      = seq[fx_n];
		seq[fx_n] = kernel_seq_stamp(ss[fx_n], false, 13);
		kernel_seq_loc(seq[fx_n], local->new_x - 1, local->new_y + 11);
		kernel_seq_depth(seq[fx_n], 1);
		kernel_synch(KERNEL_SERIES, seq[fx_n], KERNEL_SERIES, temp);
		player.commands_allowed = true;
		player.command_ready = false;
		return;
	}

	if (kernel.trigger == ROOM_409_WAIT_FOR_END_SLIDE) {
		if (local->pid_action != PID_FREEZE) {
			kernel_timing_trigger(1, ROOM_409_WAIT_FOR_END_SLIDE);
		} else {
			kernel_timing_trigger(1, ROOM_409_JUMP);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(jump_to, ledge) ||
	    player_said_2(walk_down, path_to_west) ||
	    player_said_2(walk_to, east_end_of_island) ||
	    player_said_2(walk_across, ledge)) {

		local->old_facing = local->facing;

		if (!local->standing_on) {
			if (local->on_shore == TOP_LEFT || local->on_shore == BOTTOM_LEFT) {
				if (inter_point_x < 62) {
					if (player_said_1(walk_down)) {
						local->pid_action = PID_LEAVE;
					} else if (local->on_shore) {
						local->pid_action = PID_SLIDE;
					}
					player.command_ready = false;
					return;
				} else {
					text_show(40911);
					player.command_ready = false;
					return;
				}

			} else if (local->on_shore == TOP_RIGHT || local->on_shore == BOTTOM_RIGHT) {
				if (inter_point_x > 235) {
					if (player_said_1(walk_to)) {
						local->pid_action = PID_LEAVE;
					} else if (local->on_shore) {
						local->pid_action = PID_SLIDE;
					}
					player.command_ready = false;
					return;
				} else {
					text_show(40911);
					player.command_ready = false;
					return;
				}
			}

		} else if (local->standing_on == 1) {
			if (inter_point_x > 100) {
				text_show(40911);
				player.command_ready = false;
				return;
			} else {
				local->old_x  = local->new_x;
				local->old_y  = local->new_y;
				local->new_x  = ENTER_408_X_BOTTOM - 3;
				local->new_y  = ENTER_408_Y_BOTTOM - 29;
				local->facing = FACING_WEST;
			}

		} else if (local->standing_on == 6) {
			if (inter_point_x > 100) {
				text_show(40911);
				player.command_ready = false;
				return;
			} else {
				local->old_x  = local->new_x;
				local->old_y  = local->new_y;
				local->new_x  = ENTER_408_X_BOTTOM - 2;
				local->new_y  = ENTER_408_Y_BOTTOM;
				local->facing = FACING_WEST;
			}

		} else if (local->standing_on == 15) {
			if (inter_point_x < 100) {
				text_show(40911);
				player.command_ready = false;
				return;
			} else {
				local->old_x  = local->new_x;
				local->old_y  = local->new_y;
				local->new_x  = ENTER_410_X_BOTTOM - 3;
				local->new_y  = ENTER_410_Y_BOTTOM - 29;
				local->facing = FACING_EAST;
			}

		} else if (local->standing_on == 20) {
			if (inter_point_x < 100) {
				text_show(40911);
				player.command_ready = false;
				return;
			} else {
				local->old_x  = local->new_x;
				local->old_y  = local->new_y;
				local->new_x  = ENTER_410_X_BOTTOM - 3;
				local->new_y  = ENTER_410_Y_BOTTOM;
				local->facing = FACING_EAST;
			}

		} else {
			text_show(40911);
			player.command_ready = false;
			return;
		}

		if (local->standing_on == 15 || local->standing_on == 20) {
			local->standing_on = 410;
		} else if (local->standing_on == 1 || local->standing_on == 6) {
			local->standing_on = 408;
		}

		player.commands_allowed = false;

		if (local->old_facing == local->facing) {
			kernel_timing_trigger(1, ROOM_409_JUMP);
		} else {
			kernel_timing_trigger(1, ROOM_409_TURN);
		}

		player.command_ready = false;
		return;
	}

	if (player_said_2(jump_to, floating_disk)) {

		local->old_x = local->new_x;
		local->old_y = local->new_y;

		adjoining = local->standing_on;
		which_pillar_409(&selected, &new_x, &new_y, &adjoining);

		if (local->on_shore == TOP_LEFT) {
			if (selected == 6) {
				local->pid_action = PID_SLIDE_JUMP;
				kernel_timing_trigger(1, ROOM_409_WAIT_FOR_END_SLIDE);
				local->standing_on      = selected;
				player.commands_allowed = false;
				local->new_x = new_x;
				local->new_y = new_y;
				player.command_ready = false;
				return;
			} else if (selected != 1) {
				text_show(40912);
				player.command_ready = false;
				return;
			}

		} else if (local->on_shore == BOTTOM_LEFT) {
			if (selected == 1) {
				local->pid_action = PID_SLIDE_JUMP;
				kernel_timing_trigger(1, ROOM_409_WAIT_FOR_END_SLIDE);
				local->standing_on      = selected;
				player.commands_allowed = false;
				local->new_x = new_x;
				local->new_y = new_y;
				player.command_ready = false;
				return;
			} else if (selected != 6) {
				text_show(40912);
				player.command_ready = false;
				return;
			}

		} else if (local->on_shore == TOP_RIGHT) {
			if (selected == 20) {
				local->pid_action = PID_SLIDE_JUMP;
				kernel_timing_trigger(1, ROOM_409_WAIT_FOR_END_SLIDE);
				local->standing_on      = selected;
				player.commands_allowed = false;
				local->new_x = new_x;
				local->new_y = new_y;
				player.command_ready = false;
				return;
			} else if (selected != 15) {
				text_show(40912);
				player.command_ready = false;
				return;
			}

		} else if (local->on_shore == BOTTOM_RIGHT) {
			if (selected == 15) {
				local->pid_action = PID_SLIDE_JUMP;
				kernel_timing_trigger(1, ROOM_409_WAIT_FOR_END_SLIDE);
				local->standing_on      = selected;
				player.commands_allowed = false;
				local->new_x = new_x;
				local->new_y = new_y;
				player.command_ready = false;
				return;
			} else if (selected != 20) {
				text_show(40912);
				player.command_ready = false;
				return;
			}
		}

		if (selected == local->standing_on) {
			text_show(40913);
			player.command_ready = false;
			return;

		} else if (adjoining || local->on_shore) {

			local->new_x = new_x;
			local->new_y = new_y;

			local->old_facing = local->facing;

			if (!local->on_shore) {
				if (selected == local->standing_on + 1) {
					local->facing = FACING_EAST;
				} else if (selected == local->standing_on - 1) {
					local->facing = FACING_WEST;
				} else if (selected == local->standing_on + 5) {
					local->facing = FACING_SOUTH;
				} else if (selected == local->standing_on - 5) {
					local->facing = FACING_NORTH;
				}
			}

			local->standing_on      = selected;
			player.commands_allowed = false;

			if ((local->old_facing == local->facing) || local->on_shore) {
				kernel_timing_trigger(1, ROOM_409_JUMP);
			} else {
				kernel_timing_trigger(1, ROOM_409_TURN);
			}

		} else {
			text_show(40912);
		}

		player.command_ready = false;
		return;
	}

	if (player.look_around) {
		if (game.difficulty == HARD_MODE) {
			text_show(40901);
		} else {
			text_show(40902);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(ledge)) {
			text_show(40904);
			player.command_ready = false;
			return;
		}

		if (player_said_1(floating_disk)) {
			text_show(40905);
			player.command_ready = false;
			return;
		}

		if (player_said_1(path_to_west)) {
			text_show(40907);
			player.command_ready = false;
			return;
		}

		if (player_said_1(east_end_of_island)) {
			text_show(40908);
			player.command_ready = false;
			return;
		}

		if (player_said_1(spirit_plane)) {
			text_show(40909);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_1(take) || player_said_1(push) || player_said_1(pull)) {
		if (player_said_1(floating_disk)) {
			text_show(40906);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_1(secret_message)) {
		text_show(40903);
		player.command_ready = false;
		return;
	}
}

void room_409_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.facing);
	s.syncAsSint16LE(scratch.new_x);
	s.syncAsSint16LE(scratch.new_y);
	s.syncAsSint16LE(scratch.old_x);
	s.syncAsSint16LE(scratch.old_y);
	s.syncAsSint16LE(scratch.old_facing);
	s.syncAsSint16LE(scratch.on_shore);
	s.syncAsSint16LE(scratch.standing_on);
	s.syncAsSint16LE(scratch.pid_frame);
	s.syncAsSint16LE(scratch.pid_action);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.pid_1_frame);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.move_counter);
	s.syncAsSint16LE(scratch.fire);
}

void room_409_preload() {
	room_init_code_pointer       = room_409_init;
	room_pre_parser_code_pointer = room_409_pre_parser;
	room_parser_code_pointer     = room_409_parser;
	room_daemon_code_pointer     = room_409_daemon;

	global[no_load_walker] = true;

	if (kernel.teleported_in) {
		global[player_persona] = PLAYER_IS_PID;
	}

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
