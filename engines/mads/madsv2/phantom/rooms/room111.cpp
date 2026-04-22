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
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/rooms/room111.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

static void handle_animation_listen() {
	int listen_reset_frame;

	if (kernel_anim[aa[0]].frame != local->listen_frame) {
		local->listen_frame = kernel_anim[aa[0]].frame;
		listen_reset_frame  = -1;

		switch (local->listen_frame) {
		case 6:   /* end of freeze (ear to door) */

			if (local->listen_action == CONV14_LISTEN) {
				listen_reset_frame = 5;
				break; /* keep ear to door */
			}

			if (local->listen_action == CONV14_UN_LISTEN) {
				listen_reset_frame = 7;
				break; /* back off from door */
			}
			break;
		}

		if (listen_reset_frame >= 0) {
			kernel_reset_animation(aa[0], listen_reset_frame);
			local->listen_frame = listen_reset_frame;
		}
	}
}

void room_111_init() {
	int temp_dynamic;     /* For dynamic hotspots */

	kernel_flip_hotspot(words_hook, false);

	local->delete_axe     = false;
	local->anim_0_running = false;
	local->anim_1_running = false;
	local->it_is_closed   = true;

	if (global[current_year] == 1881) {
		if ((global[jacques_name_is_known] == YES_AND_END_CONV) &&
		    (global[madame_name_is_known] == YES_AND_END_CONV) &&
		    (global[panel_in_206]) &&
		    (global[done_rich_conv_203]) &&
		    (player_has(lantern)) &&
		    ((player_has(cable_hook) && player_has(rope)) || player_has(rope_with_hook))) {
			local->it_is_closed = false;
		} else {
			local->it_is_closed = true;
		}
	} else {
		local->it_is_closed = false;
	}

	if (global[jacques_status]) {
		kernel_flip_hotspot(words_hook, true);
		kernel_flip_hotspot(words_fire_axe, false);
	}


	/* =================== Load conversation ======================== */

	conv_get(CONV_LISTEN);


	/* ==================== Load Sprite Series ====================== */

	ss[fx_take_9]    = kernel_load_series("*RDR_9", false);
	ss[fx_axe]       = kernel_load_series(kernel_name('a', 1), false);
	ss[fx_open_door] = kernel_load_series(kernel_name('a', 3), false);
	ss[fx_door]      = kernel_load_series(kernel_name('x', 0), false);

	if (global[current_year] == 1881) {
		ss[fx_broken_axe] = kernel_load_series(kernel_name('a', 2), false);
	}


	/* =================== Draw 1993 sprite ========================= */

	if (global[current_year] == 1993) {
		ss[fx_1993] = kernel_load_series(kernel_name('z', -1), false);
		kernel_draw_to_background(ss[fx_1993], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_flip_hotspot(words_lamp, false);
		temp_dynamic = kernel_add_dynamic(words_wall, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                                  WALL_X, WALL_Y, WALL_X_SIZE, WALL_Y_SIZE);
		kernel_dynamic_walk(temp_dynamic, WALL_WALK_X, WALL_WALK_Y, FACING_NORTHEAST);

	} else {
		kernel_flip_hotspot(words_light, false);
		if (global[christine_door_status] == CHRIS_DOOR_CHOPPED) {
			seq[fx_broken_axe] = kernel_seq_stamp(ss[fx_broken_axe], false, 1);
			kernel_seq_depth(seq[fx_broken_axe], 10);
			kernel_draw_to_background(ss[fx_open_door], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
			kernel_flip_hotspot(words_fire_axe, false);
			kernel_flip_hotspot(words_door_chunks, true);
			kernel_flip_hotspot(words_handle, true);
			kernel_flip_hotspot(words_axe, true);
		}
	}

	if ((global[christine_door_status] == CHRIS_IS_IN) &&
	    (previous_room != 113) && (!local->it_is_closed)) {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 3);
		kernel_seq_depth(seq[fx_door], 14);
		/* now it is set up so that CHRIS_IS_IN will apply to both times (for Daae & Florent) */
	}

	if ((global[current_year] == 1993) || (global[christine_door_status] <= CHRIS_DOOR_CLOSED)) {
		if (!global[jacques_status]) {
			seq[fx_axe] = kernel_seq_stamp(ss[fx_axe], false, 1);
			kernel_seq_depth(seq[fx_axe], 5);
			kernel_flip_hotspot(words_axe, false);
		}
		kernel_flip_hotspot(words_door_chunks, false);
		kernel_flip_hotspot(words_handle, false);
	}


	/* ====================== Previous Rooms ======================== */

	if (previous_room == 113) {
		player.x = PLAYER_X_FROM_113_SAWDUST;
		player.y = PLAYER_Y_FROM_113_SAWDUST;
		player_walk(PLAYER_X_FROM_113, PLAYER_Y_FROM_113, FACING_SOUTH);

		if (!global[leave_angel_music_on]) {
			section_1_music();
		}

		if (global[christine_door_status] != CHRIS_DOOR_CHOPPED) {
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 3);
			kernel_seq_depth(seq[fx_door], 14);

			if ((global[current_year] == 1881) && (global[leave_angel_music_on])) {
				/* Daae kicked Raoul out — send to daemon to close door */
				player_walk_trigger(ROOM_111_MADE_IT_OUT_DOOR);
				player.commands_allowed        = false;
				player.commands_allowed        = false;
				global[christine_door_status]  = CHRIS_DOOR_CLOSED;
				global[chris_kicked_raoul_out] = true;
				global[hint_that_daae_is_home_1] = true;
				global[hint_that_daae_is_home_2] = true;
			}
		}

	} else if ((previous_room == 109) || (previous_room != KERNEL_RESTORING_GAME)) {
		player_first_walk(OFF_SCREEN_X_FROM_109, OFF_SCREEN_Y_FROM_109, FACING_WEST,
		                  PLAYER_X_FROM_109, PLAYER_Y_FROM_109, FACING_WEST, true);
	}

	section_1_music();
}

