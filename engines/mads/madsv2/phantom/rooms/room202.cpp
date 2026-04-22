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
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/phantom/rooms/section2.h"
#include "mads/madsv2/phantom/rooms/room202.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void set_chandelier_positions() {
	int chandelier;
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int y;
	int xs;

	center = picture_view_x + (video_x >> 1);

	for (chandelier = 0; chandelier < NUM_CHANDELIERS; chandelier++) {

		if (seq[chandelier + fx_chandelier_0] >= 0) {
			kernel_seq_delete(seq[chandelier + fx_chandelier_0]);
		}

		difference = center - local->chandelier_base[chandelier];
		direction  = neg(sgn(difference));
		distance   = abs(difference);

		displace   = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
		displace   = sgn_in(displace, direction);

		x  = local->chandelier_base[chandelier] + displace - 1;
		y  = series_list[ss[fx_chandelier_0]]->index[0].ys - 1;
		xs = series_list[ss[fx_chandelier_0]]->index[0].xs;

		if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
		    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
			seq[chandelier + fx_chandelier_0] = -1;
		} else {
			if (local->dyn_chandeliers[chandelier] != -1) {
				kernel_delete_dynamic(local->dyn_chandeliers[chandelier]);
			}

			local->dyn_chandeliers[chandelier] = kernel_add_dynamic(words_chandelier, words_look_at, SYNTAX_SINGULAR, KERNEL_NONE,
			        x - 8, y - 12, 16, 13);

			seq[chandelier + fx_chandelier_0] =
			    kernel_seq_stamp(ss[fx_chandelier_0], false, 1);
			kernel_seq_loc(seq[chandelier + fx_chandelier_0], x, y);
			kernel_seq_depth(seq[chandelier + fx_chandelier_0], 1);
		}
	}
}

static void handle_animation_usher() {
	int random = 0;
	int usher_reset_frame;

	if (kernel_anim[aa[0]].frame != local->usher_frame) {
		local->usher_frame = kernel_anim[aa[0]].frame;
		usher_reset_frame  = -1;

		switch (local->usher_frame) {
		case 1:   /* end of freeze             */
		case 35:  /* end of point              */
		case 51:  /* end of put left arm up    */
		case 52:  /* end of talk 1             */
		case 53:  /* end of talk 2             */
		case 54:  /* end of talk 3             */
		case 13:  /* end of do you have ticket */

			switch (local->usher_action) {
			case CONV17_USHER_TALK:
				random = imath_random(1, 3);
				++local->usher_talk_count;
				if (local->usher_talk_count > 15) {
					if (player_verb == conv017_ticket_block) {
						local->usher_action = CONV17_USHER_HAVE_TICKET;
						random = 5;
					} else {
						local->usher_action = CONV17_USHER_SHUT_UP;
						random = 7;
					}
				}
				break;

			case CONV17_USHER_POINT:
				random = 4;
				break;

			case CONV17_USHER_HAVE_TICKET:
				random = 5;
				break;

			case CONV17_USHER_LEFT_UP:
				random = 6;
				break;

			default:
				random = 7;
				break;
			}

			switch (random) {
			case 1:
				usher_reset_frame = 51;
				break; /* do talk 1 */

			case 2:
				usher_reset_frame = 52;
				break; /* do talk 2 */

			case 3:
				usher_reset_frame = 53;
				break; /* do talk 3 */

			case 4:
				usher_reset_frame   = 21;
				local->usher_action = CONV17_USHER_POINT;
				break; /* point (new node) */

			case 5:
				usher_reset_frame = 1;
				break; /* do you have ticket (new node) */

			case 6:
				usher_reset_frame   = 35;
				local->usher_action = CONV17_USHER_TALK;
				break; /* put left arm up (to block the entrance) */

			default:
				usher_reset_frame = 0;
				break; /* freeze sitting still */
			}
			break;

		case 7:   /* holding hand out */
			switch (local->usher_action) {
			case CONV17_USHER_HAVE_TICKET:
				random = 1;
				break;

			default:
				random = 2;
				break;
			}

			switch (random) {
			case 1:
				usher_reset_frame = 6;
				break; /* keep hand out */

			default:
				usher_reset_frame = 7;
				break; /* go to stand still (new node) */
			}
			break;

		case 28:  /* pointing */
			switch (local->usher_action) {
			case CONV17_USHER_POINT:
				random = 1;
				++local->usher_talk_count;
				if (local->usher_talk_count > 15) {
					local->usher_action = CONV17_USHER_SHUT_UP;
					random = 2;
				}
				break;

			default:
				random = 2;
				break;
			}

			switch (random) {
			case 1:
				usher_reset_frame = 27;
				break; /* keep pointing */

			default:
				usher_reset_frame = 28;
				break; /* go to stand still (new node) */
			}
			break;
		}

		if (usher_reset_frame >= 0) {
			kernel_reset_animation(aa[0], usher_reset_frame);
			local->usher_frame = usher_reset_frame;
		}
	}
}

