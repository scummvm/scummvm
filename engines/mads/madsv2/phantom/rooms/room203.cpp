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
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/phantom/rooms/section2.h"
#include "mads/madsv2/phantom/rooms/room203.h"

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
		case 1:   /* end of freeze sitting upright and end talk 1 */
		case 2:   /* end talk 2 (sitting upright)                 */
		case 4:   /* end of nod (sitting upright)                 */
		case 11:  /* end of putting right arm down                */
		case 22:  /* end of putting both arms down                */
		case 31:  /* end of un-slouch 1                           */
		case 35:  /* end of un-slouch 2                           */
		case 29:  /* end of point                                 */

			random = imath_random(6, 45);

			switch (local->brie_action) {
			case CONV5_BRIE_TALK:
				random = imath_random(1, 2);
				++local->brie_talk_count;
				if (local->brie_talk_count > 20) {
					local->brie_action = CONV5_BRIE_SHUT_UP;
					random = 45;
				}
				break;

			case CONV5_BRIE_NOD:
				random = 3;
				local->brie_action = CONV5_BRIE_TALK;
				break;

			case CONV5_BRIE_POINT:
				random = 4;
				break;

			case CONV5_BRIE_TALK_HANDS_UP:
				random = 5;
				break;
			}

			switch (random) {
			case 1:
				brie_reset_frame = 0;
				break; /* do talk 1 (sitting upright) */

			case 2:
				brie_reset_frame = 1;
				break; /* do talk 2 (sitting upright) */

			case 3:
				brie_reset_frame = 2;
				break; /* do a nod */

			case 4:
				brie_reset_frame   = 22;
				local->brie_action = CONV5_BRIE_TALK;
				break; /* do a point */

			case 5:
				brie_reset_frame = 13;
				break; /* talk with hands up (new node) */

			case 6:
				brie_reset_frame = 4;
				break; /* put right arm up (new node) */

			case 7:
				brie_reset_frame = 29;
				break; /* do slouch 1 (new node) */

			case 8:
				brie_reset_frame = 31;
				break; /* do slouch 2 (new node) */

			default:
				brie_reset_frame = 0;
				break; /* freeze while sitting upright */
			}
			break;

		case 30:  /* end of coming to slouch 1 and freeze slouch 1 */

			random = imath_random(1, 45);

			switch (local->brie_action) {
			case CONV5_BRIE_TALK:
			case CONV5_BRIE_TALK_HANDS_UP:
			case CONV5_BRIE_POINT:
			case CONV5_BRIE_NOD:
				random = 1;
				break;
			}

			switch (random) {
			case 1:
				brie_reset_frame = 30;
				break; /* go to sit upright (new node) */

			default:
				brie_reset_frame = 29;
				break; /* freeze while slouch 1 */
			}
			break;

		case 8:   /* end of coming to right arm up and freeze */

			random = imath_random(1, 45);

			switch (local->brie_action) {
			case CONV5_BRIE_TALK:
			case CONV5_BRIE_TALK_HANDS_UP:
			case CONV5_BRIE_POINT:
			case CONV5_BRIE_NOD:
				random = 3;
				break;
			}

			switch (random) {
			case 1:
				brie_reset_frame = 11;
				break; /* go to look at fist (new node) */

			case 2:
				brie_reset_frame = 12;
				break; /* go to right arm way back */

			case 3:
				brie_reset_frame = 8;
				break; /* go to sit upright (new node) */

			default:
				brie_reset_frame = 7;
				break; /* freeze right arm up */
			}
			break;

		case 12:  /* end of look at fist freeze */

			random = imath_random(1, 45);

			switch (local->brie_action) {
			case CONV5_BRIE_TALK:
			case CONV5_BRIE_TALK_HANDS_UP:
			case CONV5_BRIE_POINT:
			case CONV5_BRIE_NOD:
				random = 1;
				break;
			}

			switch (random) {
			case 1:
				brie_reset_frame = 7;
				break; /* go to freeze right arm up (new node) */

			default:
				brie_reset_frame = 11;
				break; /* freeze look at fist */
			}
			break;

		case 13:  /* end of fist way back freeze */

			random = imath_random(1, 45);

			switch (local->brie_action) {
			case CONV5_BRIE_TALK:
			case CONV5_BRIE_TALK_HANDS_UP:
			case CONV5_BRIE_POINT:
			case CONV5_BRIE_NOD:
				random = 1;
				break;
			}

			switch (random) {
			case 1:
				brie_reset_frame = 7;
				break; /* go to freeze right arm up (new node) */

			default:
				brie_reset_frame = 12;
				break; /* freeze fist way back */
			}
			break;

		case 17:  /* end of coming to both hands up, freeze, & talk hands up 1 */
		case 18:  /* end of talk hands up 2                                    */

			switch (local->brie_action) {
			case CONV5_BRIE_TALK_HANDS_UP:
				random = imath_random(1, 2);
				++local->brie_talk_count;
				if (local->brie_talk_count > 20) {
					local->brie_action = CONV5_BRIE_SHUT_UP;
					random = 3;
				}
				break;

			case CONV5_BRIE_TALK:
			case CONV5_BRIE_POINT:
			case CONV5_BRIE_NOD:
				random = 3;
				break;
			}

			switch (random) {
			case 1:
				brie_reset_frame = 16;
				break; /* do talk 1 (hands up in air) */

			case 2:
				brie_reset_frame = 17;
				break; /* do talk 2 (hands up in air) */

			case 3:
				brie_reset_frame = 18;
				break; /* go to sit upright (new node) */
			}
			break;

		case 33:  /* end of coming to slouch 2, and freeze           */
		case 40:  /* end of coming from look at fist while slouching */

			random = imath_random(1, 45);

			switch (local->brie_action) {
			case CONV5_BRIE_TALK:
			case CONV5_BRIE_TALK_HANDS_UP:
			case CONV5_BRIE_POINT:
			case CONV5_BRIE_NOD:
				random = 1;
				break;
			}

			switch (random) {
			case 1:
				brie_reset_frame = 33;
				break; /* go to sit upright (new node) */

			case 2:
				brie_reset_frame = 35;
				break; /* go to look at fist while slouching (new node) */

			default:
				brie_reset_frame = 32;
				break; /* freeze while slouch 2 */
			}
			break;

		case 38:  /* end of coming to look at fist while slouch 2, and freeze */

			random = imath_random(1, 45);

			switch (local->brie_action) {
			case CONV5_BRIE_TALK:
			case CONV5_BRIE_TALK_HANDS_UP:
			case CONV5_BRIE_POINT:
			case CONV5_BRIE_NOD:
				random = 1;
				break;
			}

			switch (random) {
			case 1:
				brie_reset_frame = 38;
				break; /* go to slouch 2 look at fists (new node) */

			default:
				brie_reset_frame = 37;
				break; /* freeze while looking at fist slouch 2 */
			}
			break;
		}

		if (brie_reset_frame >= 0) {
			kernel_reset_animation(aa[0], brie_reset_frame);
			local->brie_frame = brie_reset_frame;
		}
	}
}

