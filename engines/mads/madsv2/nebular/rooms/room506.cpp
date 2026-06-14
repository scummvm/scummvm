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

Scene506::Scene506(RexNebularEngine *vm) : Scene5xx(vm), _doorPos(0, 0) {
	_heroFacing = FACING_DUMMY;

	_doorDepth = -1;
	_doorSpriteIdx = -1;
	_doorSequenceIdx = -1;
	_doorWord = -1;

	_labDoorFl = false;
	_firstDoorFl = false;
	_actionFl = false;
}

void room_506_synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsSint16LE(_doorPos.x);
	s.syncAsSint16LE(_doorPos.y);

	s.syncAsByte(_heroFacing);

	s.syncAsSint16LE(_doorDepth);
	s.syncAsSint16LE(_doorSpriteIdx);
	s.syncAsSint16LE(_doorSequenceIdx);
	s.syncAsSint16LE(_doorWord);

	s.syncAsByte(_labDoorFl);
	s.syncAsByte(_firstDoorFl);
	s.syncAsByte(_actionFl);
}

void Scene506::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(VERB_WALK_INTO);
	_scene->addActiveVocab(NOUN_SOFTWARE_STORE);
	_scene->addActiveVocab(NOUN_LABORATORY);
}

static void room_506_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('q', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('q', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('c', -1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXCD_3");

	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	int idx = _scene->_dynamicHotspots.add(NOUN_LABORATORY, VERB_WALK_INTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
	int hotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(65, 125), FACING_NORTHWEST);
	_scene->_dynamicHotspots.setCursor(hotspotId, CURSOR_GO_LEFT);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
	_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
	idx = _scene->_dynamicHotspots.add(NOUN_SOFTWARE_STORE, VERB_WALK_INTO, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
	hotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(112, 102), FACING_NORTHWEST);
	_scene->_dynamicHotspots.setCursor(hotspotId, CURSOR_GO_LEFT);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 13);

	_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 5);
	_firstDoorFl = true;
	_actionFl = false;

	if (_scene->_priorSceneId == 508) {
		_game._player._playerPos = Common::Point(16, 111);
		_game._player._facing = FACING_SOUTHEAST;
		_scene->_sequences.addTimer(15, 80);
		_game._player._stepEnabled = false;
	} else if (_scene->_priorSceneId == 507) {
		_game._player._playerPos = Common::Point(80, 102);
		_game._player._facing = FACING_SOUTHEAST;
		_scene->_sequences.addTimer(60, 80);
		_game._player._stepEnabled = false;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(138, 116);
		_game._player._facing = FACING_NORTHEAST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 5);
		_scene->loadAnimation(formAnimName('R', 1), 70);
	}
	sceneEntrySound();
}

void Scene506::step() {
	if (_game._trigger >= 80) {
		if (_firstDoorFl) {
			_heroFacing = FACING_SOUTHEAST;
			if (_scene->_priorSceneId == 507) {
				_doorPos = Common::Point(112, 102);
				_doorWord = 0x336;
			} else {
				_doorPos = Common::Point(65, 125);
				_doorWord = 0x37D;
			}
		}
		handleDoorSequences();
	}

	if (_game._trigger >= 70) {
		switch (_game._trigger) {
		case 70:
			_game._player._visible = true;
			_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
			_scene->_sequences.addTimer(6, 71);
			break;

		case 71:
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 5);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
			break;

		case 72:
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 5);
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	}
}