static void handle_animation_degas() {
	int random = 0;
	int degas_reset_frame;

	if (kernel_anim[aa[1]].frame != local->degas_frame) {
		local->degas_frame = kernel_anim[aa[1]].frame;
		degas_reset_frame  = -1;

		switch (local->degas_frame) {
		case 110:
			conv_release();
			break;

		case 1:   /* end of freeze             */
		case 17:  /* end of talk 1             */
		case 58:  /* end of put bag down       */
			switch (local->degas_action) {
			case CONV9_DEGAS_TALK:
				local->degas_action = CONV9_DEGAS_SHUT_UP;
				random = 1;
				break;

			case CONV9_DEGAS_BOW:
			case CONV9_DEGAS_GIRL:
			case CONV9_DEGAS_LEAVE:
				random = 2;
				break; /* pull bag up under arm (new node) */

			default:
				random = imath_random(3, 50);
				break;
			}

			switch (random) {
			case 1:
				degas_reset_frame = 1;
				break; /* do talk */

			case 2:
				degas_reset_frame = 58;
				break; /* pull bag up under arm (new node) */

			case 3:
				degas_reset_frame = 58;
				break; /* pull bag up under arm (new node) while shutting up */

			default:
				degas_reset_frame = 0;
				break; /* freeze standing still */
			}
			break;

		case 62:  /* end of pulling bag up             */
		case 55:  /* end of talking about girls        */
		case 40:  /* end of generic talk and freeze    */
		case 33:  /* end of bow                        */
			switch (local->degas_action) {
			case CONV9_DEGAS_TALK:
				local->degas_action = CONV9_DEGAS_SHUT_UP;
				random = 1;
				break;

			case CONV9_DEGAS_BOW:
				local->degas_action = CONV9_DEGAS_SHUT_UP;
				random = 2;
				break;

			case CONV9_DEGAS_GIRL:
				local->degas_action = CONV9_DEGAS_SHUT_UP;
				random = 3;
				break;

			case CONV9_DEGAS_LEAVE:
				random = 4;
				break;

			default:
				random = imath_random(5, 50);
				break;
			}

			switch (random) {
			case 1:
				degas_reset_frame = 33;
				break; /* generic talk */

			case 2:
				degas_reset_frame = 17;
				break; /* do a bow */

			case 3:
				degas_reset_frame = 42;
				break; /* put hand to mouth (talk about girl) */

			case 4:
				degas_reset_frame = 62;
				break; /* leave */

			case 5:
				degas_reset_frame = 41;
				break; /* look left (new node) */

			case 6:
				degas_reset_frame = 55;
				break; /* put bag by waist (new node) */

			default:
				degas_reset_frame = 39;
				break; /* freeze while bag is up under arm */
			}
			break;

		case 42:  /* looking left */
			switch (local->degas_action) {
			case CONV9_DEGAS_BOW:
			case CONV9_DEGAS_GIRL:
			case CONV9_DEGAS_LEAVE:
			case CONV9_DEGAS_TALK:
				random = 1;
				break;

			default:
				random = imath_random(1, 50);
				break;
			}

			switch (random) {
			case 1:
				degas_reset_frame = 39;
				break; /* look straight ahead (new node) */

			default:
				degas_reset_frame = 41;
				break; /* keep looking left */
			}
			break;
		}

		if (degas_reset_frame >= 0) {
			kernel_reset_animation(aa[1], degas_reset_frame);
			local->degas_frame = degas_reset_frame;
		}
	}
}

