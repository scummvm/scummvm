/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/scene.h"
#include "mads/nebular/nebular_scenes.h"
#include "mads/nebular/nebular_scenes6.h"

namespace MADS {

namespace Nebular {

void Scene6xx::setAAName() {
	_game._aaName = Resources::formatAAName(5);
}

void Scene6xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);
	Common::String oldName = _game._player._spritesPrefix;

	if ((_scene->_nextSceneId == 605) || (_scene->_nextSceneId == 620))
		_game._player._spritesPrefix = "";
	else if (_globals[kSexOfRex] == REX_MALE)
		_game._player._spritesPrefix = "RXM";
	else
		_game._player._spritesPrefix = "ROX";

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_game._player._scalingVelocity = true;
	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

void Scene6xx::sceneEntrySound() {
	if (!_vm->_musicFlag) {
		_vm->_sound->command(2);
		return;
	}

	switch (_scene->_nextSceneId) {
	case 601:
	case 602:
	case 603:
	case 604:
	case 605:
	case 607:
	case 608:
	case 609:
	case 610:
	case 612:
	case 620:
		_vm->_sound->command(29);
		break;
	case 611:
		_vm->_sound->command(24);
		break;
	default:
		break;
	}
}

/*------------------------------------------------------------------------*/

void Scene601::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_LASER_BEAM);
	_scene->addActiveVocab(VERB_LOOK_AT);
}

void Scene601::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXCD_4");

	if (_globals[kLaserHoleIsThere]) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
		_scene->_dynamicHotspots.add(NOUN_LASER_BEAM, VERB_LOOK_AT, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
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

	sceneEntrySound();
}

void Scene601::step() {
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

void Scene601::actions() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ENTRANCE))
		_scene->_nextSceneId = 602;
	else if (_action.isAction(VERB_GET_INSIDE, NOUN_CAR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 3);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1: {
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

		case 3: {
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
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_STREET)) {
		if (!_globals[kLaserHoleIsThere])
			_vm->_dialogs->show(60110);
		else
			_vm->_dialogs->show(60111);
	} else if (_action.isAction(VERB_LOOK, NOUN_CAR))
		_vm->_dialogs->show(60112);
	else if (_action.isAction(VERB_LOOK, NOUN_PAPERS))
		_vm->_dialogs->show(60113);
	else if (_action.isAction(VERB_LOOK, NOUN_BUILDING))
		_vm->_dialogs->show(60114);
	else if (_action.isAction(VERB_WALK_DOWN, NOUN_STREET))
		_vm->_dialogs->show(60115);
	else if (_action.isAction(VERB_LOOK, NOUN_BALCONY))
		_vm->_dialogs->show(60116);
	else if (_action.isAction(VERB_LOOK, NOUN_ENTRANCE))
		_vm->_dialogs->show(60117);
	else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(60118);
	else if (_action.isAction(VERB_LOOK, NOUN_CITY))
		_vm->_dialogs->show(60119);
	else if (_action.isAction(VERB_LOOK, NOUN_FOUNTAIN))
		_vm->_dialogs->show(60120);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene602::Scene602(MADSEngine *vm) : Scene6xx(vm) {
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

	case 2: {
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

		case 1: {
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

/*------------------------------------------------------------------------*/

Scene603::Scene603(MADSEngine *vm) : Scene6xx(vm) {
	_compactCaseHotspotId = -1;
	_noteHotspotId = -1;
}

void Scene603::synchronize(Common::Serializer &s) {
	Scene6xx::synchronize(s);

	s.syncAsSint16LE(_compactCaseHotspotId);
	s.syncAsSint16LE(_noteHotspotId);
}

void Scene603::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_COMPACT_CASE);
	_scene->addActiveVocab(NOUN_NOTE);
}

void Scene603::enter() {
	if (_game._objects[OBJ_COMPACT_CASE]._roomNumber == _scene->_currentSceneId) {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXMRD_3");
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', -1));
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_compactCaseHotspotId = _scene->_dynamicHotspots.add(NOUN_COMPACT_CASE, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_compactCaseHotspotId, Common::Point(250, 152), FACING_SOUTHEAST);
	}

	if ((_game._difficulty != DIFFICULTY_HARD) && (_game._objects[OBJ_NOTE]._roomNumber == _scene->_currentSceneId)) {
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXMRC_9");
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('p', -1));
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
		_noteHotspotId = _scene->_dynamicHotspots.add(NOUN_NOTE, VERB_WALKTO, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_noteHotspotId, Common::Point(242, 118), FACING_NORTHEAST);
	}

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(113, 134);

	sceneEntrySound();
}

void Scene603::actions() {
	if (_action.isAction(VERB_WALK_TOWARDS, NOUN_LIVINGROOM))
		_scene->_nextSceneId = 602;
	else if (_action.isAction(VERB_TAKE, NOUN_COMPACT_CASE)) {
		if ( _game._trigger || !_game._objects.isInInventory(OBJ_COMPACT_CASE)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 1, 5);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				_vm->_sound->command(9);
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_scene->_dynamicHotspots.remove(_compactCaseHotspotId);
				_game._objects.addToInventory(OBJ_COMPACT_CASE);
				_vm->_dialogs->showItem(OBJ_COMPACT_CASE, 60330);
				break;

			case 2:
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[4]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_NOTE)) {
		if ( _game._trigger || !_game._objects.isInInventory(OBJ_NOTE)) {
			if (_game._trigger == 0) {
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
				_scene->_sequences.addTimer(15, 1);
			} else if (_game._trigger == 1) {
				_vm->_sound->command(9);
				_scene->_sequences.remove(_globals._sequenceIndexes[2]);
				_scene->_dynamicHotspots.remove(_noteHotspotId);
				_game._objects.addToInventory(OBJ_NOTE);
				_scene->_sequences.remove(_globals._sequenceIndexes[3]);
				_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
				_game._player._visible = true;
				_game._player._stepEnabled = true;
			}
		} else
			_vm->_dialogs->show(60323);
	} else if (_action._lookFlag)
		_vm->_dialogs->show(60310);
	else if (_action.isAction(VERB_LOOK, NOUN_BED))
		_vm->_dialogs->show(60311);
	else if (_action.isAction(VERB_LOOK, NOUN_WIG_STAND))
		_vm->_dialogs->show(60312);
	else if (_action.isAction(VERB_TAKE, NOUN_WIG_STAND))
		_vm->_dialogs->show(60313);
	else if (_action.isAction(VERB_LOOK, NOUN_REVIEW))
		_vm->_dialogs->show(60314);
	else if (_action.isAction(VERB_LOOK, NOUN_SOUVENIR_TICKETS))
		_vm->_dialogs->show(60315);
	else if (_action.isAction(VERB_LOOK, NOUN_PHOTOGRAPH))
		_vm->_dialogs->show(60316);
	else if (_action.isAction(VERB_LOOK, NOUN_LAMP))
		_vm->_dialogs->show(60317);
	else if (_action.isAction(VERB_LOOK, NOUN_DIRECTORS_SLATE) || _action.isAction(VERB_LOOK, NOUN_CROP) || _action.isAction(VERB_LOOK, NOUN_MEGAPHONE))
		_vm->_dialogs->show(60318);
	else if (_action.isAction(VERB_LOOK, NOUN_SNAPSHOT))
		_vm->_dialogs->show(60319);
	else if (_action.isAction(VERB_TAKE, NOUN_SNAPSHOT))
		_vm->_dialogs->show(60320);
	else if (_action.isAction(VERB_LOOK, NOUN_PERFUME))
		_vm->_dialogs->show(60321);
	else if (_action.isAction(VERB_TAKE, NOUN_PERFUME))
		_vm->_dialogs->show(60322);
	else if (_action.isAction(VERB_LOOK, NOUN_NOTE)) {
		if (_game._objects[OBJ_NOTE]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60324);
	} else if (_action.isAction(VERB_LOOK, NOUN_CORNER_TABLE)) {
		if (_game._objects[OBJ_NOTE]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60326);
		else
			_vm->_dialogs->show(60325);
	} else if (_action.isAction(VERB_LOOK, NOUN_VANITY)) {
		if (_game._objects[OBJ_COMPACT_CASE]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60327);
		else
			_vm->_dialogs->show(60328);
	} else if (_action.isAction(VERB_LOOK, NOUN_COMPACT_CASE) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(60329);
	// For the next two checks, the second part of the check wasn't surrounded par parenthesis, which was obviously wrong
	else if (_action.isAction(VERB_LOOK) && (_action.isObject(NOUN_BRA) || _action.isObject(NOUN_BOA) || _action.isObject(NOUN_SLIP)))
		_vm->_dialogs->show(60331);
	else if (_action.isAction(VERB_TAKE) && (_action.isObject(NOUN_BRA) || _action.isObject(NOUN_BOA) || _action.isObject(NOUN_SLIP)))
		_vm->_dialogs->show(60332);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene604::Scene604(MADSEngine *vm) : Scene6xx(vm) {
	_timebombHotspotId = -1;
	_bombMode = -1;
	_monsterFrame = -1;

	_monsterTimer = 0;

	_monsterActive = false;
	_animationActiveFl = false;
}

void Scene604::synchronize(Common::Serializer &s) {
	Scene6xx::synchronize(s);

	s.syncAsSint16LE(_timebombHotspotId);
	s.syncAsSint16LE(_bombMode);
	s.syncAsSint16LE(_monsterFrame);

	s.syncAsUint32LE(_monsterTimer);

	s.syncAsByte(_monsterActive);
	s.syncAsByte(_animationActiveFl);
}

void Scene604::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_SEA_MONSTER);
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_TIMEBOMB);
}

void Scene604::enter() {
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXCD_9");
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(Resources::formatName(620, 'b', 0, EXT_SS, ""));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RXMRC_9");

	if (_globals[kTimebombStatus] == 1) {
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, -1);
		_timebombHotspotId = _scene->_dynamicHotspots.add(NOUN_TIMEBOMB, VERB_WALKTO, _globals._sequenceIndexes[6], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_timebombHotspotId, Common::Point(166, 118), FACING_NORTHEAST);
	}

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_TIMEBOMB);

	_vm->_palette->setEntry(252, 63, 37, 26);
	_vm->_palette->setEntry(253, 45, 24, 17);
	_animationActiveFl = false;

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(72, 149);
		_game._player._facing = FACING_NORTHEAST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
		_scene->loadAnimation(formAnimName('R', 1), 70);
		_animationActiveFl = true;
	} else {
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
	}

	_monsterTimer = _scene->_frameStartTime;
	_monsterActive = false;

	sceneEntrySound();
	_game.loadQuoteSet(0x2E7, 0x2E8, 0x2E9, 0x2EA, 0x2EB, 0x2EC, 0x2ED, 0x2EE, 0x2EF, 0x2F0, 0);
}

void Scene604::step() {
	switch (_game._trigger) {
	case 70:
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
		_scene->_sequences.addTimer(30, 71);
		break;

	case 71:
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
		_game._player._stepEnabled = true;
		_animationActiveFl = false;
		break;

	default:
		break;
	}

	if (_monsterActive && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _monsterFrame) {
			_monsterFrame = _scene->_animation[0]->getCurrentFrame();
			int nextMonsterFrame = -1;

			switch (_monsterFrame) {
			case 50:
			case 137:
			case 174: {
				int randVal = _vm->getRandomNumber(1, 1000);
				if ((randVal <= 450) && (_game._player._special)) {
					if (_game._player._special == 1)
						nextMonsterFrame = 50;
					else if (_game._player._special == 2)
						nextMonsterFrame = 84;
					else
						nextMonsterFrame = 137;
				} else if (randVal <= 150)
					nextMonsterFrame = 50;
				else if (randVal <= 300)
					nextMonsterFrame = 84;
				else if (randVal <= 450)
					nextMonsterFrame = 137;
				else if (randVal < 750)
					nextMonsterFrame = 13;
				else
					nextMonsterFrame = 114;

				}
				break;

			case 84:
				nextMonsterFrame = 14;
				break;

			default:
				break;
			}

			if ((nextMonsterFrame >= 0) && (nextMonsterFrame != _monsterFrame)) {
				_scene->_animation[0]->setCurrentFrame(nextMonsterFrame);
				_monsterFrame = nextMonsterFrame;
			}
		}
	}

	if ((!_monsterActive && !_animationActiveFl) && (_scene->_frameStartTime > (_monsterTimer + 4))) {
		_monsterTimer = _scene->_frameStartTime;
		if ((_vm->getRandomNumber(1, 1000) < 25) || !_game._visitedScenes._sceneRevisited) {
			_monsterActive = true;
			_scene->freeAnimation();
			_scene->loadAnimation(formAnimName('m', -1));
		}
	}
}

void Scene604::handleBombActions() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], false, 9, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 3);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		if (_bombMode == 1)
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 3, 1);
		else
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 3, 2);

		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		break;

	case 1:
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, -1);
		_timebombHotspotId = _scene->_dynamicHotspots.add(NOUN_TIMEBOMB, VERB_WALKTO, _globals._sequenceIndexes[6], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_timebombHotspotId, Common::Point(166, 118), FACING_NORTHEAST);
		_game._objects.setRoom(OBJ_TIMEBOMB, _scene->_currentSceneId);
		break;

	case 2:
		_scene->_sequences.remove(_globals._sequenceIndexes[6]);
		_scene->_dynamicHotspots.remove(_timebombHotspotId);
		_game._objects.addToInventory(OBJ_TIMEBOMB);
		break;

	case 3:
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[5]);
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		if (_bombMode == 1) {
			_vm->_dialogs->show(60421);
			_globals[kTimebombStatus] = TIMEBOMB_ACTIVATED;
			_globals[kTimebombTimer] = 0;
		} else {
			_vm->_dialogs->show(60423);
			_globals[kTimebombStatus] = TIMEBOMB_DEACTIVATED;
			_globals[kTimebombTimer] = 0;
		}
		break;

	default:
		break;
	}
}

