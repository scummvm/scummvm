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
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section3.h"
#include "mads/madsv2/dragonsphere/rooms/room302.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];

	int16 move_counter[8];
	int16 direction[8];
	int16 sprite_num[8];
	int16 bubble_x[8];
	int16 bubble_y[8];
	int16 dyna[8];

	int16 prevent;

	int16 death_frame;
	int16 anim_0_running;
	int16 bubbles_here;

	int16 leave;
	int16 at_end;

	int16 change_counter;

	int16 guard_1_x;
	int16 guard_1_y;
	int16 guard_2_x;
	int16 guard_2_y;
	int16 guard_leave;
	int16 guard_1_frame;
	int16 guard_2_frame;

	int16 xxx;
	int16 run_conv;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_bubble               9
#define fx_guard_1              10
#define fx_guard_2              11

#define ROOM_302_BUBBLES        60
#define ROOM_302_GUARDS         70

#define PLAYER_X_FROM_301       335
#define PLAYER_Y_FROM_301       110
#define WALK_TO_X_FROM_301      300
#define WALK_TO_Y_FROM_301      110

#define PLAYER_X_FROM_303       -20
#define PLAYER_Y_FROM_303       110
#define WALK_TO_X_FROM_303      20
#define WALK_TO_Y_FROM_303      110

#define RED                     0
#define YELLOW                  1
#define BLUE                    2

#define NORTHEAST               1
#define NORTH                   2
#define NORTHWEST               3
#define WEST                    4
#define SOUTHWEST               5
#define SOUTH                   6
#define SOUTHEAST               7
#define EAST                    8

#define MAX_LEFT_X              30
#define MAX_RIGHT_X             200

#define MAX_TOP_Y               20
#define MAX_BOTTOM_Y            110

#define MAX_MOVE_COUNTER        15

#define CONV_56_BUBBLE          56

#define DEATH_X                 99
#define DEATH_Y                 108

#define GUARD_1_X               36
#define GUARD_1_Y               60
#define GUARD_2_X               14
#define GUARD_2_Y               100


static int get_direction(int dir, int count) {
	int it = 0;

	if (local->bubble_x[count] >= MAX_RIGHT_X) {
		if (dir > 8) {
			dir = WEST;
		}
	}

	if (local->bubble_y[count] >= MAX_BOTTOM_Y) {
		if (dir > 8) {
			dir = NORTH;
		}
	}

	if (local->bubble_y[count] <= MAX_TOP_Y) {
		if (dir > 8) {
			dir = SOUTH;
		}
	}

	if (local->bubble_x[count] <= MAX_LEFT_X) {
		if (dir > 8) {
			dir = EAST;
		}
	}

	switch (dir) {
	case NORTH:
		if (imath_random(1, 2) == 1) {
			it = SOUTHWEST;
			--local->bubble_x[count];
			++local->bubble_y[count];

		} else {
			it = SOUTHEAST;
			++local->bubble_x[count];
			++local->bubble_y[count];
		}
		break;

	case SOUTH:
		if (imath_random(1, 2) == 1) {
			it = NORTHWEST;
			--local->bubble_x[count];
			--local->bubble_y[count];

		} else {
			it = NORTHEAST;
			++local->bubble_x[count];
			--local->bubble_y[count];
		}
		break;

	case EAST:
		if (imath_random(1, 2) == 1) {
			it = NORTHWEST;
			--local->bubble_x[count];
			--local->bubble_y[count];

		} else {
			it = SOUTHWEST;
			--local->bubble_x[count];
			++local->bubble_y[count];
		}
		break;

	case WEST:
		if (imath_random(1, 2) == 1) {
			it = SOUTHEAST;
			++local->bubble_x[count];
			++local->bubble_y[count];

		} else {
			it = NORTHEAST;
			++local->bubble_x[count];
			--local->bubble_y[count];
		}
		break;

	case NORTHEAST:
		if (local->bubble_x[count] >= MAX_RIGHT_X) {
			it = NORTHWEST;
			--local->bubble_x[count];
			--local->bubble_y[count];

		} else if (local->bubble_y[count] <= MAX_TOP_Y) {
			it = SOUTHEAST;
			++local->bubble_x[count];
			++local->bubble_y[count];

		} else {
		}
		break;

	case SOUTHEAST:
		if (local->bubble_x[count] >= MAX_RIGHT_X) {
			it = SOUTHWEST;
			--local->bubble_x[count];
			++local->bubble_y[count];

		} else if (local->bubble_y[count] >= MAX_BOTTOM_Y) {
			it = NORTHEAST;
			++local->bubble_x[count];
			--local->bubble_y[count];
		} else {
		}
		break;

	case NORTHWEST:
		if (local->bubble_y[count] <= MAX_TOP_Y) {
			it = SOUTHWEST;
			--local->bubble_x[count];
			++local->bubble_y[count];

		} else if (local->bubble_x[count] <= MAX_LEFT_X) {
			it = NORTHEAST;
			++local->bubble_x[count];
			--local->bubble_y[count];
		} else {
		}
		break;

	case SOUTHWEST:
		if (local->bubble_y[count] >= MAX_BOTTOM_Y) {
			it = NORTHWEST;
			--local->bubble_x[count];
			--local->bubble_y[count];

		} else if (local->bubble_x[count] <= MAX_LEFT_X) {
			it = SOUTHEAST;
			++local->bubble_x[count];
			++local->bubble_y[count];
		}
		break;
	}

	return it;
}

