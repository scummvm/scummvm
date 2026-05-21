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
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section4.h"
#include "mads/madsv2/dragonsphere/rooms/room410.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[17];
	int16 sequence[17];
	int16 animation[4];

	int16 snake_1_on;
	int16 snake_2_on;
	int16 snake_3_on;
	int16 snake_4_on;
	int16 snake_5_on;
	int16 snake_6_on;
	int16 snake_7_on;
	int16 snake_8_on;
	int16 snake_9_on;
	int16 snake_10_on;
	int16 snake_11_on;

	int16 face_1_on;
	int16 face_2_on;
	int16 face_3_on;
	int16 face_4_on;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_snake_1              0
#define fx_snake_2              1
#define fx_snake_3              2
#define fx_snake_4              3
#define fx_snake_5              4
#define fx_snake_6              5
#define fx_snake_7              6
#define fx_snake_8              7
#define fx_shift                8
#define fx_face_1               9
#define fx_face_2               10
#define fx_face_3               11
#define fx_face_4               12
#define fx_snake_9              13
#define fx_snake_10             14
#define fx_snake_11             15
#define fx_death                16

#define ROOM_410_DOOR_CLOSES    60
#define ROOM_410_FACE_OFF       70
#define ROOM_410_DIE            90

#define SNAKE_X                 109
#define SNAKE_Y                 145

#define DEATH_X                 128
#define DEATH_Y                 131

#define AFTER_SNAKE_X           200
#define AFTER_SNAKE_Y           89

#define WALK_TO_X               329
#define WALK_TO_Y               85

#define PLAYER_X_FROM_409       137
#define PLAYER_Y_FROM_409       149

static Scratch scratch;


static void room_410_init() {
	local->snake_1_on = false;
	local->snake_2_on = false;
	local->snake_3_on = false;
	local->snake_4_on = false;
	local->snake_5_on = false;
	local->snake_6_on = false;
	local->snake_7_on = false;
	local->snake_8_on = false;
	local->snake_9_on = false;
	local->snake_10_on = false;
	local->snake_11_on = false;
	local->face_1_on = false;
	local->face_2_on = false;
	local->face_3_on = false;
	local->face_4_on = false;
	kernel.disable_fastwalk = true;

	ss[fx_snake_1] = kernel_load_series(kernel_name('y', 1), false);
	ss[fx_snake_2] = kernel_load_series(kernel_name('y', 2), false);
	ss[fx_snake_3] = kernel_load_series(kernel_name('y', 3), false);
	ss[fx_snake_4] = kernel_load_series(kernel_name('y', 4), false);
	ss[fx_snake_5] = kernel_load_series(kernel_name('y', 6), false);
	ss[fx_snake_6] = kernel_load_series(kernel_name('y', 0), false);
	ss[fx_snake_7] = kernel_load_series(kernel_name('y', 5), false);
	ss[fx_snake_8] = kernel_load_series(kernel_name('y', 7), false);
	ss[fx_snake_9] = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_snake_10] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_snake_11] = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_shift] = kernel_load_series(kernel_name('b', 0), false);
	ss[fx_face_1] = kernel_load_series(kernel_name('z', 0), false);
	ss[fx_face_2] = kernel_load_series(kernel_name('z', 2), false);
	ss[fx_face_3] = kernel_load_series(kernel_name('z', 3), false);
	ss[fx_face_4] = kernel_load_series(kernel_name('z', 4), false);
	ss[fx_death] = kernel_load_series(kernel_name('b', 1), false);

	seq[fx_snake_8] = kernel_seq_pingpong(ss[fx_snake_8], false, 25, 0, 0, 0);
	kernel_seq_depth(seq[fx_snake_8], 10);
	kernel_seq_range(seq[fx_snake_8], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_snake_1] = kernel_seq_stamp(ss[fx_snake_1], false, 1);
	kernel_seq_depth(seq[fx_snake_1], 1);

	seq[fx_snake_7] = kernel_seq_stamp(ss[fx_snake_7], false, 20);
	kernel_seq_depth(seq[fx_snake_7], 1);

	global[perform_displacements] = true;
	global[move_direction_409] = false;

	if (previous_room == 409 || previous_room != KERNEL_RESTORING_GAME) {
		player.x = PLAYER_X_FROM_409;
		player.y = PLAYER_Y_FROM_409;
		player.facing = FACING_NORTHEAST;
	}

	section_4_music();
}

