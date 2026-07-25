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
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {

};

static Scratch local;


static void room_353_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(kernel_full_name(303, 'B', 0, "", EXT_SS));
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 0, 5, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

	if (_scene->_priorSceneId == 352) {
		player.x = 144;
		player.y = 95;
	}
	else {
		player.x = 139;
		player.y = 155;
	}

	section_3_music();
}

static void room_353_parser() {
	if (player.look_around)
		text_show(35315);
	else if (player_said_2(walk_through, doorway))
		_scene->_nextSceneId = 352;
	else if (player_said_2(walk_down, corridor_to_south))
		_scene->_nextSceneId = 354;
	else if (player_said_2(look, rock_chunk))
		text_show(35310);
	else if (player_said_2(look, pipes) || player_said_2(look, pipe))
		text_show(35311);
	else if (player_said_2(look, broken_beam))
		text_show(35312);
	else if (player_said_2(look, doorway))
		text_show(35313);
	else if (player_said_2(look, corridor_to_south))
		text_show(35314);
	else if (player_said_2(look, floor))
		text_show(35316);
	else if (player_said_2(look, ceiling))
		text_show(35317);
	else if (player_said_2(look, wall))
		text_show(35318);
	else
		return;

	player.command_ready = false;
}

void room_353_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_353_preload() {
	room_init_code_pointer = room_353_init;
	room_parser_code_pointer = room_353_parser;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
