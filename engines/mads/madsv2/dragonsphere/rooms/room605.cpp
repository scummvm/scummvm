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
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section6.h"
#include "mads/madsv2/dragonsphere/rooms/room605.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[23];
	int16 sequence[23];
	int16 animation[4];

	int16 rope_frame;
	int16 rope_action;
	int16 rope_talk_count;
	int16 anim_0_running;

	int16 rat_frame;
	int16 anim_1_running;

	int16 pid_frame;
	int16 pid_action;
	int16 pid_talk_count;
	int16 anim_2_running;

	int16 rat_cage_frame;
	int16 rat_cage_talk_count;
	int16 anim_3_running;

	int16 rope_id;
	int16 prevent;
	int16 prevent_2;
};

#define local ((Scratch *)(&game.scratch[0]))
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_fire_l               0
#define fx_fire_r               1
#define fx_flask_l              2
#define fx_flask_r              3
#define fx_rope                 4
#define fx_take                 5
#define fx_dead_rat             6
#define fx_lid_open             7
#define fx_lid_closed           8
#define fx_take_rat             9
#define fx_flask_empty          10
#define fx_flask_full           11
#define fx_flask_end            12
#define fx_take_flask           13
#define fx_turn_petcock         14
#define fx_fluid_left           15
#define fx_fluid_right          16
#define fx_drip                 17
#define fx_end_flow_left        18
#define fx_end_flow_right       19
#define fx_object_in_freezer    20
#define fx_reach_into_freezer   21

#define ROOM_605_FLUID_LEFT     60
#define ROOM_605_FLUID_RIGHT    62
#define ROOM_605_CLOSE_PETCOCKS 65
#define ROOM_605_PUT_IN_FREEZER 75
#define ROOM_605_TAKE_CONTENTS  80
#define ROOM_605_DRIP_SOUND     85
#define ROOM_605_RUN_ANIM       95
#define ROOM_605_NEW_ROOM       98

#define PLAYER_X_FROM_604       -15
#define PLAYER_Y_FROM_604       138
#define WALK_TO_X_FROM_604      16
#define WALK_TO_Y_FROM_604      138

#define ROPE_FREEZE             0
#define ROPE_STRIKE             1
#define ROPE_ZAP                2

#define PID_FREEZE              0
#define PID_ZAP                 1
#define PID_LEAVE               2

#define ROPE_X                  39
#define ROPE_Y                  130

#define FLASK_END_X             86
#define FLASK_END_Y             100

#define FLASK_EMPTY_X           123
#define FLASK_EMPTY_Y           100

#define TAKE_STUFF_X            239
#define TAKE_STUFF_Y            129

#define OBJECTS_MAX             47

static void handle_animation_rope() {
	int rope_reset_frame;

	if (kernel_anim[aa[0]].frame != local->rope_frame) {
		local->rope_frame = kernel_anim[aa[0]].frame;
		rope_reset_frame = -1;

		switch (local->rope_frame) {
		case 17:
			player.commands_allowed = true;
			break;

		case 18:
		case 19:
		case 20:
		case 21:
			switch (local->rope_action) {
			case ROPE_FREEZE:
				rope_reset_frame = imath_random(18, 20);
				break;

			case ROPE_STRIKE:
				player.walker_visible = false;
				rope_reset_frame      = 21;
				kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
				break;

			case ROPE_ZAP:
				player.walker_visible = false;
				rope_reset_frame      = 52;
				kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
				break;
			}
			break;

		case 52:
			if (game.difficulty == EASY_MODE) {
				text_show(60581);
			} else {
				text_show(45);
			}
			kernel.force_restart = true;
			break;

		case 97:
			rope_reset_frame = 96;
			break;
		}

		if (rope_reset_frame >= 0) {
			kernel_reset_animation(aa[0], rope_reset_frame);
			local->rope_frame = rope_reset_frame;
		}
	}
}

static void handle_animation_rat() {
	int rat_reset_frame;

	if (kernel_anim[aa[1]].frame != local->rat_frame) {
		local->rat_frame = kernel_anim[aa[1]].frame;
		rat_reset_frame = -1;

		switch (local->rat_frame) {

		case 1:
			kernel_seq_delete(seq[fx_lid_closed]);
			break;

		case 28:
			player.walker_visible   = true;
			player.commands_allowed = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
			break;

		case 50:
			local->anim_1_running = false;
			rat_reset_frame       = -1;
			seq[fx_lid_open]      = kernel_seq_stamp(ss[fx_lid_open], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_lid_open], 2);
			text_show(60569);
			kernel_flip_hotspot(words_dead_rat, true);
			break;
		}

		if (rat_reset_frame >= 0) {
			kernel_reset_animation(aa[1], rat_reset_frame);
			local->rat_frame = rat_reset_frame;
		}
	}
}

static void handle_animation_pid() {
	int pid_reset_frame;
	int tmp;

	if (kernel_anim[aa[2]].frame != local->pid_frame) {
		local->pid_frame = kernel_anim[aa[2]].frame;
		pid_reset_frame = -1;

		switch (local->pid_frame) {
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			switch (local->pid_action) {
			case PID_FREEZE:
				++local->pid_talk_count;
				if (local->pid_talk_count > 10) {
					tmp                   = imath_random(1, 3);
					local->pid_talk_count = 0;

					if (tmp == 1) {
						pid_reset_frame = local->pid_frame - 2;
					} else if (tmp == 2) {
						pid_reset_frame = local->pid_frame - 1;
					}

					if ((local->pid_frame == 5) && (tmp == 1)) {
						pid_reset_frame = 4;
					}

					if ((local->pid_frame == 10) && (tmp == 3)) {
						pid_reset_frame = 9;
					}

				} else {
					pid_reset_frame = local->pid_frame - 1;
				}
				break;

			case PID_ZAP:
				pid_reset_frame = 10;
				break;

			case PID_LEAVE:
				pid_reset_frame = 42;
				break;
			}
			break;

		case 17:
			kernel_reset_animation(aa[0], 96);
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_ANIM, aa[2]);
			break;

		case 42:
		case 48:
			kernel_abort_animation(aa[2]);
			local->anim_2_running = false;
			pid_reset_frame       = -1;

			if (player_said_2(take_magic_from, rope)) {
				player.walker_visible = true;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				sound_play(N_Bk605Music);
				kernel_abort_animation(aa[0]);
				player.walker_visible   = true;
				player.commands_allowed = true;
				global[rope_is_alive]   = false;
				local->anim_0_running   = false;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				kernel_flip_hotspot(words_rope, true);
				global[player_score] += 2;

			} else if (player_said_2(walk_through, door_to_magic_room) || player_said_2(open, door_to_magic_room) ||
			           player_said_2(pull, door_to_magic_room)) {
				player.walker_visible = true;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				player_first_walk(player.x, player.y, FACING_EAST,
				                  PLAYER_X_FROM_604, PLAYER_Y_FROM_604, FACING_WEST, false);
				player_walk_trigger(ROOM_605_NEW_ROOM);

			} else {
				kernel_reset_animation(aa[0], 22);
				kernel_synch(KERNEL_ANIM, aa[0], KERNEL_NOW, 0);
			}
			break;
		}

		if (pid_reset_frame >= 0) {
			kernel_reset_animation(aa[2], pid_reset_frame);
			local->pid_frame = pid_reset_frame;
		}
	}
}

