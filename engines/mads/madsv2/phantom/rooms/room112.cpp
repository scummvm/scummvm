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
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/rooms/room112.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

static void handle_animation_julie_practice() {
	int random = 0;
	int julie_reset_frame;

	if (kernel_anim[aa[0]].frame != local->julie_frame) {
		local->julie_frame = kernel_anim[aa[0]].frame;
		julie_reset_frame  = -1;

		switch (local->julie_frame) {
		case 56: /* end of putting right arm up             */
		case 77: /* end of a tilting head back              */
		case 83: /* end of putting right arm down - then up */
			random = imath_random(1, 3);
			while (local->just_did_option == random) {
				random = imath_random(1, 3);
			} /* so we don't repeat the same option twice */
			local->just_did_option = random;

			switch (local->julie_action) {
			case CONV3_JULIE_TALK:
			case CONV3_JULIE_I_DUNNO:
				random = 3;
				break;
			}

			switch (random) {
			case 1:
				julie_reset_frame = 71;
				break; /* do a head tilt */

			case 2:
				julie_reset_frame = 77;
				break; /* put right arm down - then up */

			case 3:
				julie_reset_frame = 56;
				break; /* put right arm down */
			}
			break;

		case 1:    /* end of talk 1 */
		case 2:    /* end of talk 2 */
		case 3:    /* end of talk 3 */
		case 22:   /* end of init movements (just entered room) */
		case 39:   /* end of dipping                      */
		case 71:   /* end of putting right arm down       */
		case 107:  /* end of turning left (facing camera) */

			switch (local->julie_action) {

			case CONV3_JULIE_PRACTICE:
				random = imath_random(1, 3);
				while (local->just_did_option == random) {
					random = imath_random(1, 3);
				}
				local->just_did_option = random;
				break;

			case CONV3_JULIE_I_DUNNO:
				random = 3;
				break;

			case CONV3_JULIE_TALK:
				random = imath_random(4, 6);
				++local->julie_talk_count;
				/* between the 3 talk movements */
				if (local->julie_talk_count > 25) {
					local->julie_action = CONV3_JULIE_PRACTICE;
				}
				break;
			}

			switch (random) {
			case 1:
				julie_reset_frame = 22;
				break; /* do a dip */

			case 2:
				julie_reset_frame = 39;
				break; /* put right arm up */

			case 3:
				julie_reset_frame = 83;
				break; /* turn right */

			case 4:
				julie_reset_frame = 0;
				break; /* talk 1 */

			case 5:
				julie_reset_frame = 1;
				break; /* talk 2 */

			case 6:
				julie_reset_frame = 2;
				break; /* talk 3 */
			}
			break;

		case 95:   /* end of turning right */

			switch (local->julie_action) {
			case CONV3_JULIE_I_DUNNO:
				random = 2;
				break;

			default:
				random = 1;
				break;
			}

			switch (random) {
			case 1:
				julie_reset_frame = 95;
				break; /* turn left (to face camera) */

			case 2:
				julie_reset_frame = 107;
				break; /* give an "I don't know" type of an action */
				       /* used during player_verb = knowghost_see  */
			}
			break;

		case 130:  /* end of an "I don't know" */
			julie_reset_frame = 95;
			local->julie_action = CONV3_JULIE_PRACTICE;
			break; /* turn left (to face camera) */
		}

		if (julie_reset_frame >= 0) {
			kernel_reset_animation(aa[0], julie_reset_frame);
			local->julie_frame = julie_reset_frame;
		}
	}
}

