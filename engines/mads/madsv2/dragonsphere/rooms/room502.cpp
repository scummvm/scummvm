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
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section5.h"
#include "mads/madsv2/dragonsphere/rooms/room502.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];
	int16 lani_frame;
	int16 lani_action;
	int16 lani_talk_count;
	int16 anim_0_running;
	int16 anim_1_running;
	int16 lani_location;
	int16 prevent;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_up             0
#define fx_down           1
#define fx_give           2

#define ROOM_502_DONE_DOWN    60
#define ROOM_502_YOU_TALK     65
#define ROOM_502_ME_TALK      67
#define ROOM_502_COME_OUT     69
#define ROOM_502_FOOT         71
#define ROOM_502_DONE_GIVING  73
#define ROOM_502_TALK         75

#define PLAYER_X_FROM_503    -20
#define PLAYER_Y_FROM_503    148
#define WALK_TO_X_FROM_503    13
#define WALK_TO_Y_FROM_503   148
#define PLAYER_X_FROM_501    207
#define PLAYER_Y_FROM_501    152
#define PLAYER_X_FROM_506    248
#define PLAYER_Y_FROM_506    109
#define WALK_TO_X_FROM_506   227
#define WALK_TO_Y_FROM_506   116

#define CONV_15_LANI          15

#define LANI_BOW              0
#define LANI_SHUT_UP          1
#define LANI_STAMP_FOOT       2
#define LANI_TALK             3
#define LANI_LEAN             4
#define LANI_KISS             5
#define LANI_GIVE             6

#define WALK_TO_LANI_X       130
#define WALK_TO_LANI_Y       120

#define BEHIND_ROCK           0
#define OUT_IN_OPEN           1
#define AGAINST_ROCK          2


static void handle_animation_lani_1() {
	int lani_reset_frame;

	if (kernel_anim[aa[1]].frame != local->lani_frame) {
		local->lani_frame = kernel_anim[aa[1]].frame;
		lani_reset_frame  = -1;

		switch (local->lani_frame) {
		case 1:
		case 2:
		case 3:
			switch (local->lani_action) {
			case LANI_TALK:
				lani_reset_frame = imath_random(0, 2);
				++local->lani_talk_count;
				if (local->lani_talk_count > 17) {
					local->lani_action     = LANI_SHUT_UP;
					local->lani_talk_count = 0;
					lani_reset_frame       = 0;
				}
				break;

			default:
				lani_reset_frame = 0;
				break;
			}
			break;
		}

		if (lani_reset_frame >= 0) {
			kernel_reset_animation(aa[1], lani_reset_frame);
			local->lani_frame = lani_reset_frame;
		}
	}
}

