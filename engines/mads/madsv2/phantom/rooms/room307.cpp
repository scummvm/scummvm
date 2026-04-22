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
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/rooms/section3.h"
#include "mads/madsv2/phantom/rooms/room307.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_307_init() {
	ss[fx_green_frame] = kernel_load_series(kernel_name('p', 0), false);
	ss[fx_door] = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_bend_down_9] = kernel_load_series("*RRD_9", false);
	ss[fx_take_9] = kernel_load_series("*RDR_9", false);

	if (global[right_door_is_open_504]) {
		kernel_flip_hotspot(words_door, true);
	} else {
		kernel_flip_hotspot(words_door, false);
	}

	/* ================ If in 1881, put green_frame ============= */

	if ((object_is_here(green_frame)) && (game.difficulty == EASY_MODE)) {
		seq[fx_green_frame] = kernel_seq_stamp(ss[fx_green_frame], false, 1);
		kernel_seq_depth(seq[fx_green_frame], 14);
	} else {
		kernel_flip_hotspot(words_green_frame, false);
	}

	/* ========================= Previous Rooms ================== */

	if (previous_room == 308) {
		player.x = START_X_FROM_308;
		player.y = START_Y_FROM_308;
		player.facing = FACING_SOUTHEAST;
		player.commands_allowed = false;
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
		if (!(global[player_score_flags] & SCORE_CHASE_CATWALK)) {
			global[player_score_flags] = global[player_score_flags] | SCORE_CHASE_CATWALK;
			global[player_score] += 5;
		}
		kernel_seq_depth(seq[fx_door], 4);
		player_walk(END_X_FROM_308, END_Y_FROM_308, FACING_EAST);
		player_walk_trigger(ROOM_307_DOOR_CLOSES);

	} else if ((previous_room == 303) || (previous_room != KERNEL_RESTORING_GAME)) {
		player_first_walk(OFF_SCREEN_X_FROM_303, OFF_SCREEN_Y_FROM_303, FACING_WEST,
			PLAYER_X_FROM_303, PLAYER_Y_FROM_303, FACING_WEST, true);
	}

	section_3_music();
}

void room_307_pre_parser() {
	if (player_said_2(exit_to, alcove)) {
		player.walk_off_edge_to_room = 303;
	}

	if (player_said_2(open, door)) {
		player_walk(OPEN_DOOR_X, OPEN_DOOR_Y, FACING_NORTHWEST);
	}
}

