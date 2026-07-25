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

static void room_391_init() {
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);
	player.walker_visible = false;
	section_3_music();
}

static void room_391_parser() {
	if (player_said_2(return_to, air_shaft))
		new_room = 313;
	else if (player_said_2(open, grate)) {
		if (global[kKickedIn391Grate])
			text_show(39113);
		else {
			text_show(39112);
			global[kKickedIn391Grate] = true;
		}

		if (global[kAfterHavoc])
			new_room = 361;
		else
			new_room = 311;
	} else if (player_said_2(look_through, grate)) {
		if (global[kAfterHavoc])
			text_show(39111);
		else
			text_show(39110);
	} else
		return;

	player.command_ready = false;
}

void room_391_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_391_preload() {
	room_init_code_pointer = room_391_init;
	room_pre_parser_code_pointer = section_3_pre_parser;
	room_parser_code_pointer = room_391_parser;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
