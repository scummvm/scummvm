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
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section2.h"
#include "mads/madsv2/dragonsphere/rooms/room203.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];

	int16 wise_frame;
	int16 wise_action;
	int16 wise_talk_count;
	int16 anim_0_running;

	int16 beard_frame;
	int16 beard_talk_count;
	int16 anim_1_running;

	int16 rock_l_base;
	int16 dyn_rock_l;

	int16 rock_r_base;
	int16 dyn_rock_r;

	int16 vine_base;
	int16 dyn_vine;

	int16 eye;
	int16 face_left;
	int16 face_right;
	int16 prevent;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

static Scratch scratch;

#define fx_rock_l       0
#define fx_rock_r       1
#define fx_vine         2
#define fx_eye          3
#define fx_face_left    4
#define fx_face_right   5
#define fx_take         6

#define ROOM_203_EYE            60
#define ROOM_203_FACE_LEFT      64
#define ROOM_203_FACE_RIGHT     68
#define ROOM_203_YOU_TALK       73
#define ROOM_203_ME_TALK        75
#define ROOM_203_NEW_ROOM       90

#define camera_ratio_1          1
#define camera_ratio_2          2

#define PLAYER_X_FROM_201       -10
#define PLAYER_Y_FROM_201       121
#define WALK_TO_X_FROM_201      15
#define WALK_TO_Y_FROM_201      121

#define PLAYER_X_FROM_204       411
#define PLAYER_Y_FROM_204       94
#define WALK_TO_X_FROM_204      385
#define WALK_TO_Y_FROM_204      104

#define PLAYER_X_FROM_205       495
#define PLAYER_Y_FROM_205       150
#define WALK_TO_X_FROM_205      466
#define WALK_TO_Y_FROM_205      147

#define EYE_LEFT    0
#define EYE_RIGHT   1
#define EYE_MOVING  2

#define FACE_NORMAL   0
#define FACE_MOVING   1
#define FACE_ABNORMAL 2
#define FACE_NOT_THERE 3
#define FACE_THERE    4

#define FREEZE  0
#define TALK    1
#define POINT   2
#define GIVE    3

#define CONV_49_KING    49

static void set_203_rock_r_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_rock_r] >= 0) {
		kernel_seq_delete(seq[fx_rock_r]);
	}

	difference = center - local->rock_r_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace = sgn_in(displace, direction);

	x  = local->rock_r_base + displace - 1;
	xs = series_list[ss[fx_rock_r]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_rock_r] = -1;
	} else {
		seq[fx_rock_r] = kernel_seq_stamp_scroll(ss[fx_rock_r], false, 1);
		kernel_seq_loc(seq[fx_rock_r], x, 155);
		kernel_seq_depth(seq[fx_rock_r], 1);
	}
}

static void set_203_rock_l_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_rock_l] >= 0) {
		kernel_seq_delete(seq[fx_rock_l]);
	}

	difference = center - local->rock_l_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace = sgn_in(displace, direction);

	x  = local->rock_l_base + displace - 1;
	xs = series_list[ss[fx_rock_l]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_rock_l] = -1;
	} else {
		seq[fx_rock_l] = kernel_seq_stamp_scroll(ss[fx_rock_l], false, 1);
		kernel_seq_loc(seq[fx_rock_l], x, 155);
		kernel_seq_depth(seq[fx_rock_l], 1);
	}
}

static void set_203_vine_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_vine] >= 0) {
		kernel_seq_delete(seq[fx_vine]);
	}

	difference = center - local->vine_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace = sgn_in(displace, direction);

	x  = local->vine_base + displace - 1;
	xs = series_list[ss[fx_vine]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_vine] = -1;
	} else {
		seq[fx_vine] = kernel_seq_stamp_scroll(ss[fx_vine], false, 1);
		kernel_seq_loc(seq[fx_vine], x, 24);
		kernel_seq_depth(seq[fx_vine], 1);
	}
}

