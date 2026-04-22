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
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/rooms/room107.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_107_init() {
	/* ==================== Load Sprite Series ====================== */

	if (global[current_year] == 1993) {
		ss[fx_1993] = kernel_load_series(kernel_name('z', -1), false);
	}

	if (object_is_here(yellow_frame)) {
		ss[fx_yellow_frame] = kernel_load_series(kernel_name('f', 0), false);
		ss[fx_bend_down_9]  = kernel_load_series("*RRD_9", false);
	}


	/* ==================== If in 1881, put yellow_frame =========== */

	if (object_is_here(yellow_frame)) {
		seq[fx_yellow_frame] = kernel_seq_stamp(ss[fx_yellow_frame], false, 1);
		kernel_seq_depth(seq[fx_yellow_frame], 14);
	} else {
		kernel_flip_hotspot(words_yellow_frame, false);
	}


	/* ==================== If in 1993, put 1993 here ============== */

	if (global[current_year] == 1993) {
		kernel_draw_to_background(ss[fx_1993], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_flip_hotspot(words_prop_table, false);
	} else {
		kernel_flip_hotspot(words_headset, false);
	}


	/* ==================== Previous Rooms ========================== */

	if (previous_room == 106) {
		player.x      = PLAYER_X_FROM_106;
		player.y      = PLAYER_Y_FROM_106;
		player.facing = FACING_SOUTHWEST;
		player_walk(WALK_TO_X_FROM_106, WALK_TO_Y_FROM_106, FACING_SOUTHWEST);

	} else if ((previous_room == 104) || (previous_room != KERNEL_RESTORING_GAME)) {

		if (player.y > 128) {
			player.x      = PLAYER_X_FROM_104_3;
			player.facing = FACING_NORTHWEST;

		} else if (player.y > 99) {
			player.x      = PLAYER_X_FROM_104_2;
			player.facing = FACING_NORTHWEST;

		} else {
			player.x      = PLAYER_X_FROM_104_1;
			player.facing = FACING_NORTHEAST;
		}

		player.y = PLAYER_Y_FROM_104;
	}

	section_1_music();
}

void room_107_parser() {
	int count = 0;

	if (player_said_2(walk_onto, stage)) {
		new_room = 104;
		goto handled;
	}

	if (player_said_2(walk, backstage)) {
		new_room = 106;
		goto handled;
	}

	if (player_said_2(take, yellow_frame) &&
	    (object_is_here(yellow_frame) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			if (global[current_year] == 1881) {
				if (player_has(green_frame))  ++count;
				if (player_has(red_frame))    ++count;
				if (player_has(blue_frame))   ++count;
				if (count < 3) global[player_score] += 5;
			}
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
			kernel_seq_delete(seq[fx_yellow_frame]);
			kernel_flip_hotspot(words_yellow_frame, false);
			inter_give_to_player(yellow_frame);
			sound_play(N_TakeObjectSnd);
			break;

		case 2:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_bend_down_9]);
			player.walker_visible = true;
			kernel_timing_trigger(20, 3);
			break;

		case 3:
			if (global[current_year] == 1881) {
				object_examine(yellow_frame, text_008_43, 0);
			} else {
				object_examine(yellow_frame, text_008_04, 0);
			}
			/* You pick up the yellow color frame */
			player.commands_allowed = true;
			break;
		}
		goto handled;
	}

	if (player.look_around) {
		text_show(text_107_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(stage)) {
			text_show(text_107_11);
			goto handled;
		}

		if (player_said_1(_in_two_)) {
			text_show(text_107_12);
			goto handled;
		}

		if (player_said_1(_in_one_)) {
			text_show(text_107_13);
			goto handled;
		}

		if (player_said_1(cyclorama)) {
			text_show(text_107_14);
			goto handled;
		}

		if (player_said_1(counterweight_system)) {
			text_show(text_107_15);
			goto handled;
		}

		if (player_said_1(purchase_lines)) {
			text_show(text_107_16);
			goto handled;
		}

		if (player_said_1(lockrail)) {
			text_show(text_107_17);
			goto handled;
		}

		if (player_said_1(stage)) {  /* Note: second 'stage' check — unreachable in original */
			text_show(text_107_18);
			goto handled;
		}

		if (player_said_1(prop_table)) {
			text_show(text_107_19);
			goto handled;
		}

		if (player_said_1(act_curtain)) {
			text_show(text_107_20);
			goto handled;
		}

		if (player_said_1(leg)) {
			text_show(text_107_21);
			goto handled;
		}

		if (player_said_1(apron)) {
			text_show(text_107_22);
			goto handled;
		}

		if (player_said_1(proscenium_arch)) {
			text_show(text_107_23);
			goto handled;
		}

		if (player_said_1(stage)) {  /* Note: third 'stage' check — unreachable in original */
			text_show(text_107_24);
			goto handled;
		}

		if (player_said_1(backstage)) {
			text_show(text_107_25);
			goto handled;
		}

		if (player_said_1(yellow_frame) && object_is_here(yellow_frame)) {
			if (global[current_year] == 1881) {
				text_show(text_107_27);
			} else {
				text_show(text_107_26);
			}
			goto handled;
		}

		if (player_said_1(headset)) {
			text_show(text_107_28);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_107_30);
			goto handled;
		}
	}

	if (player_said_2(take, headset)) {
		text_show(text_107_29);
		goto handled;
	}

	if (player_said_2(talk_to, headset)) {
		text_show(text_107_32);
		goto handled;
	}

	if (player_said_2(pull, purchase_lines)) {
		text_show(text_107_31);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_107_preload() {
	room_init_code_pointer       = room_107_init;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer     = room_107_parser;
	room_daemon_code_pointer     = NULL;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
