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

#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/config.h"
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
#include "mads/madsv2/dragonsphere/rooms/room503.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];
	int16 water_frame;
	int16 anim_0_running;
	int16 rock_base;
	int16 dyn_rock;
	int16 branch_base;
	int16 dyn_branch;
	int16 prevent;
	int32 update_clock;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_body          0
#define fx_flies         1
#define fx_gold          2
#define fx_take_gold     3
#define fx_bottom        4
#define fx_rock          5
#define fx_branch        6

#define ROOM_503_DOOR_CLOSES  60

#define PLAYER_X_FROM_505    -15
#define PLAYER_Y_FROM_505    119
#define WALK_TO_X_FROM_505    15
#define WALK_TO_Y_FROM_505   119
#define PLAYER_X_FROM_504    318
#define PLAYER_Y_FROM_504    112
#define WALK_TO_X_FROM_504   337
#define WALK_TO_Y_FROM_504   123
#define PLAYER_X_FROM_502    645
#define PLAYER_Y_FROM_502    115
#define WALK_TO_X_FROM_502   624
#define WALK_TO_Y_FROM_502   115

#define camera_ratio_1    1
#define camera_ratio_2    3

#define GOLD_X            195
#define GOLD_Y            139


static void set_rock_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int y;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_rock] >= 0) {
		kernel_seq_delete(seq[fx_rock]);
	}

	difference = center - local->rock_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace   = sgn_in(displace, direction);

	x          = local->rock_base + displace - 1;
	y          = series_list[ss[fx_rock]]->index[0].ys + 126;
	xs         = series_list[ss[fx_rock]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_rock] = -1;
	} else {
		seq[fx_rock] = kernel_seq_stamp(ss[fx_rock], false, 1);
		kernel_seq_loc(seq[fx_rock], x, y);
		kernel_seq_depth(seq[fx_rock], 1);
	}
}

static void set_branch_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int y;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_branch] >= 0) {
		kernel_seq_delete(seq[fx_branch]);
	}

	difference = center - local->branch_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace   = sgn_in(displace, direction);

	x          = local->branch_base + displace - 1;
	y          = series_list[ss[fx_branch]]->index[0].ys + 118;
	xs         = series_list[ss[fx_branch]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_branch] = -1;
	} else {
		seq[fx_branch] = kernel_seq_stamp(ss[fx_branch], false, 1);
		kernel_seq_loc(seq[fx_branch], x, y);
		kernel_seq_depth(seq[fx_branch], 1);
	}
}

static void handle_animation_water() {
	int water_reset_frame;

	if (kernel_anim[aa[0]].frame != local->water_frame) {
		local->water_frame = kernel_anim[aa[0]].frame;
		water_reset_frame  = -1;

		if (local->water_frame == 26) {
			water_reset_frame = 0;
		}

		if (water_reset_frame >= 0) {
			kernel_reset_animation(aa[0], water_reset_frame);
			local->water_frame = water_reset_frame;
		}
	}
}