static void handle_animation_lani_0() {
	int lani_reset_frame;
	int random;
	int id;

	if (kernel_anim[aa[0]].frame != local->lani_frame) {
		local->lani_frame = kernel_anim[aa[0]].frame;
		lani_reset_frame  = -1;

		switch (local->lani_frame) {
		case 1:
		case 2:
		case 3:
			if (local->lani_action != LANI_BOW) {
				++local->lani_talk_count;
				if (local->lani_talk_count > imath_random(15, 25)) {
					lani_reset_frame = imath_random(0, 2);
					local->lani_talk_count = 0;
				} else {
					lani_reset_frame = local->lani_frame - 1;
				}
			} else {
				lani_reset_frame     = 3;
				local->lani_location = OUT_IN_OPEN;
			}
			break;

		case 21:
		case 122:
			conv_release();
			break;

		case 49:
			lani_reset_frame = 153;
			break;

		case 22:
		case 123:
		case 43:
		case 30:
		case 53:
		case 75:
		case 99:
		case 87:
			if (local->lani_frame == 99) {
				sound_play(N_TakeObjectSnd);
				inter_give_to_player(amulet);
				object_examine(amulet, 819, 0);
				global[player_score] += 2;
			}

			switch (local->lani_action) {
			case LANI_SHUT_UP:
				++local->lani_talk_count;
				if (local->lani_talk_count > imath_random(20, 30)) {
					random = imath_random(1, 3);
					switch (random) {
					case 1: lani_reset_frame = 21;  break;
					case 2: lani_reset_frame = 53;  break;
					case 3: lani_reset_frame = 30;  break;
					}
					local->lani_talk_count = 0;
				} else {
					lani_reset_frame = 21;
				}
				break;

			case LANI_STAMP_FOOT:
				lani_reset_frame = 53;
				break;

			case LANI_BOW:
				lani_reset_frame = 22;
				local->lani_action = LANI_SHUT_UP;
				break;

			case LANI_TALK:
				random = imath_random(1, 2);
				if (random == 1) {
					lani_reset_frame = 75;
				} else {
					lani_reset_frame = 45;
				}
				break;

			case LANI_LEAN:
				lani_reset_frame = 123;
				break;

			case LANI_KISS:
				lani_reset_frame = 104;
				break;

			case LANI_GIVE:
				lani_reset_frame = 87;
				break;
			}
			break;

		case 81:
		case 82:
		case 83:
		case 84:
			switch (local->lani_action) {
			case LANI_TALK:
				lani_reset_frame = imath_random(81, 83);
				++local->lani_talk_count;
				if (local->lani_talk_count > 17) {
					local->lani_action     = LANI_SHUT_UP;
					local->lani_talk_count = 0;
					lani_reset_frame       = 84;
				}
				break;

			default:
				lani_reset_frame = 84;
				break;
			}
			break;

		case 97:
			player.walker_visible = false;
			seq[fx_give]          = kernel_seq_pingpong(ss[fx_give], true, 10, 0, 0, 2);
			kernel_seq_depth(seq[fx_give], 1);
			kernel_seq_player(seq[fx_give], true);
			kernel_seq_range(seq[fx_give], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_give], KERNEL_TRIGGER_EXPIRE, 0, ROOM_502_DONE_GIVING);
			break;

		case 98:
			switch (local->lani_action) {
			case LANI_GIVE:
				lani_reset_frame = 97;
				break;

			default:
				lani_reset_frame = 98;
				break;
			}
			break;

		case 134:
			kernel_abort_animation(aa[0]);
			aa[1] = kernel_run_animation(kernel_name('l', 2), 0);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
			id = kernel_add_dynamic(words_Llanie, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
			                        0, 0, 0, 0);
			kernel_dynamic_hot[id].prep = PREP_ON;
			kernel_dynamic_walk(id, WALK_TO_LANI_X, WALK_TO_LANI_Y, FACING_NORTHWEST);
			kernel_dynamic_anim(id, aa[1], 0);

			local->anim_0_running         = false;
			local->anim_1_running         = true;
			lani_reset_frame              = -1;
			player.commands_allowed       = true;
			local->lani_location          = AGAINST_ROCK;
			global[done_talking_lani_502] = true;
			local->lani_action            = LANI_SHUT_UP;
			kernel_flip_hotspot(words_stranger, false);
			break;

		case 135:
		case 136:
		case 137:
			switch (local->lani_action) {
			case LANI_TALK:
				lani_reset_frame = imath_random(134, 136);
				++local->lani_talk_count;
				if (local->lani_talk_count > 17) {
					local->lani_action     = LANI_SHUT_UP;
					local->lani_talk_count = 0;
					lani_reset_frame       = 134;
				}
				break;

			default:
				lani_reset_frame = 134;
				break;
			}
			break;

		case 59:
		case 70:
		case 104:
			switch (local->lani_action) {
			case LANI_STAMP_FOOT:
				lani_reset_frame   = 59;
				local->lani_action = LANI_SHUT_UP;
				break;

			case LANI_SHUT_UP:
				++local->lani_talk_count;
				if (local->lani_talk_count > 17) {
					random = imath_random(1, 3);
					switch (random) {
					case 1: lani_reset_frame = 58;  break;
					case 2: lani_reset_frame = 70;  break;
					case 3: lani_reset_frame = 99;  break;
					}
					local->lani_talk_count = 0;
				} else {
					lani_reset_frame = 58;
				}
				break;

			default:
				lani_reset_frame = 70;
				break;
			}
			break;

		case 102:
			switch (local->lani_action) {
			case LANI_SHUT_UP:
				lani_reset_frame = 101;
				++local->lani_talk_count;
				if (local->lani_talk_count > 17) {
					lani_reset_frame       = imath_random(101, 102);
					local->lani_talk_count = 0;
				} else {
					lani_reset_frame = 101;
				}
				break;

			default:
				lani_reset_frame = 102;
				break;
			}
			break;

		case 154:
		case 155:
		case 156:
			switch (local->lani_action) {
			case LANI_TALK:
				lani_reset_frame = imath_random(153, 155);
				++local->lani_talk_count;
				if (local->lani_talk_count > 10) {
					local->lani_action     = LANI_SHUT_UP;
					local->lani_talk_count = 0;
					lani_reset_frame       = 49;
				}
				break;

			default:
				lani_reset_frame = 49;
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

static void room_502_init() {
	// TODO: id used in kernel_dynamic_anim before being set in original.
	// Perhaps a previously undiscovered logic bug?
	int id = 0;

	kernel.disable_fastwalk = true;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->prevent        = false;
		local->lani_location  = -1;
	}

	if (previous_room == KERNEL_RESTORING_GAME) {
		if (local->anim_1_running) {
			conv_get(CONV_15_LANI);
			aa[1]                 = kernel_run_animation(kernel_name('l', 2), 0);
			local->anim_1_running = true;
			kernel_dynamic_anim(id, aa[1], 3);
			kernel_flip_hotspot(words_stranger, false);
			id = kernel_add_dynamic(words_Llanie, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
			                        0, 0, 0, 0);
			kernel_dynamic_hot[id].prep = PREP_ON;
			kernel_dynamic_walk(id, WALK_TO_LANI_X, WALK_TO_LANI_Y, FACING_NORTHWEST);
			kernel_dynamic_anim(id, aa[1], 0);
		}
	}

	if (!global[done_talking_lani_502]) {
		ss[fx_give] = kernel_load_series("*KGRD_9", false);
		conv_get(CONV_15_LANI);

		id = kernel_add_dynamic(words_Llanie, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
		                        0, 0, 0, 0);
		kernel_dynamic_hot[id].prep = PREP_ON;
		kernel_dynamic_walk(id, WALK_TO_LANI_X, WALK_TO_LANI_Y, FACING_NORTHWEST);

		if (local->lani_location == AGAINST_ROCK) {
			aa[1]                 = kernel_run_animation(kernel_name('l', 2), 0);
			local->anim_1_running = true;
			kernel_dynamic_anim(id, aa[1], 0);
		} else {
			aa[0]                 = kernel_run_animation(kernel_name('l', 1), 0);
			local->anim_0_running = true;
			kernel_dynamic_anim(id, aa[0], 3);
		}

		local->lani_action = LANI_SHUT_UP;

		if (previous_room == KERNEL_RESTORING_GAME) {
			if (local->lani_location == OUT_IN_OPEN) {
				kernel_reset_animation(aa[0], 22);
				kernel_flip_hotspot(words_stranger, false);
			} else if (local->lani_location == AGAINST_ROCK) {
				kernel_flip_hotspot(words_stranger, false);
			}

			if (conv_restore_running == CONV_15_LANI) {
				player.commands_allowed = false;
				conv_run(CONV_15_LANI);
			}
		} else {
			local->lani_location = BEHIND_ROCK;
		}
	} else {
		kernel_flip_hotspot(words_stranger, false);
	}

	if (previous_room == 503) {
		player_first_walk(PLAYER_X_FROM_503, PLAYER_Y_FROM_503, FACING_EAST,
		                  WALK_TO_X_FROM_503, WALK_TO_Y_FROM_503, FACING_EAST, true);

	} else if (previous_room == 506) {
		player.commands_allowed = false;
		player.walker_visible   = false;
		player.x                = PLAYER_X_FROM_506;
		player.y                = PLAYER_Y_FROM_506;
		player.facing           = FACING_NORTH;

		if (global[player_persona] == PLAYER_IS_KING) {
			aa[0] = kernel_run_animation(kernel_name('k', 2), ROOM_502_DONE_DOWN);
			kernel_reset_animation(aa[0], 1);
		} else {
			ss[fx_down]  = kernel_load_series(kernel_name('b', 0), false);
			seq[fx_down] = kernel_seq_backward(ss[fx_down], false, 6, 0, 0, 1);
			kernel_seq_depth(seq[fx_down], 1);
			kernel_seq_range(seq[fx_down], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_down], KERNEL_TRIGGER_EXPIRE, 0, ROOM_502_DONE_DOWN + 1);
		}
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x      = PLAYER_X_FROM_501;
		player.y      = PLAYER_Y_FROM_501;
		player.facing = FACING_NORTH;
	}

	section_5_music();
}

static void room_502_daemon() {
	if (local->anim_0_running) {
		handle_animation_lani_0();
	}

	if (local->anim_1_running) {
		handle_animation_lani_1();
	}

	if (local->anim_0_running && !local->prevent) {
		if (player.x < 60) {
			text_show(50219);
			player_walk(WALK_TO_LANI_X, WALK_TO_LANI_Y, FACING_NORTHWEST);
			player_walk_trigger(ROOM_502_TALK);
			player.commands_allowed = false;
			local->prevent          = true;
		}
	}

	if (kernel.trigger == ROOM_502_TALK) {
		kernel_timing_trigger(1, ROOM_502_TALK + 1);
	}

	if (kernel.trigger == ROOM_502_TALK + 1) {
		conv_run(CONV_15_LANI);
	}

	if (kernel.trigger == ROOM_502_DONE_DOWN) {
		player.walker_visible   = true;
		player.commands_allowed = true;
		player_walk(WALK_TO_X_FROM_506, WALK_TO_Y_FROM_506, FACING_SOUTH);
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
	}

	if (kernel.trigger == ROOM_502_DONE_DOWN + 1) {
		player.walker_visible   = true;
		player.commands_allowed = true;
		player_walk(WALK_TO_X_FROM_506, WALK_TO_Y_FROM_506, FACING_SOUTH);
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_down]);
	}

	if (kernel.trigger == ROOM_502_DONE_GIVING) {
		local->lani_action    = LANI_SHUT_UP;
		player.walker_visible = true;
		conv_release();
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_give]);
	}
}

