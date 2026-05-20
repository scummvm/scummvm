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
#include "mads/madsv2/dragonsphere/rooms/section4.h"
#include "mads/madsv2/dragonsphere/rooms/room401.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[23];
	int16 sequence[23];
	int16 animation[4];
	int16 sop_frame;
	int16 sop_action;
	int16 sop_talk_count;
	int16 anim_0_running;
	int16 prevent;
	int16 moving;
	int16 cut_scene;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

static Scratch scratch;

#define fx_red_blue_tent        0
#define fx_bones                1
#define fx_stone_marker         2
#define fx_red_rocks            3
#define fx_distant_dune         4
#define fx_medium_dune          5
#define fx_small_dune           6
#define fx_dune_rocks           7
#define fx_large_dune           8
#define fx_bush                 9
#define fx_large_dune_cactus    10
#define fx_large_round_dune     11
#define fx_grassy_dune          12
#define fx_medium_dunes         13
#define fx_large_tent           14
#define fx_small_tent           15
#define fx_tumbleweed           16
#define fx_00                   17
#define fx_take                 18
#define fx_disp_6               19
#define fx_disp_9               20
#define fx_disp_3               21
#define fx_fire                 22

#define ROOM_401_ME_TALK        60
#define ROOM_401_YOU_TALK       62
#define ROOM_401_HANDS_UP       64
#define ROOM_401_LAUGH          66
#define ROOM_401_DEATH          69
#define ROOM_401_LIFE           72
#define ROOM_401_TEXT           78
#define MUSIC                   100

#define CONV_38_SOPTUS          38

#define BONES_X                 236
#define BONES_Y                 143
#define TENT_X                  64
#define TENT_Y                  151
#define MARKER_X                193
#define MARKER_Y                139
#define FIREPIT_X               193
#define FIREPIT_Y               139
#define LEAVE_SOUTH_X           79
#define LEAVE_SOUTH_Y           150

#define RED_ROCKS_X             45
#define RED_ROCKS_Y             155
#define BUSH_X                  289
#define BUSH_Y                  155
#define MEDIUM_DUNE_X           270
#define MEDIUM_DUNE_Y           155
#define DUNE_ROCKS_X            227
#define DUNE_ROCKS_Y            155
#define LARGE_DUNE_CACTUS_X     279
#define LARGE_DUNE_CACTUS_Y     155
#define LARGE_ROUND_DUNE_X      300
#define LARGE_ROUND_DUNE_Y      155
#define MEDIUM_DUNES_X          270
#define MEDIUM_DUNES_Y          155

#define GRASSY_DUNE_X           29
#define GRASSY_DUNE_Y           131
#define LARGE_DUNE_X            32
#define LARGE_DUNE_Y            126
#define SMALL_DUNE_X            230
#define SMALL_DUNE_Y            133
#define DISTANT_DUNE_X          280
#define DISTANT_DUNE_Y          128

#define LARGE_TENT_X            70
#define LARGE_TENT_Y            130
#define SMALL_TENT_X            217
#define SMALL_TENT_Y            127
#define TUMBLEWEED_X            108
#define TUMBLEWEED_Y            149

#define FROM_NORTH_X            163
#define FROM_NORTH_Y            131
#define FROM_SOUTH_X            161
#define FROM_SOUTH_Y            143
#define FROM_EAST_X_1           330
#define FROM_EAST_Y_1           143
#define FROM_EAST_X_2           302
#define FROM_EAST_Y_2           143
#define FROM_WEST_X_1           -15
#define FROM_WEST_Y_1           143
#define FROM_WEST_X_2           15
#define FROM_WEST_Y_2           143

#define BIG_DESERT_X            140
#define BIG_DESERT_Y            136
#define DESERT_1_X              22
#define DESERT_1_Y              146
#define DESERT_2_X              36
#define DESERT_2_Y              146
#define DESERT_3_X              52
#define DESERT_3_Y              146
#define DESERT_4_X              60
#define DESERT_4_Y              146
#define DESERT_5_X              74
#define DESERT_5_Y              146
#define DESERT_6_X              87
#define DESERT_6_Y              146
#define DESERT_SOUTH_1_X        38
#define DESERT_SOUTH_1_Y        153
#define DESERT_SOUTH_2_X        78
#define DESERT_SOUTH_2_Y        153
#define DESERT_SOUTH_3_X        109
#define DESERT_SOUTH_3_Y        151

#define SHUT_UP                 0
#define TALK                    1
#define HANDS_UP                2
#define POINT_N                 3
#define POINT_S                 4
#define POINT_E                 5
#define POINT_W                 6
#define LAUGH                   7
#define NOD                     8

#define DESERTT_1_X             107
#define DESERTT_1_Y             137
#define DESERTT_2_X             121
#define DESERTT_2_Y             137

#define PID_BONES_X             225
#define PID_BONES_Y             143


