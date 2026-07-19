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
	bool _checkGuardFl;
};

static Scratch local;


static void room_311_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXCL_8");
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXCL_2");

	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

	local._checkGuardFl = false;
	_game.loadQuoteSet(0xFA, 0);

	if (_scene->_priorSceneId == 391) {
		_globals[kSexOfRex] = REX_MALE;
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_game._player._facing = FACING_SOUTH;
		_game._player._playerPos = Common::Point(166, 101);
		_scene->_sequences.addTimer(120, 71);
	} else if (_scene->_priorSceneId == 310)
		_game._player._playerPos = Common::Point(302, 145);
	else if (_scene->_priorSceneId == 320) {
		_game._player._playerPos = Common::Point(129, 113);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->loadAnimation(formAnimName('a', -1), 70);
	}

	section_3_music();
}

static void room_311_daemon() {
	if (_game._trigger == 70)
		_scene->_nextSceneId = 310;

	if (_game._trigger >= 71) {
		switch (_game._trigger) {
		case 71:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 72);
			break;

		case 72:
			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 73);
			break;

		case 73:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 3);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 3);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 74);
			break;

		case 74:
			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 4, 5);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
			break;

		case 75:
		{
			int oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 6);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
			_scene->_sequences.addTimer(15, 76);
		}
		break;

		case 76:
			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 7);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 77);
			break;

		case 77:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 8);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 78);
			break;

		case 78:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 9);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 79);
			break;

		case 79:
			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 10, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
			break;

		case 80:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);
			_game._player._stepEnabled = true;
			_game._player._visible = true;
			break;

		default:
			break;
		}
	}

	if (_game._player._moving && (_scene->_rails.getNext() > 0)) {
		int x = _game._player._prepareWalkPos.x;
		if (x < 75)
			x = 75;
		if (x > 207)
			x = 207;

		local._checkGuardFl = true;
		_game._player.startWalking(Common::Point(x, 122), FACING_SOUTH);
		_scene->_rails.resetNext();
	}
}

static void room_311_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(31119);
	else if (local._checkGuardFl) {
		local._checkGuardFl = false;
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.addQuote(250, 0, 240);
	} else if (player_said_2(sit_at, desk))
		_scene->_nextSceneId = 320;
	else if (player_said_2(climb_into, air_vent)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 50, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 3, -2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 15, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, 4);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
		{
			int oldIdx = _globals._sequenceIndexes[1];
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], oldIdx);
		}
		break;

		case 2:
		{
			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 4, 10);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		}
		break;

		case 3:
		{
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 3);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 11);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(167, 100));
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
			_scene->_sequences.addTimer(15, 4);
		}
		break;

		case 4:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 12, 14);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(167, 100));
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
			break;

		case 5:
		{
			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 15);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(167, 100));
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
			_scene->_sequences.addTimer(15, 6);
		}
		break;

		case 6:
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
			_scene->_sequences.addTimer(15, 7);
			break;

		case 7:
			_scene->_nextSceneId = 313;
			break;

		default:
			break;
		}
	} else if (player_said_2(look, desk))
		_vm->_dialogs->show(31110);
	else if (player_said_2(look, wall))
		_vm->_dialogs->show(31111);
	else if (player_said_2(look, lighting_fixture) || player_said_2(stare_at, lighting_fixture))
		_vm->_dialogs->show(31112);
	else if (player_said_2(look, lights) || player_said_2(stare_at, lights))
		_vm->_dialogs->show(31113);
	else if (player_said_2(take, lights))
		_vm->_dialogs->show(31114);
	else if (player_said_2(look, light) || player_said_2(stare_at, light))
		_vm->_dialogs->show(31115);
	else if (player_said_2(take, light))
		_vm->_dialogs->show(31116);
	else if (player_said_2(look, corridor_to_west))
		_vm->_dialogs->show(31117);
	else if (player_said_2(look, corridor_to_east))
		_vm->_dialogs->show(31118);
	else if (player_said_2(look, air_vent))
		_vm->_dialogs->show(31120);
	else
		return;

	_action._inProgress = false;
}

void room_311_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._checkGuardFl);
}

void room_311_preload() {
	room_init_code_pointer = room_311_init;
	room_parser_code_pointer = room_311_parser;
	room_daemon_code_pointer = room_311_daemon;

	if (room_id == 391)
		global[kSexOfRex] = REX_MALE;

	section_3_walker();
	section_3_interface();

	if (room_id == 304)
		*player.series_name = '\0';
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
