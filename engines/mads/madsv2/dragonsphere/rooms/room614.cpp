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
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section6.h"
#include "mads/madsv2/dragonsphere/rooms/room614.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[19];       /* Sprite series handles */
	int16 sequence[19];     /* Sequence handles      */
	int16 animation[8];     /* Animation handles     */

	int16 prevent;
	int16 doofus;
	int16 stone_taking_count;

	int16 king_enter_frame;       /* animation frame being held for king_enter stuff */
	int16 king_enter_action;      /* Type of action to run for king_enter animation */
	int16 king_enter_talk_count;  /* counter for king_enter talking */
	int16 anim_0_running;

	int16 king_float_frame;       /* animation frame being held for king_float stuff */
	int16 king_float_action;      /* Type of action to run for king_float animation */
	int16 king_float_talk_count;  /* counter for king_float talking */
	int16 anim_1_running;

	int16 pid_float_frame;       /* animation frame being held for pid_float stuff */
	int16 pid_float_action;      /* Type of action to run for pid_float animation */
	int16 pid_float_talk_count;  /* counter for pid_float talking */
	int16 anim_2_running;

	int16 wiz_float_frame;       /* animation frame being held for wiz_float stuff */
	int16 wiz_float_action;      /* Type of action to run for wiz_float animation */
	int16 wiz_float_talk_count;  /* counter for wiz_float talking */
	int16 anim_3_running;

	int16 wiz_zap_frame;         /* animation frame being held for wiz_zap stuff */
	int16 wiz_zap_action;        /* Type of action to run for wiz_zap animation */
	int16 anim_4_running;

	int16 wiz_murder_frame;       /* animation frame being held for wiz_murder stuff */
	int16 wiz_murder_action;      /* Type of action to run for wiz_murder animation */
	int16 anim_5_running;

	int16 wiz_ice_frame;       /* animation frame being held for wiz_ice stuff */
	int16 wiz_ice_action;      /* Type of action to run for wiz_ice animation */
	int16 anim_6_running;

	int16 wiz_end_frame;       /* animation frame being held for wiz_end stuff */
	int16 wiz_end_action;      /* Type of action to run for wiz_end animation */
	int16 anim_7_running;

	long clock;
	long death_timer;       /* when this counter reaches max sec, kill pid */
	int16 activate_timer;     /* If T, will start counting death_timer */

	int16 did_not_throw;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation


/* ========================= Sprite Series =================== */

#define fx_bundle               0       /* rm614p1  */
#define fx_ball                 1       /* rm614p0  */
#define fx_map                  2       /* rm614p2  */
#define fx_take                 3       /* @kgrd_7  */
#define fx_take_high            4       /* @kgrh_9  */
#define fx_power_ball           5       /* rm614x9  */
#define fx_atom_1               6       /* rm614y0  */
#define fx_atom_2               7       /* rm614y1  */
#define fx_atom_3               8       /* rm614y2  */
#define fx_atom_4               9       /* rm614y3  */
#define fx_stone_to_wiz         10      /* rm614x6  */
#define fx_stone_to_sphere      11      /* rm614x5  */
#define fx_ball_fly_1           12      /* rm614z2  */
#define fx_ball_fly_2           13      /* rm614z3  */
#define fx_ball_fly_3           14      /* rm614z4  */
#define fx_ball_fly_4           15      /* rm614z5  */
#define fx_single_ball          16      /* rm614z6  */
#define fx_sphere_base          17      /* rm614z7  */


/* ========================= Triggers ======================== */

#define ROOM_614_ME_TALK        60 
#define ROOM_614_YOU_TALK       62 
#define ROOM_614_DONE_ZAPPING   64 
#define ROOM_614_STONE_TO_WIZ   68 
#define ROOM_614_STAMP_BALL     75 


/* ========================= Other Macros ==================== */

#define CONV_31_WIZARD          31
#define CONV_32_WIZARD          32

#define PLAYER_X_FROM_613       0
#define PLAYER_Y_FROM_613       104

#define WALK_TO_X_FROM_613      37
#define WALK_TO_Y_FROM_613      105

#define BALL_X                  179
#define BALL_Y                  109

#define LENGTH_OF_LIFE          1300 

#define FLOAT_UP                0
#define TALK                    1
#define FREEZE                  2
#define MORPH                   3
#define THROW                   4
#define DIE                     5
#define RELEASE                 6
#define UNCOVER_EYES            7
#define RAISE_KING              8
#define HAND_CHIN               9
#define KILL_PID                10
#define WAITING_FOR_STONE       11
#define ICE_UP                  12
#define BACK_TO_CAMERA          13
#define TURN_TO_PID             14

#define CIRCLE_X                256
#define CIRCLE_Y                61

static Scratch scratch;


