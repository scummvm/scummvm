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

#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section3.h"
#include "mads/madsv2/dragonsphere/rooms/room303.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[10];

	int16 scroll_left_base;
	int16 dyn_scroll_left;

	int16 scroll_right_base;
	int16 dyn_scroll_right;

	int16 scroll_mid_base;
	int16 dyn_scroll_mid;

	int16 butter_frame;
	int16 butter_talk_count;
	int16 anim_0_running;

	int16 bk_frame;
	int16 bk_action;
	int16 bk_talk_count;
	int16 anim_1_running;

	int16 frog_1_frame;
	int16 frog_1_action;
	int16 frog_1_talk_count;
	int16 anim_3_running;

	int16 frog_2_frame;
	int16 frog_2_action;
	int16 frog_2_talk_count;
	int16 anim_4_running;

	int16 frog_3_frame;
	int16 frog_3_action;
	int16 frog_3_talk_count;
	int16 anim_5_running;

	int16 frog_4_frame;
	int16 frog_4_action;
	int16 frog_4_talk_count;
	int16 anim_6_running;

	int16 frog_5_frame;
	int16 frog_5_action;
	int16 frog_5_talk_count;
	int16 anim_7_running;

	int16 fli_frame;
	int16 fli_action;
	int16 fli_talk_count;
	int16 anim_8_running;

	int16 frog_6_frame;
	int16 frog_6_action;
	int16 frog_6_talk_count;
	int16 anim_9_running;

	int16 death_frame;
	int16 anim_10_running;

	int16 prevent;
	int16 prevent_2;
	int16 pid_take;

	int16 wing_type;
	int16 dont_die;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_scroll               0
#define fx_scroll_right         1
#define fx_scroll_left          2
#define fx_scroll_mid           3
#define fx_crown                4
#define fx_king_take_stone      5
#define fx_wing                 6
#define fx_butt_king            7
#define fx_flower               8
#define fx_take                 9
#define fx_king_gone            10
#define fx_king_gone_2          11

#define ROOM_303_HAND           60
#define ROOM_303_HALT           65
#define ROOM_303_ME_TALK        70
#define ROOM_303_YOU_TALK       72
#define ROOM_303_DEATH          78
#define ROOM_303_WING           83

#define camera_ratio_1          1
#define camera_ratio_2          2

#define PLAYER_X_FROM_302       493
#define PLAYER_Y_FROM_302       127
#define WALK_TO_X_FROM_302      465
#define WALK_TO_Y_FROM_302      127

#define CONV_57_KING            57

#define FREEZE                  0
#define TALK                    1
#define POINT                   2
#define GIVE                    3
#define WELCOME                 4
#define FREEZE_ABORT            5
#define EAT                     6
#define KILL                    7

#define BK_X                    449
#define BK_Y                    122

#define DUL_X                   61
#define DUL_Y                   110
#define DLR_X                   101
#define DLR_Y                   136

#define DEATH_X                 82
#define DEATH_Y                 134

#define WING_BEFORE             0
#define WING_AFTER              1
#define WING_CHANGING           2

#define AFTER_FLI_X             90
#define AFTER_FLI_Y             139

#define OPEN_X                  83
#define OPEN_Y                  131

#define PID_FLOWER_X            208
#define PID_FLOWER_Y            140

#define KING_FLOWER_X           199
#define KING_FLOWER_Y           135

#define MUSHROOM_X              388
#define MUSHROOM_Y              66


static void set_scroll_left_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_scroll_left] >= 0) {
		kernel_seq_delete(seq[fx_scroll_left]);
	}

	difference = center - local->scroll_left_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace   = sgn_in(displace, direction);

	x          = local->scroll_left_base + displace - 1;
	xs         = series_list[ss[fx_scroll_left]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_scroll_left] = -1;

	} else {

		seq[fx_scroll_left] = kernel_seq_stamp(ss[fx_scroll], false, 2);
		kernel_seq_loc(seq[fx_scroll_left], x, 155);
		kernel_seq_depth(seq[fx_scroll_left], 1);
	}
}

static void set_scroll_right_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_scroll_right] >= 0) {
		kernel_seq_delete(seq[fx_scroll_right]);
	}

	difference = center - local->scroll_right_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace   = sgn_in(displace, direction);

	x          = local->scroll_right_base + displace - 1;
	xs         = series_list[ss[fx_scroll_right]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_scroll_right] = -1;

	} else {

		seq[fx_scroll_right] = kernel_seq_stamp(ss[fx_scroll], false, 1);
		kernel_seq_loc(seq[fx_scroll_right], x, 155);
		kernel_seq_depth(seq[fx_scroll_right], 1);
	}
}

static void set_scroll_mid_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_scroll_mid] >= 0) {
		kernel_seq_delete(seq[fx_scroll_mid]);
	}

	difference = center - local->scroll_mid_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace   = sgn_in(displace, direction);

	x          = local->scroll_mid_base + displace - 1;
	xs         = series_list[ss[fx_scroll_mid]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_scroll_mid] = -1;

	} else {

		seq[fx_scroll_mid] = kernel_seq_stamp(ss[fx_scroll], false, 3);
		kernel_seq_loc(seq[fx_scroll_mid], x, 155);
		kernel_seq_depth(seq[fx_scroll_mid], 1);
	}
}

