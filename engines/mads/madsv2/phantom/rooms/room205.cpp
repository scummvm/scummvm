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
#include "mads/madsv2/phantom/rooms/room205.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

static void handle_animation_rich() {
	int random;
	int rich_reset_frame;

	if (kernel_anim[aa[0]].frame != local->rich_frame) {
		local->rich_frame = kernel_anim[aa[0]].frame;
		rich_reset_frame = -1;

		switch (local->rich_frame) {
		case 1:   /* end of talk 1 & freeze & left & right & up */
		case 2:   /* end of talk 2                              */
		case 3:   /* end of talk 3                              */
		case 19:  /* end of point                               */
		case 11:  /* end of both arms up                        */
		case 69:  /* end of return from both go                 */
		case 57:  /* end of return from collar                  */
		case 47:  /* end of return from chin                    */
		case 35:  /* end of return from hips                    */

			switch (local->rich_action) {
			case CONV18_RICH_POINT:
				random = 4; /* point */
				local->rich_action = CONV18_RICH_TALK;
				break;

			case CONV18_RICH_BOTH_ARMS_UP:
				random = 5; /* both arms up like 'what the hell' */
				local->rich_action = CONV18_RICH_TALK;
				break;

			case CONV18_RICH_BOTH_GO:
				random = 6; /* put both arms up like 'get going, man' */
				break;

			case CONV18_RICH_TALK:
				random = imath_random(1, 3);
				++local->rich_talk_count;
				if (local->rich_talk_count > 30) {
					local->rich_action = CONV18_RICH_SHUT_UP;
					random = 9;
				}
				break;

			default:  /* CONV18_RICH_SHUT_UP */
				random = imath_random(7, 50);
				while (local->just_did_option == random) {
					random = imath_random(7, 50);
				} /* so we don't repeat the same option twice */
				local->just_did_option = random;
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 0;
				break; /* do talk 1 */

			case 2:
				rich_reset_frame = 1;
				break; /* do talk 2 */

			case 3:
				rich_reset_frame = 2;
				break; /* do talk 3 */

			case 4:
				rich_reset_frame = 11;
				break; /* point */

			case 5:
				rich_reset_frame = 3;
				break; /* both arms up 'what the hell' */

			case 6:
				rich_reset_frame = 57;
				break; /* both arms up 'get going' */

			case 7:
				rich_reset_frame = 23;
				break; /* look up (new node) */

			case 8:
				rich_reset_frame = 19;
				break; /* look left (new node) */

			case 9:
				rich_reset_frame = 21;
				break; /* look right (new node) */

			case 10:
				rich_reset_frame = 25;
				break; /* put hands on hips (new node) */

			case 11:
				rich_reset_frame = 35;
				break; /* put hands on chin (new node) */

			case 12:
				rich_reset_frame = 47;
				break; /* put hands on collar (new node) */

			default:
				rich_reset_frame = 0;
				break; /* freeze standing still */
			}
			break;

		case 30:  /* put hands on hips */

			switch (local->rich_action) {
			case CONV18_RICH_POINT:
			case CONV18_RICH_BOTH_GO:
			case CONV18_RICH_BOTH_ARMS_UP:
			case CONV18_RICH_TALK:
				random = 1; /* go to freeze standing still (new node) */
				break;

			default:
				random = imath_random(1, 50);
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 30;
				break; /* goto hands by side (new node) */

			default:
				rich_reset_frame = 29;
				break; /* hands on hips */
			}
			break;

		case 24:  /* look up */

			switch (local->rich_action) {
			case CONV18_RICH_POINT:
			case CONV18_RICH_BOTH_GO:
			case CONV18_RICH_BOTH_ARMS_UP:
			case CONV18_RICH_TALK:
				random = 1; /* go to freeze standing still (new node) */
				break;

			default:
				random = imath_random(1, 30);
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 0;
				break; /* goto hands by side (new node) */

			default:
				rich_reset_frame = 23;
				break; /* look up */
			}
			break;

		case 20:  /* look left */

			switch (local->rich_action) {
			case CONV18_RICH_POINT:
			case CONV18_RICH_BOTH_GO:
			case CONV18_RICH_BOTH_ARMS_UP:
			case CONV18_RICH_TALK:
				random = 1; /* go to freeze standing still (new node) */
				break;

			default:
				random = imath_random(1, 50);
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 0;
				break; /* goto hands by side (new node) */

			default:
				rich_reset_frame = 19;
				break; /* look left */
			}
			break;

		case 22:  /* look right */

			switch (local->rich_action) {
			case CONV18_RICH_POINT:
			case CONV18_RICH_BOTH_GO:
			case CONV18_RICH_BOTH_ARMS_UP:
			case CONV18_RICH_TALK:
				random = 1; /* go to freeze standing still (new node) */
				break;

			default:
				random = imath_random(1, 50);
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 0;
				break; /* goto hands by side (new node) */

			default:
				rich_reset_frame = 21;
				break; /* look right */
			}
			break;

		case 41:  /* put hands on chin */

			switch (local->rich_action) {
			case CONV18_RICH_POINT:
			case CONV18_RICH_BOTH_GO:
			case CONV18_RICH_BOTH_ARMS_UP:
			case CONV18_RICH_TALK:
				random = 1; /* go to freeze standing still (new node) */
				break;

			default:
				random = imath_random(1, 50);
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 41;
				break; /* goto hands by side (new node) */

			default:
				rich_reset_frame = 40;
				break; /* hands on chin */
			}
			break;

		case 52:  /* put hands on collar */

			switch (local->rich_action) {
			case CONV18_RICH_POINT:
			case CONV18_RICH_BOTH_GO:
			case CONV18_RICH_BOTH_ARMS_UP:
			case CONV18_RICH_TALK:
				random = 1; /* go to freeze standing still (new node) */
				break;

			default:
				random = imath_random(1, 50);
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 52;
				break; /* goto hands by side (new node) */

			default:
				rich_reset_frame = 51;
				break; /* hands on collar */
			}
			break;

		case 65:  /* both hands up in air (get going, man) */

			switch (local->rich_action) {
			case CONV18_RICH_POINT:
			case CONV18_RICH_BOTH_GO:
			case CONV18_RICH_BOTH_ARMS_UP:
			case CONV18_RICH_TALK:
				random = 1; /* go to freeze standing still (new node) */
				break;

			default:
				random = imath_random(1, 50);
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 65;
				break; /* goto hands by side (new node) */

			default:
				rich_reset_frame = 64;
				break; /* keep hands up in air */
			}
			break;
		}

		if (rich_reset_frame >= 0) {
			kernel_reset_animation(aa[0], rich_reset_frame);
			local->rich_frame = rich_reset_frame;
		}
	}
}