void room_202_init() {
	int count;

	kernel.disable_fastwalk = true;

	local->gave               = false;
	local->chandelier_base[0] = 77;
	local->chandelier_base[1] = 192;
	local->chandelier_base[2] = 319;
	local->chandelier_base[3] = 445;
	local->chandelier_base[4] = 560;

	if (global[ticket_people_here] == USHER_AND_SELLER) {
		global[make_rich_leave_203] = true;
	}

	if ((global[degas_name_is_known]) || (global[current_year] == 1993)) {
		kernel_flip_hotspot(words_gentleman, false);
	}

	for (count = 0; count < NUM_CHANDELIERS; count++) {
		seq[fx_chandelier_0 + count]    = -1;
		local->dyn_chandeliers[count]   = -1;
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->prevent        = false;
	}

	local->converse_counter = 0;


	/* =================== Load conversations ======================== */

	conv_get(CONV_USHER_17);
	conv_get(CONV_DEGAS_9);


	/* =================== Load Sprite Series ======================= */

	ss[fx_take_9]       = kernel_load_series("*RDR_9", false);
	ss[fx_left_door]    = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_chandelier_0] = kernel_load_series(kernel_name('f', 0), false);


	if (global[ticket_people_here] == USHER_AND_SELLER) {
		aa[0]                 = kernel_run_animation(kernel_name('b', 0), 0);
		local->anim_0_running = true;
		local->usher_action   = CONV17_USHER_SHUT_UP;

	} else {
		kernel_flip_hotspot(words_usher, false);
	}

	if (global[degas_name_is_known]) {
		local->anim_1_running = false;
	}

	if ((global[current_year] == 1881) && (!global[degas_name_is_known])) {
		aa[1]                 = kernel_run_animation(kernel_name('d', 1), ROOM_202_DEGAS_DONE);
		local->anim_1_running = true;
		local->degas_action   = CONV9_DEGAS_SHUT_UP;
	}


	if (conv_restore_running == CONV_USHER_17) {
		conv_run(CONV_USHER_17);
		conv_export_value(player_has(ticket));
		conv_export_value(0);
		player.x               = USHER_X;
		player.y               = USHER_Y;
		player.facing          = FACING_NORTHEAST;
		global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
	}

	if (conv_restore_running == CONV_DEGAS_9) {
		conv_run(CONV_DEGAS_9);
		conv_export_pointer(&global[player_score]);
		player.x               = DEGAS_X;
		player.y               = DEGAS_Y;
		player.facing          = FACING_NORTHWEST;
		global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
	}


	/* ========================= Previous Rooms ===================== */

	if (previous_room == 201) {
		player.x          = PLAYER_X_FROM_201;
		player.y          = PLAYER_Y_FROM_201;
		player_walk(WALK_TO_X_FROM_201, WALK_TO_Y_FROM_201, FACING_EAST);
		seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 1);
		kernel_seq_depth(seq[fx_left_door], 14);

	} else if (previous_room == 203) {
		player.x                = WALK_TO_X_BEHIND_LEFT_DOOR;
		player.y                = WALK_TO_Y_BEHIND_LEFT_DOOR;
		player.facing           = FACING_SOUTH;
		player.commands_allowed = false;
		player_walk(FRONT_OF_LEFT_DOOR_X, FRONT_OF_LEFT_DOOR_Y, FACING_SOUTH);
		player_walk_trigger(ROOM_202_DOOR_CLOSES);
		seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 5);
		kernel_seq_depth(seq[fx_left_door], 14);

	} else if (previous_room == 204) {
		player.x          = PLAYER_X_FROM_204;
		player.y          = PLAYER_Y_FROM_204;
		player_walk(WALK_TO_X_FROM_204, WALK_TO_Y_FROM_204, FACING_SOUTH);
		seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 1);
		kernel_seq_depth(seq[fx_left_door], 14);
		camera_jump_to(RIGHT_HALF - 250, 0);

	} else if (previous_room == 205) {
		player.x          = PLAYER_X_FROM_205;
		player.y          = PLAYER_Y_FROM_205;
		player_walk(WALK_TO_X_FROM_205, WALK_TO_Y_FROM_205, FACING_SOUTH);
		camera_jump_to(RIGHT_HALF, 0);
		seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 1);
		kernel_seq_depth(seq[fx_left_door], 14);

	} else if ((previous_room == 101) || (previous_room != KERNEL_RESTORING_GAME)) {
		player.x          = PLAYER_X_FROM_101;
		player.y          = PLAYER_Y_FROM_101;
		player_walk(WALK_TO_X_FROM_101, WALK_TO_Y_FROM_101, FACING_WEST);
		camera_jump_to(RIGHT_HALF, 0);
		seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 1);
		kernel_seq_depth(seq[fx_left_door], 14);

	} else if (previous_room == KERNEL_RESTORING_GAME) {
		seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_left_door], 14);
	}

	set_chandelier_positions();
	section_2_music();
}

