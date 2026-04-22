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
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/rooms/room108.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

static void handle_animation_char() {
	int random = 0;
	int char_reset_frame;
	int delay;

	if (kernel_anim[aa[0]].frame != local->char_frame) {
		local->char_frame = kernel_anim[aa[0]].frame;
		char_reset_frame  = -1;

		switch (local->char_frame) {
		case 1:   /* end of write 1            */
		case 2:   /* end of write 2            */
		case 3:   /* end of write 3            */
		case 4:   /* end of write 4            */
		case 92:  /* end of turning to write   */
			if (local->char_action == CONV2_CHAR_WRITE) {
				random = imath_random(2, 15);
			} /* random writing motions */

			if (local->char_action == CONV2_CHAR_TURN_TO_RAOUL) {
				local->char_talk_count = 0;
				local->char_action     = CONV2_CHAR_TALK;
				random = 1;
			} /* turn to face Raoul (new node) */

			switch (random) {
			case 1:  char_reset_frame = 5;  break; /* turn to face Raoul (new node) */
			case 2:  char_reset_frame = 0;  break; /* do write 1 */
			case 3:  char_reset_frame = 1;  break; /* do write 2 */
			case 4:  char_reset_frame = 2;  break; /* do write 3 */
			default: char_reset_frame = 3;  break; /* do write 5 */
			}
			break;

		case 18:  /* end of turn to face Raoul    */
		case 19:  /* end of talk 1                */
		case 20:  /* end of talk 2                */
		case 21:  /* end of talk 3                */
		case 73:  /* end of talk 4 (left hand up) */
		case 27:  /* end of point right           */
		case 33:  /* end of point up              */
		case 52:  /* end of shut up 1             */
		case 53:  /* end of shut up 2             */
		case 54:  /* end of shut up 3             */
		case 55:  /* end of shut up 4             */
		case 56:  /* end of shut up 5             */
		case 57:  /* end of shut up 6             */
		case 62:  /* end of shut up 7             */
		case 51:  /* end of oh my                 */
		case 41:  /* end of both hands up         */

			if (local->char_frame == 18) {
				player.commands_allowed = true;
				conv_run(CONV_CHARLES);
				conv_export_pointer(&global[player_score]);
				conv_export_value(global[julie_name_is_known]);
				conv_export_value(global[observed_phan_104]);
			}

			if (local->char_action == CONV2_CHAR_TALK) {
				if (local->did_raise_hand) {
					random = imath_random(1, 3); /* so we don't raise hand again */
				} else {
					random = imath_random(1, 4);
					if (random == 4) {
						random = imath_random(1, 4); /* decrease raise hand probability */
					}
				}

				if (random == 4) {
					local->did_raise_hand = true;
				}

				++local->char_talk_count;
				if (local->char_talk_count > local->max_talk_count) {
					local->char_action        = CONV2_CHAR_SHUT_UP;
					local->char_shut_up_count = 0;
					local->prev_shut_up_frame = 10;
					random                    = 12;
				}

			} else if (local->char_action == CONV2_CHAR_SHUT_UP) {
				delay = imath_random(10, 15); /* delay for shut up motions */
				++local->char_shut_up_count;
				if (local->char_shut_up_count > delay) {
					random = imath_random(10, 16);
					local->prev_shut_up_frame = random;
					if (random == 15) {
						local->char_shut_up_count = 16; /* make him turn right once */
					} else {
						local->char_shut_up_count = 0;  /* make him freeze x times  */
					}
				} else {
					random = local->prev_shut_up_frame;
				} /* random shut up motions */

			} else if (local->char_action == CONV2_CHAR_POINT_RIGHT) {
				local->char_action = CONV2_CHAR_TALK;
				random = 5;
				/* point right */

			} else if (local->char_action == CONV2_CHAR_OH_MY) {
				local->char_action = CONV2_CHAR_TALK;
				random = 6;
				/* do oh my! */

			} else if (local->char_action == CONV2_CHAR_BOTH_HANDS_UP) {
				local->char_action = CONV2_CHAR_TALK;
				random = 7;
				/* do both hands up */

			} else if (local->char_action == CONV2_CHAR_POINT_UP) {
				local->char_action = CONV2_CHAR_TALK;
				random = 8;
				/* point up */

			} else if (local->char_action == CONV2_CHAR_WRITE) {
				random = 9;
			} /* turn back to write */

			switch (random) {
			case 1:  char_reset_frame = 18;  break; /* do talk 1 */
			case 2:  char_reset_frame = 19;  break; /* do talk 2 */
			case 3:  char_reset_frame = 20;  break; /* do talk 3 */
			case 4:
				char_reset_frame = 62;
				local->char_talk_count += 5;
				break; /* do talk 4 (put left hand up) */
			case 5:  char_reset_frame = 21;  break; /* point right */
			case 6:  char_reset_frame = 41;  break; /* do oh my! */
			case 7:  char_reset_frame = 33;  break; /* do both hands up */
			case 8:  char_reset_frame = 27;  break; /* do point up */
			case 9:  char_reset_frame = 74;  break; /* turn back to write */
			case 10: char_reset_frame = 51;  break; /* do shut up 1 */
			case 11: char_reset_frame = 53;  break; /* do shut up 2 */
			case 12: char_reset_frame = 54;  break; /* do shut up 3 */
			case 13: char_reset_frame = 55;  break; /* do shut up 4 */
			case 14: char_reset_frame = 56;  break; /* do shut up 5 */
			case 15: char_reset_frame = 57;  break; /* do shut up 6 (look right) */
			case 16: char_reset_frame = 52;  break; /* do shut up 7 */
			}
			break;
		}

		if (char_reset_frame >= 0) {
			kernel_reset_animation(aa[0], char_reset_frame);
			local->char_frame = char_reset_frame;
		}
	}
}

