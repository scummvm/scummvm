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
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/rooms/section5.h"
#include "mads/madsv2/phantom/rooms/room501.h"
#include "mads/madsv2/phantom/catacombs.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_501_init() {
	kernel_flip_hotspot(words_Christine, false);
	kernel_flip_hotspot(words_boat, false);

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->prevent_2 = false;
	}

	conv_get(CONV_MISC);

	/* ===================== Load Sprite Series ================== */

	ss[fx_flame] = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_door] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_left_door] = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_shove_off] = kernel_load_series(kernel_name('a', 1), false);
	ss[fx_christine] = kernel_load_series(kernel_name('a', 0), false);
	ss[fx_christine] = kernel_load_series("*CHR_6", PAL_MAP_ALL_TO_CLOSEST |
		PAL_MAP_ANY_TO_CLOSEST);
	ss[fx_christine] = kernel_load_series("*CHR_9", PAL_MAP_ALL_TO_CLOSEST |
		PAL_MAP_ANY_TO_CLOSEST);
	ss[fx_christine] = kernel_load_series("*CHR_8", PAL_MAP_ALL_TO_CLOSEST |
		PAL_MAP_ANY_TO_CLOSEST);
	ss[fx_take_6] = kernel_load_series("*RDRR_6", false);



	/* ======================= Stamp flame ======================= */


	seq[fx_flame] = kernel_seq_forward(ss[fx_flame], false, 6, 0, 0, 0);
	kernel_seq_depth(seq[fx_flame], 14);
	kernel_seq_range(seq[fx_flame], KERNEL_FIRST, KERNEL_LAST);

	if (previous_room == KERNEL_RESTORING_GAME) {

		if (global[christine_is_in_boat]) {
			local->anim_0_running = true;
			aa[0] = kernel_run_animation(kernel_name('b', 1), ROOM_501_START_ROWING);
			kernel_reset_animation(aa[0], 124);
			kernel_flip_hotspot_loc(words_Christine, true, CHRIS_X_LEFT, CHRIS_Y_LEFT);
			kernel_flip_hotspot(words_boat, true);
		}

		seq[fx_door] = kernel_seq_stamp
		(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 4);
		seq[fx_left_door] = kernel_seq_stamp
		(ss[fx_left_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_left_door], 4);
	}

	/* ========================= Previous Rooms ================== */


	if (previous_room == 506) {
		player.x = PLAYER_X_FROM_506;
		player.y = PLAYER_Y_FROM_506;
		player.facing = FACING_WEST;
		player.commands_allowed = false;

		if (global[christine_is_in_boat]) {
			local->anim_0_running = true;
			aa[0] = kernel_run_animation(kernel_name('b', 1), ROOM_501_START_ROWING);
			kernel_reset_animation(aa[0], 124);
			kernel_flip_hotspot_loc(words_Christine, true, CHRIS_X_LEFT, CHRIS_Y_LEFT);
			kernel_flip_hotspot(words_boat, true);
			player_walk(WALK_TO_X_FROM_506, WALK_TO_Y_FROM_506, FACING_SOUTHWEST);
			player_walk_trigger(ROOM_501_FROM_506);
			player_walk_trigger(ROOM_501_LEFT_DOOR_CLOSES);

		} else {
			local->anim_0_running = true;
			aa[0] = kernel_run_animation(kernel_name('b', 1), ROOM_501_START_ROWING);
			global[christine_is_in_boat] = true;
			kernel_flip_hotspot(words_boat, true);
			player_walk(WALK_TO_X_FROM_506, WALK_TO_Y_FROM_506, FACING_SOUTHWEST);
			player_walk_trigger(ROOM_501_FROM_506);

		}

		seq[fx_door] = kernel_seq_stamp
		(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 4);

		seq[fx_left_door] = kernel_seq_stamp
		(ss[fx_left_door], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_left_door], 1);

	} else if ((previous_room == 401) || (previous_room == 408) || (previous_room != KERNEL_RESTORING_GAME)) {
		player_first_walk(WEST_X, WEST_Y, FACING_EAST, WALK_TO_WEST_X, WALK_TO_WEST_Y, FACING_EAST, true);
		seq[fx_door] = kernel_seq_stamp
		(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 4);
		seq[fx_left_door] = kernel_seq_stamp
		(ss[fx_left_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_left_door], 4);
	}

	section_5_music();
}