void room_202_daemon() {
	if (camera_x.pan_this_frame) {
		set_chandelier_positions();
	}

	if (local->anim_0_running) {
		handle_animation_usher();
	}

	if (kernel.trigger == ROOM_202_DEGAS_DONE) {
		local->anim_1_running = false;
	}

	if (local->anim_1_running) {
		handle_animation_degas();
	}

	if ((global[current_year] == 1881) && (!global[degas_name_is_known]) &&
	    (player.x < 405) && (!local->prevent)) {
		player_walk(DEGAS_X, DEGAS_Y, FACING_NORTHWEST);
		player_walk_trigger(ROOM_202_TALK_TO_DEGAS);
		player.commands_allowed = false;
		local->prevent          = true;
	}

	if (kernel.trigger == ROOM_202_TALK_TO_DEGAS) {
		player.commands_allowed = true;
		conv_run(CONV_DEGAS_9);
		conv_export_pointer(&global[player_score]);
		global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
	}

	switch (kernel.trigger) {
	case ROOM_202_DOOR_CLOSES:
		kernel_seq_delete(seq[fx_left_door]);
		seq[fx_left_door] = kernel_seq_backward(ss[fx_left_door], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_left_door], 14);
		kernel_seq_range(seq[fx_left_door], 1, 5);
		kernel_seq_trigger(seq[fx_left_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_202_DOOR_CLOSES + 1);
		break;

	case ROOM_202_DOOR_CLOSES + 1:
		sound_play(N_DoorCloses);
		seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 1);
		kernel_seq_depth(seq[fx_left_door], 14);
		player.commands_allowed = true;
		break;
	}

	if ((global[walker_converse] == CONVERSE_HAND_WAVE) ||
	    (global[walker_converse] == CONVERSE_HAND_WAVE_2)) {
		++local->converse_counter;
		if (local->converse_counter > 200) {
			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		}
	}
}