static void handle_anim_wise() {
	int wise_reset_frame;
	int it;

	if (kernel_anim[aa[0]].frame != local->wise_frame) {
		local->wise_frame = kernel_anim[aa[0]].frame;
		wise_reset_frame  = -1;

		switch (local->wise_frame) {

		case 57:
			player.commands_allowed = false;
			player.walker_visible   = false;
			seq[fx_take] = kernel_seq_pingpong(ss[fx_take], true, 7, 0, 0, 2);
			kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 6, 1);
			kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_EXPIRE, 0, 2);
			kernel_seq_depth(seq[fx_take], 3);
			kernel_seq_range(seq[fx_take], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_player(seq[fx_take], true);
			break;

		case 58:
			if (local->wise_action == GIVE) {
				wise_reset_frame = 57;
			}
			break;

		case 1:
		case 22:
		case 40:
		case 71:
		case 63:
		case 45:
			switch (local->wise_action) {
			case FREEZE:
				wise_reset_frame = 0;
				break;
			case POINT:
				wise_reset_frame   = 40;
				local->wise_action = FREEZE;
				break;
			case GIVE:
				wise_reset_frame = 50;
				break;
			case TALK:
				it = imath_random(1, 4);
				switch (it) {
				case 1: wise_reset_frame = 4;  break;
				case 2: wise_reset_frame = 22; break;
				case 3: wise_reset_frame = 1;  break;
				case 4: wise_reset_frame = 63; break;
				}
				local->wise_action = FREEZE;
				break;
			}
			break;

		case 66:
		case 67:
		case 68:
		case 69:
			switch (local->wise_action) {
			case TALK:
				wise_reset_frame = imath_random(65, 68);
				++local->wise_talk_count;
				if (local->wise_talk_count > 15) {
					local->wise_action     = FREEZE;
					local->wise_talk_count = 0;
					wise_reset_frame       = 69;
				}
				break;
			default:
				wise_reset_frame = 69;
				break;
			}
			break;

		case 2:
		case 3:
		case 4:
			switch (local->wise_action) {
			case TALK:
				wise_reset_frame = imath_random(1, 3);
				++local->wise_talk_count;
				if (local->wise_talk_count > 15) {
					local->wise_action     = FREEZE;
					local->wise_talk_count = 0;
					wise_reset_frame       = 0;
				}
				break;
			default:
				wise_reset_frame = 0;
				break;
			}
			break;
		}

		if (wise_reset_frame >= 0) {
			kernel_reset_animation(aa[0], wise_reset_frame);
			local->wise_frame = wise_reset_frame;
		}
	}
}

static void handle_anim_beard() {
	int beard_reset_frame;

	if (kernel_anim[aa[1]].frame != local->beard_frame) {
		local->beard_frame  = kernel_anim[aa[1]].frame;
		beard_reset_frame   = -1;

		++local->beard_talk_count;
		if (local->beard_talk_count > imath_random(1, 3)) {
			if (local->beard_frame == 1) {
				beard_reset_frame = imath_random(0, 1);
			} else if (local->beard_frame == 2) {
				beard_reset_frame = imath_random(0, 2);
			} else if (local->beard_frame == 3) {
				beard_reset_frame = imath_random(1, 3);
			} else if (local->beard_frame == 4) {
				beard_reset_frame = imath_random(2, 4);
			} else if (local->beard_frame == 5) {
				beard_reset_frame = imath_random(3, 4);
			}
			local->beard_talk_count = 0;
		} else {
			beard_reset_frame = local->beard_frame - 1;
		}

		if (beard_reset_frame >= 0) {
			kernel_reset_animation(aa[1], beard_reset_frame);
			local->beard_frame = beard_reset_frame;
		}
	}
}

static void process_conv_wise() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	if (player_verb == conv049_polyquiz_b_b) {
		conv_hold();
		you_trig_flag      = true;
		me_trig_flag       = true;
		local->wise_action = GIVE;
	}

	if (player_verb == conv049_exit_b_b) {
		*conv_my_next_start         = conv049_recheck;
		conv_abort();
		you_trig_flag               = true;
		me_trig_flag                = true;
		player.commands_allowed     = false;
		player_walk(PLAYER_X_FROM_204, PLAYER_Y_FROM_204, FACING_EAST);
		player_walk_trigger(ROOM_203_NEW_ROOM);
	}

	if (kernel.trigger == ROOM_203_YOU_TALK) {
		local->wise_action = TALK;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_203_YOU_TALK);
	}

	local->wise_talk_count = 0;
}

