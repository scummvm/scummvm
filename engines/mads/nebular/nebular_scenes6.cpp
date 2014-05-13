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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

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
	_scene->addActiveVocab(0x343);
	_scene->addActiveVocab(0xD1);
}

void Scene601::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXCD_4");

	if (_globals[kLaserHoleIsThere]) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
		_scene->_dynamicHotspots.add(0x343, 0xD1, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
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
	} else if (_scene->_priorSceneId != -2) {
		_game._player._playerPos = Common::Point(229, 129);
		_game._player._facing = FACING_SOUTHWEST;
	}

	sceneEntrySound();
}

void Scene601::step() {
	switch (_game._trigger) {
	case 70:
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_activeAnimation->getNextFrameTimer() - _game._player._ticksAmount;
		_scene->_sequences.addTimer(30, 71);
		break;

	case 71:
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
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
	if (_action.isAction(0x18B, 0x378))
		_scene->_nextSceneId = 602;
	else if (_action.isAction(0x325, 0x324)) {
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
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, 0x31E)) {
		if (!_globals[kLaserHoleIsThere])
			_vm->_dialogs->show(60110);
		else
			_vm->_dialogs->show(60111);
	} else if (_action.isAction(VERB_LOOK, 0x324))
		_vm->_dialogs->show(60112);
	else if (_action.isAction(VERB_LOOK, 0x32C))
		_vm->_dialogs->show(60113);
	else if (_action.isAction(VERB_LOOK, 0x323))
		_vm->_dialogs->show(60114);
	else if (_action.isAction(0x1AD, 0x31E))
		_vm->_dialogs->show(60115);
	else if (_action.isAction(VERB_LOOK, 0x48E))
		_vm->_dialogs->show(60116);
	else if (_action.isAction(VERB_LOOK, 0x378))
		_vm->_dialogs->show(60117);
	else if (_action.isAction(VERB_LOOK, 0x18D))
		_vm->_dialogs->show(60118);
	else if (_action.isAction(VERB_LOOK, 0x38F))
		_vm->_dialogs->show(60119);
	else if (_action.isAction(VERB_LOOK, 0x3C4))
		_vm->_dialogs->show(60120);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene602::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(0xD);
	_scene->addActiveVocab(0x3D3);
	_scene->addActiveVocab(0x343);
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
		int idx = _scene->_dynamicHotspots.add(0x343, VERB_WALKTO, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(80, 134), FACING_NORTHEAST);
		_scene->changeVariant(1);
	} else
		_scene->_hotspots.activate(0x342, false);

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
	int idx = _scene->_dynamicHotspots.add(0x3D3, VERB_WALKTO, _lastSequenceIdx, Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, Common::Point(185, 113), FACING_NORTHWEST);

	if (_game._objects.isInRoom(OBJ_DOOR_KEY)) {
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('k', -1));
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 15);
		if (_globals[kSafeStatus] == 0 || _globals[kSafeStatus] == 2)
			_scene->_hotspots.activate(0x6F, false);
	} else
		_scene->_hotspots.activate(0x6F, false);

	if (_scene->_priorSceneId == 603) {
		_game._player._playerPos = Common::Point(228, 126);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId != -2) {
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
		_globals._sequenceIndexes[5] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[5], true, 12, 1, 0, 0);
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
					_scene->_hotspots.activate(0x6F, true);

				_scene->_sequences.addSubEntry(_lastSequenceIdx,
					SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
		} else {
			_scene->_sequences.remove(_lastSequenceIdx);
			if (_globals[kSafeStatus] == 1)
				_lastSpriteIdx = _globals._spriteIndexes[2];
			else
				_lastSpriteIdx = _globals._spriteIndexes[3];

			_lastSequenceIdx = _scene->_sequences.startReverseCycle(_lastSpriteIdx, false, 12, 1, 0, 0);
			_scene->_sequences.setDepth(_lastSequenceIdx, 14);
			if (_game._objects[OBJ_DOOR_KEY]._roomNumber == _scene->_currentSceneId)
				_scene->_hotspots.activate(0x6F, false);

			_scene->_sequences.addSubEntry(_lastSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		}
		break;

	case 2: {
		int synxIdx = _lastSequenceIdx;
		_lastSequenceIdx = _scene->_sequences.startCycle(_lastSpriteIdx, false, _cycleIndex);
		_scene->_sequences.setDepth(_lastSequenceIdx, 14);
		_scene->_sequences.updateTimeout(_lastSequenceIdx, synxIdx);
		int idx = _scene->_dynamicHotspots.add(0x3D3, VERB_WALKTO, _lastSequenceIdx, Common::Rect(0, 0, 0, 0));
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
	if (_action.isAction(0x18B, 0x1F9))
		_scene->_nextSceneId = 601;
	else if (_action.isAction(0x18B, 0x70))
		_scene->_nextSceneId = 603;
	else if (_action.isAction(VERB_OPEN, 0x3D3) && ((_globals[kSafeStatus] == 0) || (_globals[kSafeStatus] == 2))) {
		_safeMode = 1;
		_cycleIndex = -2;
		handleSafeActions();
	} else if (_action.isAction(VERB_CLOSE, 0x3D3) && ((_globals[kSafeStatus] == 1) || (_globals[kSafeStatus] == 3))) {
		_safeMode = 2;
		_cycleIndex = -1;
		handleSafeActions();
	} else if (_action.isAction(0x17B, 0x3A7, 0x3D3)) {
		if ((_globals[kSafeStatus] == 0) && (_game._difficulty != DIFFICULTY_HARD)) {
			_safeMode = 3;
			_cycleIndex = -2;
			handleSafeActions();
		}
	} else if ((_action.isAction(VERB_PUT, 0x120, 0x343) || _action.isAction(VERB_PUT, 0x57, 0x343)
		|| _action.isAction(0x365, 0x57, 0x343) || _action.isAction(0x365, 0x120, 0x343)) && (_globals[kSafeStatus] == 0)) {
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
			_game._player._priorTimer = _scene->_activeAnimation->getNextFrameTimer() - _game._player._ticksAmount;
			_lastSpriteIdx = _globals._spriteIndexes[3];
			_lastSequenceIdx = _scene->_sequences.startCycle(_lastSpriteIdx, false, -1);
			_scene->_sequences.setDepth(_lastSequenceIdx, 14);
			int idx = _scene->_dynamicHotspots.add(0x3D3, VERB_WALKTO, _lastSequenceIdx, Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(185, 113), FACING_NORTHWEST);
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 9);
			idx = _scene->_dynamicHotspots.add(0x343, VERB_WALKTO, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
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
	} else if (_action.isAction(VERB_TAKE, 0x6F) && (_game._trigger || _game._objects.isInRoom(OBJ_DOOR_KEY))) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[5] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[5], true, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 3);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 3, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[6]);
			_scene->_hotspots.activate(0x6F, false);
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
	else if (_action.isAction(VERB_LOOK, 0x89))
		_vm->_dialogs->show(60211);
	else if (_action.isAction(VERB_LOOK, 0x1F9))
		_vm->_dialogs->show(60212);
	else if (_action.isAction(VERB_LOOK, 0x160))
		_vm->_dialogs->show(60213);
	else if (_action.isAction(VERB_LOOK, 0x47) || _action.isAction(VERB_LOOK, 0x3CA))
		_vm->_dialogs->show(60214);
	else if (_action.isAction(VERB_LOOK, 0x492))
		_vm->_dialogs->show(60215);
	else if (_action.isAction(VERB_LOOK, 0x3D8))
		_vm->_dialogs->show(60216);
	else if (_action.isAction(VERB_LOOK, 0x289))
		_vm->_dialogs->show(60217);
	else if (_action.isAction(VERB_LOOK, 0x2F6))
		_vm->_dialogs->show(60218);
	else if (_action.isAction(VERB_LOOK, 0x491))
		_vm->_dialogs->show(60219);
	else if (_action.isAction(VERB_LOOK, 0x493))
		_vm->_dialogs->show(60220);
	else if (_action.isAction(VERB_LOOK, 0x70))
		_vm->_dialogs->show(60221);
	else if (_action.isAction(VERB_LOOK, 0x3D3)) {
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
	} else if (_action.isAction(0x17B, 0x6F, 0x3D3) || _action.isAction(0x17B, 0xFF, 0x3D3))
		_vm->_dialogs->show(60225);
	else if (_action.isAction(VERB_PULL, 0x3D3))
		_vm->_dialogs->show(60226);
	else if (_action.isAction(VERB_PUT, 0x3D8) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId)))
		_vm->_dialogs->show(60227);
	else if (_action.isAction(VERB_LOOK, 0x342))
		_vm->_dialogs->show(60228);
	else if (_action.isAction(VERB_LOOK, 0x343))
		_vm->_dialogs->show(60229);
	else if (_action.isAction(VERB_LOOK, 0x3F5))
		_vm->_dialogs->show(60231);
	else if (_action.isAction(VERB_THROW, 0x2A, 0x3D3) || _action.isAction(VERB_THROW, 0x2B, 0x3D3))
		_vm->_dialogs->show(60232);
	else if (_action.isAction(VERB_PUT, 0x171))
		_vm->_dialogs->show(60233);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene603::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(0xD);
	_scene->addActiveVocab(0x57);
	_scene->addActiveVocab(0x3A8);
}

