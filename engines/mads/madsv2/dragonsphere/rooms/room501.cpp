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
#include "mads/madsv2/dragonsphere/rooms/room501.h"

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

#define ROOM_501_BYE         60
#define MUSIC                70

#define PLAYER_X_FROM_120    325
#define PLAYER_Y_FROM_120    143
#define WALK_TO_X_FROM_120   284
#define WALK_TO_Y_FROM_120   134
#define PLAYER_X_FROM_502    158
#define PLAYER_Y_FROM_502    102


static void room_501_init() {
	viewing_at_y = ((video_y - display_y) >> 1);
	kernel_init_dialog();
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

	if (previous_room == 502) {
		player_first_walk(PLAYER_X_FROM_502, PLAYER_Y_FROM_502, FACING_SOUTHEAST,
		                  PLAYER_X_FROM_120, PLAYER_Y_FROM_120, FACING_WEST, false);
		player_walk_trigger(ROOM_501_BYE + 2);

	} else if ((previous_room == 120) || (previous_room != KERNEL_RESTORING_GAME)) {
		player_first_walk(PLAYER_X_FROM_120, PLAYER_Y_FROM_120, FACING_WEST,
		                  PLAYER_X_FROM_502, PLAYER_Y_FROM_502, FACING_NORTHWEST, false);
		player_walk_trigger(ROOM_501_BYE);
	}

	section_5_music();
}

static void room_501_daemon() {
	if (kernel.trigger == ROOM_501_BYE) {
		if (global[player_persona] == PLAYER_IS_KING) {
			player.walker_visible = false;
			aa[0] = kernel_run_animation(kernel_name('x', 1), ROOM_501_BYE + 1);
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);

		} else {
			new_room = 502;
		}
	}

	if (kernel.trigger == ROOM_501_BYE + 1) {
		new_room = 502;
	}

	if (kernel.trigger == ROOM_501_BYE + 2) {
		global[pre_room] = 501;
		if (global[dragon_my_scene] < global[dragon_high_scene]) {
			global[dragon_my_scene]++;
			new_room = 111;
		} else {
			new_room = 120;
		}
	}
}

static void room_501_pre_parser() {
}

static void room_501_parser() {
}

void room_501_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
}

void room_501_preload() {
	room_init_code_pointer       = room_501_init;
	room_pre_parser_code_pointer = room_501_pre_parser;
	room_parser_code_pointer     = room_501_parser;
	room_daemon_code_pointer     = room_501_daemon;

	section_5_walker();
	section_5_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
