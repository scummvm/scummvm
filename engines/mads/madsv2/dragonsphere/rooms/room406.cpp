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
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section4.h"
#include "mads/madsv2/dragonsphere/rooms/room406.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[7];
	int16 king_frame;
	int16 king_action;
	int16 king_talk_count;
	int16 anim_0_running;
	int16 pid_frame;
	int16 pid_action;
	int16 pid_talk_count;
	int16 anim_1_running;
	int16 cal_frame;
	int16 cal_action;
	int16 cal_talk_count;
	int16 anim_2_running;
	int16 king_walk_frame;
	int16 king_walk_action;
	int16 king_walk_talk_count;
	int16 anim_3_running;
	int16 pid_walk_frame;
	int16 pid_walk_action;
	int16 pid_walk_talk_count;
	int16 anim_4_running;
	int16 belly_before_frame;
	int16 belly_before_talk_count;
	int16 anim_5_running;
	int16 belly_after_frame;
	int16 anim_6_running;
	int16 half_bottle_base;
	int16 dyn_half_bottle;
	int16 two_bottles_base;
	int16 dyn_two_bottles;
	int16 thing_1_base;
	int16 dyn_thing_1;
	int16 thing_2_base;
	int16 dyn_thing_2;
	int16 thing_3_base;
	int16 dyn_thing_3;
	int16 pole_base;
	int16 dyn_pole;
	int16 mount_x;
	int16 mount_2_x;
	int16 num_of_stuff;
	int16 won_sop;
	int16 clock;
	int16 death_timer;
	int16 activate_timer;
	int16 panning;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_half_bottle          0
#define fx_two_bottles          1
#define fx_thing_1              2
#define fx_thing_2              3
#define fx_thing_3              4
#define fx_pole                 5
#define fx_mount                6
#define fx_mount_2              7
#define fx_smoke                8
#define fx_cup                  9

#define ROOM_406_ME_TALK        60
#define ROOM_406_YOU_TALK       62
#define ROOM_406_CUP            65

#define FREEZE                  0
#define CLAP                    1
#define BELT                    2
#define GIVE                    3
#define LOOK_GIRL               4
#define TALK                    5
#define GET_UP                  6
#define DIE                     7
#define GET_DOLL                8
#define TAKE_DRINK              9
#define PLEASE_SIT              10
#define DRINK                   11
#define POST_DRINK              12
#define LOOK_GIRL_2             13

#define camera_ratio_1          1
#define camera_ratio_2          2

#define CONV_41_KING            41
#define CONV_45_PID             45

#define LENGTH_OF_LIFE          850

#define PANNING_TO_DANCE        1
#define PANNING_AWAY_FROM_DANCE 2

static Scratch scratch;


static void set_406_mount_position() {
	int difference;
	int x;

	difference = ((picture_view_x) * 4) / 7;
	x = local->mount_x + difference;
	x += 1;

	kernel_seq_delete(seq[fx_mount]);
	seq[fx_mount] = kernel_seq_stamp(ss[fx_mount], false, 1);
	kernel_seq_loc(seq[fx_mount], x, 90);
	kernel_seq_depth(seq[fx_mount], 12);
}

static void set_406_mount_2_position() {
	int difference;
	int x;

	difference = ((picture_view_x) * 4) / 7;
	x = local->mount_2_x + difference;
	x += 1;

	kernel_seq_delete(seq[fx_mount_2]);
	seq[fx_mount_2] = kernel_seq_stamp(ss[fx_mount], true, 1);
	kernel_seq_loc(seq[fx_mount_2], x, 91);
	kernel_seq_depth(seq[fx_mount_2], 12);
}

static void set_half_bottle_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_half_bottle] >= 0) {
		kernel_seq_delete(seq[fx_half_bottle]);
	}

	difference = center - local->half_bottle_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace   = sgn_in(displace, direction);

	x          = local->half_bottle_base + displace - 1;
	xs         = series_list[ss[fx_half_bottle]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_half_bottle] = -1;
	} else {
		seq[fx_half_bottle] = kernel_seq_stamp(ss[fx_half_bottle], false, 1);
		kernel_seq_loc(seq[fx_half_bottle], x, 155);
		kernel_seq_depth(seq[fx_half_bottle], 1);
	}
}

static void set_two_bottles_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_two_bottles] >= 0) {
		kernel_seq_delete(seq[fx_two_bottles]);
	}

	difference = center - local->two_bottles_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace   = sgn_in(displace, direction);

	x          = local->two_bottles_base + displace - 1;
	xs         = series_list[ss[fx_two_bottles]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_two_bottles] = -1;
	} else {
		seq[fx_two_bottles] = kernel_seq_stamp(ss[fx_two_bottles], false, 1);
		kernel_seq_loc(seq[fx_two_bottles], x, 155);
		kernel_seq_depth(seq[fx_two_bottles], 1);
	}
}

