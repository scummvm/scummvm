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
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section6.h"
#include "mads/madsv2/dragonsphere/rooms/room603.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];

	int32 clock;
	int32 blink_timer;
	int32 blink_max;
	int16 blinking;

	int16 zapping;
	int16 number_of_zaps;
	int16 zap_x;
	int16 zap_frame;
	int16 prevent;
	int16 distance_to_eye;
};

#define local ((Scratch *)(&game.scratch[0]))
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_fire_0               0
#define fx_fire_1               1
#define fx_fire_2               2
#define fx_fire_3               3
#define fx_fire_4               4
#define fx_fire_5               5
#define fx_fire_6               6
#define fx_throw                7
#define fx_blink                8
#define fx_discharge            9
#define fx_zap                  10
#define fx_death                11
#define fx_mud                  12

#define ROOM_603_DONE_BLINKING  65
#define ROOM_603_ZAP            70
#define ROOM_603_WALK_TO_ZAP    80

#define THROW_X                 196
#define THROW_Y                 134

#define PLAYER_X_FROM_604       260
#define PLAYER_Y_FROM_604       100
#define WALK_TO_X_FROM_604      223
#define WALK_TO_Y_FROM_604      100

#define PLAYER_X_FROM_606       37
#define PLAYER_Y_FROM_606       106
#define WALK_TO_X_FROM_606      75
#define WALK_TO_Y_FROM_606      106

#define PLAYER_X_FROM_601       176
#define PLAYER_Y_FROM_601       148

#define START_ZAP_X             218
#define START_ZAP_Y             109

#define FAR                     0
#define NEAR                    1
#define TOO_CLOSE               2

#define ZAP_X                   121
#define ZAP_Y                   128


