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

#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section5.h"
#include "mads/madsv2/dragonsphere/rooms/room508.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[8];
	int16 lani_frame;
	int16 lani_action;
	int16 lani_count;
	int16 anim_0_running;
	int16 beast_frame;
	int16 beast_action;
	int16 beast_count;
	int16 anim_1_running;
	int16 king_frame;
	int16 king_frame_cliff;
	int16 king_frame_aside;
	int16 king_count;
	int16 king_action;
	int16 anim_2_running;
	int16 anim_3_running;
	int16 anim_4_running;
	int16 anim_6_running;
	int16 anim_7_running;
	int32 clock;
	int32 update_clock;
	int32 death_timer;
	int16 activate_timer;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_up_onto_ledge        0
#define fx_water                1

#define ROOM_508_DONE_UP        60
#define ROOM_508_DONE_DOWN      63
#define ROOM_508_YOU_TALK       65
#define ROOM_508_TALK_TO_LANI   67
#define ROOM_508_LANI_ATTACK    69
#define ROOM_508_KING_GET_UP    71

#define PLAYER_X_FROM_506       121
#define PLAYER_Y_FROM_506       121

#define PLAYER_X_FROM_509       6
#define PLAYER_Y_FROM_509       114

#define LANI_SHUT_UP            0
#define LANI_TALK               1
#define LANI_INVISIBLE          2
#define LANI_SCARED             3

#define LANI_X                  147
#define LANI_Y                  106

#define CONV_37_LANI            37

#define BEAST_INVISIBLE          0
#define BEAST_WAIT               1
#define BEAST_MOVE_IN            2

#define KING_INVISIBLE           0
#define KING_WAIT                1
#define KING_SWORD               2
#define KING_NOTHING             3

#define LENGTH_OF_LIFE            1300

#define RECOVER_X                41
#define RECOVER_Y                99

#define KING_UP_X                140
#define KING_UP_Y                79

#define PID_UP_X                 131
#define PID_UP_Y                 77


static void handle_animation_lani() {
	int lani_reset_frame;
	int random;

	if (kernel_anim[aa[0]].frame != local->lani_frame) {
		local->lani_frame = kernel_anim[aa[0]].frame;
		lani_reset_frame = -1;

		switch (local->lani_frame) {

		case 25:
			lani_reset_frame = 24;
			break;

		case 1:
		case 7:
		case 15:
		case 23:

			if (local->anim_3_running || local->anim_4_running || local->activate_timer) {
				lani_reset_frame = 0;

			} else switch (local->lani_action) {

			case LANI_TALK:
				random = imath_random(1, 2);
				if (random == 1) {
					lani_reset_frame = 1;
				} else {
					lani_reset_frame = 7;
				}
				local->lani_action = LANI_SHUT_UP;
				break;

			case LANI_SHUT_UP:
				++local->lani_count;
				if (local->lani_count > imath_random(15, 20)) {
					local->lani_count = 0;
					random = imath_random(1, 2);
					if (random == 1) {
						lani_reset_frame = 0;
					} else {
						lani_reset_frame = 15;
					}
				} else {
					lani_reset_frame = 0;
				}
				break;
			}
			break;

		case 19:

			if (local->anim_3_running || local->anim_4_running || local->activate_timer) {
				lani_reset_frame = 19;

			} else switch (local->lani_action) {

			case LANI_TALK:
				lani_reset_frame = 19;
				break;

			case LANI_SHUT_UP:
				++local->lani_count;
				if (local->lani_count > imath_random(15, 20)) {
					local->lani_count = 0;
					random = imath_random(1, 2);
					if (random == 1) {
						lani_reset_frame = 19;
					} else {
						lani_reset_frame = 18;
					}
				} else {
					lani_reset_frame = 18;
				}
				break;
			}
			break;
		}

		if (lani_reset_frame >= 0) {
			kernel_reset_animation(aa[0], lani_reset_frame);
			local->lani_frame = lani_reset_frame;
		}
	}
}