static void handle_animation_raoul_chair() {
	int random;
	int raoul_reset_frame;

	if (kernel_anim[aa[1]].frame != local->raoul_frame) {
		local->raoul_frame = kernel_anim[aa[1]].frame;
		raoul_reset_frame  = -1;

		switch (local->raoul_frame) {
		case 18: /* end of freeze & sit down */
		case 30: /* end of left hand point   */
		case 41: /* end of left hand up      */
		case 49: /* end of both hands up     */
		case 56: /* end of right hand point  */
		case 65: /* end of touch hand        */

			switch (local->raoul_action) {
			case CONV3_RAOUL_INVISIBLE:
				random = 6; /* turn invisible */
				break;

			case CONV3_RAOUL_GET_UP:
				random = 5; /* get up */
				local->raoul_action = CONV3_RAOUL_INVISIBLE;
				break;

			case CONV3_RAOUL_TALK:
				random = imath_random(1, 4);
				local->raoul_action = CONV3_RAOUL_SHUT_UP;
				break;

			default:
				random = imath_random(7, 50);
				while (local->just_did_option == random) {
					random = imath_random(7, 50);
				} /* so we don't repeat the same option twice */
				local->just_did_option = random;
				break;
			}

			switch (random) {
			case 1:
				raoul_reset_frame = 49;
				break; /* do a right hand point (talk 1) */

			case 2:
				raoul_reset_frame = 41;
				break; /* put both hands up (talk 2) */

			case 3:
				raoul_reset_frame = 18;
				break; /* do a left hand point (talk 3) */

			case 4:
				raoul_reset_frame = 30;
				break; /* put left hand up (talk 4) */

			case 5:
				raoul_reset_frame = 65;
				break; /* get up */

			case 6:
				raoul_reset_frame = 82;
				break; /* invisible */

			case 7:
				raoul_reset_frame = 56;
				break; /* touch head (new node) */

			default:
				raoul_reset_frame = 17;
				break; /* freeze */
			}
			break;

		case 61:  /* hand on chin */

			switch (local->raoul_action) {
			case CONV3_RAOUL_SHUT_UP:
				random = 1;
				break;

			default:
				random = 2;
				break;
			}

			switch (random) {
			case 1:
				raoul_reset_frame = 60;
				break; /* keep hand on chin */

			case 2:
				raoul_reset_frame = 61;
				break; /* put hand down */
			}
			break;

		case 83:  /* end of invisible */

			switch (local->raoul_action) {
			case CONV3_RAOUL_INVISIBLE:
				random = 1;
				break;

			default:
				random = 2;
				break;
			}

			switch (random) {
			case 1:
				raoul_reset_frame = 82;
				break; /* keep him invisible */

			case 2:
				raoul_reset_frame = 0;
				break; /* make him sit down */
			}
			break;

		case 82: /* player now has walker back */
			local->raoul_action   = CONV3_RAOUL_INVISIBLE;
			player.walker_visible = true;
			conv_release();
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
			break;
		}

		if (raoul_reset_frame >= 0) {
			kernel_reset_animation(aa[1], raoul_reset_frame);
			local->raoul_frame = raoul_reset_frame;
		}
	}
}

void room_112_init() {
	if (previous_room != KERNEL_RESTORING_GAME) {
		local->raoul_action    = CONV3_RAOUL_INVISIBLE;
		local->just_did_option = 0;
		local->display_wait    = false;
	}

	/* =================== Load conversation ======================== */

	conv_get(CONV_JULIE_PRACTICE);


	/* ============== Run animation of julie practicing ============= */

	aa[0]                 = kernel_run_animation(kernel_name('j', 1), 1);
	local->anim_0_running = true;
	local->julie_action   = CONV3_JULIE_PRACTICE;
	kernel_reset_animation(aa[0], 3); /* start Julie at frame 3 */

	if (global[julie_name_is_known] == JULIE_YES) {
		local->dynamic_julie = kernel_add_dynamic(words_Julie, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
		                                          DYNAMIC_JULIE_X, DYNAMIC_JULIE_Y,
		                                          DYNAMIC_JULIE_X_SIZE, DYNAMIC_JULIE_Y_SIZE);
		kernel_dynamic_hot[local->dynamic_julie].prep = PREP_ON;
		kernel_dynamic_walk(local->dynamic_julie, DYNAMIC_JULIE_WALK_TO_X, DYNAMIC_JULIE_WALK_TO_Y, FACING_NORTHEAST);
		kernel_flip_hotspot(words_woman, false);
	}
	aa[1]                 = kernel_run_animation(kernel_name('r', 1), 1);
	local->anim_1_running = true;
	kernel_reset_animation(aa[1], 82); /* start Raoul at frame 82 (invisible) */
	local->raoul_action   = CONV3_RAOUL_INVISIBLE;


	/* ===== If restoring into conv, run it & sit raoul down ======== */
	/* ====================== Previous Rooms ======================== */

	if (conv_restore_running == CONV_JULIE_PRACTICE) {
		conv_run(CONV_JULIE_PRACTICE);
		conv_export_pointer(&global[player_score]);
		kernel_reset_animation(aa[1], 17); /* start Raoul at just sitting there */
		local->raoul_action   = CONV3_RAOUL_SHUT_UP;
		player.x              = CHAIR_X;
		player.y              = CHAIR_Y;
		player.facing         = FACING_EAST;
		player.walker_visible = false;

	} else if ((previous_room == 110) || (previous_room != KERNEL_RESTORING_GAME)) {
		player.x      = PLAYER_X_FROM_110;
		player.y      = PLAYER_Y_FROM_110;
		player.facing = FACING_NORTH;
	}

	section_1_music();
}