static void zap_player() {
	int temp;

	if (player.y <= ZAP_Y && !local->zapping) {
		local->zapping          = true;
		player.commands_allowed = false;
		local->number_of_zaps = ((START_ZAP_X - player.x) - 20) / 10;

		if (player.x < ZAP_X) {
			player_walk(ZAP_X, player.y, FACING_EAST);
			player_walk_trigger(ROOM_603_WALK_TO_ZAP);
			local->distance_to_eye = FAR;

		} else {
			if ((player.x >= 193) && (player.x < 205)) {
				local->distance_to_eye = NEAR;
				local->number_of_zaps = 1;

			} else if (local->number_of_zaps <= 0) {
				local->distance_to_eye = TOO_CLOSE;

			} else {
				local->distance_to_eye = FAR;
				kernel_timing_trigger(1, ROOM_603_ZAP + 2);
			}

			player_cancel_walk();
			sound_play(N_EyeZap);
			seq[fx_discharge] = kernel_seq_forward(ss[fx_discharge], false, 4, 0, 0, 1);
			kernel_seq_depth(seq[fx_discharge], 5);
			kernel_seq_range(seq[fx_discharge], KERNEL_FIRST, KERNEL_LAST);

			if (local->distance_to_eye == TOO_CLOSE ||
			    local->distance_to_eye == NEAR) {
				kernel_seq_trigger(seq[fx_discharge], KERNEL_TRIGGER_SPRITE, 5, ROOM_603_ZAP);
				kernel_seq_trigger(seq[fx_discharge], KERNEL_TRIGGER_SPRITE, 14, ROOM_603_ZAP + 4);
			} else {
				kernel_seq_trigger(seq[fx_discharge], KERNEL_TRIGGER_SPRITE, 14, ROOM_603_ZAP + 4);
			}
		}
	}

	if (kernel.trigger == ROOM_603_WALK_TO_ZAP) {
		sound_play(N_EyeZap);
		local->number_of_zaps = ((START_ZAP_X - player.x) - 20) / 10;
		seq[fx_discharge] = kernel_seq_forward(ss[fx_discharge], false, 4, 0, 0, 1);
		kernel_seq_depth(seq[fx_discharge], 5);
		kernel_seq_range(seq[fx_discharge], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_discharge], KERNEL_TRIGGER_SPRITE, 14, ROOM_603_ZAP + 4);

		player.walker_visible = false;
		seq[fx_death] = kernel_seq_forward(ss[fx_death], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_death], 4);
		kernel_seq_scale(seq[fx_death], 73);
		kernel_seq_loc(seq[fx_death], player.x - 31, player.y + 5);
		kernel_synch(KERNEL_SERIES, seq[fx_death], KERNEL_PLAYER, 0);
		kernel_seq_range(seq[fx_death], 3, 5);
		kernel_seq_trigger(seq[fx_death], KERNEL_TRIGGER_EXPIRE, 0, ROOM_603_ZAP + 3);
	}

	if (kernel.trigger == ROOM_603_ZAP) {
		player.walker_visible = false;
		seq[fx_death] = kernel_seq_forward(ss[fx_death], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_death], 4);
		kernel_seq_scale(seq[fx_death], 73);
		kernel_seq_loc(seq[fx_death], player.x - 31, player.y + 5);
		kernel_synch(KERNEL_SERIES, seq[fx_death], KERNEL_PLAYER, 0);
		kernel_seq_range(seq[fx_death], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_death], KERNEL_TRIGGER_EXPIRE, 0, ROOM_603_ZAP + 5);
		kernel_seq_trigger(seq[fx_death], KERNEL_TRIGGER_SPRITE, 35, ROOM_603_ZAP + 8);
	}

	if (kernel.trigger == ROOM_603_ZAP + 4) {

		if (local->distance_to_eye == TOO_CLOSE) {
			if (player.x <= 224) {
				seq[fx_zap] = kernel_seq_forward(ss[fx_zap], false, 1, 0, 0, 1);
				kernel_seq_range(seq[fx_zap], 4, 5);
				kernel_seq_depth(seq[fx_zap], 4);
				kernel_seq_loc(seq[fx_zap], START_ZAP_X + 20, START_ZAP_Y);
			}

		} else {
			seq[fx_zap] = kernel_seq_stamp(ss[fx_zap], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_zap], 2);
			kernel_seq_loc(seq[fx_zap], local->zap_x, START_ZAP_Y);
			kernel_timing_trigger(1, ROOM_603_ZAP + 1);
		}
	}

	if (kernel.trigger == ROOM_603_ZAP + 1) {
		kernel_seq_delete(seq[fx_zap]);

		local->zap_x += -10;
		--local->number_of_zaps;
		++local->zap_frame;
		if (local->zap_frame == 4) {
			local->zap_frame = 1;
		}

		if (local->number_of_zaps >= 0) {
			seq[fx_zap] = kernel_seq_stamp(ss[fx_zap], false, local->zap_frame);
			kernel_seq_depth(seq[fx_zap], 2);
			kernel_seq_loc(seq[fx_zap], local->zap_x, START_ZAP_Y);
			kernel_timing_trigger(1, ROOM_603_ZAP + 1);

			if (local->number_of_zaps == 0) {
				if (local->distance_to_eye == FAR) {
					kernel_seq_delete(seq[fx_death]);
					seq[fx_death] = kernel_seq_forward(ss[fx_death], false, 6, 0, 0, 1);
					kernel_seq_depth(seq[fx_death], 4);
					kernel_seq_range(seq[fx_death], 6, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_death], KERNEL_TRIGGER_EXPIRE, 0, ROOM_603_ZAP + 5);
					kernel_seq_trigger(seq[fx_death], KERNEL_TRIGGER_SPRITE, 35, ROOM_603_ZAP + 8);
					kernel_seq_scale(seq[fx_death], 73);
					kernel_seq_loc(seq[fx_death], player.x - 31, player.y + 5);
				}
			}

		} else {
			seq[fx_zap] = kernel_seq_forward(ss[fx_zap], false, 1, 0, 0, 1);
			kernel_seq_range(seq[fx_zap], 4, 5);
			kernel_seq_depth(seq[fx_zap], 4);
			kernel_seq_loc(seq[fx_zap], local->zap_x, START_ZAP_Y);
		}
	}

	if (kernel.trigger == ROOM_603_ZAP + 8) {
		sound_play(N_PlayerDies);
	}

	if (kernel.trigger == ROOM_603_ZAP + 2) {
		player.walker_visible = false;
		seq[fx_death] = kernel_seq_forward(ss[fx_death], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_death], 4);
		kernel_seq_range(seq[fx_death], 1, 5);
		kernel_seq_trigger(seq[fx_death], KERNEL_TRIGGER_EXPIRE, 0, ROOM_603_ZAP + 3);
		kernel_seq_scale(seq[fx_death], 73);
		kernel_seq_loc(seq[fx_death], player.x - 31, player.y + 5);
		kernel_synch(KERNEL_SERIES, seq[fx_death], KERNEL_PLAYER, 0);
	}

	if (kernel.trigger == ROOM_603_ZAP + 3) {
		temp = seq[fx_death];
		seq[fx_death] = kernel_seq_stamp(ss[fx_death], false, 5);
		kernel_seq_depth(seq[fx_death], 4);
		kernel_seq_scale(seq[fx_death], 73);
		kernel_seq_loc(seq[fx_death], player.x - 31, player.y + 5);
		kernel_synch(KERNEL_SERIES, seq[fx_death], KERNEL_SERIES, temp);
	}

	if (kernel.trigger == ROOM_603_ZAP + 5) {
		temp = seq[fx_death];
		seq[fx_death] = kernel_seq_backward(ss[fx_death], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_death], 4);
		kernel_seq_range(seq[fx_death], 35, 39);
		kernel_seq_trigger(seq[fx_death], KERNEL_TRIGGER_EXPIRE, 0, ROOM_603_ZAP + 6);
		kernel_seq_scale(seq[fx_death], 73);
		kernel_seq_loc(seq[fx_death], player.x - 31, player.y + 5);
		kernel_synch(KERNEL_SERIES, seq[fx_death], KERNEL_SERIES, temp);
	}

	if (kernel.trigger == ROOM_603_ZAP + 6) {
		if (game.difficulty == EASY_MODE) {
			text_show(60330);
		} else {
			text_show(45);
		}
		kernel.force_restart = true;
	}
}