static void process_conversation_17() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	switch (player_verb) {
	case conv017_ticket_block:
		if (!local->gave) {
			local->usher_action = CONV17_USHER_LEFT_UP;
		}
		global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		you_trig_flag           = true;
		me_trig_flag            = true;
		break;

	case conv017_hasit_first:
		conv_you_trigger(ROOM_202_USHER_POINT);
		conv_me_trigger(ROOM_202_END);
		you_trig_flag = true;
		me_trig_flag  = true;
		break;

	case conv017_hasnot_first:
		conv_me_trigger(ROOM_202_END);
		me_trig_flag  = true;
		you_trig_flag = true;
		break;

	default:
		break;
	}

	switch (kernel.trigger) {
	case ROOM_202_END:
		global[walker_converse] = CONVERSE_NONE;
		local->gave             = false;
		me_trig_flag            = true;
		you_trig_flag           = true;
		break;

	case ROOM_202_USHER_TALK:
		global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		local->usher_action     = CONV17_USHER_TALK;
		break;

	case ROOM_202_USHER_POINT:
		local->usher_action = CONV17_USHER_POINT;
		break;

	case ROOM_202_ME_TALK:
		global[walker_converse] = imath_random(CONVERSE_HAND_WAVE, CONVERSE_HAND_WAVE_2);
		local->usher_action     = CONV17_USHER_SHUT_UP;
		local->converse_counter = 0;
		break;
	}


	/* ================= Set up me and you triggers ================ */

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_202_ME_TALK);
	} /* if me_trig_flag == true, then a me trigger is called from above, not here. */

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_202_USHER_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */

	local->usher_talk_count = 0;
}

static void process_conversation_9() {
	int you_trig_flag = false;
	int me_trig_flag  = false;
	int dyn_degas;

	switch (player_verb) {
	case conv009_dialogue_three:
		global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		conv_me_trigger(ROOM_202_DEGAS_WALK);
		you_trig_flag = true;
		me_trig_flag  = true;
		break;

	case conv009_dialogue_one:
		global[degas_name_is_known] = YES;
		break;
	}

	switch (kernel.trigger) {
	case ROOM_202_DEGAS_TALK:
		global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		switch (player_verb) {
		case conv009_dialogue_one:
			local->degas_action = CONV9_DEGAS_BOW;
			break;

		case conv009_dialogue_paint:
			local->degas_action = CONV9_DEGAS_GIRL;
			break;

		default:
			if ((player_verb != conv009_lips_abc) &&
			    (player_verb != conv009_wink_abc)) {
				local->degas_action = CONV9_DEGAS_TALK;
			}
			break;
		}
		break;

	case ROOM_202_DEGAS_WALK:
		conv_you_trigger(ROOM_202_DEGAS_WALK + 1);
		you_trig_flag = true;
		me_trig_flag  = true;
		break;

	case ROOM_202_DEGAS_WALK + 1:
		conv_me_trigger(ROOM_202_DEGAS_WALK + 2);
		local->degas_action = CONV9_DEGAS_TALK;
		you_trig_flag       = true;
		me_trig_flag        = true;
		break;

	case ROOM_202_DEGAS_WALK + 2:
		global[walker_converse] = CONVERSE_NONE;
		local->degas_action     = CONV9_DEGAS_LEAVE;

		if (global[degas_name_is_known] == YES) {
			dyn_degas = kernel_add_dynamic(words_Edgar_Degas, words_walk_to, SYNTAX_SINGULAR_MASC, KERNEL_NONE,
			                0, 0, 0, 0);
			kernel_dynamic_hot[dyn_degas].prep = PREP_ON;
			kernel_dynamic_walk(dyn_degas, DEGAS_AFTER_CONV_X, DEGAS_AFTER_CONV_Y, FACING_EAST);
			kernel_dynamic_anim(dyn_degas, aa[1], 1);

		} else {
			dyn_degas = kernel_add_dynamic(words_gentleman, words_walk_to, SYNTAX_SINGULAR_MASC, KERNEL_NONE,
			                0, 0, 0, 0);
			kernel_dynamic_hot[dyn_degas].prep = PREP_ON;
			kernel_dynamic_walk(dyn_degas, DEGAS_AFTER_CONV_X, DEGAS_AFTER_CONV_Y, FACING_EAST);
			kernel_dynamic_anim(dyn_degas, aa[1], 1);
		}

		global[degas_name_is_known] = YES_AND_END_CONV;
		you_trig_flag               = true;
		me_trig_flag                = true;
		kernel_flip_hotspot(words_gentleman, false);
		player.commands_allowed = false;
		conv_hold();
		break;

	case ROOM_202_ME_TALK:
		global[walker_converse] = imath_random(CONVERSE_HAND_WAVE, CONVERSE_HAND_WAVE_2);
		local->degas_action     = CONV9_DEGAS_SHUT_UP;
		local->converse_counter = 0;
		break;
	}


	/* ================= Set up me and you triggers ================ */

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_202_ME_TALK);
	} /* if me_trig_flag == true, then a me trigger is called from above, not here. */

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_202_DEGAS_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */
}