static void handle_animation_rich() {
	int random = 0;
	int rich_reset_frame;

	if (kernel_anim[aa[2]].frame != local->rich_frame) {
		local->rich_frame = kernel_anim[aa[2]].frame;
		rich_reset_frame  = -1;

		switch (local->rich_frame) {
		case 1:   /* end of freeze sitting upright and end talk 1           */
		case 2:   /* end talk 2 (sitting upright)                           */
		case 3:   /* end talk 3 (sitting upright)                           */
		case 25:  /* end of touch head                                      */
		case 37:  /* end of pushing his head way back                       */
		case 53:  /* end of putting both arms up                            */
		case 59:  /* end of point                                           */
		case 87:  /* end of looking at knuckle (similar to look right hand) */
		case 79:  /* end of left up don't know                              */
		case 69:  /* end of up                                              */
		case 67:  /* end of left                                            */
		case 63:  /* end of right                                           */
		case 15:  /* end of hand on chin                                    */
		case 7:   /* end of look at right hand                              */
		case 108: /* end of sitting down                                    */

			random = imath_random(8, 45);

			switch (local->rich_action) {
			case CONV8_RICH_TALK:
				random = imath_random(1, 3);
				++local->rich_talk_count;
				if (local->rich_talk_count > 20) {
					local->rich_action = CONV8_RICH_SHUT_UP;
					random = 45;
				}
				break;

			case CONV8_RICH_LEFT_UP_DONT_KNOW:
				random = 4;
				local->rich_action = CONV8_RICH_TALK;
				break;

			case CONV8_RICH_POINT:
				random = 5;
				local->rich_action = CONV8_RICH_TALK;
				break;

			case CONV8_RICH_BOTH_UP:
				random = 6;
				local->rich_action = CONV8_RICH_TALK;
				break;

			case CONV15_RICH_STAND_TALK:
				random = 7;
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 0;
				break; /* do talk 1 (sitting upright) */

			case 2:
				rich_reset_frame = 1;
				break; /* do talk 2 (sitting upright) */

			case 3:
				rich_reset_frame = 2;
				break; /* do talk 3 (sitting upright) */

			case 4:
				rich_reset_frame = 69;
				break; /* do a left arm up */

			case 5:
				rich_reset_frame   = 53;
				local->rich_action = CONV8_RICH_TALK;
				break; /* do a point */

			case 6:
				rich_reset_frame = 42;
				break; /* put both hands up */

			case 7:
				rich_reset_frame = 87;
				break; /* stand up and talk (new node) */

			case 8:
				rich_reset_frame = 25;
				break; /* push head way back (new node) */

			case 9:
				rich_reset_frame = 15;
				break; /* touch head */

			case 10:
				rich_reset_frame = 3;
				break; /* look at right hand (new node) */

			case 11:
				rich_reset_frame = 7;
				break; /* put hand on chin (new node) */

			case 12:
				rich_reset_frame = 59;
				break; /* look right (new node) */

			case 13:
				rich_reset_frame = 63;
				break; /* look left (new node) */

			case 14:
				rich_reset_frame = 67;
				break; /* look up (new node) */

			case 15:
				rich_reset_frame = 79;
				break; /* look at knuckle (new node) */

			default:
				rich_reset_frame = 0;
				break; /* freeze sitting */
			}
			break;

		case 5:   /* end of coming to look at right hand */

			random = imath_random(1, 45);

			switch (local->rich_action) {
			case CONV8_RICH_TALK:
			case CONV8_RICH_LEFT_UP_DONT_KNOW:
			case CONV8_RICH_POINT:
			case CONV8_RICH_BOTH_UP:
			case CONV15_RICH_STAND_TALK:
				random = 1;
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 5;
				break; /* go to sit upright (new node) */

			default:
				rich_reset_frame = 4;
				break; /* continue looking at right hand */
			}
			break;

		case 11:  /* end of coming to look chin */

			random = imath_random(1, 45);

			switch (local->rich_action) {
			case CONV8_RICH_TALK:
			case CONV8_RICH_LEFT_UP_DONT_KNOW:
			case CONV8_RICH_POINT:
			case CONV8_RICH_BOTH_UP:
			case CONV15_RICH_STAND_TALK:
				random = 1;
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 11;
				break; /* go to sit upright (new node) */

			default:
				rich_reset_frame = 10;
				break; /* continue looking at chin */
			}
			break;

		case 61:  /* end of coming to look right */

			random = imath_random(1, 45);

			switch (local->rich_action) {
			case CONV8_RICH_TALK:
			case CONV8_RICH_LEFT_UP_DONT_KNOW:
			case CONV8_RICH_POINT:
			case CONV8_RICH_BOTH_UP:
			case CONV15_RICH_STAND_TALK:
				random = 1;
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 61;
				break; /* go to sit upright (new node) */

			default:
				rich_reset_frame = 60;
				break; /* continue looking right */
			}
			break;

		case 65:  /* end of coming to look left */

			random = imath_random(1, 45);

			switch (local->rich_action) {
			case CONV8_RICH_TALK:
			case CONV8_RICH_LEFT_UP_DONT_KNOW:
			case CONV8_RICH_POINT:
			case CONV8_RICH_BOTH_UP:
			case CONV15_RICH_STAND_TALK:
				random = 1;
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 65;
				break; /* go to sit up (new node) */

			default:
				rich_reset_frame = 64;
				break; /* continue looking left */
			}
			break;

		case 68:  /* end of coming to look up */

			random = imath_random(1, 45);

			switch (local->rich_action) {
			case CONV8_RICH_TALK:
			case CONV8_RICH_LEFT_UP_DONT_KNOW:
			case CONV8_RICH_POINT:
			case CONV8_RICH_BOTH_UP:
			case CONV15_RICH_STAND_TALK:
				random = 1;
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 68;
				break; /* go to sit up (new node) */

			default:
				rich_reset_frame = 67;
				break; /* continue looking up */
			}
			break;

		case 83:  /* end of coming to look at knuckle */

			random = imath_random(1, 45);

			switch (local->rich_action) {
			case CONV8_RICH_TALK:
			case CONV8_RICH_LEFT_UP_DONT_KNOW:
			case CONV8_RICH_POINT:
			case CONV8_RICH_BOTH_UP:
			case CONV15_RICH_STAND_TALK:
				random = 1;
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 83;
				break; /* go to sit upright (new node) */

			default:
				rich_reset_frame = 82;
				break; /* continue looking at knuckle */
			}
			break;

		case 92:
			conv_release();
			break;

		case 93:  /* end of coming to stand up and talk 1 */
		case 94:  /* end of talk 2                        */
		case 95:  /* end of talk 3                        */

			switch (local->rich_action) {
			case CONV15_RICH_STAND_TALK:
				random = imath_random(1, 3);
				++local->rich_talk_count;
				if (local->rich_talk_count > 20) {
					local->rich_action = CONV15_RICH_STAND_SHUT_UP;
					random = 4;
				}
				break;

			default:
				random = 4;
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 92;
				break; /* do talk 1 (standing - arms out) */

			case 2:
				rich_reset_frame = 93;
				break; /* do talk 2 (standing - arms out) */

			case 3:
				rich_reset_frame = 94;
				break; /* do talk 3 (standing - arms out) */

			case 4:
				rich_reset_frame = 95;
				break; /* put arms down, but continue standing (new node) */
			}
			break;

		case 100:  /* end of coming to stand with arms by side, talk 1, and stand shut up */
		case 101:  /* end of talk 2 */
		case 102:  /* end of talk 3 */

			switch (local->rich_action) {
			case CONV15_RICH_STAND_SHUT_UP:
				random = 1;
				break;

			case CONV15_RICH_STAND_TALK:
				random = imath_random(1, 3);
				++local->rich_talk_count;
				if (local->rich_talk_count > 20) {
					local->rich_action = CONV15_RICH_STAND_SHUT_UP;
					random = 1;
				}
				break;

			default:
				random = 4;
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 99;
				break; /* do talk 1, or freeze (standing - arms by side) */

			case 2:
				rich_reset_frame = 100;
				break; /* do talk 2 (standing - arms by side) */

			case 3:
				rich_reset_frame = 101;
				break; /* do talk 3 (standing - arms by side) */

			case 4:
				rich_reset_frame = 102;
				break; /* sit down (new node) */
			}
			break;

		case 31:  /* end of putting head way back */

			random = imath_random(1, 45);

			switch (local->rich_action) {
			case CONV8_RICH_TALK:
			case CONV8_RICH_LEFT_UP_DONT_KNOW:
			case CONV8_RICH_POINT:
			case CONV8_RICH_BOTH_UP:
			case CONV15_RICH_STAND_TALK:
				random = 1;
				break;
			}

			switch (random) {
			case 1:
				rich_reset_frame = 31;
				break; /* go to sit upright (new node) */

			default:
				rich_reset_frame = 30;
				break; /* continue putting head way back */
			}
			break;
		}

		if (rich_reset_frame >= 0) {
			kernel_reset_animation(aa[2], rich_reset_frame);
			local->rich_frame = rich_reset_frame;
		}
	}
}