static void room_203_init() {
	local->eye              = EYE_LEFT;
	local->face_left        = FACE_NORMAL;
	local->face_right       = FACE_NOT_THERE;
	local->beard_talk_count = 0;
	local->wise_talk_count  = 0;
	local->prevent          = false;

	kernel_flip_hotspot(words_wise_shifter, false);
	kernel_flip_hotspot(words_shifter, false);

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
	}

	conv_get(CONV_49_KING);

	ss[fx_rock_l]     = kernel_load_series(kernel_name('x', 3), false);
	ss[fx_rock_r]     = kernel_load_series(kernel_name('x', 4), false);
	ss[fx_vine]       = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_eye]        = kernel_load_series(kernel_name('z', 0), false);
	ss[fx_face_left]  = kernel_load_series(kernel_name('z', 1), false);
	ss[fx_face_right] = kernel_load_series(kernel_name('z', 2), false);

	seq[fx_rock_l]     = -1;
	local->dyn_rock_l  = -1;
	local->rock_l_base = 64;

	seq[fx_rock_r]     = -1;
	local->dyn_rock_r  = -1;
	local->rock_r_base = 413;

	seq[fx_vine]     = -1;
	local->dyn_vine  = -1;
	local->vine_base = 70;

	if (global[player_persona] == PLAYER_IS_KING) {
		ss[fx_take]           = kernel_load_series("*KGRH_9", false);
		aa[0]                 = kernel_run_animation(kernel_name('w', 1), 0);
		local->anim_0_running = true;
		local->wise_action    = FREEZE;

		aa[1]                 = kernel_run_animation(kernel_name('b', 1), 0);
		local->anim_1_running = true;

		if (global[talked_to_wise]) {
			kernel_flip_hotspot(words_wise_shifter, true);
		} else {
			kernel_flip_hotspot(words_shifter, true);
		}
	}

	if (conv_restore_running == CONV_49_KING) {
		conv_run(CONV_49_KING);
		conv_export_value(global[talked_to_shifter]);
		conv_export_value(global[talked_to_merchant]);
		if (object[pid_doll].location == 2 ||
		    object[pid_doll].location == 201) {
			conv_export_value(1);
		} else {
			conv_export_value(0);
		}
		conv_export_value(player_has(polystone));
	}

	if (previous_room == 204) {
		player_first_walk(PLAYER_X_FROM_204, PLAYER_Y_FROM_204, FACING_SOUTHWEST,
		                  WALK_TO_X_FROM_204, WALK_TO_Y_FROM_204, FACING_SOUTHWEST, true);
		camera_jump_to(160, 0);
	} else if (previous_room == 205) {
		player_first_walk(PLAYER_X_FROM_205, PLAYER_Y_FROM_205, FACING_WEST,
		                  WALK_TO_X_FROM_205, WALK_TO_Y_FROM_205, FACING_WEST, true);
		camera_jump_to(160, 0);
	} else if ((previous_room == 201) || (previous_room != KERNEL_RESTORING_GAME)) {
		player_first_walk(PLAYER_X_FROM_201, PLAYER_Y_FROM_201, FACING_EAST,
		                  WALK_TO_X_FROM_201, WALK_TO_Y_FROM_201, FACING_EAST, true);
	}

	set_203_rock_r_position();
	set_203_rock_l_position();
	set_203_vine_position();

	section_2_music();
}

