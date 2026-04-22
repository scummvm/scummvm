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
#include "mads/madsv2/phantom/rooms/room204.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

static void handle_animation_brie() {
	int random = 0;
	int brie_reset_frame;

	if (kernel_anim[aa[0]].frame != local->brie_frame) {
		local->brie_frame = kernel_anim[aa[0]].frame;
		brie_reset_frame  = -1;

		switch (local->brie_frame) {
		case 80:  /* somewhere into Brie's leaving */
			conv_release();
			local->raoul_action = CONV22_RAOUL_SIT;
			break;

		case 173:
			player.commands_allowed = true;
			text_show(text_204_30);
			player.commands_allowed = false;
			break;

		case 174: /* end of leave */
			local->raoul_action = CONV22_RAOUL_GET_BOOK;
			brie_reset_frame    = 173;
			break;

		case 1:   /* end of freeze             */
		case 22:  /* end point                 */
		case 49:  /* end of put right arm up   */
		case 7:   /* end of talk 1             */
		case 13:  /* end of talk 2             */
		case 33:  /* end of talk 3             */
		case 61:  /* end of talk 4             */

			switch (local->brie_action) {
			case CONV22_BRIE_TALK:
				random             = imath_random(1, 4);
				local->brie_action = CONV22_BRIE_SHUT_UP;
				break;

			case CONV22_BRIE_POINT:
				random = 5;
				break;

			case CONV22_BRIE_LEAVE:
				random = 6;
				break;

			case CONV22_BRIE_RIGHT_UP:
				random = 7;
				break;

			default:
				random = 8;
				break;
			}

			switch (random) {
			case 1:
				brie_reset_frame   = 1;
				local->brie_action = CONV22_BRIE_SHUT_UP;
				break; /* do talk 1 */

			case 2:
				brie_reset_frame   = 7;
				local->brie_action = CONV22_BRIE_SHUT_UP;
				break; /* do talk 2 */

			case 3:
				brie_reset_frame   = 22;
				local->brie_action = CONV22_BRIE_SHUT_UP;
				break; /* do talk 3 */

			case 4:
				brie_reset_frame   = 49;
				local->brie_action = CONV22_BRIE_SHUT_UP;
				break; /* do talk 4 */

			case 5:
				brie_reset_frame   = 13;
				local->brie_action = CONV22_BRIE_SHUT_UP;
				break; /* point */

			case 6:
				brie_reset_frame = 61;
				break; /* leave */

			default:
				brie_reset_frame = 0;
				break; /* freeze sitting still */
			}
			break;
		}

		if (brie_reset_frame >= 0) {
			kernel_reset_animation(aa[0], brie_reset_frame);
			local->brie_frame = brie_reset_frame;
		}
	}
}

