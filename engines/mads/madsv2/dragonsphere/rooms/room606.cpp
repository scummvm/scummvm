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
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section6.h"
#include "mads/madsv2/dragonsphere/rooms/room606.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[22];
	int16 sequence[22];
	int16 animation[4];
	int16 on_floor;

	int16 up_frame;
	int16 anim_0_running;

	int16 down_frame;
	int16 anim_1_running;

	int16 suction_frame;
	int16 anim_3_running;

	int16 top_base;
	int16 dyn_top;

	int16 bottom_base;
	int16 dyn_bottom;

	int16 blue_stone_base;
	int16 dyn_blue_stone;
};

#define local ((Scratch *)(&game.scratch[0]))
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_push_high                   0
#define fx_but_top                     1
#define fx_but_bottom                  2
#define fx_but_middle_high             3
#define fx_but_middle_low              4
#define fx_elevator                    5
#define fx_scroll_top                  6
#define fx_scroll_left                 7
#define fx_scroll_right                8
#define fx_scroll_bottom               9
#define fx_push_low                    10
#define fx_fire_1                      11
#define fx_fire_2                      12
#define fx_fire_3                      13
#define fx_fire_4                      14
#define fx_fire_5                      15
#define fx_door                        16
#define fx_cool_door                   17
#define fx_blue_glow                   18
#define fx_light                       19
#define fx_blue_stone                  20
#define fx_cool_floor                  21

#define ROOM_606_LOAD_VARIANT          60
#define ROOM_606_RAT_DEATH             80
#define ROOM_606_RAT_TEXT              90

#define PLAYER_X_FROM_603              264
#define PLAYER_Y_FROM_603              377
#define WALK_TO_X_FROM_603             228
#define WALK_TO_Y_FROM_603             377

#define PLAYER_X_FROM_609              169
#define PLAYER_Y_FROM_609              568
#define WALK_TO_X_FROM_609             169
#define WALK_TO_Y_FROM_609             582

#define PLAYER_X_FROM_612              131
#define PLAYER_Y_FROM_612              103
#define WALK_TO_X_FROM_612             172
#define WALK_TO_Y_FROM_612             110

#define PLAYER_X_FROM_607              166
#define PLAYER_Y_FROM_607              351
#define WALK_TO_X_FROM_607             152
#define WALK_TO_Y_FROM_607             364

#define PLAYER_X_FROM_613              261
#define PLAYER_Y_FROM_613              125
#define WALK_TO_X_FROM_613             230
#define WALK_TO_Y_FROM_613             125

#define TOP_FLOOR                      0
#define MIDDLE_FLOOR                   256
#define BOTTOM_FLOOR                   468

#define ELEV_TOP_X                     99
#define ELEV_TOP_Y                     122

#define ELEV_MIDDLE_X                  99
#define ELEV_MIDDLE_Y                  375

#define ELEV_BOTTOM_X                  99
#define ELEV_BOTTOM_Y                  590

#define KING_MIDDLE_ELEV_X             99
#define KING_MIDDLE_ELEV_Y             368

#define KING_TOP_ELEV_X                99
#define KING_TOP_ELEV_Y                115

#define KING_BOTTOM_ELEV_X             99
#define KING_BOTTOM_ELEV_Y             584

#define camera_ratio_1                 1
#define camera_ratio_2                 3

#define RAT_X                          221
#define RAT_Y                          125


static void set_top_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int y;
	int ys;

	center = picture_view_y + (video_y >> 1);

	if (seq[fx_scroll_top] >= 0) {
		kernel_seq_delete(seq[fx_scroll_top]);
	}

	difference = center - local->top_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace = sgn_in(displace, direction);

	y  = local->top_base + displace - 1;
	ys = series_list[ss[fx_scroll_top]]->index[0].ys;

	if (((y - ((ys >> 1) + 1)) >= (picture_view_y + video_y)) ||
	    ((y + ((ys >> 1) + 1)) < picture_view_y)) {
		seq[fx_scroll_top] = -1;

	} else {
		seq[fx_scroll_top] = kernel_seq_stamp(ss[fx_scroll_top], false, 1);
		kernel_seq_loc(seq[fx_scroll_top], 161, y);
		kernel_seq_depth(seq[fx_scroll_top], 1);
	}
}