static void handle_animation_raoul() {
	int random = 0;
	int raoul_reset_frame;

	if (kernel_anim[aa[1]].frame != local->raoul_frame) {
		local->raoul_frame = kernel_anim[aa[1]].frame;
		raoul_reset_frame  = -1;

		switch (local->raoul_frame) {
		case 9:
		case 21:
		case 57:
			conv_release();
			break;

		case 10:  /* end of freeze        */
		case 22:  /* end take             */
		case 30:  /* end of talk          */
		case 34:  /* end of touching head */
		case 58:  /* end of taking letter from tray */

			if (local->raoul_frame == 22) {
				local->raoul_action = CONV5_RAOUL_SHUT_UP;
			}

			random = imath_random(5, 45);

			switch (local->raoul_action) {
			case CONV5_RAOUL_TALK:
				random = 1;
				break;

			case CONV5_RAOUL_GET_UP:
				player.commands_allowed = false;
				random = 2;
				break;

			case CONV5_RAOUL_TAKE:
			case CONV5_RAOUL_SHOW:
				random = 3;
				break;

			case CONV5_RAOUL_TAKE_TRAY:
				random = 4;
				break;

			default:
				random = imath_random(5, 100);
				break;
			}

			switch (random) {
			case 1:
				raoul_reset_frame = 22;
				break; /* do talk (new node) */

			case 2:
				raoul_reset_frame = 34;
				break; /* get up */

			case 3:
				raoul_reset_frame = 10;
				break; /* take whatever */

			case 4:
				raoul_reset_frame = 45;
				break; /* take letter from tray */

			case 5:
				raoul_reset_frame = 30;
				break; /* touch head */

			default:
				raoul_reset_frame = 9;
				break; /* freeze sitting still */
			}
			break;

		case 16:  /* middle of taking parchment */

			switch (local->raoul_action) {
			case CONV5_RAOUL_TAKE:
				random = 1;
				break;

			case CONV5_RAOUL_SHUT_UP:
			case CONV5_RAOUL_GET_UP:
				random = 2;
				break;

			case CONV5_RAOUL_SHOW:
				random = 3;
				break;
			}

			switch (random) {
			case 1:
				if (global[current_year] == 1881) {
					raoul_reset_frame = 16;
					kernel_seq_delete(seq[fx_parchment]);
					kernel_flip_hotspot(words_notice, false);
					inter_give_to_player(notice);
					sound_play(N_TakeObjectSnd);
					global_speech_ready = -1;
					object_examine(notice, text_008_14, 5);
					local->raoul_action = CONV5_RAOUL_SHUT_UP;
					/* this is for conv 8 */
				} else {
					raoul_reset_frame = 16;
					kernel_seq_delete(seq[fx_parchment]);
					kernel_flip_hotspot(words_parchment, false);
					inter_give_to_player(parchment);
					sound_play(N_TakeObjectSnd);
					global_speech_ready = -1;
					object_examine(parchment, text_008_12, 3);
					local->raoul_action = CONV5_RAOUL_SHUT_UP;
					/* this is for conv 5 */
				}
				break; /* go to sit still (new node) */

			case 2:
				if (local->show_note) {
					kernel_seq_delete(seq[fx_parchment_2]);
					local->show_note = false;
					/* take back note and sit still in chair */
				}
				break; /* go to sit still (new node) */

			default:
				if (!local->show_note) {
					seq[fx_parchment_2] = kernel_seq_stamp(ss[fx_parchment], true, 1);
					kernel_seq_depth(seq[fx_parchment_2], 3);
					conv_release();
					local->show_note = true;
					/* stamp note on desk and hold it there */
					/* the ss[parchment] is used for 2 pieces of paper */
				}
				raoul_reset_frame = 15;
				break; /* keep hand out (with note on desk) */
			}
			break;

		case 26:  /* end of coming to slouch 1 and freeze slouch 1 */

			random = imath_random(1, 45);

			switch (local->raoul_action) {
			case CONV5_RAOUL_TALK:
				random = 2;
				break;

			case CONV5_RAOUL_TAKE:
			case CONV5_RAOUL_TAKE_TRAY:
			case CONV5_RAOUL_SHOW:
			case CONV5_RAOUL_SHUT_UP:
			case CONV5_RAOUL_GET_UP:
				random = 1;
				break;
			}

			switch (random) {
			case 1:
				raoul_reset_frame = 26;
				break; /* go to sit still (new node) */

			default:
				raoul_reset_frame = 25;
				break; /* talk freeze (hand out) */
			}
			break;

		case 45:  /* end of getting up */

			local->anim_1_running   = false;
			player.walker_visible   = true;
			player.commands_allowed = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
			raoul_reset_frame = 58;
			if ((global[done_brie_conv_203]) && (global[current_year] == 1993)) {
				global[prompter_stand_status] = PROMPT_RIGHT;
				new_room = 150;
			}
			break;

		case 52:  /* middle of take note on tray */
			kernel_seq_delete(seq[fx_letter]);
			kernel_flip_hotspot(words_letter, false);
			inter_give_to_player(letter);
			sound_play(N_TakeObjectSnd);
			global_speech_ready = -1;
			object_examine(letter, text_008_13, 4);
			local->raoul_action = CONV5_RAOUL_SHUT_UP;
			break;
		}

		if (raoul_reset_frame >= 0) {
			kernel_reset_animation(aa[1], raoul_reset_frame);
			local->raoul_frame = raoul_reset_frame;
		}
	}
}