static void room_410_daemon() {
	int temp;

	if (!local->snake_2_on && imath_random(1, 300) == 1) {
		seq[fx_snake_2] = kernel_seq_forward(ss[fx_snake_2], false, 9, 0, 0, 1);
		kernel_seq_depth(seq[fx_snake_2], 3);
		kernel_seq_range(seq[fx_snake_2], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_snake_2], KERNEL_TRIGGER_EXPIRE, 0, 2);
		local->snake_2_on = true;
	}

	if (!local->snake_3_on && imath_random(1, 300) == 1) {
		seq[fx_snake_3] = kernel_seq_forward(ss[fx_snake_3], false, 7, 0, 0, 1);
		kernel_seq_depth(seq[fx_snake_3], 3);
		kernel_seq_range(seq[fx_snake_3], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_snake_3], KERNEL_TRIGGER_EXPIRE, 0, 3);
		local->snake_3_on = true;
	}

	if (!local->snake_4_on && imath_random(1, 300) == 1) {
		seq[fx_snake_4] = kernel_seq_forward(ss[fx_snake_4], false, 7, 0, 0, 1);
		kernel_seq_depth(seq[fx_snake_4], 3);
		kernel_seq_range(seq[fx_snake_4], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_snake_4], KERNEL_TRIGGER_EXPIRE, 0, 4);
		local->snake_4_on = true;
	}

	if (!local->snake_5_on && imath_random(1, 300) == 1) {
		seq[fx_snake_5] = kernel_seq_forward(ss[fx_snake_5], false, 7, 0, 0, 1);
		kernel_seq_depth(seq[fx_snake_5], 3);
		kernel_seq_range(seq[fx_snake_5], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_snake_5], KERNEL_TRIGGER_EXPIRE, 0, 5);
		local->snake_5_on = true;
	}

	if (!local->snake_9_on && imath_random(1, 300) == 1) {
		seq[fx_snake_9] = kernel_seq_forward(ss[fx_snake_9], false, 9, 0, 0, 1);
		kernel_seq_depth(seq[fx_snake_9], 3);
		kernel_seq_range(seq[fx_snake_9], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_loc(seq[fx_snake_9], imath_random(161, 207), imath_random(109, 120));
		kernel_seq_trigger(seq[fx_snake_9], KERNEL_TRIGGER_EXPIRE, 0, 6);
		local->snake_9_on = true;
	}

	if (!local->snake_10_on && imath_random(1, 300) == 1) {
		seq[fx_snake_10] = kernel_seq_forward(ss[fx_snake_10], false, 9, 0, 0, 1);
		kernel_seq_depth(seq[fx_snake_10], 3);
		kernel_seq_range(seq[fx_snake_10], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_loc(seq[fx_snake_10], imath_random(161, 207), imath_random(109, 120));
		kernel_seq_trigger(seq[fx_snake_10], KERNEL_TRIGGER_EXPIRE, 0, 7);
		local->snake_10_on = true;
	}

	if (!local->snake_11_on && imath_random(1, 300) == 1) {
		seq[fx_snake_11] = kernel_seq_forward(ss[fx_snake_11], false, 9, 0, 0, 1);
		kernel_seq_depth(seq[fx_snake_11], 3);
		kernel_seq_range(seq[fx_snake_11], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_loc(seq[fx_snake_11], imath_random(161, 207), imath_random(109, 120));
		kernel_seq_trigger(seq[fx_snake_11], KERNEL_TRIGGER_EXPIRE, 0, 8);
		local->snake_11_on = true;
	}

	switch (kernel.trigger) {
	case 1: local->snake_5_on  = false; break;
	case 2: local->snake_2_on  = false; break;
	case 3: local->snake_3_on  = false; break;
	case 4: local->snake_5_on  = false; break;
	case 5: local->snake_5_on  = false; break;
	case 6: local->snake_9_on  = false; break;
	case 7: local->snake_10_on = false; break;
	case 8: local->snake_11_on = false; break;
	}

	switch (kernel.trigger) {
	case 0:
		if (!local->snake_1_on && imath_random(1, 300) == 1) {
			kernel_seq_delete(seq[fx_snake_1]);
			seq[fx_snake_1] = kernel_seq_forward(ss[fx_snake_1], false, 8, 0, 0, 1);
			kernel_seq_depth(seq[fx_snake_1], 3);
			kernel_seq_range(seq[fx_snake_1], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_snake_1], KERNEL_TRIGGER_EXPIRE, 0, 25);
			local->snake_1_on = true;
		}
		break;

	case 25:
		temp = seq[fx_snake_1];
		seq[fx_snake_1] = kernel_seq_stamp(ss[fx_snake_1], false, 1);
		kernel_seq_depth(seq[fx_snake_1], 3);
		kernel_synch(KERNEL_SERIES, seq[fx_snake_1], KERNEL_SERIES, temp);
		local->snake_1_on = false;
		break;
	}

	switch (kernel.trigger) {
	case 0:
		if (!local->snake_6_on && imath_random(1, 300) == 1) {
			seq[fx_snake_6] = kernel_seq_forward(ss[fx_snake_6], false, 8, 0, 0, 1);
			kernel_seq_depth(seq[fx_snake_6], 3);
			kernel_seq_range(seq[fx_snake_6], KERNEL_FIRST, 7);
			kernel_seq_trigger(seq[fx_snake_6], KERNEL_TRIGGER_EXPIRE, 0, 10);
			local->snake_6_on = true;
		}
		break;

	case 10:
		temp = seq[fx_snake_6];
		seq[fx_snake_6] = kernel_seq_stamp(ss[fx_snake_6], false, 7);
		kernel_seq_depth(seq[fx_snake_6], 3);
		kernel_synch(KERNEL_SERIES, seq[fx_snake_6], KERNEL_SERIES, temp);
		kernel_timing_trigger(imath_random(100, 200), 11);
		break;

	case 11:
		kernel_seq_delete(seq[fx_snake_6]);
		seq[fx_snake_6] = kernel_seq_forward(ss[fx_snake_6], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_snake_6], 3);
		kernel_seq_range(seq[fx_snake_6], 8, 11);
		kernel_seq_trigger(seq[fx_snake_6], KERNEL_TRIGGER_EXPIRE, 0, 12);
		local->snake_6_on = true;
		break;

	case 12:
		local->snake_6_on = false;
		break;
	}

	switch (kernel.trigger) {
	case 0:
		if (!local->snake_7_on && imath_random(1, 300) == 1) {
			kernel_seq_delete(seq[fx_snake_7]);
			seq[fx_snake_7] = kernel_seq_backward(ss[fx_snake_7], false, 8, 0, 0, 1);
			kernel_seq_depth(seq[fx_snake_7], 3);
			kernel_seq_range(seq[fx_snake_7], KERNEL_FIRST, 7);
			kernel_seq_trigger(seq[fx_snake_7], KERNEL_TRIGGER_EXPIRE, 0, 19);
			local->snake_7_on = true;
		}
		break;

	case 15:
		seq[fx_snake_7] = kernel_seq_forward(ss[fx_snake_7], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_snake_7], 3);
		kernel_seq_range(seq[fx_snake_7], 7, 20);
		kernel_seq_trigger(seq[fx_snake_7], KERNEL_TRIGGER_EXPIRE, 0, 16);
		break;

	case 16:
		temp = seq[fx_snake_7];
		seq[fx_snake_7] = kernel_seq_stamp(ss[fx_snake_7], false, 20);
		kernel_seq_depth(seq[fx_snake_7], 3);
		kernel_synch(KERNEL_SERIES, seq[fx_snake_7], KERNEL_SERIES, temp);
		kernel_timing_trigger(imath_random(60, 200), 17);
		break;

	case 17:
		kernel_seq_delete(seq[fx_snake_7]);
		seq[fx_snake_7] = kernel_seq_forward(ss[fx_snake_7], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_snake_7], 3);
		kernel_seq_range(seq[fx_snake_7], 7, 20);
		kernel_seq_trigger(seq[fx_snake_7], KERNEL_TRIGGER_EXPIRE, 0, 18);
		break;

	case 18:
		temp = seq[fx_snake_7];
		seq[fx_snake_7] = kernel_seq_stamp(ss[fx_snake_7], false, 20);
		kernel_seq_depth(seq[fx_snake_7], 3);
		kernel_synch(KERNEL_SERIES, seq[fx_snake_7], KERNEL_SERIES, temp);
		local->snake_7_on = false;
		break;

	case 19:
		temp = seq[fx_snake_7];
		seq[fx_snake_7] = kernel_seq_forward(ss[fx_snake_7], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_snake_7], 3);
		kernel_seq_range(seq[fx_snake_7], KERNEL_FIRST, 7);
		kernel_seq_trigger(seq[fx_snake_7], KERNEL_TRIGGER_EXPIRE, 0, 15);
		kernel_synch(KERNEL_SERIES, seq[fx_snake_7], KERNEL_SERIES, temp);
		break;
	}

	if (!local->face_1_on && imath_random(1, 600) == 1) {
		seq[fx_face_1] = kernel_seq_forward(ss[fx_face_1], false, 9, 0, 0, 1);
		kernel_seq_depth(seq[fx_face_1], 10);
		kernel_seq_range(seq[fx_face_1], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_face_1], KERNEL_TRIGGER_EXPIRE, 0, ROOM_410_FACE_OFF);
		local->face_1_on = true;
	}

	if (!local->face_2_on && imath_random(1, 600) == 1) {
		seq[fx_face_2] = kernel_seq_forward(ss[fx_face_2], false, 9, 0, 0, 1);
		kernel_seq_depth(seq[fx_face_2], 10);
		kernel_seq_range(seq[fx_face_2], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_face_2], KERNEL_TRIGGER_EXPIRE, 0, ROOM_410_FACE_OFF + 1);
		local->face_2_on = true;
	}

	if (!local->face_3_on && imath_random(1, 600) == 1) {
		seq[fx_face_3] = kernel_seq_forward(ss[fx_face_3], false, 9, 0, 0, 1);
		kernel_seq_depth(seq[fx_face_3], 10);
		kernel_seq_range(seq[fx_face_3], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_face_3], KERNEL_TRIGGER_EXPIRE, 0, ROOM_410_FACE_OFF + 2);
		local->face_3_on = true;
	}

	if (!local->face_4_on && imath_random(1, 600) == 1) {
		seq[fx_face_4] = kernel_seq_forward(ss[fx_face_4], false, 9, 0, 0, 1);
		kernel_seq_depth(seq[fx_face_4], 10);
		kernel_seq_range(seq[fx_face_4], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_face_4], KERNEL_TRIGGER_EXPIRE, 0, ROOM_410_FACE_OFF + 3);
		local->face_4_on = true;
	}

	switch (kernel.trigger) {
	case ROOM_410_FACE_OFF:     local->face_1_on = false; break;
	case ROOM_410_FACE_OFF + 1: local->face_2_on = false; break;
	case ROOM_410_FACE_OFF + 2: local->face_3_on = false; break;
	case ROOM_410_FACE_OFF + 3: local->face_4_on = false; break;
	}
}

