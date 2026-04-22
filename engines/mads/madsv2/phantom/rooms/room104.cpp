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
#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/rooms/room104.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

static void get_rid_of_inventory() {
	if (player_has(large_note)) {
		inter_move_object(large_note, NOWHERE);
	}

	if (player_has(sandbag)) {
		inter_move_object(sandbag, NOWHERE);
	}

	if (player_has(small_note)) {
		inter_move_object(small_note, NOWHERE);
	}

	if (player_has(parchment)) {
		inter_move_object(parchment, NOWHERE);
	}

	if (player_has(book)) {
		inter_move_object(book, NOWHERE);
	}

	if (player_has(red_frame)) {
		inter_move_object(red_frame, 105);
	}

	if (player_has(yellow_frame)) {
		inter_move_object(yellow_frame, 107);
	}

	if (player_has(blue_frame)) {
		inter_move_object(blue_frame, 302);
	}

	if (player_has(green_frame)) {
		inter_move_object(green_frame, 307);
	}
}

static void handle_animation_daae_walk() {
	int daae_walk_reset_frame;

	if (kernel_anim[aa[2]].frame != local->daae_walk_frame) {
		local->daae_walk_frame = kernel_anim[aa[2]].frame;
		daae_walk_reset_frame  = -1;

		switch (local->daae_walk_frame) {
		case 1:  /* end of invisible */
			if (local->daae_walk_action == CONV7_DAAE_WALK_INVISIBLE) {
				daae_walk_reset_frame = 0;
			} else {
				kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, aa[1]);
				daae_walk_reset_frame = 1;
			}
			break;

		case 138:  /* end of walking away */
			local->daae_walk_action = CONV7_DAAE_WALK_INVISIBLE;
			daae_walk_reset_frame   = 0;
			break;
		}

		if (daae_walk_reset_frame >= 0) {
			kernel_reset_animation(aa[2], daae_walk_reset_frame);
			local->daae_walk_frame = daae_walk_reset_frame;
		}
	}
}

static void handle_animation_rich() {
	int random = 0;
	int rich_reset_frame;

	if (kernel_anim[aa[0]].frame != local->rich_frame) {
		local->rich_frame = kernel_anim[aa[0]].frame;
		rich_reset_frame  = -1;

		switch (local->rich_frame) {
		case 117: /* somewhere into Richard walking away */
			local->couple_action = CONV7_COUPLE_RAOUL_ALONE;
			break;

		case 1:  /* end of freeze       */
		case 2:  /* end of talk 1       */
		case 3:  /* end of talk 2       */
		case 4:  /* end of talk 3       */
		case 8:  /* end of head right   */
		case 14: /* end of head left    */
		case 44: /* end of head up      */
		case 48: /* end of head down    */
		case 22: /* end of hand on chin */
		case 34: /* end of arms out     */
		case 40: /* end of point        */

			if (local->rich_action == CONV7_RICH_TALK) {
				random = imath_random(1, 3);
				++local->rich_talk_count;
				if (local->rich_talk_count > 15) {
					local->rich_action = CONV7_RICH_SHUT_UP;
					random = 40;
				}
			}

			if (local->rich_action == CONV7_RICH_SHUT_UP) {
				random = imath_random(7, 80);
			}

			if (local->rich_action == CONV7_RICH_ARMS_OUT) {
				random = 4;
				local->rich_action = CONV7_RICH_TALK;
				local->rich_talk_count = 8;
			}

			if (local->rich_action == CONV7_RICH_POINT) {
				random = 5;
				local->rich_action = CONV7_RICH_TALK;
				local->rich_talk_count = 8;
			}

			if (local->rich_action == CONV7_RICH_WALK) {
				random = 6;
			}

			if (local->rich_action == CONV7_RICH_LOOK_UP) {
				random = 7;
			}

			switch (random) {
			case 1:  rich_reset_frame = 1;   break; /* do talk 1 */
			case 2:  rich_reset_frame = 2;   break; /* do talk 2 */
			case 3:  rich_reset_frame = 3;   break; /* do talk 3 */
			case 4:  rich_reset_frame = 23;  break; /* put arms out */
			case 5:  rich_reset_frame = 35;  break; /* point */
			case 6:  rich_reset_frame = 49;  break; /* walk away */
			case 7:  rich_reset_frame = 41;  break; /* look up */
			case 8:  rich_reset_frame = 45;  break; /* look down */
			case 9:  rich_reset_frame = 9;   break; /* look left */
			case 10: rich_reset_frame = 5;   break; /* look right */
			case 11: rich_reset_frame = 15;  break; /* hand on chin */
			default: rich_reset_frame = 0;   break; /* shut up */
			}
			break;

		case 125:
			rich_reset_frame = 124;
			break; /* keep Richard invisible */
		}

		if (rich_reset_frame >= 0) {
			kernel_reset_animation(aa[0], rich_reset_frame);
			local->rich_frame = rich_reset_frame;
		}
	}
}

