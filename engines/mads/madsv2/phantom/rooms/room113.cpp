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
#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/rooms/room113.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

static void handle_animation_florent() {
	int random;
	int florent_reset_frame;

	if (kernel_anim[aa[2]].frame != local->florent_frame) {
		local->florent_frame = kernel_anim[aa[2]].frame;
		florent_reset_frame  = -1;

		switch (local->florent_frame) {
		case 1:   /* end of freeze (at mirror)                */
		case 11:  /* end of pointing (and back to mirror)     */
		case 23:  /* end of brush hair (and back to mirror)   */
		case 38:  /* end talking offsets (and back to mirror) */
		case 45:  /* end holding hand                         */

			random = imath_random(4, 30);

			if (local->florent_action == CONV4_FLORENT_POINT) {
				random = 1;
			}

			if (local->florent_action == CONV4_FLORENT_TALK) {
				random = 2;
			}

			if (local->florent_action == CONV4_FLORENT_HOLD_HAND) {
				random = 3;
			}

			switch (random) {
			case 1:
				florent_reset_frame = 2;
				local->florent_action = CONV4_FLORENT_SHUT_UP;
				break; /* do point */

			case 2:
				florent_reset_frame = 28;
				break; /* go to talking offsets */

			case 3:
				florent_reset_frame = 39;
				break; /* go to hold hand */

			case 4:
				florent_reset_frame = 12;
				break; /* do brush hair */

			default:
				florent_reset_frame = 0;
				break; /* freeze looking at mirror */
			}
			break;

		case 33:    /* end of move to talking offsets */
		case 24:    /* end of talking 1               */
		case 25:    /* end of talking 2               */
		case 26:    /* end of talking 3               */
		case 27:    /* end of talking 4               */

			random = imath_random(5, 20);

			if (local->florent_action == CONV4_FLORENT_TALK) {
				random = imath_random(1, 4);
				++local->florent_talk_count;
				if (local->florent_talk_count > 5) {
					local->florent_action = CONV4_FLORENT_SHUT_UP;
					random = 5;
				}
			}

			if ((local->florent_action == CONV4_FLORENT_POINT) ||
			    (local->florent_action == CONV4_FLORENT_HOLD_HAND)) {
				random = 5;
			}

			switch (random) {
			case 1:
				florent_reset_frame = 23;
				break; /* do talk 1 */

			case 2:
				florent_reset_frame = 24;
				break; /* do talk 2 */

			case 3:
				florent_reset_frame = 25;
				break; /* do talk 3 */

			case 4:
				florent_reset_frame = 26;
				break; /* do talk 4 */

			case 5:
				florent_reset_frame = 34;
				break; /* look at mirror */

			default:
				florent_reset_frame = 32;
				break; /* freeze while pointing */
			}
			break;

		case 42:    /* end of holding hand */

			if (local->florent_action == CONV4_FLORENT_HOLD_HAND) {
				florent_reset_frame = 41; /* keep holding hand */
			}
			break;
		}

		if (florent_reset_frame >= 0) {
			kernel_reset_animation(aa[2], florent_reset_frame);
			local->florent_frame = florent_reset_frame;
		}
	}
}

static void handle_animation_13_day() {
	int random;
	int day_reset_frame;

	if (kernel_anim[aa[0]].frame != local->day_frame) {
		local->day_frame = kernel_anim[aa[0]].frame;
		day_reset_frame  = -1;

		switch (local->day_frame) {
		case 205: /* just finished walking away from him */
		case 212: /* she has back to him, by mirror - freeze her there */
			day_reset_frame = 211;
			break;

		case 182: /* player now has walker back; he can only try to talk to her & leave */
			player.walker_visible = true;
			conv_release();
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
			player.x              = PLAYER_X_AFTER_CONV13;
			player.y              = PLAYER_Y_AFTER_CONV13;
			player.turn_to_facing = FACING_NORTHEAST;
			player_demand_facing(FACING_SOUTHEAST);
			player.turn_to_facing = FACING_NORTHEAST;
			break;

		case 1:    /* end of looking at mirror           */
		case 4:    /* end of first talk                  */
		case 14:   /* end of wave at Raoul               */
		case 17:   /* end of second talk                 */
		case 20:   /* end of third talk                  */
		case 26:   /* end of looking toward Raoul        */
		case 166:  /* end of both of them shutting up    */
		case 206:  /* end of day talk while standing 1   */
		case 207:  /* end of day talk while standing 2   */
		case 208:  /* end of day talk while standing 3   */
		case 209:  /* end of raoul talk while standing 1 */
		case 210:  /* end of raoul talk while standing 2 */
		case 211:  /* end of raoul talk while standing 3 */
		case 219:  /* end of brush hair 1                */
		case 231:  /* end of brush hair 2                */
		case 253:  /* end of brush hair 3                */
			switch (local->day_action) {
			case CONV13_DAY_STAND_TALK:
				day_reset_frame = imath_random(205, 207);
				++local->stand_talk_count;
				if (local->stand_talk_count > 18) {
					local->day_action = CONV13_BOTH_SHUT_UP;
					day_reset_frame   = 165;
				}
				if (local->music_is_playing) {
					day_reset_frame = 167;
				}
				break; /* choose random if she is talking while standing */

			case CONV13_DAY_STAND_SHUT_UP:
				day_reset_frame = imath_random(208, 210);
				while (local->just_did_option == day_reset_frame) {
					day_reset_frame = imath_random(208, 210);
				} /* so we don't repeat the same option twice */
				local->just_did_option = day_reset_frame;

				++local->stand_talk_count;
				if (local->stand_talk_count > 18) {
					local->day_action = CONV13_BOTH_SHUT_UP;
					day_reset_frame   = 165;
				}

				if (local->music_is_playing) {
					day_reset_frame = 167;
				} /* if music is playing, make then separate */
				break; /* choose random if he is talking while standing */

			case CONV13_BOTH_SHUT_UP:
				day_reset_frame = 165;
				if (local->music_is_playing) {
					day_reset_frame = 167;
				} /* if music is playing, make then separate */
				break; /* make them both shut up while standing */

			case CONV13_DAY_WAVE:
				day_reset_frame   = 5;  /* make Day wave at Raoul */
				local->day_action = CONV13_DAY_TALK;
				break;

			case CONV13_DAY_MIRROR:
				random = imath_random(1, 50);
				switch (random) {
				case 1:
					day_reset_frame = 212;
					break; /* do brush hair 1 */

				case 2:
					day_reset_frame = 219;
					break; /* do brush hair 2 */

				case 3:
					day_reset_frame = 231;
					break; /* do brush hair 3 */

				default:
					day_reset_frame = 0;  /* make Day keep looking at mirror */
					break;
				}
				break;

			case CONV13_DAY_GET_READY_TALK:
				if (local->day_wants_to_talk) {
					local->day_action        = CONV13_DAY_TALK;
					local->day_wants_to_talk = false;
					day_reset_frame = 1;
				} else {
					day_reset_frame = 25;
				}
				break;

			case CONV13_DAY_TALK:
				random = imath_random(1, 3);  /* Between 3 talking movements.       */
				++local->day_talk_count;      /* Run through day_talk_count talking */
				                              /* cycles before shutting up (looking */
				                              /* toward Raoul)                      */
				if (local->day_talk_count < 6) {
					if (random == 1) {
						day_reset_frame = 1;    /* start first talk    */
					} else if (random == 2) {
						day_reset_frame = 15;   /* start second talk   */
					} else {
						day_reset_frame = 18;   /* start third talk    */
					}
				} else {
					local->day_action        = CONV13_DAY_GET_READY_TALK;
					day_reset_frame          = 25;
					local->day_wants_to_talk = false;
				}
				break;

			case CONV13_DAY_GET_UP:
				day_reset_frame = 31;  /* start getting up & going toward Raoul */
				break;
			}
			break;
		}

		if (day_reset_frame >= 0) {
			kernel_reset_animation(aa[0], day_reset_frame);
			local->day_frame = day_reset_frame;
		}
	}
}