void room_501_pre_parser() {
	if (player_said_2(walk_through, archway_to_west)) {
		if (global[lantern_status] == LANTERN_IS_OFF) {
			player.need_to_walk = false;
			text_show(text_309_18);
			player_cancel_command();

		} else if (!global[christine_is_in_boat]) {
			global_enter_catacombs(0);
		}
	}

	if ((player_said_1(door)) && (player_said_1(lock) || player_said_1(unlock) || player_said_1(open))) {
		if (inter_point_x < 287) {
			player_walk(DOOR_LEFT_X, DOOR_LEFT_Y, FACING_EAST);
		} else {
			player_walk(DOOR_RIGHT_X, DOOR_RIGHT_Y, FACING_EAST);
		}
	}

	if ((player_said_2(walk_through, door) || player_said_2(open, door)) &&
		player_has_been_in_room(506) && inter_point_x < 287) {
		switch (kernel.trigger) {
		case 0:
			player.ready_to_walk = false;
			player.need_to_walk = false;
			player.commands_allowed = false;
			conv_run(CONV_MISC);
			conv_export_value(2);
			kernel_timing_trigger(6, 1);
			break;

		case 1:
			if (conv_control.running >= 0) {
				kernel_timing_trigger(6, 1);
			} else {
				player.commands_allowed = true;
				player.command_ready = true;
				player.need_to_walk = true;
				player.ready_to_walk = true;
			}
			break;
		}
	}

	if (player_said_2(walk_through, archway_to_west) && global[lantern_status]) {
		if (global[right_door_is_open_504]) {
			player_walk(WALK_TO_CAT_NO_X, WALK_TO_CAT_NO_Y, FACING_WEST);
		}
	}
}