void room_108_init() {
	int dynamic;  /* For adding dynamic hotspots */

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running     = false;
		local->did_raise_hand     = false;
		local->char_talk_count    = 0;
		local->char_shut_up_count = 40; /* so daemon will pick a new shut up motion */
		local->max_talk_count     = 15;
	}


	/* ==================== Load conversation ======================= */

	conv_get(CONV_CHARLES);


	/* ==================== Load Sprite Series ====================== */
	/* ==================== Put headset / change podium if in 1993 = */

	if (global[current_year] == 1993) {
		ss[fx_1993] = kernel_load_series(kernel_name('z', -1), false);
		kernel_flip_hotspot(words_stool, false);
		dynamic = kernel_add_dynamic(words_stool, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                             STOOL_X, STOOL_Y, STOOL_X_SIZE, STOOL_Y_SIZE);
		kernel_dynamic_hot[dynamic].prep = PREP_ON;
		kernel_dynamic_walk(dynamic, STOOL_WALK_X, STOOL_WALK_Y, FACING_SOUTHEAST);
		kernel_draw_to_background(ss[fx_1993], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
	} else {
		kernel_flip_hotspot(words_headset, false);
		dynamic = kernel_add_dynamic(words_wall, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                             WALL_X, WALL_Y, WALL_X_SIZE, WALL_Y_SIZE);
		kernel_dynamic_hot[dynamic].prep = PREP_ON;
		kernel_dynamic_walk(dynamic, WALL_WALK_X, WALL_WALK_Y, FACING_NORTHEAST);
	}


	/* ==================== Put Charles down ======================== */

	if ((global[current_year] == 1993) && (global[done_brie_conv_203] == NO)) {
		aa[0]                 = kernel_run_animation(kernel_name('c', 1), 1);
		local->anim_0_running = true;

		if (conv_restore_running == CONV_CHARLES) {
			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
			local->char_action      = CONV2_CHAR_SHUT_UP;
			conv_run(CONV_CHARLES);
			conv_export_pointer(&global[player_score]);
			conv_export_value(global[julie_name_is_known]);
			conv_export_value(global[observed_phan_104]);
			kernel_reset_animation(aa[0], 55); /* put somewhere in the shut up mode */
			local->char_shut_up_count = 40; /* so daemon will pick a new shut up motion */
		} else {
			local->char_action = CONV2_CHAR_WRITE;
		}

		if (global[charles_name_is_known]) {
			local->dynamic_char = kernel_add_dynamic(words_Charles, words_walk_to, SYNTAX_SINGULAR_MASC, KERNEL_NONE,
			                      DYN_CHAR_X, DYN_CHAR_Y, DYN_CHAR_XS, DYN_CHAR_YS);
			kernel_dynamic_hot[local->dynamic_char].prep = PREP_ON;
		} else {
			local->dynamic_char = kernel_add_dynamic(words_gentleman, words_walk_to, SYNTAX_MASC_NOT_PROPER, KERNEL_NONE,
			                      DYN_CHAR_X, DYN_CHAR_Y, DYN_CHAR_XS, DYN_CHAR_YS);
			kernel_dynamic_hot[local->dynamic_char].prep = PREP_ON;
		}
		kernel_dynamic_walk(local->dynamic_char, DYN_CHAR_WALK_TO_X, DYN_CHAR_WALK_TO_Y, FACING_NORTHEAST);
	}


	/* ==================== Previous Rooms ========================== */

	if (previous_room == 106) {
		player.x      = PLAYER_X_FROM_106;
		player.y      = PLAYER_Y_FROM_106;
		player.facing = FACING_SOUTHEAST;
		player_walk(WALK_TO_X_FROM_106, WALK_TO_Y_FROM_106, FACING_SOUTHEAST);

	} else if ((previous_room == 104) || (previous_room != KERNEL_RESTORING_GAME)) {

		if (player.y > 128) {
			player.x      = PLAYER_X_FROM_104_3;
			player.facing = FACING_NORTHEAST;

		} else if (player.y > 99) {
			player.x      = PLAYER_X_FROM_104_2;
			player.facing = FACING_NORTHEAST;

		} else {
			player.x      = PLAYER_X_FROM_104_1;
			player.facing = FACING_NORTHWEST;
		}

		player.y = PLAYER_Y_FROM_104;
	}

	section_1_music();
}

