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

#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/room106.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

/**
 * Room local variables
 */
struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[5];     /* Animation handles     */

	int16 prevent;

	int16 mac_frame;       /* animation frame being held for Macmorn stuff */
	int16 mac_action;      /* Type of action to run for Macmorn animation */
	int16 mac_talk_count;  /* counter for Macmorn talking */
	int16 anim_0_running;

	int16 sit_frame;       /* animation frame being held for sitting stuff */
	int16 sit_action;      /* Type of action to run for sitting animation */
	int16 sit_count;       /* delay for looking in a certain direction */
	int16 anim_1_running;


	/* abduction anims */
	int16 a_mac_frame;       /* animation frame being held for Macmorn stuff */
	int16 a_mac_action;      /* Type of action to run for Macmorn animation */
	int16 a_mac_talk_count;  /* counter for Macmorn talking */
	int16 anim_2_running;

	int16 a_q_frame;       /* animation frame being held for queen stuff */
	int16 a_q_action;      /* Type of action to run for queen animation */
	int16 a_q_talk_count;  /* counter for queen talking */
	int16 anim_3_running;

	int16 a_qm_frame;       /* animation frame being held for QMother stuff */
	int16 a_qm_action;      /* Type of action to run for QMother animation */
	int16 a_qm_talk_count;  /* counter for QMother talking */
	int16 anim_4_running;

};

static Scratch scratch;


/* ========================= Sprites ========================= */

#define fx_fire_sconce       1   /* rm106y */
#define fx_door_104          2   /* rm103x0 */
#define fx_door_105          3   /* rm103x1 */
#define fx_open_door         4   /* kgrd_8  */  


/* ======================== Triggers ========================= */

#define ROOM_106_DOOR_CLOSES  70 
#define ROOM_106_DONE_ANIM    80
#define ROOM_106_POINT        85
#define ROOM_106_YOU_TALK     90
#define ROOM_106_DONE_SITTING 98
#define ROOM_106_RUN_CONV     110
#define ROOM_106_ME_TALK      115
#define ROOM_106_NEW_ROOM     120
#define ROOM_106_END_GAME_1   140
#define ROOM_106_END_GAME_2   142
#define ROOM_106_END_GAME_3   144


/* walk points */
#define START_X_ROOM_104     87
#define START_Y_ROOM_104     95
#define START_X_ROOM_105     230
#define START_Y_ROOM_105     95
#define START_X_ROOM_107     178
#define START_Y_ROOM_107     152
#define START_X_ROOM_118     325
#define START_Y_ROOM_118     135
#define START_X_ROOM_111     149
#define START_Y_ROOM_111     129

#define WALK_TO_X_FROM_104   79
#define WALK_TO_Y_FROM_104   106
#define WALK_TO_X_FROM_105   238
#define WALK_TO_Y_FROM_105   106

/* cursor points */
#define LEFT_TAPESTRY_X      39

#define MAC_CONV_X           144
#define MAC_CONV_Y           142

#define CONV_3_MACMORN       3

#define MAC_SHUT_UP          0
#define MAC_TALK             1
#define MAC_LEAVE            2
#define MAC_POINT            3
#define MAC_INVISIBLE        4

#define QUEEN_SHUT_UP          0
#define QUEEN_TALK             1
#define QUEEN_FIST             2
#define QUEEN_GET_UP           3
#define QUEEN_LEAVE            4

/* actions when King sits down in throne */
#define LOOK                 0
#define LEAVE                1

#define SIT_X                112
#define SIT_Y                130

#define CONV_34_CUT_SCENE    34


