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
#include "mads/madsv2/dragonsphere/rooms/room205.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];

	int16 top_slime_frame;
	int16 top_slime_action;
	int16 top_slime_talk_count;
	int16 anim_0_running;

	int16 face_frame;
	int16 face_action;
	int16 face_talk_count;
	int16 anim_1_running;

	int16 eye_frame;
	int16 eye_action;
	int16 eye_talk_count;
	int16 anim_3_running;

	int16 eye;
	int16 rock_talk_count;
	int16 prevent;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define ROOM_205_EYE            60
#define ROOM_205_MOUTH          67
#define ROOM_205_YOU_TALK       72
#define ROOM_205_ME_TALK        74
#define ROOM_205_WALK           78

#define fx_eye                  0
#define fx_mouth                1
#define fx_take                 2
#define fx_cycle_slime          3
#define fx_uncycle_slime        4
#define fx_bridge               5
#define fx_end_bridge_1         6
#define fx_end_bridge_2         7

#define PLAYER_X_FROM_203       112
#define PLAYER_Y_FROM_203       147

#define EYE_LEFT                0
#define EYE_RIGHT               1
#define EYE_MOVING              2

#define CONV_51_KING            51
#define CONV_52_PID             52

#define WALK_1_X                166
#define WALK_1_Y                122

#define WALK_2_X                177
#define WALK_2_Y                110

#define WALK_3_X                195
#define WALK_3_Y                110

#define HEAL_X                  176
#define HEAL_Y                  119

#define MONST_X                 152
#define MONST_Y                 126

#define SHIFT                   0
#define END                     1
#define FREEZE                  2
#define INVIS                   3
#define TALK                    4
#define BLINK                   5

#define TUNNEL_X                169
#define TUNNEL_Y                104


static void handle_anim_top_slime() {
	int top_slime_reset_frame;

	if (kernel_anim[aa[0]].frame != local->top_slime_frame) {
		local->top_slime_frame = kernel_anim[aa[0]].frame;
		top_slime_reset_frame = -1;

		switch (local->top_slime_frame) {

			case 15:
			case 26:

				switch (local->top_slime_action) {

					case SHIFT:
						if (imath_random(1, 4) == 1) {
							top_slime_reset_frame = 17;
						} else {
							top_slime_reset_frame = 0;
						}
						break;

					case END:
						top_slime_reset_frame = 15;
						break;
				}
				break;

			case 17: /* end of heal */
				top_slime_reset_frame = 26;
				break;

			case 27: /* end of invislbe */
				top_slime_reset_frame = 26;
				break;
		}

		if (top_slime_reset_frame >= 0) {
			kernel_reset_animation(aa[0], top_slime_reset_frame);
			local->top_slime_frame = top_slime_reset_frame;
		}
	}
}

static void handle_anim_face() {
	int face_reset_frame;

	if (kernel_anim[aa[1]].frame != local->face_frame) {
		local->face_frame = kernel_anim[aa[1]].frame;
		face_reset_frame = -1;

		switch (local->face_frame) {

			case 10:  /* almost end of hide face */
				player.commands_allowed = true;
				break;

			case 1:  /* end of invis     */
			case 11: /* end of hide face */

				switch (local->face_action) {

					case INVIS:
						face_reset_frame = 0;
						break;

					default:
						face_reset_frame = 1;
						break;
				}
				break;

			case 4: /* end of show face */
			case 5: /* end of freeze    */
			case 6: /* end of talk      */
			case 7: /* end of talk      */
			case 8: /* end of talk      */

				if (local->face_frame == 4) {
					local->eye_action = BLINK;
				}

				switch (local->face_action) {

					case FREEZE:
						face_reset_frame = 4;
						break;

					case TALK:
						face_reset_frame = imath_random(5, 7);
						++ local->face_talk_count;
						if (local->face_talk_count > 22) {
							local->face_action     = FREEZE;
							local->face_talk_count = 0;
							face_reset_frame       = 4;
						}
						break;

					default:
						local->eye_action = INVIS;
						face_reset_frame  = 8;
						break;
				}
				break;
		}

		if (face_reset_frame >= 0) {
			kernel_reset_animation(aa[1], face_reset_frame);
			local->face_frame = face_reset_frame;
		}
	}
}

