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
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section3.h"
#include "mads/madsv2/dragonsphere/rooms/room301.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];

	int16 girl_frame;
	int16 girl_action;
	int16 girl_talk_count;
	int16 anim_0_running;
	int16 cut_scene;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_guard                1

#define ROOM_301_ME_TALK        60
#define ROOM_301_YOU_TALK       62
#define MUSIC                   80

#define PLAYER_X_FROM_120       335
#define PLAYER_Y_FROM_120       117
#define WALK_TO_X_FROM_120      298
#define WALK_TO_Y_FROM_120      117

#define PLAYER_X_FROM_302       55
#define PLAYER_Y_FROM_302       113
#define WALK_TO_X_FROM_302      95
#define WALK_TO_Y_FROM_302      113

#define CONV_55_KING            55

#define FREEZE                  0
#define POINT                   1
#define TONGUE                  2
#define TEASE                   3
#define TALK                    4
#define LOOK_LEFT               5
#define TALK_POINT              6

#define BUB_X                   92
#define BUB_Y                   114

#define GUARD_X                 68
#define GUARD_Y                 102


static void handle_anim_girl() {
	int girl_reset_frame;
	int it;

	if (kernel_anim[aa[0]].frame != local->girl_frame) {
		local->girl_frame = kernel_anim[aa[0]].frame;
		girl_reset_frame = -1;

		switch (local->girl_frame) {
		case 1:  /* end of freeze & talk */
		case 21: /* end of talk          */
		case 33: /* end of tongue        */
		case 86: /* end of tease         */
		case 63: /* end of point left    */
		case 52: /* end of look left     */
			switch (local->girl_action) {
			case FREEZE:
				girl_reset_frame = 0;
				if (player.x < 160 && player.y > 100) {
					local->girl_action = LOOK_LEFT;
					girl_reset_frame   = 33;

				} else if (player.y > 90) {
					it = imath_random(1, 110);
					local->girl_talk_count = 0;

					if (it == 1) {
						girl_reset_frame = 20;
						local->girl_action = TONGUE;

					} else if (it == 2) {
						girl_reset_frame = 63;
					}
				}
				break;

			case TALK_POINT:
				girl_reset_frame   = 4;
				local->girl_action = POINT;
				break;

			case POINT:
				girl_reset_frame   = 52;
				local->girl_action = FREEZE;
				break;

			case TONGUE:
				girl_reset_frame   = 21;
				local->girl_action = FREEZE;
				break;

			case TEASE:
				girl_reset_frame   = 63;
				local->girl_action = FREEZE;
				break;

			case LOOK_LEFT:
				girl_reset_frame = 33;
				break;

			case TALK:
				if (imath_random(1, 2) == 1) {
					girl_reset_frame = 4;
					local->girl_action = FREEZE;
				} else {
					girl_reset_frame = 1;
				}
				break;
			}
			break;

		case 27:  /* giving the tongue */
			++local->girl_talk_count;
			if (local->girl_talk_count > imath_random(10, 15)) {
				local->girl_action = FREEZE;
			}

			if (local->girl_action == TONGUE) {
				girl_reset_frame = 26;
			}
			break;

		case 57: /* make guardian go away */
			kernel_seq_delete(seq[fx_guard]);
			kernel_flip_hotspot(words_guardian, false);
			aa[2] = kernel_run_animation(kernel_name('g', 1), 0);
			player.commands_allowed = true;
			break;

		case 42: /* looking left */
			if (local->girl_action == LOOK_LEFT) {
				girl_reset_frame = 41;

				if (player.x > 159) {
					local->girl_action = FREEZE;
					girl_reset_frame   = 42;
				}
			}
			break;

		case 2: /* end of talk */
		case 3:
		case 4:
			switch (local->girl_action) {
			case TALK:
				girl_reset_frame = imath_random(1, 3);
				++local->girl_talk_count;
				if (local->girl_talk_count > 15) {
					local->girl_action     = FREEZE;
					local->girl_talk_count = 0;
					girl_reset_frame       = 0; /* make girl shut up */
				}
				break;

			default:
				girl_reset_frame = 0; /* make girl shut up */
				break;
			}
			break;
		}

		if (girl_reset_frame >= 0) {
			kernel_reset_animation(aa[0], girl_reset_frame);
			local->girl_frame = girl_reset_frame;
		}
	}
}

