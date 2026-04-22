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
#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/rooms/room109.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_109_init() {
	local->anim_0_running = false;
	local->anim_1_running = false;
	local->anim_2_running = false;
	local->anim_3_running = false;

	/* ==================== Load Sprite Series ====================== */

	ss[fx_top_door]    = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_middle_door] = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_take_6]      = kernel_load_series("*RDR_6", false);


	/* ================ Start Flame Sprites or Lightbulbs =========== */

	if (global[current_year] == 1881) {

		ss[fx_top_flame]    = kernel_load_series(kernel_name('x', 1), false);
		ss[fx_middle_flame] = kernel_load_series(kernel_name('x', 3), false);
		ss[fx_bottom_flame] = kernel_load_series(kernel_name('x', 4), false);

		kernel_flip_hotspot(words_light_fixture, false);
		seq[fx_top_flame] = kernel_seq_stamp(ss[fx_top_flame], false, 1);
		kernel_seq_depth(seq[fx_top_flame], 14);
		seq[fx_middle_flame] = kernel_seq_stamp(ss[fx_middle_flame], false, 1);
		kernel_seq_depth(seq[fx_middle_flame], 14);
		seq[fx_bottom_flame] = kernel_seq_stamp(ss[fx_bottom_flame], false, 1);
		kernel_seq_depth(seq[fx_bottom_flame], 14);
		kernel_timing_trigger(1, ROOM_109_CHANGE_LANTERN_FLAME);

	} else {    /* current year is 1993 */

		ss[fx_top_lester]    = kernel_load_series(kernel_name('z', 0), false);
		ss[fx_middle_lester] = kernel_load_series(kernel_name('z', 1), false);
		ss[fx_bottom_lester] = kernel_load_series(kernel_name('z', 2), false);

		kernel_flip_hotspot(words_lamp, false);
		seq[fx_top_lester]    = kernel_seq_stamp(ss[fx_top_lester], false, 1);
		seq[fx_middle_lester] = kernel_seq_stamp(ss[fx_middle_lester], false, 1);
		seq[fx_bottom_lester] = kernel_seq_stamp(ss[fx_bottom_lester], false, 1);
	}


	/* ====================== Previous Rooms ======================== */

	if (previous_room == 106) {
		player.x      = PLAYER_X_FROM_106;
		player.y      = PLAYER_Y_FROM_106;
		player.facing = FACING_NORTHEAST;
		camera_jump_to(0, BOTTOM_FLOOR);
		kernel_room_scale(467, 95, 442, 78);
		local->on_floor = 1;

	} else if (previous_room == 111) {
		if (global[leave_angel_music_on]) {
			global[leave_angel_music_on] = false;
			section_1_music();
		}
		player.x      = PLAYER_X_FROM_111;
		player.y      = PLAYER_Y_FROM_111;
		player.facing = FACING_EAST;
		player_walk(WALK_TO_X_FROM_111, WALK_TO_Y_FROM_111, FACING_EAST);
		player_walk_trigger(ROOM_109_TOP_DOOR_CLOSES);
		player.commands_allowed = false;
		camera_jump_to(0, TOP_FLOOR);
		kernel_room_scale(155, 95, 130, 78);
		seq[fx_top_door] = kernel_seq_stamp(ss[fx_top_door], false, 3);
		kernel_seq_depth(seq[fx_top_door], 14);
		local->on_floor = 3;

	} else if ((previous_room == 110) || (previous_room != KERNEL_RESTORING_GAME)) {
		player.x      = PLAYER_X_FROM_110;
		player.y      = PLAYER_Y_FROM_110;
		player.facing = FACING_EAST;
		player_walk(WALK_TO_X_FROM_110, WALK_TO_Y_FROM_110, FACING_EAST);
		player_walk_trigger(ROOM_109_MIDDLE_DOOR_CLOSES);
		player.commands_allowed = false;
		camera_jump_to(0, MIDDLE_FLOOR);
		kernel_room_scale(311, 95, 286, 78);
		seq[fx_middle_door] = kernel_seq_stamp(ss[fx_middle_door], false, 3);
		kernel_seq_depth(seq[fx_middle_door], 14);
		local->on_floor = 2;

	} else if (previous_room == KERNEL_RESTORING_GAME) {
		if (local->on_floor == 2) {
			camera_jump_to(0, MIDDLE_FLOOR);
			kernel_room_scale(311, 95, 286, 78);
		} else if (local->on_floor == 3) {
			camera_jump_to(0, TOP_FLOOR);
			kernel_room_scale(155, 95, 130, 78);
		} else {
			camera_jump_to(0, BOTTOM_FLOOR);
			kernel_room_scale(467, 95, 442, 78);
		}
	}

	section_1_music();
}