void Scene603::enter() {
	if (_game._objects[OBJ_COMPACT_CASE]._roomNumber == _scene->_currentSceneId) {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXMRD_3");
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', -1));
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_compactCaseHotspotId = _scene->_dynamicHotspots.add(0x57, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_compactCaseHotspotId, Common::Point(250, 152), FACING_SOUTHEAST);
	}

	if ((_game._difficulty != DIFFICULTY_HARD) && (_game._objects[OBJ_NOTE]._roomNumber == _scene->_currentSceneId)) {
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXMRC_9");
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('p', -1));
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
		_noteHotspotId = _scene->_dynamicHotspots.add(0x3A8, VERB_WALKTO, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_noteHotspotId, Common::Point(242, 118), FACING_NORTHEAST);
	}

	if (_scene->_priorSceneId != -2)
		_game._player._playerPos = Common::Point(113, 134);

	sceneEntrySound();
}

void Scene603::actions() {
	if (_action.isAction(0x18C, 0x3F1))
		_scene->_nextSceneId = 602;
	else if (_action.isAction(VERB_TAKE, 0x57)) {
		if ( _game._trigger || !_game._objects.isInInventory(OBJ_COMPACT_CASE)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[4] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[4], false, 8, 1, 0, 0);
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
	} else if (_action.isAction(VERB_TAKE, 0x3A8)) {
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
		}
	} else if (_action._lookFlag)
		_vm->_dialogs->show(60310);
	else if (_action.isAction(VERB_LOOK, 0x1CB))
		_vm->_dialogs->show(60311);
	else if (_action.isAction(VERB_LOOK, 0x3E5))
		_vm->_dialogs->show(60312);
	else if (_action.isAction(VERB_TAKE, 0x3E5))
		_vm->_dialogs->show(60313);
	else if (_action.isAction(VERB_LOOK, 0x3EE))
		_vm->_dialogs->show(60314);
	else if (_action.isAction(VERB_LOOK, 0x3ED))
		_vm->_dialogs->show(60315);
	else if (_action.isAction(VERB_LOOK, 0x49D))
		_vm->_dialogs->show(60316);
	else if (_action.isAction(VERB_LOOK, 0x2F6))
		_vm->_dialogs->show(60317);
	else if (_action.isAction(VERB_LOOK, 0x49E) || _action.isAction(VERB_LOOK, 0x49F) || _action.isAction(VERB_LOOK, 0x3E7))
		_vm->_dialogs->show(60318);
	else if (_action.isAction(VERB_LOOK, 0x3DE))
		_vm->_dialogs->show(60319);
	else if (_action.isAction(VERB_TAKE, 0x3DE))
		_vm->_dialogs->show(60320);
	else if (_action.isAction(VERB_LOOK, 0x3DF))
		_vm->_dialogs->show(60321);
	else if (_action.isAction(VERB_TAKE, 0x3DF))
		_vm->_dialogs->show(60322);
	else if (_action.isAction(VERB_TAKE, 0x3A8))
		_vm->_dialogs->show(60323);
	else if (_action.isAction(VERB_LOOK, 0x3A8)) {
		if (_game._objects[OBJ_NOTE]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60324);
	} else if (_action.isAction(VERB_LOOK, 0x3E3)) {
		if (_game._objects[OBJ_NOTE]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60326);
		else
			_vm->_dialogs->show(60325);
	} else if (_action.isAction(VERB_LOOK, 0x3EA)) {
		if (_game._objects[OBJ_COMPACT_CASE]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60327);
		else
			_vm->_dialogs->show(60328);
	} else if (_action.isAction(VERB_LOOK, 0x57) && (_action._mainObjectSource == 4))
		_vm->_dialogs->show(60329);
	// For the next two checks, the second part of the check wasn't surrounded par parenthesis, which was obviously wrong
	else if (_action.isAction(VERB_LOOK) && (_action.isAction(0x31) || _action.isAction(0x3EA) || _action.isAction(0x3E8)))
		_vm->_dialogs->show(60331);
	else if (_action.isAction(VERB_TAKE) && (_action.isAction(0x31) || _action.isAction(0x3EA) || _action.isAction(0x3E8)))
		_vm->_dialogs->show(60332);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene604::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(0x468);
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(0x171);
}