static void send_variables() {
	int count;
	int xx;
	int it = 0;

	for (xx = 0; xx < 2; xx++) {
		if (xx == 0)
			it = global[oasis];
		if (xx == 1)
			it = global[fire_holes];

		switch (it) {
		case 7:
			for (count = 0; count < 5; count++) {
				conv_export_value(1);
			}
			break;

		case 13:
			for (count = 0; count < 2; count++) {
				conv_export_value(1);
			}
			conv_export_value(4);
			for (count = 0; count < 2; count++) {
				conv_export_value(1);
			}
			break;

		case 19:
			conv_export_value(1);
			conv_export_value(4);
			for (count = 0; count < 2; count++) {
				conv_export_value(1);
			}
			conv_export_value(4);
			break;

		case 25:
			conv_export_value(1);
			conv_export_value(4);
			conv_export_value(1);
			for (count = 0; count < 2; count++) {
				conv_export_value(4);
			}
			break;

		case 31:
			for (count = 0; count < 2; count++) {
				conv_export_value(4);
			}
			conv_export_value(1);
			for (count = 0; count < 4; count++) {
				conv_export_value(4);
			}
			break;

		case 37:
			for (count = 0; count < 5; count++) {
				conv_export_value(4);
			}
			break;

		case 45:
			conv_export_value(4);
			conv_export_value(2);
			for (count = 0; count < 3; count++) {
				conv_export_value(4);
			}
			break;

		case 53:
			conv_export_value(4);
			conv_export_value(2);
			conv_export_value(4);
			conv_export_value(2);
			conv_export_value(4);
			break;

		case 61:
			conv_export_value(2);
			for (count = 0; count < 2; count++) {
				conv_export_value(4);
			}
			for (count = 0; count < 2; count++) {
				conv_export_value(2);
			}
			break;

		case 69:
			for (count = 0; count < 3; count++) {
				conv_export_value(2);
			}
			conv_export_value(4);
			conv_export_value(2);
			break;

		case 77:
			for (count = 0; count < 5; count++) {
				conv_export_value(2);
			}
			break;
		}
	}
}

static void handle_animation_sop() {
	int sop_reset_frame;

	if (kernel_anim[aa[0]].frame != local->sop_frame) {
		local->sop_frame = kernel_anim[aa[0]].frame;
		sop_reset_frame = -1;

		switch (local->sop_frame) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 9:
		case 68:
		case 63:
			switch (local->sop_action) {
			case SHUT_UP:
				if (local->sop_frame >= 5) {
					local->sop_frame = 1;
				}

				++local->sop_talk_count;
				if (local->sop_talk_count > imath_random(15, 25)) {
					if (local->sop_frame == 1) {
						sop_reset_frame = imath_random(0, 1);
					} else if (local->sop_frame == 2) {
						sop_reset_frame = imath_random(0, 2);
					} else if (local->sop_frame == 3) {
						sop_reset_frame = imath_random(1, 3);
					} else if (local->sop_frame == 4) {
						sop_reset_frame = imath_random(2, 3);
					}
					local->sop_talk_count = 0;

				} else {
					sop_reset_frame = local->sop_frame - 1;
				}
				break;

			case HANDS_UP:
				sop_reset_frame   = 4;
				local->sop_action = TALK;
				break;

			case POINT_E:
				sop_reset_frame   = 63;
				local->sop_action = TALK;
				break;

			case POINT_N:
				sop_reset_frame   = 56;
				local->sop_action = TALK;
				break;

			default:
				sop_reset_frame = 30;
				break;
			}
			break;

		case 25:
		case 30:
		case 31:
		case 32:
		case 45:
		case 26:
		case 29:
		case 56:
		case 69:
			switch (local->sop_action) {
			case TALK:
				if (local->sop_talk_count == 0) {
					if (imath_random(1, 2) == 1) {
						sop_reset_frame = 32;
					} else {
						sop_reset_frame = 30;
						++local->sop_talk_count;
					}

				} else {
					sop_reset_frame = imath_random(29, 31);
					++local->sop_talk_count;
					if (local->sop_talk_count > 17) {
						local->sop_action     = SHUT_UP;
						local->sop_talk_count = 0;
						sop_reset_frame       = 30;
					}
				}
				break;

			case SHUT_UP:
				++local->sop_talk_count;
				if (local->sop_talk_count > imath_random(15, 25)) {
					sop_reset_frame = imath_random(0, 1);
					if (sop_reset_frame == 1) {
						sop_reset_frame = 30;
					}

				} else {
					sop_reset_frame = 30;
				}
				break;

			case POINT_S:
				sop_reset_frame   = 38;
				local->sop_action = TALK;
				break;

			case POINT_W:
				sop_reset_frame   = 45;
				local->sop_action = TALK;
				break;

			case LAUGH:
				sop_reset_frame   = 10;
				local->sop_action = SHUT_UP;
				break;

			case NOD:
				sop_reset_frame = 26;
				break;

			case HANDS_UP:
			case POINT_E:
			case POINT_N:
				sop_reset_frame = 0;
				break;
			}
			break;

		case 33:
		case 34:
		case 35:
		case 36:
			switch (local->sop_action) {
			case TALK:
				sop_reset_frame = imath_random(33, 35);
				++local->sop_talk_count;
				if (local->sop_talk_count > 17) {
					local->sop_action     = SHUT_UP;
					local->sop_talk_count = 0;
					sop_reset_frame       = 68;
				}
				break;

			default:
				sop_reset_frame = 68;
				break;
			}
			break;
		}

		if (sop_reset_frame >= 0) {
			kernel_reset_animation(aa[0], sop_reset_frame);
			local->sop_frame = sop_reset_frame;
		}
	}
}

