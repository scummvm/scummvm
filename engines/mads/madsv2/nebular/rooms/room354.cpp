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
	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_SOUTH))
		_game._player._walkOffScreenSceneId = 401;
}

static void room_354_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(35414);
	else if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_NORTH)) {
		_game._player.startWalking(Common::Point(208, 0), FACING_NORTHEAST);
		_game._player._walkOffScreenSceneId = 353;
	} else if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_EAST))
		_scene->_nextSceneId = 361;
	else if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_WEST))
		_scene->_nextSceneId = 316;
	else if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_SOUTH))
		_scene->_nextSceneId = 401;
	else if (_action.isAction(VERB_LOOK, NOUN_CONTROLS))
		_vm->_dialogs->show(35410);
	else if (_action.isAction(VERB_LOOK, NOUN_SIGNAL))
		_vm->_dialogs->show(35411);
	else if (_action.isAction(VERB_LOOK, NOUN_CATWALK))
		_vm->_dialogs->show(35412);
	else if (_action.isAction(VERB_LOOK, NOUN_AIR_DUCT))
		_vm->_dialogs->show(35413);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_NORTH))
		_vm->_dialogs->show(35415);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_SOUTH))
		_vm->_dialogs->show(35416);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_EAST))
		_vm->_dialogs->show(35417);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_WEST))
		_vm->_dialogs->show(35418);
	else if (_action.isAction(VERB_LOOK, NOUN_DEBRIS))
		_vm->_dialogs->show(35419);
	else if (_action.isAction(VERB_LOOK, NOUN_GUARD))
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
} // namespace MADSV2
} // namespace MADS
