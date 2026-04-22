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
#include "mads/madsv2/core/rail.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/rooms/room103.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_103_adjust_rails(int variant) {
	switch (variant) {
	case 0:
		rail_disconnect_node(3);
		rail_disconnect_node(4);
		rail_disconnect_node(5);
		rail_disconnect_node(6);
		rail_disconnect_node(12);
		rail_disconnect_node(13);
		rail_disconnect_node(14);
		break;

	case 1:
		rail_disconnect_node(1);
		rail_disconnect_node(2);
		rail_disconnect_node(3);
		rail_disconnect_node(4);
		rail_disconnect_node(5);
		rail_disconnect_node(6);
		rail_disconnect_node(7);
		rail_disconnect_node(9);
		rail_disconnect_node(10);
		rail_disconnect_node(11);
		break;

	case 2:
		rail_disconnect_node(1);
		rail_disconnect_node(2);
		rail_disconnect_node(5);
		rail_disconnect_node(6);
		rail_disconnect_node(7);
		rail_disconnect_node(9);
		rail_disconnect_node(10);
		rail_disconnect_node(11);
		break;

	case 3:
		rail_disconnect_node(1);
		rail_disconnect_node(2);
		rail_disconnect_node(3);
		rail_disconnect_node(4);
		rail_disconnect_node(10);
		rail_disconnect_node(11);
		break;
	}
}