static void room_614_init() {
	if (player_has(red_powerstone))    global[has_red] = true;
	if (player_has(yellow_powerstone)) global[has_yellow] = true;
	if (player_has(blue_powerstone))   global[has_blue] = true;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_2_running = false;
		local->anim_3_running = false;
		local->anim_4_running = false;
		local->anim_5_running = false;
		local->anim_6_running = false;
		local->anim_7_running = false;
		local->death_timer = 0;
		local->clock = 0;
		local->activate_timer = false;
		local->doofus = false;
		local->did_not_throw = true;
		local->stone_taking_count = 0;
	}

	if (global[player_persona] == PLAYER_IS_PID) {
		kernel_flip_hotspot(words_Sanwe, false);
		kernel_flip_hotspot_loc(words_circle_of_spheres, false, CIRCLE_X, CIRCLE_Y);
	}

	local->prevent = false;
	local->king_enter_talk_count = 0;
	local->king_float_talk_count = 0;
	local->pid_float_talk_count = 0;
	local->wiz_float_talk_count = 0;

	if (!global[wizard_dead]) {
		conv_get(CONV_31_WIZARD);
		conv_get(CONV_32_WIZARD);

		kernel_load_series(kernel_name('a', 6), false);
		kernel_load_series(kernel_name('x', 8), false);
		/* to prevent palette flashing */

		ss[fx_stone_to_wiz] = kernel_load_series(kernel_name('x', 6), false);
		ss[fx_stone_to_sphere] = kernel_load_series(kernel_name('x', 5), false);
	}

	/* This is to prevent pallette frag */
	kernel_load_series(kernel_name('b', 0), false);

	ss[fx_sphere_base] = kernel_load_series(kernel_name('z', 7), false);

	if (global[player_persona] == PLAYER_IS_KING) {
		ss[fx_ball_fly_1] = kernel_load_series(kernel_name('z', 2), false);
		ss[fx_ball_fly_2] = kernel_load_series(kernel_name('z', 3), false);
		ss[fx_ball_fly_3] = kernel_load_series(kernel_name('z', 4), false);
		ss[fx_ball_fly_4] = kernel_load_series(kernel_name('z', 5), false);

		ss[fx_atom_1] = kernel_load_series(kernel_name('y', 0), false);
		ss[fx_atom_2] = kernel_load_series(kernel_name('y', 1), false);
		ss[fx_atom_3] = kernel_load_series(kernel_name('y', 2), false);
		ss[fx_atom_4] = kernel_load_series(kernel_name('y', 3), false);

		seq[fx_atom_1] = kernel_seq_forward(ss[fx_atom_1], false, 5, 0, 0, 0);
		kernel_seq_depth(seq[fx_atom_1], 1);
		kernel_seq_range(seq[fx_atom_1], KERNEL_FIRST, KERNEL_LAST);

		seq[fx_atom_2] = kernel_seq_forward(ss[fx_atom_2], false, 5, 0, 0, 0);
		kernel_seq_depth(seq[fx_atom_2], 1);
		kernel_seq_range(seq[fx_atom_2], KERNEL_FIRST, KERNEL_LAST);

		seq[fx_atom_3] = kernel_seq_forward(ss[fx_atom_3], false, 5, 0, 0, 0);
		kernel_seq_depth(seq[fx_atom_3], 1);
		kernel_seq_range(seq[fx_atom_3], KERNEL_FIRST, KERNEL_LAST);

		seq[fx_atom_4] = kernel_seq_forward(ss[fx_atom_4], false, 5, 0, 0, 0);
		kernel_seq_depth(seq[fx_atom_4], 1);
		kernel_seq_range(seq[fx_atom_4], KERNEL_FIRST, KERNEL_LAST);

		ss[fx_power_ball] = kernel_load_series(kernel_name('x', 9), false);
		seq[fx_power_ball] = kernel_seq_stamp(ss[fx_power_ball], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_power_ball], 3);

	} else {
		seq[fx_sphere_base] = kernel_seq_stamp(ss[fx_sphere_base], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_sphere_base], 1);
		/* after explosion, stamp bottom of sphere (just a chunk of it) */
	}


	/* ===================== Load Sprite Series ================== */

	if (object_is_here(crystal_ball) || object_is_here(medicine_bundle) ||
		object_is_here(black_sphere)) {
		ss[fx_take] = kernel_load_series("*PDDRL_9", false);
	}

	if (object_is_here(black_sphere)) {
		ss[fx_single_ball] = kernel_load_series(kernel_name('z', 6), false);
		if (global[wizard_dead]) {
			seq[fx_single_ball] = kernel_seq_stamp(ss[fx_single_ball], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_single_ball], 14);
		} else {
			kernel_flip_hotspot(words_black_sphere, false);
		}
	} else {
		kernel_flip_hotspot(words_black_sphere, false);
	}

	if (object_is_here(crystal_ball)) {
		ss[fx_ball] = kernel_load_series(kernel_name('p', 0), false);
		seq[fx_ball] = kernel_seq_stamp(ss[fx_ball], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_ball], 14);
	} else {
		kernel_flip_hotspot(words_crystal_ball, false);
	}

	if (object_is_here(map)) {
		ss[fx_take_high] = kernel_load_series("*PDDRM_8", false);
		ss[fx_map] = kernel_load_series(kernel_name('p', 2), false);
		seq[fx_map] = kernel_seq_stamp(ss[fx_map], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_map], 6);
	} else {
		kernel_flip_hotspot(words_map, false);
	}

	if (object_is_here(medicine_bundle)) {
		ss[fx_bundle] = kernel_load_series(kernel_name('p', 1), false);
		seq[fx_bundle] = kernel_seq_stamp(ss[fx_bundle], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_bundle], 14);
	} else {
		kernel_flip_hotspot(words_spirit_bundle, false);
	}


	/* ========================= Previous Rooms ================== */

	if (previous_room == KERNEL_RESTORING_GAME) {
		if (!global[wizard_dead]) {
			player.walker_visible = false;
			local->death_timer = 0;
			local->clock = 0;

			if (local->anim_6_running) {
				aa[2] = kernel_run_animation(kernel_name('p', 1), 0);
				aa[6] = kernel_run_animation(kernel_name('w', 4), 0);
				kernel_reset_animation(aa[6], 31);
				/* now it's time to mimic stone */

			} else {
				aa[2] = kernel_run_animation(kernel_name('p', 1), 0);
				aa[3] = kernel_run_animation(kernel_name('w', 1), 0);
				kernel_reset_animation(aa[3], 5);
				/* now it's time to throw stone at circle of spheres */
			}
		}

	} else if (previous_room == 106) {
		player.x = WALK_TO_X_FROM_613;
		player.y = WALK_TO_Y_FROM_613;
		player.facing = FACING_EAST;

	} else if ((previous_room == 613) || (previous_room != KERNEL_RESTORING_GAME)) {
		player.walker_visible = false;
		player.commands_allowed = false;

		kernel_init_dialog();
		kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

		aa[3] = kernel_run_animation(kernel_name('w', 1), 0);
		local->anim_3_running = true;
		local->wiz_float_action = FREEZE;

		aa[0] = kernel_run_animation(kernel_name('k', 1), 0);
		local->anim_0_running = true;
		local->king_enter_action = FREEZE;
	}

	section_6_music();
}

static void handle_animation_king_enter() {
	int king_enter_reset_frame;

	if (kernel_anim[aa[0]].frame != local->king_enter_frame) {
		local->king_enter_frame = kernel_anim[aa[0]].frame;
		king_enter_reset_frame = -1;

		switch (local->king_enter_frame) {

		case 25:  /* almost end of king coming into room */
			local->wiz_float_action = TALK;
			break;

		case 26:  /* at end of king walking into room */
		case 27:  /* at end of king talking           */
		case 28:  /* at end of king talking           */
		case 29:  /* at end of king talking           */
		case 30:  /* at end of king talking           */
		case 31:  /* at end of king talking           */
		case 32:  /* at end of king talking           */
		case 33:  /* at end of king talking           */
		case 34:  /* at end of king talking           */
			switch (local->king_enter_action) {
			case FREEZE:
				king_enter_reset_frame = 25;
				break;

			case TALK:
				king_enter_reset_frame = imath_random(26, 33);
				++local->king_enter_talk_count;
				if (local->king_enter_talk_count > 15) {
					local->king_enter_action = FREEZE;
					local->king_enter_talk_count = 0;
					king_enter_reset_frame = 25; /* make king_enter shut up */
				}
				break;
			}
			break;
		}

		if (king_enter_reset_frame >= 0) {
			kernel_reset_animation(aa[0], king_enter_reset_frame);
			local->king_enter_frame = king_enter_reset_frame;
		}
	}
}

