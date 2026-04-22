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
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/rooms/room105.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_105_init() {
	global_speech_load(speech_christine_scales);

	/* ==================== Load Sprite Series ====================== */

	ss[fx_door]        = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_thunder]     = kernel_load_series(kernel_name('x', 3), false);
	ss[fx_bend_down_9] = kernel_load_series("*RRD_9", false);
	ss[fx_take_9]      = kernel_load_series("*RDR_9", false);
	ss[fx_climb_up]    = kernel_load_series(kernel_name('a', 0), false);
	ss[fx_climb_down]  = kernel_load_series(kernel_name('a', 1), false);

	if (global[current_year] == 1993) {
		ss[fx_1993] = kernel_load_series(kernel_name('z', -1), false);
	}

	if ((global[current_year] == 1881) && (!global[hint_that_daae_is_home_2])) {
		if ((global[jacques_name_is_known] == YES_AND_END_CONV) &&
		    (global[madame_name_is_known] == YES_AND_END_CONV) &&
		    (global[panel_in_206]) &&
		    (global[done_rich_conv_203]) &&
		    player_has(lantern) &&
		    ((player_has(cable_hook) && player_has(rope)) || player_has(rope_with_hook))) {
			global[hint_that_daae_is_home_2] = true;
			kernel_timing_trigger(FIVE_SECONDS, ROOM_105_INIT_TEXT);
		}
	}


	/* ==================== If in 1881, put lantern & red_frame ==== */

	if ((object_is_here(lantern)) && (global[current_year] == 1881)) {
		ss[fx_lantern]  = kernel_load_series(kernel_name('x', 0), false);
		seq[fx_lantern] = kernel_seq_stamp(ss[fx_lantern], false, 1);
		kernel_seq_depth(seq[fx_lantern], 14);
	} else {
		kernel_flip_hotspot(words_lantern, false);
	}

	if (object_is_here(red_frame)) {
		ss[fx_red_frame]  = kernel_load_series(kernel_name('x', 1), false);
		seq[fx_red_frame] = kernel_seq_stamp(ss[fx_red_frame], false, 1);
		kernel_seq_depth(seq[fx_red_frame], 14);
	} else {
		kernel_flip_hotspot(words_red_frame, false);
	}


	/* ==================== Change room if it is 1993 ============== */

	if (global[current_year] == 1993) {
		kernel_draw_to_background(ss[fx_1993], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_flip_hotspot(words_props, false);
		kernel_flip_hotspot(words_bear_prop, false);
		kernel_flip_hotspot(words_stair_unit, false);
		kernel_flip_hotspot(words_prop, false);
		kernel_flip_hotspot(words_elephant_prop, false);
		kernel_flip_hotspot(words_column_prop, false);

		/* 2 dynamics for column props in background */

		local->column_prop = kernel_add_dynamic(words_column_prop, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                     COLUMN_1_X, COLUMN_1_Y, COLUMN_1_X_SIZE, COLUMN_1_Y_SIZE);
		kernel_dynamic_walk(local->column_prop, WALK_TO_COLUMN_1_X, WALK_TO_COLUMN_1_Y, FACING_NORTHWEST);
		local->column_prop = kernel_add_dynamic(words_column_prop, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                     COLUMN_2_X, COLUMN_2_Y, COLUMN_2_X_SIZE, COLUMN_2_Y_SIZE);
		kernel_dynamic_walk(local->column_prop, WALK_TO_COLUMN_2_X, WALK_TO_COLUMN_2_Y, FACING_NORTHWEST);

		/* 3 dynamics for misc. props in foreground */

		kernel_add_dynamic(words_prop, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                   PROP_1_X, PROP_1_Y, PROP_1_X_SIZE, PROP_1_Y_SIZE);
		kernel_add_dynamic(words_prop, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                   PROP_2_X, PROP_2_Y, PROP_2_X_SIZE, PROP_2_Y_SIZE);
		kernel_add_dynamic(words_prop, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                   PROP_3_X, PROP_3_Y, PROP_3_X_SIZE, PROP_3_Y_SIZE);

		/* 3 dynamics for bear prop in foreground */

		kernel_add_dynamic(words_bear_prop, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                   BEAR_1_X, BEAR_1_Y, BEAR_1_X_SIZE, BEAR_1_Y_SIZE);
		kernel_add_dynamic(words_bear_prop, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                   BEAR_2_X, BEAR_2_Y, BEAR_2_X_SIZE, BEAR_2_Y_SIZE);
		kernel_add_dynamic(words_bear_prop, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                   BEAR_3_X, BEAR_3_Y, BEAR_3_X_SIZE, BEAR_3_Y_SIZE);

		/* 1 dynamic for light on ceiling */

		kernel_add_dynamic(words_light_fixture, words_look_at, SYNTAX_SINGULAR, KERNEL_NONE,
		                   LIGHT_X, LIGHT_Y, LIGHT_X_SIZE, LIGHT_Y_SIZE);
	}


	/* ==================== Thunder Machine ========================= */

	seq[fx_thunder] = kernel_seq_stamp(ss[fx_thunder], false, 2);
	kernel_seq_depth(seq[fx_thunder], 14);


	/* ==================== Previous Rooms ========================== */

	if ((previous_room == 106) || (previous_room == 114)) {
		player.x      = PLAYER_X_FROM_STAIRCASE;
		player.y      = PLAYER_Y_FROM_STAIRCASE;
		player.facing = FACING_WEST;
		seq[fx_door]  = kernel_seq_stamp(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 10);

	} else if ((previous_room == 103) || (previous_room != KERNEL_RESTORING_GAME)) {
		player.x                = PLAYER_X_FROM_103;
		player.y                = PLAYER_Y_FROM_103;
		player.facing           = FACING_SOUTHEAST;
		player_walk(WALK_TO_X_FROM_103, WALK_TO_Y_FROM_103, FACING_SOUTHEAST);
		player_walk_trigger(ROOM_105_DOOR_CLOSES);
		player.commands_allowed = false;
		seq[fx_door]            = kernel_seq_stamp(ss[fx_door], false, 8);
		kernel_seq_depth(seq[fx_door], 10);

	} else if (previous_room == KERNEL_RESTORING_GAME) {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 10);
	}

	section_1_music();
}

