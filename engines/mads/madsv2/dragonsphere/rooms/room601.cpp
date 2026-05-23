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
#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/quotes.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section6.h"
#include "mads/madsv2/dragonsphere/rooms/room601.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[7];

	int16 mount_left_base;
	int16 dyn_mount_left;

	int16 sky_mid_left_base;
	int16 dyn_sky_mid_left;

	int16 mount_right_base;
	int16 dyn_mount_right;

	int16 sky_mid_bot_right_base;
	int16 dyn_sky_mid_bot_right;

	int16 rock_base;
	int16 dyn_rock;

	int16 panning;
	int16 prevent;
	int16 prevent_1;
	int16 vine_talk;

	int32 update_clock;

	int16 king_0_frame;
	int16 anim_0_running;

	int16 king_1_frame;
	int16 king_1_action;
	int16 king_1_talk_count;
	int16 anim_1_running;

	int16 vine_1_frame;
	int16 vine_1_action;
	int16 vine_1_talk_count;
	int16 anim_2_running;

	int16 vine_2_frame;
	int16 vine_2_action;
	int16 vine_2_talk_count;
	int16 anim_3_running;

	int16 vine_3_frame;
	int16 vine_3_action;
	int16 vine_3_talk_count;
	int16 anim_4_running;

	int16 vine_4_frame;
	int16 vine_4_action;
	int16 vine_4_talk_count;
	int16 anim_5_running;

	int16 king_2_frame;
	int16 king_2_action;
	int16 king_2_talk_count;
	int16 anim_6_running;

	int16 door_is_open;
};

#define local ((Scratch *)(&game.scratch[0]))
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_single_morph         0
#define fx_mount_left           1
#define fx_sky_mid_left         2
#define fx_mount_right          3
#define fx_sky_mid_bot_right    4
#define fx_vine_block           5
#define fx_door                 6
#define fx_morph                7
#define fx_rock                 8

#define ROOM_601_COMMANDS_ALLOWED  60
#define ROOM_601_RUN_VINE_ANIM     70
#define ROOM_601_YOU_TALK          80
#define ROOM_601_ME_TALK           82
#define ROOM_601_DOOR_OPENS        84
#define ROOM_601_DOOR_CLOSES       90
#define MUSIC                      100

#define PLAYER_X_FROM_508       -15
#define PLAYER_Y_FROM_508       140
#define WALK_TO_X_FROM_508      20
#define WALK_TO_Y_FROM_508      140

#define PLAYER_X_FROM_603       480
#define PLAYER_Y_FROM_603       127
#define WALK_TO_X_FROM_603      480
#define WALK_TO_Y_FROM_603      140

#define camera_ratio_1          1
#define camera_ratio_2          3

#define CAMERA_MIDDLE           326
#define CAMERA_LEFT             0
#define CAMERA_RIGHT            640

#define WALK_TO_RIGHT_X_1       783
#define WALK_TO_RIGHT_Y_1       101
#define WALK_TO_RIGHT_X_2       805
#define WALK_TO_RIGHT_Y_2       117

#define WALK_TO_LEFT_X_1        161
#define WALK_TO_LEFT_Y_1        101
#define WALK_TO_LEFT_X_2        136
#define WALK_TO_LEFT_Y_2        120

#define PANNING_RIGHT           1
#define PANNING_LEFT            2

#define TOWER_DOOR_X            465
#define TOWER_DOOR_Y            140

#define ROCK_BLOCK_X            404
#define ROCK_BLOCK_Y            149
#define ROCK_X                  438
#define ROCK_Y                  147

#define WALK_VINE_LEFT_X        380
#define WALK_VINE_LEFT_Y        148

#define WALK_VINE_RIGHT_X       425
#define WALK_VINE_RIGHT_Y       146

#define NOTHING                 0
#define TALK                    1
#define DROP                    2
#define STRUGGLE                3
#define DIE                     4

#define CAUGHT_X                549
#define CAUGHT_Y                136

#define CONV36_VINE             36

#define SECOND_SET              1
#define THIRD_SET               2

#define RESTART_X               630
#define RESTART_Y               141


static void clear_all() {
	local->vine_1_talk_count = 0;
	local->vine_2_talk_count = 0;
	local->vine_3_talk_count = 0;
	local->vine_4_talk_count = 0;

	local->vine_1_action = NOTHING;
	local->vine_2_action = NOTHING;
	local->vine_3_action = NOTHING;
	local->vine_4_action = NOTHING;
}

static void handle_animation_king_0() {
	int16 king_0_reset_frame;

	if (kernel_anim[aa[0]].frame != local->king_0_frame) {
		local->king_0_frame = kernel_anim[aa[0]].frame;
		king_0_reset_frame = -1;

		switch (local->king_0_frame) {
		case 71:
			kernel_abort_animation(aa[0]);

			aa[2]                 = kernel_run_animation(kernel_name('t', 1), 0);
			local->anim_2_running = true;
			local->vine_1_action  = TALK;

			aa[3]                 = kernel_run_animation(kernel_name('t', 2), 0);
			local->anim_3_running = true;
			local->vine_2_action  = NOTHING;

			aa[4]                 = kernel_run_animation(kernel_name('t', 3), 0);
			local->anim_4_running = true;
			local->vine_3_action  = NOTHING;

			aa[5]                 = kernel_run_animation(kernel_name('t', 4), 0);
			local->anim_5_running = true;
			local->vine_4_action  = NOTHING;

			local->anim_0_running = false;
			aa[1]                 = kernel_run_animation(kernel_name('v', 2), 0);
			local->anim_1_running = true;
			local->king_1_action  = STRUGGLE;
			king_0_reset_frame    = -1;
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
			kernel_reset_animation(aa[1], 71);
			kernel_timing_trigger(ONE_SECOND, 1);
			break;
		}

		if (king_0_reset_frame >= 0) {
			kernel_reset_animation(aa[0], king_0_reset_frame);
			local->king_0_frame = king_0_reset_frame;
		}
	}
}