void room_106_init() {
	int count;

	if (global[end_of_game]) {

		local->prevent = true;
		local->anim_0_running = false;
		local->anim_2_running = false;
		local->anim_3_running = false;
		local->anim_4_running = false;
		local->anim_1_running = false;

		viewing_at_y = ((video_y - display_y) >> 1);
		kernel_init_dialog();  /* clear interface */
		kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

		player.commands_allowed = false;
		player.walker_visible = false;
		ss[fx_fire_sconce] = kernel_load_series(kernel_name('y', -1), false);
		ss[fx_door_104] = kernel_load_series(kernel_name('x', 0), false);
		ss[fx_door_105] = kernel_load_series(kernel_name('x', 1), false);

		seq[fx_fire_sconce] = kernel_seq_forward(ss[fx_fire_sconce], false, 7, 0, 0, 0);

		seq[fx_door_104] = kernel_seq_stamp(ss[fx_door_104], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door_104], 8);
		seq[fx_door_105] = kernel_seq_stamp(ss[fx_door_105], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door_105], 8);

		if (global[llanie_status] == IS_SAVED) {
			if (player_has(rare_coin) && player_has(crystal_flower) && player_has(ruby_ring) &&
				player_has(gold_nugget) && player_has(magic_music_box) && player_has(emerald)) {
				aa[0] = kernel_run_animation(kernel_name('e', 3), ROOM_106_END_GAME_1);

			} else {
				aa[0] = kernel_run_animation(kernel_name('e', 2), ROOM_106_END_GAME_1);
			}

		} else {
			aa[0] = kernel_run_animation(kernel_name('e', 1), ROOM_106_END_GAME_1);
		}

	} else {

		conv_get(CONV_3_MACMORN);

		/* Load sprite series */

		ss[fx_fire_sconce] = kernel_load_series(kernel_name('y', -1), false);
		ss[fx_door_104] = kernel_load_series(kernel_name('x', 0), false);
		ss[fx_door_105] = kernel_load_series(kernel_name('x', 1), false);

		if (previous_room != 614) {
			ss[fx_open_door] = kernel_load_series("*KGRD_8", false);
		}

		if (previous_room != KERNEL_RESTORING_GAME) {
			local->prevent = false;
			local->anim_0_running = false;
			local->anim_2_running = false;
			local->anim_3_running = false;
			local->anim_4_running = false;
			if (previous_room != 111) {
				local->anim_1_running = false;
			}
		}

		local->mac_talk_count = 0;
		local->a_q_talk_count = 0;
		local->a_qm_talk_count = 0;
		local->a_mac_talk_count = 0;

		/* Start continuous sequences */

		seq[fx_fire_sconce] = kernel_seq_forward(ss[fx_fire_sconce], false, 7, 0, 0, 0);

		if (previous_room == 107 || previous_room == 118 ||
			previous_room == 111) {
			seq[fx_door_104] = kernel_seq_stamp(ss[fx_door_104], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door_104], 8);
			seq[fx_door_105] = kernel_seq_stamp(ss[fx_door_105], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door_105], 8);

		} else if (previous_room == 614) {
			seq[fx_door_104] = kernel_seq_stamp(ss[fx_door_104], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door_104], 8);
		}

		if (previous_room == 105) {      /* Player comes from Ballroom  */
			seq[fx_door_104] = kernel_seq_stamp(ss[fx_door_104], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door_104], 8);
			seq[fx_door_105] = kernel_seq_stamp(ss[fx_door_105], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_door_105], 8);

			player.x = START_X_ROOM_105;
			player.y = START_Y_ROOM_105;
			player.facing = FACING_SOUTH;

			if (player.been_here_before) {
				player_walk(WALK_TO_X_FROM_105, WALK_TO_Y_FROM_105, FACING_SOUTH);
				player_walk_trigger(ROOM_106_DOOR_CLOSES + 2);
			} else {
				aa[0] = kernel_run_animation(kernel_name('m', 1), ROOM_106_DONE_ANIM);
				local->anim_0_running = true;
				local->mac_action = MAC_TALK;
				player.commands_allowed = false;
				player_walk(MAC_CONV_X, MAC_CONV_Y, FACING_NORTHWEST);
			}

		} else if (previous_room == 118) {      /* Player comes from Courtyard */

			local->prevent = true;

			if (player.been_here_before) {
				player_first_walk(START_X_ROOM_118, START_Y_ROOM_118, FACING_WEST,
					START_X_ROOM_118 - 30, START_Y_ROOM_118, FACING_WEST, true);
			} else {
				aa[0] = kernel_run_animation(kernel_name('m', 1), ROOM_106_DONE_ANIM);
				local->anim_0_running = true;
				local->mac_action = MAC_TALK;
				player.commands_allowed = false;
				player_first_walk(START_X_ROOM_118, START_Y_ROOM_118, FACING_WEST,
					MAC_CONV_X, MAC_CONV_Y, FACING_NORTHWEST, true);
			}

		} else if (previous_room == 107) {      /* Player comes from Council Chamber */
			player.x = START_X_ROOM_107;
			player.y = START_Y_ROOM_107;
			player.facing = FACING_NORTHWEST;

		} else if (previous_room == 614) {      /* Cut scene */

			conv_get(CONV_34_CUT_SCENE);

			kernel_init_dialog();  /* clear interface */
			kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

			player.commands_allowed = false;
			player.walker_visible = false;
			local->prevent = true;

			aa[2] = kernel_run_animation(kernel_name('m', 2), 0);
			local->anim_2_running = true;
			local->a_mac_action = MAC_INVISIBLE;
			kernel_reset_animation(aa[2], 116);

			aa[4] = kernel_run_animation(kernel_name('q', 2), 0);
			local->anim_4_running = true;
			local->a_qm_action = QUEEN_SHUT_UP;
			kernel_reset_animation(aa[4], 8);

			aa[3] = kernel_run_animation(kernel_name('q', 1), 0);
			local->anim_3_running = true;
			local->a_q_action = QUEEN_SHUT_UP;

			kernel_timing_trigger(ONE_SECOND, ROOM_106_RUN_CONV);

		} else if (previous_room == 111) {
			/* Came from looking at Dragonsphere */
			if (local->anim_1_running) {

				for (count = 0; count < room_num_spots; count++) {
					if (room_spots[count].vocab == words_king_s_throne) {
						room_spots[count].verb = words_look_at;
						room_spots[count].cursor_number = 1;
					}
				}

				aa[1] = kernel_run_animation(kernel_name('s', 1), 0);
				player.walker_visible = false;
				local->sit_action = LOOK;
				local->sit_count = 0;
				player.x = SIT_X;
				player.y = SIT_Y;
				kernel_reset_animation(aa[1], 33);

			} else {
				player.x = START_X_ROOM_111;
				player.y = START_Y_ROOM_111;
				player.facing = FACING_NORTHEAST;
			}

		} else if (previous_room != KERNEL_RESTORING_GAME) { /* Player comes from Meeting Chamber rm104 */
			seq[fx_door_105] = kernel_seq_stamp(ss[fx_door_105], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door_105], 8);
			player_walk(WALK_TO_X_FROM_104, WALK_TO_Y_FROM_104, FACING_SOUTH);
			seq[fx_door_104] = kernel_seq_stamp(ss[fx_door_104], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_door_104], 8);

			player.x = START_X_ROOM_104;
			player.y = START_Y_ROOM_104;
			player.facing = FACING_SOUTH;

			if (player.been_here_before) {
				player_walk(WALK_TO_X_FROM_104, WALK_TO_Y_FROM_104, FACING_SOUTH);
				player_walk_trigger(ROOM_106_DOOR_CLOSES);
			} else {
				aa[0] = kernel_run_animation(kernel_name('m', 1), ROOM_106_DONE_ANIM);
				local->anim_0_running = true;
				local->mac_action = MAC_TALK;
				player.commands_allowed = false;
				kernel_reset_animation(aa[0], 151);
				player_walk(MAC_CONV_X, MAC_CONV_Y, FACING_NORTHWEST);
			}

		} else if (previous_room == KERNEL_RESTORING_GAME) {

			if (local->anim_1_running) {

				for (count = 0; count < room_num_spots; count++) {
					if (room_spots[count].vocab == words_king_s_throne) {
						room_spots[count].verb = words_look_at;
						room_spots[count].cursor_number = 1;
					}
				}

				aa[1] = kernel_run_animation(kernel_name('s', 1), 0);
				player.walker_visible = false;
				local->sit_action = LOOK;
				local->sit_count = 0;
				kernel_reset_animation(aa[1], 33);
			}

			if (conv_restore_running == CONV_3_MACMORN) {
				aa[0] = kernel_run_animation(kernel_name('m', 1), ROOM_106_DONE_ANIM);
				local->anim_0_running = true;
				local->mac_action = MAC_SHUT_UP;
				player.commands_allowed = false;
				if (local->mac_frame < 20) {
					kernel_reset_animation(aa[0], 11);
				} else {
					kernel_reset_animation(aa[0], 46);
				}
				conv_run(CONV_3_MACMORN);
			}

			seq[fx_door_104] = kernel_seq_stamp
			(ss[fx_door_104], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door_104], 8);
			seq[fx_door_105] = kernel_seq_stamp
			(ss[fx_door_105], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door_105], 8);
		}
	}

	section_1_music();
}