void room_111_daemon() {
	if (local->anim_0_running) {
		handle_animation_listen();
	}

	if ((!local->delete_axe) && (local->anim_1_running) &&
	    (kernel_anim[local->animation[1]].frame == 5)) {
		kernel_seq_delete(seq[fx_axe]);
		inter_give_to_player(fire_axe);
		local->delete_axe = true;
	}

	if ((player_has(fire_axe)) && (local->anim_1_running) &&
	    (kernel_anim[local->animation[1]].frame == 36)) {
		inter_move_object(fire_axe, NOWHERE);
	}

	if ((player.x == WALK_TO_X_INSIDE_112) && (player.y == WALK_TO_Y_INSIDE_112)) {
		new_room = 113;
	}


	switch (kernel.trigger) {

	case ROOM_111_MADE_IT_OUT_DOOR:
		kernel_seq_delete(seq[fx_door]);
		seq[fx_door] = kernel_seq_backward(ss[fx_door], false, 7, 0, 0, 1);
		kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_111_CLOSED_DOOR);
		kernel_seq_depth(seq[fx_door], 14);
		kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
		break;

	case ROOM_111_CLOSED_DOOR:
		sound_play(N_DoorCloses);
		player.commands_allowed = true;
		conv_run(CONV_LISTEN);
		if (game.difficulty == EASY_MODE) {
			conv_export_value(1);
		} else {
			conv_export_value(0);
		}
		player_walk(LISTEN_X, LISTEN_Y, FACING_EAST);
		player_walk_trigger(ROOM_111_AT_LISTEN_POINT);
		break;

	case ROOM_111_AT_LISTEN_POINT:
		aa[0] = kernel_run_animation(kernel_name('l', 1), ROOM_111_DONE_LISTENING);
		kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
		local->anim_0_running = true;
		local->listen_action  = CONV14_LISTEN;
		player.walker_visible = false;
		break;

	case ROOM_111_DONE_LISTENING:
		player.walker_visible = true;
		local->anim_0_running = false;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
		kernel_timing_trigger(HALF_SECOND, ROOM_111_DONE_LISTENING + 1);
		break;

	case ROOM_111_DONE_LISTENING + 1:
		conv_release();
		break;
	}
}