static void handle_animation_beast() {
	int beast_reset_frame;

	if (kernel_anim[aa[1]].frame != local->beast_frame) {
		local->beast_frame = kernel_anim[aa[1]].frame;
		beast_reset_frame = -1;

		switch (local->beast_frame) {

		case 40:
			if (local->beast_action == BEAST_INVISIBLE) {
				beast_reset_frame = 39;
			} else {
				beast_reset_frame = 0;
			}
			break;

		case 22:
		case 23:
		case 24:
		case 25:
		case 26:

			if (local->beast_action == BEAST_WAIT) {
				++local->beast_count;
				if (local->beast_count > imath_random(7, 15)) {
					if (local->beast_frame == 22) {
						beast_reset_frame = imath_random(21, 22);
					} else if (local->beast_frame == 23) {
						beast_reset_frame = imath_random(21, 23);
					} else if (local->beast_frame == 24) {
						beast_reset_frame = imath_random(22, 24);
					} else if (local->beast_frame == 25) {
						beast_reset_frame = imath_random(23, 25);
					} else if (local->beast_frame == 26) {
						beast_reset_frame = imath_random(24, 25);
					}
					local->beast_count = 0;

				} else {
					beast_reset_frame = local->beast_frame - 1;
				}

			} else {
				beast_reset_frame     = -1;
				local->anim_1_running = false;
				local->activate_timer = false;
				kernel_abort_animation(aa[1]);

				if (local->king_action == KING_SWORD) {
					aa[3] = kernel_run_animation(kernel_name('k', 3), ROOM_508_KING_GET_UP);
					kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
					local->anim_3_running = true;
					global[llanie_status] = SHE_FELL;

				} else {
					aa[4] = kernel_run_animation(kernel_name('k', 2), 0);
					kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
					local->anim_4_running = true;
				}
			}
			break;
		}

		if (beast_reset_frame >= 0) {
			kernel_reset_animation(aa[1], beast_reset_frame);
			local->beast_frame = beast_reset_frame;
		}
	}
}

static void handle_animation_king_choice() {
	int king_reset_frame;

	if (kernel_anim[aa[2]].frame != local->king_frame) {
		local->king_frame = kernel_anim[aa[2]].frame;
		king_reset_frame = -1;

		switch (local->king_frame) {

		case 21:
			if (local->king_action == KING_INVISIBLE) {
				king_reset_frame = 20;
			} else {
				king_reset_frame   = 0;
				local->king_action = KING_WAIT;
			}
			break;

		case 7:
			player.commands_allowed = true;
			local->beast_action     = BEAST_WAIT;
			local->activate_timer   = true;
			break;

		case 9:
		case 10:

			if (local->king_action == KING_WAIT ||
			    local->king_action == KING_NOTHING) {
				++local->king_count;
				if (local->king_count > imath_random(7, 13)) {
					local->king_count = 0;
					king_reset_frame  = imath_random(8, 9);

				} else {
					king_reset_frame = local->king_frame - 1;
				}
			}
			break;

		case 17:
			break;

		case 19:
			local->beast_action = BEAST_MOVE_IN;
			king_reset_frame = 18;
			break;
		}

		if (king_reset_frame >= 0) {
			kernel_reset_animation(aa[2], king_reset_frame);
			local->king_frame = king_reset_frame;
		}
	}
}

static void handle_animation_king_aside() {
	int king_reset_frame;

	if (kernel_anim[aa[3]].frame != local->king_frame_aside) {
		local->king_frame_aside = kernel_anim[aa[3]].frame;
		king_reset_frame = -1;

		switch (local->king_frame_aside) {

		case 20:
			kernel_reset_animation(aa[0], 24);
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_ANIM, aa[3]);
			break;

		case 13:
			kernel_reset_animation(aa[2], 20);
			local->king_action = KING_INVISIBLE;
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, aa[3]);
			break;
		}

		if (king_reset_frame >= 0) {
			kernel_reset_animation(aa[3], king_reset_frame);
			local->king_frame_aside = king_reset_frame;
		}
	}
}

