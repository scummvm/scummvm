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
#include "mads/madsv2/dragonsphere/rooms/section4.h"
#include "mads/madsv2/dragonsphere/rooms/room405.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[7];
	int16 guard_frame;
	int16 guard_action;
	int16 guard_talk_count;
	int16 anim_0_running;
	int16 king_tent_frame;
	int16 king_tent_action;
	int16 king_tent_talk_count;
	int16 anim_1_running;
	int16 pid_tent_frame;
	int16 pid_tent_action;
	int16 pid_tent_talk_count;
	int16 anim_2_running;
	int16 shaman_2_frame;
	int16 shaman_2_action;
	int16 shaman_2_talk_count;
	int16 anim_3_running;
	int16 shaman_1_frame;
	int16 shaman_1_action;
	int16 shaman_1_talk_count;
	int16 anim_4_running;
	int16 king_sit_frame;
	int16 king_sit_action;
	int16 king_sit_talk_count;
	int16 anim_5_running;
	int16 pid_sit_frame;
	int16 pid_sit_action;
	int16 pid_sit_talk_count;
	int16 anim_6_running;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_test                 0
#define fx_test_2               1

#define ROOM_405_DOOR_CLOSES    60
#define ROOM_405_ME_TALK        65
#define ROOM_405_YOU_TALK       67
#define ROOM_405_GESTURE        75

#define FROM_EAST_X_1           330
#define FROM_EAST_Y_1           143
#define FROM_EAST_X_2           302
#define FROM_EAST_Y_2           143

#define CONV_40_KING_GUARD      40
#define CONV_44_PID_GUARD       44
#define CONV_39_KING_SHAMAN     39
#define CONV_43_PID_SHAMAN      43

#define SHUT_UP                 0
#define OPEN_TENT               1
#define ARM_OUT                 2
#define TALK                    3
#define ENTER_TENT              4
#define LEAVE                   5
#define GESTURE_TO_SIT          6
#define GIVE_WARNING            7
#define TAKE                    8
#define SIT                     9
#define FLOAT                   10
#define RISE                    11

#define GUARD_X                 8
#define GUARD_Y                 119
#define PLAYER_X_FROM_406       17
#define PLAYER_Y_FROM_406       124
#define SHAMAN_X                96
#define SHAMAN_Y                137

static Scratch scratch;


static void handle_animation_guard() {
	int guard_reset_frame;

	if (kernel_anim[aa[0]].frame != local->guard_frame) {
		local->guard_frame = kernel_anim[aa[0]].frame;
		guard_reset_frame = -1;

		switch (local->guard_frame) {

		case 31:
			local->king_tent_action = ENTER_TENT;
			local->pid_tent_action  = ENTER_TENT;
			if (local->guard_action == OPEN_TENT) {
				guard_reset_frame = 30;
			}
			break;

		case 38:
			new_room = 406;
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 18:
		case 39:
		case 40:
		case 41:
		case 42:
			switch (local->guard_action) {
			case TALK:
				guard_reset_frame = imath_random(38, 41);
				++ local->guard_talk_count;
				if (local->guard_talk_count > 17) {
					local->guard_action     = SHUT_UP;
					local->guard_talk_count = 0;
					guard_reset_frame       = 0;
				}
				break;

			case SHUT_UP:
				if (local->guard_frame >= 6) {
					local->guard_frame = 1;
				}

				++ local->guard_talk_count;
				if (local->guard_talk_count > imath_random(20, 30)) {
					if (local->guard_frame == 1) {
						guard_reset_frame = imath_random(0, 1);
					} else if (local->guard_frame == 2) {
						guard_reset_frame = imath_random(0, 2);
					} else if (local->guard_frame == 3) {
						guard_reset_frame = imath_random(1, 3);
					} else if (local->guard_frame == 4) {
						guard_reset_frame = imath_random(2, 4);
					} else if (local->guard_frame == 5) {
						guard_reset_frame = imath_random(3, 4);
					}
					local->guard_talk_count = 0;
				} else {
					guard_reset_frame = local->guard_frame - 1;
				}
				break;

			case OPEN_TENT:
				guard_reset_frame = 18;
				break;

			case ARM_OUT:
				guard_reset_frame   = 7;
				local->guard_action = SHUT_UP;
				break;
			}
		}

		if (guard_reset_frame >= 0) {
			kernel_reset_animation(aa[0], guard_reset_frame);
			local->guard_frame = guard_reset_frame;
		}
	}
}