static void handle_animation_giry() {
	int random;
	int giry_reset_frame;

	if (kernel_anim[aa[1]].frame != local->giry_frame) {
		local->giry_frame = kernel_anim[aa[1]].frame;
		giry_reset_frame = -1;

		switch (local->giry_frame) {
		case 77:  /* almost end of trance */
			conv_release();
			break;

		case 1:   /* end of talk 1 & freeze                     */
		case 2:   /* end of talk 2                              */
		case 3:   /* end of talk 3                              */
		case 44:  /* end of touch head                          */
		case 14:  /* end of both arms up                        */
		case 21:  /* end of nod                                 */
		case 35:  /* end of point and talk                      */
		case 56:  /* end of welcome                             */
		case 78:  /* end of trance                              */
		case 284: /* end of take                                */

			switch (local->giry_action) {
			case CONV10_GIRY_TALK:
				random = imath_random(1, 3);
				++local->giry_talk_count;
				if (local->giry_talk_count > 30) {
					local->giry_action = CONV10_GIRY_SHUT_UP;
					random = 100;
				}
				break;

			case CONV10_GIRY_HANDS_UP:
				random = 4; /* hands up */
				local->giry_action = CONV10_GIRY_TALK;
				break;

			case CONV10_GIRY_NOD:
				random = 5; /* nod */
				local->giry_action = CONV10_GIRY_TALK;
				break;

			case CONV10_GIRY_POINT_TALK:
				random = 6; /* point and talk */
				break;

			case CONV10_GIRY_WELCOME:
				local->giry_action = CONV10_GIRY_TALK;
				random = 7; /* give a welcome */
				break;

			case CONV10_GIRY_TRANCE:
				random = 8; /* go into a trance state */
				break;

			case CONV10_GIRY_LEFT_DOOR:
				random = 9; /* open left door (new node) */
				local->giry_action = CONV10_GIRY_SHUT_UP;
				break;

			case CONV10_GIRY_RIGHT_DOOR:
				random = 10; /* open right door (new node) */
				local->giry_action = CONV10_GIRY_SHUT_UP;
				break;

			case CONV11_GIRY_TAKE:
				random = 11; /* take ticket */
				break;

			default:  /* CONV10_GIRY_SHUT_UP */
				random = imath_random(12, 100);
				break;
			}

			switch (random) {
			case 1:
				giry_reset_frame = 0;
				break; /* do talk 1 */

			case 2:
				giry_reset_frame = 1;
				break; /* do talk 2 */

			case 3:
				giry_reset_frame = 2;
				break; /* do talk 3 */

			case 4:
				giry_reset_frame = 3;
				break; /* put both hands up (new node) */

			case 5:
				giry_reset_frame = 16;
				break; /* nod */

			case 6:
				giry_reset_frame = 21;
				break; /* point and talk (new node) */

			case 7:
				giry_reset_frame = 44;
				break; /* give a welcome */

			case 8:
				giry_reset_frame = 56;
				break; /* go into a trance (new node) */

			case 9:
				giry_reset_frame = 78;
				conv_hold();
				break; /* open left door (new node) */

			case 10:
				giry_reset_frame = 140;
				break; /* open right door (new node) */

			case 11:
				giry_reset_frame = 276;
				break; /* take ticket (new node) */

			case 12:
				giry_reset_frame = 35;
				break; /* touch head */

			default:
				giry_reset_frame = 0;
				break; /* freeze standing still */
			}
			break;

		case 27:  /* end of point and talk 1 and freeze */
		case 28:  /* end of point and talk 2 */
		case 29:  /* end of point and talk 3 */

			switch (local->giry_action) {
			case CONV10_GIRY_TALK:
			case CONV11_GIRY_TAKE:
			case CONV10_GIRY_SHUT_UP:
			case CONV10_GIRY_NOD:
			case CONV10_GIRY_TRANCE:
			case CONV10_GIRY_WELCOME:
			case CONV10_GIRY_LEFT_DOOR:
			case CONV10_GIRY_RIGHT_DOOR:
			case CONV10_GIRY_HANDS_UP:
				random = 4; /* go to freeze standing still (new node) */
				break;

			default:
				random = imath_random(1, 3);
				++local->giry_talk_count;
				if (local->giry_talk_count > 30) {
					local->giry_action = CONV10_GIRY_SHUT_UP;
					random = 100;
				}
				break;
			}

			switch (random) {
			case 1:
				giry_reset_frame = 26;
				break; /* point and talk 1 */

			case 2:
				giry_reset_frame = 27;
				break; /* point and talk 2 */

			case 3:
				giry_reset_frame = 28;
				break; /* point and talk 3 */

			default:
				giry_reset_frame = 29;
				break; /* hands by side */
			}
			break;

		case 265:  /* almost end of unlock box 9 */
			kernel_flip_hotspot_loc(words_Madame_Giry, true, HS_MADAME_X_R_1, HS_MADAME_Y_R_1);
			kernel_flip_hotspot_loc(words_Madame_Giry, true, HS_MADAME_X_R_2, HS_MADAME_Y_R_2);
			conv_release();
			break;

		case 274:  /* end of talk 1 by box 9 and freeze */
		case 275:  /* end of talk 2 by box 9 */
		case 276:  /* end of talk 3 by box 9 */

			switch (local->giry_action) {
			case CONV10_GIRY_TALK:
				random = imath_random(1, 3);
				++local->giry_talk_count;
				if (local->giry_talk_count > 30) {
					local->giry_action = CONV10_GIRY_SHUT_UP;
					random = 100;
				}
				break;

			default:
				random = 100; /* go to freeze standing by right door */
				break;
			}

			switch (random) {
			case 1:
				giry_reset_frame = 273;
				break; /* talk by right door 1 */

			case 2:
				giry_reset_frame = 274;
				break; /* talk by right door 2 */

			case 3:
				giry_reset_frame = 275;
				break; /* talk by right door 3 */

			default:
				giry_reset_frame = 273;
				break; /* freeze standing by right door */
			}
			break;

		case 85:  /* just starting to walk to left door */
			conv_release();
			break;

		case 110:  /* almost end of unlock box 5 */
			kernel_flip_hotspot_loc(words_Madame_Giry, true, HS_MADAME_X_L_1, HS_MADAME_Y_L_1);
			kernel_flip_hotspot_loc(words_Madame_Giry, true, HS_MADAME_X_L_2, HS_MADAME_Y_L_2);
			break;

		case 138:  /* end of talk 1 by box 5 and freeze */
		case 139:  /* end of talk 2 by box 5 */
		case 140:  /* end of talk 3 by box 5 */

			switch (local->giry_action) {
			case CONV10_GIRY_TALK:
			case CONV10_GIRY_POINT_TALK:
				random = imath_random(1, 3);
				++local->giry_talk_count;
				if (local->giry_talk_count > 30) {
					local->giry_action = CONV10_GIRY_SHUT_UP;
					random = 100;
				}
				break;

			default:
				random = 100; /* go to freeze standing by left door */
				break;
			}

			switch (random) {
			case 1:
				giry_reset_frame = 137;
				break; /* talk by left door 1 */

			case 2:
				giry_reset_frame = 138;
				break; /* talk by left door 2 */

			case 3:
				giry_reset_frame = 139;
				break; /* talk by left door 3 */

			default:
				giry_reset_frame = 137;
				break; /* freeze standing by left door */
			}
			break;

		case 66:  /* almost at end of getting to trance */
			conv_release();
			break;

		case 67:  /* end of trance and freeze */

			switch (local->giry_action) {
			case CONV10_GIRY_TALK:
			case CONV11_GIRY_TAKE:
			case CONV10_GIRY_SHUT_UP:
			case CONV10_GIRY_NOD:
			case CONV10_GIRY_WELCOME:
			case CONV10_GIRY_LEFT_DOOR:
			case CONV10_GIRY_RIGHT_DOOR:
			case CONV10_GIRY_HANDS_UP:
				random = 1; /* go to freeze standing still (new node) */
				break;

			default:
				random = 100;
				break; /* stay in trance */
			}

			switch (random) {
			case 1:
				giry_reset_frame = 67;
				break; /* goto hands by side (new node) */

			default:
				giry_reset_frame = 66;
				break; /* stay in trance */
			}
			break;

		case 8:  /* end of both hands up */
		case 9:  /* end of freeze */

			switch (local->giry_action) {
			case CONV10_GIRY_TALK:
			case CONV11_GIRY_TAKE:
			case CONV10_GIRY_SHUT_UP:
			case CONV10_GIRY_NOD:
			case CONV10_GIRY_TRANCE:
			case CONV10_GIRY_WELCOME:
			case CONV10_GIRY_LEFT_DOOR:
			case CONV10_GIRY_RIGHT_DOOR:
			case CONV10_GIRY_HANDS_UP:
				random = 1; /* go to freeze standing still (new node) */
				break;

			default:
				random = 100;
				break; /* keep both hands up */
			}

			switch (random) {
			case 1:
				giry_reset_frame = 9;
				break; /* goto hands by side (new node) */

			default:
				giry_reset_frame = 8;
				break; /* keep both hands up */
			}
			break;

		case 280:  /* hand almost fully extended for taking ticket */
			local->give_ticket = true;
			break;

		case 281:  /* hand extended for taking ticket */

			switch (local->giry_action) {
			case CONV10_GIRY_TALK:
			case CONV10_GIRY_SHUT_UP:
			case CONV10_GIRY_NOD:
			case CONV10_GIRY_TRANCE:
			case CONV10_GIRY_WELCOME:
			case CONV10_GIRY_LEFT_DOOR:
			case CONV10_GIRY_RIGHT_DOOR:
			case CONV10_GIRY_HANDS_UP:
				random = 1; /* go to freeze standing still (new node) */
				break;

			default:
				random = 100;
				break; /* keep hand up */
			}

			switch (random) {
			case 1:
				giry_reset_frame = 281;
				break; /* goto hands by side (new node) */

			default:
				giry_reset_frame = 280;
				break; /* keep hand up */
			}
			break;
		}

		if (giry_reset_frame >= 0) {
			kernel_reset_animation(aa[1], giry_reset_frame);
			local->giry_frame = giry_reset_frame;
		}
	}
}