static void handle_animation_king_cliff() {
	int king_reset_frame;

	if (kernel_anim[aa[4]].frame != local->king_frame_cliff) {
		local->king_frame_cliff = kernel_anim[aa[4]].frame;
		king_reset_frame = -1;

		switch (local->king_frame_cliff) {

		case 35:
			global_speech_go(3);
			break;

		case 42:
			kernel_reset_animation(aa[0], 24);
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_ANIM, aa[4]);
			break;

		case 49:
			if (game.difficulty == EASY_MODE) {
				text_show(50823);
			} else {
				text_show(45);
			}
			conv_reset(CONV_37_LANI);
			global[pid_just_died] = true;
			new_room              = 506;
			break;

		case 13:
			kernel_reset_animation(aa[2], 20);
			local->king_action = KING_INVISIBLE;
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, aa[4]);
			break;
		}

		if (king_reset_frame >= 0) {
			kernel_reset_animation(aa[4], king_reset_frame);
			local->king_frame_cliff = king_reset_frame;
		}
	}
}

static void room_508_init() {
	int id;

	if (global[llanie_status] == IS_SAVED) {
		global[make_504_empty] = true;
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_2_running = false;
		local->anim_3_running = false;
		local->anim_4_running = false;
		local->anim_6_running = false;
		local->anim_7_running = false;
		local->death_timer    = 0;
		local->clock          = 0;
		local->activate_timer = false;
	}

	ss[fx_water]  = kernel_load_series(kernel_name('x', 0), false);
	seq[fx_water] = kernel_seq_forward(ss[fx_water], false, 8, 0, 0, 0);
	kernel_seq_depth(seq[fx_water], 1);
	kernel_seq_range(seq[fx_water], KERNEL_FIRST, KERNEL_LAST);

	if (kernel.teleported_in) {
		inter_give_to_player(sword);
	}

	if (!global[monster_is_dead]) {
		conv_get(CONV_37_LANI);
	}

	local->lani_count  = 0;
	local->king_count  = 0;
	local->beast_count = 0;

	if (previous_room == 557 || previous_room == 506 || previous_room == 508) {

		player.commands_allowed = false;
		player.walker_visible   = false;

		if (global[player_persona] == PLAYER_IS_PID) {
			aa[0] = kernel_run_animation(kernel_name('p', 2), ROOM_508_DONE_UP);
			player.facing = FACING_NORTH;
			player.x      = PLAYER_X_FROM_506;
			player.y      = PLAYER_Y_FROM_506;

		} else {
			if (!global[monster_is_dead]) {
				aa[0]                   = kernel_run_animation(kernel_name('l', 1), 0);
				local->anim_0_running   = true;
				local->lani_action      = LANI_SHUT_UP;
				id = kernel_add_dynamic(words_Llanie, words_look_at, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
				                        0, 0, 0, 0);
				kernel_dynamic_hot[id].prep = PREP_ON;
				kernel_dynamic_anim(id, aa[0], 0);

				aa[2]                   = kernel_run_animation(kernel_name('k', 1), 0);
				local->anim_2_running   = true;
				local->king_action      = KING_INVISIBLE;
				kernel_reset_animation(aa[2], 21);

				aa[1]                   = kernel_run_animation(kernel_name('b', 1), 0);
				local->anim_1_running   = true;
				local->beast_action     = BEAST_INVISIBLE;
				kernel_reset_animation(aa[1], 40);
				id = kernel_add_dynamic(words_beast, words_look_at, SYNTAX_MASC_NOT_PROPER, KERNEL_NONE,
				                        0, 0, 0, 0);
				kernel_dynamic_hot[id].prep = PREP_ON;
				kernel_dynamic_anim(id, aa[1], 0);
			}

			aa[3] = kernel_run_animation(kernel_name('c', 1), ROOM_508_DONE_UP);
			player.facing = FACING_NORTHEAST;
			player.x      = PLAYER_X_FROM_506 + 8;
			player.y      = PLAYER_Y_FROM_506;
		}

	} else if (previous_room == 601) {
		if (global[player_persona] == PLAYER_IS_KING) {
			player.x      = KING_UP_X;
			player.y      = KING_UP_Y;
			player.facing = FACING_NORTH;
			aa[6]         = kernel_run_animation(kernel_name('k', 4), 0);
			kernel_synch(KERNEL_ANIM, aa[6], KERNEL_PLAYER, 0);
			kernel_reset_animation(aa[6], 78);
			local->anim_6_running   = true;

		} else {
			player.x      = PID_UP_X;
			player.y      = PID_UP_Y;
			player.facing = FACING_NORTH;
			aa[7]         = kernel_run_animation(kernel_name('p', 1), 0);
			kernel_synch(KERNEL_ANIM, aa[7], KERNEL_PLAYER, 0);
			kernel_reset_animation(aa[7], 92);
			local->anim_7_running   = true;
		}
		player.walker_visible   = false;
		player.commands_allowed = false;

	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player_first_walk(PLAYER_X_FROM_509,      PLAYER_Y_FROM_509, FACING_EAST,
		                  PLAYER_X_FROM_509 + 30, PLAYER_Y_FROM_509, FACING_EAST, true);

	} else if (previous_room == KERNEL_RESTORING_GAME) {

		if (!global[monster_is_dead]) {

			local->death_timer      = 0;
			player.walker_visible   = false;

			aa[0]                   = kernel_run_animation(kernel_name('l', 1), 0);
			local->anim_0_running   = true;
			local->lani_action      = LANI_SHUT_UP;
			id = kernel_add_dynamic(words_Llanie, words_look_at, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
			                        0, 0, 0, 0);
			kernel_dynamic_hot[id].prep = PREP_ON;
			kernel_dynamic_anim(id, aa[0], 0);

			aa[2]                   = kernel_run_animation(kernel_name('k', 1), 0);
			local->anim_2_running   = true;
			local->king_action      = KING_WAIT;
			kernel_reset_animation(aa[2], 9);

			aa[1]                   = kernel_run_animation(kernel_name('b', 1), 0);
			local->anim_1_running   = true;
			local->beast_action     = BEAST_WAIT;
			kernel_reset_animation(aa[1], 23);
			id = kernel_add_dynamic(words_beast, words_look_at, SYNTAX_MASC_NOT_PROPER, KERNEL_NONE,
			                        0, 0, 0, 0);
			kernel_dynamic_hot[id].prep = PREP_ON;
			kernel_dynamic_anim(id, aa[1], 0);
		}
	}

	local->update_clock = kernel.clock;

	section_5_music();
}