static void handle_animation_couple() {
	int random = 0;
	int couple_reset_frame;

	if (kernel_anim[aa[1]].frame != local->couple_frame) {
		local->couple_frame = kernel_anim[aa[1]].frame;
		couple_reset_frame  = -1;

		switch (local->couple_frame) {
		case 1:   /* end of laying flat out on the stage  */
		case 103: /* end of her talking to him 1 while he is flat on stage */
		case 104: /* end of her talking to him 2 while he is flat on stage */
		case 105: /* end of her talking to him 3 while he is flat on stage */

			if (local->couple_action == CONV7_COUPLE_STAY_DOWN) {
				couple_reset_frame = 0;

			} else {
				couple_reset_frame = imath_random(102, 104);
				++local->couple_she_talk_count;
				if (local->couple_she_talk_count > 15) {
					if (local->wants_to_get_up) {
						local->couple_action = CONV7_COUPLE_TOUCH_HEAD;
						couple_reset_frame   = 1;
					} else {
						local->couple_action = CONV7_COUPLE_STAY_DOWN;
						couple_reset_frame   = 0;
					}
				}
			}
			break;

		case 9:  /* end of waking up               */
		case 10: /* end of he talking to her 1     */
		case 11: /* end of he talking to her 2     */
		case 12: /* end of he talking to her 3     */
		case 13: /* end of she talking to him 1    */
		case 14: /* end of she talking to him 2    */
		case 15: /* end of she talking to him 3    */
		case 25: /* end of turn to face each other */
		case 33: /* end of kiss                    */
		case 41: /* end of touch head              */
			switch (local->couple_frame) {
			case 9:
				local->couple_action = CONV7_COUPLE_TOUCH_HEAD;
				break;

			case 33: /* end of kiss */
				conv_release();
				if (player_verb == conv007_kiss_abc) {
					local->couple_action = CONV7_COUPLE_SHE_TALK;
				}
				break;

			case 41: /* end of touch head */
				conv_release();
				local->sitting_up = true;
				if (local->wants_to_talk) {
					local->couple_action = CONV7_COUPLE_HE_TALK;
				} else {
					local->couple_action = CONV7_COUPLE_SHUT_UP;
				}
				break;
			}

			switch (local->couple_action) {
			case CONV7_COUPLE_SHE_TALK:
				if (local->get_ready_she_leave) {
					random = 10;
					/* get up and walk away */
				} else {
					random = imath_random(1, 3);
					++local->couple_she_talk_count;
					if (local->couple_she_talk_count > 15) {
						local->couple_action = CONV7_COUPLE_SHUT_UP;
						random               = 12;
					}
				}
				break;

			case CONV7_COUPLE_HE_TALK:
				random = imath_random(4, 6);
				++local->couple_he_talk_count;
				if (local->couple_he_talk_count > 15) {
					local->couple_action = CONV7_COUPLE_SHUT_UP;
					random               = 12;
				}
				break;

			case CONV7_COUPLE_TOUCH_HEAD:
				local->couple_action = CONV7_COUPLE_SHUT_UP;
				random = 7;
				break;

			case CONV7_COUPLE_KISS:
				local->couple_action = CONV7_COUPLE_SHUT_UP;
				random = 8;
				break;

			case CONV7_COUPLE_RAOUL_ALONE:
				random = 9;
				break;

			case CONV7_COUPLE_SHE_WALK:
				random = 10;
				break;

			case CONV7_COUPLE_SHUT_UP_R:
			case CONV7_COUPLE_HE_TALK_R:
			case CONV7_COUPLE_SHE_TALK_R:
				random = 11;
				break;

			case CONV7_COUPLE_SHUT_UP:
				random = 12;
				break;
			}

			switch (random) {
			case 1:  couple_reset_frame = 12;  break; /* do she talk 1 to Raoul */
			case 2:  couple_reset_frame = 13;  break; /* do she talk 2 to Raoul */
			case 3:  couple_reset_frame = 14;  break; /* do she talk 3 to Raoul */
			case 4:  couple_reset_frame = 9;   break; /* do Raoul talk 1 to her */
			case 5:  couple_reset_frame = 10;  break; /* do Raoul talk 2 to her */
			case 6:  couple_reset_frame = 11;  break; /* do Raoul talk 3 to her */
			case 7:  couple_reset_frame = 33;  break; /* touch head */
			case 8:  couple_reset_frame = 25;  break; /* she kiss him */
			case 9:  couple_reset_frame = 54;  break; /* he get up */
			case 10: couple_reset_frame = 41;  break; /* she get up and walk */
			case 11: couple_reset_frame = 15;  break; /* they turn to Richard */
			case 12: couple_reset_frame = 14;  break; /* freeze looking at each other */
			}
			break;

		case 17: /* end of she & Raoul turn to Richard       */
		case 18: /* end of Raoul talk to Richard 1           */
		case 19: /* end of Raoul talk to Richard 2           */
		case 20: /* end of Raoul talk to Richard 3           */
		case 21: /* end of she talk to Richard 1             */
		case 22: /* end of she talk to Richard 2             */
		case 23: /* end of she talk to Richard 3             */
			switch (local->couple_action) {
			case CONV7_COUPLE_LOOK_AT_R_SHORT:
				random = 1; /* make the couple look at Richard (short period of time) */
				++local->couple_look_rich_count;
				if (local->couple_look_rich_count > 6) {
					local->couple_action = CONV7_COUPLE_SHUT_UP;
					random               = 7;
				}
				break;

			case CONV7_COUPLE_LOOK_AT_R_LONG:
				random = 1; /* make the couple look at Richard (long period of time) */
				break;

			case CONV7_COUPLE_SHE_TALK_R:
				random = imath_random(1, 3);
				++local->couple_she_talk_count;
				if (local->couple_she_talk_count > 15) {
					local->couple_action = CONV7_COUPLE_SHUT_UP;
					random               = 7;
				}
				break;

			case CONV7_COUPLE_HE_TALK_R:
				random = imath_random(4, 6);
				++local->couple_he_talk_count;
				if (local->couple_he_talk_count > 15) {
					local->couple_action = CONV7_COUPLE_SHUT_UP_R;
					random               = 8;
				}
				break;

			case CONV7_COUPLE_SHUT_UP:
			case CONV7_COUPLE_HE_TALK:
			case CONV7_COUPLE_SHE_TALK:
			case CONV7_COUPLE_KISS:
			case CONV7_COUPLE_HE_GET_UP:
			case CONV7_COUPLE_SHE_WALK:
			case CONV7_COUPLE_TOUCH_HEAD:
				random = 7;
				break;

			case CONV7_COUPLE_SHUT_UP_R:
				random = 8;
				break;
			}

			switch (random) {
			case 1: couple_reset_frame = 20;  break; /* do she talk 1 to Rich */
			case 2: couple_reset_frame = 21;  break; /* do she talk 2 to Rich */
			case 3: couple_reset_frame = 22;  break; /* do she talk 3 to Rich */
			case 4: couple_reset_frame = 17;  break; /* do Raoul talk 1 to Rich */
			case 5: couple_reset_frame = 18;  break; /* do Raoul talk 2 to Rich */
			case 6: couple_reset_frame = 19;  break; /* do Raoul talk 3 to Rich */
			case 7: couple_reset_frame = 23;  break; /* they look at each other */
			case 8: couple_reset_frame = 20;  break; /* they freeze looking at Richard */
			}
			break;

		case 52:  /* end of she getting up - start anim d1 */
			local->daae_walk_action = CONV7_DAAE_WALK_LEAVE;
			couple_reset_frame = 54;
			break;

		case 55:  /* end of Raoul sitting up alone */
			if (local->couple_action != CONV7_COUPLE_RAOUL_ALONE) {
				couple_reset_frame = 54;
			}
			break;

		case 89:  /* almost end of getting to start pinching */
			conv_release();
			break;

		case 90:  /* end of getting to start pinching */
			if (local->couple_action != CONV7_COUPLE_PINCH) {
				couple_reset_frame = 89;
			} else {
				couple_reset_frame = 90;
				global[temp_var] = 200;
				/* This is to help me debug this pinching thing.  If the player
				   does pinch himself, then this variable will be set. */
			}
			break;

		case 102: /* end of getting on feet */
			conv_release();
			player.x              = AFTER_CONV_X;
			player.y              = AFTER_CONV_Y;
			player_demand_facing(FACING_SOUTH);
			couple_reset_frame    = 105;
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
			break;

		case 106: /* end of invisible */
			local->couple_action = CONV7_COUPLE_INVISIBLE;
			couple_reset_frame = 105;
			break;
		}

		if (couple_reset_frame >= 0) {
			kernel_reset_animation(aa[1], couple_reset_frame);
			local->couple_frame = couple_reset_frame;
		}
	}
}