void room_108_daemon() {
	if (local->anim_0_running) {
		handle_animation_char();
	}

	if ((global[walker_converse] == CONVERSE_HAND_WAVE) ||
	    (global[walker_converse] == CONVERSE_HAND_WAVE_2)) {
		++local->converse_counter;
		if (local->converse_counter > 200) {
			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		}
	}
}

static void process_conversation_charles() {
	switch (player_verb) {
	case conv002_sayone_abc:
	case conv002_nomore_first:
	case conv002_saytwo_abc:
		global[walker_converse] = CONVERSE_NONE;
		conv_me_trigger(ROOM_108_CHAR_WRITE);
		if (player_verb == conv002_saytwo_abc) {
			global[charles_name_is_known] = YES_AND_END_CONV;
		}
		break;

	case conv002_answers_job:
		conv_you_trigger(ROOM_108_CHAR_BOTH_HANDS_UP);
		local->max_talk_count = 35;
		break;

	case conv002_interrogate_chandelier:
		conv_you_trigger(ROOM_108_CHAR_POINT_UP);
		local->max_talk_count = 35;
		break;

	case conv002_interrogate_giry:
		conv_you_trigger(ROOM_108_CHAR_POINT_RIGHT);
		break;

	case conv002_interrogate_phantom:
		conv_you_trigger(ROOM_108_CHAR_OH_MY);
		break;

	case conv002_answers_please:
	case conv002_answers_house:
	case conv002_answers_go_on:
	case conv002_answers_prison:
	case conv002_answers_building:
	case conv002_answers_catacombs:
	case conv002_interrogate_mishap:
		local->max_talk_count = 35;
		break;

	case conv002_questions_one:
		if (!global[charles_name_is_known]) {
			kernel_delete_dynamic(local->dynamic_char);
			local->dynamic_char = kernel_add_dynamic(words_Charles, words_walk_to, SYNTAX_SINGULAR_MASC, KERNEL_NONE,
			                      DYN_CHAR_X, DYN_CHAR_Y, DYN_CHAR_XS, DYN_CHAR_YS);
			kernel_dynamic_hot[local->dynamic_char].prep = PREP_ON;
			kernel_dynamic_walk(local->dynamic_char, DYN_CHAR_WALK_TO_X, DYN_CHAR_WALK_TO_Y, FACING_NORTHEAST);
			global[charles_name_is_known] = true;
		}
		break;

	default:
		local->max_talk_count = 15;
		break;
	}

	switch (kernel.trigger) {
	/* ==================== Process talking triggers ================ */
	case ROOM_108_CHAR_BOTH_HANDS_UP:
		local->char_action = CONV2_CHAR_BOTH_HANDS_UP;
		break;

	case ROOM_108_CHAR_POINT_UP:
		local->char_action = CONV2_CHAR_POINT_UP;
		break;

	case ROOM_108_CHAR_POINT_RIGHT:
		local->char_action = CONV2_CHAR_POINT_RIGHT;
		break;

	case ROOM_108_CHAR_OH_MY:
		local->char_action = CONV2_CHAR_OH_MY;
		break;

	case ROOM_108_CHAR_WRITE:
		local->char_action = CONV2_CHAR_WRITE;
		break;

	case ROOM_108_CHAR_TALK:
		local->char_action      = CONV2_CHAR_TALK;
		global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		break;

	case ROOM_108_CHAR_SHUT_UP:
		global[walker_converse] = imath_random(CONVERSE_HAND_WAVE, CONVERSE_HAND_WAVE_2);
		local->converse_counter = 0;
		local->char_action      = CONV2_CHAR_SHUT_UP;
		break;
	}

	/* ==================== Set up me and you triggers ============== */

	if ((player_verb != conv002_nomore_first) &&
	    (player_verb != conv002_saytwo_abc) &&
	    (player_verb != conv002_sayone_abc) &&
	    (player_verb != conv002_interrogate_chandelier) &&
	    (player_verb != conv002_interrogate_giry) &&
	    (player_verb != conv002_interrogate_phantom) &&
	    (player_verb != conv002_answers_job) &&
	    (local->char_action != CONV2_CHAR_WRITE) &&
	    (player_verb != conv002_sayone_abc)) {
		conv_you_trigger(ROOM_108_CHAR_TALK);
		conv_me_trigger(ROOM_108_CHAR_SHUT_UP);
	}

	local->char_talk_count    = 0;
	local->char_shut_up_count = 40; /* so in daemon it will pick a new shut up movement */
	local->did_raise_hand     = false;
}

