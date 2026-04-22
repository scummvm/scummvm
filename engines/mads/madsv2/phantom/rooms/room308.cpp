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
#include "mads/madsv2/phantom/rooms/room308.h"
#include "mads/madsv2/core/conv.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_308_init() {
	global_speech_load(speech_christine_scream);

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_2_running = false;
		local->anim_3_running = false;
	}

	local->prevent = false;
	local->prevent_2 = false;

	conv_get(CONV_MISC);

	/* =================== Load sprites series =================== */

	ss[fx_raoul] = kernel_load_series(kernel_name('a', 0), false);
	ss[fx_her] = kernel_load_series(kernel_name('b', 0), false);

	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

	player.walker_visible = false;


	/* ========================= Previous Rooms ================== */


	if (previous_room == KERNEL_RESTORING_GAME) {
		switch (local->on_floor) {
		case 1:
			if (global[right_door_is_open_504]) {
				seq[fx_raoul] = kernel_seq_stamp(ss[fx_raoul], false, 1);
				kernel_seq_depth(seq[fx_raoul], 10);
				kernel_seq_loc(seq[fx_raoul], HIM_BOTH_BOTTOM_X, HIM_BOTH_BOTTOM_Y);
				seq[fx_her] = kernel_seq_stamp(ss[fx_her], false, 1);
				kernel_seq_depth(seq[fx_her], 10);
				kernel_seq_loc(seq[fx_her], HER_BOTH_BOTTOM_X, HER_BOTH_BOTTOM_Y);

			} else {
				seq[fx_raoul] = kernel_seq_stamp
				(ss[fx_raoul], false, 1);
				kernel_seq_depth(seq[fx_raoul], 10);
				kernel_seq_loc(seq[fx_raoul], RAOUL_BOTTOM_X, RAOUL_BOTTOM_Y);
			}
			break;

		case 2:
			seq[fx_raoul] = kernel_seq_stamp
			(ss[fx_raoul], false, 1);
			kernel_seq_depth(seq[fx_raoul], 10);
			kernel_seq_loc(seq[fx_raoul], RAOUL_MIDDLE_X, RAOUL_MIDDLE_Y);
			break;

		case 3:
			if (global[right_door_is_open_504] && !global[knocked_over_head]) {
				local->anim_0_running = true;
				local->prevent = true;
				aa[0] = kernel_run_animation(kernel_name('b', 2), 2);
				kernel_reset_animation(aa[0], 76);

			} else {
				local->anim_0_running = true;
				local->prevent = true;
				aa[0] = kernel_run_animation(kernel_name('u', 2), 2);
				kernel_reset_animation(aa[0], 96);
			}
			break;
		}

	} else if (previous_room == 309) {
		local->on_floor = 1;
		if (global[right_door_is_open_504]) {
			seq[fx_raoul] = kernel_seq_stamp(ss[fx_raoul], false, 1);
			kernel_seq_depth(seq[fx_raoul], 10);
			kernel_seq_loc(seq[fx_raoul], HIM_BOTH_BOTTOM_X, HIM_BOTH_BOTTOM_Y);
			seq[fx_her] = kernel_seq_stamp(ss[fx_her], false, 1);
			kernel_seq_depth(seq[fx_her], 10);
			kernel_seq_loc(seq[fx_her], HER_BOTH_BOTTOM_X, HER_BOTH_BOTTOM_Y);

		} else {
			seq[fx_raoul] = kernel_seq_stamp(ss[fx_raoul], false, 1);
			kernel_seq_depth(seq[fx_raoul], 10);
			kernel_seq_loc(seq[fx_raoul], RAOUL_BOTTOM_X, RAOUL_BOTTOM_Y);
		}

	} else if (previous_room == 206) {
		local->on_floor = 2;
		seq[fx_raoul] = kernel_seq_stamp
		(ss[fx_raoul], false, 1);
		kernel_seq_depth(seq[fx_raoul], 10);
		kernel_seq_loc(seq[fx_raoul], RAOUL_MIDDLE_X, RAOUL_MIDDLE_Y);

	} else if (previous_room == 307) {
		local->on_floor = 3;
		local->anim_0_running = true;
		local->prevent = true;
		aa[0] = kernel_run_animation(kernel_name('u', 2), 2);
		kernel_reset_animation(aa[0], 96);
	}

	if (!player.been_here_before) {
		global[player_score] += 5;
		kernel_timing_trigger(1, ROOM_308_SHOW_TEXT);
	}

	section_3_music();
}