static void handle_animation_flor() {
	int random = 0;
	int flor_reset_frame;

	if (kernel_anim[aa[1]].frame != local->flor_frame) {
		local->flor_frame = kernel_anim[aa[1]].frame;
		flor_reset_frame  = -1;

		switch (local->flor_frame) {
		case 80:  /* just as Raoul turns invisible, make him visible with aa[2] */
			kernel_reset_animation(aa[2], 1);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, aa[1]);
			local->raoul_action = CONV22_RAOUL_SHUT_UP;
			break;

		case 86:  /* a few frames just after kiss (where Florent leaves) */
			conv_release();
			break;

		case 173: /* end of leave */
			flor_reset_frame = 172;
			break;

		case 1:   /* end of Raoul laying down 1 */
		case 2:   /* end of Raoul laying down 2 */
		case 3:   /* end of Raoul laying down 3 */
			if (local->keep_raoul_down) {
				random = imath_random(1, 1000);
				if (random < 300) {
					flor_reset_frame = 0;
				} else if (random < 600) {
					flor_reset_frame = 1;
				} else {
					flor_reset_frame = 2;
				}
			}
			break;

		case 21:  /* end of Raoul awaking          */
		case 180: /* end of Raoul glancing to case */
			conv_release();
			break;

		case 22:  /* end of freeze                 */
		case 50:  /* end of talk to Brie           */
		case 30:  /* end of talk to Raoul          */
		case 174: /* end of Raoul talk to Flor 1   */
		case 175: /* end of Raoul talk to Flor 2   */
		case 176: /* end of Raoul talk to Flor 3   */
		case 181: /* end of Raoul glancing to case */

			switch (local->flor_action) {
			case CONV22_FLOR_TALK_RAOUL:
				random             = 1;
				local->flor_action = CONV22_FLOR_SHUT_UP;
				break;

			case CONV22_FLOR_TALK_BRIE:
				random             = 2;
				local->flor_action = CONV22_FLOR_SHUT_UP;
				break;

			case CONV22_FLOR_LEAVE:
				random = 3;
				break;

			case CONV22_RAOUL_GLANCE:
				random             = 4;
				local->flor_action = CONV22_FLOR_SHUT_UP;
				break;

			case CONV22_FLOR_RAOUL_TALK:
				random = imath_random(5, 7);
				++local->raoul_talk_count;
				if (local->raoul_talk_count > 17) {
					local->flor_action = CONV22_FLOR_SHUT_UP;
					random = 8;
				}
				break;

			default:
				random = 7;
				break;
			}

			switch (random) {
			case 1:
				flor_reset_frame = 22;
				break; /* do talk to Raoul */

			case 2:
				flor_reset_frame = 30;
				break; /* do talk to Brie */

			case 3:
				flor_reset_frame = 53;
				break; /* leave */

			case 4:
				flor_reset_frame = 176;
				break; /* Raoul glance over to case */

			case 5:
				flor_reset_frame = 173;
				break; /* Raoul talk to Flor 1 */

			case 6:
				flor_reset_frame = 174;
				break; /* Raoul talk to Flor 2 */

			case 7:
				flor_reset_frame = 175;
				break; /* Raoul talk to Flor 3 */

			default:
				flor_reset_frame = 21;
				break; /* freeze Florent and Raoul */
			}
			break;
		}

		if (flor_reset_frame >= 0) {
			kernel_reset_animation(aa[1], flor_reset_frame);
			local->flor_frame = flor_reset_frame;
		}
	}
}

static void handle_animation_raoul() {
	int random = 0;
	int raoul_reset_frame;

	if (kernel_anim[aa[2]].frame != local->raoul_frame) {
		local->raoul_frame = kernel_anim[aa[2]].frame;
		raoul_reset_frame  = -1;

		switch (local->raoul_frame) {
		case 213:
			player.commands_allowed = true;
			object_examine(book, text_204_31, 0);
			player.commands_allowed = false;
			break;

		case 229:
			player.commands_allowed = true;
			object_examine(book, text_204_32, 0);
			player.commands_allowed = false;
			break;

		case 237:
			kernel_abort_animation(aa[1]);
			kernel_abort_animation(aa[0]);
			local->anim_0_running = false;
			local->anim_1_running = false;
			local->anim_3_running = true;
			aa[3] = kernel_run_animation(kernel_name('e', 1), 0);
			global_speech_load(speech_phantom_cackle);
			break;

		case 253: /* end of getting book */
			raoul_reset_frame = 244;
			break;

		case 114: /* just took book — delete book */
			kernel_seq_delete(seq[fx_book]);
			inter_give_to_player(book);
			break;

		case 1:   /* end of invisible */
			if (local->raoul_action == CONV22_RAOUL_INVISIBLE) {
				raoul_reset_frame = 0;
			}
			break;

		case 31:
			if (local->raoul_action == CONV22_RAOUL_GET_BOOK) {
				raoul_reset_frame = 33;
			} else {
				raoul_reset_frame = 30;
			}
			break;

		case 33:
			raoul_reset_frame = 32;
			break;

		case 257: /* almost at end of glance */
			conv_release();
			break;

		case 10:  /* end of freeze (visible) */
		case 14:  /* end of talk 1           */
		case 20:  /* end of talk 2           */
		case 258: /* end of glance           */

			switch (local->raoul_action) {
			case CONV22_RAOUL_TALK:
				random              = imath_random(1, 2);
				local->raoul_action = CONV22_RAOUL_SHUT_UP;
				break;

			case CONV22_RAOUL_SIT:
				random = 3;
				break;

			case CONV22_RAOUL_GLANCE:
				random = 4;
				break;

			default:
				random = 5;
				break;
			}

			switch (random) {
			case 1:
				raoul_reset_frame   = 10;
				local->raoul_action = CONV22_RAOUL_SHUT_UP;
				break; /* do talk 1 */

			case 2:
				raoul_reset_frame   = 14;
				local->raoul_action = CONV22_RAOUL_SHUT_UP;
				break; /* do talk 2 */

			case 3:
				raoul_reset_frame = 20;
				break; /* sit (put both feet on floor) */

			case 4:
				raoul_reset_frame   = 253;
				local->raoul_action = CONV22_RAOUL_SHUT_UP;
				break; /* glance over to case */

			default:
				raoul_reset_frame = 9;
				break; /* freeze laying up */
			}
			break;
		}

		if (raoul_reset_frame >= 0) {
			kernel_reset_animation(aa[2], raoul_reset_frame);
			local->raoul_frame = raoul_reset_frame;
		}
	}
}

