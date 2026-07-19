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
#include "mads/madsv2/nebular/rooms/section8.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _walkThroughDoor;
};

static Scratch local;


static void room_801_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('a', -1));

	if (_scene->_priorSceneId != 802) {
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 13);
	}

	if ((_globals[kCameFromCut]) && (_globals[kCutX] != 0)) {
		_game._player._playerPos = Common::Point(_globals[kCutX], _globals[kCutY]);
		_game._player._facing = _globals[kCutFacing];
		_globals[kCutX] = 0;
		_globals[kCameFromCut] = false;
		_globals[kReturnFromCut] = false;
		_globals[kBeamIsUp] = false;
		_globals[kForceBeamDown] = false;
		_globals[kDontRepeat] = false;
	} else if (_scene->_priorSceneId == 808) {
		_game._player._playerPos = Common::Point(148, 110);
		_game._player._facing = FACING_NORTH;
	} else if (_scene->_priorSceneId == 802) {
		_game._player._playerPos = Common::Point(307, 111);
		_game._player.walk(Common::Point(270, 118), FACING_WEST);
		_game._player._visible = true;
	} else if ((_scene->_priorSceneId != RETURNING_FROM_DIALOG) && !_globals[kTeleporterCommand]) {
		_game._player._playerPos = Common::Point(8, 117);
		_game._player.walk(Common::Point(41, 115), FACING_EAST);
		_game._player._visible = true;
	}

	_globals[kBetweenRooms] = false;

	if (_globals[kTeleporterCommand]) {
		_game._player._stepEnabled = false;
		switch (_globals[kTeleporterCommand]) {
		case 1:
			_game._player._playerPos = Common::Point(8, 117);
			_globals[kTeleporterUnderstood] = true;
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 13);
			_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
			_vm->_sound->command(30);
			break;

		case 2:
			_game._player._playerPos = Common::Point(8, 117);
			_globals[kTeleporterUnderstood] = true;
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 13);
			_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
			_vm->_sound->command(30);
			break;

		case 3:
		case 4:
			_game._player._playerPos = Common::Point(8, 117);
			_game._player.walk(Common::Point(41, 115), FACING_EAST);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
		_globals[kTeleporterCommand] = 0;
	}

	local._walkThroughDoor = false;
	if (_scene->_priorSceneId == 802) {
		_game._player._stepEnabled = false;
		local._walkThroughDoor = true;
	}

	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 11, 0, 0, 0);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 9, 0, 0, 0);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -1, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);

	section_8_music();
}

static void room_801_daemon() {
	if (_game._trigger == 75) {
		if (_globals[kSexOfRex] == REX_FEMALE) {
			_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 8);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 140);
		} else {
			_game._player._stepEnabled = true;
			_game._player._visible = true;
			_game._player._playerPos = Common::Point(8, 117);
			_game._player.walk(Common::Point(41, 115), FACING_EAST);
		}
	}

	if (_game._trigger == 140) {
		_vm->_sound->command(27);
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 8);
		_scene->_sequences.addTimer(100, 141);
	}

	if (_game._trigger == 141) {
		_scene->_reloadSceneFlag = true;
		_scene->_nextSceneId = _scene->_priorSceneId;
		_globals[kTeleporterCommand] = 0;
	}

	if (_game._trigger == 80) {
		_globals[kTeleporterCommand] = 1;
		_scene->_nextSceneId = _globals[kTeleporterDestination];
		_scene->_reloadSceneFlag = true;
	}

	if (local._walkThroughDoor && (_game._player._playerPos == Common::Point(270, 118))) {
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 4, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		local._walkThroughDoor = false;
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 120);
	}

	if (_game._trigger == 120) {
		_vm->_sound->command(12);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		_game._player._stepEnabled = true;
	}

	if (_game._trigger == 90) {
		_game._player.walk(Common::Point(307, 111), FACING_EAST);
		_scene->_sequences.addTimer(80, 130);
	}

	if (_game._trigger == 130) {
		_vm->_sound->command(12);
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 4, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 110);
	}

	if (_game._trigger == 110) {
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		_scene->_nextSceneId = 802;
	}
}

static void room_801_pre_parser() {
	if (_action.isAction(words_look, words_control_panel)) {
		_game._player.walk(Common::Point(148, 110), FACING_NORTH);
		_game._player._needToWalk = true;
		_game._player._readyToWalk = true;
	}

	if (_action.isAction(words_walk_inside, words_teleporter) && _globals[kBeamIsUp]) {
		_globals[kCutX] = _game._player._playerPos.x;
		_globals[kCutY] = _game._player._playerPos.y;
		_globals[kCutFacing] = _game._player._facing;
		_globals[kForceBeamDown] = true;
		_globals[kDontRepeat] = true;
		_scene->_nextSceneId = 803;
	}
}

static void room_801_parser() {
	if (_action.isAction(words_look, words_control_panel))
		_scene->_nextSceneId = 808;
	else if (_action.isAction(words_walk_inside, words_teleporter)) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->_nextSceneId = 807;
	} else if (_action.isAction(words_walk_through, words_door) && (_game._player._playerPos == Common::Point(270, 118))) {
		_game._player._stepEnabled = false;
		_game._player._facing = FACING_EAST;
		_game._player.selectSeries();
		_globals[kBetweenRooms] = true;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 4, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 13);
		_vm->_sound->command(11);
	} else if (_action.isAction(words_look, words_ceiling))
		_vm->_dialogs->show(80110);
	else if (_action.isAction(words_look, words_monitor))
		_vm->_dialogs->show(80111);
	else if (_action.isAction(words_look, words_teleporter))
		_vm->_dialogs->show(80112);
	else if (_action.isAction(words_look, words_equipment) || _action._lookFlag)
		_vm->_dialogs->show(80113);
	else if (_action.isAction(words_look, words_speaker))
		_vm->_dialogs->show(80114);
	else if (_action.isAction(words_look, words_eye_chart))
		_vm->_dialogs->show(80115);
	else if (_action.isAction(words_look, words_wall))
		_vm->_dialogs->show(80116);
	else if (_action.isAction(words_look, words_door))
		_vm->_dialogs->show(80117);
	else
		return;

	_action._inProgress = false;
}

void room_801_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._walkThroughDoor);
}

void room_801_preload() {
	room_init_code_pointer = room_801_init;
	room_daemon_code_pointer = room_801_daemon;
	room_pre_parser_code_pointer = room_801_pre_parser;
	room_parser_code_pointer = room_801_parser;

	section_8_walker();
	section_8_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