static void handle_animation_king_float() {
	int king_float_reset_frame;
	int tmp;

	if (kernel_anim[aa[1]].frame != local->king_float_frame) {
		local->king_float_frame = kernel_anim[aa[1]].frame;
		king_float_reset_frame = -1;

		switch (local->king_float_frame) {
		case 11:  /* at end of king floating up */
		case 12:  /* at end of king floating    */
		case 13:  /* at end of king floating    */
		case 14:  /* at end of king floating    */
		case 15:  /* at end of king floating    */
			switch (local->king_float_action) {
			case FREEZE:
				tmp = imath_random(1, 3);
				if (tmp == 1) {
					king_float_reset_frame = local->king_float_frame - 2;
					if (king_float_reset_frame < 10) {
						king_float_reset_frame = 10;
					}

				} else if (tmp == 2) {
					king_float_reset_frame = local->king_float_frame - 1;
				}

				if (local->king_float_frame == 15) {
					if (king_float_reset_frame == -1) {
						king_float_reset_frame = 14;
					}
				}
				break;

			case MORPH:
				king_float_reset_frame = 15;
				break;
			}
			break;

		case 47:
			king_float_reset_frame = 46;
			local->wiz_zap_action = FREEZE;
			break;
		}

		if (king_float_reset_frame >= 0) {
			kernel_reset_animation(aa[1], king_float_reset_frame);
			local->king_float_frame = king_float_reset_frame;
		}
	}
}

static void handle_animation_pid_float() {
	int pid_float_reset_frame;
	int tmp;

	if (kernel_anim[aa[2]].frame != local->pid_float_frame) {
		local->pid_float_frame = kernel_anim[aa[2]].frame;
		pid_float_reset_frame = -1;

		switch (local->pid_float_frame) {
		case 1:  /* at end of pid floating */
		case 2:  /* at end of pid floating */
		case 3:  /* at end of pid floating */
		case 31: /* at end of pid throwing */
			switch (local->pid_float_action) {
			case FREEZE:
				tmp = imath_random(1, 3);
				if (tmp == 1) {
					pid_float_reset_frame = local->pid_float_frame - 2;
					if (pid_float_reset_frame < 0) {
						pid_float_reset_frame = 0;
					}

				} else if (tmp == 2) {
					pid_float_reset_frame = local->pid_float_frame - 1;
				}

				if (local->pid_float_frame >= 3) {
					if (pid_float_reset_frame == -1) {
						pid_float_reset_frame = 2;
					}
				}
				break;

			case THROW:
				pid_float_reset_frame = 17;
				local->pid_float_action = FREEZE;
				break;

			case ICE_UP:
				kernel_abort_animation(aa[3]); /* abort wizard */
				kernel_abort_animation(aa[2]); /* abort pid    */
				aa[7] = kernel_run_animation(kernel_name('w', 5), 0);
				kernel_synch(KERNEL_ANIM, aa[7], KERNEL_NOW, 0);
				local->anim_2_running = false;
				local->anim_3_running = false;
				local->anim_7_running = true;
				pid_float_reset_frame = -1;
				break;

			case DIE:
				pid_float_reset_frame = 3;
				break;

			case RELEASE:
				player.commands_allowed = false;
				pid_float_reset_frame = 31;
				break;
			}
			break;

		case 17: /* end of pid dieing */
			sound_play(N_PlayerDies);
			if (game.difficulty == EASY_MODE) {
				text_show(61471);
			} else {
				text_show(45);
			}
			conv_reset(CONV_31_WIZARD);
			conv_reset(CONV_32_WIZARD);
			global[no_load_walker] = false;
			new_room = 613;
			break;

		case 46:  /* at end of dropping and shielding eyes */
			switch (local->pid_float_action) {
			case RELEASE:
				pid_float_reset_frame = 45;
				break;

			case UNCOVER_EYES:
				pid_float_reset_frame = 46;
				break;
			}
			break;

		case 50: /* end of uncovering eyes */
			new_room = 106;
			break;
		}

		if (pid_float_reset_frame >= 0) {
			kernel_reset_animation(aa[2], pid_float_reset_frame);
			local->pid_float_frame = pid_float_reset_frame;
		}
	}
}

static void handle_animation_wiz_float() {
	int wiz_float_reset_frame;
	int tmp;

	if (kernel_anim[aa[3]].frame != local->wiz_float_frame) {
		local->wiz_float_frame = kernel_anim[aa[3]].frame;
		wiz_float_reset_frame = -1;

		switch (local->wiz_float_frame) {
		case 20:
			kernel_abort_animation(aa[0]);
			aa[1] = kernel_run_animation(kernel_name('k', 2), 0);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);

			local->anim_0_running = false;
			local->anim_1_running = true;
			local->king_float_action = FREEZE;
			player.commands_allowed = false;
			break;

		case 1:
			if (local->wiz_float_action == FREEZE) {
				wiz_float_reset_frame = 0;
			} else {
				local->wiz_float_action = FREEZE;
			}
			break;

		case 4:   /* end of wiz turning around to view king */
		case 27:  /* end of wiz raising king into air       */
			conv_run(CONV_31_WIZARD);
			conv_export_value(game.difficulty);
			break;

		case 5:   /* end of wiz floating     */
		case 6:   /* end of wiz floating     */
		case 7:   /* end of wiz floating     */
		case 15:  /* end of wiz talking      */
		case 28:  /* end of wiz raising king */
			switch (local->wiz_float_action) {
			case KILL_PID:
				kernel_abort_animation(aa[3]);
				aa[5] = kernel_run_animation(kernel_name('w', 3), 0);
				local->anim_3_running = false;
				local->anim_5_running = true;
				kernel_synch(KERNEL_ANIM, aa[5], KERNEL_NOW, 0);
				break;

			case WAITING_FOR_STONE:
				kernel_abort_animation(aa[3]);
				aa[6] = kernel_run_animation(kernel_name('w', 4), 0);
				local->anim_3_running = false;
				local->anim_6_running = true;
				kernel_synch(KERNEL_ANIM, aa[6], KERNEL_NOW, 0);
				break;

			case MORPH:
				wiz_float_reset_frame = 29;
				aa[4] = kernel_run_animation(kernel_name('w', 2), ROOM_614_DONE_ZAPPING);
				local->anim_4_running = true;
				local->wiz_zap_action = HAND_CHIN;
				kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, aa[3]);
				break;

			case FREEZE:
				wiz_float_reset_frame = imath_random(4, 6);
				break;

			case RAISE_KING:
				wiz_float_reset_frame = 15;
				local->wiz_float_action = FREEZE;
				break;

			case BACK_TO_CAMERA:
				kernel_abort_animation(aa[3]);
				aa[6] = kernel_run_animation(kernel_name('w', 4), 0);
				local->anim_3_running = false;
				local->anim_6_running = true;
				kernel_synch(KERNEL_ANIM, aa[6], KERNEL_NOW, 0);
				kernel_reset_animation(aa[6], 25);  /* make wiz turn back to camera */
				break;

			case TALK:
				tmp = imath_random(1, 2);
				if (tmp == 1) {
					wiz_float_reset_frame = 7;   /* new node */
				} else {
					wiz_float_reset_frame = 28;  /* new node */
				}
				break;
			}
			break;

		case 8:   /* end of wiz coming to talk */
		case 9:   /* end of wiz talking        */
		case 10:  /* end of wiz talking        */
		case 11:  /* end of wiz talking        */
			switch (local->wiz_float_action) {
			case TALK:
				wiz_float_reset_frame = imath_random(8, 10);
				++local->wiz_float_talk_count;
				if (local->wiz_float_talk_count > 15) {
					if (local->wiz_float_action != BACK_TO_CAMERA) {
						local->wiz_float_action = FREEZE;
					}
					local->wiz_float_talk_count = 0;
					wiz_float_reset_frame = 14; /* make wiz_float shut up */
				}
				break;

			default:
				wiz_float_reset_frame = 14;  /* new node */
				break;
			}
			break;

		case 29:  /* end of wiz coming to talk */
		case 12:  /* end of wiz talking        */
		case 13:  /* end of wiz talking        */
		case 14:  /* end of wiz talking        */
			switch (local->wiz_float_action) {
			case TALK:
				wiz_float_reset_frame = imath_random(11, 13);
				++local->wiz_float_talk_count;
				if (local->wiz_float_talk_count > 15) {
					if (local->wiz_float_action != BACK_TO_CAMERA) {
						local->wiz_float_action = FREEZE;
					}
					local->wiz_float_talk_count = 0;
					wiz_float_reset_frame = 14; /* make wiz_float shut up */
				}
				break;

			default:
				wiz_float_reset_frame = 14;  /* new node */
				break;
			}
			break;

		case 30:
			wiz_float_reset_frame = 29;
			break;
		}

		if (wiz_float_reset_frame >= 0) {
			kernel_reset_animation(aa[3], wiz_float_reset_frame);
			local->wiz_float_frame = wiz_float_reset_frame;
		}
	}
}