static void room_603_init() {
	local->zap_x            = START_ZAP_X;
	local->zap_frame        = 1;
	local->zapping          = false;
	local->blinking         = false;
	local->blink_timer      = 0;
	local->clock            = 0;
	local->blink_max        = imath_random(100, 1000);
	local->prevent          = false;
	kernel.disable_fastwalk = true;

	ss[fx_fire_0] = kernel_load_series(kernel_name('y', 0), false);
	ss[fx_fire_1] = kernel_load_series(kernel_name('y', 1), false);
	ss[fx_fire_2] = kernel_load_series(kernel_name('y', 2), false);
	ss[fx_fire_3] = kernel_load_series(kernel_name('y', 3), false);
	ss[fx_fire_4] = kernel_load_series(kernel_name('y', 4), false);
	ss[fx_fire_5] = kernel_load_series(kernel_name('y', 5), false);
	ss[fx_fire_6] = kernel_load_series(kernel_name('y', 6), false);

	if (player_has(mud)) {
		ss[fx_throw] = kernel_load_series("*KGTHW_8", false);
	}

	if (!global[mud_is_in_eye_603]) {
		ss[fx_blink]     = kernel_load_series(kernel_name('x', 0), false);
		ss[fx_discharge] = kernel_load_series(kernel_name('x', 1), false);
		ss[fx_zap]       = kernel_load_series(kernel_name('x', 2), false);
		ss[fx_death]     = kernel_load_series(kernel_name('a', 0), false);
	}

	ss[fx_mud] = kernel_load_series(kernel_name('x', 3), false);

	if (global[mud_is_in_eye_603]) {
		seq[fx_mud] = kernel_seq_stamp(ss[fx_mud], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_mud], 3);
	}

	seq[fx_fire_0] = kernel_seq_forward(ss[fx_fire_0], false, 7, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_0], 1);
	kernel_seq_range(seq[fx_fire_0], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_1] = kernel_seq_forward(ss[fx_fire_1], false, 7, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_1], 1);
	kernel_seq_range(seq[fx_fire_1], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_2] = kernel_seq_forward(ss[fx_fire_2], false, 7, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_2], 1);
	kernel_seq_range(seq[fx_fire_2], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_3] = kernel_seq_forward(ss[fx_fire_3], false, 7, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_3], 1);
	kernel_seq_range(seq[fx_fire_3], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_4] = kernel_seq_forward(ss[fx_fire_4], false, 7, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_4], 1);
	kernel_seq_range(seq[fx_fire_4], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_5] = kernel_seq_forward(ss[fx_fire_5], false, 7, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_5], 1);
	kernel_seq_range(seq[fx_fire_5], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_6] = kernel_seq_forward(ss[fx_fire_6], false, 7, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_6], 1);
	kernel_seq_range(seq[fx_fire_6], KERNEL_FIRST, KERNEL_LAST);

	if (previous_room == 606) {
		player_first_walk(PLAYER_X_FROM_606, PLAYER_Y_FROM_606, FACING_EAST,
		                  WALK_TO_X_FROM_606, WALK_TO_Y_FROM_606, FACING_EAST, true);

	} else if (previous_room == 604) {
		player_first_walk(PLAYER_X_FROM_604, PLAYER_Y_FROM_604, FACING_WEST,
		                  WALK_TO_X_FROM_604, WALK_TO_Y_FROM_604, FACING_WEST, true);

	} else if (previous_room == 601 || previous_room != KERNEL_RESTORING_GAME) {
		player.x      = PLAYER_X_FROM_601;
		player.y      = PLAYER_Y_FROM_601;
		player.facing = FACING_NORTH;
	}

	section_6_music();
}