static void handle_animation_king_1() {
	int16 king_1_reset_frame;

	if (kernel_anim[aa[1]].frame != local->king_1_frame) {
		local->king_1_frame = kernel_anim[aa[1]].frame;
		king_1_reset_frame = -1;

		switch (local->king_1_frame) {
		case 71:
		case 72:
		case 73:
		case 74:
		case 75:
		case 76:
		case 77:
			switch (local->king_1_action) {
			case STRUGGLE:
				king_1_reset_frame = imath_random(70, 73);
				break;

			case DROP:
				kernel_abort_animation(aa[1]);
				local->anim_1_running = false;

				kernel_abort_animation(aa[5]);
				local->anim_5_running = false;
				kernel_abort_animation(aa[4]);
				local->anim_4_running = false;
				kernel_abort_animation(aa[3]);
				local->anim_3_running = false;
				kernel_abort_animation(aa[2]);
				local->anim_2_running = false;

				global[vine_will_grab] = false;
				global[player_score]  += 3;
				local->anim_6_running  = true;
				king_1_reset_frame     = -1;
				aa[6]                  = kernel_run_animation(kernel_name('v', 3), 0);
				kernel_synch(KERNEL_ANIM, aa[6], KERNEL_NOW, 0);
				kernel_reset_animation(aa[6], 18);
				camera_x.pan_mode = CAMERA_PLAYER;
				break;

			case DIE:
				kernel_abort_animation(aa[1]);
				local->anim_1_running = false;
				aa[6]                 = kernel_run_animation(kernel_name('v', 3), 0);
				local->anim_6_running = true;
				king_1_reset_frame    = -1;
				kernel_synch(KERNEL_ANIM, aa[6], KERNEL_NOW, 0);
				break;

			case TALK:
				king_1_reset_frame = imath_random(74, 76);
				++local->king_1_talk_count;
				if (local->king_1_talk_count > 10) {
					local->king_1_action     = STRUGGLE;
					local->king_1_talk_count = 0;
					king_1_reset_frame       = 72;
				}
				break;
			}
			break;
		}

		if (king_1_reset_frame >= 0) {
			kernel_reset_animation(aa[1], king_1_reset_frame);
			local->king_1_frame = king_1_reset_frame;
		}
	}
}

static void handle_animation_king_2() {
	int16 king_2_reset_frame;

	if (kernel_anim[aa[6]].frame != local->king_2_frame) {
		local->king_2_frame = kernel_anim[aa[6]].frame;
		king_2_reset_frame = -1;

		switch (local->king_2_frame) {
		case 10:
			global_speech_go(2);
			break;

		case 17:
			if (game.difficulty == EASY_MODE) {
				text_show(60118);
			} else {
				text_show(45);
			}
			kernel.force_restart = true;
			global[vines_have_player] = false;
			break;

		case 39:
			sound_play(N_BackgroundMus);
			global[vines_have_player] = false;
			kernel_abort_animation(aa[6]);
			local->anim_6_running   = false;
			player.commands_allowed = true;
			player.walker_visible   = true;
			local->door_is_open     = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			sound_play(N_CreakyCastleDoor);
			kernel_seq_delete(seq[fx_door]);
			seq[fx_door] = kernel_seq_forward(ss[fx_door], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 2);
			kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_601_DOOR_OPENS);
			break;
		}

		if (king_2_reset_frame >= 0) {
			kernel_reset_animation(aa[6], king_2_reset_frame);
			local->king_2_frame = king_2_reset_frame;
		}
	}
}

static void handle_animation_vine_1() {
	int16 vine_1_reset_frame;

	if (kernel_anim[aa[2]].frame != local->vine_1_frame) {
		local->vine_1_frame = kernel_anim[aa[2]].frame;
		vine_1_reset_frame = -1;

		if (local->vine_1_action == TALK) {
			++local->vine_1_talk_count;
			if (local->vine_1_talk_count >= imath_random(30, 35)) {
				local->vine_1_action = NOTHING;
				vine_1_reset_frame   = 0;
			} else {
				vine_1_reset_frame = imath_random(1, 5);
			}
		} else {
			vine_1_reset_frame = 0;
		}

		if (vine_1_reset_frame >= 0) {
			kernel_reset_animation(aa[2], vine_1_reset_frame);
			local->vine_1_frame = vine_1_reset_frame;
		}
	}
}

static void handle_animation_vine_2() {
	int16 vine_2_reset_frame;

	if (kernel_anim[aa[3]].frame != local->vine_2_frame) {
		local->vine_2_frame = kernel_anim[aa[3]].frame;
		vine_2_reset_frame = -1;

		if (local->vine_2_action == TALK) {
			++local->vine_2_talk_count;
			if (local->vine_2_talk_count >= imath_random(30, 35)) {
				local->vine_2_action = NOTHING;
				vine_2_reset_frame   = 0;
			} else {
				vine_2_reset_frame = imath_random(1, 5);
			}
		} else {
			vine_2_reset_frame = 0;
		}

		if (vine_2_reset_frame >= 0) {
			kernel_reset_animation(aa[3], vine_2_reset_frame);
			local->vine_2_frame = vine_2_reset_frame;
		}
	}
}

static void handle_animation_vine_3() {
	int16 vine_3_reset_frame;

	if (kernel_anim[aa[4]].frame != local->vine_3_frame) {
		local->vine_3_frame = kernel_anim[aa[4]].frame;
		vine_3_reset_frame = -1;

		if (local->vine_3_action == TALK) {
			++local->vine_3_talk_count;
			if (local->vine_3_talk_count >= imath_random(30, 35)) {
				local->vine_3_action = NOTHING;
				vine_3_reset_frame   = 0;
			} else {
				vine_3_reset_frame = imath_random(1, 5);
			}
		} else {
			vine_3_reset_frame = 0;
		}

		if (vine_3_reset_frame >= 0) {
			kernel_reset_animation(aa[4], vine_3_reset_frame);
			local->vine_3_frame = vine_3_reset_frame;
		}
	}
}

