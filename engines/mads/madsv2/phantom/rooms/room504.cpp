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
#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/rooms/section5.h"
#include "mads/madsv2/phantom/rooms/room504.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

int conv027_second = 0;

void room_504_init() {
	int id;

	kernel.disable_fastwalk = true;

	local->input_count = 0;
	local->death_count = 0;
	local->anim_2_running = false;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->play_count = 0;
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_3_running = false;
		local->anim_4_running = false;
		local->anim_5_running = false;
		local->music_is_playing = false;
		local->fire_breath = false;
		local->prevent = false;
		local->music_choice = 0;
		local->phan_action = 0;
	}


	kernel_flip_hotspot(words_Christine, false);

	/* ==================== Load conversation ==================== */

	if (!global[right_door_is_open_504]) {
		conv_get(CONV_LISTEN);
		conv_get(CONV_MUSIC);
	} else {
		conv_get(CONV_FIGHT);
	}

	conv_get(CONV_MISC);

	/* ===================== Load Sprite Series ================== */

	ss[fx_take_9] = kernel_load_series("*RDR_9", false);
	ss[fx_music] = kernel_load_series(kernel_name('x', 8), false);

	if (!object_is_here(music_score)) {
		seq[fx_music] = kernel_seq_stamp(ss[fx_music], false, 1);
		kernel_seq_depth(seq[fx_music], 14);
		kernel_flip_hotspot(words_music_score, false);
	}

	ss[fx_trap_door] = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_burn] = kernel_load_series(kernel_name('x', 7), PAL_MAP_ALL_TO_CLOSEST |
		PAL_MAP_ANY_TO_CLOSEST);

	if (global[fight_status] == FIGHT_NOT_HAPPENED) {
		ss[fx_left_door] = kernel_load_series(kernel_name('x', 1), false);
	}

	ss[fx_ramhead] = kernel_load_series(kernel_name('x', 3), false);

	if ((previous_room == 505) || ((previous_room == 504) && global[right_door_is_open_504])) {

		if ((global[fight_status] == FIGHT_NOT_HAPPENED) && (global[coffin_status] == COFFIN_OPEN)) {
			/* start the fight sequence */

			kernel_flip_hotspot(words_Christine, true);

			kernel_draw_to_background(ss[fx_trap_door], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);

			ss[fx_burn] = kernel_load_series(kernel_name('x', 7), PAL_MAP_ALL_TO_CLOSEST |
				PAL_MAP_ANY_TO_CLOSEST);

			/* These are loaded, but not dumped from memory. An anim loads these, */
			/* so I don't care what handle is returned. I just load em to re-map  */

			ss[fx_test] = kernel_load_series(kernel_name('b', 0), false);

			ss[fx_test] = kernel_load_series("*CHR_6", PAL_MAP_ALL_TO_CLOSEST |
				PAL_MAP_ANY_TO_CLOSEST);
			ss[fx_test] = kernel_load_series("*FACERAL", PAL_MAP_ALL_TO_CLOSEST |
				PAL_MAP_ANY_TO_CLOSEST);
			ss[fx_test] = kernel_load_series("*FACEXDFR", PAL_MAP_ALL_TO_CLOSEST |
				PAL_MAP_ANY_TO_CLOSEST);
			ss[fx_test] = kernel_load_series("*FACEPHN", PAL_MAP_ALL_TO_CLOSEST |
				PAL_MAP_ANY_TO_CLOSEST);

			/* These are loaded and will be dumped from memory in Daemon */

			ss[fx_a_5] = kernel_load_series(kernel_name('a', 5), false);
			ss[fx_a_3] = kernel_load_series(kernel_name('a', 3), PAL_MAP_ALL_TO_CLOSEST |
				PAL_MAP_ANY_TO_CLOSEST);
			ss[fx_a_6] = kernel_load_series(kernel_name('a', 6), PAL_MAP_ALL_TO_CLOSEST |
				PAL_MAP_ANY_TO_CLOSEST);
			ss[fx_a_7] = kernel_load_series(kernel_name('a', 7), PAL_MAP_ALL_TO_CLOSEST |
				PAL_MAP_ANY_TO_CLOSEST);
			ss[fx_a_0] = kernel_load_series(kernel_name('a', 0), PAL_MAP_ALL_TO_CLOSEST |
				PAL_MAP_ANY_TO_CLOSEST);

			aa[3] = kernel_run_animation(kernel_name('p', 1), 0);
			id = kernel_add_dynamic(words_Phantom, words_look_at, SYNTAX_MASC_NOT_PROPER, KERNEL_NONE,
				0, 0, 0, 0);
			kernel_dynamic_hot[id].prep = PREP_ON;
			kernel_dynamic_walk(id, WALK_NONE, WALK_NONE, 5);

			kernel_dynamic_anim(id, aa[3], 4);
			kernel_dynamic_anim(id, aa[3], 8);
			kernel_dynamic_anim(id, aa[3], 13);

			local->phan_action = FIGHT_ENTER_ROOM;
			player.commands_allowed = false;
			player.walker_visible = false;
			local->anim_3_running = true;

		} else {

			if (global[fight_status]) {
				kernel_draw_to_background(ss[fx_burn], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
			}

			if (global[coffin_status] != COFFIN_OPEN) {
				seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 1);
				kernel_seq_depth(seq[fx_left_door], 14);
			}

			player.x = PLAYER_X_FROM_505;
			player.y = PLAYER_Y_FROM_505;
			player.facing = FACING_SOUTHWEST;
			player_walk(WALK_TO_X_FROM_505, WALK_TO_Y_FROM_505, FACING_SOUTHWEST);

			kernel_draw_to_background(ss[fx_trap_door], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);

			if (!player_has_been_in_room(506) && (global[coffin_status] == COFFIN_OPEN)) { /* Chris is waiting by door in 504 */

				kernel_load_variant(1);

				kernel_draw_to_background(ss[fx_burn], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);

				local->anim_5_running = true;

				/* I never unload these because at end of anim, new room = 506 */

				ss[fx_test] = kernel_load_series("*CHR_2", false);
				ss[fx_test] = kernel_load_series("*FACERAL", PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);
				ss[fx_test] = kernel_load_series("*FACEXDFR", PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);
				ss[fx_test] = kernel_load_series("*FACEPHN", PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);

				aa[3] = kernel_run_animation(kernel_name('p', 3), 0);
				id = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
					0, 0, 0, 0);
				local->phan_action = FIGHT_CHRIS_TO_DOOR;
				kernel_dynamic_hot[id].prep = PREP_ON;
				kernel_reset_animation(aa[3], 79);
				kernel_dynamic_walk(id, DYNAMIC_CHR_WALK_TO_X, DYNAMIC_CHR_WALK_TO_Y, FACING_NORTHWEST);
				kernel_dynamic_anim(id, aa[3], 0);
				kernel_dynamic_anim(id, aa[3], 1);
				kernel_dynamic_anim(id, aa[3], 2);
				kernel_dynamic_anim(id, aa[3], 3);
				kernel_dynamic_anim(id, aa[3], 4);
			}
		}

	} else if (previous_room == 506) {

		player.x = PLAYER_X_FROM_506;
		player.y = PLAYER_Y_FROM_506;
		player.facing = FACING_SOUTHEAST;
		player_walk(WALK_TO_X_FROM_506, WALK_TO_Y_FROM_506, FACING_SOUTHEAST);
		kernel_draw_to_background(ss[fx_trap_door], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_draw_to_background(ss[fx_burn], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);

	} else if (previous_room == 504) {  /* just returned from burning at the organ */
		seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 1);
		kernel_seq_depth(seq[fx_left_door], 14);

		ss[fx_right_door] = kernel_load_series(kernel_name('x', 0), false);
		seq[fx_right_door] = kernel_seq_stamp(ss[fx_right_door], false, 1);
		kernel_seq_depth(seq[fx_right_door], 14);
		kernel_draw_to_background(ss[fx_trap_door], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);

		player.x = PLAYER_X_FROM_502;
		player.y = PLAYER_Y_FROM_502;
		player.facing = FACING_EAST;

	} else if ((previous_room == 502) || (previous_room != KERNEL_RESTORING_GAME)) {

		seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 1);
		kernel_seq_depth(seq[fx_left_door], 14);

		ss[fx_right_door] = kernel_load_series(kernel_name('x', 0), false);
		seq[fx_right_door] = kernel_seq_stamp(ss[fx_right_door], false, 1);
		kernel_seq_depth(seq[fx_right_door], 14);

		aa[0] = kernel_run_animation(kernel_name('t', 1), ROOM_504_FROM_502);
		player.commands_allowed = false;
		player.walker_visible = false;
		player.x = PLAYER_X_FROM_502;
		player.y = PLAYER_Y_FROM_502;
		player.facing = FACING_EAST;

	}

	if (previous_room == KERNEL_RESTORING_GAME) {
		if (!global[right_door_is_open_504]) {
			ss[fx_right_door] = kernel_load_series(kernel_name('x', 0), false);
			seq[fx_right_door] = kernel_seq_stamp(ss[fx_right_door], false, 1);
			kernel_seq_depth(seq[fx_right_door], 14);
		}

		if (conv_restore_running == CONV_LISTEN) {
			kernel_draw_to_background(ss[fx_trap_door], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
			seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 1);
			kernel_seq_depth(seq[fx_left_door], 14);

			aa[0] = kernel_run_animation(kernel_name('l', 1), ROOM_504_END_LISTEN);
			local->anim_0_running = true;
			player.walker_visible = false;
			player.commands_allowed = false;
			player.x = RIGHT_DOOR_X;
			player.y = RIGHT_DOOR_Y;
			player.facing = FACING_EAST;
			local->listen_action = CONV19_LISTEN;
			kernel_reset_animation(aa[0], 8);
			conv_run(CONV_LISTEN);
			conv_export_value(game.difficulty);

		} else if (conv_restore_running == CONV_MUSIC) {
			kernel_draw_to_background(ss[fx_trap_door], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
			seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 1);
			kernel_seq_depth(seq[fx_left_door], 14);

			ss[fx_fire_left] = kernel_load_series(kernel_name('x', 4), PAL_MAP_ALL_TO_CLOSEST |
				PAL_MAP_ANY_TO_CLOSEST);
			ss[fx_fire_right] = kernel_load_series(kernel_name('x', 5), PAL_MAP_ALL_TO_CLOSEST |
				PAL_MAP_ANY_TO_CLOSEST);
			aa[1] = kernel_run_animation(kernel_name('o', 1), ROOM_504_DONE_PLAYING);
			kernel_reset_animation(aa[1], 22);
			player.commands_allowed = false;
			player.walker_visible = false;
			local->music_is_playing = false;
			local->anim_1_running = true;
			local->play_action = CONV27_SELECT;
			conv_run(CONV_MUSIC);

		} else if ((global[fight_status] <= FIGHT_TALKING) && (global[coffin_status] == COFFIN_OPEN)) {
			/* Christine and Raoul are in fight sequence with Phan */

			if ((local->phan_action == FIGHT_BEFORE_DODGE) || (local->phan_action == FIGHT_CHOICE)) {

				kernel_flip_hotspot(words_Christine, true);

				kernel_draw_to_background(ss[fx_trap_door], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);

				/* These are loaded, but not dumped from memory. An anim loads these, */
				/* so I don't care what handle is returned. I just load em to re-map  */

				ss[fx_test] = kernel_load_series(kernel_name('b', 0), false);

				ss[fx_test] = kernel_load_series("*CHR_6", PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);
				ss[fx_test] = kernel_load_series("*FACERAL", PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);
				ss[fx_test] = kernel_load_series("*FACEXDFR", PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);
				ss[fx_test] = kernel_load_series("*FACEPHN", PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);

				/* These are loaded and will be dumped from memory in Daemon */

				ss[fx_a_5] = kernel_load_series(kernel_name('a', 5), false);
				ss[fx_a_3] = kernel_load_series(kernel_name('a', 3), PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);
				ss[fx_a_6] = kernel_load_series(kernel_name('a', 6), PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);
				ss[fx_a_7] = kernel_load_series(kernel_name('a', 7), PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);
				ss[fx_a_0] = kernel_load_series(kernel_name('a', 0), PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);

				aa[3] = kernel_run_animation(kernel_name('p', 1), 0);
				id = kernel_add_dynamic(words_Phantom, words_look_at, SYNTAX_MASC_NOT_PROPER, KERNEL_NONE,
					0, 0, 0, 0);
				kernel_dynamic_hot[id].prep = PREP_ON;
				kernel_dynamic_walk(id, WALK_NONE, WALK_NONE, 5);

				if (local->phan_action == FIGHT_BEFORE_DODGE) {
					kernel_reset_animation(aa[3], imath_random(109, 112));
				} else if (local->phan_action == FIGHT_CHOICE) {
					kernel_reset_animation(aa[3], imath_random(148, 150));
					kernel_draw_to_background(ss[fx_burn], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
				}

				kernel_dynamic_anim(id, aa[3], 4);
				kernel_dynamic_anim(id, aa[3], 8);
				kernel_dynamic_anim(id, aa[3], 13);

				player.walker_visible = false;
				local->anim_3_running = true;

				if (conv_restore_running == CONV_FIGHT) {
					player.commands_allowed = false;
					conv_run(CONV_FIGHT);
					conv_export_value(player_has(music_score));
				}

			} else if (local->phan_action == FIGHT_SWORD) {  /* Chris is asking 'what happened' */
				kernel_draw_to_background(ss[fx_trap_door], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
				kernel_draw_to_background(ss[fx_burn], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);

				ss[fx_test] = kernel_load_series("*CHR_3", false);
				/* ss[fx_test]       = kernel_load_series(kernel_name('b', 0), false); */
				ss[fx_test] = kernel_load_series("*FACERAL", PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);
				ss[fx_test] = kernel_load_series("*FACEXDFR", PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);
				ss[fx_test] = kernel_load_series("*FACEPHN", PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);

				ss[fx_a_8] = kernel_load_series(kernel_name('a', 8), PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);

				aa[3] = kernel_run_animation(kernel_name('p', 2), ROOM_504_RUN_PART_3);
				kernel_reset_animation(aa[3], 159);

				player.x = WALK_TO_AFTER_FIGHT_X;
				player.y = WALK_TO_AFTER_FIGHT_Y;
				player.facing = FACING_NORTHEAST;
				player.walker_visible = true;
				local->anim_4_running = true;

				player.commands_allowed = false;
				conv_run(CONV_FIGHT);
				conv_export_value(player_has(music_score));
			}

		} else if (global[fight_status] == FIGHT_PARTED) {
			if (!player_has_been_in_room(506)) { /* Chris is waiting by door in 504 */
				kernel_load_variant(1);

				kernel_draw_to_background(ss[fx_trap_door], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
				kernel_draw_to_background(ss[fx_burn], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
				local->anim_5_running = true;

				/* I never unload this because at end of anim, new room = 506 */

				ss[fx_test] = kernel_load_series("*CHR_3", false);
				ss[fx_test] = kernel_load_series("*FACERAL", PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);
				ss[fx_test] = kernel_load_series("*FACEXDFR", PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);
				ss[fx_test] = kernel_load_series("*FACEPHN", PAL_MAP_ALL_TO_CLOSEST |
					PAL_MAP_ANY_TO_CLOSEST);

				aa[3] = kernel_run_animation(kernel_name('p', 3), 0);
				id = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
					0, 0, 0, 0);
				local->phan_action = FIGHT_CHRIS_TO_DOOR;
				kernel_dynamic_hot[id].prep = PREP_ON;
				kernel_reset_animation(aa[3], 79);
				kernel_dynamic_walk(id, DYNAMIC_CHR_WALK_TO_X, DYNAMIC_CHR_WALK_TO_Y, FACING_NORTHWEST);
				kernel_dynamic_anim(id, aa[3], 0);
				kernel_dynamic_anim(id, aa[3], 1);
				kernel_dynamic_anim(id, aa[3], 2);
				kernel_dynamic_anim(id, aa[3], 3);
				kernel_dynamic_anim(id, aa[3], 4);

			} else {
				kernel_draw_to_background(ss[fx_trap_door], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
				kernel_draw_to_background(ss[fx_burn], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
			}

		} else {
			kernel_draw_to_background(ss[fx_trap_door], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
			seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 1);
			kernel_seq_depth(seq[fx_left_door], 14);
			seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 1);
			kernel_seq_depth(seq[fx_left_door], 14);
			if (!global[he_listened]) {
				kernel_timing_trigger(HALF_SECOND, ROOM_504_FROM_502 + 2);
				/* start listen conversation */
			}
		}
	}

	section_5_music();
}

static void process_conversation_listen() {
	if (player_verb == conv019_talk_b_b) {
		if (!kernel.trigger) {
			player_walk(RIGHT_DOOR_X, RIGHT_DOOR_Y, FACING_EAST);
			player_walk_trigger(ROOM_504_LISTEN);
		}
	}

	if (player_verb == conv019_exit_b_b) {
		local->listen_action = CONV19_OTHER;
	}
}

static void process_conversation_play() {
	switch (player_verb) {
	case conv027_choices_one:
		*conv_my_next_start = conv027_second;
		conv_abort();
		local->play_action = CONV27_PLAY;
		local->music_choice = 1;
		break;

	case conv027_choices_two:
		*conv_my_next_start = conv027_second;
		conv_abort();
		local->play_action = CONV27_PLAY;
		local->music_choice = 2;
		break;

	case conv027_choices_three:
		*conv_my_next_start = conv027_second;
		conv_abort();
		local->play_action = CONV27_PLAY;
		local->music_choice = 3;
		break;

	case conv027_choices_four:
		*conv_my_next_start = conv027_second;
		conv_abort();
		local->play_action = CONV27_PLAY;
		local->music_choice = 4;
		break;

	case conv027_choices_five:
		*conv_my_next_start = conv027_second;
		conv_abort();
		local->play_action = CONV27_PLAY;
		local->music_choice = 5;
		break;

	case conv027_exit_a_a:
		*conv_my_next_start = conv027_second;
		conv_abort();
		local->play_action = CONV27_PLAY;
		break;
	}
}

static void process_conversation_fight() {
	switch (player_verb) {
	case conv021_fight_b_b:
		conv_hold();
		local->phan_action = FIGHT_BEFORE_DODGE;
		break;

	case conv021_ending_b_b:
		local->phan_action = FIGHT_CHOICE;
		break;

	case conv021_hasit_b_b:
	case conv021_score_b_b:
		local->phan_action = FIGHT_CHRIS_TO_DOOR;
		global[fight_status] = FIGHT_PARTED;
		break;

	case conv021_outtahere_first:
	case conv021_outtahere_second:
	case conv021_outtahere_third:
	case conv021_score_abc:
		conv_you_trigger(ROOM_504_CHRIS_TALK);
		break;
	}

	if (kernel.trigger == ROOM_504_CHRIS_TALK) {
		if (local->phan_action != FIGHT_CHRIS_TO_DOOR) {
			local->phan_action = FIGHT_CHRIS_TALK;
			local->chris_talk_count = 0;
		}
	}
}

void room_504_pre_parser() {
	if (player_said_2(walk_through, right_door) ||
		player_said_2(open, right_door)) {

		if (global[right_door_is_open_504]) {
			if ((global[fight_status] == FIGHT_PARTED) && !player_has_been_in_room(506)) {
				switch (kernel.trigger) {
				case 0:
					player_walk(BEHIND_RIGHT_DOOR_X, BEHIND_RIGHT_DOOR_Y, FACING_NORTHEAST);
					player.ready_to_walk = false;
					player.need_to_walk = false;
					player.commands_allowed = false;
					conv_run(CONV_MISC);
					conv_export_value(2);
					kernel_timing_trigger(6, 1);
					break;

				case 1:
					if (conv_control.running >= 0) {
						kernel_timing_trigger(6, 1);
					} else {
						player.commands_allowed = true;
						player.command_ready = true;
						player.need_to_walk = true;
						player.ready_to_walk = true;
					}
					break;
				}

			} else {
				player_walk(BEHIND_RIGHT_DOOR_X, BEHIND_RIGHT_DOOR_Y, FACING_NORTHEAST);
			}

		} else if (global[he_listened] || (global[fight_status] == FIGHT_NOT_HAPPENED)) {
			player_walk(RIGHT_DOOR_X, RIGHT_DOOR_Y, FACING_NORTHEAST);
		}
	}

	if (local->anim_2_running && !player_said_1(sit_in)) {
		if (player.need_to_walk) {
			local->chair_action = CHAIR_GET_UP;
			player.commands_allowed = false;
			player.ready_to_walk = false;
		}
	}

	if (player_said_2(walk_through, left_door) ||
		player_said_2(open, left_door)) {

		if ((global[fight_status] == FIGHT_NOT_HAPPENED) && (global[coffin_status] == COFFIN_OPEN) &&
			!player_has_been_in_room(506)) {
			text_show(text_504_32);
			player_cancel_command();
			goto over;

		} else if (player_has_been_in_room(506)) { /* Chris is waiting in 506 */
			player_walk(EXIT_LEFT_DOOR_X, EXIT_LEFT_DOOR_Y, FACING_NORTHWEST);

		} else if (!player_has(music_score) || (global[fight_status] == FIGHT_NOT_HAPPENED)) {
			player_walk(LEFT_DOOR_X, LEFT_DOOR_Y, FACING_NORTHWEST);
		}
	}

	if ((global[fight_status] == FIGHT_NOT_HAPPENED) && (global[coffin_status] == COFFIN_OPEN) &&
		!player_has_been_in_room(506)) {
		if (!player_said_1(look) && !player_said_1(look_at) && !player_said_1(attack) &&
			!player_said_2(take, sword) && !player_said_2(talk_to, Phantom)) {
			text_show(text_504_30);
			player_cancel_command();
		}
	}

over:
	;
}

void room_504_parser() {
	if (conv_control.running == CONV_MISC) {
		goto handled;
	}

	if (player_said_2(sit_in, large_chair)) {
		if (!local->anim_2_running) {
			local->chair_action = CHAIR_SIT;
			player.commands_allowed = false;
			player.walker_visible = false;
			local->anim_2_running = true;
			aa[2] = kernel_run_animation(kernel_name('c', 1), 0);
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
			goto handled;
		} else {
			text_show(text_504_36);
			goto handled;
		}
	}

	if (kernel.trigger == ROOM_504_LISTEN) {
		aa[0] = kernel_run_animation(kernel_name('l', 1), ROOM_504_END_LISTEN);
		local->listen_action = CONV19_LISTEN;
		player.commands_allowed = false;
		player.walker_visible = false;
		local->anim_0_running = true;
		global[he_listened] = true;
		conv_run(CONV_LISTEN);
		conv_export_value(game.difficulty);
		goto handled;
	}

	if (kernel.trigger == ROOM_504_END_LISTEN) {
		player.commands_allowed = true;
		player.walker_visible = true;
		local->anim_0_running = false;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
		goto handled;
	}

	if (conv_control.running == CONV_LISTEN) {
		process_conversation_listen();
		goto handled;
	}

	if (conv_control.running == CONV_MUSIC) {
		process_conversation_play();
		goto handled;
	}

	if (conv_control.running == CONV_FIGHT) {
		process_conversation_fight();
		goto handled;
	}

	if (kernel.trigger == ROOM_504_DONE_PLAYING) {
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
		player.x = AFTER_PLAY_X;
		player.y = AFTER_PLAY_Y;
		player.walker_visible = true;
		local->anim_1_running = false;
		player_demand_facing(FACING_EAST);
		kernel_timing_trigger(10, ROOM_504_DONE_PLAYING + 1);
		goto handled;
	}

	if (kernel.trigger == ROOM_504_DONE_PLAYING + 1) {
		player.commands_allowed = true;
		matte_deallocate_series(ss[fx_fire_right], true);
		matte_deallocate_series(ss[fx_fire_left], true);
		goto handled;
	}

	if (player_said_2(sit_on, organ_bench)) {
		if (global[right_door_is_open_504]) {
			text_show(text_504_27);
		} else {
			ss[fx_fire_left] = kernel_load_series(kernel_name('x', 4), PAL_MAP_ALL_TO_CLOSEST |
				PAL_MAP_ANY_TO_CLOSEST);
			ss[fx_fire_right] = kernel_load_series(kernel_name('x', 5), PAL_MAP_ALL_TO_CLOSEST |
				PAL_MAP_ANY_TO_CLOSEST);
			aa[1] = kernel_run_animation(kernel_name('o', 1), ROOM_504_DONE_PLAYING);
			player.commands_allowed = false;
			player.walker_visible = false;
			local->music_is_playing = false;
			local->anim_1_running = true;
			local->play_action = CONV27_SELECT;
		}
		goto handled;
	}

	if (player_said_2(walk_through, right_door) ||
		player_said_2(open, right_door)) {

		if (global[right_door_is_open_504]) {

			if (conv_control.running == CONV_MISC) {
				conv_abort();
			}

			new_room = 505;
			goto handled;

		} else {
			text_show(text_504_18);
			goto handled;
		}
	}

	if (player_said_2(walk_through, left_door) ||
		player_said_2(open, left_door)) {

		if (global[fight_status]) {
			if (player_has_been_in_room(506)) { /* Chris is waiting in 506 */
				new_room = 506;

			} else if (!player_has(music_score)) {
				text_show(text_504_25);
				/* Hey, why don't you get the music score? */

			} else {
				local->phan_action = FIGHT_CHRIS_LEAVE;
				player.commands_allowed = false;
				/* you have music score, ladies first into 506 */
			}

		} else {
			text_show(text_504_18);
			/* the door is locked tight */
		}
		goto handled;
	}

	if (player_said_2(take, music_score)) {
		switch (kernel.trigger) {
		case (0):
			if (object_is_here(music_score)) {
				global[player_score] += 5;
				player.commands_allowed = false;
				player.walker_visible = false;
				seq[fx_take_9] = kernel_seq_pingpong(ss[fx_take_9], false,
					5, 0, 0, 2);
				kernel_seq_range(seq[fx_take_9], 1, 4);
				kernel_seq_player(seq[fx_take_9], true);
				kernel_seq_trigger(seq[fx_take_9],
					KERNEL_TRIGGER_SPRITE, 4, 1);
				kernel_seq_trigger(seq[fx_take_9],
					KERNEL_TRIGGER_EXPIRE, 0, 2);
				goto handled;
			}
			break;

		case 1:
			seq[fx_music] = kernel_seq_stamp(ss[fx_music], false, 1);
			kernel_seq_depth(seq[fx_music], 14);
			kernel_flip_hotspot(words_music_score, false);
			inter_give_to_player(music_score);
			sound_play(N_TakeObjectSnd);
			goto handled;
			break;

		case 2:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_9]);
			player.walker_visible = true;
			kernel_timing_trigger(20, 3);
			goto handled;
			break;

		case 3:
			object_examine(music_score, text_008_20, 0);
			/* You pick up the music score */
			player.commands_allowed = true;
			goto handled;
			break;
		}
	}

	if (player.look_around) {
		text_show(text_504_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(floor)) {
			text_show(text_504_11);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_504_12);
			goto handled;
		}

		if (player_said_1(organ)) {
			text_show(text_504_13);
			goto handled;
		}

		if (player_said_1(organ_bench)) {
			text_show(text_504_14);
			goto handled;
		}

		if (player_said_1(music_score) && object_is_here(music_score)) {
			text_show(text_504_15);
			goto handled;
		}

		if (player_said_1(left_door)) {
			text_show(text_504_16);
			goto handled;
		}

		if (player_said_1(right_door)) {
			if (global[right_door_is_open_504]) {
				text_show(text_504_34);
			} else {
				text_show(text_504_17);
			}
			goto handled;
		}

		if (player_said_1(table)) {
			text_show(text_504_19);
			goto handled;
		}

		if (player_said_1(trap_door)) {
			text_show(text_504_20);
			goto handled;
		}

		if (player_said_1(large_chair)) {
			text_show(text_504_22);
			goto handled;
		}

		if (player_said_1(Christine)) {
			if (global[fight_status]) {
				text_show(text_504_26);
			} else {
				text_show(text_504_29);
			}
			goto handled;
		}

		if (player_said_1(Phantom)) {
			text_show(text_504_28);
			goto handled;
		}

	}

	if (player_said_2(open, trap_door)) {
		text_show(text_504_21);
		goto handled;
	}

	if (player_said_1(close)) {
		if (player_said_1(right_door)) {
			if (!global[right_door_is_open_504]) {
				text_show(text_504_33);
				goto handled;
			}
		}
	}

	if (player_said_1(close)) {
		if (player_said_1(left_door)) {
			if (!global[fight_status] && !player_has_been_in_room(506)) { /* Chris is waiting in 506 */
				text_show(text_504_33);
				goto handled;
			}
		}
	}

	if (local->anim_3_running) {
		if (player_said_2(take, sword) || player_said_2(attack, Phantom)) {
			player.commands_allowed = false;
			local->input_count = 0;
			local->phan_action = FIGHT_SWORD;
			goto handled;
		}
	}

	if (player_said_2(talk_to, Christine)) {
		conv_run(CONV_FIGHT);
		conv_export_value(player_has(music_score));
		conv_export_value(0);
		local->phan_action = FIGHT_CHRIS_TALK;
		local->chris_talk_count = 0;
		goto handled;
	}

	if (player_said_2(talk_to, Phantom)) {
		text_show(text_504_31);
		goto handled;
	}

	if (player_said_2(take, Christine)) {
		text_show(text_504_35);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}
static void handle_animation_listen() {
	int listen_reset_frame;

	if (kernel_anim[aa[0]].frame != local->listen_frame) {
		local->listen_frame = kernel_anim[aa[0]].frame;
		listen_reset_frame = -1;

		switch (local->listen_frame) {
		case 8:   /* listening    */
			conv_release();
			break;

		case 9:  /* listening     */
			if (local->listen_action == CONV19_LISTEN) {
				listen_reset_frame = 8;
				/* keep listening */
			} else {
				listen_reset_frame = 9;
				/* move away from door */
			}
			break;
		}

		if (listen_reset_frame >= 0) {
			kernel_reset_animation(aa[0], listen_reset_frame);
			local->listen_frame = listen_reset_frame;
		}
	}
}

static void handle_animation_play_organ() {
	int play_reset_frame;

	if (kernel_anim[aa[1]].frame != local->play_frame) {
		local->play_frame = kernel_anim[aa[1]].frame;
		play_reset_frame = -1;

		switch (local->play_frame) {
		case 22:   /* prompt for music */
			player.commands_allowed = true;
			conv_run(CONV_MUSIC);
			break;

		case 23:
			if (local->play_action == CONV27_SELECT) {
				play_reset_frame = 22;
				/* freeze while choosing piece */

			} else {
				player.commands_allowed = false;
				/* play organ */
				if (local->music_choice == 5) {
					local->music_is_playing = false;
					local->fire_breath = false;
					play_reset_frame = 104;
					/* get up */
				}
			}
			break;

		case 28:
			if (!local->music_is_playing) {
				local->music_is_playing = true;
				local->fire_breath = true;
				player.commands_allowed = false;

				switch (local->music_choice) {
				case 1:
					sound_play(N_Bach_dm);
					break;

				case 3:
					sound_play(N_Gigue);
					break;

				case 4:
					sound_play(N_Canon);
					break;

				case 2:
					sound_play(N_Bach_gm);
					break;
				}
			}
			break;

		case 69:   /* restart playing - maybe */
			if ((global[right_door_is_open_504]) && (local->play_count >= 2)) {
				local->play_count = 0;
				play_reset_frame = 102;
				sound_play(N_MusicOff);
				sound_play(N_BackgroundMus);
				/* get up */

			} else if (local->music_choice != global[music_selected]) {
				sound_play(N_MusicOff);
				local->fire_breath = true;
				play_reset_frame = 75;
				/* die */

			} else {
				play_reset_frame = 25;
				++local->play_count;
				if (!global[right_door_is_open_504]) {
					kernel_timing_trigger(1, ROOM_504_OPEN_RIGHT_DOOR);
					global[player_score] += 5;
				} /* keep playing (player will live) */
			}
			break;

		case 76:
			global_speech(speech_raoul_fire);
			break;

		case 90:
			sound_play(N_PlayerDies);
			break;

		case 102:   /* restart room - player died */
			++local->death_count;
			if (local->death_count >= 17) {
				kernel.force_restart = true;
			} else {
				play_reset_frame = 101;
			}
			break;
		}

		if (play_reset_frame >= 0) {
			kernel_reset_animation(aa[1], play_reset_frame);
			local->play_frame = play_reset_frame;
		}
	}
}


static void handle_animation_phantom_1() {
	int phan_reset_frame;
	/* int id; */



	if (kernel_anim[aa[3]].frame != local->phan_frame) {
		local->phan_frame = kernel_anim[aa[3]].frame;
		phan_reset_frame = -1;

		switch (local->phan_frame) {

		case 52: /* end of just as phantom comes into room (same for next 3) */
		case 53: /* Here, we'll wait for a few lines to be exchanged         */
		case 54:
		case 55:
			if (local->phan_action == FIGHT_ENTER_ROOM) {
				phan_reset_frame = imath_random(51, 54);
			} else {
				phan_reset_frame = 55;
			}
			break;

		case 41:  /* They first come into room (Phan enters from other side) */
			conv_run(CONV_FIGHT);
			conv_export_value(player_has(music_score));
			conv_export_value(0);
			break;  /* Chris says 'It's him' */

		case 78:
			conv_release();
			break; /* phantom is now jumping onto table and Raoul is walking to center of room */

		case 169:
			sound_play(N_AllFade);
			global_speech(speech_raoul_fire);
			break;

		case 180:
			sound_play(N_PlayerDies);
			break;

		case 187:
			local->death_count = 0;
			break;

		case 189:
			++local->death_count;
			if (local->death_count >= 29) {
				kernel.force_restart = true;
			} else {
				phan_reset_frame = 188;
			}
			break;

		case 110: /* Raoul stepped closer to phan - fireball coming */
		case 111: /* Raoul stepped closer to phan - fireball coming */
		case 112: /* Raoul stepped closer to phan - fireball coming */
		case 113: /* Raoul stepped closer to phan - fireball coming */
			if (local->phan_action == FIGHT_BEFORE_DODGE) {
				phan_reset_frame = imath_random(109, 112);
			} else {
				phan_reset_frame = 113;
			}
			break;

		case 142:
			kernel_draw_to_background(ss[fx_burn], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
			break;

		case 143:
			player.commands_allowed = true;
			break;

		case 149: /* end of wait for player input 3 */
		case 150: /* end of wait for player input 3 */
		case 151: /* end of wait for player input 3 */

			++local->input_count;

			if (local->phan_action == FIGHT_SWORD) {
				player.commands_allowed = false;
				phan_reset_frame = 200;

			} else if (local->input_count >= 9) {
				player.commands_allowed = false;
				phan_reset_frame = 151;

			} else if (local->phan_action == FIGHT_CHOICE) {
				phan_reset_frame = imath_random(148, 150);
			}
			break;

		case 227:

			kernel_abort_animation(aa[3]);
			matte_deallocate_series(ss[fx_a_0], true);
			matte_deallocate_series(ss[fx_a_7], true);
			matte_deallocate_series(ss[fx_a_6], true);
			matte_deallocate_series(ss[fx_a_3], true);
			matte_deallocate_series(ss[fx_a_5], true);

			ss[fx_test] = kernel_load_series("*CHR_3", PAL_MAP_ALL_TO_CLOSEST |
				PAL_MAP_ANY_TO_CLOSEST);
			ss[fx_a_8] = kernel_load_series(kernel_name('a', 8), PAL_MAP_ALL_TO_CLOSEST |
				PAL_MAP_ANY_TO_CLOSEST);

			/* What a fucking nightmare! */

			aa[3] = kernel_run_animation(kernel_name('p', 2), ROOM_504_RUN_PART_3);

			kernel_reset_animation(aa[3], 27);
			phan_reset_frame = -1;
			local->anim_3_running = false;
			local->anim_4_running = true;  /* still keep aa[3], though */
			break;

		}

		if (phan_reset_frame >= 0) {
			kernel_reset_animation(aa[3], phan_reset_frame);
			local->phan_frame = phan_reset_frame;
		}
	}
}



static void handle_animation_phantom_2() {
	int phan_reset_frame;

	if (kernel_anim[aa[3]].frame != local->phan_frame) {
		local->phan_frame = kernel_anim[aa[3]].frame;
		phan_reset_frame = -1;

		switch (local->phan_frame) {

		case 78:
			global_speech(speech_phantom_cackle);
			break;

		case 119:
			player.x = AFTER_FIGHT_X;
			player.y = AFTER_FIGHT_Y;
			player.facing = FACING_WEST;
			player.walker_visible = true;
			global[player_score] += 5;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[3]);
			player_walk(WALK_TO_AFTER_FIGHT_X, WALK_TO_AFTER_FIGHT_Y, FACING_NORTHEAST);
			kernel_flip_hotspot(words_Christine, false);
			player_walk_trigger(ROOM_504_COMPLETE_CONV);
			sound_play(N_AllFade);
			break;

		case 150:
			sound_play(N_BackgroundMus);
			break;

		case 160:
			if (local->phan_action == FIGHT_CHRIS_TALK) {
				phan_reset_frame = 160;
			} else if (local->phan_action != FIGHT_CHRIS_TO_DOOR) {
				phan_reset_frame = 159;
			} else {
				phan_reset_frame = 164;
			}
			break;

		case 161:
		case 162:
		case 163:
		case 164:
			if (local->phan_action == FIGHT_CHRIS_TO_DOOR) {
				phan_reset_frame = 159;
			} else {
				phan_reset_frame = imath_random(160, 162);
			}
			++local->chris_talk_count;
			if (local->chris_talk_count > 10) {
				phan_reset_frame = 159;
				if (local->phan_action != FIGHT_CHRIS_TO_DOOR) {
					local->phan_action = FIGHT_SWORD;
				}
			}
			break;

		}

		if (phan_reset_frame >= 0) {
			kernel_reset_animation(aa[3], phan_reset_frame);
			local->phan_frame = phan_reset_frame;
		}
	}
}

static void handle_animation_phantom_3() {
	int phan_reset_frame;

	if (kernel_anim[aa[3]].frame != local->phan_frame) {
		local->phan_frame = kernel_anim[aa[3]].frame;
		phan_reset_frame = -1;

		switch (local->phan_frame) {

		case 58:
			player.commands_allowed = true;
			break;

		case 80:
			if (local->phan_action == FIGHT_CHRIS_TALK) {
				phan_reset_frame = 116;
			} else if (local->phan_action != FIGHT_CHRIS_LEAVE) {
				phan_reset_frame = 79;
			}
			break;

		case 115:
			player_walk(EXIT_LEFT_DOOR_X, EXIT_LEFT_DOOR_Y, FACING_NORTHWEST);
			player_walk_trigger(ROOM_504_EXIT_INTO_506);
			break;

		case 116:
			phan_reset_frame = 115;
			break;

		case 117:
		case 118:
		case 119:
			phan_reset_frame = imath_random(116, 118);
			++local->chris_talk_count;
			if (local->chris_talk_count > 10) {
				phan_reset_frame = 79;
				if (local->phan_action != FIGHT_CHRIS_LEAVE) {
					local->phan_action = FIGHT_CHRIS_TO_DOOR;
				}
			}
			break;
		}

		if (phan_reset_frame >= 0) {
			kernel_reset_animation(aa[3], phan_reset_frame);
			local->phan_frame = phan_reset_frame;
		}
	}
}


static void handle_animation_chair() {
	int chair_reset_frame;

	if (kernel_anim[aa[2]].frame != local->chair_frame) {
		local->chair_frame = kernel_anim[aa[2]].frame;
		chair_reset_frame = -1;

		switch (local->chair_frame) {

		case 24:
			player.commands_allowed = true;
			break;

		case 25:
		case 26:
		case 30:
		case 31:

			if (!local->prevent) {
				local->prevent = true;
				text_show(text_504_24);
				/* someone is watching you */
			}

			if (local->chair_action == CHAIR_SIT) {
				if (imath_random(1, 5) == 1) {
					chair_reset_frame = imath_random(24, 30);

				} else {
					chair_reset_frame = local->chair_frame - 1;
				}

			} else {
				chair_reset_frame = 31; /* get up */
			}
			break;

		case 47:
			chair_reset_frame = -1;
			player.commands_allowed = true;
			player.walker_visible = true;
			player.ready_to_walk = true;
			local->anim_2_running = false;
			local->prevent = false;
			kernel_abort_animation(aa[2]);
			break;
		}

		if (chair_reset_frame >= 0) {
			kernel_reset_animation(aa[2], chair_reset_frame);
			local->chair_frame = chair_reset_frame;
		}
	}
}


void room_504_daemon() {
	int id;

	if (local->anim_0_running) {
		handle_animation_listen();
	}

	if (local->anim_1_running) {
		handle_animation_play_organ();
	}

	if (local->anim_2_running) {
		handle_animation_chair();
	}

	if (local->anim_3_running) {
		handle_animation_phantom_1();
	}

	if (local->anim_4_running) {
		handle_animation_phantom_2();
	}

	if (local->anim_5_running) {
		handle_animation_phantom_3();
	}

	if (kernel.trigger == ROOM_504_COMPLETE_CONV) {
		player.commands_allowed = false;
		conv_run(CONV_FIGHT);
		conv_export_value(player_has(music_score));
		conv_export_value(1);
		global[fight_status] = FIGHT_TALKING;
		/* after phantom dissapears into chair, chris & raoul say a few lines */
	}

	if (kernel.trigger == ROOM_504_FROM_502) {
		player.walker_visible = true;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
		kernel_draw_to_background(ss[fx_trap_door], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_timing_trigger(HALF_SECOND, ROOM_504_FROM_502 + 1);
	}

	if (kernel.trigger == ROOM_504_FROM_502 + 1) {
		player.commands_allowed = true;
		kernel_timing_trigger(ONE_SECOND, ROOM_504_FROM_502 + 2);
	}

	if (kernel.trigger == ROOM_504_FROM_502 + 2) {
		global[he_listened] = true;
		player.commands_allowed = false;
		conv_run(CONV_LISTEN);
		conv_export_value(game.difficulty);
	}

	if (kernel.trigger == ROOM_504_OPEN_RIGHT_DOOR) {
		sound_play(N_SecretDoor);
		global[right_door_is_open_504] = true;
		kernel_seq_delete(seq[fx_right_door]);
		seq[fx_right_door] = kernel_seq_forward(ss[fx_right_door], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_right_door], 1);
		kernel_seq_range(seq[fx_right_door], KERNEL_FIRST, KERNEL_LAST);

		seq[fx_ramhead] = kernel_seq_forward(ss[fx_ramhead], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_ramhead], 1);
		kernel_seq_range(seq[fx_ramhead], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_ramhead], KERNEL_TRIGGER_EXPIRE, 0, ROOM_504_NEXT_FIRE);
	}

	if (kernel.trigger == ROOM_504_NEXT_FIRE) {
		seq[fx_fire_left] = kernel_seq_forward(ss[fx_fire_left], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_fire_left], 1);
		kernel_seq_range(seq[fx_fire_left], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_fire_left], KERNEL_TRIGGER_EXPIRE, 0, ROOM_504_NEXT_FIRE + 1);
	}

	if (kernel.trigger == ROOM_504_NEXT_FIRE + 1) {
		seq[fx_fire_right] = kernel_seq_forward(ss[fx_fire_right], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_fire_right], 1);
		kernel_seq_range(seq[fx_fire_right], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_fire_right], KERNEL_TRIGGER_EXPIRE, 0, ROOM_504_NEXT_FIRE + 2);
	}

	if (kernel.trigger == ROOM_504_NEXT_FIRE + 2) {
		local->fire_breath = true;
	}

	if (local->fire_breath) {
		seq[fx_ramhead] = kernel_seq_forward(ss[fx_ramhead], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_ramhead], 1);
		kernel_seq_range(seq[fx_ramhead], KERNEL_FIRST, KERNEL_LAST);
		seq[fx_fire_left] = kernel_seq_forward(ss[fx_fire_left], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_fire_left], 1);
		kernel_seq_range(seq[fx_fire_left], KERNEL_FIRST, KERNEL_LAST);
		seq[fx_fire_right] = kernel_seq_forward(ss[fx_fire_right], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_fire_right], 1);
		kernel_seq_range(seq[fx_fire_right], KERNEL_FIRST, KERNEL_LAST);
		local->fire_breath = false;
	}

	if (kernel.trigger == ROOM_504_RUN_PART_3) {

		kernel_abort_animation(aa[3]);
		matte_deallocate_series(ss[fx_a_8], true);

		local->anim_4_running = false;
		local->anim_5_running = true;

		/* I never unload these because at end of anim, new room = 506 */

		ss[fx_test] = kernel_load_series("*CHR_2", PAL_MAP_ALL_TO_CLOSEST |
			PAL_MAP_ANY_TO_CLOSEST);
		ss[fx_test] = kernel_load_series("*CHR_3", PAL_MAP_ALL_TO_CLOSEST |
			PAL_MAP_ANY_TO_CLOSEST);

		kernel_load_variant(1);

		aa[3] = kernel_run_animation(kernel_name('p', 3), 0);
		id = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
			0, 0, 0, 0);
		kernel_dynamic_hot[id].prep = PREP_ON;
		kernel_dynamic_walk(id, DYNAMIC_CHR_WALK_TO_X, DYNAMIC_CHR_WALK_TO_Y, FACING_NORTHWEST);
		kernel_dynamic_anim(id, aa[3], 0);
		kernel_dynamic_anim(id, aa[3], 1);
		kernel_dynamic_anim(id, aa[3], 2);
		kernel_dynamic_anim(id, aa[3], 3);
		kernel_dynamic_anim(id, aa[3], 4);
	}

	if (kernel.trigger == ROOM_504_END_LISTEN) {
		player.commands_allowed = true;
		player.walker_visible = true;
		local->anim_0_running = false;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
	}

	if (kernel.trigger == ROOM_504_EXIT_INTO_506) {
		new_room = 506;
	}

	if (kernel.trigger == ROOM_504_DONE_PLAYING) {
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
		player.x = AFTER_PLAY_X;
		player.y = AFTER_PLAY_Y;
		player.walker_visible = true;
		local->anim_1_running = false;
		player_demand_facing(FACING_EAST);
		kernel_timing_trigger(10, ROOM_504_DONE_PLAYING + 1);
	}

	if (kernel.trigger == ROOM_504_DONE_PLAYING + 1) {
		player.commands_allowed = true;
		matte_deallocate_series(ss[fx_fire_right], true);
		matte_deallocate_series(ss[fx_fire_left], true);
	}
}

void room_504_preload() {
	room_init_code_pointer = room_504_init;
	room_pre_parser_code_pointer = room_504_pre_parser;
	room_parser_code_pointer = room_504_parser;
	room_daemon_code_pointer = room_504_daemon;

	section_5_walker();
	section_5_interface();

	vocab_make_active(words_Phantom);
	vocab_make_active(words_Christine);
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
