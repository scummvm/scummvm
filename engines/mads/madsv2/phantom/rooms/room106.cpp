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

/*      room106i.c / room106d.c / room106p.c
 *      room106i.c by Brian Reynolds, room106p.c by Paul Lahaise    7-Jan-93
 *
 *      NOTE: the original source used 'case' as a vocabulary word identifier.
 *      Since 'case' is a C++ reserved keyword, it has been renamed 'prop_case'
 *      throughout this file (maps to words_prop_case).
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
#include "mads/madsv2/phantom/rooms/room106.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_106_init() {
	int dyn_floor;

	global_speech_load(speech_christine_scales);

	/* ==================== Load Sprite Series ====================== */

	if (global[current_year] == 1993) {
		if (!player_has(sandbag)) {
			ss[fx_bend_down_9] = kernel_load_series("*RRD_9", false);
			ss[fx_sandbag]     = kernel_load_series(kernel_name('x', 0), false);
		}
	} else {
		ss[fx_1881] = kernel_load_series(kernel_name('z', -1), false);
		if (object_is_here(cable_hook) && !player_has(rope_with_hook)) {
			ss[fx_bend_down_9] = kernel_load_series("*RRD_9", false);
			ss[fx_cable_hook]  = kernel_load_series(kernel_name('p', 0), false);
		}
	}

	ss[fx_yikes]      = kernel_load_series(kernel_name('a', 0), false);
	ss[fx_door]       = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_take_9]     = kernel_load_series("*RDR_9", false);
	ss[fx_climb_up]   = kernel_load_series(kernel_name('a', 1), false);
	ss[fx_climb_down] = kernel_load_series(kernel_name('a', 2), false);


	if ((global[current_year] == 1881) && (!global[hint_that_daae_is_home_1])) {
		if ((global[jacques_name_is_known] == YES_AND_END_CONV) &&
		    (global[madame_name_is_known] == YES_AND_END_CONV) &&
		    (global[panel_in_206]) &&
		    (global[done_rich_conv_203]) &&
		    (player_has(lantern)) &&
		    ((player_has(cable_hook) && player_has(rope)) || player_has(rope_with_hook))) {
			global[hint_that_daae_is_home_1] = true;
			kernel_timing_trigger(FIVE_SECONDS, ROOM_106_INIT_TEXT);
		}
	}


	/* ==================== If sandbag has dropped, show it ======== */

	if ((global[sandbag_status] == SANDBAG_DROPPED) && (global[current_year] == 1993) &&
	    (object_is_here(sandbag))) {
		seq[fx_sandbag] = kernel_seq_stamp(ss[fx_sandbag], false, KERNEL_LAST);
		local->dynamic_sandbag = kernel_add_dynamic(words_sandbag, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                         SANDBAG_X, SANDBAG_Y, SANDBAG_XS, SANDBAG_YS);
		kernel_dynamic_walk(local->dynamic_sandbag, SANDBAG_WALK_TO_X, SANDBAG_WALK_TO_Y, FACING_NORTHEAST);
		kernel_seq_depth(seq[fx_sandbag], 2);
	}

	if (global[current_year] == 1881) {
		kernel_draw_to_background(ss[fx_1881], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_flip_hotspot(words_big_prop, false);
		kernel_flip_hotspot(words_statue, false);
		kernel_flip_hotspot(words_plant_prop, false);
		kernel_flip_hotspot(words_pedestal, false);
		kernel_flip_hotspot(words_sandbag, false);
		kernel_flip_hotspot(words_crate, false);
		kernel_add_dynamic(words_sandbag, words_look_at, SYNTAX_SINGULAR, KERNEL_NONE,
		                   DYN_SANDBAG_1_X, DYN_SANDBAG_1_Y, DYN_SANDBAG_1_X_SIZE, DYN_SANDBAG_1_Y_SIZE);
		kernel_add_dynamic(words_sandbag, words_look_at, SYNTAX_SINGULAR, KERNEL_NONE,
		                   DYN_SANDBAG_2_X, DYN_SANDBAG_2_Y, DYN_SANDBAG_2_X_SIZE, DYN_SANDBAG_2_Y_SIZE);
		kernel_add_dynamic(words_sandbag, words_look_at, SYNTAX_SINGULAR, KERNEL_NONE,
		                   DYN_SANDBAG_3_X, DYN_SANDBAG_3_Y, DYN_SANDBAG_3_X_SIZE, DYN_SANDBAG_3_Y_SIZE);
		kernel_add_dynamic(words_sandbag, words_look_at, SYNTAX_SINGULAR, KERNEL_NONE,
		                   DYN_SANDBAG_4_X, DYN_SANDBAG_4_Y, DYN_SANDBAG_4_X_SIZE, DYN_SANDBAG_4_Y_SIZE);
		dyn_floor = kernel_add_dynamic(words_stage, words_walk_across, SYNTAX_SINGULAR, KERNEL_NONE,
		                               DYN_FLOOR_1_X, DYN_FLOOR_1_Y, DYN_FLOOR_1_X_SIZE, DYN_FLOOR_1_Y_SIZE);
		kernel_dynamic_walk(dyn_floor, DYN_FLOOR_1_WALK_TO_X, DYN_FLOOR_1_WALK_TO_Y, 5);
		dyn_floor = kernel_add_dynamic(words_stage, words_walk_across, SYNTAX_SINGULAR, KERNEL_NONE,
		                               DYN_FLOOR_2_X, DYN_FLOOR_2_Y, DYN_FLOOR_2_X_SIZE, DYN_FLOOR_2_Y_SIZE);
		kernel_dynamic_walk(dyn_floor, DYN_FLOOR_2_WALK_TO_X, DYN_FLOOR_2_WALK_TO_Y, 5);

	} else {
		kernel_flip_hotspot(words_boxes, false);
		kernel_flip_hotspot(words_prop_case, false);  /* 'case' renamed: C++ keyword */
	}


	if ((object_is_here(cable_hook)) && (global[current_year] == 1881) && !player_has(rope_with_hook)) {
		seq[fx_cable_hook] = kernel_seq_stamp(ss[fx_cable_hook], false, 1);
		kernel_seq_depth(seq[fx_cable_hook], 3);
	} else {
		kernel_flip_hotspot(words_cable_hook, false);
	}


	/* ==================== Previous Rooms ========================== */

	if (previous_room == 109) {
		player.x      = WALK_TO_X_BEHIND_DOOR;
		player.y      = WALK_TO_Y_BEHIND_DOOR - 2; /* -2 gives walking time during fade in */
		player.facing = FACING_SOUTH;
		player_walk(FRONT_OF_DOOR_X, FRONT_OF_DOOR_Y, FACING_SOUTH);
		player_walk_trigger(ROOM_106_DOOR_CLOSES);
		player.commands_allowed = false;
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 5);
		kernel_seq_depth(seq[fx_door], 14);

	} else if ((previous_room == 105) || (previous_room == 301)) {
		player.x      = WALK_TO_AFTER_ANIM_X - 1;  /* So daemon won't send us to 105 or 321 */
		player.y      = WALK_TO_AFTER_ANIM_Y;
		player.facing = FACING_WEST;
		player_walk(PLAYER_X_FROM_105, PLAYER_Y_FROM_105, FACING_WEST);
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 14);

	} else if (previous_room == 107) {
		player.x      = PLAYER_X_FROM_107;
		player.y      = PLAYER_Y_FROM_107;
		player.facing = FACING_WEST;
		player_walk(WALK_TO_X_FROM_107, WALK_TO_Y_FROM_107, FACING_SOUTHEAST);
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 14);

	} else if ((previous_room == 108) || (previous_room != KERNEL_RESTORING_GAME)) {
		player_first_walk(OFF_SCREEN_X_FROM_108, OFF_SCREEN_Y_FROM_108, FACING_SOUTHEAST,
		                  PLAYER_X_FROM_108, PLAYER_Y_FROM_108, FACING_NORTHEAST, true);
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 14);

	} else if (previous_room == KERNEL_RESTORING_GAME) {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 14);
	}

	section_1_music();
}

