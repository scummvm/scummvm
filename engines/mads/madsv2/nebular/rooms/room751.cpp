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

#include "common/scummsys.h"
#include "math/utils.h"
#include "mads/madsv2/nebular/nebular.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

Scene751::Scene751(RexNebularEngine *vm) : Scene7xx(vm) {
	_rexHandingLine = false;
}

void room_751_synchronize(Common::Serializer &s) {
	Scene7xx::synchronize(s);

	s.syncAsByte(_rexHandingLine);
}

void Scene751::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_FISHING_LINE);
	_scene->addActiveVocab(VERB_WALKTO);
}

static void room_751_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*RM701X0");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('f', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RM202A1");

	if (!_game._visitedScenes._sceneRevisited)
		_rexHandingLine = false;

	if (_globals[kLineStatus] == 2 || _globals[kLineStatus] == 3) {
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
		int idx = _scene->_dynamicHotspots.add(NOUN_FISHING_LINE, VERB_WALKTO, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(268, 140), FACING_NORTHWEST);
	}

	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);

	if (_scene->_priorSceneId == 752) {
		_game._player._playerPos = Common::Point(309, 138);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId == 710) {
		_game._player._playerPos = Common::Point(154, 129);
		_game._player._facing = FACING_NORTH;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(155, 129));
		_scene->_sequences.addTimer(15, 70);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(22, 131);
		_game._player._facing = FACING_EAST;
		_game._player._stepEnabled = false;
		_scene->_sequences.addTimer(60, 60);
	} else if (_rexHandingLine) {
		_game._player._visible = false;
		_game._player._playerPos = Common::Point(268, 140);
		_game._player._facing = FACING_NORTHWEST;
		_game._player._visible = false;
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 7);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
	} else if (_globals[kLineStatus] == 2) {
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
		int idx = _scene->_dynamicHotspots.add(NOUN_FISHING_LINE, VERB_WALKTO, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(268, 140), FACING_NORTHWEST);
	}

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_FISHING_LINE);
		_game._objects.addToInventory(OBJ_BINOCULARS);
	}

	sceneEntrySound();
	_game.loadQuoteSet(0x30A, 0x30B, 0x30C, 0x30D, 0x30E, 0);

	if (_globals[kTimebombTimer] > 0)
		_globals[kTimebombTimer] = 10200;
}

void Scene751::step() {
	switch (_game._trigger) {
	case 70:
		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[4], false, 6, 1, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(155, 129));
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		break;

	case 71:
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[4]);
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}

	if ((_globals[kTimebombTimer] >= 10800) && (_globals[kTimebombStatus] == 1)) {
		_globals[kTimebombStatus] = 3;
		_globals[kTimebombTimer] = 0;
		_globals[kCheckDaemonTimebomb] = false;
		_scene->_nextSceneId = 620;
	}

	switch (_game._trigger) {
	case 60:
		_vm->_sound->command(16);
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 1, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
		break;

	case 61:
		_game._player.walk(Common::Point(61, 131), FACING_EAST);
		_scene->_sequences.addTimer(120, 62);
		break;

	case 62:
		_vm->_sound->command(17);
		_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 5, 1, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 63);
		break;

	case 63:
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
		_game._player._stepEnabled = true;
		_scene->_kernelMessages.reset();
		break;

	default:
		break;
	}
}

static void room_751_pre_parser() {
	if (_action.isAction(VERB_LOOK, NOUN_TALL_BUILDING))
		_game._player.walk(Common::Point(154, 129), FACING_NORTHEAST);

	if (_action.isAction(VERB_LOOK, NOUN_BINOCULARS, NOUN_TALL_BUILDING))
		_game._player.walk(Common::Point(154, 129), FACING_NORTH);

	if (_action.isAction(VERB_WALKTO, NOUN_EAST_END_OF_PLATFORM))
		_game._player._walkOffScreenSceneId = 752;

	if (!_rexHandingLine)
		return;

	if (_action.isAction(VERB_LOOK) || _action.isObject(NOUN_FISHING_LINE) || _action.isAction(VERB_TALKTO))
		_game._player._needToWalk = false;

	if ((!_action.isAction(VERB_PUT, NOUN_FISHING_LINE, NOUN_HOOK) || !_action.isAction(VERB_TIE, NOUN_FISHING_LINE, NOUN_HOOK) || !_action.isAction(VERB_ATTACH, NOUN_FISHING_LINE, NOUN_HOOK))
		&& (_game._player._needToWalk)) {
		switch (_game._trigger) {
		case 0:
			_game._player._readyToWalk = false;
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 11, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, 7);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
			_game._player._visible = true;
			_rexHandingLine = false;
			_game._player._stepEnabled = true;
			_game._player._readyToWalk = true;
			break;

		default:
			break;
		}
	}
}

