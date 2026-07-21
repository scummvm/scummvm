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

static void room_354_init() {
	_globals[kAfterHavoc] = true;
	_globals[kTeleporterRoom + 1] = 351;

	if (_scene->_priorSceneId == 361)
		_game._player._playerPos = Common::Point(231, 110);
	else if (_scene->_priorSceneId == 401) {
		_game._player._playerPos = Common::Point(106, 152);
		_game._player._facing = FACING_NORTH;
	} else if (_scene->_priorSceneId == 316)
		_game._player._playerPos = Common::Point(71, 107);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(167, 57);

	section_3_music();
}

static void room_354_pre_parser() {
	if (player_said_2(walk_down, corridor_to_south))
		_game._player._walkOffScreenSceneId = 401;
}

static void room_354_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(35414);
	else if (player_said_2(walk_down, corridor_to_north)) {
		_game._player.startWalking(Common::Point(208, 0), FACING_NORTHEAST);
		_game._player._walkOffScreenSceneId = 353;
	} else if (player_said_2(walk_down, corridor_to_east))
		_scene->_nextSceneId = 361;
	else if (player_said_2(walk_down, corridor_to_west))
		_scene->_nextSceneId = 316;
	else if (player_said_2(walk_down, corridor_to_south))
		_scene->_nextSceneId = 401;
	else if (player_said_2(look, controls))
		_vm->_dialogs->show(35410);
	else if (player_said_2(look, signal))
		_vm->_dialogs->show(35411);
	else if (player_said_2(look, catwalk))
		_vm->_dialogs->show(35412);
	else if (player_said_2(look, air_duct))
		_vm->_dialogs->show(35413);
	else if (player_said_2(look, corridor_to_north))
		_vm->_dialogs->show(35415);
	else if (player_said_2(look, corridor_to_south))
		_vm->_dialogs->show(35416);
	else if (player_said_2(look, corridor_to_east))
		_vm->_dialogs->show(35417);
	else if (player_said_2(look, corridor_to_west))
		_vm->_dialogs->show(35418);
	else if (player_said_2(look, debris))
		_vm->_dialogs->show(35419);
	else if (player_said_2(look, guard))
		_vm->_dialogs->show(35420);
	else
		return;

	_action._inProgress = false;
}

void room_354_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_354_preload() {
	room_init_code_pointer = room_354_init;
	room_pre_parser_code_pointer = room_354_pre_parser;
	room_parser_code_pointer = room_354_parser;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