static void room_303_init() {
	if (!player.been_here_before) {
		++global[dragon_high_scene];
		global[player_score] += 8;
	}

	local->butter_talk_count = 0;
	local->pid_take          = false;
	local->prevent_2         = false;
	local->prevent           = false;
	local->wing_type         = WING_BEFORE;
	local->dont_die          = false;
	local->anim_10_running   = false;

	local->frog_1_talk_count = 0;
	local->frog_2_talk_count = 0;
	local->frog_3_talk_count = 0;
	local->frog_4_talk_count = 0;
	local->frog_5_talk_count = 0;
	local->frog_6_talk_count = 0;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_3_running = false;
		local->anim_4_running = false;
		local->anim_5_running = false;
		local->anim_6_running = false;
		local->anim_7_running = false;
		local->anim_8_running = false;
		local->anim_9_running = false;
	}

	if (global[player_persona] == PLAYER_IS_PID) {
		ss[fx_king_gone_2]  = kernel_load_series(kernel_name('w', 9), false);
		seq[fx_king_gone_2] = kernel_seq_stamp(ss[fx_king_gone_2], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_king_gone_2], 1);

		kernel_flip_hotspot(words_Butterfly_King, false);
	}

	if (global[player_persona] == PLAYER_IS_KING) {
		kernel_flip_hotspot_loc(words_mushroom, false, MUSHROOM_X, MUSHROOM_Y);
		conv_get(CONV_57_KING);
		ss[fx_king_take_stone]  = kernel_load_series("*KGRH_9", false);
	}

	if (object_is_here(crystal_flower)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			ss[fx_take] = kernel_load_series("*KGRM_3", false);
		} else {
			ss[fx_take] = kernel_load_series("*PDRL_9", false);
		}
		ss[fx_flower]  = kernel_load_series(kernel_name('p', 1), false);
		seq[fx_flower] = kernel_seq_stamp(ss[fx_flower], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_flower], 3);

	} else {
		kernel_flip_hotspot(words_crystal_flower, false);
	}

	ss[fx_scroll]    = kernel_load_series("*RM303FX", false);

	if (global[player_persona] == PLAYER_IS_KING) {
		ss[fx_wing]      = kernel_load_series(kernel_name('w', 0), false);
		ss[fx_butt_king] = kernel_load_series(kernel_name('e', 3), false);
		seq[fx_wing] = kernel_seq_stamp(ss[fx_wing], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_wing], 10);
	}

	seq[fx_scroll_left]     = -1;
	local->dyn_scroll_left  = -1;
	local->scroll_left_base = 69;

	seq[fx_scroll_right]     = -1;
	local->dyn_scroll_right  = -1;
	local->scroll_right_base = 418;

	seq[fx_scroll_mid]     = -1;
	local->dyn_scroll_mid  = -1;
	local->scroll_mid_base = 255;

	if (object_is_here(key_crown)) {
		ss[fx_crown]  = kernel_load_series(kernel_name('p', 0), false);
		seq[fx_crown] = kernel_seq_stamp(ss[fx_crown], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_crown], 14);
	} else {
		kernel_flip_hotspot(words_crown, false);
	}

	aa[0]                 = kernel_run_animation(kernel_name('b', 1), 0);
	local->anim_0_running = true;

	aa[4]                 = kernel_run_animation(kernel_name('f', 4), 0);
	local->anim_4_running = true;
	local->frog_2_action  = FREEZE;

	if (conv_restore_running == CONV_57_KING) {
		aa[1]                   = kernel_run_animation(kernel_name('u', 1), 0);
		local->anim_1_running   = true;
		local->bk_action        = FREEZE;
		player.commands_allowed = false;
		conv_run(CONV_57_KING);

	} else {
		aa[9]                 = kernel_run_animation(kernel_name('f', 6), 0);
		local->anim_9_running = true;
		local->frog_6_action  = FREEZE;

		aa[5]                 = kernel_run_animation(kernel_name('f', 5), 0);
		local->anim_5_running = true;
		local->frog_3_action  = FREEZE;

		aa[6]                 = kernel_run_animation(kernel_name('f', 3), 0);
		local->anim_6_running = true;
		local->frog_4_action  = FREEZE;

		aa[7]                 = kernel_run_animation(kernel_name('f', 2), 0);
		local->anim_7_running = true;
		local->frog_5_action  = FREEZE;

		aa[3]                 = kernel_run_animation(kernel_name('f', 1), 0);
		local->anim_3_running = true;
		local->frog_1_action  = FREEZE;

		if (global[player_persona] == PLAYER_IS_KING) {
			seq[fx_butt_king] = kernel_seq_stamp(ss[fx_butt_king], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_butt_king], 1);
		}
	}


	if ((previous_room == 302) || (previous_room != KERNEL_RESTORING_GAME)) {
		camera_jump_to(160, 0);
		player_first_walk(PLAYER_X_FROM_302, PLAYER_Y_FROM_302, FACING_WEST,
		                  WALK_TO_X_FROM_302, WALK_TO_Y_FROM_302, FACING_WEST, true);
	}

	set_scroll_left_position();
	set_scroll_right_position();
	set_scroll_mid_position();

	section_3_music();
}