static void set_bottom_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int y;
	int ys;

	center = picture_view_y + (video_y >> 1);

	if (seq[fx_scroll_bottom] >= 0) {
		kernel_seq_delete(seq[fx_scroll_bottom]);
	}

	difference = center - local->bottom_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace = sgn_in(displace, direction);

	y  = local->bottom_base + displace - 1;
	ys = series_list[ss[fx_scroll_bottom]]->index[0].ys;

	if (((y - ((ys >> 1) + 1)) >= (picture_view_y + video_y)) ||
	    ((y + ((ys >> 1) + 1)) < picture_view_y)) {
		seq[fx_scroll_bottom] = -1;

	} else {
		seq[fx_scroll_bottom] = kernel_seq_stamp(ss[fx_scroll_bottom], false, 1);
		kernel_seq_loc(seq[fx_scroll_bottom], 160, y);
		kernel_seq_depth(seq[fx_scroll_bottom], 1);
	}
}

static void set_stone_position() {
	int difference;
	int y;

	difference = (468 - picture_view_y) / 6;
	y = (series_list[ss[fx_blue_stone]]->index[0].y + 460) - difference;
	kernel_seq_delete(seq[fx_blue_stone]);
	seq[fx_blue_stone] = kernel_seq_stamp(ss[fx_blue_stone], false, 1);
	kernel_seq_loc(seq[fx_blue_stone], 160, y);
	kernel_seq_depth(seq[fx_blue_stone], 4);
}

static void handle_anim_up() {
	int up_reset_frame;

	if (kernel_anim[aa[0]].frame != local->up_frame) {
		local->up_frame = kernel_anim[aa[0]].frame;
		up_reset_frame = -1;

		switch (local->up_frame) {

		case 290:
			camera_pan_to(&camera_y, picture_view_y - 155);
			break;

		case 81:
			camera_pan_to(&camera_y, picture_view_y - 155);
			break;

		case 184:
			camera_pan_to(&camera_y, MIDDLE_FLOOR);
			break;

		case 420:
			camera_pan_to(&camera_y, TOP_FLOOR);
			break;

		case 1:
		case 217:
			kernel_seq_delete(seq[fx_elevator]);
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_NOW, 0);
			break;

		case 469:
			sound_play(N_ElevatorOff);
			kernel_abort_animation(aa[0]);
			local->on_floor         = 3;
			local->anim_0_running   = false;
			player.walker_visible   = true;
			up_reset_frame          = -1;
			player_demand_location(KING_TOP_ELEV_X, KING_TOP_ELEV_Y);
			player_demand_facing(FACING_SOUTH);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);

			kernel_room_scale(145, 85, 103, 67);

			seq[fx_elevator] = kernel_seq_stamp(ss[fx_elevator], false, 1);
			kernel_seq_depth(seq[fx_elevator], 14);
			kernel_seq_loc(seq[fx_elevator], ELEV_TOP_X, ELEV_TOP_Y);
			kernel_synch(KERNEL_SERIES, seq[fx_elevator], KERNEL_NOW, 0);
			kernel_timing_trigger(TENTH_SECOND, ROOM_606_LOAD_VARIANT);

			if (global[rat_cage_is_open] && !global[rat_melted]) {
				aa[2] = kernel_run_animation(kernel_name('r', 1), ROOM_606_RAT_DEATH);
			} else {
				player.commands_allowed = true;
			}
			break;

		case 215:
			sound_play(N_ElevatorOff);
			kernel_abort_animation(aa[0]);
			local->on_floor         = 2;
			local->anim_0_running   = false;
			player.walker_visible   = true;
			player.commands_allowed = true;
			up_reset_frame          = -1;
			player_demand_location(KING_MIDDLE_ELEV_X, KING_MIDDLE_ELEV_Y);
			player_demand_facing(FACING_SOUTH);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);

			kernel_room_scale(401, 85, 359, 67);

			seq[fx_elevator] = kernel_seq_stamp(ss[fx_elevator], false, 1);
			kernel_seq_depth(seq[fx_elevator], 14);
			kernel_seq_loc(seq[fx_elevator], ELEV_MIDDLE_X, ELEV_MIDDLE_Y);
			kernel_synch(KERNEL_SERIES, seq[fx_elevator], KERNEL_NOW, 0);
			kernel_timing_trigger(TENTH_SECOND, ROOM_606_LOAD_VARIANT);
			break;
		}

		if (up_reset_frame >= 0) {
			kernel_reset_animation(aa[0], up_reset_frame);
			local->up_frame = up_reset_frame;
		}
	}
}