static void room_508_daemon() {
	int32 dif;
	int dist;

	if (local->anim_0_running) {
		handle_animation_lani();
	}

	if (local->anim_1_running) {
		handle_animation_beast();
	}

	if (local->anim_2_running) {
		handle_animation_king_choice();
	}

	if (local->anim_3_running) {
		handle_animation_king_aside();
	}

	if (local->anim_4_running) {
		handle_animation_king_cliff();
	}

	if (kernel.trigger == ROOM_508_DONE_UP) {
		player.walker_visible   = true;
		if (!global[monster_is_dead]) {
			player_walk(LANI_X, LANI_Y, FACING_NORTHEAST);
			player_walk_trigger(ROOM_508_TALK_TO_LANI);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[3]);
		} else {
			player.commands_allowed = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
		}
	}

	if (kernel.trigger == ROOM_508_TALK_TO_LANI) {
		conv_run(CONV_37_LANI);
	}

	if (kernel.trigger == ROOM_508_KING_GET_UP) {
		sound_play(N_BackgroundMus);
		kernel_abort_animation(aa[3]);
		kernel_abort_animation(aa[2]);
		kernel_abort_animation(aa[0]);
		local->anim_3_running   = false;
		local->anim_0_running   = false;
		local->anim_2_running   = false;
		player.walker_visible   = true;
		player.commands_allowed = true;
		global[monster_is_dead] = true;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player_demand_location(RECOVER_X, RECOVER_Y);
		player_demand_facing(FACING_SOUTH);
		++global[dragon_high_scene];
		text_show(50817);
	}

	if (local->activate_timer) {
		dif = kernel.clock - local->clock;
		if ((dif >= 0) && (dif <= 4)) {
			local->death_timer += dif;
		} else {
			local->death_timer += 1;
		}
		local->clock = kernel.clock;

		if (local->death_timer >= LENGTH_OF_LIFE) {
			local->king_action      = KING_NOTHING;
			local->beast_action     = BEAST_MOVE_IN;
			player.commands_allowed = false;
			local->activate_timer   = false;
		}
	}

	if (local->anim_6_running) {
		if (kernel_anim[aa[6]].frame == 75) {
			new_room = 601;

		} else if (kernel_anim[aa[6]].frame == 151) {
			kernel_abort_animation(aa[6]);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.walker_visible   = true;
			player.commands_allowed = true;
			local->anim_6_running   = false;
			player_walk(KING_UP_X + 4, KING_UP_Y + 10, FACING_SOUTH);
		}
	}

	if (local->anim_7_running) {
		if (kernel_anim[aa[7]].frame == 90) {
			new_room = 601;

		} else if (kernel_anim[aa[7]].frame == 181) {
			kernel_abort_animation(aa[7]);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.walker_visible   = true;
			player.commands_allowed = true;
			local->anim_7_running   = false;
			player_walk(KING_UP_X + 15, KING_UP_Y + 10, FACING_SOUTH);
		}
	}

	if (kernel.clock >= local->update_clock) {
		dist = 127 - ((imath_hypot(player.x - 12, player.y - 119) * 127) / 378);
		if (!sound_off) {
			sound_queue(N_005Waterfall, dist);
		}
		local->update_clock = kernel.clock + player.frame_delay;
	}
}