static void process_conv_lani() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	switch (player_verb) {
	case conv015_seeher_show:
	case conv015_seeher_who:
	case conv015_seeher_comeout:
		you_trig_flag = true;
		me_trig_flag  = true;
		if (!kernel.trigger) {
			conv_you_trigger(ROOM_502_COME_OUT);
		}
		break;

	case conv015_exit_b_b:
		if (local->lani_location != AGAINST_ROCK) {
			you_trig_flag = true;
			me_trig_flag  = true;
			local->lani_action = LANI_LEAN;
		}
		break;

	case conv015_gift3_b_b:
		you_trig_flag      = true;
		me_trig_flag       = true;
		local->lani_action = LANI_GIVE;
		conv_hold();
		break;

	case conv015_kiss_b_b:
		you_trig_flag      = true;
		me_trig_flag       = true;
		local->lani_action = LANI_KISS;
		sound_play(N_TheKiss);
		conv_hold();
		break;

	case conv015_react_gohome:
	case conv015_answer_go:
		you_trig_flag = true;
		me_trig_flag  = true;
		if (!kernel.trigger) {
			conv_you_trigger(ROOM_502_FOOT);
		}
		break;
	}

	if (kernel.trigger == ROOM_502_COME_OUT) {
		local->lani_action = LANI_BOW;
		conv_hold();
	}

	if (kernel.trigger == ROOM_502_FOOT) {
		local->lani_action = LANI_STAMP_FOOT;
	}

	if (kernel.trigger == ROOM_502_YOU_TALK) {
		if (local->lani_action != LANI_BOW       &&
		    local->lani_action != LANI_STAMP_FOOT &&
		    local->lani_action != LANI_LEAN       &&
		    local->lani_action != LANI_KISS       &&
		    local->lani_action != LANI_GIVE) {
			local->lani_action = LANI_TALK;
		}
	}

	if (kernel.trigger == ROOM_502_ME_TALK) {
		if (local->lani_action != LANI_BOW       &&
		    local->lani_action != LANI_STAMP_FOOT &&
		    local->lani_action != LANI_LEAN       &&
		    local->lani_action != LANI_KISS       &&
		    local->lani_action != LANI_GIVE) {
			local->lani_action = LANI_SHUT_UP;
		}
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_502_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_502_ME_TALK);
	}

	local->lani_talk_count = 0;
}

