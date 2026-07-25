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

#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"
#include "mads/nebular/rooms/forcefield.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	Forcefield _forcefield;
};

static Scratch local;


static void room_310_init() {
	g_sprite_ids[1] = kernel_load_series("*SC003x0", 0);
	g_sprite_ids[0] = kernel_load_series("*SC003x1", 0);
	g_sprite_ids[2] = kernel_load_series("*SC003x2", 0);

	init_forcefield(&local._forcefield, true);

	g_sprite_ids[3] = kernel_load_series(kernel_full_name(307, 'X', 0, "", KERNEL_SS), 0);
	g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
	kernel_seq_loc(g_sequence_ids[3], 127, 78);
	kernel_seq_depth(g_sequence_ids[3], 15);

	player.walker_visible = false;
	player.commands_allowed = false;
	kernel_run_animation(kernel_name('a', -1), 70);

	section_3_music();
}

static void room_310_daemon() {
	handle_forcefield(&local._forcefield, &g_sprite_ids[0]);

	if (kernel.trigger == 70)
		new_room = 309;
}

void room_310_synchronize(Common::Serializer &s) {
	local._forcefield.synchronize(s);
}

void room_310_preload() {
	local._forcefield.init();

	room_init_code_pointer = room_310_init;
	room_daemon_code_pointer = room_310_daemon;
	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