void Scene604::actions() {
	if (_action.isAction(VERB_GET_INSIDE, NOUN_CAR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1: {
			int syncIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], syncIdx);
			_scene->_sequences.addTimer(6, 2);
			}
			break;

		case 2:
			_game._player._visible = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 10, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3: {
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
	} else if ((_action.isAction(VERB_PUT, NOUN_LEDGE) || _action.isAction(VERB_PUT, NOUN_VIEWPORT) || _action.isAction(VERB_THROW, NOUN_VIEWPORT))
		&& (_action.isObject(NOUN_BOMB) || _action.isObject(NOUN_BOMBS)))
		_vm->_dialogs->show(60420);
	else if (_action.isAction(VERB_PUT, NOUN_TIMEBOMB, NOUN_LEDGE) || _action.isAction(VERB_PUT, NOUN_TIMEBOMB, NOUN_VIEWPORT)) {
		_bombMode = 1;
		if ((_game._difficulty == DIFFICULTY_HARD) || _globals[kWarnedFloodCity])
			handleBombActions();
		else if (
			(_game._objects.isInInventory(OBJ_POLYCEMENT) && (_game._objects.isInInventory(OBJ_CHICKEN) || _game._objects.isInInventory(OBJ_CHICKEN_BOMB)))
			 && (_globals[kLineStatus] == LINE_TIED || (_game._difficulty == DIFFICULTY_EASY && !_globals[kBoatRaised]))
			)
			// The original can get in an impossible state at this point, if the player has
			// combined the chicken with the bomb before placing the timer bomb on the ledge.
			// Therefore, we also allow the player to place the bomb if the chicken bomb is
			// in the inventory.
			handleBombActions();
		else if (_game._difficulty == DIFFICULTY_EASY)
			_vm->_dialogs->show(60424);
		else {
			_vm->_dialogs->show(60425);
			_globals[kWarnedFloodCity] = true;
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_TIMEBOMB)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_TIMEBOMB)) {
			_bombMode = 2;
			handleBombActions();
		}
	} else if (_action._lookFlag)
		_vm->_dialogs->show(60411);
	else if (_action.isAction(VERB_LOOK, NOUN_VIEWPORT)) {
		if (_monsterActive) {
			_vm->_dialogs->show(60413);
		} else {
			_vm->_dialogs->show(60412);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(60414);
	else if (_action.isAction(VERB_LOOK, NOUN_VENT))
		_vm->_dialogs->show(60415);
	else if (_action.isAction(VERB_LOOK, NOUN_INDICATOR))
		_vm->_dialogs->show(60416);
	else if (_action.isAction(VERB_LOOK, NOUN_SCULPTURE))
		_vm->_dialogs->show(60417);
	else if (_action.isAction(VERB_LOOK, NOUN_CAR))
		_vm->_dialogs->show(60418);
	else if (_action.isAction(VERB_LOOK, NOUN_FOUNTAIN))
		_vm->_dialogs->show(60419);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene605::setup() {
	_game._player._spritesPrefix = "";
	setAAName();
}

void Scene605::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('r', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('b', -1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('l', -1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('p', -1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('n', -1));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('f', -1));

	_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 15, 0, 0, 0);
	_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 17, 0, 0, 0);
	_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 14, 0, 0, 0);
	_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 13, 0, 0, 0);
	_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], false, 17, 0, 0, 0);
	_globals._sequenceIndexes[6] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[6], false, 18, 0, 0, 0);

	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->_sequences.addTimer(600, 70);
	_scene->_userInterface.setup(kInputLimitedSentences);
	sceneEntrySound();
	_vm->_sound->command(22);
}

void Scene605::step() {
	if (_game._trigger == 70) {
		_vm->_sound->command(23);
		if (_globals[kResurrectRoom] >= 700)
			_vm->_dialogs->show(60598);
		else
			_vm->_dialogs->show(60599);

		_scene->_nextSceneId = _globals[kResurrectRoom];
	}
}

void Scene605::actions() {
	return;
}

/*------------------------------------------------------------------------*/

Scene607::Scene607(MADSEngine *vm) : Scene6xx(vm) {
	_dogTimer = 0;
	_lastFrameTime = 0;

	_dogLoop = false;
	_dogEatsRex = false;
	_dogBarking = false;
	_shopAvailable = false;

	_animationMode = -1;
	_animationActive = -1;
	_counter = -1;
}

void Scene607::synchronize(Common::Serializer &s) {
	Scene6xx::synchronize(s);

	s.syncAsUint32LE(_dogTimer);
	s.syncAsUint32LE(_lastFrameTime);

	s.syncAsByte(_dogLoop);
	s.syncAsByte(_dogEatsRex);
	s.syncAsByte(_dogBarking);
	s.syncAsByte(_shopAvailable);

	s.syncAsSint16LE(_animationMode);
	s.syncAsSint16LE(_animationActive);
	s.syncAsSint16LE(_counter);
}

void Scene607::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_OBNOXIOUS_DOG);
	_scene->addActiveVocab(VERB_WALKTO);
}

void Scene607::enter() {
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXCD_3");

	if (!_game._visitedScenes._sceneRevisited && (_scene->_priorSceneId != 608))
		_globals[kDogStatus] = DOG_PRESENT;

	if ((_scene->_priorSceneId == 608) && (_globals[kDogStatus] < DOG_GONE))
		_globals[kDogStatus] = DOG_GONE;

	_animationActive = 0;

	if ((_globals[kDogStatus] == DOG_PRESENT) && (_game._difficulty != DIFFICULTY_EASY)) {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('g', 3));
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('g', 7));
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('g', 0));

		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
		_dogBarking = false;
		_dogLoop = false;
		_shopAvailable = false;
		_dogEatsRex = false;
		_dogTimer = 0;
	} else
		_scene->_hotspots.activate(NOUN_OBNOXIOUS_DOG, false);

	_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);

	if (_scene->_priorSceneId == 608) {
		_game._player._playerPos = Common::Point(297, 50);
		_game._player._facing = FACING_SOUTHEAST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(40, 104);
		_game._player._facing = FACING_SOUTHEAST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
		_scene->loadAnimation(formAnimName('R', 1), 80);
	} else if (_globals[kDogStatus] == DOG_LEFT) {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('g', 3));
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('g', 7));
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('g', 0));
	}

	sceneEntrySound();

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_BONES);

	_vm->_palette->setEntry(252, 63, 44, 30);
	_vm->_palette->setEntry(253, 63, 20, 22);
	_game.loadQuoteSet(0x2F8, 0x2F7, 0x2F6, 0x2F9, 0x2FA, 0);
}

void Scene607::step() {
	if (_globals[kDogStatus] == DOG_LEFT) {
		int32 diff = _scene->_frameStartTime - _lastFrameTime;
		if ((diff >= 0) && (diff <= 4))
			_dogTimer += diff;
		else
			_dogTimer++;

		_lastFrameTime = _scene->_frameStartTime;
	}

	if ((_dogTimer >= 480) && !_dogLoop && !_shopAvailable && (_globals[kDogStatus] == DOG_LEFT) && !_game._player._special) {
		_vm->_sound->command(14);
		_dogLoop = true;
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 10, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 91);
		_dogLoop = false;
		_dogTimer = 0;
	}

	if (_game._trigger == 91) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
		_dogBarking = false;
		_globals[kDogStatus] = DOG_PRESENT;
		_scene->_hotspots.activate(NOUN_OBNOXIOUS_DOG, true);
	}

	if (!_dogEatsRex && (_game._difficulty != DIFFICULTY_EASY) && !_animationActive && (_globals[kDogStatus] == DOG_PRESENT)
	 && !_dogBarking && (_vm->getRandomNumber(1, 50) == 10)) {
		_dogBarking = true;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 5, 8, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
		_scene->_kernelMessages.reset();
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 2, 100);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		_counter = 0;
	}

	if ((_game._trigger == 70) && !_dogEatsRex && (_globals[kDogStatus] == DOG_PRESENT) && !_animationActive) {
		int syncIdx = _globals._sequenceIndexes[1];
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], syncIdx);
		_scene->_kernelMessages.reset();
		_dogBarking = false;
	}

	if (_game._trigger == 100) {
		_counter++;
		_vm->_sound->command(12);

		if ((_counter >= 1) && (_counter <= 4)) {
			Common::Point pos(0, 0);
			switch (_counter) {
			case 1:
				pos = Common::Point(237, 5);
				break;

			case 2:
				pos = Common::Point(270, 15);
				break;

			case 3:
				pos = Common::Point(237, 25);
				break;

			case 4:
				pos = Common::Point(270, 36);
				break;

			default:
				break;
			}
			_scene->_kernelMessages.add(pos, 0xFDFC, 0, 0, 120, _game.getQuote(0x2F9));
		}
	}

	if (_game._player._moving && (_game._difficulty != DIFFICULTY_EASY) && !_shopAvailable && (_globals[kDogStatus] == DOG_PRESENT) && (_scene->_rails.getNext() > 0)) {
		_game._player.cancelCommand();
		_game._player.startWalking(Common::Point(268, 72), FACING_NORTHEAST);
		_scene->_rails.resetNext();
	}

	if ((_game._player._special > 0) && (_game._difficulty != DIFFICULTY_EASY) && (_globals[kDogStatus] == DOG_PRESENT) && _game._player._stepEnabled)
		_game._player._stepEnabled = false;

	if ((_game._difficulty != DIFFICULTY_EASY) && (_globals[kDogStatus] == DOG_PRESENT) && (_game._player._playerPos == Common::Point(268, 72))
	 && (_game._trigger || !_dogEatsRex)) {
		_dogEatsRex = true;
		switch (_game._trigger) {
		case 91:
		case 0:
			_animationActive = 1;
			_game._player._visible = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 10, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -1, 7);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2FA));
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 60);
			_scene->_sequences.addTimer(10, 64);
			break;

		case 60: {
			int syncIdx = _globals._sequenceIndexes[4];
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 5, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 8, 45);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], syncIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
			}
			break;

		case 61: {
			int syncIdx = _globals._sequenceIndexes[4];
			_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 15, 3, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 46, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], syncIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 62);
			}
			break;

		case 62: {
			int syncIdx = _globals._sequenceIndexes[4];
			_animationActive = 2;
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], syncIdx);
			_scene->_sequences.addTimer(60, 63);
			}
			break;

		case 63:
			_vm->_dialogs->show(60729);
			_animationActive = 0;
			_dogEatsRex = false;
			_scene->_reloadSceneFlag = true;
			_game._player._stepEnabled = true;
			break;

		case 64:
			if (_dogEatsRex && (_animationActive == 1)) {
				_vm->_sound->command(12);
				_scene->_sequences.addTimer(10, 64);
			}
			break;

		default:
			break;
		}
	}

	switch (_game._trigger) {
	case 80:
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
		_scene->_sequences.addTimer(6, 81);
		break;

	case 81:
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 82);
		break;

	case 82:
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

void Scene607::handleThrowingBone() {
	_animationActive = -1;
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_game._player._visible = false;
		_scene->loadAnimation(formAnimName('D', _animationMode), 1);
		break;

	case 1:
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;

		if (_animationMode != 1)
			_scene->_hotspots.activate(NOUN_OBNOXIOUS_DOG, false);
		else {
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
		}

		_dogBarking = false;
		if (_game._objects.isInInventory(OBJ_BONE)) {
			_game._objects.setRoom(OBJ_BONE, 1);
			if (_animationMode == 1)
				_globals[kBone202Status] = 0;
		} else {
			_game._objects.setRoom(OBJ_BONES, 1);
			_game._objects.addToInventory(OBJ_BONE);
		}

		_scene->_sequences.addTimer(60, 2);
		break;

	case 2: {
		int quoteId = 0x2F8;
		if (_animationMode == 1)
			quoteId = 0x2F7;

		if (_animationMode == 2) {
			_globals[kDogStatus] = DOG_LEFT;
			_dogTimer = 0;
		}

		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(quoteId));
		_scene->_sequences.addTimer(60, 3);
		}
		break;

	case 3:
		_game._player._stepEnabled = true;
		_animationActive = 0;
		break;

	default:
		break;
	}
}

void Scene607::preActions() {
	if (_action.isAction(VERB_TALKTO, NOUN_OBNOXIOUS_DOG))
		_game._player._needToWalk = false;

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_SIDE_ENTRANCE) && (_globals[kDogStatus] == DOG_LEFT) && (_game._difficulty != DIFFICULTY_EASY)) {
		_shopAvailable = true;
		_dogTimer = 0;
	}

	if (_action.isAction(VERB_THROW, NOUN_BONES, NOUN_OBNOXIOUS_DOG) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_OBNOXIOUS_DOG))
		_game._player.walk(Common::Point(193, 100), FACING_NORTHEAST);

	if (_action.isAction(VERB_THROW, NOUN_BONES, NOUN_FENCE) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_FENCE))
		_game._player.walk(Common::Point(201, 107), FACING_SOUTHEAST);
}

