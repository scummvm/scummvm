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
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section2.h"
#include "mads/madsv2/dragonsphere/rooms/room201.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[26];       /* Sprite series handles */
	int16 sequence[26];     /* Sequence handles      */
	int16 animation[7];     /* Animation handles     */

	int16 lg_rock_base;
	int16 dyn_lg_rock;

	int16 md_rock_base;
	int16 dyn_md_rock;

	int16 sm_rock_base;
	int16 dyn_sm_rock;

	int16 lg_tree_base;
	int16 dyn_lg_tree;

	int16 vine_base;
	int16 dyn_vine;

	int16 mount_left_base;
	int16 dyn_mount_left;

	int16 mount_right_base;
	int16 dyn_mount_right;

	int16 vine_2_base;
	int16 dyn_vine_2;

	int16 vine_3_base;
	int16 dyn_vine_3;

	int16 num;
	int16 hand_status;

	int16 guard_left_frame;       /* animation frame being held for guard_left stuff */
	int16 guard_left_action;      /* Type of action to run for guard_left animation */
	int16 guard_left_talk_count;  /* counter for guard_left talking */
	int16 anim_0_running;

	int16 guard_right_frame;       /* animation frame being held for guard_right stuff */
	int16 guard_right_action;      /* Type of action to run for guard_right animation */
	int16 guard_right_talk_count;  /* counter for guard_right talking */
	int16 anim_1_running;
	int16 anim_3_running;

	int16 pid_frame;       /* animation frame being held for pid stuff */
	int16 pid_action;      /* Type of action to run for pid animation */
	int16 pid_talk_count;  /* counter for pid talking */
	int16 anim_2_running;

	int16 throw_frame;       /* animation frame being held for throw stuff */
	int16 anim_4_running;

	int16 take_frame;       /* animation frame being held for take stuff */
	int16 anim_5_running;

	int16 death_frame;       /* animation frame being held for death stuff */
	int16 anim_6_running;

	int16 prevent;
	int16 prevent_1;

	long clock;
	long death_timer;       /* when this counter reaches max sec, kill pid */
	int16 activate_timer;     /* If T, will start counting death_timer */
	int16 left_ready_to_fall;
	int16 right_ready_to_fall;

	int16 testicle_1_on;
	int16 testicle_2_on;
	int16 testicle_3_on;
	int16 testicle_4_on;

	int16 death_is_on;
	int16 special_shit;

	int16 crossed_line;
	byte cut_scene;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

static Scratch scratch;


/* ========================= Sprite Series =================== */

#define fx_lg_rock              0       /* rm201x0 */
#define fx_md_rock              1       /* rm201x2 */
#define fx_sm_rock              2       /* rm201x3 */
#define fx_lg_tree              3       /* rm201x1 */
#define fx_vine                 4       /* rm201x4 */
#define fx_tree_1               5       /* rm201x5 */
#define fx_tree_2               6       /* rm201x6 */
#define fx_tree_3               7       /* rm201x7 */
#define fx_tree_4               8       /* rm201x8 */
#define fx_vine_2               9       /* rm201x5 */
#define fx_vine_3               10      /* rm201x6 */
#define fx_throw                11      /* rm201x7 */
#define fx_smoke                12      /* rm201z1 */
#define fx_hand                 13      /* rm201z0 */
#define fx_take                 14      /* kgrh_9  */
#define fx_guard_1_dead         15      /* rm201g2 */
#define fx_guard_2_dead         16      /* rm201g3 */

#define fx_testicle_1           17      /* rm201y0 */
#define fx_testicle_2           18      /* rm201y2 */
#define fx_testicle_3           19      /* rm201y3 */
#define fx_testicle_4           20      /* rm201y4 */
#define fx_guard_stand_l        21      /* rm201g6 */
#define fx_guard_stand_r        22      /* rm201g7 */
#define fx_pair_testes          23      /* rm201m1 */
#define fx_ripple               24      /* rm201m2 */




/* ======================== Triggers ========================= */

#define ROOM_201_HAND           60
#define ROOM_201_HALT           65
#define ROOM_201_ME_TALK        70
#define ROOM_201_YOU_TALK       72
#define ROOM_201_GIVE           78
#define ROOM_201_DEATH          82
#define ROOM_201_TENT_3         90
#define ROOM_201_TENT_5         94
#define ROOM_201_TENT_6         98
#define ROOM_201_DEATH_2        102
#define ROOM_201_SPLASH         110
#define MUSIC                   120


/* ========================= Other Macros ==================== */

#define camera_ratio_1          1
#define camera_ratio_2          2


#define PLAYER_X_FROM_120       -15
#define PLAYER_Y_FROM_120       105
#define WALK_TO_X_FROM_120      50
#define WALK_TO_Y_FROM_120      109

#define PLAYER_X_FROM_203       805
#define PLAYER_Y_FROM_203       116
#define WALK_TO_X_FROM_203      783
#define WALK_TO_Y_FROM_203      126

#define CONV_47_KING            47
#define CONV_54_PID             54


#define WALK_3_X                121
#define WALK_3_Y                112

#define WALK_4_X                1
#define WALK_4_Y                104

#define HAND_OPEN               0
#define HAND_MOVING             1
#define HAND_CLOSED             2

#define FREEZE                  0
#define TALK                    1
#define GIVE_BOTTLE             2
#define GIVE_NOTHING            3
#define POINT                   4
#define FALL                    5
#define HALT                    6
#define SWITCH_TO_LEFT          7
#define SWITCH_TO_RIGHT         8
#define DRINK                   9
#define KILL                    10
#define LOOK_DOWN               11
#define HEAL                    12
#define HAND_TO_LEFT            13
#define HAND_TO_RIGHT           14
#define TAKE_OUT_BOTTLE         15
#define TAKE_BOTTLE             16
#define TAKE_BOTTLE_FROM_RIGHT  17
#define UNHALT                  18
#define HALT_FREEZE             19
#define PUT_AWAY                20
#define UNHALT_FALL             21
#define DUMP_ANIMS              22
#define UNHALT_DUMP_ANIMS       23

#define LENGTH_OF_LIFE          1300


#define GUARD_1_X               68
#define GUARD_1_Y               78

#define GUARD_2_X               89
#define GUARD_2_Y               91

#define DEATH_X                 626
#define DEATH_Y                 148

#define THROW_X                 586
#define THROW_Y                 145


static void set_lg_rock_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_lg_rock] >= 0) {
		kernel_seq_delete(seq[fx_lg_rock]);
	}

	difference = center - local->lg_rock_base;
	direction = neg(sgn(difference));
	distance = abs(difference);

	displace = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace = sgn_in(displace, direction);

	x = local->lg_rock_base + displace - 1;
	xs = series_list[ss[fx_lg_rock]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
		((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_lg_rock] = -1;

	} else {

		seq[fx_lg_rock] = kernel_seq_stamp(ss[fx_lg_rock], false, 1);
		kernel_seq_loc(seq[fx_lg_rock], x, 155);
		kernel_seq_depth(seq[fx_lg_rock], 1);
	}
}

static void set_md_rock_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_md_rock] >= 0) {
		kernel_seq_delete(seq[fx_md_rock]);
	}

	difference = center - local->md_rock_base;
	direction = neg(sgn(difference));
	distance = abs(difference);

	displace = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace = sgn_in(displace, direction);

	x = local->md_rock_base + displace - 1;
	xs = series_list[ss[fx_md_rock]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
		((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_md_rock] = -1;

	} else {

		seq[fx_md_rock] = kernel_seq_stamp(ss[fx_md_rock], false, 1);
		kernel_seq_loc(seq[fx_md_rock], x, 155);
		kernel_seq_depth(seq[fx_md_rock], 1);
	}
}

static void set_sm_rock_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_sm_rock] >= 0) {
		kernel_seq_delete(seq[fx_sm_rock]);
	}

	difference = center - local->sm_rock_base;
	direction = neg(sgn(difference));
	distance = abs(difference);

	displace = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace = sgn_in(displace, direction);

	x = local->sm_rock_base + displace - 1;
	xs = series_list[ss[fx_sm_rock]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
		((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_sm_rock] = -1;

	} else {

		seq[fx_sm_rock] = kernel_seq_stamp(ss[fx_sm_rock], false, 1);
		kernel_seq_loc(seq[fx_sm_rock], x, 155);
		kernel_seq_depth(seq[fx_sm_rock], 1);
	}
}

static void set_lg_tree_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_lg_tree] >= 0) {
		kernel_seq_delete(seq[fx_lg_tree]);
	}

	difference = center - local->lg_tree_base;
	direction = neg(sgn(difference));
	distance = abs(difference);

	displace = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace = sgn_in(displace, direction);

	x = local->lg_tree_base + displace - 1;
	xs = series_list[ss[fx_lg_tree]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
		((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_lg_tree] = -1;

	} else {

		seq[fx_lg_tree] = kernel_seq_stamp(ss[fx_lg_tree], false, 1);
		kernel_seq_loc(seq[fx_lg_tree], x, 155);
		kernel_seq_depth(seq[fx_lg_tree], 1);
	}
}