static void handle_animation_4_raoul() {
	int raoul_reset_frame;

	if (global[florent_name_is_known] == YES_AND_END_CONV) {
		local->raoul_action = CONV4_RAOUL_INVISIBLE;
	}

	if (kernel_anim[aa[3]].frame != local->raoul_frame) {
		local->raoul_frame = kernel_anim[aa[3]].frame;
		raoul_reset_frame  = -1;

		switch (local->raoul_frame) {

		case 9:    /* end of first open mouth  and freeze */
		case 10:   /* end of second open mouth */
		case 11:   /* end of third open mouth  */
		case 34:   /* end of invisible frame   */
		case 33:   /* end of take_note         */

			if (local->raoul_frame == 33) {
				conv_release();
			}

			switch (local->raoul_action) {

			case CONV4_RAOUL_TAKE_NOTE:
				raoul_reset_frame   = 20;
				local->raoul_action = CONV4_RAOUL_SHUT_UP;
				break;

			case CONV4_RAOUL_GET_UP:
				raoul_reset_frame  = 38; /* make Raoul get up */
				local->raoul_is_up = true;
				conv_hold();
				break;

			case CONV4_RAOUL_SHUT_UP:
				raoul_reset_frame = 8;  /* make Raoul frozen */
				break;

			case CONV4_RAOUL_TALK:
				raoul_reset_frame = imath_random(7, 9);  /* between 3 talking frames */
				++local->raoul_talk_count;    /* run through so many talk cycles */

				if (local->raoul_talk_count > 17) {
					local->raoul_action = CONV4_RAOUL_SHUT_UP;
					raoul_reset_frame   = 8; /* make Raoul frozen */
				}
				break;

			case CONV4_RAOUL_INVISIBLE:
				raoul_reset_frame = 33;  /* keep Raoul invisible */
				break;

			case CONV4_RAOUL_CHIN:
				raoul_reset_frame = 11;  /* put hand on chin */
				break;
			}
			break;

		case 42:   /* end of getting up */

			switch (local->raoul_action) {

			case CONV4_RAOUL_GET_UP:
				if (!local->prevent_2) {
					conv_release();
				}
				raoul_reset_frame     = 33;  /* keep Raoul invisible */
				local->raoul_action   = CONV4_RAOUL_INVISIBLE;
				player.walker_visible = true;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[3]);
				break;
			}
			break;

		case 19:  /* end of hand on chin */
			if ((local->raoul_action == CONV4_RAOUL_TAKE_NOTE) && (!player_has(small_note))) {
				raoul_reset_frame   = 20;  /* make Raoul talk */
				local->raoul_action = CONV4_RAOUL_SHUT_UP;

			} else if (local->raoul_action == CONV4_RAOUL_GET_UP) {
				raoul_reset_frame  = 38; /* make Raoul get up */
				local->raoul_is_up = true;

			} else if (local->raoul_action == CONV4_RAOUL_TALK) {
				raoul_reset_frame = 9;  /* make Raoul talk */

			} else {
				raoul_reset_frame   = 8;  /* make Raoul frozen */
				local->raoul_action = CONV4_RAOUL_SHUT_UP;
			}
			break;
		}

		if (raoul_reset_frame >= 0) {
			kernel_reset_animation(aa[3], raoul_reset_frame);
			local->raoul_frame = raoul_reset_frame;
		}
	}
}

static void handle_animation_13_raoul() {
	int raoul_reset_frame;

	if (kernel_anim[aa[1]].frame != local->raoul_frame) {
		local->raoul_frame = kernel_anim[aa[1]].frame;
		raoul_reset_frame  = -1;

		switch (local->raoul_frame) {
		case 9:    /* end of first open mouth  and freeze */
		case 10:   /* end of second open mouth */
		case 11:   /* end of third open mouth  */
		case 34:   /* end of invisible frame   */
			switch (local->raoul_action) {
			case CONV13_RAOUL_SHUT_UP:
				raoul_reset_frame = 8;  /* make Raoul frozen */
				break;

			case CONV13_RAOUL_TALK:
				raoul_reset_frame = imath_random(7, 9);  /* between 3 talking frames        */
				++local->raoul_talk_count;               /* run through so many talk cycles */

				if (local->raoul_talk_count > 17) {
					local->raoul_action = CONV13_RAOUL_SHUT_UP;
					raoul_reset_frame   = 8; /* make Raoul frozen */
				}
				break;

			case CONV13_RAOUL_INVISIBLE:
				raoul_reset_frame = 33;  /* keep Raoul invisible */
				break;

			case CONV13_RAOUL_CHIN:
				raoul_reset_frame = 11;  /* put hand on chin */
				break;
			}
			break;

		case 19:  /* end of hand on chin */
			if (local->raoul_action == CONV13_RAOUL_TALK) {
				raoul_reset_frame = 9;  /* make Raoul talk */
			} else {
				raoul_reset_frame   = 8;  /* make Raoul frozen */
				local->raoul_action = CONV13_RAOUL_SHUT_UP;
			}
			break;
		}

		if (raoul_reset_frame >= 0) {
			kernel_reset_animation(aa[1], raoul_reset_frame);
			local->raoul_frame = raoul_reset_frame;
		}
	}
}