static void handle_anim_eye() {
	int eye_reset_frame;

	if (kernel_anim[aa[3]].frame != local->eye_frame) {
		local->eye_frame = kernel_anim[aa[3]].frame;
		eye_reset_frame = -1;

		switch (local->eye_frame) {

			case 1:  /* end of invis */
			case 3:  /* end of blink */

				switch (local->eye_action) {

					case BLINK:
						if (imath_random(1, 30) == 1) {
							eye_reset_frame = 1;
						} else {
							eye_reset_frame = 0;
						}
						break;

					default:
						eye_reset_frame = 0;
						break;
				}
				break;
		}

		if (eye_reset_frame >= 0) {
			kernel_reset_animation(aa[3], eye_reset_frame);
			local->eye_frame = eye_reset_frame;
		}
	}
}

static void room_205_init() {
	local->eye             = EYE_LEFT;
	local->prevent         = false;
	local->rock_talk_count = 0;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_3_running = false;
	}

	if (global[player_persona] == PLAYER_IS_KING) {
		conv_get(CONV_51_KING);
	} else {
		conv_get(CONV_52_PID);
	}

	ss[fx_cycle_slime]   = kernel_load_series(kernel_name('z', 0), false);
	ss[fx_uncycle_slime] = kernel_load_series(kernel_name('z', 5), false);
	ss[fx_bridge]        = kernel_load_series(kernel_name('z', 4), false);
	ss[fx_eye]           = kernel_load_series(kernel_name('y', 0), false);
	ss[fx_mouth]         = kernel_load_series(kernel_name('y', 1), false);
	ss[fx_end_bridge_1]  = kernel_load_series(kernel_name('z', 6), false);
	ss[fx_end_bridge_2]  = kernel_load_series(kernel_name('z', 7), false);

	if (global[slime_healed]) {
		seq[fx_end_bridge_1] = kernel_seq_stamp(ss[fx_end_bridge_1], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_end_bridge_1], 8);
		seq[fx_end_bridge_2] = kernel_seq_stamp(ss[fx_end_bridge_2], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_end_bridge_2], 13);

	} else {
		kernel_flip_hotspot_loc(words_ground, false, TUNNEL_X, TUNNEL_Y);
		seq[fx_cycle_slime] = kernel_seq_stamp(ss[fx_cycle_slime], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_cycle_slime], 10);

		aa[3]                 = kernel_run_animation(kernel_name('e', 1), 0);
		local->anim_3_running = true;
		local->eye_action     = INVIS;

		aa[1]                 = kernel_run_animation(kernel_name('s', 2), 0);
		local->anim_1_running = true;
		local->face_action    = INVIS;

		aa[0]                 = kernel_run_animation(kernel_name('s', 1), 0);
		local->anim_0_running = true;
	}

	if (previous_room == 206) {
		player.commands_allowed = false;
		player.x                = WALK_3_X;
		player.y                = WALK_3_Y;
		player.facing           = FACING_WEST;
		player_walk(WALK_2_X, WALK_2_Y, 5);
		player_walk_trigger(ROOM_205_WALK);

	} else if (previous_room == 203 || previous_room != KERNEL_RESTORING_GAME) {
		player.x      = PLAYER_X_FROM_203;
		player.y      = PLAYER_Y_FROM_203;
		player.facing = FACING_NORTHEAST;
	}

	section_2_music();
}