void room_108_parser() {
	if (conv_control.running == CONV_CHARLES) {
		process_conversation_charles();
		goto handled;
	}

	if ((player_said_2(talk_to, gentleman)) ||
	    (player_said_2(talk_to, Charles))) {
		local->char_action      = CONV2_CHAR_TURN_TO_RAOUL;
		player.commands_allowed = false;
		goto handled;
	}

	if (player_said_2(walk_onto, stage)) {
		new_room = 104;
		goto handled;
	}

	if (player_said_2(walk, backstage)) {
		new_room = 106;
		goto handled;
	}

	if (player.look_around) {
		text_show(text_108_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(wall)) {
			text_show(text_107_30);  /* shared look-at-wall response */
			goto handled;
		}

		if (player_said_1(stage)) {
			text_show(text_108_11);
			goto handled;
		}

		if (player_said_1(_in_two_)) {
			text_show(text_108_12);
			goto handled;
		}

		if (player_said_1(_in_one_)) {
			text_show(text_108_13);
			goto handled;
		}

		if (player_said_1(proscenium_arch)) {
			text_show(text_108_14);
			goto handled;
		}

		if (player_said_1(act_curtain)) {
			text_show(text_108_15);
			goto handled;
		}

		if (player_said_1(leg)) {
			text_show(text_108_16);
			goto handled;
		}

		if (player_said_1(cyclorama)) {
			text_show(text_108_17);
			goto handled;
		}

		if (player_said_1(flats)) {
			text_show(text_108_18);
			goto handled;
		}

		if (player_said_1(stagemanager_s_post)) {
			if (global[current_year] == 1993) {
				text_show(text_108_19);
			} else {
				text_show(text_108_20);
			}
			goto handled;
		}

		if (player_said_1(stool)) {
			text_show(text_108_21);
			goto handled;
		}

		if (player_said_1(backstage)) {
			text_show(text_108_22);
			goto handled;
		}

		if (player_said_1(stage)) {  /* Note: second 'stage' check — unreachable in original */
			text_show(text_108_23);
			goto handled;
		}

		if (player_said_1(headset)) {
			text_show(text_108_24);
			goto handled;
		}

		if (player_said_1(wall)) {  /* Note: second 'wall' check — unreachable in original */
			text_show(text_108_26);
			goto handled;
		}

		if (player_said_1(Charles) || player_said_1(gentleman)) {
			text_show(text_108_27);
			goto handled;
		}
	}

	if (player_said_2(take, Charles) || player_said_2(take, gentleman)) {
		text_show(text_108_28);
		goto handled;
	}

	if (player_said_2(take, headset)) {
		text_show(text_108_25);
		goto handled;
	}

	if (player_said_2(take, stool)) {
		if (global[current_year] == 1993) {
			text_show(text_108_29);
			goto handled;
		}
	}

	if (player_said_2(talk_to, headset)) {
		text_show(text_108_30);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_108_preload() {
	room_init_code_pointer       = room_108_init;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer     = room_108_parser;
	room_daemon_code_pointer     = room_108_daemon;

	section_1_walker();
	section_1_interface();

	if (global[current_year] == 1993) {
		kernel_initial_variant = 1;
	}

	vocab_make_active(words_gentleman);
	vocab_make_active(words_Charles);
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