static void handle_animation_6_julie() {
	int julie_reset_frame;
	int random = 0;

	if (kernel_anim[aa[4]].frame != local->julie_frame) {
		local->julie_frame = kernel_anim[aa[4]].frame;
		julie_reset_frame  = -1;

		switch (local->julie_frame) {

		case 1:    /* end of talk 1 */
		case 2:    /* end of talk 2 */
		case 3:    /* end of talk 3 */
		case 11:   /* end of cry 1  */
		case 16:   /* end of cry 2  */
		case 17:   /* end of freeze */
			switch (local->julie_action) {
			case CONV6_JULIE_TALK:
				random = imath_random(1, 3);
				++local->julie_talk_count;
				if (local->julie_talk_count > 20) {
					local->julie_action = CONV6_JULIE_CRY;
					random = 6;
				}
				break;

			case CONV6_JULIE_CRY:
				random = imath_random(4, 20);
				break;
			}

			switch (random) {
			case 1:
				julie_reset_frame = 0;
				break; /* start talk 1 */

			case 2:
				julie_reset_frame = 1;
				break; /* start talk 2 */

			case 3:
				julie_reset_frame = 2;
				break; /* start talk 3 */

			case 4:
				julie_reset_frame = 12;
				break; /* start cry 1 */

			case 5:
				julie_reset_frame = 4;
				break; /* start cry 2 */

			default:
				julie_reset_frame = 16;
				break; /* start freeze */
				break;
			}
			break;
		}

		if (julie_reset_frame >= 0) {
			kernel_reset_animation(aa[4], julie_reset_frame);
			local->julie_frame = julie_reset_frame;
		}
	}
}

