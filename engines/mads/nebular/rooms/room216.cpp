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
#include "mads/nebular/rooms/section2.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

void room_216_init() {
	player.commands_allowed = false;
	player.walker_visible = false;

	inter_reset_dialog();
	kernel_set_interface_mode(INTER_CONVERSATION);
	kernel_run_animation(kernel_name('A', -1), 60);

	section_2_music();
}

void room_216_daemon() {
	if (kernel.trigger == 60)
		new_room = 215;
}

void room_216_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_216_preload() {
	room_init_code_pointer = room_216_init;
	room_daemon_code_pointer = room_216_daemon;

	section_2_walker();
	section_2_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