static void handle_animation_mac() {
	int mac_reset_frame;

	if (kernel_anim[aa[0]].frame != local->mac_frame) {
		local->mac_frame = kernel_anim[aa[0]].frame;
		mac_reset_frame = -1;

		switch (local->mac_frame) {

		case 1:
		case 151:
			if (player.y < 110) {
				if (local->mac_frame == 1) {
					mac_reset_frame = 0;
				} else {
					mac_reset_frame = 150;
				}
			}
			break;

		case 8:    /* stand - king from right */
		case 157:  /* stand - king from left */
			player.commands_allowed = false;
			conv_run(CONV_3_MACMORN);
			break;

		case 9:
		case 158:
			conv_update(true);
			break;  /* I want conv to start while player is walking into room.  Parser
					   code won't be called.  But that's ok for this anim. */

		case 11:   /* end of standing and talk */
		case 12:   /* end of talk */
		case 13:   /* end of talk */
		case 159:  /* end of standing */

			if (local->mac_action == MAC_TALK) {

				mac_reset_frame = imath_random(10, 12);
				++local->mac_talk_count;
				if (local->mac_talk_count > 15) {
					local->mac_action = MAC_SHUT_UP;
					local->mac_talk_count = 0;
					mac_reset_frame = 13; /* step off of platform (new node) */
				}

			} else {
				mac_reset_frame = 13; /* step off of platform (new node) */
			}
			break;

		case 46:   /* end of coming off of platform */
		case 76:   /* end of point                  */
		case 160:  /* end of talk                   */
		case 161:  /* end of talk                   */
		case 162:  /* end of talk                   */

			switch (local->mac_action) {

			case MAC_TALK:
				mac_reset_frame = imath_random(159, 161);
				++local->mac_talk_count;
				if (local->mac_talk_count > 15) {
					local->mac_action = MAC_SHUT_UP;
					local->mac_talk_count = 0;
					mac_reset_frame = 45; /* step off of platform (new node) */
				}
				break;

			case MAC_POINT:
				mac_reset_frame = 46;  /* talk */
				local->mac_action = MAC_SHUT_UP;
				break;

			case MAC_LEAVE:
				mac_reset_frame = 76;  /* leave */
				break;

			case MAC_SHUT_UP:
				mac_reset_frame = 45;  /* shut up */
				break;
			}
			break;

		case 51:
			mac_reset_frame = 62;
			/* skip over long delay of holding up arm (Ginny stuck all these frames in) */
			break;

		case 148:
			kernel_abort_animation(aa[0]);
			player.commands_allowed = true;
			local->anim_0_running = false;
			mac_reset_frame = -1;
			break;

		case 115:
			kernel_seq_delete(seq[fx_door_104]);
			sound_play(N_DoorOpens);
			seq[fx_door_104] = kernel_seq_forward(ss[fx_door_104],
				false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door_104], 8);
			kernel_seq_trigger(seq[fx_door_104],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_106_DOOR_CLOSES + 4);
			break;

		case 136:
			kernel_seq_delete(seq[fx_door_104]);
			sound_play(N_DoorCloses);
			seq[fx_door_104] = kernel_seq_backward(ss[fx_door_104],
				false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door_104], 8);
			kernel_seq_trigger(seq[fx_door_104],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_106_DOOR_CLOSES + 5);
			break;
		}

		if (mac_reset_frame >= 0) {
			kernel_reset_animation(aa[0], mac_reset_frame);
			local->mac_frame = mac_reset_frame;
		}
	}
}