void Scene607::actions() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_SIDE_ENTRANCE))
		_scene->_nextSceneId = 608;
	else if (_action.isAction(VERB_GET_INSIDE, NOUN_CAR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1: {
			int syncIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
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

		case 3: {
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
	} else if (_action.isAction(VERB_THROW, NOUN_BONES, NOUN_OBNOXIOUS_DOG) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_OBNOXIOUS_DOG)) {
		if (_game._difficulty != DIFFICULTY_EASY) {
			_animationMode = 1;
			_scene->_kernelMessages.reset();
			if (_game._trigger == 0)
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2F6));

			handleThrowingBone();
		}
	} else if ((_action.isAction(VERB_THROW, NOUN_BONES, NOUN_FENCE) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_FENCE)) && (_game._difficulty != DIFFICULTY_EASY)
		 && ((_globals[kDogStatus] == DOG_PRESENT) || _game._trigger)) {
		_animationMode = 2;
		if (_game._trigger == 0) {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2F6));
		}
		handleThrowingBone();
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_STREET)) {
		if ((_globals[kDogStatus] == DOG_PRESENT) || (_game._difficulty == DIFFICULTY_EASY))
			_vm->_dialogs->show(60710);
		else
			_vm->_dialogs->show(60711);
	} else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(60712);
	else if (_action.isAction(VERB_LOOK, NOUN_FENCE))
		_vm->_dialogs->show(60713);
	else if (_action.isAction(VERB_LOOK, NOUN_CAR))
		_vm->_dialogs->show(60714);
	else if (_action.isAction(VERB_LOOK, NOUN_MANHOLE))
		_vm->_dialogs->show(60715);
	else if (_action.isAction(VERB_LOOK, NOUN_FIRE_HYDRANT) && (_globals[kDogStatus] == DOG_PRESENT))
		_vm->_dialogs->show(60716);
	else if (_action.isAction(VERB_LOOK, NOUN_SIGN))
		_vm->_dialogs->show(60717);
	else if (_action.isAction(VERB_LOOK, NOUN_BROKEN_WINDOW))
		_vm->_dialogs->show(60718);
	else if (_action.isAction(VERB_LOOK, NOUN_GARAGE_DOOR))
		_vm->_dialogs->show(60719);
	else if (_action.isAction(VERB_LOOK, NOUN_SIDEWALK))
		_vm->_dialogs->show(60720);
	else if (_action.isAction(VERB_LOOK, NOUN_AIR_HOSE))
		_vm->_dialogs->show(60721);
	else if (_action.isAction(VERB_LOOK, NOUN_AUTO_SHOP)) {
		if (_globals[kDogStatus] == DOG_PRESENT)
			_vm->_dialogs->show(60723);
		else
			_vm->_dialogs->show(60722);
	} else if (_action.isAction(VERB_LOOK, NOUN_SIDE_ENTRANCE)) {
		if (_globals[kDogStatus] == DOG_PRESENT)
			_vm->_dialogs->show(60725);
		else
			_vm->_dialogs->show(60724);
	} else if (_action.isAction(VERB_LOOK, NOUN_OBNOXIOUS_DOG))
		_vm->_dialogs->show(60726);
	else if (_action.isAction(VERB_TALKTO, NOUN_OBNOXIOUS_DOG))
		_vm->_dialogs->show(60727);
	else if (_action.isAction(VERB_LOOK, NOUN_BARRICADE))
		_vm->_dialogs->show(60728);
	else if (_action.isAction(VERB_WALK_DOWN, NOUN_STREET))
		_vm->_dialogs->show(60730);
	else if (_action.isObject(NOUN_GARAGE_DOOR) && (_action.isAction(VERB_OPEN) || _action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL)))
		_vm->_dialogs->show(60731);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene608::Scene608(MADSEngine *vm) : Scene6xx(vm) {
	_carMode = -1;
	_carFrame = -1;
	_carMoveMode = -1;
	_dogDeathMode = -1;
	_carHotspotId = -1;
	_barkCount = -1;
	_polycementHotspotId = -1;
	_animationMode = -1;
	_nextTrigger = -1;
	_throwMode = -1;

	_resetPositionsFl = false;
	_dogActiveFl = false;
	_dogBarkingFl = false;
	_dogFirstEncounter = false;
	_rexBeingEaten = false;
	_dogHitWindow = false;
	_checkFl = false;
	_dogSquashFl = false;
	_dogSafeFl = false;
	_buttonPressedonTimeFl = false;
	_dogUnderCar = false;
	_dogYelping = false;

	_dogWindowTimer = -1;
	_dogRunTimer = -1;

	_dogTimer1 = 0;
	_dogTimer2 = 0;
}

void Scene608::synchronize(Common::Serializer &s) {
	Scene6xx::synchronize(s);

	s.syncAsSint16LE(_carMode);
	s.syncAsSint16LE(_carFrame);
	s.syncAsSint16LE(_carMoveMode);
	s.syncAsSint16LE(_dogDeathMode);
	s.syncAsSint16LE(_carHotspotId);
	s.syncAsSint16LE(_barkCount);
	s.syncAsSint16LE(_polycementHotspotId);
	s.syncAsSint16LE(_animationMode);
	s.syncAsSint16LE(_nextTrigger);
	s.syncAsSint16LE(_throwMode);

	s.syncAsByte(_resetPositionsFl);
	s.syncAsByte(_dogActiveFl);
	s.syncAsByte(_dogBarkingFl);
	s.syncAsByte(_dogFirstEncounter);
	s.syncAsByte(_rexBeingEaten);
	s.syncAsByte(_dogHitWindow);
	s.syncAsByte(_checkFl);
	s.syncAsByte(_dogSquashFl);
	s.syncAsByte(_dogSafeFl);
	s.syncAsByte(_buttonPressedonTimeFl);
	s.syncAsByte(_dogUnderCar);
	s.syncAsByte(_dogYelping);

	s.syncAsSint32LE(_dogWindowTimer);
	s.syncAsSint32LE(_dogRunTimer);

	s.syncAsUint32LE(_dogTimer1);
	s.syncAsUint32LE(_dogTimer2);
}

void Scene608::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_POLYCEMENT);
	_scene->addActiveVocab(NOUN_CAR);
	_scene->addActiveVocab(NOUN_OBNOXIOUS_DOG);
}

void Scene608::resetDogVariables() {
	_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
	int idx = _scene->_dynamicHotspots.add(NOUN_OBNOXIOUS_DOG, VERB_WALKTO, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, Common::Point(194, 142), FACING_EAST);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 4);
	_dogBarkingFl = false;
	_dogFirstEncounter = false;
}

void Scene608::restoreAnimations() {
	_scene->freeAnimation();
	_carMode = 0;
	_game._player._stepEnabled = true;
	if (_throwMode == 6)
		_dogSquashFl = true;

	if (_globals[kCarStatus] == CAR_UP) {
		_scene->_sequences.remove(_globals._sequenceIndexes[6]);
		_scene->_sequences.remove(_globals._sequenceIndexes[7]);
		_scene->loadAnimation(formAnimName('A', -1));
	} else {
		_scene->_sequences.remove(_globals._sequenceIndexes[8]);
		_scene->_sequences.remove(_globals._sequenceIndexes[6]);
		_scene->_sequences.remove(_globals._sequenceIndexes[7]);
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(6);
	}
}

void Scene608::setCarAnimations() {
	_scene->freeAnimation();
	if (_globals[kCarStatus] == CAR_UP) {
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[6], Common::Point(143, 98));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 6);
		_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[7], Common::Point(141, 67));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 15);
	} else {
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[6], Common::Point(143, 128));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 6);
		_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[7], Common::Point(141, 97));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 15);
		_globals._sequenceIndexes[8] = _scene->_sequences.startCycle(_globals._spriteIndexes[8], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[8], Common::Point(144, 126));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 5);
	}
}

void Scene608::handleThrowingBone() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		setCarAnimations();
		_scene->_sequences.remove(_globals._sequenceIndexes[5]);
		_animationMode = -1;
		_game._player._visible = false;
		_carMode = _throwMode;
		if (_throwMode == 4)
			_scene->loadAnimation(formAnimName('X', 2), 1);
		else if (_throwMode == 5)
			_scene->loadAnimation(formAnimName('X', 1), 1);
		else
			_scene->loadAnimation(formAnimName('X', 3), 1);
		break;

	case 1:
		_nextTrigger = 1;
		_scene->_sequences.addTimer(1, 2);
		break;

	case 2:
		if (_nextTrigger != 2)
			_scene->_sequences.addTimer(1, 2);
		else {
			if (_game._objects.isInInventory(OBJ_BONE))
				_game._objects.setRoom(OBJ_BONE, 1);
			else {
				_game._objects.setRoom(OBJ_BONES, 1);
				_game._objects.addToInventory(OBJ_BONE);
			}
			_scene->_sequences.addTimer(60, 3);
		}
		break;

	case 3:
		if (_throwMode != 6) {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x304));
			_scene->_sequences.addTimer(120, 4);
		} else
			restoreAnimations();
		break;

	case 4:
		restoreAnimations();
		break;

	default:
		break;
	}
}

void Scene608::enter() {
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMRD_7");
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXMRC_9");
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('c', 2));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('c', 1));

	if (_game._objects.isInRoom(OBJ_POLYCEMENT)) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('g', -1));
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		int idx = _scene->_dynamicHotspots.add(NOUN_POLYCEMENT, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_polycementHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(249, 129), FACING_NORTHEAST);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
	}

	if (_game._objects.isInRoom(OBJ_REARVIEW_MIRROR)) {
		_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('m', -1));
		_globals._sequenceIndexes[12] = _scene->_sequences.startCycle(_globals._spriteIndexes[12], false, 1);
		int idx = _scene->_dynamicHotspots.add(NOUN_REARVIEW_MIRROR, VERB_WALKTO, _globals._sequenceIndexes[12], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(71, 113), FACING_NORTHEAST);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 15);
	}

	if (_game._difficulty == DIFFICULTY_HARD) {
		_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('g', 0));
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('g', 1));
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('g', 2));
		_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('h', 2));
		_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('h', 3));
		_rexBeingEaten = false;

		if (!_game._visitedScenes._sceneRevisited) {
			_globals[kDogStatus] = DOG_GONE;
			_dogActiveFl = true;
		} else
			_dogActiveFl = (_globals[kDogStatus] != DOG_DEAD);
	} else {
		_globals[kDogStatus] = DOG_DEAD;
		_dogActiveFl = false;
	}

	_dogSquashFl = false;
	_buttonPressedonTimeFl = false;
	_dogWindowTimer = 0;
	_dogRunTimer = 0;
	_dogHitWindow = false;
	_checkFl = false;
	_dogUnderCar = false;
	_dogYelping = false;


	if (!_game._visitedScenes._sceneRevisited)
		_globals[kCarStatus] = CAR_UP;

	_animationMode = 0;
	_carMoveMode = 0;
	_carFrame = -1;

	if (_globals[kCarStatus] == CAR_UP) {
		_carMode = 0;
		_dogDeathMode = 0;
		_resetPositionsFl = false;
		int idx = _scene->_dynamicHotspots.add(NOUN_CAR, VERB_WALKTO, -1, Common::Rect(99, 69, 99 + 82, 69 + 25));
		_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('A', -1));
	} else if (_globals[kCarStatus] == CAR_DOWN) {
		_carMode = 0;
		_dogDeathMode = 0;
		_resetPositionsFl = false;
		int idx = _scene->_dynamicHotspots.add(NOUN_CAR, VERB_WALKTO, -1, Common::Rect(100, 100, 100 + 82, 100 + 25));
		_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(6);
	} else if (_globals[kCarStatus] == CAR_SQUASHES_DOG) {
		_carMode = 2;
		_dogDeathMode = 0;
		_resetPositionsFl = false;
		int idx = _scene->_dynamicHotspots.add(NOUN_CAR, VERB_WALKTO, -1, Common::Rect(99, 69, 99 + 82, 69 + 25));
		_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('C', -1));
	} else if (_globals[kCarStatus] == CAR_SQUASHES_DOG_AGAIN) {
		_carMode = 1;
		_dogDeathMode = 2;
		_resetPositionsFl = true;
		int idx = _scene->_dynamicHotspots.add(NOUN_CAR, VERB_WALKTO, -1, Common::Rect(99, 69, 99 + 82, 69 + 25));
		_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('B', -1));
	} else {
		_carMode = 3;
		_dogDeathMode = 2;
		_resetPositionsFl = true;
		int idx = _scene->_dynamicHotspots.add(NOUN_CAR, VERB_WALKTO, -1, Common::Rect(100, 100, 100 + 82, 100 + 25));
		_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('D', -1));
	}

	_vm->_palette->setEntry(252, 63, 44, 30);
	_vm->_palette->setEntry(253, 63, 20, 22);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(46, 132);
		_game._player._facing = FACING_EAST;
		if (_game._difficulty == DIFFICULTY_HARD) {
			if (!_game._visitedScenes._sceneRevisited)
				_dogFirstEncounter = true;
			else if (_dogActiveFl)
				resetDogVariables();
		}
	} else if ((_game._difficulty == DIFFICULTY_HARD) && !_dogFirstEncounter && _dogActiveFl) {
		if (!_dogUnderCar)
			resetDogVariables();
		else {
			_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 9, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 10, 11);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 6);
		}
	}

	sceneEntrySound();

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_BONES);

	_game.loadQuoteSet(0x2FB, 0x2FC, 0x2FE, 0x2FD, 0x2FF, 0x300, 0x301, 0x302, 0x303, 0x304, 0);
}