void room_103_init(void) {
	int prompt_x;
	int prompt_y;
	int prompt_facing;
	int dyn_floor_3;
	int dyn_floor_4;
	int dyn_floor_5;
	int dyn_floor_6;
	int dyn_floor_7;
	int dyn_floor_8;
	int dyn_jacques_body_1;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running    = false;
		local->anim_1_running    = false;
		local->anim_2_running    = false;
		local->anim_3_running    = false;
		local->anim_4_running    = false;
		local->anim_5_running    = false;
		local->anim_6_running    = false;
		local->climb_thru        = false;
		local->frame_guard       = false;
		local->sit_on_it         = false;
		local->jacques_action    = CONV12_JAC_SHUT_UP;
		local->just_did_option   = 0;
		local->top_of_stand      = NO;
	}


	/* =================== Load Sprite Series ==================== */

	if (global[jacques_status] >= JACQUES_IS_DEAD) {
		if (object_is_here(key)) {
			ss[fx_key]         = kernel_load_series(kernel_name('x', 2), false);
			ss[fx_bend_down_9] = kernel_load_series("*RRD_9", false);
		}
		ss[fx_steps]        = kernel_load_series(kernel_name('f', 3), false);
		ss[fx_jacques_body] = kernel_load_series(kernel_name('c', 1), false);
		ss[fx_broken_promp] = kernel_load_series(kernel_name('f', 1), false);

	} else {
		ss[fx_steps]   = kernel_load_series(kernel_name('f', 0), false);
		ss[fx_on_seat] = kernel_load_series(kernel_name('a', 3), false);
	}

	ss[fx_lever]        = kernel_load_series(kernel_name('x', 3), false);
	ss[fx_up_trap_door] = kernel_load_series(kernel_name('a', 2), false);
	ss[fx_rail_pieces]  = kernel_load_series(kernel_name('f', 2), false);
	ss[fx_trap_door]    = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_door]         = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_take_6]       = kernel_load_series("*RDR_6", false);


	/* ==================== Adjust rail nodes ==================== */

	room_103_adjust_rails(kernel_initial_variant);


	kernel_flip_hotspot(words_Jacques, false);
	kernel_flip_hotspot(words_key, false);


	/* ================== Load conversation ====================== */

	conv_get(CONV_JACQUES);


	/* ================= Stamp trap door & lever ================= */

	if (global[trap_door_status] == TRAP_DOOR_IS_CLOSED) {
		seq[fx_trap_door] = kernel_seq_stamp(ss[fx_trap_door], false, 1);
		kernel_seq_depth(seq[fx_trap_door], 14);
		seq[fx_lever] = kernel_seq_stamp(ss[fx_lever], false, 1);
		kernel_seq_depth(seq[fx_lever], 3);
	}

	if (global[trap_door_status] == TRAP_DOOR_IS_OPEN) {
		seq[fx_trap_door] = kernel_seq_stamp(ss[fx_trap_door], false, 5);
		kernel_seq_depth(seq[fx_trap_door], 14);
		seq[fx_lever] = kernel_seq_stamp(ss[fx_lever], false, 2);
		kernel_seq_depth(seq[fx_lever], 3);
	}


	if (global[jacques_status] == JACQUES_IS_ALIVE) {
		prompt_x       = DYN_PROMPT_R_WALK_TO_X_JAC;
		prompt_y       = DYN_PROMPT_R_WALK_TO_Y_JAC;
		prompt_facing  = FACING_NORTHEAST;
	} else {
		prompt_x       = DYN_PROMPT_R_WALK_TO_X;
		prompt_y       = DYN_PROMPT_R_WALK_TO_Y;
		prompt_facing  = FACING_NORTHWEST;
	}


	/* ========================= Stairs ========================== */

	if ((global[prompter_stand_status] == PROMPT_RIGHT) || (global[current_year] == 1881)) {

		if (global[jacques_status] >= JACQUES_IS_DEAD) {
			seq[fx_broken_promp] = kernel_seq_stamp(ss[fx_broken_promp], false, 1);
			kernel_seq_depth(seq[fx_broken_promp], 1);
			seq[fx_steps] = kernel_seq_stamp(ss[fx_steps], false, 1);
			kernel_seq_depth(seq[fx_steps], 4);
			kernel_seq_loc(seq[fx_steps], PROMPT_RIGHT_X, PROMPT_RIGHT_Y);

		} else {
			seq[fx_steps] = kernel_seq_stamp(ss[fx_steps], false, 1);
			kernel_seq_depth(seq[fx_steps], 4);
			kernel_seq_loc(seq[fx_steps], PROMPT_RIGHT_X, PROMPT_RIGHT_Y);

			seq[fx_rail_pieces] = kernel_seq_stamp(ss[fx_rail_pieces], false, 1);
			kernel_seq_depth(seq[fx_rail_pieces], 1);
		}

		local->prompt_1 = kernel_add_dynamic(words_prompter_s_stand, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                    DYN_PROMPT_RIGHT_1_X, DYN_PROMPT_RIGHT_1_Y,
		                    DYN_PROMPT_X_SIZE_1, DYN_PROMPT_Y_SIZE_1);
		kernel_dynamic_hot[local->prompt_1].prep = PREP_ON;
		kernel_dynamic_walk(local->prompt_1, prompt_x, prompt_y, prompt_facing);
		local->prompt_2 = kernel_add_dynamic(words_prompter_s_stand, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                    DYN_PROMPT_RIGHT_2_X, DYN_PROMPT_RIGHT_2_Y,
		                    DYN_PROMPT_X_SIZE_2, DYN_PROMPT_Y_SIZE_2);
		kernel_dynamic_hot[local->prompt_2].prep = PREP_ON;
		kernel_dynamic_walk(local->prompt_2, prompt_x, prompt_y, prompt_facing);
		local->prompt_3 = kernel_add_dynamic(words_prompter_s_stand, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                    DYN_PROMPT_RIGHT_3_X, DYN_PROMPT_RIGHT_3_Y,
		                    DYN_PROMPT_X_SIZE_3, DYN_PROMPT_Y_SIZE_3);
		kernel_dynamic_hot[local->prompt_3].prep = PREP_ON;
		kernel_dynamic_walk(local->prompt_3, prompt_x, prompt_y, prompt_facing);
		local->prompt_4 = kernel_add_dynamic(words_prompter_s_stand, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                    DYN_PROMPT_RIGHT_4_X, DYN_PROMPT_RIGHT_4_Y,
		                    DYN_PROMPT_X_SIZE_4, DYN_PROMPT_Y_SIZE_4);
		kernel_dynamic_hot[local->prompt_4].prep = PREP_ON;
		kernel_dynamic_walk(local->prompt_4, prompt_x, prompt_y, prompt_facing);
		local->prompt_5 = kernel_add_dynamic(words_prompter_s_stand, words_climb, SYNTAX_SINGULAR, KERNEL_NONE,
		                    DYN_PROMPT_RIGHT_5_X, DYN_PROMPT_LEFT_5_Y,
		                    DYN_PROMPT_X_SIZE_5, DYN_PROMPT_Y_SIZE_5);
		kernel_dynamic_hot[local->prompt_5].prep = PREP_ON;
		kernel_dynamic_walk(local->prompt_5, PROMPT_UP_RIGHT_X, PROMPT_UP_RIGHT_Y, FACING_SOUTHWEST);
		kernel_dynamic_cursor(local->prompt_5, CURSOR_UP);

		local->floor_r_1 = kernel_add_dynamic(words_floor, words_walk_across, SYNTAX_SINGULAR, KERNEL_NONE,
		                    DYN_FLOOR_R_1_X, DYN_FLOOR_R_1_Y,
		                    DYN_FLOOR_R_1_X_SIZE, DYN_FLOOR_R_1_Y_SIZE);
		kernel_dynamic_hot[local->floor_r_1].prep = PREP_ON;
		kernel_dynamic_walk(local->floor_r_1, DYN_FLOOR_R_1_WALK_TO_X, DYN_FLOOR_R_1_WALK_TO_Y, 5);
		local->floor_r_2 = kernel_add_dynamic(words_floor, words_walk_across, SYNTAX_SINGULAR, KERNEL_NONE,
		                    DYN_FLOOR_R_2_X, DYN_FLOOR_R_2_Y,
		                    DYN_FLOOR_R_2_X_SIZE, DYN_FLOOR_R_2_Y_SIZE);
		kernel_dynamic_hot[local->floor_r_2].prep = PREP_ON;
		kernel_dynamic_walk(local->floor_r_2, DYN_FLOOR_R_2_WALK_TO_X, DYN_FLOOR_R_2_WALK_TO_Y, 5);

		if ((global[jacques_status] == JACQUES_IS_ALIVE) && (global[current_year] == 1881)) {
			if (global[jacques_name_is_known] >= YES) {
				local->man = kernel_add_dynamic(words_Jacques, words_walk_to, SYNTAX_SINGULAR_MASC, KERNEL_NONE,
				               DYN_JAC_X, DYN_JAC_Y,
				               DYN_JAC_X_SIZE, DYN_JAC_Y_SIZE);
				kernel_dynamic_hot[local->man].prep = PREP_ON;
				kernel_dynamic_walk(local->man, DYN_JAC_WALK_TO_X, DYN_JAC_WALK_TO_Y, FACING_NORTHWEST);
			} else {
				local->man = kernel_add_dynamic(words_gentleman, words_walk_to, SYNTAX_MASC_NOT_PROPER, KERNEL_NONE,
				               DYN_JAC_X, DYN_JAC_Y,
				               DYN_JAC_X_SIZE, DYN_JAC_Y_SIZE);
				kernel_dynamic_hot[local->man].prep = PREP_ON;
				kernel_dynamic_walk(local->man, DYN_JAC_WALK_TO_X, DYN_JAC_WALK_TO_Y, FACING_NORTHWEST);
			}
			dyn_floor_3 = kernel_add_dynamic(words_floor, words_walk_across, SYNTAX_SINGULAR, KERNEL_NONE,
			               DYN_FLOOR_R_3_X, DYN_FLOOR_R_3_Y,
			               DYN_FLOOR_R_3_X_SIZE, DYN_FLOOR_R_3_Y_SIZE);
			kernel_dynamic_hot[dyn_floor_3].prep = PREP_ON;
			kernel_dynamic_walk(dyn_floor_3, DYN_FLOOR_R_3_WALK_TO_X, DYN_FLOOR_R_3_WALK_TO_Y, 5);
			dyn_floor_4 = kernel_add_dynamic(words_floor, words_walk_across, SYNTAX_SINGULAR, KERNEL_NONE,
			               DYN_FLOOR_R_4_X, DYN_FLOOR_R_4_Y,
			               DYN_FLOOR_R_4_X_SIZE, DYN_FLOOR_R_4_Y_SIZE);
			kernel_dynamic_hot[dyn_floor_4].prep = PREP_ON;
			kernel_dynamic_walk(dyn_floor_4, DYN_FLOOR_R_4_WALK_TO_X, DYN_FLOOR_R_4_WALK_TO_Y, 5);

		} else if ((global[jacques_status] >= JACQUES_IS_DEAD) && (global[current_year] == 1881)) {
			/* Jacques is dead - stamp him, key, & dynamics */
			seq[fx_jacques_body] = kernel_seq_stamp(ss[fx_jacques_body], false, 1);
			kernel_seq_depth(seq[fx_jacques_body], 3);
			if (object_is_here(key)) {
				seq[fx_key] = kernel_seq_stamp(ss[fx_key], false, 1);
				kernel_seq_depth(seq[fx_key], 14);
				kernel_flip_hotspot(words_key, true);
			}
			kernel_flip_hotspot(words_Jacques, true);
			kernel_delete_dynamic(local->floor_r_2);
			dyn_jacques_body_1 = kernel_add_dynamic(words_Jacques, words_walk_to, SYNTAX_SINGULAR_MASC, KERNEL_NONE,
			                     DYN_JAC_BODY_X, DYN_JAC_BODY_Y,
			                     DYN_JAC_BODY_X_SIZE, DYN_JAC_BODY_Y_SIZE);
			kernel_dynamic_hot[dyn_jacques_body_1].prep = PREP_ON;
			kernel_dynamic_walk(dyn_jacques_body_1, DYN_JAC_BODY_WALK_TO_X, DYN_JAC_BODY_WALK_TO_Y, FACING_NORTHEAST);
			dyn_floor_5 = kernel_add_dynamic(words_floor, words_walk_across, SYNTAX_SINGULAR, KERNEL_NONE,
			               DYN_FLOOR_R_5_X, DYN_FLOOR_R_5_Y,
			               DYN_FLOOR_R_5_X_SIZE, DYN_FLOOR_R_5_Y_SIZE);
			kernel_dynamic_hot[dyn_floor_5].prep = PREP_ON;
			kernel_dynamic_walk(dyn_floor_5, DYN_FLOOR_R_5_WALK_TO_X, DYN_FLOOR_R_5_WALK_TO_Y, 5);
			dyn_floor_6 = kernel_add_dynamic(words_floor, words_walk_across, SYNTAX_SINGULAR, KERNEL_NONE,
			               DYN_FLOOR_R_6_X, DYN_FLOOR_R_6_Y,
			               DYN_FLOOR_R_6_X_SIZE, DYN_FLOOR_R_6_Y_SIZE);
			kernel_dynamic_hot[dyn_floor_6].prep = PREP_ON;
			kernel_dynamic_walk(dyn_floor_6, DYN_FLOOR_R_6_WALK_TO_X, DYN_FLOOR_R_6_WALK_TO_Y, 5);
			dyn_floor_7 = kernel_add_dynamic(words_floor, words_walk_across, SYNTAX_SINGULAR, KERNEL_NONE,
			               DYN_FLOOR_R_7_X, DYN_FLOOR_R_7_Y,
			               DYN_FLOOR_R_7_X_SIZE, DYN_FLOOR_R_7_Y_SIZE);
			kernel_dynamic_hot[dyn_floor_7].prep = PREP_ON;
			kernel_dynamic_walk(dyn_floor_7, DYN_FLOOR_R_7_WALK_TO_X, DYN_FLOOR_R_7_WALK_TO_Y, 5);
			dyn_floor_8 = kernel_add_dynamic(words_floor, words_walk_across, SYNTAX_SINGULAR, KERNEL_NONE,
			               DYN_FLOOR_R_8_X, DYN_FLOOR_R_8_Y,
			               DYN_FLOOR_R_8_X_SIZE, DYN_FLOOR_R_8_Y_SIZE);
			kernel_dynamic_hot[dyn_floor_8].prep = PREP_ON;
			kernel_dynamic_walk(dyn_floor_8, DYN_FLOOR_R_8_WALK_TO_X, DYN_FLOOR_R_8_WALK_TO_Y, 5);
		}

	} else if (global[prompter_stand_status] == PROMPT_LEFT) {

		seq[fx_steps] = kernel_seq_stamp(ss[fx_steps], false, 1);
		kernel_seq_depth(seq[fx_steps], 4);
		kernel_seq_loc(seq[fx_steps], PROMPT_LEFT_X, PROMPT_LEFT_Y);

		local->prompt_1 = kernel_add_dynamic(words_prompter_s_stand, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                    DYN_PROMPT_LEFT_1_X, DYN_PROMPT_LEFT_1_Y,
		                    DYN_PROMPT_X_SIZE_1, DYN_PROMPT_Y_SIZE_1);
		kernel_dynamic_hot[local->prompt_1].prep = PREP_ON;
		kernel_dynamic_walk(local->prompt_1, DYN_PROMPT_L_WALK_TO_X, DYN_PROMPT_L_WALK_TO_Y, FACING_NORTHWEST);

		local->prompt_2 = kernel_add_dynamic(words_prompter_s_stand, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                    DYN_PROMPT_LEFT_2_X, DYN_PROMPT_LEFT_2_Y,
		                    DYN_PROMPT_X_SIZE_2, DYN_PROMPT_Y_SIZE_2);
		kernel_dynamic_hot[local->prompt_2].prep = PREP_ON;
		kernel_dynamic_walk(local->prompt_2, DYN_PROMPT_L_WALK_TO_X, DYN_PROMPT_L_WALK_TO_Y, FACING_NORTHWEST);

		local->prompt_3 = kernel_add_dynamic(words_prompter_s_stand, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                    DYN_PROMPT_LEFT_3_X, DYN_PROMPT_LEFT_3_Y,
		                    DYN_PROMPT_X_SIZE_3, DYN_PROMPT_Y_SIZE_3);
		kernel_dynamic_hot[local->prompt_3].prep = PREP_ON;
		kernel_dynamic_walk(local->prompt_3, DYN_PROMPT_L_WALK_TO_X, DYN_PROMPT_L_WALK_TO_Y, FACING_NORTHWEST);

		local->prompt_5 = kernel_add_dynamic(words_prompter_s_stand, words_climb, SYNTAX_SINGULAR, KERNEL_NONE,
		                    DYN_PROMPT_LEFT_5_X, DYN_PROMPT_LEFT_5_Y,
		                    DYN_PROMPT_X_SIZE_5, DYN_PROMPT_Y_SIZE_5);
		kernel_dynamic_hot[local->prompt_5].prep = PREP_ON;
		kernel_dynamic_walk(local->prompt_5, PROMPT_UP_LEFT_X, PROMPT_UP_LEFT_Y, FACING_SOUTHWEST);
		kernel_dynamic_cursor(local->prompt_5, CURSOR_UP);

		local->floor_l_1 = kernel_add_dynamic(words_floor, words_walk_across, SYNTAX_SINGULAR, KERNEL_NONE,
		                    DYN_FLOOR_L_1_X, DYN_FLOOR_L_1_Y,
		                    DYN_FLOOR_L_1_X_SIZE, DYN_FLOOR_L_1_Y_SIZE);
		kernel_dynamic_hot[local->floor_l_1].prep = PREP_ON;
		kernel_dynamic_walk(local->floor_l_1, DYN_FLOOR_L_1_WALK_TO_X, DYN_FLOOR_L_1_WALK_TO_Y, 5);
		local->floor_l_2 = kernel_add_dynamic(words_floor, words_walk_across, SYNTAX_SINGULAR, KERNEL_NONE,
		                    DYN_FLOOR_L_2_X, DYN_FLOOR_L_2_Y,
		                    DYN_FLOOR_L_2_X_SIZE, DYN_FLOOR_L_2_Y_SIZE);
		kernel_dynamic_hot[local->floor_l_2].prep = PREP_ON;
		kernel_dynamic_walk(local->floor_l_2, DYN_FLOOR_L_2_WALK_TO_X, DYN_FLOOR_L_2_WALK_TO_Y, 5);
	}


	/* ============= Put Jacques here if alive =================== */

	if ((global[jacques_status] == JACQUES_IS_ALIVE) && (global[current_year] == 1881)) {
		aa[0]                 = kernel_run_animation(kernel_name('j', 1), 1);
		local->anim_0_running = true;
		kernel_reset_animation(aa[0], 9); /* start Jacques at frame 9 */
	}


	/* ====================== Previous Rooms ===================== */

	if (previous_room == 104) {
		player.commands_allowed = false;
		player.walker_visible   = false;
		if (global[room_103_104_transition] == PEEK_THROUGH) {
			global[room_103_104_transition] = NEW_ROOM;
			seq[fx_on_seat] = kernel_seq_stamp(ss[fx_on_seat], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_on_seat], 4);
			local->top_of_stand = YES_LEFT;
			player.x            = PROMPT_UP_LEFT_X;
			player.y            = PROMPT_UP_LEFT_Y;
			kernel_timing_trigger(1, ROOM_103_GET_OFF_SEAT);
			/* stamp on seat, then let Daemon get him off */

		} else {
			local->top_of_stand     = YES_RIGHT;
			aa[3]                   = kernel_run_animation(kernel_name('w', 1), 0);
			local->anim_3_running   = true;
			player.commands_allowed = true;
			player.x                = PROMPT_UP_RIGHT_X;
			player.y                = PROMPT_UP_RIGHT_Y;
			kernel_reset_animation(aa[3], 36);
			/* stamp him on right stairs on top */

			kernel_seq_delete(seq[fx_steps]);
			seq[fx_steps] = kernel_seq_stamp(ss[fx_steps], false, 1);
			kernel_seq_depth(seq[fx_steps], 13);
			kernel_seq_loc(seq[fx_steps], PROMPT_RIGHT_X, PROMPT_RIGHT_Y);
		}

		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 14);

	} else if (previous_room == 102) {
		player_first_walk(OFF_SCREEN_X_FROM_102, OFF_SCREEN_Y_FROM_102, FACING_EAST,
		                  PLAYER_X_FROM_102, PLAYER_Y_FROM_102, FACING_EAST, true);
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 14);

	} else if ((previous_room == 105) || (previous_room != KERNEL_RESTORING_GAME)) {
		player.x                = PLAYER_X_FROM_105;
		player.y                = PLAYER_Y_FROM_105;
		player.facing           = FACING_WEST;
		player.commands_allowed = false;
		player_walk(WALK_TO_X_FROM_105, WALK_TO_Y_FROM_105, FACING_WEST);
		player_walk_trigger(ROOM_103_DOOR_CLOSES);
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 6);
		kernel_seq_depth(seq[fx_door], 14);

	} else if (previous_room == KERNEL_RESTORING_GAME) {
		if (local->top_of_stand == YES_LEFT) {
			player.walker_visible = false;
			aa[5]                 = kernel_run_animation(kernel_name('w', 3), 0);
			local->anim_5_running = true;
			kernel_reset_animation(aa[5], 33);

		} else if (local->top_of_stand == YES_RIGHT) {
			kernel_seq_delete(seq[fx_steps]);
			seq[fx_steps] = kernel_seq_stamp(ss[fx_steps], false, 1);
			kernel_seq_depth(seq[fx_steps], 13);
			kernel_seq_loc(seq[fx_steps], PROMPT_RIGHT_X, PROMPT_RIGHT_Y);
			player.walker_visible   = false;
			aa[3]                   = kernel_run_animation(kernel_name('w', 1), 0);
			local->anim_3_running   = true;
			player.commands_allowed = true;
			kernel_reset_animation(aa[3], 36);

		} else {
			if (conv_restore_running == CONV_JACQUES) {
				conv_run(CONV_JACQUES);
				conv_export_pointer(&global[player_score]);
				conv_export_value(global[music_selected]);
				global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
			}
		}
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 14);
	}

	section_1_music();
}