static void handle_animation_sit() {
	int sit_reset_frame;

	if (kernel_anim[aa[1]].frame != local->sit_frame) {
		local->sit_frame = kernel_anim[aa[1]].frame;
		sit_reset_frame = -1;

		switch (local->sit_frame) {

		case 31:
			player.commands_allowed = true;
			break;

		case 32:  /* end of look around 1 and end of sitting down */
		case 33:  /* end of look around 2      */
		case 34:  /* end of look around 3      */
		case 35:  /* end of look around 4      */

			if (local->sit_action == LOOK) {
				if (local->sit_count >= imath_random(10, 20)) {
					local->sit_count = 0;
					switch (local->sit_frame) {
					case 31:
					case 32: sit_reset_frame = 32; break;
					case 33: sit_reset_frame = imath_random(31, 33); break;
					case 34: sit_reset_frame = imath_random(32, 34); break;
					case 35: sit_reset_frame = 33; break;
					}
				} else {
					sit_reset_frame = local->sit_frame - 1;
					++local->sit_count;
				}

			} else switch (local->sit_frame) {
			case 31:
			case 32:
			case 33: sit_reset_frame = 35; player.commands_allowed = false; break;
			case 34: sit_reset_frame = 32; break;
			case 35: sit_reset_frame = 33; break;
			}
			break;

		case 62:
			player.walker_visible = true;
			player.commands_allowed = true;
			local->anim_1_running = false;
			player_demand_facing(FACING_SOUTH);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
			player.ready_to_walk = true;
			break;
		}

		if (sit_reset_frame >= 0) {
			kernel_reset_animation(aa[1], sit_reset_frame);
			local->sit_frame = sit_reset_frame;
		}
	}
}


static void handle_animation_a_macmorn() {
	int a_mac_reset_frame;
	int random;

	if (kernel_anim[aa[2]].frame != local->a_mac_frame) {
		local->a_mac_frame = kernel_anim[aa[2]].frame;
		a_mac_reset_frame = -1;

		switch (local->a_mac_frame) {

		case 114: /* just when macmorn and queen leave bottom of screen */

			/* Test for existance of PDD_6.SS.  If not here, */
			/* then this is demo version else continue game. */
			/* if (!env_exist (filename)) {
			  text_show (10625);
			  game.going = false;
			  win_status = 1;
			}  */

			new_room = 614;
			break;


		case 116: /* end of invisible */
			if (local->a_mac_action == MAC_INVISIBLE) {
				a_mac_reset_frame = 115;

			} else {
				a_mac_reset_frame = 0;
				sound_play(N_QueenMother);
			}
			break;

		case 15:  /* somewhere into macmorn coming into room */
		case 89:  /* right when macmorn grabs the queen      */
			conv_release();
			break;

		case 37:  /* end of walking into room */
		case 38:  /* end of freeze */
		case 39:  /* end of talk 1 */
		case 40:  /* end of talk 2 */
		case 41:  /* end of talk 3 */
		case 50:  /* end of come from talk */

			switch (local->a_mac_action) {

			case MAC_TALK:
				if (local->a_mac_talk_count == 0) {
					random = imath_random(1, 2);
					if (random == 1) {
						a_mac_reset_frame = 40;
					} else {
						a_mac_reset_frame = imath_random(37, 39);
						++local->a_mac_talk_count;
					}

				} else {
					a_mac_reset_frame = imath_random(37, 39);
					++local->a_mac_talk_count;
					if (local->a_mac_talk_count > 15) {
						local->a_mac_action = MAC_SHUT_UP;
						local->a_mac_talk_count = 0;
						a_mac_reset_frame = 37; /* make mac shut up */
					}
				}
				break;

			case MAC_SHUT_UP:
				a_mac_reset_frame = 37; /* make mac shut up */
				break;

			case MAC_LEAVE:
				a_mac_reset_frame = 50; /* make leave */
				break;
			}
			break;

		case 44:  /* end of talk 1 */
		case 45:  /* end of talk 2 */
		case 46:  /* end of talk 3 */

			switch (local->a_mac_action) {

			case MAC_TALK:
				a_mac_reset_frame = imath_random(43, 45);
				++local->a_mac_talk_count;
				if (local->a_mac_talk_count > 15) {
					local->a_mac_action = MAC_SHUT_UP;
					local->a_mac_talk_count = 0;
					a_mac_reset_frame = 46; /* make mac shut up */
				}
				break;

			default:
				a_mac_reset_frame = 46; /* make mac shut up */
				break;
			}
			break;

		case 61:
			kernel_reset_animation(aa[3], 129);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
			break;
		}

		if (a_mac_reset_frame >= 0) {
			kernel_reset_animation(aa[2], a_mac_reset_frame);
			local->a_mac_frame = a_mac_reset_frame;
		}
	}
}