static void room_203_daemon() {
	int temp;

	if (local->anim_0_running) {
		handle_anim_wise();
	}

	if (local->anim_1_running) {
		handle_anim_beard();
	}

	if (camera_x.pan_this_frame) {
		set_203_rock_r_position();
		set_203_rock_l_position();
		set_203_vine_position();
	}

	if (local->eye != EYE_MOVING && imath_random(1, 320) == 1) {

		if (local->eye == EYE_LEFT) {
			if (imath_random(1, 2) == 1) {
				seq[fx_eye] = kernel_seq_pingpong_scroll(ss[fx_eye], false, 5, 0, 0, 2);
				kernel_seq_depth(seq[fx_eye], 1);
				kernel_seq_range(seq[fx_eye], 1, 4);
				kernel_seq_trigger(seq[fx_eye], KERNEL_TRIGGER_EXPIRE, 0, ROOM_203_EYE);
			} else {
				seq[fx_eye] = kernel_seq_forward_scroll(ss[fx_eye], false, 8, 0, 0, 1);
				kernel_seq_depth(seq[fx_eye], 1);
				kernel_seq_range(seq[fx_eye], 5, 7);
				kernel_seq_trigger(seq[fx_eye], KERNEL_TRIGGER_EXPIRE, 0, ROOM_203_EYE + 1);
			}
		} else if (local->eye == EYE_RIGHT) {
			kernel_seq_delete(seq[fx_eye]);
			seq[fx_eye] = kernel_seq_backward_scroll(ss[fx_eye], false, 8, 0, 0, 1);
			kernel_seq_depth(seq[fx_eye], 1);
			kernel_seq_range(seq[fx_eye], 5, 7);
			kernel_seq_trigger(seq[fx_eye], KERNEL_TRIGGER_EXPIRE, 0, ROOM_203_EYE + 2);
		}

		local->eye = EYE_MOVING;
	}

	if (kernel.trigger == ROOM_203_EYE ||
	    kernel.trigger == ROOM_203_EYE + 2) {
		local->eye = EYE_LEFT;
	}

	if (kernel.trigger == ROOM_203_EYE + 1) {
		temp = seq[fx_eye];
		seq[fx_eye] = kernel_seq_stamp_scroll(ss[fx_eye], false, 7);
		kernel_seq_depth(seq[fx_eye], 1);
		kernel_synch(KERNEL_SERIES, seq[fx_eye], KERNEL_SERIES, temp);
		local->eye = EYE_RIGHT;
	}

	if (local->face_left != FACE_MOVING && imath_random(1, 320) == 1) {

		if (local->face_left == FACE_NORMAL) {
			seq[fx_face_left] = kernel_seq_forward_scroll(ss[fx_face_left], false, 9, 0, 0, 1);
			kernel_seq_depth(seq[fx_face_left], 1);
			kernel_seq_range(seq[fx_face_left], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_face_left], KERNEL_TRIGGER_EXPIRE, 0, ROOM_203_FACE_LEFT);
		} else {
			kernel_seq_delete(seq[fx_face_left]);
			seq[fx_face_left] = kernel_seq_backward_scroll(ss[fx_face_left], false, 9, 0, 0, 1);
			kernel_seq_depth(seq[fx_face_left], 1);
			kernel_seq_range(seq[fx_face_left], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_face_left], KERNEL_TRIGGER_EXPIRE, 0, ROOM_203_FACE_LEFT + 1);
		}

		local->face_left = FACE_MOVING;
	}

	if (kernel.trigger == ROOM_203_FACE_LEFT) {
		temp = seq[fx_face_left];
		seq[fx_face_left] = kernel_seq_stamp_scroll(ss[fx_face_left], false, 4);
		kernel_seq_depth(seq[fx_face_left], 1);
		kernel_synch(KERNEL_SERIES, seq[fx_face_left], KERNEL_SERIES, temp);
		local->face_left = FACE_ABNORMAL;
	}

	if (kernel.trigger == ROOM_203_FACE_LEFT + 1) {
		local->face_left = FACE_NORMAL;
	}

	if (local->face_right == FACE_NOT_THERE && imath_random(1, 500) == 1) {
		seq[fx_face_right] = kernel_seq_forward_scroll(ss[fx_face_right], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_face_right], 10);
		kernel_seq_range(seq[fx_face_right], 1, 8);
		kernel_seq_trigger(seq[fx_face_right], KERNEL_TRIGGER_EXPIRE, 0, ROOM_203_FACE_RIGHT);
		local->face_right = FACE_MOVING;
	}

	if (local->face_right == FACE_THERE && imath_random(1, 500) == 1) {
		kernel_seq_delete(seq[fx_face_right]);
		seq[fx_face_right] = kernel_seq_backward_scroll(ss[fx_face_right], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_face_right], 10);
		kernel_seq_range(seq[fx_face_right], 1, 8);
		kernel_seq_trigger(seq[fx_face_right], KERNEL_TRIGGER_EXPIRE, 0, ROOM_203_FACE_RIGHT + 1);
		local->face_right = FACE_MOVING;
	}

	if (local->face_right == FACE_THERE && imath_random(1, 320) == 1) {
		kernel_seq_delete(seq[fx_face_right]);
		seq[fx_face_right] = kernel_seq_pingpong_scroll(ss[fx_face_right], false, 7, 0, 0, 2);
		kernel_seq_depth(seq[fx_face_right], 10);
		kernel_seq_range(seq[fx_face_right], 8, 9);
		kernel_seq_trigger(seq[fx_face_right], KERNEL_TRIGGER_EXPIRE, 0, ROOM_203_FACE_RIGHT + 2);
		local->face_right = FACE_MOVING;
	}

	if (kernel.trigger == ROOM_203_FACE_RIGHT) {
		temp = seq[fx_face_right];
		seq[fx_face_right] = kernel_seq_stamp_scroll(ss[fx_face_right], false, 8);
		kernel_seq_depth(seq[fx_face_right], 10);
		kernel_synch(KERNEL_SERIES, seq[fx_face_right], KERNEL_SERIES, temp);
		local->face_right = FACE_THERE;
	}

	if (kernel.trigger == ROOM_203_FACE_RIGHT + 1) {
		local->face_right = FACE_NOT_THERE;
	}

	if (kernel.trigger == ROOM_203_FACE_RIGHT + 2) {
		temp = seq[fx_face_right];
		seq[fx_face_right] = kernel_seq_stamp_scroll(ss[fx_face_right], false, 8);
		kernel_seq_depth(seq[fx_face_right], 10);
		kernel_synch(KERNEL_SERIES, seq[fx_face_right], KERNEL_SERIES, temp);
		local->face_right = FACE_THERE;
	}

	switch (kernel.trigger) {
	case 1:
		if (local->prevent) {
			++global[player_score];
			sound_play(N_TakeObjectSnd);
			inter_give_to_player(polystone);
			object_examine(polystone, 20329, 0);
			local->wise_action = FREEZE;
		}
		local->prevent = true;
		break;

	case 2:
		player.walker_visible   = true;
		player.commands_allowed = true;
		local->prevent          = false;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take]);
		conv_release();
		break;
	}
}

