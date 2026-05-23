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
#include "mads/madsv2/dragonsphere/rooms/section6.h"
#include "mads/madsv2/dragonsphere/rooms/room613.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */

	int16 water_frame;       /* animation frame being held for water stuff */
	int16 anim_0_running;

	int16 old_x;
	int16 old_y;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

/* ========================= Sprite Series =================== */

#define fx_fire_1               0       /* rm613x1  */
#define fx_fire_2               1       /* rm613x2  */
#define fx_fire_3               2       /* rm613x3  */
#define fx_fire_4               3       /* rm613x4  */
#define fx_fire_5               4       /* rm613x5  */
#define fx_fire_6               5       /* rm613x6  */
#define fx_left_coal            6       /* rm613c0  */
#define fx_right_coal           7       /* rm613c1  */
#define fx_king_melt            8       /* rm613a1  */
#define fx_door                 9       /* rm613x0  */

/* ========================= Triggers ======================== */



/* ========================= Other Macros ==================== */

#define PLAYER_X_FROM_606	       25 
#define PLAYER_Y_FROM_606	       132 
#define WALK_TO_X_FROM_606             61 
#define WALK_TO_Y_FROM_606             132

#define PLAYER_X_FROM_614	       613
#define PLAYER_Y_FROM_614	       132
#define WALK_TO_X_FROM_614             578
#define WALK_TO_Y_FROM_614             132

#define RIGHT_HALF                     320

static Scratch scratch;


static void room_613_init() {
	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
	}

	ss[fx_fire_1] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_fire_2] = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_fire_3] = kernel_load_series(kernel_name('x', 3), false);
	ss[fx_fire_4] = kernel_load_series(kernel_name('x', 4), false);
	ss[fx_fire_5] = kernel_load_series(kernel_name('x', 5), false);
	ss[fx_fire_6] = kernel_load_series(kernel_name('x', 6), false);

	seq[fx_fire_1] = kernel_seq_forward(ss[fx_fire_1], false, 5, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_1], 1);
	kernel_seq_range(seq[fx_fire_1], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_2] = kernel_seq_forward(ss[fx_fire_2], false, 6, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_2], 1);
	kernel_seq_range(seq[fx_fire_2], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_3] = kernel_seq_forward(ss[fx_fire_3], false, 7, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_3], 1);
	kernel_seq_range(seq[fx_fire_3], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_4] = kernel_seq_forward(ss[fx_fire_4], false, 5, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_4], 1);
	kernel_seq_range(seq[fx_fire_4], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_5] = kernel_seq_forward(ss[fx_fire_5], false, 6, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_5], 1);
	kernel_seq_range(seq[fx_fire_5], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_6] = kernel_seq_forward(ss[fx_fire_6], false, 7, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_6], 1);
	kernel_seq_range(seq[fx_fire_6], KERNEL_FIRST, KERNEL_LAST);



	/* ========================= Previous Rooms ================== */

	if (previous_room == 612) {

		camera_x.pan_mode = CAMERA_MANUAL;
		viewing_at_y = ((video_y - display_y) >> 1);

		aa[0] = kernel_run_animation(kernel_name('w', 1), 0);
		global[floor_is_cool] = true;
		local->anim_0_running = true;
		player.walker_visible = false;
		player.commands_allowed = false;

	} else if (previous_room == 614) {
		player.x = WALK_TO_X_FROM_614;
		player.y = WALK_TO_Y_FROM_614;
		player.facing = FACING_EAST;
		camera_jump_to(RIGHT_HALF, 0);

		if (global[has_red])              inter_give_to_player(red_powerstone);
		if (global[has_yellow])           inter_give_to_player(yellow_powerstone);
		if (global[has_blue])             inter_give_to_player(blue_powerstone);
		if (global[object_imitated] >= 0) inter_give_to_player(polystone);

		ss[fx_left_coal] = kernel_load_series(kernel_name('c', 0), false);
		seq[fx_left_coal] = kernel_seq_stamp(ss[fx_left_coal], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_left_coal], 14);

		ss[fx_right_coal] = kernel_load_series(kernel_name('c', 1), false);
		seq[fx_right_coal] = kernel_seq_stamp(ss[fx_right_coal], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_right_coal], 14);

	} else if ((previous_room == 606) || (previous_room != KERNEL_RESTORING_GAME)) {

		if (global[floor_is_cool]) {
			ss[fx_left_coal] = kernel_load_series(kernel_name('c', 0), false);
			seq[fx_left_coal] = kernel_seq_stamp(ss[fx_left_coal], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_left_coal], 14);

			ss[fx_right_coal] = kernel_load_series(kernel_name('c', 1), false);
			seq[fx_right_coal] = kernel_seq_stamp(ss[fx_right_coal], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_right_coal], 14);

			player_first_walk(PLAYER_X_FROM_606, PLAYER_Y_FROM_606, FACING_EAST,
				WALK_TO_X_FROM_606, WALK_TO_Y_FROM_606, FACING_EAST, true);

		} else {

			player.commands_allowed = false;
			player.walker_visible = false;
			local->old_x = player.x;
			local->old_y = player.y;
			player.x = 50;

			if (player_has(teleport_door)) {
				kernel_timing_trigger(HALF_SECOND, 4);

			} else if (!global[rat_melted] && global[rat_cage_is_open]) {
				viewing_at_y = ((video_y - display_y) >> 1);
				kernel_timing_trigger(HALF_SECOND, 3);
				global[rat_melted] = true;

			} else {
				viewing_at_y = ((video_y - display_y) >> 1);
				ss[fx_king_melt] = kernel_load_series(kernel_name('a', 1), false);
				kernel_timing_trigger(HALF_SECOND, 2);
			}

			if (!player_has(teleport_door)) {
				ss[fx_door] = kernel_load_series(kernel_name('x', 0), false);
				seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_door], 3);
			}
		}

	} else {  /* RESTORING GAME */
		ss[fx_left_coal] = kernel_load_series(kernel_name('c', 0), false);
		seq[fx_left_coal] = kernel_seq_stamp(ss[fx_left_coal], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_left_coal], 14);

		ss[fx_right_coal] = kernel_load_series(kernel_name('c', 1), false);
		seq[fx_right_coal] = kernel_seq_stamp(ss[fx_right_coal], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_right_coal], 14);
	}

	section_6_music();
}

