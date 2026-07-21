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

#include "math/utils.h"
#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section5.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_551_init() {
	if (_globals[kSexOfRex] == REX_MALE)
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 0));
	else
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 1));

	if (_scene->_priorSceneId == 501)
		_game._player._playerPos = Common::Point(18, 130);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(124, 119);
		_game._player._facing = FACING_NORTH;
	}

	if (_globals[kTeleporterCommand]) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;

		char sepChar;
		if (_globals[kSexOfRex] == REX_MALE)
			sepChar = 'e';
		else
			sepChar = 'u';

		int suffixNum;
		int trigger;

		switch (_globals[kTeleporterCommand]) {
		case 1:
			suffixNum = 3;
			trigger = 75;
			_globals[kTeleporterUnderstood] = true;
			break;

		case 2:
			suffixNum = 1;
			trigger = 80;
			break;

		case 4:
			suffixNum = 2;
			trigger = 90;
			break;

		default:
			trigger = 0;
			suffixNum = 0;
		}

		_globals[kTeleporterCommand] = 0;

		if (suffixNum > 0)
			_scene->loadAnimation(formAnimName(sepChar, suffixNum), trigger);
		else {
			_game._player._visible = true;
			_game._player._stepEnabled = true;
		}
	}

	section_5_music();
}

static void room_551_daemon() {
	switch (_game._trigger) {
	case 75:
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		break;

	case 80:
		_globals[kTeleporterCommand] = 1;
		_scene->_nextSceneId = _globals[kTeleporterDestination];
		_scene->_reloadSceneFlag = true;
		break;

	case 90:
		if (_globals[kSexOfRex] == REX_MALE) {
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 8);
		} else {
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
		}
		_vm->_sound->command(28);
		_scene->_sequences.addTimer(60, 91);
		break;

	case 91:
		_scene->_reloadSceneFlag = true;
		break;

	default:
		break;
	}
}

static void room_551_pre_parser() {
	if (player_said_1(walk_down) && (player_said_1(street_to_west) || player_said_1(sidewalk_to_west)))
		_game._player._walkOffScreenSceneId = 501;
}

static void room_551_parser() {
	if (player_said_2(step_into, teleporter))
		_scene->_nextSceneId = 502;
	else if ((_action._lookFlag))
		_vm->_dialogs->show(55117);
	else if (player_said_2(look, skeleton))
		_vm->_dialogs->show(55110);
	else if (player_said_2(look, elevator_shaft))
		_vm->_dialogs->show(55111);
	else if (player_said_2(walkto, elevator_shaft))
		_vm->_dialogs->show(55112);
	else if (player_said_2(look, building))
		_vm->_dialogs->show(55113);
	else if (player_said_2(look, teleporter))
		_vm->_dialogs->show(55114);
	else if (player_said_2(look, sidewalk_to_west)) {
		if (_game._visitedScenes.exists(505))
			_vm->_dialogs->show(55116);
		else
			_vm->_dialogs->show(55115);
	} else if (player_said_2(look, sidewalk))
		_vm->_dialogs->show(55118);
	else if (player_said_2(look, equipment_overhead))
		_vm->_dialogs->show(55119);
	else if (player_said_2(look, railing))
		_vm->_dialogs->show(55120);
	else
		return;

	_action._inProgress = false;
}

void room_551_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_551_preload() {
	room_init_code_pointer = room_551_init;
	room_daemon_code_pointer = room_551_daemon;
	room_pre_parser_code_pointer = room_551_pre_parser;
	room_parser_code_pointer = room_551_parser;

	section_5_walker();
	section_5_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
