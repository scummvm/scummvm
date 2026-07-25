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

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_301_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('x', 0), 0);
	g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 9, 0, 0, 0);

	global[kMeteorologistStatus] = METEOROLOGIST_GONE;
	global[kTeleporterCommand] = TELEPORTER_NONE;

	player.commands_allowed = false;
	player.walker_visible = false;
	kernel_run_animation(kernel_name('a', -1), 60);

	section_3_music();
}

static void room_301_daemon() {
	if (kernel.trigger == 60)
		new_room = 302;
}

void room_301_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_301_preload() {
	room_init_code_pointer = room_301_init;
	room_daemon_code_pointer = room_301_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