static void handle_animation_water() {
	int water_reset_frame;

	if (kernel_anim[aa[0]].frame != local->water_frame) {
		local->water_frame = kernel_anim[aa[0]].frame;
		water_reset_frame = -1;

		switch (local->water_frame) {

		case 39:  /* end of steam at left screen */
			camera_pan_to(&camera_x, 320);
			break;

		case 90:  /* end of anim */
			if (camera_x.panning) {
				water_reset_frame = 70;

			} else {
				camera_x.pan_mode = CAMERA_PLAYER;
				global[no_load_walker] = false;
				new_room = 612;
			}
			break;
		}

		if (water_reset_frame >= 0) {
			kernel_reset_animation(aa[0], water_reset_frame);
			local->water_frame = water_reset_frame;
		}
	}
}

static void room_613_daemon() {
	if (local->anim_0_running) {
		handle_animation_water();
	}

	switch (kernel.trigger) {
	case 1:
		player.x = local->old_x;
		player.y = local->old_y;
		new_room = 606;
		break;

	case 10:
		/* if (game.difficulty == EASY_MODE) { */
		  /* text_show (61316); */
		/* } */
		player.x = local->old_x;
		player.y = local->old_y;
		new_room = 606;
		break;

	case 2:
		seq[fx_king_melt] = kernel_seq_forward(ss[fx_king_melt], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_king_melt], 3);
		kernel_seq_range(seq[fx_king_melt], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_king_melt], KERNEL_TRIGGER_SPRITE, 46, 5);
		kernel_seq_trigger(seq[fx_king_melt], KERNEL_TRIGGER_SPRITE, 3, 6);
		kernel_seq_trigger(seq[fx_king_melt], KERNEL_TRIGGER_EXPIRE, 0, 10);
		break;

	case 3:
		aa[0] = kernel_run_animation(kernel_name('r', 1), 1);
		break;

	case 4:
		aa[0] = kernel_run_animation(kernel_name('k', 1), 1);
		break;

	case 5:
		sound_play(N_MeltInHotRoom);
		break;

	case 6:
		sound_play(N_AppearInHotRoom);
		break;
	}
}

static void room_613_pre_parser() {
	if (player_said_1(doorway_to_west) && player.need_to_walk) {
		if (!player_said_1(walk_through)) {
			player.need_to_walk = false;
		}
	}

	if (player_said_1(dragon_door) && player.need_to_walk) {
		if (!player_said_1(walk_through)) {
			player.need_to_walk = false;
		}
	}
}

static void room_613_parser() {
	if (player_said_2(walk_through, doorway_to_west)) {
		new_room = 606;
		goto handled;
	}

	if (player_said_2(walk_through, dragon_door)) {
		new_room = 614;
		goto handled;
	}

	if (player.look_around) {
		text_show(61301);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(remains)) {
			text_show(61302);
			goto handled;
		}

		if (player_said_1(window)) {
			text_show(61306);
			goto handled;
		}

		if (player_said_1(iron_floor)) {
			text_show(61309);
			goto handled;
		}

		if (player_said_1(dragon_bones)) {
			text_show(61312);
			goto handled;
		}

		if (player_said_1(doorway_to_west)) {
			text_show(61314);
			goto handled;
		}

		if (player_said_1(dragon_door)) {
			text_show(61315);
			goto handled;
		}
	}

	if (player_said_2(take, remains)) {
		text_show(61303);
		goto handled;
	}

	if (player_said_2(push, remains)) {
		text_show(61304);
		goto handled;
	}

	if (player_said_2(put_magic_into, remains)) {
		text_show(61305);
		goto handled;
	}

	if (player_said_2(open, window) ||
		player_said_2(close, window)) {
		text_show(61307);
		goto handled;
	}

	if (player_said_2(put, window) ||
		player_said_2(throw, window)) {
		text_show(61308);
		goto handled;
	}

	if (player_said_2(open, iron_floor) ||
		player_said_2(close, iron_floor)) {
		text_show(61310);
		goto handled;
	}

	if (player_said_2(put, iron_floor)) {
		text_show(61311);
		goto handled;
	}

	if (player_said_2(take, dragon_bones) ||
		player_said_2(push, dragon_bones) ||
		player_said_2(pull, dragon_bones)) {
		text_show(61313);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_613_synchronize(Common::Serializer &s) {
	for (int i = 0; i < 15; i++) s.syncAsSint16LE(local->sprite[i]);
	for (int i = 0; i < 15; i++) s.syncAsSint16LE(local->sequence[i]);
	for (int i = 0; i < 4; i++)  s.syncAsSint16LE(local->animation[i]);
	s.syncAsSint16LE(local->water_frame);
	s.syncAsSint16LE(local->anim_0_running);
	s.syncAsSint16LE(local->old_x);
	s.syncAsSint16LE(local->old_y);
}

void room_613_preload() {
	room_init_code_pointer = room_613_init;
	room_pre_parser_code_pointer = room_613_pre_parser;
	room_parser_code_pointer = room_613_parser;
	room_daemon_code_pointer = room_613_daemon;

	if (room_id == 612) {
		global[no_load_walker] = true;
	}

	section_6_walker();
	section_6_interface();

	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
