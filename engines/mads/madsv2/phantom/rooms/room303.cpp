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
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/rail.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/quotes.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/rooms/section3.h"
#include "mads/madsv2/phantom/rooms/room303.h"
#include "mads/madsv2/core/conv.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_303_init(void) {
	local->anim_0_running = false;
	local->frame_guard = false;

	if (global[right_door_is_open_504]) {
		conv_get(CONV_MISC);
	}

	/* ==================== Load Sprite Series =================== */

	if (global[current_year] == 1993) {
		ss[fx_1993] = kernel_load_series(kernel_name('z', -1), false);
		ss[fx_large_note] = kernel_load_series(kernel_name('p', 0), false);
	}

	if ((object_is_here(large_note)) && (global[current_year] == 1993)) {
		seq[fx_large_note] = kernel_seq_stamp(ss[fx_large_note], false, 1);
		kernel_seq_depth(seq[fx_large_note], 4);
	} else {
		kernel_flip_hotspot(words_large_note, false);
	}

	/* =============== If in 1993, stamp down z's ================ */

	if (global[current_year] == 1993) {
		kernel_draw_to_background(ss[fx_1993], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);

	} else {
		local->dynamic_hemp = kernel_add_dynamic(words_chandelier_cable, words_climb_down, SYNTAX_SINGULAR, KERNEL_NONE,
			DYN_HEMP_X, DYN_HEMP_Y, DYN_HEMP_XS, DYN_HEMP_YS);
		kernel_dynamic_walk(local->dynamic_hemp, DYN_HEMP_WALK_TO_X, DYN_HEMP_WALK_TO_Y, FACING_NORTHWEST);
		kernel_dynamic_hot[local->dynamic_hemp].prep = PREP_ON;
		kernel_dynamic_cursor(local->dynamic_hemp, 3);
	}

	/* ========================= Previous Rooms ================== */

	if (previous_room == 307) {
		player_first_walk(OFF_SCREEN_X_FROM_307, OFF_SCREEN_Y_FROM_307, FACING_EAST,
			PLAYER_X_FROM_307, PLAYER_Y_FROM_307, FACING_EAST, true);

	} else if (previous_room == 304) {
		aa[0] = kernel_run_animation(kernel_name('u', 1), ROOM_303_DONE_CLIMBING_UP);
		player.commands_allowed = false;
		player.walker_visible = false;

	} else if (previous_room == 305) {
		inter_give_to_player(sword);
		player.x = PLAYER_X_FROM_305;
		player.y = PLAYER_Y_FROM_305;
		player.facing = FACING_SOUTHWEST;

	} else if ((previous_room == 302) || (previous_room != KERNEL_RESTORING_GAME)) {
		player_first_walk(OFF_SCREEN_X_FROM_302, OFF_SCREEN_Y_FROM_302, FACING_WEST,
			PLAYER_X_FROM_302, PLAYER_Y_FROM_302, FACING_WEST, true);
	}

	rail_disconnect_line(5, 9);
	rail_disconnect_line(5, 12);
	rail_disconnect_line(5, 8);
	rail_disconnect_line(6, 3);
	rail_disconnect_line(6, 2);
	rail_disconnect_line(11, 3);
	rail_disconnect_line(11, 4);
	rail_disconnect_line(10, 2);
	rail_disconnect_line(4, 9);
	rail_disconnect_line(8, 0);

	section_3_music();
}

void room_303_pre_parser(void) {
	if (player_said_2(exit_to, catwalk)) {
		if (!global[right_door_is_open_504]) {
			if (inter_point_x > 160) {
				player.walk_off_edge_to_room = 302;
			} else {
				player.walk_off_edge_to_room = 307;
			}
		}
	}

	if (player_said_2(climb_into, hole) || player_said_2(climb_down, chandelier_cable)) {
		player_walk(CLIMB_X, CLIMB_Y, FACING_SOUTHWEST);
	}
}