void room_109_daemon() {
	int flame_frame;      /* Random # for lamp frame will go here */
	int flame_delay;      /* Random # for flame delay will go here */

	if (local->anim_0_running) {
		if (kernel_anim[aa[0]].frame == 80) {
			camera_pan_to(&camera_y, MIDDLE_FLOOR);
		}
	}

	if (local->anim_1_running) {
		if (kernel_anim[aa[1]].frame == 80) {
			camera_pan_to(&camera_y, TOP_FLOOR);
		}
	}

	if (local->anim_2_running) {
		if (kernel_anim[aa[2]].frame == 7) {
			camera_pan_to(&camera_y, BOTTOM_FLOOR);
		}
	}

	if (local->anim_3_running) {
		if (kernel_anim[aa[3]].frame == 14) {
			camera_pan_to(&camera_y, MIDDLE_FLOOR);
		}
	}

	/* ========= Close the top door when player enters top floor ===== */

	switch (kernel.trigger) {
	case ROOM_109_TOP_DOOR_CLOSES:
		kernel_seq_delete(seq[fx_top_door]);
		seq[fx_top_door] = kernel_seq_backward(ss[fx_top_door], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_top_door], 14);
		kernel_seq_range(seq[fx_top_door], 1, 3);
		kernel_seq_trigger(seq[fx_top_door],
		                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_109_TOP_DOOR_CLOSES + 1);
		break;

	case ROOM_109_TOP_DOOR_CLOSES + 1:
		sound_play(N_DoorCloses);
		player.commands_allowed = true;
		break;
	}


	/* ==== Close middle door when player enters middle floor ======= */

	switch (kernel.trigger) {
	case ROOM_109_MIDDLE_DOOR_CLOSES:
		kernel_seq_delete(seq[fx_middle_door]);
		seq[fx_middle_door] = kernel_seq_backward(ss[fx_middle_door], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_middle_door], 14);
		kernel_seq_range(seq[fx_middle_door], 1, 3);
		kernel_seq_trigger(seq[fx_middle_door],
		                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_109_MIDDLE_DOOR_CLOSES + 1);
		break;

	case ROOM_109_MIDDLE_DOOR_CLOSES + 1:
		sound_play(N_DoorCloses);
		player.commands_allowed = true;
		break;
	}


	/* ========= Randomly flicker the lamps on ALL walls ============ */

	switch (kernel.trigger) {
	case ROOM_109_CHANGE_LANTERN_FLAME:
		flame_frame = imath_random(1, 3);
		flame_delay = imath_random(4, 7);
		kernel_seq_delete(seq[fx_top_flame]);
		kernel_seq_delete(seq[fx_middle_flame]);
		kernel_seq_delete(seq[fx_bottom_flame]);

		seq[fx_top_flame] = kernel_seq_stamp(ss[fx_top_flame], false, flame_frame);
		kernel_seq_depth(seq[fx_top_flame], 14);

		seq[fx_middle_flame] = kernel_seq_stamp(ss[fx_middle_flame], false, flame_frame);
		kernel_seq_depth(seq[fx_middle_flame], 14);

		seq[fx_bottom_flame] = kernel_seq_stamp(ss[fx_bottom_flame], false, flame_frame);
		kernel_seq_depth(seq[fx_bottom_flame], 14);

		kernel_timing_trigger(flame_delay, ROOM_109_CHANGE_LANTERN_FLAME);
		break;
	}
}

void room_109_pre_parser() {
	if (player_said_2(open, door)) {
		if (local->on_floor == 3) {
			player_walk(WALK_TO_X_FROM_111, WALK_TO_Y_FROM_111, FACING_WEST);
		} else if (local->on_floor == 2) {
			player_walk(WALK_TO_X_FROM_110, WALK_TO_Y_FROM_110, FACING_WEST);
		}
	}
}

