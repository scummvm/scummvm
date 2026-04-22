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
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/rooms/room101.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/core/inter.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_101_init(void) {
	kernel.disable_fastwalk = true;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->execute_chan = RESET;
		local->execute_wipe = RESET;
		local->start_walking = false;
		local->start_walking_0 = false;
		local->anim_0_running = false;
		local->anim_0_running = true;

		/* ======== When false, camera hasn't panned past 208 ======== */
		local->start_sitting_down = false;
	}


	/* =================== Load conversations ==================== */

	conv_get(CONV_BRIE_MOTIONS_TO_RAOUL);
	conv_get(CONV_BRIE_BY_CHANDELIER);


	/* =================== Load sprite series ==================== */
	/* =========== If in 1993, put crashed chandelier ============ */

	if (global[current_year] == 1993) {
		ss[fx_chandelier] = kernel_load_series(kernel_name('z', -1), false);
		seq[fx_chandelier] = kernel_seq_stamp(ss[fx_chandelier], false, 1);
		kernel_seq_depth(seq[fx_chandelier], 14);
	} else {
		kernel_flip_hotspot(words_chandelier, false);
	}


	/* ================ Run anims & set up dynamic =============== */

	if (global[brie_talk_status] == BEFORE_BRIE_MOTIONS) {
		player_first_walk(OFF_SCREEN_X_FROM_202, OFF_SCREEN_Y_FROM_202, FACING_EAST,
			PLAYER_X_FROM_202, PLAYER_Y_FROM_202, FACING_EAST, true);

		local->brie_calling_position = CONV0_BRIE_ARMS_AT_SIDE;

		local->brie_chandelier_position = CONV1_BRIE_SHOULD_FREEZE_STAND;

		player_walk_trigger(ROOM_101_START_CONV_MOTIONS);

		aa[1] = kernel_run_animation(kernel_name('b', 9), 1);
		aa[0] = kernel_run_animation(kernel_name('b', 8), 1);

		local->anim_0_running = true;
		local->anim_1_running = true;

		local->dynamic_brie = kernel_add_dynamic(words_Monsieur_Brie, words_walk_to, SYNTAX_SINGULAR_MASC, KERNEL_NONE,
			0, 0, 0, 0);
		kernel_dynamic_hot[local->dynamic_brie].prep = PREP_ON;
		kernel_dynamic_walk(local->dynamic_brie, DYNAMIC_BRIE_WALK_TO_X, DYNAMIC_BRIE_WALK_TO_Y, 5);
		kernel_dynamic_anim(local->dynamic_brie, aa[0], 0);
		kernel_dynamic_anim(local->dynamic_brie, aa[0], 1);
		kernel_dynamic_anim(local->dynamic_brie, aa[0], 2);
		kernel_dynamic_anim(local->dynamic_brie, aa[0], 3);
		kernel_dynamic_anim(local->dynamic_brie, aa[0], 4);

		local->dynamic_brie_2 = kernel_add_dynamic(words_Monsieur_Brie, words_walk_to, SYNTAX_SINGULAR_MASC, KERNEL_NONE,
			0, 0, 0, 0);
		kernel_dynamic_hot[local->dynamic_brie_2].prep = PREP_ON;
		kernel_dynamic_walk(local->dynamic_brie_2, DYN_BRIE_1_WALK_TO_X, DYN_BRIE_1_WALK_TO_Y, 5);
		kernel_dynamic_anim(local->dynamic_brie_2, aa[1], 1);
		kernel_dynamic_anim(local->dynamic_brie_2, aa[1], 2);

		local->talk_count = 0;

	} else if (global[brie_talk_status] == BEFORE_CHANDELIER_CONV) {
		aa[1] = kernel_run_animation(kernel_name('b', 9), 1);
		local->dynamic_brie = kernel_add_dynamic(words_Monsieur_Brie, words_walk_to, SYNTAX_SINGULAR_MASC, KERNEL_NONE,
			0, 0, 0, 0);
		kernel_dynamic_hot[local->dynamic_brie].prep = PREP_ON;
		kernel_dynamic_anim(local->dynamic_brie, aa[1], 1);
		kernel_dynamic_anim(local->dynamic_brie, aa[1], 2);
		local->anim_1_running = true;
		local->talk_count = 0;
		local->brie_chandelier_position = CONV1_BRIE_SHOULD_FREEZE_STAND;

		/* ============= If restoring into conv, run it ============== */

		if (conv_restore_running == CONV_BRIE_BY_CHANDELIER) {
			conv_run(CONV_BRIE_BY_CHANDELIER);
			conv_export_pointer(&global[player_score]);
			local->brie_chandelier_position = CONV1_BRIE_SHOULD_FREEZE_SIT;
			kernel_reset_animation(aa[1], 25); /* the sitting position */
		}


	} else {

		/* ======================= Previous Rooms ==================== */

		if (previous_room == 202) {
			if (global[jacques_status] == JACQUES_IS_DEAD) {
				global[jacques_status] = JACQUES_IS_DEAD_RICH_GONE;
			}
			player_first_walk(OFF_SCREEN_X_FROM_202, OFF_SCREEN_Y_FROM_202, FACING_EAST,
				PLAYER_X_FROM_202, PLAYER_Y_FROM_202, FACING_EAST, true);

		} else if ((previous_room == 102) || (previous_room != KERNEL_RESTORING_GAME)) {
			player_first_walk(OFF_SCREEN_X_FROM_102, OFF_SCREEN_Y_FROM_102, FACING_WEST,
				PLAYER_X_FROM_102, PLAYER_Y_FROM_102, FACING_WEST, true);
			camera_jump_to(RIGHT_HALF, 0);
		}
	}

	section_1_music();
}