void room_303_parser(void) {
	if (((player_said_2(take, large_note)) && (object_is_here(large_note))) || ((kernel.trigger > 0) && kernel.trigger < 3)) {
		switch (kernel.trigger) {
		case (0):
			player.commands_allowed = false;
			player.walker_visible = false;
			aa[0] = kernel_run_animation(kernel_name('n', 1), 1);
			local->anim_0_running = true;
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
			global[player_score] += 5;
			break;

		case 1:
			local->anim_0_running = false;
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
			kernel_timing_trigger(20, 2);
			break;

		case 2:
			object_examine(large_note, text_008_18, 7);
			/* You pick up the large note */
			player.commands_allowed = true;
			break;
		}
		goto handled;
	}

	if (player_said_2(climb_into, hole) || player_said_2(climb_down, chandelier_cable)) {
		if (global[current_year] == 1881) {
			switch (kernel.trigger) {
			case 0:
				if (global[right_door_is_open_504]) {
					text_show(text_303_31);
				}
				aa[0] = kernel_run_animation(kernel_name('d', 1), 3);
				/* chose 3 because other triggers above use 3 */
				player.commands_allowed = false;
				player.walker_visible = false;
				kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
				break;

			case 3:
				new_room = 304;
				break;
			}
		} else {
			text_show(text_303_25);
			/* the rope is cut, you would fall to your death */
		}
		goto handled;
	}


	if (player_said_2(exit_to, catwalk)) {
		if (global[right_door_is_open_504]) {
			if (global_prefer_roland) {
				sound_play(N_WomanScream003);
			} else {
				global_speech_go(speech_woman_scream);
			}
			conv_run(CONV_MISC);
			conv_export_value(4);
			goto handled;
		}
	}


	if (player.look_around) {
		text_show(text_303_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_2(exit_to, catwalk)) {
			text_show(text_303_16);
			goto handled;
		}

		if (player_said_1(catwalk)) {
			text_show(text_303_11);
			goto handled;
		}

		if (player_said_1(grid)) {
			text_show(text_303_12);
			goto handled;
		}

		if (player_said_1(chandelier_cable)) {
			if (global[current_year] == 1993) {
				text_show(text_303_17);
			} else {
				if (global[right_door_is_open_504]) {
					text_show(text_303_30);
				} else {
					text_show(text_303_29);
				}
			}
			goto handled;
		}

		if (player_said_1(hemp)) {
			text_show(text_303_13);
			goto handled;
		}

		if (player_said_1(back_wall)) {
			text_show(text_303_14);
			goto handled;
		}

		if (player_said_1(ductwork)) {
			text_show(text_303_15);
			goto handled;
		}

		if (player_said_1(crate)) {
			text_show(text_303_18);
			goto handled;
		}

		if (player_said_1(support)) {
			text_show(text_303_19);
			goto handled;
		}

		if (player_said_1(piece_of_wood)) {
			text_show(text_303_20);
			goto handled;
		}

		if (player_said_1(railing)) {
			text_show(text_303_21);
			goto handled;
		}

		if (player_said_1(chandelier_trap)) {
			text_show(text_303_22);
			goto handled;
		}

		if (player_said_1(hole)) {
			if (global[current_year] == 1993) {
				text_show(text_303_26);
			} else {
				text_show(text_303_23);
			}
			goto handled;
		}

		if (player_said_1(large_note) && object_is_here(large_note)) {
			text_show(text_303_24);
			goto handled;
		}
	}

	if (player_said_2(walk_to, hole)) {
		text_show(text_303_25);
		goto handled;
	}

	if (player_said_2(take, hemp)) {
		text_show(text_303_27);
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

void room_303_daemon(void) {
	if (kernel.trigger == ROOM_303_DONE_CLIMBING_UP) {
		player.x = CLIMB_X;
		player.y = CLIMB_Y;
		player.commands_allowed = true;
		player.walker_visible = true;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
		player_demand_facing(FACING_SOUTHWEST);
	}

	if (local->anim_0_running) {
		if ((kernel_anim[aa[0]].frame >= 6) && (!local->frame_guard)) {
			local->frame_guard = true;
			kernel_seq_delete(seq[fx_large_note]);
			kernel_flip_hotspot(words_large_note, false);
			inter_give_to_player(large_note);
			sound_play(N_TakeObjectSnd);
		}
	}
}

void room_303_preload(void) {
	room_init_code_pointer = room_303_init;
	room_pre_parser_code_pointer = room_303_pre_parser;
	room_parser_code_pointer = room_303_parser;
	room_daemon_code_pointer = room_303_daemon;

	section_3_walker();
	section_3_interface();

	vocab_make_active(words_chandelier_cable);
	vocab_make_active(words_climb_down);
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