static void set_thing_1_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_thing_1] >= 0) {
		kernel_seq_delete(seq[fx_thing_1]);
	}

	difference = center - local->thing_1_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / 6);
	displace   = sgn_in(displace, direction);

	x          = local->thing_1_base + displace - 1;
	xs         = series_list[ss[fx_thing_1]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_thing_1] = -1;
	} else {
		seq[fx_thing_1] = kernel_seq_stamp(ss[fx_thing_1], false, 1);
		kernel_seq_loc(seq[fx_thing_1], x, 45);
		kernel_seq_depth(seq[fx_thing_1], 1);
	}
}

static void set_thing_2_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_thing_2] >= 0) {
		kernel_seq_delete(seq[fx_thing_2]);
	}

	difference = center - local->thing_2_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / 3);
	displace   = sgn_in(displace, direction);

	x          = local->thing_2_base + displace - 1;
	xs         = series_list[ss[fx_thing_2]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_thing_2] = -1;
	} else {
		seq[fx_thing_2] = kernel_seq_stamp(ss[fx_thing_2], false, 1);
		kernel_seq_loc(seq[fx_thing_2], x, 61);
		kernel_seq_depth(seq[fx_thing_2], 1);
	}
}

static void set_thing_3_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_thing_3] >= 0) {
		kernel_seq_delete(seq[fx_thing_3]);
	}

	difference = center - local->thing_3_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / 4);
	displace   = sgn_in(displace, direction);

	x          = local->thing_3_base + displace - 1;
	xs         = series_list[ss[fx_thing_3]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_thing_3] = -1;
	} else {
		seq[fx_thing_3] = kernel_seq_stamp(ss[fx_thing_3], false, 1);
		kernel_seq_loc(seq[fx_thing_3], x, 86);
		kernel_seq_depth(seq[fx_thing_3], 1);
	}
}

static void set_pole_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_pole] >= 0) {
		kernel_seq_delete(seq[fx_pole]);
	}

	difference = center - local->pole_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace   = sgn_in(displace, direction);

	x          = local->pole_base + displace - 1;
	xs         = series_list[ss[fx_pole]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_pole] = -1;
	} else {
		seq[fx_pole] = kernel_seq_stamp(ss[fx_pole], false, 1);
		kernel_seq_loc(seq[fx_pole], x, 155);
		kernel_seq_depth(seq[fx_pole], 1);
	}
}

static void handle_animation_king() {
	int king_reset_frame;
	int count;

	if (kernel_anim[aa[0]].frame != local->king_frame) {
		local->king_frame = kernel_anim[aa[0]].frame;
		king_reset_frame = -1;

		switch (local->king_frame) {
		case 19:
		case 53:
			conv_run(CONV_41_KING);
			conv_export_pointer(&global[dance_points]);
			conv_export_pointer(&global[game_points]);
			conv_export_pointer(&global[clue_points]);
			conv_export_value(global[talked_to_soptus]);
			conv_export_value(global[pid_talk_shamon]);
			conv_export_value(local->num_of_stuff);
			conv_export_value(global[prizes_owed_to_player]);
			break;

		case 47:
			for (count = 1; count <= global[prizes_owed_to_player]; count ++) {
				if (object_is_here(dates)) {
					sound_play(N_TakeObjectSnd);
					inter_give_to_player(dates);
					object_examine(dates, 814, 0);
					++ global[dragon_high_scene];
					++ global[player_score];
				} else if (object_is_here(statue)) {
					sound_play(N_TakeObjectSnd);
					inter_give_to_player(statue);
					object_examine(statue, 815, 0);
					++ global[player_score];
				} else if (object_is_here(ruby_ring)) {
					sound_play(N_TakeObjectSnd);
					inter_give_to_player(ruby_ring);
					object_examine(ruby_ring, 841, 0);
					global[player_score] += 5;
				}
			}
			local->cal_action             = FREEZE;
			global[prizes_owed_to_player] = 0;
			break;

		case 56:
			if (local->king_action == LOOK_GIRL) {
				global[dance_music_on] = true;
				camera_pan_to(&camera_x, 160);
				local->panning = PANNING_TO_DANCE;
				if (local->belly_before_talk_count != 0) {
					kernel_abort_animation(aa[5]);
					aa[6]                 = kernel_run_animation(kernel_name('d', 2), 0);
					local->anim_5_running = false;
					local->anim_6_running = true;
				}
			}
			break;

		case 57:
			if (local->king_action == LOOK_GIRL ||
			    local->king_action == LOOK_GIRL_2) {
				king_reset_frame = 56;
			}
			break;

		case 43:
			kernel_abort_animation(aa[0]);
			local->anim_0_running = false;
			local->anim_3_running = true;
			aa[3]                 = kernel_run_animation(kernel_name('k', 2), 0);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
			kernel_reset_animation(aa[3], 75);
			break;

		case 20:
		case 71:
		case 66:
		case 55:
		case 54:
			switch (local->king_action) {
			case FREEZE:
				king_reset_frame = 54;
				break;

			case LOOK_GIRL:
			case LOOK_GIRL_2:
				king_reset_frame   = 55;
				break;

			case GIVE:
				king_reset_frame   = 43;
				local->king_action = FREEZE;
				break;

			case TALK:
				king_reset_frame   = 66;
				local->king_action = FREEZE;
				break;

			case GET_UP:
				king_reset_frame   = 21;
				local->king_action = FREEZE;
				break;
			}
			break;
		}

		if (king_reset_frame >= 0) {
			kernel_reset_animation(aa[0], king_reset_frame);
			local->king_frame = king_reset_frame;
		}
	}
}