static void animate_bubbles() {
	int count;
	int it;

	kernel_purge_dynamic();

	++local->change_counter;
	if (local->change_counter == 2) {
		local->change_counter = 0;
	}

	for (count = 0; count < 8; count++) {
		++local->move_counter[count];

		++local->sprite_num[count];
		if (local->sprite_num[count] >= 30) {
			local->sprite_num[count] = 1;
		}

		if (local->move_counter[count] > MAX_MOVE_COUNTER) {
			local->move_counter[count] = 0;

			switch (local->direction[count]) {
			case NORTH:
				local->direction[count] = (int16)imath_random(1, 3);
				break;

			case SOUTH:
				local->direction[count] = (int16)imath_random(5, 7);
				break;

			case EAST:
				it = imath_random(1, 3);
				if (it == 1) local->direction[count] = NORTHEAST;
				if (it == 2) local->direction[count] = EAST;
				if (it == 3) local->direction[count] = SOUTHEAST;
				break;

			case WEST:
				local->direction[count] = (int16)imath_random(3, 5);
				break;

			case NORTHEAST:
				it = imath_random(1, 3);
				if (it == 1) local->direction[count] = NORTH;
				if (it == 2) local->direction[count] = NORTHEAST;
				if (it == 3) local->direction[count] = EAST;
				break;

			case SOUTHEAST:
				local->direction[count] = (int16)imath_random(6, 8);
				break;

			case NORTHWEST:
				local->direction[count] = (int16)imath_random(2, 4);
				break;

			case SOUTHWEST:
				local->direction[count] = (int16)imath_random(4, 6);
				break;
			}
		}

		if (local->bubble_x[count] >= MAX_RIGHT_X  ||
		    local->bubble_y[count] >= MAX_BOTTOM_Y ||
		    local->bubble_y[count] <= MAX_TOP_Y    ||
		    local->bubble_x[count] <= MAX_LEFT_X) {

			local->direction[count]    = (int16)get_direction(local->direction[count], count);
			local->move_counter[count] = 0;
		}

		if (local->change_counter == 0 || game.difficulty == HARD_MODE) {
			switch (local->direction[count]) {
			case NORTH:
				--local->bubble_y[count];
				break;

			case SOUTH:
				++local->bubble_y[count];
				break;

			case EAST:
				++local->bubble_x[count];
				break;

			case WEST:
				--local->bubble_x[count];
				break;

			case NORTHEAST:
				++local->bubble_x[count];
				--local->bubble_y[count];
				break;

			case SOUTHEAST:
				++local->bubble_x[count];
				++local->bubble_y[count];
				break;

			case NORTHWEST:
				--local->bubble_x[count];
				--local->bubble_y[count];
				break;

			case SOUTHWEST:
				--local->bubble_x[count];
				++local->bubble_y[count];
				break;
			}
		}

		if (local->bubble_x[count] >= MAX_RIGHT_X  ||
		    local->bubble_y[count] >= MAX_BOTTOM_Y ||
		    local->bubble_y[count] <= MAX_TOP_Y    ||
		    local->bubble_x[count] <= MAX_LEFT_X) {

			local->direction[count]    = (int16)get_direction(local->direction[count], count);
			local->move_counter[count] = 0;
		}

		if (count == 0 && local->sprite_num[count] == 1) {
			sound_play(N_RalphIsRed);
		}

		kernel_seq_delete(seq[count]);
		seq[count] = kernel_seq_stamp(ss[fx_bubble], false, local->sprite_num[count]);
		kernel_seq_depth(seq[count], 2);
		kernel_seq_loc(seq[count], local->bubble_x[count], local->bubble_y[count]);

		local->dyna[count] = kernel_add_dynamic(words_sprite, words_look_at, SYNTAX_SINGULAR,
		        KERNEL_NONE, local->bubble_x[count] - 5, local->bubble_y[count] - 8,
		                     11, 11);
		kernel_dynamic_hot[local->dyna[count]].prep = PREP_ON;
	}

	kernel_timing_trigger(5, ROOM_302_BUBBLES);
}