static void set_vine_position() {
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
	direction = neg(sgn(difference));
	distance = abs(difference);

	displace = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace = sgn_in(displace, direction);

	x = local->vine_base + displace - 1;
	xs = series_list[ss[fx_vine]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
		((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_vine] = -1;

	} else {

		seq[fx_vine] = kernel_seq_stamp(ss[fx_vine], false, 1);
		kernel_seq_loc(seq[fx_vine], x, 25);
		kernel_seq_depth(seq[fx_vine], 1);
	}
}

static void set_vine_2_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_vine_2] >= 0) {
		kernel_seq_delete(seq[fx_vine_2]);
	}

	difference = center - local->vine_2_base;
	direction = neg(sgn(difference));
	distance = abs(difference);

	displace = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace = sgn_in(displace, direction);

	x = local->vine_2_base + displace - 1;
	xs = series_list[ss[fx_vine_2]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
		((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_vine_2] = -1;

	} else {

		seq[fx_vine_2] = kernel_seq_stamp(ss[fx_vine_2], false, 1);
		kernel_seq_loc(seq[fx_vine_2], x, 16);
		kernel_seq_depth(seq[fx_vine_2], 1);
	}
}

static void set_vine_3_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_vine_3] >= 0) {
		kernel_seq_delete(seq[fx_vine_3]);
	}

	difference = center - local->vine_3_base;
	direction = neg(sgn(difference));
	distance = abs(difference);

	displace = (int)(((long)distance * camera_ratio_1) / 6);
	displace = sgn_in(displace, direction);

	x = local->vine_3_base + displace - 1;
	xs = series_list[ss[fx_vine_3]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
		((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_vine_3] = -1;

	} else {

		seq[fx_vine_3] = kernel_seq_stamp(ss[fx_vine_3], false, 1);
		kernel_seq_loc(seq[fx_vine_3], x, 34);
		kernel_seq_depth(seq[fx_vine_3], 10);
	}
}

static void room_201_init() {
	kernel.disable_fastwalk = true;
	local->hand_status = HAND_OPEN;
	local->death_timer = 0;
	local->clock = 0;
	local->prevent_1 = false;
	local->special_shit = false;
	local->cut_scene = false;

	if (global[player_persona] == PLAYER_IS_PID) {
		local->death_is_on = true;
	} else {
		local->death_is_on = false;
	}

	if (global[player_persona] == PLAYER_IS_KING) {
		local->testicle_1_on = false;
		local->testicle_2_on = false;
		local->testicle_3_on = false;
		local->testicle_4_on = false;
	} else {
		local->testicle_1_on = true;
		local->testicle_2_on = true;
		local->testicle_3_on = true;
		local->testicle_4_on = true;
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_2_running = false;
		local->anim_3_running = false;
		local->anim_4_running = false;
		local->anim_5_running = false;
		local->anim_6_running = false;
		local->activate_timer = false;
		local->left_ready_to_fall = false;
		local->right_ready_to_fall = false;
		local->crossed_line = false;
	}

	if (previous_room == 120 || previous_room == 201 || previous_room != KERNEL_RESTORING_GAME) {
		local->prevent = false;
	} else {
		local->prevent = true;
	}

	kernel_flip_hotspot(words_pool_monster, false);

	if (!global[dome_up]) {
		kernel_flip_hotspot(words_spell_shield, false);
	}

	if (global[reset_conv] >= 0) {
		if (!global[given_object_before]) {
			conv_reset(global[reset_conv]);
		}
		global[reset_conv] = -1;
	}

	if (global[player_persona] == PLAYER_IS_KING) {
		conv_get(CONV_47_KING);
	} else {
		conv_get(CONV_54_PID);
	}

	ss[fx_lg_rock] = kernel_load_series(kernel_name('x', 0), false);
	flag_used[10] = false;
	ss[fx_md_rock] = kernel_load_series(kernel_name('x', 2), false);
	flag_used[10] = false;
	ss[fx_sm_rock] = kernel_load_series(kernel_name('x', 3), false);
	flag_used[10] = false;
	ss[fx_lg_tree] = kernel_load_series(kernel_name('x', 1), false);
	flag_used[10] = false;
	ss[fx_vine] = kernel_load_series(kernel_name('x', 4), false);
	flag_used[10] = false;
	ss[fx_vine_2] = kernel_load_series(kernel_name('x', 5), false);
	flag_used[10] = false;
	ss[fx_vine_3] = kernel_load_series(kernel_name('x', 6), false);
	flag_used[10] = false;

	if (global[player_persona] == PLAYER_IS_KING) {
		ss[fx_take] = kernel_load_series("*KGRH_9", false);
	}

	if (!global[guards_are_asleep]) {
		ss[fx_guard_stand_l] = kernel_load_series(kernel_name('g', 6), false);
		ss[fx_guard_stand_r] = kernel_load_series(kernel_name('g', 7), false);
	}

	ss[fx_smoke] = kernel_load_series(kernel_name('z', 1), false);
	seq[fx_smoke] = kernel_seq_forward(ss[fx_smoke], false, 8, 0, 0, 0);
	kernel_seq_depth(seq[fx_smoke], 1);
	kernel_seq_range(seq[fx_smoke], KERNEL_FIRST, KERNEL_LAST);

	ss[fx_hand] = kernel_load_series(kernel_name('z', 0), false);
	flag_used[10] = false;

	ss[fx_pair_testes] = kernel_load_series(kernel_name('m', 1), false);
	flag_used[10] = false;
	ss[fx_ripple] = kernel_load_series(kernel_name('m', 2), false);
	flag_used[10] = false;


	if (global[dome_up]) {
		seq[fx_ripple] = kernel_seq_forward(ss[fx_ripple], false, 20, 0, 0, 0);
		kernel_seq_depth(seq[fx_ripple], 11);
		kernel_seq_range(seq[fx_ripple], KERNEL_FIRST, KERNEL_LAST);
	}

	if (object_is_here(tentacle_parts) && global[dome_up]) {
		seq[fx_pair_testes] = kernel_seq_stamp(ss[fx_pair_testes], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_pair_testes], 10);

	} else {
		kernel_flip_hotspot(words_tentacles, false);
	}


	if (global[player_persona] == PLAYER_IS_KING && !global[dome_up]) {
		ss[fx_testicle_1] = kernel_load_series(kernel_name('y', 0), false);
		/* flag_used[10]     = false; */
		ss[fx_testicle_2] = kernel_load_series(kernel_name('y', 2), false);
		/* flag_used[10]     = false; */
		ss[fx_testicle_3] = kernel_load_series(kernel_name('y', 3), false);
		/* flag_used[10]     = false; */
		ss[fx_testicle_4] = kernel_load_series(kernel_name('y', 4), false);
		/* flag_used[10]     = false; */
	}

	ss[fx_guard_1_dead] = kernel_load_series(kernel_name('g', 2), false);
	flag_used[10] = false;
	ss[fx_guard_2_dead] = kernel_load_series(kernel_name('g', 3), false);
	flag_used[10] = false;


	if (global[guards_are_asleep]) {
		kernel_flip_hotspot_loc(words_guards, false, GUARD_1_X, GUARD_1_Y);
		kernel_flip_hotspot_loc(words_guards, false, GUARD_2_X, GUARD_2_Y);
		seq[fx_guard_1_dead] = kernel_seq_stamp(ss[fx_guard_1_dead], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_guard_1_dead], 13);
		seq[fx_guard_2_dead] = kernel_seq_stamp(ss[fx_guard_2_dead], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_guard_2_dead], 13);

	} else {
		kernel_flip_hotspot(words_guards, false);
		kernel_flip_hotspot_loc(words_guards, true, GUARD_1_X, GUARD_1_Y);
		kernel_flip_hotspot_loc(words_guards, true, GUARD_2_X, GUARD_2_Y);
	}

	seq[fx_lg_rock] = -1;
	local->dyn_lg_rock = -1;
	local->lg_rock_base = 100;

	seq[fx_md_rock] = -1;
	local->dyn_md_rock = -1;
	local->md_rock_base = 663;

	seq[fx_sm_rock] = -1;
	local->dyn_sm_rock = -1;
	local->sm_rock_base = 703;

	seq[fx_lg_tree] = -1;
	local->dyn_lg_tree = -1;
	local->lg_tree_base = 390;

	seq[fx_vine] = -1;
	local->dyn_vine = -1;
	local->vine_base = 720;

	seq[fx_vine_2] = -1;
	local->dyn_vine_2 = -1;
	local->vine_2_base = 480;

	seq[fx_vine_3] = -1;
	local->dyn_vine_3 = -1;
	local->vine_3_base = 540;

	if (!global[guards_are_asleep]) {

		if (global[player_persona] == PLAYER_IS_KING && global[object_given_201] != -1 &&
			conv_restore_running != CONV_47_KING) {
			seq[fx_guard_stand_l] = kernel_seq_stamp(ss[fx_guard_stand_l], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_guard_stand_l], 11);

		} else {
			aa[0] = kernel_run_animation(kernel_name('u', 1), 0);
			local->anim_0_running = true;
			local->guard_left_action = FREEZE;
		}

		if (global[player_persona] == PLAYER_IS_KING) {
			if (global[object_given_201] != -1 && conv_restore_running != CONV_47_KING) {
				seq[fx_guard_stand_r] = kernel_seq_stamp(ss[fx_guard_stand_r], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_guard_stand_r], 10);

			} else {
				aa[1] = kernel_run_animation(kernel_name('u', 3), 0);
				local->anim_1_running = true;
				local->guard_right_action = FREEZE;
				kernel_reset_animation(aa[1], 7);
			}

		} else {
			aa[3] = kernel_run_animation(kernel_name('u', 2), 0);
			local->anim_3_running = true;
			local->guard_right_action = FREEZE;
			kernel_reset_animation(aa[3], 7);
		}
	}

	if (conv_restore_running == CONV_47_KING) {
		kernel_reset_animation(aa[1], 6);
		local->guard_right_action = HALT_FREEZE;
		conv_run(CONV_47_KING);
		conv_export_value(0);
		conv_export_value(0);

	} else if (conv_restore_running == CONV_54_PID || (global[player_persona] == PLAYER_IS_PID && local->activate_timer)) {
		player.x = WALK_TO_X_FROM_120;
		player.y = WALK_TO_Y_FROM_120;
		player.facing = FACING_NORTHEAST;
		player.walker_visible = false;
		local->guard_right_action = HALT_FREEZE;

		aa[2] = kernel_run_animation(kernel_name('p', 1), 0);
		local->anim_2_running = true;
		local->pid_action = FREEZE;

		kernel_reset_animation(aa[3], 6);

		if (!local->activate_timer) {
			conv_run(CONV_54_PID);
			conv_export_value(0);
		}
	}

	if (local->activate_timer) {
		local->guard_right_action = HALT_FREEZE;
		if (global[player_persona] == PLAYER_IS_KING) {
			kernel_reset_animation(aa[1], 6);

		} else {
			kernel_reset_animation(aa[3], 6);
		}
	}


	if (previous_room == 203) {
		player_first_walk(PLAYER_X_FROM_203, PLAYER_Y_FROM_203, FACING_SOUTHWEST,
			WALK_TO_X_FROM_203, WALK_TO_Y_FROM_203, FACING_SOUTHWEST, true);
		camera_jump_to(480, 0);  /* jump to right half */

	} else if ((previous_room == 120 || previous_room == 201 || previous_room != KERNEL_RESTORING_GAME)) {

		if (global[player_persona] == PLAYER_IS_KING) {
			if (global[king_got_stabbed] == 1) {
				player.x = DEATH_X - 20;
				player.y = DEATH_Y;
				player.facing = FACING_EAST;
				global[king_got_stabbed] = false;
				camera_jump_to(480, 0);
				/* from being eaten */

			} else if (global[king_got_stabbed]) {
				player.x = WALK_TO_X_FROM_120 + 5;
				player.y = WALK_TO_Y_FROM_120 + 20;
				player.facing = FACING_SOUTHEAST;
				global[king_got_stabbed] = false;
				/* from being stabbed */

			} else if (global[given_object_before]) {
				player_first_walk(PLAYER_X_FROM_120, PLAYER_Y_FROM_120, FACING_EAST,
					WALK_TO_X_FROM_120, WALK_TO_Y_FROM_120, FACING_NORTHEAST, true);
				player_walk_trigger(ROOM_201_HALT);

			} else {
				player_first_walk(PLAYER_X_FROM_120, PLAYER_Y_FROM_120, FACING_EAST,
					WALK_TO_X_FROM_120, WALK_TO_Y_FROM_120, FACING_EAST, true);
				player_walk_trigger(ROOM_201_HALT);
			}

		} else {
			if (global[guards_are_asleep]) {
				player_first_walk(PLAYER_X_FROM_120, PLAYER_Y_FROM_120, FACING_EAST,
					20, 106, FACING_EAST, true);

			} else {
				player_first_walk(PLAYER_X_FROM_120, PLAYER_Y_FROM_120, FACING_EAST,
					WALK_TO_X_FROM_120, WALK_TO_Y_FROM_120, FACING_NORTHEAST, true);
				player_walk_trigger(ROOM_201_HALT);
			}
		}
	}

	if (global[pid_just_died]) {
		inter_give_to_player(soptus_soporific);
		global[pid_just_died] = false;
	}

	if (global[king_got_stabbed] && global[object_given_201] != -1) {
		global[king_got_stabbed] = false;
		player.x = 57;
		player.y = 123;
		player.facing = FACING_SOUTHEAST;
		local->prevent = true;
	}

	set_lg_rock_position();
	set_md_rock_position();
	set_sm_rock_position();
	set_lg_tree_position();
	set_vine_position();
	set_vine_2_position();
	set_vine_3_position();

	/* kernel_timing_trigger (10, MUSIC); */

	section_2_music();
}