static void handle_animation_pid() {
	int pid_reset_frame;
	int count;

	if (kernel_anim[aa[1]].frame != local->pid_frame) {
		local->pid_frame = kernel_anim[aa[1]].frame;
		pid_reset_frame = -1;

		switch (local->pid_frame) {
		case 21:
		case 47:
			conv_run(CONV_45_PID);
			conv_export_pointer(&global[pid_has_been_healed_sop]);
			conv_export_value(local->won_sop);
			conv_export_value(global[prizes_owed_to_player]);
			break;

		case 61:
		case 78:
			conv_release();
			break;

		case 39:
			for (count = 1; count <= global[prizes_owed_to_player]; count ++) {
				if (object_is_here(dates)) {
					sound_play(N_TakeObjectSnd);
					inter_give_to_player(dates);
					object_examine(dates, 814, 0);
					++ global[player_score];
				} else if (object_is_here(statue)) {
					sound_play(N_TakeObjectSnd);
					inter_give_to_player(statue);
					object_examine(statue, 815, 0);
					++ global[player_score];
				} else if (object_is_here(ruby_ring)) {
					sound_play(N_TakeObjectSnd);
					inter_give_to_player(ruby_ring);
					object_examine(ruby_ring, 841, 0);
					global[player_score] += 5;
				} else if (object_is_here(bottle_of_flies)) {
					sound_play(N_TakeObjectSnd);
					inter_give_to_player(bottle_of_flies);
					object_examine(bottle_of_flies, 816, 0);
					++ global[player_score];
				} else if (object_is_here(soptus_soporific)) {
					sound_play(N_TakeObjectSnd);
					inter_give_to_player(soptus_soporific);
					object_examine(soptus_soporific, 831, 0);
					++ global[player_score];
				}
			}
			local->cal_action             = FREEZE;
			global[prizes_owed_to_player] = 0;
			break;

		case 56:
			conv_release();
			break;

		case 57:
			if (local->pid_action == TAKE_DRINK) {
				pid_reset_frame = 56;
			}
			break;

		case 127:
			kernel_abort_animation(aa[1]);
			local->anim_1_running = false;
			local->anim_4_running = true;
			aa[4]                 = kernel_run_animation(kernel_name('p', 2), 0);
			kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
			kernel_reset_animation(aa[4], 77);
			break;

		case 54:
			local->cal_action = FREEZE;
			break;

		case 62:
			if (local->pid_action == DRINK) {
				pid_reset_frame = 61;
			}
			break;

		case 70:
			seq[fx_cup] = kernel_seq_forward(ss[fx_cup], false, 6, 0, 0, 1);
			kernel_seq_depth(seq[fx_cup], 1);
			kernel_seq_range(seq[fx_cup], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_cup], KERNEL_TRIGGER_EXPIRE, 0, ROOM_406_CUP);
			kernel_synch(KERNEL_SERIES, seq[fx_cup], KERNEL_ANIM, aa[1]);
			break;

		case 79:
			switch (local->pid_action) {
			case GET_DOLL:
				pid_reset_frame = 79;
				break;

			case DIE:
				pid_reset_frame = 90;
				break;

			case POST_DRINK:
			default:
				pid_reset_frame = 78;
				break;
			}
			break;

		case 94:
			global_speech_go(6);
			break;

		case 104:
			sound_play(N_PlayerDies);
			text_show(40615);
			conv_reset(CONV_45_PID);
			global[pid_just_died] = true;
			new_room              = 405;
			break;

		case 89:
			conv_run(CONV_45_PID);
			conv_export_pointer(&global[pid_has_been_healed_sop]);
			conv_export_value(1);
			conv_export_value(global[prizes_owed_to_player]);
			break;

		case 85:
			pid_reset_frame = 127;
			break;

		case 140:
			text_show(40614);
			pid_reset_frame = 85;
			break;

		case 22:
		case 32:
		case 23:
		case 48:
		case 90:
			switch (local->pid_action) {
			case FREEZE:
				pid_reset_frame = 22;
				break;

			case TAKE_DRINK:
				pid_reset_frame = 48;
				break;

			case GET_DOLL:
				pid_reset_frame   = 79;
				local->pid_action = FREEZE;
				break;

			case GIVE:
				pid_reset_frame   = 32;
				local->pid_action = FREEZE;
				break;

			case TALK:
				pid_reset_frame   = 23;
				local->pid_action = FREEZE;
				break;

			case DIE:
				pid_reset_frame   = 90;
				local->pid_action = FREEZE;
				break;

			case GET_UP:
				pid_reset_frame   = 104;
				local->pid_action = FREEZE;
				break;
			}
			break;
		}

		if (pid_reset_frame >= 0) {
			kernel_reset_animation(aa[1], pid_reset_frame);
			local->pid_frame = pid_reset_frame;
		}
	}
}