static void handle_animation_000(void) {
	int random;
	int reset_frame;

	if (kernel_anim[aa[0]].frame != local->brie_calling_frame) {
		local->brie_calling_frame = kernel_anim[aa[0]].frame;
		reset_frame = -1;
		switch (local->brie_calling_frame) {

			/* Choices while Brie has his hand up */

		case 1:   /* end of arms at side */
		case 9:   /* end of TALK_1 */
		case 12:  /* end of TALK_2 */

			if (local->brie_calling_position == CONV0_BRIE_RAISING_ARMS) {
				if (local->brie_calling_frame == 9) {
					if (local->start_walking_0) {
						reset_frame = 13;  /* start CALLING_TURN_AND_WALK */
						local->brie_calling_position = CONV0_BRIE_SHOULD_WALK;
					} else {
						local->brie_calling_position = CONV0_BRIE_SHOULD_TALK;
					}
				} else {
					reset_frame = 1;
				}
			}

			if (local->brie_calling_position == CONV0_BRIE_ARMS_AT_SIDE) {
				if (local->start_walking_0) {
					reset_frame = 60;  /* start to turn around */
					local->brie_calling_position = CONV0_BRIE_SHOULD_WALK;
				} else {
					reset_frame = 0;
				}
			}

			if (local->brie_calling_position == CONV0_BRIE_SHOULD_TALK) {

				if (local->start_walking_0) {
					reset_frame = 13;  /* start CALLING_TURN_AND_WALK */
					local->brie_calling_position = CONV0_BRIE_SHOULD_WALK;

				} else {
					random = imath_random(1, 2); /* between TALK_1 and TALK_2 */
					++local->talk_count; /* run through so many talk cycles  */
					/* before putting arms down to side */
					if (local->talk_count < 18) {
						if (random == 1) {
							reset_frame = 7;   /* start TALK_1 */
						} else {
							reset_frame = 10;    /* start TALK_2 */
						}
					} else {
						reset_frame = 54;      /* put arms down at side */
						local->brie_calling_position = CONV0_BRIE_ARMS_AT_SIDE;
					}
				}
			}
			break;

		case 53:  /* end of CALLING_TURN_AND_WALK */
			local->anim_0_running = false;
			kernel_abort_animation(aa[0]);
			break;

		case 59:
			if (local->start_walking_0) {
				reset_frame = 60;      /* start to turn */
				local->brie_calling_position = CONV0_BRIE_SHOULD_WALK;
			} else {
				reset_frame = 0;  /* keep Brie standing still */
				local->brie_calling_position = CONV0_BRIE_ARMS_AT_SIDE;
			}
			break;

		case 66:
			reset_frame = 24;    /* make Brie walk off of screen */
			break;
		}

		if (reset_frame >= 0) {
			kernel_reset_animation(aa[0], reset_frame);
			local->brie_calling_frame = reset_frame;
		}
	}
}