void Scene604::enter() {
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXCD_9");
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(Resources::formatName(620, 'b', 0, EXT_SS, ""));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RXMRC_9");

	if (_globals[kTimebombStatus] == 1) {
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, -1);
		_timebombHotspotId = _scene->_dynamicHotspots.add(0x171, VERB_WALKTO, _globals._sequenceIndexes[6], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_timebombHotspotId, Common::Point(166, 118), FACING_NORTHEAST);
	}

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_TIMEBOMB);

	_vm->_palette->setEntry(252, 63, 37, 26);
	_vm->_palette->setEntry(253, 45, 24, 17);
	_animationActiveFl = false;

	if (_scene->_priorSceneId != -2) {
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
		_game._player._priorTimer = _scene->_activeAnimation->getNextFrameTimer() - _game._player._ticksAmount;
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

	if (_monsterActive && (_scene->_activeAnimation != nullptr)) {
		if (_scene->_activeAnimation->getCurrentFrame() != _monsterFrame) {
			_monsterFrame = _scene->_activeAnimation->getCurrentFrame();
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
				_scene->_activeAnimation->setCurrentFrame(nextMonsterFrame);
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
		_globals._sequenceIndexes[5] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[5], false, 9, 1, 0, 0);
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
		_timebombHotspotId = _scene->_dynamicHotspots.add(0x171, VERB_WALKTO, _globals._sequenceIndexes[6], Common::Rect(0, 0, 0, 0));
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
	if (_action.isAction(0x325, 0x324)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
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
	} else if ((_action.isAction(VERB_PUT, 0x3F6) || _action.isAction(VERB_PUT, 0x181) || _action.isAction(VERB_THROW, 0x181))
		&& (_action.isAction(0x2A) || _action.isAction(0x2B)))
		_vm->_dialogs->show(60420);
	else if (_action.isAction(VERB_PUT, 0x171, 0x3F6) || _action.isAction(VERB_PUT, 0x171, 0x181)) {
		_bombMode = 1;
		if ((_game._difficulty == DIFFICULTY_HARD) || _globals[kWarnedFloodCity])
			handleBombActions();
		else if ((_game._objects.isInInventory(OBJ_POLYCEMENT) && _game._objects.isInInventory(OBJ_CHICKEN))
			 && ((_globals[kLineStatus] == LINE_TIED) || ((_game._difficulty == DIFFICULTY_EASY) && (!_globals[kBoatRaised]))))
			handleBombActions();
		else if (_game._difficulty == DIFFICULTY_EASY)
			_vm->_dialogs->show(60424);
		else {
			_vm->_dialogs->show(60425);
			_globals[kWarnedFloodCity] = true;
		}
	} else if (_action.isAction(VERB_TAKE, 0x171)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_TIMEBOMB)) {
			_bombMode = 2;
			handleBombActions();
		}
	} else if (_action._lookFlag)
		_vm->_dialogs->show(60411);
	else if (_action.isAction(VERB_LOOK, 0x181)) {
		if (_monsterActive) {
			_vm->_dialogs->show(60413);
		} else {
			_vm->_dialogs->show(60412);
		}
	} else if (_action.isAction(VERB_LOOK, 0x18D))
		_vm->_dialogs->show(60414);
	else if (_action.isAction(VERB_LOOK, 0x3F4))
		_vm->_dialogs->show(60415);
	else if (_action.isAction(VERB_LOOK, 0x479))
		_vm->_dialogs->show(60416);
	else if (_action.isAction(VERB_LOOK, 0x3F2))
		_vm->_dialogs->show(60417);
	else if (_action.isAction(VERB_LOOK, 0x324))
		_vm->_dialogs->show(60418);
	else if (_action.isAction(VERB_LOOK, 0x3C4))
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

	_globals._sequenceIndexes[1] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[1], false, 15, 0, 0, 0);
	_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], false, 17, 0, 0, 0);
	_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 14, 0, 0, 0);
	_globals._sequenceIndexes[4] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[4], false, 13, 0, 0, 0);
	_globals._sequenceIndexes[5] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[5], false, 17, 0, 0, 0);
	_globals._sequenceIndexes[6] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[6], false, 18, 0, 0, 0);

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

void Scene607::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(0x471);
	_scene->addActiveVocab(VERB_WALKTO);
}