void room_113_init() {
	if (previous_room != KERNEL_RESTORING_GAME) {
		local->standing_talking  = false;
		local->day_wants_to_talk = false;
		local->music_is_playing  = false;
		local->right_after_kiss  = false;
		local->anim_0_running    = false;
		local->anim_1_running    = false;
		local->anim_2_running    = false;
		local->anim_3_running    = false;
		local->anim_4_running    = false;
		local->prevent_1         = false;
		local->prevent_2         = false;
		local->raoul_is_up       = false;
		local->arms_are_out      = false;
	}

	kernel_flip_hotspot(words_small_note,    false);
	kernel_flip_hotspot(words_dressing_gown, false);
	kernel_flip_hotspot(words_Christine,     false);
	kernel_flip_hotspot(words_Julie,         false);
	kernel_flip_hotspot(words_light_fixture, false);
	/* these will be turned on if needed */


	/* =================== Load Sprite Series ======================= */

	if (global[current_year] == 1993) {
		ss[fx_1993] = kernel_load_series(kernel_name('z', -1), false);
		ss[fx_dress] = kernel_load_series(kernel_name('f', 0), false);

		if (object_is_here(small_note)) {
			ss[fx_small_note] = kernel_load_series(kernel_name('f', 1), false);
		}

		if ((global[done_brie_conv_203] == YES) ||
		    (global[done_brie_conv_203] == YES_AND_CHASE)) {
			ss[fx_corpse] = kernel_load_series(kernel_name('c', 3), false);
		}
	}


	/* =================== Load conversation ======================== */

	if (global[current_year] == 1993) {
		conv_get(CONV_FLORENT);
		if ((global[done_brie_conv_203] == YES) || (global[done_brie_conv_203] == YES_AND_CHASE)) {
			/* completed conv with brie in 203 - show dead Florent */
			conv_get(CONV_FLORENT_DEAD);
		}
	} else {
		ss[fx_face_1] = kernel_load_series("*faceral",  false);
		ss[fx_face_2] = kernel_load_series("*facecrsd", false);
		conv_get(CONV_LOVE_SCENE);
	}


	/* ============== If 1993, put florent here ===================== */

	if (global[current_year] == 1993) {
		kernel_draw_to_background(ss[fx_1993], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_flip_hotspot(words_vase, false);
		kernel_flip_hotspot(words_fan, false);
		kernel_flip_hotspot_loc(words_light_fixture, true, LIGHT_1993_X, LIGHT_1993_Y);
	} else {
		kernel_flip_hotspot_loc(words_light_fixture, true, LIGHT_1881_X, LIGHT_1881_Y);
		kernel_flip_hotspot(words_notice, false);
	}

	if (previous_room == KERNEL_RESTORING_GAME) {
		if (global[current_year] == 1881) { /* start Daae anim */
			if (local->day_action <= CONV13_DAY_WAVE) {
				aa[1]                 = kernel_run_animation(kernel_name('r', 1), 1);
				local->anim_1_running = true;
			}

			switch (local->raoul_action) {
			case CONV13_RAOUL_INVISIBLE:
				kernel_reset_animation(aa[1], 33); /* this will keep Raoul shutting up (on couch) */
				break;

			default:
				local->raoul_action   = CONV13_RAOUL_SHUT_UP;
				player.walker_visible = false;
				kernel_reset_animation(aa[1], 8); /* this will keep Raoul shutting up (on couch) */
				break;
			}

			if (local->music_is_playing) {
				aa[0]                     = kernel_run_animation(kernel_name('d', 2), 0);
				local->anim_0_running     = true;
				local->chris_daae_dynamic = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
				                                               0, 0, 0, 0);
				kernel_dynamic_hot[local->chris_daae_dynamic].prep = PREP_ON;
				kernel_dynamic_anim(local->chris_daae_dynamic, aa[0], 13);
				kernel_dynamic_anim(local->chris_daae_dynamic, aa[0], 14);
				kernel_dynamic_anim(local->chris_daae_dynamic, aa[0], 16);

				kernel_reset_animation(aa[0], 212); /* keep her standing by mirror */
				player.walker_visible = true;
				local->anim_1_running = false;

			} else {

				switch (local->day_action) {
				case CONV13_DAY_TALK:
				case CONV13_DAY_GET_READY_TALK:
				case CONV13_DAY_MIRROR:
				case CONV13_DAY_WAVE:
					aa[0]                 = kernel_run_animation(kernel_name('d', 1), 0);
					local->anim_0_running = true;
					local->day_action     = CONV13_DAY_MIRROR;
					kernel_reset_animation(aa[0], 208); /* keep her looking into dresser mirror */
					kernel_flip_hotspot(words_Christine, true);
					kernel_flip_hotspot_loc(words_Christine, false, CHRIS_DEAD_X, CHRIS_DEAD_Y);
					break;

				/* case CONV13_DAY_STAND_TALK:     */
				/* case CONV13_DAY_STAND_SHUT_UP:  */
				/* case CONV13_BOTH_SHUT_UP:        */

				default:
					aa[0]                 = kernel_run_animation(kernel_name('d', 2), 0);
					local->anim_0_running = true;
					local->day_action     = CONV13_BOTH_SHUT_UP;
					player.walker_visible = false;
					kernel_reset_animation(aa[0], 165); /* keep both of them looking at each other while standing */
					break;
				}
				local->chris_daae_dynamic = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
				                                               0, 0, 0, 0);
				kernel_dynamic_hot[local->chris_daae_dynamic].prep = PREP_ON;
				kernel_dynamic_anim(local->chris_daae_dynamic, aa[0], 13);
				kernel_dynamic_anim(local->chris_daae_dynamic, aa[0], 14);
				kernel_dynamic_anim(local->chris_daae_dynamic, aa[0], 16);
			}

			if (local->day_action == CONV13_DAY_MIRROR) {
				kernel_flip_hotspot(words_Christine, true);
				kernel_flip_hotspot_loc(words_Christine, false, CHRIS_DEAD_X, CHRIS_DEAD_Y);

			} else {
				player.x      = PLAYER_X_AFTER_CONV13;
				player.y      = PLAYER_Y_AFTER_CONV13;
				player.facing = FACING_NORTHEAST;
			}

		} else if ((global[done_brie_conv_203] == YES) || (global[done_brie_conv_203] == YES_AND_CHASE)) {
			/* completed conv with brie in 203 - show dead Florent */

			aa[4]                       = kernel_run_animation(kernel_name('x', 1), 1);
			local->julie_action         = CONV6_JULIE_CRY;
			local->anim_4_running       = true;
			global[make_brie_leave_203] = true;
			seq[fx_dress]               = kernel_seq_stamp(ss[fx_dress], false, 1);
			seq[fx_corpse]              = kernel_seq_stamp(ss[fx_corpse], false, 1);
			kernel_seq_depth(seq[fx_dress],  14);
			kernel_seq_depth(seq[fx_corpse], 14);
			kernel_flip_hotspot(words_dressing_gown, true);
			kernel_flip_hotspot(words_Julie,         true);
			kernel_flip_hotspot_loc(words_Christine, true, CHRIS_DEAD_X, CHRIS_DEAD_Y);

		} else if (global[current_year] == 1993) { /* start Florent anim */
			seq[fx_dress] = kernel_seq_stamp(ss[fx_dress], false, 1);
			kernel_seq_depth(seq[fx_dress], 14);
			aa[2]                 = kernel_run_animation(kernel_name('f', 1), 1);
			local->anim_2_running = true;  /* anim of Florent by dresser */

			if (local->florent_action != CONV4_FLORENT_HOLD_HAND) {
				local->florent_action = CONV4_FLORENT_SHUT_UP;
			}

			aa[3]                 = kernel_run_animation(kernel_name('r', 1), 1);
			local->anim_3_running = true;  /* anim of Raoul on couch */

			if (local->raoul_action == CONV4_RAOUL_INVISIBLE) {
				kernel_reset_animation(aa[3], 33); /* this will keep Raoul invisible */

			} else {
				kernel_reset_animation(aa[3], 8); /* this will keep Raoul frozen on couch */
				local->raoul_action   = CONV4_RAOUL_SHUT_UP;
				player.x              = COUCH_X;
				player.y              = COUCH_Y;
				player.facing         = FACING_SOUTH;
				player.walker_visible = false;
			}

			if (local->florent_action == CONV4_FLORENT_HOLD_HAND) {
				kernel_reset_animation(aa[2], 41); /* this will make Florent hold Raoul's hand */
			}

			if (global[florent_name_is_known] >= YES) {
				local->chris_flor_dynamic = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
				                                               DYN_FLORENT_X, DYN_FLORENT_Y,
				                                               DYN_FLORENT_X_SIZE, DYN_FLORENT_Y_SIZE);
				kernel_dynamic_hot[local->chris_flor_dynamic].prep = PREP_ON;
				kernel_dynamic_walk(local->chris_flor_dynamic, SCREEN_X, SCREEN_Y, FACING_SOUTHWEST);

			} else {
				local->chris_flor_dynamic = kernel_add_dynamic(words_woman, words_walk_to, SYNTAX_FEM_NOT_PROPER, KERNEL_NONE,
				                                               DYN_FLORENT_X, DYN_FLORENT_Y,
				                                               DYN_FLORENT_X_SIZE, DYN_FLORENT_Y_SIZE);
				kernel_dynamic_hot[local->chris_flor_dynamic].prep = PREP_ON;
				kernel_dynamic_walk(local->chris_flor_dynamic, SCREEN_X, SCREEN_Y, FACING_SOUTHWEST);
			}

			kernel_flip_hotspot(words_dressing_gown, true);

			if (object_is_here(small_note)) {
				kernel_flip_hotspot(words_small_note, true);
				seq[fx_small_note] = kernel_seq_stamp(ss[fx_small_note], false, 1);
				kernel_seq_depth(seq[fx_small_note], 13);
			}
		}

	} else if (global[current_year] == 1993) {
		if ((global[done_brie_conv_203] == YES) || (global[done_brie_conv_203] == YES_AND_CHASE)) {
			/* completed conv with brie in 203 - show dead Florent */

			if (!(global[player_score_flags] & SCORE_DEAD_FLORENT)) {
				global[player_score_flags] = global[player_score_flags] | SCORE_DEAD_FLORENT;
				global[player_score] += 5;
			}

			aa[4]                       = kernel_run_animation(kernel_name('x', 1), 1);
			local->julie_action         = CONV6_JULIE_CRY;
			local->anim_4_running       = true;
			global[make_brie_leave_203] = true;
			player.x                    = PLAYER_X_FROM_111;
			player.y                    = PLAYER_Y_FROM_111;
			player.facing               = FACING_NORTH;
			player.commands_allowed     = false;
			seq[fx_dress]               = kernel_seq_stamp(ss[fx_dress], false, 1);
			seq[fx_corpse]              = kernel_seq_stamp(ss[fx_corpse], false, 1);
			kernel_seq_depth(seq[fx_dress],  14);
			kernel_seq_depth(seq[fx_corpse], 14);
			kernel_flip_hotspot(words_dressing_gown, true);
			kernel_flip_hotspot(words_Julie,         true);
			kernel_flip_hotspot_loc(words_Christine, true, CHRIS_DEAD_X, CHRIS_DEAD_Y);
			player_walk(CORPSE_X, CORPSE_Y, FACING_NORTHEAST);
			player_walk_trigger(ROOM_113_AT_CORPSE);

		} else {  /* walk in & start conv with Florent */
			seq[fx_dress] = kernel_seq_stamp(ss[fx_dress], false, 1);
			kernel_seq_depth(seq[fx_dress], 14);
			aa[2]                 = kernel_run_animation(kernel_name('f', 1), 1);
			local->florent_action = CONV4_FLORENT_SHUT_UP;
			local->anim_2_running = true;

			aa[3]                 = kernel_run_animation(kernel_name('r', 1), 1);
			local->raoul_action   = CONV4_RAOUL_INVISIBLE;
			local->anim_3_running = true;
			kernel_reset_animation(aa[3], 33); /* this will keep Raoul invisible */

			if (global[florent_name_is_known] >= YES) {
				local->chris_flor_dynamic = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
				                                               DYN_FLORENT_X, DYN_FLORENT_Y,
				                                               DYN_FLORENT_X_SIZE, DYN_FLORENT_Y_SIZE);
				kernel_dynamic_hot[local->chris_flor_dynamic].prep = PREP_ON;
				kernel_dynamic_walk(local->chris_flor_dynamic, SCREEN_X, SCREEN_Y, FACING_SOUTHWEST);

			} else {
				local->chris_flor_dynamic = kernel_add_dynamic(words_woman, words_walk_to, SYNTAX_FEM_NOT_PROPER, KERNEL_NONE,
				                                               DYN_FLORENT_X, DYN_FLORENT_Y,
				                                               DYN_FLORENT_X_SIZE, DYN_FLORENT_Y_SIZE);
				kernel_dynamic_hot[local->chris_flor_dynamic].prep = PREP_ON;
				kernel_dynamic_walk(local->chris_flor_dynamic, SCREEN_X, SCREEN_Y, FACING_SOUTHWEST);
			}

			player.x      = PLAYER_X_FROM_111;
			player.y      = PLAYER_Y_FROM_111;
			player.facing = FACING_NORTH;

			kernel_flip_hotspot(words_dressing_gown, true);
			if (object_is_here(small_note)) {
				kernel_flip_hotspot(words_small_note, true);
				seq[fx_small_note] = kernel_seq_stamp(ss[fx_small_note], false, 1);
				kernel_seq_depth(seq[fx_small_note], 13);
			}
		}

	} else if ((global[current_year] == 1881) && (!global[chris_kicked_raoul_out])) {

		aa[1]                 = kernel_run_animation(kernel_name('r', 1), 1);
		local->raoul_action   = CONV13_RAOUL_INVISIBLE;
		local->anim_1_running = true;
		kernel_reset_animation(aa[1], 33); /* this will keep Raoul invisible */

		aa[0]                 = kernel_run_animation(kernel_name('d', 1), 1);
		local->day_action     = CONV13_DAY_MIRROR;
		local->anim_0_running = true;

		kernel_flip_hotspot(words_Christine, true);
		kernel_flip_hotspot_loc(words_Christine, false, CHRIS_DEAD_X, CHRIS_DEAD_Y);
		kernel_timing_trigger(ONE_SECOND, ROOM_113_AT_DRESSER);
		player.x      = PLAYER_X_FROM_111;
		player.y      = PLAYER_Y_FROM_111;
		player.facing = FACING_NORTH;
		/* Raoul comes into room, Daae is sitting down by mirror */

	} else if ((global[current_year] == 1881) && (global[chris_kicked_raoul_out] == CHRIS_KICKED_YES)) {
		global[chris_kicked_raoul_out] = CAME_INTO_EMPTY_113; /* so we don't repeat these actions */
		player.x                = PLAYER_X_FROM_111;
		player.y                = PLAYER_Y_FROM_111;
		player.facing           = FACING_NORTH;
		global[came_from_fade]  = true;
		player.commands_allowed = false;
		global[player_score]   += 8;
		kernel_timing_trigger(ONE_SECOND, ROOM_113_WAIT_FOR_FADE_IN);
		/* Raoul comes into room and she is gone */

	} else if (previous_room == 111) {
		player.x      = PLAYER_X_FROM_111;
		player.y      = PLAYER_Y_FROM_111;
		player.facing = FACING_NORTH;
	}


	/* ============= If restoring game into conv, run it ============ */

	switch (conv_restore_running) {
	case CONV_LOVE_SCENE:
		conv_run(CONV_LOVE_SCENE);
		conv_export_pointer(&global[player_score]);
		break;

	case CONV_FLORENT:
		conv_run(CONV_FLORENT);
		conv_export_pointer(&global[player_score]);
		break;

	case CONV_FLORENT_DEAD:
		conv_run(CONV_FLORENT_DEAD);
		break;
	}

	section_1_music();
}