static void handle_anim_down() {
	int down_reset_frame;

	if (kernel_anim[aa[1]].frame != local->down_frame) {
		local->down_frame = kernel_anim[aa[1]].frame;
		down_reset_frame = -1;

		switch (local->down_frame) {

		case 21:
			camera_pan_to(&camera_y, picture_view_y + 100);
			break;

		case 113:
			camera_pan_to(&camera_y, BOTTOM_FLOOR);
			break;

		case 245:
			camera_pan_to(&camera_y, picture_view_y + 120);
			break;

		case 370:
			camera_pan_to(&camera_y, MIDDLE_FLOOR);
			break;

		case 217:
		case 1:
			kernel_seq_delete(seq[fx_elevator]);
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_NOW, 0);
			break;

		case 470:
			sound_play(N_ElevatorOff);
			kernel_abort_animation(aa[1]);
			local->on_floor         = 2;
			local->anim_1_running   = false;
			player.walker_visible   = true;
			player.commands_allowed = true;
			down_reset_frame        = -1;
			player_demand_location(KING_MIDDLE_ELEV_X, KING_MIDDLE_ELEV_Y);
			player_demand_facing(FACING_SOUTH);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);

			kernel_room_scale(401, 85, 359, 67);

			seq[fx_elevator] = kernel_seq_stamp(ss[fx_elevator], false, 1);
			kernel_seq_depth(seq[fx_elevator], 14);
			kernel_seq_loc(seq[fx_elevator], ELEV_MIDDLE_X, ELEV_MIDDLE_Y);
			kernel_synch(KERNEL_SERIES, seq[fx_elevator], KERNEL_NOW, 0);
			kernel_timing_trigger(TENTH_SECOND, ROOM_606_LOAD_VARIANT);
			break;

		case 216:
			sound_play(N_ElevatorOff);
			kernel_abort_animation(aa[1]);
			local->on_floor         = 1;
			local->anim_1_running   = false;
			player.walker_visible   = true;
			player.commands_allowed = true;
			down_reset_frame        = -1;
			player_demand_location(KING_BOTTOM_ELEV_X, KING_BOTTOM_ELEV_Y);
			player_demand_facing(FACING_SOUTH);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);

			kernel_room_scale(613, 85, 571, 67);

			seq[fx_elevator] = kernel_seq_stamp(ss[fx_elevator], false, 1);
			kernel_seq_depth(seq[fx_elevator], 14);
			kernel_seq_loc(seq[fx_elevator], ELEV_BOTTOM_X, ELEV_BOTTOM_Y);
			kernel_synch(KERNEL_SERIES, seq[fx_elevator], KERNEL_NOW, 0);
			kernel_timing_trigger(TENTH_SECOND, ROOM_606_LOAD_VARIANT);
			break;
		}

		if (down_reset_frame >= 0) {
			kernel_reset_animation(aa[1], down_reset_frame);
			local->down_frame = down_reset_frame;
		}
	}
}

static void handle_anim_suction() {
	int suction_reset_frame;

	if (kernel_anim[aa[3]].frame != local->suction_frame) {
		local->suction_frame = kernel_anim[aa[3]].frame;
		suction_reset_frame = -1;

		switch (local->suction_frame) {
		case 11:
			kernel_seq_delete(seq[fx_cool_door]);
			kernel_seq_delete(seq[fx_door]);
			kernel_flip_hotspot(words_teleportal, false);
			kernel_flip_hotspot(words_door_frame, false);
			break;

		case 46:
			inter_give_to_player(teleport_door);
			object_examine(teleport_door, 60629, 0);
			kernel_abort_animation(aa[3]);
			global[player_score]    += 1;
			suction_reset_frame     = -1;
			player.commands_allowed = true;
			player.walker_visible   = true;
			local->anim_3_running   = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			break;
		}

		if (suction_reset_frame >= 0) {
			kernel_reset_animation(aa[3], suction_reset_frame);
			local->suction_frame = suction_reset_frame;
		}
	}
}