static void handle_animation_cal() {
	int cal_reset_frame;

	if (kernel_anim[aa[2]].frame != local->cal_frame) {
		local->cal_frame = kernel_anim[aa[2]].frame;
		cal_reset_frame = -1;

		switch (local->cal_frame) {
		case 1:
		case 2:
		case 3:
			if (local->cal_action == TALK) {
				cal_reset_frame = imath_random(0, 2);
				++ local->cal_talk_count;
				if (local->cal_talk_count > 24) {
					local->cal_action     = FREEZE;
					local->cal_talk_count = 0;
					cal_reset_frame       = 90;
				}
			} else {
				cal_reset_frame = 90;
			}
			break;

		case 34:
			local->king_action = GIVE;
			local->pid_action  = GIVE;
			break;

		case 35:
			if (local->cal_action == GIVE) {
				cal_reset_frame = 34;
			}
			break;

		case 64:
			local->pid_action = TAKE_DRINK;
			break;

		case 65:
			if (local->cal_action == TAKE_DRINK) {
				cal_reset_frame = 64;
			}
			break;

		case 87:
			global[dance_music_on] = true;
			camera_pan_to(&camera_x, 160);
			local->panning = PANNING_TO_DANCE;
			if (local->belly_before_talk_count != 0) {
				kernel_abort_animation(aa[5]);
				aa[6]                 = kernel_run_animation(kernel_name('d', 2), 0);
				local->anim_5_running = false;
				local->anim_6_running = true;
			}
			break;

		case 88:
			if (local->cal_action == CLAP) {
				cal_reset_frame = 87;
			}
			break;

		case 94:
			if (local->cal_action == LOOK_GIRL) {
				cal_reset_frame = 93;
			} else {
				cal_reset_frame = 98;
			}
			break;

		case 14:
		case 24:
		case 40:
		case 70:
		case 90:
		case 91:
		case 97:
		case 115:
			switch (local->cal_action) {
			case TALK:
				cal_reset_frame = imath_random(14, 15);
				if (cal_reset_frame == 14) {
					local->cal_action = FREEZE;
				} else {
					cal_reset_frame = 0;
				}
				break;

			case FREEZE:
				cal_reset_frame = 90;
				break;

			case CLAP:
				cal_reset_frame = 70;
				break;

			case TAKE_DRINK:
				cal_reset_frame = 40;
				break;

			case GIVE:
				cal_reset_frame = 24;
				break;

			case LOOK_GIRL:
				cal_reset_frame = 91;
				break;

			case PLEASE_SIT:
				cal_reset_frame   = 3;
				local->cal_action = FREEZE;
				break;
			}
			break;
		}

		if (cal_reset_frame >= 0) {
			kernel_reset_animation(aa[2], cal_reset_frame);
			local->cal_frame = cal_reset_frame;
		}
	}
}

static void handle_animation_king_walk() {
	int king_walk_reset_frame;

	if (kernel_anim[aa[3]].frame != local->king_walk_frame) {
		local->king_walk_frame = kernel_anim[aa[3]].frame;
		king_walk_reset_frame = -1;

		switch (local->king_walk_frame) {
		case 60:
			local->cal_action = PLEASE_SIT;
			break;

		case 74:
			kernel_abort_animation(aa[3]);
			local->anim_3_running = false;
			aa[0]                 = kernel_run_animation(kernel_name('k', 1), 0);
			local->king_action    = FREEZE;
			local->anim_0_running = true;
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_NOW, 0);
			break;

		case 146:
			camera_x.pan_mode = CAMERA_PLAYER;
			new_room          = 405;
			break;
		}

		if (king_walk_reset_frame >= 0) {
			kernel_reset_animation(aa[3], king_walk_reset_frame);
			local->king_walk_frame = king_walk_reset_frame;
		}
	}
}

static void handle_animation_pid_walk() {
	int pid_walk_reset_frame;

	if (kernel_anim[aa[4]].frame != local->pid_walk_frame) {
		local->pid_walk_frame = kernel_anim[aa[4]].frame;
		pid_walk_reset_frame = -1;

		switch (local->pid_walk_frame) {
		case 60:
			local->cal_action = PLEASE_SIT;
			break;

		case 75:
			kernel_abort_animation(aa[4]);
			local->anim_4_running = false;
			aa[1]                 = kernel_run_animation(kernel_name('p', 1), 0);
			local->pid_action     = FREEZE;
			local->anim_1_running = true;
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
			break;

		case 143:
			camera_x.pan_mode = CAMERA_PLAYER;
			new_room          = 405;
			break;
		}

		if (pid_walk_reset_frame >= 0) {
			kernel_reset_animation(aa[4], pid_walk_reset_frame);
			local->pid_walk_frame = pid_walk_reset_frame;
		}
	}
}

