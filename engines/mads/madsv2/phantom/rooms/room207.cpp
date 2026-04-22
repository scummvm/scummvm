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
#include "mads/madsv2/core/rail.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/quotes.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/rooms/section2.h"
#include "mads/madsv2/phantom/rooms/room207.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_207_daemon() {
	if (local->anim_0_running && !local->prevent) {
		if (kernel_anim[aa[0]].frame == 6) {
			if (global_prefer_roland) {
				sound_play(N_Applause002);
			} else {
				global_speech_go(speech_applause);
			}
			local->prevent = true;
		}
	}
}

void room_207_pre_parser() {
	if (player_said_2(take, seat)) {
		player_walk(SEAT_X, SEAT_Y, FACING_NORTH);
	}
}

void room_207_parser() {
	if (player_said_2(take, seat)) {
		switch (kernel.trigger) {
		case 0:
			global[player_score]    += 5;
			player.commands_allowed  = false;
			player.walker_visible    = false;
			sound_play(N_MusicFade);
			seq[fx_sit] = kernel_seq_forward(ss[fx_sit], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_sit], 10);
			kernel_seq_range(seq[fx_sit], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_sit], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			seq[fx_sit] = kernel_seq_stamp(ss[fx_sit], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_sit], 10);
			kernel_timing_trigger(TWO_SECONDS, 2);
			kernel_timing_trigger(FOUR_SECONDS, 3);
			break;

		case 2:
			seq[fx_lights] = kernel_seq_forward(ss[fx_lights], false, 6, 0, 0, 1);
			kernel_seq_depth(seq[fx_lights], 1);
			kernel_seq_range(seq[fx_lights], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_lights], KERNEL_TRIGGER_EXPIRE, 0, 4);
			break;

		case 3:
			aa[0] = kernel_run_animation(kernel_name('s', 1), 5);
			local->anim_0_running = true;
			kernel_seq_delete(seq[fx_curtain]);
			/* run anim of Christine singing */
			break;

		case 4:
			seq[fx_lights] = kernel_seq_stamp(ss[fx_lights], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_lights], 1);
			break;

		case 5:
			new_room = 208;
			break;
		}
		goto handled;
	}


	if (player_said_2(exit_to, loge_corridor)) {
		new_room = 205;
		goto handled;
	}

	if (player.look_around) {
		text_show(text_207_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(wall)) {
			text_show(text_207_11);
			goto handled;
		}

		if (player_said_1(floor)) {
			text_show(text_207_12);
			goto handled;
		}

		if (player_said_1(left_column)) {
			text_show(text_207_13);
			goto handled;
		}

		if (player_said_1(right_column)) {
			text_show(text_207_14);
			goto handled;
		}

		if (player_said_1(rail)) {
			text_show(text_207_15);
			goto handled;
		}

		if (player_said_1(seat)) {
			text_show(text_207_16);
			goto handled;
		}

		if (player_said_1(loge_corridor)) {
			text_show(text_207_17);
			goto handled;
		}

		if (player_said_1(stage)) {
			text_show(text_207_18);
			goto handled;
		}

		if (player_said_1(house)) {
			text_show(text_207_19);
			goto handled;
		}

		if (player_said_1(ceiling)) {
			text_show(text_207_20);
			goto handled;
		}

		if (player_said_1(house_light)) {
			text_show(text_207_21);
			goto handled;
		}
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_207_init() {
	global_speech_load(speech_applause);

	local->prevent        = false;
	local->anim_0_running = false;

	/* Load sprite series */

	ss[fx_sit]     = kernel_load_series(kernel_name('a', 0), false);
	ss[fx_curtain] = kernel_load_series(kernel_name('c', 0), false);
	ss[fx_lights]  = kernel_load_series(kernel_name('x', 0), false);

	seq[fx_curtain] = kernel_seq_stamp(ss[fx_curtain], false, 1);
	kernel_seq_depth(seq[fx_curtain], 10);


	/* Previous Rooms */

	if ((previous_room == 205) || (previous_room != KERNEL_RESTORING_GAME)) {
		player.x      = PLAYER_X_FROM_205;
		player.y      = PLAYER_Y_FROM_205;
		player.facing = FACING_NORTH;
	}

	section_2_music();
}

void room_207_preload() {
	room_init_code_pointer = room_207_init;
	room_pre_parser_code_pointer = room_207_pre_parser;
	room_parser_code_pointer = room_207_parser;
	room_daemon_code_pointer = room_207_daemon;

	section_2_walker();
	section_2_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