static void handle_anim_guard_left() {
	int guard_left_reset_frame;

	if (kernel_anim[aa[0]].frame != local->guard_left_frame) {
		local->guard_left_frame = kernel_anim[aa[0]].frame;
		guard_left_reset_frame = -1;

		switch (local->guard_left_frame) {

		case 18:  /* end of talk frame */
		case 19:  /* end of talk frame */
		case 20:  /* end of talk frame */

			switch (local->guard_left_action) {
			case TALK:
				guard_left_reset_frame = imath_random(17, 19);
				++local->guard_left_talk_count;
				if (local->guard_left_talk_count > 17) {
					local->guard_left_action = FREEZE;
					local->guard_left_talk_count = 0;
					guard_left_reset_frame = 20; /* make guard shut up */
				}
				break;

			default:
				guard_left_reset_frame = 20; /* make guard shut up */
				break;
			}
			break;

		case 53:  /* guard almost has his hand out */
			player.walker_visible = false;
			seq[fx_take] = kernel_seq_pingpong(ss[fx_take], false, 7, 0, 0, 2);
			kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 6, ROOM_201_GIVE);
			kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_EXPIRE, 0, ROOM_201_GIVE + 1);
			kernel_seq_depth(seq[fx_take], 2);
			kernel_seq_range(seq[fx_take], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_player(seq[fx_take], true);
			break;

		case 54: /* guard has hand out */
			if (local->guard_left_action == GIVE_NOTHING) {
				guard_left_reset_frame = 53;
			}
			break;

		case 61:
			local->pid_action = TAKE_BOTTLE;
			break;

		case 62:  /* guards hand is all the way out */
			if (local->guard_left_action == GIVE_BOTTLE) {
				guard_left_reset_frame = 61;
			}
			break;

		case 106:  /* pid passes bottle to left guard */
			local->pid_action = FREEZE;
			inter_move_object(soptus_soporific, 201);
			break;

		case 102:  /* end of die */
			kernel_abort_animation(aa[0]);
			local->anim_0_running = false;
			seq[fx_guard_1_dead] = kernel_seq_stamp(ss[fx_guard_1_dead], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_guard_1_dead], 13);
			kernel_synch(KERNEL_SERIES, seq[fx_guard_1_dead], KERNEL_NOW, 0);
			break;

		case 1:   /* end of freeze               */
		case 15:  /* end of halt                 */
		case 66:  /* end of give bottle          */
		case 68:  /* end of switch to right hand */
		case 70:  /* end of switch to left hand  */
		case 58:  /* end of give nothing         */
		case 50:  /* end of point                */
		case 24:  /* end of talk                 */
		case 38:  /* end of talk                 */
		case 82:  /* end of take a swig          */
		case 109: /* end of take bottle          */

			switch (local->guard_left_action) {

			case FREEZE:
				guard_left_reset_frame = 0;
				break;

			case TALK:
				if (imath_random(1, 2) == 1) {
					guard_left_reset_frame = 24;
					local->guard_left_action = FREEZE;
				} else {
					guard_left_reset_frame = 15;
				}
				break;

			case POINT:
				guard_left_reset_frame = 38;
				local->guard_left_action = FREEZE;
				break;

			case SWITCH_TO_LEFT:
				guard_left_reset_frame = 68;
				local->guard_left_action = GIVE_BOTTLE;
				break;

			case SWITCH_TO_RIGHT:
				guard_left_reset_frame = 66;
				local->guard_left_action = DRINK;
				break;

			case GIVE_BOTTLE:
				guard_left_reset_frame = 58;
				break;

			case TAKE_BOTTLE:
				guard_left_reset_frame = 102;
				local->guard_left_action = SWITCH_TO_RIGHT;
				break;

			case GIVE_NOTHING:
				guard_left_reset_frame = 50;
				break;

			case HALT:
				guard_left_reset_frame = 1;
				local->guard_left_action = FREEZE;
				break;

			case DRINK:
				guard_left_reset_frame = 70;
				local->guard_left_action = SWITCH_TO_LEFT;
				break;

			case DUMP_ANIMS:
				if (local->anim_1_running) {
					guard_left_reset_frame = 0;

				} else {
					kernel_abort_animation(aa[0]);
					seq[fx_guard_stand_l] = kernel_seq_stamp(ss[fx_guard_stand_l], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_guard_stand_l], 13);
					kernel_synch(KERNEL_SERIES, seq[fx_guard_stand_l], KERNEL_NOW, 0);
					local->anim_0_running = false;
					player.commands_allowed = true;
				}
				break;

			case FALL:
				local->left_ready_to_fall = true;
				guard_left_reset_frame = 0;
				break;
			}
			break;
		}

		if (guard_left_reset_frame >= 0) {
			kernel_reset_animation(aa[0], guard_left_reset_frame);
			local->guard_left_frame = guard_left_reset_frame;
		}
	}
}