static void handle_animation_vine_4() {
	int16 vine_4_reset_frame;

	if (kernel_anim[aa[5]].frame != local->vine_4_frame) {
		local->vine_4_frame = kernel_anim[aa[5]].frame;
		vine_4_reset_frame = -1;

		if (local->vine_4_action == TALK) {
			++local->vine_4_talk_count;
			if (local->vine_4_talk_count >= imath_random(30, 35)) {
				local->vine_4_action = NOTHING;
				vine_4_reset_frame   = 0;
			} else {
				vine_4_reset_frame = imath_random(1, 5);
			}
		} else {
			vine_4_reset_frame = 0;
		}

		if (vine_4_reset_frame >= 0) {
			kernel_reset_animation(aa[5], vine_4_reset_frame);
			local->vine_4_frame = vine_4_reset_frame;
		}
	}
}

static void set_rockk_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int y;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_rock] >= 0) {
		kernel_seq_delete(seq[fx_rock]);
	}

	difference = center - local->rock_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace   = sgn_in(displace, direction);

	x          = local->rock_base + displace - 1;
	y          = series_list[ss[fx_rock]]->index[0].ys + 126;
	xs         = series_list[ss[fx_rock]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_rock] = -1;
	} else {
		seq[fx_rock] = kernel_seq_stamp(ss[fx_rock], false, 1);
		kernel_seq_loc(seq[fx_rock], x, y);
		kernel_seq_depth(seq[fx_rock], 1);
	}
}

static void set_mount_left_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_mount_left] >= 0) {
		kernel_seq_delete(seq[fx_mount_left]);
	}

	difference = center - local->mount_left_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / 2);
	displace   = sgn_in(displace, direction);

	x          = local->mount_left_base - displace - 1;
	xs         = series_list[ss[fx_mount_left]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_mount_left] = -1;
	} else {
		seq[fx_mount_left] = kernel_seq_stamp(ss[fx_mount_left], false, 1);
		kernel_seq_loc(seq[fx_mount_left], x, 130);
		kernel_seq_depth(seq[fx_mount_left], 15);
	}
}

static void set_sky_mid_left_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_sky_mid_left] >= 0) {
		kernel_seq_delete(seq[fx_sky_mid_left]);
	}

	difference = center - local->sky_mid_left_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / 3);
	displace   = sgn_in(displace, direction);

	x          = local->sky_mid_left_base - displace - 1;
	xs         = series_list[ss[fx_sky_mid_left]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_sky_mid_left] = -1;
	} else {
		seq[fx_sky_mid_left] = kernel_seq_stamp(ss[fx_sky_mid_left], false, 1);
		kernel_seq_loc(seq[fx_sky_mid_left], x, 44);
		kernel_seq_depth(seq[fx_sky_mid_left], 15);
	}
}

static void set_mount_right_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_mount_right] >= 0) {
		kernel_seq_delete(seq[fx_mount_right]);
	}

	difference = center - local->mount_right_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / 2);
	displace   = sgn_in(displace, direction);

	x          = local->mount_right_base - displace - 1;
	xs         = series_list[ss[fx_mount_right]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_mount_right] = -1;
	} else {
		seq[fx_mount_right] = kernel_seq_stamp(ss[fx_mount_right], false, 1);
		kernel_seq_loc(seq[fx_mount_right], x, 155);
		kernel_seq_depth(seq[fx_mount_right], 15);
	}
}

static void set_sky_mid_bot_right_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_sky_mid_bot_right] >= 0) {
		kernel_seq_delete(seq[fx_sky_mid_bot_right]);
	}

	difference = center - local->sky_mid_bot_right_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / 3);
	displace   = sgn_in(displace, direction);

	x          = local->sky_mid_bot_right_base - displace - 1;
	xs         = series_list[ss[fx_sky_mid_bot_right]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_sky_mid_bot_right] = -1;
	} else {
		seq[fx_sky_mid_bot_right] = kernel_seq_stamp(ss[fx_sky_mid_bot_right], false, 1);
		kernel_seq_loc(seq[fx_sky_mid_bot_right], x, 60);
		kernel_seq_depth(seq[fx_sky_mid_bot_right], 15);
	}
}

static void process_conv_vine() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	if (player_verb == conv036_question2_b_b) {
		conv_hold();
		you_trig_flag    = true;
		me_trig_flag     = true;
		local->vine_talk = SECOND_SET;
	}

	if (player_verb == conv036_question3_b_b) {
		conv_hold();
		you_trig_flag    = true;
		me_trig_flag     = true;
		local->vine_talk = THIRD_SET;
	}

	if (player_verb == conv036_die_b_b) {
		conv_abort();
		conv_reset(CONV36_VINE);
		you_trig_flag        = true;
		me_trig_flag         = true;
		local->king_1_action = DIE;
	}

	if (player_verb == conv036_ok_b_b) {
		conv_abort();
		you_trig_flag        = true;
		me_trig_flag         = true;
		local->king_1_action = DROP;
	}

	if (kernel.trigger == ROOM_601_YOU_TALK) {
		local->king_1_action = STRUGGLE;
		local->vine_1_action = TALK;
		local->vine_2_action = TALK;
		local->vine_3_action = TALK;
		local->vine_4_action = TALK;
	}

	if (kernel.trigger == ROOM_601_ME_TALK) {
		local->king_1_action = TALK;
		local->vine_1_action = NOTHING;
		local->vine_2_action = NOTHING;
		local->vine_3_action = NOTHING;
		local->vine_4_action = NOTHING;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_601_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_601_ME_TALK);
	}

	local->vine_1_talk_count = 0;
	local->vine_2_talk_count = 0;
	local->vine_3_talk_count = 0;
	local->vine_4_talk_count = 0;
	local->king_1_talk_count = 0;
}