void room_105_daemon() {
	/* ==================== Close the door when player enters ======= */

	switch (kernel.trigger) {
	case ROOM_105_INIT_TEXT:
		global_speech_go(speech_christine_scales);
		kernel_timing_trigger(TWO_SECONDS, ROOM_105_INIT_TEXT + 1);
		break;

	case ROOM_105_INIT_TEXT + 1:
		text_show(text_105_37);
		break;

	case ROOM_105_DOOR_CLOSES:
		kernel_seq_delete(seq[fx_door]);
		seq[fx_door] = kernel_seq_backward(ss[fx_door], false,
		                                   8, 0, 0, 1);
		kernel_seq_depth(seq[fx_door], 10);
		kernel_seq_range(seq[fx_door], 1, 8);
		kernel_seq_trigger(seq[fx_door],
		                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_105_DOOR_CLOSES + 1);
		sound_play(N_SqueakyDoor);
		break;

	case ROOM_105_DOOR_CLOSES + 1:
		sound_play(N_DoorCloses);
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 10);
		player.commands_allowed = true;
		break;
	}
}

void room_105_pre_parser() {
	if (player_said_2(walk_through, door_to_pit)) {
		player.walk_off_edge_to_room = 102;
	}

	if (player_said_2(open, door) || player_said_2(unlock, door) || player_said_2(lock, door)) {
		player_walk(FRONT_OF_DOOR_X, FRONT_OF_DOOR_Y, FACING_NORTHWEST);
	}
}