void room_205_daemon() {
	if (local->anim_0_running) {
		handle_animation_rich();
	}

	if (local->anim_1_running) {
		handle_animation_giry();
	}

	if ((global[walker_converse] == CONVERSE_HAND_WAVE) ||
	    (global[walker_converse] == CONVERSE_HAND_WAVE_2)) {
		++local->converse_counter;
		if (local->converse_counter > 200) {
			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		}
	}

	if (local->give_ticket && !player_said_1(give)) {
		global[walker_converse] = CONVERSE_NONE;
		player_walk(player.x + 5, player.y - 10, FACING_NORTHWEST);
		player_walk_trigger(ROOM_205_TAKE_TICKET);
		local->give_ticket = false;
	}

	switch (kernel.trigger) {
	case ROOM_205_TAKE_TICKET:
		player.walker_visible = false;
		seq[fx_take_9] = kernel_seq_pingpong(ss[fx_take_9], true, 5, 0, 0, 2);
		kernel_seq_range(seq[fx_take_9], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_player(seq[fx_take_9], true);
		kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, ROOM_205_TAKE_TICKET + 2);
		kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_SPRITE, 4, ROOM_205_TAKE_TICKET + 1);
		break;

	case ROOM_205_TAKE_TICKET + 1:
		inter_move_object(ticket, NOWHERE);
		local->giry_action = CONV10_GIRY_SHUT_UP;
		break;

	case ROOM_205_TAKE_TICKET + 2:
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_9]);
		player.walker_visible   = true;
		player.commands_allowed = true;
		conv_release();
		break;
	}


	/* Complete left door closing */

	switch (kernel.trigger) {
	case ROOM_205_LEFT_DOOR_CLOSES:
		kernel_seq_delete(seq[fx_left_door]);
		seq[fx_left_door] = kernel_seq_backward(ss[fx_left_door], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_left_door], 14);
		kernel_seq_range(seq[fx_left_door], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_left_door],
		                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_205_LEFT_DOOR_CLOSES + 1);
		break;

	case ROOM_205_LEFT_DOOR_CLOSES + 1:
		seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_left_door], 14);
		sound_play(N_DoorCloses);
		player.commands_allowed = true;
		break;

	case ROOM_205_RIGHT_DOOR_CLOSES:
		kernel_seq_delete(seq[fx_right_door]);
		seq[fx_right_door] = kernel_seq_backward(ss[fx_right_door], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_right_door], 14);
		kernel_seq_range(seq[fx_right_door], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_right_door],
		                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_205_RIGHT_DOOR_CLOSES + 1);
		break;

	case ROOM_205_RIGHT_DOOR_CLOSES + 1:
		seq[fx_right_door] = kernel_seq_stamp(ss[fx_right_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_right_door], 14);
		sound_play(N_DoorCloses);
		player.commands_allowed = true;
		break;
	}
}

