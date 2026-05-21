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
#include "mads/madsv2/core/hspot.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section4.h"
#include "mads/madsv2/dragonsphere/rooms/room411.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[5];
	int16 bird_1_frame;
	int16 bird_1_action;
	int16 bird_1_talk_count;
	int16 anim_0_running;
	int16 bird_2_frame;
	int16 bird_2_action;
	int16 bird_2_talk_count;
	int16 anim_1_running;
	int16 bird_3_frame;
	int16 bird_3_action;
	int16 bird_3_talk_count;
	int16 anim_2_running;
	int16 bird_death_frame;
	int16 bird_death_action;
	int16 bird_death_talk_count;
	int16 anim_3_running;
	int16 zap_frame;
	int16 anim_4_running;
	int16 prevent;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_egg          0
#define fx_grapes       1
#define fx_throw        2
#define fx_take         3
#define fx_fall         4

#define ROOM_411_COMMANDS_ALLOWED  40
#define ROOM_411_DONE_TAKING       50
#define ROOM_411_KILL_PID          60
#define ROOM_411_FALL              65

#define PLAYER_X_FROM_410   -13
#define PLAYER_Y_FROM_410   112
#define WALK_TO_X_FROM_410   13
#define WALK_TO_Y_FROM_410  112

#define FREEZE          0
#define KILL            1
#define EAT             2
#define THROW           3
#define WAIT_FOR_KILL   4

#define BIRD_X          122
#define BIRD_Y           95

static Scratch scratch;


static void handle_animation_bird_1() {
	int bird_1_reset_frame;

	if (kernel_anim[aa[0]].frame != local->bird_1_frame) {
		local->bird_1_frame = kernel_anim[aa[0]].frame;
		bird_1_reset_frame = -1;

		switch (local->bird_1_frame) {

		case 6:
			kernel_seq_delete(seq[fx_grapes]);
			kernel_flip_hotspot(words_magic_grapes, false);
			global[grapes_have_grown] = GRAPES_NOT_THERE;
			break;

		case 20:
		case 21:
		case 22:
		case 23:
			switch (local->bird_1_action) {
			case WAIT_FOR_KILL:
				if (local->bird_1_frame == 23) {
					bird_1_reset_frame = 21;
				} else if (local->bird_1_frame == 22) {
					bird_1_reset_frame = 20;
				} else {
					text_show(41145);
					kernel_abort_animation(aa[0]);
					local->anim_0_running = false;
					local->anim_3_running = true;
					aa[3] = kernel_run_animation(kernel_name('d', 1), 0);
					kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
				}
				break;

			case FREEZE:
			case THROW:
				if (imath_random(1, 12) == 1 && local->bird_1_frame == 21 && global[grapes_have_grown] == GRAPES_GROWN) {
					bird_1_reset_frame = 0;
				} else {
					++local->bird_1_talk_count;
					if (local->bird_1_talk_count > imath_random(10, 15)) {
						if (local->bird_1_frame == 21) {
							bird_1_reset_frame = imath_random(20, 21);
						} else if (local->bird_1_frame == 22) {
							bird_1_reset_frame = imath_random(20, 22);
						} else if (local->bird_1_frame == 23) {
							bird_1_reset_frame = imath_random(21, 22);
						}
						local->bird_1_talk_count = 0;
					} else {
						bird_1_reset_frame = local->bird_1_frame - 1;
					}

					if (local->bird_1_action == THROW ||
					    local->bird_1_action == WAIT_FOR_KILL) {
						bird_1_reset_frame = 20;
					}
				}
				break;
			}
			break;
		}

		if (bird_1_reset_frame >= 0) {
			kernel_reset_animation(aa[0], bird_1_reset_frame);
			local->bird_1_frame = bird_1_reset_frame;
		}
	}
}

static void handle_animation_bird_2() {
	int bird_2_reset_frame;

	if (kernel_anim[aa[1]].frame != local->bird_2_frame) {
		local->bird_2_frame = kernel_anim[aa[1]].frame;
		bird_2_reset_frame = -1;

		switch (local->bird_2_frame) {
		case 5:
			inter_move_object(fruit, NOWHERE);
			break;

		case 8:
			player.walker_visible   = true;
			player.commands_allowed = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
			break;

		case 36:
			kernel_abort_animation(aa[1]);
			aa[0] = kernel_run_animation(kernel_name('b', 1), 0);
			kernel_reset_animation(aa[0], 21);
			player.commands_allowed = true;
			local->anim_1_running   = false;
			local->anim_0_running   = true;
			local->bird_1_action    = FREEZE;
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_NOW, 0);
			break;
		}

		if (bird_2_reset_frame >= 0) {
			kernel_reset_animation(aa[1], bird_2_reset_frame);
			local->bird_2_frame = bird_2_reset_frame;
		}
	}
}

