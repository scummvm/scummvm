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

static void room_405_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*ROXCL_8");

	if (_scene->_priorSceneId == 401) {
		_game._player._playerPos = Common::Point(23, 123);
		_game._player._facing = FACING_EAST;
	} else if (_scene->_priorSceneId == 406) {
		_game._player._playerPos = Common::Point(300, 128);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId == 408) {
		_game._player._playerPos = Common::Point(154, 109);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId == 413) {
		_game._player._playerPos = Common::Point(284, 109);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(23, 123);
		_game._player._facing = FACING_EAST;
	}

	if (_globals[kArmoryDoorOpen])
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
	else
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);

	if (_scene->_roomChanged) {
		_globals[kArmoryDoorOpen] = false;
		_game._objects.addToInventory(OBJ_SECURITY_CARD);
	}

	_game.loadQuoteSet(0x24F, 0);
	section_4_music();
}

static void room_405_daemon() {
	if (_game._trigger == 80) {
		_scene->_sequences.addTimer(20, 81);
		_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
		_game._player._visible = true;
	}

	if (_game._trigger == 81) {
		_game._player._stepEnabled = true;
		_vm->_dialogs->show(40525);
	}

	if (_game._trigger == 70) {
		_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
		_game._player._visible = true;
		_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		_vm->_sound->command(19);
	}

	if (_game._trigger == 71) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_globals[kArmoryDoorOpen] = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_game._player._stepEnabled = true;
	}

	if (_game._trigger == 75) {
		_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
		_game._player._visible = true;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
		_globals[kArmoryDoorOpen] = true;
		_game._player._stepEnabled = true;
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2],
			false, 1);
		_vm->_sound->command(19);
	}
}

static void room_405_pre_parser() {
	if (_action.isAction(VERB_TAKE))
		_game._player._needToWalk = false;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_WEST))
		_game._player._walkOffScreenSceneId = 401;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_EAST))
		_game._player._walkOffScreenSceneId = 406;

	if (_action.isAction(VERB_CLOSE, NOUN_WIDE_DOOR) && _globals[kArmoryDoorOpen])
		_game._player.walk(Common::Point(212, 113), FACING_NORTH);
}

static void room_405_parser() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR))
		_scene->_nextSceneId = 413;
	else if (_action.isAction(VERB_WALK_THROUGH, NOUN_WIDE_DOOR) && _globals[kArmoryDoorOpen])
		_scene->_nextSceneId = 408;
	else if (_action.isAction(VERB_WALK_THROUGH, NOUN_WIDE_DOOR) && !_globals[kArmoryDoorOpen])
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 60, _game.getQuote(0x24F));
	else if (_action.isAction(VERB_PUT, NOUN_SECURITY_CARD, NOUN_CARD_SLOT) && !_globals[kArmoryDoorOpen]) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 7, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
		Common::Point msgPos = Common::Point(_game._player._playerPos.x, _game._player._playerPos.y + 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[3], msgPos);
		_scene->_sequences.setScale(_globals._sequenceIndexes[3], 87);
	} else if ((_action.isAction(VERB_PUT, NOUN_SECURITY_CARD, NOUN_CARD_SLOT) || _action.isAction(VERB_CLOSE, NOUN_WIDE_DOOR)) && _globals[kArmoryDoorOpen]) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 7, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[3], _game._player._playerPos);
		_scene->_sequences.setScale(_globals._sequenceIndexes[3], 87);
	} else if (_action.isAction(VERB_PUT, NOUN_CARD_SLOT)) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 7, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[3], _game._player._playerPos);
		_scene->_sequences.setScale(_globals._sequenceIndexes[3], 87);
	} else if (_action.isAction(VERB_LOOK, NOUN_CANNON_BALLS))
		_vm->_dialogs->show(40510);
	else if (_action.isAction(VERB_TAKE, NOUN_CANNON_BALLS))
		_vm->_dialogs->show(40511);
	else if (_action.isAction(VERB_LOOK, NOUN_WATER_FOUNTAIN))
		_vm->_dialogs->show(40512);
	else if (_action.isAction(VERB_LOOK, NOUN_BACKBOARD) || _action.isAction(VERB_LOOK, NOUN_HOOP))
		_vm->_dialogs->show(40513);
	else if (_action.isAction(VERB_LOOK, NOUN_LIGHT))
		_vm->_dialogs->show(40514);
	else if (_action.isAction(VERB_LOOK, NOUN_CARD_SLOT))
		_vm->_dialogs->show(40515);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_EAST))
		_vm->_dialogs->show(40516);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_WEST))
		_vm->_dialogs->show(40517);
	else if (_action.isAction(VERB_LOOK, NOUN_MONITOR))
		_vm->_dialogs->show(40518);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR) || _action._lookFlag)
		_vm->_dialogs->show(40519);
	else if (_action.isAction(VERB_LOOK, NOUN_WIDE_DOOR)) {
		if (_globals[kArmoryDoorOpen])
			_vm->_dialogs->show(40521);
		else
			_vm->_dialogs->show(40520);
	} else if (_action.isAction(VERB_LOOK, NOUN_DOOR))
		_vm->_dialogs->show(40522);
	else if (_action.isAction(VERB_LOOK, NOUN_COACH_LAMP))
		_vm->_dialogs->show(40523);
	else if (_action.isAction(VERB_LOOK, NOUN_SUPPORT))
		_vm->_dialogs->show(40524);
	else
		return;

	_action._inProgress = false;
}

void room_405_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_405_preload() {
	room_init_code_pointer = room_405_init;
	room_pre_parser_code_pointer = room_405_pre_parser;
	room_parser_code_pointer = room_405_parser;
	room_daemon_code_pointer = room_405_daemon;

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