static void process_conv_lani() {
	int you_trig_flag = false;

	if (player_verb == conv037_next_b_b) {
		local->beast_action = BEAST_WAIT;
		sound_play(N_Battle);
	}

	if (player_verb == conv037_exit_b_b) {
		conv_abort();
		local->king_action      = KING_WAIT;
		player.walker_visible   = false;
		player.commands_allowed = false;
		kernel_reset_animation(aa[2], 0);
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[2]);
	}

	if (kernel.trigger == ROOM_508_YOU_TALK) {
		local->lani_action = LANI_TALK;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_508_YOU_TALK);
	}

	local->lani_count = 0;
}

static void room_508_pre_parser() {
	if (player_said_2(climb_down, rough_stone)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			player_walk(PLAYER_X_FROM_506 + 8, PLAYER_Y_FROM_506, FACING_NORTHEAST);
		}
	}

	if (player_said_2(climb_up, rough_stone)) {
		if (global[player_persona] == PLAYER_IS_PID) {
			player_walk(PID_UP_X, PID_UP_Y, FACING_NORTH);
		}
	}

	if (player_said_1(path_to_west) && player.need_to_walk) {
		if (!player_said_1(follow)) {
			player.need_to_walk = false;
		}
	}

	if (local->anim_0_running) {
		if (player.need_to_walk) {
			player.need_to_walk = false;
		}
	}
}