static void handle_animation_king_tent() {
	int king_tent_reset_frame;

	if (kernel_anim[aa[1]].frame != local->king_tent_frame) {
		local->king_tent_frame = kernel_anim[aa[1]].frame;
		king_tent_reset_frame = -1;

		switch (local->king_tent_frame) {
		case 33:
			local->guard_action = SHUT_UP;
			break;

		case 1:
		case 13:
			switch (local->king_tent_action) {
			case SHUT_UP:
				king_tent_reset_frame = 0;
				break;

			case TALK:
				king_tent_reset_frame   = 1;
				local->king_tent_action = SHUT_UP;
				break;

			case ENTER_TENT:
				king_tent_reset_frame = 19;
				break;

			case LEAVE:
				local->anim_1_running   = false;
				king_tent_reset_frame   = -1;
				player.walker_visible   = true;
				player.commands_allowed = true;
				king_tent_reset_frame   = 19;
				kernel_abort_animation(aa[1]);
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				break;
			}
			break;
		}

		if (king_tent_reset_frame >= 0) {
			kernel_reset_animation(aa[1], king_tent_reset_frame);
			local->king_tent_frame = king_tent_reset_frame;
		}
	}
}

static void handle_animation_pid_tent() {
	int pid_tent_reset_frame;

	if (kernel_anim[aa[2]].frame != local->pid_tent_frame) {
		local->pid_tent_frame = kernel_anim[aa[2]].frame;
		pid_tent_reset_frame = -1;

		switch (local->pid_tent_frame) {

		case 14:
			local->guard_action = SHUT_UP;
			break;

		case 1:
			switch (local->pid_tent_action) {
			case SHUT_UP:
				pid_tent_reset_frame = 0;
				break;

			case ENTER_TENT:
				pid_tent_reset_frame = 1;
				break;

			case LEAVE:
				local->anim_2_running   = false;
				pid_tent_reset_frame    = -1;
				player.walker_visible   = true;
				player.commands_allowed = true;
				pid_tent_reset_frame    = 19;
				kernel_abort_animation(aa[2]);
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				break;
			}
			break;
		}

		if (pid_tent_reset_frame >= 0) {
			kernel_reset_animation(aa[2], pid_tent_reset_frame);
			local->pid_tent_frame = pid_tent_reset_frame;
		}
	}
}

static void handle_animation_shaman_2() {
	int shaman_2_reset_frame;

	if (kernel_anim[aa[3]].frame != local->shaman_2_frame) {
		local->shaman_2_frame = kernel_anim[aa[3]].frame;
		shaman_2_reset_frame = -1;

		switch (local->shaman_2_frame) {
		case 53:
			new_room = 408;
			break;

		case 85:
			local->pid_sit_frame    = SHUT_UP;
			shaman_2_reset_frame    = 2;
			kernel_reset_animation(aa[6], 16);
			kernel_synch(KERNEL_ANIM, aa[6], KERNEL_ANIM, aa[3]);

			conv_run(CONV_43_PID_SHAMAN);

			if (player_has(new_bundle) && !player_has(medicine_bundle)) {
				conv_export_value(1);
			} else if (!player_has(new_bundle) && player_has(medicine_bundle)) {
				conv_export_value(1);
			} else {
				conv_export_value(0);
			}

			if (player_has(new_bundle) && player_has(medicine_bundle)) {
				conv_export_value(1);
			} else {
				conv_export_value(0);
			}

			conv_export_value(player_has(soul_egg));
			conv_export_value(game.difficulty);
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 11:
			switch (local->shaman_2_action) {
			case SHUT_UP:
				if (local->shaman_2_frame >= 5) {
					local->shaman_2_frame = 1;
				}

				++ local->shaman_2_talk_count;
				if (local->shaman_2_talk_count > imath_random(15, 20)) {
					if (local->shaman_2_frame == 1) {
						shaman_2_reset_frame = imath_random(0, 1);
					} else if (local->shaman_2_frame == 2) {
						shaman_2_reset_frame = imath_random(0, 2);
					} else if (local->shaman_2_frame == 3) {
						shaman_2_reset_frame = imath_random(1, 3);
					} else if (local->shaman_2_frame == 4) {
						shaman_2_reset_frame = imath_random(2, 3);
					}
					local->shaman_2_talk_count = 0;
				} else {
					shaman_2_reset_frame = local->shaman_2_frame - 1;
				}
				break;

			case GESTURE_TO_SIT:
				shaman_2_reset_frame   = 10;
				local->shaman_2_action = SHUT_UP;
				break;

			case RISE:
				shaman_2_reset_frame = 11;
				kernel_abort_animation(aa[6]);
				local->anim_6_running = false;
				kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
				break;
			}
			break;
		}

		if (shaman_2_reset_frame >= 0) {
			kernel_reset_animation(aa[3], shaman_2_reset_frame);
			local->shaman_2_frame = shaman_2_reset_frame;
		}
	}
}

