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
#include "mads/nebular/rooms/section6.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_601_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXCD_4");

	if (_globals[kLaserHoleIsThere]) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
		_scene->_dynamicHotspots.add(words_laser_beam, words_look_at, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
	}

	_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 3);

	if (_scene->_priorSceneId == 504) {
		_game._player._playerPos = Common::Point(73, 148);
		_game._player._facing = FACING_WEST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 3);
		_scene->loadAnimation(formAnimName('R', 1), 70);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(229, 129);
		_game._player._facing = FACING_SOUTHWEST;
	}

	section_6_music();
}

static void room_601_daemon() {
	switch (_game._trigger) {
	case 70:
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
		_scene->_sequences.addTimer(30, 71);
		break;

	case 71:
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 3);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

static void room_601_parser() {
	if (player_said_2(walk_through, entrance))
		_scene->_nextSceneId = 602;
	else if (player_said_2(get_inside, car)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 3);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], syncIdx);
			_scene->_sequences.addTimer(6, 2);
		}
		break;

		case 2:
			_game._player._visible = false;
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 10, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
			_scene->_nextSceneId = 504;
		}
		break;

		default:
			break;
		}
	} else if (_action._lookFlag || player_said_2(look, street)) {
		if (!_globals[kLaserHoleIsThere])
			_vm->_dialogs->show(60110);
		else
			_vm->_dialogs->show(60111);
	} else if (player_said_2(look, car))
		_vm->_dialogs->show(60112);
	else if (player_said_2(look, papers))
		_vm->_dialogs->show(60113);
	else if (player_said_2(look, building))
		_vm->_dialogs->show(60114);
	else if (player_said_2(walk_down, street))
		_vm->_dialogs->show(60115);
	else if (player_said_2(look, balcony))
		_vm->_dialogs->show(60116);
	else if (player_said_2(look, entrance))
		_vm->_dialogs->show(60117);
	else if (player_said_2(look, wall))
		_vm->_dialogs->show(60118);
	else if (player_said_2(look, city))
		_vm->_dialogs->show(60119);
	else if (player_said_2(look, fountain))
		_vm->_dialogs->show(60120);
	else
		return;

	_action._inProgress = false;
}

void room_601_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_601_preload() {
	room_init_code_pointer = room_601_init;
	room_daemon_code_pointer = room_601_daemon;
	room_parser_code_pointer = room_601_parser;

	section_6_walker();
	section_6_interface();
	_scene->addActiveVocab(words_laser_beam);
	_scene->addActiveVocab(words_look_at);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
