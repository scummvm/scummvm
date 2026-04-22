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
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/phantom/mads/quotes.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/rooms/section3.h"
#include "mads/madsv2/phantom/rooms/room301.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_301_init() {
	kernel.disable_fastwalk = true;

	local->anim_0_running = false;
	local->guard = false;
	local->prevent = false;

	global_speech_load(speech_raoul_catwalk);

	kernel_flip_hotspot(words_cable, false);
	kernel_flip_hotspot(words_stool, false);

	/* ==================== Load Sprite Series =================== */

	ss[fx_down_stairs] = kernel_load_series(kernel_name('a', 0), false);


	/* =============== If in 1993, stamp down z's ================ */

	if (global[current_year] == 1993) {

		ss[fx_push] = kernel_load_series(kernel_name('a', 1), false);

		ss[fx_1993_0] = kernel_load_series(kernel_name('z', 0), false);
		ss[fx_1993_1] = kernel_load_series(kernel_name('z', 1), false);
		ss[fx_1993_2] = kernel_load_series(kernel_name('z', 2), false);
		ss[fx_1993_3] = kernel_load_series(kernel_name('z', 3), false);
		ss[fx_1993_4] = kernel_load_series(kernel_name('z', 4), false);
		ss[fx_1993_5] = kernel_load_series(kernel_name('z', 5), false);
		ss[fx_1993_6] = kernel_load_series(kernel_name('z', 6), false);

		/* two dynamic lights */
		local->dynamic_light = kernel_add_dynamic(words_lighting_instrument, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
			DYN_LIGHT_1_X, DYN_LIGHT_1_Y, DYN_LIGHT_1_XS, DYN_LIGHT_1_YS);
		kernel_dynamic_walk(local->dynamic_light, DYN_LIGHT_1_WALK_TO_X, DYN_LIGHT_1_WALK_TO_Y, FACING_NORTHWEST);
		local->dynamic_light = kernel_add_dynamic(words_lighting_instrument, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
			DYN_LIGHT_2_X, DYN_LIGHT_2_Y, DYN_LIGHT_2_XS, DYN_LIGHT_2_YS);
		kernel_dynamic_walk(local->dynamic_light, DYN_LIGHT_2_WALK_TO_X, DYN_LIGHT_2_WALK_TO_Y, FACING_NORTHWEST);

		seq[fx_1993_0] = kernel_seq_stamp(ss[fx_1993_0], false, 1);
		kernel_seq_depth(seq[fx_1993_0], 14);

		seq[fx_1993_1] = kernel_seq_stamp(ss[fx_1993_1], false, 1);
		kernel_seq_depth(seq[fx_1993_1], 14);

		seq[fx_1993_2] = kernel_seq_stamp(ss[fx_1993_2], false, 1);
		kernel_seq_depth(seq[fx_1993_2], 14);

		seq[fx_1993_3] = kernel_seq_stamp(ss[fx_1993_3], false, 1);
		kernel_seq_depth(seq[fx_1993_3], 14);

		seq[fx_1993_4] = kernel_seq_stamp(ss[fx_1993_4], false, 1);
		kernel_seq_depth(seq[fx_1993_4], 14);

		seq[fx_1993_5] = kernel_seq_stamp(ss[fx_1993_5], false, 1);
		kernel_seq_depth(seq[fx_1993_5], 14);

		seq[fx_1993_6] = kernel_seq_stamp(ss[fx_1993_6], false, 1);
		kernel_seq_depth(seq[fx_1993_6], 14);

		kernel_flip_hotspot(words_cable, true);

	} else {
		/* two dynamic sandbags */
		local->dynamic_sand = kernel_add_dynamic(words_sandbag, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
			DYN_SAND_1_X, DYN_SAND_1_Y, DYN_SAND_1_XS, DYN_SAND_1_YS);
		kernel_dynamic_walk(local->dynamic_sand, DYN_SAND_1_WALK_TO_X, DYN_SAND_1_WALK_TO_Y, FACING_NORTHWEST);
		kernel_add_dynamic(words_sandbag, words_look_at, SYNTAX_SINGULAR, KERNEL_NONE,
			DYN_SAND_2_X, DYN_SAND_2_Y, DYN_SAND_2_XS, DYN_SAND_2_YS);
		kernel_flip_hotspot(words_stool, true);
		kernel_flip_hotspot(words_big_prop, false);
	}


	/* ========================= Previous Rooms ================== */

	if (previous_room == 302) {
		player_first_walk(OFF_SCREEN_X_FROM_302, OFF_SCREEN_Y_FROM_302, FACING_WEST,
			PLAYER_X_FROM_302, PLAYER_Y_FROM_302, FACING_EAST, true);
		camera_jump_to(LEFT_STAGE, 0);

	} else if ((previous_room == 106) || (previous_room != KERNEL_RESTORING_GAME)) {
		player.x = PLAYER_X_FROM_106;
		player.y = PLAYER_Y_FROM_106;
		player.facing = FACING_WEST;
		camera_jump_to(RIGHT_STAGE, 0);
	}

	if (!player.been_here_before) {
		ss[fx_shadow] = kernel_load_series(kernel_name('x', 0), false);
		seq[fx_shadow] = kernel_seq_forward(ss[fx_shadow], false, 7, 0, 0, 1);
		kernel_seq_depth(seq[fx_shadow], 5);
		kernel_seq_range(seq[fx_shadow], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_shadow], KERNEL_TRIGGER_EXPIRE, 0, ROOM_301_SHOW_TEXT);
	}
	section_3_music();
}