static void room_503_init() {
	kernel.disable_fastwalk = true;
	local->prevent = false;

	ss[fx_rock] = kernel_load_series(kernel_name('r', 0), false);
	ss[fx_branch] = kernel_load_series(kernel_name('r', 1), false);

	if (global[monster_is_dead]) {
		ss[fx_flies] = kernel_load_series(kernel_name('x', 1), false);
		ss[fx_body] = kernel_load_series(kernel_name('x', 0), false);
		seq[fx_body] = kernel_seq_stamp(ss[fx_body], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_body], 14);
		seq[fx_flies] = kernel_seq_forward(ss[fx_flies], false, 4, 0, 0, 0);
		kernel_seq_depth(seq[fx_flies], 1);
		kernel_seq_range(seq[fx_flies], KERNEL_FIRST, KERNEL_LAST);

	} else {
		kernel_flip_hotspot(words_beast, false);
	}

	if (object_is_here(gold_nugget)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			ss[fx_take_gold] = kernel_load_series("*KGRD_7", false);
		} else {
			ss[fx_take_gold] = kernel_load_series("*PDRL_9", false);
		}
		ss[fx_gold] = kernel_load_series(kernel_name('p', 0), false);
		seq[fx_gold] = kernel_seq_stamp(ss[fx_gold], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_gold], 14);

	} else {
		kernel_flip_hotspot(words_gold_nugget, false);
	}

	if (!global[waterfall_diverted]) {
		ss[fx_bottom] = kernel_load_series(kernel_name('x', 2), false);
		seq[fx_bottom] = kernel_seq_forward(ss[fx_bottom], false, 6, 0, 0, 0);
		kernel_seq_depth(seq[fx_bottom], 3);
		kernel_seq_range(seq[fx_bottom], KERNEL_FIRST, KERNEL_LAST);

		aa[0] = kernel_run_animation(kernel_name('w', 1), 0);
		local->anim_0_running = true;

	} else {
		local->anim_0_running = false;
	}

	if (previous_room == 505) {
		player_first_walk(PLAYER_X_FROM_505, PLAYER_Y_FROM_505, FACING_EAST,
			WALK_TO_X_FROM_505, WALK_TO_Y_FROM_505, FACING_EAST, true);

	} else if (previous_room == 504) {
		player_first_walk(PLAYER_X_FROM_504, PLAYER_Y_FROM_504, FACING_SOUTHEAST,
			WALK_TO_X_FROM_504, WALK_TO_Y_FROM_504, FACING_SOUTHEAST, true);
		camera_jump_to(160, 0);

	} else if ((previous_room == 502) || (previous_room != KERNEL_RESTORING_GAME)) {
		player_first_walk(PLAYER_X_FROM_502, PLAYER_Y_FROM_502, FACING_WEST,
			WALK_TO_X_FROM_502, WALK_TO_Y_FROM_502, FACING_WEST, true);
		camera_jump_to(320, 0);
	}

	seq[fx_rock] = -1;
	local->dyn_rock = -1;
	local->rock_base = 75;

	seq[fx_branch] = -1;
	local->dyn_branch = -1;
	local->branch_base = 558;

	set_rock_position();
	set_branch_position();

	local->update_clock = kernel.clock;

	section_5_music();
}

static void room_503_daemon() {
	int dist;

	if (local->anim_0_running) {
		handle_animation_water();
	}

	if (camera_x.pan_this_frame) {
		set_rock_position();
		set_branch_position();
	}

	if (kernel.clock >= local->update_clock) {
		dist = 127 - ((imath_hypot(player.x - 287, player.y - 146) * 127) / 378);
		if (!sound_off) {
			if (player.x < 33 || player.x > 534) {
				sound_queue(N_005Waterfall, 42);
			} else {
				sound_queue(N_005Waterfall, dist);
			}
		}
		local->update_clock = kernel.clock + player.frame_delay;
	}

	if (player.x > 432 && player.x < 550 && global[monster_is_dead]) {
		sound_play(N_005FlyingInsect);
	}
}

static void room_503_pre_parser() {
	if (player_said_2(walk_down, path_to_west)) {
		player.walk_off_edge_to_room = 505;
	}

	if (player_said_2(walk_down, path_to_east)) {
		player.walk_off_edge_to_room = 502;
	}

	if (player_said_2(take, gold_nugget) && !player_has(gold_nugget) &&
	    global[player_persona] == PLAYER_IS_PID) {
		player_walk(GOLD_X, GOLD_Y, FACING_NORTHWEST);
	}

	if (player_said_1(cave) && player.need_to_walk) {
		if (!player_said_1(walk_into)) {
			player.need_to_walk = false;
		}
	}
}