static void handle_animation_001(void) {
	int random;
	int reset_frame;

	if (kernel_anim[aa[1]].frame != local->brie_chandelier_frame) {
		local->brie_chandelier_frame = kernel_anim[aa[1]].frame;
		reset_frame = -1;
		switch (local->brie_chandelier_frame) {

			/* Choices while Brie is sitting on armrest */

		case 1:     /* end of freezing in standing position */
			if (local->brie_chandelier_position == CONV1_BRIE_SHOULD_FREEZE_STAND) {
				reset_frame = 0;  /* stick BRIE in the standing still position */
			}
			break;

		case 11:    /* end of sitting down on armrest */
		case 14:    /* end of TALK_1 */
		case 17:    /* end of TALK_2 */
		case 19:    /* end of TALK_3 */
		case 26:    /* end of raising hands and sitting still frame */
		case 44:    /* end of motioning to chandelier */
		case 333:   /* end of wiping forehead */

			if (local->talk_count == local->execute_chan) {
				local->brie_chandelier_position = CONV1_BRIE_SHOULD_MOTION_CHAN;
				++local->talk_count;
				local->execute_chan = RESET;
			}

			if (local->talk_count == local->execute_wipe) {
				local->brie_chandelier_position = CONV1_BRIE_SHOULD_WIPE;
				++local->talk_count;
				local->execute_wipe = RESET;
			}

			if (local->start_walking) {
				if (conv_control.running == CONV_BRIE_BY_CHANDELIER) {
					if (local->talk_count > 13) {
						local->brie_chandelier_position = CONV1_BRIE_SHOULD_WALK;
					}
				} else {
					local->brie_chandelier_position = CONV1_BRIE_SHOULD_WALK;
				}
			}

			switch (local->brie_chandelier_position) {

			case CONV1_BRIE_SHOULD_TALK:
				random = imath_random(1, 3); /* between 4 talking movements */
				++local->talk_count;         /* run through so many talk cycles  */
				/* before putting arms down to side */
				if (local->talk_count < 15) {
					if (random == 1) {
						reset_frame = 12;    /* start TALK_1 */
					} else if (random == 2) {
						reset_frame = 14;    /* start TALK_2 */
					} else if (random == 3) {
						reset_frame = 17;    /* start TALK_3 */
					}
				} else {
					local->brie_chandelier_position = CONV1_BRIE_SHOULD_FREEZE_SIT;
					reset_frame = 25;
				}
				break;

			case CONV1_BRIE_SHOULD_WALK:
				global[brie_talk_status] = AFTER_CONVS_0_AND_1;
				reset_frame = 45;  /* start walking away */
				if (conv_control.running == CONV_BRIE_BY_CHANDELIER) {
					conv_abort(); /* so the player.commands_allowed = false          */
					/* that is to follow doesn't screw up conversation */
				}
				kernel_delete_dynamic(local->dynamic_brie);
				player.commands_allowed = false;
				break;

			case CONV1_BRIE_SHOULD_MOTION_CHAN:
				reset_frame = 27;  /* start to motion to chandelier */
				local->brie_chandelier_position = CONV1_BRIE_SHOULD_TALK;
				break;

			case CONV1_BRIE_SHOULD_WIPE:
				reset_frame = 316;  /* start to motion to wipe forehead */
				local->brie_chandelier_position = CONV1_BRIE_SHOULD_TALK;
				break;

			case CONV1_BRIE_SHOULD_RAISE_HANDS:
				reset_frame = 21;  /* raise hands like 'I dunno' or  */
				/* more like 'are you stupid?'    */
				local->brie_chandelier_position = CONV1_BRIE_SHOULD_TALK;
				break;

			case CONV1_BRIE_SHOULD_FREEZE_SIT:
				reset_frame = 25;  /* stick BRIE in the sitting still position */
				break;
			}
			break;

		case 315:  /* end of CALLING_TURN_AND_WALK */
			/* global[brie_talk_status] = AFTER_CONVS_0_AND_1; */
			kernel_abort_animation(aa[1]);
			break;
		}

		if (reset_frame >= 0) {
			kernel_reset_animation(aa[1], reset_frame);
			local->brie_chandelier_frame = reset_frame;
		}
	}
}