static void room_603_daemon() {
	int32 dif;

	if (!global[mud_is_in_eye_603] && !local->blinking) {
		dif = kernel.clock - local->clock;
		if ((dif >= 0) && (dif <= 4)) {
			local->blink_timer += dif;
		} else {
			local->blink_timer += 1;
		}
		local->clock = kernel.clock;

		if (local->blink_timer >= local->blink_max) {
			local->blinking = true;
			seq[fx_blink]   = kernel_seq_pingpong(ss[fx_blink], false, 6, 0, 0, 2);
			kernel_seq_depth(seq[fx_blink], 5);
			kernel_seq_range(seq[fx_blink], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_blink],
			                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_603_DONE_BLINKING);
		}
	}

	if (kernel.trigger == ROOM_603_DONE_BLINKING) {
		local->blinking    = false;
		local->blink_timer = 0;
		local->blink_max   = imath_random(300, 1000);
	}

	if (!global[mud_is_in_eye_603]) {
		zap_player();
	}

	if (player.x < ZAP_X && !global[mud_is_in_eye_603]) {
		if (player.walking) {
			if (player.next_special_code > 0) {
				player_cancel_command();
				player_start_walking(ZAP_X, ZAP_Y, FACING_EAST);
				player_walk_trigger(ROOM_603_WALK_TO_ZAP);
				player.next_special_code = 0;
			}
		}
	}
}

static void room_603_pre_parser() {
	if (player_said_3(throw, mud, eye) && player_has(mud)) {
		player_walk(THROW_X, THROW_Y, FACING_EAST);
	}

	if (player_said_1(doorway_to_east) && player.need_to_walk) {
		if (!player_said_1(walk_through)) {
			player.need_to_walk = false;
		}
	}

	if (player_said_1(doorway_to_west) && player.need_to_walk) {
		if (!player_said_1(walk_through)) {
			player.need_to_walk = false;
		}
	}
}