static void handle_anim_butter() {
	int butter_reset_frame;

	if (kernel_anim[aa[0]].frame != local->butter_frame) {
		local->butter_frame = kernel_anim[aa[0]].frame;
		butter_reset_frame = -1;

		switch (local->butter_frame) {
		case 1:
		case 2:
		case 38:
			if (local->butter_frame > 2) {
				local->butter_frame = 1;
			}

			++local->butter_talk_count;

			if (local->butter_talk_count > imath_random(5, 10)) {
				if (imath_random(1, 11) == 1) {
					butter_reset_frame = 2;

				} else if (imath_random(1, 2) == 1) {
					butter_reset_frame = 0;

				} else {
					butter_reset_frame = 1;
				}

				local->butter_talk_count = 0;

			} else {
				butter_reset_frame = local->butter_frame - 1;
			}
			break;

		case 19:
		case 20:
		case 21:
		case 18:
			if (local->butter_frame < 19) {
				local->butter_frame = 20;
			}

			++local->butter_talk_count;

			if (local->butter_talk_count > imath_random(5, 10)) {
				if (imath_random(1, 11) == 1) {
					butter_reset_frame = 21;

				} else if (local->butter_frame == 19) {
					butter_reset_frame = imath_random(18, 19);
				} else if (local->butter_frame == 20) {
					butter_reset_frame = imath_random(18, 20);
				} else if (local->butter_frame == 21) {
					butter_reset_frame = imath_random(19, 20);
				}
				local->butter_talk_count = 0;

			} else {
				butter_reset_frame = local->butter_frame - 1;
			}
			break;
		}

		if (butter_reset_frame >= 0) {
			kernel_reset_animation(aa[0], butter_reset_frame);
			local->butter_frame = butter_reset_frame;
		}
	}
}

static void handle_anim_bk() {
	int bk_reset_frame;
	int it;

	if (kernel_anim[aa[1]].frame != local->bk_frame) {
		local->bk_frame = kernel_anim[aa[1]].frame;
		bk_reset_frame = -1;

	switch (local->bk_frame) {
		case 1:
		case 16:
		case 76:
		case 113:
		case 33:
		case 51:
		case 95:
			if (local->bk_frame == 76) {
				kernel_timing_trigger(1, 2);
			}

			switch (local->bk_action) {
			case TALK:
				it = imath_random(1, 4);
				switch (it) {
				case 1:
					bk_reset_frame = 1;
					break;

				case 2:
					if (local->bk_action != GIVE) {
						local->bk_action = FREEZE;
					}
					bk_reset_frame   = 16;
					break;

				case 3:
					if (local->bk_action != GIVE) {
						local->bk_action = FREEZE;
					}
					bk_reset_frame   = 33;
					break;

				case 4:
					if (local->bk_action != GIVE) {
						local->bk_action = FREEZE;
					}
					bk_reset_frame   = 76;
					break;
				}
				break;

			case POINT:
				bk_reset_frame   = 95;
				local->bk_action = FREEZE;
				break;

			case FREEZE:
				bk_reset_frame = 0;
				break;

			case FREEZE_ABORT:
				kernel_abort_animation(aa[1]);
				local->anim_1_running = false;
				seq[fx_butt_king]     = kernel_seq_stamp(ss[fx_butt_king], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_butt_king], 1);
				kernel_synch(KERNEL_SERIES, seq[fx_butt_king], KERNEL_NOW, 0);
				player.commands_allowed = true;

				aa[9]                 = kernel_run_animation(kernel_name('f', 6), 0);
				local->anim_9_running = true;
				local->frog_6_action  = FREEZE;

				aa[5]                 = kernel_run_animation(kernel_name('f', 5), 0);
				local->anim_5_running = true;
				local->frog_3_action  = FREEZE;

				aa[6]                 = kernel_run_animation(kernel_name('f', 3), 0);
				local->anim_6_running = true;
				local->frog_4_action  = FREEZE;

				aa[7]                 = kernel_run_animation(kernel_name('f', 2), 0);
				local->anim_7_running = true;
				local->frog_5_action  = FREEZE;

				aa[3]                 = kernel_run_animation(kernel_name('f', 1), 0);
				local->anim_3_running = true;
				local->frog_1_action  = FREEZE;
				break;

			case WELCOME:
				bk_reset_frame   = 4;
				local->bk_action = FREEZE;
				break;

			case GIVE:
				bk_reset_frame   = 51;
				local->bk_action = FREEZE;
				break;
			}
			break;

		case 2:
		case 3:
		case 4:
			switch (local->bk_action) {
			case TALK:
				bk_reset_frame = imath_random(1, 3);
				++local->bk_talk_count;
				if (local->bk_talk_count > 15) {
					local->bk_action    = FREEZE;
					local->bk_talk_count = 0;
					bk_reset_frame       = 1;
				}
				break;

			default:
				bk_reset_frame = 0;
				break;
			}
			break;
		}

		if (bk_reset_frame >= 0) {
			kernel_reset_animation(aa[1], bk_reset_frame);
			local->bk_frame = bk_reset_frame;
		}
	}
}

static void handle_anim_frog_1() {
	int frog_1_reset_frame;
	int it;

	if (kernel_anim[aa[3]].frame != local->frog_1_frame) {
		local->frog_1_frame = kernel_anim[aa[3]].frame;
		frog_1_reset_frame = -1;

		switch (local->frog_1_frame) {
		case 2:
			switch (local->frog_1_action) {
				case FREEZE:
					frog_1_reset_frame = 1;
					++local->frog_1_talk_count;
					if (local->frog_1_talk_count > imath_random(30, 40)) {
						local->frog_1_talk_count = 0;
						frog_1_reset_frame       = 0;
					}
					break;

				default:
					frog_1_reset_frame = 0;
					break;
			}
			break;

		case 8:
			switch (local->frog_1_action) {
				case FREEZE:
					frog_1_reset_frame = 7;
					++local->frog_1_talk_count;
					if (local->frog_1_talk_count > imath_random(5, 10)) {
						local->frog_1_talk_count = 0;
						frog_1_reset_frame       = 15;
					}
					break;

				default:
					frog_1_reset_frame = 15;
					break;
			}
			break;

		case 1:
		case 15:
		case 21:
		case 30:
		case 37:
		switch (local->frog_1_action) {
			case EAT:
				frog_1_reset_frame = 8;
				break;

			case KILL:
				kernel_abort_animation(aa[3]);
				local->anim_3_running = false;
				player.walker_visible = false;
				if (global[player_persona] == PLAYER_IS_KING) {
					aa[3] = kernel_run_animation(kernel_name('d', 1), ROOM_303_DEATH + 1);
				} else {
					aa[3] = kernel_run_animation(kernel_name('d', 2), ROOM_303_DEATH + 1);
				}
				kernel_synch(KERNEL_ANIM, aa[3], KERNEL_PLAYER, 0);
				local->anim_10_running = true;
				break;

			case FREEZE:
				it = imath_random(1, 9);
				switch (it) {
					case 1: case 2: frog_1_reset_frame = 1; break;
					case 3: case 4: frog_1_reset_frame = 2; break;
					case 5: case 6: frog_1_reset_frame = 21; break;
					case 7: case 8: frog_1_reset_frame = 30; break;
					case 9: frog_1_reset_frame = 8; break;
				}
				break;
			}
			break;
		}

		if (frog_1_reset_frame >= 0) {
			kernel_reset_animation(aa[3], frog_1_reset_frame);
			local->frog_1_frame = frog_1_reset_frame;
		}
	}
}