void room_109_parser() {
	int temp;  /* For synching purposes */

	if (player_said_2(walk, backstage)) {
		new_room = 106;
		goto handled;
	}

	if (player_said_2(climb_up, staircase)) {
		if (local->on_floor == 2) {
			switch (kernel.trigger) {
			case 0:
				player_walk(START_CLIMB_MIDDLE_X, START_CLIMB_MIDDLE_Y, FACING_EAST);
				player_walk_trigger(1);
				break;

			case 1:
				local->anim_1_running   = true;
				player.commands_allowed = false;
				player.walker_visible   = false;
				aa[1] = kernel_run_animation(kernel_name('w', 2), 2);
				kernel_synch(KERNEL_ANIM, aa[1], KERNEL_PLAYER, 0);
				break;

			case 2:
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
				kernel_room_scale(155, 95, 130, 78);
				local->anim_1_running = false;
				player.walker_visible = true;
				player.x              = END_CLIMB_MIDDLE_FLOOR_X;
				player.y              = END_CLIMB_MIDDLE_FLOOR_Y;
				player_walk(END_CLIMB_MIDDLE_FLOOR_X + 20, END_CLIMB_MIDDLE_FLOOR_Y + 6, FACING_SOUTHWEST);
				player_walk_trigger(3);
				break;

			case 3:
				player.commands_allowed = true;
				local->on_floor = 3;
				break;
			}

		} else if (local->on_floor == 1) {
			switch (kernel.trigger) {
			case 0:
				player_walk(START_CLIMB_BOTTOM_X, START_CLIMB_BOTTOM_Y, FACING_EAST);
				player_walk_trigger(1);
				break;

			case 1:
				local->anim_0_running   = true;
				player.commands_allowed = false;
				player.walker_visible   = false;
				aa[0] = kernel_run_animation(kernel_name('w', 1), 2);
				kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
				break;

			case 2:
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
				kernel_room_scale(311, 95, 286, 78);
				local->anim_0_running = false;
				player.walker_visible = true;
				player.x              = END_CLIMB_BOTTOM_FLOOR_X;
				player.y              = END_CLIMB_BOTTOM_FLOOR_Y;
				player_walk(END_CLIMB_BOTTOM_FLOOR_X + 25, END_CLIMB_BOTTOM_FLOOR_Y + 4, FACING_SOUTHWEST);
				player_walk_trigger(3);
				break;

			case 3:
				player.commands_allowed = true;
				local->on_floor = 2;
				break;
			}
		}
		goto handled;
	}

	if (player_said_2(climb_down, staircase)) {
		if (local->on_floor == 2) {
			switch (kernel.trigger) {
			case 0:
				player_walk(START_DOWN_MIDDLE_X + 18, START_DOWN_MIDDLE_Y - 1, FACING_WEST);
				player_walk_trigger(1);
				break;

			case 1:
				player_walk(START_DOWN_MIDDLE_X, START_DOWN_MIDDLE_Y, FACING_WEST);
				player_walk_trigger(2);
				break;

			case 2:
				local->anim_2_running   = true;
				player.commands_allowed = false;
				player.walker_visible   = false;
				aa[2] = kernel_run_animation(kernel_name('w', 3), 3);
				kernel_synch(KERNEL_ANIM, aa[2], KERNEL_PLAYER, 0);
				break;

			case 3:
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[2]);
				kernel_room_scale(467, 95, 442, 78);
				local->anim_2_running = false;
				player.walker_visible = true;
				player.x              = END_DOWN_MIDDLE_FLOOR_X;
				player.y              = END_DOWN_MIDDLE_FLOOR_Y;
				player_walk(END_DOWN_MIDDLE_FLOOR_X - 25, END_DOWN_MIDDLE_FLOOR_Y, FACING_SOUTHEAST);
				player_walk_trigger(4);
				break;

			case 4:
				player.commands_allowed = true;
				local->on_floor         = 1;
				break;
			}

		} else if (local->on_floor == 3) {
			switch (kernel.trigger) {
			case 0:
				player_walk(START_DOWN_TOP_X + 18, START_DOWN_TOP_Y - 1, FACING_WEST);
				player_walk_trigger(1);
				break;

			case 1:
				player_walk(START_DOWN_TOP_X, START_DOWN_TOP_Y, FACING_WEST);
				player_walk_trigger(2);
				break;

			case 2:
				local->anim_3_running   = true;
				player.commands_allowed = false;
				player.walker_visible   = false;
				aa[3] = kernel_run_animation(kernel_name('w', 4), 3);
				kernel_synch(KERNEL_ANIM, aa[3], KERNEL_PLAYER, 0);
				break;

			case 3:
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[3]);
				kernel_room_scale(311, 95, 286, 78);
				local->anim_3_running = false;
				player.walker_visible = true;
				player.x              = END_DOWN_TOP_FLOOR_X;
				player.y              = END_DOWN_TOP_FLOOR_Y;
				player_walk(END_DOWN_TOP_FLOOR_X - 19, END_DOWN_TOP_FLOOR_Y - 2, FACING_SOUTHEAST);
				player_walk_trigger(4);
				break;

			case 4:
				player.commands_allowed = true;
				local->on_floor         = 2;
				break;
			}
		}
		goto handled;
	}

	if (player_said_2(walk_through, door) || player_said_2(open, door) || kernel.trigger) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			seq[fx_take_6] = kernel_seq_pingpong(ss[fx_take_6], true, 5, 0, 0, 2);
			kernel_seq_range(seq[fx_take_6], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_player(seq[fx_take_6], true);
			kernel_seq_trigger(seq[fx_take_6], KERNEL_TRIGGER_EXPIRE, 0, 2);
			kernel_seq_trigger(seq[fx_take_6], KERNEL_TRIGGER_SPRITE, 4, ROOM_109_DOOR_OPENS);
			break;

		case ROOM_109_DOOR_OPENS:
			sound_play(N_DoorOpens);
			if (local->on_floor == 3) {
				seq[fx_top_door] = kernel_seq_forward(ss[fx_top_door], false, 8, 0, 0, 1);
				kernel_seq_depth(seq[fx_top_door], 14);
				kernel_seq_range(seq[fx_top_door], 1, 3);
				kernel_seq_trigger(seq[fx_top_door],
				                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_109_DOOR_OPENS + 1);
			} else if (local->on_floor == 2) {
				seq[fx_middle_door] = kernel_seq_forward(ss[fx_middle_door], false, 8, 0, 0, 1);
				kernel_seq_depth(seq[fx_middle_door], 14);
				kernel_seq_range(seq[fx_middle_door], 1, 3);
				kernel_seq_trigger(seq[fx_middle_door],
				                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_109_DOOR_OPENS + 1);
			}
			break;

		case ROOM_109_DOOR_OPENS + 1:
			if (local->on_floor == 3) {
				temp = seq[fx_top_door];
				seq[fx_top_door] = kernel_seq_stamp(ss[fx_top_door], false, 3);
				kernel_synch(KERNEL_SERIES, seq[fx_top_door], KERNEL_SERIES, temp);
				kernel_seq_depth(seq[fx_top_door], 14);
			} else if (local->on_floor == 2) {
				temp = seq[fx_middle_door];
				seq[fx_middle_door] = kernel_seq_stamp(ss[fx_middle_door], false, 3);
				kernel_synch(KERNEL_SERIES, seq[fx_middle_door], KERNEL_SERIES, temp);
				kernel_seq_depth(seq[fx_middle_door], 14);
			}
			break;

		case 2:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_6]);
			if (local->on_floor == 3) {
				player_walk(WALK_TO_X_OPEN_TOP_DOOR, WALK_TO_Y_OPEN_TOP_DOOR, FACING_WEST);
			} else if (local->on_floor == 2) {
				player_walk(WALK_TO_X_OPEN_MIDDLE_DOOR, WALK_TO_Y_OPEN_MIDDLE_DOOR, FACING_WEST);
			}
			kernel_timing_trigger(THREE_SECONDS, 3);
			break;

		case 3:
			if (local->on_floor == 3) {
				new_room = 111;
			} else if (local->on_floor == 2) {
				new_room = 110;
			}
			break;
		}
		goto handled;
	}

	if (player.look_around) {
		text_show(text_109_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(staircase)) {
			if (local->on_floor == 1) {
				text_show(text_109_11);
			} else if (local->on_floor == 2) {
				text_show(text_109_21);
			} else if (local->on_floor == 3) {
				text_show(text_109_22);
			}
			goto handled;
		}

		if (player_said_1(floor)) {
			text_show(text_109_12);
			goto handled;
		}

		if (player_said_1(backstage)) {
			text_show(text_109_13);
			goto handled;
		}

		if (player_said_1(door)) {
			text_show(text_109_14);
			goto handled;
		}

		if (player_said_1(railing)) {
			text_show(text_109_15);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_109_16);
			goto handled;
		}

		if (player_said_1(light_fixture)) {
			text_show(text_109_17);
			goto handled;
		}

		if (player_said_1(lamp)) {
			text_show(text_109_18);
			goto handled;
		}

		if (player_said_1(hole)) {
			text_show(text_109_19);
			goto handled;
		}

		if (player_said_1(ceiling)) {
			text_show(text_109_20);
			goto handled;
		}
	}

	if (player_said_2(take, lamp)) {
		text_show(text_109_24);
		goto handled;
	}

	if (player_said_2(close, door)) {
		text_show(text_109_23);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_109_preload() {
	room_init_code_pointer       = room_109_init;
	room_pre_parser_code_pointer = room_109_pre_parser;
	room_parser_code_pointer     = room_109_parser;
	room_daemon_code_pointer     = room_109_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