static void handle_animation_wiz_zap() {
	int wiz_zap_reset_frame;
	int count = 0;

	if (kernel_anim[aa[4]].frame != local->wiz_zap_frame) {
		local->wiz_zap_frame = kernel_anim[aa[4]].frame;
		wiz_zap_reset_frame = -1;

		switch (local->wiz_zap_frame) {
		case 9:
			kernel_reset_animation(aa[1], 16);
			break;

		case 34:
			if (local->wiz_zap_action == HAND_CHIN) {
				wiz_zap_reset_frame = 33;
			}
			break;

		case 40:
			kernel_abort_animation(aa[4]); /* wiz_zap    */
			kernel_abort_animation(aa[1]); /* king float */
			kernel_abort_animation(aa[3]); /* wiz_float  */
			local->anim_4_running = false;
			local->anim_1_running = false;

			aa[2] = kernel_run_animation(kernel_name('p', 1), 0);
			local->anim_2_running = true;
			local->pid_float_action = FREEZE;
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);

			aa[3] = kernel_run_animation(kernel_name('w', 1), 0);
			local->wiz_float_action = FREEZE;
			kernel_reset_animation(aa[3], 5);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);

			conv_run(CONV_32_WIZARD);
			conv_export_value(game.difficulty);
			if (global[object_imitated] == 10 || global[object_imitated] == 11 ||
				global[object_imitated] == 12) {
				conv_export_value(1);
			} else {
				conv_export_value(0);
			}
			conv_export_value(local->did_not_throw);
			if (player_has(red_powerstone))    ++count;
			if (player_has(blue_powerstone))   ++count;
			if (player_has(yellow_powerstone)) ++count;
			conv_export_value(count);
			break;
		}

		if (wiz_zap_reset_frame >= 0) {
			kernel_reset_animation(aa[4], wiz_zap_reset_frame);
			local->wiz_zap_frame = wiz_zap_reset_frame;
		}
	}
}

static void handle_animation_wiz_murder() {
	int wiz_murder_reset_frame;

	if (kernel_anim[aa[5]].frame != local->wiz_murder_frame) {
		local->wiz_murder_frame = kernel_anim[aa[5]].frame;
		wiz_murder_reset_frame = -1;

		switch (local->wiz_murder_frame) {
		case 9:
			local->pid_float_action = DIE;
			break;

		case 15:
			wiz_murder_reset_frame = 14;
			break;
		}

		if (wiz_murder_reset_frame >= 0) {
			kernel_reset_animation(aa[5], wiz_murder_reset_frame);
			local->wiz_murder_frame = wiz_murder_reset_frame;
		}
	}
}

static void handle_animation_wiz_ice() {
	int wiz_ice_reset_frame;

	if (kernel_anim[aa[6]].frame != local->wiz_ice_frame) {
		local->wiz_ice_frame = kernel_anim[aa[6]].frame;
		wiz_ice_reset_frame = -1;

		switch (local->wiz_ice_frame) {
		case 7:
			sound_play(N_CatchStone);
			seq[fx_stone_to_wiz] = kernel_seq_forward(ss[fx_stone_to_wiz], false, 9, 0, 0, 1);
			kernel_seq_depth(seq[fx_stone_to_wiz], 2);
			kernel_seq_range(seq[fx_stone_to_wiz], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_stone_to_wiz],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_614_STONE_TO_WIZ);

			++local->stone_taking_count;

			if (local->stone_taking_count < 3) {
				if (player_has(red_powerstone)) {
					inter_move_object(red_powerstone, NOWHERE);

				} else if (player_has(yellow_powerstone)) {
					inter_move_object(yellow_powerstone, NOWHERE);

				} else if (player_has(blue_powerstone)) {
					inter_move_object(blue_powerstone, NOWHERE);

				} else if (player_has(polystone)) {
					if (player_has(polystone)) {
						if (global[object_imitated] == 10 ||
							global[object_imitated] == 11 ||
							global[object_imitated] == 12) {
							inter_move_object(polystone, NOWHERE);
						}
					}
				}

			} else {
				if (player_has(polystone)) {
					if (global[object_imitated] == 10 ||
						global[object_imitated] == 11 ||
						global[object_imitated] == 12) {
						inter_move_object(polystone, NOWHERE);
					}

				} else if (player_has(red_powerstone)) {
					inter_move_object(red_powerstone, NOWHERE);

				} else if (player_has(yellow_powerstone)) {
					inter_move_object(yellow_powerstone, NOWHERE);

				} else if (player_has(blue_powerstone)) {
					inter_move_object(blue_powerstone, NOWHERE);
				}
			}
			break;

		case 8:  /* end of waiting for stone */
			if (local->wiz_ice_action == WAITING_FOR_STONE) {
				wiz_ice_reset_frame = 7;
			}
			break;

		case 20: /* end of crushing stone */
			kernel_abort_animation(aa[6]);
			local->anim_6_running = false;

			aa[3] = kernel_run_animation(kernel_name('w', 1), 0);
			local->wiz_float_action = FREEZE;
			local->anim_3_running = true;

			kernel_reset_animation(aa[3], 5);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);

			conv_run(CONV_32_WIZARD);
			conv_export_value(game.difficulty);
			if (global[object_imitated] == 10 || global[object_imitated] == 11 ||
				global[object_imitated] == 12) {
				conv_export_value(1);
			} else {
				conv_export_value(0);
			}
			conv_export_value(local->did_not_throw);
			break;

		case 29:
			player.commands_allowed = true;
			break;

		case 30: /* end of wiz floating with back to camera */
		case 31: /* end of wiz floating with back to camera */
		case 32: /* end of wiz floating with back to camera */
		case 33: /* end of wiz floating with back to camera */

			switch (local->wiz_ice_action) {
			case TURN_TO_PID:
				wiz_ice_reset_frame = 64;
				break;

			case BACK_TO_CAMERA:
				wiz_ice_reset_frame = imath_random(29, 32);
				break;
			}
			break;

		case 69:  /* end of turning to pid (after crushing stones) */
			switch (local->wiz_ice_action) {
			case KILL_PID:
				kernel_abort_animation(aa[6]);
				aa[5] = kernel_run_animation(kernel_name('w', 3), 0);
				local->anim_6_running = false;
				local->anim_5_running = true;
				kernel_synch(KERNEL_ANIM, aa[5], KERNEL_NOW, 0);
				break;

			case TURN_TO_PID:
				wiz_ice_reset_frame = 68;
				break;
			}
			break;
		}

		if (wiz_ice_reset_frame >= 0) {
			kernel_reset_animation(aa[6], wiz_ice_reset_frame);
			local->wiz_ice_frame = wiz_ice_reset_frame;
		}
	}
}