static void handle_animation_shaman_1() {
	int shaman_1_reset_frame;

	if (kernel_anim[aa[4]].frame != local->shaman_1_frame) {
		local->shaman_1_frame = kernel_anim[aa[4]].frame;
		shaman_1_reset_frame = -1;

		switch (local->shaman_1_frame) {
		case 77:
			shaman_1_reset_frame = 76;
			break;

		case 42:
			local->king_sit_action = TAKE;
			break;

		case 43:
			if (global[player_persona] == PLAYER_IS_PID) {
				if (player_has(medicine_bundle)) {
					inter_move_object(medicine_bundle, NOWHERE);
					global[player_score] += 3;
				} else {
					inter_move_object(new_bundle, NOWHERE);
					global[player_score] += 3;
				}
				local->pid_sit_action = SHUT_UP;
			} else if (local->shaman_1_action == TAKE) {
				shaman_1_reset_frame = 42;
			}
			break;

		case 46:
			if (global[player_persona] == PLAYER_IS_PID) {
				local->shaman_1_action = RISE;
				local->shaman_2_action = RISE;
			}
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 17:
		case 28:
		case 49:
		case 47:
		case 104:
			switch (local->shaman_1_action) {
			case SHUT_UP:
				if (local->shaman_1_frame >= 8) {
					local->shaman_1_frame = 1;
				}

				++ local->shaman_1_talk_count;
				if (local->shaman_1_talk_count > imath_random(12, 18)) {
					if (local->shaman_1_frame == 1) {
						shaman_1_reset_frame = imath_random(0, 1);
					} else if (local->shaman_1_frame == 2) {
						shaman_1_reset_frame = imath_random(0, 2);
					} else if (local->shaman_1_frame == 3) {
						shaman_1_reset_frame = imath_random(1, 3);
					} else if (local->shaman_1_frame == 4) {
						shaman_1_reset_frame = imath_random(2, 4);
					} else if (local->shaman_1_frame == 5) {
						shaman_1_reset_frame = imath_random(3, 5);
					} else if (local->shaman_1_frame == 6) {
						shaman_1_reset_frame = imath_random(5, 6);
					} else if (local->shaman_1_frame == 7) {
						shaman_1_reset_frame = imath_random(5, 6);
					}
					local->shaman_1_talk_count = 0;
				} else {
					shaman_1_reset_frame = local->shaman_1_frame - 1;
				}
				break;

			case TALK:
				shaman_1_reset_frame = 28;
				break;

			case GESTURE_TO_SIT:
				shaman_1_reset_frame   = 7;
				local->shaman_1_action = SHUT_UP;
				break;

			case GIVE_WARNING:
				shaman_1_reset_frame   = 17;
				local->shaman_1_action = SHUT_UP;
				break;

			case RISE:
				shaman_1_reset_frame = 49;
				break;

			case TAKE:
				shaman_1_reset_frame = 40;
				break;
			}
			break;

		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
			switch (local->shaman_1_action) {
			case TALK:
				shaman_1_reset_frame = imath_random(29, 34);
				++ local->shaman_1_talk_count;
				if (local->shaman_1_talk_count > 18) {
					local->shaman_1_action     = SHUT_UP;
					local->shaman_1_talk_count = 0;
					shaman_1_reset_frame       = 47;
				}
				break;

			default:
				shaman_1_reset_frame = 47;
				break;
			}
			break;
		}

		if (shaman_1_reset_frame >= 0) {
			kernel_reset_animation(aa[4], shaman_1_reset_frame);
			local->shaman_1_frame = shaman_1_reset_frame;
		}
	}
}

static void handle_animation_king_sit() {
	int king_sit_reset_frame;

	if (kernel_anim[aa[5]].frame != local->king_sit_frame) {
		local->king_sit_frame = kernel_anim[aa[5]].frame;
		king_sit_reset_frame = -1;

		switch (local->king_sit_frame) {
		case 23:
			++ global[player_score];
			++ global[dragon_high_scene];
			sound_play(N_TakeObjectSnd);
			inter_give_to_player(yellow_powerstone);
			object_examine(yellow_powerstone, 40528, 0);
			local->shaman_1_action = SHUT_UP;
			local->king_sit_action = SHUT_UP;
			break;

		case 48:
			if (local->king_sit_action == TALK) {
				king_sit_reset_frame = 47;
			} else {
				king_sit_reset_frame = 0;
			}
			break;

		case 14:
		case 26:
			conv_release();
			break;

		case 15:
		case 33:
		case 27:
			switch (local->king_sit_action) {

			case SHUT_UP:
				king_sit_reset_frame = 14;
				break;

			case TALK:
				king_sit_reset_frame = 27;
				break;

			case TAKE:
				king_sit_reset_frame = 19;
				break;

			case LEAVE:
				king_sit_reset_frame   = 33;
				local->king_sit_action = SHUT_UP;
				break;
			}
			break;

		case 28:
		case 29:
		case 30:
			switch (local->king_sit_action) {
			case TALK:
				king_sit_reset_frame = imath_random(28, 29);
				++ local->king_sit_talk_count;
				if (local->king_sit_talk_count > 8) {
					local->king_sit_action     = SHUT_UP;
					local->king_sit_talk_count = 0;
					king_sit_reset_frame       = 30;
				}
				break;

			default:
				king_sit_reset_frame = 30;
				break;
			}
			break;

		case 47:
			player.walker_visible   = true;
			player.commands_allowed = true;
			local->anim_5_running   = false;
			kernel_abort_animation(aa[5]);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			break;
		}

		if (king_sit_reset_frame >= 0) {
			kernel_reset_animation(aa[5], king_sit_reset_frame);
			local->king_sit_frame = king_sit_reset_frame;
		}
	}
}