static void climb_stairs_right(void) {
	int stairs_reset_frame;

	if (kernel_anim[aa[3]].frame != local->stairs_frame) {
		local->stairs_frame = kernel_anim[aa[3]].frame;
		stairs_reset_frame = -1;

		if (local->stairs_frame == 37) {
			stairs_reset_frame      = 36;
			local->top_of_stand     = YES_RIGHT;
			player.commands_allowed = true;
		}

		if (local->stairs_frame == 2) {
			kernel_seq_delete(seq[fx_steps]);
			seq[fx_steps] = kernel_seq_stamp(ss[fx_steps], false, 1);
			kernel_seq_depth(seq[fx_steps], 13);
			kernel_seq_loc(seq[fx_steps], PROMPT_RIGHT_X, PROMPT_RIGHT_Y);
		}

		if (stairs_reset_frame >= 0) {
			kernel_reset_animation(aa[3], stairs_reset_frame);
			local->stairs_frame = stairs_reset_frame;
		}
	}
}

static void decend_stairs_right(void) {
	int stairs_reset_frame;

	if (kernel_anim[aa[4]].frame != local->stairs_frame) {
		local->stairs_frame = kernel_anim[aa[4]].frame;
		stairs_reset_frame  = -1;

		if (local->stairs_frame == 2) {
			kernel_seq_delete(seq[fx_steps]);
			seq[fx_steps] = kernel_seq_stamp(ss[fx_steps], false, 1);
			kernel_seq_depth(seq[fx_steps], 4);
			kernel_seq_loc(seq[fx_steps], PROMPT_RIGHT_X, PROMPT_RIGHT_Y);
		}

		if (stairs_reset_frame >= 0) {
			kernel_reset_animation(aa[4], stairs_reset_frame);
			local->stairs_frame = stairs_reset_frame;
		}
	}
}

static void climb_stairs_left(void) {
	int stairs_reset_frame;

	if (kernel_anim[aa[5]].frame != local->stairs_frame) {
		local->stairs_frame = kernel_anim[aa[5]].frame;
		stairs_reset_frame  = -1;

		if (local->stairs_frame == 34) {
			stairs_reset_frame      = 33;
			local->top_of_stand     = YES_LEFT;
			player.commands_allowed = true;
		}

		if (local->stairs_frame == 2) {
			kernel_seq_delete(seq[fx_steps]);
			seq[fx_steps] = kernel_seq_stamp(ss[fx_steps], false, 1);
			kernel_seq_depth(seq[fx_steps], 13);
			kernel_seq_loc(seq[fx_steps], PROMPT_LEFT_X, PROMPT_LEFT_Y);
		}

		if (stairs_reset_frame >= 0) {
			kernel_reset_animation(aa[5], stairs_reset_frame);
			local->stairs_frame = stairs_reset_frame;
		}
	}
}