static void handle_anim_frog_2() {
	int frog_2_reset_frame;
	int it;

	if (kernel_anim[aa[4]].frame != local->frog_2_frame) {
		local->frog_2_frame = kernel_anim[aa[4]].frame;
		frog_2_reset_frame = -1;

		switch (local->frog_2_frame) {
		case 2:
			switch (local->frog_2_action) {
				case FREEZE:
					frog_2_reset_frame = 1;
					++local->frog_2_talk_count;
					if (local->frog_2_talk_count > imath_random(30, 40)) {
						local->frog_2_talk_count = 0;
						frog_2_reset_frame       = 0;
					}
					break;

				default:
					frog_2_reset_frame = 0;
					break;
			}
			break;

		case 6:
			switch (local->frog_2_action) {
				case FREEZE:
					frog_2_reset_frame = 5;
					++local->frog_2_talk_count;
					if (local->frog_2_talk_count > imath_random(5, 10)) {
						local->frog_2_talk_count = 0;
						frog_2_reset_frame       = 6;
					}
					break;

				default:
					frog_2_reset_frame = 6;
					break;
			}
			break;

		case 1:
		case 8:
		case 9:

			switch (local->frog_2_action) {

				case FREEZE:
					it = imath_random(1, 3);
					switch (it) {
						case 1: frog_2_reset_frame = 8; break;
						case 2: frog_2_reset_frame = 1; break;
						case 3: frog_2_reset_frame = 2; break;
					}
					break;
			}
			break;
		}

		if (frog_2_reset_frame >= 0) {
			kernel_reset_animation(aa[4], frog_2_reset_frame);
			local->frog_2_frame = frog_2_reset_frame;
		}
	}
}

static void handle_anim_frog_3() {
	int frog_3_reset_frame;
	int it;

	if (kernel_anim[aa[5]].frame != local->frog_3_frame) {
		local->frog_3_frame = kernel_anim[aa[5]].frame;
		frog_3_reset_frame = -1;

		switch (local->frog_3_frame) {
		case 2:
			switch (local->frog_3_action) {
			case FREEZE:
				frog_3_reset_frame = 1;
				++local->frog_3_talk_count;
				if (local->frog_3_talk_count > imath_random(30, 40)) {
					local->frog_3_talk_count = 0;
					frog_3_reset_frame       = 0;
				}
				break;

			default:
				frog_3_reset_frame = 0;
				break;
			}
			break;

		case 1:
		case 6:
		switch (local->frog_3_action) {
			case FREEZE:
				it = imath_random(1, 5);
				switch (it) {
					case 1:  frog_3_reset_frame = 2; break;
					default: frog_3_reset_frame = 1; break;
				}
				break;

			case EAT:
				frog_3_reset_frame = 2;
				break;
			}
			break;
		}

		if (frog_3_reset_frame >= 0) {
			kernel_reset_animation(aa[5], frog_3_reset_frame);
			local->frog_3_frame = frog_3_reset_frame;
		}
	}
}

static void handle_anim_frog_4() {
	int frog_4_reset_frame;
	int it;

	if (kernel_anim[aa[6]].frame != local->frog_4_frame) {
		local->frog_4_frame = kernel_anim[aa[6]].frame;
		frog_4_reset_frame = -1;

		switch (local->frog_4_frame) {
		case 2:
			switch (local->frog_4_action) {
			case FREEZE:
				frog_4_reset_frame = 1;
				++local->frog_4_talk_count;
				if (local->frog_4_talk_count > imath_random(30, 40)) {
					local->frog_4_talk_count = 0;
					frog_4_reset_frame       = 0;
				}
				break;

			default:
				frog_4_reset_frame = 0;
				break;
			}
			break;

		case 5:
			switch (local->frog_4_action) {
			case FREEZE:
				frog_4_reset_frame = 4;
				++local->frog_4_talk_count;
				if (local->frog_4_talk_count > imath_random(30, 40)) {
					local->frog_4_talk_count = 0;
					frog_4_reset_frame       = 5;
				}
				break;

			default:
				frog_4_reset_frame = 5;
				break;
			}
			break;

		case 1:
		case 6:
		case 7:
		case 11:
			switch (local->frog_4_action) {
			case EAT:
				frog_4_reset_frame = 7;
				break;

			case FREEZE:
				it = imath_random(1, 4);
				switch (it) {
					case 1: frog_4_reset_frame = 2; break;
					case 2: frog_4_reset_frame = 6; break;
					default: frog_4_reset_frame = 1; break;
				}
				break;
			}
			break;
		}

		if (frog_4_reset_frame >= 0) {
			kernel_reset_animation(aa[6], frog_4_reset_frame);
			local->frog_4_frame = frog_4_reset_frame;
		}
	}
}

