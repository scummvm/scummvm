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

static void handleRexInGrate() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.addTimer(15, 1);
		break;

	case 1:
		_scene->_sequences.setDone(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 12, 3, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 2, 3);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		break;

	case 2:
	{
		int oldIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 4, 8);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], oldIdx);

		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 2, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
	}
	break;

	case 3:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
	}
	break;

	case 4:
	{
		int oldIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 10, 11);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], oldIdx);

		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 5, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
	}
	break;

	case 5:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);

		oldIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 12);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], oldIdx);
		_scene->_sequences.addTimer(15, 6);
	}
	break;

	case 6:
		_scene->_sequences.setDone(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 13, 14);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);

		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 2, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 7);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 8);
		break;

	case 7:
		_scene->_sequences.setDone(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 15);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], -1);
		break;

	case 8:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addTimer(15, 9);
	}
	break;

	case 9:
		_scene->_sequences.setDone(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.addTimer(48, 10);
		break;

	case 10:
		_scene->_nextSceneId = 313;
		break;

	default:
		break;
	}
}

static void handleRoxInGrate() {
	int temp;
	int temp1;

	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.addTimer(15, 1);
		break;

	case 1:
		_scene->_sequences.setDone(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], false, 17, 3, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 2, 3);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		break;

	case 2:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 17, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 2, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);

		temp = _globals._sequenceIndexes[5];
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 17, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 4, 8);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], temp);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
		break;

	case 3:
		temp1 = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], temp1);
		break;

	case 4:
		temp = _globals._sequenceIndexes[5];
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 17, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 10, 11);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], temp);

		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 17, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 5, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
		break;

	case 5:
		temp = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], temp);

		temp = _globals._sequenceIndexes[5];
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 12);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], temp);
		_scene->_sequences.addTimer(20, 6);
		break;

	case 6:
		_scene->_sequences.setDone(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 17, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 13, 15);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);

		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 17, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 2, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 7);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 8);
		break;

	case 7:
		_scene->_sequences.setDone(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 16);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], -1);
		break;

	case 8:
		temp = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], temp);
		_scene->_sequences.addTimer(20, 9);
		break;

	case 9:
		_scene->_sequences.setDone(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.addTimer(48, 10);
		break;

	case 10:
		_scene->_nextSceneId = 313;
		break;

	default:
		break;
	}
}

static void room_316_init() {
	if (_globals[kSexOfRex] == REX_MALE) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('g', -1));
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXCL_8");
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*RXCL_2");
	} else {
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('g', 0));
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*ROXCL_8");
	}

	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('v', 0));
	_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);

	if (_scene->_priorSceneId == 366) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_game._player._playerPos = Common::Point(78, 87);
		_game._player._facing = FACING_SOUTH;
		_scene->_sequences.addTimer(48, 70);
	} else if (_scene->_priorSceneId == 321) {
		_game._player._playerPos = Common::Point(153, 102);
		_game._player._facing = FACING_SOUTH;
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_vm->_sound->command(44);
		int spriteIdx = (_globals[kSexOfRex] == REX_MALE) ? 1 : 2;
		_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[spriteIdx], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 60);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(291, 126);

	section_3_music();
	_game.loadQuoteSet(0xFD, 0);
}

static void room_316_daemon() {
	if (_game._trigger == 60) {
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[1]);
		_game._player._visible = true;
		_game._player._stepEnabled = true;
	}

	if (_game._trigger >= 70) {
		switch (_game._trigger) {
		case 70:
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 2, 6);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);

			_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 5);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[6]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
			break;

		case 71:
		{
			int synxIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], synxIdx);
		}
		break;

		case 72:
		{
			int synxIdx = _globals._sequenceIndexes[6];
			_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 6, 9);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[6]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[6], synxIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 73);
		}
		break;

		case 73:
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 5);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[6]);

			_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 10, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[6]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 74);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
			break;

		case 74:
		{
			int synxIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], synxIdx);
		}
		break;

		case 75:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[6]);
			_game._player._stepEnabled = true;
			_game._player._visible = true;
			break;

		default:
			break;
		}
	}
}

static void room_316_pre_parser() {
	if (player_said_2(walk_down, corridor_to_east)) {
		if (_globals[kAfterHavoc])
			_game._player._walkOffScreenSceneId = 354;
		else
			_game._player._walkOffScreenSceneId = 304;
	}
}

static void room_316_parser() {
	if (player_said_2(climb_into, air_vent)) {
		if (_globals[kSexOfRex] == REX_FEMALE)
			handleRoxInGrate();
		else
			handleRexInGrate();
	} else if (player_said_2(walk_up, ramp) || player_said_2(walk_onto, platform)) {
		switch (_game._trigger) {
		case 0:
			if (_globals[kCityFlooded]) {
				_vm->_dialogs->show(31623);
			} else {
				_vm->_sound->command(45);
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				if (_globals[kSexOfRex] == REX_MALE) {
					_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 7, 1, 0, 0);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, 7);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				} else {
					_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				}
			}
			break;

		case 1:
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 7, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 8, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
			_scene->_kernelMessages.reset();
			if (!_game._visitedScenes.exists(321))
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(253));

			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 7, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -2, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
			_scene->_sequences.addTimer(48, 4);
			break;

		case 3:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -2, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
			_scene->_sequences.addTimer(48, 4);
			break;

		case 4:
			_scene->_nextSceneId = 321;
			break;

		default:
			break;
		}
	} else if (player_said_2(look, platform))
		_vm->_dialogs->show(31610);
	else if (player_said_2(look, strange_device)) {
		if (_game._visitedScenes.exists(321))
			_vm->_dialogs->show(31612);
		else
			_vm->_dialogs->show(31611);
	} else if (player_said_2(look, controls))
		_vm->_dialogs->show(31613);
	else if (player_said_2(look, equipment))
		_vm->_dialogs->show(31614);
	else if (player_said_2(look, panel))
		_vm->_dialogs->show(31615);
	else if (player_said_2(look, monitor))
		_vm->_dialogs->show(31616);
	else if (player_said_2(look, ramp))
		_vm->_dialogs->show(31617);
	else if (player_said_2(look, air_vent))
		_vm->_dialogs->show(31618);
	else if (player_said_2(look, corridor_to_east)) {
		if (!_globals[kAfterHavoc]) {
			if (_game._difficulty != DIFFICULTY_EASY)
				_vm->_dialogs->show(31620);
			else
				_vm->_dialogs->show(31619);
		}
	} else if (player_said_2(look, floor))
		_vm->_dialogs->show(31621);
	else if (player_said_2(look, support))
		_vm->_dialogs->show(31622);
	else
		return;

	_action._inProgress = false;
}

void room_316_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_316_preload() {
	room_init_code_pointer = room_316_init;
	room_pre_parser_code_pointer = room_316_pre_parser;
	room_parser_code_pointer = room_316_parser;
	room_daemon_code_pointer = room_316_daemon;

	if (room_id == 366)
		global[kSexOfRex] = REX_MALE;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
