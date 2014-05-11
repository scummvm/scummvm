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
			_game._objects.setRoom(OBJ_BONE, NOWHERE);
			if (_animationMode == 1)
				_globals[kBone202Status] = 0;
		} else { 
			_game._objects.setRoom(OBJ_BONES, NOWHERE);
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

void Scene620::setup() {
	_game._player._spritesPrefix = "";
	setAAName();
}

void Scene620::enter() {
	_globals._spriteIndexes[1]  = _scene->_sprites.addSprites(formAnimName('b', 0));
	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
	_game._player._stepEnabled = false;
	_game._player._visible   = false;
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