void Scene607::enter() {
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXCD_3");

	if (!_game._visitedScenes._sceneRevisited && (_scene->_priorSceneId != 608))
		_globals[kDogStatus] = 1;

	if ((_scene->_priorSceneId == 608) && (_globals[kDogStatus] < 3))
		_globals[kDogStatus] = 3;

	_animationActive = 0;

	if ((_globals[kDogStatus] == 1) && (_game._difficulty != DIFFICULTY_EASY)) {
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
		_scene->_hotspots.activate(0x471, false);

	_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);

	if (_scene->_priorSceneId == 608) {
		_game._player._playerPos = Common::Point(297, 50);
		_game._player._facing = FACING_SOUTHEAST;
	} else if (_scene->_priorSceneId != -2) {
		_game._player._playerPos = Common::Point(40, 104);
		_game._player._facing = FACING_SOUTHEAST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
		_scene->loadAnimation(formAnimName('R', 1), 80);
	} else if (_globals[kDogStatus] == 2) {
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
	if (_globals[kDogStatus] == 2) {
		int32 diff = _scene->_frameStartTime - _lastFrameTime;
		if ((diff >= 0) && (diff <= 4))
			_dogTimer += diff;
		else
			_dogTimer++;

		_lastFrameTime = _scene->_frameStartTime;
	}

	if ((_dogTimer >= 480) && !_dogLoop && !_shopAvailable && (_globals[kDogStatus] == 2) && !_game._player._special) {
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
		_globals[kDogStatus] = 1;
		_scene->_hotspots.activate(0x471, true);
	}

	if (!_dogEatsRex && (_game._difficulty != DIFFICULTY_EASY) && !_animationActive && (_globals[kDogStatus] == 1)
	 && !_dogBarking && (_vm->getRandomNumber(1, 50) == 10)) {
		_dogBarking = true;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[1], false, 5, 8, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
		_scene->_kernelMessages.reset();
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 2, 100);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		_counter = 0;
	}

	if ((_game._trigger == 70) && !_dogEatsRex && (_globals[kDogStatus] == 1) && !_animationActive) {
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

	if (_game._player._moving && (_game._difficulty != DIFFICULTY_EASY) && !_shopAvailable && (_globals[kDogStatus] == 1) && (_scene->_rails.getNext() > 0)) {
		_game._player.cancelCommand();
		_game._player.startWalking(Common::Point(268, 72), FACING_NORTHEAST);
		_scene->_rails.resetNext();
	}

	if ((_game._player._special > 0) && (_game._difficulty != DIFFICULTY_EASY) && (_globals[kDogStatus] == 1) && _game._player._stepEnabled)
		_game._player._stepEnabled = false;

	if ((_game._difficulty != DIFFICULTY_EASY) && (_globals[kDogStatus] == 1) && (_game._player._playerPos == Common::Point(268, 72))
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
			_globals._sequenceIndexes[4] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[4], false, 15, 3, 0, 0);
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
		_game._player._priorTimer = _scene->_activeAnimation->getNextFrameTimer() - _game._player._ticksAmount;
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
		_game._player._priorTimer = _scene->_activeAnimation->getNextFrameTimer() - _game._player._ticksAmount;

		if (_animationMode != 1)
			_scene->_hotspots.activate(0x471, false);
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
			_globals[kDogStatus] = 2;
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
	if (_action.isAction(VERB_TALKTO, 0x471))
		_game._player._needToWalk = false;

	if (_action.isAction(0x18B, 0x478) && (_globals[kDogStatus] == 2) && (_game._difficulty != DIFFICULTY_EASY)) {
		_shopAvailable = true;
		_dogTimer = 0;
	}

	if (_action.isAction(VERB_THROW, 0x2D, 0x471) || _action.isAction(VERB_THROW, 0x2C, 0x471))
		_game._player.walk(Common::Point(193, 100), FACING_NORTHEAST);

	if (_action.isAction(VERB_THROW, 0x2D, 0x2C3) || _action.isAction(VERB_THROW, 0x2C, 0x2C3))
		_game._player.walk(Common::Point(201, 107), FACING_SOUTHEAST);
}

void Scene607::actions() {
	if (_action.isAction(0x18B, 0x478))
		_scene->_nextSceneId = 608;
	else if (_action.isAction(0x325, 0x324)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
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
	} else if (_action.isAction(VERB_THROW, 0x2D, 0x471) || _action.isAction(VERB_THROW, 0x2C, 0x471)) {
		if (_game._difficulty != DIFFICULTY_EASY) {
			_animationMode = 1;
			_scene->_kernelMessages.reset();
			if (_game._trigger == 0)
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2F6));

			handleThrowingBone();
		}
	} else if ((_action.isAction(VERB_THROW, 0x2D, 0x2C3) || _action.isAction(VERB_THROW, 0x2C, 0x2C3)) && (_game._difficulty != DIFFICULTY_EASY)
		 && ((_globals[kDogStatus] == 1) || _game._trigger)) {
		_animationMode = 2;
		if (_game._trigger == 0) {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2F6));
		}
		handleThrowingBone();
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, 0x31E)) {
		if ((_globals[kDogStatus] == 1) || (_game._difficulty == DIFFICULTY_EASY))
			_vm->_dialogs->show(60710);
		else
			_vm->_dialogs->show(60711);
	} else if (_action.isAction(VERB_LOOK, 0x18D))
		_vm->_dialogs->show(60712);
	else if (_action.isAction(VERB_LOOK, 0x2C3))
		_vm->_dialogs->show(60713);
	else if (_action.isAction(VERB_LOOK, 0x324))
		_vm->_dialogs->show(60714);
	else if (_action.isAction(VERB_LOOK, 0x3FB))
		_vm->_dialogs->show(60715);
	else if (_action.isAction(VERB_LOOK, 0x1E6) && (_globals[kDogStatus] == 1))
		_vm->_dialogs->show(60716);
	else if (_action.isAction(VERB_LOOK, 0x244))
		_vm->_dialogs->show(60717);
	else if (_action.isAction(VERB_LOOK, 0x3FD))
		_vm->_dialogs->show(60718);
	else if (_action.isAction(VERB_LOOK, 0x3FF))
		_vm->_dialogs->show(60719);
	else if (_action.isAction(VERB_LOOK, 0x31D))
		_vm->_dialogs->show(60720);
	else if (_action.isAction(VERB_LOOK, 0x3F9))
		_vm->_dialogs->show(60721);
	else if (_action.isAction(VERB_LOOK, 0x3FA)) {
		if (_globals[kDogStatus] == 1)
			_vm->_dialogs->show(60723);
		else
			_vm->_dialogs->show(60722);
	} else if (_action.isAction(VERB_LOOK, 0x478)) {
		if (_globals[kDogStatus] == 1)
			_vm->_dialogs->show(60725);
		else
			_vm->_dialogs->show(60724);
	} else if (_action.isAction(VERB_LOOK, 0x471))
		_vm->_dialogs->show(60726);
	else if (_action.isAction(VERB_TALKTO, 0x471))
		_vm->_dialogs->show(60727);
	else if (_action.isAction(VERB_LOOK, 0x31F))
		_vm->_dialogs->show(60728);
	else if (_action.isAction(0x1AD, 0x31E))
		_vm->_dialogs->show(60730);
	else if (_action.isAction(0x3FF) && (_action.isAction(VERB_OPEN) || _action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL)))
		_vm->_dialogs->show(60731);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene608::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(0xD);
	_scene->addActiveVocab(0x115);
	_scene->addActiveVocab(0x324);
	_scene->addActiveVocab(0x471);
}