static void handle_animation_a_queen() {
	int a_q_reset_frame;
	int random;

	if (kernel_anim[aa[3]].frame != local->a_q_frame) {
		local->a_q_frame = kernel_anim[aa[3]].frame;
		a_q_reset_frame = -1;

		switch (local->a_q_frame) {

		case 1:  /* on throne          */
		case 2:  /* on throne          */
		case 3:  /* on throne          */
		case 16: /* end talk           */
		case 37: /* end talk with fist */

			switch (local->a_q_action) {

			case QUEEN_TALK:
				a_q_reset_frame = 3;
				local->a_q_action = QUEEN_SHUT_UP;
				/* local->a_q_talk_count = 100; */
				break;

			case QUEEN_FIST:
				a_q_reset_frame = 20;
				local->a_q_action = QUEEN_SHUT_UP;
				/* local->a_q_talk_count = 100; */
				break;

			case QUEEN_SHUT_UP:
				++local->a_q_talk_count;
				if (local->a_q_talk_count > imath_random(15, 25)) {
					local->a_q_talk_count = 0;
					random = imath_random(1, 3);
					switch (random) {
					case 1: a_q_reset_frame = imath_random(0, 1); break;
					case 2: a_q_reset_frame = imath_random(0, 2); break;
					case 3: a_q_reset_frame = imath_random(1, 2); break;
					}

				} else {
					a_q_reset_frame = local->a_q_frame - 1;
				}
				break;

			case QUEEN_GET_UP:
				a_q_reset_frame = 37;
				local->a_q_action = QUEEN_SHUT_UP;
				/* local->a_q_talk_count = 100; */
				break;
			}
			break;

		case 42: /* almost end of getting up */
			conv_release();
			break;

		case 81:  /* end of get up from throne and freeze standing */
		case 93:  /* end of talk                                   */

			switch (local->a_q_action) {

			case QUEEN_TALK:
				a_q_reset_frame = 81;
				local->a_q_action = QUEEN_SHUT_UP;
				break;

			default:
				a_q_reset_frame = 80; /* stay standing */
				break;
			}
			break;

		case 98: /* when queen turns to walk south, send her to 131 to make her invisible */
			a_q_reset_frame = 130;
			break;

		case 131:  /* keep queen invisible when she turns to walk south */
			a_q_reset_frame = 130;
			break;
		}

		if (a_q_reset_frame >= 0) {
			kernel_reset_animation(aa[3], a_q_reset_frame);
			local->a_q_frame = a_q_reset_frame;
		}
	}
}


static void handle_animation_a_qmother() {
	int a_qm_reset_frame;
	int random;

	if (kernel_anim[aa[4]].frame != local->a_qm_frame) {
		local->a_qm_frame = kernel_anim[aa[4]].frame;
		a_qm_reset_frame = -1;

		switch (local->a_qm_frame) {

		case 9:   /* end of freeze */
		case 10:  /* end of freeze */
		case 23:  /* end of freeze */

			switch (local->a_qm_action) {

			case QUEEN_TALK:
				a_qm_reset_frame = 23;
				break;

			case QUEEN_FIST:
				a_qm_reset_frame = 1;
				local->a_qm_action = QUEEN_SHUT_UP;
				break;

			case QUEEN_SHUT_UP:
				++local->a_qm_talk_count;
				if (local->a_qm_talk_count > imath_random(15, 25)) {
					local->a_qm_talk_count = 0;
					random = imath_random(1, 4);
					switch (random) {
					case 1: a_qm_reset_frame = 8;  break;
					case 2: a_qm_reset_frame = 9;  break;
					case 3: a_qm_reset_frame = 12; break;
					case 4: a_qm_reset_frame = 23; break;
					}

				} else {
					a_qm_reset_frame = local->a_qm_frame - 1;
				}
				break;
			}
			break;

		case 18:  /* end of freeze */

			switch (local->a_qm_action) {

			case QUEEN_TALK:
			case QUEEN_FIST:
				a_qm_reset_frame = 18;
				break;

			case QUEEN_SHUT_UP:
				++local->a_qm_talk_count;
				if (local->a_qm_talk_count > imath_random(20, 30)) {
					local->a_qm_talk_count = 0;
					a_qm_reset_frame = imath_random(17, 18);

				} else {
					a_qm_reset_frame = 17;
				}
				break;
			}
			break;

		case 24:  /* end of freeze */
		case 25:  /* end of freeze */
		case 32:  /* end of talk   */
		case 44:  /* end of talk   */
		case 54:  /* end of talk   */

			switch (local->a_qm_action) {

			case QUEEN_TALK:
				random = imath_random(1, 3);
				switch (random) {
				case 1: a_qm_reset_frame = 25;  break;
				case 2: a_qm_reset_frame = 32;  break;
				case 3: a_qm_reset_frame = 44;  break;
				}
				local->a_qm_action = QUEEN_SHUT_UP;
				break;

			case QUEEN_FIST:
				a_qm_reset_frame = 9;
				break;

			case QUEEN_SHUT_UP:
				++local->a_qm_talk_count;
				if (local->a_qm_talk_count > imath_random(15, 25)) {
					local->a_qm_talk_count = 0;
					random = imath_random(1, 3);
					switch (random) {
					case 1: a_qm_reset_frame = 23;  break;
					case 2: a_qm_reset_frame = 24;  break;
					case 3: a_qm_reset_frame = 9;   break;
					}

				} else {
					a_qm_reset_frame = local->a_qm_frame - 1;
				}
				break;
			}
			break;
		}

		if (a_qm_reset_frame >= 0) {
			kernel_reset_animation(aa[4], a_qm_reset_frame);
			local->a_qm_frame = a_qm_reset_frame;
		}
	}
}