static void handle_animation_bird_3() {
	int bird_3_reset_frame;

	if (kernel_anim[aa[2]].frame != local->bird_3_frame) {
		local->bird_3_frame = kernel_anim[aa[2]].frame;
		bird_3_reset_frame = -1;

		switch (local->bird_3_frame) {
		case 5:
			inter_move_object(dates, NOWHERE);
			break;

		case 8:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[2]);
			break;

		case 42:
			bird_3_reset_frame = 23;
			break;

		case 18:
			sound_play(N_BigBirdCall);
			break;

		case 24:
		case 33:
			if (local->bird_3_action == KILL) {
				bird_3_reset_frame = 33;
			} else {
				player.commands_allowed = true;
				if (imath_random(1, 3) == 1) {
					bird_3_reset_frame = 24;
				} else {
					bird_3_reset_frame = 18;
				}
			}
			break;

		case 37:
			kernel_abort_animation(aa[2]);
			text_show(41145);
			local->anim_2_running = false;
			local->anim_3_running = true;
			aa[3] = kernel_run_animation(kernel_name('d', 1), 0);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
			break;
		}

		if (bird_3_reset_frame >= 0) {
			kernel_reset_animation(aa[2], bird_3_reset_frame);
			local->bird_3_frame = bird_3_reset_frame;
		}
	}
}

static void handle_animation_bird_death() {
	int bird_death_reset_frame;

	if (kernel_anim[aa[3]].frame != local->bird_death_frame) {
		local->bird_death_frame = kernel_anim[aa[3]].frame;
		bird_death_reset_frame = -1;

		switch (local->bird_death_frame) {
		case 27:
			seq[fx_fall] = kernel_seq_forward(ss[fx_fall], false, 4, 0, 0, 1);
			kernel_seq_depth(seq[fx_fall], 1);
			kernel_seq_range(seq[fx_fall], 6, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_fall], KERNEL_TRIGGER_EXPIRE, 0, ROOM_411_FALL);
			kernel_synch(KERNEL_SERIES, seq[fx_fall], KERNEL_ANIM, aa[3]);
			break;

		case 1:
			player.walker_visible = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[3]);
			break;

		case 9:
			global_speech_go(7);
			break;

		case 48:
			if (game.difficulty == EASY_MODE) {
				text_show(41165);
			} else {
				text_show(45);
			}
			global[pid_just_died] = true;
			--global[player_score];
			kernel.force_restart  = true;
			break;
		}

		if (bird_death_reset_frame >= 0) {
			kernel_reset_animation(aa[3], bird_death_reset_frame);
			local->bird_death_frame = bird_death_reset_frame;
		}
	}
}

static void handle_animation_zap() {
	int zap_reset_frame;

	if (kernel_anim[aa[4]].frame != local->zap_frame) {
		local->zap_frame = kernel_anim[aa[4]].frame;
		zap_reset_frame = -1;

		switch (local->zap_frame) {
		case 30:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[4]);
			break;

		case 42:
			local->anim_4_running   = false;
			player.commands_allowed = true;
			if (global[grapes_have_grown] == GRAPES_GROWN) {
				text_show(41152);
				global[grapes_are_dead] = true;
			} else {
				text_show(41109);
				global[grapes_are_dead] = true;
			}
			break;
		}

		if (zap_reset_frame >= 0) {
			kernel_reset_animation(aa[4], zap_reset_frame);
			local->zap_frame = zap_reset_frame;
		}
	}
}