void Scene608::step() {
	if (_dogFirstEncounter) {
		long diff = _scene->_frameStartTime - _dogTimer1;
		if ((diff >= 0) && (diff <= 1))
			_dogWindowTimer += diff;
		else
			_dogWindowTimer++;

		_dogTimer1 = _scene->_frameStartTime;
	}

	if (_dogActiveFl && (_dogWindowTimer >= 2) && !_dogHitWindow) {
		_dogHitWindow = true;
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 11, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
		_vm->_sound->command(14);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		_dogWindowTimer = 0;
	}

	if (_game._trigger == 70)
		resetDogVariables();

	if ((_game._difficulty == DIFFICULTY_HARD) && !_animationMode && _dogActiveFl && !_dogFirstEncounter && !_dogUnderCar) {
		if (!_dogBarkingFl) {
			if (_vm->getRandomNumber(1, 50) == 10) {
				_dogBarkingFl = true;
				_scene->_sequences.remove(_globals._sequenceIndexes[5]);
				_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], false, 5, 8, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 4);
				int idx = _scene->_dynamicHotspots.add(NOUN_OBNOXIOUS_DOG, VERB_WALKTO, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots.setPosition(idx, Common::Point(194, 142), FACING_EAST);
				_barkCount = 0;
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 2, 100);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 60);
			}
		} else if (_game._trigger == 60) {
			int syncIdx = _globals._sequenceIndexes[5];
			_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 6);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], syncIdx);
			_scene->_kernelMessages.reset();
			_dogBarkingFl = false;
		}
	}

	if ((_game._trigger == 100) && _dogBarkingFl) {
		_vm->_sound->command(12);
		_barkCount++;

		if ((_barkCount >= 1) && (_barkCount <= 4)) {
			Common::Point _barkPos(0, 0);
			switch (_barkCount) {
			case 1:
				_barkPos = Common::Point(197, 66);
				break;

			case 2:
				_barkPos = Common::Point(230, 76);
				break;

			case 3:
				_barkPos = Common::Point(197, 86);
				break;

			case 4:
				_barkPos = Common::Point(230, 97);
				break;

			default:
				break;
			}
			_scene->_kernelMessages.add(_barkPos, 0xFDFC, 0, 0, 120, _game.getQuote(0x2FB));
		}
	}

	if (_dogSquashFl && !_dogFirstEncounter && _dogUnderCar && _dogActiveFl) {
		long diff = _scene->_frameStartTime - _dogTimer2;
		if ((diff >= 0) && (diff <= 4))
			_dogRunTimer += diff;
		else
			_dogRunTimer++;

		_dogTimer2 = _scene->_frameStartTime;
	}

	if (_dogRunTimer >= 480 && !_checkFl && !_buttonPressedonTimeFl && !_dogFirstEncounter && _dogUnderCar && _dogActiveFl) {
		_checkFl = true;
		_dogSquashFl = false;
		_dogSafeFl = true;
		_checkFl = false;
		_dogRunTimer = 0;
	} else {
		_dogSafeFl = false;
		if (_game._player._moving && (_game._difficulty == DIFFICULTY_HARD) && _dogActiveFl && (_scene->_rails.getNext() > 0) && _dogUnderCar)
			_dogSafeFl = true;
	}

	if (_dogActiveFl && _dogSafeFl && !_buttonPressedonTimeFl) {
		_dogDeathMode = 0;
		_globals[kCarStatus] = CAR_UP;
		_carMode = 0;
		_scene->_sequences.remove(_globals._sequenceIndexes[10]);
		_dogUnderCar = false;
		_dogYelping = false;
		_scene->_kernelMessages.reset();
		_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 92);
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2FF));
	}

	if (_game._trigger == 92) {
		resetDogVariables();
		_animationMode = 0;
	}

	if ((_carMode == 4) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_animation[0]->getCurrentFrame();

			if (_carFrame == 10) {
				_game._player._visible = true;
				_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
			} else if (_carFrame == 56) {
				resetDogVariables();
				_animationMode = 0;
				_nextTrigger = 2;
			}
		}
	}

	if ((_carMode == 5) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_animation[0]->getCurrentFrame();
			if (_carFrame == 10) {
				_game._player._visible = true;
				_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
			} else if (_carFrame == 52) {
				resetDogVariables();
				_animationMode = 0;
				_nextTrigger = 2;
			}
		}
	}

	if ((_carMode == 6) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_animation[0]->getCurrentFrame();

			if (_carFrame == 11) {
				_game._player._visible = true;
				_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
			} else if (_carFrame == 41) {
				_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 9, 0, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 10, 11);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 6);
				_dogUnderCar = true;
				_nextTrigger = 2;
			}
		}
	}

	if (_dogUnderCar) {
		if (!_dogYelping) {
			if (_vm->getRandomNumber(1, 50) == 10) {
				_dogYelping = true;
				_barkCount = 0;
				_scene->_sequences.addTimer(12, 110);
				_scene->_sequences.addTimer(22, 111);
				_scene->_sequences.addTimer(120, 112);
			}
			_scene->_kernelMessages.reset();
		}
	} else
		_dogYelping = false;

	if (_game._trigger == 110) {
		_vm->_sound->command(12);
		_scene->_kernelMessages.add(Common::Point(150, 97), 0xFDFC, 0, 0, 60, _game.getQuote(0x303));
	}

	if (_game._trigger == 111) {
		_vm->_sound->command(12);
		_scene->_kernelMessages.add(Common::Point(183, 93), 0xFDFC, 0, 0, 60, _game.getQuote(0x303));
	}

	if (_game._trigger == 112)
		_dogYelping = false;

	if ((_carMode == 0) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			if ((_globals[kCarStatus] == CAR_UP) || (_globals[kCarStatus] == CAR_DOWN)) {
				switch (_carMoveMode) {
				case 0:
					if (_globals[kCarStatus] == CAR_UP)
						nextFrame = 0;
					else
						nextFrame = 6;
					break;

				case 1:
					if (_scene->_animation[0]->getCurrentFrame() >= 12) {
						nextFrame = 0;
						_carMoveMode = 0;
						_globals[kCarStatus] = CAR_UP;
					}
					break;

				case 2:
					if (_scene->_animation[0]->getCurrentFrame() >= 6) {
						nextFrame = 6;
						_carMoveMode = 0;
						_globals[kCarStatus] = CAR_DOWN;
					}
					break;

				default:
					break;
				}
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				_carFrame = nextFrame;
			}
		}
	}

	if ((_carMode == 2) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			if (_carMoveMode == 0)
				nextFrame = 28;
			else if (_scene->_animation[0]->getCurrentFrame() >= 28) {
				nextFrame = 28;
				_carMoveMode = 0;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				_carFrame = nextFrame;
			}
		}
	}

	if ((_carMode == 3) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			if (_resetPositionsFl) {
				nextFrame = 0;
				_carMoveMode = 0;
			} else if (_carMoveMode == 0)
				nextFrame = 6;
			else if (_scene->_animation[0]->getCurrentFrame() >= 6) {
				nextFrame = 6;
				_carMoveMode = 0;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				_carFrame = nextFrame;
			}
		}
	}


	if ((_carMode == 1) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			if (_resetPositionsFl) {
				nextFrame = 0;
				_carMoveMode = 0;
			} else if (_carMoveMode == 0)
				nextFrame = 6;
			else if (_scene->_animation[0]->getCurrentFrame() >= 6) {
				nextFrame = 6;
				_carMoveMode = 0;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				_carFrame = nextFrame;
			}
		}
	}

	if (_game._player._moving && (_game._difficulty == DIFFICULTY_HARD) && _dogActiveFl && (_scene->_rails.getNext() > 0)) {
		_game._player.cancelCommand();
		_game._player.startWalking(Common::Point(194, 142), FACING_EAST);
		_scene->_rails.resetNext();
		if (_dogUnderCar)
			_dogSafeFl = true;
	}

	if (_game._player._special > 0 && (_game._difficulty == DIFFICULTY_HARD) && _dogActiveFl && _game._player._stepEnabled)
		_game._player._stepEnabled = false;

	if ((_game._difficulty == DIFFICULTY_HARD) && _dogActiveFl && (_game._player._playerPos == Common::Point(194, 142))
	 && (_game._trigger || !_rexBeingEaten)) {
		_rexBeingEaten = true;
		switch (_game._trigger) {
		case 0:
			_scene->_sequences.remove(_globals._sequenceIndexes[5]);
			_animationMode = 1;
			_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 10, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], 1, 2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 5);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
			_scene->_sequences.addTimer(10, 85);
			break;

		case 80:
			_game._player._visible = false;
			_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 10, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], 3, 5);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 5);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[9], -1);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2FC));
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
			break;

		case 81: {
			int syncIdx = _globals._sequenceIndexes[9];
			_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 5, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], 6, 38);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 5);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[9], syncIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 82);
			}
			break;

		case 82: {
			int syncIdx = _globals._sequenceIndexes[9];
			_globals._sequenceIndexes[9] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[9], false, 15, 5, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], 39, 40);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 5);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[9], syncIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 83);
			}
			break;

		case 83: {
			_animationMode = 2;
			int syncIdx = _globals._sequenceIndexes[9];
			_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 5);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[9], syncIdx);
			_scene->_sequences.addTimer(60, 84);
			}
			break;

		case 84:
			_rexBeingEaten = false;
			_animationMode = 0;
			_scene->_reloadSceneFlag = true;
			_game._player._stepEnabled = true;
			break;

		case 85:
			if (_rexBeingEaten && (_animationMode == 1)) {
				_vm->_sound->command(12);
				_scene->_sequences.addTimer(10, 85);
			}
			break;

		default:
			break;
		}
	}
}

void Scene608::preActions() {
	_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;

	if ((_action.isAction(VERB_THROW, NOUN_BONE, NOUN_REAR_OF_GARAGE) || _action.isAction(VERB_THROW, NOUN_BONES, NOUN_REAR_OF_GARAGE)
		|| _action.isAction(VERB_THROW, NOUN_BONE, NOUN_FRONT_OF_GARAGE) || _action.isAction(VERB_THROW, NOUN_BONES, NOUN_FRONT_OF_GARAGE)
		|| _action.isAction(VERB_THROW, NOUN_BONES, NOUN_OBNOXIOUS_DOG) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_OBNOXIOUS_DOG)) && _dogActiveFl) {
		_game._player._stepEnabled = false;
		_game._player.walk(Common::Point(56, 146), FACING_EAST);
	}

	if ((_action.isAction(VERB_THROW, NOUN_BONES, NOUN_AREA_BEHIND_CAR) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_AREA_BEHIND_CAR)
		|| _action.isAction(VERB_THROW, NOUN_BONES, NOUN_DANGER_ZONE) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_DANGER_ZONE)) && _dogActiveFl) {
		_game._player._stepEnabled = false;
		_game._player.walk(Common::Point(75, 136), FACING_EAST);
	}

	if (_action.isAction(VERB_PUSH, NOUN_DOWN_BUTTON) && _dogUnderCar) {
		_buttonPressedonTimeFl = true;
		_dogDeathMode = 1;
	} else
		_buttonPressedonTimeFl = false;
}