void Scene608::resetDogVariables() {
	_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
	int idx = _scene->_dynamicHotspots.add(0x471, VERB_WALKTO, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
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

	if (_globals[kCarStatus] == 0) {
		_scene->_sequences.remove(_globals._sequenceIndexes[6]);
		_scene->_sequences.remove(_globals._sequenceIndexes[7]);
		_scene->loadAnimation(formAnimName('A', -1));
	} else {
		_scene->_sequences.remove(_globals._sequenceIndexes[8]);
		_scene->_sequences.remove(_globals._sequenceIndexes[6]);
		_scene->_sequences.remove(_globals._sequenceIndexes[7]);
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_activeAnimation->setCurrentFrame(6);
	}
}

void Scene608::setCarAnimations() {
	_scene->freeAnimation();
	if (_globals[kCarStatus] == 0) {
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 1);
		_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[6], Common::Point(143, 98));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 6);
		_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, 1);
		_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[7], Common::Point(141, 67));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 15);
	} else {
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 1);
		_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[6], Common::Point(143, 128));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 6);
		_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, 1);
		_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[7], Common::Point(141, 97));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 15);
		_globals._sequenceIndexes[8] = _scene->_sequences.startCycle(_globals._spriteIndexes[8], false, 1);
		_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[8], Common::Point(144, 126));
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
		int idx = _scene->_dynamicHotspots.add(0x115, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_polycementHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(249, 129), FACING_NORTHEAST);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
	}

	if (_game._objects.isInRoom(OBJ_REARVIEW_MIRROR)) {
		_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('m', -1));
		_globals._sequenceIndexes[12] = _scene->_sequences.startCycle(_globals._spriteIndexes[12], false, 1);
		int idx = _scene->_dynamicHotspots.add(0x120, VERB_WALKTO, _globals._sequenceIndexes[12], Common::Rect(0, 0, 0, 0));
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
			_globals[kDogStatus] = 3;
			_dogActiveFl = true;
		} else
			_dogActiveFl = (_globals[kDogStatus] != 4);
	} else {
		_globals[kDogStatus] = 4;
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
		_globals[kCarStatus] = 0;

	_animationMode = 0;
	_carMoveMode = 0;
	_carFrame = -1;

	if (_globals[kCarStatus] == 0) {
		_carMode = 0;
		_dogDeathMode = 0;
		_resetPositionsFl = false;
		int idx = _scene->_dynamicHotspots.add(0x324, VERB_WALKTO, -1, Common::Rect(99, 69, 99 + 82, 69 + 25));
		_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('A', -1));
	} else if (_globals[kCarStatus] == 3) {
		_carMode = 0;
		_dogDeathMode = 0;
		_resetPositionsFl = false;
		int idx = _scene->_dynamicHotspots.add(0x324, VERB_WALKTO, -1, Common::Rect(100, 100, 100 + 82, 100 + 25));
		_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_activeAnimation->setCurrentFrame(6);
	} else if (_globals[kCarStatus] == 1) {
		_carMode = 2;
		_dogDeathMode = 0;
		_resetPositionsFl = false;
		int idx = _scene->_dynamicHotspots.add(0x324, VERB_WALKTO, -1, Common::Rect(99, 69, 99 + 82, 69 + 25));
		_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('C', -1));
	} else if (_globals[kCarStatus] == 2) {
		_carMode = 1;
		_dogDeathMode = 2;
		_resetPositionsFl = true;
		int idx = _scene->_dynamicHotspots.add(0x324, VERB_WALKTO, -1, Common::Rect(99, 69, 99 + 82, 69 + 25));
		_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('B', -1));
	} else {
		_carMode = 3;
		_dogDeathMode = 2;
		_resetPositionsFl = true;
		int idx = _scene->_dynamicHotspots.add(0x324, VERB_WALKTO, -1, Common::Rect(100, 100, 100 + 82, 100 + 25));
		_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('D', -1));
	}

	_vm->_palette->setEntry(252, 63, 44, 30);
	_vm->_palette->setEntry(253, 63, 20, 22);

	if (_scene->_priorSceneId != -2) {
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
			_globals._sequenceIndexes[10] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[10], false, 9, 0, 0, 0);
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
				_globals._sequenceIndexes[5] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[5], false, 5, 8, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 4);
				int idx = _scene->_dynamicHotspots.add(0x471, VERB_WALKTO, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
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

	// CHECKME: _checkFl is always false?
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
		_globals[kCarStatus] = 0;
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

	if ((_carMode == 4) && (_scene->_activeAnimation != nullptr)) {
		if (_scene->_activeAnimation->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_activeAnimation->getCurrentFrame();
			int nextFrame = -1;

			if (_carFrame == 10) {
				_game._player._visible = true;
				_game._player._priorTimer = _scene->_activeAnimation->getNextFrameTimer() - _game._player._ticksAmount;
			}

			if (_carFrame == 56) {
				resetDogVariables();
				_animationMode = 0;
				_nextTrigger = 2;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_activeAnimation->getCurrentFrame())) {
				_scene->_activeAnimation->setCurrentFrame(nextFrame);
				_carFrame = nextFrame;
			}
		}
	}

	if ((_carMode == 5) && (_scene->_activeAnimation != nullptr)) {
		if (_scene->_activeAnimation->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_activeAnimation->getCurrentFrame();
			int nextFrame = -1;

			if (_carFrame == 10) {
				_game._player._visible = true;
				_game._player._priorTimer = _scene->_activeAnimation->getNextFrameTimer() - _game._player._ticksAmount;
			}

			if (_carFrame == 52) {
				resetDogVariables();
				_animationMode = 0;
				_nextTrigger = 2;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_activeAnimation->getCurrentFrame())) {
				_scene->_activeAnimation->setCurrentFrame(nextFrame);
				_carFrame = nextFrame;
			}
		}
	}


	if ((_carMode == 6) && (_scene->_activeAnimation != nullptr)) {
		if (_scene->_activeAnimation->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_activeAnimation->getCurrentFrame();
			int nextFrame = -1;

			if (_carFrame == 11) {
				_game._player._visible = true;
				_game._player._priorTimer = _scene->_activeAnimation->getNextFrameTimer() - _game._player._ticksAmount;
			}

			if (_carFrame == 41) {
				_globals._sequenceIndexes[10] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[10], false, 9, 0, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 10, 11);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 6);
				_dogUnderCar = true;
				_nextTrigger = 2;
			}


			if ((nextFrame >= 0) && (nextFrame != _scene->_activeAnimation->getCurrentFrame())) {
				_scene->_activeAnimation->setCurrentFrame(nextFrame);
				_carFrame = nextFrame;
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

	if ((_carMode == 0) && (_scene->_activeAnimation != nullptr)) {
		if (_scene->_activeAnimation->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_activeAnimation->getCurrentFrame();
			int nextFrame = -1;

			if ((_globals[kCarStatus] == 0) || (_globals[kCarStatus] == 3)) {
				switch (_carMoveMode) {
				case 0:
					if (_globals[kCarStatus] == 0)
						nextFrame = 0;
					else
						nextFrame = 6;
					break;

				case 1:
					if (_scene->_activeAnimation->getCurrentFrame() >= 12) {
						nextFrame = 0;
						_carMoveMode = 0;
						_globals[kCarStatus] = 0;
					}
					break;

				case 2:
					if (_scene->_activeAnimation->getCurrentFrame() >= 6) {
						nextFrame = 6;
						_carMoveMode = 0;
						_globals[kCarStatus] = 3;
					}
					break;

				default:
					break;
				}
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_activeAnimation->getCurrentFrame())) {
				_scene->_activeAnimation->setCurrentFrame(nextFrame);
				_carFrame = nextFrame;
			}
		}
	}

	if ((_carMode == 2) && (_scene->_activeAnimation != nullptr)) {
		if (_scene->_activeAnimation->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_activeAnimation->getCurrentFrame();
			int nextFrame = -1;

			if (_carMoveMode == 0)
				nextFrame = 28;
			else if (_scene->_activeAnimation->getCurrentFrame() >= 28) {
				nextFrame = 28;
				_carMoveMode = 0;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_activeAnimation->getCurrentFrame())) {
				_scene->_activeAnimation->setCurrentFrame(nextFrame);
				_carFrame = nextFrame;
			}
		}
	}

	if ((_carMode == 3) && (_scene->_activeAnimation != nullptr)) {
		if (_scene->_activeAnimation->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_activeAnimation->getCurrentFrame();
			int nextFrame = -1;

			if (_resetPositionsFl) {
				nextFrame = 0;
				_carMoveMode = 0;
			} else if (_carMoveMode == 0)
				nextFrame = 6;
			else if (_scene->_activeAnimation->getCurrentFrame() >= 6) {
				nextFrame = 6;
				_carMoveMode = 0;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_activeAnimation->getCurrentFrame())) {
				_scene->_activeAnimation->setCurrentFrame(nextFrame);
				_carFrame = nextFrame;
			}
		}
	}


	if ((_carMode == 1) && (_scene->_activeAnimation != nullptr)) {
		if (_scene->_activeAnimation->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_activeAnimation->getCurrentFrame();
			int nextFrame = -1;

			if (_resetPositionsFl) {
				nextFrame = 0;
				_carMoveMode = 0;
			} else if (_carMoveMode == 0)
				nextFrame = 6;
			else if (_scene->_activeAnimation->getCurrentFrame() >= 6) {
				nextFrame = 6;
				_carMoveMode = 0;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_activeAnimation->getCurrentFrame())) {
				_scene->_activeAnimation->setCurrentFrame(nextFrame);
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
			_globals._sequenceIndexes[9] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[9], false, 15, 5, 0, 0);
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

	if ((_action.isAction(VERB_THROW, 0x2C, 0x410) || _action.isAction(VERB_THROW, 0x2D, 0x410)
		|| _action.isAction(VERB_THROW, 0x2C, 0x411) || _action.isAction(VERB_THROW, 0x2D, 0x411)
		|| _action.isAction(VERB_THROW, 0x2D, 0x471) || _action.isAction(VERB_THROW, 0x2C, 0x471)) && _dogActiveFl) {
		_game._player._stepEnabled = false;
		_game._player.walk(Common::Point(56, 146), FACING_EAST);
	}

	if ((_action.isAction(VERB_THROW, 0x2D, 0x41D) || _action.isAction(VERB_THROW, 0x2C, 0x41D)
		|| _action.isAction(VERB_THROW, 0x2D, 0x41E) || _action.isAction(VERB_THROW, 0x2C, 0x41E)) && _dogActiveFl) {
		_game._player._stepEnabled = false;
		_game._player.walk(Common::Point(75, 136), FACING_EAST);
	}

	if (_action.isAction(VERB_PUSH, 0x403) && _dogUnderCar) {
		_buttonPressedonTimeFl = true;
		_dogDeathMode = 1;
	} else
		_buttonPressedonTimeFl = false;
}