static void room_606_init() {
	local->anim_0_running = false;
	local->anim_1_running = false;
	local->anim_3_running = false;

	ss[fx_cool_door] = kernel_load_series(kernel_name('z', 0), false);

	if (object_is_here(teleport_door)) {
		if (global[door_is_cool]) {
			seq[fx_cool_door] = kernel_seq_stamp(ss[fx_cool_door], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_cool_door], 8);
		}
	}

	if (global[floor_is_cool]) {
		ss[fx_cool_floor]  = kernel_load_series(kernel_name('x', 7), false);
		seq[fx_cool_floor] = kernel_seq_stamp(ss[fx_cool_floor], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_cool_floor], 14);
	}

	ss[fx_fire_1] = kernel_load_series(kernel_name('y', 0), false);
	ss[fx_fire_2] = kernel_load_series(kernel_name('y', 1), false);
	ss[fx_fire_3] = kernel_load_series(kernel_name('y', 2), false);
	ss[fx_fire_4] = kernel_load_series(kernel_name('y', 3), false);
	ss[fx_fire_5] = kernel_load_series(kernel_name('y', 4), false);

	seq[fx_fire_1] = kernel_seq_forward(ss[fx_fire_1], false, 6, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_1], 14);
	kernel_seq_range(seq[fx_fire_1], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_2] = kernel_seq_forward(ss[fx_fire_2], false, 6, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_2], 14);
	kernel_seq_range(seq[fx_fire_2], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_3] = kernel_seq_forward(ss[fx_fire_3], false, 6, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_3], 14);
	kernel_seq_range(seq[fx_fire_3], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_4] = kernel_seq_forward(ss[fx_fire_4], false, 6, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_4], 14);
	kernel_seq_range(seq[fx_fire_4], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_5] = kernel_seq_forward(ss[fx_fire_5], false, 6, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_5], 14);
	kernel_seq_range(seq[fx_fire_5], KERNEL_FIRST, KERNEL_LAST);

	ss[fx_elevator]        = kernel_load_series(kernel_name('x', 4), false);
	ss[fx_push_high]       = kernel_load_series(kernel_name('a', 0), false);
	ss[fx_push_low]        = kernel_load_series(kernel_name('a', 1), false);
	ss[fx_but_top]         = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_but_bottom]      = kernel_load_series(kernel_name('x', 3), false);
	ss[fx_but_middle_high] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_but_middle_low]  = kernel_load_series(kernel_name('x', 2), false);

	ss[fx_scroll_top]    = kernel_load_series(kernel_name('r', 0), false);
	ss[fx_scroll_bottom] = kernel_load_series(kernel_name('r', 1), false);

	if (global[torch_is_in_609]) {
		ss[fx_light]  = kernel_load_series(kernel_name('x', 6), false);
		seq[fx_light] = kernel_seq_stamp(ss[fx_light], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_light], 9);
	}

	if (!player_has(blue_powerstone)) {
		ss[fx_blue_stone] = kernel_load_series(kernel_name('p', 3), false);
		ss[fx_blue_glow]  = kernel_load_series(kernel_name('p', 1), false);
		seq[fx_blue_glow] = kernel_seq_stamp(ss[fx_blue_glow], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_blue_glow], 10);
	}

	if (object_is_here(teleport_door)) {
		ss[fx_door]  = kernel_load_series(kernel_name('p', 0), false);
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 9);

	} else {
		kernel_flip_hotspot(words_teleportal, false);
		kernel_flip_hotspot(words_door_frame, false);
	}

	seq[fx_elevator] = kernel_seq_stamp(ss[fx_elevator], false, 1);
	kernel_seq_depth(seq[fx_elevator], 14);

	if (previous_room == 612) {
		player.x      = PLAYER_X_FROM_612;
		player.y      = PLAYER_Y_FROM_612;
		player.facing = FACING_EAST;

		player_walk(WALK_TO_X_FROM_612, WALK_TO_Y_FROM_612, FACING_SOUTH);
		camera_jump_to(0, TOP_FLOOR);
		kernel_room_scale(145, 85, 103, 67);
		local->on_floor = 3;
		kernel_seq_loc(seq[fx_elevator], ELEV_TOP_X, ELEV_TOP_Y);

	} else if (previous_room == 603) {
		player_first_walk(PLAYER_X_FROM_603, PLAYER_Y_FROM_603, FACING_WEST,
		                  WALK_TO_X_FROM_603, WALK_TO_Y_FROM_603, FACING_WEST, true);
		camera_jump_to(0, MIDDLE_FLOOR);
		kernel_room_scale(401, 85, 359, 67);
		local->on_floor = 2;
		kernel_seq_loc(seq[fx_elevator], ELEV_MIDDLE_X, ELEV_MIDDLE_Y);

	} else if (previous_room == 607) {
		player_first_walk(PLAYER_X_FROM_607, PLAYER_Y_FROM_607, FACING_SOUTH,
		                  WALK_TO_X_FROM_607, WALK_TO_Y_FROM_607, FACING_SOUTH, true);
		camera_jump_to(0, MIDDLE_FLOOR);
		kernel_room_scale(401, 85, 359, 67);
		local->on_floor = 2;
		kernel_seq_loc(seq[fx_elevator], ELEV_MIDDLE_X, ELEV_MIDDLE_Y);

	} else if (previous_room == 613) {
		if (!object_is_here(teleport_door)) {
			if (!global[floor_is_cool]) {
				player.x      = WALK_TO_X_FROM_613;
				player.y      = WALK_TO_Y_FROM_613;
				player.facing = FACING_WEST;

			} else {
				player_first_walk(PLAYER_X_FROM_613, PLAYER_Y_FROM_613, FACING_WEST,
				                  WALK_TO_X_FROM_613, WALK_TO_Y_FROM_613, FACING_WEST, true);
			}
		}
		camera_jump_to(0, TOP_FLOOR);
		kernel_room_scale(145, 85, 103, 67);
		local->on_floor = 3;
		kernel_seq_loc(seq[fx_elevator], ELEV_TOP_X, ELEV_TOP_Y);
		kernel_timing_trigger(1, ROOM_606_RAT_TEXT);

	} else if ((previous_room == 609) || (previous_room != KERNEL_RESTORING_GAME)) {
		player_first_walk(PLAYER_X_FROM_609, PLAYER_Y_FROM_609, FACING_SOUTH,
		                  WALK_TO_X_FROM_609, WALK_TO_Y_FROM_609, FACING_SOUTH, true);
		camera_jump_to(0, BOTTOM_FLOOR);
		kernel_room_scale(613, 85, 571, 67);
		local->on_floor = 1;
		kernel_seq_loc(seq[fx_elevator], ELEV_BOTTOM_X, ELEV_BOTTOM_Y);

	} else if (previous_room == KERNEL_RESTORING_GAME) {

		if (local->on_floor == 2) {
			camera_jump_to(0, MIDDLE_FLOOR);
			kernel_room_scale(401, 85, 359, 67);
			kernel_seq_loc(seq[fx_elevator], ELEV_MIDDLE_X, ELEV_MIDDLE_Y);

		} else if (local->on_floor == 3) {
			camera_jump_to(0, TOP_FLOOR);
			kernel_room_scale(145, 85, 103, 67);
			kernel_seq_loc(seq[fx_elevator], ELEV_TOP_X, ELEV_TOP_Y);

		} else {
			camera_jump_to(0, BOTTOM_FLOOR);
			kernel_room_scale(613, 85, 571, 67);
			kernel_seq_loc(seq[fx_elevator], ELEV_BOTTOM_X, ELEV_BOTTOM_Y);
		}
	}

	seq[fx_scroll_top]    = -1;
	local->dyn_top        = -1;
	local->top_base       = 68;

	seq[fx_scroll_bottom] = -1;
	local->dyn_bottom     = -1;
	local->bottom_base    = 610;

	seq[fx_blue_stone]     = -1;
	local->dyn_blue_stone  = -1;
	local->blue_stone_base = 540;

	if (!player_has(blue_powerstone)) {
		seq[fx_blue_stone] = kernel_seq_stamp(ss[fx_blue_stone], false, 1);
		kernel_seq_loc(seq[fx_blue_stone], 160, 550);
		kernel_seq_depth(seq[fx_blue_stone], 4);
	}

	set_top_position();
	set_bottom_position();

	section_6_music();
}