static void room_301_init() {
	kernel.disable_fastwalk = true;
	local->cut_scene        = false;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
	}

	if (global[bubbles_up_in_301]) {

		ss[fx_guard]  = kernel_load_series(kernel_name('x', 8), false);

		seq[fx_guard] = kernel_seq_forward(ss[fx_guard], false, 6, 0, 0, 0);
		kernel_seq_depth(seq[fx_guard], 3);
		kernel_seq_loc(seq[fx_guard], GUARD_X, GUARD_Y);
		kernel_seq_range(seq[fx_guard], KERNEL_FIRST, KERNEL_LAST);

	} else {
		kernel_flip_hotspot(words_guardian, false);
	}

	if (global[player_persona] == PLAYER_IS_KING) {
		conv_get(CONV_55_KING);

		aa[0]                 = kernel_run_animation(kernel_name('f', 1), 0);
		local->anim_0_running = true;
		local->girl_action    = FREEZE;

	} else {
		kernel_flip_hotspot(words_faerie, false);
		local->anim_0_running = false;
	}

	if (previous_room == 302) {
		player_first_walk(PLAYER_X_FROM_302, PLAYER_Y_FROM_302, FACING_EAST,
		                  WALK_TO_X_FROM_302, WALK_TO_Y_FROM_302, FACING_EAST, true);

	} else if ((previous_room == 120) || (previous_room != KERNEL_RESTORING_GAME)) {
		player_first_walk(PLAYER_X_FROM_120, PLAYER_Y_FROM_120, FACING_WEST,
		                  WALK_TO_X_FROM_120, WALK_TO_Y_FROM_120, FACING_WEST, true);
	}

	section_3_music();
}

static void room_301_daemon() {
	if (local->anim_0_running) {
		handle_anim_girl();
	}

	if (kernel.trigger == MUSIC) {
		sound_play(N_BackgroundMus);
	}

	if (local->cut_scene && !player.walking) {
		local->cut_scene = false;
		global[dragon_my_scene]--;
	}
}

static void process_conv_girl() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	if (player_verb == conv055_exit_b_b) {
		*conv_my_next_start      = conv055_restart;
		conv_abort();
		local->girl_action        = TALK_POINT;
		global[bubbles_up_in_301] = false;
		global[player_score]     += 2;
		you_trig_flag             = true;
		me_trig_flag              = true;
		player.commands_allowed   = false;
	}

	if (kernel.trigger == ROOM_301_YOU_TALK) {
		if (player_verb == conv055_greet_only) {
			local->girl_action = TEASE;
		} else if (local->girl_action != POINT && local->girl_action != TALK_POINT) {
			local->girl_action = TALK;
		}
	}

	if (kernel.trigger == ROOM_301_ME_TALK) {
		local->girl_action = FREEZE;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_301_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_301_ME_TALK);
	}

	local->girl_talk_count = 0;
}

