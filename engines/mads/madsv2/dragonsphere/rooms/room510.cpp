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
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section5.h"
#include "mads/madsv2/dragonsphere/rooms/room510.h"

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
	int16 move_counter;
	int16 first_jump;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_n                    0
#define fx_ew                   1
#define fx_s                    2
#define fx_belt                 3

#define ROOM_510_DONE_ENTER_FROM_509 60
#define ROOM_510_JUMP                65
#define ROOM_510_END_OF_JUMP         67
#define ROOM_510_TURN                70
#define ROOM_510_END_OF_TURN         75
#define ROOM_510_WHOA_SOUTH          80
#define ROOM_510_WHOA_NORTH          85
#define ROOM_510_WAIT_FOR_END_SLIDE  90
#define ROOM_510_SOUND               120

#define BASE_X                  133
#define BASE_Y                  19

#define ENTER_509_X_BOTTOM      308
#define ENTER_509_Y_BOTTOM      48

#define ENTER_512_X_BOTTOM      98
#define ENTER_512_Y_BOTTOM      106

#define NEITHER                 0
#define TOP_LEFT                1
#define BOTTOM_LEFT             2
#define TOP_RIGHT               3
#define BOTTOM_RIGHT            4

#define PID_SLIDE               0
#define PID_LEAVE               1
#define PID_FREEZE              2
#define PID_SLIDE_JUMP          3


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
		case 101:
		case 115:
			new_room = 509;
			break;

		case 122:
			pid_reset_frame = 101;
			break;

		case 130:
			pid_reset_frame = 86;
			break;

		case 57:
		case 84:
		case 14:
		case 28:
		case 85:
		case 86:
		case 149:
			if (local->pid_frame == 14 && !player.been_here_before) {
				pid_reset_frame = 131;

			} else {

				if (local->pid_frame == 57 || local->pid_frame == 84) {
					local->pid_action = PID_FREEZE;
				}

				switch (local->pid_action) {
				case PID_FREEZE:
					if (!no_commands) {
						player.commands_allowed = true;
					}
					if (local->on_shore == TOP_RIGHT) {
						pid_reset_frame = 84;
					} else {
						pid_reset_frame = 85;
					}
					break;

				case PID_SLIDE:
				case PID_SLIDE_JUMP:
					player.commands_allowed = false;
					if (local->on_shore == TOP_RIGHT) {
						pid_reset_frame = 28;
						local->on_shore = BOTTOM_RIGHT;
						if (local->pid_action == PID_SLIDE) {
							local->new_y    = ENTER_509_Y_BOTTOM;
						}
					} else {
						pid_reset_frame = 57;
						local->on_shore = TOP_RIGHT;
						if (local->pid_action == PID_SLIDE) {
							local->new_y    = ENTER_509_Y_BOTTOM - 29;
						}
					}
					break;

				case PID_LEAVE:
					player.commands_allowed = false;
					if (local->on_shore == TOP_RIGHT) {
						pid_reset_frame = 115;
					} else {
						pid_reset_frame = 123;
					}
					break;
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

static void room_510_init() {
	global[grid_position]      = 5;
	global[grid_position + 1]  = 4;
	global[grid_position + 2]  = 9;
	global[grid_position + 3]  = 10;
	global[grid_position + 4]  = 15;
	global[grid_position + 5]  = 20;
	global[grid_position + 6]  = 19;
	global[grid_position + 7]  = 24;
	global[grid_position + 8]  = 23;
	global[grid_position + 9]  = 18;

	global[grid_position + 10] = 13;
	global[grid_position + 11] = 8;
	global[grid_position + 12] = 3;
	global[grid_position + 13] = 2;
	global[grid_position + 14] = 1;
	global[grid_position + 15] = 6;
	global[grid_position + 16] = 7;
	global[grid_position + 17] = 12;

	global[grid_position + 18] = 17;
	global[grid_position + 19] = 22;
	global[grid_position + 20] = 21;
	global[grid_position + 21] = 16;
	global[grid_position + 22] = 11;

	if (player_has(magic_belt)) {
		ss[fx_belt]  = kernel_load_series(kernel_name('p', 0), false);
		seq[fx_belt] = kernel_seq_stamp(ss[fx_belt], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_belt], 1);
	}

	if (kernel.teleported_in) {
		global[player_persona] = PLAYER_IS_PID;
	}

	local->first_jump = false;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->standing_on    = 0;
		if (global[move_direction_510]) {
			local->move_counter   = 0;
		} else {
			local->move_counter   = 0;
		}
	}

	ss[fx_n]     = kernel_load_series(kernel_name('b', 0), false);
	ss[fx_s]     = kernel_load_series(kernel_name('b', 3), false);
	ss[fx_ew]    = kernel_load_series(kernel_name('b', 1), false);

	player.walker_visible   = false;

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
			aa[0] = kernel_run_animation(kernel_name('p', 1), 0);
			if (local->on_shore == TOP_RIGHT) {
				kernel_reset_animation(aa[0], 85);
			} else {
				kernel_reset_animation(aa[0], 86);
			}
		}

	} else if (previous_room == 512 || (previous_room == 505 && !global[move_direction_510])) {
		local->on_shore   = false;
		local->facing     = FACING_EAST;
		local->old_facing = FACING_EAST;
		local->old_x      = ENTER_512_X_BOTTOM;
		local->first_jump = true;

		if (player.x == 1) {
			local->old_y       = ENTER_512_Y_BOTTOM - 29;
			local->new_x       = ENTER_512_X_BOTTOM + 35;
			local->new_y       = ENTER_512_Y_BOTTOM - 29;
			local->standing_on = 11;

		} else {
			local->old_y       = ENTER_512_Y_BOTTOM;
			local->new_x       = ENTER_512_X_BOTTOM + 35;
			local->new_y       = ENTER_512_Y_BOTTOM;
			local->standing_on = 16;
		}

		kernel_timing_trigger(1, ROOM_510_JUMP);
		player.commands_allowed = false;

	} else if (previous_room == 509 || previous_room != KERNEL_RESTORING_GAME) {
		aa[0] = kernel_run_animation(kernel_name('p', 1), 0);
		player.commands_allowed = false;
		local->anim_0_running   = true;
		local->pid_action       = PID_FREEZE;
		local->on_shore         = BOTTOM_RIGHT;
		local->facing           = FACING_WEST;
		local->new_x            = ENTER_509_X_BOTTOM;
		local->new_y            = ENTER_509_Y_BOTTOM;
	}

	section_5_music();
}