static void room_606_daemon() {
	if (local->anim_0_running) {
		handle_anim_up();
	}

	if (local->anim_1_running) {
		handle_anim_down();
	}

	if (local->anim_3_running) {
		handle_anim_suction();
	}

	if (kernel.trigger == ROOM_606_LOAD_VARIANT) {
		kernel_load_variant(0);
	}

	if (camera_y.pan_this_frame) {
		set_top_position();
		set_bottom_position();
		if (!player_has(blue_powerstone)) {
			set_stone_position();
		}
	}

	if (kernel.trigger == ROOM_606_RAT_DEATH) {
		new_room = 613;
	}

	if (kernel.trigger == ROOM_606_RAT_TEXT) {
		if (global[rat_melted]) {
			text_show(60624);
			global[rat_melted] = 1;
		}
	}
}

static void room_606_pre_parser() {
	if (player_said_2(walk_through, doorway_to_dark_room)) {
		if (!player_has(torch) && !global[torch_is_in_609]) {
			text_show(60644);
			player_cancel_command();
		}
	}

	if (player_said_1(doorway) && player.need_to_walk) {
		if (!player_said_1(walk_through)) {
			player.need_to_walk = false;
		}
	}

	if (player_said_1(doorway_to_cell) && player.need_to_walk) {
		if (!player_said_1(walk_through)) {
			player.need_to_walk = false;
		}
	}

	if (player_said_1(machine_room_doorway) && player.need_to_walk) {
		if (!player_said_1(walk_through)) {
			player.need_to_walk = false;
		}
	}

	if (player_said_1(doorway_to_dark_room) && player.need_to_walk) {
		if (!player_said_1(walk_through)) {
			player.need_to_walk = false;
		}
	}

	if (player_said_1(eye_chamber_doorway) && player.need_to_walk) {
		if (!player_said_1(walk_through)) {
			player.need_to_walk = false;
		}
	}
}