static void handle_animation_wiz_end() {
	int wiz_end_reset_frame;

	if (kernel_anim[aa[7]].frame != local->wiz_end_frame) {
		local->wiz_end_frame = kernel_anim[aa[7]].frame;
		wiz_end_reset_frame = -1;

		switch (local->wiz_end_frame) {
		case 10: /* just threw stone */
			inter_move_object(blue_powerstone, NOWHERE);
			break;

		case 16: /* make conv run just before explosion  */
			local->activate_timer = true;
			local->death_timer = LENGTH_OF_LIFE + 1;
			break;

		case 25: /* explosion starts */
			kernel_seq_delete(seq[fx_atom_1]);
			kernel_seq_delete(seq[fx_atom_2]);
			kernel_seq_delete(seq[fx_atom_3]);
			kernel_seq_delete(seq[fx_atom_4]);
			kernel_seq_delete(seq[fx_power_ball]);

			seq[fx_ball_fly_1] = kernel_seq_forward(ss[fx_ball_fly_1], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_ball_fly_1], 2);
			kernel_seq_range(seq[fx_ball_fly_1], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_ball_fly_1], KERNEL_TRIGGER_EXPIRE, 0, ROOM_614_STAMP_BALL);

			seq[fx_ball_fly_2] = kernel_seq_forward(ss[fx_ball_fly_2], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_ball_fly_2], 2);
			kernel_seq_range(seq[fx_ball_fly_2], KERNEL_FIRST, KERNEL_LAST);

			seq[fx_ball_fly_3] = kernel_seq_forward(ss[fx_ball_fly_3], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_ball_fly_3], 2);
			kernel_seq_range(seq[fx_ball_fly_3], KERNEL_FIRST, KERNEL_LAST);

			seq[fx_ball_fly_4] = kernel_seq_forward(ss[fx_ball_fly_4], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_ball_fly_4], 2);
			kernel_seq_range(seq[fx_ball_fly_4], KERNEL_FIRST, KERNEL_LAST);
			break;

		case 49: /* just when wizard is going to shatter */
			if (conv_control.running == CONV_32_WIZARD) {
				conv_abort();
			}
			break;

		case 73:
			local->anim_7_running = false;
			global[wizard_dead] = true;
			global[player_persona] = PLAYER_IS_PID;
			global[dragon_high_scene] = 0;
			global[wins_in_desert] = 0;
			global[player_score] += 10;
			new_room = 106;
			text_show(61437);
			/* go to cut scene */
			break;
		}

		if (wiz_end_reset_frame >= 0) {
			kernel_reset_animation(aa[7], wiz_end_reset_frame);
			local->wiz_end_frame = wiz_end_reset_frame;
		}
	}
}

static void room_614_daemon() {
	long dif;  /* for timer stuff */

	if (local->anim_0_running) {
		handle_animation_king_enter();
	}

	if (local->anim_1_running) {
		handle_animation_king_float();
	}

	if (local->anim_2_running) {
		handle_animation_pid_float();
	}

	if (local->anim_3_running) {
		handle_animation_wiz_float();
	}

	if (local->anim_4_running) {
		handle_animation_wiz_zap();
	}

	if (local->anim_5_running) {
		handle_animation_wiz_murder();
	}

	if (local->anim_6_running) {
		handle_animation_wiz_ice();
	}

	if (local->anim_7_running) {
		handle_animation_wiz_end();
	}

	if (kernel.trigger == ROOM_614_STONE_TO_WIZ) {
		local->wiz_ice_action = FREEZE;
	}

	if (kernel.trigger == ROOM_614_STAMP_BALL) {
		seq[fx_single_ball] = kernel_seq_stamp(ss[fx_single_ball], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_single_ball], 14);
		kernel_synch(KERNEL_SERIES, seq[fx_single_ball], KERNEL_SERIES, seq[fx_ball_fly_1]);
	}

	if (local->activate_timer) {
		if (local->death_timer == 0) {
			kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
		}

		dif = kernel.clock - local->clock;
		if ((dif >= 0) && (dif <= 4)) {
			local->death_timer += dif;
		} else {
			local->death_timer += 1;
		}
		local->clock = kernel.clock;

		if (local->death_timer >= LENGTH_OF_LIFE) {
			local->wiz_ice_action = TURN_TO_PID;
			player.commands_allowed = false;
			/* turn back to kill pid or just kill him */
			local->activate_timer = false;

			kernel_init_dialog();
			kernel_set_interface_mode(INTER_LIMITED_SENTENCES);
			conv_run(CONV_32_WIZARD);
			conv_export_value(game.difficulty);
			if (global[object_imitated] == 10 || global[object_imitated] == 11 ||
				global[object_imitated] == 12) {
				conv_export_value(1);
			} else {
				conv_export_value(0);
			}
			conv_export_value(local->did_not_throw);
		}
	}

	if (global[object_imitated] == 10 ||   /* red    */
		global[object_imitated] == 11 ||   /* yellow */
		global[object_imitated] == 12) {   /* blue   */

		if (local->activate_timer && !local->doofus) {
			kernel_init_dialog();
			kernel_set_interface_mode(INTER_LIMITED_SENTENCES);
			local->death_timer = LENGTH_OF_LIFE + 1;
		}
	}
}