static void handle_animation_daae() {
	int random = 0;
	int daae_reset_frame;

	if (kernel_anim[aa[3]].frame != local->daae_frame) {
		local->daae_frame = kernel_anim[aa[3]].frame;
		daae_reset_frame  = -1;

		switch (local->daae_frame) {
		case 20:  /* just entered room */
			conv_release();
			local->rich_action = CONV15_RICH_STAND_TALK;
			break;

		case 175: /* make Richard sit when she is almost out the door (after conv with her) */
			local->rich_action = CONV8_RICH_SHUT_UP;
			break;

		case 198: /* almost out the door (after conv with her), release */
			conv_release();
			break;

		case 201:
			kernel_timing_trigger(1, ROOM_203_DOOR_CLOSES);
			break;

		case 76:  /* end of walking into room and freeze */
		case 92:  /* end of talk to Richard 1            */
		case 102: /* end of talk to Richard 2            */
		case 123: /* end of talk to Raoul                */

			switch (local->daae_action) {
			case CONV15_DAAE_TALK_RICH:
				random = imath_random(1, 2);
				local->daae_action = CONV15_DAAE_TALK_RAOUL;
				break;

			case CONV15_DAAE_TALK_RAOUL:
				random = 3;
				local->daae_action = CONV15_DAAE_SHUT_UP;
				break;

			case CONV15_DAAE_LEAVE:
				random = 4;
				break;

			default:
				random = 5;
				break;
			}

			switch (random) {
			case 1:
				daae_reset_frame = 79;
				break; /* do talk to Richard 1 */

			case 2:
				daae_reset_frame = 92;
				break; /* do talk to Richard 2 */

			case 3:
				daae_reset_frame = 102;
				break; /* talk to Raoul */

			case 4:
				daae_reset_frame = 123;
				break; /* leave */

			case 5:
				daae_reset_frame = 75;
				break; /* freeze */
			}
			break;
		}

		if (daae_reset_frame >= 0) {
			kernel_reset_animation(aa[3], daae_reset_frame);
			local->daae_frame = daae_reset_frame;
		}
	}
}