static void room_606_parser() {
	int id;

	if (player_said_3(put, ratsicle, door_frame)) {
		switch (kernel.trigger) {
		case 0:
			player.walker_visible   = false;
			player.commands_allowed = false;
			aa[2]                   = kernel_run_animation(kernel_name('w', 1), 1);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_PLAYER, 0);
			player.command_ready = false;
			return;

		case 1:
			kernel_abort_animation(aa[2]);
			seq[fx_cool_door] = kernel_seq_stamp(ss[fx_cool_door], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_cool_door], 8);
			kernel_synch(KERNEL_SERIES, seq[fx_cool_door], KERNEL_NOW, 0);
			global[player_score]    += 5;
			global[door_is_cool]    = true;
			player.walker_visible   = true;
			player.commands_allowed = true;
			inter_move_object(ratsicle, NOWHERE);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			text_show(60633);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(take, teleportal) ||
	    player_said_2(pull, teleportal) ||
	    player_said_2(take, door_frame) ||
	    player_said_2(pull, door_frame)) {
		if (global[door_is_cool]) {
			text_show(60628);
			player.command_ready = false;
			return;

		} else {
			text_show(60614);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_3(put, tentacle_parts, teleportal) ||
	    player_said_3(put, tentacle_parts, door_frame)) {
		if (global[door_is_cool]) {
			player.walker_visible   = false;
			player.commands_allowed = false;
			local->anim_3_running   = true;
			aa[3]                   = kernel_run_animation(kernel_name('t', 1), 0);
			global[player_score]   += 5;
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_PLAYER, 0);
			player.command_ready = false;
			return;

		} else {
			text_show(60635);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(walk_through, doorway_to_dark_room)) {
		if (!(global[player_score_flags] & SCORE_ENTER_609)) {
			global[player_score_flags] = global[player_score_flags] | SCORE_ENTER_609;
			global[player_score] += 1;
		}

		new_room = 609;
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_through, eye_chamber_doorway)) {
		new_room = 603;
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_through, machine_room_doorway)) {
		new_room = 612;
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_through, doorway_to_cell)) {
		new_room = 607;
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_through, doorway)) {
		new_room = 613;
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_through, teleportal)) {
		switch (kernel.trigger) {
		case 0:
			player.walker_visible   = false;
			player.commands_allowed = false;
			aa[2]                   = kernel_run_animation(kernel_name('k', 1), 1);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_PLAYER, 0);
			break;

		case 1:
			new_room = 613;
			break;
		}
		player.command_ready = false;
		return;
	}

	if (local->on_floor == 1) {
		if (player_said_2(push, button)) {
			switch (kernel.trigger) {
			case 0:
				kernel_load_variant(1);
				global[used_elevator]   = true;
				player.walker_visible   = false;
				player.commands_allowed = false;
				seq[fx_push_high]       = kernel_seq_pingpong(ss[fx_push_high], false, 7, 0, 0, 2);
				kernel_seq_depth(seq[fx_push_high], 2);
				kernel_seq_range(seq[fx_push_high], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_push_high], KERNEL_TRIGGER_EXPIRE, 0, 1);
				kernel_seq_trigger(seq[fx_push_high], KERNEL_TRIGGER_SPRITE, 6, 2);
				kernel_seq_player(seq[fx_push_high], false);
				kernel_synch(KERNEL_SERIES, seq[fx_push_high], KERNEL_PLAYER, 0);
				break;

			case 1:
				if (!global[platform_clicked_606]) {
					text_show(60610);
				}
				sound_play(N_ElevatorOn);
				global[platform_clicked_606] = true;
				local->anim_0_running        = true;
				aa[0]                        = kernel_run_animation(kernel_name('u', 1), 0);
				kernel_synch(KERNEL_ANIM, aa[0], KERNEL_SERIES, seq[fx_push_high]);
				break;

			case 2:
				sound_play(N_ElevatorButton);
				seq[fx_but_bottom] = kernel_seq_pingpong(ss[fx_but_bottom], false, 7, 0, 0, 2);
				kernel_seq_depth(seq[fx_but_bottom], 10);
				kernel_seq_range(seq[fx_but_bottom], KERNEL_FIRST, KERNEL_LAST);
				break;
			}
			player.command_ready = false;
			return;
		}

	} else if (local->on_floor == 2) {
		if (player_said_2(push, top_button)) {
			switch (kernel.trigger) {
			case 0:
				kernel_load_variant(1);
				global[used_elevator]   = true;
				player.walker_visible   = false;
				player.commands_allowed = false;
				seq[fx_push_high]       = kernel_seq_pingpong(ss[fx_push_high], false, 7, 0, 0, 2);
				kernel_seq_depth(seq[fx_push_high], 2);
				kernel_seq_range(seq[fx_push_high], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_push_high], KERNEL_TRIGGER_EXPIRE, 0, 1);
				kernel_seq_trigger(seq[fx_push_high], KERNEL_TRIGGER_SPRITE, 6, 2);
				kernel_seq_player(seq[fx_push_high], false);
				kernel_synch(KERNEL_SERIES, seq[fx_push_high], KERNEL_PLAYER, 0);
				break;

			case 1:
				if (!global[platform_clicked_606]) {
					text_show(60610);
				}
				sound_play(N_ElevatorOn);
				global[platform_clicked_606] = true;
				local->anim_0_running   = true;
				aa[0]                   = kernel_run_animation(kernel_name('u', 1), 0);
				kernel_reset_animation(aa[0], 217);
				kernel_synch(KERNEL_ANIM, aa[0], KERNEL_SERIES, seq[fx_push_high]);
				break;

			case 2:
				sound_play(N_ElevatorButton);
				seq[fx_but_middle_high] = kernel_seq_pingpong(ss[fx_but_middle_high], false, 7, 0, 0, 2);
				kernel_seq_depth(seq[fx_but_middle_high], 10);
				kernel_seq_range(seq[fx_but_middle_high], KERNEL_FIRST, KERNEL_LAST);
				break;
			}
			player.command_ready = false;
			return;

		} else if (player_said_2(push, bottom_button)) {
			switch (kernel.trigger) {
			case 0:
				kernel_load_variant(1);
				global[used_elevator]   = true;
				player.walker_visible   = false;
				player.commands_allowed = false;
				seq[fx_push_low]        = kernel_seq_pingpong(ss[fx_push_low], false, 7, 0, 0, 2);
				kernel_seq_depth(seq[fx_push_low], 2);
				kernel_seq_range(seq[fx_push_low], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_push_low], KERNEL_TRIGGER_EXPIRE, 0, 1);
				kernel_seq_trigger(seq[fx_push_low], KERNEL_TRIGGER_SPRITE, 6, 2);
				kernel_seq_player(seq[fx_push_low], false);
				kernel_synch(KERNEL_SERIES, seq[fx_push_low], KERNEL_PLAYER, 0);
				break;

			case 1:
				if (!global[platform_clicked_606]) {
					text_show(60610);
				}
				sound_play(N_ElevatorOn);
				global[platform_clicked_606] = true;
				local->anim_1_running   = true;
				aa[1]                   = kernel_run_animation(kernel_name('d', 1), 0);
				kernel_synch(KERNEL_ANIM, aa[1], KERNEL_SERIES, seq[fx_push_low]);
				break;

			case 2:
				sound_play(N_ElevatorButton);
				seq[fx_but_middle_low] = kernel_seq_pingpong(ss[fx_but_middle_low], false, 7, 0, 0, 2);
				kernel_seq_depth(seq[fx_but_middle_low], 10);
				kernel_seq_range(seq[fx_but_middle_low], KERNEL_FIRST, KERNEL_LAST);
				break;
			}
			player.command_ready = false;
			return;
		}

	} else if (local->on_floor == 3) {
		if (player_said_2(push, button)) {
			switch (kernel.trigger) {
			case 0:
				kernel_load_variant(1);
				global[used_elevator]   = true;
				player.walker_visible   = false;
				player.commands_allowed = false;
				seq[fx_push_high]       = kernel_seq_pingpong(ss[fx_push_high], false, 7, 0, 0, 2);
				kernel_seq_depth(seq[fx_push_high], 2);
				kernel_seq_range(seq[fx_push_high], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_push_high], KERNEL_TRIGGER_EXPIRE, 0, 1);
				kernel_seq_trigger(seq[fx_push_high], KERNEL_TRIGGER_SPRITE, 6, 2);
				kernel_seq_player(seq[fx_push_high], false);
				kernel_synch(KERNEL_SERIES, seq[fx_push_high], KERNEL_PLAYER, 0);
				break;

			case 1:
				if (!global[platform_clicked_606]) {
					text_show(60610);
				}
				sound_play(N_ElevatorOn);
				global[platform_clicked_606] = true;
				local->anim_1_running   = true;
				aa[1]                   = kernel_run_animation(kernel_name('d', 1), 0);
				kernel_reset_animation(aa[1], 217);
				kernel_synch(KERNEL_ANIM, aa[1], KERNEL_SERIES, seq[fx_push_high]);
				break;

			case 2:
				sound_play(N_ElevatorButton);
				seq[fx_but_top] = kernel_seq_pingpong(ss[fx_but_top], false, 7, 0, 0, 2);
				kernel_seq_depth(seq[fx_but_top], 10);
				kernel_seq_range(seq[fx_but_top], KERNEL_FIRST, KERNEL_LAST);
				break;
			}
			player.command_ready = false;
			return;
		}
	}

	if (player.look_around) {
		switch (local->on_floor) {
		case 1:
			if (global[torch_is_in_609] || player_has(torch)) {
				text_show(60602);
			} else {
				text_show(60601);
			}
			break;

		case 2:
			if (global[used_elevator]) {
				text_show(60627);
			} else {
				text_show(60603);
			}
			break;

		case 3:
			if (object_is_here(teleport_door) && global[door_is_cool]) {
				text_show(60605);
			} else if (object_is_here(teleport_door) && !global[door_is_cool]) {
				text_show(60604);
			} else if (!object_is_here(teleport_door)) {
				text_show(60606);
			}
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(eye_chamber_doorway)) {
			text_show(60607);
			player.command_ready = false;
			return;
		}

		if (player_said_1(doorway)) {
			text_show(60642);
			player.command_ready = false;
			return;
		}

		if (player_said_1(button) ||
		    player_said_1(top_button) ||
		    player_said_1(bottom_button)) {

			if (global[used_elevator]) {
				text_show(60609);
			} else {
				text_show(60608);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(big_skull)) {
			text_show(60640);
			player.command_ready = false;
			return;
		}

		if (player_said_1(skull)) {
			text_show(60613);
			player.command_ready = false;
			return;
		}

		if (player_said_1(machine_room_doorway)) {
			text_show(60619);
			player.command_ready = false;
			return;
		}

		if (player_said_1(doorway_to_cell)) {
			text_show(60620);
			player.command_ready = false;
			return;
		}

		if (player_said_1(window)) {
			text_show(60621);
			player.command_ready = false;
			return;
		}

		if (player_said_1(teleportal)) {
			if (global[rat_cage_is_open]) {
				text_show(60626);
			} else {
				text_show(60625);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(door_frame)) {
			if (global[door_is_cool]) {
				text_show(60631);
			} else {
				text_show(60630);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(doorway_to_dark_room)) {
			if (global[torch_is_in_609] || player_has(torch)) {
				text_show(60637);
			} else {
				text_show(60636);
			}
			player.command_ready = false;
			return;
		}
	}

	if (player_said_1(big_skull) ||
	    player_said_1(skull)) {
		text_show(60612);
		player.command_ready = false;
		return;
	}

	if (player_said_2(sword, attack) ||
	    player_said_2(sword, carve_up) ||
	    player_said_2(sword, thrust)) {

		if (player_said_1(button) ||
		    player_said_1(top_button) ||
		    player_said_1(bottom_button)) {

			text_show(60611);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(open, window)) {
		text_show(60622);
		player.command_ready = false;
		return;
	}

	if (player_said_2(throw, window)) {
		text_show(60623);
		player.command_ready = false;
		return;
	}

	if (player_said_3(put, flask_of_acid, door_frame) ||
	    player_said_2(pour_contents_of, door_frame)) {
		text_show(60639);
		player.command_ready = false;
		return;
	}

	id = object_named(player_main_noun);

	if (player_said_2(put, door_frame) && player_has(id)) {
		if (!global[door_is_cool]) {
			text_show(60645);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(put, teleportal)) {
		text_show(60643);
		player.command_ready = false;
		return;
	}
}

void room_606_synchronize(Common::Serializer &s) {
	for (int i = 0; i < 22; i++) s.syncAsSint16LE(local->sprite[i]);
	for (int i = 0; i < 22; i++) s.syncAsSint16LE(local->sequence[i]);
	for (int i = 0; i < 4; i++)  s.syncAsSint16LE(local->animation[i]);
	s.syncAsSint16LE(local->on_floor);
	s.syncAsSint16LE(local->up_frame);
	s.syncAsSint16LE(local->anim_0_running);
	s.syncAsSint16LE(local->down_frame);
	s.syncAsSint16LE(local->anim_1_running);
	s.syncAsSint16LE(local->suction_frame);
	s.syncAsSint16LE(local->anim_3_running);
	s.syncAsSint16LE(local->top_base);
	s.syncAsSint16LE(local->dyn_top);
	s.syncAsSint16LE(local->bottom_base);
	s.syncAsSint16LE(local->dyn_bottom);
	s.syncAsSint16LE(local->blue_stone_base);
	s.syncAsSint16LE(local->dyn_blue_stone);
}

void room_606_preload() {
	room_init_code_pointer = room_606_init;
	room_pre_parser_code_pointer = room_606_pre_parser;
	room_parser_code_pointer = room_606_parser;
	room_daemon_code_pointer = room_606_daemon;

	section_6_walker();
	section_6_interface();

	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
