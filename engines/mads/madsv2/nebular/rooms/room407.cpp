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
#include "mads/madsv2/nebular/rooms/section4.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _fromNorth;
	int16 _dest_x;
	int16 _dest_y;
};

static Scratch local;


static void room_407_init() {
	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		local._fromNorth = false;

	if (_scene->_priorSceneId == 318) {
		_game._player._playerPos = Common::Point(172, 92);
		_game._player._facing = FACING_SOUTH;
		local._fromNorth = true;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(172, 132);
		_game._player._facing = FACING_NORTH;
	}

	_game.loadQuoteSet(0x250, 0);
	section_4_music();
}

static void room_407_daemon() {
	if (_game._trigger == 70) {
		_scene->_nextSceneId = 318;
		_scene->_reloadSceneFlag = true;
	}

	if (_game._trigger == 80) {
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		local._fromNorth = false;
		_game._player.walk(Common::Point(173, 104), FACING_SOUTH);
	}
}

static void room_407_pre_parser() {
	if (_action.isAction(VERB_TAKE))
		_game._player._needToWalk = false;

	if (_action.isAction(VERB_LOOK, NOUN_DOOR))
		_game._player._needToWalk = true;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_NORTH)) {
		_game._player.walk(Common::Point(172, 91), FACING_NORTH);
		local._fromNorth = false;
	}

	if (_game._player._needToWalk && local._fromNorth) {
		if (_globals[kSexOfRex] == REX_MALE) {
			local._dest_x = 171;
			local._dest_y = 95;
		} else {
			local._dest_x = 173;
			local._dest_y = 96;
		}
		_game._player.walk(Common::Point(local._dest_x, local._dest_y), FACING_SOUTH);
	}
}

static void room_407_parser() {
	if (_game._player._playerPos.x == local._dest_x && _game._player._playerPos.y == local._dest_y && local._fromNorth) {
		if (_globals[kSexOfRex] == REX_MALE) {
			_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_vm->_sound->command(21);
			_scene->loadAnimation(formAnimName('s', 1), 70);
			_globals[kHasBeenScanned] = true;
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 60, _game.getQuote(60));
			_vm->_sound->command(22);
		}

		if (_globals[kSexOfRex] == REX_FEMALE) {
			_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_vm->_sound->command(21);
			_scene->loadAnimation(formAnimName('s', 2), 80);
			_vm->_sound->command(23);
			_globals[kHasBeenScanned] = true;
		}
	}

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_SOUTH) && !local._fromNorth)
		_scene->_nextSceneId = 406;
	else if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_NORTH))
		_scene->_nextSceneId = 318;
	else if (_action.isAction(VERB_LOOK, NOUN_SCANNER)) {
		if (_globals[kHasBeenScanned])
			_vm->_dialogs->show(40711);
		else
			_vm->_dialogs->show(40710);
	} else if (_action.isAction(VERB_LOOK, NOUN_DOOR))
		_vm->_dialogs->show(40712);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_SOUTH))
		_vm->_dialogs->show(40713);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_NORTH))
		_vm->_dialogs->show(40714);
	else if (_action._lookFlag)
		_vm->_dialogs->show(40715);
	else
		return;

	_action._inProgress = false;
}

void room_407_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._fromNorth);
	s.syncAsSint16LE(local._dest_x);
	s.syncAsSint16LE(local._dest_y);
}

void room_407_preload() {
	room_init_code_pointer = room_407_init;
	room_pre_parser_code_pointer = room_407_pre_parser;
	room_parser_code_pointer = room_407_parser;
	room_daemon_code_pointer = room_407_daemon;

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