static void process_conv_sop() {
	int you_trig_flag  = false;
	int me_trig_flag   = false;

	switch (player_verb) {
	case conv038_third_idont:
	case conv038_knowledge_no:
	case conv038_knowledge_idont:
	case conv038_fourth_nound:
		if (!kernel.trigger) {
			conv_you_trigger(ROOM_401_HANDS_UP);
		}
		you_trig_flag = true;
		me_trig_flag  = true;
		break;

	case conv038_knowledge_yes:
		if (!kernel.trigger) {
			conv_you_trigger(ROOM_401_LIFE);
		}
		you_trig_flag = true;
		me_trig_flag  = true;
		break;

	case conv038_third_yes:
		if (!kernel.trigger) {
			conv_you_trigger(ROOM_401_DEATH);
		}
		you_trig_flag = true;
		me_trig_flag  = true;
		break;

	case conv038_third_blab:
		if (!kernel.trigger) {
			conv_you_trigger(ROOM_401_LAUGH);
		}
		you_trig_flag = true;
		me_trig_flag  = true;
		break;
	}

	if (kernel.trigger == ROOM_401_DEATH) {
		if (global[fire_holes] < 37) {
			local->sop_action = POINT_N;
		} else if (global[fire_holes] > 37) {
			local->sop_action = POINT_S;
		} else {
			local->sop_action = POINT_W;
		}
	}

	if (kernel.trigger == ROOM_401_LIFE) {
		if (global[oasis] < 37) {
			local->sop_action = POINT_N;
		} else if (global[oasis] > 37) {
			local->sop_action = POINT_S;
		} else {
			local->sop_action = POINT_W;
		}
	}

	if (kernel.trigger == ROOM_401_HANDS_UP) {
		local->sop_action = HANDS_UP;
	}

	if (kernel.trigger == ROOM_401_LAUGH) {
		local->sop_action = LAUGH;
	}

	if (kernel.trigger == ROOM_401_ME_TALK) {
		local->sop_action = SHUT_UP;
	}

	if (kernel.trigger == ROOM_401_YOU_TALK) {
		local->sop_action = TALK;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_401_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_401_ME_TALK);
	}

	local->sop_talk_count = 0;
}