static void leave_screen() {
	int count;

	kernel_purge_dynamic();
	kernel_flip_hotspot(words_guardian, false);

	local->bubble_x[0] += -3;
	local->bubble_x[1] += -3;
	local->bubble_y[2] += -3;
	local->bubble_y[3] += -3;
	local->bubble_y[4] += -3;
	local->bubble_y[5] += 3;
	local->bubble_y[6] += 3;
	local->bubble_y[7] += 3;

	for (count = 0; count < 8; count++) {

		++local->sprite_num[count];
		if (local->sprite_num[count] >= 30) {
			local->sprite_num[count] = 1;
		}

		kernel_seq_delete(seq[count]);
		seq[count] = kernel_seq_stamp(ss[fx_bubble], false, local->sprite_num[count]);
		kernel_seq_depth(seq[count], 2);
		kernel_seq_loc(seq[count], local->bubble_x[count], local->bubble_y[count]);
	}

	if (local->bubble_x[0] < -5 &&
	    local->bubble_x[1] < -5 &&

	    local->bubble_y[2] < -7 &&
	    local->bubble_y[3] < -7 &&
	    local->bubble_y[4] < -7 &&

	    local->bubble_y[5] > 155 &&
	    local->bubble_y[6] > 155 &&
	    local->bubble_y[7] > 155) {

		local->leave            = false;
		local->bubbles_here     = false;
		player.commands_allowed = true;
	}
}

static void guard_leave_screen() {
	kernel_seq_delete(seq[fx_guard_1]);
	kernel_seq_delete(seq[fx_guard_2]);

	local->guard_1_x += 3;
	local->guard_1_y += 3;

	local->guard_2_x += 3;
	local->guard_2_y += -3;

	++local->guard_1_frame;
	if (local->guard_1_frame == 9) {
		local->guard_1_frame = 1;
	}

	++local->guard_2_frame;
	if (local->guard_2_frame == 6) {
		local->guard_2_frame = 1;
	}

	seq[fx_guard_1] = kernel_seq_stamp(ss[fx_guard_1], false, local->guard_1_frame);
	kernel_seq_depth(seq[fx_guard_1], 1);
	kernel_seq_loc(seq[fx_guard_1], local->guard_1_x, local->guard_1_y);

	seq[fx_guard_2] = kernel_seq_stamp(ss[fx_guard_2], false, local->guard_2_frame);
	kernel_seq_depth(seq[fx_guard_2], 1);
	kernel_seq_loc(seq[fx_guard_2], local->guard_2_x, local->guard_2_y);
}