void room_106_daemon() {
	switch (kernel.trigger) {
	case ROOM_106_INIT_TEXT:
		global_speech_go(speech_christine_scales);
		kernel_timing_trigger(TWO_SECONDS, ROOM_106_INIT_TEXT + 1);
		break;

	case ROOM_106_INIT_TEXT + 1:
		text_show(text_106_37);
		break;

	case ROOM_106_DOOR_CLOSES:
		kernel_seq_delete(seq[fx_door]);
		seq[fx_door] = kernel_seq_backward(ss[fx_door], false,
		                                   8, 0, 0, 1);
		kernel_seq_depth(seq[fx_door], 14);
		kernel_seq_range(seq[fx_door], 1, 5);
		kernel_seq_trigger(seq[fx_door],
		                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_106_DOOR_CLOSES + 1);
		break;

	case ROOM_106_DOOR_CLOSES + 1:
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 14);
		sound_play(N_DoorCloses);
		player.commands_allowed = true;
		break;
	}
}

void room_106_pre_parser() {
	if (player_said_2(exit_to, stage_left_wing)) {
		player.walk_off_edge_to_room = 108;
	}

	if (player_said_2(open, door)) {
		player_walk(FRONT_OF_DOOR_X, FRONT_OF_DOOR_Y, FACING_NORTHWEST);
	}
}