static void handle_animation_pid_sit() {
	int pid_sit_reset_frame;

	if (kernel_anim[aa[6]].frame != local->pid_sit_frame) {
		local->pid_sit_frame = kernel_anim[aa[6]].frame;
		pid_sit_reset_frame = -1;

		switch (local->pid_sit_frame) {
		case 37:
			if (global[player_persona] == PLAYER_IS_PID) {
				local->shaman_1_action = TAKE;
			}
			break;

		case 38:
			if (global[player_persona] == PLAYER_IS_KING) {
				local->shaman_1_action = TAKE;
			} else if (local->pid_sit_action == TAKE) {
				pid_sit_reset_frame = 37;
			}
			break;

		case 62:
			pid_sit_reset_frame = 61;
			break;

		case 61:
			if (local->pid_sit_action == TALK) {
				pid_sit_reset_frame = 60;
			} else {
				pid_sit_reset_frame = 0;
			}
			break;

		case 41:
			conv_release();
			break;

		case 17:
		case 27:
		case 42:
			switch (local->pid_sit_action) {

			case SHUT_UP:
				pid_sit_reset_frame = 16;
				break;

			case TALK:
				pid_sit_reset_frame   = 17;
				local->pid_sit_action = SHUT_UP;
				break;

			case TAKE:
				pid_sit_reset_frame = 27;
				break;

			case LEAVE:
				pid_sit_reset_frame   = 42;
				local->pid_sit_action = SHUT_UP;
				break;
			}
			break;

		case 60:
			player.walker_visible   = true;
			player.commands_allowed = true;
			local->anim_6_running   = false;
			kernel_abort_animation(aa[6]);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			break;
		}

		if (pid_sit_reset_frame >= 0) {
			kernel_reset_animation(aa[6], pid_sit_reset_frame);
			local->pid_sit_frame = pid_sit_reset_frame;
		}
	}
}

