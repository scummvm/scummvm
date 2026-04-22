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
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/rooms/room110.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_110_init() {
	/* ==================== Load Sprite Series ====================== */

	ss[fx_door]        = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_door_closed] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_take_9]      = kernel_load_series("*RDR_9", false);


	/* =========== Put light, cork board, julie's door ============== */

	if (global[current_year] == 1993) {
		ss[fx_1993] = kernel_load_series(kernel_name('z', -1), false);
		kernel_draw_to_background(ss[fx_1993], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_flip_hotspot(words_poster, false);
		kernel_flip_hotspot(words_waste_basket, false);

		if (global[done_brie_conv_203] == NO) {
			if (global[julies_door] == FULLY_OPEN) {
				seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 3);
				kernel_seq_depth(seq[fx_door], 8);
			}

		} else {
			seq[fx_door_closed] = kernel_seq_stamp(ss[fx_door_closed], false, 1);
			kernel_seq_depth(seq[fx_door_closed], 8);
		}

	} else {
		kernel_flip_hotspot(words_bulletin_board, false);
		kernel_flip_hotspot(words_paper, false);
		kernel_flip_hotspot(words_trash_bucket, false);
	}

	/* ====================== Previous Rooms ======================== */

	if (previous_room == 112) {
		player.x      = PLAYER_X_FROM_112;
		player.y      = PLAYER_Y_FROM_112;
		player.facing = FACING_SOUTH;
		player_walk(WALK_TO_X_FROM_112, WALK_TO_Y_FROM_112, FACING_SOUTH);

	} else if ((previous_room == 109) || (previous_room != KERNEL_RESTORING_GAME)) {
		player_first_walk(OFF_SCREEN_X_FROM_109, OFF_SCREEN_Y_FROM_109, FACING_WEST,
		                  PLAYER_X_FROM_109, PLAYER_Y_FROM_109, FACING_WEST, true);
	}

	section_1_music();
}

void room_110_pre_parser() {
	if (player_said_2(exit_to, stairwell)) {
		player.walk_off_edge_to_room = 109;
	}

	if (player_said_2(open, left_door) || player_said_2(unlock, left_door) ||
	    player_said_2(lock, left_door)) {
		player_walk(FRONT_LEFT_DOOR_X, FRONT_LEFT_DOOR_Y, FACING_NORTHEAST);
	}

	if (player_said_2(open, right_door) || player_said_2(walk_through, right_door) ||
	    player_said_2(unlock, right_door) || player_said_2(lock, right_door)) {

		if ((global[current_year] == 1881) || (global[done_brie_conv_203] >= YES)) {
			player_walk(FRONT_RIGHT_DOOR_X, FRONT_RIGHT_DOOR_Y, FACING_NORTHEAST);

		} else if ((global[julies_door] == CRACKED_OPEN) || player_said_1(open)) {
			player_walk(FRONT_RIGHT_DOOR_X + 2, FRONT_RIGHT_DOOR_Y - 3, FACING_NORTHEAST);
			/* get closer to door because it is already open */

		} else if (global[julies_door] == FULLY_OPEN) {
			player_walk(INSIDE_RIGHT_DOOR_X, INSIDE_RIGHT_DOOR_Y, FACING_NORTHEAST);
			/* door is fully open, walk into room */
		}
	}

	if (player_said_2(look, paper)) {
		player.need_to_walk = true;
	}
}