static void handle_anim_guard_right() {
	int guard_right_reset_frame;

	if (kernel_anim[aa[1]].frame != local->guard_right_frame) {
		local->guard_right_frame = kernel_anim[aa[1]].frame;
		guard_right_reset_frame = -1;

		switch (local->guard_right_frame) {
		case 50:
			global_speech_go(7); /* hrul */
			break;

		case 57:
			global_speech_go(6); /* stab */
			break;

		case 14:
			local->pid_action = FREEZE;
			inter_move_object(soptus_soporific, 201);
			break;

		case 31:
			inter_give_to_player(soptus_soporific);
			break;

		case 36: /* arm is out to give back bottle */
			local->pid_action = TAKE_BOTTLE_FROM_RIGHT;
			break;

		case 47:  /* just about to get stabbed */
			player.walker_visible = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
			break;

		case 73:
			if (game.difficulty == EASY_MODE) {
				text_show(20164);
			} else {
				text_show(45);
			}
			global[king_got_stabbed] = true;
			kernel.force_restart = true;
			break;

		case 1:   /* end of freeze     */
		case 6:   /* end of halt       */
		case 7:   /* end of freeze     */
		case 37:  /* end of take drink */
		case 80:  /* end of stand up   */

			switch (local->guard_right_action) {

			case HALT:
				guard_right_reset_frame = 1;
				local->guard_right_action = HALT_FREEZE;
				break;

			case FREEZE:
				guard_right_reset_frame = 0;
				break;

			case HALT_FREEZE:
				guard_right_reset_frame = 6;
				break;

			case TAKE_BOTTLE:
				guard_right_reset_frame = 7;
				break;

			case DUMP_ANIMS:
				kernel_abort_animation(aa[1]);
				seq[fx_guard_stand_r] = kernel_seq_stamp(ss[fx_guard_stand_r], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_guard_stand_r], 13);
				kernel_synch(KERNEL_SERIES, seq[fx_guard_stand_r], KERNEL_NOW, 0);
				local->anim_1_running = false;
				break;

			case UNHALT:
			case UNHALT_DUMP_ANIMS:
				guard_right_reset_frame = 73;
				if (local->guard_right_action == UNHALT) {
					local->guard_right_action = FREEZE;
				} else if (local->guard_right_action == UNHALT_DUMP_ANIMS) {
					local->guard_right_action = DUMP_ANIMS;
				}
				break;

			case KILL:
				guard_right_reset_frame = 37;
				break;

			case FALL:
				guard_right_reset_frame = 56;
				local->guard_right_action = FREEZE;
				break;
			}
			break;
		}

		if (guard_right_reset_frame >= 0) {
			kernel_reset_animation(aa[1], guard_right_reset_frame);
			local->guard_right_frame = guard_right_reset_frame;
		}
	}
}

static void handle_anim_guard_right_pid() {
	int guard_right_reset_frame;

	if (kernel_anim[aa[3]].frame != local->guard_right_frame) {
		local->guard_right_frame = kernel_anim[aa[3]].frame;
		guard_right_reset_frame = -1;

		switch (local->guard_right_frame) {
		case 43:
			global_speech_go(7); /* hurl */
			break;

		case 47:
			global_speech_go(6); /* stab */
			break;

		case 38:  /* start stab sequence */
			kernel_abort_animation(aa[2]);
			local->anim_2_running = false;
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
			break;

		case 58:  /* end of death sequence */
			if (game.difficulty == EASY_MODE) {
				text_show(20165);
			} else {
				text_show(45);
			}
			conv_reset(CONV_54_PID);
			kernel.force_restart = true;
			break;

		case 29:
			local->pid_action = TAKE_BOTTLE_FROM_RIGHT;
			break;

		case 30:  /* holding bottle out for pid to take */
			if (local->guard_right_action == GIVE_BOTTLE) {
				guard_right_reset_frame = 29;
			}
			break;

		case 14:  /* just took bottle from pid */
			inter_move_object(soptus_soporific, 201);
			local->pid_action = FREEZE;
			break;

		case 75:  /* end of die */
			kernel_abort_animation(aa[3]);
			seq[fx_guard_2_dead] = kernel_seq_stamp(ss[fx_guard_2_dead], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_guard_2_dead], 13);
			kernel_synch(KERNEL_SERIES, seq[fx_guard_2_dead], KERNEL_NOW, 0);
			local->anim_3_running = false;
			break;

		case 1:   /* end of freeze     */
		case 6:   /* end of halt       */
		case 7:   /* end of freeze     */
		case 37:  /* end of take drink */
		case 82:  /* end of stand up   */

			switch (local->guard_right_action) {

			case HALT:
				guard_right_reset_frame = 1;
				local->guard_right_action = HALT_FREEZE;
				break;

			case FREEZE:
				guard_right_reset_frame = 0;
				break;

			case UNHALT_FALL:
				guard_right_reset_frame = 75;
				local->guard_right_action = FALL;
				break;

			case HALT_FREEZE:
				guard_right_reset_frame = 6;
				break;

			case TAKE_BOTTLE:
				guard_right_reset_frame = 7;
				local->guard_right_action = GIVE_BOTTLE;
				break;

			case UNHALT:
				guard_right_reset_frame = 75;
				local->guard_right_action = FREEZE;
				break;

			case KILL:
				guard_right_reset_frame = 37;
				break;

			case FALL:
				local->right_ready_to_fall = true;
				guard_right_reset_frame = 0;
				break;
			}
			break;
		}

		if (guard_right_reset_frame >= 0) {
			kernel_reset_animation(aa[3], guard_right_reset_frame);
			local->guard_right_frame = guard_right_reset_frame;
		}
	}
}

static void handle_anim_death() {
	int death_reset_frame;

	if (kernel_anim[aa[6]].frame != local->death_frame) {
		local->death_frame = kernel_anim[aa[6]].frame;
		death_reset_frame = -1;

		switch (local->death_frame) {

		case 38:
		case 43:
			if (speech_system_active && speech_on) {
				global_speech_go(3);  /* crunch1 */
			} else {
				sound_play(N_PoolMonsterEatsPid);
			}
			break;

		case 50:
			if (speech_system_active && speech_on) {
				global_speech_go(4);  /* crunch2 */
			} else {
				sound_play(N_PoolMonsterEatsPid);
			}
			break;
		}

		if (death_reset_frame >= 0) {
			kernel_reset_animation(aa[6], death_reset_frame);
			local->death_frame = death_reset_frame;
		}
	}
}

static void handle_anim_pid() {
	int pid_reset_frame;

	if (kernel_anim[aa[2]].frame != local->pid_frame) {
		local->pid_frame = kernel_anim[aa[2]].frame;
		pid_reset_frame = -1;

		switch (local->pid_frame) {

		case 43:  /* almost end of take a swig (pid) */
			conv_release();
			break;

		case 67:
			local->guard_left_action = FREEZE;
			break;

		case 74:
			local->guard_right_action = HALT_FREEZE;
			break;

		case 68:  /* pid's arm is extended - taking bottle from left guard */
			inter_give_to_player(soptus_soporific);
			break;

		case 26:  /* end of heal */
			local->guard_left_action = FALL;
			local->guard_right_action = UNHALT_FALL;
			kernel_abort_animation(aa[2]);
			local->anim_2_running = false;
			player.walker_visible = true;
			player.commands_allowed = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			break;

		case 1:   /* end of freeze          */
		case 2:   /* end of look down       */
		case 50:  /* end of put bottle away */
		case 55:  /* end of give to left    */
		case 64:  /* end of give to right   */

			switch (local->pid_action) {

			case FREEZE:
				pid_reset_frame = 0;
				break;

			case LOOK_DOWN:
				pid_reset_frame = 1;
				break;

			case HEAL:
				pid_reset_frame = 2;
				local->pid_action = FREEZE;
				break;

			case TAKE_OUT_BOTTLE:
				pid_reset_frame = 26;
				local->pid_action = FREEZE;
				break;

			case TAKE_BOTTLE:
				pid_reset_frame = 64;
				local->pid_action = HAND_TO_RIGHT;
				break;

			case FALL:
				pid_reset_frame = 79;
				break;

			case TAKE_BOTTLE_FROM_RIGHT:
				local->pid_action = PUT_AWAY;
				pid_reset_frame = 70;
				break;

			case HAND_TO_LEFT:
			case HAND_TO_RIGHT:
			case DRINK:
				pid_reset_frame = 26;
				break;
			}
			break;

		case 31:  /* almost end of take out bottle */
			conv_run(CONV_54_PID);
			conv_export_value(1);
			break;

		case 52:
			local->guard_left_action = TAKE_BOTTLE;
			break;

		case 53:  /* pid's arm is extended to left guard */
			if (local->pid_action == HAND_TO_LEFT) {
				pid_reset_frame = 52;
			}
			break;

		case 59:
			local->guard_right_action = TAKE_BOTTLE;
			break;

		case 60:  /* pid's arm is extended to right guard */
			if (local->pid_action == HAND_TO_RIGHT) {
				pid_reset_frame = 59;
			}
			break;

		case 97:  /* end of fall */
			if (game.difficulty == EASY_MODE) {
				text_show(20166);
			} else {
				text_show(45);
			}
			global[pid_just_died] = true;
			kernel.force_restart = true;
			conv_reset(CONV_54_PID);
			break;

		case 32:  /* end of take out drink  */
		case 33:  /* end of freeze          */
		case 44:  /* end of drink           */
		case 70:  /* end of take from left  */
		case 79:  /* end of take from right */

			switch (local->pid_action) {

			case FREEZE:
				pid_reset_frame = 32;
				break;

			case DRINK:
				pid_reset_frame = 33;
				local->pid_action = FREEZE;
				break;

			case HAND_TO_LEFT:
				pid_reset_frame = 50;
				break;

			case HAND_TO_RIGHT:
				pid_reset_frame = 55;
				break;

			case PUT_AWAY:
				pid_reset_frame = 44;
				local->pid_action = FREEZE;
				local->death_timer = 0;
				local->clock = 0;
				local->activate_timer = 1;
				player.commands_allowed = true;
				break;

			default:
				pid_reset_frame = 44;
				break;
			}
			break;
		}

		if (pid_reset_frame >= 0) {
			kernel_reset_animation(aa[2], pid_reset_frame);
			local->pid_frame = pid_reset_frame;
		}
	}
}

static void handle_anim_throw() {
	int throw_reset_frame;

	if (kernel_anim[aa[4]].frame != local->throw_frame) {
		local->throw_frame = kernel_anim[aa[4]].frame;
		throw_reset_frame = -1;

		switch (local->throw_frame) {

		case 11:  /* just threw shieldstone */
			inter_move_object(shieldstone, NOWHERE);
			break;
		}

		if (throw_reset_frame >= 0) {
			kernel_reset_animation(aa[4], throw_reset_frame);
			local->throw_frame = throw_reset_frame;
		}
	}
}