void room_113_daemon() {
	if (local->anim_1_running) {
		handle_animation_13_raoul();
	}

	if (local->anim_4_running) {
		handle_animation_6_julie();
	}

	if (local->anim_2_running) {
		handle_animation_florent();
	}

	if (local->anim_3_running) {
		handle_animation_4_raoul();

		if ((!player_has(small_note)) && (kernel_anim[aa[3]].frame == 26)) {
			kernel_flip_hotspot(words_small_note, false);
			inter_give_to_player(small_note);
			kernel_seq_delete(seq[fx_small_note]);
			sound_play(N_TakeObjectSnd);
			global_speech_ready = -1;
			object_examine(small_note, text_008_06, 2);
		}
	}

	if ((kernel_anim[aa[0]].frame == 86) && (!local->prevent_2)) {
		kernel_abort_animation(aa[0]);
		local->prevent_2 = true;
		aa[0]            = kernel_run_animation(kernel_name('d', 2), 0);
		kernel_reset_animation(aa[0], 86);
		kernel_synch(KERNEL_ANIM, aa[0], KERNEL_NOW, 0);
		kernel_process_all_animations();
	}

	if (local->anim_0_running) {
		handle_animation_13_day();

		if ((kernel_anim[aa[0]].frame == 95) && (!local->arms_are_out)) {
			/* her hands are outstretched */
			local->standing_talking = true;
			local->day_action       = CONV13_BOTH_SHUT_UP;
			local->raoul_action     = CONV13_RAOUL_INVISIBLE;
			local->arms_are_out     = true;
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_ANIM, aa[1]);
		}

		if ((kernel_anim[aa[0]].frame == 165) && (!local->right_after_kiss)) {
			/* immediately after kiss */
			local->right_after_kiss = true;
		}
	}


	switch (kernel.trigger) {
	case ROOM_113_WAIT_FOR_FADE_IN:
		text_show(text_113_31);
		player_walk(MIRROR_X, MIRROR_Y, FACING_EAST);
		player_walk_trigger(ROOM_113_BY_MIRROR);
		break;

	case ROOM_113_BY_MIRROR:
		text_show(text_113_32);
		text_show(text_113_33);
		new_room = 150;
		global[leave_angel_music_on] = false;
		break;

	case ROOM_113_AT_CORPSE:
		player.commands_allowed = true;
		conv_run(CONV_FLORENT_DEAD);
		/* once he gets to corpse of Florent, start conv 6 */
		break;

	case ROOM_113_AT_DRESSER:
		if (!player.been_here_before) {
			text_show(text_113_42);
		}
		break;
	}
}