static void room_410_pre_parser() {
	if (inter_point_x < 271 && inter_point_y > 112) {
	} else {
		if (player_said_1(walk_across) || player_said_1(walk_through)) {
			player_walk(DEATH_X, DEATH_Y, FACING_NORTHEAST);
		} else if (!player_said_1(walk_to)) {
			player.need_to_walk = false;
		}
	}

	if (player_said_1(walk_through)) {
		player_walk(DEATH_X, DEATH_Y, FACING_NORTHEAST);
	}

	if (player_said_1(shift_into_snake) ||
	    player_said_2(invoke_power_of, crystal_ball)) {
		player_walk(SNAKE_X, SNAKE_Y, FACING_NORTHEAST);
	}

	if (player_said_2(walk_down, path_to_east)) {
		player_walk(DEATH_X, DEATH_Y, FACING_NORTHEAST);
	}
}

static void room_410_parser() {
	if (player_said_2(walk_down, path_to_south)) {
		text_show(41005);
		new_room = 408;
		player.command_ready = false;
		return;
	}

	if (kernel.trigger == ROOM_410_DIE) {
		sound_play(N_PlayerDies);
		player.command_ready = false;
		return;
	}

	if (((inter_point_x > 271 || inter_point_y < 112) && player_said_1(walk_across)) ||
	    player_said_2(walk_down, path_to_east) || player_said_1(walk_through)) {
		switch (kernel.trigger) {
		case 0:
			text_show(41004);
			player.walker_visible   = false;
			player.commands_allowed = false;
			seq[fx_death] = kernel_seq_forward(ss[fx_death], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_death], 1);
			kernel_seq_range(seq[fx_death], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_death], KERNEL_TRIGGER_SPRITE, 31, ROOM_410_DIE);
			kernel_seq_trigger(seq[fx_death], KERNEL_TRIGGER_EXPIRE, 0, 1);
			kernel_synch(KERNEL_SERIES, seq[fx_death], KERNEL_PLAYER, 0);
			break;

		case 1:
			if (game.difficulty == EASY_MODE) {
				text_show(41027);
			} else {
				text_show(45);
			}
			kernel.force_restart = true;
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_1(shift_into_snake) ||
	    player_said_2(invoke_power_of, crystal_ball)) {
		switch (kernel.trigger) {
		case 0:
			sound_play(N_InvokeCrystalBall);
			player.walker_visible   = false;
			player.commands_allowed = false;
			seq[fx_shift] = kernel_seq_forward(ss[fx_shift], false, 6, 0, 0, 1);
			kernel_seq_depth(seq[fx_shift], 1);
			kernel_seq_range(seq[fx_shift], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_shift], KERNEL_TRIGGER_EXPIRE, 0, 1);
			kernel_seq_trigger(seq[fx_shift], KERNEL_TRIGGER_SPRITE, 53, 3);
			kernel_synch(KERNEL_SERIES, seq[fx_shift], KERNEL_PLAYER, 0);
			player_demand_location(AFTER_SNAKE_X, AFTER_SNAKE_Y);
			player_demand_facing(FACING_NORTHEAST);
			sound_play(N_SnakeHiss);
			break;

		case 1:
			global[player_score] += 3;
			if (player_said_1(crystal_ball)) {
				text_show(41023);
				text_show(970);
				inter_move_object(crystal_ball, NOWHERE);
				global[crystal_ball_dead] = true;
			} else {
				text_show(41024);
			}
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_shift]);
			player_walk(WALK_TO_X, WALK_TO_Y, FACING_EAST);
			player_walk_trigger(2);
			break;

		case 2:
			new_room = 411;
			break;

		case 3:
			sound_play(N_SnakeHiss);
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player.look_around) {
		text_show(41001);
		player.command_ready = false;
		return;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(spirit_tree)) {
			text_show(41003);
			player.command_ready = false;
			return;
		}

		if (player_said_1(snake_pit)) {
			text_show(41008);
			player.command_ready = false;
			return;
		}

		if (player_said_1(ledge)) {
			text_show(41011);
			player.command_ready = false;
			return;
		}

		if (player_said_1(path_to_east)) {
			text_show(41016);
			player.command_ready = false;
			return;
		}

		if (player_said_1(path_to_south)) {
			text_show(41017);
			player.command_ready = false;
			return;
		}

		if (player_said_1(spirit_plane)) {
			text_show(41018);
			player.command_ready = false;
			return;
		}

		if (player_said_1(gnarled_root)) {
			if (inter_point_x > 86 && inter_point_y > 95) {
				text_show(41014);
			} else {
				text_show(41013);
			}
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(talk_to, spirit_tree)) {
		text_show(41007);
		player.command_ready = false;
		return;
	}

	if (player_said_1(spirit_tree)) {
		text_show(41028);
		player.command_ready = false;
		return;
	}

	if (player_said_2(close, snake_pit)) {
		text_show(41009);
		player.command_ready = false;
		return;
	}

	if (player_said_1(snake_pit)) {
		if (player_said_1(put) || player_said_1(throw)) {
			text_show(41010);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(sword, attack) ||
	    player_said_2(sword, carve_up) ||
	    player_said_2(sword, thrust) ||
	    player_said_2(pour_contents_of, floor)) {
		if (player_said_1(ledge)) {
			text_show(41012);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(sword, attack) ||
	    player_said_2(sword, carve_up) ||
	    player_said_2(sword, thrust) ||
	    player_said_1(take) ||
	    player_said_1(push) ||
	    player_said_1(pull) ||
	    player_said_2(pour_contents_of, floor)) {
		if (player_said_1(gnarled_root)) {
			text_show(41015);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(gaze_into, crystal_ball)) {
		text_show(41022);
		player.command_ready = false;
		return;
	}
}

void room_410_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.snake_1_on);
	s.syncAsSint16LE(scratch.snake_2_on);
	s.syncAsSint16LE(scratch.snake_3_on);
	s.syncAsSint16LE(scratch.snake_4_on);
	s.syncAsSint16LE(scratch.snake_5_on);
	s.syncAsSint16LE(scratch.snake_6_on);
	s.syncAsSint16LE(scratch.snake_7_on);
	s.syncAsSint16LE(scratch.snake_8_on);
	s.syncAsSint16LE(scratch.snake_9_on);
	s.syncAsSint16LE(scratch.snake_10_on);
	s.syncAsSint16LE(scratch.snake_11_on);
	s.syncAsSint16LE(scratch.face_1_on);
	s.syncAsSint16LE(scratch.face_2_on);
	s.syncAsSint16LE(scratch.face_3_on);
	s.syncAsSint16LE(scratch.face_4_on);
}

void room_410_preload() {
	room_init_code_pointer       = room_410_init;
	room_pre_parser_code_pointer = room_410_pre_parser;
	room_parser_code_pointer     = room_410_parser;
	room_daemon_code_pointer     = room_410_daemon;

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