static void room_601_init() {
	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_2_running = false;
		local->anim_3_running = false;
		local->anim_4_running = false;
		local->anim_5_running = false;
		local->anim_6_running = false;
		local->prevent_1      = false;
		local->door_is_open   = false;
	}

	local->king_1_talk_count = 0;
	local->vine_1_talk_count = 0;
	local->vine_2_talk_count = 0;
	local->vine_3_talk_count = 0;
	local->vine_4_talk_count = 0;
	local->vine_talk         = 0;

	kernel.disable_fastwalk = true;
	local->panning          = false;
	local->prevent          = false;

	if (global[player_persona] == PLAYER_IS_KING) {
		kernel.quotes = quote_load(
		    quote_vine_1_a,
		    quote_vine_1_b,
		    quote_vine_1_c,
		    quote_vine_1_d,
		    quote_vine_1_d1,
		    quote_vine_1_e,
		    quote_vine_1_f,
		    quote_vine_1_g,
		    quote_vine_1_h,
		    quote_vine_1_i,
		    quote_vine_1_i1,
		    quote_vine_1_j,
		    quote_vine_1_k,
		    quote_vine_1_k1,
		    quote_vine_1_l,

		    quote_vine_2_a,
		    quote_vine_2_a1,
		    quote_vine_2_b,
		    quote_vine_2_b1,
		    quote_vine_2_c,
		    quote_vine_2_d,
		    quote_vine_2_d1,
		    quote_vine_2_d2,
		    quote_vine_2_e,
		    quote_vine_2_e1,
		    quote_vine_2_f,
		    quote_vine_2_g,
		    quote_vine_2_g1,
		    quote_vine_2_h,
		    quote_vine_2_i,
		    quote_vine_2_i1,

		    quote_vine_3_a,
		    quote_vine_3_a1,
		    quote_vine_3_b,
		    quote_vine_3_b1,
		    quote_vine_3_c,
		    quote_vine_3_d,
		    quote_vine_3_d1,
		    quote_vine_3_e,
		    quote_vine_3_f,
		    quote_vine_3_f1,
		    quote_vine_3_g,
		    quote_vine_3_h,
		    quote_vine_3_i, 0);
	}

	ss[fx_door] = kernel_load_series(kernel_name('y', 0), false);

	if (previous_room == 603 || previous_room == 614 || local->door_is_open) {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_door], 3);
	} else if (global[player_persona] == PLAYER_IS_KING) {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 3);
	}

	if (global[player_persona] == PLAYER_IS_KING) {
		ss[fx_vine_block]  = kernel_load_series(kernel_name('z', 1), false);
		seq[fx_vine_block] = kernel_seq_stamp(ss[fx_vine_block], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_vine_block], 3);
	} else {
		kernel_flip_hotspot(words_tangle, false);
		kernel_flip_hotspot_loc(words_rock, false, ROCK_BLOCK_X, ROCK_BLOCK_Y);
		kernel_flip_hotspot_loc(words_rock, true, ROCK_X, ROCK_Y);
	}

	ss[fx_rock]              = kernel_load_series(kernel_name('r', 1), false);
	ss[fx_mount_left]        = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_sky_mid_left]      = kernel_load_series(kernel_name('c', 1), false);
	ss[fx_mount_right]       = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_sky_mid_bot_right] = kernel_load_series(kernel_name('c', 5), false);

	if (global[player_persona] == PLAYER_IS_PID) {
		ss[fx_single_morph] = kernel_load_series(kernel_name('x', 0), false);
		ss[fx_morph]        = kernel_load_series(kernel_name('x', 3), false);
	}

	if (global[player_persona] == PLAYER_IS_KING) {
		conv_get(CONV36_VINE);
	}

	if (previous_room == KERNEL_RESTORING_GAME) {
		if (conv_restore_running == CONV36_VINE) {
			aa[2]                 = kernel_run_animation(kernel_name('t', 1), 0);
			local->anim_2_running = true;
			local->vine_1_action  = NOTHING;

			aa[3]                 = kernel_run_animation(kernel_name('t', 2), 0);
			local->anim_3_running = true;
			local->vine_2_action  = NOTHING;

			aa[4]                 = kernel_run_animation(kernel_name('t', 3), 0);
			local->anim_4_running = true;
			local->vine_3_action  = NOTHING;

			aa[5]                 = kernel_run_animation(kernel_name('t', 4), 0);
			local->anim_5_running = true;
			local->vine_4_action  = NOTHING;

			aa[1]                 = kernel_run_animation(kernel_name('v', 2), 0);
			local->anim_1_running = true;
			local->king_1_action  = STRUGGLE;
			kernel_reset_animation(aa[1], 72);

			player.commands_allowed = false;
			player.walker_visible   = false;
			conv_run(CONV36_VINE);
		}
	}

	if (previous_room == 601) {
		camera_jump_to(400, 0);
		player.x      = RESTART_X;
		player.y      = RESTART_Y;
		player.facing = FACING_WEST;

	} else if (previous_room == 603 || previous_room == 614) {
		camera_jump_to(CAMERA_MIDDLE, 0);
		player.x                = PLAYER_X_FROM_603;
		player.y                = PLAYER_Y_FROM_603;
		player.facing           = FACING_SOUTH;
		player.commands_allowed = false;
		player_walk(WALK_TO_X_FROM_603, WALK_TO_Y_FROM_603, FACING_SOUTH);
		player_walk_trigger(ROOM_601_DOOR_CLOSES);

	} else if (previous_room == 508 || previous_room != KERNEL_RESTORING_GAME) {
		player_first_walk(PLAYER_X_FROM_508, PLAYER_Y_FROM_508, FACING_EAST,
		                  WALK_TO_X_FROM_508, WALK_TO_Y_FROM_508, FACING_EAST, true);
	}

	if (global[player_persona] == PLAYER_IS_PID && previous_room != 614) {
		seq[fx_single_morph] = kernel_seq_stamp(ss[fx_single_morph], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_single_morph], 3);
		kernel_flip_hotspot(words_tower_door, false);
	}

	seq[fx_mount_left]            = -1;
	local->dyn_mount_left         = -1;
	local->mount_left_base        = -24;

	seq[fx_sky_mid_left]          = -1;
	local->dyn_sky_mid_left       = -1;
	local->sky_mid_left_base      = 23;

	seq[fx_mount_right]           = -1;
	local->dyn_mount_right        = -1;
	local->mount_right_base       = 951;

	seq[fx_sky_mid_bot_right]     = -1;
	local->dyn_sky_mid_bot_right  = -1;
	local->sky_mid_bot_right_base = 913;

	seq[fx_rock]                  = -1;
	local->dyn_rock               = -1;
	local->rock_base              = 100;

	local->update_clock           = kernel.clock;

	set_mount_left_position();
	set_sky_mid_left_position();
	set_mount_right_position();
	set_sky_mid_bot_right_position();
	set_rockk_position();

	section_6_music();
}