static void room_751_parser() {
	if (_action.isAction(VERB_WALK_ALONG, NOUN_PLATFORM))
		; // Nothing
	else if (_action.isAction(VERB_LOOK, NOUN_BINOCULARS, NOUN_TALL_BUILDING)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(155, 129));
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = _globals._sequenceIndexes[4];
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(155, 129));
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], syncIdx);
			_scene->_sequences.addTimer(15, 2);
		}
		break;

		case 2:
			_scene->_nextSceneId = 710;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_STEP_INTO, NOUN_ELEVATOR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_vm->_sound->command(16);
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x30D));
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			_game._player.walk(Common::Point(22, 131), FACING_EAST);
			_scene->_sequences.addTimer(120, 3);
			break;

		case 3:
			_vm->_sound->command(17);
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 5, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
			break;

		case 4:
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
			_scene->_sequences.addTimer(60, 5);
			break;

		case 5:
			_game._player._stepEnabled = true;
			_scene->_nextSceneId = 513;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PUT, NOUN_FISHING_LINE, NOUN_HOOK) || _action.isAction(VERB_TIE, NOUN_FISHING_LINE, NOUN_HOOK) || _action.isAction(VERB_ATTACH, NOUN_FISHING_LINE, NOUN_HOOK)) {
		if (_globals[kLineStatus] == 1) {
			switch (_game._trigger) {
			case 0:
				_game._player._visible = false;
				_game._player._stepEnabled = false;
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, 6);
				_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], -1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
			{
				int syncIdx = _globals._sequenceIndexes[2];
				_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 7);
				_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], syncIdx);
				_scene->_sequences.addTimer(30, 2);
			}
			break;

			case 2:
				_rexHandingLine = true;
				_scene->_sequences.remove(_globals._sequenceIndexes[2]);
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 8, -2);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
			{
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
				_game._player._visible = true;
				_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
				int idx = _scene->_dynamicHotspots.add(NOUN_FISHING_LINE, VERB_WALKTO, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots.setPosition(idx, Common::Point(268, 140), FACING_NORTHWEST);
				_scene->_kernelMessages.reset();
				_game._objects.setRoom(OBJ_FISHING_LINE, _scene->_currentSceneId);
				_rexHandingLine = false;
				_globals[kLineStatus] = 2;
				_game._player._stepEnabled = true;
				_vm->_dialogs->show(75120);
			}
			break;

			default:
				break;
			}
		}
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_CITY))
		_vm->_dialogs->show(75110);
	else if (_action.isAction(VERB_LOOK, NOUN_ELEVATOR))
		_vm->_dialogs->show(75112);
	else if (_action.isAction(VERB_LOOK, NOUN_PLATFORM))
		_vm->_dialogs->show(75113);
	else if (_action.isAction(VERB_LOOK, NOUN_CEMENT_PYLON))
		_vm->_dialogs->show(75114);
	else if ((_action.isAction(VERB_LOOK, NOUN_HOOK) || _action.isAction(VERB_LOOK, NOUN_FISHING_LINE))
		&& (_globals[kLineStatus] == 2 || _globals[kLineStatus] == 3))
		_vm->_dialogs->show(75116);
	else if (_action.isAction(VERB_LOOK, NOUN_HOOK))
		_vm->_dialogs->show(75115);
	else if (_action.isAction(VERB_LOOK, NOUN_ROCK))
		_vm->_dialogs->show(75117);
	else if (_action.isAction(VERB_TAKE, NOUN_ROCK))
		_vm->_dialogs->show(75118);
	else if (_action.isAction(VERB_LOOK, NOUN_EAST_END_OF_PLATFORM))
		_vm->_dialogs->show(75119);
	else if (_action.isAction(VERB_TAKE, NOUN_FISHING_LINE) && (_globals[kLineStatus] == 3 || _globals[kLineStatus] == 2))
		_vm->_dialogs->show(75121);
	else if (_action.isAction(VERB_LOOK, NOUN_TALL_BUILDING))
		_vm->_dialogs->show(75122);
	else if (_action.isAction(VERB_TIE, NOUN_FISHING_LINE, NOUN_CEMENT_PYLON) || _action.isAction(VERB_ATTACH, NOUN_FISHING_LINE, NOUN_CEMENT_PYLON))
		_vm->_dialogs->show(75123);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