void Scene608::actions() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOORWAY))
		_scene->_nextSceneId = 607;
	else if (_action.isAction(VERB_PUSH, NOUN_DOWN_BUTTON)) {
		_game._player._stepEnabled = true;
		switch (_game._trigger) {
		case 0:
			if ((_globals[kCarStatus] == CAR_UP) || (_globals[kCarStatus] == CAR_SQUASHES_DOG) || (_globals[kCarStatus] == CAR_SQUASHES_DOG_AGAIN)) {
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], true, 6, 2, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 2);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			} else
				_vm->_dialogs->show(60839);
			break;

		case 1:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);
			_game._player._visible = true;
			if (_dogDeathMode == 0)
				_carMode = 0;
			else if (_dogDeathMode == 1) {
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x300));
				_globals[kCarStatus] = CAR_SQUASHES_DOG;
				_carMode = 2;
				_globals[kDogStatus] = DOG_DEAD;
				_dogActiveFl = false;
				_dogUnderCar = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[10]);
				_scene->freeAnimation();
				_scene->loadAnimation(formAnimName('C', -1));
			} else {
				_resetPositionsFl = false;
				_carMode = 1;
				_scene->freeAnimation();
				_scene->loadAnimation(formAnimName('B', -1));
			}

			_carMoveMode = 2;
			_scene->_sequences.addTimer(1, 2);
			break;

		case 2:
			if (_carMoveMode != 0)
				_scene->_sequences.addTimer(1, 2);
			else {
				_scene->_dynamicHotspots.remove(_carHotspotId);
				int idx = _scene->_dynamicHotspots.add(NOUN_CAR, VERB_WALKTO, -1, Common::Rect(100, 100, 100 + 82, 100 + 25));
				_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
				if (_globals[kCarStatus] == CAR_SQUASHES_DOG)
					_scene->_sequences.addTimer(120, 3);
				else {
					if (_dogDeathMode == 0)
						_globals[kCarStatus] = CAR_DOWN;
					else {
						_globals[kCarStatus] = CAR_DOWN_ON_SQUASHED_DOG;
						_carMode = 3;
						_dogDeathMode = 2;
					}
					_game._player._stepEnabled = true;
				}
			}
			break;

		case 3:
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x302));
			_globals[kCarStatus] = CAR_DOWN_ON_SQUASHED_DOG;
			_carMode = 3;
			_dogDeathMode = 2;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PUSH, NOUN_UP_BUTTON)) {
		switch (_game._trigger) {
		case 0:
			if ((_globals[kCarStatus] == CAR_DOWN) || (_globals[kCarStatus] == CAR_DOWN_ON_SQUASHED_DOG)) {
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], true, 6, 2, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 3);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			} else
				_vm->_dialogs->show(60840);
			break;

		case 1:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);
			_game._player._visible = true;
			if (_dogDeathMode == 0)
				_carMode = 0;
			else {
				_carMode = 3;
				_resetPositionsFl = false;
				_scene->freeAnimation();
				_scene->loadAnimation(formAnimName('D', -1));
			}
			_carMoveMode = 1;
			_scene->_sequences.addTimer(1, 2);
			break;

		case 2: {
			if (_carMoveMode != 0)
				_scene->_sequences.addTimer(1, 2);
			else if (_dogDeathMode == 0)
				_globals[kCarStatus] = CAR_UP;
			else if (_dogDeathMode == 2) {
				_globals[kCarStatus] = CAR_SQUASHES_DOG_AGAIN;
				_carMode = 3;
				_dogDeathMode = 2;
			}
			_scene->_dynamicHotspots.remove(_carHotspotId);
			int idx = _scene->_dynamicHotspots.add(NOUN_CAR, VERB_WALKTO, -1, Common::Rect(99, 69, 99 + 82, 69 + 25));
			_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
			_game._player._stepEnabled = true;
			}
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_THROW, NOUN_BONE, NOUN_REAR_OF_GARAGE) || _action.isAction(VERB_THROW, NOUN_BONES, NOUN_REAR_OF_GARAGE)
					|| _action.isAction(VERB_THROW, NOUN_BONES, NOUN_OBNOXIOUS_DOG) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_OBNOXIOUS_DOG)) {
		_game._player._stepEnabled = true;
		if (_dogActiveFl) {
			if (_game._trigger == 0) {
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2FE));
			}
			_throwMode = 4;
			handleThrowingBone();
		} else
			_vm->_dialogs->show(60841);
	} else if (_action.isAction(VERB_THROW, NOUN_BONE, NOUN_FRONT_OF_GARAGE) || _action.isAction(VERB_THROW, NOUN_BONES, NOUN_FRONT_OF_GARAGE)) {
		_game._player._stepEnabled = true;
		if (_dogActiveFl) {
			if (_game._trigger == 0) {
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2FD));
			}
			_throwMode = 5;
			handleThrowingBone();
		} else
			_vm->_dialogs->show(60841);
	} else if (_action.isAction(VERB_THROW, NOUN_BONES, NOUN_AREA_BEHIND_CAR) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_AREA_BEHIND_CAR)
					|| _action.isAction(VERB_THROW, NOUN_BONES, NOUN_DANGER_ZONE) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_DANGER_ZONE)) {
		_game._player._stepEnabled = true;
		if ((_globals[kCarStatus] == CAR_UP) && _dogActiveFl) {
			if (_dogActiveFl) {
				if (_game._trigger == 0) {
					_scene->_kernelMessages.reset();
					_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x301));
				}
				_throwMode = 6;
				handleThrowingBone();
			} else
				_vm->_dialogs->show(60841);
		} else
			_vm->_dialogs->show(60842);
	} else if (_action.isAction(VERB_TAKE, NOUN_POLYCEMENT) && (_game._trigger || !_game._objects.isInInventory(OBJ_POLYCEMENT))) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], true, 6, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_vm->_sound->command(9);
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_scene->_dynamicHotspots.remove(_polycementHotspotId);
			break;

		case 2:
			_game._objects.addToInventory(OBJ_POLYCEMENT);
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_vm->_dialogs->showItem(OBJ_POLYCEMENT, 60833);
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_REARVIEW_MIRROR) && (_game._trigger || !_game._objects.isInInventory(OBJ_REARVIEW_MIRROR))) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], true, 6, 2, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 2, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_vm->_sound->command(9);
			_scene->_sequences.remove(_globals._sequenceIndexes[12]);
			_game._objects.addToInventory(OBJ_REARVIEW_MIRROR);
			_vm->_dialogs->showItem(OBJ_REARVIEW_MIRROR, 60827);
			break;

		case 2:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action._lookFlag) {
		if (_game._difficulty != DIFFICULTY_HARD)
			_vm->_dialogs->show(60810);
		else if (_globals[kDogStatus] == DOG_DEAD)
			_vm->_dialogs->show(60812);
		else
			_vm->_dialogs->show(60811);
	} else if (_action.isAction(VERB_LOOK) && (_action.isObject(NOUN_MUFFLER) || _action.isObject(NOUN_CAR_SEAT) || _action.isObject(NOUN_HUBCAP)
					|| _action.isObject(NOUN_COILS) || _action.isObject(NOUN_QUARTER_PANEL)))
		_vm->_dialogs->show(60813);
	else if (_action.isAction(VERB_TAKE) && (_action.isObject(NOUN_MUFFLER) || _action.isObject(NOUN_CAR_SEAT) || _action.isObject(NOUN_HUBCAP)
					|| _action.isObject(NOUN_COILS) || _action.isObject(NOUN_QUARTER_PANEL)))
		_vm->_dialogs->show(60814);
	else if (_action.isAction(VERB_LOOK, NOUN_GARAGE_FLOOR) || _action.isAction(VERB_LOOK, NOUN_FRONT_OF_GARAGE) || _action.isAction(VERB_LOOK, NOUN_REAR_OF_GARAGE)) {
		if (_dogActiveFl)
			_vm->_dialogs->show(60815);
		else
			_vm->_dialogs->show(60816);
	} else if (_action.isAction(VERB_LOOK, NOUN_SPARE_RIBS))
		_vm->_dialogs->show(60817);
	else if (_action.isAction(VERB_TAKE, NOUN_SPARE_RIBS)) {
		if (_game._difficulty == DIFFICULTY_HARD)
			_vm->_dialogs->show(60818);
		else
			_vm->_dialogs->show(60819);
	} else if (_action.isAction(VERB_LOOK, NOUN_UP_BUTTON))
		_vm->_dialogs->show(60820);
	else if (_action.isAction(VERB_LOOK, NOUN_DOWN_BUTTON))
		_vm->_dialogs->show(60821);
	else if (_action.isAction(VERB_LOOK, NOUN_TRASH_CAN))
		_vm->_dialogs->show(60822);
	else if (_action.isAction(VERB_LOOK, NOUN_CALENDAR))
		_vm->_dialogs->show(60823);
	else if (_action.isAction(VERB_LOOK, NOUN_STORAGE_BOX)) {
		if (_game._objects[OBJ_REARVIEW_MIRROR]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60825);
		else
			_vm->_dialogs->show(60824);
	} else if (_action.isAction(VERB_OPEN, NOUN_STORAGE_BOX))
		_vm->_dialogs->show(60826);
	else if (_action.isAction(VERB_LOOK, NOUN_REARVIEW_MIRROR) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(60828);
	else if (_action.isAction(VERB_LOOK, NOUN_TOOL_BOX)) {
		if (_game._objects[OBJ_POLYCEMENT]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60829);
		else
			_vm->_dialogs->show(60830);
	} else if (_action.isAction(VERB_OPEN, NOUN_TOOL_BOX))
		_vm->_dialogs->show(60831);
	else if ((_action.isAction(VERB_LOOK, NOUN_POLYCEMENT)) && (_game._objects.isInRoom(OBJ_POLYCEMENT)))
		_vm->_dialogs->show(60832);
	else if (_action.isAction(VERB_LOOK, NOUN_GREASE_CAN) || _action.isAction(VERB_LOOK, NOUN_OIL_CAN))
		_vm->_dialogs->show(60834);
	else if (_action.isAction(VERB_LOOK, NOUN_CAR_LIFT))
		_vm->_dialogs->show(60835);
	else if (_action.isAction(VERB_LOOK, NOUN_CHAIR) || _action.isAction(VERB_LOOK, NOUN_HAT))
		_vm->_dialogs->show(60836);
	else if (_action.isAction(VERB_LOOK, NOUN_DANGER_ZONE))
		_vm->_dialogs->show(60838);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene609::Scene609(MADSEngine *vm) : Scene6xx(vm) {
	_videoDoorMode = -1;
}

void Scene609::synchronize(Common::Serializer &s) {
	Scene6xx::synchronize(s);

	s.syncAsSint16LE(_videoDoorMode);
}

void Scene609::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene609::enter() {
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

	sceneEntrySound();
	_game.loadQuoteSet(0x305, 0x306, 0x307, 0x308, 0x309, 0);
}

void Scene609::step() {
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
		_scene->_sequences.remove( _globals._sequenceIndexes[2]);
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

void Scene609::enterStore() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		if (_videoDoorMode == 2)
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
		if (_videoDoorMode == 1) {
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

void Scene609::preActions() {
	if (_action.isAction(VERB_UNLOCK, NOUN_DOOR_KEY, NOUN_VIDEO_STORE_DOOR))
		_game._player.walk(Common::Point(78, 99), FACING_NORTHWEST);
}

void Scene609::actions() {
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
			_videoDoorMode = 2;
			enterStore();
		}
	} else if (_action.isAction(VERB_UNLOCK, NOUN_DOOR_KEY, NOUN_VIDEO_STORE_DOOR)) {
		_videoDoorMode = 1;
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

		case 1: {
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

		case 3: {
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

/*------------------------------------------------------------------------*/

Scene610::Scene610(MADSEngine *vm) : Scene6xx(vm) {
	_handsetHotspotId = -1;
	_checkVal = -1;

	_cellCharging = false;

	_cellChargingTimer = -1;
	_lastFrameTimer = 0;
}

void Scene610::synchronize(Common::Serializer &s) {
	Scene6xx::synchronize(s);

	s.syncAsSint16LE(_handsetHotspotId);
	s.syncAsSint16LE(_checkVal);

	s.syncAsByte(_cellCharging);

	s.syncAsSint32LE(_cellChargingTimer);
	s.syncAsUint32LE(_lastFrameTimer);
}

void Scene610::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_PHONE_HANDSET);
	_scene->addActiveVocab(VERB_WALKTO);
}

void Scene610::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('p', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMRC_9");
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 1));

	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 60, 0, 0, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 13);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 30, 0, 0, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);

	if (!_game._visitedScenes._sceneRevisited)
		_cellCharging = false;

	if (_game._objects[OBJ_PHONE_HANDSET]._roomNumber == _scene->_currentSceneId) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
		_handsetHotspotId = _scene->_dynamicHotspots.add(NOUN_PHONE_HANDSET, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_handsetHotspotId, Common::Point(132, 121), FACING_NORTHWEST);
		if ((_globals[kHandsetCellStatus] == 2) && (_game._difficulty == DIFFICULTY_HARD) && !_globals[kDurafailRecharged])
			_globals[kHandsetCellStatus] = 1;
	}

	if (_scene->_roomChanged && _game._difficulty != DIFFICULTY_EASY)
		_game._objects.addToInventory(OBJ_PENLIGHT);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(175, 152);
		_game._player._facing = FACING_NORTHWEST;
	}

	sceneEntrySound();
}

void Scene610::step() {
	if (_cellCharging) {
		long diff = _scene->_frameStartTime - _lastFrameTimer;
		if ((diff >= 0) && (diff <= 60))
			_cellChargingTimer += diff;
		else
			_cellChargingTimer++;

		_lastFrameTimer = _scene->_frameStartTime;
	}

	// CHECKME: _checkVal is always false, could be removed
	if ((_cellChargingTimer >= 60) && !_checkVal) {
		_checkVal = true;
		_globals[kHandsetCellStatus] = 1;
		_cellCharging = false;
		_checkVal = false;
		_cellChargingTimer = 0;
	}
}

void Scene610::actions() {
	if (_action.isAction(VERB_EXIT_FROM, NOUN_VIDEO_STORE))
		_scene->_nextSceneId = 609;
	else if (_action.isAction(VERB_TAKE, NOUN_PHONE_HANDSET)) {
		if ( _game._trigger || !_game._objects.isInInventory(OBJ_PHONE_HANDSET)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], true, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 2, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				_vm->_sound->command(9);
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_scene->_dynamicHotspots.remove(_handsetHotspotId);
				_game._objects.addToInventory(OBJ_PHONE_HANDSET);
				_vm->_dialogs->showItem(OBJ_PHONE_HANDSET, 61017);
				break;

			case 2:
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
	} else if (_action.isAction(VERB_PUT, NOUN_PHONE_HANDSET, NOUN_PHONE_CRADLE)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], true, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 2, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
			_handsetHotspotId = _scene->_dynamicHotspots.add(NOUN_PHONE_HANDSET, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(_handsetHotspotId, Common::Point(132, 121), FACING_NORTHWEST);
			_game._objects.setRoom(OBJ_PHONE_HANDSET, _scene->_currentSceneId);
			break;

		case 2:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			if ((_globals[kHandsetCellStatus] == 2) && (_game._difficulty == DIFFICULTY_HARD) && !_globals[kDurafailRecharged])
				_cellCharging = true;

			_vm->_dialogs->show(61032);
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_PIPPY_BILLBOARD))
		_vm->_dialogs->show(61010);
	else if (_action.isAction(VERB_LOOK, NOUN_CIVILIZATION_AD))
		_vm->_dialogs->show(61011);
	else if (_action.isAction(VERB_LOOK, NOUN_MARX_BROS_POSTER))
		_vm->_dialogs->show(61012);
	else if (_action.isAction(VERB_LOOK, NOUN_VIDEO_MONITOR))
		_vm->_dialogs->show(61013);
	else if (_action.isAction(VERB_LOOK, NOUN_VIDEO_STORE))
		_vm->_dialogs->show(61014);
	else if (_action._lookFlag)
		_vm->_dialogs->show(61015);
	else if (_action.isAction(VERB_LOOK, NOUN_LOGO))
		_vm->_dialogs->show(61018);
	else if (_action.isAction(VERB_LOOK, NOUN_CEMENT)) {
		if (_game._visitedScenes.exists(601))
			_vm->_dialogs->show(61020);
		else
			_vm->_dialogs->show(61019);
	} else if (_action.isAction(VERB_LOOK, NOUN_COUNTER))
		_vm->_dialogs->show(61021);
	else if (_action.isAction(VERB_LOOK, NOUN_PHONE_ANTENNA))
		_vm->_dialogs->show(61022);
	else if (_action.isAction(VERB_LOOK, NOUN_SMELLY_SNEAKER))
		_vm->_dialogs->show(61023);
	else if (_action.isAction(VERB_TAKE, NOUN_SMELLY_SNEAKER))
		_vm->_dialogs->show(61024);
	else if (_action.isAction(VERB_LOOK, NOUN_SPOTLIGHT))
		_vm->_dialogs->show(61025);
	else if (_action.isAction(VERB_LOOK, NOUN_PHONE_HANDSET) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(61026);
	else if (_action.isAction(VERB_LOOK, NOUN_PHONE_CRADLE))
		_vm->_dialogs->show(61027);
	else if (_action.isAction(VERB_LOOK, NOUN_RETURN_SLOT))
		_vm->_dialogs->show(61028);
	else if (_action.isAction(VERB_PUT, NOUN_RETURN_SLOT)
		&& _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId)))
		_vm->_dialogs->show(61029);
	else if ( _action.isObject(NOUN_CLASSIC_VIDEOS) || _action.isObject(NOUN_MORE_CLASSIC_VIDEOS) || _action.isObject(NOUN_DRAMA_VIDEOS)
		|| _action.isObject(NOUN_NEW_RELEASE_VIDEOS) || _action.isObject(NOUN_PORNO_VIDEOS) || _action.isObject(NOUN_EDUCATIONAL_VIDEOS)
		|| _action.isObject(NOUN_INSTRUCTIONAL_VIDEOS) || _action.isObject(NOUN_WORKOUT_VIDEOS) || _action.isObject(NOUN_FOREIGN_VIDEOS)
		|| _action.isObject(NOUN_ADVENTURE_VIDEOS) || _action.isObject(NOUN_COMEDY_VIDEOS)) {
		if (_action.isAction(VERB_LOOK))
			_vm->_dialogs->show(61030);
		else if (_action.isAction(VERB_TAKE))
			_vm->_dialogs->show(61031);
		else
			return;
	} else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene611::Scene611(MADSEngine *vm) : Scene6xx(vm), _defaultDialogPos(0, 0) {
	_seenRatFl = false;
	_eyesRunningFl = false;
	_shouldRemoveEyes = false;
	_ratPresentFl = false;
	_duringDialogFl = false;
	_resetBatterieText = false;
	_hermitTalkingFl = false;
	_hermitMovingFl = false;
	_alreadyTalkingFl = false;
	_giveBatteriesFl = false;
	_startTradingFl = false;
	_check1Fl = false;
	_stickFingerFl = false;

	_randVal = -1;
	_ratHotspotId = -1;
	_hermitDialogNode = -1;
	_hermitDisplayedQuestion = -1;
	_nextFrame = -1;
	_hermitMode = -1;

	_ratTimer = 0;
}