static void room_508_parser() {
	if (conv_control.running == CONV_37_LANI) {
		process_conv_lani();
		player.command_ready = false;
		return;
	}

	if (player_said_2(climb_up, rough_stone)) {
		if (local->anim_0_running) {
			text_show(50815);
			player.command_ready = false;
			return;

		} else if (global[player_persona] == PLAYER_IS_KING) {
			aa[6] = kernel_run_animation(kernel_name('k', 4), 0);
			kernel_synch(KERNEL_ANIM, aa[6], KERNEL_PLAYER, 0);
			local->anim_6_running   = true;

		} else {
			aa[7] = kernel_run_animation(kernel_name('p', 1), 0);
			kernel_synch(KERNEL_ANIM, aa[7], KERNEL_PLAYER, 0);
			local->anim_7_running   = true;
		}
		player.walker_visible   = false;
		player.commands_allowed = false;
		player.command_ready = false;
		return;
	}

	if (local->anim_0_running) {
		if (player_said_1(walk_across) ||
		    player_said_1(walk_to)) {
			text_show(50815);
			player.command_ready = false;
			return;
		}
	}

	if (kernel.trigger == ROOM_508_DONE_DOWN) {
		new_room = 506;
		player.command_ready = false;
		return;
	}

	if (player_said_2(invoke, signet_ring)) {
		if (local->anim_0_running) {
			text_show(50824);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(follow, path_to_west)) {
		if (local->anim_0_running) {
			text_show(50815);
		} else {
			new_room = 509;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(climb_up, rough_stone)) {
		if (local->anim_0_running) {
			text_show(50815);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(climb_down, rough_stone)) {
		if (local->anim_0_running) {
			text_show(50815);
			player.command_ready = false;
			return;

		} else {
			player.commands_allowed = false;
			player.walker_visible   = false;

			if (global[player_persona] == PLAYER_IS_PID) {
				aa[0] = kernel_run_animation(kernel_name('p', 3), ROOM_508_DONE_DOWN);
				kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);

			} else {
				aa[0] = kernel_run_animation(kernel_name('c', 2), ROOM_508_DONE_DOWN);
				kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
			}
			player.command_ready = false;
			return;
		}
	}

	if (player.look_around) {
		text_show(50801);
		player.command_ready = false;
		return;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(sky)) {
			text_show(50802);
			player.command_ready = false;
			return;
		}

		if (player_said_1(edge_of_cliff)) {
			text_show(50803);
			player.command_ready = false;
			return;
		}

		if (player_said_1(mountainside)) {
			text_show(50804);
			player.command_ready = false;
			return;
		}

		if (player_said_1(rough_stone)) {
			if (inter_point_y < 100) {
				text_show(50805);
			} else {
				text_show(50806);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(path_to_west)) {
			text_show(50807);
			player.command_ready = false;
			return;
		}

		if (player_said_1(waterfall)) {
			text_show(50808);
			player.command_ready = false;
			return;
		}

		if (player_said_1(ledge)) {
			text_show(50812);
			player.command_ready = false;
			return;
		}

		if (player_said_1(beast)) {
			text_show(50813);
			player.command_ready = false;
			return;
		}

		if (player_said_1(Llanie)) {
			text_show(50814);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_3(fill, goblet, waterfall) ||
	    player_said_3(put, goblet, waterfall)) {
		text_show(50809);
		player.command_ready = false;
		return;
	}

	if (player_said_2(put, waterfall)) {
		if (player_said_1(torch)) {
			text_show(50810);
		} else {
			text_show(50811);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_3(sword, attack, beast) ||
	    player_said_3(sword, carve_up, beast) ||
	    player_said_3(sword, thrust, beast) ||
	    player_said_2(take, sword)) {

		if (local->anim_0_running) {
			++global[player_score];
			local->king_action      = KING_SWORD;
			player.commands_allowed = false;
			sound_play(N_Battle);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_3(throw, shieldstone, beast)) {
		text_show(50818);
		player.command_ready = false;
		return;
	}

	if (player_said_2(talk_to, Llanie)) {
		text_show(50820);
		player.command_ready = false;
		return;
	}

	if (player_said_2(talk_to, beast)) {
		text_show(50819);
		player.command_ready = false;
		return;
	}

	if (player_said_2(push, Llanie)) {
		text_show(50822);
		player.command_ready = false;
		return;
	}

	if (player_said_2(push, beast)) {
		text_show(50821);
		player.command_ready = false;
		return;
	}
}

void room_508_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.lani_frame);
	s.syncAsSint16LE(scratch.lani_action);
	s.syncAsSint16LE(scratch.lani_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.beast_frame);
	s.syncAsSint16LE(scratch.beast_action);
	s.syncAsSint16LE(scratch.beast_count);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.king_frame);
	s.syncAsSint16LE(scratch.king_frame_cliff);
	s.syncAsSint16LE(scratch.king_frame_aside);
	s.syncAsSint16LE(scratch.king_count);
	s.syncAsSint16LE(scratch.king_action);
	s.syncAsSint16LE(scratch.anim_2_running);
	s.syncAsSint16LE(scratch.anim_3_running);
	s.syncAsSint16LE(scratch.anim_4_running);
	s.syncAsSint16LE(scratch.anim_6_running);
	s.syncAsSint16LE(scratch.anim_7_running);
	s.syncAsSint32LE(scratch.clock);
	s.syncAsSint32LE(scratch.update_clock);
	s.syncAsSint32LE(scratch.death_timer);
	s.syncAsSint16LE(scratch.activate_timer);
}

void room_508_preload() {
	room_init_code_pointer       = room_508_init;
	room_pre_parser_code_pointer = room_508_pre_parser;
	room_parser_code_pointer     = room_508_parser;
	room_daemon_code_pointer     = room_508_daemon;

	section_5_walker();
	section_5_interface();

	vocab_make_active(words_beast);
	vocab_make_active(words_Llanie);
	vocab_make_active(words_look_at);
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