static void room_601_daemon() {
	int temp;
	int dist;
	int time;

	if (local->anim_0_running) {
		handle_animation_king_0();
	}

	if (local->anim_1_running) {
		handle_animation_king_1();
	}

	if (local->anim_2_running) {
		handle_animation_vine_1();
	}

	if (local->anim_3_running) {
		handle_animation_vine_2();
	}

	if (local->anim_4_running) {
		handle_animation_vine_3();
	}

	if (local->anim_5_running) {
		handle_animation_vine_4();
	}

	if (local->anim_6_running) {
		handle_animation_king_2();
	}

	if (local->panning && !camera_x.panning) {
		if (local->panning == PANNING_RIGHT) {
			player_demand_location(WALK_TO_RIGHT_X_1, WALK_TO_RIGHT_Y_1);
			player_walk(WALK_TO_RIGHT_X_2, WALK_TO_RIGHT_Y_2, FACING_SOUTHWEST);

		} else if (local->panning == PANNING_LEFT) {
			player_demand_location(WALK_TO_LEFT_X_1, WALK_TO_LEFT_Y_1);
			player_walk(WALK_TO_LEFT_X_2, WALK_TO_LEFT_Y_2, FACING_SOUTHEAST);
		}

		local->panning    = false;
		local->prevent    = false;
		camera_x.pan_mode = CAMERA_PLAYER;
		player_walk_trigger(ROOM_601_COMMANDS_ALLOWED);
	}

	if (kernel.trigger == ROOM_601_COMMANDS_ALLOWED) {
		player.commands_allowed = true;
	}

	if (local->panning == PANNING_RIGHT &&
	    !local->prevent && picture_view_x > 160) {
		local->prevent = true;
		if (!sound_off) {
			sound_queue(N_006Waterfall, 42);
		}
	}

	if (local->panning == PANNING_LEFT &&
	    !local->prevent && picture_view_x < 160) {
		local->prevent = true;
		if (!sound_off) {
			sound_queue(N_006Waterfall, 90);
		}
	}

	if (kernel.clock >= local->update_clock && !local->panning) {
		dist = 127 - ((imath_hypot(player.x - 143, player.y - 107) * 127) / 378);
		if (!sound_off) {
			if (player.x > 395) {
				sound_queue(N_006Waterfall, 42);
			} else {
				sound_queue(N_006Waterfall, dist);
			}
		}
		local->update_clock = kernel.clock + player.frame_delay;
	}

	if (camera_x.pan_this_frame) {
		set_mount_left_position();
		set_mount_right_position();
		set_sky_mid_left_position();
		set_sky_mid_bot_right_position();
		set_rockk_position();
	}

	if (player.x > 520 && player.x < 620 && player.y > 130 && global[vine_will_grab] &&
	    !local->prevent_1) {

		local->prevent_1        = true;
		player.commands_allowed = false;
		player_walk(CAUGHT_X, CAUGHT_Y, FACING_WEST);
		player_walk_trigger(ROOM_601_RUN_VINE_ANIM);
		camera_pan_to(&camera_x, 400);
	}

	if (kernel.trigger == ROOM_601_RUN_VINE_ANIM) {
		aa[0]                 = kernel_run_animation(kernel_name('v', 1), 0);
		local->anim_0_running = true;
		player.walker_visible = false;
		kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
	}

	switch (local->vine_talk) {
	case SECOND_SET:
		kernel_timing_trigger(HALF_SECOND, 14);
		local->vine_talk = 0;
		break;

	case THIRD_SET:
		kernel_timing_trigger(HALF_SECOND, 24);
		local->vine_talk = 0;
		break;
	}

	switch (kernel.trigger) {
	case 1:
		time = 100;
		global_speech_go(8);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_1_a),
		                   33, 22, MESSAGE_COLOR, time, 2, 0);
		clear_all();
		local->vine_1_action = TALK;
		break;

	case 2:
		time = 100;
		kernel_message_purge();
		global_speech_go(9);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_1_b),
		                   218, 38, MESSAGE_COLOR, time, 3, 0);
		clear_all();
		local->vine_4_action = TALK;
		break;

	case 3:
		time = 100;
		kernel_message_purge();
		global_speech_go(10);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_1_c),
		                   206, 5, MESSAGE_COLOR, time, 4, 0);
		clear_all();
		local->vine_3_action = TALK;
		break;

	case 4:
		time = 160;
		kernel_message_purge();
		global_speech_go(11);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_1_d),
		                   218, 28, MESSAGE_COLOR, time, 5, 0);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_1_d1),
		                   218, 43, MESSAGE_COLOR, time, 0, 0);
		clear_all();
		local->vine_4_action = TALK;
		break;

	case 5:
		time = 100;
		kernel_message_purge();
		global_speech_go(12);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_1_e),
		                   171, 6, MESSAGE_COLOR, time, 6, 0);
		clear_all();
		local->vine_2_action = TALK;
		break;

	case 6:
		time = 100;
		kernel_message_purge();
		global_speech_go(13);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_1_f),
		                   206, 5, MESSAGE_COLOR, time, 7, 0);
		clear_all();
		local->vine_3_action = TALK;
		break;

	case 7:
		time = 100;
		kernel_message_purge();
		global_speech_go(14);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_1_g),
		                   53, 24, MESSAGE_COLOR, time, 8, 0);
		clear_all();
		local->vine_1_action = TALK;
		break;

	case 8:
		time = 100;
		kernel_message_purge();
		global_speech_go(15);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_1_h),
		                   91, 21, MESSAGE_COLOR, time, 9, 0);
		clear_all();
		local->vine_2_action = TALK;
		break;

	case 9:
		time = 100;
		kernel_message_purge();
		global_speech_go(16);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_1_i),
		                   218, 23, MESSAGE_COLOR, time, 10, 0);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_1_i1),
		                   218, 38, MESSAGE_COLOR, time, 0, 0);
		clear_all();
		local->vine_4_action = TALK;
		break;

	case 10:
		time = 100;
		kernel_message_purge();
		global_speech_go(17);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_1_j),
		                   27, 32, MESSAGE_COLOR, time, 11, 0);
		clear_all();
		local->vine_1_action = TALK;
		break;

	case 11:
		time = 100;
		kernel_message_purge();
		global_speech_go(18);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_1_k),
		                   219, 19, MESSAGE_COLOR, time, 12, 0);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_1_k1),
		                   219, 34, MESSAGE_COLOR, time, 0, 0);
		clear_all();
		local->vine_4_action = TALK;
		break;

	case 12:
		time = 100;
		kernel_message_purge();
		global_speech_go(19);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_1_l),
		                   91, 21, MESSAGE_COLOR, time, 13, 0);
		clear_all();
		local->vine_2_action = TALK;
		break;

	case 13:
		conv_run(CONV36_VINE);
		global[vines_have_player] = true;
		break;

	case 14:
		time = 140;
		kernel_message_purge();
		global_speech_go(20);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_2_a),
		                   17, 22, MESSAGE_COLOR, time, 15, 0);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_2_a1),
		                   17, 37, MESSAGE_COLOR, time, 0, 0);
		clear_all();
		local->vine_1_action = TALK;
		break;

	case 15:
		time = 100;
		kernel_message_purge();
		global_speech_go(21);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_2_b),
		                   218, 22, MESSAGE_COLOR, time, 16, 0);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_2_b1),
		                   218, 37, MESSAGE_COLOR, time, 0, 0);
		clear_all();
		local->vine_4_action = TALK;
		break;

	case 16:
		time = 100;
		kernel_message_purge();
		global_speech_go(22);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_2_c),
		                   206, 5, MESSAGE_COLOR, time, 17, 0);
		clear_all();
		local->vine_3_action = TALK;
		break;

	case 17:
		time = 190;
		kernel_message_purge();
		global_speech_go(23);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_2_d),
		                   218, 21, MESSAGE_COLOR, time, 18, 0);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_2_d1),
		                   218, 36, MESSAGE_COLOR, time, 0, 0);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_2_d2),
		                   218, 51, MESSAGE_COLOR, time, 0, 0);
		clear_all();
		local->vine_4_action = TALK;
		break;

	case 18:
		time = 100;
		kernel_message_purge();
		global_speech_go(24);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_2_e),
		                   120, 21, MESSAGE_COLOR, time, 19, 0);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_2_e1),
		                   120, 36, MESSAGE_COLOR, time, 0, 0);
		clear_all();
		local->vine_2_action = TALK;
		break;

	case 19:
		time = 100;
		kernel_message_purge();
		global_speech_go(25);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_2_f),
		                   206, 5, MESSAGE_COLOR, time, 20, 0);
		clear_all();
		local->vine_3_action = TALK;
		break;

	case 20:
		time = 180;
		kernel_message_purge();
		global_speech_go(26);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_2_g),
		                   23, 24, MESSAGE_COLOR, time, 21, 0);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_2_g1),
		                   3, 39, MESSAGE_COLOR, time, 0, 0);
		clear_all();
		local->vine_1_action = TALK;
		break;

	case 21:
		time = 100;
		kernel_message_purge();
		global_speech_go(27);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_2_h),
		                   166, 5, MESSAGE_COLOR, time, 22, 0);
		clear_all();
		local->vine_2_action = TALK;
		break;

	case 22:
		time = 100;
		kernel_message_purge();
		global_speech_go(28);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_2_i),
		                   218, 23, MESSAGE_COLOR, time, 23, 0);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_2_i1),
		                   218, 38, MESSAGE_COLOR, time, 0, 0);
		clear_all();
		local->vine_4_action = TALK;
		break;

	case 23:
		conv_release();
		break;

	case 24:
		time = 100;
		kernel_message_purge();
		global_speech_go(29);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_3_a),
		                   23, 22, MESSAGE_COLOR, time, 25, 0);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_3_a1),
		                   23, 37, MESSAGE_COLOR, time, 0, 0);
		clear_all();
		local->vine_1_action = TALK;
		break;

	case 25:
		time = 100;
		kernel_message_purge();
		global_speech_go(30);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_3_b),
		                   218, 22, MESSAGE_COLOR, time, 26, 0);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_3_b1),
		                   218, 37, MESSAGE_COLOR, time, 0, 0);
		clear_all();
		local->vine_4_action = TALK;
		break;

	case 26:
		time = 100;
		kernel_message_purge();
		global_speech_go(31);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_3_c),
		                   23, 22, MESSAGE_COLOR, time, 27, 0);
		clear_all();
		local->vine_1_action = TALK;
		break;

	case 27:
		time = 160;
		kernel_message_purge();
		global_speech_go(32);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_3_d),
		                   218, 28, MESSAGE_COLOR, time, 28, 0);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_3_d1),
		                   218, 43, MESSAGE_COLOR, time, 0, 0);
		clear_all();
		local->vine_4_action = TALK;
		break;

	case 28:
		time = 100;
		kernel_message_purge();
		global_speech_go(33);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_3_e),
		                   91, 21, MESSAGE_COLOR, time, 29, 0);
		clear_all();
		local->vine_2_action = TALK;
		break;

	case 29:
		time = 160;
		kernel_message_purge();
		global_speech_go(34);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_3_f),
		                   206, 5, MESSAGE_COLOR, time, 30, 0);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_3_f1),
		                   206, 20, MESSAGE_COLOR, time, 0, 0);
		clear_all();
		local->vine_3_action = TALK;
		break;

	case 30:
		time = 100;
		kernel_message_purge();
		global_speech_go(35);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_3_g),
		                   3, 24, MESSAGE_COLOR, time, 31, 0);
		clear_all();
		local->vine_1_action = TALK;
		break;

	case 31:
		time = 100;
		kernel_message_purge();
		global_speech_go(36);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_3_h),
		                   200, 4, MESSAGE_COLOR, time, 32, 0);
		clear_all();
		local->vine_3_action = TALK;
		break;

	case 32:
		time = 200;
		kernel_message_purge();
		global_speech_go(37);
		kernel_message_add(quote_string(kernel.quotes, quote_vine_3_i),
		                   218, 33, MESSAGE_COLOR, time, 33, 0);
		clear_all();
		local->vine_4_action = TALK;
		break;

	case 33:
		conv_release();
		break;
	}

	if (kernel.trigger == ROOM_601_DOOR_OPENS) {
		temp = seq[fx_door];
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_door], 2);
		kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, temp);
	}

	if (kernel.trigger == ROOM_601_DOOR_CLOSES) {
		if (global[player_persona] == PLAYER_IS_PID) {
			kernel_load_variant(1);
		}
		sound_play(N_CreakyCastleDoor);
		kernel_seq_delete(seq[fx_door]);
		seq[fx_door] = kernel_seq_backward(ss[fx_door], false, 7, 0, 0, 1);
		kernel_seq_depth(seq[fx_door], 3);
		kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_601_DOOR_CLOSES + 1);
	}

	if (kernel.trigger == ROOM_601_DOOR_CLOSES + 1) {
		if (global[player_persona] == PLAYER_IS_PID) {
			sound_play(N_CastleDoorMorphs);
			seq[fx_morph] = kernel_seq_forward(ss[fx_morph], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_morph], 3);
			kernel_seq_range(seq[fx_morph], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_morph], KERNEL_TRIGGER_EXPIRE, 0, ROOM_601_DOOR_CLOSES + 2);
			kernel_synch(KERNEL_SERIES, seq[fx_morph], KERNEL_SERIES, seq[fx_door]);
		} else {
			temp = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door], 3);
			kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, temp);
			player.commands_allowed = true;
		}
	}

	if (kernel.trigger == ROOM_601_DOOR_CLOSES + 2) {
		temp = seq[fx_morph];
		seq[fx_morph] = kernel_seq_stamp(ss[fx_morph], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_morph], 3);
		kernel_synch(KERNEL_SERIES, seq[fx_morph], KERNEL_SERIES, temp);

		kernel_flip_hotspot(words_tower_door, false);
		player.commands_allowed = true;
	}

	if (kernel.trigger == MUSIC) {
		if (global[vines_have_player]) {
			sound_play(N_EerieSounds);
		} else {
			sound_play(N_BackgroundMus);
		}
	}
}