void Scene611::synchronize(Common::Serializer &s) {
	Scene6xx::synchronize(s);

	s.syncAsByte(_seenRatFl);
	s.syncAsByte(_eyesRunningFl);
	s.syncAsByte(_shouldRemoveEyes);
	s.syncAsByte(_ratPresentFl);
	s.syncAsByte(_duringDialogFl);
	s.syncAsByte(_resetBatterieText);
	s.syncAsByte(_hermitTalkingFl);
	s.syncAsByte(_hermitMovingFl);
	s.syncAsByte(_alreadyTalkingFl);
	s.syncAsByte(_giveBatteriesFl);
	s.syncAsByte(_startTradingFl);
	s.syncAsByte(_check1Fl);
	s.syncAsByte(_stickFingerFl);

	s.syncAsSint16LE(_randVal);
	s.syncAsSint16LE(_ratHotspotId);
	s.syncAsSint16LE(_hermitDialogNode);
	s.syncAsSint16LE(_hermitDisplayedQuestion);
	s.syncAsSint16LE(_nextFrame);
	s.syncAsSint16LE(_hermitMode);

	s.syncAsUint32LE(_ratTimer);

	s.syncAsSint16LE(_defaultDialogPos.x);
	s.syncAsSint16LE(_defaultDialogPos.y);
}

void Scene611::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_RAT);
	_scene->addActiveVocab(VERB_WALKTO);
}

void Scene611::handleRatMoves() {
	_ratPresentFl = false;
	_scene->_sequences.remove(_globals._sequenceIndexes[1]);
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 12, 1, 0, 0);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 11, -2);
	_ratTimer = _game._player._priorTimer;
	_scene->_dynamicHotspots.remove(_ratHotspotId);
}

void Scene611::handleTrading() {
	if (_game._objects.isInInventory(OBJ_DURAFAIL_CELLS))
		_game._objects.setRoom(OBJ_DURAFAIL_CELLS, 1);

	if (_game._objects.isInInventory(OBJ_PHONE_CELLS))
		_game._objects.setRoom(OBJ_PHONE_CELLS, 1);

	_game._objects.addToInventory(OBJ_FAKE_ID);
}

void Scene611::setDialogNode(int node) {
	if (node > 0)
		_hermitDialogNode = node;

	_game._player._stepEnabled = true;

	switch (node) {
	case 0:
		_scene->_userInterface.setup(kInputBuildingSentences);
		_duringDialogFl = false;
		_hermitDialogNode = 0;
		break;

	case 1:
		_dialog1.start();
		_duringDialogFl = true;
		_hermitDialogNode = 1;
		break;

	case 2:
		_dialog2.start();
		_duringDialogFl = true;
		_hermitDialogNode = 2;
		break;

	default:
		break;
	}
}

bool Scene611::check2ChargedBatteries() {
	if ((_game._objects.isInInventory(OBJ_DURAFAIL_CELLS) && !_game._objects.isInInventory(OBJ_PHONE_CELLS))
		|| (!_game._objects.isInInventory(OBJ_DURAFAIL_CELLS) && _game._objects.isInInventory(OBJ_PHONE_CELLS)))
		return true;

	return false;
}

bool Scene611::check4ChargedBatteries() {
	if (_game._objects.isInInventory(OBJ_DURAFAIL_CELLS) && _game._objects.isInInventory(OBJ_PHONE_CELLS)
			&& _globals[kDurafailRecharged])
		return true;

	return false;
}

void Scene611::handleTalking(int delay) {
	if (_hermitTalkingFl)
		_alreadyTalkingFl = true;

	_hermitTalkingFl = true;
	_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
	_scene->_sequences.addTimer(delay, 100);
}

void Scene611::handleSubDialog1() {
	switch (_action._activeAction._verbId) {
	case 0x287:
		_stickFingerFl = true;
		_nextFrame = 34;
		_hermitMovingFl = false;
		_hermitMode = 5;
		displayHermitQuestions(5);
		_dialog1.write(0x287, false);
		if (!_dialog1.read(0x288))
			_dialog1.write(0x28E, true);

		_dialog2.write(0x29C, true);
		_dialog2.write(0x29D, true);
		_dialog2.write(0x29E, true);
		setDialogNode(2);
		break;

	case 0x288:
		handleTalking(500);
		displayHermitQuestions(6);
		_dialog1.write(0x288, false);
		_dialog1.write(0x289, true);
		if (!_dialog1.read(0x287))
			_dialog1.write(0x28E, true);

		setDialogNode(1);
		break;

	case 0x289:
		handleTalking(500);
		displayHermitQuestions(10);
		_dialog1.write(0x289, false);
		_dialog1.write(0x28A, true);
		_dialog1.write(0x28B, true);
		setDialogNode(1);
		break;

	case 0x28A:
		handleTalking(500);
		displayHermitQuestions(11);
		_dialog1.write(0x28A, false);
		setDialogNode(1);
		break;

	case 0x28B:
		handleTalking(500);
		displayHermitQuestions(12);
		_dialog1.write(0x28C, true);
		_dialog1.write(0x28D, true);
		_dialog1.write(0x28B, false);
		setDialogNode(1);
		break;

	case 0x28C:
		handleTalking(500);
		displayHermitQuestions(13);
		_dialog1.write(0x28C, false);
		setDialogNode(1);
		break;

	case 0x28D:
		handleTalking(500);
		displayHermitQuestions(14);
		_dialog1.write(0x290, true);
		_dialog1.write(0x28D, false);
		_dialog1.write(0x28F, true);
		setDialogNode(1);
		break;

	case 0x28E:
		handleTalking(500);
		displayHermitQuestions(15);
		_dialog1.write(0x295, true);
		_dialog1.write(0x28E, false);
		setDialogNode(1);
		break;

	case 0x290:
		handleTalking(500);
		displayHermitQuestions(17);
		_dialog1.write(0x290, false);
		_dialog1.write(0x28e, false);

		if (!_dialog1.read(0x28F))
			_dialog1.write(0x291, true);

		setDialogNode(1);
		break;

	case 0x291:
		handleTalking(500);
		displayHermitQuestions(18);
		_dialog1.write(0x291, false);
		if ((!_game._objects.isInInventory(OBJ_DURAFAIL_CELLS)) && (!_game._objects.isInInventory(OBJ_PHONE_CELLS))) {
			_dialog1.write(0x292, true);
			_dialog1.write(0x293, true);
		}

		if ((_game._objects.isInInventory(OBJ_DURAFAIL_CELLS)) || (_game._objects.isInInventory(OBJ_PHONE_CELLS)))
			_dialog1.write(0x294, true);

		// WORKAROUND: Fix bug in the original where the option to give Hermit batteries
		// would be given before the player even has any batteries
		_globals[kHermitWantsBatteries] = true;

		setDialogNode(1);
		break;

	case 0x28F:
		handleTalking(500);
		displayHermitQuestions(16);
		_dialog1.write(0x28F, false);
		if (!_dialog1.read(0x290))
			_dialog1.write(0x291, true);

		setDialogNode(1);
		break;

	case 0x295:
		handleTalking(500);
		displayHermitQuestions(20);
		_dialog1.write(0x295, false);
		setDialogNode(1);
		break;

	case 0x292:
		handleTalking(500);
		displayHermitQuestions(19);
		_dialog1.write(0x292, false);
		_dialog1.write(0x293, false);
		setDialogNode(1);
		break;

	case 0x293: {
		handleTalking(200);
		_scene->_kernelMessages.reset();

		Common::String curQuote = _game.getQuote(0x2D1);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y), 0xFDFC, 0, 0, 120, curQuote);

		curQuote = _game.getQuote(0x2D2);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 14), 0xFDFC, 0, 0, 120, curQuote);

		_dialog1.write(0x293, false);
		setDialogNode(0);
		}
		break;

	case 0x294: {
		bool hermitPleasedFl = false;

		switch (_game._difficulty) {
		case DIFFICULTY_EASY:
			hermitPleasedFl = _game._objects.isInInventory(OBJ_DURAFAIL_CELLS) || _game._objects.isInInventory(OBJ_PHONE_CELLS);
			break;

		case DIFFICULTY_MEDIUM:
			hermitPleasedFl = _game._objects.isInInventory(OBJ_DURAFAIL_CELLS) && _game._objects.isInInventory(OBJ_PHONE_CELLS);
			break;

		default: // HARD
			hermitPleasedFl = check4ChargedBatteries();
			break;
		}

		if (hermitPleasedFl) {
			_hermitDisplayedQuestion = 21;
			if (!_giveBatteriesFl)
				setDialogNode(0);
			else
				_giveBatteriesFl = false;
		} else if (((_game._difficulty == DIFFICULTY_MEDIUM) || (_game._difficulty == DIFFICULTY_HARD)) && check2ChargedBatteries()) {
			_hermitDisplayedQuestion = 22;
			if (!_giveBatteriesFl)
				setDialogNode(0);
			else
				_giveBatteriesFl = false;
		} else {
			_hermitDisplayedQuestion = 23;
			if (!_giveBatteriesFl)
				setDialogNode(0);
			else
				_giveBatteriesFl = false;
		}
		_startTradingFl = true;
		}
		break;

	case 0x296: {
		_scene->_kernelMessages.reset();

		Common::String curQuote = _game.getQuote(0x2E6);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 14), 0xFDFC, 0, 0, 120, curQuote);

		setDialogNode(0);
		handleTalking(200);
		}
		break;

	default:
		break;
	}
}

void Scene611::handleSubDialog2() {
	switch (_action._activeAction._verbId) {
	case 0x29C:
		displayHermitQuestions(7);
		setDialogNode(1);
		handleTalking(500);
		break;

	case 0x29D:
		displayHermitQuestions(8);
		setDialogNode(1);
		handleTalking(500);
		break;

	case 0x29E:
		displayHermitQuestions(9);
		setDialogNode(1);
		handleTalking(500);
		break;

	case 0x29F: {
		_scene->_kernelMessages.reset();
		Common::String curQuote = _game.getQuote(0x2A7);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 14), 0xFDFC, 0, 0, 120, curQuote);
		setDialogNode(0);
		_dialog2.write(0x29F, false);
		}
		break;

	default:
		break;
	}
}

void Scene611::handleDialog() {
	if (_game._trigger == 0) {
		_scene->_kernelMessages.reset();
		_game._player._stepEnabled = false;

		Common::String curQuote = _game.getQuote(_action._activeAction._verbId);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);

		if (width > 200) {
			Common::String subQuote1, subQuote2;
			_game.splitQuote(curQuote, subQuote1, subQuote2);
			_scene->_kernelMessages.add(Common::Point(0, -14), 0x1110, 34, 0, 150, subQuote1);

			if (_action._activeAction._verbId == 0x29D)
				_scene->_kernelMessages.add(Common::Point(-18, 0), 0x1110, 34, 1, 150, subQuote2);
			else if (_action._activeAction._verbId == 0x28A)
				_scene->_kernelMessages.add(Common::Point(-10, 0), 0x1110, 34, 1, 150, subQuote2);
			else
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 1, 150, subQuote2);

			_scene->_sequences.addTimer(170, 50);
		} else {
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 1, 120, curQuote);
			_scene->_sequences.addTimer(140, 50);
		}
	} else if (_game._trigger == 50) {
		if (_hermitDialogNode == 1)
			handleSubDialog1();
		else if (_hermitDialogNode == 2)
			handleSubDialog2();
	}
}