static void handle_anim_frog_5() {
	int frog_5_reset_frame;
	int it;

	if (kernel_anim[aa[7]].frame != local->frog_5_frame) {
		local->frog_5_frame = kernel_anim[aa[7]].frame;
		frog_5_reset_frame = -1;

		switch (local->frog_5_frame) {
		case 2:
			switch (local->frog_5_action) {
			case FREEZE:
				frog_5_reset_frame = 1;
				++local->frog_5_talk_count;
				if (local->frog_5_talk_count > imath_random(30, 40)) {
					local->frog_5_talk_count = 0;
					frog_5_reset_frame       = 0;
				}
				break;

			default:
				frog_5_reset_frame = 0;
				break;
			}
			break;

		case 5:
			switch (local->frog_5_action) {
				case FREEZE:
					frog_5_reset_frame = 4;
					++local->frog_5_talk_count;
					if (local->frog_5_talk_count > imath_random(30, 40)) {
						local->frog_5_talk_count = 0;
						frog_5_reset_frame       = 5;
					}
					break;

				default:
					frog_5_reset_frame = 5;
					break;
			}
			break;

		case 1:
		case 7:
		case 8:
		case 9:
		case 15:
		switch (local->frog_5_action) {
			case FREEZE:
				it = imath_random(1, 7);
				switch (it) {
					case 1: frog_5_reset_frame = 2; break;
					case 2: frog_5_reset_frame = 7; break;
					case 3: frog_5_reset_frame = 8; break;
					case 4: frog_5_reset_frame = 9; break;
					default: frog_5_reset_frame = 1; break;
				}
				break;

			case EAT:
				frog_5_reset_frame = 9;
				break;
			}
			break;
		}

		if (frog_5_reset_frame >= 0) {
			kernel_reset_animation(aa[7], frog_5_reset_frame);
			local->frog_5_frame = frog_5_reset_frame;
		}
	}
}

static void handle_anim_frog_6() {
	int frog_6_reset_frame;
	int it;

	if (kernel_anim[aa[9]].frame != local->frog_6_frame) {
		local->frog_6_frame = kernel_anim[aa[9]].frame;
		frog_6_reset_frame = -1;

		switch (local->frog_6_frame) {
		case 2:
			switch (local->frog_6_action) {
			case FREEZE:
				frog_6_reset_frame = 1;
				++local->frog_6_talk_count;
				if (local->frog_6_talk_count > imath_random(30, 40)) {
					local->frog_6_talk_count = 0;
					frog_6_reset_frame       = 0;
				}
				break;

			default:
				frog_6_reset_frame = 0;
				break;
			}
			break;

		case 1:
		case 10:
			switch (local->frog_6_action) {
			case FREEZE:
				it = imath_random(1, 5);
				switch (it) {
					case 1: frog_6_reset_frame = 2; break;
					default: frog_6_reset_frame = 1; break;
				}
				break;

			case EAT:
				frog_6_reset_frame = 2;
				break;
			}
			break;
		}

		if (frog_6_reset_frame >= 0) {
			kernel_reset_animation(aa[9], frog_6_reset_frame);
			local->frog_6_frame = frog_6_reset_frame;
		}
	}
}

static void handle_anim_fli() {
	int fli_reset_frame;

	if (kernel_anim[aa[8]].frame != local->fli_frame) {
		local->fli_frame = kernel_anim[aa[8]].frame;
		fli_reset_frame = -1;

		switch (local->fli_frame) {
		case 15:
			inter_move_object(bottle_of_flies, NOWHERE);
			break;

		case 19:
			local->frog_1_action = EAT;
			local->frog_3_action = EAT;
			local->frog_4_action = EAT;
			local->frog_5_action = EAT;
			local->frog_6_action = EAT;
			break;

		case 61:
			local->frog_1_action = FREEZE;
			local->frog_3_action = FREEZE;
			local->frog_4_action = FREEZE;
			local->frog_5_action = FREEZE;
			local->frog_6_action = FREEZE;
			break;

		case 77:
			player.walker_visible   = true;
			player.commands_allowed = true;
			kernel_abort_animation(aa[8]);
			local->anim_8_running = false;
			local->dont_die       = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			object_examine(key_crown, 30315, 0);
			break;

		case 39:
			kernel_flip_hotspot(words_crown, false);
			kernel_seq_delete(seq[fx_crown]);
			sound_play(N_TakeObjectSnd);
			inter_give_to_player(key_crown);
			++global[player_score];
			break;
		}

		if (fli_reset_frame >= 0) {
			kernel_reset_animation(aa[8], fli_reset_frame);
			local->fli_frame = fli_reset_frame;
		}
	}
}

static void handle_anim_death() {
	int death_reset_frame;

	if (kernel_anim[aa[3]].frame != local->death_frame) {
		local->death_frame = kernel_anim[aa[3]].frame;
		death_reset_frame = -1;

		switch (local->death_frame) {
		case 11:
			if (speech_system_active && speech_on) {
				global_speech_go(2);
			} else {
				sound_play(N_ToadEatsPlayer);
			}
			break;

		case 25:
			if (speech_system_active && speech_on) {
				global_speech_go(3);
			} else {
				sound_play(N_ToadEatsPlayer);
			}
			break;
		}

		if (death_reset_frame >= 0) {
			kernel_reset_animation(aa[3], death_reset_frame);
			local->death_frame = death_reset_frame;
		}
	}
}