static void process_conv_31() {
	int you_trig_flag = false;

	switch (player_verb) {
	case conv031_hello_only:
	case conv031_whoare_only:
		if (!kernel.trigger) {
			conv_me_trigger(ROOM_614_ME_TALK);
		}
		break;

	case conv031_wellwell_b_b:
		*conv_my_next_start = conv031_wellwell;
		local->wiz_float_action = RAISE_KING;
		you_trig_flag = true;
		conv_abort();
		break;

	case conv031_exit_b_b:
		local->wiz_float_action = MORPH;
		you_trig_flag = true;
		conv_abort();
		break;
	}

	if (kernel.trigger == ROOM_614_ME_TALK) {
		local->king_enter_action = TALK;
	}

	if (kernel.trigger == ROOM_614_YOU_TALK) {
		local->wiz_float_action = TALK;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_614_YOU_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */


	local->king_enter_talk_count = 0;
	local->wiz_float_talk_count = 0;
}

static void process_conv_32() {
	int you_trig_flag = false;

	switch (player_verb) {
	case conv032_pid_dies_b_b:
		conv_abort();
		local->wiz_float_action = KILL_PID;
		local->wiz_ice_action = KILL_PID;
		break;

	case conv032_pid_death_b_b:
		conv_abort();
		local->wiz_float_action = KILL_PID;
		break;

	case 7:
		*conv_my_next_start = 8;
		conv_abort();
		local->wiz_ice_action = WAITING_FOR_STONE;
		local->wiz_float_action = WAITING_FOR_STONE;
		break;

	case 10:
		*conv_my_next_start = 11;
		conv_abort();
		local->wiz_ice_action = WAITING_FOR_STONE;
		local->wiz_float_action = WAITING_FOR_STONE;
		break;

	case 17:
		*conv_my_next_start = 18;
		conv_abort();
		local->activate_timer = true;
		player.commands_allowed = true;
		break;

	case 22:
		*conv_my_next_start = 23;
		conv_abort();
		local->wiz_ice_action = WAITING_FOR_STONE;
		local->wiz_float_action = WAITING_FOR_STONE;
		break;

	case conv032_pain_b_b:
		*conv_my_next_start = conv032_pain;
		conv_abort();
		local->activate_timer = true;
		player.commands_allowed = true;
		break;
	}

	if (kernel.trigger == ROOM_614_ME_TALK) {
		local->king_enter_action = TALK;
	}

	if (kernel.trigger == ROOM_614_YOU_TALK) {
		local->wiz_float_action = TALK;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_614_YOU_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */

	local->king_enter_talk_count = 0;
	local->wiz_float_talk_count = 0;
}

static void room_614_pre_parser() {
	if (global[player_persona] == PLAYER_IS_KING) {
		player.need_to_walk = false;
	}

	if (player_said_1(doorway_to_corridor) && player.need_to_walk) {
		if (!player_said_1(walk_through)) {
			player.need_to_walk = false;
		}
	}

	if (player_said_2(walk_through, doorway_to_corridor)) {
		if (global[player_persona] == PLAYER_IS_PID) {
			if (object_is_here(medicine_bundle) ||
				object_is_here(black_sphere) ||
				object_is_here(crystal_ball) ||
				object_is_here(map)) {
				text_show(61459);
				player_cancel_command();
			}
		}
	}
}

static void room_614_parser() {
	if (conv_control.running == CONV_31_WIZARD) {
		process_conv_31();
		goto handled;
	}

	if (conv_control.running == CONV_32_WIZARD) {
		process_conv_32();
		goto handled;
	}

	if (kernel.trigger == ROOM_614_DONE_ZAPPING) {
		local->anim_4_running = false;
		goto handled;
	}

	if (player_said_2(take, crystal_ball)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			text_show(61433);
			goto handled;

		} else switch (kernel.trigger) {
		case 0:
			if (!player_has(crystal_ball)) {
				player.commands_allowed = false;
				player.walker_visible = false;
				seq[fx_take] = kernel_seq_pingpong(ss[fx_take], false, 7, 0, 0, 2);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 2, 1);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_EXPIRE, 0, 2);
				kernel_seq_depth(seq[fx_take], 3);
				kernel_seq_range(seq[fx_take], KERNEL_FIRST, 3);
				kernel_seq_player(seq[fx_take], true);
				goto handled;
			}
			break;

		case 1:
			if (local->prevent) {
				kernel_seq_delete(seq[fx_ball]);
				kernel_flip_hotspot(words_crystal_ball, false);
				++global[player_score];
				sound_play(N_TakeObjectSnd);
				inter_give_to_player(crystal_ball);
				object_examine(crystal_ball, 61458, 0);
			}
			local->prevent = true;
			goto handled;
			break;

		case 2:
			player.walker_visible = true;
			player.commands_allowed = true;
			local->prevent = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take]);
			goto handled;
			break;
		}
	}

	if (player_said_2(take, black_sphere)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			text_show(61433);
			goto handled;

		} else switch (kernel.trigger) {
		case 0:
			if (!player_has(black_sphere)) {
				player.commands_allowed = false;
				player.walker_visible = false;
				seq[fx_take] = kernel_seq_pingpong(ss[fx_take], false, 7, 0, 0, 2);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 4, 1);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_EXPIRE, 0, 2);
				kernel_seq_depth(seq[fx_take], 3);
				kernel_seq_range(seq[fx_take], KERNEL_FIRST, 5);
				kernel_seq_player(seq[fx_take], true);
				goto handled;
			}
			break;

		case 1:
			if (local->prevent) {
				kernel_seq_delete(seq[fx_single_ball]);
				kernel_flip_hotspot(words_black_sphere, false);
				++global[player_score];
				sound_play(N_TakeObjectSnd);
				inter_give_to_player(black_sphere);
				if (game.difficulty == HARD_MODE) {
					object_examine(black_sphere, 61453, 0);
				} else {
					object_examine(black_sphere, 61454, 0);
				}
			}
			local->prevent = true;
			goto handled;
			break;

		case 2:
			player.walker_visible = true;
			player.commands_allowed = true;
			local->prevent = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take]);
			goto handled;
			break;
		}
	}

	if (player_said_2(take, map)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			text_show(61433);
			goto handled;

		} else switch (kernel.trigger) {
		case 0:
			if (!player_has(map)) {
				player.commands_allowed = false;
				player.walker_visible = false;
				seq[fx_take_high] = kernel_seq_pingpong(ss[fx_take_high], true, 7, 0, 0, 2);
				kernel_seq_trigger(seq[fx_take_high], KERNEL_TRIGGER_SPRITE, 3, 1);
				kernel_seq_trigger(seq[fx_take_high], KERNEL_TRIGGER_EXPIRE, 0, 2);
				kernel_seq_depth(seq[fx_take_high], 3);
				kernel_seq_range(seq[fx_take_high], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_player(seq[fx_take_high], true);
				goto handled;
			}
			break;

		case 1:
			if (local->prevent) {
				kernel_seq_delete(seq[fx_map]);
				kernel_flip_hotspot(words_map, false);
				++global[player_score];
				sound_play(N_TakeObjectSnd);
				inter_give_to_player(map);
				object_examine(map, 61451, 0);
			}
			local->prevent = true;
			goto handled;
			break;

		case 2:
			player.walker_visible = true;
			player.commands_allowed = true;
			local->prevent = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_high]);
			goto handled;
			break;
		}
	}

	if (player_said_2(take, spirit_bundle)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			text_show(61433);
			goto handled;

		} else {
			switch (kernel.trigger) {
			case 0:
				if (!player_has(medicine_bundle)) {
					player.commands_allowed = false;
					player.walker_visible = false;
					seq[fx_take] = kernel_seq_pingpong(ss[fx_take], false, 7, 0, 0, 2);
					kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 2, 1);
					kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_EXPIRE, 0, 2);
					kernel_seq_depth(seq[fx_take], 3);
					kernel_seq_range(seq[fx_take], KERNEL_FIRST, 3);
					kernel_seq_player(seq[fx_take], true);
					goto handled;
				}
				break;

			case 1:
				if (local->prevent) {
					kernel_seq_delete(seq[fx_bundle]);
					kernel_flip_hotspot(words_spirit_bundle, false);
					++global[player_score];
					sound_play(N_TakeObjectSnd);
					inter_give_to_player(medicine_bundle);
					object_examine(medicine_bundle, 61456, 0);
					global[had_spirit_bundle] = true;
				}
				local->prevent = true;
				goto handled;
				break;

			case 2:
				player.walker_visible = true;
				player.commands_allowed = true;
				local->prevent = false;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take]);
				goto handled;
				break;
			}
		}
	}

	if (player_said_2(walk_through, doorway_to_corridor)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			text_show(61413);
		} else {
			text_show(61460);
			new_room = 601;
		}
		goto handled;
	}

	if (player.look_around) {
		if (local->anim_2_running) {
			if (game.difficulty == HARD_MODE) {
				text_show(61401);
			} else {
				text_show(61402);
			}
		} else {
			text_show(61438);
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(chest)) {
			if (local->anim_2_running) {
				text_show(61403);
			} else {
				if (object_is_here(medicine_bundle) && !object_is_here(crystal_ball)) {
					text_show(61466);
				} else if (!object_is_here(medicine_bundle) && object_is_here(crystal_ball)) {
					text_show(61466);
				} else if (!object_is_here(medicine_bundle) && !object_is_here(crystal_ball)) {
					text_show(61467);
				} else {
					text_show(61439);
				}
			}
			goto handled;
		}

		if (player_said_1(telescope)) {
			if (local->anim_2_running) {
				text_show(61404);
			} else {
				text_show(61440);
			}
			goto handled;
		}

		if (player_said_1(wall)) {
			if (local->anim_2_running) {
				text_show(61405);
			} else {
				text_show(61441);
			}
			goto handled;
		}

		if (player_said_1(dragon_sculpture)) {
			if (local->anim_2_running) {
				text_show(61406);
			} else {
				text_show(61442);
			}
			goto handled;
		}

		if (player_said_1(circle_of_spheres)) {
			if (local->anim_2_running) {
				if (game.difficulty == HARD_MODE) {
					text_show(61407);
				} else {
					text_show(61408);
				}
			} else if (player_has(black_sphere)) {
				text_show(61463);
			} else {
				text_show(61443);
			}
			goto handled;
		}

		if (player_said_1(scroll)) {
			if (local->anim_2_running) {
				text_show(61409);
			} else {
				if (inter_point_x > 295 && inter_point_y < 53) {
					text_show(61474);
				} else {
					text_show(61444);
				}
			}
			goto handled;
		}

		if (player_said_1(window)) {
			if (local->anim_2_running) {
				text_show(61410);
			} else {
				text_show(61445);
			}
			goto handled;
		}

		if (player_said_1(bookshelf)) {
			if (local->anim_2_running) {
				text_show(61411);
			} else {
				text_show(61446);
			}
			goto handled;
		}

		if (player_said_1(stone_bed)) {
			if (local->anim_2_running) {
				text_show(61412);
			} else {
				text_show(61447);
			}
			goto handled;
		}

		if (player_said_1(doorway_to_corridor)) {
			if (local->anim_2_running) {
				text_show(61413);
			} else {
				text_show(61448);
			}
			goto handled;
		}

		if (player_said_1(strange_map)) {
			if (local->anim_2_running) {
				if (game.difficulty == HARD_MODE) {
					text_show(61414);
				} else {
					text_show(61415);
				}
			} else {
				text_show(61461);
			}
			goto handled;
		}

		if (player_said_1(floor)) {
			text_show(61416);
			goto handled;
		}

		if (player_said_1(map) && object_is_here(map)) {
			text_show(61450);
			goto handled;
		}

		if (player_said_1(black_sphere) && object_is_here(black_sphere)) {
			text_show(61452);
			goto handled;
		}

		if (player_said_1(spirit_bundle) && object_is_here(medicine_bundle)) {
			text_show(61455);
			goto handled;
		}

		if (player_said_1(crystal_ball) && object_is_here(crystal_ball)) {
			text_show(61457);
			goto handled;
		}

		if (player_said_1(Sanwe)) {
			if (local->anim_6_running) {
				text_show(61465);
			} else {
				text_show(61464);
			}
			goto handled;
		}
	}

	if (player_said_2(invoke, signet_ring)) {
		if (global[wizard_dead]) {
			if (object_is_here(medicine_bundle) ||
				object_is_here(black_sphere) ||
				object_is_here(crystal_ball) ||
				object_is_here(map)) {
				text_show(61459);
				goto handled;
			}

		} else if (local->anim_2_running) {
			text_show(61418);
			goto handled;
		}
	}

	if (player_said_2(make_noise, birdcall)) {
		if (local->anim_6_running) {
			text_show(61431);
			goto handled;

		} else if (local->anim_2_running) {
			text_show(61419);
			goto handled;
		}
	}

	if (player_said_2(sword, attack) ||
		player_said_2(sword, carve_up) ||
		player_said_2(sword, thrust)) {

		if (local->anim_6_running) {
			text_show(61432);
			goto handled;

		} else if (local->anim_2_running) {
			text_show(61420);
			goto handled;
		}
	}

	if (player_said_2(drink_from, goblet) ||
		player_said_2(eat, fruit) ||
		player_said_2(gnaw, bone)) {
		if (local->anim_2_running) {
			text_show(61421);
			goto handled;
		}
	}

	if (player_said_2(invoke, amulet)) {
		text_show(61424);
		goto handled;
	}

	if (player_said_2(wave, torch)) {
		text_show(61425);
		goto handled;
	}

	if (player_said_2(pour, flask_full_of_acid)) {
		if (local->anim_2_running) {
			text_show(61426);
			goto handled;
		}
	}

	if (player_said_2(drink, flask_of_acid)) {
		if (local->anim_2_running) {
			text_show(61427);
			goto handled;
		}
	}

	if (player_said_1(polish)) {
		if (player_said_1(blue_powerstone) ||
			player_said_1(red_powerstone) ||
			player_said_1(yellow_powerstone)) {
			text_show(61423);
			goto handled;
		}
	}

	if (player_said_2(throw, circle_of_spheres) ||
		player_said_2(give, circle_of_spheres) ||
		player_said_2(put, circle_of_spheres)) {

		if (player_said_1(blue_stone)) {
			if (local->wiz_ice_action == BACK_TO_CAMERA) {
				local->pid_float_action = ICE_UP;
				local->did_not_throw = false;
				player.commands_allowed = false;
				local->activate_timer = false;
				kernel_init_dialog();
				kernel_set_interface_mode(INTER_LIMITED_SENTENCES);
				goto handled;

			} else if (game.difficulty == EASY_MODE) {
				text_show(61472);
				goto handled;
			}

		} else if (player_has(object_named(player_main_noun))) {
			text_show(61436);
			goto handled;
		}
	}

	if (player_said_2(throw, blue_stone) || player_said_2(put, blue_stone) ||
		player_said_2(give, blue_stone)) {
		if (!player_said_1(circle_of_spheres) && !player_said_1(Sanwe)) {
			if (local->wiz_ice_action == BACK_TO_CAMERA) {
				text_show(61470);
				goto handled;
			}

		} else if (player_said_1(Sanwe)) {
			text_show(61462);
			goto handled;
		}
	}

	if (player_said_1(give) || player_said_1(put) || player_said_1(throw)) {
		if (player_said_1(Sanwe)) {
			text_show(61435);
			goto handled;
		}
	}

	if (player_said_2(talk_to, Sanwe)) {
		text_show(61434);
		goto handled;
	}

	if (player_said_1(take) ||
		player_said_1(open) ||
		player_said_1(close) ||
		player_said_1(push) ||
		player_said_1(pull)) {

		if (local->anim_6_running) {
			text_show(61433);
			goto handled;
		}
	}

	if (player_said_1(walk_across) ||
		player_said_1(walk_to)) {

		if (local->wiz_ice_action == BACK_TO_CAMERA) {
			text_show(61473);
			goto handled;

		} else if (global[player_persona] == PLAYER_IS_KING) {
			text_show(61417);
			goto handled;
		}
	}

	if (player_said_1(throw) && global[player_persona] == PLAYER_IS_KING) {
		if (player_has(object_named(player_main_noun)) ||
			player_has(object_named(player_second_noun))) {
			text_show(61417);
			goto handled;
		}
	}

	if (player_said_1(take) ||
		player_said_1(give) ||
		player_said_1(talk_to) ||
		player_said_1(put) ||
		player_said_1(open) ||
		player_said_1(close) ||
		player_said_1(push) ||
		player_said_1(pull)) {

		if (local->anim_2_running) {
			text_show(61417);
			goto handled;
		}
	}

	if (player_said_2(drink, goblet)) {
		if (local->anim_2_running) {
			text_show(61421);
			goto handled;
		}
	}

	if (player_said_2(polish, blue_stone)) {
		if (local->anim_2_running) {
			text_show(61423);
			goto handled;
		}
	}

	if (player_said_1(pour_contents_of)) {
		if (local->anim_2_running) {
			text_show(61426);
			goto handled;
		}
	}

	if (player_said_2(drink, flask_full_of_acid)) {
		if (local->anim_2_running) {
			text_show(61427);
			goto handled;
		}
	}

	if (player_said_2(tie, rope)) {
		if (local->anim_2_running) {
			text_show(61428);
			goto handled;
		}
	}

	if (player_said_1(vortex_stone)) {
		if (local->anim_2_running) {
			text_show(61429);
			goto handled;
		}
	}

	if (player_said_2(open, chest)) {
		if (!local->anim_2_running) {
			text_show(61468);
			goto handled;
		}
	}

	if (player_said_2(close, chest)) {
		if (!local->anim_2_running) {
			text_show(61469);
			goto handled;
		}
	}

	if (player_said_2(speak_words_on, parchment)) {
		if (local->anim_2_running) {
			text_show(61430);
			goto handled;
		}
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_614_synchronize(Common::Serializer &s) {
	for (int i = 0; i < 19; i++) s.syncAsSint16LE(local->sprite[i]);
	for (int i = 0; i < 19; i++) s.syncAsSint16LE(local->sequence[i]);
	for (int i = 0; i < 8; i++)  s.syncAsSint16LE(local->animation[i]);
	s.syncAsSint16LE(local->prevent);
	s.syncAsSint16LE(local->doofus);
	s.syncAsSint16LE(local->stone_taking_count);
	s.syncAsSint16LE(local->king_enter_frame);
	s.syncAsSint16LE(local->king_enter_action);
	s.syncAsSint16LE(local->king_enter_talk_count);
	s.syncAsSint16LE(local->anim_0_running);
	s.syncAsSint16LE(local->king_float_frame);
	s.syncAsSint16LE(local->king_float_action);
	s.syncAsSint16LE(local->king_float_talk_count);
	s.syncAsSint16LE(local->anim_1_running);
	s.syncAsSint16LE(local->pid_float_frame);
	s.syncAsSint16LE(local->pid_float_action);
	s.syncAsSint16LE(local->pid_float_talk_count);
	s.syncAsSint16LE(local->anim_2_running);
	s.syncAsSint16LE(local->wiz_float_frame);
	s.syncAsSint16LE(local->wiz_float_action);
	s.syncAsSint16LE(local->wiz_float_talk_count);
	s.syncAsSint16LE(local->anim_3_running);
	s.syncAsSint16LE(local->wiz_zap_frame);
	s.syncAsSint16LE(local->wiz_zap_action);
	s.syncAsSint16LE(local->anim_4_running);
	s.syncAsSint16LE(local->wiz_murder_frame);
	s.syncAsSint16LE(local->wiz_murder_action);
	s.syncAsSint16LE(local->anim_5_running);
	s.syncAsSint16LE(local->wiz_ice_frame);
	s.syncAsSint16LE(local->wiz_ice_action);
	s.syncAsSint16LE(local->anim_6_running);
	s.syncAsSint16LE(local->wiz_end_frame);
	s.syncAsSint16LE(local->wiz_end_action);
	s.syncAsSint16LE(local->anim_7_running);
	s.syncAsSint32LE(local->clock);
	s.syncAsSint32LE(local->death_timer);
	s.syncAsSint16LE(local->activate_timer);
	s.syncAsSint16LE(local->did_not_throw);
}

void room_614_preload() {
	room_init_code_pointer = room_614_init;
	room_pre_parser_code_pointer = room_614_pre_parser;
	room_parser_code_pointer = room_614_parser;
	room_daemon_code_pointer = room_614_daemon;

	if (global[wizard_dead]) {
		global[no_load_walker] = false;
	} else {
		global[no_load_walker] = true;
	}

	section_6_walker();
	section_6_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