static void process_conversation_listen() {
	if (player_verb == conv014_second_angel) {
		conv_you_trigger(ROOM_111_LISTEN);
	}

	if (player_verb == conv014_eighth_inside) {
		conv_you_trigger(ROOM_111_UN_LISTEN);
	}

	if (kernel.trigger == ROOM_111_LISTEN) {
		local->listen_action = CONV14_LISTEN;
	}

	if (kernel.trigger == ROOM_111_UN_LISTEN) {
		local->listen_action = CONV14_UN_LISTEN;
		conv_hold();
	}

	/* ================= Set up me and you triggers ================= */

	if ((player_verb != conv014_second_angel) &&
	    (player_verb != conv014_eighth_inside)) {
		conv_you_trigger(ROOM_111_NOTHING);
	}
}

void room_111_pre_parser() {
	if (player_said_2(exit_to, stairwell)) {
		if (global[leave_angel_music_on]) {
			text_show(text_111_39);
			player.need_to_walk = false;
			player_cancel_command();
			/* You shouldn't leave now, Christine is behind door */
		} else {
			player.walk_off_edge_to_room = 109;
		}
	}

	if (player_said_2(take, fire_axe) && object_is_here(fire_axe) && global[current_year] == 1881) {
		if ((global[christine_door_status] == CHRIS_DOOR_CLOSED) &&
		    (global[chris_kicked_raoul_out]) && (global[jacques_status] == JACQUES_IS_ALIVE)) {
			player_walk(LISTEN_X, LISTEN_Y, FACING_EAST);  /* also for taking axe */
		}
	}

	if (player_said_2(open, right_door) || player_said_2(unlock, right_door) ||
	    player_said_2(lock, right_door)) {
		player_walk(FRONT_RIGHT_DOOR_X, FRONT_RIGHT_DOOR_Y, FACING_NORTHEAST);
	}

	if (player_said_2(open, left_door) || player_said_2(unlock, left_door) ||
	    player_said_2(lock, left_door)) {
		player_walk(FRONT_LEFT_DOOR_X, FRONT_LEFT_DOOR_Y, FACING_NORTHEAST);
	}

	if ((player_said_2(walk_through, left_door) || player_said_2(unlock, left_door) ||
	     player_said_2(lock, left_door)) && (!local->it_is_closed)) {
		if ((global[christine_door_status] == CHRIS_DOOR_CHOPPED) ||
		    (global[christine_door_status] == CHRIS_IS_IN)) {
			player_walk(WALK_TO_X_INSIDE_112, WALK_TO_Y_INSIDE_112, FACING_NORTHEAST);
		}
	}
}