static void handle_anim_take() {
	int take_reset_frame;

	if (kernel_anim[aa[5]].frame != local->take_frame) {
		local->take_frame = kernel_anim[aa[5]].frame;
		take_reset_frame = -1;

		switch (local->take_frame) {

		case 6:  /* just took tentacle parts */
			kernel_seq_delete(seq[fx_pair_testes]);
			kernel_flip_hotspot(words_tentacles, false);
			++global[player_score];
			sound_play(N_TakeObjectSnd);
			inter_give_to_player(tentacle_parts);
			object_examine(tentacle_parts, 20146, 0);
			break;
		}

		if (take_reset_frame >= 0) {
			kernel_reset_animation(aa[5], take_reset_frame);
			local->take_frame = take_reset_frame;
		}
	}
}

static void room_201_daemon() {
	long dif;
	int temp;

	if (local->anim_0_running) {
		handle_anim_guard_left();
	}

	if (local->anim_1_running) {
		handle_anim_guard_right();
	}

	if (local->anim_2_running) {
		handle_anim_pid();
	}

	if (local->anim_3_running) {
		handle_anim_guard_right_pid();
	}

	if (local->anim_4_running) {
		handle_anim_throw();
	}

	if (local->anim_5_running) {
		handle_anim_take();
	}

	if (local->anim_6_running) {
		handle_anim_death();
	}

	if (camera_x.pan_this_frame) {
		set_lg_rock_position();
		set_md_rock_position();
		set_sm_rock_position();
		set_lg_tree_position();
		set_vine_position();
		set_vine_2_position();
		set_vine_3_position();
	}

	if (imath_random(1, 300) == 1) {
		if (local->hand_status == HAND_OPEN) {
			seq[fx_hand] = kernel_seq_forward(ss[fx_hand], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_hand], 1);
			kernel_seq_range(seq[fx_hand], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_hand], KERNEL_TRIGGER_EXPIRE, 0, ROOM_201_HAND);
			local->hand_status = HAND_MOVING;

		} else if (local->hand_status == HAND_CLOSED) {
			kernel_seq_delete(seq[fx_hand]);
			seq[fx_hand] = kernel_seq_backward(ss[fx_hand], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_hand], 1);
			kernel_seq_range(seq[fx_hand], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_hand], KERNEL_TRIGGER_EXPIRE, 0, ROOM_201_HAND + 1);
			local->hand_status = HAND_MOVING;
		}
	}

	if (kernel.trigger == ROOM_201_HAND) {
		seq[fx_hand] = kernel_seq_stamp(ss[fx_hand], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_hand], 1);
		local->hand_status = HAND_CLOSED;
	}

	if (kernel.trigger == ROOM_201_HAND + 1) {
		local->hand_status = HAND_OPEN;
	}

	if (kernel.trigger == ROOM_201_HALT) {

		if (global[player_persona] == PLAYER_IS_KING) {
			conv_run(CONV_47_KING);
			conv_export_value(0);
			conv_export_value(0);

		} else {
			player.walker_visible = false;
			aa[2] = kernel_run_animation(kernel_name('p', 1), 0);
			local->anim_2_running = true;
			local->pid_action = FREEZE;
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_PLAYER, 0);

			conv_run(CONV_54_PID);
			conv_export_value(0);
		}
	}

	if (!local->prevent && player.x > 30) {
		local->prevent = true;
		kernel_timing_trigger(HALF_SECOND, ROOM_201_HALT + 1);
		local->guard_right_action = HALT;
	}

	if (kernel.trigger == ROOM_201_HALT + 1) {
		local->guard_left_action = HALT;
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

			if (global[player_persona] == PLAYER_IS_KING) {
				if (conv_control.running != CONV_47_KING) {
					conv_run(CONV_47_KING);
					conv_export_value(1);
					conv_export_value(1);
				}

				local->death_timer = 0;
				local->clock = 0;


			} else {
				if (local->activate_timer == 1) {
					text_show(20157);
					local->activate_timer = false;
					local->pid_action = FALL;
					player.commands_allowed = false;
					kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

				} else {
					local->activate_timer = false;
					conv_run(CONV_54_PID);
					conv_export_value(0);
					conv_export_value(0);
				}
			}
		}
	}

	switch (kernel.trigger) {
	case ROOM_201_GIVE:
		if (local->prevent_1) {
			local->guard_left_action = FREEZE;
			if (global[object_given_201] == -1) {
				inter_move_object(object_named(player_main_noun), 201);
				global[object_given_201] = object_named(player_main_noun);

			} else {
				inter_give_to_player(global[object_given_201]);
				global[object_given_201] = -1;
				sound_play(N_TakeObjectSnd);
			}
		}
		local->prevent_1 = true;
		break;

	case ROOM_201_GIVE + 1:
		player.walker_visible = true;
		local->prevent_1 = false;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take]);

		if (global[object_given_201] != -1) {
			player.commands_allowed = true;

			if (global[given_object_before]) {
				local->guard_left_action = DUMP_ANIMS;
				local->guard_right_action = UNHALT_DUMP_ANIMS;

			} else {
				conv_run(CONV_47_KING);
				conv_export_value(0);
				conv_export_value(1);
			}

		} else {
			player_walk(PLAYER_X_FROM_120, PLAYER_Y_FROM_120, FACING_WEST);
			player.walking = true;

			global[pre_room] = 201;
			if (global[dragon_my_scene] < global[dragon_high_scene]) {
				global[dragon_my_scene]++;
				player.walk_off_edge_to_room = 111;
				local->cut_scene = true;
			} else {
				player.walk_off_edge_to_room = 120;
			}
		}
		break;
	}

	if (local->left_ready_to_fall && local->right_ready_to_fall && !global[guards_are_asleep]) {
		kernel_abort_animation(aa[3]);
		kernel_abort_animation(aa[0]);
		local->anim_3_running = false;
		local->anim_0_running = false;
		global[guards_are_asleep] = true;
		kernel_flip_hotspot(words_guards, true);
		kernel_flip_hotspot_loc(words_guards, false, GUARD_1_X, GUARD_1_Y);
		kernel_flip_hotspot_loc(words_guards, false, GUARD_2_X, GUARD_2_Y);
		sound_play(N_GuardCollapses);

		aa[0] = kernel_run_animation(kernel_name('f', 1), 1);
	}

	if (kernel.trigger == 1) {
		kernel_abort_animation(aa[0]);
		seq[fx_guard_1_dead] = kernel_seq_stamp(ss[fx_guard_1_dead], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_guard_1_dead], 13);
		kernel_synch(KERNEL_SERIES, seq[fx_guard_1_dead], KERNEL_NOW, 0);

		seq[fx_guard_2_dead] = kernel_seq_stamp(ss[fx_guard_2_dead], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_guard_2_dead], 13);
		kernel_synch(KERNEL_SERIES, seq[fx_guard_2_dead], KERNEL_NOW, 0);

		player.commands_allowed = true;
	}

	if (!local->testicle_1_on && !global[dome_up] && imath_random(1, 200) == 1) {
		kernel_flip_hotspot_loc(words_pool_monster, true, 559, 123);
		seq[fx_testicle_1] = kernel_seq_forward(ss[fx_testicle_1], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_testicle_1], 9);
		kernel_seq_range(seq[fx_testicle_1], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_testicle_1], KERNEL_TRIGGER_SPRITE, 9, ROOM_201_SPLASH);
		kernel_seq_trigger(seq[fx_testicle_1], KERNEL_TRIGGER_EXPIRE, 0, 10);
		local->testicle_1_on = true;
	}

	if (!local->testicle_2_on && !global[dome_up] && imath_random(1, 200) == 1) {
		kernel_flip_hotspot_loc(words_pool_monster, true, 708, 109);
		seq[fx_testicle_2] = kernel_seq_forward(ss[fx_testicle_2], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_testicle_2], 9);
		kernel_seq_range(seq[fx_testicle_2], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_testicle_2], KERNEL_TRIGGER_SPRITE, 10, ROOM_201_SPLASH);
		kernel_seq_trigger(seq[fx_testicle_2], KERNEL_TRIGGER_EXPIRE, 0, 11);
		local->testicle_2_on = true;
	}

	if (!local->testicle_3_on && !global[dome_up] && imath_random(1, 200) == 1) {
		kernel_flip_hotspot_loc(words_pool_monster, true, 670, 108);
		seq[fx_testicle_3] = kernel_seq_forward(ss[fx_testicle_3], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_testicle_3], 10);
		kernel_seq_range(seq[fx_testicle_3], KERNEL_FIRST, 4);
		kernel_seq_trigger(seq[fx_testicle_3], KERNEL_TRIGGER_SPRITE, 15, ROOM_201_SPLASH);
		kernel_seq_trigger(seq[fx_testicle_3], KERNEL_TRIGGER_EXPIRE, 0, ROOM_201_TENT_3);
		local->testicle_3_on = true;
	}

	if (!local->testicle_4_on && !global[dome_up] && imath_random(1, 200) == 1) {
		kernel_flip_hotspot_loc(words_pool_monster, true, 615, 109);
		seq[fx_testicle_4] = kernel_seq_forward(ss[fx_testicle_4], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_testicle_4], 10);
		kernel_seq_range(seq[fx_testicle_4], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_testicle_4], KERNEL_TRIGGER_SPRITE, 9, ROOM_201_SPLASH);
		kernel_seq_trigger(seq[fx_testicle_4], KERNEL_TRIGGER_EXPIRE, 0, 13);
		local->testicle_4_on = true;
	}

	switch (kernel.trigger) {
	case 10:
		local->testicle_1_on = false;
		kernel_flip_hotspot_loc(words_pool_monster, false, 559, 123);
		break;

	case 11:
		local->testicle_2_on = false;
		kernel_flip_hotspot_loc(words_pool_monster, false, 708, 109);
		break;

	case 12:
		local->testicle_3_on = false;
		kernel_flip_hotspot_loc(words_pool_monster, false, 670, 108);
		break;

	case 13:
		local->testicle_4_on = false;
		kernel_flip_hotspot_loc(words_pool_monster, false, 615, 109);
		break;
	}

	if (kernel.trigger == ROOM_201_TENT_3) {
		temp = seq[fx_testicle_3];
		if (imath_random(1, 5) == 1) {
			seq[fx_testicle_3] = kernel_seq_pingpong(ss[fx_testicle_3], false, 8, 0, 0, 2);
			kernel_seq_depth(seq[fx_testicle_3], 10);
			kernel_seq_range(seq[fx_testicle_3], 5, 11);
			kernel_seq_trigger(seq[fx_testicle_3], KERNEL_TRIGGER_EXPIRE, 0, ROOM_201_TENT_3);

		} else {
			seq[fx_testicle_3] = kernel_seq_forward(ss[fx_testicle_3], false, 8, 0, 0, 1);
			kernel_seq_depth(seq[fx_testicle_3], 10);
			kernel_seq_range(seq[fx_testicle_3], 6, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_testicle_3], KERNEL_TRIGGER_EXPIRE, 0, 12);
		}
		kernel_synch(KERNEL_SERIES, seq[fx_testicle_3], KERNEL_SERIES, temp);
	}

	if (!local->death_is_on && !global[dome_up] && global[player_persona] == PLAYER_IS_KING) {
		if (player.x >= 620) {
			player_walk(DEATH_X, DEATH_Y, FACING_EAST);
			player_walk_trigger(ROOM_201_DEATH_2);
			local->death_is_on = true;
			player.commands_allowed = false;
		}
	}

	if (kernel.trigger == ROOM_201_DEATH_2) {
		player.walker_visible = false;
		player.commands_allowed = false;
		aa[6] = kernel_run_animation(kernel_name('d', 1), ROOM_201_DEATH_2 + 1);
		local->anim_6_running = true;
		kernel_synch(KERNEL_ANIM, aa[3], KERNEL_PLAYER, 0);
	}

	if (kernel.trigger == ROOM_201_DEATH_2 + 1) {
		if (game.difficulty == EASY_MODE) {
			text_show(20167);
		} else {
			text_show(45);
		}
		global[king_got_stabbed] = 1;  /* 1 is treated differently from true in init code */
		kernel.force_restart = true;
	}

	if (kernel.trigger == ROOM_201_SPLASH) {
		if (player.x > 400) {
			sound_play(N_TentaclesSplash);
		}
	}

	if (player.x > 300) {
		local->crossed_line = true;
	}

	if (kernel.trigger == MUSIC) {
		sound_play(N_BackgroundMus);
	}

	if (local->cut_scene && !player.walking) {
		local->cut_scene = false;
		global[dragon_my_scene]--;
	}
}