void room_405_init() {
	global[perform_displacements] = true;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_2_running = false;
		local->anim_3_running = false;
		local->anim_4_running = false;
		local->anim_5_running = false;
		local->anim_6_running = false;
	}

	if (global[pid_just_died] && previous_room == 406) {
		if (!(global[object_flags] & 1)) {
			inter_move_object(dates, 406);
			--global[player_score];
		}

		if (!(global[object_flags] & 2)) {
			inter_move_object(statue, 406);
			--global[player_score];
		}

		if (!(global[object_flags] & 4)) {
			inter_move_object(ruby_ring, 406);
			global[player_score] += -5;
		}

		if (!(global[object_flags] & 8)) {
			inter_move_object(bottle_of_flies, 406);
			--global[player_score];
		}

		if (!(global[object_flags] & 16)) {
			inter_move_object(soptus_soporific, 406);
			--global[player_score];
		}

		global[object_flags] = 0;
		global[pid_just_died] = false;
		global[prizes_owed_to_player] = 0;
		global[wins_in_desert] = global[save_wins_in_desert];
		conv_reset(CONV_44_PID_GUARD);
	}

	if (global[player_persona] == PLAYER_IS_KING) {
		conv_get(CONV_40_KING_GUARD);
		conv_get(CONV_39_KING_SHAMAN);
	} else {
		conv_get(CONV_44_PID_GUARD);
		conv_get(CONV_43_PID_SHAMAN);
	}

	/* guard */
	aa[0] = kernel_run_animation(kernel_name('u', 1), 0);
	local->anim_0_running = true;
	local->guard_action = SHUT_UP;

	/* shaman on left */
	aa[4] = kernel_run_animation(kernel_name('s', 1), 0);
	local->anim_4_running = true;
	local->shaman_1_action = SHUT_UP;

	/* shaman on right */
	aa[3] = kernel_run_animation(kernel_name('s', 2), 0);
	local->anim_3_running = true;
	local->shaman_2_action = SHUT_UP;

	if (conv_restore_running == CONV_40_KING_GUARD ||
		conv_restore_running == CONV_44_PID_GUARD) {
		player.walker_visible = false;
		player.commands_allowed = false;
		player.x = GUARD_X;
		player.y = GUARD_Y;
		player.facing = FACING_NORTHEAST;

		if (global[player_persona] == PLAYER_IS_KING) {
			aa[1] = kernel_run_animation(kernel_name('k', 1), 0);
			local->anim_1_running = true;
			local->king_tent_action = SHUT_UP;
			conv_run(CONV_40_KING_GUARD);
			conv_export_pointer(&global[game_points]);
			conv_export_pointer(&global[dance_points]);
			conv_export_pointer(&global[clue_points]);
			conv_export_value(global[pid_talk_shamon]);
		} else {
			aa[2] = kernel_run_animation(kernel_name('p', 1), 0);
			local->anim_2_running = true;
			local->pid_tent_action = SHUT_UP;
			conv_run(CONV_44_PID_GUARD);
			conv_export_value(global[pid_talk_shamon]);
		}

	} else if (conv_restore_running == CONV_39_KING_SHAMAN ||
		conv_restore_running == CONV_43_PID_SHAMAN) {
		player.commands_allowed = false;
		player.walker_visible = false;

		if (global[player_persona] == PLAYER_IS_KING) {
			player.x = SHAMAN_X;
			player.y = SHAMAN_Y;
		} else {
			player.x = SHAMAN_X + 2;
			player.y = SHAMAN_Y + 2;
		}

		player.facing = FACING_NORTHWEST;

		if (global[player_persona] == PLAYER_IS_KING) {
			aa[5] = kernel_run_animation(kernel_name('k', 2), 0);
			local->anim_5_running = true;
			local->king_sit_action = SHUT_UP;
			kernel_reset_animation(aa[5], 15);

			conv_run(CONV_39_KING_SHAMAN);
			conv_export_value(global[talked_to_soptus]);
			if (player_has(red_powerstone) || player_has(blue_powerstone)) {
				conv_export_value(1);
			} else {
				conv_export_value(0);
			}
		} else {
			aa[6] = kernel_run_animation(kernel_name('p', 2), 0);
			local->anim_6_running = true;
			local->pid_sit_action = SHUT_UP;
			kernel_reset_animation(aa[6], 17);

			player.commands_allowed = false;
			conv_run(CONV_43_PID_SHAMAN);

			conv_export_value(player_has(medicine_bundle));
			conv_export_value(player_has(new_bundle));
			conv_export_value(player_has(soul_egg));
			conv_export_value(game.difficulty);
		}

	} else if (previous_room == 408) {

		if (global[player_persona] == PLAYER_IS_KING) {
			player.x = SHAMAN_X;
			player.y = SHAMAN_Y;
		} else {
			player.x = SHAMAN_X + 2;
			player.y = SHAMAN_Y + 2;
		}

		player.facing = FACING_NORTHWEST;
		player.commands_allowed = false;
		player.walker_visible = false;

		kernel_reset_animation(aa[4], 78);
		kernel_reset_animation(aa[3], 54);

		aa[6] = kernel_run_animation(kernel_name('p', 2), 0);
		local->anim_6_running = true;
		kernel_reset_animation(aa[6], 62);

	} else if (previous_room == 406) {
		player.x = PLAYER_X_FROM_406;
		player.y = PLAYER_Y_FROM_406;
		player.facing = FACING_SOUTHEAST;

	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player_first_walk(FROM_EAST_X_1, FROM_EAST_Y_1, FACING_WEST,
			FROM_EAST_X_2, FROM_EAST_Y_2, FACING_WEST, true);
	}

	section_4_music();
}

static void room_405_daemon() {
	if (local->anim_0_running) {
		handle_animation_guard();
	}

	if (local->anim_1_running) {
		handle_animation_king_tent();
	}

	if (local->anim_2_running) {
		handle_animation_pid_tent();
	}

	if (local->anim_3_running) {
		handle_animation_shaman_2();
	}

	if (local->anim_4_running) {
		handle_animation_shaman_1();
	}

	if (local->anim_5_running) {
		handle_animation_king_sit();
	}

	if (local->anim_6_running) {
		handle_animation_pid_sit();
	}
}