void room_501_parser() {
	int temp;

	if (conv_control.running == CONV_MISC) {
		goto handled;
	}

	if (player_said_2(walk_through, archway_to_west) && (global[christine_is_in_boat])) {
		conv_run(CONV_MISC);
		conv_export_value(3);
		goto handled;
	}

	if (player_said_2(talk_to, Christine)) {
		conv_run(CONV_MISC);
		conv_export_value(1);
		goto handled;
	}

	if (player_said_2(climb_into, boat)) {
		if (player_has(oar)) {
			local->anim_0_running = false;
		} else {
			text_show(text_501_23);
		}
		goto handled;
	}

	if (player_said_2(unlock, door) || player_said_2(lock, door)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			seq[fx_take_6] = kernel_seq_forward(ss[fx_take_6], false, 5, 0, 0, 1);
			kernel_seq_range(seq[fx_take_6], 1, 4);
			kernel_seq_player(seq[fx_take_6], true);
			kernel_seq_trigger(seq[fx_take_6], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			temp = seq[fx_take_6];
			seq[fx_take_6] = kernel_seq_stamp(ss[fx_take_6], false, 4);
			kernel_synch(KERNEL_SERIES, seq[fx_take_6], KERNEL_SERIES, temp);
			kernel_seq_player(seq[fx_take_6], false);
			kernel_timing_trigger(QUARTER_SECOND, 2);
			sound_play(N_DoorHandle005);
			break;

		case 2:
			kernel_seq_delete(seq[fx_take_6]);
			seq[fx_take_6] = kernel_seq_backward(ss[fx_take_6], false, 5, 0, 0, 1);
			kernel_seq_range(seq[fx_take_6], 1, 4);
			kernel_seq_player(seq[fx_take_6], false);
			kernel_seq_trigger(seq[fx_take_6], KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_6]);
			player.walker_visible = true;
			text_show(text_501_22);
			/* the key won't work */
			player.commands_allowed = true;
			break;
		}
		goto handled;
	}

	if (player_said_2(walk_through, door) || player_said_2(open, door)) {
		if (inter_point_x < 287) {  /* the left door */
			if (!global[christine_is_in_boat]) {  /* lock the left door */
				switch (kernel.trigger) {
				case 0:
					player.commands_allowed = false;
					player.walker_visible = false;
					seq[fx_take_6] = kernel_seq_forward(ss[fx_take_6], false, 5, 0, 0, 1);
					kernel_seq_range(seq[fx_take_6], 1, 4);
					kernel_seq_player(seq[fx_take_6], true);
					kernel_seq_trigger(seq[fx_take_6], KERNEL_TRIGGER_EXPIRE, 0, 1);
					break;

				case 1:
					temp = seq[fx_take_6];
					seq[fx_take_6] = kernel_seq_stamp(ss[fx_take_6], false, 4);
					kernel_synch(KERNEL_SERIES, seq[fx_take_6], KERNEL_SERIES, temp);
					kernel_seq_player(seq[fx_take_6], false);
					kernel_timing_trigger(QUARTER_SECOND, 2);
					sound_play(N_DoorHandle005);
					break;

				case 2:
					kernel_seq_delete(seq[fx_take_6]);
					seq[fx_take_6] = kernel_seq_backward(ss[fx_take_6], false, 5, 0, 0, 1);
					kernel_seq_range(seq[fx_take_6], 1, 4);
					kernel_seq_player(seq[fx_take_6], false);
					kernel_seq_trigger(seq[fx_take_6], KERNEL_TRIGGER_EXPIRE, 0, 3);
					break;

				case 3:
					kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_6]);
					player.walker_visible = true;
					text_show(text_501_20);
					/* the door is locked */
					player.commands_allowed = true;
					break;
				}
				goto handled;

			} else {  /* Christine is in boat, unlock left door (to get oar) */

				switch (kernel.trigger) {
				case (0):
					player.commands_allowed = false;
					player.walker_visible = false;
					seq[fx_take_6] = kernel_seq_pingpong(ss[fx_take_6], false,
						5, 0, 0, 2);
					kernel_seq_range(seq[fx_take_6], 1, 4);
					kernel_seq_player(seq[fx_take_6], true);
					kernel_seq_trigger(seq[fx_take_6],
						KERNEL_TRIGGER_SPRITE, 4, ROOM_501_DOOR_OPENS);
					kernel_seq_trigger(seq[fx_take_6],
						KERNEL_TRIGGER_EXPIRE, 0, ROOM_501_DOOR_OPENS + 2);
					break;

				case ROOM_501_DOOR_OPENS:
					kernel_seq_delete(seq[fx_left_door]);
					seq[fx_left_door] = kernel_seq_forward(ss[fx_left_door], false,
						8, 0, 0, 1);
					kernel_seq_depth(seq[fx_left_door], 4);
					kernel_seq_range(seq[fx_left_door], KERNEL_FIRST, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_left_door],
						KERNEL_TRIGGER_EXPIRE, 0, ROOM_501_DOOR_OPENS + 1);
					sound_play(N_DoorOpens);
					break;

				case ROOM_501_DOOR_OPENS + 1:
					seq[fx_left_door] = kernel_seq_stamp
					(ss[fx_left_door], false, KERNEL_LAST);
					kernel_seq_depth(seq[fx_left_door], 1);
					player_walk(PLAYER_X_FROM_506, PLAYER_Y_FROM_506, FACING_EAST);
					player_walk_trigger(ROOM_501_DOOR_OPENS + 3);
					break;

				case ROOM_501_DOOR_OPENS + 2:
					player.walker_visible = true;
					break;

				case ROOM_501_DOOR_OPENS + 3:
					conv_abort();
					new_room = 506;
					break;
				}
				goto handled;
			}

		} else { /* the right door */

			if (!global[christine_is_in_boat]) {
				switch (kernel.trigger) {
				case (0):
					player.commands_allowed = false;
					player.walker_visible = false;
					seq[fx_take_6] = kernel_seq_pingpong(ss[fx_take_6], false,
						5, 0, 0, 2);
					kernel_seq_range(seq[fx_take_6], 1, 4);
					kernel_seq_player(seq[fx_take_6], true);
					kernel_seq_trigger(seq[fx_take_6],
						KERNEL_TRIGGER_SPRITE, 4, ROOM_501_DOOR_OPENS);
					kernel_seq_trigger(seq[fx_take_6],
						KERNEL_TRIGGER_EXPIRE, 0, ROOM_501_DOOR_OPENS + 2);
					break;

				case ROOM_501_DOOR_OPENS:
					temp = seq[fx_door];
					kernel_seq_delete(seq[fx_door]);
					seq[fx_door] = kernel_seq_forward(ss[fx_door], false,
						8, 0, 0, 1);
					kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, temp);
					kernel_seq_depth(seq[fx_door], 4);
					kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_door],
						KERNEL_TRIGGER_EXPIRE, 0, ROOM_501_DOOR_OPENS + 1);
					sound_play(N_DoorOpens);
					break;

				case ROOM_501_DOOR_OPENS + 1:
					player_walk(PLAYER_X_FROM_502, PLAYER_Y_FROM_502, FACING_NORTHWEST);
					player_walk_trigger(ROOM_501_DOOR_OPENS + 3);
					break;

				case ROOM_501_DOOR_OPENS + 2:
					player.walker_visible = true;
					break;

				case ROOM_501_DOOR_OPENS + 3:
					seq[fx_door] = kernel_seq_backward(ss[fx_door], false,
						8, 0, 0, 1);
					kernel_seq_depth(seq[fx_door], 1);
					kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_door],
						KERNEL_TRIGGER_EXPIRE, 0, ROOM_501_DOOR_OPENS + 4);
					sound_play(N_DoorCloses);
					break;

				case ROOM_501_DOOR_OPENS + 4:
					seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 5);
					kernel_seq_depth(seq[fx_door], 1);
					new_room = 502;
					break;
				}
				goto handled;

			} else {  /* she is in the boat - lock the right door */

				switch (kernel.trigger) {
				case (0):
					player.commands_allowed = false;
					player.walker_visible = false;
					seq[fx_take_6] = kernel_seq_forward(ss[fx_take_6], false, 5, 0, 0, 1);
					kernel_seq_range(seq[fx_take_6], 1, 4);
					kernel_seq_player(seq[fx_take_6], true);
					kernel_seq_trigger(seq[fx_take_6], KERNEL_TRIGGER_EXPIRE, 0, 1);
					break;

				case 1:
					temp = seq[fx_take_6];
					seq[fx_take_6] = kernel_seq_stamp(ss[fx_take_6], false, 4);
					kernel_synch(KERNEL_SERIES, seq[fx_take_6], KERNEL_SERIES, temp);
					kernel_seq_player(seq[fx_take_6], false);
					kernel_timing_trigger(QUARTER_SECOND, 2);
					sound_play(N_DoorHandle501);
					break;

				case 2:
					kernel_seq_delete(seq[fx_take_6]);
					seq[fx_take_6] = kernel_seq_backward(ss[fx_take_6], false, 5, 0, 0, 1);
					kernel_seq_range(seq[fx_take_6], 1, 4);
					kernel_seq_player(seq[fx_take_6], false);
					kernel_seq_trigger(seq[fx_take_6], KERNEL_TRIGGER_EXPIRE, 0, 3);
					break;

				case 3:
					kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_6]);
					player.walker_visible = true;
					text_show(text_501_20);
					/* the door is locked */
					player.commands_allowed = true;
					break;
				}
				goto handled;
			}
		}
	}

	if (player.look_around) {
		text_show(text_501_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(floor)) {
			text_show(text_501_11);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_501_12);
			goto handled;
		}

		if (player_said_1(ceiling)) {
			text_show(text_501_13);
			goto handled;
		}

		if (player_said_1(lake)) {
			text_show(text_501_14);
			goto handled;
		}

		if (player_said_1(boat)) {
			text_show(text_501_26);
			goto handled;
		}

		if (player_said_1(torch)) {
			text_show(text_501_17);
			goto handled;
		}

		if (player_said_1(archway_to_west)) {
			text_show(text_501_18);
			goto handled;
		}

		if (player_said_1(door)) {
			if (inter_point_x < 287) {  /* the left door */
				if (player_has_been_in_room(506)) {
					text_show(text_501_27);
				} else {
					text_show(text_501_19);
				}
			} else {
				if (player_has_been_in_room(506)) {
					text_show(text_501_28);
				} else {
					text_show(text_501_19);
				}
			}
			goto handled;
		}

		if (player_said_1(column)) {
			text_show(text_501_21);
			goto handled;
		}

		if (player_said_1(Christine)) {
			text_show(text_501_24);
			goto handled;
		}
	}

	if (player_said_2(take, torch)) {
		text_show(text_501_25);
		goto handled;
	}

	if (player_said_2(take, Christine)) {
		text_show(text_501_29);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_501_daemon() {
	int id;

	/* ==================== Close right door ===================== */

	switch (kernel.trigger) {
	case ROOM_501_LEFT_DOOR_CLOSES:
		kernel_seq_delete(seq[fx_left_door]);
		seq[fx_left_door] = kernel_seq_backward(ss[fx_left_door], false,
			8, 0, 0, 1);
		kernel_seq_depth(seq[fx_left_door], 10);
		kernel_seq_range(seq[fx_left_door], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_left_door],
			KERNEL_TRIGGER_EXPIRE, 0, ROOM_501_LEFT_DOOR_CLOSES + 1);
		break;

	case ROOM_501_LEFT_DOOR_CLOSES + 1:
		sound_play(N_DoorCloses);
		seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 1);
		kernel_seq_depth(seq[fx_left_door], 10);
		if (!global[chris_will_take_seat]) {
			player.commands_allowed = true;
		}
		global[chris_will_take_seat] = false;
		break;
	}

	switch (kernel.trigger) {
	case ROOM_501_DOOR_CLOSES:
		seq[fx_door] = kernel_seq_backward(ss[fx_door], false,
			8, 0, 0, 1);
		kernel_seq_depth(seq[fx_door], 4);
		kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_door],
			KERNEL_TRIGGER_EXPIRE, 0, ROOM_501_DOOR_CLOSES + 1);
		break;

	case ROOM_501_DOOR_CLOSES + 1:
		sound_play(N_DoorCloses);
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 4);
		player.commands_allowed = true;
		break;

	case ROOM_501_FROM_506:
		player_walk(WALK_TO_X_FROM_506 - 5, WALK_TO_Y_FROM_506 + 6, FACING_NORTHWEST);
		kernel_reset_animation(aa[0], 2);
		id = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
			0, 0, 0, 0);
		kernel_dynamic_anim(id, aa[0], 0);
		kernel_dynamic_anim(id, aa[0], 1);
		kernel_dynamic_anim(id, aa[0], 2);
		kernel_dynamic_anim(id, aa[0], 3);
		break;

	case ROOM_501_START_ROWING:
		player.commands_allowed = false;
		player.walker_visible = false;
		seq[fx_shove_off] = kernel_seq_forward(ss[fx_shove_off], false,
			9, 0, 0, 1);
		kernel_seq_range(seq[fx_shove_off], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_shove_off],
			KERNEL_TRIGGER_EXPIRE, 0, ROOM_501_SHOVE_OFF);
		break;

	case ROOM_501_SHOVE_OFF:
		global[player_score] += 5;
		new_room = 310;
		break;
	}

	if (local->anim_0_running) {
		if (kernel_anim[aa[0]].frame == 103) {
			kernel_flip_hotspot_loc(words_Christine, true, CHRIS_X_RIGHT, CHRIS_Y_RIGHT);
		}

		if ((kernel_anim[aa[0]].frame == 28) && (!local->prevent_2)) {
			local->prevent_2 = true;
			kernel_timing_trigger(1, ROOM_501_LEFT_DOOR_CLOSES);
		}

		if (kernel_anim[aa[0]].frame == 124) {
			kernel_flip_hotspot_loc(words_Christine, false, CHRIS_X_RIGHT, CHRIS_Y_RIGHT);
			kernel_flip_hotspot_loc(words_Christine, true, CHRIS_X_LEFT, CHRIS_Y_LEFT);
		}

		if (kernel_anim[aa[0]].frame == 125) {
			kernel_reset_animation(aa[0], 124);
			/* repeat Christine waiting in boat */
		}

		if (kernel_anim[aa[0]].frame == 1) {
			kernel_reset_animation(aa[0], 0);
			/* have boat sitting at dock while Chris is getting into it */
		}

		if (kernel_anim[aa[0]].frame == 84) {
			player.commands_allowed = true;
		}
	}
}

void room_501_preload() {
	room_init_code_pointer = room_501_init;
	room_pre_parser_code_pointer = room_501_pre_parser;
	room_parser_code_pointer = room_501_parser;
	room_daemon_code_pointer = room_501_daemon;

	section_5_walker();
	section_5_interface();

	vocab_make_active(words_Christine);
	vocab_make_active(words_look_at);
	vocab_make_active(words_walk_to);
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