void room_307_parser() {
	int temp;  /* for synching purposes */
	int count = 0;

	if (player_said_2(take, green_frame) &&
		(object_is_here(green_frame) || kernel.trigger)) {
		switch (kernel.trigger) {
		case (0):

			if (global[current_year] == 1881) {
				if (player_has(yellow_frame)) ++count;
				if (player_has(red_frame))    ++count;
				if (player_has(blue_frame))   ++count;
				if (count < 3) global[player_score] += 5;
			}

			player.commands_allowed = false;
			player.walker_visible = false;
			seq[fx_bend_down_9] = kernel_seq_pingpong(ss[fx_bend_down_9], false,
				5, 0, 0, 2);
			kernel_seq_range(seq[fx_bend_down_9], 1, 5);
			kernel_seq_player(seq[fx_bend_down_9], true);
			kernel_seq_trigger(seq[fx_bend_down_9],
				KERNEL_TRIGGER_SPRITE, 5, 1);
			kernel_seq_trigger(seq[fx_bend_down_9],
				KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			kernel_seq_delete(seq[fx_green_frame]);
			kernel_flip_hotspot(words_green_frame, false);
			inter_give_to_player(green_frame);
			sound_play(N_TakeObjectSnd);
			break;

		case 2:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_bend_down_9]);
			player.walker_visible = true;
			kernel_timing_trigger(20, 3);
			break;

		case 3:
			if (global[current_year] == 1881) {
				object_examine(green_frame, text_008_45, 0);
			} else {
				object_examine(green_frame, text_008_19, 0);
			}
			/* You pick up the green color frame */
			player.commands_allowed = true;
			break;
		}
		goto handled;
	}

	if (player_said_2(walk_through, door) || player_said_2(open, door)) {
		switch (kernel.trigger) {
		case (0):
			player.commands_allowed = false;
			player.walker_visible = false;
			seq[fx_take_9] = kernel_seq_pingpong(ss[fx_take_9], true,
				5, 0, 0, 2);
			kernel_seq_range(seq[fx_take_9], 1, 4);
			kernel_seq_player(seq[fx_take_9], true);
			kernel_seq_trigger(seq[fx_take_9],
				KERNEL_TRIGGER_SPRITE, 4, ROOM_307_DOOR_OPENS);
			kernel_seq_trigger(seq[fx_take_9],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_307_DOOR_OPENS + 2);
			break;

		case ROOM_307_DOOR_OPENS:
			sound_play(N_DoorOpens);
			seq[fx_door] = kernel_seq_forward(ss[fx_door], false,
				8, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 14);
			kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_door],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_307_DOOR_OPENS + 1);
			break;

		case ROOM_307_DOOR_OPENS + 1:
			temp = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
			kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, temp);
			kernel_seq_depth(seq[fx_door], 14);
			kernel_timing_trigger(10, ROOM_307_DOOR_OPENS + 3);
			break;

		case ROOM_307_DOOR_OPENS + 2:
			player.walker_visible = true;
			break;

		case ROOM_307_DOOR_OPENS + 3:
			new_room = 308;
			break;
		}
		goto handled;
	}

	if (player.look_around) {
		text_show(text_307_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(column)) {
			if (global[right_door_is_open_504]) {
				text_show(text_307_25);
			} else {
				text_show(text_307_11);
			}
			goto handled;
		}

		if (player_said_1(catwalk)) {
			text_show(text_307_12);
			goto handled;
		}

		if (player_said_1(grid)) {
			text_show(text_307_13);
			goto handled;
		}

		if (player_said_1(girder)) {
			text_show(text_307_14);
			goto handled;
		}

		if (player_said_1(gridwork)) {
			text_show(text_307_15);
			goto handled;
		}

		if (player_said_1(hemp)) {
			text_show(text_307_16);
			goto handled;
		}

		if (player_said_1(back_wall)) {
			text_show(text_307_17);
			goto handled;
		}

		if (player_said_1(ductwork)) {
			text_show(text_307_18);
			goto handled;
		}

		if ((player_said_1(green_frame)) && (object_is_here(green_frame))) {
			if (global[current_year] == 1993) {
				text_show(text_307_19);
				goto handled;
			} else {
				text_show(text_307_20);
				goto handled;
			}
		}

		if (player_said_1(alcove)) {
			text_show(text_307_21);
			goto handled;
		}

		if (player_said_1(railing)) {
			text_show(text_307_22);
			goto handled;
		}

		if (player_said_1(door)) {
			text_show(text_307_26);
			goto handled;
		}
	}

	if (player_said_2(take, hemp)) {
		text_show(text_307_23);
		goto handled;
	}

	if (player_said_2(pull, hemp)) {
		text_show(text_301_41);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_307_daemon() {
	switch (kernel.trigger) {
	case ROOM_307_DOOR_CLOSES:
		kernel_seq_delete(seq[fx_door]);
		seq[fx_door] = kernel_seq_backward(ss[fx_door], false,
			8, 0, 0, 1);
		kernel_seq_depth(seq[fx_door], 10);
		kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_door],
			KERNEL_TRIGGER_EXPIRE, 0, ROOM_307_DOOR_CLOSES + 1);
		break;

	case ROOM_307_DOOR_CLOSES + 1:
		sound_play(N_DoorCloses);
		player.commands_allowed = true;
		break;
	}
}

void room_307_preload() {
	room_init_code_pointer = room_307_init;
	room_pre_parser_code_pointer = room_307_pre_parser;
	room_parser_code_pointer = room_307_parser;
	room_daemon_code_pointer = room_307_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