void Scene506::handleDoorSequences() {
	_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;

	if (_firstDoorFl) {
		if (_action.isAction(VERB_WALK_INTO, NOUN_SOFTWARE_STORE) || ((_scene->_priorSceneId == 507) && !_actionFl)) {
			_doorDepth = 13;
			_doorSpriteIdx = _globals._spriteIndexes[2];
			_doorSequenceIdx = _globals._sequenceIndexes[2];
			_labDoorFl = false;
		} else {
			_doorDepth = 10;
			_doorSpriteIdx = _globals._spriteIndexes[1];
			_doorSequenceIdx = _globals._sequenceIndexes[1];
			_labDoorFl = true;
		}
		_firstDoorFl = false;
	}

	switch (_game._trigger) {
	case 0:
	case 80:
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_doorSequenceIdx);
		_doorSequenceIdx = _scene->_sequences.addSpriteCycle(_doorSpriteIdx, false, 7, 1, 0, 0);
		_scene->_sequences.setDepth(_doorSequenceIdx, _doorDepth);
		_scene->changeVariant(1);
		_scene->_sequences.addSubEntry(_doorSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 81);
		break;


	case 81:
		_doorSequenceIdx = _scene->_sequences.startCycle(_doorSpriteIdx, false, -2);
		_scene->_sequences.setDepth(_doorSequenceIdx, _doorDepth);
		_game._player._walkAnywhere = true;
		_game._player.walk(_doorPos, _heroFacing);
		_scene->_sequences.addTimer(120, 82);
		break;

	case 82:
		_scene->_sequences.remove(_doorSequenceIdx);
		_doorSequenceIdx = _scene->_sequences.addReverseSpriteCycle(_doorSpriteIdx, false, 7, 1, 0, 0);
		_scene->_sequences.setDepth(_doorSequenceIdx, _doorDepth);
		if (_actionFl)
			_scene->_sequences.addSubEntry(_doorSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 84);
		else
			_scene->_sequences.addSubEntry(_doorSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 83);

		break;

	case 83:
	{
		_doorSequenceIdx = _scene->_sequences.startCycle(_doorSpriteIdx, false, 1);
		int idx = _scene->_dynamicHotspots.add(_doorWord, VERB_WALK_INTO, _doorSequenceIdx, Common::Rect(0, 0, 0, 0));
		int hotspotId = _scene->_dynamicHotspots.setPosition(idx, _doorPos, FACING_NORTHWEST);
		_scene->_dynamicHotspots.setCursor(hotspotId, CURSOR_GO_LEFT);
		_scene->_sequences.setDepth(_doorSequenceIdx, _doorDepth);
		_firstDoorFl = true;
		if (_labDoorFl) {
			_globals._spriteIndexes[1] = _doorSpriteIdx;
			_globals._sequenceIndexes[1] = _doorSequenceIdx;
		} else {
			_globals._spriteIndexes[2] = _doorSpriteIdx;
			_globals._sequenceIndexes[2] = _doorSequenceIdx;
		}
		_game._player._stepEnabled = true;

	}
	break;

	case 84:
		_actionFl = false;
		_game._player._stepEnabled = true;
		if (_labDoorFl)
			_scene->_nextSceneId = 508;
		else
			_scene->_nextSceneId = 507;

		break;

	default:
		break;
	}
}

static void room_506_parser() {
	if (_action.isAction(VERB_WALK_INTO, NOUN_LABORATORY)) {
		if (_firstDoorFl) {
			_heroFacing = FACING_NORTHWEST;
			_doorPos = Common::Point(16, 111);
		}
		_actionFl = true;
		handleDoorSequences();
	} else if (_action.isAction(VERB_WALK_INTO, NOUN_SOFTWARE_STORE)) {
		if (_firstDoorFl) {
			_heroFacing = FACING_NORTHWEST;
			_doorPos = Common::Point(80, 102);
		}
		_actionFl = true;
		handleDoorSequences();
	} else if (_action.isAction(VERB_GET_INTO, NOUN_CAR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 5);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
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
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_STREET))
		_vm->_dialogs->show(50618);
	else if (_action.isAction(VERB_LOOK, NOUN_RESTAURANT))
		_vm->_dialogs->show(50610);
	else if (_action.isAction(VERB_LOOK, NOUN_MOTEL))
		_vm->_dialogs->show(50611);
	else if (_action.isAction(VERB_LOOK, NOUN_CYCLE_SHOP))
		_vm->_dialogs->show(50612);
	else if (_action.isAction(VERB_LOOK, NOUN_AIR_BIKE))
		_vm->_dialogs->show(50613);
	else if (_action.isAction(VERB_TAKE, NOUN_AIR_BIKE))
		_vm->_dialogs->show(50614);
	else if (_action.isAction(VERB_LOOK, NOUN_SOFTWARE_STORE))
		_vm->_dialogs->show(50615);
	else if (_action.isAction(VERB_LOOK, NOUN_LABORATORY))
		_vm->_dialogs->show(50616);
	else if (_action.isAction(VERB_LOOK, NOUN_STREET_TO_WEST) || _action.isAction(VERB_WALK_DOWN, NOUN_STREET_TO_WEST))
		_vm->_dialogs->show(50617);
	else if (_action.isAction(VERB_LOOK, NOUN_SOFTWARE_STORE_SIGN))
		_vm->_dialogs->show(50619);
	else if (_action.isAction(VERB_LOOK, NOUN_CAR))
		_vm->_dialogs->show(50620);
	else if (_action.isAction(VERB_LOOK, NOUN_SKY))
		_vm->_dialogs->show(50621);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