static void room_411_init() {
	global[perform_displacements] = true;

	if (global[pid_just_died]) {
		inter_move_object(soul_egg, 411);
		global[pid_just_died] = false;
		local->bird_3_action  = FREEZE;
	}

	global[grapes_have_grown] = GRAPES_NOT_THERE;
	local->prevent            = false;

	ss[fx_fall]   = kernel_load_series(kernel_name('a', 0), false);
	ss[fx_grapes] = kernel_load_series(kernel_name('p', 1), false);
	ss[fx_take]   = kernel_load_series(kernel_name('a', 1), false);
	kernel_load_series(kernel_name('y', 2), false);

	kernel_flip_hotspot(words_magic_grapes, false);

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_2_running = false;
		local->anim_3_running = false;
		local->anim_4_running = false;
	}

	ss[fx_egg] = kernel_load_series(kernel_name('p', 0), false);

	if (object_is_here(soul_egg)) {
		seq[fx_egg] = kernel_seq_stamp(ss[fx_egg], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_egg], 2);
	} else {
		kernel_flip_hotspot(words_soul_egg, false);
	}

	if (object_is_here(black_sphere)) {
		seq[fx_egg] = kernel_seq_stamp(ss[fx_egg], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_egg], 2);
	} else {
		kernel_flip_hotspot(words_black_sphere, false);
	}

	if (local->anim_2_running || global[roc_is_chewing_dates]) {
		local->bird_1_action  = FREEZE;
		local->anim_2_running = true;
		aa[2] = kernel_run_animation(kernel_name('t', 2), 0);
		kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
		kernel_reset_animation(aa[2], 24);
	} else {
		aa[0]                 = kernel_run_animation(kernel_name('b', 1), 0);
		local->anim_0_running = true;
		local->bird_1_action  = FREEZE;
		kernel_reset_animation(aa[0], 21);
	}

	if (previous_room == 410 || previous_room != KERNEL_RESTORING_GAME) {
		player_first_walk(PLAYER_X_FROM_410, PLAYER_Y_FROM_410, FACING_EAST,
		                  WALK_TO_X_FROM_410, WALK_TO_Y_FROM_410, FACING_EAST, true);
	}

	section_4_music();
}

static void room_411_daemon() {
	int temp;

	if (local->anim_0_running) {
		handle_animation_bird_1();
	}

	if (local->anim_1_running) {
		handle_animation_bird_2();
	}

	if (local->anim_2_running) {
		handle_animation_bird_3();
	}

	if (local->anim_3_running) {
		handle_animation_bird_death();
	}

	if (local->anim_4_running) {
		handle_animation_zap();
	}

	if (kernel.trigger == ROOM_411_FALL) {
		temp = seq[fx_fall];
		seq[fx_fall] = kernel_seq_stamp(ss[fx_fall], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_fall], 1);
		kernel_synch(KERNEL_SERIES, seq[fx_fall], KERNEL_SERIES, temp);
	}

	if (!global[grapes_have_grown] && imath_random(1, 100) == 1 && !global[grapes_are_dead]) {
		kernel_flip_hotspot(words_magic_grapes, true);
		global[grapes_have_grown] = GRAPES_GROWING;
		seq[fx_grapes] = kernel_seq_forward(ss[fx_grapes], false, 7, 0, 0, 1);
		kernel_seq_depth(seq[fx_grapes], 2);
		kernel_seq_range(seq[fx_grapes], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_grapes], KERNEL_TRIGGER_EXPIRE, 0, 1);
	}

	if (kernel.trigger == 1) {
		temp = seq[fx_grapes];
		seq[fx_grapes] = kernel_seq_stamp(ss[fx_grapes], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_grapes], 2);
		global[grapes_have_grown] = GRAPES_GROWN;
		kernel_synch(KERNEL_SERIES, seq[fx_grapes], KERNEL_SERIES, temp);
	}
}

static void room_411_pre_parser() {
	if (player_said_2(walk_down, path_to_west)) {
		player.walk_off_edge_to_room = 408;
	}

	if (player_said_1(Roc_s_nest)) {
		if (player.x == BIRD_X && player.y == BIRD_Y) {
			player.need_to_walk = false;
		} else if (player.need_to_walk) {
			player_walk(BIRD_X, BIRD_Y, FACING_NORTHEAST);
		}
	}

	if (player_said_2(take_magic_from, magic_grapes) ||
	    player_said_2(take_magic_from, grape_vine)) {
		player_walk(BIRD_X, BIRD_Y, FACING_NORTHEAST);
	}

	if (player_said_3(throw, Roc, fruit) ||
	    player_said_3(throw, Roc, dates) ||
	    player_said_3(give, Roc, dates) ||
	    player_said_3(give, Roc, fruit) ||
	    player_said_3(put, black_sphere, Roc_s_nest) ||
	    player_said_3(throw, black_sphere, Roc_s_nest)) {

		if (player.x == BIRD_X && player.y == BIRD_Y) {
			player.need_to_walk = false;
		} else {
			player_walk(BIRD_X, BIRD_Y, FACING_NORTHEAST);
		}
	}

	if (global[roc_is_chewing_dates]) {
		if (player.need_to_walk && player_has(soul_egg) && !object_is_here(black_sphere)) {
			local->bird_3_action    = KILL;
			player.commands_allowed = false;
			player_cancel_command();
		}
	}
}