void room_105_parser() {
	int temp;       /* For synching purposes */
	int count = 0;

	if ((player_said_2(push, thunder_machine)) ||
	    (player_said_2(pull, thunder_machine))) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			aa[0] = kernel_run_animation(kernel_name('t', 1), ROOM_105_DUN_THUNDER);
			kernel_seq_delete(seq[fx_thunder]);
			kernel_synch(KERNEL_SERIES, seq[fx_thunder], KERNEL_ANIM, aa[0]);
			kernel_seq_player(aa[0], false);
			break;

		case ROOM_105_DUN_THUNDER:
			seq[fx_thunder] = kernel_seq_stamp(ss[fx_thunder], false, 2);
			kernel_seq_depth(seq[fx_thunder], 14);
			kernel_synch(KERNEL_SERIES, seq[fx_thunder], KERNEL_ANIM, aa[0]);
			player.commands_allowed = true;
			player.walker_visible   = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
			break;
		}
		goto handled;
	}

	if (player_said_2(climb_up, circular_staircase)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			aa[0] = kernel_run_animation(kernel_name('u', 1), 1);
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
			break;

		case 1:
			new_room = 106;
			break;
		}
		goto handled;
	}

	if (player_said_2(climb_down, circular_staircase)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			aa[0] = kernel_run_animation(kernel_name('d', 1), 1);
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
			break;

		case 1:
			new_room = 114;
			break;
		}
		goto handled;
	}

	if (player_said_2(take, red_frame) &&
	    (object_is_here(red_frame) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			if (global[current_year] == 1881) {
				if (player_has(yellow_frame)) ++count;
				if (player_has(green_frame))  ++count;
				if (player_has(blue_frame))   ++count;
				if (count < 3) global[player_score] += 5;
			}
			player.commands_allowed = false;
			player.walker_visible   = false;
			seq[fx_bend_down_9] = kernel_seq_pingpong(ss[fx_bend_down_9], true,
			                                          5, 0, 0, 2);
			kernel_seq_range(seq[fx_bend_down_9], 1, 5);
			kernel_seq_player(seq[fx_bend_down_9], true);
			kernel_seq_trigger(seq[fx_bend_down_9],
			                   KERNEL_TRIGGER_SPRITE, 5, 1);
			kernel_seq_trigger(seq[fx_bend_down_9],
			                   KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			kernel_seq_delete(seq[fx_red_frame]);
			kernel_flip_hotspot(words_red_frame, false);
			inter_give_to_player(red_frame);
			sound_play(N_TakeObjectSnd);
			break;

		case 2:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_bend_down_9]);
			player.walker_visible = true;
			kernel_timing_trigger(20, 3);
			break;

		case 3:
			if (global[current_year] == 1881) {
				object_examine(red_frame, text_008_42, 0);
			} else {
				object_examine(red_frame, text_008_02, 0);
			}
			/* You pick up the red color frame */
			player.commands_allowed = true;
			break;
		}
		goto handled;
	}

	if (player_said_2(take, lantern) &&
	    (object_is_here(lantern) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			global[player_score]    += 5;
			player.commands_allowed  = false;
			player.walker_visible    = false;
			seq[fx_take_9] = kernel_seq_pingpong(ss[fx_take_9], true,
			                                     5, 0, 0, 2);
			kernel_seq_range(seq[fx_take_9], 1, 4);
			kernel_seq_player(seq[fx_take_9], true);
			kernel_seq_trigger(seq[fx_take_9],
			                   KERNEL_TRIGGER_SPRITE, 4, 1);
			kernel_seq_trigger(seq[fx_take_9],
			                   KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			kernel_seq_delete(seq[fx_lantern]);
			kernel_flip_hotspot(words_lantern, false);
			inter_give_to_player(lantern);
			sound_play(N_TakeObjectSnd);
			break;

		case 2:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_9]);
			player.walker_visible = true;
			kernel_timing_trigger(20, 3);
			break;

		case 3:
			object_examine(lantern, text_008_01, 0);
			/* You pick up the lantern */
			player.commands_allowed = true;
			break;
		}
		goto handled;
	}

	if (player_said_2(walk_through, door) || player_said_2(open, door) || (kernel.trigger) ||
	    player_said_2(unlock, door) || player_said_2(lock, door)) {
		if ((global[current_year] == 1881) && !player_said_1(unlock) && !player_said_1(lock)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take_9] = kernel_seq_pingpong(ss[fx_take_9], true,
				                                     5, 0, 0, 2);
				kernel_seq_range(seq[fx_take_9], 1, 4);
				kernel_seq_player(seq[fx_take_9], true);
				kernel_seq_trigger(seq[fx_take_9],
				                   KERNEL_TRIGGER_SPRITE, 4, ROOM_105_DOOR_OPENS);
				kernel_seq_trigger(seq[fx_take_9],
				                   KERNEL_TRIGGER_EXPIRE, 0, 2);
				break;

			case ROOM_105_DOOR_OPENS:
				sound_play(N_DoorOpens);
				kernel_seq_delete(seq[fx_door]);
				seq[fx_door] = kernel_seq_forward(ss[fx_door], false,
				                                  8, 0, 0, 1);
				kernel_seq_depth(seq[fx_door], 14);
				kernel_seq_range(seq[fx_door], 1, 8);
				kernel_seq_trigger(seq[fx_door],
				                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_105_DOOR_OPENS + 1);
				sound_play(N_SqueakyDoor);
				break;

			case ROOM_105_DOOR_OPENS + 1:
				temp         = seq[fx_door];
				seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 8);
				kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, temp);
				kernel_seq_depth(seq[fx_door], 14);
				player_walk(WALK_TO_X_OPEN_DOOR, WALK_TO_Y_OPEN_DOOR, FACING_NORTHWEST);
				break;

			case 2:
				player.walker_visible = true;
				kernel_timing_trigger(THREE_SECONDS, 3);
				break;

			case 3:
				new_room = 103;
				break;
			}

		} else {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take_9] = kernel_seq_forward(ss[fx_take_9], true, 5, 0, 0, 1);
				kernel_seq_range(seq[fx_take_9], 1, 4);
				kernel_seq_player(seq[fx_take_9], true);
				kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				temp           = seq[fx_take_9];
				seq[fx_take_9] = kernel_seq_stamp(ss[fx_take_9], true, 4);
				kernel_synch(KERNEL_SERIES, seq[fx_take_9], KERNEL_SERIES, temp);
				kernel_seq_player(seq[fx_take_9], false);
				sound_play(N_DoorHandle);
				kernel_timing_trigger(QUARTER_SECOND, 2);
				break;

			case 2:
				kernel_seq_delete(seq[fx_take_9]);
				seq[fx_take_9] = kernel_seq_backward(ss[fx_take_9], true, 5, 0, 0, 1);
				kernel_seq_range(seq[fx_take_9], 1, 4);
				kernel_seq_player(seq[fx_take_9], false);
				kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_9]);
				player.walker_visible = true;
				if (player_said_1(lock) || player_said_1(unlock)) {
					text_show(text_000_32);
					/* the key does not work here */
				} else {
					text_show(text_105_36);
					/* the door is locked */
				}
				player.commands_allowed = true;
				break;
			}
		}
		goto handled;
	}

	if (player.look_around) {
		if (global[current_year] == 1993) {
			text_show(text_105_10);
		} else {
			text_show(text_105_11);
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(floor)) {
			text_show(text_105_12);
			goto handled;
		}

		if (player_said_1(circular_staircase)) {
			text_show(text_105_13);
			goto handled;
		}

		if (player_said_1(lantern) && object_is_here(lantern)) {
			text_show(text_105_14);
			goto handled;
		}

		if (player_said_1(red_frame) && object_is_here(red_frame)) {
			if (global[current_year] == 1881) {
				text_show(text_105_30);
			} else {
				text_show(text_105_15);
			}
			goto handled;
		}

		if (player_said_1(door)) {
			text_show(text_105_16);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_105_17);
			goto handled;
		}

		if (player_said_1(prop_table)) {
			text_show(text_105_18);
			goto handled;
		}

		if (player_said_1(bear_prop)) {
			if (global[current_year] == 1993) {
				text_show(text_105_19);
			} else {
				text_show(text_105_38);
			}
			goto handled;
		}

		if (player_said_1(elephant_prop)) {
			text_show(text_105_20);
			goto handled;
		}

		if (player_said_1(column_prop)) {
			if (global[current_year] == 1993) {
				text_show(text_105_21);
			} else {
				text_show(text_105_39);
			}
			goto handled;
		}

		if (player_said_1(thunder_machine)) {
			if (global[current_year] == 1993) {
				text_show(text_105_22);
			} else {
				text_show(text_105_40);
			}
			goto handled;
		}

		if (player_said_1(stair_unit)) {
			text_show(text_105_23);
			goto handled;
		}

		if (player_said_1(prop)) {
			text_show(text_105_24);
			goto handled;
		}

		if (player_said_1(props)) {
			text_show(text_105_25);
			goto handled;
		}

		if (player_said_1(exit_sign)) {
			text_show(text_105_26);
			goto handled;
		}

		if (player_said_1(flats)) {
			text_show(text_105_27);
			goto handled;
		}

		if (player_said_1(hemp)) {
			text_show(text_105_28);
			goto handled;
		}

		if (player_said_1(pipe)) {
			text_show(text_105_29);
			goto handled;
		}

		if (player_said_1(graffiti)) {
			text_show(text_105_31);
			goto handled;
		}

		if (player_said_1(light_fixture)) {
			text_show(text_105_35);
			goto handled;
		}
	}

	if (player_said_2(talk_to, bear_prop)) {
		text_show(text_105_32);
		goto handled;
	}

	if (player_said_2(take, bear_prop)) {
		text_show(text_105_33);
		goto handled;
	}

	if (player_said_2(close, door)) {
		text_show(text_105_34);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_105_preload() {
	room_init_code_pointer       = room_105_init;
	room_pre_parser_code_pointer = room_105_pre_parser;
	room_parser_code_pointer     = room_105_parser;
	room_daemon_code_pointer     = room_105_daemon;

	if (global[current_year] == 1993) {
		kernel_initial_variant = 1;
	}

	section_1_walker();
	section_1_interface();

	vocab_make_active(words_light_fixture);
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