void room_106_daemon() {
	int temp;

	if (local->anim_0_running) {
		handle_animation_mac();
	}

	if (local->anim_1_running) {
		handle_animation_sit();
	}

	if (local->anim_2_running) {
		handle_animation_a_macmorn();
	}

	if (local->anim_3_running) {
		handle_animation_a_queen();
	}

	if (local->anim_4_running) {
		handle_animation_a_qmother();
	}

	if (player.y > 108 && !local->prevent && player.x > 160 && !player.been_here_before) {
		local->prevent = true;
		kernel_timing_trigger(1, ROOM_106_DOOR_CLOSES + 2);
	}  /* close door upon entering from RIGHT side when going to converse with Macmorn */

	if (player.y > 108 && !local->prevent && player.x < 160 && !player.been_here_before) {
		local->prevent = true;
		kernel_timing_trigger(1, ROOM_106_DOOR_CLOSES);
	}  /* close door upon entering from LEFT side when going to converse with Macmorn */

	/* Close either door 104 or 105 when player enters */

	switch (kernel.trigger) {

	case ROOM_106_DOOR_CLOSES: /* left door closes */
		kernel_seq_delete(seq[fx_door_104]);
		sound_play(N_DoorCloses);
		seq[fx_door_104] = kernel_seq_backward(ss[fx_door_104], false,
			6, 0, 0, 1);
		kernel_seq_depth(seq[fx_door_104], 5);
		kernel_seq_range(seq[fx_door_104], 1, 3);
		kernel_seq_trigger(seq[fx_door_104],
			KERNEL_TRIGGER_EXPIRE, 0, ROOM_106_DOOR_CLOSES + 1);
		break;

	case ROOM_106_DOOR_CLOSES + 1:
		temp = seq[fx_door_104];
		seq[fx_door_104] = kernel_seq_stamp(ss[fx_door_104], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door_104], 5);
		kernel_synch(KERNEL_SERIES, seq[fx_door_104], KERNEL_SERIES, temp);
		if (player.been_here_before) {
			player.commands_allowed = true;
		}
		break;

	case ROOM_106_DOOR_CLOSES + 2:  /* right door closes */
		kernel_seq_delete(seq[fx_door_105]);
		sound_play(N_DoorCloses);
		seq[fx_door_105] = kernel_seq_backward(ss[fx_door_105], false,
			6, 0, 0, 1);
		kernel_seq_depth(seq[fx_door_105], 5);
		kernel_seq_range(seq[fx_door_105], 1, 3);
		kernel_seq_trigger(seq[fx_door_105],
			KERNEL_TRIGGER_EXPIRE, 0, ROOM_106_DOOR_CLOSES + 3);
		break;

	case ROOM_106_DOOR_CLOSES + 3:
		temp = seq[fx_door_105];
		seq[fx_door_105] = kernel_seq_stamp(ss[fx_door_105], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door_105], 5);
		kernel_synch(KERNEL_SERIES, seq[fx_door_105], KERNEL_SERIES, temp);
		if (player.been_here_before) {
			player.commands_allowed = true;
		}
		break;
	}

	switch (kernel.trigger) {
	case ROOM_106_DOOR_CLOSES + 4:
		temp = seq[fx_door_104];
		seq[fx_door_104] = kernel_seq_stamp(ss[fx_door_104], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_door_104], 8);
		kernel_synch(KERNEL_SERIES, seq[fx_door_104], KERNEL_SERIES, temp);
		break;

	case ROOM_106_DOOR_CLOSES + 5:
		player.commands_allowed = true;
		temp = seq[fx_door_104];
		seq[fx_door_104] = kernel_seq_stamp(ss[fx_door_104], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door_104], 5);
		kernel_synch(KERNEL_SERIES, seq[fx_door_104], KERNEL_SERIES, temp);
		break;
	}

	if (kernel.trigger == ROOM_106_RUN_CONV) {
		conv_run(CONV_34_CUT_SCENE);
	}

	if (kernel.trigger == ROOM_106_END_GAME_1) {
		if (global[llanie_status] == IS_SAVED) {
			new_room = 909;
		} else {
			new_room = 206;
		}
	}
}

static void process_conversation_mac() {
	int you_trig_flag = false;

	if (player_verb == conv003_replies_b_b) {
		local->mac_action = MAC_SHUT_UP;
		you_trig_flag = true;
	}

	if (player_verb == conv003_exit_b_b) {
		local->mac_action = MAC_LEAVE;
		you_trig_flag = true;
	}

	if (player_verb == conv003_replies_defeat) {
		if (!kernel.trigger) {
			conv_you_trigger(ROOM_106_POINT);
			you_trig_flag = true;
		}
	}

	switch (kernel.trigger) {
	case ROOM_106_POINT:
		local->mac_action = MAC_POINT;
		you_trig_flag = true;
		break;

	case ROOM_106_YOU_TALK:
		local->mac_action = MAC_TALK;
		break;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_106_YOU_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */

	local->mac_talk_count = 0;
}

static void process_conv_abduction() {
	int you_trig_flag = false;
	int me_trig_flag = false;

	switch (player_verb) {
	case conv034_exit_b_b:
		you_trig_flag = true;
		me_trig_flag = true;
		local->a_mac_action = MAC_LEAVE;
		break;

	case conv034_exit_a_a:
		you_trig_flag = true;
		me_trig_flag = true;
		break;

	case conv034_five_b_b:
		conv_hold();
		you_trig_flag = true;
		me_trig_flag = true;
		local->a_q_action = QUEEN_GET_UP;
		break;

	case conv034_eight_b_b:
		conv_hold();
		you_trig_flag = true;
		me_trig_flag = true;
		local->a_mac_action = MAC_SHUT_UP;
		break;
	}

	if (kernel.trigger == ROOM_106_YOU_TALK) {
		if (player_verb == conv034_eight_only ||
			player_verb == conv034_final_only ||
			player_verb == conv034_nine_only) {
			local->a_mac_action = MAC_TALK;
			local->a_qm_action = QUEEN_SHUT_UP;

		} else {
			local->a_q_action = QUEEN_TALK;
			local->a_qm_action = QUEEN_SHUT_UP;
		}
	}

	if (kernel.trigger == ROOM_106_ME_TALK) {
		local->a_q_action = QUEEN_SHUT_UP;
		if (player_verb == conv034_seven_only) {
			local->a_qm_action = QUEEN_FIST;
		} else if (player_verb != conv034_exit_b_b) {
			local->a_qm_action = QUEEN_TALK;
		}
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_106_YOU_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_106_ME_TALK);
	} /* if me_trig_flag == true, then a me trigger is called from above, not here. */

	local->a_mac_talk_count = 0;
	local->a_q_talk_count = 0;
	local->a_qm_talk_count = 0;
}