static void room_301_pre_parser() {
	if (!player_said_2(walk_down, path_to_east) && local->cut_scene) {
		local->cut_scene = false;
		global[dragon_my_scene]--;
	}

	if (player_said_2(walk_down, path_to_east)) {
		global[pre_room] = 301;
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

	if (player_said_1(entrance_to_maze) && player.need_to_walk) {
		if (global[bubbles_up_in_301]) {
			player_walk(BUB_X, BUB_Y, FACING_WEST);

		} else if (!player_said_1(walk_through)) {
			player.need_to_walk = false;
		}
	}
}

static void room_301_parser() {
	if (conv_control.running == CONV_55_KING) {
		process_conv_girl();
		goto handled;
	}

	if (player_said_2(talk_to, faerie)) {
		conv_run(CONV_55_KING);
		goto handled;
	}

	if (player_said_2(walk_through, entrance_to_maze)) {
		if (global[bubbles_up_in_301]) {
			text_show(30101);
		} else {
			new_room = 302;
		}
		goto handled;
	}

	if (player.look_around) {
		text_show(30122);
		if (global[bubbles_up_in_301]) {
			text_show(30123);
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(mushroom)) {
			text_show(30102);
			goto handled;
		}

		if (player_said_1(ground)) {
			text_show(30105);
			goto handled;
		}

		if (player_said_1(hedge)) {
			text_show(30106);
			goto handled;
		}

		if (player_said_1(boulder)) {
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(30110);
			} else {
				text_show(30111);
			}
			goto handled;
		}

		if (player_said_1(path_to_east)) {
			text_show(30112);
			goto handled;
		}

		if (player_said_1(faerie)) {
			text_show(30113);
			goto handled;
		}

		if (player_said_1(rock)) {
			text_show(30124);
			goto handled;
		}

		if (player_said_1(guardian)) {
			text_show(30125);
			goto handled;
		}

		if (player_said_1(entrance_to_maze)) {
			if (global[bubbles_up_in_301]) {
				text_show(30118);
			} else {
				text_show(30119);
			}
			goto handled;
		}

		if (player_said_1(grass)) {
			text_show(30120);
			goto handled;
		}

		if (player_said_1(topiary_toad)) {
			text_show(30128);
			goto handled;
		}
	}

	if (player_said_2(talk_to, topiary_toad) || player_said_2(give, topiary_toad)) {
		text_show(30129);
		goto handled;
	}

	if (player_said_3(sword, attack, mushroom) ||
	    player_said_3(sword, carve_up, mushroom) ||
	    player_said_3(sword, thrust, mushroom) ||
	    player_said_2(take, mushroom) ||
	    player_said_2(pull, mushroom)) {
		text_show(30103);
		goto handled;
	}

	if (player_said_3(sword, attack, hedge) ||
	    player_said_3(sword, carve_up, hedge) ||
	    player_said_3(sword, thrust, hedge) ||
	    player_said_2(take, hedge) ||
	    player_said_2(open, hedge) ||
	    player_said_2(push, hedge) ||
	    player_said_2(pour_contents_of, hedge) ||
	    player_said_2(pull, hedge)) {
		text_show(30107);
		goto handled;
	}

	if (player_said_3(sword, attack, faerie) ||
	    player_said_3(sword, carve_up, faerie) ||
	    player_said_3(sword, thrust, faerie) ||
	    player_said_2(push, faerie) ||
	    player_said_2(pour_contents_of, faerie) ||
	    player_said_2(throw, faerie) ||
	    player_said_2(pull, faerie)) {
		text_show(30114);
		goto handled;
	}

	if (player_said_3(sword, attack, grass) ||
	    player_said_3(sword, carve_up, grass) ||
	    player_said_3(sword, thrust, grass) ||
	    player_said_2(take, grass) ||
	    player_said_2(pour_contents_of, grass) ||
	    player_said_2(pull, grass)) {
		text_show(30121);
		goto handled;
	}

	if (player_said_2(open, flies)) {
		text_show(30108);
		goto handled;
	}

	if (player_said_2(put, mushroom)) {
		text_show(30104);
		goto handled;
	}

	if (player_said_2(take_magic_from, entrance_to_maze)) {
		if (global[bubbles_up_in_301]) {
			text_show(30109);
			goto handled;
		}
	}

	if (player_said_2(give, faerie)) {
		text_show(30115);
		goto handled;
	}

	if (player_said_2(talk_to, guardian)) {
		text_show(30126);
		goto handled;
	}

	if (player_said_2(take, bush) || player_said_2(pull, bush)) {
		text_show(30117);
		goto handled;
	}

	if (player_said_1(push) ||
	    player_said_1(pull) ||
	    player_said_2(sword, attack) ||
	    player_said_2(sword, carve_up) ||
	    player_said_2(sword, thrust) ||
	    player_said_1(pour_contents_of)) {

		if (player_said_1(guardian)) {
			text_show(30127);
			goto handled;
		}
	}

	if (player_said_2(take_magic_from, guardian)) {
		text_show(30220);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_301_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.girl_frame);
	s.syncAsSint16LE(scratch.girl_action);
	s.syncAsSint16LE(scratch.girl_talk_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.cut_scene);
}

void room_301_preload() {
	room_init_code_pointer       = room_301_init;
	room_pre_parser_code_pointer = room_301_pre_parser;
	room_parser_code_pointer     = room_301_parser;
	room_daemon_code_pointer     = room_301_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