static void process_conv_king_guards() {
	int you_trig_flag = false;
	int me_trig_flag = false;

	if (player_verb == conv047_prebribe_giver) {
		if (!kernel.trigger) {
			player_demand_facing(FACING_NORTHEAST);
		}
	}

	if (player_verb == conv047_offer_wasnt ||
		player_verb == conv047_boon3_suppose) {
		if (!kernel.trigger) {
			global[player_score] += 3;
		}
	}

	if (player_verb == conv047_collat2_only) {
		global[given_object_before] = true;
	}

	if (player_verb == conv047_offer_never) {
		local->special_shit = true;
	}

	if (player_verb == conv047_give_b_b) { /* first time around */
		*conv_my_next_start = conv047_postbribe;
		you_trig_flag = true;
		me_trig_flag = true;
		local->activate_timer = true;
		conv_abort();
	}

	if (player_verb == conv047_exit_i_i) {
		*conv_my_next_start = conv047_protect;
		you_trig_flag = true;
		me_trig_flag = true;
		conv_abort();
		player.commands_allowed = false;
		local->guard_right_action = UNHALT;
		player_walk(PLAYER_X_FROM_120, PLAYER_Y_FROM_120, FACING_WEST);
		/* player.walking = true; */

		global[pre_room] = 201;
		if (global[dragon_my_scene] < global[dragon_high_scene]) {
			global[dragon_my_scene]++;
			player.walk_off_edge_to_room = 111;
			/* local->cut_scene = true; */
		} else {
			player.walk_off_edge_to_room = 120;
		}
	}

	if (player_verb == conv047_give_d_d) {
		*conv_my_next_start = conv047_usesame;
		you_trig_flag = true;
		me_trig_flag = true;
		local->activate_timer = true;
		conv_abort();
	}

	if (player_verb == conv047_kill_b_b) {
		*conv_my_next_start = conv047_heythere;
		conv_abort();
		you_trig_flag = true;
		me_trig_flag = true;
		local->guard_right_action = KILL;
		player.commands_allowed = false;
	}

	if (player_verb == conv047_exit_b_b) {
		*conv_my_next_start = conv047_protect;
		local->guard_left_action = GIVE_NOTHING;
		you_trig_flag = true;
		me_trig_flag = true;
		conv_abort();
		player.commands_allowed = false;
		local->guard_right_action = UNHALT;
	}

	if (player_verb == conv047_exit_k_k) {
		*conv_my_next_start = conv047_protect;
		you_trig_flag = true;
		me_trig_flag = true;
		conv_abort();
		player_walk(PLAYER_X_FROM_120, PLAYER_Y_FROM_120, FACING_WEST);
		/* player.walking = true; */

		global[pre_room] = 201;
		if (global[dragon_my_scene] < global[dragon_high_scene]) {
			global[dragon_my_scene]++;
			player.walk_off_edge_to_room = 111;
			/* local->cut_scene = true; */
		} else {
			player.walk_off_edge_to_room = 120;
		}

		player.commands_allowed = false;
		local->guard_right_action = UNHALT;
		global[reset_conv] = 47;
	}

	if (player_verb == conv047_exit_d_d) {
		*conv_my_next_start = conv047_heythere;
		you_trig_flag = true;
		me_trig_flag = true;
		conv_abort();
		player.commands_allowed = false;
		local->guard_left_action = DUMP_ANIMS;
		local->guard_right_action = UNHALT_DUMP_ANIMS;
	}

	if (player_verb == conv047_exit_f_f) {
		*conv_my_next_start = conv047_askback;
		you_trig_flag = true;
		me_trig_flag = true;
		conv_abort();
		player.commands_allowed = false;
		local->guard_left_action = DUMP_ANIMS;
		/* local->guard_right_action = UNHALT_DUMP_ANIMS; */
		local->guard_right_action = DUMP_ANIMS;
	}

	if (player_verb == conv047_take_back_b_b) {
		*conv_my_next_start = conv047_protect;
		local->guard_left_action = GIVE_NOTHING;
		you_trig_flag = true;
		me_trig_flag = true;
		conv_abort();
		player.commands_allowed = false;
	}

	if (player_verb == conv047_take_back_d_d) {
		*conv_my_next_start = conv047_usesame;
		conv_abort();
		local->guard_left_action = GIVE_NOTHING;
		you_trig_flag = true;
		me_trig_flag = true;
		player.commands_allowed = false;
	}

	if (kernel.trigger == ROOM_201_YOU_TALK) {
		if (player_verb != conv047_kingsay_escort &&
			player_verb != conv047_kingsay_why &&
			player_verb != conv047_myguys_whydirt &&
			player_verb != conv047_prebribe_giver &&
			player_verb != conv047_reaction2_only &&
			player_verb != conv047_offer_boon &&
			player_verb != conv047_boon2_only &&
			player_verb != conv047_collat2_only &&
			player_verb != conv047_askback_heythere &&
			player_verb != conv047_heythere_only) {
			local->guard_left_action = TALK;
		}
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_201_YOU_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */
}

static void process_conv_pid_guards() {
	int you_trig_flag = false;
	int me_trig_flag = false;

	if (player_verb == conv054_death_b_b) {
		conv_abort();
		you_trig_flag = true;
		me_trig_flag = true;
		local->guard_right_action = KILL;
		player.commands_allowed = false;
	}

	if (player_verb == conv054_show_drink_b_b) {
		local->death_timer = 0;
		local->clock = 0;
		you_trig_flag = true;
		me_trig_flag = true;
		local->activate_timer = true;
	}

	if (player_verb == conv054_take_swig_b_b) {
		conv_hold();
		local->pid_action = DRINK;
		you_trig_flag = true;
		me_trig_flag = true;
	}

	if (player_verb == conv054_offer_b_b) {
		local->pid_action = HAND_TO_LEFT;
		you_trig_flag = true;
		me_trig_flag = true;
		conv_abort();
		player.commands_allowed = false;
	}

	if (player_verb == conv054_exit_b_b) {
		*conv_my_next_start = conv054_restart;
		you_trig_flag = true;
		me_trig_flag = true;
		conv_abort();
		kernel_abort_animation(aa[2]);

		player.walker_visible = true;
		player.commands_allowed = false;
		local->guard_right_action = UNHALT;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player_walk(PLAYER_X_FROM_120, PLAYER_Y_FROM_120, FACING_WEST);
		/* player.walking = true; */

		global[pre_room] = 201;
		if (global[dragon_my_scene] < global[dragon_high_scene]) {
			global[dragon_my_scene]++;
			player.walk_off_edge_to_room = 111;
			/* local->cut_scene = true; */
		} else {
			player.walk_off_edge_to_room = 120;
		}
	}

	if (kernel.trigger == ROOM_201_YOU_TALK) {
		local->guard_left_action = TALK;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_201_YOU_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */
}

static void room_201_pre_parser() {
	if (local->activate_timer) {
		player.need_to_walk = false;
	}

	if (player_said_2(talk_to, guards)) {
		if (global[player_persona] == PLAYER_IS_KING && !local->activate_timer) {
			player_walk(WALK_TO_X_FROM_120, WALK_TO_Y_FROM_120, FACING_NORTHEAST);
		}
	}

	if (player_said_3(throw, shieldstone, pool) ||
		player_said_3(put, shieldstone, pool) ||
		player_said_3(throw, shieldstone, pool_monster) ||
		player_said_3(put, shieldstone, pool_monster)) {
		player_walk(THROW_X, THROW_Y, FACING_NORTH);
	}

	if (player_said_2(walk_down, path_to_west)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			if (global[object_given_201] == -1) {
				/* conv_reset (CONV_47_KING); */
				global[reset_conv] = 47;

				player_walk(PLAYER_X_FROM_120, PLAYER_Y_FROM_120, FACING_EAST);
				/* player.walking = true; */

				player.commands_allowed = false;

				global[pre_room] = 201;
				if (global[dragon_my_scene] < global[dragon_high_scene]) {
					if (!local->cut_scene) {
						global[dragon_my_scene]++;
					}
					player.walk_off_edge_to_room = 111;
					/* local->cut_scene = true; */

				} else if (local->cut_scene) {
					player.walk_off_edge_to_room = 111;

				} else {
					player.walk_off_edge_to_room = 120;
				}

			} else {
				if (global[player_persona] == PLAYER_IS_KING && !local->activate_timer) {
					player_walk(WALK_TO_X_FROM_120, WALK_TO_Y_FROM_120, FACING_NORTHEAST);

				} else {
					player_walk(WALK_TO_X_FROM_120, WALK_TO_Y_FROM_120, FACING_EAST);
				}
			}

		} else {

			if (local->activate_timer == 1) {
				text_show(20162);
				player_cancel_command();
				/* you need to heal yourself */

			} else {
				global[pre_room] = 201;
				if (global[dragon_my_scene] < global[dragon_high_scene]) {
					if (!local->cut_scene) {
						global[dragon_my_scene]++;
					}
					player.walk_off_edge_to_room = 111;
					local->cut_scene = true;

				} else if (local->cut_scene) {
					player.walk_off_edge_to_room = 111;

				} else {
					player.walk_off_edge_to_room = 120;
				}
			}
		}
	}

	if (player_said_2(walk_down, path_to_east)) {
		player.walk_off_edge_to_room = 203;
	}
}

static void room_201_parser() {
	int id;

	if (conv_control.running == CONV_47_KING) {
		process_conv_king_guards();
		goto handled;
	}

	if (conv_control.running == CONV_54_PID) {
		process_conv_pid_guards();
		goto handled;
	}

	if (player_said_2(take, tentacles)) {
		switch (kernel.trigger) {
		case 0:
			if (!player_has(tentacle_parts)) {
				aa[5] = kernel_run_animation(kernel_name('b', 1), 1);
				local->anim_5_running = true;
				player.commands_allowed = false;
				player.walker_visible = false;
				kernel_synch(KERNEL_ANIM, aa[5], KERNEL_PLAYER, 0);
			}
			break;

		case 1:
			local->anim_5_running = false;
			player.commands_allowed = true;
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[5]);
			break;
		}
		goto handled;
	}

	if (player_said_3(throw, shieldstone, pool) ||
		player_said_3(put, shieldstone, pool) ||
		player_said_3(throw, shieldstone, pool_monster) ||
		player_said_3(put, shieldstone, pool_monster)) {
		switch (kernel.trigger) {
		case 0:
			aa[4] = kernel_run_animation(kernel_name('t', 1), 1);
			local->anim_4_running = true;
			player.commands_allowed = false;
			player.walker_visible = false;
			++global[dragon_high_scene];
			kernel_synch(KERNEL_ANIM, aa[4], KERNEL_PLAYER, 0);
			break;

		case 1:
			seq[fx_ripple] = kernel_seq_forward(ss[fx_ripple], false, 20, 0, 0, 0);
			kernel_seq_depth(seq[fx_ripple], 3);
			kernel_seq_range(seq[fx_ripple], KERNEL_FIRST, KERNEL_LAST);
			kernel_synch(KERNEL_SERIES, seq[fx_ripple], KERNEL_ANIM, aa[4]);

			seq[fx_pair_testes] = kernel_seq_forward(ss[fx_pair_testes], false, 20, 0, 0, 0);
			kernel_seq_depth(seq[fx_pair_testes], 10);
			kernel_seq_range(seq[fx_pair_testes], KERNEL_FIRST, KERNEL_LAST);
			kernel_synch(KERNEL_SERIES, seq[fx_pair_testes], KERNEL_ANIM, aa[4]);
			kernel_flip_hotspot(words_tentacles, true);

			local->anim_4_running = false;
			player.commands_allowed = true;
			player.walker_visible = true;
			global[dome_up] = true;
			kernel_flip_hotspot(words_spell_shield, true);
			global[player_score] += 4;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[4]);

			if (game.difficulty == HARD_MODE) {
				text_show(20138);
			} else {
				text_show(20137);
			}
			break;
		}
		goto handled;
	}

	if (player_said_2(walk_down, path_to_west)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			kernel_seq_delete(seq[fx_guard_stand_l]);
			aa[0] = kernel_run_animation(kernel_name('u', 1), 0);
			local->anim_0_running = true;
			local->guard_left_action = FREEZE;

			kernel_seq_delete(seq[fx_guard_stand_r]);
			aa[1] = kernel_run_animation(kernel_name('u', 3), 0);
			local->anim_1_running = true;
			local->guard_right_action = FREEZE;
			kernel_reset_animation(aa[1], 7);

			if (local->crossed_line) {
				local->guard_right_action = KILL;
				player.commands_allowed = false;

			} else {
				conv_run(CONV_47_KING);
				conv_export_value(0);
				conv_export_value(1);
			}
			local->crossed_line = false;
			goto handled;
		}
	}

	if (player_said_1(heal_self) && !global[guards_are_asleep] &&
		global[player_persona] == PLAYER_IS_PID) {
		if (local->activate_timer == 1) {
			local->pid_action = HEAL;
			player.commands_allowed = false;
			local->activate_timer = false;
			global[player_score] += 2;
			goto handled;
		}
	}

	if (player_said_2(talk_to, guards)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			if (local->activate_timer) {
				text_show(20159);

			} else {
				kernel_seq_delete(seq[fx_guard_stand_l]);
				aa[0] = kernel_run_animation(kernel_name('u', 1), 0);
				local->anim_0_running = true;
				local->guard_left_action = FREEZE;

				kernel_seq_delete(seq[fx_guard_stand_r]);
				aa[1] = kernel_run_animation(kernel_name('u', 3), 0);
				local->anim_1_running = true;
				local->guard_right_action = FREEZE;
				kernel_reset_animation(aa[1], 7);

				conv_run(CONV_47_KING);
				conv_export_value(0);
				conv_export_value(1);
			}
			goto handled;

		} if (global[guards_are_asleep]) {
			text_show(20152);
			goto handled;
		}
	}

	id = object_named(player_main_noun);

	if (player_said_2(give, guards) && local->activate_timer && player_has(id)) {
		local->activate_timer = false;

		if (global[player_persona] == PLAYER_IS_KING) {
			player.commands_allowed = false;
			local->guard_left_action = GIVE_NOTHING;

			/* if (global[given_object_before]) { */
			  /* local->guard_right_action = UNHALT; */
			/* } */

		} else {
			if (player_said_1(soporific)) {
				local->pid_action = TAKE_OUT_BOTTLE;

			} else {
				conv_run(CONV_54_PID);
				conv_export_value(0);
			}
		}
		goto handled;
	}

	if (player.look_around) {
		text_show(20101);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(path)) {
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(20148);
			} else {
				text_show(20102);
			}
			goto handled;
		}

		if (player_said_1(guardhouse)) {
			text_show(20105);
			goto handled;
		}

		if (player_said_1(guards)) {
			if (global[guards_are_asleep]) {
				text_show(20152);
			} else {
				text_show(20110);
			}
			goto handled;
		}

		if (player_said_1(Slathan_ni_Patan)) {
			text_show(20114);
			goto handled;
		}

		if (player_said_1(path_to_west)) {
			text_show(20116);
			goto handled;
		}

		if (player_said_1(ear_rock)) {
			text_show(20117);
			goto handled;
		}

		if (player_said_1(nose_rock)) {
			text_show(20122);
			goto handled;
		}

		if (player_said_1(mouth_rock)) {
			text_show(20125);
			goto handled;
		}

		if (player_said_1(bone_tree)) {
			text_show(20130);
			goto handled;
		}

		if (player_said_1(pool)) {
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(20149);
			} else if (global[dome_up]) {
				text_show(20139);
			} else {
				text_show(20134);
			}
			goto handled;
		}

		if (player_said_1(path_to_east)) {
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(20151);
			} else {
				text_show(20140);
			}
			goto handled;
		}

		if (player_said_1(pool_monster)) {
			if (global[dome_up]) {
				text_show(20139);
			} else {
				text_show(20141);
			}
			goto handled;
		}

		if (player_said_1(tentacles) && object_is_here(tentacle_parts) && global[dome_up]) {
			text_show(20145);
			goto handled;
		}

		if (player_said_1(spell_shield)) {
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(20150);
			} else {
				text_show(20147);
			}
			goto handled;
		}
	}

	if (player_said_2(put, path)) {
		text_show(20103);
		goto handled;
	}

	if (player_said_2(talk_to, path) ||
		player_said_2(give, path) ||
		player_said_2(throw, path)) {
		text_show(20104);
		goto handled;
	}

	if (player_said_2(open, guardhouse) ||
		player_said_2(close, guardhouse)) {
		text_show(20106);
		goto handled;
	}

	if (player_said_2(push, guardhouse) ||
		player_said_2(pull, guardhouse) ||
		player_said_2(throw, guardhouse)) {
		text_show(20107);
		goto handled;
	}

	if (player_said_2(put, guardhouse) ||
		player_said_2(give, guardhouse)) {
		text_show(20108);
		goto handled;
	}

	if (player_said_2(talk_to, guardhouse)) {
		text_show(20109);
		goto handled;
	}

	if (player_said_2(push, guards) ||
		player_said_2(pull, guards)) {
		if (global[guards_are_asleep]) {
			text_show(20155);
			goto handled;
		}
	}

	if (player_said_3(sword, attack, guards) ||
		player_said_3(sword, carve_up, guards) ||
		player_said_3(sword, thrust, guards) ||
		player_said_2(pour_contents_of, guards) ||
		player_said_2(throw, guards)) {

		if (global[guards_are_asleep]) {
			text_show(20156);
			goto handled;
		}
	}

	if (player_said_3(sword, attack, guards) ||
		player_said_3(sword, carve_up, guards) ||
		player_said_3(sword, thrust, guards) ||
		player_said_2(push, guards) ||
		player_said_2(pull, guards)) {
		text_show(20111);
		goto handled;
	}

	if (player_said_2(give, guards)) {
		if (global[object_given_201] != -1) {
			text_show(20112);
			goto handled;

		} else if (global[guards_are_asleep]) {
			text_show(20154);
			goto handled;
		}
	}


	if (player_said_1(take) ||
		player_said_1(give) ||
		player_said_1(push) ||
		player_said_1(pull) ||
		player_said_1(talk_to) ||
		player_said_1(open) ||
		player_said_1(close) ||
		player_said_1(walk_down) ||
		player_said_1(put) ||
		player_said_1(throw)) {
		if (local->activate_timer) {
			text_show(20113);
			goto handled;

		} else if (local->activate_timer == 1) {
			text_show(20153);
			goto handled;
		}
	}

	if (player_said_3(throw, shieldstone, guards) ||
		player_said_3(throw, teleportal, guards) ||
		player_said_2(pour_contents_of, guards)) {
		text_show(20115);
		goto handled;
	}

	if (player_said_2(take, ear_rock) ||
		player_said_2(take, nose_rock) ||
		player_said_2(take, mouth_rock)) {
		text_show(20118);
		goto handled;
	}

	if (player_said_2(talk_to, ear_rock)) {
		text_show(20119);
		goto handled;
	}


	if (player_said_1(give) && !player_said_1(fruit) && !player_said_1(bone)) {
		if (player_said_1(nose_rock) ||
			player_said_1(ear_rock) ||
			player_said_1(mouth_rock)) {
			text_show(20120);
			goto handled;
		}
	}

	if (player_said_2(make_noise, birdcall)) {
		text_show(20121);
		goto handled;
	}

	if (player_said_2(talk_to, nose_rock)) {
		text_show(20123);
		goto handled;
	}

	if (player_said_1(give) && player_said_1(nose_rock)) {
		if (player_said_1(bone) || player_said_1(fruit)) {
			text_show(20124);
			goto handled;
		}
	}

	if (player_said_2(close, mouth_rock)) {
		text_show(20126);
		goto handled;
	}

	if (player_said_1(put) && !player_said_1(fruit) && !player_said_1(bone)) {
		if (player_said_1(mouth_rock)) {
			text_show(20127);
			goto handled;
		}
	}

	if (player_said_1(put) || player_said_1(give)) {
		if (player_said_1(fruit) || player_said_1(bone)) {
			if (player_said_1(mouth_rock)) {
				text_show(20128);
				goto handled;
			}
		}
	}

	if (player_said_2(talk_to, mouth_rock)) {
		sound_play(N_MouthRockTalks);
		text_show(20129);
		goto handled;
	}

	if (player_said_2(talk_to, bone_tree) ||
		player_said_2(give, bone_tree)) {
		if (!player_said_1(bone)) {
			text_show(20131);
			goto handled;
		}
	}

	if (player_said_2(pull, bone_tree)) {
		text_show(20132);
		goto handled;
	}

	if (player_said_1(put) || player_said_1(give)) {
		if (player_said_1(bone) && player_said_1(bone_tree)) {
			text_show(20133);
			goto handled;
		}
	}

	if (player_said_2(heal, guards)) {
		text_show(20155);
		goto handled;
	}

	if (player_said_2(invoke, signet_ring) && !global[guards_are_asleep]) {
		if (local->activate_timer == 1 && global[player_persona] == PLAYER_IS_PID) {
			text_show(20163);
			/* waystation can't help you now */

		} else if (global[object_given_201] == -1) {
			text_show(20113);
			/* they are looking at you to fulfill your promise */

		} else {
			text_show(20158);
			/* you would confuse the guards */
		}
		goto handled;
	}

	if (player_said_3(put, flask_of_acid, pool) ||
		player_said_3(throw, flask_of_acid, pool) ||
		player_said_2(pour_contents_of, pool)) {
		text_show(20136);
		goto handled;
	}

	if ((player_said_1(put) || player_said_1(throw)) && player_said_1(pool)) {
		if (!player_said_1(shieldstone) && !player_said_1(pour_contents_of)) {
			text_show(20135);
			goto handled;
		}
	}

	if (player_said_3(sword, attack, pool_monster) ||
		player_said_3(sword, carve_up, pool_monster) ||
		player_said_3(sword, thrust, pool_monster) ||
		player_said_2(push, pool_monster) ||
		player_said_2(pull, pool_monster) ||
		player_said_2(take, pool_monster)) {
		text_show(20142);
		goto handled;
	}

	if (player_said_2(talk_to, pool_monster)) {
		text_show(20143);
		goto handled;
	}

	if (player_said_2(take_magic_from, pool_monster)) {
		text_show(20144);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_201_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.lg_rock_base);
	s.syncAsSint16LE(scratch.dyn_lg_rock);
	s.syncAsSint16LE(scratch.md_rock_base);
	s.syncAsSint16LE(scratch.dyn_md_rock);
	s.syncAsSint16LE(scratch.sm_rock_base);
	s.syncAsSint16LE(scratch.dyn_sm_rock);
	s.syncAsSint16LE(scratch.lg_tree_base);
	s.syncAsSint16LE(scratch.dyn_lg_tree);
	s.syncAsSint16LE(scratch.vine_base);
	s.syncAsSint16LE(scratch.dyn_vine);
	s.syncAsSint16LE(scratch.mount_left_base);
	s.syncAsSint16LE(scratch.dyn_mount_left);
	s.syncAsSint16LE(scratch.mount_right_base);
	s.syncAsSint16LE(scratch.dyn_mount_right);
	s.syncAsSint16LE(scratch.vine_2_base);
	s.syncAsSint16LE(scratch.dyn_vine_2);
	s.syncAsSint16LE(scratch.vine_3_base);
	s.syncAsSint16LE(scratch.dyn_vine_3);
	s.syncAsSint16LE(scratch.num);
	s.syncAsSint16LE(scratch.hand_status);
	s.syncAsSint16LE(scratch.guard_left_frame);
	s.syncAsSint16LE(scratch.guard_left_action);
	s.syncAsSint16LE(scratch.guard_left_talk_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.guard_right_frame);
	s.syncAsSint16LE(scratch.guard_right_action);
	s.syncAsSint16LE(scratch.guard_right_talk_count);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.anim_3_running);
	s.syncAsSint16LE(scratch.pid_frame);
	s.syncAsSint16LE(scratch.pid_action);
	s.syncAsSint16LE(scratch.pid_talk_count);
	s.syncAsSint16LE(scratch.anim_2_running);
	s.syncAsSint16LE(scratch.throw_frame);
	s.syncAsSint16LE(scratch.anim_4_running);
	s.syncAsSint16LE(scratch.take_frame);
	s.syncAsSint16LE(scratch.anim_5_running);
	s.syncAsSint16LE(scratch.death_frame);
	s.syncAsSint16LE(scratch.anim_6_running);
	s.syncAsSint16LE(scratch.prevent);
	s.syncAsSint16LE(scratch.prevent_1);
	s.syncAsSint32LE(scratch.clock);
	s.syncAsSint32LE(scratch.death_timer);
	s.syncAsSint16LE(scratch.activate_timer);
	s.syncAsSint16LE(scratch.left_ready_to_fall);
	s.syncAsSint16LE(scratch.right_ready_to_fall);
	s.syncAsSint16LE(scratch.testicle_1_on);
	s.syncAsSint16LE(scratch.testicle_2_on);
	s.syncAsSint16LE(scratch.testicle_3_on);
	s.syncAsSint16LE(scratch.testicle_4_on);
	s.syncAsSint16LE(scratch.death_is_on);
	s.syncAsSint16LE(scratch.special_shit);
	s.syncAsSint16LE(scratch.crossed_line);
	s.syncAsByte(scratch.cut_scene);
}

void room_201_preload() {
	room_init_code_pointer = room_201_init;
	room_pre_parser_code_pointer = room_201_pre_parser;
	room_parser_code_pointer = room_201_parser;
	room_daemon_code_pointer = room_201_daemon;

	section_2_walker();
	section_2_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