static void room_205_daemon() {
	int temp;

	if (local->anim_0_running) {
		handle_anim_top_slime();
	}

	if (local->anim_1_running) {
		handle_anim_face();
	}

	if (local->anim_3_running) {
		handle_anim_eye();
	}

	if (local->eye != EYE_MOVING && imath_random(1, 400) == 1) {

		if (local->eye == EYE_RIGHT) {
			if (imath_random(1, 2) == 1) {   /* blink */
				kernel_seq_delete(seq[fx_eye]);
				seq[fx_eye] = kernel_seq_pingpong(ss[fx_eye], false, 5, 0, 0, 2);
				kernel_seq_depth(seq[fx_eye], 2);
				kernel_seq_range(seq[fx_eye], 3, 4);
				kernel_seq_trigger(seq[fx_eye], KERNEL_TRIGGER_EXPIRE, 0, ROOM_205_EYE + 1);

			} else {  /* look left */
				kernel_seq_delete(seq[fx_eye]);
				seq[fx_eye] = kernel_seq_backward(ss[fx_eye], false, 8, 0, 0, 1);
				kernel_seq_depth(seq[fx_eye], 2);
				kernel_seq_range(seq[fx_eye], 1, 2);
				kernel_seq_trigger(seq[fx_eye], KERNEL_TRIGGER_EXPIRE, 0, ROOM_205_EYE);
			}

		} else if (local->eye == EYE_LEFT) {  /* look right */
			seq[fx_eye] = kernel_seq_forward(ss[fx_eye], false, 8, 0, 0, 1);
			kernel_seq_depth(seq[fx_eye], 2);
			kernel_seq_range(seq[fx_eye], 1, 2);
			kernel_seq_trigger(seq[fx_eye], KERNEL_TRIGGER_EXPIRE, 0, ROOM_205_EYE + 1);
		}

		local->eye = EYE_MOVING;
	}

	if (kernel.trigger == ROOM_205_EYE) {
		local->eye = EYE_LEFT;
	}

	if (kernel.trigger == ROOM_205_EYE + 1) {
		temp = seq[fx_eye];
		seq[fx_eye] = kernel_seq_stamp(ss[fx_eye], false, 2);
		kernel_seq_depth(seq[fx_eye], 2);
		kernel_synch(KERNEL_SERIES, seq[fx_eye], KERNEL_SERIES, temp);
		local->eye = EYE_RIGHT;
	}

	if (local->rock_talk_count == 1) {
		seq[fx_mouth] = kernel_seq_stamp(ss[fx_mouth], false, imath_random(1, 3));
		kernel_seq_depth(seq[fx_mouth], 1);
		kernel_timing_trigger(9, ROOM_205_MOUTH);
		++ local->rock_talk_count;
	}

	if (kernel.trigger == ROOM_205_MOUTH) {
		++ local->rock_talk_count;
		kernel_seq_delete(seq[fx_mouth]);
		if (local->rock_talk_count < 15) {
			seq[fx_mouth] = kernel_seq_stamp(ss[fx_mouth], false, imath_random(1, 3));
			kernel_seq_depth(seq[fx_mouth], 1);
			kernel_timing_trigger(9, ROOM_205_MOUTH);

		} else {
			kernel_timing_trigger(9, ROOM_205_MOUTH + 1);
			local->rock_talk_count = 0;
		}
	}

	if (kernel.trigger == ROOM_205_MOUTH + 1) {
		text_show(20527);
	}

	if (kernel.trigger == ROOM_205_WALK) {
		player_walk(WALK_1_X, WALK_1_Y, 5);
		player_walk_trigger(ROOM_205_WALK + 1);
	}

	if (kernel.trigger == ROOM_205_WALK) {
		player.commands_allowed = true;
	}
}

static void process_conv_face_king() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	if (player_verb == conv051_exit_b_b) {
		*conv_my_next_start = conv051_nopass;
		conv_abort();
		local->face_action = INVIS;
		you_trig_flag      = true;
		me_trig_flag       = true;
	}

	if (kernel.trigger == ROOM_205_YOU_TALK) {
		local->face_action = TALK;
	}

	if (kernel.trigger == ROOM_205_ME_TALK) {
		local->face_action = FREEZE;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_205_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_205_ME_TALK);
	}

	local->face_talk_count = 0;
}