static void room_601_pre_parser() {
	if (player_said_2(walk_to, tangle)) {
		if (player.x <= 385) {
			player_walk(WALK_VINE_LEFT_X, WALK_VINE_LEFT_Y, 5);
		} else {
			player_walk(WALK_VINE_RIGHT_X, WALK_VINE_RIGHT_Y, 5);
		}
	}

	if (player_said_2(walk_to, rock) && global[player_persona] == PLAYER_IS_KING &&
	    (inter_point_x >= 386 && inter_point_x <= 422) &&
	    (inter_point_y >= 141 && inter_point_y <= 155)) {

		if (player.x <= 385) {
			player_walk(WALK_VINE_LEFT_X, WALK_VINE_LEFT_Y, 5);
		} else {
			player_walk(WALK_VINE_RIGHT_X, WALK_VINE_RIGHT_Y, 5);
		}

	} else if ((player_said_1(walk_to) || player_said_1(walk_down) || player_said_1(walk_through)) &&
	           global[player_persona] == PLAYER_IS_KING) {
		if (player.x <= 385) {
			if (inter_point_x >= 423) {
				switch (kernel.trigger) {
				case 0:
					player_walk(WALK_VINE_LEFT_X, WALK_VINE_LEFT_Y, 5);
					player_walk_trigger(1);
					break;

				case 1:
					if (!player_said_1(tangle)) {
						text_show(60114);
					}
					player_cancel_command();
					break;
				}
			}

		} else {
			if (inter_point_x <= 422) {
				switch (kernel.trigger) {
				case 0:
					player_walk(WALK_VINE_RIGHT_X, WALK_VINE_RIGHT_Y, 5);
					player_walk_trigger(1);
					break;

				case 1:
					if (!player_said_1(tangle)) {
						text_show(60114);
					}
					player_cancel_command();
					break;
				}
			}
		}
	}

	if (player_said_2(walk_down, path_to_Hightower)) {
		player.walk_off_edge_to_room = 508;
	}

	if (player_said_2(walk_through, tower_door) || player_said_2(open, tower_door) ||
	    player_said_2(pull, tower_door)) {
		if (local->door_is_open) {
			player_walk(PLAYER_X_FROM_603, PLAYER_Y_FROM_603, FACING_NORTH);
		}
	}
}