static void room_510_daemon() {
	int temp;

	if (local->anim_0_running) {
		handle_animation_pid();
	}

	switch (kernel.trigger) {
	case ROOM_510_JUMP:
		seq[fx_ew] = kernel_seq_forward(ss[fx_ew], true, 5, 0, 0, 1);
		kernel_seq_depth(seq[fx_ew], 1);
		kernel_seq_range(seq[fx_ew], 16, 34);
		kernel_seq_trigger(seq[fx_ew], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_END_OF_JUMP);
		kernel_seq_trigger(seq[fx_ew], KERNEL_TRIGGER_SPRITE, 30, ROOM_510_SOUND);
		kernel_seq_loc(seq[fx_ew], local->old_x + 2, local->new_y + 19);
		break;

	case ROOM_510_END_OF_JUMP:
		temp       = seq[fx_ew];
		seq[fx_ew] = kernel_seq_stamp(ss[fx_ew], true, 13);
		kernel_seq_loc(seq[fx_ew], local->new_x + 2, local->new_y + 19);
		kernel_seq_depth(seq[fx_ew], 1);
		kernel_synch(KERNEL_SERIES, seq[fx_ew], KERNEL_SERIES, temp);

		if (global[global[max_grid_value]] == local->standing_on) {
			++local->move_counter;
		} else {
			new_room = 511;
			if (local->first_jump) {
				global[pid_just_died] = true;
			}
		}
		local->first_jump       = false;
		player.commands_allowed = true;
		break;
	}

	if (kernel.trigger == ROOM_510_SOUND) {
		sound_play(N_JumpThwang);
	}
}