static void process_conv_face_pid() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	if (player_verb == conv052_exit_b_b) {
		*conv_my_next_start = conv052_nopass;
		conv_abort();
		local->face_action = INVIS;
		you_trig_flag      = true;
		me_trig_flag       = true;
	}

	if (kernel.trigger == ROOM_205_YOU_TALK) {
		local->face_action = TALK;
	}

	if (kernel.trigger == ROOM_205_ME_TALK) {
		local->face_action = FREEZE;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_205_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_205_ME_TALK);
	}

	local->face_talk_count = 0;
}

static void room_205_pre_parser() {
	if (player_said_2(heal, shifting_monster) && !global[slime_healed]) {
		player_walk(HEAL_X, HEAL_Y, FACING_NORTH);
	}

	if (player_said_2(walk_to, shifter_village) ||
	    player_said_2(walk_across, ground) && inter_point_y < 117) {
		if (global[slime_healed]) {
			switch (kernel.trigger) {
			case 0:
				player_walk(WALK_1_X, WALK_1_Y, 5);
				player_walk_trigger(1);
				break;

			case 1:
				player.commands_allowed = false;
				player_walk(WALK_2_X, WALK_2_Y, 5);
				player_walk_trigger(2);
				break;

			case 2:
				player_walk(WALK_3_X, WALK_3_Y, 5);
				break;
			}
		} else {
			player_walk(MONST_X, MONST_Y, FACING_NORTHEAST);
		}
	}
}

