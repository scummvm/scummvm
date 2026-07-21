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
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_351_init() {
	_globals[kAfterHavoc] = -1;
	_globals[kTeleporterRoom + 1] = 351;

	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*ROXRC_7");
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXRD_7");

	if (_game._objects.isInRoom(OBJ_CREDIT_CHIP)) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);
	} else
		_scene->_hotspots.activate(words_credit_chip, false);

	if (_scene->_priorSceneId == 352)
		_game._player._playerPos = Common::Point(148, 152);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(207, 81);
		_game._player._facing = FACING_NORTH;
	}

	if (_globals[kTeleporterCommand]) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;

		char sepChar = 'a';
		if (_globals[kSexOfRex] != REX_MALE)
			sepChar = 'b';

		int suffixNum = -1;
		int trigger = 0;

		switch (_globals[kTeleporterCommand]) {
		case 1:
			suffixNum = 0;
			trigger = 60;
			_globals[kTeleporterCommand] = true;
			break;

		case 2:
			suffixNum = 1;
			trigger = 61;
			break;

		case 3:
		case 4:
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_game._player._turnToFacing = FACING_SOUTH;
			suffixNum = -1;
			break;

		default:
			break;
		}

		_globals[kTeleporterCommand] = 0;

		if (suffixNum >= 0)
			_scene->loadAnimation(formAnimName(sepChar, suffixNum), trigger);
	}

	section_3_music();
}

static void room_351_daemon() {
	if (_game._trigger == 60) {
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_game._player._turnToFacing = FACING_SOUTH;
	}

	if (_game._trigger == 61) {
		_globals[kTeleporterCommand] = 1;
		_scene->_nextSceneId = _globals[kTeleporterDestination];
		_scene->_reloadSceneFlag = true;
	}
}

static void room_351_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(35121);
	else if (player_said_2(step_into, teleporter))
		_scene->_nextSceneId = 322;
	else if (player_said_2(walk_down, corridor_to_south))
		_scene->_nextSceneId = 352;
	else if (player_said_2(take, credit_chip)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_CREDIT_CHIP)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				if (_globals[kSexOfRex] == REX_FEMALE) {
					_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 5, 2, 0, 0);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 5, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				} else {
					_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 5, 2, 0, 0);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 6, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				}
				break;

			case 1:
				_scene->_hotspots.activate(words_credit_chip, false);
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_game._objects.addToInventory(OBJ_CREDIT_CHIP);
				break;

			case 2:
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				_vm->_dialogs->showItem(OBJ_CREDIT_CHIP, 0x32F);
				break;

			default:
				break;
			}
		}
	} else if (player_said_2(look, view_screen))
		_vm->_dialogs->show(35110);
	else if (player_said_2(look, rip_in_floor))
		_vm->_dialogs->show(35111);
	else if (player_said_2(look, fire_hydrant))
		_vm->_dialogs->show(35112);
	else if (player_said_2(look, guard)) {
		if (_game._objects[0xF]._roomNumber == 351)
			_vm->_dialogs->show(35114);
		else
			_vm->_dialogs->show(35113);
	} else if (player_said_2(look, equipment))
		_vm->_dialogs->show(35115);
	else if (player_said_2(look, desk))
		_vm->_dialogs->show(35116);
	else if (player_said_2(look, machine))
		_vm->_dialogs->show(35117);
	else if (player_said_2(look, teleporter))
		_vm->_dialogs->show(35118);
	else if (player_said_2(look, control_panel))
		_vm->_dialogs->show(35119);
	else if (player_said_2(look, corridor_to_south))
		_vm->_dialogs->show(35120);
	else if (player_said_2(look, pole))
		_vm->_dialogs->show(35122);
	else
		return;

	_action._inProgress = false;
}

void room_351_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_351_preload() {
	room_init_code_pointer = room_351_init;
	room_parser_code_pointer = room_351_parser;
	room_daemon_code_pointer = room_351_daemon;

	if (_scene->_currentSceneId == 391)
		_globals[kSexOfRex] = REX_MALE;

	section_3_walker();
	section_3_interface();
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