static void room_601_parser() {
	int temp;

	if (conv_control.running == CONV36_VINE) {
		process_conv_vine();
		player.command_ready = false;
		return;
	}

	switch (kernel.trigger) {
	case ROOM_601_DOOR_OPENS:
		player_walk(PLAYER_X_FROM_603, PLAYER_Y_FROM_603, FACING_NORTH);
		player_walk_trigger(ROOM_601_DOOR_OPENS + 1);
		temp = seq[fx_door];
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_door], 3);
		kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, temp);
		player.command_ready = false;
		return;

	case ROOM_601_DOOR_OPENS + 1:
		sound_play(N_CreakyCastleDoor);
		kernel_seq_delete(seq[fx_door]);
		seq[fx_door] = kernel_seq_backward(ss[fx_door], false, 7, 0, 0, 1);
		kernel_seq_depth(seq[fx_door], 2);
		kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_601_DOOR_OPENS + 2);
		player.command_ready = false;
		return;

	case ROOM_601_DOOR_OPENS + 2:
		temp = seq[fx_door];
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 1);
		kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, temp);
		new_room = 603;
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_through, tower_door) || player_said_2(open, tower_door) ||
	    player_said_2(pull, tower_door)) {

		if (global[vine_will_grab]) {
			player.command_ready = false;
			return;
		}

		if (local->door_is_open) {
			kernel_timing_trigger(1, ROOM_601_DOOR_OPENS + 1);
		} else {
			sound_play(N_CreakyCastleDoor);
			kernel_seq_delete(seq[fx_door]);
			seq[fx_door] = kernel_seq_forward(ss[fx_door], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 3);
			kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_601_DOOR_OPENS);
		}
		player.commands_allowed = false;
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_down, path_behind_tower)) {
		camera_x.pan_mode = CAMERA_MANUAL;
		if (inter_point_x < 300) {
			player.commands_allowed = false;
			local->panning          = PANNING_RIGHT;
			camera_pan_to(&camera_x, CAMERA_RIGHT);
		} else {
			player.commands_allowed = false;
			local->panning          = PANNING_LEFT;
			camera_pan_to(&camera_x, CAMERA_LEFT);
		}
		player.command_ready = false;
		return;
	}

	if (player.look_around) {
		if (global[player_persona] == PLAYER_IS_KING) {
			text_show(60101);
		} else {
			text_show(60111);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(tower_door)) {
			text_show(60102);
			player.command_ready = false;
			return;
		}

		if (player_said_1(tower)) {
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(60104);
			} else {
				text_show(60111);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(path_behind_tower)) {
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(60116);
			} else {
				text_show(60112);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(mountainside)) {
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(60105);
			} else {
				text_show(60113);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(waterfall)) {
			text_show(60106);
			player.command_ready = false;
			return;
		}

		if (player_said_1(path_to_Hightower)) {
			text_show(60108);
			player.command_ready = false;
			return;
		}

		if (player_said_1(path_around_tower)) {
			text_show(60109);
			player.command_ready = false;
			return;
		}

		if (player_said_1(tangle)) {
			text_show(60114);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(talk_to, tower_door)) {
		text_show(60103);
		player.command_ready = false;
		return;
	}

	if (player_said_3(fill, goblet, waterfall) ||
	    player_said_3(put, goblet, waterfall)) {
		text_show(60117);
		player.command_ready = false;
		return;
	}

	if (player_said_2(put, waterfall)) {
		text_show(60107);
		player.command_ready = false;
		return;
	}

	if (player_said_2(talk_to, tangle)) {
		if (!global[vine_will_grab]) {
			text_show(60110);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(sword, attack) ||
	    player_said_2(sword, carve_up) ||
	    player_said_2(sword, thrust)) {

		if (player_said_1(tangle)) {
			text_show(60115);
			player.command_ready = false;
			return;
		}
	}
}

void room_601_synchronize(Common::Serializer &s) {
	for (int i = 0; i < 15; i++) s.syncAsSint16LE(local->sprite[i]);
	for (int i = 0; i < 15; i++) s.syncAsSint16LE(local->sequence[i]);
	for (int i = 0; i < 7; i++)  s.syncAsSint16LE(local->animation[i]);
	s.syncAsSint16LE(local->mount_left_base);
	s.syncAsSint16LE(local->dyn_mount_left);
	s.syncAsSint16LE(local->sky_mid_left_base);
	s.syncAsSint16LE(local->dyn_sky_mid_left);
	s.syncAsSint16LE(local->mount_right_base);
	s.syncAsSint16LE(local->dyn_mount_right);
	s.syncAsSint16LE(local->sky_mid_bot_right_base);
	s.syncAsSint16LE(local->dyn_sky_mid_bot_right);
	s.syncAsSint16LE(local->rock_base);
	s.syncAsSint16LE(local->dyn_rock);
	s.syncAsSint16LE(local->panning);
	s.syncAsSint16LE(local->prevent);
	s.syncAsSint16LE(local->prevent_1);
	s.syncAsSint16LE(local->vine_talk);
	s.syncAsSint32LE(local->update_clock);
	s.syncAsSint16LE(local->king_0_frame);
	s.syncAsSint16LE(local->anim_0_running);
	s.syncAsSint16LE(local->king_1_frame);
	s.syncAsSint16LE(local->king_1_action);
	s.syncAsSint16LE(local->king_1_talk_count);
	s.syncAsSint16LE(local->anim_1_running);
	s.syncAsSint16LE(local->vine_1_frame);
	s.syncAsSint16LE(local->vine_1_action);
	s.syncAsSint16LE(local->vine_1_talk_count);
	s.syncAsSint16LE(local->anim_2_running);
	s.syncAsSint16LE(local->vine_2_frame);
	s.syncAsSint16LE(local->vine_2_action);
	s.syncAsSint16LE(local->vine_2_talk_count);
	s.syncAsSint16LE(local->anim_3_running);
	s.syncAsSint16LE(local->vine_3_frame);
	s.syncAsSint16LE(local->vine_3_action);
	s.syncAsSint16LE(local->vine_3_talk_count);
	s.syncAsSint16LE(local->anim_4_running);
	s.syncAsSint16LE(local->vine_4_frame);
	s.syncAsSint16LE(local->vine_4_action);
	s.syncAsSint16LE(local->vine_4_talk_count);
	s.syncAsSint16LE(local->anim_5_running);
	s.syncAsSint16LE(local->king_2_frame);
	s.syncAsSint16LE(local->king_2_action);
	s.syncAsSint16LE(local->king_2_talk_count);
	s.syncAsSint16LE(local->anim_6_running);
	s.syncAsSint16LE(local->door_is_open);
}

void room_601_preload() {
	room_init_code_pointer = room_601_init;
	room_pre_parser_code_pointer = room_601_pre_parser;
	room_parser_code_pointer = room_601_parser;
	room_daemon_code_pointer = room_601_daemon;

	if (global[player_persona] == PLAYER_IS_PID && room_id != 614) {
		kernel_initial_variant = 1;
	}

	section_6_walker();
	section_6_interface();

	vocab_make_active(words_rat);
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