static void handle_player_walk() {
	if (player.sprite != local->last_player_step) {
		local->last_player_step = player.sprite;
		switch (player.facing) {
		case FACING_NORTH:
		case FACING_SOUTH:
			if ((player.sprite == 5) || (player.sprite == 11))
				sound_play(N_EchoSteps);
			break;

		case FACING_NORTHEAST:
		case FACING_NORTHWEST:
		case FACING_SOUTHEAST:
		case FACING_SOUTHWEST:
			if ((player.sprite == 7) || (player.sprite == 14))
				sound_play(N_EchoSteps);
			break;

		case FACING_EAST:
		case FACING_WEST:
			if ((player.sprite == 8) || (player.sprite == 16))
				sound_play(N_EchoSteps);
			break;
		}
	}
}

void room_104_init() {
	kernel.disable_fastwalk = true;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running      = false;
		local->anim_1_running      = false;
		local->anim_2_running      = false;
		local->wants_to_talk       = false;
		local->wants_to_get_up     = false;
		local->sitting_up          = false;
		local->get_ready_she_leave = false;
		local->wants_to_stand_up   = false;
	}

	/* ==================== Load Sprite Series ====================== */

	ss[fx_climb_thru] = kernel_load_series(kernel_name('a', 0), false);
	ss[fx_fall]       = kernel_load_series(kernel_name('a', 6), false);
	ss[fx_trap_door]  = kernel_load_series(kernel_name('x', 0), false);

	if (global[current_year] == 1993) {
		ss[fx_chandelier] = kernel_load_series(kernel_name('z', 0), false);
	}


	/* ==================== Load conversation ======================= */

	conv_get(CONV_AFTER_FALL_FROM_301);


	/* ==================== Do trap door stuff ====================== */

	if (global[trap_door_status] == TRAP_DOOR_IS_CLOSED) {
		seq[fx_trap_door] = kernel_seq_stamp(ss[fx_trap_door], false, 1);
		kernel_seq_depth(seq[fx_trap_door], 15);
	} else {
		seq[fx_trap_door] = kernel_seq_stamp(ss[fx_trap_door], false, 2);
		kernel_seq_depth(seq[fx_trap_door], 15);
	}


	/* ==================== Previous Rooms ========================== */

	if (previous_room == KERNEL_RESTORING_GAME) {

		if (conv_restore_running == CONV_AFTER_FALL_FROM_301) {

			aa[1]                   = kernel_run_animation(kernel_name('r', 1), 1);
			/* anim of couple on floor of stage (Raoul & Daae) */

			aa[2]                   = kernel_run_animation(kernel_name('d', 1), 1);
			local->daae_walk_action = CONV7_DAAE_WALK_INVISIBLE;
			/* anim of Daae walking off stage */

			aa[0]                   = kernel_run_animation(kernel_name('m', 1), 1);
			/* anim of Monsieur Richard */

			if (local->couple_action < CONV7_COUPLE_STAY_DOWN) {
				local->couple_action = CONV7_COUPLE_SHUT_UP;
				local->rich_action   = CONV7_RICH_SHUT_UP;
				kernel_reset_animation(aa[1], 14);
				/* make the couple look at each other on the floor */
			} else {
				local->couple_action = CONV7_COUPLE_INVISIBLE;
				local->rich_action   = CONV7_RICH_WALK;
				kernel_reset_animation(aa[1], 105);
				/* make the couple invisible */
				kernel_reset_animation(aa[0], 216);
				/* make Richard invisible */
			}

			conv_run(CONV_AFTER_FALL_FROM_301);
			conv_export_pointer(&global[player_score]);
			player.walker_visible = false;
		}

	} else if (previous_room == 301) {

		aa[1]                 = kernel_run_animation(kernel_name('r', 1), 1);
		local->anim_1_running = true;
		local->couple_action  = CONV7_COUPLE_STAY_DOWN;
		/* anim of couple on floor of stage (Raoul & Daae) */

		aa[2]                   = kernel_run_animation(kernel_name('d', 1), 1);
		local->anim_2_running   = true;
		local->daae_walk_action = CONV7_DAAE_WALK_INVISIBLE;
		/* anim of Daae walking off stage */

		aa[0]                 = kernel_run_animation(kernel_name('m', 1), 1);
		local->anim_0_running = true;
		local->rich_action    = CONV7_RICH_SHUT_UP;
		/* anim of Monsieur Richard */

		get_rid_of_inventory();

		player.walker_visible          = false;
		player.num_rooms_been_in       = 0;
		global[trap_door_status]       = TRAP_DOOR_IS_CLOSED;
		global[current_year]           = 1881;
		global[prompter_stand_status]  = PROMPT_RIGHT;
		global[ticket_people_here]     = SELLER;
		global[make_brie_leave_203]    = false;
		/* we are in 1881 now, so the stand should be on right */
		player.x = 161; /* picked this number from my nose. don't want the camera to pan */

		player_discover_room(301); /* so the shadow does not appear again */
		player_discover_room(101); /* so the opening fugue does not repeat */

		camera_jump_to(LEFT_STAGE + 60, 0);

		kernel_timing_trigger(1, ROOM_104_INIT_TEXT);

	} else if (previous_room == 103) {
		if (global[room_103_104_transition] == PEEK_THROUGH) {
			kernel_init_dialog();  /* clear interface */
			kernel_set_interface_mode(INTER_CONVERSATION);

			if (!global[observed_phan_104]) {
				aa[4] = kernel_run_animation(kernel_name('p', 1), ROOM_104_END_PHANTOM);
				player.x                = 319; /* pulled this number from my ass (so camera won't scroll) */
				player.commands_allowed = false;
				player.walker_visible   = false;
				camera_jump_to(MIDDLE_STAGE, 0);
				/* player will peek through box, and see Phantom */

			} else {
				aa[5] = kernel_run_animation(kernel_name('p', 2), ROOM_104_END_PHANTOM + 1);
				player.x                = 319; /* pulled this number from my ass (so camera won't scroll) */
				player.commands_allowed = false;
				player.walker_visible   = false;
				camera_jump_to(MIDDLE_STAGE, 0);
				/* player will peek through box, and see nothing */
			}

		} else {
			player.x = NEW_ROOM_FROM_103_X;
			player.y = NEW_ROOM_FROM_103_Y;
			player.facing = FACING_SOUTH;
			camera_jump_to(MIDDLE_STAGE, 0);
		}

	} else if (previous_room == 102) { /* return from death */
		switch (global[death_location]) {
		case FAR_PIT:
			player.x = RESTORE_X_FAR;
			player.y = RESTORE_Y_FAR;
			camera_jump_to(RIGHT_STAGE, 0);
			break;

		case MIDDLE_PIT:
			player.x = RESTORE_X_MIDDLE;
			player.y = RESTORE_Y_MIDDLE;
			camera_jump_to(MIDDLE_STAGE, 0);
			break;

		case NEAR_PIT:
			player.x = RESTORE_X_NEAR;
			player.y = RESTORE_Y_NEAR;
			break;
		}

	} else if (previous_room == 108) {
		if (player.x > 213) {
			player.y = PLAYER_Y_FROM_108_3;
		} else if (player.x > 110) {
			player.y = PLAYER_Y_FROM_108_2;
		} else {
			player.y = PLAYER_Y_FROM_108_1;
		}

		player_first_walk(OFF_SCREEN_X_FROM_108, player.y, FACING_EAST,
		                  PLAYER_X_FROM_108, player.y, FACING_EAST, true);

	} else if ((previous_room == 107) || (previous_room != KERNEL_RESTORING_GAME)) {
		if (player.x > 191) {
			player.y = PLAYER_Y_FROM_107_3;
		} else if (player.x > 104) {
			player.y = PLAYER_Y_FROM_107_2;
		} else {
			player.y = PLAYER_Y_FROM_107_1;
		}

		player_first_walk(OFF_SCREEN_X_FROM_107, player.y, FACING_WEST,
		                  PLAYER_X_FROM_107, player.y, FACING_WEST, true);

		camera_jump_to(RIGHT_STAGE, 0);
	}


	/* ==================== If in 1993, put chandelier here ========= */

	if (global[current_year] == 1993) {
		seq[fx_chandelier] = kernel_seq_stamp(ss[fx_chandelier], false, 1);
		kernel_seq_depth(seq[fx_chandelier], 14);
	} else {
		kernel_flip_hotspot(words_chandelier, false);
	}


	/* ==================== Do trap door stuff ====================== */

	if (global[trap_door_status] == TRAP_DOOR_IS_CLOSED) {
		seq[fx_trap_door] = kernel_seq_stamp(ss[fx_trap_door], false, 1);
		kernel_seq_depth(seq[fx_trap_door], 15);
	} else {
		seq[fx_trap_door] = kernel_seq_stamp(ss[fx_trap_door], false, 2);
		kernel_seq_depth(seq[fx_trap_door], 15);
	}

	section_1_music();
}

