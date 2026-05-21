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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section4.h"
#include "mads/madsv2/dragonsphere/rooms/room412.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_test         0
#define fx_test_2       1

static Scratch scratch;


static void room_412_init() {
	global[perform_displacements] = true;

	viewing_at_y = ((video_y - display_y) >> 1);
	kernel_init_dialog();
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

	player.commands_allowed = false;
	player.walker_visible   = false;
	aa[0] = kernel_run_animation(kernel_name('w', 1), 1);
}

static void room_412_daemon() {
	if (kernel.trigger == 1) {
		text_show(41201);
		global[from_direction] = FROM_EAST;
		global[desert_room]    = 42;
		global[desert_counter] = 0;
		new_room               = 401;
		global[no_load_walker] = false;
	}
}

static void room_412_pre_parser() {
}

static void room_412_parser() {
}

void room_412_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
}

void room_412_preload() {
	room_init_code_pointer       = room_412_init;
	room_pre_parser_code_pointer = room_412_pre_parser;
	room_parser_code_pointer     = room_412_parser;
	room_daemon_code_pointer     = room_412_daemon;

	global[no_load_walker] = true;

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