static void room_502_pre_parser() {
	if (player_said_2(walk_down, path_to_west)) {
		player.walk_off_edge_to_room = 503;
	}
}

static void room_502_parser() {
	if (conv_control.running == CONV_15_LANI) {
		process_conv_lani();
		player.command_ready = false;
		return;
	}

	if (kernel.trigger == ROOM_502_TALK) {
		kernel_timing_trigger(1, ROOM_502_TALK + 1);
		player.command_ready = false;
		return;
	}

	if (kernel.trigger == ROOM_502_TALK + 1) {
		conv_run(CONV_15_LANI);
		local->prevent = true;
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_down, path_to_west)) {
		if (local->anim_0_running) {
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(walk_down, path_to_south)) {
		new_room = 501;
		player.command_ready = false;
		return;
	}

	if (player.look_around) {
		if (global[sorceror_defeated]) {
			text_show(50214);

		} else if (game.difficulty == EASY_MODE) {
			text_show(50201);
			if (global[llanie_status] == BEFORE_FALL) {
				text_show(50202);
			}

		} else {
			text_show(50201);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(climb_up, rough_stone)) {
		switch (kernel.trigger) {
		case 0:
			if (!global[done_talking_lani_502]) {
				text_show(50219);
				player_walk(WALK_TO_LANI_X, WALK_TO_LANI_Y, FACING_NORTHWEST);
				player_walk_trigger(ROOM_502_TALK);
				player.commands_allowed = false;

			} else {
				player.commands_allowed = false;
				player.walker_visible   = false;
				if (global[player_persona] == PLAYER_IS_KING) {
					aa[0] = kernel_run_animation(kernel_name('k', 1), 1);
					kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
				} else {
					aa[3] = kernel_run_animation(kernel_name('p', 2), 1);
					kernel_synch(KERNEL_ANIM, aa[3], KERNEL_PLAYER, 0);
				}
			}
			break;

		case 1:
			new_room = 506;
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(path_to_south)) {
			text_show(50203);
			player.command_ready = false;
			return;
		}

		if (player_said_1(path_to_west)) {
			if (!global[found_lani_504] && global[monster_is_dead]) {
				text_show(50213);
			} else if (global[sorceror_defeated]) {
				text_show(50215);
			} else {
				text_show(50204);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(rough_stone)) {
			text_show(50205);
			player.command_ready = false;
			return;
		}

		if (player_said_1(large_rock)) {
			if (local->anim_0_running || local->anim_1_running) {
				if (local->lani_location == AGAINST_ROCK) {
					text_show(50212);
				} else if (game.difficulty == EASY_MODE) {
					text_show(50207);
				} else {
					text_show(50206);
				}
			} else {
				if (!global[monster_is_dead]) {
					text_show(50220);
				} else {
					text_show(50206);
				}
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(small_rock)) {
			text_show(50208);
			player.command_ready = false;
			return;
		}

		if (player_said_1(mountainside)) {
			text_show(50210);
			player.command_ready = false;
			return;
		}

		if (player_said_1(Llanie)) {
			text_show(50211);
			player.command_ready = false;
			return;
		}

		if (player_said_1(stranger)) {
			text_show(50219);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(take, small_rock)) {
		text_show(50209);
		player.command_ready = false;
		return;
	}

	if (player_said_3(give, amulet, Llanie)) {
		text_show(50217);
		player.command_ready = false;
		return;
	}

	if (player_said_3(put, tentacle_parts, mountainside)) {
		text_show(50218);
		player.command_ready = false;
		return;
	}

	if (player_said_1(talk_to)) {
		if (player_said_1(Llanie) || player_said_1(stranger)) {
			if (local->lani_location != AGAINST_ROCK) {
				player.commands_allowed = false;
			}
			conv_run(CONV_15_LANI);
			local->prevent = true;
			player.command_ready = false;
			return;
		}
	}
}

void room_502_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(local->lani_frame);
	s.syncAsSint16LE(local->lani_action);
	s.syncAsSint16LE(local->lani_talk_count);
	s.syncAsSint16LE(local->anim_0_running);
	s.syncAsSint16LE(local->anim_1_running);
	s.syncAsSint16LE(local->lani_location);
	s.syncAsSint16LE(local->prevent);
}

void room_502_preload() {
	room_init_code_pointer       = room_502_init;
	room_pre_parser_code_pointer = room_502_pre_parser;
	room_parser_code_pointer     = room_502_parser;
	room_daemon_code_pointer     = room_502_daemon;

	section_5_walker();
	section_5_interface();

	vocab_make_active(words_Llanie);
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
