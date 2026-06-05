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

Scene602::Scene602(RexNebularEngine *vm) : Scene6xx(vm) {
	_lastSpriteIdx = -1;
	_lastSequenceIdx = -1;
	_cycleIndex = -1;
	_safeMode = -1;
}

void Scene602::synchronize(Common::Serializer &s) {
	Scene6xx::synchronize(s);

	s.syncAsSint16LE(_lastSpriteIdx);
	s.syncAsSint16LE(_lastSequenceIdx);
	s.syncAsSint16LE(_cycleIndex);
	s.syncAsSint16LE(_safeMode);
}

void Scene602::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_SAFE);
	_scene->addActiveVocab(NOUN_LASER_BEAM);
}

void Scene602::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('h', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('l', 0));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RXMRC_9");

	if (!_game._visitedScenes._sceneRevisited)
		_globals[kSafeStatus] = 0;

	if (_globals[kLaserHoleIsThere]) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 9);
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 9);
		int idx = _scene->_dynamicHotspots.add(NOUN_LASER_BEAM, VERB_WALKTO, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(80, 134), FACING_NORTHEAST);
		_scene->changeVariant(1);
	} else
		_scene->_hotspots.activate(NOUN_HOLE, false);

	if (_globals[kSafeStatus] == 0) {
		_lastSpriteIdx = _globals._spriteIndexes[2];
		_cycleIndex = -1;
	} else if (_globals[kSafeStatus] == 1) {
		_lastSpriteIdx = _globals._spriteIndexes[2];
		_cycleIndex = -2;
	} else if (_globals[kSafeStatus] == 3) {
		_lastSpriteIdx = _globals._spriteIndexes[3];
		_cycleIndex = -2;
	} else {
		_lastSpriteIdx = _globals._spriteIndexes[3];
		_cycleIndex = -1;
	}

	_lastSequenceIdx = _scene->_sequences.startCycle(_lastSpriteIdx, false, _cycleIndex);
	_scene->_sequences.setDepth(_lastSequenceIdx, 14);
	int idx = _scene->_dynamicHotspots.add(NOUN_SAFE, VERB_WALKTO, _lastSequenceIdx, Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, Common::Point(185, 113), FACING_NORTHWEST);

	if (_game._objects.isInRoom(OBJ_DOOR_KEY)) {
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('k', -1));
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 15);
		if (_globals[kSafeStatus] == 0 || _globals[kSafeStatus] == 2)
			_scene->_hotspots.activate(NOUN_DOOR_KEY, false);
	} else
		_scene->_hotspots.activate(NOUN_DOOR_KEY, false);

	if (_scene->_priorSceneId == 603) {
		_game._player._playerPos = Common::Point(228, 126);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(50, 127);
		_game._player._facing = FACING_EAST;
	}

	sceneEntrySound();
	_game.loadQuoteSet(0x2F1, 0x2F2, 0x2F3, 0);

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_NOTE);
		_game._objects.addToInventory(OBJ_REARVIEW_MIRROR);
		_game._objects.addToInventory(OBJ_COMPACT_CASE);
	}
}