void room_101_daemon(void) {
	/* =============== Goto conversation functions =============== */

	if (local->anim_0_running) {
		handle_animation_000();
	}

	if ((global[walker_converse] == CONVERSE_HAND_WAVE) ||
		(global[walker_converse] == CONVERSE_HAND_WAVE_2)) {
		++local->converse_counter;
		if (local->converse_counter > 200) {
			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		}
	}

	if (local->anim_1_running) {
		handle_animation_001();
		/* ====== Turn on commands if Brie has walked past Raoul ===== */

		if (kernel_anim[aa[1]].frame == 80) {
			player.commands_allowed = true;
			player_walk_trigger(ROOM_101_START_CONV_CHAN);
		}
	}


	/* ========== Check camera - to make Brie sit down =========== */

	if ((picture_view_x > 200) && (local->start_sitting_down == false) &&
		(global[brie_talk_status] != AFTER_CONVS_0_AND_1)) {
		local->start_sitting_down = true;
		player_walk(DYNAMIC_BRIE_WALK_TO_X, DYNAMIC_BRIE_WALK_TO_Y, FACING_NORTHEAST);
		player.commands_allowed = false;
		player_walk_trigger(ROOM_101_START_CONV_CHAN);
		local->brie_chandelier_position = CONV1_BRIE_SHOULD_FREEZE_SIT;
	}


	/* ============ Once I get to Brie, start conv 1 ============= */

	if (kernel.trigger == ROOM_101_START_CONV_CHAN) {
		player.commands_allowed = true;
		conv_run(CONV_BRIE_BY_CHANDELIER);
		conv_export_pointer(&global[player_score]);
		local->brie_chandelier_frame = RESET;
		local->talk_count = 0;
	}


	/* ====== When Raoul walks from off-screen, start conv ======= */

	if (kernel.trigger == ROOM_101_START_CONV_MOTIONS) {
		conv_run(CONV_BRIE_MOTIONS_TO_RAOUL);
		local->brie_calling_position = CONV0_BRIE_RAISING_ARMS;
	}
}

static void process_conversation_0(void) {
	conv_me_trigger(ROOM_101_CONV0_MAKE_BRIE_WALK);

	if (kernel.trigger == ROOM_101_CONV0_MAKE_BRIE_WALK) {
		global[brie_talk_status] = BEFORE_CHANDELIER_CONV;
		local->start_walking_0 = true;
	}
}



static void process_conversation_1(void) {
	int you_trig_flag = false;

	if ((player_verb >= conv001_speech_talk) &&
		(player_verb <= conv001_point_two_abc)) {

		if (kernel.trigger == ROOM_101_BRIE_START_TALKING) {
			switch (player_verb) {
			case conv001_speech_talk:
				local->brie_chandelier_position = CONV1_BRIE_SHOULD_WIPE;
				local->execute_wipe = 2;
				break;

			case conv001_continue_abc:
				local->brie_chandelier_position = CONV1_BRIE_SHOULD_TALK;
				local->execute_chan = 9;  /* on talk_count #9, motion to chandelier */
				break;

			case conv001_what_one:
				local->brie_chandelier_position = CONV1_BRIE_SHOULD_MOTION_CHAN;
				local->execute_chan = RESET;
				local->execute_wipe = RESET;
				break;

			case conv001_everything_copycat:
				local->brie_chandelier_position = CONV1_BRIE_SHOULD_RAISE_HANDS;
				local->execute_chan = RESET;
				local->execute_wipe = RESET;
				break;

			case conv001_yesno_yes:
			case conv001_everything_byebye:
			case conv001_speak_byebye:
			case conv001_saytwo_1:
			case conv001_instructions_three:
				local->start_walking = true;
				local->execute_chan = RESET;
				local->execute_wipe = RESET;
				global[walker_converse] = CONVERSE_NONE;
				conv_you_trigger(ROOM_101_STOP_CONVERSING);
				you_trig_flag = true;
				break;

			default:
				break;
			}
		}

		if (!you_trig_flag) {
			conv_you_trigger(ROOM_101_BRIE_START_TALKING);
		}

		conv_me_trigger(ROOM_101_BRIE_STOP_TALKING);

		local->talk_count = 0;

		if (kernel.trigger == ROOM_101_BRIE_START_TALKING) {
			if (!local->start_walking) {
				global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
			}
			local->brie_chandelier_position = CONV1_BRIE_SHOULD_TALK;
		} else if (kernel.trigger == ROOM_101_BRIE_STOP_TALKING) {
			if (local->start_walking == false) {
				local->brie_chandelier_position = CONV1_BRIE_SHOULD_FREEZE_SIT;
				local->execute_chan = RESET;
				local->execute_wipe = RESET;
				if (!local->start_walking) {
					global[walker_converse] = imath_random(CONVERSE_HAND_WAVE, CONVERSE_HAND_WAVE_2);
				}
				local->converse_counter = 0;
			}
		}
	}
}