static void room_303_daemon() {
	int temp;

	if (local->anim_0_running) {
		handle_anim_butter();
	}

	if (local->anim_1_running) {
		handle_anim_bk();
	}

	if (local->anim_3_running) {
		handle_anim_frog_1();
	}

	if (local->anim_4_running) {
		handle_anim_frog_2();
	}

	if (local->anim_5_running) {
		handle_anim_frog_3();
	}

	if (local->anim_6_running) {
		handle_anim_frog_4();
	}

	if (local->anim_7_running) {
		handle_anim_frog_5();
	}

	if (local->anim_8_running) {
		handle_anim_fli();
	}

	if (local->anim_9_running) {
		handle_anim_frog_6();
	}

	if (local->anim_10_running) {
		handle_anim_death();
	}

	if (camera_x.pan_this_frame) {
		set_scroll_left_position();
		set_scroll_right_position();
		set_scroll_mid_position();
	}

	if (local->pid_take) {
		local->pid_take = false;
		player.commands_allowed = false;
		player.walker_visible   = false;
		seq[fx_king_take_stone] = kernel_seq_forward(ss[fx_king_take_stone], true, 7, 0, 0, 1);
		kernel_seq_trigger(seq[fx_king_take_stone], KERNEL_TRIGGER_EXPIRE, 0, 1);
		kernel_seq_player(seq[fx_king_take_stone], true);
		kernel_seq_range(seq[fx_king_take_stone], KERNEL_FIRST, KERNEL_LAST);
	}

	switch (kernel.trigger) {
	case 1:
		local->bk_action = GIVE;
		temp             = seq[fx_king_take_stone];
		seq[fx_king_take_stone] = kernel_seq_stamp(ss[fx_king_take_stone], true, KERNEL_LAST);
		kernel_seq_player(seq[fx_king_take_stone], false);
		kernel_synch(KERNEL_SERIES, seq[fx_king_take_stone], KERNEL_SERIES, temp);
		break;

	case 2:
		sound_play(N_TakeObjectSnd);
		++global[player_score];

		if (object_is_here(red_powerstone)) {
			inter_give_to_player(red_powerstone);
			object_examine(red_powerstone, 30301, 0);

		} else if (object_is_here(bird_figurine)) {
			inter_give_to_player(bird_figurine);
			object_examine(bird_figurine, 30302, 0);
			++global[dragon_high_scene];
		}

		kernel_seq_delete(seq[fx_king_take_stone]);
		seq[fx_king_take_stone] = kernel_seq_backward(ss[fx_king_take_stone], true, 7, 0, 0, 1);
		kernel_seq_trigger(seq[fx_king_take_stone], KERNEL_TRIGGER_EXPIRE, 0, 3);
		kernel_seq_player(seq[fx_king_take_stone], false);
		kernel_seq_range(seq[fx_king_take_stone], KERNEL_FIRST, KERNEL_LAST);
		break;

	case 3:
		player.walker_visible   = true;
		player.commands_allowed = true;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_king_take_stone]);
		conv_release();
		break;
	}

	if ((player.x >= DUL_X) && (player.y >= DUL_Y) && (player.x <= DLR_X) && (player.y <= DLR_Y) &&
	     !local->prevent_2 && !local->dont_die) {
		local->prevent_2 = true;
		if (global[player_persona] == PLAYER_IS_KING) {
			player_walk(DEATH_X, DEATH_Y - 2, FACING_NORTH);
		} else {
			player_walk(DEATH_X, DEATH_Y, FACING_NORTH);
		}
		player.commands_allowed = false;
		player_walk_trigger(ROOM_303_DEATH);
	}

	if (kernel.trigger == ROOM_303_DEATH) {
		local->frog_1_action = KILL;
	}

	if (kernel.trigger == ROOM_303_DEATH + 1) {
		if (game.difficulty == EASY_MODE) {
			text_show(30331);
		} else {
			text_show(45);
		}
		kernel.force_restart = true;
	}

	if (local->wing_type != WING_CHANGING && imath_random(1, 1200) == 1 &&
	    global[player_persona] == PLAYER_IS_KING) {

		kernel_seq_delete(seq[fx_wing]);

		if (local->wing_type == WING_BEFORE) {
			seq[fx_wing] = kernel_seq_forward(ss[fx_wing], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_wing], 10);
			kernel_seq_range(seq[fx_wing], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_wing], KERNEL_TRIGGER_EXPIRE, 0, ROOM_303_WING);
			local->wing_type = WING_CHANGING;

		} else {
			seq[fx_wing] = kernel_seq_backward(ss[fx_wing], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_wing], 10);
			kernel_seq_range(seq[fx_wing], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_wing], KERNEL_TRIGGER_EXPIRE, 0, ROOM_303_WING + 1);
			local->wing_type = WING_CHANGING;
		}
	}

	if (kernel.trigger == ROOM_303_WING) {
		seq[fx_wing] = kernel_seq_stamp(ss[fx_wing], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_wing], 10);
		local->wing_type = WING_AFTER;
	}

	if (kernel.trigger == ROOM_303_WING + 1) {
		seq[fx_wing] = kernel_seq_stamp(ss[fx_wing], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_wing], 10);
		local->wing_type = WING_BEFORE;
	}
}