void room_308_parser() {
	switch (kernel.trigger) {
	case 1:
		new_room = 206;
		goto handled;
		break;

	case 2:
		new_room = 307;
		goto handled;
		break;

	case 3:
		new_room = 309;
		goto handled;
		break;
	}

	if (player_said_2(exit_to, middle_level)) {
		switch (local->on_floor) {
		case 1:  /* on bottom floor */
			if (global[right_door_is_open_504]) {
				aa[0] = kernel_run_animation(kernel_name('b', 1), 1);
				kernel_seq_delete(seq[fx_raoul]);
				kernel_seq_delete(seq[fx_her]);
				player.commands_allowed = false;

			} else {
				aa[0] = kernel_run_animation(kernel_name('u', 1), 1);
				kernel_seq_delete(seq[fx_raoul]);
				player.commands_allowed = false;
			}
			goto handled;
			break;

		case 2:  /* on middle floor */
			aa[0] = kernel_run_animation(kernel_name('m', 1), 1);
			kernel_seq_delete(seq[fx_raoul]);
			player.commands_allowed = false;
			goto handled;
			break;

		case 3:  /* on top floor (it was blocked) */
			if (global[right_door_is_open_504] && !global[knocked_over_head]) {
				kernel_abort_animation(aa[0]);
				aa[0] = kernel_run_animation(kernel_name('b', 4), 1);
				player.commands_allowed = false;
				local->anim_2_running = false;

			} else {
				kernel_abort_animation(aa[0]);
				aa[0] = kernel_run_animation(kernel_name('x', 1), 1);
				player.commands_allowed = false;
				local->anim_0_running = false;
				local->anim_1_running = false;
			}
			goto handled;
			break;
		}
	}

	if (player_said_2(exit_to, upper_level)) {
		switch (local->on_floor) {
		case 1:  /* on bottom floor */
			if (global[right_door_is_open_504]) {
				aa[0] = kernel_run_animation(kernel_name('b', 2), 1);
				kernel_seq_delete(seq[fx_raoul]);
				kernel_seq_delete(seq[fx_her]);
				player.commands_allowed = false;
				local->anim_2_running = true;
				local->on_floor = 3;

			} else {
				aa[0] = kernel_run_animation(kernel_name('u', 2), 2);
				kernel_seq_delete(seq[fx_raoul]);
				player.commands_allowed = false;
				local->anim_0_running = true;
				local->on_floor = 3;
			}
			goto handled;
			break;

		case 2:  /* on middle floor */
			if (global[right_door_is_open_504]) {
				aa[0] = kernel_run_animation(kernel_name('u', 3), 2);
				kernel_seq_delete(seq[fx_raoul]);
				player.commands_allowed = false;
				goto handled;

			} else {
				aa[0] = kernel_run_animation(kernel_name('u', 3), 2);
				kernel_seq_delete(seq[fx_raoul]);
				player.commands_allowed = false;
				local->anim_1_running = true;
				local->on_floor = 3;
				goto handled;
			}
			break;

		case 3:  /* on top floor (door is locked */
			if (global[right_door_is_open_504]) {
				global[top_floor_locked] = false;

			} else {
				local->prevent_2 = false;
				local->prevent = false;
			}
			goto handled;
			break;
		}
	}

	if (player_said_2(exit_to, lower_level)) {
		switch (local->on_floor) {
		case 1:  /* on bottom floor */
			if (global[right_door_is_open_504]) {
				conv_run(CONV_MISC);
				conv_export_value(1);

			} else {
				aa[0] = kernel_run_animation(kernel_name('l', 1), 3);
				kernel_seq_delete(seq[fx_raoul]);
				player.commands_allowed = false;
			}
			goto handled;
			break;

		case 2:  /* on middle floor */
			if (global[right_door_is_open_504]) {
				if (global_prefer_roland) {
					sound_play(N_WomanScream003);
				} else {
					global_speech(speech_woman_scream);
				}
				conv_run(CONV_MISC);
				conv_export_value(6);

			} else {
				aa[0] = kernel_run_animation(kernel_name('d', 1), 3);
				kernel_seq_delete(seq[fx_raoul]);
				player.commands_allowed = false;
			}
			goto handled;
			break;

		case 3:  /* on top floor (it was blocked) */

			if (global[right_door_is_open_504] && !global[knocked_over_head]) {
				conv_run(CONV_MISC);
				conv_export_value(5);

			} else if (global[right_door_is_open_504] && global[knocked_over_head]) {
				if (global_prefer_roland) {
					sound_play(N_WomanScream003);
				} else {
					global_speech(speech_woman_scream);
				}
				conv_run(CONV_MISC);
				conv_export_value(6);

			} else {
				kernel_abort_animation(aa[0]);
				aa[0] = kernel_run_animation(kernel_name('x', 2), 3);
				player.commands_allowed = false;
				local->anim_0_running = false;
				local->anim_1_running = false;
			}
			goto handled;
			break;
		}
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_308_daemon() {
	if (kernel.trigger == ROOM_308_SHOW_TEXT) {
		text_show(text_308_10);
	}

	if (local->anim_2_running) {
		if (kernel_anim[aa[0]].frame == 77) {
			kernel_reset_animation(aa[0], 76);
			if (!local->prevent) {
				player.commands_allowed = true;
				text_show(text_308_11);
				local->prevent = true;
			}
		}
	}

	if (local->anim_0_running) {
		if (kernel_anim[aa[0]].frame == 97) {
			if (global[top_floor_locked]) {
				kernel_reset_animation(aa[0], 96);
				if (!local->prevent) {
					player.commands_allowed = true;
					text_show(text_308_11);
					local->prevent = true;
				}
			}
		} else if (kernel_anim[aa[0]].frame == 116) {
			global[top_floor_locked] = true;
			new_room = 307;
		}
	}

	if (local->anim_1_running) {
		if (kernel_anim[aa[0]].frame == 51) {
			if (global[top_floor_locked]) {
				kernel_reset_animation(aa[0], 50);
				if (!local->prevent) {
					player.commands_allowed = true;
					text_show(text_308_11);
					local->prevent = true;
				}
			}
		}
	}
}

void room_308_preload() {
	room_init_code_pointer = room_308_init;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = room_308_parser;
	room_daemon_code_pointer = room_308_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