void room_203_daemon() {
	if (local->anim_0_running) {
		handle_animation_brie();
	}

	if (local->anim_1_running) {
		handle_animation_raoul();
	}

	if (local->anim_2_running) {
		handle_animation_rich();
	}

	if (local->anim_3_running) {
		handle_animation_daae();
	}

	if ((global[walker_converse] == CONVERSE_HAND_WAVE) ||
	    (global[walker_converse] == CONVERSE_HAND_WAVE_2)) {
		++local->converse_counter;
		if (local->converse_counter > 200) {
			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		}
	}

	switch (kernel.trigger) {
	case ROOM_203_DOOR_CLOSES:
		kernel_seq_delete(seq[fx_door]);
		seq[fx_door] = kernel_seq_backward(ss[fx_door], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_door], 10);
		kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_203_DOOR_CLOSES + 1);
		break;

	case ROOM_203_DOOR_CLOSES + 1:
		sound_play(N_DoorCloses);
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 14);
		if (conv_control.running != CONV_RD_15) {
			player.commands_allowed = true;
		}
		break;
	}
}

static void process_conversation_brie() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	switch (player_verb) {
	case conv005_crime_missing:
	case conv005_rumors_accident:
		conv_you_trigger(ROOM_203_BRIE_TALK_HANDS_UP);
		you_trig_flag = true;
		break;

	case conv005_murder_b_aa:
		if (!kernel.trigger) {
			conv_hold();
			if (global_prefer_roland) {
				sound_play(N_WomanScream002);
			} else {
				global_speech(speech_woman_scream);
			}
			kernel_timing_trigger(ONE_SECOND, ROOM_203_AFTER_SCREAM);
		}
		break;

	case conv005_scream_help:
		global[done_brie_conv_203] = YES;
		global[chris_f_status]     = CHRIS_F_IS_DEAD;
		break; /* by setting this flag, Florent is dead */

	case conv005_rumors_sandbag:
		conv_you_trigger(ROOM_203_BRIE_POINT);
		you_trig_flag = true;
		break;

	case conv005_anyone_b_sign_it_b:
		local->raoul_action = CONV5_RAOUL_SHUT_UP;
		conv_hold();
		break;

	case conv005_nowhere_nothing:
	case conv005_report_adieu:
	case conv005_goman_abc:
	case conv005_readbook_two_abc:
		conv_you_trigger(ROOM_203_GET_UP);
		you_trig_flag = true;
		me_trig_flag  = true;
		break;

	case conv005_lets_see_b_what_have_b:
		if (!kernel.trigger) {
			conv_hold();
			local->raoul_action = CONV5_RAOUL_SHOW;
		}
		break;

	case conv005_point_one_b_abc:
		if (!kernel.trigger) {
			conv_hold();
			local->raoul_action = CONV5_RAOUL_TAKE;
		}
		break;

	default:
		break;
	}

	switch (kernel.trigger) {
	case ROOM_203_AFTER_SCREAM:
		conv_release();
		break;

	case ROOM_203_GET_UP:
		conv_me_trigger(ROOM_203_GET_UP + 1);
		you_trig_flag = true;
		me_trig_flag  = true;
		break;

	case ROOM_203_GET_UP + 1:
		local->raoul_action = CONV5_RAOUL_GET_UP;
		break;

	case ROOM_203_BRIE_TALK_HANDS_UP:
		local->brie_action = CONV5_BRIE_TALK_HANDS_UP;
		break;

	case ROOM_203_BRIE_POINT:
		local->brie_action = CONV5_BRIE_POINT;
		break;

	case ROOM_203_TAKE_NOTE:
		conv_hold();
		local->raoul_action = CONV5_RAOUL_TAKE;
		break;

	case ROOM_203_BRIE_START_TALKING:
		local->brie_action = CONV5_BRIE_TALK;
		break;

	case ROOM_203_RAOUL_START_TALKING:
		local->brie_action = CONV5_BRIE_SHUT_UP;
		break;

	default:
		break;
	}


	/* ================= Set up me and you triggers ================ */

	if (!me_trig_flag) {
		if (local->raoul_action != CONV5_RAOUL_SHOW) {
			conv_me_trigger(ROOM_203_RAOUL_START_TALKING);
		}
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_203_BRIE_START_TALKING);
	}

	local->brie_talk_count  = 0;
	local->raoul_talk_count = 0;
}

static void process_conversation_rich() {
	int you_trig_flag = false;
	int me_trig_flag  = false;
	int16 *value_1;
	int16 *value_2;

	switch (player_verb) {
	case conv008_tellabout_have:
		conv_you_trigger(ROOM_203_RICH_BOTH_UP);
		you_trig_flag = true;
		break;

	case conv008_tellabout_rumor:
		conv_you_trigger(ROOM_203_RICH_POINT);
		you_trig_flag = true;
		break;

	case conv008_things_two:
		conv_you_trigger(ROOM_203_RICH_LEFT_UP_DONT_KNOW);
		you_trig_flag = true;
		break;

	case conv008_things_three:
	case conv008_actions_byebye:
	case conv008_later_final:
	case conv008_nomore_first:
	case conv008_christine_three:
		conv_you_trigger(ROOM_203_GET_UP);
		you_trig_flag = true;
		me_trig_flag  = true;
		break;

	case conv008_actions_b_b:
		if (object_is_here(notice)) {
			if (!kernel.trigger) {
				conv_hold();
				local->raoul_action = CONV5_RAOUL_TAKE;
			}
			break;
		}
		break;

	case conv008_actions_d_d:
		if (object_is_here(letter)) {
			if (!kernel.trigger) {
				conv_hold();
				local->raoul_action = CONV5_RAOUL_TAKE_TRAY;
			}
			break;
		}
		break;
	}

	switch (kernel.trigger) {
	case ROOM_203_GET_UP:
		conv_me_trigger(ROOM_203_GET_UP + 1);
		you_trig_flag = true;
		me_trig_flag  = true;
		break;

	case ROOM_203_GET_UP + 1:
		local->raoul_action = CONV5_RAOUL_GET_UP;
		value_1 = conv_get_variable(conv008_var_actions_done);
		value_2 = conv_get_variable(conv008_var_christine_done);
		if ((*value_1) && (*value_2)) {
			global[done_rich_conv_203]   = true;
			global[madame_giry_shows_up] = true;
		}
		break;

	case ROOM_203_RICH_BOTH_UP:
		local->rich_action = CONV8_RICH_BOTH_UP;
		break;

	case ROOM_203_RICH_POINT:
		local->rich_action = CONV8_RICH_POINT;
		break;

	case ROOM_203_RICH_LEFT_UP_DONT_KNOW:
		local->rich_action = CONV8_RICH_LEFT_UP_DONT_KNOW;
		break;

	case ROOM_203_TAKE_NOTE:
		conv_hold();
		local->raoul_action = CONV5_RAOUL_TAKE;
		break;

	case ROOM_203_RICH_START_TALKING:
		local->rich_action = CONV8_RICH_TALK;
		break;

	case ROOM_203_RAOUL_START_TALKING:
		local->rich_action = CONV8_RICH_SHUT_UP;
		break;

	default:
		break;
	}


	/* ================= Set up me and you triggers ================ */

	if (!me_trig_flag) {
		if (local->raoul_action != CONV5_RAOUL_SHOW) {
			conv_me_trigger(ROOM_203_RAOUL_START_TALKING);
		}
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_203_RICH_START_TALKING);
	}

	local->brie_talk_count  = 0;
	local->raoul_talk_count = 0;
}