static void room_205_parser() {
	int temp;

	if (conv_control.running == CONV_51_KING) {
		process_conv_face_king();
		goto handled;
	}

	if (conv_control.running == CONV_52_PID) {
		process_conv_face_pid();
		goto handled;
	}

	if (player_said_2(walk_down, path_to_south)) {
		new_room = 203;
		goto handled;
	}

	if (player_said_2(heal, shifting_monster)) {
		if (!global[slime_healed]) switch (kernel.trigger) {
			case 0:
				player.walker_visible   = false;
				player.commands_allowed = false;
				local->top_slime_action = END;
				aa[2]                   = kernel_run_animation(kernel_name('h', 1), 1);
				kernel_synch(KERNEL_ANIM, aa[2], KERNEL_PLAYER, 0);
				break;

			case 1:
				kernel_abort_animation(aa[2]);
				kernel_abort_animation(aa[0]);
				kernel_seq_delete(seq[fx_cycle_slime]);
				local->anim_0_running   = false;
				player.walker_visible   = true;
				seq[fx_bridge]          = kernel_seq_forward(ss[fx_bridge], false, 7, 0, 0, 1);
				kernel_seq_depth(seq[fx_bridge], 8);
				kernel_seq_range(seq[fx_bridge], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_bridge], KERNEL_TRIGGER_EXPIRE, 0, 2);
				kernel_synch(KERNEL_SERIES, seq[fx_bridge], KERNEL_NOW, 0);
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				break;

			case 2:
				temp                    = seq[fx_bridge];
				player.commands_allowed = true;
				global[slime_healed]    = true;
				global[player_score]   += 2;
				seq[fx_end_bridge_1] = kernel_seq_stamp(ss[fx_end_bridge_1], false, KERNEL_LAST);
				kernel_seq_depth(seq[fx_end_bridge_1], 8);
				seq[fx_end_bridge_2] = kernel_seq_stamp(ss[fx_end_bridge_2], false, KERNEL_LAST);
				kernel_seq_depth(seq[fx_end_bridge_2], 13);

				kernel_synch(KERNEL_SERIES, seq[fx_end_bridge_1], KERNEL_SERIES, temp);
				kernel_synch(KERNEL_SERIES, seq[fx_end_bridge_2], KERNEL_SERIES, temp);
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				kernel_flip_hotspot_loc(words_ground, true, TUNNEL_X, TUNNEL_Y);
				break;

		} else {
			if (global[slime_healed] == 1) {
				text_show(20533);

			} else if (global[slime_healed] == 2) {
				text_show(20534);

			} else {
				text_show(20528);
			}
		}
		goto handled;
	}

	if (player_said_2(talk_to, shifting_monster)) {
		if (global[slime_healed]) {
			text_show(20529);

		} else {
			player.commands_allowed = false;
			if (global[player_persona] == PLAYER_IS_KING) {
				conv_run(CONV_51_KING);

			} else {
				conv_run(CONV_52_PID);
				conv_export_value(true);
				conv_export_value(true);
			}
		}
		goto handled;
	}

	if (player_said_2(walk_to, shifter_village) ||
	    player_said_2(walk_across, ground) && inter_point_y < 117) {
		if (global[slime_healed]) {
			new_room = 206;

		} else {
			if (global[player_persona] == PLAYER_IS_KING) {
				conv_run(CONV_51_KING);

			} else {
				conv_run(CONV_52_PID);
				conv_export_value(true);
				conv_export_value(true);
			}
		}
		goto handled;
	}

	if (player_said_2(invoke, signet_ring)) {
		if (global[object_given_201] != -1) {
			text_show(20158);
			goto handled;
		}
	}

	if (player.look_around) {
		if (global[slime_healed]) {
			text_show(20518);
		} else {
			text_show(20501);
		}
		goto handled;
	}

	if (player_said_3(sword, attack, shifting_monster) ||
	    player_said_3(sword, carve_up, shifting_monster) ||
	    player_said_3(sword, thrust, shifting_monster) ||
	    player_said_2(pour_contents_of, shifting_monster) ||
	    player_said_2(push, shifting_monster) ||
	    player_said_2(pull, shifting_monster) ||
	    player_said_2(look, shifting_monster) ||
	    player_said_2(open, shifting_monster) ||
	    player_said_2(close, shifting_monster)) {

		if (global[slime_healed]) {
			text_show(20523);
			goto handled;
		}
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(Slathan_ni_Patan)) {
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(20502);
			} else {
				text_show(20519);
			}
			goto handled;
		}

		if (player_said_1(shifter_boulder)) {
			text_show(20503);
			goto handled;
		}

		if (player_said_1(dead_tree)) {
			text_show(20506);
			goto handled;
		}

		if (player_said_1(rock_tumble)) {
			text_show(20508);
			goto handled;
		}

		if (player_said_1(shifter_village)) {
			if (global[slime_healed]) {
				text_show(20521);
			} else {
				text_show(20509);
			}
			goto handled;
		}

		if (player_said_1(pit)) {
			text_show(20511);
			goto handled;
		}

		if (player_said_1(path_to_south)) {
			text_show(20512);
			goto handled;
		}

		if (player_said_1(Slathan_sky)) {
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(20513);
			} else {
				text_show(20522);
			}
			goto handled;
		}

		if (player_said_1(ground)) {
			text_show(20514);
			goto handled;
		}

		if (player_said_1(shifting_monster)) {
			text_show(20515);
			goto handled;
		}

		if (player_said_1(mouth_rock)) {
			text_show(20526);
			goto handled;
		}
	}

	if (player_said_2(talk_to, mouth_rock)) {
		sound_play(N_MouthRockTalks);
		local->rock_talk_count = 1;
		goto handled;
	}

	if (player_said_2(talk_to, shifter_boulder)) {
		text_show(20505);
		goto handled;
	}

	if (player_said_1(take) || player_said_1(push) ||
	    player_said_1(pull)) {

		if (player_said_1(boulder)) {
			text_show(20504);
			goto handled;
		}

		if (player_said_1(dead_tree)) {
			text_show(20507);
			goto handled;
		}
	}

	if (player_said_2(make_noise, birdcall)) {
		text_show(20510);
		goto handled;
	}

	if (player_said_3(sword, attack, shifting_monster) ||
	    player_said_3(sword, carve_up, shifting_monster) ||
	    player_said_3(sword, thrust, shifting_monster) ||
	    player_said_2(pour_contents_of, shifting_monster) ||
	    player_said_2(push, shifting_monster) ||
	    player_said_2(pull, shifting_monster) ||
	    player_said_2(open, shifting_monster) ||
	    player_said_2(close, shifting_monster)) {

		text_show(20516);
		goto handled;
	}

	if (player_said_2(take_magic_from, shifting_monster)) {
		text_show(20517);
		goto handled;
	}

	if (player_said_2(speak_words_on, parchment)) {
		text_show(20524);
		goto handled;
	}

	if (player_said_2(heal, dead_tree)) {
		text_show(20520);
		goto handled;
	}

	if (player_said_2(wear, key_crown) && !global[slime_healed]) {
		text_show(20525);
		goto handled;
	}

	if (player_said_2(gaze_into, crystal_ball)) {
		if (!global[slime_healed]) {
			text_show(20530);
			goto handled;
		}
	}

	if (player_said_1(shifter_village)) {
		if (!global[slime_healed]) {
			text_show(20535);
			goto handled;
		}
	}

	if (player_said_2(invoke_power_of, crystal_ball) ||
	    player_said_1(heal_self)) {
		if (!global[slime_healed]) {
			switch (kernel.trigger) {
				case 0:
					if (player_said_1(heal_self)) {
						sound_play(N_002HealMeSnd);

					} else {
						sound_play(N_InvokeCrystalBall);
					}
					kernel_abort_animation(aa[0]);
					kernel_seq_delete(seq[fx_cycle_slime]);
					local->anim_0_running   = false;
					player.commands_allowed = false;
					local->top_slime_action = END;
					seq[fx_bridge]          = kernel_seq_forward(ss[fx_bridge], false, 7, 0, 0, 1);
					global[player_score]   += 2;
					kernel_seq_depth(seq[fx_bridge], 8);
					kernel_seq_range(seq[fx_bridge], KERNEL_FIRST, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_bridge], KERNEL_TRIGGER_EXPIRE, 0, 1);
					kernel_synch(KERNEL_SERIES, seq[fx_bridge], KERNEL_NOW, 0);
					kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
					goto handled;
					break;

				case 1:
					temp                    = seq[fx_bridge];
					player.commands_allowed = true;
					seq[fx_end_bridge_1]    = kernel_seq_stamp(ss[fx_end_bridge_1], false, KERNEL_LAST);
					seq[fx_end_bridge_2]    = kernel_seq_stamp(ss[fx_end_bridge_2], false, KERNEL_LAST);
					kernel_seq_depth(seq[fx_end_bridge_1], 8);
					kernel_seq_depth(seq[fx_end_bridge_2], 13);

					kernel_synch(KERNEL_SERIES, seq[fx_end_bridge_1], KERNEL_SERIES, temp);
					kernel_synch(KERNEL_SERIES, seq[fx_end_bridge_2], KERNEL_SERIES, temp);
					kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
					kernel_flip_hotspot_loc(words_ground, true, TUNNEL_X, TUNNEL_Y);

					if (player_said_1(crystal_ball)) {
						text_show(20531);
						global[crystal_ball_dead] = true;
						inter_move_object(crystal_ball, NOWHERE);
						global[slime_healed] = 2;
						text_show(970);

					} else {
						text_show(20532);
						global[slime_healed] = 1;
					}
					player.commands_allowed = true;
					goto handled;
					break;
			}
		}
	}


	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_205_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.top_slime_frame);
	s.syncAsSint16LE(scratch.top_slime_action);
	s.syncAsSint16LE(scratch.top_slime_talk_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.face_frame);
	s.syncAsSint16LE(scratch.face_action);
	s.syncAsSint16LE(scratch.face_talk_count);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.eye_frame);
	s.syncAsSint16LE(scratch.eye_action);
	s.syncAsSint16LE(scratch.eye_talk_count);
	s.syncAsSint16LE(scratch.anim_3_running);
	s.syncAsSint16LE(scratch.eye);
	s.syncAsSint16LE(scratch.rock_talk_count);
	s.syncAsSint16LE(scratch.prevent);
}

void room_205_preload() {
	room_init_code_pointer       = room_205_init;
	room_pre_parser_code_pointer = room_205_pre_parser;
	room_parser_code_pointer     = room_205_parser;
	room_daemon_code_pointer     = room_205_daemon;

	section_2_walker();
	section_2_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