static void handle_anim_death() {
	int death_reset_frame;

	if (kernel_anim[aa[0]].frame != local->death_frame) {
		local->death_frame = kernel_anim[aa[0]].frame;
		death_reset_frame = -1;

		switch (local->death_frame) {
		case 6:
			player.walker_visible = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
			break;

		case 22:
			seq[fx_guard_1] = kernel_seq_forward(ss[fx_guard_1], false, 7, 0, 0, 0);
			kernel_seq_depth(seq[fx_guard_1], 13);
			kernel_seq_range(seq[fx_guard_1], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_loc(seq[fx_guard_1], GUARD_1_X, GUARD_1_Y);
			kernel_seq_scale(seq[fx_guard_1], 100);
			kernel_synch(KERNEL_SERIES, seq[fx_guard_1], KERNEL_ANIM, aa[0]);
			break;

		case 27:
			if (game.difficulty == EASY_MODE) {
				text_show(30226);
			} else {
				text_show(45);
			}
			kernel.force_restart = true;
			break;
		}

		if (death_reset_frame >= 0) {
			kernel_reset_animation(aa[0], death_reset_frame);
			local->death_frame = death_reset_frame;
		}
	}
}

static void room_302_init() {
	int count;

	kernel.disable_fastwalk = true;

	if (!global[bubble_wont_attack]) {
		conv_get(CONV_56_BUBBLE);
	}

	local->run_conv = -1;
	local->xxx = 0;

	local->change_counter = 0;

	if ((game.difficulty == EASY_MODE && global[bubble_wont_attack]) ||
	    (game.difficulty == HARD_MODE && player_has_been_in_room(303))) {
		local->bubbles_here = false;
		kernel_flip_hotspot(words_guardian, false);

	} else {
		local->bubbles_here = true;
		ss[fx_bubble]       = kernel_load_series(kernel_name('x', 9), false);
		ss[fx_guard_1]      = kernel_load_series(kernel_name('x', 1), false);
		ss[fx_guard_2]      = kernel_load_series(kernel_name('x', 2), false);

		seq[fx_guard_1] = kernel_seq_forward(ss[fx_guard_1], false, 7, 0, 0, 0);
		kernel_seq_depth(seq[fx_guard_1], 13);
		kernel_seq_range(seq[fx_guard_1], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_loc(seq[fx_guard_1], GUARD_1_X, GUARD_1_Y);
		kernel_seq_scale(seq[fx_guard_1], 100);

		seq[fx_guard_2] = kernel_seq_forward(ss[fx_guard_2], false, 7, 0, 0, 0);
		kernel_seq_depth(seq[fx_guard_2], 2);
		kernel_seq_range(seq[fx_guard_2], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_loc(seq[fx_guard_2], GUARD_2_X, GUARD_2_Y);
		kernel_seq_scale(seq[fx_guard_2], 100);

		for (count = 0; count < 8; count++) {
			if (previous_room != KERNEL_RESTORING_GAME) {
				local->move_counter[count] = (int16)imath_random(1, MAX_MOVE_COUNTER);
				local->direction[count]    = (int16)imath_random(1, 8);
				local->sprite_num[count]   = (int16)imath_random(1, 30);
				local->bubble_x[count]     = (int16)imath_random(MAX_LEFT_X + 1, MAX_RIGHT_X - 1);
				local->bubble_y[count]     = (int16)imath_random(MAX_TOP_Y + 1, MAX_BOTTOM_Y - 1);
			}

			seq[count] = kernel_seq_stamp(ss[fx_bubble], false, local->sprite_num[count]);
			kernel_seq_depth(seq[count], 2);
			kernel_seq_loc(seq[count], local->bubble_x[count], local->bubble_y[count]);

			local->dyna[count] = kernel_add_dynamic(words_sprite, words_look_at, SYNTAX_SINGULAR,
			                                        seq[count], 0, 0, 0, 0);
		}
		kernel_timing_trigger(3, ROOM_302_BUBBLES);
	}

	local->prevent        = false;
	local->anim_0_running = false;
	local->leave          = false;
	local->at_end         = false;
	local->guard_1_x      = GUARD_1_X;
	local->guard_1_y      = GUARD_1_Y;
	local->guard_2_x      = GUARD_2_X;
	local->guard_2_y      = GUARD_2_Y;
	local->guard_leave    = false;
	local->guard_1_frame  = 1;
	local->guard_2_frame  = 1;


	if (conv_restore_running == CONV_56_BUBBLE) {
		conv_run(CONV_56_BUBBLE);
		conv_export_value(1);
	}


	if (previous_room == 303) {
		player_first_walk(PLAYER_X_FROM_303, PLAYER_Y_FROM_303, FACING_EAST,
		                  WALK_TO_X_FROM_303, WALK_TO_Y_FROM_303, FACING_EAST, true);

	} else if ((previous_room == 301) || (previous_room != KERNEL_RESTORING_GAME)) {
		player_first_walk(PLAYER_X_FROM_301, PLAYER_Y_FROM_301, FACING_WEST,
		                  WALK_TO_X_FROM_301, WALK_TO_Y_FROM_301, FACING_WEST, true);
	}

	section_3_music();
}

static void room_302_daemon() {
	if (local->anim_0_running) {
		handle_anim_death();
	}

	if (kernel.trigger == ROOM_302_BUBBLES && local->bubbles_here && !local->leave) {
		animate_bubbles();
	}

	if (local->leave) {
		leave_screen();
	}

	if (local->guard_leave || kernel.trigger == ROOM_302_GUARDS) {
		if (local->guard_1_y < 170 || local->guard_2_y > -14) {
			guard_leave_screen();
		}
	}

	if (player.x < 140 && !local->prevent && !player_has_been_in_room(303)) {
		if (!global[bubble_wont_attack]) {
			player_cancel_command();
			player_walk(DEATH_X, DEATH_Y, FACING_WEST);
			player_walk_trigger(1);
			local->prevent          = true;
			player.commands_allowed = false;

		} else if (local->bubbles_here) {
			player_cancel_command();
			player_walk(PLAYER_X_FROM_303, PLAYER_Y_FROM_303, FACING_WEST);
			player.walk_off_edge_to_room = 303;
			local->prevent          = true;
			player.commands_allowed = false;
		}
	}

	if (kernel.trigger == 1) {
		kernel_seq_delete(seq[fx_guard_1]);
		aa[0]                 = kernel_run_animation(kernel_name('p', 1), 0);
		local->anim_0_running = true;
	}

	if (local->at_end && conv_control.running != CONV_56_BUBBLE) {
		local->leave               = true;
		local->guard_leave         = true;
		local->at_end              = false;
		global[bubble_wont_attack] = true;
		player.commands_allowed    = false;
		sound_play(N_EveryoneScatter);
	}
}

static void process_conv_bubb() {
	if (player_verb == conv056_ralphre2_permit) {
		if (game.difficulty == EASY_MODE) {
			local->at_end         = true;
		} else {
			global[bubble_wont_attack] = true;
		}
	}
}

static void room_302_pre_parser() {
	int count;

	if (player_said_1(talk_to)) {
		player.need_to_walk = false;

		local->run_conv = -1;

		for (count = 0; count < 8; count++) {
			if (inter_point_x >= local->bubble_x[count] - 6 &&
			    inter_point_x <= local->bubble_x[count] + 6 &&
			    inter_point_y >= local->bubble_y[count] - 9 &&
			    inter_point_y <= local->bubble_y[count] + 3) {
				goto over;
			}
		}

over:
		if (count != 8) {
			global[bubble_wont_attack] = false;
			local->run_conv = count;
		}
	}

	if (player_said_1(take_magic_from)) {
		player.need_to_walk = false;
	}

	if (player_said_2(walk_down, path_to_east)) {
		player.walk_off_edge_to_room = 301;
	}

	if (player_said_2(walk_down, path_to_west)) {
		player.walk_off_edge_to_room = 303;
	}
}

static void room_302_parser() {
	if (conv_control.running == CONV_56_BUBBLE) {
		process_conv_bubb();
		goto handled;
	}

	if (local->run_conv >= 0) {
		if (local->run_conv == 8) goto fucked_up;

		conv_run(CONV_56_BUBBLE);

		if (local->sprite_num[local->run_conv] <= 10) {
			conv_export_value(3 * local->run_conv);

		} else if (local->sprite_num[local->run_conv] <= 20) {
			conv_export_value(2 + (3 * local->run_conv));

		} else {
			conv_export_value(1 + (3 * local->run_conv));
		}
		local->run_conv = -1;
		goto handled;

fucked_up:
		;
	}

	if (player.look_around) {
		if (local->bubbles_here) {
			if (game.difficulty == EASY_MODE) {
				text_show(30222);
			} else {
				text_show(30201);
			}
		} else {
			text_show(30202);
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(topiary_gargoyle)) {
			text_show(30203);
			goto handled;
		}

		if (player_said_1(topiary_dragon)) {
			text_show(30204);
			goto handled;
		}

		if (player_said_1(topiary_toadstool)) {
			if (game.difficulty == EASY_MODE) {
				text_show(30223);
			} else {
				text_show(30205);
			}
			goto handled;
		}

		if (player_said_1(path_to_east)) {
			text_show(30210);
			goto handled;
		}

		if (player_said_1(path)) {
			text_show(30211);
			goto handled;
		}

		if (player_said_1(maze)) {
			if (local->bubbles_here) {
				text_show(30212);
			} else {
				text_show(30213);
			}
			goto handled;
		}

		if (player_said_1(path_to_west)) {
			if (local->bubbles_here) {
				text_show(30208);
			} else {
				text_show(30209);
			}
			goto handled;
		}

		if (player_said_1(sprite)) {
			if (game.difficulty == EASY_MODE) {
				text_show(30224);
			} else {
				text_show(30214);
			}
			goto handled;
		}

		if (player_said_1(guardian)) {
			text_show(30217);
			goto handled;
		}
	}

	if (player_said_1(take) ||
	    player_said_1(push) ||
	    player_said_1(pull) ||
	    player_said_1(throw) ||
	    player_said_2(sword, attack) ||
	    player_said_2(sword, carve_up) ||
	    player_said_2(sword, thrust) ||
	    player_said_1(pour_contents_of)) {

		if (player_said_1(topiary_gargoyle) ||
		    player_said_1(topiary_dragon) ||
		    player_said_1(topiary_toadstool)) {

			text_show(30206);
			goto handled;
		}
	}

	if (player_said_1(talk_to)) {
		if (player_said_1(topiary_gargoyle) ||
		    player_said_1(topiary_dragon) ||
		    player_said_1(topiary_toadstool)) {
			if (game.difficulty == EASY_MODE) {
				text_show(30223);
			} else {
				text_show(30207);
			}
			goto handled;
		}
	}

	if (player_said_1(push) ||
	    player_said_1(pull) ||
	    player_said_2(sword, attack) ||
	    player_said_2(sword, carve_up) ||
	    player_said_2(sword, thrust) ||
	    player_said_1(pour_contents_of)) {

		if (player_said_1(guardian)) {
			text_show(30217);
			goto handled;
		}
	}

	if (player_said_2(sword, attack) ||
	    player_said_2(sword, carve_up) ||
	    player_said_2(sword, thrust) ||
	    player_said_1(pour_contents_of)) {

		if (player_said_1(sprite)) {
			text_show(30225);
			goto handled;
		}
	}

	if (player_said_2(open, maze)) {
		text_show(30221);
		goto handled;
	}

	if (player_said_2(talk_to, guardian)) {
		text_show(30219);
		goto handled;
	}

	if (player_said_2(take, sprite)) {
		text_show(30215);
		goto handled;
	}

	if (player_said_2(give, sprite)) {
		text_show(30216);
		goto handled;
	}

	if (player_said_2(take_magic_from, guardian) ||
	    player_said_2(take_magic_from, sprite)) {
		text_show(30220);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_302_error() {
}

void room_302_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)        s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)      s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation)     s.syncAsSint16LE(v);
	for (int16 &v : scratch.move_counter)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.direction)     s.syncAsSint16LE(v);
	for (int16 &v : scratch.sprite_num)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.bubble_x)      s.syncAsSint16LE(v);
	for (int16 &v : scratch.bubble_y)      s.syncAsSint16LE(v);
	for (int16 &v : scratch.dyna)          s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.prevent);
	s.syncAsSint16LE(scratch.death_frame);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.bubbles_here);
	s.syncAsSint16LE(scratch.leave);
	s.syncAsSint16LE(scratch.at_end);
	s.syncAsSint16LE(scratch.change_counter);
	s.syncAsSint16LE(scratch.guard_1_x);
	s.syncAsSint16LE(scratch.guard_1_y);
	s.syncAsSint16LE(scratch.guard_2_x);
	s.syncAsSint16LE(scratch.guard_2_y);
	s.syncAsSint16LE(scratch.guard_leave);
	s.syncAsSint16LE(scratch.guard_1_frame);
	s.syncAsSint16LE(scratch.guard_2_frame);
	s.syncAsSint16LE(scratch.xxx);
	s.syncAsSint16LE(scratch.run_conv);
}

void room_302_preload() {
	room_init_code_pointer       = room_302_init;
	room_pre_parser_code_pointer = room_302_pre_parser;
	room_parser_code_pointer     = room_302_parser;
	room_daemon_code_pointer     = room_302_daemon;

	section_3_walker();
	section_3_interface();

	vocab_make_active(words_sprite);
	vocab_make_active(words_look_at);

	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