static void decend_stairs_left(void) {
	int stairs_reset_frame;

	if (kernel_anim[aa[6]].frame != local->stairs_frame) {
		local->stairs_frame = kernel_anim[aa[6]].frame;
		stairs_reset_frame  = -1;

		if (local->stairs_frame == 2) {
			kernel_seq_delete(seq[fx_steps]);
			seq[fx_steps] = kernel_seq_stamp(ss[fx_steps], false, 1);
			kernel_seq_depth(seq[fx_steps], 4);
			kernel_seq_loc(seq[fx_steps], PROMPT_LEFT_X, PROMPT_LEFT_Y);
		}

		if (stairs_reset_frame >= 0) {
			kernel_reset_animation(aa[6], stairs_reset_frame);
			local->stairs_frame = stairs_reset_frame;
		}
	}
}

static void handle_animation_jacques(void) {
	int random;
	int jacques_reset_frame;

	if (kernel_anim[aa[0]].frame != local->jacques_frame) {
		local->jacques_frame = kernel_anim[aa[0]].frame;
		jacques_reset_frame = -1;

		switch (local->jacques_frame) {
		case 1:   /* end of talk 1            */
		case 2:   /* end of talk 2            */
		case 3:   /* end of talk 3            */
		case 9:   /* end of point up & freeze */
		case 17:  /* end of I dunno 1         */
		case 33:  /* end of brush arm         */
		case 23:  /* end of touch head        */
		case 51:  /* end of put arms on knees (from other node) */

			switch (local->jacques_action) {
			case CONV12_JAC_POINT:
				random = 4; /* point up */
				local->jacques_action = CONV12_JAC_TALK;
				break;

			case CONV12_JAC_I_DUNNO_1:
				random = 5; /* i dunno 1 */
				local->jacques_action = CONV12_JAC_TALK;
				break;

			case CONV12_JAC_I_DUNNO_2:
				random = 6; /* i dunno 2 */
				break;

			case CONV12_JAC_TALK:
				random = imath_random(1, 3);
				++local->jacques_talk_count;
				if (local->jacques_talk_count > 22) {
					local->jacques_action = CONV12_JAC_SHUT_UP;
					random = 9;
				}
				break;

			default:
				random = imath_random(6, 50);
				while (local->just_did_option == random) {
					random = imath_random(6, 50);
				} /* so we don't repeat the same option twice */
				local->just_did_option = random;
				break;
			}

			switch (random) {
			case 1:
				jacques_reset_frame = 0;
				break; /* do talk 1 */
			case 2:
				jacques_reset_frame = 1;
				break; /* do talk 2 */
			case 3:
				jacques_reset_frame = 2;
				break; /* do talk 3 */
			case 4:
				jacques_reset_frame = 4;
				break; /* point up */
			case 5:
				jacques_reset_frame = 10;
				break; /* i dunno 1 */
			case 6:
				jacques_reset_frame = 34;
				break; /* fold arms (goto next node) */
			case 7:
				jacques_reset_frame = 24;
				break; /* brush arm */
			case 8:
				jacques_reset_frame = 18;
				break; /* turn head */
			default:
				jacques_reset_frame = 8;
				break; /* arms frozen on knees */
			}
			break;

		case 36:   /* end of freeze */
		case 40:   /* end i dunno 2 */
		case 48:   /* end of fold hands (from other node) */

			switch (local->jacques_action) {
			case CONV12_JAC_POINT:
			case CONV12_JAC_I_DUNNO_1:
			case CONV12_JAC_TALK:
				random = 2; /* goto arms on knees (other node) */
				break;

			case CONV12_JAC_I_DUNNO_2:
				random = 1; /* i dunno 2 */
				local->jacques_action = CONV12_JAC_TALK;
				break;

			default:
				random = imath_random(2, 50);
				while (local->just_did_option == random) {
					random = imath_random(2, 50);
				}
				local->just_did_option = random;
				break;
			}

			switch (random) {
			case 1:
				jacques_reset_frame = 37;
				break; /* i dunno 2 */
			case 2:
				jacques_reset_frame = 49;
				break; /* put arms on knees (goto other node) */
			case 3:
				jacques_reset_frame = 41;
				break; /* put hands on side */
			default:
				jacques_reset_frame = 35;
				break; /* hands folded frozen */
			}
			break;

		case 44:   /* end of hands on side and frozen */

			random = imath_random(1, 50);
			while (local->just_did_option == random) {
				random = imath_random(1, 50);
			}
			local->just_did_option = random;

			switch (local->jacques_action) {
			case CONV12_JAC_POINT:
			case CONV12_JAC_I_DUNNO_1:
			case CONV12_JAC_I_DUNNO_2:
			case CONV12_JAC_TALK:
				random = 1; /* fold hands (other node) */
				break;

			default:
				random = imath_random(1, 50);
				while (local->just_did_option == random) {
					random = imath_random(1, 50);
				}
				local->just_did_option = random;
				break;
			}

			switch (random) {
			case 1:
				jacques_reset_frame = 45;
				break; /* fold hands (goto other node) */
			default:
				jacques_reset_frame = 43;
				break; /* freeze hands on side */
			}
			break;
		}

		if (jacques_reset_frame >= 0) {
			kernel_reset_animation(aa[0], jacques_reset_frame);
			local->jacques_frame = jacques_reset_frame;
		}
	}
}

void room_103_daemon(void) {
	int temp;  /* for synching purposes */

	if (local->anim_0_running) {
		handle_animation_jacques();
	}

	if (local->anim_3_running) {
		climb_stairs_right();
	}

	if (local->anim_5_running) {
		climb_stairs_left();
	}

	if (local->anim_4_running) {
		decend_stairs_right();
	}

	if (local->anim_6_running) {
		decend_stairs_left();
	}


	/* =========== Deal with talker offsets during conv ========== */

	if ((global[walker_converse] == CONVERSE_HAND_WAVE) ||
	    (global[walker_converse] == CONVERSE_HAND_WAVE_2)) {
		++local->converse_counter;
		if (local->converse_counter > 200) {
			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		}
	}


	/* ========= Close door when player enters from 105 ========== */

	switch (kernel.trigger) {
	case ROOM_103_DOOR_CLOSES:
		kernel_seq_delete(seq[fx_door]);
		seq[fx_door] = kernel_seq_backward(ss[fx_door], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_door], 14);
		kernel_seq_range(seq[fx_door], 1, 6);
		kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_103_DOOR_CLOSES + 1);
		sound_play(N_SqueakyDoor);
		break;

	case ROOM_103_DOOR_CLOSES + 1:
		temp = seq[fx_door];
		sound_play(N_DoorCloses);
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 1);
		kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, temp);
		kernel_seq_depth(seq[fx_door], 14);
		player.commands_allowed = true;
		break;

	case ROOM_103_DIED:
		new_room             = 104;
		kernel.force_restart = true;
		break;
	}


	/* ==================== Get off of seat ====================== */

	switch (kernel.trigger) {
	case ROOM_103_GET_OFF_SEAT:
		kernel_seq_delete(seq[fx_on_seat]);
		seq[fx_on_seat] = kernel_seq_backward(ss[fx_on_seat], false, 6, 0, 0, 1);
		kernel_seq_trigger(seq[fx_on_seat], KERNEL_TRIGGER_EXPIRE, 0, ROOM_103_GET_OFF_SEAT + 1);
		kernel_seq_range(seq[fx_on_seat], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_depth(seq[fx_on_seat], 4);
		break;

	case ROOM_103_GET_OFF_SEAT + 1:
		aa[5]                   = kernel_run_animation(kernel_name('w', 3), 0);
		local->anim_5_running   = true;
		kernel_reset_animation(aa[5], 33);
		player.commands_allowed = true;
		kernel_synch(KERNEL_ANIM, aa[5], KERNEL_SERIES, seq[fx_on_seat]);
		break;
	}


	/* ================= Open/close trap door ==================== */

	switch (kernel.trigger) {
	case 0:
		if (local->anim_1_running) {
			if ((kernel_anim[aa[1]].frame == 10) && (!local->frame_guard)) {
				sound_play(N_TrapDoor001);
				local->frame_guard = true;
				kernel_seq_delete(seq[fx_trap_door]);
				kernel_seq_delete(seq[fx_lever]);
				seq[fx_trap_door] = kernel_seq_forward(ss[fx_trap_door], false, 6, 0, 0, 1);
				kernel_seq_depth(seq[fx_trap_door], 14);
				kernel_seq_range(seq[fx_trap_door], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_trap_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_103_SWITCH_DOWN);
			}

		} else if (local->anim_2_running) {
			if ((kernel_anim[aa[2]].frame == 7) && (!local->frame_guard)) {
				sound_play(N_TrapDoor001);
				local->frame_guard = true;
				kernel_seq_delete(seq[fx_trap_door]);
				kernel_seq_delete(seq[fx_lever]);
				seq[fx_trap_door] = kernel_seq_backward(ss[fx_trap_door], false, 6, 0, 0, 1);
				kernel_seq_depth(seq[fx_trap_door], 14);
				kernel_seq_range(seq[fx_trap_door], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_trap_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_103_SWITCH_UP);
			}
		}
		break;

	case ROOM_103_SWITCH_DOWN:
		seq[fx_trap_door] = kernel_seq_stamp(ss[fx_trap_door], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_trap_door], 14);
		seq[fx_lever] = kernel_seq_stamp(ss[fx_lever], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_lever], 2);
		local->frame_guard = false;
		break;

	case ROOM_103_SWITCH_UP:
		seq[fx_trap_door] = kernel_seq_stamp(ss[fx_trap_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_trap_door], 14);
		seq[fx_lever] = kernel_seq_stamp(ss[fx_lever], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_lever], 2);
		local->frame_guard = false;
		break;
	}

	if ((global[jacques_status] == JAC_DEAD_RICH_GONE_SEEN_BODY) &&
	    (!sound_queue(N_IsAnySoundOn))) {
		sound_play(N_BackMus1stTime);
	}
}