static void process_conversation_18() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	switch (player_verb) {
	case conv018_begin_first:
		global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		local->rich_action = CONV18_RICH_TALK;
		break;

	case conv018_adieu_bye:
	case conv018_nomore_huh:
		conv_me_trigger(ROOM_205_END);
		global[ran_conv_in_205] = true;
		me_trig_flag            = true;
		you_trig_flag           = true;
		break;

	default:
		break;
	}

	switch (kernel.trigger) {
	case ROOM_205_END:
		global[walker_converse] = CONVERSE_NONE;
		me_trig_flag            = true;
		you_trig_flag           = true;
		break;

	case ROOM_205_RICHARD_TALK:
		global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		local->rich_action = CONV18_RICH_TALK;
		break;

	case ROOM_205_ME_TALK:
		global[walker_converse] = imath_random(CONVERSE_HAND_WAVE, CONVERSE_HAND_WAVE_2);
		local->rich_action = CONV18_RICH_SHUT_UP;
		local->converse_counter = 0;
		break;

	default:
		break;
	}


	/* Set up me and you triggers */

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_205_ME_TALK);
	} /* if me_trig_flag == true, then a me trigger is called from above, not here. */

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_205_RICHARD_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */

	local->rich_talk_count = 0;
}

static void process_conversation_10() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	switch (player_verb) {
	case conv010_beginning_who:
		conv_you_trigger(ROOM_205_WELCOME);
		you_trig_flag = true;
		global[madame_name_is_known] = YES;
		kernel_flip_hotspot(words_woman, false);
		kernel_flip_hotspot_loc(words_Madame_Giry, true, HS_MADAME_X_M_1, HS_MADAME_Y_M_1);
		kernel_flip_hotspot_loc(words_Madame_Giry, true, HS_MADAME_X_M_2, HS_MADAME_Y_M_2);
		break;

	case conv010_beginning_you:
		global[madame_name_is_known] = YES;
		kernel_flip_hotspot(words_woman, false);
		kernel_flip_hotspot_loc(words_Madame_Giry, true, HS_MADAME_X_M_1, HS_MADAME_Y_M_1);
		kernel_flip_hotspot_loc(words_Madame_Giry, true, HS_MADAME_X_M_2, HS_MADAME_Y_M_2);
		break;

	case conv010_unlock_b_box_5:
		if (!kernel.trigger) {
			local->giry_action = CONV10_GIRY_LEFT_DOOR;
			global[madame_name_is_known] = YES_AND_END_CONV;
			global[madame_giry_loc] = LEFT;
			kernel_load_variant(2);
			rail_disconnect_node(6);
			rail_disconnect_node(7);
			rail_disconnect_node(8);
			rail_disconnect_node(9);
			/* these rail nodes are only needed if Richard is in this room (after kidnapping) */
			kernel_flip_hotspot_loc(words_Madame_Giry, false, HS_MADAME_X_M_1, HS_MADAME_Y_M_1);
			kernel_flip_hotspot_loc(words_Madame_Giry, false, HS_MADAME_X_M_2, HS_MADAME_Y_M_2);
			you_trig_flag = true;
			me_trig_flag  = true;
			if (global[doors_in_205] == BOTH_LOCKED) {
				global[doors_in_205] = LEFT_OPEN;
			} else if (global[doors_in_205] == RIGHT_OPEN) {
				global[doors_in_205] = BOTH_OPEN;
			}
		}
		break;

	case conv010_nomore_abc:
	case conv010_ghost_interest:
		conv_you_trigger(ROOM_205_POINT_TALK);
		you_trig_flag = true;
		break;

	case conv010_trance_b_box_5:
		if ((!kernel.trigger) && (!local->no_hold)) {
			conv_hold();
			local->giry_action = CONV10_GIRY_TRANCE;
		} else {
			local->no_hold = false;
		}
		break;

	case conv010_dialogue_one:
		conv_you_trigger(ROOM_205_NOD);
		you_trig_flag = true;
		break;

	case conv010_beginning_pardon:
		if (!kernel.trigger) {
			conv_you_trigger(ROOM_205_BOTH_HANDS_UP);
		}
		me_trig_flag  = true;
		you_trig_flag = true;
		break;

	case conv010_box_b_b:
	case conv010_box_d_d:
		if ((!kernel.trigger) && (local->giry_action == CONV10_GIRY_TRANCE)) {
			conv_hold();
			local->giry_action = CONV10_GIRY_TALK;
		}
		break;

	case conv010_hasthem_abc:
	case conv010_noyoudont_abc:
	case conv010_gethem_abc:
	case conv010_byebye_first:
	case conv010_dialogue_three:
	case conv010_ghost_byebye:
	case conv010_box_byebye:
	case conv010_bye_two_solong:
		conv_you_trigger(ROOM_205_END);
		you_trig_flag = true;
		break;

	default:
		break;
	}

	switch (kernel.trigger) {
	case ROOM_205_END:
		switch (player_verb) {
		case conv010_hasthem_abc:
		case conv010_dialogue_three:
		case conv010_box_byebye:
		case conv010_ghost_byebye:
		case conv010_byebye_first:
		case conv010_bye_two_solong:
			local->giry_action = CONV10_GIRY_TALK;
			break;
		}
		global[walker_converse] = CONVERSE_NONE;
		me_trig_flag            = true;
		you_trig_flag           = true;
		break;

	case ROOM_205_WELCOME:
		local->giry_action = CONV10_GIRY_WELCOME;
		break;

	case ROOM_205_BOTH_HANDS_UP:
		local->giry_action      = CONV10_GIRY_HANDS_UP;
		global[walker_converse] = CONVERSE_NONE;
		me_trig_flag            = true;
		you_trig_flag           = true;
		break;

	case ROOM_205_NOD:
		local->giry_action = CONV10_GIRY_NOD;
		break;

	case ROOM_205_POINT_TALK:
		if (player_verb == conv010_nomore_abc) {
			global[walker_converse] = CONVERSE_NONE;
			me_trig_flag            = true;
			you_trig_flag           = true;
		}
		local->giry_action = CONV10_GIRY_POINT_TALK;
		break;

	case ROOM_205_GIRY_TALK:
		if (global[walker_converse] != CONVERSE_NONE) {
			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		}
		if (local->giry_action != CONV10_GIRY_TRANCE &&
		    local->giry_action != CONV10_GIRY_LEFT_DOOR &&
		    local->giry_action != CONV10_GIRY_RIGHT_DOOR) {
			local->giry_action = CONV10_GIRY_TALK;
		}
		break;

	case ROOM_205_ME_TALK:
		if (global[walker_converse] != CONVERSE_NONE) {
			global[walker_converse] = imath_random(CONVERSE_HAND_WAVE, CONVERSE_HAND_WAVE_2);
		}
		if (local->giry_action != CONV10_GIRY_TRANCE &&
		    local->giry_action != CONV10_GIRY_LEFT_DOOR &&
		    local->giry_action != CONV10_GIRY_RIGHT_DOOR) {
			local->giry_action = CONV10_GIRY_SHUT_UP;
		}
		local->converse_counter = 0;
		break;
	}


	/* Set up me and you triggers */

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_205_ME_TALK);
	} /* if me_trig_flag == true, then a me trigger is called from above, not here. */

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_205_GIRY_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */

	local->giry_talk_count = 0;
}