static void process_conv_king_shaman() {
	int you_trig_flag  = false;
	int me_trig_flag   = false;

	if (player_verb == conv039_greeting_only) {
		if (!kernel.trigger) {
			conv_you_trigger(ROOM_405_GESTURE);
		}
		me_trig_flag  = true;
		you_trig_flag = true;
	}

	if (player_verb == conv039_choices_b_b) {
		local->king_sit_action = SHUT_UP;
		you_trig_flag          = true;
		me_trig_flag           = true;
		conv_hold();
	}

	if (player_verb == conv039_gift_b_b) {
		local->shaman_1_action = TAKE;
		you_trig_flag          = true;
		me_trig_flag           = true;
		conv_hold();
	}

	if (player_verb == conv039_exit_b_b || player_verb == conv039_exit_d_d) {
		local->king_sit_action = LEAVE;
		you_trig_flag          = true;
		me_trig_flag           = true;
	}

	if (player_verb == conv039_pre_addon_b_b) {
		local->king_sit_action = SHUT_UP;
		you_trig_flag          = true;
		me_trig_flag           = true;
		conv_hold();
	}

	if (kernel.trigger == ROOM_405_GESTURE) {
		local->shaman_1_action = GESTURE_TO_SIT;
		local->shaman_2_action = GESTURE_TO_SIT;
	}

	if (kernel.trigger == ROOM_405_YOU_TALK) {
		local->shaman_1_action = TALK;
		if (local->king_sit_action != LEAVE) {
			local->king_sit_action = SHUT_UP;
		}
	}

	if (kernel.trigger == ROOM_405_ME_TALK) {
		local->shaman_1_action = SHUT_UP;
		if (local->king_sit_action != LEAVE) {
			local->king_sit_action = TALK;
		}
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_405_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_405_ME_TALK);
	}

	local->shaman_1_talk_count = 0;
	local->king_sit_talk_count = 0;
}

static void process_conv_pid_shaman() {
	int you_trig_flag  = false;
	int me_trig_flag   = false;

	if (player_verb == conv043_rise_b_b) {
		*conv_my_next_start = conv043_homenow;
		conv_abort();
		local->shaman_1_action = RISE;
		local->shaman_2_action = RISE;
		me_trig_flag           = true;
		you_trig_flag          = true;
	}

	if (player_verb == conv043_rise_d_d) {
		*conv_my_next_start = conv043_homenow;
		conv_abort();
		local->pid_sit_action = TAKE;
		me_trig_flag          = true;
		you_trig_flag         = true;
	}

	if (player_verb == conv043_exit_b_b) {
		*conv_my_next_start = conv043_greeting;
		conv_abort();
		local->pid_sit_action = LEAVE;
		you_trig_flag         = true;
		me_trig_flag          = true;
	}

	if (player_verb == conv043_exit_d_d) {
		*conv_my_next_start = conv043_restart;
		conv_abort();
		local->pid_sit_action = LEAVE;
		you_trig_flag         = true;
		me_trig_flag          = true;
	}

	if (kernel.trigger == ROOM_405_YOU_TALK) {
		if (local->shaman_1_action != RISE) {
			local->shaman_1_action = TALK;
		}

		if (local->pid_sit_action != LEAVE &&
		    local->pid_sit_action != TAKE) {
			local->pid_sit_action = SHUT_UP;
		}
	}

	if (kernel.trigger == ROOM_405_ME_TALK) {
		if (local->shaman_1_action != RISE) {
			local->shaman_1_action = SHUT_UP;
		}

		if (local->pid_sit_action != LEAVE &&
		    local->pid_sit_action != TAKE) {
			local->pid_sit_action = TALK;
		}
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_405_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_405_ME_TALK);
	}

	local->shaman_1_talk_count = 0;
	local->pid_sit_talk_count  = 0;
}

static void process_conv_king_guard() {
	int you_trig_flag  = false;
	int me_trig_flag   = false;

	switch (player_verb) {
	case conv040_leave_b_b:
		*conv_my_next_start     = conv040_repeater;
		local->king_tent_action = LEAVE;
		you_trig_flag           = true;
		me_trig_flag            = true;
		conv_abort();
		break;

	case conv040_enter_b_b:
		*conv_my_next_start = conv040_repeater;
		local->guard_action = OPEN_TENT;
		you_trig_flag       = true;
		me_trig_flag        = true;
		conv_abort();
		break;
	}

	if (kernel.trigger == ROOM_405_YOU_TALK) {
		local->guard_action     = TALK;
		local->king_tent_action = SHUT_UP;
	}

	if (kernel.trigger == ROOM_405_ME_TALK) {
		local->guard_action     = SHUT_UP;
		local->king_tent_action = TALK;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_405_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_405_ME_TALK);
	}

	local->guard_talk_count     = 0;
	local->king_tent_talk_count = 0;
}