static void room_401_init() {
	int count;
	int count2 = global[fire_holes] - 1;
	int skip   = 0;

	int front_right = false;
	int front_left  = false;
	int back_left   = false;
	int back_right  = false;


	if (global[player_persona] == PLAYER_IS_PID) {
		global[perform_displacements] = true;
	}

	if (global[desert_room] == 42 && global[player_persona] == PLAYER_IS_KING) {
		conv_get(CONV_38_SOPTUS);
	}

	local->cut_scene         = false;
	local->prevent           = false;
	ss[fx_bush]              = -1;
	ss[fx_red_rocks]         = -1;
	ss[fx_medium_dune]       = -1;
	ss[fx_large_dune_cactus] = -1;
	ss[fx_medium_dunes]      = -1;
	ss[fx_00]                = -1;

	local->moving  = false;
	ss[fx_disp_3]  = kernel_load_series("*KG4DIS3", false);
	ss[fx_disp_6]  = kernel_load_series("*KG4DIS6", false);
	ss[fx_disp_9]  = kernel_load_series("*KG4DIS9", false);

	if (global[desert_room] != 42) {
		kernel_flip_hotspot(words_tent, false);
		kernel_flip_hotspot(words_sign, false);
		kernel_flip_hotspot(words_bones, false);
		kernel_flip_hotspot(words_trader, false);
		kernel_flip_hotspot(words_firepit, false);
		kernel_flip_hotspot(words_lean_to, false);

	} else if (global[player_persona] == PLAYER_IS_PID || player_has_been_in_room(405)) {
		kernel_flip_hotspot(words_trader, false);

	} else if (global[player_persona] == PLAYER_IS_KING) {
		kernel_flip_hotspot_loc(words_desert, false, DESERTT_1_X, DESERTT_1_Y);
		kernel_flip_hotspot_loc(words_desert, true,  DESERTT_2_X, DESERTT_2_Y);
	}

	if (global[player_persona] == PLAYER_IS_KING) {
		ss[fx_take] = kernel_load_series("*KGRL_6", false);
	} else {
		ss[fx_take] = kernel_load_series("*PDRL_9", false);
	}

	if (global[desert_room] == 42) {
		global[desert_counter] = 0;

		kernel_room_scale(155, 80, 129, 60);
		ss[fx_red_blue_tent]  = kernel_load_series("*DUNE04", false);
		seq[fx_red_blue_tent] = kernel_seq_stamp(ss[fx_red_blue_tent], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_red_blue_tent], 3);
		kernel_seq_loc(seq[fx_red_blue_tent], TENT_X, TENT_Y);

		ss[fx_bones]  = kernel_load_series("*DUNE05", false);
		seq[fx_bones] = kernel_seq_stamp(ss[fx_bones], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_bones], 5);
		kernel_seq_loc(seq[fx_bones], BONES_X, BONES_Y);

		ss[fx_stone_marker]  = kernel_load_series("*DUNE06", false);
		seq[fx_stone_marker] = kernel_seq_stamp(ss[fx_stone_marker], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_stone_marker], 6);
		kernel_seq_loc(seq[fx_stone_marker], MARKER_X, MARKER_Y);

		ss[fx_fire]  = kernel_load_series(kernel_name('h', 0), false);
		seq[fx_fire] = kernel_seq_stamp(ss[fx_fire], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_fire], 14);

		kernel_flip_hotspot_loc(words_desert_to_south, false, DESERT_SOUTH_1_X, DESERT_SOUTH_1_Y);
		kernel_flip_hotspot_loc(words_desert_to_south, false, DESERT_SOUTH_2_X, DESERT_SOUTH_2_Y);
		kernel_flip_hotspot_loc(words_desert_to_south, false, DESERT_SOUTH_3_X, DESERT_SOUTH_3_Y);

	} else {
		kernel_flip_hotspot_loc(words_desert, false, DESERT_1_X, DESERT_1_Y);
		kernel_flip_hotspot_loc(words_desert, false, DESERT_2_X, DESERT_2_Y);
		kernel_flip_hotspot_loc(words_desert, false, DESERT_3_X, DESERT_3_Y);
		kernel_flip_hotspot_loc(words_desert, false, DESERT_4_X, DESERT_4_Y);
		kernel_flip_hotspot_loc(words_desert, false, DESERT_5_X, DESERT_5_Y);
		kernel_flip_hotspot_loc(words_desert, false, DESERT_6_X, DESERT_6_Y);
		kernel_flip_hotspot_loc(words_desert, true,  BIG_DESERT_X, BIG_DESERT_Y);

		for (count = 0; count < 77; count++) {
			++skip; if (skip == 7) skip = 1;
			++count2; if (count2 == 78) count2 = 1;

			if (count2 == global[desert_room]) {
				switch (skip) {
				case 1:
					ss[fx_medium_dune]  = kernel_load_series("*DUNE07", false);
					seq[fx_medium_dune] = kernel_seq_stamp(ss[fx_medium_dune], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_medium_dune], 1);
					kernel_seq_loc(seq[fx_medium_dune], MEDIUM_DUNE_X, MEDIUM_DUNE_Y);
					break;

				case 2:
					ss[fx_dune_rocks]  = kernel_load_series("*DUNE09", false);
					seq[fx_dune_rocks] = kernel_seq_stamp(ss[fx_dune_rocks], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_dune_rocks], 1);
					kernel_seq_loc(seq[fx_dune_rocks], DUNE_ROCKS_X, DUNE_ROCKS_Y);
					break;

				case 3:
					ss[fx_bush]  = kernel_load_series("*DUNE11", false);
					seq[fx_bush] = kernel_seq_stamp(ss[fx_bush], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_bush], 1);
					kernel_seq_loc(seq[fx_bush], BUSH_X, BUSH_Y);
					break;

				case 4:
					ss[fx_large_dune_cactus]  = kernel_load_series("*DUNE12", false);
					seq[fx_large_dune_cactus] = kernel_seq_stamp(ss[fx_large_dune_cactus], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_large_dune_cactus], 1);
					kernel_seq_loc(seq[fx_large_dune_cactus], LARGE_DUNE_CACTUS_X, LARGE_DUNE_CACTUS_Y);
					break;

				case 5:
					ss[fx_large_round_dune]  = kernel_load_series("*DUNE13", false);
					seq[fx_large_round_dune] = kernel_seq_stamp(ss[fx_large_round_dune], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_large_round_dune], 1);
					kernel_seq_loc(seq[fx_large_round_dune], LARGE_ROUND_DUNE_X, LARGE_ROUND_DUNE_Y);
					break;

				case 6:
					ss[fx_medium_dunes]  = kernel_load_series("*DUNE15", false);
					seq[fx_medium_dunes] = kernel_seq_stamp(ss[fx_medium_dunes], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_medium_dunes], 1);
					kernel_seq_loc(seq[fx_medium_dunes], MEDIUM_DUNES_X, MEDIUM_DUNES_Y);
					break;
				}

				if (skip <= 6) front_right = true;
			}
		}

		/* put down LEFT foreground sprites */

		count2 = 29;
		skip   = 0;

		for (count = 0; count < 77; count++) {
			++skip; if (skip == 9) skip = 1;
			++count2; if (count2 == 78) count2 = 1;

			if (count2 == global[desert_room]) {
				switch (skip) {
				case 1:
					if (ss[fx_red_rocks] == -1) {
						ss[fx_red_rocks]  = kernel_load_series("*DUNE01", false);
						seq[fx_red_rocks] = kernel_seq_stamp(ss[fx_red_rocks], false, KERNEL_FIRST);
						kernel_seq_depth(seq[fx_red_rocks], 1);
						kernel_seq_loc(seq[fx_red_rocks], RED_ROCKS_X, RED_ROCKS_Y);
					}
					break;

				case 2:
					if (ss[fx_bush] == -1) {
						ss[fx_bush]  = kernel_load_series("*DUNE11", false);
						seq[fx_bush] = kernel_seq_stamp(ss[fx_bush], false, KERNEL_FIRST);
						kernel_seq_depth(seq[fx_bush], 1);
						kernel_seq_loc(seq[fx_bush], 0, BUSH_Y);
					}
					break;

				case 3:
					if (ss[fx_medium_dune] == -1) {
						ss[fx_medium_dune]  = kernel_load_series("*DUNE07", false);
						seq[fx_medium_dune] = kernel_seq_stamp(ss[fx_medium_dune], false, KERNEL_FIRST);
						kernel_seq_depth(seq[fx_medium_dune], 1);
						kernel_seq_loc(seq[fx_medium_dune], 0, MEDIUM_DUNE_Y);
					}
					break;

				case 4:
					if (ss[fx_large_dune_cactus] == -1) {
						ss[fx_large_dune_cactus]  = kernel_load_series("*DUNE12", false);
						seq[fx_large_dune_cactus] = kernel_seq_stamp(ss[fx_large_dune_cactus], false, KERNEL_FIRST);
						kernel_seq_depth(seq[fx_large_dune_cactus], 1);
						kernel_seq_loc(seq[fx_large_dune_cactus], 0, LARGE_DUNE_CACTUS_Y);
					}
					break;

				case 5:
					if (ss[fx_medium_dunes] == -1) {
						ss[fx_medium_dunes]  = kernel_load_series("*DUNE15", false);
						seq[fx_medium_dunes] = kernel_seq_stamp(ss[fx_medium_dunes], false, KERNEL_FIRST);
						kernel_seq_depth(seq[fx_medium_dunes], 1);
						kernel_seq_loc(seq[fx_medium_dunes], 0, MEDIUM_DUNES_Y);
					}
					break;

				case 6:
					if (ss[fx_00] == -1) {
						ss[fx_00]  = kernel_load_series("*DUNE00", false);
						seq[fx_00] = kernel_seq_stamp(ss[fx_00], false, KERNEL_FIRST);
						kernel_seq_depth(seq[fx_00], 1);
						kernel_seq_loc(seq[fx_00], 0, MEDIUM_DUNES_Y);
					}
					break;
				}

				if (skip <= 6) front_left = true;
			}
		}

		/* put down LEFT background sprites */

		count2 = global[oasis] - 1;
		skip   = 0;

		for (count = 0; count < 77; count++) {
			++skip; if (skip == 5) skip = 1;
			++count2; if (count2 == 78) count2 = 1;

			if (count2 == global[desert_room]) {

				switch (skip) {
				case 1:
					ss[fx_large_dune]  = kernel_load_series("*DUNE10", false);
					seq[fx_large_dune] = kernel_seq_stamp(ss[fx_large_dune], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_large_dune], 14);
					kernel_seq_loc(seq[fx_large_dune], LARGE_DUNE_X, LARGE_DUNE_Y);
					break;

				case 2:
					ss[fx_grassy_dune]  = kernel_load_series("*DUNE14", false);
					seq[fx_grassy_dune] = kernel_seq_stamp(ss[fx_grassy_dune], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_grassy_dune], 14);
					kernel_seq_loc(seq[fx_grassy_dune], GRASSY_DUNE_X, GRASSY_DUNE_Y);
					break;
				}

				if (skip <= 2) back_left  = true;
			}
		}

		/* put down RIGHT background sprites */

		count2 = global[oasis] - 1;
		skip   = 0;

		for (count = 0; count < 77; count++) {

			++skip; if (skip == 4) skip = 1;
			++count2; if (count2 == 78) count2 = 1;

			if (count2 == global[desert_room]) {
				switch (skip) {
				case 1:
					if (front_right && !back_left) {
						ss[fx_distant_dune]  = kernel_load_series("*DUNE02", false);
						seq[fx_distant_dune] = kernel_seq_stamp(ss[fx_distant_dune], false, KERNEL_FIRST);
						kernel_seq_depth(seq[fx_distant_dune], 14);
						kernel_seq_loc(seq[fx_distant_dune], 40, DISTANT_DUNE_Y);

					} else if (front_left && !back_right) {
						ss[fx_distant_dune]  = kernel_load_series("*DUNE02", false);
						seq[fx_distant_dune] = kernel_seq_stamp(ss[fx_distant_dune], false, KERNEL_FIRST);
						kernel_seq_depth(seq[fx_distant_dune], 14);
						kernel_seq_loc(seq[fx_distant_dune], DISTANT_DUNE_X, DISTANT_DUNE_Y);
					}
					break;
				}

				if (skip == 1) back_right = true;
			}
		}
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room == 404) {
			global[desert_counter] = 0;
			player.x               = FROM_EAST_X_2;
			player.y               = FROM_EAST_Y_2;
			player.facing          = FACING_WEST;

		} else {
			switch (global[from_direction]) {
			case FROM_NORTH:
				player.x = FROM_NORTH_X;
				player.y = FROM_NORTH_Y;
				player.facing = FACING_SOUTH;
				break;

			case FROM_SOUTH:
				player.x = FROM_SOUTH_X;
				player.y = FROM_SOUTH_Y;
				player.facing = FACING_NORTH;
				break;

			case FROM_EAST:
				player_first_walk(FROM_EAST_X_1, FROM_EAST_Y_1, FACING_WEST,
					FROM_EAST_X_2, FROM_EAST_Y_2, FACING_WEST, true);
				break;

			case FROM_WEST:
				player_first_walk(FROM_WEST_X_1, FROM_WEST_Y_1, FACING_EAST,
					FROM_WEST_X_2, FROM_WEST_Y_2, FACING_EAST, true);
				break;
			}
		}
	}

	if (global[desert_room] == 42 && global[player_persona] == PLAYER_IS_KING &&
	    !player_has_been_in_room(405)) {
		aa[0]                 = kernel_run_animation(kernel_name('s', 1), 0);
		local->anim_0_running = true;
		local->sop_action     = SHUT_UP;

		if (conv_restore_running == CONV_38_SOPTUS) {
			conv_run(CONV_38_SOPTUS);
			send_variables();
		}
	}

	section_4_music();
}

