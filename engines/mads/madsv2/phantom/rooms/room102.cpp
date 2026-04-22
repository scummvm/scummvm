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
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/rooms/room102.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_102_init(void) {
	int death_x = 0;
	int death_y = 0;
	int death_scale = 0;
	int death_depth = 0;

	local->anim_0_running = false;

	/* =================== Load Sprite Series ==================== */

	ss[fx_door]  = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_death] = kernel_load_series("*RAL86", false);


	/* =========== If in 1993, put chandelier here =============== */

	if (global[current_year] == 1993) {
		ss[fx_chandelier] = kernel_load_series(kernel_name('z', -1), false);
		kernel_draw_to_background(ss[fx_chandelier], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_seq_depth(seq[fx_chandelier], 14);
	} else {
		kernel_flip_hotspot(words_chandelier, false);
	}


	/* ======================= Previous Rooms ==================== */

	if (previous_room == 101) {
		player.x      = PLAYER_X_FROM_101;
		player.y      = PLAYER_Y_FROM_101;
		player.facing = FACING_SOUTHEAST;
		seq[fx_door]  = kernel_seq_stamp(ss[fx_door], false, 4);
		player_walk(PLAYER_X_FROM_101 - 14, PLAYER_Y_FROM_101 + 8, FACING_SOUTHEAST);
		kernel_seq_depth(seq[fx_door], 14);

	} else if (previous_room == 104) {
		player.commands_allowed = false;
		player.walker_visible   = false;
		switch (global[death_location]) {
		case FAR_PIT:
			death_x     = 221;
			death_y     = 57;
			death_scale = 50;
			death_depth = 14;
			break;

		case MIDDLE_PIT:
			death_x     = 219;
			death_y     = 85;
			death_scale = 60;
			death_depth = 6;
			break;

		case NEAR_PIT:
			death_x     = 257;
			death_y     = 138;
			death_scale = 76;
			death_depth = 1;
			break;

		default:
			break;
		}

		kernel_init_dialog();
		kernel_set_interface_mode(INTER_CONVERSATION);
		seq[fx_death] = kernel_seq_stamp(ss[fx_death], false, 1);
		kernel_seq_depth(seq[fx_death], death_depth);
		kernel_seq_loc(seq[fx_death], death_x, death_y);
		kernel_seq_scale(seq[fx_death], death_scale);
		kernel_timing_trigger(TWO_SECONDS, ROOM_102_DEATH);
		seq[fx_door]  = kernel_seq_stamp(ss[fx_door], false, 4);
		kernel_seq_depth(seq[fx_door], 14);

	} else if ((previous_room == 103) || (previous_room != KERNEL_RESTORING_GAME)) {
		player.x              = PLAYER_X_FROM_103;
		player.y              = PLAYER_Y_FROM_103;
		player.facing         = FACING_WEST;
		local->anim_0_running = true;
		local->animation[0]   = kernel_run_animation(kernel_name('d', 1), ROOM_102_DOOR_CLOSES);

	} else if (previous_room == KERNEL_RESTORING_GAME) {
		seq[fx_door]  = kernel_seq_stamp(ss[fx_door], false, 4);
		kernel_seq_depth(seq[fx_door], 14);
	}

	section_1_music();
}


void room_102_daemon(void) {
	if (kernel.trigger == ROOM_102_DOOR_CLOSES) {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 4);
		kernel_seq_depth(seq[fx_door], 14);
		local->anim_0_running = false;
	}

	if (kernel.trigger == ROOM_102_DEATH) {
		if (global[death_location] == FAR_PIT) {
			text_show(text_102_32);
		} else {
			text_show(text_102_29);
		}
		sound_play(N_BackgroundMus);
		new_room = 104;
	}
}


void room_102_pre_parser(void) {
	if ((player_said_2(open, orchestra_door)) ||
	    (player_said_2(push, orchestra_door))) {
		player_walk(FRONT_OF_DOOR_X, FRONT_OF_DOOR_Y, FACING_EAST);
	}
}


void room_102_parser(void) {
	if (player_said_2(walk_down, aisle)) {
		new_room = 101;
		goto handled;
	}

	if ((player_said_2(walk_through, orchestra_door)) ||
	    (player_said_2(push, orchestra_door)) ||
	    (player_said_2(open, orchestra_door))) {
		if (local->anim_0_running) {
			kernel_timing_trigger(QUARTER_SECOND, ROOM_102_TRY_AGAIN);
			player.commands_allowed = false;

		} else {
			switch (kernel.trigger) {
			case ROOM_102_TRY_AGAIN:
			case 0:
				kernel_seq_delete(seq[fx_door]);
				local->animation[0] = kernel_run_animation(kernel_name('d', 0), 1);
				player.commands_allowed = false;
				player.walker_visible   = false;
				break;

			case 1:
				new_room = 103;
				break;
			}
		}
		goto handled;
	}

	if (player.look_around) {
		text_show(text_102_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(pit)) {
			text_show(text_102_11);
			goto handled;
		}

		if (player_said_1(seats)) {
			if (global[current_year] == 1881) {
				text_show(text_102_12);
			} else {
				text_show(text_102_30);
			}
			goto handled;
		}

		if (player_said_1(orchestra_door)) {
			text_show(text_102_13);
			goto handled;
		}

		if (player_said_1(conductor_s_stand)) {
			text_show(text_102_14);
			goto handled;
		}

		if (player_said_1(music_stand) || player_said_1(music_stands)) {
			text_show(text_102_15);
			goto handled;
		}

		if (player_said_1(prompter_s_box)) {
			text_show(text_102_17);
			goto handled;
		}

		if (player_said_1(stage)) {
			text_show(text_102_18);
			goto handled;
		}

		if (player_said_1(apron)) {
			text_show(text_102_19);
			goto handled;
		}

		if (player_said_1(side_wall)) {
			text_show(text_102_20);
			goto handled;
		}

		if (player_said_1(folding_chairs)) {
			text_show(text_102_21);
			goto handled;
		}

		if (player_said_1(aisle)) {
			text_show(text_102_22);
			goto handled;
		}

		if (player_said_1(proscenium_arch)) {
			text_show(text_102_23);
			goto handled;
		}

		if (player_said_1(act_curtain)) {
			text_show(text_102_24);
			goto handled;
		}

		if (player_said_1(_in_one_)) {
			text_show(text_102_25);
			goto handled;
		}

		if (player_said_1(_in_two_)) {
			text_show(text_102_26);
			goto handled;
		}

		if (player_said_1(leg)) {
			text_show(text_102_27);
			goto handled;
		}

		if (player_said_1(chandelier)) {
			text_show(text_102_31);
			goto handled;
		}
	}

	if (player_said_2(close, orchestra_door)) {
		text_show(text_102_28);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_102_preload(void) {
	room_init_code_pointer = room_102_init;
	room_pre_parser_code_pointer = room_102_pre_parser;
	room_parser_code_pointer = room_102_parser;
	room_daemon_code_pointer = room_102_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