static void room_503_parser() {
	if (player_said_2(walk_into, cave)) {
		new_room = 504;
		player.command_ready = false;
		return;
	}

	if (player.look_around) {
		text_show(50301);
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, gold_nugget) ||
	    player_said_2(pull, gold_nugget)) {

		switch (kernel.trigger) {
		case 0:
			if (!player_has(gold_nugget)) {
				player.commands_allowed = false;
				player.walker_visible   = false;
				if (global[player_persona] == PLAYER_IS_KING) {
					seq[fx_take_gold] = kernel_seq_forward(ss[fx_take_gold], false, 7, 0, 0, 1);
					kernel_seq_trigger(seq[fx_take_gold], KERNEL_TRIGGER_EXPIRE, 0, 1);
				} else {
					seq[fx_take_gold] = kernel_seq_pingpong(ss[fx_take_gold], true, 7, 0, 0, 2);
					kernel_seq_trigger(seq[fx_take_gold], KERNEL_TRIGGER_SPRITE, 4, 2);
					kernel_seq_trigger(seq[fx_take_gold], KERNEL_TRIGGER_EXPIRE, 0, 3);
				}
				kernel_seq_depth(seq[fx_take_gold], 1);
				kernel_seq_range(seq[fx_take_gold], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_player(seq[fx_take_gold], true);
				player.command_ready = false;
				return;
			}
			break;

		case 1:
			kernel_seq_delete(seq[fx_gold]);
			kernel_flip_hotspot(words_gold_nugget, false);
			player.walker_visible   = true;
			player.commands_allowed = true;
			global[player_score]   += 5;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_gold]);
			sound_play(N_TakeObjectSnd);
			inter_give_to_player(gold_nugget);
			object_examine(gold_nugget, 50319, 0);
			player.command_ready = false;
			return;

		case 2:
			if (local->prevent) {
				kernel_seq_delete(seq[fx_gold]);
				kernel_flip_hotspot(words_gold_nugget, false);
				global[player_score]   += 5;
				sound_play(N_TakeObjectSnd);
				inter_give_to_player(gold_nugget);
				object_examine(gold_nugget, 50319, 0);
			}
			local->prevent = true;
			player.command_ready = false;
			return;

		case 3:
			player.walker_visible   = true;
			player.commands_allowed = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_gold]);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(cave)) {
			if (!player_has_been_in_room(504)) {
				if (global[waterfall_diverted]) {
					text_show(50309);
				} else {
					text_show(50308);
				}
			} else {
				text_show(50310);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(path_to_west)) {
			text_show(50313);
			player.command_ready = false;
			return;
		}

		if (player_said_1(path_to_east)) {
			text_show(50314);
			player.command_ready = false;
			return;
		}

		if (player_said_1(mountainside)) {
			text_show(50315);
			player.command_ready = false;
			return;
		}

		if (player_said_1(waterfall)) {
			text_show(50316);
			player.command_ready = false;
			return;
		}

		if (player_said_1(ground)) {
			text_show(50317);
			player.command_ready = false;
			return;
		}

		if (player_said_1(gold_nugget) && object_is_here(gold_nugget)) {
			text_show(50318);
			player.command_ready = false;
			return;
		}

		if (player_said_1(puddle)) {
			text_show(50303);
			player.command_ready = false;
			return;
		}

		if (player_said_1(beast)) {
			if (global[found_lani_504]) {
				text_show(50321);
			} else {
				text_show(50320);
			}
			player.command_ready = false;
			return;
		}
	}

	if (player_said_3(fill, goblet, waterfall) ||
	    player_said_3(fill, goblet, puddle)) {
		text_show(50305);
		player.command_ready = false;
		return;
	}

	if (player_said_2(put, waterfall) ||
	    player_said_2(put, puddle)) {
		if (player_said_1(torch)) {
			text_show(50306);
			player.command_ready = false;
			return;
		} else if (player_has(object_named(player_main_noun))) {
			text_show(50307);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_1(cave)) {
		if (player_said_1(open) || player_said_1(close)) {
			text_show(50311);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(throw, cave)) {
		text_show(50312);
		player.command_ready = false;
		return;
	}

	if (player_said_2(heal, beast)) {
		text_show(50322);
		player.command_ready = false;
		return;
	}

	if (player_said_2(talk_to, beast)) {
		text_show(50323);
		player.command_ready = false;
		return;
	}
}

void room_503_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(local->water_frame);
	s.syncAsSint16LE(local->anim_0_running);
	s.syncAsSint16LE(local->rock_base);
	s.syncAsSint16LE(local->dyn_rock);
	s.syncAsSint16LE(local->branch_base);
	s.syncAsSint16LE(local->dyn_branch);
	s.syncAsSint16LE(local->prevent);
	s.syncAsSint32LE(local->update_clock);
}

void room_503_preload() {
	room_init_code_pointer       = room_503_init;
	room_pre_parser_code_pointer = room_503_pre_parser;
	room_parser_code_pointer     = room_503_parser;
	room_daemon_code_pointer     = room_503_daemon;

	section_5_walker();
	section_5_interface();

	if (global[monster_is_dead]) {
		kernel_initial_variant = 1;
	}
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