static void room_401_daemon() {
	int temp;

	if (local->anim_0_running) {
		handle_animation_sop();
	}

	if (player.walker_visible && (player.commands_allowed || (conv_control.running >= 0)) && !player.walking &&
	   (player.facing == player.turn_to_facing) && !local->moving && global[player_persona] == PLAYER_IS_KING) {

		switch (player.facing) {
		case FACING_EAST:
		case FACING_WEST:
			if (imath_random(1, 500) == 1) {
				if (imath_random(1, 2) == 1) {
					player.walker_visible = false;
					local->moving         = true;
					if (player.facing == FACING_EAST) {
						seq[fx_disp_6] = kernel_seq_forward(ss[fx_disp_6], false, 6, 0, 0, 1);
					} else {
						seq[fx_disp_6] = kernel_seq_forward(ss[fx_disp_6], true, 6, 0, 0, 1);
					}
					kernel_seq_player(seq[fx_disp_6], true);
					kernel_seq_range(seq[fx_disp_6], 8, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_disp_6], KERNEL_TRIGGER_EXPIRE, 0, 1);

				} else {
					player.walker_visible = false;
					local->moving         = true;
					if (player.facing == FACING_EAST) {
						seq[fx_disp_6] = kernel_seq_forward(ss[fx_disp_6], false, 6, 0, 0, 1);
					} else {
						seq[fx_disp_6] = kernel_seq_forward(ss[fx_disp_6], true, 6, 0, 0, 1);
					}
					kernel_seq_player(seq[fx_disp_6], true);
					kernel_seq_range(seq[fx_disp_6], 1, 7);
					kernel_seq_trigger(seq[fx_disp_6], KERNEL_TRIGGER_EXPIRE, 0, 2);
				}
			}
			break;

		case FACING_SOUTHEAST:
		case FACING_SOUTHWEST:
			if (imath_random(1, 500) == 1) {
				if (imath_random(1, 2) == 1) {
					player.walker_visible = false;
					local->moving         = true;
					if (player.facing == FACING_SOUTHEAST) {
						seq[fx_disp_3] = kernel_seq_forward(ss[fx_disp_3], false, 6, 0, 0, 1);
					} else {
						seq[fx_disp_3] = kernel_seq_forward(ss[fx_disp_3], true, 6, 0, 0, 1);
					}
					kernel_seq_player(seq[fx_disp_3], true);
					kernel_seq_range(seq[fx_disp_3], 1, 17);
					kernel_seq_trigger(seq[fx_disp_3], KERNEL_TRIGGER_EXPIRE, 0, 1);

				} else {
					player.walker_visible = false;
					local->moving         = true;
					if (player.facing == FACING_SOUTHEAST) {
						seq[fx_disp_3] = kernel_seq_forward(ss[fx_disp_3], false, 6, 0, 0, 1);
					} else {
						seq[fx_disp_3] = kernel_seq_forward(ss[fx_disp_3], true, 6, 0, 0, 1);
					}
					kernel_seq_player(seq[fx_disp_3], true);
					kernel_seq_range(seq[fx_disp_3], 18, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_disp_3], KERNEL_TRIGGER_EXPIRE, 0, 4);
				}
			}
			break;


		case FACING_NORTHWEST:
		case FACING_NORTHEAST:
			if (imath_random(1, 500) == 1) {
				player.walker_visible = false;
				local->moving         = true;
				if (player.facing == FACING_NORTHEAST) {
					seq[fx_disp_9] = kernel_seq_forward(ss[fx_disp_9], false, 6, 0, 0, 1);
				} else {
					seq[fx_disp_9] = kernel_seq_forward(ss[fx_disp_9], true, 6, 0, 0, 1);
				}
				kernel_seq_player(seq[fx_disp_9], true);
				kernel_seq_range(seq[fx_disp_9], 1, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_disp_9], KERNEL_TRIGGER_EXPIRE, 0, 1);
			}
			break;
		}
	}

	if (local->moving) {
		switch (kernel.trigger) {
		case 1:
			player.walker_visible = true;
			local->moving = false;
			if (player.facing == FACING_WEST || player.facing == FACING_EAST) {
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_disp_6]);

			} else if (player.facing == FACING_NORTHWEST || player.facing == FACING_NORTHEAST) {
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_disp_9]);

			} else if (player.facing == FACING_SOUTHWEST || player.facing == FACING_SOUTHEAST) {
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_disp_3]);
			}
			break;

		case 2:
			temp = seq[fx_disp_6];
			if (player.facing == FACING_EAST) {
				seq[fx_disp_6] = kernel_seq_stamp(ss[fx_disp_6], false, 7);
			} else {
				seq[fx_disp_6] = kernel_seq_stamp(ss[fx_disp_6], true, 7);
			}
			kernel_seq_player(seq[fx_disp_6], false);
			kernel_timing_trigger(imath_random(30, 150), 3);
			kernel_synch(KERNEL_SERIES, seq[fx_disp_6], KERNEL_SERIES, temp);
			break;

		case 3:
			kernel_seq_delete(seq[fx_disp_6]);
			if (player.facing == FACING_EAST) {
				seq[fx_disp_6] = kernel_seq_backward(ss[fx_disp_6], false, 6, 0, 0, 1);
			} else {
				seq[fx_disp_6] = kernel_seq_backward(ss[fx_disp_6], true, 6, 0, 0, 1);
			}
			kernel_seq_player(seq[fx_disp_6], false);
			kernel_seq_range(seq[fx_disp_6], 1, 7);
			kernel_seq_trigger(seq[fx_disp_6], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 4:
			temp = seq[fx_disp_3];
			if (player.facing == FACING_SOUTHEAST) {
				seq[fx_disp_3] = kernel_seq_stamp(ss[fx_disp_3], false, KERNEL_LAST);
			} else {
				seq[fx_disp_3] = kernel_seq_stamp(ss[fx_disp_3], true, KERNEL_LAST);
			}
			kernel_seq_player(seq[fx_disp_3], false);
			kernel_timing_trigger(imath_random(30, 150), 5);
			kernel_synch(KERNEL_SERIES, seq[fx_disp_3], KERNEL_SERIES, temp);
			break;

		case 5:
			kernel_seq_delete(seq[fx_disp_3]);
			if (player.facing == FACING_SOUTHEAST) {
				seq[fx_disp_3] = kernel_seq_backward(ss[fx_disp_3], false, 6, 0, 0, 1);
			} else {
				seq[fx_disp_3] = kernel_seq_backward(ss[fx_disp_3], true, 6, 0, 0, 1);
			}
			kernel_seq_player(seq[fx_disp_3], false);
			kernel_seq_range(seq[fx_disp_3], 18, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_disp_3], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;
		}
	}

	if (kernel.trigger == ROOM_401_TEXT) {
		text_show(40103);
	}

	if (kernel.trigger == MUSIC) {
		sound_play(N_WindWhistles);
		sound_play(N_BackgroundMus);
	}

	if (local->cut_scene && !player.walking) {
		local->cut_scene = false;
		global[dragon_my_scene]--;
	}
}