void room_106_parser() {
	int temp; /* For synching purposes */

	if ((player_said_2(take, sandbag)) && (inter_point_y < 61)) {
		text_show(text_106_35);
		goto handled;
		/* the sandbags are too high up! */

	} else if (player_said_2(take, sandbag)) {
		if (object_is_here(sandbag)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible   = false;
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
				kernel_seq_delete(seq[fx_sandbag]);
				kernel_delete_dynamic(local->dynamic_sandbag);
				sound_play(N_TakeObjectSnd);
				break;

			case 2:
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_bend_down_9]);
				player.walker_visible = true;
				kernel_timing_trigger(20, 3);
				break;

			case 3:
				inter_give_to_player(sandbag);
				/* You pick up the sandbag. */
				object_examine(sandbag, text_008_03, 0);
				player.commands_allowed = true;
				break;
			}
			goto handled;
		}
	}

	if (player_said_2(take, cable_hook)) {
		if (object_is_here(cable_hook)) {
			switch (kernel.trigger) {
			case 0:
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
				global[player_score] += 5;
				break;

			case 1:
				kernel_seq_delete(seq[fx_cable_hook]);
				kernel_flip_hotspot(words_cable_hook, false);
				sound_play(N_TakeObjectSnd);
				break;

			case 2:
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_bend_down_9]);
				player.walker_visible = true;
				kernel_timing_trigger(20, 3);
				break;

			case 3:
				inter_give_to_player(cable_hook);
				/* You pick up the cable hook. */
				object_examine(cable_hook, text_008_22, 0);
				player.commands_allowed = true;
				break;
			}
			goto handled;
		}
	}

	switch (kernel.trigger) {
	case ROOM_106_CLIMB_UP:
		player.commands_allowed = false;
		player.walker_visible   = false;
		aa[0] = kernel_run_animation(kernel_name('u', 1), ROOM_106_CLIMB_UP + 1);
		kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
		goto handled;
		break;

	case ROOM_106_CLIMB_UP + 1:
		kernel_timing_trigger(TWO_SECONDS, ROOM_106_CLIMB_UP + 2);
		goto handled;
		break;

	case ROOM_106_CLIMB_UP + 2:
		new_room = 301;
		goto handled;
		break;

	case ROOM_106_CLIMB_DOWN:
		player.commands_allowed = false;
		player.walker_visible   = false;
		aa[0] = kernel_run_animation(kernel_name('d', 1), ROOM_106_CLIMB_DOWN + 1);
		kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
		goto handled;
		break;

	case ROOM_106_CLIMB_DOWN + 1:
		new_room = 105;
		goto handled;
		break;
	}

	if (player_said_2(exit_to, stage_right_wing)) {
		new_room = 107;
		goto handled;
	}

	if (player_said_1(climb_down) || player_said_1(climb_up)) {
		if (global[sandbag_status] == SANDBAG_DROPPED) {
			switch (kernel.trigger) {
			case 0:
				aa[0]                   = kernel_run_animation(kernel_name('s', -1), 1);
				player.commands_allowed = false;
				player.walker_visible   = false;
				break;

			case 1:
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
				player.walker_visible = true;
				player.x              = AFTER_LOOK_ANIM_X;
				player.y              = AFTER_LOOK_ANIM_Y;
				player_demand_facing(FACING_EAST);
				player_walk(WALK_TO_AFTER_ANIM_X, WALK_TO_AFTER_ANIM_Y, FACING_EAST);

				if (player_said_1(climb_down)) {
					player_walk_trigger(ROOM_106_CLIMB_DOWN);
				} else {
					player_walk_trigger(ROOM_106_CLIMB_UP);
				}
				break;
			}
		} else {
			switch (kernel.trigger) {
			case 0:
				seq[fx_sandbag] = kernel_seq_forward(ss[fx_sandbag], false,
				                                     4, 0, 0, 1);
				kernel_seq_depth(seq[fx_sandbag], 2);
				kernel_seq_range(seq[fx_sandbag], 1, 9);
				kernel_timing_trigger(TENTH_SECOND, 2);
				kernel_seq_trigger(seq[fx_sandbag],
				                   KERNEL_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				sound_play(N_SandbagThud);
				seq[fx_sandbag] = kernel_seq_stamp(ss[fx_sandbag], false, KERNEL_LAST);
				kernel_seq_depth(seq[fx_sandbag], 2);
				local->dynamic_sandbag = kernel_add_dynamic(words_sandbag, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
				                         SANDBAG_X, SANDBAG_Y, SANDBAG_XS, SANDBAG_YS);
				kernel_dynamic_walk(local->dynamic_sandbag, SANDBAG_WALK_TO_X, SANDBAG_WALK_TO_Y, FACING_NORTHEAST);
				break;

			case 2:
				player.commands_allowed = false;
				player.walker_visible   = false;
				aa[0] = kernel_run_animation(kernel_name('s', 1), 3);
				kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
				break;

			case 3:
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
				player.walker_visible   = true;
				player.commands_allowed = true;
				player.x = AFTER_LOOK_ANIM_X + 3;
				player.y = AFTER_LOOK_ANIM_Y - 3;
				player_demand_facing(FACING_SOUTHEAST);
				global[sandbag_status] = SANDBAG_DROPPED;
				text_show(text_106_32);
				/* sandbag just fell */
				break;
			}
		}
		goto handled;
	}

	if (player_said_2(walk_through, door) || player_said_2(open, door)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			seq[fx_take_9] = kernel_seq_pingpong(ss[fx_take_9], false,
			                                     5, 0, 0, 2);
			kernel_seq_range(seq[fx_take_9], 1, 4);
			kernel_seq_player(seq[fx_take_9], true);
			kernel_seq_trigger(seq[fx_take_9],
			                   KERNEL_TRIGGER_SPRITE, 4, ROOM_106_DOOR_OPENS);
			kernel_seq_trigger(seq[fx_take_9],
			                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_106_DOOR_OPENS + 2);
			break;

		case ROOM_106_DOOR_OPENS:
			sound_play(N_DoorOpens);
			kernel_seq_delete(seq[fx_door]);
			seq[fx_door] = kernel_seq_forward(ss[fx_door], false,
			                                  8, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 14);
			kernel_seq_range(seq[fx_door], 1, 5);
			kernel_seq_trigger(seq[fx_door],
			                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_106_DOOR_OPENS + 1);
			break;

		case ROOM_106_DOOR_OPENS + 1:
			temp = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 5);
			kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, temp);
			kernel_seq_depth(seq[fx_door], 14);
			break;

		case ROOM_106_DOOR_OPENS + 2:
			player.walker_visible = true;
			player_walk(WALK_TO_X_BEHIND_DOOR, WALK_TO_Y_BEHIND_DOOR, FACING_NORTH);
			player_walk_trigger(ROOM_106_DOOR_OPENS + 3);
			break;

		case ROOM_106_DOOR_OPENS + 3:
			kernel_seq_delete(seq[fx_door]);
			seq[fx_door] = kernel_seq_backward(ss[fx_door], false,
			                                   8, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 1);
			kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_door],
			                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_106_DOOR_OPENS + 4);
			break;

		case ROOM_106_DOOR_OPENS + 4:
			sound_play(N_DoorCloses);
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door], 1);
			new_room = 109;
			break;
		}
		goto handled;
	}

	if (player.look_around) {
		text_show(text_106_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(stage_right_wing)) {
			text_show(text_106_11);
			goto handled;
		}

		if (player_said_1(stage_left_wing)) {
			text_show(text_106_12);
			goto handled;
		}

		if (player_said_1(_in_two_)) {
			text_show(text_106_13);
			goto handled;
		}

		if (player_said_1(cyclorama)) {
			text_show(text_106_14);
			goto handled;
		}

		if (player_said_1(stage)) {
			text_show(text_106_15);
			goto handled;
		}

		if (player_said_1(pedestal)) {
			text_show(text_106_16);
			goto handled;
		}

		if (player_said_1(plant_prop)) {
			text_show(text_106_17);
			goto handled;
		}

		if (player_said_1(sandbag)) {
			if (inter_point_y < 60) {
				text_show(text_106_18);
				goto handled;
			} else {
				if (object_is_here(sandbag)) {
					text_show(text_106_33);
					goto handled;
				}
			}
		}

		if (player_said_1(statue)) {
			text_show(text_106_19);
			goto handled;
		}

		if (player_said_1(circular_staircase)) {
			text_show(text_106_20);
			goto handled;
		}

		if (player_said_1(batten)) {
			text_show(text_106_21);
			goto handled;
		}

		if (player_said_1(door)) {
			text_show(text_106_22);
			goto handled;
		}

		if (player_said_1(boxes) || player_said_1(box)) {
			text_show(text_106_23);
			goto handled;
		}

		if (player_said_1(big_prop)) {
			text_show(text_106_24);
			goto handled;
		}

		if (player_said_1(crate)) {
			text_show(text_106_25);
			goto handled;
		}

		if (player_said_1(prop_case)) {  /* 'case' renamed: C++ keyword */
			if (global[current_year] == 1881) {
				text_show(text_106_38);
			} else {
				text_show(text_106_36);
			}
			goto handled;
		}

		if (player_said_1(ventilation_duct)) {
			text_show(text_106_26);
			goto handled;
		}

		if (player_said_1(junk)) {
			text_show(text_106_27);
			goto handled;
		}

		if (player_said_1(flats)) {
			text_show(text_106_28);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_106_29);
			goto handled;
		}

		if (player_said_1(cable_hook) && object_is_here(cable_hook)) {
			text_show(text_106_39);
			goto handled;
		}
	}

	if (player_said_2(talk_to, statue)) {
		text_show(text_106_30);
		goto handled;
	}

	if (player_said_2(close, door)) {
		text_show(text_106_34);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_106_preload() {
	room_init_code_pointer       = room_106_init;
	room_pre_parser_code_pointer = room_106_pre_parser;
	room_parser_code_pointer     = room_106_parser;
	room_daemon_code_pointer     = room_106_daemon;

	if (global[current_year] == 1881) {
		kernel_initial_variant = 1;
	}
	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