static void process_conversation_dead() {
	switch (kernel.trigger) {

	/* =================== Process talking triggers ================ */

	case ROOM_113_JULIE_START_TALKING:
		local->julie_action = CONV6_JULIE_TALK;
		break;

	case ROOM_113_JULIE_STOP_TALKING:
		local->julie_action = CONV6_JULIE_CRY;
		break;
	}

	/* ================= Set up me and you triggers ================ */

	conv_you_trigger(ROOM_113_JULIE_START_TALKING);
	conv_me_trigger(ROOM_113_JULIE_STOP_TALKING);

	local->julie_talk_count = 0;
}

static void process_conversation_florent() {
	int random;

	switch (player_verb) {
	case conv004_dontleave_help:
		if (!local->prevent_2) {
			kernel_timing_trigger(TWO_SECONDS, ROOM_113_DELAY_BEFORE_WALK);
			conv_you_trigger(ROOM_113_PUT_ON_HOLD);
			local->florent_action = CONV4_FLORENT_TALK;
			local->prevent_2      = true;
		}
		break;

	case conv004_promises_continue:
	case conv004_promises_safe:
		conv_you_trigger(ROOM_113_HOLD_HAND);
		break;

	case conv004_promises_return:
	case conv004_promises_cant:
		conv_you_trigger(ROOM_113_END_HOLD_HAND);
		break;

	case conv004_name_raoul:
	case conv004_name_dear:
		if (!local->prevent_1) {
			global[florent_name_is_known] = YES;
			conv_you_trigger(ROOM_113_BEFORE_COUCH);
			kernel_delete_dynamic(local->chris_flor_dynamic);
			local->chris_flor_dynamic = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
			                                               DYN_FLORENT_X, DYN_FLORENT_Y,
			                                               DYN_FLORENT_X_SIZE, DYN_FLORENT_Y_SIZE);
			kernel_dynamic_hot[local->chris_flor_dynamic].prep = PREP_ON;
			kernel_dynamic_walk(local->chris_flor_dynamic, SCREEN_X, SCREEN_Y, FACING_SOUTHWEST);
			local->prevent_1 = true;
		}
		break;

	case conv004_anything_take:
		local->florent_action = CONV4_FLORENT_POINT;
		conv_you_trigger(ROOM_113_TAKE_NOTE);
		break;

	case conv004_imgone_bye:
		if (local->raoul_action != CONV4_RAOUL_INVISIBLE) {
			local->raoul_action = CONV4_RAOUL_GET_UP;
		}
		break;
	}

	switch (kernel.trigger) {
	case ROOM_113_PUT_ON_HOLD:
		conv_hold();
		break;

	case ROOM_113_END_HOLD_HAND:
		local->florent_action        = CONV4_FLORENT_SHUT_UP;
		global[stop_walker_disabled] = false;
		break;

#if 0
	// TODO: This case was mapped to a second "case 80" in the release executable, and can
	// never be reached. Need to see if the label "ROOM_113_HOLD_HAND", which is 98, is
	// legitimate, and this case can be re-enabled
	case ROOM_113_HOLD_HAND:
		local->florent_action = CONV4_FLORENT_HOLD_HAND;
		break;
#endif

	case ROOM_113_MADE_IT_TO_FLORENT:
		global[stop_walker_disabled]  = true;
		global[florent_name_is_known] = YES_AND_END_CONV;
		conv_release();
		break;

	case ROOM_113_DELAY_BEFORE_WALK:
		player_walk(FLORENT_TOUCH_HAND_X, FLORENT_TOUCH_HAND_Y, FACING_SOUTHWEST);
		player_walk_trigger(ROOM_113_MADE_IT_TO_FLORENT);
		break;

	case ROOM_113_TAKE_NOTE:
		local->raoul_action = CONV4_RAOUL_TAKE_NOTE;
		conv_hold();
		break;

	case ROOM_113_BEFORE_COUCH:
		local->florent_action = CONV4_FLORENT_TALK;
		kernel_timing_trigger(TWO_SECONDS, ROOM_113_BETWEEN_COUCH);
		conv_hold();		// hold so options don't appear in interface
		break;

	case ROOM_113_BETWEEN_COUCH:
		player_walk(COUCH_X, COUCH_Y, FACING_SOUTH);
		player_walk_trigger(ROOM_113_AT_COUCH);
		break;

	case ROOM_113_AT_COUCH:
		conv_release();
		player.walker_visible = false;
		local->raoul_action   = CONV4_RAOUL_SHUT_UP;
		kernel_synch(KERNEL_ANIM, aa[3], KERNEL_PLAYER, 0);
		kernel_reset_animation(aa[3], 1);
		/* when made it to couch, run frame 1 of sitting him down */
		/* and make options appear in interface */
		break;


	/* =================== Process talking triggers ================ */

	case ROOM_113_FLORENT_START_TALKING2:
		if ((local->florent_action != CONV4_FLORENT_HOLD_HAND) &&
		    (local->florent_action != CONV4_FLORENT_POINT)) {
			random = imath_random(1, 5);
			if (random == 1) {
				local->florent_action = CONV4_FLORENT_TALK;
			}
		}

		if ((local->raoul_action != CONV4_RAOUL_INVISIBLE) &&
		    (local->raoul_action != CONV4_RAOUL_TAKE_NOTE) &&
		    (local->raoul_action != CONV4_RAOUL_GET_UP) &&
		    (!local->raoul_is_up)) {
			local->raoul_action = CONV4_RAOUL_SHUT_UP;
			random = imath_random(1, 2);	// chance for hand on chin
			if (random == 1) {
				local->raoul_action = CONV4_RAOUL_CHIN;
			} /* if Raoul is visible, stop talking and maybe put hand on chin */
		}
		break;

	case ROOM_113_RAOUL_START_TALKING:
		if ((player_verb != conv004_name_raoul) &&
		    (player_verb != conv004_name_dear)  &&
		    (player_verb != conv004_name_sorry) &&
		    (player_verb != conv004_bon_jour_hello) &&
		    (player_verb != conv004_who_who_are) &&
		    (player_verb != conv004_monsieur_again)) {
			/* I don't want her to stick arm up for these player_verbs */

			if ((local->raoul_action != CONV4_RAOUL_TAKE_NOTE) &&
			    (local->raoul_action != CONV4_RAOUL_GET_UP) &&
			    (!local->raoul_is_up)) {
				local->raoul_action = CONV4_RAOUL_TALK;
			}

			if ((local->florent_action != CONV4_FLORENT_HOLD_HAND) &&
			    (local->florent_action != CONV4_FLORENT_POINT)) {
				local->florent_action = CONV4_FLORENT_SHUT_UP;
			}
		}
		break;

	default:
		break;
	}

	/* ================= Set up me and you triggers ================ */

	if ((player_verb != conv004_name_raoul)        &&
	    (player_verb != conv004_name_dear)         &&
	    (player_verb != conv004_anything_take)     &&
	    (player_verb != conv004_promises_return)   &&
	    (player_verb != conv004_promises_cant)     &&
	    (player_verb != conv004_promises_continue) &&
	    (player_verb != conv004_promises_safe)     &&
	    (player_verb != conv004_dontleave_help)) {
		conv_you_trigger(ROOM_113_FLORENT_START_TALKING);
	} /* there is a you trigger called above for the above player_verbs */

	conv_me_trigger(ROOM_113_RAOUL_START_TALKING);

	local->raoul_talk_count   = 0;
	local->florent_talk_count = 0;
}