void room_104_daemon() {
	if (local->anim_0_running) {
		handle_animation_rich();
	}

	if (local->anim_1_running) {
		handle_animation_couple();
	}

	if (local->anim_2_running) {
		handle_animation_daae_walk();
	}

	if (player.walking) {
		handle_player_walk();
	}

	if (kernel.trigger == ROOM_104_INIT_TEXT) {
		text_show(text_104_34);
		conv_run(CONV_AFTER_FALL_FROM_301);
		conv_export_pointer(&global[player_score]);
	}

	if (kernel.trigger == ROOM_104_END_PHANTOM) {
		new_room = 103;
		player.x = 400; /* in 103, because Player_x == 400, will stop phantom music
		                   and start normal background music */
		global[room_103_104_transition] = PEEK_THROUGH;
	}

	if (kernel.trigger == ROOM_104_END_PHANTOM + 1) {
		new_room = 103;
		global[room_103_104_transition] = PEEK_THROUGH;
	}
}

static void process_conversation_fall() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	switch (player_verb) {
	case conv007_where_pushed:
		conv_you_trigger(ROOM_104_POINT);
		you_trig_flag = true;
		break;

	case conv007_dashing_london:
		conv_you_trigger(ROOM_104_ARMS_OUT);
		you_trig_flag = true;
		break;

	case conv007_office_abc:
		conv_you_trigger(ROOM_104_GET_TO_SHE_WALK);
		you_trig_flag = true;
		break;

	case conv007_solo_alone:
		if (!kernel.trigger) {
			conv_hold();
			local->rich_action = CONV7_RICH_WALK;
		}
		break;

	case conv007_pinch_wait_b_nothing_b:
		local->couple_action = CONV7_COUPLE_PINCH;
		me_trig_flag         = true;
		you_trig_flag        = true;
		conv_hold();
		break;

	case conv007_dashing_tuxedo:
		conv_you_trigger(ROOM_104_RICHARD_TALK);
		conv_me_trigger(ROOM_104_HE_TALK_TO_RICH);
		you_trig_flag = true;
		me_trig_flag  = true;
		break;

	case conv007_badfall_abc:
		conv_you_trigger(ROOM_104_SHE_TALK_TO_RICH);
		you_trig_flag = true;
		break;

	case conv007_daaeb_intro_c:
		if (!local->wants_to_get_up) {
			conv_you_trigger(ROOM_104_SIT_UP);
			you_trig_flag          = true;
			local->wants_to_get_up = true;
		}
		break;

	case conv007_afterkiss_abc:
	case conv007_final_goaway:
		if (!kernel.trigger) {
			local->rich_action   = CONV7_RICH_SHUT_UP;
			local->couple_action = CONV7_COUPLE_KISS;
			conv_hold();
		}
		break;

	case conv007_richard_intro_b:
	case conv007_where_killed:
	case conv007_youraoul_abc:
	case conv007_delirious_abc:
	case conv007_long_abc:
	case conv007_worry_abc:
	case conv007_answers_abc:
		conv_you_trigger(ROOM_104_RICHARD_TALK);
		you_trig_flag = true;
		break;
	}

	switch (kernel.trigger) {
	case ROOM_104_COMMANDS_ALLOWED_NOT:
		conv_release();
		if (local->couple_action != CONV7_COUPLE_INVISIBLE) {
			player.commands_allowed = false;
		}
		break;

	case ROOM_104_POINT:
		local->rich_action = CONV7_RICH_POINT;
		break;

	case ROOM_104_ARMS_OUT:
		local->rich_action = CONV7_RICH_ARMS_OUT;
		break;

	case ROOM_104_GET_TO_SHE_WALK:
		local->rich_action        = CONV7_RICH_TALK;
		local->get_ready_he_leave = true;
		local->couple_action      = CONV7_COUPLE_SHE_WALK;
		break;

	case ROOM_104_SIT_UP:
		conv_hold();
		local->couple_action = CONV7_COUPLE_SHE_TALK_LAY;
		break;

	case ROOM_104_SHE_TALK_TO_RICH:
		local->rich_action   = CONV7_RICH_SHUT_UP;
		local->couple_action = CONV7_COUPLE_SHE_TALK_R;
		break;

	case ROOM_104_HE_TALK_TO_RICH:
		local->rich_action   = CONV7_RICH_SHUT_UP;
		local->couple_action = CONV7_COUPLE_HE_TALK_R;
		break;

	case ROOM_104_RICHARD_TALK:
		local->rich_action = CONV7_RICH_TALK;

		if (local->sitting_up) {
			if (player_verb == conv007_long_abc) {
				local->couple_look_rich_count = 0;
				local->couple_action = CONV7_COUPLE_LOOK_AT_R_SHORT;

			} else if ((player_verb == conv007_worry_abc) ||
			           (player_verb == conv007_dashing_tuxedo)) {
				local->couple_look_rich_count = 0;
				local->couple_action = CONV7_COUPLE_LOOK_AT_R_LONG;

			} else {
				local->couple_action = CONV7_COUPLE_SHUT_UP;
			}
		} else {
			local->couple_action = CONV7_COUPLE_STAY_DOWN;
		}
		break;

	case ROOM_104_SHE_TALK:
		if ((!local->get_ready_she_leave) && (!local->get_ready_he_leave) &&
		    (local->couple_action != CONV7_COUPLE_PINCH) && (!local->wants_to_stand_up)) {
			local->rich_action = CONV7_RICH_SHUT_UP;
			if (local->sitting_up) {
				local->couple_action = CONV7_COUPLE_SHE_TALK;
			} else {
				local->couple_action = CONV7_COUPLE_SHE_TALK_LAY;
			}
		}
		break;

	case ROOM_104_ME_TALK:
		if ((!local->get_ready_she_leave) && (!local->get_ready_he_leave) &&
		    (local->couple_action != CONV7_COUPLE_PINCH) && (!local->wants_to_stand_up)) {
			local->rich_action = CONV7_RICH_SHUT_UP;
			if ((local->sitting_up) && (!local->get_ready_she_leave)) {
				local->couple_action = CONV7_COUPLE_HE_TALK;
				local->rich_action   = CONV7_RICH_SHUT_UP;
			}
		}
		break;
	}


	/* ==================== Set up me and you triggers ============== */

	if (!me_trig_flag) {
		if (!local->get_ready_she_leave) {
			conv_me_trigger(ROOM_104_ME_TALK);
		}
	} /* if me_trig_flag == true, then a me trigger is called from above, not here. */

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_104_SHE_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */

	local->rich_talk_count       = 0;
	local->couple_he_talk_count  = 0;
	local->couple_she_talk_count = 0;
}