static void process_conv_jacques(void) {
	int quit_converse = false;
	int16 *value_1;

	switch (player_verb) {
	case conv012_hello_four:
	case conv012_byebye_first:
	case conv012_questions_three:
	case conv012_seen_byebye:
	case conv012_tell_byebye:
	case conv012_nomore_first:
		global[walker_converse] = CONVERSE_NONE;
		value_1 = conv_get_variable(conv012_var_questions_done);
		if (*value_1) {
			global[jacques_name_is_known] = YES_AND_END_CONV;
		}
		quit_converse = true;
		break;

	case conv012_questions_one:
		conv_you_trigger(ROOM_103_JACQUES_POINT);
		break;

	case conv012_tell_knewhim:
		conv_you_trigger(ROOM_103_JACQUES_I_DUNNO_2);
		break;

	case conv012_seen_mask:
		conv_you_trigger(ROOM_103_JACQUES_I_DUNNO_1);
		break;

	case conv012_hello_one:
		conv_you_trigger(ROOM_103_JACQUES_I_DUNNO_1);
		if (global[jacques_name_is_known] == NO) {
			global[jacques_name_is_known] = YES;
			kernel_delete_dynamic(local->man);
			local->man = kernel_add_dynamic(words_Jacques, words_walk_to, SYNTAX_SINGULAR_MASC, KERNEL_NONE,
			               DYN_JAC_X, DYN_JAC_Y,
			               DYN_JAC_X_SIZE, DYN_JAC_Y_SIZE);
			kernel_dynamic_hot[local->man].prep = PREP_ON;
			kernel_dynamic_walk(local->man, DYN_JAC_WALK_TO_X, DYN_JAC_WALK_TO_Y, FACING_NORTHWEST);
		}
		break;
	}

	if ((player_verb != conv012_questions_one) &&
	    (player_verb != conv012_tell_knewhim)  &&
	    (player_verb != conv012_hello_one)     &&
	    (player_verb != conv012_seen_mask))    {
		conv_you_trigger(ROOM_103_JACQUES_TALK);
	}

	conv_me_trigger(ROOM_103_JACQUES_SHUT_UP);

	switch (kernel.trigger) {
	case ROOM_103_JACQUES_I_DUNNO_2:
		if (!quit_converse) {
			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		}
		local->jacques_action = CONV12_JAC_I_DUNNO_2;
		break;

	case ROOM_103_JACQUES_I_DUNNO_1:
		if (!quit_converse) {
			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		}
		local->jacques_action = CONV12_JAC_I_DUNNO_1;
		break;

	case ROOM_103_JACQUES_POINT:
		if (!quit_converse) {
			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		}
		local->jacques_action = CONV12_JAC_POINT;
		break;

	case ROOM_103_JACQUES_TALK:
		if (!quit_converse) {
			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		}
		local->jacques_action = CONV12_JAC_TALK;
		break;

	case ROOM_103_JACQUES_SHUT_UP:
		if (!quit_converse) {
			global[walker_converse] = imath_random(CONVERSE_HAND_WAVE, CONVERSE_HAND_WAVE_2);
		}
		local->converse_counter   = 0;
		local->jacques_action = CONV12_JAC_SHUT_UP;
		break;
	}

	local->jacques_talk_count = 0;
}

void room_103_pre_parser(void) {
	if (player_said_2(open, door)) {
		player.need_to_walk = true;
	}

	if (player_said_2(open, door_to_pit)) {
		player_walk(LEAVE_LEFT_X, LEAVE_LEFT_Y, FACING_WEST);
		player.walk_off_edge_to_room = 102;
	}

	/* ================ Climb down prompter's stand ============== */

	if ((local->top_of_stand != NO) && !player_said_2(climb_through, trap_door) &&
	    !player_said_2(open, trap_door) &&
	    !player_said_2(climb, prompter_s_stand) &&
	    !player_said_2(push, trap_door) &&
	    !player_said_2(look_through, prompter_s_box) &&
	    !player_said_2(pull, trap_door)) {

		if (player_said_1(pull) || player_said_1(push)) {
			if ((!player_said_1(lever)) && (!kernel.trigger)) {
				player.need_to_walk = false;
				/* so he does not walk down the staircase */
			}
		}

		if ((player_said_2(push, prompter_s_stand)) || (player_said_2(pull, prompter_s_stand))) {
			if (!kernel.trigger) {
				player.need_to_walk = true;
				/* so he will walk down the staircase */
			}
		}

		if (player.need_to_walk) {
			if (global[prompter_stand_status] == PROMPT_LEFT) {
				switch (kernel.trigger) {
				case 0:
					kernel_abort_animation(aa[5]);
					player.ready_to_walk    = false;
					player.walker_visible   = false;
					player.commands_allowed = false;
					local->anim_5_running   = false;
					local->anim_6_running   = true;
					aa[6]                   = kernel_run_animation(kernel_name('w', 4), 1);
					break;

				case 1:
					player.x                = PROMPT_UP_LEFT_X;
					player.y                = PROMPT_UP_LEFT_Y;
					player.commands_allowed = true;
					player.walker_visible   = true;
					local->top_of_stand     = NO;
					player.ready_to_walk    = true;
					if (player_said_2(push, prompter_s_stand)) {
						player.need_to_walk = false;
					}
					local->anim_6_running = false;
					player_demand_facing(FACING_NORTHEAST);
					kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[6]);
					break;
				}

			} else {
				switch (kernel.trigger) {
				case 0:
					kernel_abort_animation(aa[3]);
					player.ready_to_walk = false;
					if (player_said_2(push, prompter_s_stand)) {
						player.need_to_walk = true;
					}
					player.walker_visible   = false;
					player.commands_allowed = false;
					local->anim_3_running   = false;
					local->anim_4_running   = true;
					aa[4]                   = kernel_run_animation(kernel_name('w', 2), 1);
					break;

				case 1:
					player.x                = PROMPT_UP_RIGHT_X;
					player.y                = PROMPT_UP_RIGHT_Y;
					player.commands_allowed = true;
					player.ready_to_walk    = true;
					if (player_said_2(push, prompter_s_stand)) {
						player.need_to_walk = false;
					}
					player.walker_visible   = true;
					local->top_of_stand     = NO;
					local->anim_4_running   = false;
					player_demand_facing(FACING_NORTHEAST);
					kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[4]);
					break;
				}
			}
		}
	}

	if (local->top_of_stand == NO) {
		if ((player_said_2(push, prompter_s_stand)) || (player_said_2(pull, prompter_s_stand))) {
			if (global[current_year] == 1993) {
				if (global[prompter_stand_status] == PROMPT_LEFT) {
					player_walk(WALKER_LEFT_PROMPT_X, WALKER_LEFT_PROMPT_Y, FACING_WEST);
				} else {
					player_walk(WALKER_RIGHT_PROMPT_X, WALKER_RIGHT_PROMPT_Y, FACING_WEST);
				}
			}
		}
	}

	if ((player_said_2(look_through, prompter_s_box)) && (local->top_of_stand == NO) &&
	    (global[prompter_stand_status] == PROMPT_LEFT) && (global[current_year] == 1993)) {
		player_walk(PROMPT_UP_LEFT_X, PROMPT_UP_LEFT_Y, FACING_SOUTHWEST);
	}

	if ((player_said_2(climb_through, trap_door)) && (local->top_of_stand == NO) &&
	    (global[prompter_stand_status] == PROMPT_RIGHT) && (global[trap_door_status] == TRAP_DOOR_IS_OPEN)) {
		player_walk(PROMPT_UP_RIGHT_X, PROMPT_UP_RIGHT_Y, FACING_SOUTHWEST);
	}

	if (player_said_2(walk_through, door_to_pit)) {
		player.walk_off_edge_to_room = 102;
	}

	if ((player_said_2(open, door) || player_said_2(unlock, door) || player_said_2(lock, door)) && (local->top_of_stand == NO)) {
		player_walk(WALK_TO_X_FROM_105, WALK_TO_Y_FROM_105, FACING_EAST);
	}
}