void room_301_pre_parser(void) {
	if (player_said_2(exit_to, catwalk_over_house)) {
		player.walk_off_edge_to_room = 302;
	}
}

void room_301_parser(void) {
	if (player_said_2(climb_down, circular_staircase)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			aa[0] = kernel_run_animation(kernel_name('d', 1), 1);
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
			break;

		case 1:
			new_room = 106;
			break;
		}
		goto handled;
	}

	if (player.look_around) {
		text_show(text_301_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(grid)) {
			text_show(text_301_11);
			goto handled;
		}

		if (player_said_1(catwalk)) {
			text_show(text_301_12);
			goto handled;
		}

		if (player_said_1(side_wall)) {
			text_show(text_301_13);
			goto handled;
		}

		if (player_said_1(back_wall)) {
			text_show(text_301_14);
			goto handled;
		}

		if (player_said_1(support)) {
			text_show(text_301_15);
			goto handled;
		}

		if (player_said_1(act_curtain)) {
			text_show(text_301_16);
			goto handled;
		}

		if (player_said_1(house)) {
			text_show(text_301_17);
			goto handled;
		}

		if (player_said_1(other_catwalk)) {
			text_show(text_301_18);
			goto handled;
		}

		if (player_said_1(gridwork)) {
			text_show(text_301_19);
			goto handled;
		}

		if (player_said_1(beam_position)) {
			text_show(text_301_20);
			goto handled;
		}

		if (player_said_1(lighting_instrument)) {
			if (global[current_year] == 1993) {
				text_show(text_301_21);
			} else {
				text_show(text_301_22);
			}
			goto handled;
		}

		if (player_said_1(tarp)) {
			if (global[current_year] == 1993) {
				text_show(text_301_23);
			} else {
				text_show(text_301_40);
			}
			goto handled;
		}

		if (player_said_1(counterweight_system)) {
			text_show(text_301_24);
			goto handled;
		}

		if (player_said_1(sandbag)) {
			if (player.main_object_source == STROKE_INTERFACE) {
				text_show(text_301_25);
				goto handled;
			}
		}

		if (player_said_1(batten)) {
			text_show(text_301_26);
			goto handled;
		}

		if (player_said_1(stool)) {
			text_show(text_301_27);
			goto handled;
		}

		if (player_said_1(hemp)) {
			text_show(text_301_28);
			goto handled;
		}

		if (player_said_1(circular_staircase)) {
			text_show(text_301_29);
			goto handled;
		}

		if (player_said_1(catwalk_over_house)) {
			text_show(text_301_30);
			goto handled;
		}

		if (player_said_1(staircase_post)) {
			text_show(text_301_31);
			goto handled;
		}

		if (player_said_1(railing)) {
			text_show(text_301_32);
			goto handled;
		}

		if (player_said_1(cyclorama)) {
			text_show(text_301_33);
			goto handled;
		}

		if (player_said_1(big_prop)) {
			text_show(text_301_34);
			goto handled;
		}

		if (player_said_1(proscenium_arch)) {
			text_show(text_301_35);
			goto handled;
		}

		if (player_said_1(cable)) {
			text_show(text_301_36);
			goto handled;
		}
	}

	if (player_said_2(take, hemp)) {
		text_show(text_301_38);
		goto handled;
	}

	if (player_said_2(pull, hemp)) {
		text_show(text_301_41);
		goto handled;
	}

	if (player_said_2(take, sandbag)) {
		if (player.main_object_source == STROKE_INTERFACE) {
			text_show(text_301_39);
			goto handled;
		}
	}

	if (player_said_2(take, tarp)) {
		text_show(text_301_42);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_301_daemon(void) {
	if (global[done_brie_conv_203] == YES_AND_CHASE) {
		if ((picture_view_x < 320) && (player.x < 350)) {
			player_cancel_command();
			player_walk(PUSH_X, PUSH_Y, FACING_NORTHWEST);
			player_walk_trigger(ROOM_301_WALK_TO_PUSH_LOC);
			global[done_brie_conv_203] = NO;
			player.commands_allowed = false;
		}
	}

	if (kernel.trigger == ROOM_301_WALK_TO_PUSH_LOC) {
		aa[0] = kernel_run_animation(kernel_name('p', 1), 0);
		local->anim_0_running = true;
		player.walker_visible = false;
		global[player_score] += 10;
		kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
	}

	if (local->anim_0_running && !local->prevent) {
		if (kernel_anim[aa[0]].frame == 50) {
			global_speech_go(speech_raoul_catwalk);
			local->prevent = true;
		}
	}

	if (local->anim_0_running) {
		if (kernel_anim[aa[0]].frame == 61) {
			kernel_reset_animation(aa[0], 60);
			if (!local->guard) {
				sound_play(N_AllFade);
				kernel_timing_trigger(SEVEN_SECONDS, ROOM_301_DONE_PUSHING);
				local->guard = true;
			}
		}
	}

	if (kernel.trigger == ROOM_301_DONE_PUSHING) {
		inter_turn_off_object();
		inter_screen_update();
		new_room = 104;
	}

	if (kernel.trigger == ROOM_301_SHOW_TEXT) {
		kernel_timing_trigger(ONE_SECOND, ROOM_301_SHOW_TEXT + 1);
	}

	if (kernel.trigger == ROOM_301_SHOW_TEXT + 1) {
		text_show(text_301_37);
	}
}

void room_301_preload(void) {
	room_init_code_pointer = room_301_init;
	room_pre_parser_code_pointer = room_301_pre_parser;
	room_parser_code_pointer = room_301_parser;
	room_daemon_code_pointer = room_301_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