void Scene611::displayHermitQuestions(int question) {
	_scene->_kernelMessages.reset();
	_hermitDisplayedQuestion = question;

	switch (question) {
	case 1: {
		Common::String curQuote = _game.getQuote(0x281);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x282);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 14), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		}
		break;

	case 2: {
		Common::String curQuote = _game.getQuote(0x283);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x284);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 14), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		}
		break;

	case 3: {
		Common::String curQuote = _game.getQuote(0x285);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 14), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		}
		break;

	case 4: {
		Common::String curQuote = _game.getQuote(0x286);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 14), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		}
		break;

	case 5: {
		Common::String curQuote = _game.getQuote(0x297);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y - 14), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x298);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x299);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 14), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		}
		break;

	case 6: {
		Common::String curQuote = _game.getQuote(0x29A);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x29B);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 14), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		}
		break;

	case 7: {
		Common::String curQuote = _game.getQuote(0x2A0);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2A1);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		}
		break;

	case 8: {
		Common::String curQuote = _game.getQuote(0x2A2);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2A3);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2A4);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		}
		break;

	case 9: {
		Common::String curQuote = _game.getQuote(0x2A5);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2A6);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		}
		break;

	case 10: {
		Common::String curQuote = _game.getQuote(0x2A8);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2A9);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2AA);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		 }
		 break;

	case 11: {
		Common::String curQuote = _game.getQuote(0x2AB);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 17), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2AC);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2AD);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2AE);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		 }
		 break;

	case 12: {
		Common::String curQuote = _game.getQuote(0x2AF);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 17), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2B0);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2B1);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2B2);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		 }
		 break;

	case 13: {
		Common::String curQuote = _game.getQuote(0x2B3);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 3), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2B4);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 17), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2B5);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2B6);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		_scene->_kernelMessages.add(Common::Point(11, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x2B7));
		_scene->_kernelMessages.add(Common::Point(11, _defaultDialogPos.y + 73), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x2B8));
		_scene->_kernelMessages.add(Common::Point(11, _defaultDialogPos.y + 87), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x2B9));
		 }
		 break;

	case 14: {
		Common::String curQuote = _game.getQuote(0x2BA);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 17), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2BB);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2BC);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2BD);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		 }
		 break;

	case 15: {
		Common::String curQuote = _game.getQuote(0x2BE);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 17), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2BF);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2C0);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2C1);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		 }
		 break;

	case 16: {
		Common::String curQuote = _game.getQuote(0x2C2);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 3), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2C3);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 17), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2C4);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2C5);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2C6);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		 }
		 break;

	case 17: {
		Common::String curQuote = _game.getQuote(0x2C7);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 17), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2C8);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2C0);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2CA);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		 }
		 break;

	case 18: {
		Common::String curQuote = _game.getQuote(0x2CB);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2CC);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2CD);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		 }
		 break;

	case 19: {
		Common::String curQuote = _game.getQuote(0x2CE);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2CF);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2D0);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		 }
		 break;

	case 20: {
		Common::String curQuote = _game.getQuote(0x2E1);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 3), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2E2);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 17), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2E3);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 31), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2E4);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);

		curQuote = _game.getQuote(0x2E5);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, curQuote);
		 }
		 break;

	case 21: {
		Common::String curQuote = _game.getQuote(0x2D3);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 3), 0xFDFC, 0, 0, 800, curQuote);

		curQuote = _game.getQuote(0x2D4);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 17), 0xFDFC, 0, 0, 800, curQuote);

		curQuote = _game.getQuote(0x2D5);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 31), 0xFDFC, 0, 0, 800, curQuote);

		curQuote = _game.getQuote(0x2D6);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, 800, curQuote);

		curQuote = _game.getQuote(0x2D7);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, 800, curQuote);
		}
		break;

	case 22: {
		Common::String curQuote = _game.getQuote(0x2D8);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 17), 0xFDFC, 0, 0, 700, curQuote);

		curQuote = _game.getQuote(0x2D9);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 31), 0xFDFC, 0, 0, 700, curQuote);

		curQuote = _game.getQuote(0x2DA);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, 700, curQuote);

		curQuote = _game.getQuote(0x2DB);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, 700, curQuote);
		 }
		 break;

	case 23: {
		Common::String curQuote = _game.getQuote(0x2DC);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 3), 0xFDFC, 0, 0, 700, curQuote);

		curQuote = _game.getQuote(0x2DD);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 17), 0xFDFC, 0, 0, 700, curQuote);

		curQuote = _game.getQuote(0x2DE);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 31), 0xFDFC, 0, 0, 700, curQuote);

		curQuote = _game.getQuote(0x2DF);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 45), 0xFDFC, 0, 0, 700, curQuote);

		curQuote = _game.getQuote(0x2E0);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 59), 0xFDFC, 0, 0, 700, curQuote);
		}
		break;

	default:
		break;
	 }
}

void Scene611::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXMRC_9");

	_game.loadQuoteSet(0x279, 0x27A, 0x27B, 0x27C, 0x27D, 0x27E, 0x27F, 0x280, 0x281, 0x282, 0x283, 0x284,
		0x285, 0x286, 0x287, 0x288, 0x289, 0x28A, 0x28B, 0x28C, 0x28D, 0x28E, 0x28F, 0x290, 0x291, 0x292,
		0x293, 0x294, 0x295, 0x296, 0x297, 0x298, 0x299, 0x29A, 0x29B, 0x29C, 0x29D, 0x29E, 0x29F, 0x2A0,
		0x2A1, 0x2A2, 0x2A3, 0x2A4, 0x2A5, 0x2A6, 0x2A7, 0x2A8, 0x2A9, 0x2AA, 0x2AB, 0x2AC, 0x2AD, 0x2AE,
		0x2AF, 0x2B0, 0x2B1, 0x2B2, 0x2B3, 0x2B4, 0x2B5, 0x2B6, 0x2B7, 0x2B8, 0x2B9, 0x2BA, 0x2BB, 0x2BC,
		0x2BD, 0x2BE, 0x2BF, 0x2C0, 0x2C1, 0x2C2, 0x2C3, 0x2C4, 0x2C5, 0x2C6, 0x2C7, 0x2C8, 0x2C9, 0x2CA,
		0x2CB, 0x2CC, 0x2CD, 0x2CE, 0x2CF, 0x2D0, 0x2D1, 0x2D2, 0x2D3, 0x2D4, 0x2D5, 0x2D6, 0x2D7, 0x2D8,
		0x2D9, 0x2DA, 0x2DB, 0x2DC, 0x2DD, 0x2DE, 0x2DF, 0x2E0, 0x2E1, 0x2E2, 0x2E3, 0x2E4, 0x2E5, 0x2E6,
		0x323, 0x324, 0);

	_dialog1.setup(kConvHermit1, 0x287, 0x288, 0x289, 0x28A, 0x28B, 0x28C, 0x28D, 0x28E, 0x28F, 0x290,
		0x291, 0x292, 0x293, 0x294, 0x295, 0x296, 0);

	_dialog2.setup(kConvHermit2, 0x29C, 0x29D, 0x29E, 0x29F, 0);

	if (!_game._visitedScenes._sceneRevisited) {
		_dialog1.set(kConvHermit1, 0x287, 0x288, 0x296, 0);
		_dialog2.set(kConvHermit2, 0x29F, 0);
	}

	_vm->_palette->setEntry(252, 51, 51, 47);
	_vm->_palette->setEntry(253, 37, 37, 37);

	_ratPresentFl = false;
	_seenRatFl = true;
	_eyesRunningFl = false;
	_shouldRemoveEyes = false;
	_randVal = 0;
	_defaultDialogPos = Common::Point(264, 43);
	_giveBatteriesFl = false;
	_resetBatterieText = false;
	_alreadyTalkingFl = false;
	_startTradingFl = false;

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(22, 132);
		_game._player._facing = FACING_EAST;
		_duringDialogFl = false;
	}

	if (!_globals[kHasTalkedToHermit]) {
		_scene->loadAnimation(Resources::formatName(611, 'h', -1, EXT_AA, ""), 0);
		_nextFrame = 47;
		_hermitMode = 1;
		_hermitTalkingFl = false;
		_hermitMovingFl = true;
		_check1Fl = true;
		_stickFingerFl = false;
	} else {
		_hermitMode = 0;
		_scene->_hotspots.activate(NOUN_HERMIT, false);
	}

	// WORKAROUND: Fix original adding 'give batteries' option even if you don't have them
	if (_globals[kHermitWantsBatteries]) {
		if ((_game._objects.isInInventory(OBJ_DURAFAIL_CELLS)) || (_game._objects.isInInventory(OBJ_PHONE_CELLS)))
			_dialog1.write(0x294, true);
	}

	if (_duringDialogFl) {
		_game._player._playerPos = Common::Point(237, 129);
		_game._player._facing = FACING_NORTHEAST;

		switch (_hermitDialogNode) {
	case 0:
		_scene->_userInterface.setup(kInputBuildingSentences);
		_hermitDialogNode = 1;
		break;

	case 1:
		_dialog1.start();
		break;

	case 2:
		_dialog2.start();
		break;

	default:
		break;
		}
		displayHermitQuestions(_hermitDisplayedQuestion);
	}

	sceneEntrySound();
}

void Scene611::step() {
	if (_seenRatFl && (_vm->getRandomNumber(1, 100) == 10)) {
		_seenRatFl = false;
		_scene->_sequences.addTimer(1, 80);
	}

	if (_game._trigger == 80) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 8);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_ratPresentFl = true;
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
	} else if (_game._trigger == 81) {
		int syncId = _globals._sequenceIndexes[1];
		_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 20, 0, 0, 0);
		int idx = _scene->_dynamicHotspots.add(NOUN_RAT, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_ratHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(272, 154), FACING_SOUTHEAST);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 9, 10);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], syncId);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_ratTimer = _game._player._priorTimer;
	}

	if (_ratPresentFl && ((_game._player._priorTimer - _ratTimer) > 1200))
		handleRatMoves();

	if (!_eyesRunningFl) {
		_randVal = _vm->getRandomNumber(1, 30);
		_eyesRunningFl = true;
		_scene->_sequences.addTimer(1, 70);
	}

	if (_game._trigger == 70) {
		switch (_randVal) {
		case 2:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_shouldRemoveEyes = true;
			_scene->_sequences.addTimer(60, 71);
			break;

		case 6:
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 12, 3, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 2, 4);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			break;

		case 7:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 5);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_shouldRemoveEyes = true;
			_scene->_sequences.addTimer(60, 71);
			break;

		case 9:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 6);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_shouldRemoveEyes = true;
			_scene->_sequences.addTimer(60, 71);
			break;

		case 13:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 7);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_shouldRemoveEyes = true;
			_scene->_sequences.addTimer(60, 71);
			break;

		case 14:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 8);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_shouldRemoveEyes = true;
			_scene->_sequences.addTimer(60, 71);
			break;

		case 15:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 24, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 5, 8);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			break;

		case 17:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 20, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 9, 11);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			break;

		case 21:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 9);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_shouldRemoveEyes = true;
			_scene->_sequences.addTimer(60, 71);
			break;

		case 25:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 10);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_shouldRemoveEyes = true;
			_scene->_sequences.addTimer(60, 71);
			break;

		case 27:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 11);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_shouldRemoveEyes = true;
			_scene->_sequences.addTimer(60, 71);
			break;

		case 29:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 20, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			break;

		default:
			_scene->_sequences.addTimer(1, 71);
			break;
		}
	}

	if (_game._trigger == 71) {
		if (_shouldRemoveEyes) {
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_shouldRemoveEyes = false;
		}
		_eyesRunningFl = false;
		_randVal = 0;
	}

	if (_game._trigger == 100) {
		if (_alreadyTalkingFl)
			_alreadyTalkingFl = false;
		else
			_hermitMovingFl = true;
	}

	if (_stickFingerFl && (_scene->_animation[0]->getCurrentFrame() == 47)) {
		_stickFingerFl = false;
		_hermitMovingFl = true;
		_hermitMode = 1;
	}

	if (_scene->_animation[0] != nullptr && (_scene->_animation[0]->getCurrentFrame() == 240) && _check1Fl) {
		_check1Fl = false;
		_scene->_kernelMessages.add(Common::Point(33, 88), 0xFDFC, 0, 0, 90, _game.getQuote(0x27E));
		_scene->_sequences.addTimer(120, 120);
	}

	if (_game._trigger == 120) {
		int msgIdx = _scene->_kernelMessages.add(Common::Point(28, 102), 0xFDFC, 0, 0, 90, _game.getQuote(0x27F));
		_scene->_kernelMessages.setQuoted(msgIdx, 4, true);
		_scene->_sequences.addTimer(100, 121);
	}

	if (_game._trigger == 121) {
		int msgIdx = _scene->_kernelMessages.add(Common::Point(23, 116), 0xFDFC, 0, 0, 90, _game.getQuote(0x280));
		_scene->_kernelMessages.setQuoted(msgIdx, 4, true);
	}

	if (_hermitMode == 1) {
		if (_startTradingFl) {
			_hermitMode = 6;
			_hermitMovingFl = false;
			_hermitTalkingFl = false;
			_scene->_sequences.addTimer(1, 110);
		} else if (_hermitTalkingFl) {
			_hermitMode = 2;
			_nextFrame = 18;
			_hermitMovingFl = false;
		} else {
			switch (_vm->getRandomNumber(1, 5)) {
			case 1:
				_nextFrame = 46;
				break;

			case 2:
				_nextFrame = 47;
				break;

			case 3:
				_nextFrame = 48;
				break;

			case 4:
				_nextFrame = 49;
				break;

			case 5:
				_nextFrame = 50;
				break;

			default:
				break;
			}
		}
	}

	if (_hermitMode == 2) {
		if (_startTradingFl) {
			_hermitMode = 6;
			_hermitMovingFl = false;
			_hermitTalkingFl = false;
			_scene->_sequences.addTimer(1, 110);
		} else if (_hermitMovingFl) {
			_hermitMode = 1;
			_nextFrame = 47;
			_hermitTalkingFl = false;
		} else {
			switch (_vm->getRandomNumber(1, 4)) {
			case 1:
				_nextFrame = 18;
				break;

			case 2:
				_nextFrame = 20;
				break;

			case 3:
				_nextFrame = 22;
				break;

			case 4:
				_nextFrame = 24;
				break;

			default:
				break;
			}
		}
	}

	if (_scene->_animation[0] != nullptr && _scene->_animation[0]->getCurrentFrame() == 254)
		_game._player._stepEnabled = true;

	if (_game._trigger == 110) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 2);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 111);
	}

	if (_game._trigger == 111) {
		int syncIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 2);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
		_nextFrame = 1;
	}

	if (_game._trigger == 112) {
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_game._player._visible = true;
	}

	if (_hermitMode == 6) {
		if ((_scene->_animation[0]->getCurrentFrame() == 9) && _check1Fl) {
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 112);
			_check1Fl = false;
		}

		if ((_scene->_animation[0]->getCurrentFrame() == 17) && !_check1Fl) {
			_nextFrame = 26;
			_hermitMode = 4;
			_check1Fl = true;
		}
	}

	if (_hermitMode == 4) {
		if ((_scene->_animation[0]->getCurrentFrame() == 33) && _check1Fl) {
			displayHermitQuestions(_hermitDisplayedQuestion);
			_nextFrame = 1;
			_check1Fl = false;
		}

		if ((_scene->_animation[0]->getCurrentFrame() == 9) && !_check1Fl) {
			_nextFrame = 8;
			_scene->_sequences.addTimer(1, 113);
			_check1Fl = true;
		}
	}

	if (_game._trigger == 113) {
		_game._player._visible = false;
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 2);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 114);
	}

	if (_game._trigger == 114) {
		_resetBatterieText = true;
		int syncIdx = _globals._sequenceIndexes[3];
		_nextFrame = 10;
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 2);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 115);
	}

	if ((_nextFrame >= 0) && (_nextFrame != _scene->_animation[0]->getCurrentFrame())) {
		_scene->_animation[0]->setCurrentFrame(_nextFrame);
		_nextFrame = -1;
	}

	if (_game._trigger == 115) {
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_game._player._visible = true;
		if (_hermitDisplayedQuestion == 21) {
			_game._player._stepEnabled = false;
			handleTrading();
			_hermitMode = 0;
			_startTradingFl = false;
			_nextFrame = 52;
			_globals[kHasTalkedToHermit] = true;
			_scene->_hotspots.activate(NOUN_HERMIT, false);
		} else {
			_game._player._stepEnabled = true;
			_hermitMode = 1;
			_nextFrame = 47;
			_hermitTalkingFl = false;
			_startTradingFl = false;
			_check1Fl = true;
		}
	}
}