static void process_conversation_love() {
	int random;

	switch (player_verb) {
	case conv013_returned_abc:
	case conv013_again_abc:
		if (!local->prevent_1) {
			conv_you_trigger(ROOM_113_BEFORE_COUCH);
			local->prevent_1 = true;
		}
		break;

	case conv013_lovescene_openline:
		/* if the player verb == "come here my love" then hold conversation and
		   set up trigger to wait for frame 96 (arms are held out toward Raoul) */
		if (!local->arms_are_out) {
			local->day_action = CONV13_DAY_GET_UP;	// time to get up
			kernel_timing_trigger(SMALL_NUMBER_OF_TICKS, ROOM_113_CHECK_FOR_ARMS_OUT);
			kernel_init_dialog();  /* clear interface */
			kernel_set_interface_mode(INTER_CONVERSATION);
			kernel_flip_hotspot(words_Christine, false);
			conv_hold();
		}
		break;

	case conv013_practice_first:
		sound_play(N_AngelMus001);
		global[leave_angel_music_on] = true;
		break;

	/* case conv013_finale_violin: */
	/* case conv013_finale_adieu: */
	case conv013_biteme_b_b:
		if (!local->music_is_playing) {
			conv_hold();
			local->day_action       = CONV13_BOTH_SHUT_UP;
			local->music_is_playing = true;
			local->chris_daae_dynamic = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
			                                               0, 0, 0, 0);
			kernel_dynamic_hot[local->chris_daae_dynamic].prep = PREP_ON;
			kernel_dynamic_anim(local->chris_daae_dynamic, aa[0], 14);
			kernel_dynamic_anim(local->chris_daae_dynamic, aa[0], 16);
		}
		break;
	}

	/* if daemon code does not set local->arms_are_out to true, then run this code */
	/* again.  When it does reach frame 96 (and set to true), then resume conv to  */
	/* say "come here my love with arms outreached.                                */

	switch (kernel.trigger) {
	case ROOM_113_BEFORE_COUCH:
		kernel_timing_trigger(TWO_SECONDS, ROOM_113_BETWEEN_COUCH);
		conv_hold(); /* hold so options don't appear in interface */
		local->day_action        = CONV13_DAY_GET_READY_TALK;
		local->day_wants_to_talk = true;
		break;

	case ROOM_113_BETWEEN_COUCH:
		player_walk(COUCH_X, COUCH_Y, FACING_SOUTH);
		player_walk_trigger(ROOM_113_AT_COUCH);
		break; /* give a 3 sec. delay before walking to couch */

	case ROOM_113_AT_COUCH:
		conv_release();
		player.walker_visible = false;
		local->raoul_action   = CONV13_RAOUL_SHUT_UP;
		kernel_synch(KERNEL_ANIM, aa[1], KERNEL_PLAYER, 0);
		kernel_reset_animation(aa[1], 1);
		/* when made it to couch, run frame 1 of sitting him down */
		/* and make options appear in interface */
		break;

	case ROOM_113_CHECK_FOR_ARMS_OUT:
	case ROOM_113_WAIT_TO_SIT:
		if (local->arms_are_out) {
			conv_release();		// let her say "come here my love"
			conv_you_trigger(ROOM_113_HOLD_OPTIONS); // so we can not display options
		}
		break;

	case ROOM_113_HOLD_OPTIONS:
		conv_hold();
		kernel_timing_trigger(SMALL_NUMBER_OF_TICKS, ROOM_113_CHECK_FOR_AFTER_KISS);
		break;

	case ROOM_113_CHECK_FOR_AFTER_KISS:
		if (local->right_after_kiss) { /* frame 165 */
			conv_release();
		} else {
			kernel_timing_trigger(SMALL_NUMBER_OF_TICKS, ROOM_113_CHECK_FOR_AFTER_KISS);
		}
		break;


	/* =================== Process talking triggers ================ */

	case ROOM_113_DAY_START_TALKING:
		if (local->standing_talking) {
			local->day_action = CONV13_DAY_STAND_TALK;

		} else {
			if (player_verb == conv013_dreams_romantic) {
				local->day_action = CONV13_DAY_WAVE;
			} else {
				local->day_action        = CONV13_DAY_GET_READY_TALK;
				local->day_wants_to_talk = true;
			} /* if dreams_romantic, then have her wave hand, else just talk */

			if (local->raoul_action != CONV13_RAOUL_INVISIBLE) {
				local->raoul_action = CONV13_RAOUL_SHUT_UP;
				random = imath_random(1, 2); /* chance for hand on chin */
				if (random == 1) {
					local->raoul_action = CONV13_RAOUL_CHIN;
				} /* if Raoul is visible, stop talking and maybe put hand on chin */
			}
		}
		break;

	case ROOM_113_RAOUL_START_TALKING:
		if (player_verb != conv013_intro_hello) {
			if (local->standing_talking) {
				local->day_action = CONV13_DAY_STAND_SHUT_UP;

			} else if (local->day_action != CONV13_DAY_GET_UP) {
				local->day_action        = CONV13_DAY_GET_READY_TALK;
				local->raoul_action      = CONV13_RAOUL_TALK;
				local->day_wants_to_talk = false;
			}
		}
		break;

	default:
		break;
	}


	/* ================= Set up me and you triggers ================ */

	if ((player_verb != conv013_lovescene_openline) &&
	    (player_verb != conv013_returned_abc) &&
	    (player_verb != conv013_again_abc)) {
		conv_you_trigger(ROOM_113_DAY_START_TALKING);
	} /* there is a you trigger called above for 3 other player_verbs */

	conv_me_trigger(ROOM_113_RAOUL_START_TALKING);

	local->raoul_talk_count = 0;
	local->day_talk_count   = 0;
	local->stand_talk_count = 0;
}