void room_111_parser() {
	int temp;   /* For synching purposes */

	if (conv_control.running == CONV_LISTEN) {
		process_conversation_listen();
		goto handled;
	}

	if (player_said_2(walk_through, right_door) || player_said_2(open, right_door) ||
	    player_said_1(unlock) || player_said_1(lock)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			seq[fx_take_9] = kernel_seq_forward(ss[fx_take_9], false, 5, 0, 0, 1);
			kernel_seq_range(seq[fx_take_9], 1, 4);
			kernel_seq_player(seq[fx_take_9], true);
			kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, 1);
			goto handled;
			break; // dead code; preserved from original

		case 1:
			temp = seq[fx_take_9];
			seq[fx_take_9] = kernel_seq_stamp(ss[fx_take_9], false, 4);
			kernel_synch(KERNEL_SERIES, seq[fx_take_9], KERNEL_SERIES, temp);
			kernel_seq_player(seq[fx_take_9], false);
			kernel_timing_trigger(HALF_SECOND, 2);
			sound_play(N_DoorHandle);
			goto handled;
			break; // dead code; preserved from original

		case 2:
			kernel_seq_delete(seq[fx_take_9]);
			seq[fx_take_9] = kernel_seq_backward(ss[fx_take_9], false, 5, 0, 0, 1);
			kernel_seq_range(seq[fx_take_9], 1, 4);
			kernel_seq_player(seq[fx_take_9], false);
			kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, 3);
			goto handled;
			break; // dead code; preserved from original

		case 3:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_9]);
			player.walker_visible = true;
			if (player_said_1(lock) || player_said_1(unlock)) {
				text_show(text_000_32);
				/* the key won't work */
			} else {
				text_show(text_111_23);
				/* the door is locked */
			}
			player.commands_allowed = true;
			goto handled;
			break; // dead code; preserved from original
		}
	}


	if (player_said_2(walk_through, left_door) || player_said_2(open, left_door) ||
	    player_said_2(unlock, left_door) || player_said_2(lock, left_door)) {
		if ((global[christine_door_status] == CHRIS_DOOR_CLOSED) || (local->it_is_closed) ||
		     player_said_2(unlock, left_door) || player_said_2(lock, left_door)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take_9] = kernel_seq_forward(ss[fx_take_9], false, 5, 0, 0, 1);
				kernel_seq_range(seq[fx_take_9], 1, 4);
				kernel_seq_player(seq[fx_take_9], true);
				kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, 1);
				goto handled;
				break; // dead code; preserved from original

			case 1:
				temp = seq[fx_take_9];
				seq[fx_take_9] = kernel_seq_stamp(ss[fx_take_9], false, 4);
				kernel_synch(KERNEL_SERIES, seq[fx_take_9], KERNEL_SERIES, temp);
				kernel_seq_player(seq[fx_take_9], false);
				kernel_timing_trigger(HALF_SECOND, 2);
				sound_play(N_DoorHandle);
				goto handled;
				break; // dead code; preserved from original

			case 2:
				kernel_seq_delete(seq[fx_take_9]);
				seq[fx_take_9] = kernel_seq_backward(ss[fx_take_9], false, 5, 0, 0, 1);
				kernel_seq_range(seq[fx_take_9], 1, 4);
				kernel_seq_player(seq[fx_take_9], false);
				kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, 3);
				goto handled;
				break; // dead code; preserved from original

			case 3:
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_9]);
				player.walker_visible = true;

				if (player_said_1(lock) || player_said_1(unlock)) {
					text_show(text_000_32);
					/* the key does not work here */
				} else if ((global[chris_kicked_raoul_out]) /*&& (local->it_is_closed)*/ &&
				           (global[ticket_people_here] == SELLER) &&
				           (global[jacques_status] == JACQUES_IS_ALIVE)) {
					text_show(text_111_35);
					/* the door is locked! */
				} else {
					text_show(text_111_24);
					/* the door is locked no one is inside */
				}
				player.commands_allowed = true;
				goto handled;
				break; // dead code; preserved from original
			}

		} else if (global[christine_door_status] == CHRIS_DOOR_CHOPPED) {
			text_show(text_111_37);
			goto handled;
			/* the door is chopped up */
		} else {
			text_show(text_111_26);
			goto handled;  /* the door is already open, someone is inside */
		}
	}

	if (player_said_2(take, fire_axe)) {
		if ((global[current_year] == 1881) &&
		    (global[jacques_status] == JACQUES_IS_ALIVE) &&
		    (global[christine_door_status] == CHRIS_DOOR_CLOSED) &&
		    (global[chris_kicked_raoul_out])) {
			switch (kernel.trigger) {
			case 0:
				aa[1] = kernel_run_animation(kernel_name('q', -1), 1);
				kernel_flip_hotspot(words_fire_axe, false);
				kernel_flip_hotspot(words_door_chunks, true);
				kernel_flip_hotspot(words_handle, true);
				kernel_flip_hotspot(words_axe, true);
				local->anim_1_running   = true;
				player.commands_allowed = false;
				player.walker_visible   = false;
				break;

			case 1:
				global[christine_door_status] = CHRIS_DOOR_CHOPPED;
				seq[fx_broken_axe] = kernel_seq_stamp(ss[fx_broken_axe], false, 1);
				kernel_synch(KERNEL_SERIES, seq[fx_broken_axe], KERNEL_ANIM, local->animation[1]);
				kernel_seq_depth(seq[fx_broken_axe], 10);
				kernel_draw_to_background(ss[fx_open_door], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, local->animation[1]);
				player.walker_visible = true;
				player.x              = AFTER_AXE_ANIM_X;
				player.y              = AFTER_AXE_ANIM_Y;
				local->anim_1_running = false;
				player_walk(WALK_TO_X_INSIDE_112, WALK_TO_Y_INSIDE_112, FACING_NORTHEAST);
				player_demand_facing(FACING_NORTHWEST);
				break;
			}
		} else {
			if (global[current_year] == 1993 && global[make_brie_leave_203]) {
				text_show(text_111_42);
				/* you would not want to hack up a murderer (right to life) */
			} else {
				text_show(text_111_34);
				/* the axe is fine where it is */
			}
		}
		goto handled;
	}

	if (player.look_around) {
		if (global[current_year] == 1993) {
			text_show(text_111_10);
		} else {
			text_show(text_111_11);
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(floor)) {
			text_show(text_111_12);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_111_13);
			goto handled;
		}

		if (player_said_1(ceiling)) {
			text_show(text_111_14);
			goto handled;
		}

		if (player_said_1(stairwell)) {
			text_show(text_111_15);
			goto handled;
		}

		if (player_said_1(table)) {
			text_show(text_111_18);
			goto handled;
		}

		if (player_said_1(plant)) {
			text_show(text_111_19);
			goto handled;
		}

		if (player_said_1(fire_axe)) {
			text_show(text_111_20);
			goto handled;
		}

		if (player_said_1(handle)) {
			text_show(text_111_21);
			goto handled;
		}

		if (player_said_1(axe)) {
			text_show(text_111_22);
			goto handled;
		}

		if (player_said_1(door_chunks)) {
			text_show(text_111_29);
			goto handled;
		}

		if (player_said_1(light)) {
			text_show(text_111_31);
			goto handled;
		}

		if (player_said_1(lamp)) {
			text_show(text_111_32);
			goto handled;
		}

		if (player_said_1(hook)) {
			text_show(text_111_41);
			goto handled;
		}

		if (player_said_1(left_door)) {
			if (global[jacques_status]) {  /* true means he is dead in some way */
				text_show(text_111_40);
			} else if (global[christine_door_status] == CHRIS_DOOR_CHOPPED) {
				text_show(text_111_30);
			} else if ((global[chris_kicked_raoul_out]) && (global[ticket_people_here] == SELLER)) {
				text_show(text_111_36);
			} else if ((global[christine_door_status] == CHRIS_DOOR_CLOSED) || (local->it_is_closed)) {
				text_show(text_111_17);
			} else if (global[christine_door_status] == CHRIS_IS_IN) {
				text_show(text_111_16);
			}
			goto handled;
		}

		if (player_said_1(right_door)) {
			text_show(text_111_17);
			goto handled;
		}
	}

	if (player_said_2(close, right_door)) {
		text_show(text_111_28);
		goto handled;
	}

	if (player_said_2(close, left_door)) {
		if (local->it_is_closed) {
			text_show(text_111_28);
		} else if ((global[christine_door_status] == CHRIS_DOOR_CHOPPED) &&
		           (!global[ticket_people_here])) {
			text_show(text_111_38);
		} else if (global[christine_door_status] == CHRIS_DOOR_CLOSED) {
			text_show(text_111_27);
		} else if (global[christine_door_status] == CHRIS_IS_IN) {
			text_show(text_111_25);
		}
		goto handled;
	}

	if (player_said_2(open, left_door)) {
		if (global[christine_door_status] == CHRIS_IS_IN) {
			text_show(text_111_26);
		}
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}


void room_111_preload() {
	room_init_code_pointer       = room_111_init;
	room_pre_parser_code_pointer = room_111_pre_parser;
	room_parser_code_pointer     = room_111_parser;
	room_daemon_code_pointer     = room_111_daemon;

	if (global[current_year] == 1993) {
		kernel_initial_variant = 1;
	}

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