static void handle_animation_end() {
	if ((kernel_anim[aa[3]].frame == 15) && (!local->prevent_3)) {
		global_speech_go(speech_phantom_cackle);
		local->prevent_3 = true;
	}

	if ((kernel_anim[aa[3]].frame == 26) && (!local->prevent_2)) {
		kernel_timing_trigger(FIVE_SECONDS, ROOM_204_BYE);
		kernel_message_add(quote_string(kernel.quotes, quote_204a0),
		                   123, 137, MESSAGE_COLOR, SIX_SECONDS, 0, 0);
		local->prevent_2 = true;
	}

	if (kernel_anim[aa[3]].frame == 27) {
		kernel_reset_animation(aa[3], 12);
	}
}

void room_204_daemon() {
	if (local->anim_0_running) {
		handle_animation_brie();
	}

	if (local->anim_1_running) {
		handle_animation_flor();
	}

	if (local->anim_2_running) {
		handle_animation_raoul();
	}

	if (local->anim_3_running) {
		handle_animation_end();
	}

	if (kernel.trigger == ROOM_204_BYE) {
		new_room = 250;
	}

	if ((conv_control.running != CONV_END_22) && (!local->prevent) && (local->end_of_game)) {
		player.commands_allowed = false;
		local->prevent          = true;
	}

	switch (kernel.trigger) {
	case ROOM_204_DOOR_CLOSES:
		seq[fx_door] = kernel_seq_backward(ss[fx_door], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_door], 10);
		kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_204_DOOR_CLOSES + 1);
		break;

	case ROOM_204_DOOR_CLOSES + 1:
		sound_play(N_DoorCloses);
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 5);
		player.commands_allowed = true;
		break;
	}
}

static void process_conversation_22() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	switch (player_verb) {
	case conv022_resolution_florent:
	case conv022_resolution_fall:
	case conv022_resolution_alone:
		if (local->keep_raoul_down) {
			conv_hold();
			local->keep_raoul_down = false;
		}
		break;

	case conv022_kiss_b_b:
		if (!kernel.trigger) {
			local->flor_action     = CONV22_FLOR_LEAVE;
			local->florent_is_gone = true;
			you_trig_flag          = true;
			me_trig_flag           = true;
			conv_hold();
		}
		break;

	case conv022_leave_b_b:
		you_trig_flag = true;
		me_trig_flag  = true;
		if (!kernel.trigger) {
			local->brie_action = CONV22_BRIE_LEAVE;
			conv_hold();
		}
		break;

	case conv022_notes_b_b:
		if (!kernel.trigger) {
			local->raoul_action = CONV22_RAOUL_GLANCE;
			local->flor_action  = CONV22_RAOUL_GLANCE;
			you_trig_flag       = true;
			me_trig_flag        = true;
			conv_hold();
		}
		break;
	}

	switch (kernel.trigger) {
	case ROOM_204_YOU_TALK:
		if (local->florent_is_gone) {
			if ((player_verb != conv022_kiss_abc) &&
			    (player_verb != conv022_look_abc)) {
				local->brie_action = CONV22_BRIE_TALK;
			}

		} else {
			switch (player_verb) {
			case conv022_second_next:
			case conv022_resolution_fall:
			case conv022_resolution_alone:
			case conv022_dead_strangled:
			case conv022_object_abc:
			case conv022_do_abc:
			case conv022_notes_hadany:
			case conv022_notes_here:
			case conv022_notes_chandelier:
			case conv022_notes_audience:
				local->brie_action = CONV22_BRIE_TALK;
				break;

			default:
				local->flor_action = CONV22_FLOR_TALK_RAOUL;
				break;
			}
		}
		break;

	case ROOM_204_ME_TALK:
		if (local->florent_is_gone) {
			if (local->raoul_action != CONV22_RAOUL_SIT) {
				local->raoul_action = CONV22_RAOUL_TALK;
			}
		} else {
			local->flor_action = CONV22_FLOR_RAOUL_TALK;
		}
		break;
	}


	/* ================= Set up me and you triggers ================ */

	if (!me_trig_flag) {
		if (!local->keep_raoul_down) {
			conv_me_trigger(ROOM_204_ME_TALK);
		}
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_204_YOU_TALK);
	}

	local->raoul_talk_count = 0;
}