void Scene608::actions() {
	if (_action.isAction(0x18B, 0x70))
		_scene->_nextSceneId = 607;
	else if (_action.isAction(VERB_PUSH, 0x403)) {
		_game._player._stepEnabled = true;
		switch (_game._trigger) {
		case 0:
			if ((_globals[kCarStatus] == 0) || (_globals[kCarStatus] == 1) || (_globals[kCarStatus] == 2)) {
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], true, 6, 2, 0, 0);
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
				_globals[kCarStatus] = 1;
				_carMode = 2;
				_globals[kDogStatus] = 4;
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
				int idx = _scene->_dynamicHotspots.add(0x324, VERB_WALKTO, -1, Common::Rect(100, 100, 100 + 82, 100 + 25));
				_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
				if (_globals[kCarStatus] == 1)
					_scene->_sequences.addTimer(120, 3);
				else {
					if (_dogDeathMode == 0)
						_globals[kCarStatus] = 3;
					else {
						_globals[kCarStatus] = 4;
						_carMode = 3;
						_dogDeathMode = 2;
					}
					_game._player._stepEnabled = true;
				}
			}
			break;

		case 3:
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x302));
			_globals[kCarStatus] = 4;
			_carMode = 3;
			_dogDeathMode = 2;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PUSH, 0x402)) {
		switch (_game._trigger) {
		case 0:
			if ((_globals[kCarStatus] == 3) || (_globals[kCarStatus] == 4)) {
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], true, 6, 2, 0, 0);
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
				_globals[kCarStatus] = 0;
			else if (_dogDeathMode == 2) {
				_globals[kCarStatus] = 2;
				_carMode = 3;
				_dogDeathMode = 2;
			}
			_scene->_dynamicHotspots.remove(_carHotspotId);
			int idx = _scene->_dynamicHotspots.add(0x324, VERB_WALKTO, -1, Common::Rect(99, 69, 99 + 82, 69 + 25));
			_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
			_game._player._stepEnabled = true;
			}
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_THROW, 0x2C, 0x410) || _action.isAction(VERB_THROW, 0x2D, 0x410)
					|| _action.isAction(VERB_THROW, 0x2D, 0x471) || _action.isAction(VERB_THROW, 0x2C, 0x471)) {
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
	} else if (_action.isAction(VERB_THROW, 0x2C, 0x411) || _action.isAction(VERB_THROW, 0x2D, 0x411)) {
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
	} else if (_action.isAction(VERB_THROW, 0x2D, 0x41D) || _action.isAction(VERB_THROW, 0x2C, 0x41D)
					|| _action.isAction(VERB_THROW, 0x2D, 0x41E) || _action.isAction(VERB_THROW, 0x2C, 0x41E)) {
		_game._player._stepEnabled = true;
		if ((_globals[kCarStatus] == 0) && _dogActiveFl) {
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
	} else if (_action.isAction(VERB_TAKE, 0x115) && (_game._trigger || !_game._objects.isInInventory(OBJ_POLYCEMENT))) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], true, 6, 2, 0, 0);
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
	} else if (_action.isAction(VERB_TAKE, 0x120) && (_game._trigger || !_game._objects.isInInventory(OBJ_REARVIEW_MIRROR))) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], true, 6, 2, 0, 0);
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
		else if (_globals[kDogStatus] == 4)
			_vm->_dialogs->show(60812);
		else
			_vm->_dialogs->show(60811);
	} else if (_action.isAction(VERB_LOOK) && (_action.isAction(0x412) || _action.isAction(0x408) || _action.isAction(0x414)
					|| _action.isAction(0x40D) || _action.isAction(0x41C)))
		_vm->_dialogs->show(60813);
	else if (_action.isAction(VERB_TAKE) && (_action.isAction(0x412) || _action.isAction(0x408) || _action.isAction(0x414)
					|| _action.isAction(0x40D) || _action.isAction(0x41C)))
		_vm->_dialogs->show(60814);
	else if (_action.isAction(VERB_LOOK, 0x409) || _action.isAction(VERB_LOOK, 0x411) || _action.isAction(VERB_LOOK, 0x410)) {
		if (_dogActiveFl)
			_vm->_dialogs->show(60815);
		else
			_vm->_dialogs->show(60816);
	} else if (_action.isAction(VERB_LOOK, 0x495))
		_vm->_dialogs->show(60817);
	else if (_action.isAction(VERB_TAKE, 0x495)) {
		if (_game._difficulty == DIFFICULTY_HARD)
			_vm->_dialogs->show(60818);
		else
			_vm->_dialogs->show(60819);
	} else if (_action.isAction(VERB_LOOK, 0x402))
		_vm->_dialogs->show(60820);
	else if (_action.isAction(VERB_LOOK, 0x403))
		_vm->_dialogs->show(60821);
	else if (_action.isAction(VERB_LOOK, 0x41A))
		_vm->_dialogs->show(60822);
	else if (_action.isAction(VERB_LOOK, 0x418))
		_vm->_dialogs->show(60823);
	else if (_action.isAction(VERB_LOOK, 0x330)) {
		if (_game._objects[OBJ_REARVIEW_MIRROR]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60825);
		else
			_vm->_dialogs->show(60824);
	} else if (_action.isAction(VERB_OPEN, 0x330))
		_vm->_dialogs->show(60826);
	else if (_action.isAction(VERB_LOOK, 0x120) && (_action._mainObjectSource == 4))
		_vm->_dialogs->show(60828);
	else if (_action.isAction(VERB_LOOK, 0x406)) {
		if (_game._objects[OBJ_POLYCEMENT]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60829);
		else
			_vm->_dialogs->show(60830);
	} else if (_action.isAction(VERB_OPEN, 0x406))
		_vm->_dialogs->show(60831);
	else if ((_action.isAction(VERB_LOOK, 0x115)) && (_game._objects.isInRoom(OBJ_POLYCEMENT)))
		_vm->_dialogs->show(60832);
	else if (_action.isAction(VERB_LOOK, 0x417) || _action.isAction(VERB_LOOK, 0x40E))
		_vm->_dialogs->show(60834);
	else if (_action.isAction(VERB_LOOK, 0x407))
		_vm->_dialogs->show(60835);
	else if (_action.isAction(VERB_LOOK, 0x47) || _action.isAction(VERB_LOOK, 0x4A0))
		_vm->_dialogs->show(60836);
	else if (_action.isAction(VERB_LOOK, 0x41E))
		_vm->_dialogs->show(60838);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

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
	} else if (_scene->_priorSceneId != -2) {
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
		_scene->_hotspots.activate(0x425, false);
		_game._player.walk(Common::Point(101, 100), FACING_EAST);
		_scene->_sequences.addTimer(180, 62);
		break;

	case 62:
		_scene->_sequences.remove( _globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_hotspots.activate(0x425, true);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 63);
		break;

	case 63:
		if (!_globals[kHasTalkedToHermit] && (_game._difficulty != DIFFICULTY_HARD)) {
			_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 26, 2, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[3], Common::Point(287, 73));
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
		_game._player._priorTimer = _scene->_activeAnimation->getNextFrameTimer() - _game._player._ticksAmount;
		_scene->_sequences.addTimer(6, 71);
		break;

	case 71:
		if (!_globals[kHasTalkedToHermit]) {
			_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 26, 2, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[3], Common::Point(287, 73));
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
		_globals._sequenceIndexes[5] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[5], true, 11, 2, 0, 0);
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
		_scene->_hotspots.activate(0x425, false);
		if (_videoDoorMode == 1) {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 180, _game.getQuote(0x307));
		}
		_game._player.walk(Common::Point(23, 90), FACING_WEST);
		_scene->_sequences.addTimer(180, 7);
		break;

	case 7:
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 8);
		break;

	case 8:
		_scene->_hotspots.activate(0x425, true);
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
	if (_action.isAction(0x17B, 0x6F, 0x425))
		_game._player.walk(Common::Point(78, 99), FACING_NORTHWEST);
}