static void handle_animation_belly_before() {
	int belly_before_reset_frame;

	if (kernel_anim[aa[5]].frame != local->belly_before_frame) {
		local->belly_before_frame = kernel_anim[aa[5]].frame;
		belly_before_reset_frame = -1;

		switch (local->belly_before_frame) {
		case 37:
		case 73:
			if (local->panning == PANNING_TO_DANCE && !camera_x.panning) {
				++ local->belly_before_talk_count;
				if (local->belly_before_talk_count >= 3) {
					camera_pan_to(&camera_x, 0);
					local->panning = PANNING_AWAY_FROM_DANCE;
				}
			}

			if (imath_random(1, 2) == 1) {
				belly_before_reset_frame = 0;
			} else {
				belly_before_reset_frame = 38;
			}
			break;
		}

		if (belly_before_reset_frame >= 0) {
			kernel_reset_animation(aa[5], belly_before_reset_frame);
			local->belly_before_frame = belly_before_reset_frame;
		}
	}
}

static void handle_animation_belly_after() {
	int belly_after_reset_frame;

	if (kernel_anim[aa[6]].frame != local->belly_after_frame) {
		local->belly_after_frame = kernel_anim[aa[6]].frame;
		belly_after_reset_frame = -1;

		switch (local->belly_after_frame) {
		case 92:
			camera_pan_to(&camera_x, 0);
			local->panning = PANNING_AWAY_FROM_DANCE;
			break;

		case 100:
			belly_after_reset_frame = 99;
			break;

		case 24:
			if (local->panning == PANNING_TO_DANCE && camera_x.panning) {
				belly_after_reset_frame = 0;
			}
			break;
		}

		if (belly_after_reset_frame >= 0) {
			kernel_reset_animation(aa[6], belly_after_reset_frame);
			local->belly_after_frame = belly_after_reset_frame;
		}
	}
}

