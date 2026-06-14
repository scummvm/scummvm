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


void Scene357::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

static void room_357_init() {
	_globals[kAfterHavoc] = true;
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));
	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

	if (_scene->_priorSceneId == 318)
		_game._player._playerPos = Common::Point(298, 142);
	else if (_scene->_priorSceneId == 313)
		_game._player._playerPos = Common::Point(127, 101);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(15, 148);

	section_3_music();
}

static void room_357_pre_parser() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_EAST))
		_game._player._walkOffScreenSceneId = 318;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_WEST))
		_game._player._walkOffScreenSceneId = 358;
}

static void room_357_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(35715);
	else if (_action.isAction(VERB_LOOK, NOUN_AIR_VENT))
		_vm->_dialogs->show(35710);
	else if (_action.isAction(VERB_CLIMB_INTO, NOUN_AIR_VENT))
		_vm->_dialogs->show(35711);
	else if (_action.isAction(VERB_LOOK, NOUN_BED))
		_vm->_dialogs->show(35712);
	else if (_action.isAction(VERB_LOOK, NOUN_SINK))
		_vm->_dialogs->show(35713);
	else if (_action.isAction(VERB_LOOK, NOUN_TOILET))
		_vm->_dialogs->show(35714);
	else if (_action.isAction(VERB_LOOK, NOUN_CELL_WALL))
		_vm->_dialogs->show(35716);
	else if (_action.isAction(VERB_LOOK, NOUN_LIGHT))
		_vm->_dialogs->show(35717);
	else if (_action.isAction(VERB_LOOK, NOUN_RIP_IN_FLOOR))
		_vm->_dialogs->show(35718);
	else if (_action.isAction(VERB_LOOK, NOUN_DEBRIS))
		_vm->_dialogs->show(35719);
	else if (_action.isAction(VERB_TAKE, NOUN_DEBRIS))
		_vm->_dialogs->show(35720);
	else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(35721);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_EAST))
		_vm->_dialogs->show(35722);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_WEST))
		_vm->_dialogs->show(35723);
	else
		return;

	_action._inProgress = false;
}

void room_357_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_357_preload() {
	room_init_code_pointer = room_357_init;
	room_pre_parser_code_pointer = room_357_pre_parser;
	room_parser_code_pointer = room_357_parser;
	room_daemon_code_pointer = room_357_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
