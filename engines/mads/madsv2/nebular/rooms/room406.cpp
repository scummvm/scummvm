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
	bool _hitStorageDoor;
};

static Scratch local;


static void room_406_init() {
	_game._player._visible = true;
	if (_scene->_priorSceneId == 405) {
		_game._player._playerPos = Common::Point(15, 129);
		_game._player._facing = FACING_EAST;
	} else if (_scene->_priorSceneId == 407) {
		_game._player._playerPos = Common::Point(270, 127);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId == 410) {
		_game._player._playerPos = Common::Point(30, 108);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId == 411) {
		_game._player._playerPos = Common::Point(153, 108);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(15, 129);
		_game._player._facing = FACING_EAST;
	}

	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*ROXCL_8");
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 1));

	if (_scene->_roomChanged) {
		_globals[kStorageDoorOpen] = false;
		_game._objects.addToInventory(OBJ_SECURITY_CARD);
	}

	if (!_globals[kStorageDoorOpen])
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);

	if (_scene->_priorSceneId != 411)
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	else {
		_game._player._stepEnabled = false;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 3, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
		_vm->_sound->command(19);
	}

	_game.loadQuoteSet(0x24F, 0);
	local._hitStorageDoor = false;
	section_4_music();
}

static void room_406_daemon() {
	if (_game._trigger == 90) {
		_game._player._stepEnabled = true;
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	}

	if (_game._trigger == 80)
		_scene->_nextSceneId = 411;

	if (_game._trigger == 100) {
		_vm->_dialogs->show(40622);
		local._hitStorageDoor = true;
	}

	if (_game._trigger == 110) {
		_scene->_sequences.addTimer(20, 111);
		_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
		_game._player._visible = true;
	}

	if (_game._trigger == 111) {
		_game._player._stepEnabled = true;
		_vm->_dialogs->show(40613);
	}

	if (_game._trigger == 70) {
		_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
		_game._player._visible = true;
		_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 4, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		_vm->_sound->command(19);
	}

	if (_game._trigger == 71) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_globals[kStorageDoorOpen] = false;
		_game._player._stepEnabled = true;
	}

	if (_game._trigger == 75) {
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 4, 1, 0, 0);
		_globals[kStorageDoorOpen] = true;
		_game._player._stepEnabled = true;
		_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
		_game._player._visible = true;
		_vm->_sound->command(19);
	}
}

static void room_406_pre_parser() {
	if (player_said_2(walk_down, corridor_to_west))
		_game._player._walkOffScreenSceneId = 405;

	if (player_said_2(walk_down, corridor_to_east))
		_game._player._walkOffScreenSceneId = 407;

	if (player_said_1(take))
		_game._player._needToWalk = false;

	if (player_said_2(look, sign) || player_said_2(look, trash))
		_game._player._needToWalk = true;
}

static void room_406_parser() {
	if (player_said_2(walk_through, door) && (_game._player._targetPos.x > 100)) {
		_game._player._stepEnabled = false;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 3, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
		_vm->_sound->command(19);
	} else if (player_said_2(walk_through, door) && _globals[kStorageDoorOpen] && (_game._player._targetPos.x < 100))
		_scene->_nextSceneId = 410;
	else if (player_said_2(walk_through, door) && !_globals[kStorageDoorOpen] && (_game._player._targetPos.x < 100)) {
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 60, _game.getQuote(0x24F));
		if (!local._hitStorageDoor) {
			_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
			_scene->_sequences.addTimer(80, 100);
		}
	} else if (player_said_3(put, security_card, card_slot) && !_globals[kStorageDoorOpen]) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 7, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
		Common::Point msgPos = Common::Point(_game._player._playerPos.x, _game._player._playerPos.y + 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], msgPos);
		_scene->_sequences.setScale(_globals._sequenceIndexes[2], 87);
	} else if (player_said_3(put, security_card, card_slot) && _globals[kStorageDoorOpen]) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 7, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		Common::Point msgPos = Common::Point(_game._player._playerPos.x, _game._player._playerPos.y + 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], msgPos);
		_scene->_sequences.setScale(_globals._sequenceIndexes[2], 87);
	} else if (player_said_2(put, card_slot)) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 7, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 110);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], _game._player._playerPos);
		_scene->_sequences.setScale(_globals._sequenceIndexes[2], 87);
	} else if (player_said_2(look, trash))
		_vm->_dialogs->show(40610);
	else if (player_said_2(take, trash))
		_vm->_dialogs->show(40611);
	else if (player_said_2(look, card_slot))
		_vm->_dialogs->show(40612);
	else if (player_said_2(look, fire_extinguisher))
		_vm->_dialogs->show(40614);
	else if (player_said_2(take, fire_extinguisher))
		_vm->_dialogs->show(40615);
	else if (player_said_2(look, corridor_to_east))
		_vm->_dialogs->show(40616);
	else if (player_said_2(look, corridor_to_west))
		_vm->_dialogs->show(40617);
	else if (player_said_2(look, corridor) || _action._lookFlag)
		_vm->_dialogs->show(40618);
	else if (player_said_2(look, wall))
		_vm->_dialogs->show(40619);
	else if (player_said_2(look, door)) {
		if (_globals[kStorageDoorOpen])
			_vm->_dialogs->show(40621);
		else
			_vm->_dialogs->show(40620);
	} else if (player_said_2(look, monitor))
		_vm->_dialogs->show(40623);
	else if (player_said_2(look, signpost))
		_vm->_dialogs->show(40624);
	else if (player_said_2(take, signpost))
		_vm->_dialogs->show(40625);
	else if (player_said_2(look, boulder))
		_vm->_dialogs->show(40626);
	else if (player_said_2(take, boulder))
		_vm->_dialogs->show(40627);
	else if (player_said_2(look, sign))
		_vm->_dialogs->show(40628);
	else if (player_said_2(take, sign))
		_vm->_dialogs->show(40629);
	else
		return;

	_action._inProgress = false;
}

void room_406_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._hitStorageDoor);
}

void room_406_preload() {
	room_init_code_pointer = room_406_init;
	room_pre_parser_code_pointer = room_406_pre_parser;
	room_parser_code_pointer = room_406_parser;
	room_daemon_code_pointer = room_406_daemon;

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