static void room_411_parser() {
	if (player_said_2(take_magic_from, magic_grapes) ||
	    player_said_2(take_magic_from, grape_vine)) {

		if (global[grapes_are_dead]) {
			text_show(41162);
		} else if (global[roc_is_chewing_dates]) {
			text_show(41167);
		} else {
			player.commands_allowed = false;
			player.walker_visible   = false;
			local->anim_4_running   = true;
			aa[4]                   = kernel_run_animation(kernel_name('x', 1), 0);
			global[player_score]   += 3;
			kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, soul_egg)) {
		switch (kernel.trigger) {
		case 0:
			if (!player_has(soul_egg)) {
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take] = kernel_seq_pingpong(ss[fx_take], false, 7, 0, 0, 2);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 12, 1);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_EXPIRE, 0, 2);
				kernel_seq_depth(seq[fx_take], 1);
				kernel_seq_range(seq[fx_take], KERNEL_FIRST, KERNEL_LAST);
				kernel_synch(KERNEL_SERIES, seq[fx_take], KERNEL_PLAYER, 0);
				player.command_ready = false;
				return;
			}
			break;

		case 1:
			if (local->prevent) {
				kernel_seq_delete(seq[fx_egg]);
				kernel_flip_hotspot(words_soul_egg, false);
				++global[player_score];
				sound_play(N_TakeObjectSnd);
				inter_give_to_player(soul_egg);
				if (local->anim_2_running) {
					object_examine(soul_egg, 41151, 0);
				} else {
					object_examine(soul_egg, 41161, 0);
				}
			}
			local->prevent = true;
			player.command_ready = false;
			return;

		case 2:
			player.walker_visible = true;
			local->prevent        = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take]);

			if (!local->anim_2_running) {
				local->bird_1_action = WAIT_FOR_KILL;
			} else {
				player.commands_allowed = true;
			}

			player.command_ready = false;
			return;
		}
	}

	if (player_said_3(put, black_sphere, Roc_s_nest) ||
	    player_said_3(throw, black_sphere, Roc_s_nest)) {

		if (!player_has(soul_egg)) {
			text_show(41138);
			player.command_ready = false;
			return;
		} else switch (kernel.trigger) {
		case 0:
			if (player_has(black_sphere)) {
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take] = kernel_seq_pingpong(ss[fx_take], false, 7, 0, 0, 2);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 12, 1);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_EXPIRE, 0, 2);
				kernel_seq_depth(seq[fx_take], 1);
				kernel_seq_range(seq[fx_take], KERNEL_FIRST, KERNEL_LAST);
				kernel_synch(KERNEL_SERIES, seq[fx_take], KERNEL_PLAYER, 0);
				player.command_ready = false;
				return;
			}
			break;

		case 1:
			if (local->prevent) {
				ss[fx_egg]  = kernel_load_series(kernel_name('p', 0), false);
				seq[fx_egg] = kernel_seq_stamp(ss[fx_egg], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_egg], 2);
				kernel_flip_hotspot(words_black_sphere, true);

				global[player_score] += 3;
				inter_move_object(black_sphere, 411);
			}
			local->prevent = true;
			player.command_ready = false;
			return;

		case 2:
			text_show(41147);
			player.walker_visible   = true;
			player.commands_allowed = true;
			local->prevent          = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take]);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_3(throw, Roc, fruit) ||
	    player_said_3(give, Roc, fruit)) {

		if (global[roc_is_chewing_dates]) {
			text_show(41160);
		} else if (!global[grapes_are_dead]) {
			text_show(41135);
		} else {
			local->bird_1_action    = THROW;
			player.commands_allowed = false;

			if (local->bird_1_frame != 20) {
				kernel_timing_trigger(1, 2);
			} else {
				kernel_abort_animation(aa[0]);
				local->bird_1_action  = FREEZE;
				player.walker_visible = false;
				local->anim_1_running = true;
				local->anim_0_running = false;
				aa[1] = kernel_run_animation(kernel_name('t', 1), 0);
				kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
			}
		}
		player.command_ready = false;
		return;
	}

	if (player_said_3(throw, Roc, dates) ||
	    player_said_3(give, Roc, dates)) {

		if (global[roc_is_chewing_dates]) {
			text_show(41160);
		} else if (!global[grapes_are_dead]) {
			text_show(41135);
		} else {
			local->bird_1_action    = THROW;
			player.commands_allowed = false;

			if (local->bird_1_frame != 20) {
				kernel_timing_trigger(1, 2);
			} else {
				global[roc_is_chewing_dates] = true;
				global[player_score] += 3;
				kernel_abort_animation(aa[0]);
				local->bird_1_action  = FREEZE;
				local->bird_3_action  = FREEZE;
				player.walker_visible = false;
				local->anim_2_running = true;
				local->anim_0_running = false;
				aa[2] = kernel_run_animation(kernel_name('t', 2), 0);
				kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
			}
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(invoke_power_of, crystal_ball)) {
		if (!global[roc_is_chewing_dates]) switch (kernel.trigger) {
		case 0:
			sound_play(N_InvokeCrystalBall);
			text_show(970);
			player.commands_allowed = false;
			kernel_timing_trigger(FOUR_SECONDS, 1);
			global[player_score] += 3;
			kernel_abort_animation(aa[0]);
			local->bird_1_action  = FREEZE;
			local->bird_3_action  = FREEZE;
			local->anim_2_running = true;
			local->anim_0_running = false;
			aa[2] = kernel_run_animation(kernel_name('t', 2), 0);
			kernel_reset_animation(aa[2], 38);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
			player.command_ready = false;
			return;

		case 1:
			global[roc_is_chewing_dates] = true;
			global[crystal_ball_dead]    = true;
			player.commands_allowed      = true;
			inter_move_object(crystal_ball, NOWHERE);
			text_show(41149);
			player.command_ready = false;
			return;
		}
	}

	if (player.look_around) {
		if (object_is_here(soul_egg)) {
			text_show(41101);
		} else if (object_is_here(black_sphere)) {
			text_show(41102);
		} else {
			text_show(41121);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(spirit_tree)) {
			text_show(41103);
			player.command_ready = false;
			return;
		}

		if (player_said_1(grape_vine)) {
			text_show(41105);
			player.command_ready = false;
			return;
		}

		if (player_said_1(black_sphere) && object_is_here(black_sphere)) {
			text_show(41155);
			player.command_ready = false;
			return;
		}

		if (player_said_1(magic_grapes)) {
			if (global[grapes_are_dead]) {
				text_show(41158);
			} else if (game.difficulty == HARD_MODE) {
				text_show(41110);
			} else {
				text_show(41111);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(sand)) {
			text_show(41118);
			player.command_ready = false;
			return;
		}

		if (player_said_1(path_to_west)) {
			text_show(41119);
			player.command_ready = false;
			return;
		}

		if (player_said_1(Roc_s_nest)) {
			if (object_is_here(soul_egg)) {
				text_show(41120);
			} else if (object_is_here(black_sphere)) {
				text_show(41102);
			} else {
				text_show(41121);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(spirit_plane)) {
			text_show(41125);
			player.command_ready = false;
			return;
		}

		if (player_said_1(Roc)) {
			if (local->anim_2_running) {
				if (player_has(dates)) {
					text_show(41157);
				} else if (player_has(soul_egg)) {
					text_show(41159);
				} else {
					text_show(41128);
				}
			} else if (global[grapes_are_dead]) {
				text_show(41127);
			} else {
				text_show(41126);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(soul_egg) && object_is_here(soul_egg)) {
			text_show(41144);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(talk_to, spirit_tree)) {
		text_show(41104);
		player.command_ready = false;
		return;
	}

	if (player_said_2(sword, attack) ||
	    player_said_2(sword, carve_up) ||
	    player_said_2(sword, thrust) ||
	    player_said_2(pour, flask_full_of_acid)) {

		if (player_said_1(grape_vine) ||
		    player_said_1(spirit_tree)) {
			text_show(61407);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(sword, attack) ||
	    player_said_2(sword, carve_up) ||
	    player_said_2(sword, thrust) ||
	    player_said_1(pull) ||
	    player_said_2(pour, flask_full_of_acid)) {

		if (player_said_1(Roc_s_nest)) {
			text_show(41122);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(heal, spirit_tree)) {
		text_show(41108);
		player.command_ready = false;
		return;
	}

	if (player_said_2(open, Roc_s_nest)) {
		if (object_is_here(soul_egg)) {
			text_show(41123);
		} else {
			text_show(41124);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, magic_grapes) ||
	    player_said_2(pull, magic_grapes) ||
	    player_said_2(heal, magic_grapes)) {
		text_show(41112);
		player.command_ready = false;
		return;
	}

	if (player_said_1(take_magic_from) || player_said_1(put_magic_into)) {
		if (player_said_1(spirit_tree) ||
		    player_said_1(grape_vine)) {
			text_show(41108);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(take, grape_vine) || player_said_2(pull, grape_vine)) {
		if (global[grapes_are_dead]) {
			text_show(41117);
		} else {
			text_show(41106);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(throw, magic_grapes)) {
		if (player_said_1(mud)) {
			text_show(41115);
		} else if (player_said_1(soporific)) {
			text_show(41116);
		} else {
			text_show(41113);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(sword, attack) ||
	    player_said_2(sword, carve_up) ||
	    player_said_2(sword, thrust)) {

		if (player_said_1(magic_grapes)) {
			text_show(41114);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(sword, attack) ||
	    player_said_2(sword, carve_up) ||
	    player_said_2(sword, thrust) ||
	    player_said_1(push) ||
	    player_said_1(pull)) {

		if (player_said_1(Roc)) {
			text_show(41129);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(talk_to, Roc)) {
		text_show(41130);
		player.command_ready = false;
		return;
	}

	if (player_said_2(throw, Roc) ||
	    player_said_2(give, Roc)) {

		if (player_said_1(bone) || player_said_1(tentacle_parts)) {
			text_show(41132);
		} else if (player_said_1(soporific)) {
			text_show(41141);
		} else if (player_said_1(dead_rat) || player_said_1(ratsicle)) {
			if (global[grapes_are_dead]) {
				text_show(41154);
			} else {
				text_show(41135);
			}
		} else if (player_said_1(bird_figurine)) {
			if (object_is_here(soul_egg)) {
				text_show(41139);
			} else {
				text_show(41140);
			}
		} else {
			if (object_is_here(soul_egg)) {
				text_show(41139);
			} else {
				text_show(41140);
			}
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(make_noise, birdcall) ||
	    player_said_2(rub, bird_figurine) ||
	    player_said_2(rub, birdcall)) {
		text_show(41136);
		player.command_ready = false;
		return;
	}

	if (player_said_2(open, flies)) {
		if (global[grapes_are_dead]) {
			text_show(41137);
		} else {
			text_show(41135);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_1(shift_into_bear) || player_said_1(shift_into_seal) ||
	    player_said_1(shift_into_snake)) {
		text_show(41142);
		player.command_ready = false;
		return;
	}

	if (player_said_2(speak_words_on, parchment)) {
		text_show(41143);
		player.command_ready = false;
		return;
	}

	if (player_said_2(gaze_into, crystal_ball)) {
		if (!global[roc_is_chewing_dates]) {
			text_show(41148);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(put_magic_into, magic_grapes) ||
	    player_said_2(put_magic_into, grape_vine)) {
		if (global[grapes_are_dead]) {
			text_show(41163);
		} else {
			text_show(41164);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, black_sphere) && !player_has(black_sphere)) {
		text_show(41166);
		player.command_ready = false;
		return;
	}
}

void room_411_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.bird_1_frame);
	s.syncAsSint16LE(scratch.bird_1_action);
	s.syncAsSint16LE(scratch.bird_1_talk_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.bird_2_frame);
	s.syncAsSint16LE(scratch.bird_2_action);
	s.syncAsSint16LE(scratch.bird_2_talk_count);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.bird_3_frame);
	s.syncAsSint16LE(scratch.bird_3_action);
	s.syncAsSint16LE(scratch.bird_3_talk_count);
	s.syncAsSint16LE(scratch.anim_2_running);
	s.syncAsSint16LE(scratch.bird_death_frame);
	s.syncAsSint16LE(scratch.bird_death_action);
	s.syncAsSint16LE(scratch.bird_death_talk_count);
	s.syncAsSint16LE(scratch.anim_3_running);
	s.syncAsSint16LE(scratch.zap_frame);
	s.syncAsSint16LE(scratch.anim_4_running);
	s.syncAsSint16LE(scratch.prevent);
}

void room_411_preload() {
	room_init_code_pointer       = room_411_init;
	room_pre_parser_code_pointer = room_411_pre_parser;
	room_parser_code_pointer     = room_411_parser;
	room_daemon_code_pointer     = room_411_daemon;

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