static void room_401_pre_parser() {
	if (local->moving) {
		switch (player.facing) {
		case FACING_EAST:
		case FACING_WEST:
			kernel_seq_delete(seq[fx_disp_6]);
			player.walker_visible = true;
			break;

		case FACING_NORTHEAST:
		case FACING_NORTHWEST:
			kernel_seq_delete(seq[fx_disp_9]);
			player.walker_visible = true;
			break;

		case FACING_SOUTHEAST:
		case FACING_SOUTHWEST:
			kernel_seq_delete(seq[fx_disp_3]);
			player.walker_visible = true;
			break;
		}
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		local->moving = false;
	}

	if (global[desert_room] == 42) {
		if (player_said_2(cross, desert_to_south) && inter_point_x < 98) {
			player_walk(LEAVE_SOUTH_X, LEAVE_SOUTH_Y, FACING_SOUTH);
		}
	}

	if (player_said_1(cross)) {
		if (!player_said_1(desert_to_east) && local->cut_scene) {
			local->cut_scene = false;
			global[dragon_my_scene]--;
		}

		if (player_has_been_in_room(405) && player_said_1(desert_to_west)) {
			global[from_direction]       = FROM_EAST;
			player.walk_off_edge_to_room = 405;

		} else if (player_has_been_in_room(405) && player_said_1(desert_to_east)) {
			global[pre_room] = 401;
			if (global[dragon_my_scene] < global[dragon_high_scene]) {
				if (!local->cut_scene) {
					global[dragon_my_scene]++;
				}
				player.walk_off_edge_to_room = 111;
				local->cut_scene             = true;
			} else {
				if (!local->cut_scene) {
					player.walk_off_edge_to_room = 120;
				}
			}

		} else if (player_said_1(desert_to_east) && global[desert_room] % 7 == 0) {
			global[pre_room] = 401;
			if (global[dragon_my_scene] < global[dragon_high_scene]) {
				if (!local->cut_scene) {
					global[dragon_my_scene]++;
				}
				player.walk_off_edge_to_room = 111;
				local->cut_scene             = true;

			} else if (local->cut_scene) {
				player.walk_off_edge_to_room = 111;

			} else {
				player.walk_off_edge_to_room = 120;
			}

		} else if (player_said_1(desert_to_east) || player_said_1(desert_to_west)) {
			player.walk_off_edge_to_room = 400;
		}
	}

	if (player_said_2(take, bones) && global[player_persona] == PLAYER_IS_PID) {
		player_walk(PID_BONES_X, PID_BONES_Y, FACING_NORTHEAST);
	}
}