void Scene602::handleSafeActions() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], true, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 3);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 3, 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		break;

	case 1:
		if (_safeMode == 1 || _safeMode == 3) {
			if (_globals[kSafeStatus] == 0 && _safeMode == 1) {
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2F1));
				_scene->_sequences.addTimer(120, 4);
			} else {
				_scene->_sequences.remove(_lastSequenceIdx);
				if (_safeMode == 3)
					_lastSpriteIdx = _globals._spriteIndexes[2];
				else
					_lastSpriteIdx = _globals._spriteIndexes[3];

				_lastSequenceIdx = _scene->_sequences.addSpriteCycle(_lastSpriteIdx, false, 12, 1, 0, 0);
				_scene->_sequences.setDepth(_lastSequenceIdx, 14);
				if (_game._objects[OBJ_DOOR_KEY]._roomNumber == _scene->_currentSceneId)
					_scene->_hotspots.activate(NOUN_DOOR_KEY, true);

				_scene->_sequences.addSubEntry(_lastSequenceIdx,
					SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
		} else {
			_scene->_sequences.remove(_lastSequenceIdx);
			if (_globals[kSafeStatus] == 1)
				_lastSpriteIdx = _globals._spriteIndexes[2];
			else
				_lastSpriteIdx = _globals._spriteIndexes[3];

			_lastSequenceIdx = _scene->_sequences.startPingPongCycle(_lastSpriteIdx, false, 12, 1, 0, 0);
			_scene->_sequences.setDepth(_lastSequenceIdx, 14);
			if (_game._objects[OBJ_DOOR_KEY]._roomNumber == _scene->_currentSceneId)
				_scene->_hotspots.activate(NOUN_DOOR_KEY, false);

			_scene->_sequences.addSubEntry(_lastSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		}
		break;

	case 2:
	{
		int synxIdx = _lastSequenceIdx;
		_lastSequenceIdx = _scene->_sequences.startCycle(_lastSpriteIdx, false, _cycleIndex);
		_scene->_sequences.setDepth(_lastSequenceIdx, 14);
		_scene->_sequences.updateTimeout(_lastSequenceIdx, synxIdx);
		int idx = _scene->_dynamicHotspots.add(NOUN_SAFE, VERB_WALKTO, _lastSequenceIdx, Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(185, 113), FACING_NORTHWEST);
		if (_safeMode == 3) {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2F3));
			_scene->_sequences.addTimer(120, 4);
		} else
			_scene->_sequences.addTimer(60, 4);
		break;
	}

	case 3:
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[5]);
		_game._player._visible = true;
		break;

	case 4:
		if (_safeMode == 1) {
			if (_globals[kSafeStatus] == 2)
				_globals[kSafeStatus] = 3;
		} else if (_safeMode == 2) {
			if (_globals[kSafeStatus] == 3)
				_globals[kSafeStatus] = 2;
			else
				_globals[kSafeStatus] = 0;
		} else
			_globals[kSafeStatus] = 1;

		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