void room_204_pre_parser() {
	if (player_said_2(look, bookcase)) {
		player.need_to_walk = true;
	}

	if (player_said_2(look, book) && object_is_here(book)) {
		player.need_to_walk = true;
	}

	if (player_said_2(open, door)) {
		player_walk(DOOR_X, DOOR_Y, FACING_WEST);
	}
}

void room_204_parser() {
	if (conv_control.running == CONV_END_22) {
		process_conversation_22();
		goto handled;
	}

	if (player_said_2(walk_through, door) || player_said_2(open, door)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			seq[fx_take_6] = kernel_seq_pingpong(ss[fx_take_6], true, 5, 0, 0, 2);
			kernel_seq_range(seq[fx_take_6], 1, 4);
			kernel_seq_player(seq[fx_take_6], true);
			kernel_seq_trigger(seq[fx_take_6], KERNEL_TRIGGER_SPRITE, 4, ROOM_204_DOOR_OPENS);
			kernel_seq_trigger(seq[fx_take_6], KERNEL_TRIGGER_EXPIRE, 0, ROOM_204_DOOR_OPENS + 1);
			break;

		case ROOM_204_DOOR_OPENS:
			sound_play(N_DoorOpens);
			kernel_seq_delete(seq[fx_door]);
			seq[fx_door] = kernel_seq_forward(ss[fx_door], false, 8, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 14);
			kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
			break;

		case ROOM_204_DOOR_OPENS + 1:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_6]);
			player_walk(WALK_TO_X_BEHIND_DOOR, WALK_TO_Y_BEHIND_DOOR, FACING_WEST);
			player_walk_trigger(ROOM_204_DOOR_OPENS + 2);
			break;

		case ROOM_204_DOOR_OPENS + 2:
			new_room = 203;
			break;
		}
		goto handled;
	}

	if (player_said_2(take, book) &&
	    (object_is_here(book) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			seq[fx_reach_up_7] = kernel_seq_pingpong(ss[fx_reach_up_7], false, 5, 0, 0, 2);
			kernel_seq_range(seq[fx_reach_up_7], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_player(seq[fx_reach_up_7], true);
			kernel_seq_trigger(seq[fx_reach_up_7], KERNEL_TRIGGER_SPRITE, 8, 1);
			kernel_seq_trigger(seq[fx_reach_up_7], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			kernel_seq_delete(seq[fx_book]);
			kernel_flip_hotspot(words_book, false);
			inter_give_to_player(book);
			sound_play(N_TakeObjectSnd);
			break;

		case 2:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_reach_up_7]);
			player.walker_visible = true;
			kernel_timing_trigger(20, 3);
			break;

		case 3:
			object_examine(book, text_008_15, 0);
			global[read_book]       = true;
			global[player_score]    += 5;
			player.commands_allowed = true;
			break;
		}
		goto handled;
	}

	if (player_said_2(exit_to, grand_foyer)) {
		new_room = 202;
		goto handled;
	}

	if (player.look_around) {
		text_show(text_204_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(wall)) {
			text_show(text_204_11);
			goto handled;
		}

		if (player_said_1(floor)) {
			text_show(text_204_12);
			goto handled;
		}

		if (player_said_1(rug)) {
			text_show(text_204_13);
			goto handled;
		}

		if (player_said_1(glass_case)) {
			text_show(text_204_14);
			global[looked_at_case] = true;
			goto handled;
		}

		if (player_said_1(ceiling)) {
			if (global[sandbag_status] == SANDBAG_SECURE) {
				text_show(text_204_29);
			} else {
				text_show(text_204_16);
			}
			goto handled;
		}

		if (player_said_1(bookcase)) {
			if ((global[can_find_book_library]) && (global[current_year] == 1993)) {
				if ((inter_point_x < 46) && (!player_has(book))) {
					if (!global[scanned_bookcase]) {
						text_show(text_204_33);
						kernel_flip_hotspot(words_book, true);
						global[scanned_bookcase] = true;
					} else {
						text_show(text_204_37);
					}
				} else {
					text_show(text_204_17);
				}
			} else {
				text_show(text_204_17);
			}
			goto handled;
		}

		if (player_said_1(sofa)) {
			text_show(text_204_18);
			goto handled;
		}

		if (player_said_1(end_table)) {
			text_show(text_204_19);
			goto handled;
		}

		if (player_said_1(lamp)) {
			text_show(text_204_20);
			goto handled;
		}

		if (player_said_1(bust)) {
			text_show(text_204_21);
			goto handled;
		}

		if (player_said_1(coffee_table)) {
			text_show(text_204_22);
			goto handled;
		}

		if (player_said_1(comfy_chair)) {
			text_show(text_204_23);
			goto handled;
		}

		if (player_said_1(decorative_vase)) {
			text_show(text_204_24);
			goto handled;
		}

		if (player_said_1(painting)) {
			text_show(text_204_25);
			goto handled;
		}

		if (player_said_1(grand_foyer)) {
			text_show(text_204_26);
			goto handled;
		}

		if (player_said_1(door)) {
			text_show(text_204_27);
			goto handled;
		}

		if (player_said_1(window)) {
			text_show(text_204_28);
			goto handled;
		}

		if (player_said_1(book) && object_is_here(book)) {
			text_show(text_204_34);
			goto handled;
		}
	}

	if (player_said_2(talk_to, bust)) {
		text_show(text_204_36);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_204_init() {
	int coffee_table;   /* Dynamic for coffee table / object iterator */
	int chair;          /* Dynamic for chair */
	int exit;           /* Dynamic for exits */

	local->prevent_3 = false;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running  = false;
		local->anim_1_running  = false;
		local->anim_2_running  = false;
		local->anim_3_running  = false;
		local->florent_is_gone = false;
		local->prevent         = false;
		local->prevent_2       = false;
		local->end_of_game     = false;
		local->keep_raoul_down = true;
	}

	if (global[ticket_people_here] == USHER_AND_SELLER) {
		global[make_rich_leave_203] = true;
	}

	if (global[right_door_is_open_504]) {
		local->end_of_game = true;
	}

	if (local->end_of_game) {
		if (room_loaded_walk) {
			buffer_free(&scr_walk);
		}
	}

	kernel_flip_hotspot(words_book, false);


	/* =================== Load conversations ====================== */

	conv_get(CONV_END_22);


	/* =================== Load Sprite Series ====================== */

	ss[fx_book]       = kernel_load_series(kernel_name('p',  0), false);
	ss[fx_door]       = kernel_load_series(kernel_name('x',  6), false);
	ss[fx_reach_up_7] = kernel_load_series("*RALRH_9", false);
	ss[fx_take_6]     = kernel_load_series("*RDRR_6",  false);


	/* =================== If book is here, stamp it =============== */

	if (object_is_here(book) || (global[current_year] == 1881) || (local->end_of_game)) {
		seq[fx_book] = kernel_seq_stamp(ss[fx_book], false, 1);
		kernel_seq_depth(seq[fx_book], 5);
		if ((global[scanned_bookcase]) && (global[current_year] == 1993)) {
			kernel_flip_hotspot(words_book, true);
		}
	}


	/* =================== Draw 1993 sprite ======================== */

	if ((global[current_year] == 1993) || local->end_of_game) {
		ss[fx_1993] = kernel_load_series(kernel_name('z', -1), false);
		kernel_draw_to_background(ss[fx_1993], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		matte_deallocate_series(ss[fx_1993], true);
		kernel_flip_hotspot(words_candle,       false);
		kernel_flip_hotspot(words_bust,         false);
		kernel_flip_hotspot(words_coffee_table, false);

		/* 1 dynamic for coffee_table */
		coffee_table = kernel_add_dynamic(words_coffee_table, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                    COFFEE_X, COFFEE_Y, COFFEE_X_SIZE, COFFEE_Y_SIZE);
		kernel_dynamic_walk(coffee_table, WALK_TO_COFFEE_X, WALK_TO_COFFEE_Y, FACING_SOUTHEAST);

		/* 2 dynamics for exits */
		exit = kernel_add_dynamic(words_grand_foyer, words_exit_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                    EXIT_1_X, EXIT_1_Y, EXIT_1_X_SIZE, EXIT_1_Y_SIZE);
		kernel_dynamic_walk(exit, WALK_TO_EXIT_1_X, WALK_TO_EXIT_1_Y, FACING_SOUTH);
		kernel_dynamic_cursor(exit, 3);

		exit = kernel_add_dynamic(words_grand_foyer, words_exit_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                    EXIT_2_X, EXIT_2_Y, EXIT_2_X_SIZE, EXIT_2_Y_SIZE);
		kernel_dynamic_walk(exit, WALK_TO_EXIT_2_X, WALK_TO_EXIT_2_Y, FACING_SOUTH);
		kernel_dynamic_cursor(exit, 3);

	} else {
		kernel_flip_hotspot(words_light,      false);
		kernel_flip_hotspot(words_glass_case, false);

		/* 3 dynamics for chairs */
		chair = kernel_add_dynamic(words_comfy_chair, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                    CHAIR_1_X, CHAIR_1_Y, CHAIR_1_X_SIZE, CHAIR_1_Y_SIZE);
		kernel_dynamic_walk(chair, WALK_TO_CHAIR_X, WALK_TO_CHAIR_Y, FACING_SOUTHEAST);

		chair = kernel_add_dynamic(words_comfy_chair, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                    CHAIR_2_X, CHAIR_2_Y, CHAIR_2_X_SIZE, CHAIR_2_Y_SIZE);
		kernel_dynamic_walk(chair, WALK_TO_CHAIR_X, WALK_TO_CHAIR_Y, FACING_SOUTHEAST);

		chair = kernel_add_dynamic(words_comfy_chair, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                    CHAIR_3_X, CHAIR_3_Y, CHAIR_3_X_SIZE, CHAIR_3_Y_SIZE);
		kernel_dynamic_walk(chair, WALK_TO_CHAIR_X, WALK_TO_CHAIR_Y, FACING_SOUTHEAST);

		/* 2 dynamics for exits */
		exit = kernel_add_dynamic(words_grand_foyer, words_exit_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                    EXIT_3_X, EXIT_3_Y, EXIT_3_X_SIZE, EXIT_3_Y_SIZE);
		kernel_dynamic_walk(exit, WALK_TO_EXIT_3_X, WALK_TO_EXIT_3_Y, FACING_SOUTH);
		kernel_dynamic_cursor(exit, 3);

		exit = kernel_add_dynamic(words_grand_foyer, words_exit_to, SYNTAX_SINGULAR, KERNEL_NONE,
		                    EXIT_4_X, EXIT_4_Y, EXIT_4_X_SIZE, EXIT_4_Y_SIZE);
		kernel_dynamic_walk(exit, WALK_TO_EXIT_4_X, WALK_TO_EXIT_4_Y, FACING_SOUTH);
		kernel_dynamic_cursor(exit, 3);
	}


	/* =================== Draw glass_case ========================= */

	if ((previous_room == 306) || (local->end_of_game)) {
		ss[fx_case] = kernel_load_series(kernel_name('f', 0), false);
		kernel_draw_to_background(ss[fx_case], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);

	} else {
		if (global[current_year] == 1993) {
			ss[fx_broken_case] = kernel_load_series(kernel_name('f', 1), false);
			kernel_draw_to_background(ss[fx_broken_case], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
			matte_deallocate_series(ss[fx_broken_case], true);
		}
	}


	/* =================== Previous Rooms ========================== */

	if (previous_room == KERNEL_RESTORING_GAME) {

		if (local->end_of_game) {

			kernel.quotes = quote_load(quote_204a0, 0);

			aa[2]                 = kernel_run_animation(kernel_name('r', 1), 0);
			local->anim_2_running = true;

			if (local->florent_is_gone) {
				kernel_reset_animation(aa[2], 9); /* put Raoul sitting up, talking to Brie */
				local->raoul_action = CONV22_RAOUL_SHUT_UP;
			} else {
				kernel_reset_animation(aa[2], 32); /* this is the invisible frame */
				local->raoul_action = CONV22_RAOUL_INVISIBLE;
			}

			aa[0]                 = kernel_run_animation(kernel_name('m', 1), 0);
			local->anim_0_running = true;
			local->brie_action    = CONV22_BRIE_SHUT_UP;

			aa[1]                 = kernel_run_animation(kernel_name('f', 1), 0);
			local->anim_1_running = true;
			local->flor_action    = CONV22_FLOR_SHUT_UP;

			if (local->florent_is_gone) {
				kernel_reset_animation(aa[1], 172); /* make Florent invisible */
			} else {
				if (!local->keep_raoul_down) {
					kernel_reset_animation(aa[1], 21); /* this will make Raoul sit up with Florent there */
				}
			}

			player.walker_visible = false;

			conv_run(CONV_END_22);
			conv_export_pointer(&global[player_score]);

		} else {
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 1);
			kernel_seq_depth(seq[fx_door], 5);
		}

	} else if (previous_room == 202) {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 5);
		if (global[current_year] == 1993) {
			player.x = PLAYER_X_FROM_202_1993;
			player.y = PLAYER_Y_FROM_202_1993;
		} else {
			player.x = PLAYER_X_FROM_202_1881;
			player.y = PLAYER_Y_FROM_202_1881;
		}
		player.facing = FACING_NORTHWEST;

	} else if (previous_room == 150) {
		/* Move all objects to inventory/room 1 for end-of-game */
		for (coffee_table = 0; coffee_table < num_objects; coffee_table++) {
			inter_move_object(coffee_table, 1);
		}

		kernel.quotes = quote_load(quote_204a0, 0);

		aa[2]                  = kernel_run_animation(kernel_name('r', 1), 0);
		local->anim_2_running  = true;
		local->raoul_action    = CONV22_RAOUL_INVISIBLE;
		kernel_reset_animation(aa[2], 32); /* this is the invisible frame */

		aa[0]                  = kernel_run_animation(kernel_name('m', 1), 0);
		local->anim_0_running  = true;
		local->brie_action     = CONV22_BRIE_SHUT_UP;

		aa[1]                  = kernel_run_animation(kernel_name('f', 1), 0);
		local->anim_1_running  = true;
		local->keep_raoul_down = true;
		local->flor_action     = CONV22_FLOR_SHUT_UP;

		player.walker_visible  = false;
		local->end_of_game     = true;

		conv_run(CONV_END_22);
		conv_export_pointer(&global[player_score]);

	} else if ((previous_room == 203) || (previous_room != KERNEL_RESTORING_GAME)) {
		player_first_walk(WALK_TO_X_BEHIND_DOOR - 10, WALK_TO_Y_BEHIND_DOOR, FACING_EAST,
		                  WALK_TO_X_FROM_203, WALK_TO_Y_FROM_203, FACING_EAST, true);
		player_walk_trigger(ROOM_204_DOOR_CLOSES);
		player.commands_allowed = false;
	}

	section_2_music();
}

void room_204_preload() {
	room_init_code_pointer       = room_204_init;
	room_pre_parser_code_pointer = room_204_pre_parser;
	room_parser_code_pointer     = room_204_parser;
	room_daemon_code_pointer     = room_204_daemon;

	if ((global[current_year] == 1993) || (global[right_door_is_open_504])) {
		kernel_initial_variant = 1;
	}

	section_2_walker();
	section_2_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