static void handle_animation_rat_cage() {
	int rat_cage_reset_frame;

	if (kernel_anim[aa[3]].frame != local->rat_cage_frame) {
		local->rat_cage_frame = kernel_anim[aa[3]].frame;
		rat_cage_reset_frame = -1;

		switch (local->rat_cage_frame) {
		case 1:
		case 2:
		case 3:
		case 4:
			++local->rat_cage_talk_count;
			if (local->rat_cage_talk_count > imath_random(2, 5)) {
				rat_cage_reset_frame       = imath_random(0, 3);
				local->rat_cage_talk_count = 0;
			} else {
				rat_cage_reset_frame = local->rat_cage_frame - 1;
			}
			break;
		}

		if (rat_cage_reset_frame >= 0) {
			kernel_reset_animation(aa[3], rat_cage_reset_frame);
			local->rat_cage_frame = rat_cage_reset_frame;
		}
	}
}

static void room_605_init() {
	int count;
	int shit_here = false;

	kernel_flip_hotspot(words_flask, false);
	kernel_flip_hotspot(words_flask_full_of_acid, false);

	local->prevent        = false;
	local->prevent_2      = false;
	local->pid_talk_count = 0;

	if (kernel.teleported_in) {
		inter_give_to_player(power_vacuum_stone);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_2_running = false;
		local->anim_3_running = false;
	}

	ss[fx_flask_l] = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_flask_r] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_fire_l]  = kernel_load_series(kernel_name('x', 4), false);
	ss[fx_fire_r]  = kernel_load_series(kernel_name('x', 5), false);

	ss[fx_flask_empty] = kernel_load_series(kernel_name('y', 3), false);
	ss[fx_flask_full]  = kernel_load_series(kernel_name('y', 4), false);

	ss[fx_turn_petcock]   = kernel_load_series(kernel_name('a', 1), false);
	ss[fx_fluid_left]     = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_fluid_right]    = kernel_load_series(kernel_name('x', 3), false);
	ss[fx_drip]           = kernel_load_series(kernel_name('x', 6), false);

	ss[fx_end_flow_right] = kernel_load_series(kernel_name('y', 6), false);
	ss[fx_end_flow_left]  = kernel_load_series(kernel_name('y', 7), false);

	ss[fx_object_in_freezer]  = kernel_load_series(kernel_name('y', 5), false);
	ss[fx_reach_into_freezer] = kernel_load_series(kernel_name('a', 0), false);

	for (count = 10; count < 33; count++) {
		flag_used[count] = false;
	}

	if (global[object_is_in_freezer_605] == HAS_USED_FREEZER) {
		seq[fx_object_in_freezer] = kernel_seq_stamp(ss[fx_object_in_freezer], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_object_in_freezer], 14);
	}

	if (object_is_here(flask)) {
		ss[fx_flask_end]  = kernel_load_series(kernel_name('p', 9), false);
		seq[fx_flask_end] = kernel_seq_stamp(ss[fx_flask_end], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_flask_end], 5);
		kernel_flip_hotspot_loc(words_flask, true, FLASK_END_X, FLASK_END_Y);
	}

	if (global[flask_on_plate] == YES_EMPTY) {
		kernel_flip_hotspot_loc(words_flask, true, FLASK_EMPTY_X, FLASK_EMPTY_Y);
		seq[fx_flask_empty] = kernel_seq_stamp(ss[fx_flask_empty], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_flask_empty], 5);

	} else if (global[flask_on_plate] == YES_ACID) {
		kernel_flip_hotspot(words_flask_full_of_acid, true);
		seq[fx_flask_full] = kernel_seq_stamp(ss[fx_flask_full], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_flask_full], 5);
	}

	ss[fx_lid_open] = kernel_load_series(kernel_name('y', 0), false);

	if (global[rat_cage_is_open]) {
		seq[fx_lid_open] = kernel_seq_stamp(ss[fx_lid_open], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_lid_open], 2);
	} else {
		ss[fx_lid_closed]  = kernel_load_series(kernel_name('y', 2), false);
		seq[fx_lid_closed] = kernel_seq_stamp(ss[fx_lid_closed], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_lid_closed], 5);
	}

	if (object_is_here(dead_rat)) {
		ss[fx_take_rat]  = kernel_load_series(kernel_name('a', 5), false);
		ss[fx_dead_rat]  = kernel_load_series(kernel_name('y', 1), false);
		seq[fx_dead_rat] = kernel_seq_stamp(ss[fx_dead_rat], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_dead_rat], 2);
	} else {
		kernel_flip_hotspot(words_dead_rat, false);
	}

	if (!global[rat_cage_is_open]) {
		kernel_flip_hotspot(words_dead_rat, false);
	}

	ss[fx_take_flask] = kernel_load_series("*KGDRM_9", false);
	ss[fx_rope]       = kernel_load_series(kernel_name('x', 9), false);

	if (global[fluid_is_dripping]) {
		seq[fx_fluid_left] = kernel_seq_pingpong(ss[fx_fluid_left], false, 7, 0, 0, 0);
		kernel_seq_depth(seq[fx_fluid_left], 5);
		kernel_seq_range(seq[fx_fluid_left], 16, KERNEL_LAST);

		seq[fx_fluid_right] = kernel_seq_pingpong(ss[fx_fluid_right], false, 7, 0, 0, 0);
		kernel_seq_depth(seq[fx_fluid_right], 5);
		kernel_seq_range(seq[fx_fluid_right], 16, KERNEL_LAST);
	}

	if (!global[rat_cage_is_open]) {
		aa[3]                 = kernel_run_animation(kernel_name('r', 3), 0);
		local->anim_3_running = true;
	}

	if (global[rope_is_alive]) {
		aa[0]                   = kernel_run_animation(kernel_name('r', 1), 0);
		local->anim_0_running   = true;
		local->rope_action      = ROPE_FREEZE;
		player.commands_allowed = false;
		local->rope_id = kernel_add_dynamic(words_rope, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                                    0, 0, 0, 0);
		kernel_dynamic_hot[local->rope_id].prep = PREP_ON;
		kernel_dynamic_anim(local->rope_id, aa[0], 0);
		kernel_dynamic_anim(local->rope_id, aa[0], 1);

		if (previous_room == KERNEL_RESTORING_GAME) {
			aa[2]                   = kernel_run_animation(kernel_name('p', 1), 0);
			local->anim_2_running   = true;
			local->pid_action       = PID_FREEZE;
			player.commands_allowed = true;
			player.walker_visible   = false;
			kernel_reset_animation(aa[2], 5);
			kernel_reset_animation(aa[0], 19);

		} else {
			player_first_walk(PLAYER_X_FROM_604, PLAYER_Y_FROM_604, FACING_EAST,
			                  ROPE_X, ROPE_Y, FACING_EAST, false);
			player_walk_trigger(ROOM_605_RUN_ANIM);
		}

	} else if (previous_room == 604 || previous_room != KERNEL_RESTORING_GAME) {
		player_first_walk(PLAYER_X_FROM_604, PLAYER_Y_FROM_604, FACING_EAST,
		                  WALK_TO_X_FROM_604, WALK_TO_Y_FROM_604, FACING_EAST, true);
	}

	if (object_is_here(rope)) {
		seq[fx_rope] = kernel_seq_stamp(ss[fx_rope], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_rope], 14);

	} else {
		kernel_flip_hotspot(words_rope, false);
	}

	seq[fx_fire_l] = kernel_seq_forward(ss[fx_fire_l], false, 8, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_l], 5);
	kernel_seq_range(seq[fx_fire_l], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_r] = kernel_seq_forward(ss[fx_fire_r], false, 8, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_r], 5);
	kernel_seq_range(seq[fx_fire_r], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_flask_l] = kernel_seq_forward(ss[fx_flask_l], false, 7, 0, 0, 0);
	kernel_seq_depth(seq[fx_flask_l], 5);
	kernel_seq_range(seq[fx_flask_l], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_flask_r] = kernel_seq_forward(ss[fx_flask_r], false, 7, 0, 0, 0);
	kernel_seq_depth(seq[fx_flask_r], 5);
	kernel_seq_range(seq[fx_flask_r], KERNEL_FIRST, KERNEL_LAST);

	for (count = 0; count < OBJECTS_MAX; count++) {
		if (object[count].location == FREEZER) {
			shit_here = true;
		}
	}

	if (!shit_here) {
		kernel_flip_hotspot(words_contents_of_freezer, false);
	}

	section_6_music();
}