static void room_203_pre_parser() {
	if (player_said_1(cave) && player.need_to_walk) {
		if (!player_said_1(walk_into)) {
			player.need_to_walk = false;
		}
	}

	if (player_said_2(walk_down, path_to_east)) {
		player.walk_off_edge_to_room = 205;
	}

	if (player_said_2(walk_down, path_to_west)) {
		player.walk_off_edge_to_room = 201;
	}
}

static void room_203_parser() {
	if (conv_control.running == CONV_49_KING) {
		process_conv_wise();
		goto handled;
	}

	if (player_said_2(walk_into, cave) ||
		kernel.trigger == ROOM_203_NEW_ROOM) {
		new_room = 204;
		goto handled;
	}

	if (player_said_2(talk_to, shifter) || player_said_2(talk_to, wise_shifter)) {
		conv_run(CONV_49_KING);
		conv_export_value(global[talked_to_shifter]);
		conv_export_value(global[talked_to_merchant]);
		if (object[pid_doll].location == 2 ||
			object[pid_doll].location == 201) {
			conv_export_value(1);
		} else {
			conv_export_value(0);
		}
		conv_export_value(player_has(polystone));

		global[talked_to_wise] = true;
		kernel_flip_hotspot(words_wise_shifter, true);
		kernel_flip_hotspot(words_shifter, false);
		goto handled;
	}

	if (player_said_2(invoke, signet_ring)) {
		if (global[object_given_201] != -1) {
			text_show(20158);
			goto handled;
		}
	}

	if (player.look_around) {
		if (global[player_persona] == PLAYER_IS_KING) {
			text_show(20301);
		} else {
			text_show(20326);
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(path)) {
			text_show(20302);
			goto handled;
		}

		if (player_said_1(eye_rock) || player_said_1(eye)) {
			text_show(20303);
			goto handled;
		}

		if (player_said_1(body_tree)) {
			text_show(20309);
			goto handled;
		}

		if (player_said_1(clearing)) {
			text_show(20315);
			goto handled;
		}

		if (player_said_1(boulders)) {
			text_show(20316);
			goto handled;
		}

		if (player_said_1(path_to_west)) {
			text_show(20318);
			goto handled;
		}

		if (player_said_1(rock)) {
			text_show(20331);
			goto handled;
		}

		if (player_said_1(path_to_east)) {
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(20319);
			} else {
				text_show(20327);
			}
			goto handled;
		}

		if (player_said_1(cave)) {
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(20320);
			} else {
				text_show(20328);
			}
			goto handled;
		}

		if (player_said_1(Slathan_ni_Patan)) {
			text_show(20323);
			goto handled;
		}

		if (player_said_1(shifter)) {
			text_show(20324);
			goto handled;
		}

		if (player_said_1(wise_shifter)) {
			text_show(20325);
			goto handled;
		}
	}

	if (player_said_2(take, eye_rock) || player_said_2(take, eye)) {
		text_show(20304);
		goto handled;
	}

	if (player_said_1(eye) || player_said_1(eye_rock)) {
		if (player_said_1(open) || player_said_1(close)) {
			text_show(20305);
			goto handled;
		}

		if (player_said_2(throw, mud) || player_said_2(put, mud) ||
			player_said_1(pour_contents_of)) {
			text_show(20308);
			goto handled;
		}
	}

	if (player_said_2(give, eye_rock) || player_said_2(give, eye)) {
		text_show(20307);
		goto handled;
	}

	if (player_said_2(open, body_tree)) {
		text_show(20310);
		goto handled;
	}

	if (player_said_2(close, body_tree)) {
		text_show(20311);
		goto handled;
	}

	if (player_said_1(body_tree)) {
		if (player_said_2(put, torch) ||
			player_said_2(sword, attack) ||
			player_said_2(sword, carve_up) ||
			player_said_2(sword, thrust) ||
			player_said_1(pour_contents_of)) {
			text_show(20312);
			goto handled;
		}
	}

	if (player_said_2(talk_to, body_tree)) {
		text_show(20313);
		goto handled;
	}

	if (player_said_2(talk_to, eye_rock) || player_said_2(talk_to, eye)) {
		text_show(20306);
		goto handled;
	}

	if (player_said_2(talk_to, rock)) {
		text_show(20330);
		goto handled;
	}

	if (player_said_2(give, body_tree)) {
		text_show(20314);
		goto handled;
	}

	if (player_said_2(talk_to, boulders) || player_said_2(talk_to, boulder)) {
		text_show(20332);
		goto handled;
	}

	if (player_said_1(boulders)) {
		if (player_said_1(take) ||
			player_said_1(push) ||
			player_said_1(pull)) {
			text_show(20317);
			goto handled;
		}
	}

	if (player_said_1(cave)) {
		if (player_said_1(open) || player_said_1(close)) {
			text_show(20321);
			goto handled;
		}
	}

	if (player_said_3(throw, torch, cave) || player_said_3(put, torch, cave)) {
		text_show(20322);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_203_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.wise_frame);
	s.syncAsSint16LE(scratch.wise_action);
	s.syncAsSint16LE(scratch.wise_talk_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.beard_frame);
	s.syncAsSint16LE(scratch.beard_talk_count);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.rock_l_base);
	s.syncAsSint16LE(scratch.dyn_rock_l);
	s.syncAsSint16LE(scratch.rock_r_base);
	s.syncAsSint16LE(scratch.dyn_rock_r);
	s.syncAsSint16LE(scratch.vine_base);
	s.syncAsSint16LE(scratch.dyn_vine);
	s.syncAsSint16LE(scratch.eye);
	s.syncAsSint16LE(scratch.face_left);
	s.syncAsSint16LE(scratch.face_right);
	s.syncAsSint16LE(scratch.prevent);
}

void room_203_preload() {
	room_init_code_pointer       = room_203_init;
	room_pre_parser_code_pointer = room_203_pre_parser;
	room_parser_code_pointer     = room_203_parser;
	room_daemon_code_pointer     = room_203_daemon;

	section_2_walker();
	section_2_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