void Scene602::actions() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_HALLWAY))
		_scene->_nextSceneId = 601;
	else if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOORWAY))
		_scene->_nextSceneId = 603;
	else if (_action.isAction(VERB_OPEN, NOUN_SAFE) && ((_globals[kSafeStatus] == 0) || (_globals[kSafeStatus] == 2))) {
		_safeMode = 1;
		_cycleIndex = -2;
		handleSafeActions();
	} else if (_action.isAction(VERB_CLOSE, NOUN_SAFE) && ((_globals[kSafeStatus] == 1) || (_globals[kSafeStatus] == 3))) {
		_safeMode = 2;
		_cycleIndex = -1;
		handleSafeActions();
	} else if (_action.isAction(VERB_UNLOCK, NOUN_COMBINATION, NOUN_SAFE)) {
		if ((_globals[kSafeStatus] == 0) && (_game._difficulty != DIFFICULTY_HARD)) {
			_safeMode = 3;
			_cycleIndex = -2;
			handleSafeActions();
		}
	} else if ((_action.isAction(VERB_PUT, NOUN_REARVIEW_MIRROR, NOUN_LASER_BEAM) || _action.isAction(VERB_PUT, NOUN_COMPACT_CASE, NOUN_LASER_BEAM)
		|| _action.isAction(VERB_REFLECT, NOUN_COMPACT_CASE, NOUN_LASER_BEAM) || _action.isAction(VERB_REFLECT, NOUN_REARVIEW_MIRROR, NOUN_LASER_BEAM)) && (_globals[kSafeStatus] == 0)) {
		switch (_game._trigger) {
		case 0:
			_vm->_dialogs->show(60230);
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[4]);
			_scene->_sequences.remove(_lastSequenceIdx);
			_scene->loadAnimation(formAnimName('L', 1), 1);
			break;

		case 1:
		{
			_game._player._visible = true;
			_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
			_lastSpriteIdx = _globals._spriteIndexes[3];
			_lastSequenceIdx = _scene->_sequences.startCycle(_lastSpriteIdx, false, -1);
			_scene->_sequences.setDepth(_lastSequenceIdx, 14);
			int idx = _scene->_dynamicHotspots.add(NOUN_SAFE, VERB_WALKTO, _lastSequenceIdx, Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(185, 113), FACING_NORTHWEST);
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 9);
			idx = _scene->_dynamicHotspots.add(NOUN_LASER_BEAM, VERB_WALKTO, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(80, 134), FACING_NORTHEAST);
			_scene->_sequences.addTimer(60, 2);
		}
		break;

		case 2:
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2F2));
			_globals[kSafeStatus] = 2;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_DOOR_KEY) && (_game._trigger || _game._objects.isInRoom(OBJ_DOOR_KEY))) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], true, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 3);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 3, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[6]);
			_scene->_hotspots.activate(NOUN_DOOR_KEY, false);
			_vm->_sound->command(9);
			_game._objects.addToInventory(OBJ_DOOR_KEY);
			break;

		case 2:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[5]);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_vm->_dialogs->showItem(OBJ_DOOR_KEY, 835);
			break;

		default:
			break;
		}
	} else if (_action._lookFlag)
		_vm->_dialogs->show(60210);
	else if (_action.isAction(VERB_LOOK, NOUN_FLOOR))
		_vm->_dialogs->show(60211);
	else if (_action.isAction(VERB_LOOK, NOUN_HALLWAY))
		_vm->_dialogs->show(60212);
	else if (_action.isAction(VERB_LOOK, NOUN_TABLE))
		_vm->_dialogs->show(60213);
	else if (_action.isAction(VERB_LOOK, NOUN_CHAIR) || _action.isAction(VERB_LOOK, NOUN_LOUNGE_CHAIR))
		_vm->_dialogs->show(60214);
	else if (_action.isAction(VERB_LOOK, NOUN_NEON_LIGHTS))
		_vm->_dialogs->show(60215);
	else if (_action.isAction(VERB_LOOK, NOUN_FIREPLACE))
		_vm->_dialogs->show(60216);
	else if (_action.isAction(VERB_LOOK, NOUN_PICTURE))
		_vm->_dialogs->show(60217);
	else if (_action.isAction(VERB_LOOK, NOUN_LAMP))
		_vm->_dialogs->show(60218);
	else if (_action.isAction(VERB_LOOK, NOUN_MASKS))
		_vm->_dialogs->show(60219);
	else if (_action.isAction(VERB_LOOK, NOUN_GLASS_BLOCK_WALL))
		_vm->_dialogs->show(60220);
	else if (_action.isAction(VERB_LOOK, NOUN_DOORWAY))
		_vm->_dialogs->show(60221);
	else if (_action.isAction(VERB_LOOK, NOUN_SAFE)) {
		if (_globals[kSafeStatus] == 0)
			_vm->_dialogs->show(60222);
		else if (_globals[kSafeStatus] == 1) {
			if (!_game._objects.isInRoom(OBJ_DOOR_KEY))
				_vm->_dialogs->show(60223);
			else
				_vm->_dialogs->show(60224);
		} else if (_globals[kSafeStatus] == 2)
			_vm->_dialogs->show(60234);
		else if (_game._objects.isInRoom(OBJ_DOOR_KEY))
			_vm->_dialogs->show(60235);
		else
			_vm->_dialogs->show(60236);
	} else if (_action.isAction(VERB_UNLOCK, NOUN_DOOR_KEY, NOUN_SAFE) || _action.isAction(VERB_UNLOCK, NOUN_PADLOCK_KEY, NOUN_SAFE))
		_vm->_dialogs->show(60225);
	else if (_action.isAction(VERB_PULL, NOUN_SAFE))
		_vm->_dialogs->show(60226);
	else if (_action.isAction(VERB_PUT, NOUN_FIREPLACE) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId)))
		_vm->_dialogs->show(60227);
	else if (_action.isAction(VERB_LOOK, NOUN_HOLE))
		_vm->_dialogs->show(60228);
	else if (_action.isAction(VERB_LOOK, NOUN_LASER_BEAM))
		_vm->_dialogs->show(60229);
	else if (_action.isAction(VERB_LOOK, NOUN_FLOWER_BOX))
		_vm->_dialogs->show(60231);
	else if (_action.isAction(VERB_THROW, NOUN_BOMB, NOUN_SAFE) || _action.isAction(VERB_THROW, NOUN_BOMBS, NOUN_SAFE))
		_vm->_dialogs->show(60232);
	else if (_action.isAction(VERB_PUT, NOUN_TIMEBOMB))
		_vm->_dialogs->show(60233);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