static void which_pillar(int *it, int *new_x, int *new_y, int *adjoining) {
	int offset;

	if (inter_point_x < 150) {
		offset = 0;

	} else if (inter_point_x < 190) {
		offset = 1;

	} else if (inter_point_x < 220) {
		offset = 2;

	} else if (inter_point_x < 260) {
		offset = 3;

	} else {
		offset = 4;
	}

	if (inter_point_y < 35) {
		*it    = 1 + offset;
		*new_y = BASE_Y;

	} else if (inter_point_y < 65) {
		*it    = 6 + offset;
		*new_y = BASE_Y + 29;

	} else if (inter_point_y < 90) {
		*it    = 11 + offset;
		*new_y = BASE_Y + 58;

	} else if (inter_point_y < 120) {
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

static void room_510_pre_parser() {
	player.need_to_walk = false;
}

static void room_510_parser() {
	int temp;
	int selected;
	int new_x;
	int new_y;
	int adjoining;

	if (kernel.trigger == ROOM_510_SOUND) {
		sound_play(N_JumpThwang);
		player.command_ready = false;
		return;
	}

	switch (kernel.trigger) {
	case ROOM_510_JUMP:
		if (local->on_shore) {
			kernel_abort_animation(aa[0]);
			local->anim_0_running = false;
			player.commands_allowed = false;

		} else switch (local->facing) {
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

		switch (local->facing) {
		case FACING_NORTH:
			seq[fx_n] = kernel_seq_forward(ss[fx_n], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_n], 1);
			kernel_seq_range(seq[fx_n], 15, 34);
			kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_END_OF_JUMP);
			kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_SPRITE, 30, ROOM_510_SOUND);
			kernel_seq_loc(seq[fx_n], local->new_x, local->new_y + 40);
			break;

		case FACING_SOUTH:
			seq[fx_s] = kernel_seq_forward(ss[fx_s], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_s], 1);
			kernel_seq_range(seq[fx_s], 15, 33);
			kernel_seq_trigger(seq[fx_s], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_END_OF_JUMP);
			kernel_seq_trigger(seq[fx_s], KERNEL_TRIGGER_SPRITE, 29, ROOM_510_SOUND);
			kernel_seq_loc(seq[fx_s], local->new_x, local->new_y + 7);
			break;

		case FACING_EAST:
			seq[fx_ew] = kernel_seq_forward(ss[fx_ew], true, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_ew], 1);
			kernel_seq_range(seq[fx_ew], 16, 34);
			kernel_seq_trigger(seq[fx_ew], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_END_OF_JUMP);
			kernel_seq_trigger(seq[fx_ew], KERNEL_TRIGGER_SPRITE, 30, ROOM_510_SOUND);
			if (!local->on_shore) {
				kernel_seq_loc(seq[fx_ew], local->old_x + 2, local->new_y + 19);
			}
			break;

		case FACING_WEST:
			seq[fx_ew] = kernel_seq_forward(ss[fx_ew], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_ew], 1);
			kernel_seq_range(seq[fx_ew], 16, 34);
			kernel_seq_trigger(seq[fx_ew], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_END_OF_JUMP);
			kernel_seq_trigger(seq[fx_ew], KERNEL_TRIGGER_SPRITE, 30, ROOM_510_SOUND);
			if (local->on_shore == TOP_RIGHT) {
				kernel_seq_loc(seq[fx_ew], ENTER_509_X_BOTTOM, ENTER_509_Y_BOTTOM - 11);
				kernel_synch(KERNEL_SERIES, seq[fx_ew], KERNEL_NOW, 0);
				local->on_shore = NEITHER;
			} else if (local->on_shore == BOTTOM_RIGHT) {
				kernel_seq_loc(seq[fx_ew], ENTER_509_X_BOTTOM + 2, ENTER_509_Y_BOTTOM + 17);
				kernel_synch(KERNEL_SERIES, seq[fx_ew], KERNEL_NOW, 0);
				local->on_shore = NEITHER;
			} else if (local->on_shore == NEITHER) {
				kernel_seq_loc(seq[fx_ew], local->old_x + 3, local->new_y + 19);
			}
			break;
		}

		player.command_ready = false;
		return;

	case ROOM_510_END_OF_JUMP:
		if (local->standing_on < 500) {
			if (global[move_direction_510]) {
				if (global[grid_position + local->move_counter] != local->standing_on) {
					new_room = 511;
				}
			} else {
				if (global[global[max_grid_value] - local->move_counter] != local->standing_on) {
					new_room = 511;
				}
			}
		}

		++local->move_counter;

		switch (local->facing) {
		case FACING_NORTH:
			temp      = seq[fx_n];
			seq[fx_n] = kernel_seq_stamp(ss[fx_n], false, 13);
			kernel_seq_loc(seq[fx_n], local->new_x - 1, local->new_y + 11);
			kernel_seq_depth(seq[fx_n], 1);
			kernel_synch(KERNEL_SERIES, seq[fx_n], KERNEL_SERIES, temp);
			local->facing           = FACING_NORTH;
			local->on_shore         = NEITHER;
			if (imath_random(1, 5) == 1) {
				kernel_timing_trigger(1, ROOM_510_WHOA_NORTH);
			} else {
				player.commands_allowed = true;
			}
			break;

		case FACING_SOUTH:
			temp      = seq[fx_s];
			seq[fx_s] = kernel_seq_stamp(ss[fx_s], true, 12);
			kernel_seq_loc(seq[fx_s], local->new_x + 3, local->new_y + 36);
			kernel_seq_depth(seq[fx_s], 1);
			kernel_synch(KERNEL_SERIES, seq[fx_s], KERNEL_SERIES, temp);
			local->facing           = FACING_SOUTH;
			local->on_shore         = NEITHER;
			if (imath_random(1, 5) == 1) {
				kernel_timing_trigger(1, ROOM_510_WHOA_SOUTH);
			} else {
				player.commands_allowed = true;
			}
			break;

		case FACING_EAST:
			temp       = seq[fx_ew];
			if (local->standing_on == 509) {
				aa[0] = kernel_run_animation(kernel_name('p', 1), 0);
				kernel_synch(KERNEL_ANIM, aa[0], KERNEL_SERIES, temp);
				if (local->new_y == ENTER_509_Y_BOTTOM) {
					kernel_reset_animation(aa[0], 87);
				} else {
					kernel_reset_animation(aa[0], 102);
				}
				local->anim_0_running = true;

			} else {
				seq[fx_ew] = kernel_seq_stamp(ss[fx_ew], true, 13);
				kernel_seq_loc(seq[fx_ew], local->new_x + 2, local->new_y + 19);
				kernel_seq_depth(seq[fx_ew], 1);
				kernel_synch(KERNEL_SERIES, seq[fx_ew], KERNEL_SERIES, temp);
				player.commands_allowed = true;
				local->facing           = FACING_EAST;
				local->on_shore         = NEITHER;
			}
			break;

		case FACING_WEST:
			temp       = seq[fx_ew];
			seq[fx_ew] = kernel_seq_stamp(ss[fx_ew], false, 13);
			kernel_seq_loc(seq[fx_ew], local->new_x + 2, local->new_y + 19);
			kernel_seq_depth(seq[fx_ew], 1);
			kernel_synch(KERNEL_SERIES, seq[fx_ew], KERNEL_SERIES, temp);
			player.commands_allowed = true;
			local->facing           = FACING_WEST;
			local->on_shore         = NEITHER;
			if (local->standing_on == 512) {
				if (!(global[player_score_flags] & SCORE_CROSS_PILLARS)) {
					global[player_score_flags] = global[player_score_flags] | SCORE_CROSS_PILLARS;
					global[player_score] += 3;
				}
				new_room = 512;
			}
			break;
		}
		player.command_ready = false;
		return;

	case ROOM_510_TURN:
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
			kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_END_OF_TURN);
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
			kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_END_OF_TURN);
			break;

		case FACING_WEST:
			kernel_seq_delete(seq[fx_ew]);
			switch (local->facing) {
			case FACING_EAST:
				seq[fx_n] = kernel_seq_backward(ss[fx_n], false, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 36, 39);
				kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_TURN + 2);
				kernel_seq_loc(seq[fx_n], local->old_x + 2, local->old_y + 11);
				break;

			case FACING_NORTH:
				seq[fx_n] = kernel_seq_backward(ss[fx_n], false, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 36, 39);
				kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_END_OF_TURN);
				kernel_seq_loc(seq[fx_n], local->old_x + 1, local->old_y + 11);
				break;

			case FACING_SOUTH:
				seq[fx_n] = kernel_seq_forward(ss[fx_n], false, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 40, 43);
				kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_END_OF_TURN);
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
				kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_END_OF_TURN);
				kernel_seq_loc(seq[fx_n], local->old_x + 2, local->old_y + 11);
				break;

			case FACING_WEST:
				seq[fx_n] = kernel_seq_backward(ss[fx_n], true, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 36, 39);
				kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_TURN + 1);
				kernel_seq_loc(seq[fx_n], local->old_x + 2, local->old_y + 11);
				break;

			case FACING_SOUTH:
				seq[fx_n] = kernel_seq_forward(ss[fx_n], true, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_n], 40, 43);
				kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_END_OF_TURN);
				kernel_seq_loc(seq[fx_n], local->old_x + 2, local->old_y + 11);
				break;
			}
			kernel_seq_depth(seq[fx_n], 1);
			break;
		}
		player.command_ready = false;
		return;

	case ROOM_510_TURN + 1:
		seq[fx_n] = kernel_seq_forward(ss[fx_n], false, 7, 0, 0, 1);
		kernel_seq_range(seq[fx_n], 36, 39);
		kernel_seq_depth(seq[fx_n], 1);
		kernel_seq_loc(seq[fx_n], local->old_x + 2, local->old_y + 11);
		kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_END_OF_TURN);
		player.command_ready = false;
		return;

	case ROOM_510_TURN + 2:
		seq[fx_n] = kernel_seq_forward(ss[fx_n], true, 7, 0, 0, 1);
		kernel_seq_range(seq[fx_n], 36, 39);
		kernel_seq_depth(seq[fx_n], 1);
		kernel_seq_loc(seq[fx_n], local->old_x + 2, local->old_y + 11);
		kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_END_OF_TURN);
		player.command_ready = false;
		return;

	case ROOM_510_END_OF_TURN:
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
		kernel_timing_trigger(4, ROOM_510_JUMP);
		player.command_ready = false;
		return;

	case ROOM_510_WHOA_SOUTH:
		sound_play(N_CryOfDismay);
		kernel_seq_delete(seq[fx_s]);
		seq[fx_s] = kernel_seq_forward(ss[fx_s], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_s], 1);
		kernel_seq_range(seq[fx_s], 35, 54);
		kernel_seq_loc(seq[fx_s], local->new_x + 1, local->new_y + 37);
		kernel_seq_trigger(seq[fx_s], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_WHOA_SOUTH + 1);
		player.command_ready = false;
		return;

	case ROOM_510_WHOA_SOUTH + 1:
		temp      = seq[fx_s];
		seq[fx_s] = kernel_seq_stamp(ss[fx_s], true, 12);
		kernel_seq_loc(seq[fx_s], local->new_x + 3, local->new_y + 36);
		kernel_seq_depth(seq[fx_s], 1);
		kernel_synch(KERNEL_SERIES, seq[fx_s], KERNEL_SERIES, temp);
		player.commands_allowed = true;
		player.command_ready = false;
		return;

	case ROOM_510_WHOA_NORTH:
		sound_play(N_CryOfDismay);
		kernel_seq_delete(seq[fx_n]);
		seq[fx_n] = kernel_seq_forward(ss[fx_n], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_n], 1);
		kernel_seq_range(seq[fx_n], 44, 63);
		kernel_seq_loc(seq[fx_n], local->new_x - 1, local->new_y + 11);
		kernel_seq_trigger(seq[fx_n], KERNEL_TRIGGER_EXPIRE, 0, ROOM_510_WHOA_NORTH + 1);
		player.command_ready = false;
		return;

	case ROOM_510_WHOA_NORTH + 1:
		temp      = seq[fx_n];
		seq[fx_n] = kernel_seq_stamp(ss[fx_n], false, 13);
		kernel_seq_loc(seq[fx_n], local->new_x - 1, local->new_y + 11);
		kernel_seq_depth(seq[fx_n], 1);
		kernel_synch(KERNEL_SERIES, seq[fx_n], KERNEL_SERIES, temp);
		player.commands_allowed = true;
		player.command_ready = false;
		return;
	}

	if (kernel.trigger == ROOM_510_WAIT_FOR_END_SLIDE) {
		if (local->pid_action != PID_FREEZE) {
			kernel_timing_trigger(1, ROOM_510_WAIT_FOR_END_SLIDE);
		} else {
			kernel_timing_trigger(1, ROOM_510_JUMP);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(jump_to, ledge) ||
	    player_said_2(walk_down, path_to_east) ||
	    player_said_2(walk_across, ledge)) {

		if (!local->standing_on) {
			if (inter_point_x > 100) {
				if (player_said_1(walk_down)) {
					local->pid_action = PID_LEAVE;
				} else if (local->on_shore) {
					local->pid_action = PID_SLIDE;
				}
				player.command_ready = false;
				return;

			} else {
				text_show(51007);
				player.command_ready = false;
				return;
			}

		} else if (local->standing_on == 11) {
			if (inter_point_x > 100) {
				text_show(51008);
				player.command_ready = false;
				return;

			} else {
				local->old_x = local->new_x;
				local->old_y = local->new_y;
				local->new_x = ENTER_512_X_BOTTOM - 3;
				local->new_y = ENTER_512_Y_BOTTOM - 29;
			}

		} else if (local->standing_on == 16) {
			if (inter_point_x > 100) {
				text_show(51008);
				player.command_ready = false;
				return;

			} else {
				local->old_x = local->new_x;
				local->old_y = local->new_y;
				local->new_x = ENTER_512_X_BOTTOM - 2;
				local->new_y = ENTER_512_Y_BOTTOM;
			}

		} else if (local->standing_on == 5) {
			if (inter_point_x < 100) {
				text_show(51008);
				player.command_ready = false;
				return;

			} else {
				local->old_x = local->new_x;
				local->old_y = local->new_y;
				local->new_x = ENTER_509_X_BOTTOM - 3;
				local->new_y = ENTER_509_Y_BOTTOM - 29;
			}

		} else if (local->standing_on == 10) {
			if (inter_point_x < 100) {
				text_show(51008);
				player.command_ready = false;
				return;

			} else {
				local->old_x = local->new_x;
				local->old_y = local->new_y;
				local->new_x = ENTER_509_X_BOTTOM - 3;
				local->new_y = ENTER_509_Y_BOTTOM;
			}

		} else if (inter_point_x < 100) {
			text_show(51007);
			player.command_ready = false;
			return;

		} else {
			text_show(51008);
			player.command_ready = false;
			return;
		}

		local->old_facing = local->facing;

		if (local->standing_on == 5 || local->standing_on == 10) {
			local->facing      = FACING_EAST;
			local->standing_on = 509;

		} else if (local->standing_on == 11 || local->standing_on == 16) {
			local->facing      = FACING_WEST;
			local->standing_on = 512;
		}

		player.commands_allowed = false;

		if (local->old_facing == local->facing) {
			kernel_timing_trigger(1, ROOM_510_JUMP);
		} else {
			kernel_timing_trigger(1, ROOM_510_TURN);
		}

		player.command_ready = false;
		return;
	}

	if (player_said_2(jump_to, pillar)) {
		local->old_x = local->new_x;
		local->old_y = local->new_y;

		adjoining = local->standing_on;
		which_pillar(&selected, &new_x, &new_y, &adjoining);

		if (local->on_shore == TOP_RIGHT) {
			if (selected == 10) {
				local->pid_action = PID_SLIDE_JUMP;
				kernel_timing_trigger(1, ROOM_510_WAIT_FOR_END_SLIDE);
				local->standing_on      = selected;
				player.commands_allowed = false;
				local->new_x = new_x;
				local->new_y = new_y;
				player.command_ready = false;
				return;

			} else if (selected != 5) {
				text_show(51009);
				player.command_ready = false;
				return;
			}

		} else if (local->on_shore == BOTTOM_RIGHT) {
			if (selected == 5) {
				local->pid_action = PID_SLIDE_JUMP;
				kernel_timing_trigger(1, ROOM_510_WAIT_FOR_END_SLIDE);
				local->standing_on      = selected;
				player.commands_allowed = false;
				local->new_x = new_x;
				local->new_y = new_y;
				player.command_ready = false;
				return;

			} else if (selected != 10) {
				text_show(51009);
				player.command_ready = false;
				return;
			}
		}

		if (selected == local->standing_on) {
			text_show(51010);
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
				kernel_timing_trigger(1, ROOM_510_JUMP);
			} else {
				kernel_timing_trigger(1, ROOM_510_TURN);
			}

		} else {
			text_show(51009);
			player.command_ready = false;
			return;
		}

		player.command_ready = false;
		return;
	}

	if (player.look_around) {
		text_show(51001);
		player.command_ready = false;
		return;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(pillar)) {
			text_show(51002);
			player.command_ready = false;
			return;
		}

		if (player_said_1(ledge)) {
			text_show(51003);
			player.command_ready = false;
			return;
		}

		if (player_said_1(path_to_east)) {
			text_show(51004);
			player.command_ready = false;
			return;
		}

		if (player_said_1(nest)) {
			text_show(51005);
			player.command_ready = false;
			return;
		}

		if (player_said_1(abyss)) {
			text_show(51006);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(look_into, abyss)) {
		text_show(51006);
		player.command_ready = false;
		return;
	}
}

void room_510_synchronize(Common::Serializer &s) {
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
	s.syncAsSint16LE(scratch.move_counter);
	s.syncAsSint16LE(scratch.first_jump);
}

void room_510_preload() {
	room_init_code_pointer       = room_510_init;
	room_pre_parser_code_pointer = room_510_pre_parser;
	room_parser_code_pointer     = room_510_parser;
	room_daemon_code_pointer     = room_510_daemon;

	section_5_walker();
	section_5_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
