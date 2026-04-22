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
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/quotes.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/rooms/section3.h"
#include "mads/madsv2/phantom/rooms/room302.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_302_init(void) {
	/* =================== Load sprite series ==================== */

	ss[fx_blue_frame] = kernel_load_series(kernel_name('p', 0), false);
	ss[fx_bend_down_9] = kernel_load_series("*RRD_9", false);


	/* ================ If in 1881, put blue_frame ============= */

	if (object_is_here(blue_frame)) {
		seq[fx_blue_frame] = kernel_seq_stamp(ss[fx_blue_frame], false, 1);
		kernel_seq_depth(seq[fx_blue_frame], 14);
	} else {
		kernel_flip_hotspot(words_blue_frame, false);
	}


	/* =============== If in 1993, stamp down z's ================ */

	if (global[current_year] == 1993) {
		ss[fx_1993] = kernel_load_series(kernel_name('z', -1), false);
		kernel_draw_to_background(ss[fx_1993], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
	}

	/* ========================= Previous Rooms ================== */

	if (previous_room == 303) {
		player_first_walk(OFF_SCREEN_X_FROM_303, OFF_SCREEN_Y_FROM_303, FACING_WEST,
			PLAYER_X_FROM_303, PLAYER_Y_FROM_303, FACING_EAST, true);
	} else if ((previous_room == 301) || (previous_room != KERNEL_RESTORING_GAME)) {
		player_first_walk(OFF_SCREEN_X_FROM_301, OFF_SCREEN_Y_FROM_301, FACING_WEST,
			PLAYER_X_FROM_301, PLAYER_Y_FROM_301, FACING_WEST, true);
	}
	section_3_music();
}

void room_302_pre_parser(void) {
	if (player_said_2(exit_to, catwalk)) {
		if (inter_point_x > 160) {
			player.walk_off_edge_to_room = 301;
		} else {
			player.walk_off_edge_to_room = 303;
		}
	}
}

void room_302_parser(void) {
	int count = 0;

	if (player_said_2(take, blue_frame) &&
		(object_is_here(blue_frame) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			if (global[current_year] == 1881) {
				if (player_has(yellow_frame)) ++count;
				if (player_has(red_frame))    ++count;
				if (player_has(green_frame))  ++count;
				if (count < 3) global[player_score] += 5;
			}

			player.commands_allowed = false;
			player.walker_visible = false;
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
			kernel_seq_delete(seq[fx_blue_frame]);
			kernel_flip_hotspot(words_blue_frame, false);
			inter_give_to_player(blue_frame);
			sound_play(N_TakeObjectSnd);
			break;

		case 2:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_bend_down_9]);
			player.walker_visible = true;
			kernel_timing_trigger(20, 3);
			break;

		case 3:
			if (global[current_year] == 1881) {
				object_examine(blue_frame, text_008_44, 0);
			} else {
				object_examine(blue_frame, text_008_17, 0);
			}
			/* You pick up the blue color frame */
			player.commands_allowed = true;
			break;
		}
		goto handled;
	}

	if (player.look_around) {
		text_show(text_302_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(catwalk)) {
			text_show(text_302_11);
			goto handled;
		}

		if (player_said_1(grid)) {
			text_show(text_302_12);
			goto handled;
		}

		if (player_said_1(hemp)) {
			text_show(text_302_13);
			goto handled;
		}

		if (player_said_1(side_wall)) {
			text_show(text_302_14);
			goto handled;
		}

		if (player_said_1(catwalk)) {
			text_show(text_302_15);
			goto handled;
		}

		if (player_said_1(railing)) {
			text_show(text_302_16);
			goto handled;
		}

		if (player_said_1(beam_position)) {
			text_show(text_302_17);
			goto handled;
		}

		if (player_said_1(lighting_instrument)) {
			if (global[current_year] == 1993) {
				text_show(text_302_18);
			} else {
				text_show(text_302_19);
			}
			goto handled;
		}

		if (player_said_1(blue_frame) && object_is_here(blue_frame)) {
			if (global[current_year] == 1993) {
				text_show(text_302_20);
			} else {
				text_show(text_302_21);
			}
			goto handled;
		}

		if (player_said_1(support)) {
			text_show(text_302_22);
			goto handled;
		}

		if (player_said_1(other_catwalk)) {
			text_show(text_302_23);
			goto handled;
		}
	}

	if (player_said_2(take, hemp)) {
		text_show(text_302_24);
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

void room_302_preload(void) {
	room_init_code_pointer = room_302_init;
	room_pre_parser_code_pointer = room_302_pre_parser;
	room_parser_code_pointer = room_302_parser;
	room_daemon_code_pointer = NULL;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