void room_106_pre_parser() {
	int count;

	if (player_said_2(walk_through, door_to_courtyard) || player_said_2(open, door_to_courtyard) ||
		player_said_2(pull, door_to_courtyard)) {
		player.walk_off_edge_to_room = 118;
	}

	if ((player_said_1(look_at) || player_said_1(look)) &&
		player_said_1(Dragonsphere)) {

		if (!(global[player_score_flags] & SCORE_LOOK_SPHERE_106)) {
			global[player_score_flags] = global[player_score_flags] | SCORE_LOOK_SPHERE_106;
			global[player_score] += 1;
		}

		player.need_to_walk = true;
		/* player_walk(START_X_ROOM_111, START_Y_ROOM_111, FACING_NORTHEAST); */
	}

	if (local->anim_1_running) {

		if ((player_said_1(look_at) || player_said_1(look)) &&
			player_said_1(king_s_throne)) {
			player.need_to_walk = false;

		} else if ((player_said_1(look_at) || player_said_1(look)) &&
			player_said_1(Dragonsphere)) {
			player.need_to_walk = false;

		} else if (player.need_to_walk) {
			player.ready_to_walk = false;
			local->sit_action = LEAVE;
			for (count = 0; count < room_num_spots; count++) {
				if (room_spots[count].vocab == words_king_s_throne) {
					room_spots[count].verb = words_sit_on;
					room_spots[count].cursor_number = 3;
				}
			}
		}
	}
}