static void room_406_init() {
	int count;
	int o1 = false;
	int o2 = false;
	int o3 = false;
	int o4 = false;
	int o5 = false;

	global[no_load_walker] = false;

	global[perform_displacements] = true;
	camera_x.pan_mode = CAMERA_MANUAL;
	local->death_timer = 0;
	local->clock = 0;
	local->panning = false;
	local->belly_before_talk_count = 0;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_2_running = false;
		local->anim_3_running = false;
		local->anim_4_running = false;
		local->anim_5_running = false;
		local->anim_6_running = false;
		local->won_sop = false;
		local->activate_timer = false;
	}

	if (previous_room == 405 && global[player_persona] == PLAYER_IS_PID) {
		if (player_has(dates)) {
			global[object_flags] = global[object_flags] | 1;
		}

		if (player_has(statue)) {
			global[object_flags] = global[object_flags] | 2;
		}

		if (player_has(ruby_ring)) {
			global[object_flags] = global[object_flags] | 4;
		}

		if (player_has(bottle_of_flies)) {
			global[object_flags] = global[object_flags] | 8;
		}

		if (player_has(soptus_soporific)) {
			global[object_flags] = global[object_flags] | 16;
		}

		global[save_wins_in_desert] = global[wins_in_desert];
	}

	ss[fx_cup] = kernel_load_series(kernel_name('x', 0), false);

	if (local->activate_timer) {
		seq[fx_cup] = kernel_seq_stamp(ss[fx_cup], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_cup], 1);
	} else {
		kernel_set_interface_mode(INTER_LIMITED_SENTENCES);
		player.commands_allowed = false;
	}

	if (global[player_persona] == PLAYER_IS_KING) {
		conv_get(CONV_41_KING);
	} else {
		conv_get(CONV_45_PID);
	}

	local->num_of_stuff = 0;

	if (global[player_persona] == PLAYER_IS_KING) {
		if (object_is_here(dates))     ++local->num_of_stuff;
		if (object_is_here(statue))    ++local->num_of_stuff;
		if (object_is_here(ruby_ring)) ++local->num_of_stuff;
	} else {
		if (object_is_here(bottle_of_flies))   ++local->num_of_stuff;
		if (object_is_here(soptus_soporific))  ++local->num_of_stuff;
	}

	if (global[player_persona] == PLAYER_IS_KING) {
		kernel_load_series(kernel_name('a', 0), false);
	} else {
		kernel_load_series(kernel_name('b', 0), false);
	}

	ss[fx_smoke] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_thing_1] = kernel_load_series(kernel_name('p', 2), false);
	ss[fx_thing_2] = kernel_load_series(kernel_name('p', 4), false);
	ss[fx_half_bottle] = kernel_load_series(kernel_name('p', 0), false);
	ss[fx_thing_3] = kernel_load_series(kernel_name('p', 6), false);
	ss[fx_pole] = kernel_load_series(kernel_name('p', 5), false);
	ss[fx_two_bottles] = kernel_load_series(kernel_name('p', 1), false);
	ss[fx_mount] = kernel_load_series(kernel_name('p', 7), false);

	seq[fx_half_bottle] = -1;
	local->dyn_half_bottle = -1;
	local->half_bottle_base = 61;

	seq[fx_two_bottles] = -1;
	local->dyn_two_bottles = -1;
	local->two_bottles_base = 75;

	seq[fx_thing_1] = -1;
	local->dyn_thing_1 = -1;
	local->thing_1_base = 228;

	seq[fx_thing_2] = -1;
	local->dyn_thing_2 = -1;
	local->thing_2_base = 273;

	seq[fx_thing_3] = -1;
	local->dyn_thing_3 = -1;
	local->thing_3_base = 300;

	seq[fx_pole] = -1;
	local->dyn_pole = -1;
	local->pole_base = 284;

	seq[fx_smoke] = kernel_seq_forward(ss[fx_smoke], false, 8, 0, 0, 0);
	kernel_seq_depth(seq[fx_smoke], 3);
	kernel_seq_range(seq[fx_smoke], KERNEL_FIRST, KERNEL_LAST);

	set_half_bottle_position();
	set_two_bottles_position();
	set_thing_1_position();
	set_thing_2_position();
	set_thing_3_position();
	set_pole_position();

	local->mount_x = 54;
	seq[fx_mount] = kernel_seq_stamp(ss[fx_mount], false, 1);
	kernel_seq_loc(seq[fx_mount], local->mount_x, 90);
	kernel_seq_depth(seq[fx_mount], 12);

	local->mount_2_x = 280;
	seq[fx_mount_2] = kernel_seq_stamp(ss[fx_mount], true, 1);
	kernel_seq_loc(seq[fx_mount_2], local->mount_2_x, 91);
	kernel_seq_depth(seq[fx_mount_2], 12);

	/* cal */
	aa[2] = kernel_run_animation(kernel_name('c', 1), 0);
	local->anim_2_running = true;
	local->cal_action = FREEZE;
	kernel_reset_animation(aa[2], 90);

	if (global[player_persona] == PLAYER_IS_KING) {
		/* dancer */
		aa[5] = kernel_run_animation(kernel_name('d', 1), 0);
		local->anim_5_running = true;
	}

	if (global[player_persona] == PLAYER_IS_KING) {
		if (local->anim_0_running || previous_room == 407 || previous_room == KERNEL_RESTORING_GAME) {
			aa[0] = kernel_run_animation(kernel_name('k', 1), 0);
			local->king_action = FREEZE;
			local->anim_0_running = true;
			kernel_reset_animation(aa[0], 54);
		} else {
			aa[3] = kernel_run_animation(kernel_name('k', 2), 0);
			local->anim_3_running = true;
		}
	} else {
		if (local->anim_1_running || previous_room == 407 || previous_room == KERNEL_RESTORING_GAME) {
			aa[1] = kernel_run_animation(kernel_name('p', 1), 0);
			local->pid_action = FREEZE;
			local->anim_1_running = true;
			if (local->activate_timer) {
				kernel_reset_animation(aa[1], 79);
			} else {
				kernel_reset_animation(aa[1], 23);
			}
		} else {
			aa[4] = kernel_run_animation(kernel_name('p', 2), 0);
			local->anim_4_running = true;
		}
	}

	if (global[player_persona] == PLAYER_IS_KING) {
		if (conv_restore_running == CONV_41_KING || previous_room == 407) {
			conv_run(CONV_41_KING);
			conv_export_pointer(&global[dance_points]);
			conv_export_pointer(&global[game_points]);
			conv_export_pointer(&global[clue_points]);
			conv_export_value(global[talked_to_soptus]);
			conv_export_value(global[pid_talk_shamon]);
			conv_export_value(local->num_of_stuff);
			conv_export_value(global[prizes_owed_to_player]);
		}
	} else {
		for (count = 1; count <= global[prizes_owed_to_player]; count++) {
			if (object_is_here(dates) && !o1) {
				o1 = true;
			} else if (object_is_here(statue) && !o2) {
				o2 = true;
			} else if (object_is_here(ruby_ring) && !o3) {
				o3 = true;
			} else if (object_is_here(bottle_of_flies) && !o4) {
				o4 = true;
			} else if (object_is_here(soptus_soporific) && !o5) {
				o5 = true;
				local->won_sop = true;
			}
		}

		if (conv_restore_running == CONV_45_PID || previous_room == 407) {
			conv_run(CONV_45_PID);
			conv_export_pointer(&global[pid_has_been_healed_sop]);
			conv_export_value(local->won_sop);
			conv_export_value(global[prizes_owed_to_player]);
		}
	}

	section_4_music();
}

void room_406_daemon() {
	int dif;
	int temp;

	if (local->anim_0_running) {
		handle_animation_king();
	}

	if (local->anim_1_running) {
		handle_animation_pid();
	}

	if (local->anim_2_running) {
		handle_animation_cal();
	}

	if (local->anim_3_running) {
		handle_animation_king_walk();
	}

	if (local->anim_4_running) {
		handle_animation_pid_walk();
	}

	if (local->anim_5_running) {
		handle_animation_belly_before();
	}

	if (local->anim_6_running) {
		handle_animation_belly_after();
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
			kernel_set_interface_mode(INTER_LIMITED_SENTENCES);
			local->pid_action       = DIE;
			local->activate_timer   = false;
			player.commands_allowed = false;
		}
	}

	if (kernel.trigger == ROOM_406_CUP) {
		temp = seq[fx_cup];
		seq[fx_cup] = kernel_seq_stamp(ss[fx_cup], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_cup], 1);
		kernel_synch(KERNEL_SERIES, seq[fx_cup], KERNEL_SERIES, temp);
	}

	if (camera_x.pan_this_frame) {
		set_half_bottle_position();
		set_two_bottles_position();
		set_thing_1_position();
		set_thing_2_position();
		set_thing_3_position();
		set_pole_position();
		set_406_mount_position();
		set_406_mount_2_position();
	}

	if (local->panning == PANNING_AWAY_FROM_DANCE && !camera_x.panning) {
		conv_run(CONV_41_KING);
		conv_export_pointer(&global[dance_points]);
		conv_export_pointer(&global[game_points]);
		conv_export_pointer(&global[clue_points]);
		conv_export_value(global[talked_to_soptus]);
		conv_export_value(global[pid_talk_shamon]);
		conv_export_value(local->num_of_stuff);
		conv_export_value(global[prizes_owed_to_player]);
		local->panning     = false;
		local->king_action = FREEZE;
		local->cal_action  = FREEZE;
	}
}