static void room_605_daemon() {
	if (local->anim_0_running) {
		handle_animation_rope();
	}

	if (local->anim_1_running) {
		handle_animation_rat();
	}

	if (local->anim_2_running) {
		handle_animation_pid();
	}

	if (local->anim_3_running) {
		handle_animation_rat_cage();
	}

	if (global[fluid_is_dripping] && !local->prevent_2) {
		seq[fx_drip] = kernel_seq_forward(ss[fx_drip], false, 6, 0, 0, 0);
		kernel_seq_depth(seq[fx_drip], 5);
		kernel_seq_range(seq[fx_drip], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_drip], KERNEL_TRIGGER_SPRITE, 8, ROOM_605_DRIP_SOUND);
		local->prevent_2 = true;
	}

	if (kernel.trigger == ROOM_605_DRIP_SOUND) {
		sound_play(N_AcidDrips);
	}

	if (kernel.trigger == ROOM_605_NEW_ROOM) {
		new_room = 604;
	}

	if (kernel.trigger == ROOM_605_RUN_ANIM) {
		player.walker_visible   = false;
		aa[2]                   = kernel_run_animation(kernel_name('p', 1), 0);
		local->anim_2_running   = true;
		local->pid_action       = PID_FREEZE;
		kernel_synch(KERNEL_ANIM, aa[2], KERNEL_PLAYER, 0);
	}
}

static void room_605_pre_parser() {
	int count;
	int stuff_still_here = false;

	if (player_said_1(door_to_magic_room) && player.need_to_walk) {
		if (!player_said_1(walk_through) && !player_said_1(open) &&
		    !player_said_1(pull)) {
			player.need_to_walk = false;
		}
	}

	if (player_said_2(walk_through, door_to_magic_room) || player_said_2(open, door_to_magic_room) ||
	    player_said_2(pull, door_to_magic_room)) {
		for (count = 0; count < OBJECTS_MAX; count++) {
			if (object[count].location == FREEZER) {
				stuff_still_here = true;
			}
		}

		if (stuff_still_here) {
			text_show(60579);
			player_cancel_command();

		} else {
			player.walk_off_edge_to_room = 604;
		}
	}

	if (player_said_2(take, contents_of_freezer)) {
		player_walk(TAKE_STUFF_X, TAKE_STUFF_Y, FACING_NORTHEAST);
	}

	if (local->anim_2_running) {

		if (player_said_2(take_magic_from, rope) && global[rope_is_alive]) {
			local->pid_action       = PID_ZAP;
			player.commands_allowed = false;
			player.need_to_walk     = false;
			player_cancel_command();

		} else if (player.need_to_walk) {
			if (player_said_1(wave) ||
			    player_said_3(give, torch, rope) ||
			    player_said_3(put, torch, rope) ||
			    player_said_3(throw, torch, rope)) {
				text_show(60568);

			} else {
				local->pid_action       = PID_LEAVE;
				player.commands_allowed = false;
			}
			player.need_to_walk = false;
			player_cancel_command();
		}
	}
}