void room_110_parser() {
	int temp;  /* For synching purposes */

	if ((player_said_2(walk_through, right_door) || player_said_2(unlock, right_door) ||
	     player_said_2(lock, right_door)) && (global[done_brie_conv_203] == NO) &&
	    (global[current_year] == 1993) && (global[julies_door] == FULLY_OPEN)) {
		new_room = 112;
		goto handled;
	}

	if (player_said_2(walk_through, left_door) || player_said_2(open, left_door) ||
	    player_said_2(unlock, left_door) || player_said_2(lock, left_door)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			seq[fx_take_9] = kernel_seq_forward(ss[fx_take_9], false, 5, 0, 0, 1);
			kernel_seq_range(seq[fx_take_9], 1, 4);
			kernel_seq_player(seq[fx_take_9], true);
			kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			temp = seq[fx_take_9];
			seq[fx_take_9] = kernel_seq_stamp(ss[fx_take_9], false, 4);
			kernel_synch(KERNEL_SERIES, seq[fx_take_9], KERNEL_SERIES, temp);
			kernel_seq_player(seq[fx_take_9], false);
			kernel_timing_trigger(HALF_SECOND, 2);
			sound_play(N_DoorHandle);
			break;

		case 2:
			kernel_seq_delete(seq[fx_take_9]);
			seq[fx_take_9] = kernel_seq_backward(ss[fx_take_9], false, 5, 0, 0, 1);
			kernel_seq_range(seq[fx_take_9], 1, 4);
			kernel_seq_player(seq[fx_take_9], false);
			kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_9]);
			player.walker_visible   = true;
			player.commands_allowed = true;
			if (player_said_1(lock) || player_said_1(unlock)) {
				text_show(text_000_32);
				/* the key does not work here */
			} else {
				text_show(text_110_22);
				/* the door is locked */
			}
			break;
		}
		goto handled;
	}

	if (player_said_2(walk_through, right_door) || player_said_2(open, right_door) ||
	    player_said_2(unlock, right_door) || player_said_2(lock, right_door)) {
		if ((global[current_year] == 1881) || (global[done_brie_conv_203] >= YES) &&
		     !player_said_2(unlock, right_door) && !player_said_2(lock, right_door)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take_9] = kernel_seq_forward(ss[fx_take_9], false, 5, 0, 0, 1);
				kernel_seq_range(seq[fx_take_9], 1, 4);
				kernel_seq_player(seq[fx_take_9], true);
				kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				temp = seq[fx_take_9];
				seq[fx_take_9] = kernel_seq_stamp(ss[fx_take_9], false, 4);
				kernel_synch(KERNEL_SERIES, seq[fx_take_9], KERNEL_SERIES, temp);
				kernel_seq_player(seq[fx_take_9], false);
				kernel_timing_trigger(HALF_SECOND, 2);
				sound_play(N_DoorHandle);
				break;

			case 2:
				kernel_seq_delete(seq[fx_take_9]);
				seq[fx_take_9] = kernel_seq_backward(ss[fx_take_9], false, 5, 0, 0, 1);
				kernel_seq_range(seq[fx_take_9], 1, 4);
				kernel_seq_player(seq[fx_take_9], false);
				kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_9]);
				player.walker_visible   = true;
				player.commands_allowed = true;
				if (player_said_1(lock) || player_said_1(unlock)) {
					text_show(text_000_32);
					/* the key does not work here */
				} else {
					text_show(text_110_23);
					/* The door is locked */
				}
				break;
			}
			goto handled;

		} else if (global[julies_door] == CRACKED_OPEN) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take_9] = kernel_seq_pingpong(ss[fx_take_9], false, 7, 0, 0, 2);
				kernel_seq_range(seq[fx_take_9], 1, 4);
				kernel_seq_player(seq[fx_take_9], true);
				kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_SPRITE, 4, 1);
				kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, 3);
				break;

			case 1:
				sound_play(N_DoorOpens);
				seq[fx_door] = kernel_seq_forward(ss[fx_door], false, 10, 0, 0, 1);
				kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_depth(seq[fx_door], 10);
				kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2:
				seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 3);
				kernel_seq_depth(seq[fx_door], 10);
				break;

			case 3:
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_9]);
				player.walker_visible = true;
				/* The door has been pushed open all of the way */
				player_walk(PLAYER_X_FROM_112, PLAYER_Y_FROM_112, FACING_NORTHEAST);
				player_walk_trigger(4);
				break;

			case 4:
				new_room = 112;
				global[julies_door] = FULLY_OPEN;
				break;
			}
			goto handled;
		}
	}

	if (player.look_around) {
		text_show(text_110_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(floor)) {
			text_show(text_110_11);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_110_12);
			goto handled;
		}

		if (player_said_1(ceiling)) {
			text_show(text_110_13);
			goto handled;
		}

		if (player_said_1(stairwell)) {
			text_show(text_110_14);
			goto handled;
		}

		if (player_said_1(right_door)) {
			if (global[done_brie_conv_203] >= YES) {
				text_show(text_110_16);
			} else if (global[chris_f_status] == CHRIS_F_IS_ALIVE) {
				text_show(text_110_15);
			} else {
				text_show(text_110_16);
			}
			goto handled;
		}

		if (player_said_1(left_door)) {
			text_show(text_110_16);
			goto handled;
		}

		if ((player_said_1(waste_basket)) || (player_said_1(trash_bucket))) {
			text_show(text_110_17);
			goto handled;
		}

		if (player_said_1(poster)) {
			text_show(text_110_18);
			goto handled;
		}

		if (player_said_1(bulletin_board)) {
			text_show(text_110_19);
			goto handled;
		}

		if (player_said_1(paper)) {
			text_show(text_110_29);
			goto handled;
		}

		if (player_said_2(take, waste_basket) || player_said_2(take, trash_bucket)) {
			text_show(text_110_20);
			goto handled;
		}

		if (player_said_1(light_fixture)) {
			if (global[current_year] == 1993) {
				text_show(text_110_21);
			} else {
				text_show(text_110_28);
			}
			goto handled;
		}
	}

	if (player_said_2(take, paper)) {
		text_show(text_110_30);
		goto handled;
	}

	if (player_said_2(close, right_door)) {
		if (global[done_brie_conv_203] >= YES) {
			text_show(text_110_26);
		} else if (global[chris_f_status] == CHRIS_F_IS_ALIVE) {
			text_show(text_110_24);
		} else {
			text_show(text_110_26);
		}
		goto handled;
	}

	if (player_said_2(close, left_door)) {
		text_show(text_110_25);
		goto handled;
	}

	if (player_said_2(open, right_door)) {
		if ((global[chris_f_status] == CHRIS_F_IS_ALIVE) &&
		    (global[done_brie_conv_203] == NO)) {
			text_show(text_110_27);
		}
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_110_preload() {
	room_init_code_pointer       = room_110_init;
	room_pre_parser_code_pointer = room_110_pre_parser;
	room_parser_code_pointer     = room_110_parser;
	room_daemon_code_pointer     = NULL;

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