static void process_conversation_rd() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	switch (player_verb) {
	case conv015_exchange_ticket:
	case conv015_exchange_relief:
	case conv015_exchange_opera:
		global[christine_door_status] = CHRIS_DOOR_CLOSED;
		global[ticket_people_here]    = USHER_AND_SELLER;
		me_trig_flag                  = true;
		you_trig_flag                 = true;
		conv_you_trigger(ROOM_203_END);
		break;

	case conv015_daae_b_b:
		if (!kernel.trigger) {
			conv_hold();
			sound_play(N_DoorOpens);
			kernel_seq_delete(seq[fx_door]);
			seq[fx_door] = kernel_seq_forward(ss[fx_door], false, 8, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 1);
			kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_203_DOOR_OPENS);
			you_trig_flag = true;
			me_trig_flag  = true;
		}
		break;

	case conv015_damned_b_b:
		if (!kernel.trigger) {
			conv_hold();
			local->daae_action = CONV15_DAAE_LEAVE;
		}
		break;

	default:
		break;
	}

	switch (kernel.trigger) {
	case ROOM_203_END:
		global[walker_converse] = CONVERSE_NONE;
		me_trig_flag            = true;
		you_trig_flag           = true;
		break;

	case ROOM_203_DOOR_OPENS:
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_door], 1);
		aa[3]                 = kernel_run_animation(kernel_name('d', 1), 0);
		local->anim_3_running = true;
		local->daae_action    = CONV15_DAAE_SHUT_UP;
		break;

	case ROOM_203_RICH_START_TALKING:
		if (global[walker_converse] != CONVERSE_NONE) {
			global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
		}

		if ((player_verb == conv015_next_second) ||
		    (player_verb == conv015_professional_fourth)) {
			local->daae_action = CONV15_DAAE_TALK_RICH;
		} else if (local->rich_action == CONV15_RICH_STAND_SHUT_UP) {
			local->rich_action = CONV15_RICH_STAND_TALK;
		} else if (local->rich_action != CONV15_RICH_STAND_TALK) {
			local->rich_action = CONV8_RICH_TALK;
		}
		break;

	case ROOM_203_RAOUL_START_TALKING:
		if (global[walker_converse] != CONVERSE_NONE) {
			global[walker_converse] = imath_random(CONVERSE_HAND_WAVE, CONVERSE_HAND_WAVE_2);
		}

		if ((local->rich_action != CONV15_RICH_STAND_TALK) &&
		    (local->rich_action != CONV15_RICH_STAND_SHUT_UP)) {
			local->rich_action = CONV8_RICH_SHUT_UP;
		}
		local->converse_counter = 0;
		break;

	default:
		break;
	}


	/* ================= Set up me and you triggers ================ */

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_203_RAOUL_START_TALKING);
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_203_RICH_START_TALKING);
	}

	local->brie_talk_count  = 0;
	local->raoul_talk_count = 0;
}

void room_203_pre_parser() {
	if (player_said_2(talk_to, Monsieur_Richard)) {
		if (global[came_from_fade]) {
			player_walk(PLAYER_X_FROM_113, PLAYER_Y_FROM_113, FACING_NORTHEAST);
		} else {
			player_walk(FRONT_CHAIR_X, FRONT_CHAIR_Y, FACING_NORTHWEST);
		}
	}

	if (player_said_2(talk_to, Monsieur_Brie)) {
		player_walk(FRONT_CHAIR_X, FRONT_CHAIR_Y, FACING_NORTHWEST);
	}

	if (player_said_2(open, door)) {
		player_walk(DOOR_X, DOOR_Y, FACING_EAST);
	}
}

