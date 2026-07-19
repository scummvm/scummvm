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
		int idx = _scene->_dynamicHotspots.add(words_target_module, words_walkto, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(283, 128), FACING_NORTHEAST);
	}
	section_4_music();
}

static void room_408_pre_parser() {
	if ((_action.isAction(words_take) && !_action.isObject(words_target_module)) || _action.isAction(words_pull, words_pin) || _action.isAction(words_open, words_carton))
		_game._player._needToWalk = false;

	if ((_action.isAction(words_look, words_target_module) && _game._objects.isInRoom(OBJ_TARGET_MODULE)) || _action.isAction(words_look, words_chest))
		_game._player._needToWalk = true;
}

static void room_408_parser() {
	if (_action.isAction(words_walk_into, words_corridor_to_south)) {
		_scene->_nextSceneId = 405;
		_vm->_sound->command(58);
	} else if (_action.isAction(words_take, words_target_module) && (_game._objects.isInRoom(OBJ_TARGET_MODULE) || _game._trigger)) {
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
	} else if (_action.isAction(words_look, words_armored_vehicle))
		_vm->_dialogs->show(40810);
	else if (_action.isAction(words_take, words_armored_vehicle) || _action.isAction(words_take, words_anvil) || _action.isAction(words_take, words_two_ton_weight))
		_vm->_dialogs->show(40811);
	else if (_action.isAction(words_look, words_missile))
		_vm->_dialogs->show(40812);
	else if (_action.isAction(words_take, words_missile))
		_vm->_dialogs->show(40813);
	else if (_action.isAction(words_look, words_grenade))
		_vm->_dialogs->show(40814);
	else if (_action.isAction(words_take, words_grenade))
		_vm->_dialogs->show(40815);
	else if (_action.isAction(words_take, words_pin) || _action.isAction(words_pull, words_pin))
		_vm->_dialogs->show(40816);
	else if (_action.isAction(words_look, words_blimp))
		_vm->_dialogs->show(40817);
	else if (_action.isAction(words_take, words_blimp))
		_vm->_dialogs->show(40818);
	else if (_action.isAction(words_look, words_ammunition))
		_vm->_dialogs->show(40819);
	else if (_action.isAction(words_take, words_ammunition))
		_vm->_dialogs->show(40820);
	else if (_action.isAction(words_look, words_catapult))
		_vm->_dialogs->show(40821);
	else if (_action.isAction(words_take, words_catapult))
		_vm->_dialogs->show(40822);
	else if (_action.isAction(words_look, words_chest)) {
		if (_game._objects.isInRoom(OBJ_TARGET_MODULE))
			_vm->_dialogs->show(40823);
		else
			_vm->_dialogs->show(40824);
	} else if (_action.isAction(words_take, words_chest))
		_vm->_dialogs->show(40825);
	else if (_action.isAction(words_look, words_suit_of_armor))
		_vm->_dialogs->show(40826);
	else if (_action.isAction(words_take, words_suit_of_armor))
		_vm->_dialogs->show(40827);
	else if (_action.isAction(words_look, words_escape_hatch))
		_vm->_dialogs->show(40828);
	else if (_action.isAction(words_open, words_escape_hatch) || _action.isAction(words_pull, words_escape_hatch))
		_vm->_dialogs->show(40829);
	else if (_action.isAction(words_look, words_barrels))
		_vm->_dialogs->show(40830);
	else if (_action.isAction(words_take, words_barrels))
		_vm->_dialogs->show(40831);
	else if (_action.isAction(words_look, words_inflatable_raft))
		_vm->_dialogs->show(40832);
	else if (_action.isAction(words_take, words_inflatable_raft))
		_vm->_dialogs->show(40833);
	else if (_action.isAction(words_look, words_tomato))
		_vm->_dialogs->show(40834);
	else if (_action.isAction(words_take, words_tomato))
		_vm->_dialogs->show(40835);
	else if (_action.isAction(words_look, words_anvil))
		_vm->_dialogs->show(40836);
	else if (_action.isAction(words_look, words_two_ton_weight))
		_vm->_dialogs->show(40837);
	else if (_action.isAction(words_look, words_powder_container))
		_vm->_dialogs->show(40838);
	else if (_action.isAction(words_look, words_powder_puff))
		_vm->_dialogs->show(40839);
	else if (_action.isAction(words_take, words_powder_puff))
		_vm->_dialogs->show(40840);
	else if (_action.isAction(words_look, words_carton))
		_vm->_dialogs->show(40841);
	else if (_action.isAction(words_take, words_carton))
		_vm->_dialogs->show(40842);
	else if (_action.isAction(words_open, words_carton))
		_vm->_dialogs->show(40843);
	else if (_action.isAction(words_look, words_corridor_to_south))
		_vm->_dialogs->show(40844);
	else if (_action._lookFlag)
		_vm->_dialogs->show(40845);
	else if (_action.isAction(words_look, words_target_module) && _game._objects.isInRoom(OBJ_TARGET_MODULE))
		_vm->_dialogs->show(40846);
	else if (_action.isAction(words_look, words_loading_ramp))
		_vm->_dialogs->show(40848);
	else if (_action.isAction(words_open, words_chest))
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
	_scene->addActiveVocab(words_target_module);
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