static void process_conversation_11() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	switch (player_verb) {
	case conv011_has_b_ticket:
		if (!kernel.trigger) {
			conv_hold();
			local->giry_action = CONV11_GIRY_TAKE;
		}
		break;

	case conv011_enter_b_a:
		if (!kernel.trigger) {
			conv_hold();
			local->giry_action = CONV10_GIRY_RIGHT_DOOR;
			conv_you_trigger(ROOM_205_END);
			player_walk(225, 79, FACING_NORTHEAST);
			you_trig_flag = true;
			global[madame_giry_loc] = RIGHT;
			kernel_load_variant(4);
			rail_disconnect_node(6);
			rail_disconnect_node(7);
			rail_disconnect_node(8);
			rail_disconnect_node(9);
			/* these rail nodes are only needed if Richard is in this room (after kidnapping) */
			kernel_flip_hotspot_loc(words_Madame_Giry, false, HS_MADAME_X_M_1, HS_MADAME_Y_M_1);
			kernel_flip_hotspot_loc(words_Madame_Giry, false, HS_MADAME_X_M_2, HS_MADAME_Y_M_2);
			if (global[doors_in_205] == BOTH_LOCKED) {
				global[doors_in_205] = RIGHT_OPEN;
			} else if (global[doors_in_205] == LEFT_OPEN) {
				global[doors_in_205] = BOTH_OPEN;
			}
		}
		break;

	case conv011_hasnot_abc:
	case conv011_boxoffice_abc:
	case conv011_enter_it_abc:
	case conv011_enjoy_abc:
	case conv011_five_nono:
		conv_you_trigger(ROOM_205_END);
		you_trig_flag = true;
		break;

	default:
		break;
	}

	switch (kernel.trigger) {
	case ROOM_205_END:
		switch (player_verb) {
		case conv011_has_abc:
		case conv011_hasnot_abc:
		case conv011_enjoy_abc:
		case conv011_boxoffice_abc:
		case conv011_five_nono:
			local->giry_action = CONV10_GIRY_TALK;
			break;
		}
		global[walker_converse] = CONVERSE_NONE;
		me_trig_flag            = true;
		you_trig_flag           = true;
		break;

	case ROOM_205_WALK:
		conv_hold();
		local->giry_action = CONV10_GIRY_RIGHT_DOOR;
		break;

	case ROOM_205_GIRY_TALK:
		if (global[walker_converse] != CONVERSE_NONE) {
			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		}
		if (local->giry_action != CONV11_GIRY_TAKE) {
			local->giry_action = CONV10_GIRY_TALK;
		}
		break;

	case ROOM_205_ME_TALK:
		if (global[walker_converse] != CONVERSE_NONE) {
			global[walker_converse] = imath_random(CONVERSE_HAND_WAVE, CONVERSE_HAND_WAVE_2);
		}
		if (local->giry_action != CONV11_GIRY_TAKE) {
			local->giry_action = CONV10_GIRY_SHUT_UP;
		}
		local->converse_counter = 0;
		break;
	}


	/* Set up me and you triggers */

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_205_ME_TALK);
	} /* if me_trig_flag == true, then a me trigger is called from above, not here. */

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_205_GIRY_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */

	local->giry_talk_count = 0;
}

void room_205_pre_parser() {
	if ((player_said_1(box_five)) && (player_said_1(lock) || player_said_1(unlock) || player_said_1(open))) {
		player_walk(FRONT_5_X, FRONT_5_Y, FACING_NORTHEAST);
	}

	if ((player_said_1(box_six)) && (player_said_1(lock) || player_said_1(unlock) || player_said_1(open))) {
		player_walk(FRONT_6_X, FRONT_6_Y, FACING_NORTHEAST);
	}

	if ((player_said_1(box_seven)) && (player_said_1(lock) || player_said_1(unlock) || player_said_1(open))) {
		player_walk(FRONT_7_X, FRONT_7_Y, FACING_NORTHEAST);
	}

	if ((player_said_1(box_eight)) && (player_said_1(lock) || player_said_1(unlock) || player_said_1(open))) {
		player_walk(FRONT_8_X, FRONT_8_Y, FACING_NORTHEAST);
	}

	if ((player_said_1(box_nine)) && (player_said_1(lock) || player_said_1(unlock) || player_said_1(open))) {
		player_walk(FRONT_9_X, FRONT_9_Y, FACING_NORTHEAST);
	}

	if (player_said_2(walk_down_stairs_to, grand_foyer)) {
		if ((global[doors_in_205] != RIGHT_OPEN) && (global[doors_in_205] != BOTH_OPEN)) {
			player.walk_off_edge_to_room = 202;
			global[madame_giry_loc] = MIDDLE;
		}
	}
}