void Scene611::preActions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_ALLEY))
		_game._player._walkOffScreenSceneId = 609;

	if (_resetBatterieText)
		_scene->_kernelMessages.reset();
}

void Scene611::actions() {
	if (_game._screenObjects._inputMode == kInputConversation)
		handleDialog();
	else if ((_action.isAction(VERB_GIVE, NOUN_PHONE_CELLS, NOUN_HERMIT)) || (_action.isAction(VERB_GIVE, NOUN_DURAFAIL_CELLS, NOUN_HERMIT))) {
		_action._activeAction._verbId = 0x294;
		_giveBatteriesFl = true;
		handleSubDialog1();
	} else if (_action.isAction(VERB_GIVE, NOUN_HERMIT)) {
		_scene->_kernelMessages.reset();

		Common::String curQuote = _game.getQuote(0x323);
		int width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		int quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y), 0xFDFC, 0, 0, 120, curQuote);

		curQuote = _game.getQuote(0x324);
		width = _vm->_font->getWidth(curQuote, _scene->_textSpacing);
		quotePosX = _defaultDialogPos.x - (width / 2);
		_scene->_kernelMessages.add(Common::Point(quotePosX, _defaultDialogPos.y + 14), 0xFDFC, 0, 0, 120, curQuote);
	} else if (_game._trigger == 90) {
		if (_dialog2.read(0x29C) && _dialog2.read(0x29D) && _dialog2.read(0x29E)) {
			handleTalking(180);
			if (_vm->getRandomNumber(1, 2) == 1)
				displayHermitQuestions(1);
			else
				displayHermitQuestions(2);
		} else {
			handleTalking(180);
			if (_vm->getRandomNumber(1, 2) == 1)
				displayHermitQuestions(3);
			else
				displayHermitQuestions(4);
		}

		_duringDialogFl = true;
		if (_dialog2.read(0x29F)) {
			_hermitDialogNode = 1;
			_dialog1.start();
			_duringDialogFl = true;
		} else {
			_hermitDialogNode = 2;
			_dialog2.write(0x29F, true);
			_dialog2.start();
			_duringDialogFl = true;
		}
	} else if (_action.isAction(VERB_TALKTO, NOUN_HERMIT)) {
		if (!_dialog1.read(0x287)) {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x27A));
			_scene->_sequences.addTimer(120, 90);
		} else {
			int nextQuote = 0;
			switch (_vm->getRandomNumber(1, 3)) {
			case 1:
				nextQuote = 0x27B;
				break;

			case 2:
				nextQuote = 0x27C;
				break;

			case 3:
				nextQuote = 0x27D;
				break;

			default:
				break;
			}
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(nextQuote));
			_scene->_sequences.addTimer(120, 90);
		}
	} else if ((_action.isAction(VERB_WALKTO) || _action.isAction(VERB_LOOK)) && _action.isObject(NOUN_RAT)) {
		switch (_game._trigger) {
		case 0:
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x279));
			_scene->_sequences.addTimer(60, 1);
			break;

		case 1:
			handleRatMoves();
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action._lookFlag) {
		if (_globals[kHasTalkedToHermit])
			_vm->_dialogs->show(61111);
		else
			_vm->_dialogs->show(61110);
	} else if (_action.isAction(VERB_LOOK, NOUN_HERMIT))
		_vm->_dialogs->show(61112);
	else if (_action.isAction(VERB_LOOK, NOUN_TRASH))
		_vm->_dialogs->show(61113);
	else if (_action.isAction(VERB_TAKE, NOUN_TRASH))
		_vm->_dialogs->show(61114);
	else if (_action.isAction(VERB_LOOK, NOUN_CARDBOARD_BOX))
		_vm->_dialogs->show(61115);
	else if (_action.isAction(VERB_TAKE, NOUN_CARDBOARD_BOX))
		_vm->_dialogs->show(61116);
	else if (_action.isAction(VERB_OPEN, NOUN_CARDBOARD_BOX))
		_vm->_dialogs->show(61117);
	else if (_action.isAction(VERB_LOOK, NOUN_REFRIGERATOR))
		_vm->_dialogs->show(61118);
	else if (_action.isAction(VERB_OPEN, NOUN_REFRIGERATOR))
		_vm->_dialogs->show(61119);
	else if (_action.isAction(VERB_TAKE, NOUN_REFRIGERATOR))
		_vm->_dialogs->show(61120);
	else if (_action.isAction(VERB_LOOK, NOUN_BUILDING))
		_vm->_dialogs->show(61121);
	else if (_action.isAction(VERB_LOOK, NOUN_GRAFFITI))
		_vm->_dialogs->show(61122);
	else if (_action.isAction(VERB_LOOK, NOUN_METAL_PIPE))
		_vm->_dialogs->show(61123);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene612::Scene612(MADSEngine *vm) : Scene6xx(vm) {
	_actionMode = -1;
	_cycleIndex = -1;
}

void Scene612::synchronize(Common::Serializer &s) {
	Scene6xx::synchronize(s);

	s.syncAsSint16LE(_actionMode);
	s.syncAsSint16LE(_cycleIndex);
}

void Scene612::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_FISHING_LINE);
	_scene->addActiveVocab(VERB_WALKTO);
}

void Scene612::handleWinchMovement() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 1, 5);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 5, 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		break;

	case 1:
		// CHECKME: Is the "else" block useful as action is always equal to 1 at this point?
		// Or is it a missing bit of code we could fix?
		if (_actionMode == 1) {
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 17, 7, 0, 0);
			_vm->_sound->command(19);
			_game._objects.setRoom(OBJ_PADLOCK_KEY, 1);
			_globals[kBoatRaised] = false;
		} else {
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 17, 9, 0, 0);
			_vm->_sound->command(18);
		}
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		break;

	case 2:
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[4]);
		_game._player._visible = true;
		break;

	case 3: {
		int syncIdx = _globals._sequenceIndexes[2];
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, _cycleIndex);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], syncIdx);
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2F4));
		_game._player._stepEnabled = true;

		_vm->_dialogs->show(61217);
		}
		break;

	default:
		break;
	}
}

void Scene612::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('p', -1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXCD_3");
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXRC_6");

	if ((_globals[kLineStatus] == 2) || (_globals[kLineStatus] == 3)) {
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('f', -1));
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
		int idx = _scene->_dynamicHotspots.add(NOUN_FISHING_LINE, VERB_WALKTO, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(34, 117), FACING_SOUTHEAST);
	}

	if (_globals[kBoatRaised])
		_cycleIndex = -2;
	else
		_cycleIndex = -1;

	_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, _cycleIndex);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(280, 75);
		_game._player._facing = FACING_SOUTHWEST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 3);
		_scene->loadAnimation(formAnimName('R', 1), 70);
	}

	sceneEntrySound();

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_PADLOCK_KEY);

	_game.loadQuoteSet(0x2F5, 0x2F4, 0);
}

void Scene612::step() {
	switch (_game._trigger) {
	case 70:
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
		_scene->_sequences.addTimer(6, 71);
		break;

	case 71:
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

void Scene612::actions() {
	if (_action.isAction(VERB_GET_INSIDE, NOUN_CAR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1: {
			int syncIdx = _globals._sequenceIndexes[1];
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 3);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], syncIdx);
			_scene->_sequences.addTimer(6, 2);
			}
			break;

		case 2:
			_game._player._visible = false;
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], true, 10, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3: {
			int syncIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], true, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
			_scene->_nextSceneId = 504;
			}
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_UNLOCK, NOUN_PADLOCK_KEY, NOUN_CONTROL_BOX)) {
		_cycleIndex = -2;
		_actionMode = 1;
		handleWinchMovement();
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_EXPRESSWAY))
		_vm->_dialogs->show(61210);
	else if (_action.isAction(VERB_LOOK, NOUN_ROPE) || _action.isAction(VERB_LOOK, NOUN_ARMATURE)) {
		if (_globals[kBoatRaised])
			_vm->_dialogs->show(61211);
		else
			_vm->_dialogs->show(61212);
	} else if (_action.isAction(VERB_TAKE, NOUN_ROPE))
		_vm->_dialogs->show(61213);
	else if (_action.isAction(VERB_LOOK, NOUN_CONTROL_BOX)) {
		if (_globals[kBoatRaised])
			_vm->_dialogs->show(61214);
		else
			_vm->_dialogs->show(61216);
	} else if (_action.isAction(VERB_OPEN, NOUN_CONTROL_BOX))
		_vm->_dialogs->show(61215);
	else if (_action.isAction(VERB_LOOK, NOUN_BUILDINGS))
		_vm->_dialogs->show(61218);
	else if (_action.isAction(VERB_LOOK, NOUN_DOME))
		_vm->_dialogs->show(61219);
	else if (_action.isAction(VERB_LOOK, NOUN_STATUE))
		_vm->_dialogs->show(61220);
	else if (_action.isAction(VERB_LOOK, NOUN_MAINTENANCE_BUILDING))
		_vm->_dialogs->show(61221);
	else if (_action.isAction(VERB_OPEN, NOUN_MAINTENANCE_BUILDING))
		_vm->_dialogs->show(61222);
	else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(61223);
	else if (_action.isAction(VERB_LOOK, NOUN_SUPPORT))
		_vm->_dialogs->show(61224);
	else if (_action.isAction(VERB_WALK_DOWN, NOUN_EXPRESSWAY_TO_EAST) || _action.isAction(VERB_WALK_DOWN, NOUN_EXPRESSWAY_TO_WEST))
		_vm->_dialogs->show(61225);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene620::setup() {
	_game._player._spritesPrefix = "";
	setAAName();
}

void Scene620::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('b', 0));
	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
	_game._player._stepEnabled = false;
	_game._player._visible = false;
	_scene->_sequences.addTimer(30, 70);
	_scene->_userInterface.setup(kInputLimitedSentences);
	sceneEntrySound();
}

void Scene620::step() {
	switch (_game._trigger) {
	case 70:
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_scene->loadAnimation(formAnimName('E', -1), 71);
		break;

	case 71:
		if (_scene->_priorSceneId == 751) {
			_globals[kCityFlooded] = true;
			_globals[kTeleporterRoom + 5] = 0;
			_scene->_nextSceneId = 701;
		} else if (_scene->_priorSceneId == 752) {
			_globals[kCityFlooded] = true;
			_globals[kTeleporterRoom + 5] = 0;
			_scene->_nextSceneId = 702;
		} else if (_scene->_priorSceneId < 501 || _scene->_priorSceneId > 752) {
			_globals[kCityFlooded] = true;
			_globals[kTeleporterRoom + 5] = 0;
			_scene->_nextSceneId = _scene->_priorSceneId;
		} else if (_scene->_priorSceneId >= 501 && _scene->_priorSceneId <= 612) {
			_globals[kResurrectRoom] = _globals[kHoverCarLocation];
			_game._objects.addToInventory(OBJ_TIMEBOMB);
			_globals[kTimebombStatus] = 0;
			_globals[kTimebombTimer] = 0;
			_scene->_nextSceneId = 605;
		}
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