void room_113_pre_parser() {
	if (player_said_2(walk_up, aisle)) {
		player.walk_off_edge_to_room = 101;
	}

	if (player_said_2(look, mirror)) {
		player_walk(MIRROR_X, MIRROR_Y, FACING_EAST);
	}

	if ((!player_said_2(exit_to, corridor)) &&
	    (!global[chris_kicked_raoul_out]) &&
	    (!player_said_2(look, Christine)) &&
	    (!player_said_2(take, Christine)) &&
	    (local->music_is_playing)) {
		conv_run(CONV_LOVE_SCENE);
		conv_export_pointer(&global[player_score]);
		player_cancel_command();
		/* if player said anything but leave room during conv */
		/* with Daae, then re-enter conv */

	} else if ((player_said_2(talk_to, woman)) ||
	           (player_said_2(talk_to, Christine))) {
		player_walk(SCREEN_X, SCREEN_Y, FACING_SOUTHWEST);
	}

	if ((!player_said_2(exit_to, corridor)) &&
	    (!player_said_2(look, Julie)) &&
	    (!player_said_2(look, Christine)) &&
	    (!player.look_around) &&
	    (global[done_brie_conv_203] != NO) &&
	    (global[current_year] == 1993)) {

		if (player_said_2(talk_to, Christine)) {
			player.need_to_walk = false;

		} else if (player_said_1(look)) {
			player.need_to_walk = false;

		} else if (player_said_2(talk_to, Julie)) {
			player.need_to_walk = false;
			conv_run(CONV_FLORENT_DEAD);
			player_cancel_command();

		} else {
			text_show(text_113_50);
			player.need_to_walk = false;
			player_cancel_command();
		}

	} /* if player said anything but leave room during conv */
	  /* with Julie (dead Florent is here), then re-enter conv */
}


void room_113_parser() {
	if (conv_control.running == CONV_LOVE_SCENE) {
		process_conversation_love();
		goto handled;
	}

	if (conv_control.running == CONV_FLORENT) {
		process_conversation_florent();
		goto handled;
	}

	if (conv_control.running == CONV_FLORENT_DEAD) {
		process_conversation_dead();
		goto handled;
	}

	if ((player_said_2(talk_to, woman)) ||
	    (player_said_2(talk_to, Christine))) {
		if (global[current_year] == 1881) {
			player.commands_allowed = true;
			conv_run(CONV_LOVE_SCENE);
			conv_export_pointer(&global[player_score]);
		} else {
			if (global[done_brie_conv_203]) {
				text_show(text_113_48);
				/* talk to dead Florent */
			} else {
				player.commands_allowed = true;
				conv_run(CONV_FLORENT);
				conv_export_pointer(&global[player_score]);
			}
		} /* once he gets to dresser, start conv 13 or 4 */
		goto handled;
	}

	if (player_said_2(exit_to, corridor)) {
		if (global[done_brie_conv_203] == YES) {
			global[done_brie_conv_203] = YES_AND_CHASE;
		}
		new_room = 111;
		goto handled;
	}

	if (player.look_around) {
		if (global[current_year] == 1993) {
			if (global[done_brie_conv_203]) {
				text_show(text_113_40); /* when Christine's body is here */
			} else {
				text_show(text_113_10);
			}
		} else {
			text_show(text_113_11);
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(floor)) {
			text_show(text_113_12);
			goto handled;
		}

		if (player_said_1(rug)) {
			text_show(text_113_13);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_113_14);
			goto handled;
		}

		if (player_said_1(couch)) {
			text_show(text_113_15);
			goto handled;
		}

		if (player_said_1(mural)) {
			text_show(text_113_16);
			goto handled;
		}

		if (player_said_1(painting)) {
			if (global[current_year] == 1881) {
				text_show(text_113_17);
			} else {
				text_show(text_113_43);
			}
			goto handled;
		}

		// NOTE: duplicate player_said_1(painting) check — unreachable dead code from original
		if (player_said_1(painting)) {
			text_show(text_113_17);
			goto handled;
		}

		if (player_said_1(dressing_table)) {
			text_show(text_113_18);
			goto handled;
		}

		if (player_said_1(chair)) {
			text_show(text_113_19);
			goto handled;
		}

		if (player_said_1(mirror)) {
			if (global[current_year] == 1993) {
				text_show(text_113_44);
			} else {
				text_show(text_113_20);
			}
			goto handled;
		}

		if (player_said_1(fan)) {
			text_show(text_113_21);
			goto handled;
		}

		if (player_said_1(vase)) {
			text_show(text_113_22);
			goto handled;
		}

		if (player_said_1(hat_rack)) {
			text_show(text_113_23);
			goto handled;
		}

		if (player_said_1(light_fixture)) {
			text_show(text_113_24);
			goto handled;
		}

		if (player_said_1(ceiling)) {
			text_show(text_113_25);
			goto handled;
		}

		if (player_said_1(wardrobe)) {
			text_show(text_113_26);
			goto handled;
		}

		if (player_said_1(dressing_screen)) {
			text_show(text_113_27);
			goto handled;
		}

		if (player_said_1(corridor)) {
			text_show(text_113_28);
			goto handled;
		}

		if (player_said_1(dressing_gown)) {
			text_show(text_113_30);
			goto handled;
		}

		if (player_said_1(small_note) && object_is_here(small_note)) {
			text_show(text_113_49);
			goto handled;
		}

		if (player_said_1(Christine)) {
			if (global[done_brie_conv_203]) { /* completed conv with brie in 203 - Florent is dead */
				text_show(text_113_38);
				goto handled;
			}
		}

		if (player_said_1(Julie)) {
			text_show(text_113_39);
			goto handled;
		}

		if (player_said_1(notice)) {
			text_show(text_113_47);
			goto handled;
		}

		if (player_said_1(clothes_dummy)) {
			if (global[current_year] == 1993) {
				text_show(text_113_45);
			} else {
				text_show(text_113_46);
			}
			goto handled;
		}

		if (player_said_1(woman) || player_said_1(Christine)) {
			if (global[current_year] == 1993) {
				text_show(text_113_41);
			} else {
				if (local->music_is_playing) { /* at end of love scene */
					text_show(text_113_36);
				} else {
					text_show(text_113_42);
				}
			}
			goto handled;
		}
	}

	if (player_said_2(open, wardrobe)) {
		text_show(text_113_29);
		goto handled;
	}

	if (player_said_2(take, small_note)) {
		if (object_is_here(small_note)) {
			text_show(text_113_34);
			goto handled;
		}
	}

	if (player_said_2(take, dressing_gown)) {
		text_show(text_113_35);
		goto handled;
	}

	if (player_said_2(take, Christine) || player_said_2(take, woman)) {
		text_show(text_113_37);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_113_preload() {
	room_init_code_pointer       = room_113_init;
	room_pre_parser_code_pointer = room_113_pre_parser;
	room_parser_code_pointer     = room_113_parser;
	room_daemon_code_pointer     = room_113_daemon;

	section_1_walker();
	section_1_interface();

	vocab_make_active(words_Christine);
	vocab_make_active(words_woman);
	vocab_make_active(words_Julie);
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