static void room_605_parser() {
	int id;
	int count;
	int removed = 0;
	int last_removed = 0;
	int was_rat_removed;
	int are_any_inorganic;
	int are_any_organic;
	int temp;
	int stuff_still_here = false;

	if (player_said_1(wave) ||
	    player_said_3(give, torch, rope) ||
	    player_said_3(put, torch, rope) ||
	    player_said_3(throw, torch, rope)) {
		text_show(60568);
		player.command_ready = false;
		return;
	}

	if (player_said_2(invoke, signet_ring)) {
		for (count = 0; count < OBJECTS_MAX; count++) {
			if (object[count].location == FREEZER) {
				stuff_still_here = true;
			}
		}

		if (stuff_still_here) {
			text_show(60579);
			player.command_ready = false;
			return;
		}
	}

	switch (kernel.trigger) {
	case ROOM_605_CLOSE_PETCOCKS:
		player.commands_allowed = false;
		player.walker_visible   = false;
		seq[fx_turn_petcock]    = kernel_seq_pingpong(ss[fx_turn_petcock], false, 7, 0, 0, 2);
		kernel_seq_trigger(seq[fx_turn_petcock], KERNEL_TRIGGER_SPRITE, 6, ROOM_605_CLOSE_PETCOCKS + 2);
		kernel_seq_trigger(seq[fx_turn_petcock], KERNEL_TRIGGER_EXPIRE, 0, ROOM_605_CLOSE_PETCOCKS + 3);
		kernel_seq_player(seq[fx_turn_petcock], true);
		kernel_seq_depth(seq[fx_turn_petcock], 1);
		kernel_seq_range(seq[fx_turn_petcock], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_scale(seq[fx_turn_petcock], 100);
		player.command_ready = false;
		return;

	case ROOM_605_CLOSE_PETCOCKS + 1:
		if (local->prevent) {
			sound_play(N_TurnPetcock);
			kernel_seq_delete(seq[fx_fluid_left]);
			seq[fx_end_flow_left] = kernel_seq_forward(ss[fx_end_flow_left], false, 8, 0, 0, 1);
			kernel_seq_depth(seq[fx_end_flow_left], 5);
			kernel_seq_range(seq[fx_end_flow_left], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_end_flow_left], KERNEL_TRIGGER_EXPIRE, 0, ROOM_605_CLOSE_PETCOCKS + 6);
		}
		local->prevent = true;
		player.command_ready = false;
		return;

	case ROOM_605_CLOSE_PETCOCKS + 2:
		if (local->prevent) {
			sound_play(N_TurnPetcock);
			if (global[fluid_is_dripping]) {
				kernel_seq_delete(seq[fx_drip]);
				global[fluid_is_dripping] = false;
				local->prevent_2 = false;
			}
			kernel_seq_delete(seq[fx_fluid_right]);
			seq[fx_end_flow_right] = kernel_seq_forward(ss[fx_end_flow_right], false, 8, 0, 0, 1);
			kernel_seq_depth(seq[fx_end_flow_right], 5);
			kernel_seq_range(seq[fx_end_flow_right], KERNEL_FIRST, KERNEL_LAST);
		}
		local->prevent = true;
		player.command_ready = false;
		return;

	case ROOM_605_CLOSE_PETCOCKS + 3:
		kernel_timing_trigger(TENTH_SECOND, ROOM_605_CLOSE_PETCOCKS + 4);
		player.walker_visible = true;
		local->prevent        = false;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_turn_petcock]);
		player.command_ready = false;
		return;

	case ROOM_605_CLOSE_PETCOCKS + 4:
		player.walker_visible = false;
		seq[fx_turn_petcock]  = kernel_seq_pingpong(ss[fx_turn_petcock], true, 7, 0, 0, 2);
		kernel_seq_trigger(seq[fx_turn_petcock], KERNEL_TRIGGER_SPRITE, 6, ROOM_605_CLOSE_PETCOCKS + 1);
		kernel_seq_trigger(seq[fx_turn_petcock], KERNEL_TRIGGER_EXPIRE, 0, ROOM_605_CLOSE_PETCOCKS + 5);
		kernel_seq_player(seq[fx_turn_petcock], true);
		kernel_seq_depth(seq[fx_turn_petcock], 1);
		kernel_seq_range(seq[fx_turn_petcock], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_scale(seq[fx_turn_petcock], 100);
		player.command_ready = false;
		return;

	case ROOM_605_CLOSE_PETCOCKS + 5:
		player.walker_visible = true;
		local->prevent        = false;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_turn_petcock]);
		player.command_ready = false;
		return;

	case ROOM_605_CLOSE_PETCOCKS + 6:
		text_show(60515);
		player.commands_allowed = true;
		player.command_ready = false;
		return;
	}

	if (kernel.trigger == ROOM_605_FLUID_LEFT) {
		temp = seq[fx_fluid_left];
		seq[fx_fluid_left] = kernel_seq_pingpong(ss[fx_fluid_left], false, 7, 0, 0, 0);
		kernel_seq_depth(seq[fx_fluid_left], 5);
		kernel_seq_range(seq[fx_fluid_left], 16, KERNEL_LAST);
		player.command_ready = false;
		return;
	}

	if (kernel.trigger == ROOM_605_FLUID_RIGHT) {
		temp = seq[fx_fluid_right];
		seq[fx_fluid_right] = kernel_seq_pingpong(ss[fx_fluid_right], false, 7, 0, 0, 0);
		kernel_seq_depth(seq[fx_fluid_right], 5);
		kernel_seq_range(seq[fx_fluid_right], 16, KERNEL_LAST);

		if (!global[flask_on_plate]) {
			player.commands_allowed = true;
		}

		text_show(60514);
		player.command_ready = false;
		return;
	}

	if (player_said_2(open, rat_cage)) {
		if (!global[rat_cage_is_open]) {
			kernel_abort_animation(aa[3]);
			local->anim_3_running    = false;
			aa[1]                    = kernel_run_animation(kernel_name('r', 2), 0);
			local->anim_1_running    = true;
			player.walker_visible    = false;
			player.commands_allowed  = false;
			global[rat_cage_is_open] = true;
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_PLAYER, 0);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, aa[1]);

			id = kernel_add_dynamic(words_rat, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
			                        0, 0, 0, 0);
			kernel_dynamic_hot[id].prep = PREP_ON;
			kernel_dynamic_anim(id, aa[1], 1);

		} else {
			text_show(60548);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(close, rat_cage)) {
		if (global[rat_cage_is_open]) {
			text_show(60577);
		} else {
			text_show(60548);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, rope)) {
		switch (kernel.trigger) {
		case 0:
			if (!player_has(rope)) {
				player.commands_allowed = false;
				player.walker_visible   = false;
				ss[fx_take]  = kernel_load_series("*KGDRD_7", false);
				seq[fx_take] = kernel_seq_forward(ss[fx_take], false, 7, 0, 0, 1);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 12, 1);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 13, 2);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_EXPIRE, 0, 3);
				kernel_seq_range(seq[fx_take], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_depth(seq[fx_take], 2);
				kernel_seq_player(seq[fx_take], true);
				player.command_ready = false;
				return;
			}
			break;

		case 1:
			kernel_seq_delete(seq[fx_rope]);
			player.command_ready = false;
			return;

		case 2:
			kernel_load_variant(0);
			kernel_flip_hotspot(words_rope, false);
			++global[player_score];
			sound_play(N_TakeObjectSnd);
			inter_give_to_player(rope);
			object_examine(rope, 60505, 0);
			player.command_ready = false;
			return;

		case 3:
			player.walker_visible   = true;
			player.commands_allowed = true;
			local->prevent          = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take]);
			matte_deallocate_series(ss[fx_take], true);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(take, dead_rat)) {
		switch (kernel.trigger) {
		case 0:
			if (!player_has(dead_rat)) {
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take_rat] = kernel_seq_pingpong(ss[fx_take_rat], false, 7, 0, 0, 2);
				kernel_seq_trigger(seq[fx_take_rat], KERNEL_TRIGGER_SPRITE, 3, 2);
				kernel_seq_trigger(seq[fx_take_rat], KERNEL_TRIGGER_EXPIRE, 0, 3);
				kernel_seq_depth(seq[fx_take_rat], 1);
				kernel_seq_range(seq[fx_take_rat], KERNEL_FIRST, KERNEL_LAST);
				player.command_ready = false;
				return;
			}
			break;

		case 2:
			if (local->prevent) {
				kernel_seq_delete(seq[fx_dead_rat]);
				kernel_flip_hotspot(words_dead_rat, false);
				++global[player_score];
				sound_play(N_TakeObjectSnd);
				inter_give_to_player(dead_rat);
				object_examine(dead_rat, 60507, 0);
			}
			local->prevent = true;
			player.command_ready = false;
			return;

		case 3:
			player.walker_visible   = true;
			player.commands_allowed = true;
			local->prevent          = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_rat]);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(take, flask) ||
	    player_said_2(take, flask_full_of_acid)) {
		switch (kernel.trigger) {
		case 0:
			if ((!player_has(flask_full_of_acid)) || (!player_has(flask))) {
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take_flask]      = kernel_seq_pingpong(ss[fx_take_flask], false, 7, 0, 0, 2);
				kernel_seq_trigger(seq[fx_take_flask], KERNEL_TRIGGER_SPRITE, 6, 2);
				kernel_seq_trigger(seq[fx_take_flask], KERNEL_TRIGGER_EXPIRE, 0, 3);
				kernel_seq_depth(seq[fx_take_flask], 1);
				kernel_seq_range(seq[fx_take_flask], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_player(seq[fx_take_flask], true);
				kernel_seq_loc(seq[fx_take_flask], player.x - 3, player.y + 7);
				player.command_ready = false;
				return;
			}
			break;

		case 2:
			if (local->prevent) {
				if (inter_point_x < 92) {
					kernel_seq_delete(seq[fx_flask_end]);
					kernel_flip_hotspot_loc(words_flask, false, FLASK_END_X, FLASK_END_Y);
					sound_play(N_TakeObjectSnd);
					inter_give_to_player(flask);
					object_examine(flask, 60509, 0);

				} else {
					if (player_said_1(flask_full_of_acid)) {
						kernel_seq_delete(seq[fx_flask_full]);
						kernel_flip_hotspot(words_flask_full_of_acid, false);
						inter_give_to_player(flask_full_of_acid);
						sound_play(N_TakeObjectSnd);
						object_examine(flask_full_of_acid, 60571, 0);

					} else {
						kernel_seq_delete(seq[fx_flask_empty]);
						kernel_flip_hotspot(words_flask, false);
						inter_give_to_player(flask);
					}
				}
			}
			global[flask_on_plate] = NONE;
			local->prevent         = true;
			player.command_ready = false;
			return;

		case 3:
			player.walker_visible   = true;
			player.commands_allowed = true;
			local->prevent          = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_flask]);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_3(put, teleport_door, freezer) ||
	    player_said_3(throw, teleport_door, freezer)) {
		id = object_named(player_second_noun);
		if (id == teleport_door) {
			text_show(60582);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_3(put, flask, metal_plate)) {
		switch (kernel.trigger) {
		case 0:
			if (player_has(flask)) {
				if (global[fluid_is_dripping]) {
					text_show(60511);
					player.command_ready = false;
					return;

				} else {
					player.commands_allowed = false;
					player.walker_visible   = false;
					seq[fx_take_flask]      = kernel_seq_pingpong(ss[fx_take_flask], false, 7, 0, 0, 2);
					kernel_seq_trigger(seq[fx_take_flask], KERNEL_TRIGGER_SPRITE, 6, 2);
					kernel_seq_trigger(seq[fx_take_flask], KERNEL_TRIGGER_EXPIRE, 0, 3);
					kernel_seq_depth(seq[fx_take_flask], 1);
					kernel_seq_range(seq[fx_take_flask], KERNEL_FIRST, KERNEL_LAST);
					kernel_seq_player(seq[fx_take_flask], true);
					kernel_seq_loc(seq[fx_take_flask], player.x - 3, player.y + 7);
					player.command_ready = false;
					return;
				}
			}
			break;

		case 2:
			if (local->prevent) {
				seq[fx_flask_empty] = kernel_seq_stamp(ss[fx_flask_empty], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_flask_empty], 5);
				kernel_flip_hotspot(words_flask, false);
				kernel_flip_hotspot_loc(words_flask, true, FLASK_EMPTY_X, FLASK_EMPTY_Y);
				inter_move_object(flask, NOWHERE);
				global[flask_on_plate] = YES_EMPTY;
			}
			local->prevent = true;
			player.command_ready = false;
			return;

		case 3:
			text_show(60510);
			player.walker_visible   = true;
			player.commands_allowed = true;
			local->prevent          = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_flask]);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(close, petcock)) {
		if (global[fluid_is_dripping]) {
			kernel_timing_trigger(1, ROOM_605_CLOSE_PETCOCKS);
		} else {
			text_show(60574);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(turn, petcock) || player_said_2(open, petcock)) {
		if (player_said_2(turn, petcock) && global[fluid_is_dripping] && !kernel.trigger) {
			kernel_timing_trigger(1, ROOM_605_CLOSE_PETCOCKS);
			player.command_ready = false;
			return;

		} else if (player_said_1(open) && global[fluid_is_dripping] && !kernel.trigger) {
			text_show(60575);
			player.command_ready = false;
			return;

		} else if (global[flask_on_plate] == YES_ACID && !kernel.trigger) {
			text_show(60578);
			player.command_ready = false;
			return;

		} else {
			switch (kernel.trigger) {
			case 0:
				if (global[flask_on_plate] == NONE) {
					global[fluid_is_dripping] = true;
				}

				player.commands_allowed = false;
				player.walker_visible   = false;
				if (inter_point_x < 116) {
					seq[fx_turn_petcock] = kernel_seq_pingpong(ss[fx_turn_petcock], true, 7, 0, 0, 2);
					kernel_seq_trigger(seq[fx_turn_petcock], KERNEL_TRIGGER_SPRITE, 6, 1);
				} else {
					seq[fx_turn_petcock] = kernel_seq_pingpong(ss[fx_turn_petcock], false, 7, 0, 0, 2);
					kernel_seq_trigger(seq[fx_turn_petcock], KERNEL_TRIGGER_SPRITE, 6, 2);
				}
				kernel_seq_player(seq[fx_turn_petcock], true);
				kernel_seq_trigger(seq[fx_turn_petcock], KERNEL_TRIGGER_EXPIRE, 0, 3);
				kernel_seq_depth(seq[fx_turn_petcock], 1);
				kernel_seq_range(seq[fx_turn_petcock], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_scale(seq[fx_turn_petcock], 100);
				if (!(global[player_score_flags] & SCORE_MAKE_ACID_DRIP_605)) {
					global[player_score_flags] = global[player_score_flags] | SCORE_MAKE_ACID_DRIP_605;
					global[player_score] += 1;
				}
				player.command_ready = false;
				return;

			case 1:
				if (local->prevent) {
					sound_play(N_TurnPetcock);
					seq[fx_fluid_left] = kernel_seq_forward(ss[fx_fluid_left], false, 8, 0, 0, 1);
					kernel_seq_depth(seq[fx_fluid_left], 5);
					kernel_seq_range(seq[fx_fluid_left], KERNEL_FIRST, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_fluid_left],
					    KERNEL_TRIGGER_EXPIRE, 0, ROOM_605_FLUID_LEFT);
				}
				local->prevent = true;
				player.command_ready = false;
				return;

			case 2:
				if (local->prevent) {
					sound_play(N_TurnPetcock);
					seq[fx_fluid_right] = kernel_seq_forward(ss[fx_fluid_right], false, 8, 0, 0, 1);
					kernel_seq_depth(seq[fx_fluid_right], 5);
					kernel_seq_range(seq[fx_fluid_right], KERNEL_FIRST, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_fluid_right],
					    KERNEL_TRIGGER_EXPIRE, 0, ROOM_605_FLUID_RIGHT);
				}
				local->prevent = true;
				player.command_ready = false;
				return;

			case 3:
				kernel_timing_trigger(TENTH_SECOND, 4);
				player.walker_visible = true;
				local->prevent        = false;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_turn_petcock]);
				player.command_ready = false;
				return;

			case 4:
				player.walker_visible = false;

				if (inter_point_x > 116) {
					seq[fx_turn_petcock] = kernel_seq_pingpong(ss[fx_turn_petcock], true, 7, 0, 0, 2);
					kernel_seq_trigger(seq[fx_turn_petcock], KERNEL_TRIGGER_SPRITE, 6, 1);

				} else {
					seq[fx_turn_petcock] = kernel_seq_pingpong(ss[fx_turn_petcock], false, 7, 0, 0, 2);
					kernel_seq_trigger(seq[fx_turn_petcock], KERNEL_TRIGGER_SPRITE, 6, 2);
				}
				kernel_seq_player(seq[fx_turn_petcock], true);
				kernel_seq_trigger(seq[fx_turn_petcock], KERNEL_TRIGGER_EXPIRE, 0, 5);
				kernel_seq_depth(seq[fx_turn_petcock], 1);
				kernel_seq_range(seq[fx_turn_petcock], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_scale(seq[fx_turn_petcock], 100);
				player.command_ready = false;
				return;

			case 5:
				player.walker_visible = true;
				local->prevent        = false;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_turn_petcock]);

				if (global[flask_on_plate] == YES_EMPTY) {
					kernel_seq_delete(seq[fx_flask_empty]);
					seq[fx_flask_full] = kernel_seq_stamp(ss[fx_flask_full], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_flask_full], 5);
					kernel_flip_hotspot(words_flask, false);
					kernel_flip_hotspot(words_flask_full_of_acid, true);
					global[flask_on_plate] = YES_ACID;
					kernel_timing_trigger(FOUR_SECONDS, 6);
				}
				player.command_ready = false;
				return;

			case 6:
				++global[player_score];
				text_show(60570);
				kernel_timing_trigger(TENTH_SECOND, ROOM_605_CLOSE_PETCOCKS);
				player.command_ready = false;
				return;
			}
		}
	}

	if (player_said_2(put, freezer) && !kernel.trigger) {
		if (player_said_1(ratsicle)) {
			text_show(60562);
			player.command_ready = false;
			return;

		} else {
			id = object_named(player_main_noun);
			if (player_has(id)) {
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_reach_into_freezer] = kernel_seq_pingpong(ss[fx_reach_into_freezer], false, 7, 0, 0, 2);
				kernel_seq_trigger(seq[fx_reach_into_freezer], KERNEL_TRIGGER_SPRITE, 8, ROOM_605_PUT_IN_FREEZER);
				kernel_seq_trigger(seq[fx_reach_into_freezer], KERNEL_TRIGGER_EXPIRE, 0, ROOM_605_PUT_IN_FREEZER + 1);
				kernel_seq_range(seq[fx_reach_into_freezer], KERNEL_FIRST, KERNEL_LAST);
				player.command_ready = false;
				return;
			}
		}
	}

	switch (kernel.trigger) {
	case ROOM_605_PUT_IN_FREEZER:
		id = object_named(player_main_noun);
		kernel_flip_hotspot(words_contents_of_freezer, true);
		inter_move_object(id, FREEZER);
		if (id == dead_rat) {
			++global[player_score];
		}
		if (global[object_is_in_freezer_605] != HAS_USED_FREEZER) {
			seq[fx_object_in_freezer] = kernel_seq_stamp(ss[fx_object_in_freezer], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_object_in_freezer], 14);
		}
		player.command_ready = false;
		return;

	case ROOM_605_PUT_IN_FREEZER + 1:
		player.walker_visible   = true;
		player.commands_allowed = true;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_reach_into_freezer]);

		if (player_said_1(bone) || player_said_1(fruit) || player_said_1(flies) ||
		    player_said_1(feathers) || player_said_1(dead_rat) || player_said_1(partly_built_bundle) ||
		    player_said_1(dates) || player_said_1(tentacle_parts)) {
			text_show(60553);

		} else if (!player_said_1(bone) && !player_said_1(fruit) && !player_said_1(flies) &&
		           !player_said_1(feathers) && !player_said_1(dead_rat) && !player_said_1(partly_built_bundle) &&
		           !player_said_1(dates) && !player_said_1(tentacle_parts)) {

			if (global[object_is_in_freezer_605] == NEVER_USED_FREEZER ||
			    game.difficulty == HARD_MODE) {
				text_show(60551);

			} else if ((global[object_is_in_freezer_605] != NEVER_USED_FREEZER) &&
			           (game.difficulty == EASY_MODE)) {
				text_show(60554);
			}
		}
		global[object_is_in_freezer_605] = HAS_USED_FREEZER;
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, contents_of_freezer) && !kernel.trigger) {
		if (global[object_is_in_freezer_605] == HAS_USED_FREEZER) {
			player.walker_visible      = false;
			seq[fx_reach_into_freezer] = kernel_seq_pingpong(ss[fx_reach_into_freezer], false, 7, 0, 0, 2);
			kernel_seq_trigger(seq[fx_reach_into_freezer], KERNEL_TRIGGER_SPRITE, 8, ROOM_605_TAKE_CONTENTS);
			kernel_seq_trigger(seq[fx_reach_into_freezer], KERNEL_TRIGGER_EXPIRE, 0, ROOM_605_TAKE_CONTENTS + 1);
			kernel_seq_range(seq[fx_reach_into_freezer], KERNEL_FIRST, KERNEL_LAST);
		} else {
			text_show(60576);
		}
		player.command_ready = false;
		return;
	}

	switch (kernel.trigger) {
	case ROOM_605_TAKE_CONTENTS:
		global[object_is_in_freezer_605] = HAS_USED_AND_EMPTY;
		was_rat_removed                  = false;
		are_any_inorganic                = false;
		are_any_organic                  = false;

		kernel_flip_hotspot(words_contents_of_freezer, false);

		for (count = 0; count < OBJECTS_MAX; count++) {
			if (object[count].location == FREEZER) {
				++removed;
				last_removed = count;

				if (count == dead_rat) {
					was_rat_removed = true;
					inter_give_to_player(ratsicle);
					inter_move_object(dead_rat, NOWHERE);
				} else {
					inter_give_to_player(count);
				}

				if (last_removed == bone || last_removed == fruit || last_removed == bottle_of_flies ||
				    last_removed == feathers || last_removed == dead_rat || last_removed == partly_built_bundle ||
				    last_removed == dates || last_removed == tentacle_parts) {
					are_any_organic = true;

				} else {
					are_any_inorganic = true;
				}
			}
		}

		if (removed == 1) {
			if (last_removed == bone || last_removed == fruit || last_removed == bottle_of_flies ||
			    last_removed == feathers || last_removed == partly_built_bundle ||
			    last_removed == dates || last_removed == tentacle_parts) {
				text_show(60555);

			} else if (last_removed == dead_rat) {
				text_show(60556);

			} else if (last_removed != bone && last_removed != fruit && last_removed != bottle_of_flies &&
			           last_removed != feathers && last_removed != dead_rat && last_removed != partly_built_bundle &&
			           last_removed != dates && last_removed != tentacle_parts) {
				text_show(60552);
			}

		} else if (!are_any_inorganic && !was_rat_removed) {
			text_show(60558);

		} else if (!are_any_inorganic && was_rat_removed) {
			text_show(60559);

		} else if (are_any_inorganic && are_any_organic &&
		           !was_rat_removed && game.difficulty == HARD_MODE) {
			text_show(60560);

		} else if (are_any_inorganic && are_any_organic &&
		           !was_rat_removed && game.difficulty == EASY_MODE) {
			text_show(60563);

		} else if (are_any_inorganic && are_any_organic &&
		           was_rat_removed) {
			text_show(60561);

		} else if (are_any_inorganic && !are_any_organic &&
		           !was_rat_removed) {
			text_show(60557);
		}

		kernel_seq_delete(seq[fx_object_in_freezer]);
		player.command_ready = false;
		return;

	case ROOM_605_TAKE_CONTENTS + 1:
		player.walker_visible   = true;
		player.commands_allowed = true;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_reach_into_freezer]);
		player.command_ready = false;
		return;
	}

	if (player.look_around) {
		text_show(60501);
		if (global[rope_is_alive]) {
			text_show(60502);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(rope) && object_is_here(rope)) {
			if (global[rope_is_alive]) {
				text_show(60503);
			} else {
				text_show(60504);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(rat_cage)) {
			if (global[rat_cage_is_open]) {
				if (player_has(dead_rat)) {
					text_show(60536);
				} else {
					text_show(60535);
				}
			} else {
				text_show(60534);
			}

			player.command_ready = false;
			return;
		}

		if (player_said_1(flask) && player.main_object_source == STROKE_INTERFACE) {
			if (inter_point_x < 118) {
				text_show(60508);
			} else {
				text_show(60573);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(flask_full_of_acid) && object_is_here(flask_full_of_acid)) {
			text_show(60572);
			player.command_ready = false;
			return;
		}

		if (player_said_1(petcock)) {
			if (global[fluid_is_dripping]) {
				text_show(60513);
			} else {
				text_show(60512);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(flame)) {
			text_show(60516);
			player.command_ready = false;
			return;
		}

		if (player_said_1(cobwebs)) {
			text_show(60517);
			player.command_ready = false;
			return;
		}

		if (player_said_1(nozzle)) {
			if (global[fluid_is_dripping]) {
				text_show(60519);
			} else {
				text_show(60518);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(retort)) {
			if (game.difficulty == HARD_MODE) {
				text_show(60521);
			} else {
				text_show(60520);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(shaft_of_light)) {
			text_show(60522);
			player.command_ready = false;
			return;
		}

		if (player_said_1(metal_plate)) {
			if (global[fluid_is_dripping]) {
				text_show(60526);
			} else {
				text_show(60525);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(musty_chart)) {
			if (game.difficulty == HARD_MODE) {
				text_show(60528);
			} else {
				text_show(60527);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(ancient_barrel)) {
			text_show(60529);
			player.command_ready = false;
			return;
		}

		if (player_said_1(contents_of_freezer)) {
			if (global[object_is_in_freezer_605] == HAS_USED_FREEZER) {
				text_show(60524);
			} else {
				text_show(60523);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(cage)) {
			text_show(60530);
			player.command_ready = false;
			return;
		}

		if (player_said_1(balance)) {
			text_show(60531);
			player.command_ready = false;
			return;
		}

		if (player_said_1(dead_rat) && !player_has(dead_rat)) {
			text_show(60506);
			player.command_ready = false;
			return;
		}

		if (player_said_1(shelf)) {
			text_show(60532);
			player.command_ready = false;
			return;
		}

		if (player_said_1(tubing)) {
			text_show(60533);
			player.command_ready = false;
			return;
		}

		if (player_said_1(door_to_magic_room)) {
			text_show(60537);
			player.command_ready = false;
			return;
		}

		if (player_said_1(freezer)) {
			text_show(60538);
			player.command_ready = false;
			return;
		}

		if (player_said_1(cabinet)) {
			text_show(60539);
			player.command_ready = false;
			return;
		}

		if (player_said_1(lab_table)) {
			text_show(60540);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_1(push) || player_said_1(pull) || player_said_1(take)) {
		if (player_said_1(lab_table) || player_said_1(freezer)) {
			text_show(60549);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(take, rat_cage)) {
		if (player_has(dead_rat)) {
			text_show(60544);
		} else {
			text_show(60543);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, retort)) {
		text_show(60541);
		player.command_ready = false;
		return;
	}

	if (player_said_2(throw, rope) && global[rope_is_alive]) {
		if (player_said_1(shieldstone)) {
			text_show(60565);

		} else {
			text_show(60567);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_1(open) || player_said_1(close)) {
		if (player_said_1(freezer)) {
			if (global[object_is_in_freezer_605] == NEVER_USED_FREEZER) {
				text_show(60545);
			} else {
				text_show(60546);
			}
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(take, balance) ||
	    player_said_2(take, musty_chart) ||
	    player_said_2(take, cage)) {
		text_show(60542);
		player.command_ready = false;
		return;
	}

	if (player_said_1(open) || player_said_1(close)) {
		if (player_said_1(cage)) {
			text_show(60547);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_3(put, flask, metal_plate)) {
		if (global[flask_on_plate] == YES_EMPTY) {
			text_show(60583);
			player.command_ready = false;
			return;

		} else if (!player_has(flask)) {
			text_show(60580);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_3(put, goblet, metal_plate)) {
		text_show(60566);
		player.command_ready = false;
		return;
	}
}

void room_605_synchronize(Common::Serializer &s) {
	for (int i = 0; i < 23; i++) s.syncAsSint16LE(local->sprite[i]);
	for (int i = 0; i < 23; i++) s.syncAsSint16LE(local->sequence[i]);
	for (int i = 0; i < 4; i++)  s.syncAsSint16LE(local->animation[i]);
	s.syncAsSint16LE(local->rope_frame);
	s.syncAsSint16LE(local->rope_action);
	s.syncAsSint16LE(local->rope_talk_count);
	s.syncAsSint16LE(local->anim_0_running);
	s.syncAsSint16LE(local->rat_frame);
	s.syncAsSint16LE(local->anim_1_running);
	s.syncAsSint16LE(local->pid_frame);
	s.syncAsSint16LE(local->pid_action);
	s.syncAsSint16LE(local->pid_talk_count);
	s.syncAsSint16LE(local->anim_2_running);
	s.syncAsSint16LE(local->rat_cage_frame);
	s.syncAsSint16LE(local->rat_cage_talk_count);
	s.syncAsSint16LE(local->anim_3_running);
	s.syncAsSint16LE(local->rope_id);
	s.syncAsSint16LE(local->prevent);
	s.syncAsSint16LE(local->prevent_2);
}

void room_605_preload() {
	room_init_code_pointer = room_605_init;
	room_pre_parser_code_pointer = room_605_pre_parser;
	room_parser_code_pointer = room_605_parser;
	room_daemon_code_pointer = room_605_daemon;

	if (!player_has(rope)) {
		kernel_initial_variant = 1;
	}

	section_6_walker();
	section_6_interface();

	vocab_make_active(words_rat);
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