void room_104_pre_parser() {
	if (player_said_2(exit, stage_left)) {
		player.walk_off_edge_to_room = 108;
	}

	if (player_said_2(exit, stage_right)) {
		player.walk_off_edge_to_room = 107;
	}

	if (player_said_2(open, trap_door) || player_said_2(close, trap_door)) {
		player_walk(320, 92, FACING_NORTH);
	}
}

void room_104_parser() {
	if (conv_control.running == CONV_AFTER_FALL_FROM_301) {
		process_conversation_fall();
		goto handled;
	}

	if (player_said_2(climb_through, trap_door)) {
		if (global[trap_door_status] == TRAP_DOOR_IS_OPEN) {
			switch (kernel.trigger) {
			case 0:
				player.walker_visible = false;
				seq[fx_climb_thru] = kernel_seq_forward(ss[fx_climb_thru], false,
				                                        8, 0, 0, 1);
				kernel_seq_depth(seq[fx_climb_thru], 13);
				kernel_seq_range(seq[fx_climb_thru], 1, 16);
				kernel_seq_trigger(seq[fx_climb_thru],
				                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_104_CLIMB_THRU);
				break;

			case ROOM_104_CLIMB_THRU:
				new_room = 103;
				global[room_103_104_transition] = NEW_ROOM;
				player.commands_allowed = true;
				break;
			}
		} else {
			text_show(text_104_29);
			/* go through trap door when it's closed */
		}
		goto handled;
	}

	if (player_said_2(jump_into, orchestra_pit)) {
		switch (kernel.trigger) {
		case 0:
			if (player.x > 400) {
				global[death_location] = FAR_PIT;
			} else if (player.x > 200) {
				global[death_location] = MIDDLE_PIT;
			} else {
				global[death_location] = NEAR_PIT;
			}

			kernel_load_variant(2);

			if (global[trap_door_status] == TRAP_DOOR_IS_CLOSED) {
				kernel_draw_to_background(ss[fx_trap_door], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
			} else {
				kernel_draw_to_background(ss[fx_trap_door], 2, KERNEL_HOME, KERNEL_HOME, 0, 100);
			}

			text_show(text_104_26);
			player.walker_visible   = false;
			player.commands_allowed = false;
			seq[fx_fall] = kernel_seq_forward(ss[fx_fall], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_fall], 15);
			kernel_seq_range(seq[fx_fall], KERNEL_FIRST, 4);
			kernel_seq_loc(seq[fx_fall], player.x, player.y);
			kernel_seq_player(seq[fx_fall], true);
			kernel_seq_trigger(seq[fx_fall],
			                   KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			seq[fx_fall] = kernel_seq_pingpong(ss[fx_fall], false, 1, 0, 0, 10);
			kernel_timing_trigger(ONE_SECOND, 2);
			kernel_seq_range(seq[fx_fall], 4, 4);
			kernel_seq_depth(seq[fx_fall], 15);
			kernel_seq_player(seq[fx_fall], true);
			kernel_seq_motion(seq[fx_fall], 0, 0, 200);
			break;

		case 2:
			sound_play(N_AllFade);
			sound_play(N_PlayerFalls);
			new_room = 102;
			break;
		}
		goto handled;
	}

	if (player.look_around) {
		if (global[current_year] == 1993) {
			text_show(text_104_10);
		} else {
			text_show(text_104_11);
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(stage)) {
			text_show(text_104_12);
			goto handled;
		}

		if (player_said_1(apron)) {
			text_show(text_104_13);
			goto handled;
		}

		if (player_said_1(proscenium_arch)) {
			text_show(text_104_14);
			goto handled;
		}

		if (player_said_1(act_curtain)) {
			text_show(text_104_15);
			goto handled;
		}

		if (player_said_1(orchestra_pit)) {
			text_show(text_104_16);
			goto handled;
		}

		if (player_said_1(conductor_s_stand)) {
			text_show(text_104_17);
			goto handled;
		}

		if ((player_said_1(music_stand)) || (player_said_1(music_stands))) {
			text_show(text_104_18);
			goto handled;
		}

		if (player_said_1(prompter_s_box)) {
			text_show(text_104_19);
			goto handled;
		}

		if (player_said_1(trap_door)) {
			text_show(text_104_20);
			goto handled;
		}

		if (player_said_1(house)) {
			if (global[current_year] == 1881) {
				text_show(text_104_21);
			} else {
				text_show(text_104_27);
			}
			goto handled;
		}

		if (player_said_1(stage_left)) {
			text_show(text_104_22);
			goto handled;
		}

		if (player_said_1(stage_right)) {
			text_show(text_104_23);
			goto handled;
		}

		if (player_said_1(chandelier)) {
			text_show(text_104_28);
			goto handled;
		}

		if (player_said_1(Monsieur_Richard)) {
			text_show(text_104_33);
			goto handled;
		}
	}

	if (player_said_2(jump_into, orchestra_pit)) {
		text_show(text_104_26);
		new_room = 102;
		goto handled;
	}

	if (player_said_2(open, prompter_s_box) || player_said_2(close, prompter_s_box)) {
		text_show(text_104_30);
		goto handled;
	}

	if (player_said_2(open, trap_door)) {
		if (global[trap_door_status] == TRAP_DOOR_IS_OPEN) {
			text_show(text_104_24);
		} else {
			text_show(text_104_32);
		}
		goto handled;
	}

	if (player_said_2(close, trap_door)) {
		if (global[trap_door_status] == TRAP_DOOR_IS_CLOSED) {
			text_show(text_104_25);
		} else {
			text_show(text_104_33);
		}
		goto handled;
	}

	if (player_said_2(take, chandelier)) {
		text_show(text_104_35);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_104_preload() {
	room_init_code_pointer       = room_104_init;
	room_pre_parser_code_pointer = room_104_pre_parser;
	room_parser_code_pointer     = room_104_parser;
	room_daemon_code_pointer     = room_104_daemon;

	section_1_walker();
	section_1_interface();

	if (global[trap_door_status] == TRAP_DOOR_IS_CLOSED) {
		kernel_initial_variant = 1;
	}

	vocab_make_active(words_Monsieur_Richard);
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