void room_112_daemon() {
	if (local->anim_0_running) {
		handle_animation_julie_practice();
	}

	if (local->anim_1_running) {
		handle_animation_raoul_chair();
	}
}

static void process_conv_julie_practice() {
	switch (player_verb) {
	case conv003_return_byebye:
	case conv003_yourself_byebye:
	case conv003_questions_three:
	case conv003_nomore_byebye:
	case conv003_adios_byebye:
		conv_you_trigger(ROOM_112_GET_UP); /* you */
		global[julie_name_is_known] = JULIE_YES;
		break;

	case conv003_name_oops:
		conv_you_trigger(ROOM_112_GET_UP);
		global[julie_name_is_known] = NO_AND_QUIT_CONV;
		break;

	case conv003_first_howdy:
		if (!kernel.trigger) {
			if (local->raoul_action == CONV3_RAOUL_INVISIBLE) {
				conv_you_trigger(ROOM_112_BEFORE_CHAIR);
			}
		}
		break;

	case conv003_knowghost_look:
	case conv003_return_dreams:
	case conv003_return_she:
		local->julie_action = CONV3_JULIE_TALK;
		local->raoul_action = CONV3_RAOUL_SHUT_UP;
		break;

	case conv003_name_i_am:
	case conv003_name_form:
		if (global[julie_name_is_known] < JULIE_YES) {
			global[julie_name_is_known] = JULIE_YES;
			kernel_flip_hotspot(words_woman, false);
			local->dynamic_julie = kernel_add_dynamic(words_Julie, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
			                                          DYNAMIC_JULIE_X, DYNAMIC_JULIE_Y,
			                                          DYNAMIC_JULIE_X_SIZE, DYNAMIC_JULIE_Y_SIZE);
			kernel_dynamic_hot[local->dynamic_julie].prep = PREP_ON;
			kernel_dynamic_walk(local->dynamic_julie, DYNAMIC_JULIE_WALK_TO_X, DYNAMIC_JULIE_WALK_TO_Y, FACING_NORTHEAST);
		}
		break;

	case conv003_knowghost_see:
		local->julie_action = CONV3_JULIE_I_DUNNO;
		local->raoul_action = CONV3_RAOUL_SHUT_UP;
		break;
	}

	switch (kernel.trigger) {
	case ROOM_112_GET_UP:
		local->raoul_action = CONV3_RAOUL_GET_UP;
		conv_hold();
		break;

	case ROOM_112_BEFORE_CHAIR:
		kernel_timing_trigger(TWO_SECONDS, ROOM_112_BETWEEN_CHAIR);
		conv_hold(); /* hold so options don't appear in interface */
		break;

	case ROOM_112_BETWEEN_CHAIR:
		player_walk(CHAIR_X, CHAIR_Y, FACING_EAST);
		player_walk_trigger(ROOM_112_AT_CHAIR);
		break; /* give a 3 sec. delay before walking to chair */

	case ROOM_112_AT_CHAIR:
		conv_release();
		player.walker_visible = false;
		kernel_synch(KERNEL_ANIM, aa[1], KERNEL_PLAYER, 0);
		kernel_reset_animation(aa[1], 1);
		local->raoul_action = CONV3_RAOUL_SHUT_UP;
		/* when made it to chair, run frame 1 of sitting him down */
		/* and make options appear in interface */
		break;

	case ROOM_112_RAOUL_TALK:
		local->julie_action = CONV3_JULIE_PRACTICE;
		if (local->raoul_action != CONV3_RAOUL_INVISIBLE) {
			local->raoul_action = CONV3_RAOUL_TALK;
		}
		break;

	case ROOM_112_JULIE_TALK:
		local->julie_action = CONV3_JULIE_TALK;
		local->raoul_action = CONV3_RAOUL_SHUT_UP;
		break;
	}

	if ((player_verb != conv003_first_howdy) &&
	    (player_verb != conv003_bon_jour_hello) &&
	    (player_verb != conv003_name_oops) &&
	    (player_verb != conv003_return_byebye) &&
	    (player_verb != conv003_yourself_byebye) &&
	    (player_verb != conv003_questions_three) &&
	    (player_verb != conv003_nomore_byebye) &&
	    (player_verb != conv003_second_aloha) &&
	    (player_verb != conv003_adios_byebye)) {
		conv_you_trigger(ROOM_112_JULIE_TALK);
		conv_me_trigger(ROOM_112_RAOUL_TALK);
	}

	if (player_verb == conv003_return_giry) {
		global[can_find_book_library] = true;
	}

	local->julie_talk_count = 0;
	local->raoul_talk_count = 0;
}

