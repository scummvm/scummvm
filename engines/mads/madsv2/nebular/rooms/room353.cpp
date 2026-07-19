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

struct Scratch {

};

static Scratch local;


static void room_353_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(Resources::formatName(303, 'B', 0, EXT_SS, ""));
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 0, 5, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

	if (_scene->_priorSceneId == 352)
		_game._player._playerPos = Common::Point(144, 95);
	else
		_game._player._playerPos = Common::Point(139, 155);

	section_3_music();
}

static void room_353_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(35315);
	else if (_action.isAction(words_walk_through, words_doorway))
		_scene->_nextSceneId = 352;
	else if (_action.isAction(words_walk_down, words_corridor_to_south))
		_scene->_nextSceneId = 354;
	else if (_action.isAction(words_look, words_rock_chunk))
		_vm->_dialogs->show(35310);
	else if (_action.isAction(words_look, words_pipes) || _action.isAction(words_look, words_pipe))
		_vm->_dialogs->show(35311);
	else if (_action.isAction(words_look, words_broken_beam))
		_vm->_dialogs->show(35312);
	else if (_action.isAction(words_look, words_doorway))
		_vm->_dialogs->show(35313);
	else if (_action.isAction(words_look, words_corridor_to_south))
		_vm->_dialogs->show(35314);
	else if (_action.isAction(words_look, words_floor))
		_vm->_dialogs->show(35316);
	else if (_action.isAction(words_look, words_ceiling))
		_vm->_dialogs->show(35317);
	else if (_action.isAction(words_look, words_wall))
		_vm->_dialogs->show(35318);
	else
		return;

	_action._inProgress = false;
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
} // namespace MADSV2
} // namespace MADS