static void room_303_pre_parser() {
	if (player_said_2(walk_down, path_to_east)) {
		player.walk_off_edge_to_room = 302;
	}

	if (player_said_2(open, flies) || player_said_3(give, flies, toads)) {
		player_walk(OPEN_X, OPEN_Y, FACING_NORTH);
		local->dont_die = true;
	}

	if (player_said_2(take, crystal_flower) && object_is_here(crystal_flower)) {
		if (global[player_persona] == PLAYER_IS_PID) {
			player_walk(PID_FLOWER_X, PID_FLOWER_Y, FACING_NORTHEAST);
		} else {
			player_walk(KING_FLOWER_X, KING_FLOWER_Y, FACING_SOUTHEAST);
		}
	}
}

static void process_conv_king() {
	int you_trig_flag  = false;

	if (player_verb == conv057_exit_b_b) {
		*conv_my_next_start = conv057_startquiz;
		local->bk_action    = FREEZE_ABORT;
		you_trig_flag       = true;
		conv_abort();
	}

	if (player_verb == conv057_exit_d_d) {
		*conv_my_next_start = conv057_newquiz;
		local->bk_action    = FREEZE_ABORT;
		you_trig_flag       = true;
		conv_abort();
	}

	if (player_verb == conv057_exit_f_f) {
		conv_hold();
		you_trig_flag   = true;
		local->pid_take = true;
	}

	if (player_verb == conv057_exit_h_h) {
		*conv_my_next_start = conv057_restart;
		local->bk_action    = FREEZE_ABORT;
		you_trig_flag       = true;
		conv_abort();
	}

	if (player_verb == conv057_respons_b_b) {
		conv_hold();
		you_trig_flag   = true;
		local->pid_take = true;
	}

	if (kernel.trigger == ROOM_303_YOU_TALK) {
		if (player_verb == conv057_startquiz_only) {
			local->bk_action = WELCOME;
		} else if (local->bk_action != GIVE) {
			local->bk_action = TALK;
		}
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_303_YOU_TALK);
	}

	local->bk_talk_count = 0;
}

