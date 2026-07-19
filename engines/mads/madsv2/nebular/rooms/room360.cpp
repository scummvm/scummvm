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
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/mads/inventory.h"
#include "mads/madsv2/nebular/mads/words.h"
#include "mads/madsv2/nebular/rooms/section3.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_360_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));
	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

	if (_scene->_priorSceneId == 359)
		_game._player._playerPos = Common::Point(304, 143);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(13, 141);

	section_3_music();
}

static void room_360_pre_parser() {
	if (player_said_2(walk_down, corridor_to_east))
		_game._player._walkOffScreenSceneId = 359;

	if (player_said_2(walk_down, corridor_to_west))
		_game._player._walkOffScreenSceneId = 361;
}

static void room_360_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(36015);
	else if (player_said_2(look, corridor_to_west))
		_vm->_dialogs->show(36010);
	else if (player_said_2(look, corridor_to_east))
		_vm->_dialogs->show(36011);
	else if (player_said_2(look, bed))
		_vm->_dialogs->show(36012);
	else if (player_said_2(look, sink))
		_vm->_dialogs->show(36013);
	else if (player_said_2(look, toilet))
		_vm->_dialogs->show(36014);
	else if (player_said_2(look, air_vent))
		_vm->_dialogs->show(36016);
	else if (player_said_2(look, corridor))
		_vm->_dialogs->show(36017);
	else if (player_said_2(look, wall))
		_vm->_dialogs->show(36018);
	else
		return;

	_action._inProgress = false;
}

void room_360_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_360_preload() {
	room_init_code_pointer = room_360_init;
	room_pre_parser_code_pointer = room_360_pre_parser;
	room_parser_code_pointer = room_360_parser;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
