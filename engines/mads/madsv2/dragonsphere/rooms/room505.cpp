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
#include "mads/madsv2/dragonsphere/rooms/section5.h"
#include "mads/madsv2/dragonsphere/rooms/room505.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define ROOM_505_DONE_ANIM       60

#define PLAYER_X_FROM_503        330
#define PLAYER_Y_FROM_503        148
#define WALK_TO_X_FROM_503       306
#define WALK_TO_Y_FROM_503       143
#define PLAYER_X_FROM_505        165
#define PLAYER_Y_FROM_505        144


static void room_505_init() {
	if (previous_room == 511 || previous_room == 512) {
		if (player_has(magic_belt)) {
			player.x                = PLAYER_X_FROM_505;
			player.y                = PLAYER_Y_FROM_505;
			player.facing           = FACING_SOUTH;
			aa[0]                   = kernel_run_animation(kernel_name('p', 2), ROOM_505_DONE_ANIM + 1);
			player.commands_allowed = false;
			player.walker_visible   = false;

		} else {
			viewing_at_y = ((video_y - display_y) >> 1);
			kernel_init_dialog();
			kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

			sound_play(N_PlayerDies);

			aa[0]                   = kernel_run_animation(kernel_name('p', 1), ROOM_505_DONE_ANIM);
			player.commands_allowed = false;
			player.walker_visible   = false;
		}

	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player_first_walk(PLAYER_X_FROM_503, PLAYER_Y_FROM_503, FACING_NORTHWEST,
		                  WALK_TO_X_FROM_503, WALK_TO_Y_FROM_503, FACING_NORTHWEST, true);
	}

	section_5_music();
}

static void room_505_daemon() {
	if (kernel.trigger == ROOM_505_DONE_ANIM) {
		if (game.difficulty == EASY_MODE) {
			text_show(51011);
		} else {
			text_show(45);
		}
		if (global[pid_just_died]) {
			global[pid_just_died] = false;
			new_room = 512;
		} else {
			new_room = 510;
		}
	}

	if (kernel.trigger == ROOM_505_DONE_ANIM + 1) {
		player.commands_allowed = true;
		player.walker_visible   = true;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
	}
}

static void room_505_pre_parser() {
	if (player_said_2(walk_down, path_to_east)) {
		player.walk_off_edge_to_room = 503;
	}
}

static void room_505_parser() {
	if (player.look_around) {
		text_show(1);
		player.command_ready = false;
		return;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(rock_tree)) {
			text_show(2);
			player.command_ready = false;
			return;
		}

		if (player_said_1(ground)) {
			text_show(3);
			player.command_ready = false;
			return;
		}

		if (player_said_1(mountainside)) {
			text_show(4);
			player.command_ready = false;
			return;
		}

		if (player_said_1(rock_tumble)) {
			text_show(5);
			player.command_ready = false;
			return;
		}

		if (player_said_1(path_to_east)) {
			text_show(6);
			player.command_ready = false;
			return;
		}
	}
}

void room_505_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
}

void room_505_error() {
}

void room_505_preload() {
	room_init_code_pointer       = room_505_init;
	room_pre_parser_code_pointer = room_505_pre_parser;
	room_parser_code_pointer     = room_505_parser;
	room_daemon_code_pointer     = room_505_daemon;

	section_5_walker();
	section_5_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