static void room_303_parser() {
	if (local->prevent_2) {
		goto handled;
	}

	if (conv_control.running == CONV_57_KING) {
		process_conv_king();
		goto handled;
	}

	if (player_said_2(open, flies) || player_said_3(give, flies, toads)) {
		aa[8]                 = kernel_run_animation(kernel_name('t', 1), 0);
		local->anim_8_running = true;
		kernel_synch(KERNEL_ANIM, aa[8], KERNEL_PLAYER, 0);
		player.walker_visible   = false;
		player.commands_allowed = false;
		global[player_score]   += 3;
		player_demand_location(AFTER_FLI_X, AFTER_FLI_Y);
		player_demand_facing(FACING_SOUTH);
		goto handled;
	}

	if (player_said_2(take, crystal_flower) || player_said_2(pull, crystal_flower)) {
		switch (kernel.trigger) {
		case 0:
			if (!player_has(crystal_flower)) {
				player.commands_allowed = false;
				player.walker_visible   = false;
				if (global[player_persona] == PLAYER_IS_KING) {
					seq[fx_take] = kernel_seq_pingpong(ss[fx_take], false, 7, 0, 0, 2);
					kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 5, 2);
					kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_EXPIRE, 0, 3);

				} else {
					seq[fx_take] = kernel_seq_pingpong(ss[fx_take], false, 7, 0, 0, 2);
					kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 4, 2);
					kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_EXPIRE, 0, 3);
				}
				kernel_seq_depth(seq[fx_take], 1);
				kernel_seq_range(seq[fx_take], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_player(seq[fx_take], true);
				goto handled;
			}
			break;

		case 2:
			if (local->prevent) {
				kernel_seq_delete(seq[fx_flower]);
				kernel_flip_hotspot(words_crystal_flower, false);
				global[player_score] += 5;
				sound_play(N_CrystalPing);
				inter_give_to_player(crystal_flower);
				object_examine(crystal_flower, 30329, 0);
			}
			local->prevent = true;
			goto handled;
			break;

		case 3:
			player.walker_visible   = true;
			player.commands_allowed = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take]);
			goto handled;
			break;
		}
	}

	if (player_said_2(talk_to, Butterfly_King)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			kernel_abort_animation(aa[3]);
			kernel_abort_animation(aa[7]);
			kernel_abort_animation(aa[6]);
			kernel_abort_animation(aa[5]);
			kernel_abort_animation(aa[9]);
			local->anim_3_running = false;
			local->anim_7_running = false;
			local->anim_6_running = false;
			local->anim_5_running = false;
			local->anim_9_running = false;

			player.commands_allowed = false;
			kernel_seq_delete(seq[fx_butt_king]);
			aa[1]                 = kernel_run_animation(kernel_name('u', 1), 0);
			local->anim_1_running = true;
			local->bk_action      = FREEZE;

			conv_run(CONV_57_KING);
		}
		goto handled;
	}

	if (player.look_around) {
		text_show(30330);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(mushroom)) {
			if (inter_point_x < 333 ||
			   (inter_point_x > 436 && inter_point_y > 86)) {
				text_show(30307);

			} else {
				if (global[player_persona] == PLAYER_IS_KING) {
					text_show(30304);
				} else {
					text_show(30303);
				}
			}
			goto handled;
		}

		if (player_said_1(ground)) {
			text_show(30308);
			goto handled;
		}

		if (player_said_1(flowers)) {
			text_show(30309);
			if (object_is_here(crystal_flower)) {
				text_show(30310);
			}
			goto handled;
		}

		if (player_said_1(crown)) {
			text_show(30313);
			if (game.difficulty == EASY_MODE && global[can_view_crown_hole]) {
				text_show(30314);
			}
			goto handled;
		}

		if (player_said_1(tree_stump)) {
			text_show(30316);
			goto handled;
		}

		if (player_said_1(toads)) {
			text_show(30320);
			goto handled;
		}

		if (player_said_1(Butterfly_King)) {
			text_show(30325);
			goto handled;
		}

		if (player_said_1(sanctuary_woods)) {
			text_show(30327);
			goto handled;
		}

		if (player_said_1(path_to_east)) {
			text_show(30332);
			goto handled;
		}

		if (player_said_1(crystal_flower) && object_is_here(crystal_flower)) {
			text_show(30328);
			goto handled;
		}
	}

	if (player_said_2(take, mushroom)) {
		text_show(30305);
		goto handled;
	}

	if (player_said_3(sword, attack, mushroom) ||
	    player_said_3(sword, carve_up, mushroom) ||
	    player_said_3(sword, thrust, mushroom) ||
	    player_said_2(push, mushroom) ||
	    player_said_2(pull, mushroom)) {
		text_show(30306);
		goto handled;
	}

	if (player_said_2(take, flowers) || player_said_2(pull, flowers)) {
		text_show(30311);
		goto handled;
	}

	if (player_said_2(open, flowers) || player_said_2(close, flowers)) {
		text_show(30312);
		goto handled;
	}

	if (player_said_2(take, tree_stump)) {
		text_show(30317);
		goto handled;
	}

	if (player_said_2(open, tree_stump)) {
		text_show(30318);
		goto handled;
	}

	if (player_said_2(put, tree_stump)) {
		text_show(30319);
		goto handled;
	}

	if (player_said_2(take, toads)) {
		text_show(30320);
		goto handled;
	}

	if (player_said_2(close, toads)) {
		text_show(30322);
		goto handled;
	}

	if (player_said_2(talk_to, toads)) {
		text_show(30323);
		goto handled;
	}

	if (player_said_2(throw, toads) || player_said_2(give, toads)) {
		if (player_said_1(fruit) ||
		    player_said_1(bone) ||
		    player_said_1(dates) ||
		    player_said_1(ratsicle) ||
		    player_said_1(dead_rat) ||
		    player_said_1(tentacle_parts)) {
			text_show(30324);
			goto handled;
		}
	}

	if (player_said_3(sword, attack, Butterfly_King) ||
	    player_said_3(sword, carve_up, Butterfly_King) ||
	    player_said_3(sword, thrust, Butterfly_King) ||
	    player_said_2(throw, Butterfly_King) ||
	    player_said_2(push, Butterfly_King) ||
	    player_said_2(pull, Butterfly_King)) {
		text_show(30326);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_303_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)     s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)   s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation)  s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.scroll_left_base);
	s.syncAsSint16LE(scratch.dyn_scroll_left);
	s.syncAsSint16LE(scratch.scroll_right_base);
	s.syncAsSint16LE(scratch.dyn_scroll_right);
	s.syncAsSint16LE(scratch.scroll_mid_base);
	s.syncAsSint16LE(scratch.dyn_scroll_mid);
	s.syncAsSint16LE(scratch.butter_frame);
	s.syncAsSint16LE(scratch.butter_talk_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.bk_frame);
	s.syncAsSint16LE(scratch.bk_action);
	s.syncAsSint16LE(scratch.bk_talk_count);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.frog_1_frame);
	s.syncAsSint16LE(scratch.frog_1_action);
	s.syncAsSint16LE(scratch.frog_1_talk_count);
	s.syncAsSint16LE(scratch.anim_3_running);
	s.syncAsSint16LE(scratch.frog_2_frame);
	s.syncAsSint16LE(scratch.frog_2_action);
	s.syncAsSint16LE(scratch.frog_2_talk_count);
	s.syncAsSint16LE(scratch.anim_4_running);
	s.syncAsSint16LE(scratch.frog_3_frame);
	s.syncAsSint16LE(scratch.frog_3_action);
	s.syncAsSint16LE(scratch.frog_3_talk_count);
	s.syncAsSint16LE(scratch.anim_5_running);
	s.syncAsSint16LE(scratch.frog_4_frame);
	s.syncAsSint16LE(scratch.frog_4_action);
	s.syncAsSint16LE(scratch.frog_4_talk_count);
	s.syncAsSint16LE(scratch.anim_6_running);
	s.syncAsSint16LE(scratch.frog_5_frame);
	s.syncAsSint16LE(scratch.frog_5_action);
	s.syncAsSint16LE(scratch.frog_5_talk_count);
	s.syncAsSint16LE(scratch.anim_7_running);
	s.syncAsSint16LE(scratch.fli_frame);
	s.syncAsSint16LE(scratch.fli_action);
	s.syncAsSint16LE(scratch.fli_talk_count);
	s.syncAsSint16LE(scratch.anim_8_running);
	s.syncAsSint16LE(scratch.frog_6_frame);
	s.syncAsSint16LE(scratch.frog_6_action);
	s.syncAsSint16LE(scratch.frog_6_talk_count);
	s.syncAsSint16LE(scratch.anim_9_running);
	s.syncAsSint16LE(scratch.death_frame);
	s.syncAsSint16LE(scratch.anim_10_running);
	s.syncAsSint16LE(scratch.prevent);
	s.syncAsSint16LE(scratch.prevent_2);
	s.syncAsSint16LE(scratch.pid_take);
	s.syncAsSint16LE(scratch.wing_type);
	s.syncAsSint16LE(scratch.dont_die);
}

void room_303_preload() {
	room_init_code_pointer       = room_303_init;
	room_pre_parser_code_pointer = room_303_pre_parser;
	room_parser_code_pointer     = room_303_parser;
	room_daemon_code_pointer     = room_303_daemon;

	section_3_walker();
	section_3_interface();

	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