void room_205_parser() {
	int temp;
	int frame_counter = 0;

	if (conv_control.running == CONV_RICHARD_18) {
		process_conversation_18();
		goto handled;
	}

	if (conv_control.running == CONV_GIRY_10) {
		process_conversation_10();
		goto handled;
	}

	if (conv_control.running == CONV_GIRY_11) {
		process_conversation_11();
		goto handled;
	}

	if ((player_said_2(open, box_five)) || (player_said_2(enter, box_five))) {
		if (global[ticket_people_here] == USHER_AND_SELLER) {
			if (global[madame_giry_loc] == RIGHT) {
				conv_run(CONV_GIRY_11);
				conv_export_value(player_has(ticket));
				conv_export_value(3);
				conv_export_value(0);
				/* it is reserved, put box on left */
				goto handled;
			} else {
				conv_run(CONV_GIRY_11);
				conv_export_value(player_has(ticket));
				conv_export_value(4);
				conv_export_value(0);
				/* it is reserved, put box on right */
				goto handled;
			}
		}
	}

	if (player_said_2(walk_down_stairs_to, grand_foyer)) {
		if (global[ticket_people_here] == USHER_AND_SELLER) {
			if (global[madame_giry_loc] == RIGHT) {
				conv_run(CONV_GIRY_11);
				conv_export_value(player_has(ticket));
				conv_export_value(1);
				conv_export_value(0);
				/* performance is about to begin, put box on left */
			} else {
				conv_run(CONV_GIRY_11);
				conv_export_value(player_has(ticket));
				conv_export_value(2);
				conv_export_value(0);
				/* performance is about to begin, put box on right */
			}
			goto handled;
		}
	}

	if ((player_said_1(enter)) || (player_said_1(open)) || player_said_1(unlock) || player_said_1(lock)) {
		if (((player_said_1(box_five)) && ((global[doors_in_205] == BOTH_LOCKED) || (global[doors_in_205] == RIGHT_OPEN))
		  || player_said_1(unlock) || player_said_1(lock))
		                                         ||
		 ((player_said_1(box_nine)) && ((global[doors_in_205] == BOTH_LOCKED) || (global[doors_in_205] == LEFT_OPEN)))
		                                         ||
		  (player_said_1(box_six)) || (player_said_1(box_seven)) || (player_said_1(box_eight))) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take_9] = kernel_seq_forward(ss[fx_take_9], false, 5, 0, 0, 1);
				kernel_seq_range(seq[fx_take_9], 1, 4);
				kernel_seq_player(seq[fx_take_9], true);
				kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, 1);
				goto handled;
				break;

			case 1:
				temp = seq[fx_take_9];
				seq[fx_take_9] = kernel_seq_stamp(ss[fx_take_9], false, 4);
				kernel_synch(KERNEL_SERIES, seq[fx_take_9], KERNEL_SERIES, temp);
				kernel_seq_player(seq[fx_take_9], false);
				sound_play(N_DoorHandle002);
				kernel_timing_trigger(QUARTER_SECOND, 2);
				goto handled;
				break;

			case 2:
				kernel_seq_delete(seq[fx_take_9]);
				seq[fx_take_9] = kernel_seq_backward(ss[fx_take_9], false, 5, 0, 0, 1);
				kernel_seq_range(seq[fx_take_9], 1, 4);
				kernel_seq_player(seq[fx_take_9], false);
				kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, 3);
				goto handled;
				break;

			case 3:
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_9]);
				player.walker_visible   = true;
				player.commands_allowed = true;
				if (player_said_1(unlock) || player_said_1(lock)) {
					text_show(text_205_28);
					/* the key does not work on this door */
				} else {
					text_show(text_205_27);
					/* the door is locked */
				}
				goto handled;
				break;
			}

		} else if (((player_said_1(box_five)) && ((global[doors_in_205] == BOTH_OPEN) || (global[doors_in_205] == LEFT_OPEN)))
		                                         ||
		 ((player_said_1(box_nine)) && ((global[doors_in_205] == BOTH_OPEN) || (global[doors_in_205] == RIGHT_OPEN)))) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take_9] = kernel_seq_pingpong(ss[fx_take_9], false, 5, 0, 0, 2);
				kernel_seq_range(seq[fx_take_9], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_player(seq[fx_take_9], true);
				kernel_seq_trigger(seq[fx_take_9],
				                   KERNEL_TRIGGER_EXPIRE, 0, 2);
				kernel_seq_trigger(seq[fx_take_9],
				                   KERNEL_TRIGGER_SPRITE, 4, ROOM_205_DOOR_OPENS);
				goto handled;
				break;

			case ROOM_205_DOOR_OPENS:
				sound_play(N_DoorOpens);
				if (player_said_1(box_five)) {
					kernel_seq_delete(seq[fx_left_door]);
					seq[fx_left_door] = kernel_seq_forward(ss[fx_left_door], false, 8, 0, 0, 1);
					kernel_seq_depth(seq[fx_left_door], 14);
					kernel_seq_range(seq[fx_left_door], KERNEL_FIRST, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_left_door],
					       KERNEL_TRIGGER_EXPIRE, 0, ROOM_205_DOOR_OPENS + 1);

				} else if (player_said_1(box_nine)) {
					kernel_seq_delete(seq[fx_right_door]);
					seq[fx_right_door] = kernel_seq_forward(ss[fx_right_door], false, 8, 0, 0, 1);
					kernel_seq_depth(seq[fx_right_door], 14);
					kernel_seq_range(seq[fx_right_door], KERNEL_FIRST, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_right_door],
					       KERNEL_TRIGGER_EXPIRE, 0, ROOM_205_DOOR_OPENS + 1);
				}
				goto handled;
				break;

			case ROOM_205_DOOR_OPENS + 1:
				if (player_said_1(box_five)) {
					temp = seq[fx_left_door];
					seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, KERNEL_LAST);
					kernel_synch(KERNEL_SERIES, seq[fx_left_door], KERNEL_SERIES, temp);
					kernel_seq_depth(seq[fx_left_door], 14);

				} else if (player_said_1(box_nine)) {
					temp = seq[fx_right_door];
					seq[fx_right_door] = kernel_seq_stamp(ss[fx_right_door], false, KERNEL_LAST);
					kernel_synch(KERNEL_SERIES, seq[fx_right_door], KERNEL_SERIES, temp);
					kernel_seq_depth(seq[fx_right_door], 14);
				}
				goto handled;
				break;

			case 2:
				player.walker_visible = true;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_9]);
				if (player_said_1(box_five)) {
					player_walk(BEHIND_LEFT_X, BEHIND_LEFT_Y, FACING_NORTH);
					player_walk_trigger(3);

				} else if (player_said_1(box_nine)) {
					player_walk(BEHIND_RIGHT_X, BEHIND_RIGHT_Y, FACING_NORTH);
					player_walk_trigger(3);
				}
				goto handled;
				break;

			case 3:
				if (player_said_1(box_five)) {
					new_room = 206;
					global[madame_giry_loc] = MIDDLE;

				} else if (player_said_1(box_nine)) {
					new_room = 207;
					global[madame_giry_loc] = MIDDLE;
				}
				goto handled;
				break;
			}
		}
	}

	if (player_said_2(talk_to, Monsieur_Richard)) {
		conv_run(CONV_RICHARD_18);
		goto handled;
	}

	if ((player_said_2(talk_to, Madame_Giry)) ||
	    (player_said_2(talk_to, woman)) ||
	    (player_said_3(give, ticket, Madame_Giry))) {
		if (global[ticket_people_here] == USHER_AND_SELLER) {
			if ((global[doors_in_205] == RIGHT_OPEN) || (global[doors_in_205] == BOTH_OPEN)) {
				if (global[madame_giry_loc] == RIGHT) {
					conv_run(CONV_GIRY_11);
					conv_export_value(player_has(ticket));
					conv_export_value(1);

					if (player_said_1(give)) {
						conv_export_value(1);
					} else {
						conv_export_value(0);
					}

					/* performance is about to begin, put box on left */
				} else {
					conv_run(CONV_GIRY_11);
					conv_export_value(player_has(ticket));
					conv_export_value(2);

					if (player_said_1(give)) {
						conv_export_value(1);
					} else {
						conv_export_value(0);
					}

					/* performance is about to begin, put box on right */
				}
			} else {
				conv_run(CONV_GIRY_11);
				conv_export_value(player_has(ticket));
				conv_export_value(0);

				if (player_said_1(give)) {
					conv_export_value(1);
				} else {
					conv_export_value(0);
				}
			}
			goto handled;

		} else {

			if (player_has(red_frame))    ++frame_counter;
			if (player_has(green_frame))  ++frame_counter;
			if (player_has(yellow_frame)) ++frame_counter;
			if (player_has(blue_frame))   ++frame_counter;

			conv_run(CONV_GIRY_10);
			conv_export_pointer(&global[player_score]);
			conv_export_value(game.difficulty);

			if (frame_counter > 2) {
				conv_export_value(1);
			} else {
				conv_export_value(0);
			}

			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
			goto handled;
		}
	}

	if (player.look_around) {
		text_show(text_205_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(box_six)) {
			text_show(text_205_11);
			goto handled;
		}

		if (player_said_1(box_seven)) {
			text_show(text_205_12);
			goto handled;
		}

		if (player_said_1(box_eight)) {
			text_show(text_205_13);
			goto handled;
		}

		if (player_said_1(box_nine)) {
			if ((global[doors_in_205] == BOTH_LOCKED) ||
			    (global[doors_in_205] == LEFT_OPEN)) {
				text_show(text_205_16);
			} else {
				text_show(text_205_17);
			}
			goto handled;
		}

		if (player_said_1(box_five)) {
			if ((global[doors_in_205] == BOTH_LOCKED) ||
			    (global[doors_in_205] == RIGHT_OPEN)) {
				text_show(text_205_14);
			} else {
				text_show(text_205_15);
			}
			goto handled;
		}

		if (player_said_1(floor)) {
			text_show(text_205_18);
			goto handled;
		}

		if (player_said_1(marble_column)) {
			text_show(text_205_19);
			goto handled;
		}

		if (player_said_1(ceiling)) {
			text_show(text_205_20);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_205_21);
			goto handled;
		}

		if (player_said_1(bust)) {
			text_show(text_205_22);
			goto handled;
		}

		if (player_said_1(carpet)) {
			text_show(text_205_23);
			goto handled;
		}

		if (player_said_1(grand_foyer)) {
			text_show(text_205_24);
			goto handled;
		}

		if ((player_said_1(woman)) ||
		    (player_said_1(Madame_Giry))) {
			text_show(text_205_25);
			goto handled;
		}

		if (player_said_1(Monsieur_Richard)) {
			text_show(text_205_26);
			goto handled;
		}
	}

	if (player_said_2(talk_to, bust)) {
		text_show(text_205_29);
		goto handled;
	}

	if ((player_said_2(open, box_ten)) || (player_said_2(enter, box_ten)) ||
	    (player_said_2(look, box_ten))) {
		text_show(text_205_13);
		goto handled;
	}

	if (player_said_1(take)) {
		if (player_said_1(woman) | player_said_1(Madame_Giry)) {
			text_show(text_205_30);
			goto handled;
		}
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_205_init() {
	int dynamic_giry;    /* for dynamic HS's of Giry */
	int frame_counter = 0;

	kernel.disable_fastwalk = true;

	if (global[jacques_status] != JACQUES_IS_DEAD) {
		rail_disconnect_node(6);
		rail_disconnect_node(7);
		rail_disconnect_node(8);
		rail_disconnect_node(9);
		/* these rail nodes are only needed if Richard is in this room (after kidnapping) */
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->just_did_option = false;
		local->anim_0_running  = false;
		local->anim_1_running  = false;
	}

	local->converse_counter = 0;
	local->no_hold          = false;
	local->give_ticket      = false;

	/* Load sprite series */

	ss[fx_left_door]  = kernel_load_series(kernel_name('f', 0), false);
	ss[fx_right_door] = kernel_load_series(kernel_name('f', 1), false);
	ss[fx_take_9]     = kernel_load_series("*RDR_9", false);


	kernel_flip_hotspot(words_Monsieur_Richard, false);
	kernel_flip_hotspot(words_Madame_Giry, false);
	kernel_flip_hotspot(words_woman, false);

	/* Load conversations */

	conv_get(CONV_RICHARD_18);
	conv_get(CONV_GIRY_10);
	conv_get(CONV_GIRY_11);


	if (global[current_year] == 1881) {
		if ((global[madame_giry_shows_up]) && (global[jacques_status] == JACQUES_IS_ALIVE)) {
			aa[1]                 = kernel_run_animation(kernel_name('g', 1), 1);
			local->anim_1_running = true;
			local->giry_action    = CONV10_GIRY_SHUT_UP;

			dynamic_giry = kernel_add_dynamic(words_Madame_Giry, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
			                                  0, 0, 0, 0);
			kernel_dynamic_hot[dynamic_giry].prep = PREP_ON;
			kernel_dynamic_walk(dynamic_giry, DYN_GIRY_X_L, DYN_GIRY_Y_L, FACING_NORTHWEST);
			kernel_dynamic_anim(dynamic_giry, aa[1], 1);
			kernel_dynamic_anim(dynamic_giry, aa[1], 2);

			switch (global[madame_giry_loc]) {
			case LEFT:
				kernel_reset_animation(aa[1], 138); /* freeze Giry by left door */
				kernel_flip_hotspot_loc(words_Madame_Giry, true, HS_MADAME_X_L_1, HS_MADAME_Y_L_1);
				kernel_flip_hotspot_loc(words_Madame_Giry, true, HS_MADAME_X_L_2, HS_MADAME_Y_L_2);
				break;

			case MIDDLE:
				if (global[madame_name_is_known] >= YES) {
					kernel_flip_hotspot_loc(words_Madame_Giry, true, HS_MADAME_X_M_1, HS_MADAME_Y_M_1);
					kernel_flip_hotspot_loc(words_Madame_Giry, true, HS_MADAME_X_M_2, HS_MADAME_Y_M_2);

				} else {
					kernel_flip_hotspot(words_woman, true);
				}
				break;

			case RIGHT:
				kernel_reset_animation(aa[1], 273); /* freeze Giry by right door */
				kernel_flip_hotspot_loc(words_Madame_Giry, true, HS_MADAME_X_R_1, HS_MADAME_Y_R_1);
				kernel_flip_hotspot_loc(words_Madame_Giry, true, HS_MADAME_X_R_2, HS_MADAME_Y_R_2);
				break;
			}
		}

		if (previous_room == KERNEL_RESTORING_GAME) {
			if (conv_restore_running == CONV_GIRY_10) {

				if (player_has(red_frame))    ++frame_counter;
				if (player_has(green_frame))  ++frame_counter;
				if (player_has(yellow_frame)) ++frame_counter;
				if (player_has(blue_frame))   ++frame_counter;

				conv_run(CONV_GIRY_10);
				conv_export_pointer(&global[player_score]);
				conv_export_value(game.difficulty);

				if (frame_counter > 2) {
					conv_export_value(1);
				} else {
					conv_export_value(0);
				}

				global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
				local->no_hold = true;

				if (local->giry_action == CONV10_GIRY_TRANCE) {
					kernel_reset_animation(aa[1], 66);
				} else {
					local->giry_action = CONV10_GIRY_SHUT_UP;
				}

			} else if (conv_restore_running == CONV_GIRY_11) {

				conv_run(CONV_GIRY_11);
				conv_export_value(player_has(ticket));
				conv_export_value(0);
				conv_export_value(0);

				global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
			}
		}
	}


	if (previous_room == KERNEL_RESTORING_GAME) {

		seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 1);
		kernel_seq_depth(seq[fx_left_door], 14);

		seq[fx_right_door] = kernel_seq_stamp(ss[fx_right_door], false, 1);
		kernel_seq_depth(seq[fx_right_door], 14);

		if (global[jacques_status] == JACQUES_IS_DEAD) {
			aa[0]                 = kernel_run_animation(kernel_name('b', 9), 1);
			local->anim_0_running = true;
			local->rich_action    = CONV18_RICH_SHUT_UP;
			kernel_flip_hotspot(words_Monsieur_Richard, true);

			if (conv_restore_running == CONV_RICHARD_18) {
				global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
				local->rich_action      = CONV18_RICH_SHUT_UP;
				conv_run(CONV_RICHARD_18);
				kernel_reset_animation(aa[0], 1); /* shut Richard up */
			}
		}


	/* Previous Rooms */

	} else if (previous_room == 206) {
		player.x      = BEHIND_LEFT_X;
		player.y      = BEHIND_LEFT_Y;
		player.facing = FACING_SOUTH;
		player_walk(FRONT_5_X + 4, FRONT_5_Y, FACING_SOUTH);
		player_walk_trigger(ROOM_205_LEFT_DOOR_CLOSES);
		player.commands_allowed = false;
		seq[fx_right_door] = kernel_seq_stamp(ss[fx_right_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_right_door], 14);
		seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_left_door], 14);
		if (global[jacques_status] == JACQUES_IS_DEAD) {
			aa[0]                 = kernel_run_animation(kernel_name('b', 9), 1);
			local->anim_0_running = true;
			local->rich_action    = CONV18_RICH_SHUT_UP;
			kernel_flip_hotspot(words_Monsieur_Richard, true);
		}

	} else if (previous_room == 207) {
		player.x      = BEHIND_RIGHT_X;
		player.y      = BEHIND_RIGHT_Y;
		player.facing = FACING_SOUTH;
		player_walk(FRONT_9_X + 4, FRONT_9_Y, FACING_SOUTH);
		player_walk_trigger(ROOM_205_RIGHT_DOOR_CLOSES);
		player.commands_allowed = false;
		seq[fx_right_door] = kernel_seq_stamp(ss[fx_right_door], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_right_door], 14);
		seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_left_door], 14);

	} else if (previous_room == 150) {
		aa[0]                      = kernel_run_animation(kernel_name('b', 9), 1);
		local->anim_0_running      = true;
		local->rich_action         = CONV18_RICH_SHUT_UP;
		player.x                   = BRIE_X;
		player.y                   = BRIE_Y;
		player.facing              = FACING_NORTHWEST;
		global[doors_in_205]       = LEFT_OPEN;
		global[ticket_people_here] = NEITHER;
		global[walker_converse]    = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		seq[fx_right_door]         = kernel_seq_stamp(ss[fx_right_door], false, KERNEL_FIRST);
		seq[fx_left_door]          = kernel_seq_stamp(ss[fx_left_door],  false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_right_door], 14);
		kernel_seq_depth(seq[fx_left_door],  14);
		kernel_flip_hotspot(words_Monsieur_Richard, true);
		conv_run(CONV_RICHARD_18);

	} else if ((previous_room == 202) || (previous_room != KERNEL_RESTORING_GAME)) {

		if (global[jacques_status] == JACQUES_IS_DEAD) {
			aa[0]                 = kernel_run_animation(kernel_name('b', 9), 1);
			local->anim_0_running = true;
			local->rich_action    = CONV18_RICH_SHUT_UP;
			kernel_flip_hotspot(words_Monsieur_Richard, true);
		}

		seq[fx_left_door] = kernel_seq_stamp(ss[fx_left_door], false, 1);
		kernel_seq_depth(seq[fx_left_door], 14);

		seq[fx_right_door] = kernel_seq_stamp(ss[fx_right_door], false, 1);
		kernel_seq_depth(seq[fx_right_door], 14);

		player_first_walk(OFF_SCREEN_X_FROM_202, OFF_SCREEN_Y_FROM_202, FACING_EAST,
		                  WALK_TO_X_FROM_202, WALK_TO_Y_FROM_202, FACING_NORTHEAST, true);
	}

	section_2_music();
}

void room_205_preload() {
	room_init_code_pointer = room_205_init;
	room_pre_parser_code_pointer = room_205_pre_parser;
	room_parser_code_pointer = room_205_parser;
	room_daemon_code_pointer = room_205_daemon;

	section_2_walker();
	section_2_interface();

	if (global[current_year] == 1881) {
		if (global[jacques_status] == JACQUES_IS_DEAD) {
			kernel_initial_variant = 3;

		} else if (global[jacques_status] == JACQUES_IS_ALIVE) {
			switch (global[madame_giry_loc]) {
			case LEFT:
				kernel_initial_variant = 2;
				break;

			case MIDDLE:
				kernel_initial_variant = 1;
				break;
			}
		}
	}
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