void Scene609::actions() {
	if (_action.isAction(0x18C, 0x423))
		_scene->_nextSceneId = 611;
	else if (_action.isAction(0x18B, 0x425)) {
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
	} else if (_action.isAction(0x17B, 0x6F, 0x425)) {
		_videoDoorMode = 1;
		enterStore();
	} else if (_action.isAction(0x325, 0x324)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
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
	else if (_action.isAction(VERB_LOOK, 0x31E))
		_vm->_dialogs->show(60911);
	else if (_action.isAction(VERB_LOOK, 0x421))
		_vm->_dialogs->show(60912);
	else if (_action.isAction(VERB_LOOK, 0x427))
		_vm->_dialogs->show(60913);
	else if (_action.isAction(VERB_LOOK, 0x33A))
		_vm->_dialogs->show(60914);
	else if (_action.isAction(VERB_LOOK, 0x2CD))
		_vm->_dialogs->show(60915);
	else if (_action.isAction(VERB_LOOK, 0x324))
		_vm->_dialogs->show(60916);
	else if (_action.isAction(VERB_LOOK, 0x41F))
		_vm->_dialogs->show(60917);
	else if (_action.isAction(VERB_LOOK, 0x425)) {
		if (!_globals[kBeenInVideoStore])
			_vm->_dialogs->show(60918);
		else
			_vm->_dialogs->show(60919);
	} else if (_action.isAction(0x1AD, 0x31E))
		_vm->_dialogs->show(60730);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene610::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(0x108);
	_scene->addActiveVocab(0xD);
}

void Scene610::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('p', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMRC_9");
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 1));

	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 60, 0, 0, 0);
	_scene->_sequences.setDepth (_globals._sequenceIndexes[4], 13);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 30, 0, 0, 0);
	_scene->_sequences.setDepth (_globals._sequenceIndexes[3], 9);

	if (!_game._visitedScenes._sceneRevisited)
		_cellCharging = false;

	if (_game._objects[OBJ_PHONE_HANDSET]._roomNumber == _scene->_currentSceneId) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
		_handsetHotspotId = _scene->_dynamicHotspots.add(0x108, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_handsetHotspotId, Common::Point(132, 121), FACING_NORTHWEST);
		if ((_globals[kHandsetCellStatus] == 2) && (_game._difficulty == DIFFICULTY_HARD) && !_globals[kDurafailRecharged])
			_globals[kHandsetCellStatus] = 1;
	}

	if (_scene->_roomChanged && _game._difficulty != DIFFICULTY_EASY)
		_game._objects.addToInventory(OBJ_PENLIGHT);

	if (_scene->_priorSceneId != -2) {
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
	if (_action.isAction(0x1CE, 0x427))
		_scene->_nextSceneId = 609;
	else if (_action.isAction(VERB_TAKE, 0x108)) {
		if ( _game._trigger || !_game._objects.isInInventory(OBJ_PHONE_HANDSET)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], true, 8, 1, 0, 0);
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
	} else if (_action.isAction(VERB_PUT, 0x108, 0x42F)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], true, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 2, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
			_handsetHotspotId = _scene->_dynamicHotspots.add(0x108, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
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
	} else if (_action.isAction(VERB_LOOK, 0x442))
		_vm->_dialogs->show(61010);
	else if (_action.isAction(VERB_LOOK, 0x44B))
		_vm->_dialogs->show(61011);
	else if (_action.isAction(VERB_LOOK, 0x441))
		_vm->_dialogs->show(61012);
	else if (_action.isAction(VERB_LOOK, 0x43C))
		_vm->_dialogs->show(61013);
	else if (_action.isAction(VERB_LOOK, 0x427))
		_vm->_dialogs->show(61014);
	else if (_action._lookFlag)
		_vm->_dialogs->show(61015);
	else if (_action.isAction(VERB_LOOK, 0x4A1))
		_vm->_dialogs->show(61018);
	else if (_action.isAction(VERB_LOOK, 0x498)) {
		if (_game._visitedScenes.exists(601))
			_vm->_dialogs->show(61020);
		else
			_vm->_dialogs->show(61019);
	} else if (_action.isAction(VERB_LOOK, 0x29F))
		_vm->_dialogs->show(61021);
	else if (_action.isAction(VERB_LOOK, 0x429))
		_vm->_dialogs->show(61022);
	else if (_action.isAction(VERB_LOOK, 0x42D))
		_vm->_dialogs->show(61023);
	else if (_action.isAction(VERB_TAKE, 0x42D))
		_vm->_dialogs->show(61024);
	else if (_action.isAction(VERB_LOOK, 0x431))
		_vm->_dialogs->show(61025);
	else if (_action.isAction(VERB_LOOK, 0x108) && (_action._mainObjectSource == 4))
		_vm->_dialogs->show(61026);
	else if (_action.isAction(VERB_LOOK, 0x42F))
		_vm->_dialogs->show(61027);
	else if (_action.isAction(VERB_LOOK, 0x42A))
		_vm->_dialogs->show(61028);
	else if (_action.isAction(VERB_PUT, 0x42A)
		&& _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId)))
		_vm->_dialogs->show(61029);
	else if ( _action.isAction(0x44F) || _action.isAction(0x444) || _action.isAction(0x430)
		|| _action.isAction(0x44C) || _action.isAction(0x44D) || _action.isAction(0x446)
		|| _action.isAction(0x497) || _action.isAction(0x449) || _action.isAction(0x44E)
		|| _action.isAction(0x450) || _action.isAction(0x42C)) {
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