void room_112_pre_parser() {
	/* if we want to talk to Julie and she has already told us to sit down
	   in the past, then automatically make Raoul walk to chair */
	if ((player_said_2(talk_to, Julie)) ||
	    (player_said_2(talk_to, woman))) {
		if (global[julie_name_is_known] > JULIE_NO) {
			player_walk(CHAIR_X, CHAIR_Y, FACING_EAST);
		}
	}
}

void room_112_parser() {
	if (conv_control.running == CONV_JULIE_PRACTICE) {
		process_conv_julie_practice();
		goto handled;
	}

	if ((player_said_2(talk_to, Julie)) ||
	    (player_said_2(talk_to, woman))) {
		if (global[julie_name_is_known] > JULIE_NO) {
			/* if we want to talk to Julie and she has already told us to sit down
			   in the past, then automatically make Raoul sit down in chair */
			player.walker_visible = false;
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_PLAYER, 0);
			kernel_reset_animation(aa[1], 1);
			local->raoul_action = CONV3_RAOUL_SHUT_UP;
		}
		conv_run(CONV_JULIE_PRACTICE);
		conv_export_pointer(&global[player_score]);
		goto handled;
	}

	if (player_said_2(exit_to, corridor)) {
		new_room = 110;
		goto handled;
	}

	if (player.look_around) {
		text_show(text_112_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(wall)) {
			text_show(text_112_11);
			goto handled;
		}

		if (player_said_1(floor)) {
			text_show(text_112_12);
			goto handled;
		}

		if (player_said_1(mirror)) {
			text_show(text_112_13);
			goto handled;
		}

		if (player_said_1(ballet_bar)) {
			text_show(text_112_14);
			goto handled;
		}

		if (player_said_1(corridor)) {
			text_show(text_112_15);
			goto handled;
		}

		if (player_said_1(throw_rugs)) {
			text_show(text_112_16);
			goto handled;
		}

		if (player_said_1(dressing_screen)) {
			text_show(text_112_17);
			goto handled;
		}

		if (player_said_1(dressing_table)) {
			text_show(text_112_18);
			goto handled;
		}

		if (player_said_1(chair)) {
			text_show(text_112_19);
			goto handled;
		}

		if (player_said_1(plant)) {
			text_show(text_112_20);
			goto handled;
		}

		if (player_said_1(coat_rack)) {
			text_show(text_112_21);
			goto handled;
		}

		if (player_said_1(umbrella)) {
			text_show(text_112_22);
			goto handled;
		}

		if (player_said_1(paintings)) {
			text_show(text_112_23);
			goto handled;
		}

		if (player_said_1(trash_bucket)) {
			text_show(text_112_24);
			goto handled;
		}

		if (player_said_1(shelf)) {
			text_show(text_112_25);
			goto handled;
		}

		if (player_said_1(container)) {
			text_show(text_112_26);
			goto handled;
		}

		if (player_said_1(torn_poster)) {
			text_show(text_112_27);
			goto handled;
		}

		if (player_said_1(poster)) {
			text_show(text_112_28);
			goto handled;
		}

		if ((player_said_1(review)) || (player_said_1(reviews))) {
			text_show(text_112_29);
			goto handled;
		}

		if (player_said_1(Julie)) {
			text_show(text_112_31);
			goto handled;
		}

		if (player_said_1(costume_rack)) {
			text_show(text_112_32);
			goto handled;
		}

		if (player_said_1(light_fixture)) {
			text_show(text_112_33);
			goto handled;
		}

		if (player_said_1(wardrobe)) {
			text_show(text_112_34);
			goto handled;
		}

		if (player_said_1(woman)) {
			text_show(text_112_37);
			goto handled;
		}
	}

	if (player_said_1(take)) {
		if (player_said_1(woman) || player_said_1(Julie)) {
			text_show(text_112_38);
			goto handled;
		}
	}

	if (player_said_2(open, wardrobe)) {
		text_show(text_112_35);
		goto handled;
	}

	if (player_said_2(open, container)) {
		text_show(text_112_36);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}


void room_112_preload() {
	room_init_code_pointer       = room_112_init;
	room_pre_parser_code_pointer = room_112_pre_parser;
	room_parser_code_pointer     = room_112_parser;
	room_daemon_code_pointer     = room_112_daemon;

	section_1_walker();
	section_1_interface();

	vocab_make_active(words_woman);
	vocab_make_active(words_Julie);
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