void room_202_pre_parser() {
	if (player_said_2(open, left_door)) {
		player_walk(FRONT_OF_LEFT_DOOR_X, FRONT_OF_LEFT_DOOR_Y, FACING_NORTHEAST);
	}

	if ((global[ticket_people_here] == USHER_AND_SELLER) && (player_said_2(walk_through, right_archway))) {
		player_walk(USHER_X, USHER_Y, FACING_NORTHEAST);
	}

	if (player_said_2(take, gentleman) || player_said_2(take, Edgar_Degas)) {
		player.need_to_walk = false;
	}
}

void room_202_parser() {
	int temp;  /* for synching purposes */

	if (conv_control.running == CONV_USHER_17) {
		process_conversation_17();
		goto handled;
	}

	if (conv_control.running == CONV_DEGAS_9) {
		process_conversation_9();
		goto handled;
	}

	if (global[ticket_people_here] == USHER_AND_SELLER) {
		if (player_said_2(walk_through, right_archway) || player_said_2(talk_to, usher)) {
			conv_run(CONV_USHER_17);
			conv_export_value(player_has(ticket));
			conv_export_value(0);
			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
			goto handled;
		}
	}

	if (player_said_3(give, ticket, usher)) {
		local->gave = true;
		conv_run(CONV_USHER_17);
		conv_export_value(player_has(ticket));
		conv_export_value(1);
		global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		goto handled;
	}

	if ((player_said_2(talk_to, gentleman)) ||
	    (player_said_2(talk_to, Edgar_Degas))) {
		if (!global[degas_name_is_known]) {
			conv_run(CONV_DEGAS_9);
			conv_export_pointer(&global[player_score]);
			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
			goto handled;
		} else {
			text_show(text_202_24);
			goto handled;
		}
	}

	if (player_said_2(walk_through, left_door) || player_said_2(open, left_door)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			seq[fx_take_9] = kernel_seq_pingpong(ss[fx_take_9], false, 5, 0, 0, 2);
			kernel_seq_range(seq[fx_take_9], 1, 4);
			kernel_seq_player(seq[fx_take_9], true);
			kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_SPRITE, 4, ROOM_202_DOOR_OPENS);
			kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, ROOM_202_DOOR_OPENS + 2);
			break;

		case ROOM_202_DOOR_OPENS:
			kernel_seq_delete(seq[fx_left_door]);
			seq[fx_left_door] = kernel_seq_forward(ss[fx_left_door], false, 8, 0, 0, 1);
			kernel_seq_depth(seq[fx_left_door], 14);
			kernel_seq_range(seq[fx_left_door], 1, 5);
			kernel_seq_trigger(seq[fx_left_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_202_DOOR_OPENS + 1);
			sound_play(N_DoorOpens);
			break;

		case ROOM_202_DOOR_OPENS + 1:
			temp              = seq[fx_left_door];
			seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 5);
			kernel_synch(KERNEL_SERIES, seq[fx_left_door], KERNEL_SERIES, temp);
			kernel_seq_depth(seq[fx_left_door], 14);
			player_walk(WALK_TO_X_BEHIND_LEFT_DOOR, WALK_TO_Y_BEHIND_LEFT_DOOR, FACING_NORTH);
			player_walk_trigger(ROOM_202_DOOR_OPENS + 3);
			break;

		case ROOM_202_DOOR_OPENS + 2:
			player.walker_visible = true;
			break;

		case ROOM_202_DOOR_OPENS + 3:
			kernel_seq_delete(seq[fx_left_door]);
			seq[fx_left_door] = kernel_seq_backward(ss[fx_left_door], false, 8, 0, 0, 1);
			kernel_seq_depth(seq[fx_left_door], 1);
			kernel_seq_range(seq[fx_left_door], 1, 5);
			kernel_seq_trigger(seq[fx_left_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_202_DOOR_OPENS + 4);
			sound_play(N_DoorCloses);
			break;

		case ROOM_202_DOOR_OPENS + 4:
			seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 5);
			kernel_seq_depth(seq[fx_left_door], 1);
			new_room = 203;
			break;
		}
		goto handled;
	}

	if (player_said_2(walk_through, left_archway)) {
		new_room = 201;
		goto handled;
	}

	if (player_said_2(walk_through, right_archway)) {
		new_room = 101;
		goto handled;
	}

	if (player_said_2(walk_through, middle_door)) {
		new_room = 204;
		goto handled;
	}

	if (player_said_2(walk_through, right_door)) {
		new_room = 205;
		goto handled;
	}

	if (player.look_around) {
		text_show(text_202_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(floor)) {
			text_show(text_202_11);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_202_12);
			goto handled;
		}

		if (player_said_1(left_archway)) {
			text_show(text_202_13);
			goto handled;
		}

		if (player_said_1(right_archway)) {
			text_show(text_202_14);
			goto handled;
		}

		if (player_said_1(left_door)) {
			text_show(text_202_15);
			goto handled;
		}

		if (player_said_1(middle_door)) {
			text_show(text_202_16);
			goto handled;
		}

		if (player_said_1(right_door)) {
			text_show(text_202_17);
			goto handled;
		}

		if (player_said_1(light_fixture)) {
			text_show(text_202_18);
			goto handled;
		}

		if (player_said_1(overdoor_medallion)) {
			text_show(text_202_19);
			goto handled;
		}

		if (player_said_1(decorative_molding)) {
			text_show(text_202_20);
			goto handled;
		}

		if (player_said_1(painting)) {
			text_show(text_202_21);
			goto handled;
		}

		if (player_said_1(Edgar_Degas) || player_said_1(gentleman)) {
			text_show(text_202_23);
			goto handled;
		}

		if (player_said_1(usher)) {
			text_show(text_202_25);
			goto handled;
		}

		if (player_said_1(chandelier)) {
			text_show(text_202_18);
			goto handled;
		}
	}

	if (player_said_1(take)) {
		if (player_said_1(gentleman) || player_said_1(Edgar_Degas)) {
			text_show(text_202_26);
			goto handled;
		}
	}

	if (player_said_2(take, usher)) {
		text_show(text_202_27);
		goto handled;
	}

	if (player_said_2(close, door)) {
		text_show(text_202_22);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_202_preload() {
	room_init_code_pointer       = room_202_init;
	room_pre_parser_code_pointer = room_202_pre_parser;
	room_parser_code_pointer     = room_202_parser;
	room_daemon_code_pointer     = room_202_daemon;

	section_2_walker();
	section_2_interface();

	if (global[ticket_people_here] == USHER_AND_SELLER) {
		kernel_initial_variant = 1;
	}

	vocab_make_active(words_chandelier);
	vocab_make_active(words_Edgar_Degas);
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