void room_101_pre_parser(void) {
	if (player_said_2(exit_to, orchestra_pit)) {
		if ((global[brie_talk_status] == AFTER_CONVS_0_AND_1) || (local->start_walking)) {
			player.walk_off_edge_to_room = 102;
			global[brie_talk_status] = AFTER_CONVS_0_AND_1;
		} else {
			conv_run(CONV_BRIE_MOTIONS_TO_RAOUL);
			player.need_to_walk = false;
		}
	}

	if (player_said_2(exit_to, grand_foyer)) {
		if ((global[brie_talk_status] == AFTER_CONVS_0_AND_1) || (local->start_walking)) {
			player.walk_off_edge_to_room = 202;
		} else {
			conv_run(CONV_BRIE_MOTIONS_TO_RAOUL);
			player.need_to_walk = false;
		}
	}

	if (player_said_2(take, Monsieur_Brie)) {
		text_show(text_101_21);
	}

	if (player_said_2(talk_to, Monsieur_Brie)) {
		if (global[brie_talk_status] == AFTER_CONVS_0_AND_1) {
			player.need_to_walk = false;
		}
	}
}


void room_101_parser(void) {
	if (conv_control.running == CONV_BRIE_MOTIONS_TO_RAOUL) {
		process_conversation_0();
		goto handled;
	}

	if (conv_control.running == CONV_BRIE_BY_CHANDELIER) {
		process_conversation_1();
		goto handled;
	}

	if (player.look_around) {
		if (global[current_year] == 1993) {
			text_show(text_101_10);
		} else {
			text_show(text_101_11);
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(aisle)) {
			text_show(text_101_12);
			goto handled;
		}

		if (player_said_1(chandelier)) {
			text_show(text_101_13);
			goto handled;
		}

		if (player_said_1(back_wall)) {
			text_show(text_101_14);
			goto handled;
		}

		if (player_said_1(side_wall)) {
			text_show(text_101_15);
			goto handled;
		}

		if (player_said_1(seats)) {
			if (global[current_year] == 1881) {
				text_show(text_101_16);
			} else {
				if ((global[brie_talk_status] > BEFORE_CHANDELIER_CONV) ||
					(local->start_walking)) {
					text_show(text_101_19);
				} else {
					text_show(text_101_16);
				}
			}
			goto handled;
		}

		if (player_said_1(grand_foyer)) {
			text_show(text_101_17);
			goto handled;
		}

		if (player_said_1(orchestra_pit)) {
			text_show(text_101_18);
			goto handled;
		}

		if (player_said_1(Monsieur_Brie)) {
			text_show(text_101_20);
			goto handled;
		}
	}


	if (player_said_2(talk_to, Monsieur_Brie)) {
		if (global[brie_talk_status] == AFTER_CONVS_0_AND_1) {
			text_show(text_101_22);
		}
		goto handled;
	}

	if (player_said_2(take, Monsieur_Brie)) {
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_101_preload(void) {
	room_init_code_pointer = room_101_init;
	room_pre_parser_code_pointer = room_101_pre_parser;
	room_parser_code_pointer = room_101_parser;
	room_daemon_code_pointer = room_101_daemon;

	section_1_walker();
	section_1_interface();

	vocab_make_active(words_Monsieur_Brie);
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
