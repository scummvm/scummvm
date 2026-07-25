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

#include "mads/core/config.h"
#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_399_init() {
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);
	player.walker_visible = false;
	section_3_music();
}

static void room_399_parser() {
	if (player_said_2(return_to, air_shaft))
		new_room = 313;
	else if (player_said_2(look_through, grate)) {
		if (global[kAfterHavoc]) {
			if ((game.difficulty != DIFFICULTY_HARD) && (object[OBJ_SECURITY_CARD].location == 359))
				text_show(38911);
			else
				text_show(38912);
		} else
			text_show(38910);
	} else if (player_said_2(open, grate)) {
		if (global[kAfterHavoc])
			text_show(38914);
		else
			text_show(38913);
	} else
		return;

	player.command_ready = false;
}

void room_399_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_399_preload() {
	room_init_code_pointer = room_399_init;
	room_pre_parser_code_pointer = section_3_pre_parser;
	room_parser_code_pointer = room_399_parser;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