static void room_401_parser() {
	int count;
	int count2 = global[oasis] - 1;
	int roomNum = 400;

	if (conv_control.running == CONV_38_SOPTUS) {
		process_conv_sop();
		goto handled;
	}

	if (player_said_2(talk_to, trader)) {
		conv_run(CONV_38_SOPTUS);
		send_variables();
		goto handled;
	}

	if (player_said_2(take, bones)) {
		switch (kernel.trigger) {
		case 0:
			if (!player_has(bone)) {
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take] = kernel_seq_pingpong(ss[fx_take], false, 6, 0, 0, 2);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_EXPIRE, 0, 2);
				kernel_seq_depth(seq[fx_take], 3);
				kernel_seq_range(seq[fx_take], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_player(seq[fx_take], true);
				if (global[player_persona] == PLAYER_IS_KING) {
					kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 5, 1);
				} else {
					kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 4, 1);
				}
				goto handled;
			}
			break;

		case 1:
			if (local->prevent) {
				if (!(global[player_score_flags] & SCORE_TAKE_BONE)) {
					global[player_score_flags] = global[player_score_flags] | SCORE_TAKE_BONE;
					global[player_score] += 1;
				}
				sound_play(N_TakeObjectSnd);
				inter_give_to_player(bone);
				object_examine(bone, 40111, 0);
			}
			local->prevent = true;
			goto handled;
			break;

		case 2:
			player.walker_visible   = true;
			player.commands_allowed = true;
			local->prevent          = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take]);
			goto handled;
			break;
		}
	}

	if (player_said_2(take, bones) && player_has(bone)) {
		text_show(40112);
		goto handled;
	}

	if (player_said_1(cross)) {
		if (player_said_1(desert_to_north) || player_said_1(desert_to_south)) {
			if (player_said_1(desert_to_north)) {
				global[desert_room] = global[desert_room] - 7;
				global[from_direction] = FROM_SOUTH;

			} else if (player_said_1(desert_to_south)) {
				global[desert_room] += 7;
				global[from_direction] = FROM_NORTH;
			}

			for (count = 0; count < 77; count++) {
				++roomNum;
				if (roomNum == 404)
					roomNum = 401;
				++count2;
				if (count2 == 78)
					count2 = 1;

				if (count2 == global[desert_room]) {
					if (global[desert_room] == 42)                 roomNum = 401;
					if (global[desert_room] == global[oasis])      roomNum = 454;
					if (global[desert_room] == global[fire_holes]) roomNum = 412;
					goto over;
				}
			}

over:

			++global[desert_counter];
			if (player_has_been_in_room(405)) {
				global[from_direction] = FROM_EAST;
				new_room               = 405;

			} else if (global[desert_counter] == 6 && !player_has_been_in_room(405)) {
				new_room = 404;

			} else if (room_id == roomNum) {
				kernel.force_restart = true;

			} else {
				new_room = roomNum;
			}
			goto handled;
		}
	}

	if (player.look_around) {
		if (global[desert_room] == 42) {
			text_show(40101);
		} else {
			text_show(40201);
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(desert_to_north) ||
		    player_said_1(desert_to_south) ||
		    player_said_1(desert_to_east)  ||
		    player_said_1(desert_to_west)) {
			text_show(40102);
			goto handled;
		}

		if (player_said_1(sky)) {
			text_show(40103);
			goto handled;
		}

		if (player_said_1(lean_to)) {
			text_show(40105);
			goto handled;
		}

		if (player_said_1(trader)) {
			text_show(40106);
			goto handled;
		}

		if (player_said_1(bones)) {
			text_show(40107);
			goto handled;
		}

		if (player_said_1(sign)) {
			text_show(40108);
			goto handled;
		}

		if (player_said_1(firepit)) {
			text_show(40118);
			goto handled;
		}
	}

	if (player_said_2(take, desert)) {
		text_show(40104);
		goto handled;
	}

	if (player_said_2(take, lean_to)) {
		text_show(40109);
		goto handled;
	}

	if (player_said_2(take, trader)) {
		text_show(40110);
		goto handled;
	}

	if (player_said_2(take, sign)) {
		text_show(40113);
		goto handled;
	}

	if (player_said_2(open, lean_to) ||
	    player_said_2(close, lean_to)) {
		text_show(40114);
		goto handled;
	}

	if (player_said_2(push, lean_to) ||
	    player_said_2(pull, lean_to)) {
		text_show(40115);
		goto handled;
	}

	if (player_said_2(pull, sign)) {
		text_show(40116);
		goto handled;
	}

	if (player_said_2(give, trader)) {
		text_show(40117);
		goto handled;
	}

	if (player_said_2(put, firepit) ||
	    player_said_2(throw, firepit)) {
		text_show(40119);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_401_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.sop_frame);
	s.syncAsSint16LE(scratch.sop_action);
	s.syncAsSint16LE(scratch.sop_talk_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.prevent);
	s.syncAsSint16LE(scratch.moving);
	s.syncAsSint16LE(scratch.cut_scene);
}

void room_401_preload() {
	room_init_code_pointer = room_401_init;
	room_pre_parser_code_pointer = room_401_pre_parser;
	room_parser_code_pointer = room_401_parser;
	room_daemon_code_pointer = room_401_daemon;

	if (global[desert_room] == 42) {
		kernel_initial_variant = 1;
	}

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