void room_106_parser() {
	int temp;
	int count;

	if (conv_control.running == CONV_3_MACMORN) {
		process_conversation_mac();
		goto handled;
	}

	if (conv_control.running == CONV_34_CUT_SCENE) {
		process_conv_abduction();
		goto handled;
	}

	if (player.look_around) {
		text_show(10601);
		goto handled;
	}

	if (player_said_2(sit_on, king_s_throne)) {

		for (count = 0; count < room_num_spots; count++) {
			if (room_spots[count].vocab == words_king_s_throne) {
				room_spots[count].verb = words_look_at;
				room_spots[count].cursor_number = 1;
			}
		}

		aa[1] = kernel_run_animation(kernel_name('s', 1), 0);
		player.commands_allowed = false;
		player.walker_visible = false;
		local->anim_1_running = true;
		local->sit_action = LOOK;
		local->sit_count = 0;
		kernel_synch(KERNEL_ANIM, aa[1], KERNEL_PLAYER, 0);
		goto handled;
	}

	if (player_said_2(walk_through, door_to_meeting_room) || player_said_2(open, door_to_meeting_room) ||
		player_said_2(pull, door_to_meeting_room)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			seq[fx_open_door] = kernel_seq_pingpong(ss[fx_open_door], false, 7, 0, 0, 2);
			kernel_seq_player(seq[fx_open_door], true);
			kernel_seq_trigger(seq[fx_open_door],
				KERNEL_TRIGGER_SPRITE, 2, 1);
			kernel_seq_trigger(seq[fx_open_door],
				KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 1:
			kernel_seq_delete(seq[fx_door_104]);
			sound_play(N_DoorOpens);
			seq[fx_door_104] = kernel_seq_forward(ss[fx_door_104],
				false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door_104], 8);
			kernel_seq_trigger(seq[fx_door_104],
				KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			temp = seq[fx_door_104];
			seq[fx_door_104] = kernel_seq_stamp(ss[fx_door_104], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_door_104], 8);
			kernel_synch(KERNEL_SERIES, seq[fx_door_104], KERNEL_SERIES, temp);
			break;

		case 3:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_open_door]);
			kernel_timing_trigger(1, 4);
			break;

		case 4:
			player_walk(START_X_ROOM_104, START_Y_ROOM_104, FACING_NORTH);
			player_walk_trigger(5);
			break;

		case 5:
			kernel_seq_delete(seq[fx_door_104]);
			sound_play(N_DoorCloses);
			seq[fx_door_104] = kernel_seq_backward(ss[fx_door_104], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door_104], 1);
			kernel_seq_trigger(seq[fx_door_104], KERNEL_TRIGGER_EXPIRE, 0, 6);
			break;

		case 6:
			temp = seq[fx_door_104];
			seq[fx_door_104] = kernel_seq_stamp(ss[fx_door_104], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door_104], 1);
			kernel_synch(KERNEL_SERIES, seq[fx_door_104], KERNEL_SERIES, temp);
			kernel_timing_trigger(6, 7);
			break;

		case 7:
			new_room = 104;
			break;
		}
		goto handled;
	}

	if (player_said_2(walk_through, door_to_ballroom) || player_said_2(open, door_to_ballroom) ||
		player_said_2(pull, door_to_ballroom)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			seq[fx_open_door] = kernel_seq_pingpong(ss[fx_open_door], true, 7, 0, 0, 2);
			kernel_seq_player(seq[fx_open_door], true);
			kernel_seq_trigger(seq[fx_open_door],
				KERNEL_TRIGGER_SPRITE, 2, 1);
			kernel_seq_trigger(seq[fx_open_door],
				KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 1:
			kernel_seq_delete(seq[fx_door_105]);
			sound_play(N_DoorOpens);
			seq[fx_door_105] = kernel_seq_forward(ss[fx_door_105],
				false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door_105], 8);
			kernel_seq_trigger(seq[fx_door_105],
				KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			temp = seq[fx_door_105];
			seq[fx_door_105] = kernel_seq_stamp(ss[fx_door_105], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_door_105], 8);
			kernel_synch(KERNEL_SERIES, seq[fx_door_105], KERNEL_SERIES, temp);
			break;

		case 3:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_open_door]);
			kernel_timing_trigger(1, 4);
			break;

		case 4:
			player_walk(START_X_ROOM_105 - 7, START_Y_ROOM_105 - 2, FACING_NORTH);
			player_walk_trigger(5);
			break;

		case 5:
			kernel_seq_delete(seq[fx_door_105]);
			sound_play(N_DoorCloses);
			seq[fx_door_105] = kernel_seq_backward(ss[fx_door_105], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door_105], 1);
			kernel_seq_trigger(seq[fx_door_105], KERNEL_TRIGGER_EXPIRE, 0, 6);
			break;

		case 6:
			temp = seq[fx_door_105];
			seq[fx_door_105] = kernel_seq_stamp(ss[fx_door_105], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door_105], 1);
			kernel_synch(KERNEL_SERIES, seq[fx_door_105], KERNEL_SERIES, temp);
			kernel_timing_trigger(6, 7);
			break;

		case 7:
			new_room = 105;
			break;
		}
		goto handled;
	}

	if (player_said_2(walk_through, door_to_council_room) || player_said_2(open, door_to_council_room) ||
		player_said_2(pull, door_to_council_room)) {
		new_room = 107;
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(Dragonsphere)) {
			new_room = 111;
			goto handled;
		}

		if (player_said_1(chair)) {
			text_show(10602);
			goto handled;
		}

		if (player_said_1(queen_s_throne)) {
			text_show(10603);
			goto handled;
		}

		if (player_said_1(king_s_throne)) {
			if (local->anim_1_running) {
				text_show(10624);
			} else {
				text_show(10605);
			}
			goto handled;
		}

		if (player_said_1(door_to_meeting_room)) {
			text_show(10606);
			goto handled;
		}

		if (player_said_1(door_to_courtyard)) {
			text_show(10607);
			goto handled;
		}

		if (player_said_1(door_to_ballroom)) {
			text_show(10608);
			goto handled;
		}

		if (player_said_1(table)) {
			if (inter_point_x < 120) {
				text_show(10609);
			} else {
				text_show(10623);
			}
			goto handled;
		}

		if (player_said_1(carpet)) {
			text_show(10610);
			goto handled;
		}

		if (player_said_1(tapestry)) {
			if (inter_point_x <= LEFT_TAPESTRY_X) {
				text_show(10612);
			} else {
				text_show(10613);
			}
			goto handled;
		}

		if (player_said_1(platform)) {
			text_show(10616);
			goto handled;
		}

		if (player_said_1(step)) {
			text_show(10617);
			goto handled;
		}

		if (player_said_1(door_to_council_room)) {
			text_show(10618);
			goto handled;
		}

		if (player_said_1(wall_plaque)) {
			text_show(10619);
			goto handled;
		}

		if (player_said_1(floor)) {
			text_show(10621);
			goto handled;
		}
	}

	if (player_said_2(sit_on, queen_s_throne)) {
		text_show(10604);
		goto handled;
	}

	if (player_said_2(pull, tapestry)) {
		text_show(10614);
		goto handled;
	}

	if (player_said_2(take, Dragonsphere)) {
		text_show(10615);
		goto handled;
	}

	if (player_said_2(pull, carpet)) {
		text_show(10611);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_106_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.prevent);
	s.syncAsSint16LE(scratch.mac_frame);
	s.syncAsSint16LE(scratch.mac_action);
	s.syncAsSint16LE(scratch.mac_talk_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.sit_frame);
	s.syncAsSint16LE(scratch.sit_action);
	s.syncAsSint16LE(scratch.sit_count);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.a_mac_frame);
	s.syncAsSint16LE(scratch.a_mac_action);
	s.syncAsSint16LE(scratch.a_mac_talk_count);
	s.syncAsSint16LE(scratch.anim_2_running);
	s.syncAsSint16LE(scratch.a_q_frame);
	s.syncAsSint16LE(scratch.a_q_action);
	s.syncAsSint16LE(scratch.a_q_talk_count);
	s.syncAsSint16LE(scratch.anim_3_running);
	s.syncAsSint16LE(scratch.a_qm_frame);
	s.syncAsSint16LE(scratch.a_qm_action);
	s.syncAsSint16LE(scratch.a_qm_talk_count);
	s.syncAsSint16LE(scratch.anim_4_running);
}

void room_106_preload() {
	room_init_code_pointer = room_106_init;
	room_pre_parser_code_pointer = room_106_pre_parser;
	room_parser_code_pointer = room_106_parser;
	room_daemon_code_pointer = room_106_daemon;

	if (global[end_of_game]) {
		global[no_load_walker] = true;
	}

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
