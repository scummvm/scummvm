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
#include "mads/madsv2/nebular/rooms/section6.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _videoDoorMode;
};

static Scratch local;


static void room_609_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('h', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXCD_9");
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RXMRC_9");

	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
	_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);

	if (!_game._visitedScenes._sceneRevisited)
		_globals[kBeenInVideoStore] = false;

	if (_scene->_priorSceneId == 611) {
		_game._player._playerPos = Common::Point(264, 69);
		_game._player._facing = FACING_SOUTHWEST;
	} else if (_scene->_priorSceneId == 610) {
		_game._player._playerPos = Common::Point(23, 90);
		_game._player._facing = FACING_EAST;
		_scene->_sequences.addTimer(60, 60);
		_game._player._stepEnabled = false;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(86, 136);
		_game._player._facing = FACING_NORTHEAST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
		_scene->loadAnimation(formAnimName('R', 1), 70);
	}

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_DOOR_KEY);
		if (_game._difficulty != DIFFICULTY_EASY)
			_game._objects.addToInventory(OBJ_PENLIGHT);
	}

	section_6_music();
	_game.loadQuoteSet(0x305, 0x306, 0x307, 0x308, 0x309, 0);
}

static void room_609_daemon() {
	switch (_game._trigger) {
	case 60:
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
		break;

	case 61:
		_scene->_hotspots.activate(NOUN_VIDEO_STORE_DOOR, false);
		_game._player.walk(Common::Point(101, 100), FACING_EAST);
		_scene->_sequences.addTimer(180, 62);
		break;

	case 62:
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_hotspots.activate(NOUN_VIDEO_STORE_DOOR, true);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 63);
		break;

	case 63:
		if (!_globals[kHasTalkedToHermit] && (_game._difficulty != DIFFICULTY_HARD)) {
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 26, 2, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(287, 73));
			_scene->_sequences.setScale(_globals._sequenceIndexes[3], 47);
		}
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 70:
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
		_scene->_sequences.addTimer(6, 71);
		break;

	case 71:
		if (!_globals[kHasTalkedToHermit]) {
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 26, 2, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(287, 73));
			_scene->_sequences.setScale(_globals._sequenceIndexes[3], 47);
		}
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

static void enterStore() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		if (local._videoDoorMode == 2)
			_scene->_sequences.addTimer(1, 4);
		else {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x305));
			_scene->_sequences.addTimer(120, 1);
		}
		break;

	case 1:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x306));
		_scene->_sequences.addTimer(60, 2);
		break;

	case 2:
		_game._player._visible = false;
		_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], true, 11, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 2);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		break;

	case 3:
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[5]);
		_game._player._visible = true;
		_game._objects.setRoom(OBJ_DOOR_KEY, 1);
		_scene->_sequences.addTimer(15, 4);
		break;

	case 4:
		_game._player._visible = false;
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], true, 1);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.addTimer(15, 5);
		break;

	case 5:
		_scene->_sequences.remove(_globals._sequenceIndexes[5]);
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_game._player._visible = true;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 6);
		break;

	case 6:
		_scene->_hotspots.activate(NOUN_VIDEO_STORE_DOOR, false);
		if (local._videoDoorMode == 1) {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 180, _game.getQuote(0x307));
		}
		_game._player.walk(Common::Point(23, 90), FACING_WEST);
		_scene->_sequences.addTimer(180, 7);
		break;

	case 7:
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 8);
		break;

	case 8:
		_scene->_hotspots.activate(NOUN_VIDEO_STORE_DOOR, true);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		_globals[kBeenInVideoStore] = true;
		_game._player._stepEnabled = true;
		_scene->_nextSceneId = 610;
		break;

	default:
		break;
	}
}

static void room_609_pre_parser() {
	if (_action.isAction(VERB_UNLOCK, NOUN_DOOR_KEY, NOUN_VIDEO_STORE_DOOR))
		_game._player.walk(Common::Point(78, 99), FACING_NORTHWEST);
}

static void room_609_parser() {
	if (_action.isAction(VERB_WALK_TOWARDS, NOUN_ALLEY))
		_scene->_nextSceneId = 611;
	else if (_action.isAction(VERB_WALK_THROUGH, NOUN_VIDEO_STORE_DOOR)) {
		if (!_globals[kBeenInVideoStore]) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x308));
				_scene->_sequences.addTimer(120, 1);
				break;

			case 1:
				_game._player._visible = false;
				_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], true, 1);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
				_scene->_sequences.addTimer(30, 2);
				break;

			case 2:
				_scene->_sequences.remove(_globals._sequenceIndexes[5]);
				_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
				_game._player._visible = true;
				_scene->_sequences.addTimer(60, 3);
				break;

			case 3:
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x309));
				_scene->_sequences.addTimer(120, 4);
				break;

			case 4:
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		} else {
			local._videoDoorMode = 2;
			enterStore();
		}
	} else if (_action.isAction(VERB_UNLOCK, NOUN_DOOR_KEY, NOUN_VIDEO_STORE_DOOR)) {
		local._videoDoorMode = 1;
		enterStore();
	} else if (_action.isAction(VERB_GET_INSIDE, NOUN_CAR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = _globals._sequenceIndexes[1];
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 5);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], syncIdx);
			_scene->_sequences.addTimer(6, 2);
		}
		break;

		case 2:
			_game._player._visible = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 10, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = _globals._sequenceIndexes[4];
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], syncIdx);
			_scene->_nextSceneId = 504;
		}
		break;

		default:
			break;
		}
	} else if (_action._lookFlag)
		_vm->_dialogs->show(60910);
	else if (_action.isAction(VERB_LOOK, NOUN_STREET))
		_vm->_dialogs->show(60911);
	else if (_action.isAction(VERB_LOOK, NOUN_SPOT_A_POT))
		_vm->_dialogs->show(60912);
	else if (_action.isAction(VERB_LOOK, NOUN_VIDEO_STORE))
		_vm->_dialogs->show(60913);
	else if (_action.isAction(VERB_LOOK, NOUN_BILLBOARD))
		_vm->_dialogs->show(60914);
	else if (_action.isAction(VERB_LOOK, NOUN_STATUE))
		_vm->_dialogs->show(60915);
	else if (_action.isAction(VERB_LOOK, NOUN_CAR))
		_vm->_dialogs->show(60916);
	else if (_action.isAction(VERB_LOOK, NOUN_NEWSSTAND))
		_vm->_dialogs->show(60917);
	else if (_action.isAction(VERB_LOOK, NOUN_VIDEO_STORE_DOOR)) {
		if (!_globals[kBeenInVideoStore])
			_vm->_dialogs->show(60918);
		else
			_vm->_dialogs->show(60919);
	} else if (_action.isAction(VERB_WALK_DOWN, NOUN_STREET))
		_vm->_dialogs->show(60730);
	else
		return;

	_action._inProgress = false;
}

void room_609_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._videoDoorMode);
}

void room_609_preload() {
	room_init_code_pointer = room_609_init;
	room_daemon_code_pointer = room_609_daemon;
	room_pre_parser_code_pointer = room_609_pre_parser;
	room_parser_code_pointer = room_609_parser;

	section_6_walker();
	section_6_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