static void process_conv_king_cal() {
	int you_trig_flag  = false;
	int me_trig_flag   = false;

	if (player_verb == conv041_to_game_b_b) {
		*conv_my_next_start = conv041_postgame;
		conv_abort();
		you_trig_flag = true;
		me_trig_flag  = true;
		new_room = 407;
	}

	if (player_verb == conv041_to_game_d_d) {
		*conv_my_next_start = conv041_leaving;
		conv_abort();
		you_trig_flag = true;
		me_trig_flag  = true;
		new_room = 407;
	}

	if (player_verb == conv041_dance_b_b) {
		sound_play(N_BellyDanceMusic);
		*conv_my_next_start = conv041_indance;
		conv_abort();
		local->king_action = LOOK_GIRL_2;
		local->cal_action  = CLAP;
		you_trig_flag = true;
		me_trig_flag  = true;
	}

	if (player_verb == conv041_postdance_b_b) {
		*conv_my_next_start = conv041_postdanc;
		conv_abort();
		local->king_action = LOOK_GIRL;
		local->cal_action  = LOOK_GIRL;
		you_trig_flag = true;
		me_trig_flag  = true;
	}

	if (player_verb == conv041_danceyn_no) {
		if (!kernel.trigger) {
			global[wins_till_prize] = 3;
		}
	}

	if (player_verb == conv041_postdanc_only) {
		if (!kernel.trigger) {
			sound_play(N_Bk406Music);
			global[dance_music_on] = false;
			if (global[game_points] > 6) {
				global[wins_till_prize] = 1;
			} else {
				global[wins_till_prize] = 2;
			}
		}
	}

	if (player_verb == conv041_exit_b_b ||
	    player_verb == conv041_exit_d_d ||
	    player_verb == conv041_exit_f_f ||
	    player_verb == conv041_give_stuff_f_f) {
		local->king_action = GET_UP;
		you_trig_flag      = true;
		me_trig_flag       = true;
	}

	if (player_verb == conv041_give_stuff_a_a ||
	    player_verb == conv041_give_stuff_c_c) {
		you_trig_flag = true;
		me_trig_flag  = true;
	}

	if (player_verb == conv041_give_stuff_b_b ||
	    player_verb == conv041_give_stuff_d_d) {
		local->cal_action = GIVE;
		you_trig_flag     = true;
		me_trig_flag      = true;
	}

	if (kernel.trigger == ROOM_406_YOU_TALK) {
		if (local->king_action != GIVE) {
			local->cal_action  = TALK;
		}
		if (local->king_action != GET_UP) {
			local->king_action = FREEZE;
		}
	}

	if (kernel.trigger == ROOM_406_ME_TALK) {
		if (local->cal_action != GIVE) {
			local->cal_action  = FREEZE;
		}
		if (local->king_action != GET_UP) {
			local->king_action = TALK;
		}
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_406_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_406_ME_TALK);
	}

	local->cal_talk_count  = 0;
	local->king_talk_count = 0;
}