static void process_conv_pid_guard() {
	int you_trig_flag  = false;
	int me_trig_flag   = false;

	switch (player_verb) {
	case conv044_leave_b_b:
		*conv_my_next_start = conv044_greet;
		conv_abort();
		local->pid_tent_action = LEAVE;
		you_trig_flag = true;
		me_trig_flag  = true;
		break;

	case conv044_enter_b_b:
		*conv_my_next_start = conv044_repeater;
		conv_abort();
		local->guard_action = OPEN_TENT;
		you_trig_flag = true;
		me_trig_flag  = true;
		break;
	}

	if (kernel.trigger == ROOM_405_YOU_TALK) {
		local->guard_action = TALK;
	}

	if (kernel.trigger == ROOM_405_ME_TALK) {
		local->guard_action = SHUT_UP;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_405_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_405_ME_TALK);
	}

	local->guard_talk_count = 0;
}

static void room_405_pre_parser() {
	if (player_said_1(cross)) {
		global[from_direction]       = FROM_WEST;
		global[desert_room]          = 42;
		player.walk_off_edge_to_room = 401;
	}

	if (player_said_2(talk_to, shaman) ||
	    player_said_3(give, spirit_bundle, shaman) ||
	    player_said_3(give, new_bundle, shaman)) {
		if (global[player_persona] == PLAYER_IS_PID) {
			player_walk(SHAMAN_X + 2, SHAMAN_Y + 2, FACING_NORTHWEST);
		}
	}
}