void room_103_parser(void) {
	int temp;  /* for synching purposes */

	if (player_said_2(open, door_to_pit)) {
		new_room = 102;
		goto handled;
	}

	if (conv_control.running == CONV_JACQUES) {
		process_conv_jacques();
		goto handled;
	}

	if (player_said_2(climb_through, trap_door)) {
		if (global[trap_door_status] == TRAP_DOOR_IS_CLOSED) {
			text_show(text_103_33);
			goto handled;
			/* trap door is closed */
		} else if (global[prompter_stand_status] == PROMPT_LEFT) {
			text_show(text_103_41);
			goto handled;
			/* you can't reach the trap door when the stand is under prompters box */
		}
	}

	if ((player_said_2(look_through, prompter_s_box)) && (global[prompter_stand_status] == PROMPT_RIGHT)) {
		text_show(text_103_42);
		/* you can't reach the prompters box when the stand is under trap door */
		goto handled;
	}


	/* ================ Climb prompter's stand =================== */

	if ((player_said_2(climb, prompter_s_stand) && local->top_of_stand == NO) ||
	   ((player_said_2(look_through, prompter_s_box)) && (local->top_of_stand == NO)) ||
	   ((player_said_2(climb_through, trap_door)) && (local->top_of_stand == NO))) {
		if (global[prompter_stand_status] == PROMPT_LEFT) {
			switch (kernel.trigger) {
			case 0:
				if (player_said_2(look_through, prompter_s_box)) {
					local->sit_on_it = true;
					aa[5] = kernel_run_animation(kernel_name('w', 3), ROOM_103_LOOK_THRU_BOX);
				} else {
					aa[5] = kernel_run_animation(kernel_name('w', 3), 0);
				}
				player.walker_visible   = false;
				player.commands_allowed = false;
				local->anim_5_running   = true;
				kernel_synch(KERNEL_ANIM, aa[5], KERNEL_PLAYER, 0);
				goto handled;
				break;
			}

		} else {
			switch (kernel.trigger) {
			case 0:
				if (player_said_2(climb_through, trap_door)) {
					local->climb_thru = true;
					aa[3] = kernel_run_animation(kernel_name('w', 1), ROOM_103_CLIMB_OUT_TRAP);
				} else {
					aa[3] = kernel_run_animation(kernel_name('w', 1), 0);
				}
				player.walker_visible   = false;
				player.commands_allowed = false;
				local->anim_3_running   = true;
				kernel_synch(KERNEL_ANIM, aa[3], KERNEL_PLAYER, 0);
				goto handled;
				break;
			}
		}
	}

	if (player_said_2(climb, prompter_s_stand)) {
		if (local->top_of_stand != NO) {
			goto handled;
			/* so nothing is said */
		}
	}


	/* ================ Push prompter's stand ==================== */

	if ((player_said_2(push, prompter_s_stand)) || (player_said_2(pull, prompter_s_stand))) {
		if (global[current_year] == 1993) {
			if (global[prompter_stand_status] == PROMPT_LEFT) {
				switch (kernel.trigger) {
				case 0:
					if (global[prompter_stand_status] == PROMPT_LEFT) {
						player.commands_allowed = false;
						player.walker_visible   = false;
						aa[0]                   = kernel_run_animation(kernel_name('s', 1), ROOM_103_DONE_MOVING_PROMPT);
						player.clock            = kernel.clock;
						kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
						kernel_seq_delete(seq[fx_steps]);
					}
					break;

				case ROOM_103_DONE_MOVING_PROMPT:
					if (global[prompter_stand_status] == PROMPT_LEFT) {
						seq[fx_rail_pieces] = kernel_seq_stamp(ss[fx_rail_pieces], false, 1);
						kernel_seq_depth(seq[fx_rail_pieces], 1);
						seq[fx_steps] = kernel_seq_stamp(ss[fx_steps], false, 1);
						kernel_seq_depth(seq[fx_steps], 4);
						kernel_seq_loc(seq[fx_steps], PROMPT_RIGHT_X, PROMPT_RIGHT_Y);
						kernel_synch(KERNEL_SERIES, seq[fx_steps], KERNEL_ANIM, aa[0]);
						global[prompter_stand_status] = PROMPT_RIGHT;
						player.commands_allowed       = true;
						player.walker_visible         = true;
						player.x                      = AFTER_STEPS_FROM_LEFT_X;
						player.y                      = AFTER_STEPS_FROM_LEFT_Y;
						player_demand_facing(FACING_EAST);
						kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);

						kernel_delete_dynamic(local->prompt_1);
						kernel_delete_dynamic(local->prompt_2);
						kernel_delete_dynamic(local->prompt_3);
						kernel_delete_dynamic(local->prompt_5);
						kernel_delete_dynamic(local->floor_l_1);
						kernel_delete_dynamic(local->floor_l_2);
						local->prompt_1 = kernel_add_dynamic(words_prompter_s_stand, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
						                  DYN_PROMPT_RIGHT_1_X, DYN_PROMPT_RIGHT_1_Y,
						                  DYN_PROMPT_X_SIZE_1, DYN_PROMPT_Y_SIZE_1);
						kernel_dynamic_hot[local->prompt_1].prep = PREP_ON;
						kernel_dynamic_walk(local->prompt_1, DYN_PROMPT_R_WALK_TO_X, DYN_PROMPT_R_WALK_TO_Y, FACING_NORTHWEST);
						local->prompt_2 = kernel_add_dynamic(words_prompter_s_stand, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
						                  DYN_PROMPT_RIGHT_2_X, DYN_PROMPT_RIGHT_2_Y,
						                  DYN_PROMPT_X_SIZE_2, DYN_PROMPT_Y_SIZE_2);
						kernel_dynamic_hot[local->prompt_2].prep = PREP_ON;
						kernel_dynamic_walk(local->prompt_2, DYN_PROMPT_R_WALK_TO_X, DYN_PROMPT_R_WALK_TO_Y, FACING_NORTHWEST);
						local->prompt_3 = kernel_add_dynamic(words_prompter_s_stand, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
						                  DYN_PROMPT_RIGHT_3_X, DYN_PROMPT_RIGHT_3_Y,
						                  DYN_PROMPT_X_SIZE_3, DYN_PROMPT_Y_SIZE_3);
						kernel_dynamic_hot[local->prompt_3].prep = PREP_ON;
						kernel_dynamic_walk(local->prompt_3, DYN_PROMPT_R_WALK_TO_X, DYN_PROMPT_R_WALK_TO_Y, FACING_NORTHWEST);
						local->prompt_4 = kernel_add_dynamic(words_prompter_s_stand, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
						                  DYN_PROMPT_RIGHT_4_X, DYN_PROMPT_RIGHT_4_Y,
						                  DYN_PROMPT_X_SIZE_4, DYN_PROMPT_Y_SIZE_4);
						kernel_dynamic_hot[local->prompt_4].prep = PREP_ON;
						kernel_dynamic_walk(local->prompt_4, DYN_PROMPT_R_WALK_TO_X, DYN_PROMPT_R_WALK_TO_Y, FACING_NORTHWEST);
						local->prompt_5 = kernel_add_dynamic(words_prompter_s_stand, words_climb, SYNTAX_SINGULAR, KERNEL_NONE,
						                  DYN_PROMPT_RIGHT_5_X, DYN_PROMPT_LEFT_5_Y,
						                  DYN_PROMPT_X_SIZE_5, DYN_PROMPT_Y_SIZE_5);
						kernel_dynamic_hot[local->prompt_5].prep = PREP_ON;
						kernel_dynamic_walk(local->prompt_5, PROMPT_UP_RIGHT_X, PROMPT_UP_RIGHT_Y, FACING_SOUTHWEST);
						kernel_dynamic_cursor(local->prompt_5, CURSOR_UP);

						local->floor_r_1 = kernel_add_dynamic(words_floor, words_walk_across, SYNTAX_SINGULAR, KERNEL_NONE,
						                   DYN_FLOOR_R_1_X, DYN_FLOOR_R_1_Y,
						                   DYN_FLOOR_R_1_X_SIZE, DYN_FLOOR_R_1_Y_SIZE);
						kernel_dynamic_hot[local->floor_r_1].prep = PREP_ON;
						kernel_dynamic_walk(local->floor_r_1, DYN_FLOOR_R_1_WALK_TO_X, DYN_FLOOR_R_1_WALK_TO_Y, 5);
						local->floor_r_2 = kernel_add_dynamic(words_floor, words_walk_across, SYNTAX_SINGULAR, KERNEL_NONE,
						                   DYN_FLOOR_R_2_X, DYN_FLOOR_R_2_Y,
						                   DYN_FLOOR_R_2_X_SIZE, DYN_FLOOR_R_2_Y_SIZE);
						kernel_dynamic_hot[local->floor_r_2].prep = PREP_ON;
						kernel_dynamic_walk(local->floor_r_2, DYN_FLOOR_R_2_WALK_TO_X, DYN_FLOOR_R_2_WALK_TO_Y, 5);
						kernel_load_variant(1);
						room_103_adjust_rails(1);
					}
					break;
				}
			} else {
				switch (kernel.trigger) {
				case 0:
					if (global[prompter_stand_status] == PROMPT_RIGHT) {
						player.commands_allowed = false;
						player.walker_visible   = false;
						aa[0]                   = kernel_run_animation(kernel_name('s', 2), ROOM_103_DONE_MOVING_PROMPT);
						player.clock            = kernel.clock;
						kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
						kernel_seq_delete(seq[fx_rail_pieces]);
						kernel_seq_delete(seq[fx_steps]);
					}
					break;

				case ROOM_103_DONE_MOVING_PROMPT:
					if (global[prompter_stand_status] == PROMPT_RIGHT) {
						seq[fx_steps] = kernel_seq_stamp(ss[fx_steps], false, 1);
						kernel_seq_depth(seq[fx_steps], 4);
						kernel_seq_loc(seq[fx_steps], PROMPT_LEFT_X, PROMPT_LEFT_Y);
						kernel_synch(KERNEL_SERIES, seq[fx_steps], KERNEL_ANIM, aa[0]);
						global[prompter_stand_status] = PROMPT_LEFT;
						player.commands_allowed = true;
						player.walker_visible   = true;
						player.x                = AFTER_STEPS_FROM_RIGHT_X;
						player.y                = AFTER_STEPS_FROM_RIGHT_Y;
						player_demand_facing(FACING_WEST);
						kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
						kernel_delete_dynamic(local->prompt_1);
						kernel_delete_dynamic(local->prompt_2);
						kernel_delete_dynamic(local->prompt_3);
						kernel_delete_dynamic(local->prompt_4);
						kernel_delete_dynamic(local->prompt_5);
						kernel_delete_dynamic(local->floor_r_1);
						kernel_delete_dynamic(local->floor_r_2);
						local->prompt_1 = kernel_add_dynamic(words_prompter_s_stand, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
						                  DYN_PROMPT_LEFT_1_X, DYN_PROMPT_LEFT_1_Y,
						                  DYN_PROMPT_X_SIZE_1, DYN_PROMPT_Y_SIZE_1);
						kernel_dynamic_hot[local->prompt_1].prep = PREP_ON;
						kernel_dynamic_walk(local->prompt_1, DYN_PROMPT_L_WALK_TO_X, DYN_PROMPT_L_WALK_TO_Y, FACING_NORTHWEST);
						local->prompt_2 = kernel_add_dynamic(words_prompter_s_stand, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
						                  DYN_PROMPT_LEFT_2_X, DYN_PROMPT_LEFT_2_Y,
						                  DYN_PROMPT_X_SIZE_2, DYN_PROMPT_Y_SIZE_2);
						kernel_dynamic_hot[local->prompt_2].prep = PREP_ON;
						kernel_dynamic_walk(local->prompt_2, DYN_PROMPT_L_WALK_TO_X, DYN_PROMPT_L_WALK_TO_Y, FACING_NORTHWEST);
						local->prompt_3 = kernel_add_dynamic(words_prompter_s_stand, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
						                  DYN_PROMPT_LEFT_3_X, DYN_PROMPT_LEFT_3_Y,
						                  DYN_PROMPT_X_SIZE_3, DYN_PROMPT_Y_SIZE_3);
						kernel_dynamic_hot[local->prompt_3].prep = PREP_ON;
						kernel_dynamic_walk(local->prompt_3, DYN_PROMPT_L_WALK_TO_X, DYN_PROMPT_L_WALK_TO_Y, FACING_NORTHWEST);
						local->floor_l_1 = kernel_add_dynamic(words_floor, words_walk_across, SYNTAX_SINGULAR, KERNEL_NONE,
						                   DYN_FLOOR_L_1_X, DYN_FLOOR_L_1_Y,
						                   DYN_FLOOR_L_1_X_SIZE, DYN_FLOOR_L_1_Y_SIZE);
						kernel_dynamic_hot[local->floor_l_1].prep = PREP_ON;
						kernel_dynamic_walk(local->floor_l_1, DYN_FLOOR_L_1_WALK_TO_X, DYN_FLOOR_L_1_WALK_TO_Y, 5);
						local->floor_l_2 = kernel_add_dynamic(words_floor, words_walk_across, SYNTAX_SINGULAR, KERNEL_NONE,
						                   DYN_FLOOR_L_2_X, DYN_FLOOR_L_2_Y,
						                   DYN_FLOOR_L_2_X_SIZE, DYN_FLOOR_L_2_Y_SIZE);
						kernel_dynamic_hot[local->floor_l_2].prep = PREP_ON;
						kernel_dynamic_walk(local->floor_l_2, DYN_FLOOR_L_2_WALK_TO_X, DYN_FLOOR_L_2_WALK_TO_Y, 5);
						local->prompt_5 = kernel_add_dynamic(words_prompter_s_stand, words_climb, SYNTAX_SINGULAR, KERNEL_NONE,
						                   DYN_PROMPT_LEFT_5_X, DYN_PROMPT_LEFT_5_Y,
						                   DYN_PROMPT_X_SIZE_5, DYN_PROMPT_Y_SIZE_5);
						kernel_dynamic_hot[local->prompt_5].prep = PREP_ON;
						kernel_dynamic_walk(local->prompt_5, PROMPT_UP_LEFT_X, PROMPT_UP_LEFT_Y, FACING_SOUTHWEST);
						kernel_dynamic_cursor(local->prompt_5, CURSOR_UP);
						kernel_load_variant(0);
						room_103_adjust_rails(0);
					}
					break;
				}
			}
		} else {
			if (global[jacques_name_is_known]) {
				text_show(text_103_40);
			} else {
				text_show(text_103_50);
			}
			/* Jacques would fall over */
		}
		goto handled;
	}


	/* ================ Open door on right ======================= */

	if (player_said_2(walk_through, door) || player_said_2(open, door) ||
	    player_said_2(unlock, door) || player_said_2(lock, door)) {
		if ((global[current_year] == 1881) && !player_said_2(lock, door) && !player_said_2(unlock, door)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take_6] = kernel_seq_pingpong(ss[fx_take_6], false, 5, 0, 0, 2);
				kernel_seq_range(seq[fx_take_6], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_player(seq[fx_take_6], true);
				kernel_seq_trigger(seq[fx_take_6], KERNEL_TRIGGER_EXPIRE, 0, 2);
				kernel_seq_trigger(seq[fx_take_6], KERNEL_TRIGGER_SPRITE, 3, ROOM_103_DOOR_OPENS);
				break;

			case ROOM_103_DOOR_OPENS:
				sound_play(N_DoorOpens);
				kernel_seq_delete(seq[fx_door]);
				seq[fx_door] = kernel_seq_forward(ss[fx_door], false, 8, 0, 0, 1);
				kernel_seq_depth(seq[fx_door], 14);
				kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_103_DOOR_OPENS + 1);
				sound_play(N_SqueakyDoor);
				break;

			case ROOM_103_DOOR_OPENS + 1:
				temp = seq[fx_door];
				seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
				kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, temp);
				kernel_seq_depth(seq[fx_door], 14);
				break;

			case 2:
				player.walker_visible = true;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_6]);
				player_walk(WALK_TO_X_OPEN_DOOR, WALK_TO_Y_OPEN_DOOR, FACING_EAST);
				kernel_timing_trigger(TWO_SECONDS, 3);
				break;

			case 3:
				new_room = 105;
				break;
			}
		} else {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take_6] = kernel_seq_forward(ss[fx_take_6], false, 5, 0, 0, 1);
				kernel_seq_range(seq[fx_take_6], 1, 4);
				kernel_seq_player(seq[fx_take_6], true);
				kernel_seq_trigger(seq[fx_take_6], KERNEL_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				temp = seq[fx_take_6];
				seq[fx_take_6] = kernel_seq_stamp(ss[fx_take_6], false, 4);
				kernel_synch(KERNEL_SERIES, seq[fx_take_6], KERNEL_SERIES, temp);
				kernel_seq_player(seq[fx_take_6], false);
				kernel_timing_trigger(QUARTER_SECOND, 2);
				sound_play(N_DoorHandle);
				break; 

			case 2:
				kernel_seq_delete(seq[fx_take_6]);
				seq[fx_take_6] = kernel_seq_backward(ss[fx_take_6], false, 5, 0, 0, 1);
				kernel_seq_range(seq[fx_take_6], 1, 4);
				kernel_seq_player(seq[fx_take_6], false);
				kernel_seq_trigger(seq[fx_take_6], KERNEL_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_6]);
				player.walker_visible = true;
				if (player_said_1(lock) || player_said_1(unlock)) {
					text_show(text_000_32);
					/* the key does not work here */
				} else {
					text_show(text_103_35);
					/* the door is locked */
				}
				player.commands_allowed = true;
				break;
			}
		}
		goto handled;
	}


	/* ======================== Take key ========================= */

	if (player_said_2(take, key) &&
	    (object_is_here(key) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			seq[fx_bend_down_9] = kernel_seq_pingpong(ss[fx_bend_down_9], false, 5, 0, 0, 2);
			kernel_seq_range(seq[fx_bend_down_9], 1, 5);
			kernel_seq_player(seq[fx_bend_down_9], true);
			kernel_seq_trigger(seq[fx_bend_down_9], KERNEL_TRIGGER_SPRITE, 5, 1);
			kernel_seq_trigger(seq[fx_bend_down_9], KERNEL_TRIGGER_EXPIRE, 0, 2);
			goto handled;
			break;

		case 1:
			kernel_seq_delete(seq[fx_key]);
			kernel_flip_hotspot(words_key, false);
			inter_give_to_player(key);
			sound_play(N_TakeObjectSnd);
			goto handled;
			break;

		case 2:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_bend_down_9]);
			player.walker_visible = true;
			kernel_timing_trigger(20, 3);
			goto handled;
			break;

		case 3:
			global[player_score] += 5;
			object_examine(key, text_008_00, 0);
			/* you pick up the key */
			player.commands_allowed = true;
			goto handled;
			break;
		}
	}


	/* ============= Run conv if talk to Jacques ================= */

	if (player_said_2(talk_to, gentleman) ||
	    player_said_2(talk_to, Jacques)) {
		if (global[jacques_status] == JACQUES_IS_ALIVE) {
			conv_run(CONV_JACQUES);
			conv_export_pointer(&global[player_score]);
			conv_export_value(global[music_selected]);
		} else {
			text_show(text_103_43);
			/* you are talking to a dead man */
		}
		goto handled;
	}


	/* ================= Climb thru trap door ==================== */

	if (player_said_2(climb_through, trap_door) || local->climb_thru) {
		if ((local->top_of_stand == YES_RIGHT) && (global[trap_door_status] == TRAP_DOOR_IS_OPEN)) {
			switch (kernel.trigger) {
			case 0:
			case ROOM_103_CLIMB_OUT_TRAP:

				if (!(global[player_score_flags] & SCORE_TRAP_DOOR)) {
					global[player_score_flags] = global[player_score_flags] | SCORE_TRAP_DOOR;
					global[player_score] += 3;
				}

				kernel_abort_animation(aa[3]);
				local->anim_3_running   = false;
				local->climb_thru       = false;
				player.commands_allowed = false;
				seq[fx_up_trap_door]    = kernel_seq_forward(ss[fx_up_trap_door], false, 6, 0, 0, 1);
				kernel_seq_depth(seq[fx_up_trap_door], 5);
				kernel_seq_range(seq[fx_up_trap_door], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_up_trap_door], KERNEL_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				global[room_103_104_transition] = NEW_ROOM;
				new_room = 104;
				break;
			}
		}
		goto handled;
	}


	/* ============= Looking thru prompter's box ================= */

	if (player_said_2(look_through, prompter_s_box) || local->sit_on_it) {
		if (local->top_of_stand == YES_LEFT) {
			switch (kernel.trigger) {
			case 0:
			case ROOM_103_LOOK_THRU_BOX:
				kernel_abort_animation(aa[5]);
				local->anim_5_running   = false;
				player.commands_allowed = false;
				seq[fx_on_seat]         = kernel_seq_forward(ss[fx_on_seat], false, 6, 0, 0, 1);
				kernel_seq_depth(seq[fx_on_seat], 4);
				kernel_seq_range(seq[fx_on_seat], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_on_seat], KERNEL_TRIGGER_EXPIRE, 0, 1);
				if (!local->sit_on_it) {
					kernel_synch(KERNEL_SERIES, seq[fx_on_seat], KERNEL_ANIM, aa[5]);
				}
				local->sit_on_it = false;
				break;

			case 1:
				global[room_103_104_transition] = PEEK_THROUGH;
				new_room = 104;
				break;
			}
		}
		goto handled;
	}


	/* =========== Pull lever to open/close trap door ============ */

	if ((player_said_2(push, lever)) || (player_said_2(pull, lever))) {
		if (global[trap_door_status] == TRAP_DOOR_IS_CLOSED) {
			switch (kernel.trigger) {
			case 0:
				aa[1] = kernel_run_animation(kernel_name('l', 1), 1);
				local->anim_1_running   = true;
				player.walker_visible   = false;
				player.commands_allowed = false;
				kernel_synch(KERNEL_ANIM, aa[1], KERNEL_PLAYER, 0);
				break;

			case 1:
				local->anim_1_running    = false;
				player.walker_visible    = true;
				global[trap_door_status] = TRAP_DOOR_IS_OPEN;
				player.commands_allowed  = true;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
				break;
			}
		} else {
			switch (kernel.trigger) {
			case 0:
				aa[2] = kernel_run_animation(kernel_name('l', 2), 1);
				local->anim_2_running   = true;
				player.walker_visible   = false;
				player.commands_allowed = false;
				kernel_synch(KERNEL_ANIM, aa[2], KERNEL_PLAYER, 0);
				break;

			case 1:
				local->anim_2_running    = false;
				player.walker_visible    = true;
				global[trap_door_status] = TRAP_DOOR_IS_CLOSED;
				player.commands_allowed  = true;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[2]);
				break;
			}
		}
		goto handled;
	}


	/* ======================== Look around ====================== */

	if (player.look_around) {
		if ((global[current_year] == 1881) && (global[jacques_status] >= JACQUES_IS_DEAD)) {
			text_show(text_103_11);
		} else {
			text_show(text_103_10);
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(prompter_s_stand)) {
			if (global[jacques_status] >= JACQUES_IS_DEAD) {
				text_show(text_103_49);
			} else if (global[current_year] == 1993) {
				text_show(text_103_12);
			} else {
				text_show(text_103_45);
			}
			goto handled;
		}

		if (player_said_1(floor)) {
			text_show(text_103_13);
			goto handled;
		}

		if (player_said_1(trap_ceiling)) {
			text_show(text_103_14);
			goto handled;
		}

		if (player_said_1(door)) {
			text_show(text_103_15);
			goto handled;
		}

		if (player_said_1(door_to_pit)) {
			text_show(text_103_16);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_103_17);
			goto handled;
		}

		if (player_said_1(prompter_s_box)) {
			text_show(text_103_18);
			goto handled;
		}

		if (player_said_1(trap_door)) {
			text_show(text_103_19);
			goto handled;
		}

		if (player_said_1(junk)) {
			if (global[current_year] == 1993) {
				text_show(text_103_20);
			} else {
				text_show(text_103_46);
			}
			goto handled;
		}

		if (player_said_1(carton)) {
			if (global[current_year] == 1993) {
				text_show(text_103_21);
			} else {
				text_show(text_103_47);
			}
			goto handled;
		}

		if (player_said_1(garbage_can)) {
			if (global[current_year] == 1993) {
				text_show(text_103_22);
			} else {
				text_show(text_103_48);
			}
			goto handled;
		}

		if (player_said_1(cable)) {
			text_show(text_103_23);
			goto handled;
		}

		if (player_said_1(Jacques) || player_said_1(gentleman)) {
			if (global[jacques_status] == JACQUES_IS_ALIVE) {
				text_show(text_103_24);
			} else {
				text_show(text_103_25);
			}
			goto handled;
		}

		if (player_said_1(key)) {
			if (object_is_here(key)) {
				text_show(text_103_26);
				goto handled;
			}
		}

		if (player_said_1(stair_unit)) {
			text_show(text_103_27);
			goto handled;
		}

		if (player_said_1(exposed_brick)) {
			text_show(text_103_28);
			goto handled;
		}

		if (player_said_1(water_pipe)) {
			text_show(text_103_29);
			goto handled;
		}

		if (player_said_1(prompter_s_seat)) {
			text_show(text_103_38);
			goto handled;
		}

		if (player_said_1(lever)) {
			text_show(text_103_39);
			goto handled;
		}
	}

	if (player_said_2(close, door_to_pit)) {
		text_show(text_103_31);
		goto handled;
	}

	if (player_said_2(close, door)) {
		text_show(text_103_31);
		goto handled;
	}

	if ((player_said_2(open, trap_door)) || (player_said_2(close, trap_door))) {
		text_show(text_103_44);
		goto handled;
	}

	if (player_said_2(take, Jacques) || player_said_2(take, gentleman)) {
		if (global[jacques_status] == JACQUES_IS_ALIVE) {
			if (global[jacques_name_is_known]) {
				text_show(text_103_36);
			} else {
				text_show(text_103_51);
			}
		} else {
			text_show(text_103_37);
		}
		goto handled;
	}


	goto done;

handled:
	player.command_ready = false;

done:
	;
}


void room_103_preload(void) {
	room_init_code_pointer       = room_103_init;
	room_pre_parser_code_pointer = room_103_pre_parser;
	room_parser_code_pointer     = room_103_parser;
	room_daemon_code_pointer     = room_103_daemon;

	section_1_walker();
	section_1_interface();

	kernel_initial_variant = 0;

	if ((global[prompter_stand_status] == PROMPT_RIGHT) || (global[current_year] == 1881)) {
		kernel_initial_variant = 1;
		if ((global[jacques_status] == JACQUES_IS_ALIVE) && (global[current_year] == 1881)) {
			kernel_initial_variant = 2;
		} else if ((global[jacques_status] >= JACQUES_IS_DEAD) && (global[current_year] == 1881)) {
			kernel_initial_variant = 3;
		}
	}

	vocab_make_active(words_prompter_s_stand);
	vocab_make_active(words_Jacques);
	vocab_make_active(words_gentleman);
	vocab_make_active(words_climb);
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