static void process_conv_pid_cal() {
	int you_trig_flag  = false;
	int me_trig_flag   = false;

	if (player_verb == conv045_to_game_b_b) {
		*conv_my_next_start = conv045_giver;
		conv_abort();
		you_trig_flag = true;
		me_trig_flag  = true;
		new_room = 407;
	}

	if (player_verb == conv045_egaming_only) {
		global[wins_till_prize] = 1;
	}

	if (player_verb == conv045_hgaming_only) {
		global[wins_till_prize] = 2;
	}

	if (player_verb == conv045_exit_b_b) {
		local->pid_action = GET_UP;
		you_trig_flag      = true;
		me_trig_flag       = true;
	}

	if (player_verb == conv045_give_stuff_a_a) {
		you_trig_flag = true;
		me_trig_flag  = true;
	}

	if (player_verb == conv045_moredrink_b_b) {
		if (!kernel.trigger) {
			conv_hold();
			local->cal_action = TAKE_DRINK;
		}
		you_trig_flag     = true;
		me_trig_flag      = true;
	}

	if (player_verb == conv045_passout_b_b) {
		local->pid_action = DRINK;
		you_trig_flag     = true;
		me_trig_flag      = true;
	}

	if (player_verb == conv045_passout_e_e) {
		conv_hold();
		local->pid_action = POST_DRINK;
		you_trig_flag     = true;
		me_trig_flag      = true;
	}

	if (player_verb == conv045_give_stuff_b_b) {
		local->cal_action = GIVE;
		you_trig_flag     = true;
		me_trig_flag      = true;
	}

	if (player_verb == conv045_timer_b_b) {
		*conv_my_next_start = conv045_revive;
		conv_abort();
		kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
		you_trig_flag           = true;
		me_trig_flag            = true;
		player.commands_allowed = true;
		local->activate_timer   = true;
	}

	if (kernel.trigger == ROOM_406_YOU_TALK) {
		if (local->cal_action != GIVE) {
			local->cal_action  = TALK;
		}

		if (local->pid_action != GET_UP &&
		    local->pid_action != TAKE_DRINK &&
		    local->pid_action != DRINK &&
		    local->pid_action != POST_DRINK) {
			local->pid_action = FREEZE;
		}
	}

	if (kernel.trigger == ROOM_406_ME_TALK) {
		if (local->cal_action != GIVE) {
			local->cal_action  = FREEZE;
		}

		if (local->pid_action != GET_UP &&
		    local->pid_action != TAKE_DRINK &&
		    local->pid_action != DRINK &&
		    local->pid_action != POST_DRINK) {
			local->pid_action = TALK;
		}
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_406_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_406_ME_TALK);
	}

	local->cal_talk_count  = 0;
	local->pid_talk_count = 0;
}

void room_406_pre_parser() {
}

void room_406_parser() {
	if (conv_control.running == CONV_41_KING) {
		process_conv_king_cal();
		goto handled;
	}

	if (conv_control.running == CONV_45_PID) {
		process_conv_pid_cal();
		goto handled;
	}

	if (player_said_1(heal_self)) {
		kernel_set_interface_mode(INTER_LIMITED_SENTENCES);
		local->pid_action       = GET_DOLL;
		local->activate_timer   = false;
		player.commands_allowed = false;
		global[player_score]   += 3;
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (!player_has(object_named(player_main_noun))) {
			text_show(40612);
			goto handled;
		}
	} else {
		text_show(40613);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_406_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.king_frame);
	s.syncAsSint16LE(scratch.king_action);
	s.syncAsSint16LE(scratch.king_talk_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.pid_frame);
	s.syncAsSint16LE(scratch.pid_action);
	s.syncAsSint16LE(scratch.pid_talk_count);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.cal_frame);
	s.syncAsSint16LE(scratch.cal_action);
	s.syncAsSint16LE(scratch.cal_talk_count);
	s.syncAsSint16LE(scratch.anim_2_running);
	s.syncAsSint16LE(scratch.king_walk_frame);
	s.syncAsSint16LE(scratch.king_walk_action);
	s.syncAsSint16LE(scratch.king_walk_talk_count);
	s.syncAsSint16LE(scratch.anim_3_running);
	s.syncAsSint16LE(scratch.pid_walk_frame);
	s.syncAsSint16LE(scratch.pid_walk_action);
	s.syncAsSint16LE(scratch.pid_walk_talk_count);
	s.syncAsSint16LE(scratch.anim_4_running);
	s.syncAsSint16LE(scratch.belly_before_frame);
	s.syncAsSint16LE(scratch.belly_before_talk_count);
	s.syncAsSint16LE(scratch.anim_5_running);
	s.syncAsSint16LE(scratch.belly_after_frame);
	s.syncAsSint16LE(scratch.anim_6_running);
	s.syncAsSint16LE(scratch.half_bottle_base);
	s.syncAsSint16LE(scratch.dyn_half_bottle);
	s.syncAsSint16LE(scratch.two_bottles_base);
	s.syncAsSint16LE(scratch.dyn_two_bottles);
	s.syncAsSint16LE(scratch.thing_1_base);
	s.syncAsSint16LE(scratch.dyn_thing_1);
	s.syncAsSint16LE(scratch.thing_2_base);
	s.syncAsSint16LE(scratch.dyn_thing_2);
	s.syncAsSint16LE(scratch.thing_3_base);
	s.syncAsSint16LE(scratch.dyn_thing_3);
	s.syncAsSint16LE(scratch.pole_base);
	s.syncAsSint16LE(scratch.dyn_pole);
	s.syncAsSint16LE(scratch.mount_x);
	s.syncAsSint16LE(scratch.mount_2_x);
	s.syncAsSint16LE(scratch.num_of_stuff);
	s.syncAsSint16LE(scratch.won_sop);
	s.syncAsSint16LE(scratch.clock);
	s.syncAsSint16LE(scratch.death_timer);
	s.syncAsSint16LE(scratch.activate_timer);
	s.syncAsSint16LE(scratch.panning);
}

void room_406_preload() {
	room_init_code_pointer       = room_406_init;
	room_pre_parser_code_pointer = room_406_pre_parser;
	room_parser_code_pointer     = room_406_parser;
	room_daemon_code_pointer     = room_406_daemon;

	global[no_load_walker] = true;

	section_4_walker();
	section_4_interface();
}

void room_406_error() {
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
