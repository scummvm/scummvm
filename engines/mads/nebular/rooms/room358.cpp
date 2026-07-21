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

static void room_358_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));
	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

	if (_scene->_priorSceneId == 357)
		_game._player._playerPos = Common::Point(305, 142);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(12, 141);

	section_3_music();
}

static void room_358_pre_parser() {
	if (player_said_2(walk_down, corridor_to_east))
		_game._player._walkOffScreenSceneId = 357;

	if (player_said_2(walk_down, corridor_to_west))
		_game._player._walkOffScreenSceneId = 359;
}

static void room_358_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(35815);
	else if (player_said_2(look, corridor_to_west))
		_vm->_dialogs->show(35810);
	else if (player_said_2(look, corridor_to_east))
		_vm->_dialogs->show(35811);
	else if (player_said_2(look, bed))
		_vm->_dialogs->show(35812);
	else if (player_said_2(look, sink))
		_vm->_dialogs->show(35813);
	else if (player_said_2(look, toilet))
		_vm->_dialogs->show(35814);
	else if (player_said_2(look, corridor) || player_said_2(look, wall))
		_vm->_dialogs->show(35816);
	else if (player_said_2(look, air_vent))
		_vm->_dialogs->show(35817);
	else
		return;

	_action._inProgress = false;
}

void room_358_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_358_preload() {
	room_init_code_pointer = room_358_init;
	room_pre_parser_code_pointer = room_358_pre_parser;
	room_parser_code_pointer = room_358_parser;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