void room_203_parser() {
	if (conv_control.running == CONV_BRIE_5) {
		process_conversation_brie();
		goto handled;
	}

	if (conv_control.running == CONV_RICH_8) {
		process_conversation_rich();
		goto handled;
	}

	if (conv_control.running == CONV_RD_15) {
		process_conversation_rd();
		goto handled;
	}

	if (player_said_2(walk_through, door) || player_said_2(open, door) || kernel.trigger) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			seq[fx_take_6] = kernel_seq_pingpong(ss[fx_take_6], false, 5, 0, 0, 2);
			kernel_seq_range(seq[fx_take_6], 1, 4);
			kernel_seq_player(seq[fx_take_6], true);
			kernel_seq_trigger(seq[fx_take_6], KERNEL_TRIGGER_SPRITE, 4, ROOM_203_DOOR_OPENS);
			kernel_seq_trigger(seq[fx_take_6], KERNEL_TRIGGER_EXPIRE, 0, ROOM_203_DOOR_OPENS + 2);
			break;

		case ROOM_203_DOOR_OPENS:
			sound_play(N_DoorOpens);
			kernel_seq_delete(seq[fx_door]);
			seq[fx_door] = kernel_seq_forward(ss[fx_door], false, 8, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 1);
			kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_203_DOOR_OPENS + 1);
			break;

		case ROOM_203_DOOR_OPENS + 1:
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_door], 1);
			break;

		case ROOM_203_DOOR_OPENS + 2:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_6]);
			player_walk(WALK_TO_X_BEHIND_DOOR, WALK_TO_Y_BEHIND_DOOR, FACING_WEST);
			player_walk_trigger(ROOM_203_DOOR_OPENS + 3);
			break;

		case ROOM_203_DOOR_OPENS + 3:
			new_room = 204;
			break;
		}
		goto handled;
	}

	if (player_said_2(talk_to, Monsieur_Brie)) {
		aa[1] = kernel_run_animation(kernel_name('c', 1), 0);
		local->anim_1_running   = true;
		player.walker_visible   = false;
		local->raoul_action     = CONV5_RAOUL_SHUT_UP;
		kernel_synch(KERNEL_ANIM, aa[1], KERNEL_PLAYER, 0);

		conv_run(CONV_BRIE_5);
		conv_hold();
		conv_export_pointer(&global[player_score]);
		conv_export_value(player_has(small_note));
		conv_export_value(global[read_book]);
		conv_export_value(player_has(large_note));
		conv_export_value(global[looked_at_case]);
		if (global[charles_name_is_known] == YES_AND_END_CONV) {
			conv_export_value(1);
		} else {
			conv_export_value(0);
		}
		conv_export_value(global[can_find_book_library]);
		conv_export_value(global[florent_name_is_known]);
		conv_export_value(global[sandbag_status]);
		conv_export_value(global[observed_phan_104]);
		goto handled;
	}

	if (player_said_2(talk_to, Monsieur_Richard)) {
		if (global[came_from_fade]) {
			conv_run(CONV_RD_15); /* came from 113 */
			conv_export_pointer(&global[player_score]);
			conv_export_pointer(&global[christine_told_envelope]);

		} else {
			aa[1] = kernel_run_animation(kernel_name('c', 1), 0);
			local->anim_1_running   = true;
			player.walker_visible   = false;
			player.commands_allowed = false;
			local->raoul_action     = CONV5_RAOUL_SHUT_UP;
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_PLAYER, 0);

			conv_run(CONV_RICH_8);
			conv_export_pointer(&global[player_score]);
			conv_hold();
		}
		goto handled;
	}

	if (player_said_2(exit_to, grand_foyer)) {
		new_room = 202;
		goto handled;
	}

	if (player.look_around) {
		if (global[make_brie_leave_203]) {
			text_show(text_203_37);
		} else {
			text_show(text_203_10);
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(wall)) {
			text_show(text_203_11);
			goto handled;
		}

		if (player_said_1(floor)) {
			text_show(text_203_12);
			goto handled;
		}

		if (player_said_1(bookcase)) {
			text_show(text_203_13);
			goto handled;
		}

		if (player_said_1(doorway)) {
			text_show(text_203_14);
			goto handled;
		}

		if (player_said_1(comfy_chair)) {
			text_show(text_203_15);
			goto handled;
		}

		if (player_said_1(desk)) {
			text_show(text_203_16);
			goto handled;
		}

		if (player_said_1(manager_s_chair)) {
			text_show(text_203_17);
			goto handled;
		}

		if (player_said_1(desk_lamp)) {
			text_show(text_203_18);
			goto handled;
		}

		if (player_said_1(lamp)) {
			text_show(text_203_19);
			goto handled;
		}

		if (player_said_1(light_fixture)) {
			text_show(text_203_20);
			goto handled;
		}

		if (player_said_1(window)) {
			text_show(text_203_21);
			goto handled;
		}

		if (player_said_1(sheers)) {
			text_show(text_203_22);
			goto handled;
		}

		if (player_said_1(tapestry)) {
			text_show(text_203_23);
			goto handled;
		}

		if (player_said_1(grand_foyer)) {
			text_show(text_203_24);
			goto handled;
		}

		if (player_said_1(table)) {
			text_show(text_203_25);
			goto handled;
		}

		if (player_said_1(candle)) {
			text_show(text_203_26);
			goto handled;
		}

		if (player_said_1(Monsieur_Brie)) {
			text_show(text_203_27);
			goto handled;
		}

		if (player_said_1(Monsieur_Richard)) {
			text_show(text_203_28);
			goto handled;
		}

		if (player_said_1(parchment) && object_is_here(parchment)) {
			text_show(text_203_29);
			goto handled;
		}

		if (player_said_1(letter) && object_is_here(letter)) {
			text_show(text_203_31);
			goto handled;
		}

		if (player_said_1(notice) && object_is_here(notice)) {
			text_show(text_203_33);
			goto handled;
		}
	}

	if (player_said_2(take, parchment)) {
		text_show(text_203_30);
		goto handled;
	}

	if (player_said_2(take, letter)) {
		text_show(text_203_32);
		goto handled;
	}

	if (player_said_2(take, notice)) {
		text_show(text_203_34);
		goto handled;
	}

	if (player_said_2(take, Monsieur_Brie)) {
		text_show(text_203_35);
		goto handled;
	}

	if (player_said_2(take, Monsieur_Richard)) {
		text_show(text_203_36);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_203_init() {
	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_2_running = false;
		local->anim_3_running = false;
		local->show_note      = false;
	}

	local->converse_counter = 0;

	kernel_flip_hotspot(words_letter,    false);
	kernel_flip_hotspot(words_parchment, false);
	kernel_flip_hotspot(words_notice,    false);


	/* =================== Load Sprite Series ====================== */

	ss[fx_parchment] = kernel_load_series(kernel_name('p', 1), false);
	ss[fx_letter]    = kernel_load_series(kernel_name('p', 0), false);
	ss[fx_door]      = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_take_6]    = kernel_load_series("*RDR_6", false);


	/* =================== Load conversations ====================== */

	if (global[current_year] == 1993) {
		ss[fx_1993] = kernel_load_series(kernel_name('z', -1), false);
		conv_get(CONV_BRIE_5);
	} else {
		conv_get(CONV_RICH_8);
		conv_get(CONV_RD_15);  /* RD is Richard & Daae */
	}


	/* =================== Draw 1993 sprite ======================== */

	if (global[current_year] == 1993) {
		if (object_is_here(parchment)) {
			seq[fx_parchment] = kernel_seq_stamp(ss[fx_parchment], false, 1);
			kernel_seq_depth(seq[fx_parchment], 3);
			kernel_flip_hotspot(words_parchment, true);
		}

		kernel_flip_hotspot(words_Monsieur_Richard, false);
		kernel_flip_hotspot(words_manager_s_chair,  false);

		if (!global[make_brie_leave_203]) {
			aa[0]                 = kernel_run_animation(kernel_name('b', 9), 1);
			local->anim_0_running = true;
			local->brie_action    = CONV5_BRIE_SHUT_UP;
		} else {
			kernel_flip_hotspot(words_Monsieur_Brie,   false);
			kernel_flip_hotspot(words_manager_s_chair, true);
		}

		if (previous_room == KERNEL_RESTORING_GAME) {
			if (conv_restore_running == CONV_BRIE_5) {
				local->brie_action      = CONV5_BRIE_SHUT_UP;
				local->raoul_action     = CONV5_RAOUL_SHUT_UP;
				local->anim_1_running   = true;
				player.walker_visible   = false;
				player.commands_allowed = false;
				aa[1] = kernel_run_animation(kernel_name('c', 1), 0);
				kernel_reset_animation(aa[1], 9);
				conv_run(CONV_BRIE_5);
				conv_export_pointer(&global[player_score]);
				conv_export_value(player_has(small_note));
				conv_export_value(global[read_book]);
				conv_export_value(player_has(large_note));
				conv_export_value(global[looked_at_case]);
				conv_export_value(global[charles_name_is_known]);
				conv_export_value(global[can_find_book_library]);
				conv_export_value(global[florent_name_is_known]);
				conv_export_value(global[sandbag_status]);
				conv_export_value(global[observed_phan_104]);
			}
		}

		kernel_draw_to_background(ss[fx_1993], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_flip_hotspot(words_candle, false);

	} else if (global[jacques_status] == JACQUES_IS_ALIVE) {

		kernel_flip_hotspot(words_desk_lamp,        false);
		kernel_flip_hotspot(words_Monsieur_Brie,    false);
		kernel_flip_hotspot(words_manager_s_chair,  false);

		if (!global[make_rich_leave_203]) {
			aa[2]                 = kernel_run_animation(kernel_name('r', 1), 1);
			local->anim_2_running = true;
			local->rich_action    = CONV8_RICH_SHUT_UP;

		} else {
			kernel_flip_hotspot(words_Monsieur_Richard, false);
			kernel_flip_hotspot(words_manager_s_chair,  true);
		}

		if (previous_room == KERNEL_RESTORING_GAME) {
			if (conv_restore_running == CONV_RICH_8) {
				aa[1] = kernel_run_animation(kernel_name('c', 1), 0);
				kernel_reset_animation(aa[1], 9); /* make Raoul frozen while sitting down */
				local->anim_1_running   = true;
				player.walker_visible   = false;
				local->raoul_action     = CONV5_RAOUL_SHUT_UP;
				conv_run(CONV_RICH_8);
				conv_export_pointer(&global[player_score]);
			}
		}

		if (object_is_here(letter)) {
			seq[fx_letter] = kernel_seq_stamp(ss[fx_letter], false, 1);
			kernel_seq_depth(seq[fx_letter], 3);
			kernel_flip_hotspot(words_letter, true);
		}

		if (object_is_here(notice)) {
			seq[fx_parchment] = kernel_seq_stamp(ss[fx_parchment], false, 1);
			kernel_seq_depth(seq[fx_parchment], 3);
			kernel_flip_hotspot(words_notice, true);
		}

	} else {
		kernel_flip_hotspot(words_Monsieur_Brie,    false);
		kernel_flip_hotspot(words_Monsieur_Richard, false);
		kernel_flip_hotspot(words_desk_lamp,        false);
	}


	if (conv_restore_running == CONV_RD_15) {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 14);
		player.x      = PLAYER_X_FROM_113;
		player.y      = PLAYER_Y_FROM_113;
		player.facing = FACING_NORTHEAST;
		conv_run(CONV_RD_15);
		conv_export_pointer(&global[player_score]);
		conv_export_pointer(&global[christine_told_envelope]);
		global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);
	}


	/* ========================= Previous Rooms ===================== */

	if (previous_room == KERNEL_RESTORING_GAME) {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 14);

	} else if (previous_room == 202) {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 14);
		player.x      = PLAYER_X_FROM_202;
		player.y      = PLAYER_Y_FROM_202;
		player.facing = FACING_NORTH;

	} else if (previous_room == 150) {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 14);
		player.x      = PLAYER_X_FROM_113;
		player.y      = PLAYER_Y_FROM_113;
		player.facing = FACING_NORTHEAST;
		conv_run(CONV_RD_15);
		conv_export_pointer(&global[player_score]);
		conv_export_pointer(&global[christine_told_envelope]);
		global[walker_converse] = imath_random(CONVERSE_LEAN, CONVERSE_HAND_CHIN);

	} else if ((previous_room == 204) || (previous_room != KERNEL_RESTORING_GAME)) {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_door], 1);
		player.x                = WALK_TO_X_BEHIND_DOOR;
		player.y                = WALK_TO_Y_BEHIND_DOOR;
		player.facing           = FACING_SOUTHWEST;
		player_walk(WALK_TO_X_FROM_204, WALK_TO_Y_FROM_204, FACING_WEST);
		player_walk_trigger(ROOM_203_DOOR_CLOSES);
		player.commands_allowed = false;
	}

	section_2_music();
}

void room_203_preload() {
	room_init_code_pointer       = room_203_init;
	room_pre_parser_code_pointer = room_203_pre_parser;
	room_parser_code_pointer     = room_203_parser;
	room_daemon_code_pointer     = room_203_daemon;

	if (global[current_year] == 1993) {
		kernel_initial_variant = 1;
	}

	section_2_walker();
	section_2_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