static void room_603_parser() {
	int temp;

	if (local->zapping) {
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_through, doorway_to_east)) {
		new_room = 604;
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_through, doorway_to_west)) {
		new_room = 606;
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_through, tower_door) || player_said_2(open, tower_door) ||
	    player_said_2(pull, tower_door)) {
		new_room = 601;
		player.command_ready = false;
		return;
	}

	if (player_said_3(throw, mud, eye)) {
		switch (kernel.trigger) {
		case 0:
			global[player_score] += 2;
			player_demand_facing(FACING_NORTH);
			global[mud_is_in_eye_603] = true;
			player.walker_visible     = false;
			player.commands_allowed   = false;
			seq[fx_throw] = kernel_seq_forward(ss[fx_throw], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_throw], 1);
			kernel_seq_range(seq[fx_throw], 4, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_throw], KERNEL_TRIGGER_SPRITE, 12, 2);
			kernel_seq_trigger(seq[fx_throw], KERNEL_TRIGGER_EXPIRE, 0, 1);
			kernel_seq_scale(seq[fx_throw], 73);
			kernel_seq_loc(seq[fx_throw], player.x + 1, player.y + 6);
			kernel_synch(KERNEL_SERIES, seq[fx_throw], KERNEL_PLAYER, 0);
			break;

		case 1:
			player.walker_visible = true;
			player_demand_location(player.x + 1, player.y - 2);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_throw]);
			break;

		case 2:
			inter_move_object(mud, NOWHERE);
			seq[fx_mud] = kernel_seq_forward(ss[fx_mud], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_mud], 3);
			kernel_seq_range(seq[fx_mud], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_mud], KERNEL_TRIGGER_EXPIRE, 0, 3);
			kernel_seq_trigger(seq[fx_mud], KERNEL_TRIGGER_SPRITE, 6, 4);
			break;

		case 3:
			temp = seq[fx_mud];
			seq[fx_mud] = kernel_seq_stamp(ss[fx_mud], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_mud], 2);
			kernel_synch(KERNEL_SERIES, seq[fx_mud], KERNEL_SERIES, temp);
			player.commands_allowed = true;
			text_show(60316);
			break;

		case 4:
			sound_play(N_MudInTheEye);
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player.look_around) {
		text_show(60301);
		player.command_ready = false;
		return;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(skull)) {
			text_show(60302);
			player.command_ready = false;
			return;
		}

		if (player_said_1(doorway_to_east)) {
			if (player_has_been_in_room(604)) {
				text_show(60304);
			} else {
				text_show(60303);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(doorway_to_west)) {
			if (player_has_been_in_room(606)) {
				text_show(60306);
			} else {
				text_show(60305);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(eye)) {
			if (global[mud_is_in_eye_603]) {
				text_show(60322);
			} else {
				if (game.difficulty == HARD_MODE) {
					text_show(60308);
				} else {
					text_show(60307);
				}
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(statue)) {
			if (player.main_object_source == STROKE_INTERFACE) {
				text_show(60309);
				player.command_ready = false;
				return;
			}
		}

		if (player_said_1(pillar)) {
			text_show(60310);
			player.command_ready = false;
			return;
		}

		if (player_said_1(tower_door)) {
			text_show(60311);
			player.command_ready = false;
			return;
		}

		if (player_said_1(floor)) {
			text_show(60312);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_3(sword, attack, eye) ||
	    player_said_3(sword, carve_up, eye)) {
		if (global[mud_is_in_eye_603]) {
			text_show(60323);
		} else {
			text_show(60313);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_3(sword, thrust, eye)) {
		if (global[mud_is_in_eye_603]) {
			text_show(60323);
		} else {
			text_show(60314);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(throw, eye)) {
		if (global[mud_is_in_eye_603]) {
			text_show(60323);
		} else if (player_said_1(shieldstone)) {
			text_show(60320);
		} else {
			text_show(60315);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(put, eye)) {
		if (global[mud_is_in_eye_603]) {
			text_show(60323);
		} else {
			text_show(60321);
		}
		player.command_ready = false;
		return;
	}

	if ((player_said_1(take) || player_said_1(pull)) &&
	    (player_said_1(skull) || player_said_1(pillar))) {
		text_show(60317);
		player.command_ready = false;
		return;
	}

	if (player_said_2(push, statue)) {
		text_show(60318);
		player.command_ready = false;
		return;
	}

	if (player_said_1(eye) && (player_said_1(take) || player_said_1(pull))) {
		if (global[mud_is_in_eye_603]) {
			text_show(60323);
		} else {
			text_show(60319);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(close, eye)) {
		if (global[mud_is_in_eye_603]) {
			text_show(60327);
		} else {
			text_show(60324);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(open, eye)) {
		if (global[mud_is_in_eye_603]) {
			text_show(60323);
		} else {
			text_show(60325);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(talk_to, eye)) {
		text_show(60328);
		player.command_ready = false;
		return;
	}
}

void room_603_synchronize(Common::Serializer &s) {
	for (int i = 0; i < 15; i++) s.syncAsSint16LE(local->sprite[i]);
	for (int i = 0; i < 15; i++) s.syncAsSint16LE(local->sequence[i]);
	for (int i = 0; i < 4; i++)  s.syncAsSint16LE(local->animation[i]);
	s.syncAsSint32LE(local->clock);
	s.syncAsSint32LE(local->blink_timer);
	s.syncAsSint32LE(local->blink_max);
	s.syncAsSint16LE(local->blinking);
	s.syncAsSint16LE(local->zapping);
	s.syncAsSint16LE(local->number_of_zaps);
	s.syncAsSint16LE(local->zap_x);
	s.syncAsSint16LE(local->zap_frame);
	s.syncAsSint16LE(local->prevent);
	s.syncAsSint16LE(local->distance_to_eye);
}

void room_603_preload() {
	room_init_code_pointer = room_603_init;
	room_pre_parser_code_pointer = room_603_pre_parser;
	room_parser_code_pointer = room_603_parser;
	room_daemon_code_pointer = room_603_daemon;

	section_6_walker();
	section_6_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
