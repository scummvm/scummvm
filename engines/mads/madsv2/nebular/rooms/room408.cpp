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

static void room_408_init() {
	_game._player._playerPos = Common::Point(137, 150);
	_game._player._facing = FACING_NORTH;

	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*ROXRC_7");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('m', -1));

	if (_game._objects.isInRoom(OBJ_TARGET_MODULE)) {
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 3);
		int idx = _scene->_dynamicHotspots.add(NOUN_TARGET_MODULE, VERB_WALKTO, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(283, 128), FACING_NORTHEAST);
	}
	section_4_music();
}

static void room_408_pre_parser() {
	if ((_action.isAction(VERB_TAKE) && !_action.isObject(NOUN_TARGET_MODULE)) || _action.isAction(VERB_PULL, NOUN_PIN) || _action.isAction(VERB_OPEN, NOUN_CARTON))
		_game._player._needToWalk = false;

	if ((_action.isAction(VERB_LOOK, NOUN_TARGET_MODULE) && _game._objects.isInRoom(OBJ_TARGET_MODULE)) || _action.isAction(VERB_LOOK, NOUN_CHEST))
		_game._player._needToWalk = true;
}

static void room_408_parser() {
	if (_action.isAction(VERB_WALK_INTO, NOUN_CORRIDOR_TO_SOUTH)) {
		_scene->_nextSceneId = 405;
		_vm->_sound->command(58);
	} else if (_action.isAction(VERB_TAKE, NOUN_TARGET_MODULE) && (_game._objects.isInRoom(OBJ_TARGET_MODULE) || _game._trigger)) {
		switch (_game._trigger) {
		case 0:
			_vm->_sound->command(57);
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], true, 7, 2, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[1]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 2, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_game._objects.addToInventory(OBJ_TARGET_MODULE);
			_vm->_dialogs->showItem(OBJ_TARGET_MODULE, 40847);
			break;

		case 2:
			_game._player._priorTimer = _game._player._ticksAmount + _scene->_frameStartTime;
			_game._player._visible = true;
			_scene->_sequences.addTimer(20, 3);
			break;

		case 3:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_ARMORED_VEHICLE))
		_vm->_dialogs->show(40810);
	else if (_action.isAction(VERB_TAKE, NOUN_ARMORED_VEHICLE) || _action.isAction(VERB_TAKE, NOUN_ANVIL) || _action.isAction(VERB_TAKE, NOUN_TWO_TON_WEIGHT))
		_vm->_dialogs->show(40811);
	else if (_action.isAction(VERB_LOOK, NOUN_MISSILE))
		_vm->_dialogs->show(40812);
	else if (_action.isAction(VERB_TAKE, NOUN_MISSILE))
		_vm->_dialogs->show(40813);
	else if (_action.isAction(VERB_LOOK, NOUN_GRENADE))
		_vm->_dialogs->show(40814);
	else if (_action.isAction(VERB_TAKE, NOUN_GRENADE))
		_vm->_dialogs->show(40815);
	else if (_action.isAction(VERB_TAKE, NOUN_PIN) || _action.isAction(VERB_PULL, NOUN_PIN))
		_vm->_dialogs->show(40816);
	else if (_action.isAction(VERB_LOOK, NOUN_BLIMP))
		_vm->_dialogs->show(40817);
	else if (_action.isAction(VERB_TAKE, NOUN_BLIMP))
		_vm->_dialogs->show(40818);
	else if (_action.isAction(VERB_LOOK, NOUN_AMMUNITION))
		_vm->_dialogs->show(40819);
	else if (_action.isAction(VERB_TAKE, NOUN_AMMUNITION))
		_vm->_dialogs->show(40820);
	else if (_action.isAction(VERB_LOOK, NOUN_CATAPULT))
		_vm->_dialogs->show(40821);
	else if (_action.isAction(VERB_TAKE, NOUN_CATAPULT))
		_vm->_dialogs->show(40822);
	else if (_action.isAction(VERB_LOOK, NOUN_CHEST)) {
		if (_game._objects.isInRoom(OBJ_TARGET_MODULE))
			_vm->_dialogs->show(40823);
		else
			_vm->_dialogs->show(40824);
	} else if (_action.isAction(VERB_TAKE, NOUN_CHEST))
		_vm->_dialogs->show(40825);
	else if (_action.isAction(VERB_LOOK, NOUN_SUIT_OF_ARMOR))
		_vm->_dialogs->show(40826);
	else if (_action.isAction(VERB_TAKE, NOUN_SUIT_OF_ARMOR))
		_vm->_dialogs->show(40827);
	else if (_action.isAction(VERB_LOOK, NOUN_ESCAPE_HATCH))
		_vm->_dialogs->show(40828);
	else if (_action.isAction(VERB_OPEN, NOUN_ESCAPE_HATCH) || _action.isAction(VERB_PULL, NOUN_ESCAPE_HATCH))
		_vm->_dialogs->show(40829);
	else if (_action.isAction(VERB_LOOK, NOUN_BARRELS))
		_vm->_dialogs->show(40830);
	else if (_action.isAction(VERB_TAKE, NOUN_BARRELS))
		_vm->_dialogs->show(40831);
	else if (_action.isAction(VERB_LOOK, NOUN_INFLATABLE_RAFT))
		_vm->_dialogs->show(40832);
	else if (_action.isAction(VERB_TAKE, NOUN_INFLATABLE_RAFT))
		_vm->_dialogs->show(40833);
	else if (_action.isAction(VERB_LOOK, NOUN_TOMATO))
		_vm->_dialogs->show(40834);
	else if (_action.isAction(VERB_TAKE, NOUN_TOMATO))
		_vm->_dialogs->show(40835);
	else if (_action.isAction(VERB_LOOK, NOUN_ANVIL))
		_vm->_dialogs->show(40836);
	else if (_action.isAction(VERB_LOOK, NOUN_TWO_TON_WEIGHT))
		_vm->_dialogs->show(40837);
	else if (_action.isAction(VERB_LOOK, NOUN_POWDER_CONTAINER))
		_vm->_dialogs->show(40838);
	else if (_action.isAction(VERB_LOOK, NOUN_POWDER_PUFF))
		_vm->_dialogs->show(40839);
	else if (_action.isAction(VERB_TAKE, NOUN_POWDER_PUFF))
		_vm->_dialogs->show(40840);
	else if (_action.isAction(VERB_LOOK, NOUN_CARTON))
		_vm->_dialogs->show(40841);
	else if (_action.isAction(VERB_TAKE, NOUN_CARTON))
		_vm->_dialogs->show(40842);
	else if (_action.isAction(VERB_OPEN, NOUN_CARTON))
		_vm->_dialogs->show(40843);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_SOUTH))
		_vm->_dialogs->show(40844);
	else if (_action._lookFlag)
		_vm->_dialogs->show(40845);
	else if (_action.isAction(VERB_LOOK, NOUN_TARGET_MODULE) && _game._objects.isInRoom(OBJ_TARGET_MODULE))
		_vm->_dialogs->show(40846);
	else if (_action.isAction(VERB_LOOK, NOUN_LOADING_RAMP))
		_vm->_dialogs->show(40848);
	else if (_action.isAction(VERB_OPEN, NOUN_CHEST))
		_vm->_dialogs->show(40849);
	else
		return;

	_action._inProgress = false;
}

void room_408_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_408_preload() {
	room_init_code_pointer = room_408_init;
	room_pre_parser_code_pointer = room_408_pre_parser;
	room_parser_code_pointer = room_408_parser;

	section_4_walker();
	section_4_interface();
	_scene->addActiveVocab(NOUN_TARGET_MODULE);
	_scene->addActiveVocab(VERB_WALKTO);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