static void room_405_parser() {
	if (conv_control.running == CONV_40_KING_GUARD) {
		process_conv_king_guard();
		goto handled;
	}

	if (conv_control.running == CONV_44_PID_GUARD) {
		process_conv_pid_guard();
		goto handled;
	}

	if (conv_control.running == CONV_39_KING_SHAMAN) {
		process_conv_king_shaman();
		goto handled;
	}

	if (conv_control.running == CONV_43_PID_SHAMAN) {
		process_conv_pid_shaman();
		goto handled;
	}

	if (player_said_2(talk_to, shaman) ||
	    player_said_3(give, spirit_bundle, shaman) ||
	    player_said_3(give, new_bundle, shaman)) {

		global[pid_talk_shamon] = true;

		if (global[player_persona] == PLAYER_IS_KING) {
			local->king_sit_frame   = -1;
			aa[5]                   = kernel_run_animation(kernel_name('k', 2), 0);
			local->anim_5_running   = true;
			local->king_sit_action  = TALK;
			player.walker_visible   = false;
			player.commands_allowed = false;
			kernel_reset_animation(aa[5], 48);
			kernel_synch(KERNEL_ANIM, aa[5], KERNEL_PLAYER, 0);

			conv_run(CONV_39_KING_SHAMAN);
			conv_export_value(global[talked_to_soptus]);
			if (player_has(red_powerstone) || player_has(blue_powerstone)) {
				conv_export_value(1);
			} else {
				conv_export_value(0);
			}
		} else {
			aa[6]                   = kernel_run_animation(kernel_name('p', 2), 0);
			local->anim_6_running   = true;
			local->pid_sit_action   = SHUT_UP;
			player.walker_visible   = false;
			player.commands_allowed = false;
			kernel_synch(KERNEL_ANIM, aa[6], KERNEL_PLAYER, 0);

			conv_run(CONV_43_PID_SHAMAN);

			conv_export_value(player_has(medicine_bundle));
			conv_export_value(player_has(new_bundle));
			conv_export_value(player_has(soul_egg));
			conv_export_value(game.difficulty);
		}
		goto handled;
	}

	if (player_said_2(walk_into, tent) ||
	    player_said_2(talk_to, guard)) {
		player.walker_visible   = false;
		player.commands_allowed = false;

		if (global[player_persona] == PLAYER_IS_KING) {
			aa[1]                   = kernel_run_animation(kernel_name('k', 1), 0);
			local->anim_1_running   = true;
			local->king_tent_action = SHUT_UP;
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_PLAYER, 0);

			conv_run(CONV_40_KING_GUARD);
			conv_export_pointer(&global[game_points]);
			conv_export_pointer(&global[dance_points]);
			conv_export_pointer(&global[clue_points]);
			conv_export_value(global[pid_talk_shamon]);
		} else {
			aa[2]                   = kernel_run_animation(kernel_name('p', 1), 0);
			local->anim_2_running   = true;
			local->pid_tent_action  = SHUT_UP;
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_PLAYER, 0);
			conv_run(CONV_44_PID_GUARD);
			conv_export_value(global[pid_talk_shamon]);
		}
		local->guard_action = ARM_OUT;
		goto handled;
	}

	if (player_said_1(cross)) {
		text_show(40517);
		global[from_direction] = FROM_WEST;
		global[desert_room]    = 42;
		new_room               = 401;
		goto handled;
	}

	if (player.look_around) {
		text_show(40501);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(tent)) {
			text_show(40502);
			goto handled;
		}

		if (player_said_1(palm_tree)) {
			text_show(40505);
			goto handled;
		}

		if (player_said_1(oasis)) {
			text_show(40507);
			goto handled;
		}

		if (player_said_1(sand)) {
			text_show(40508);
			goto handled;
		}

		if (player_said_1(moon)) {
			text_show(40511);
			goto handled;
		}

		if (player_said_1(pool)) {
			text_show(40513);
			goto handled;
		}

		if (player_said_1(desert_to_east)) {
			text_show(40516);
			goto handled;
		}

		if (player_said_1(desert_sky)) {
			text_show(40518);
			goto handled;
		}

		if (player_said_1(guard)) {
			text_show(40523);
			goto handled;
		}

		if (player_said_1(shaman)) {
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(40527);
			} else if (global[talked_to_soptus]) {
				text_show(40519);
			} else {
				text_show(40520);
			}
			goto handled;
		}
	}

	if (player_said_2(open, tent)) {
		text_show(40503);
		goto handled;
	}

	if (player_said_2(take, pool) || player_said_3(fill, goblet, pool)) {
		text_show(40514);
		goto handled;
	}

	if (player_said_2(push, tent) || player_said_2(pull, tent)) {
		text_show(40504);
		goto handled;
	}

	if (player_said_2(push, palm_tree) || player_said_2(pull, palm_tree)) {
		text_show(40506);
		goto handled;
	}

	if (player_said_2(take, moon) || player_said_2(pull, moon)) {
		text_show(40512);
		goto handled;
	}

	if (player_said_2(push, guard) || player_said_2(throw, guard)) {
		text_show(40524);
		goto handled;
	}

	if (player_said_2(sword, attack) ||
	    player_said_2(sword, carve_up) ||
	    player_said_2(sword, thrust)) {
		if (player_said_1(guard)) {
			text_show(40525);
			goto handled;
		}
	}

	if (player_said_3(give, partial_bundle, shaman)) {
		text_show(40526);
		goto handled;
	}

	if (player_said_2(take, sand)) {
		text_show(40509);
		goto handled;
	}

	if (player_said_2(throw, sand)) {
		text_show(40510);
		goto handled;
	}

	if (player_said_2(push, shaman)) {
		text_show(40521);
		goto handled;
	}

	if (player_said_2(give, shaman)) {
		if (player_said_1(partly_built_bundle)) {
			text_show(40526);
			goto handled;
		} else if (player_has(object_named(player_main_noun))) {
			text_show(40522);
			goto handled;
		}
	}

	if (player_said_2(put, pool) ||
	    player_said_2(throw, pool) ||
	    player_said_2(pour_contents_of, pool)) {
		text_show(40515);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_405_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.guard_frame);
	s.syncAsSint16LE(scratch.guard_action);
	s.syncAsSint16LE(scratch.guard_talk_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.king_tent_frame);
	s.syncAsSint16LE(scratch.king_tent_action);
	s.syncAsSint16LE(scratch.king_tent_talk_count);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.pid_tent_frame);
	s.syncAsSint16LE(scratch.pid_tent_action);
	s.syncAsSint16LE(scratch.pid_tent_talk_count);
	s.syncAsSint16LE(scratch.anim_2_running);
	s.syncAsSint16LE(scratch.shaman_2_frame);
	s.syncAsSint16LE(scratch.shaman_2_action);
	s.syncAsSint16LE(scratch.shaman_2_talk_count);
	s.syncAsSint16LE(scratch.anim_3_running);
	s.syncAsSint16LE(scratch.shaman_1_frame);
	s.syncAsSint16LE(scratch.shaman_1_action);
	s.syncAsSint16LE(scratch.shaman_1_talk_count);
	s.syncAsSint16LE(scratch.anim_4_running);
	s.syncAsSint16LE(scratch.king_sit_frame);
	s.syncAsSint16LE(scratch.king_sit_action);
	s.syncAsSint16LE(scratch.king_sit_talk_count);
	s.syncAsSint16LE(scratch.anim_5_running);
	s.syncAsSint16LE(scratch.pid_sit_frame);
	s.syncAsSint16LE(scratch.pid_sit_action);
	s.syncAsSint16LE(scratch.pid_sit_talk_count);
	s.syncAsSint16LE(scratch.anim_6_running);
}

void room_405_preload() {
	room_init_code_pointer       = room_405_init;
	room_pre_parser_code_pointer = room_405_pre_parser;
	room_parser_code_pointer     = room_405_parser;
	room_daemon_code_pointer     = room_405_daemon;

	global[no_load_walker] = false;

	section_4_walker();
	section_4_interface();
}

void room_405_error() {
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
