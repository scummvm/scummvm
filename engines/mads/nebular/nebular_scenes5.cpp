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
#include "mads/nebular/nebular_scenes5.h"

namespace MADS {

namespace Nebular {

void Scene5xx::setAAName() {
	_game._aaName = Resources::formatAAName(5);
}

void Scene5xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);

	Common::String oldName = _game._player._spritesPrefix;

	if ((_scene->_nextSceneId == 502) || (_scene->_nextSceneId == 504) || (_scene->_nextSceneId == 505) || (_scene->_nextSceneId == 515))
		_game._player._spritesPrefix = "";
	else if (_globals[kSexOfRex] == REX_MALE)
		_game._player._spritesPrefix = "RXM";
	else if ((_scene->_nextSceneId == 501) || (_scene->_nextSceneId == 503) || (_scene->_nextSceneId == 551))
		_game._player._spritesPrefix = "ROX";

	_game._player._scalingVelocity = true;

	if ((_scene->_nextSceneId == 512) || (_scene->_nextSceneId == 507))
		_game._player._scalingVelocity = false;

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

void Scene5xx::sceneEntrySound() {
	if (!_vm->_musicFlag) {
		_vm->_sound->command(2);
		return;
	}

	switch (_scene->_nextSceneId) {
	case 501:
	case 502:
	case 504:
	case 505:
	case 506:
	case 507:
	case 508:
	case 511:
	case 512:
	case 513:
	case 515:
	case 551:
		if (_scene->_priorSceneId == 503)
			_vm->_sound->command(38);
		else
			_vm->_sound->command(29);
		break;
	case 503:
		_vm->_sound->command(41);
		break;
	default:
		break;
	}
}

/*------------------------------------------------------------------------*/

Scene501::Scene501(MADSEngine *vm) : Scene5xx(vm) {
	_mainSequenceId = -1;
	_mainSpriteId = -1;
	_doorHotspotid = -1;

	_rexPunched = false;
}

void Scene501::synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsSint16LE(_mainSequenceId);
	s.syncAsSint16LE(_mainSpriteId);
	s.syncAsSint16LE(_doorHotspotid);
	s.syncAsByte(_rexPunched);
}

void Scene501::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_DOOR);
	_scene->addActiveVocab(VERB_WALK_THROUGH);
}

void Scene501::handleSlotActions() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		int numTicks, frameIndex;
		if (_globals[kSexOfRex] == REX_MALE) {
			_mainSpriteId = _globals._spriteIndexes[4];
			numTicks = 8;
			frameIndex = 3;
		} else {
			_mainSpriteId = _globals._spriteIndexes[5];
			numTicks = 10;
			frameIndex = 2;
		}

		_mainSequenceId = _scene->_sequences.startPingPongCycle(_mainSpriteId, false, numTicks, 1, 0, 0);
		_scene->_sequences.setAnimRange(_mainSequenceId, 1, frameIndex);
		_scene->_sequences.setMsgLayout(_mainSequenceId);
		_vm->_sound->command(10);
		_scene->_sequences.addSubEntry(_mainSequenceId, SEQUENCE_TRIGGER_SPRITE, frameIndex, 1);
		_scene->_sequences.addSubEntry(_mainSequenceId, SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		break;

	case 1:
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 12, 6, 0, 0);
		break;

	case 2:
		_scene->_sequences.updateTimeout(-1, _mainSequenceId);
		_game._player._visible = true;
		_scene->_sequences.addTimer(15, 3);
		break;

	case 3:
		_game._player.walk(Common::Point(282, 110), FACING_NORTH);
		_scene->_sequences.addTimer(60, 4);
		break;

	default:
		break;
	}
}

void Scene501::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 0));

	if (_globals[kSexOfRex] == REX_MALE) {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXMRC_9");
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*RXCD_7");
	} else {
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*ROXRC_9");
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites("*ROXCD_7");
	}

	_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	int idx = _scene->_dynamicHotspots.add(NOUN_DOOR, VERB_WALK_THROUGH, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
	_doorHotspotid = _scene->_dynamicHotspots.setPosition(idx,Common::Point(282, 110), FACING_NORTH);
	_scene->_dynamicHotspots.setCursor(_doorHotspotid, CURSOR_GO_UP);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
	_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
	_rexPunched = true;

	if (_scene->_priorSceneId == 504) {
		_game._player._stepEnabled = false;
		_game._player._playerPos = Common::Point(74, 121);
		_game._player._facing = FACING_NORTHWEST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
		if (_globals[kSexOfRex] == REX_MALE)
			_scene->loadAnimation(formAnimName('G', 2), 70);
		else
			_scene->loadAnimation(formAnimName('R', 2), 70);
	} else if (_scene->_priorSceneId == 503) {
		_game._player._playerPos = Common::Point(317, 102);
		_game._player._facing = FACING_SOUTHWEST;
		_scene->_sequences.addTimer(15, 80);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(299, 131);

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_FAKE_ID);
		_game._objects.addToInventory(OBJ_SECURITY_CARD);
		_game._objects.addToInventory(OBJ_ID_CARD);
	}

	sceneEntrySound();
	_game.loadQuoteSet(0x275, 0x276, 0x277, 0);

	if (!_game._visitedScenes._sceneRevisited)
		_scene->_sequences.addTimer(2, 90);
}

void Scene501::step() {
	if (_game._trigger == 90)
		_vm->_dialogs->show(50127);

	if (_game._trigger >= 80) {
		switch (_game._trigger) {
		case 80:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 12, 6, 0, 0);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_vm->_sound->command(11);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
			break;

		case 81:
			_scene->_dynamicHotspots.remove(_doorHotspotid);
			_game._player.walk(Common::Point(276, 110), FACING_SOUTHWEST);
			_scene->_sequences.addTimer(120, 82);
			break;

		case 82:
			_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_vm->_sound->command(12);
			_doorHotspotid = _scene->_dynamicHotspots.add(NOUN_DOOR, VERB_WALK_THROUGH, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(_globals._sequenceIndexes[3], Common::Point(282, 110), FACING_NORTH);
			_scene->_dynamicHotspots.setCursor(_doorHotspotid, CURSOR_GO_UP);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 83);
			break;

		case 83:
			_game._player._stepEnabled = true;
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			break;

		default:
			break;
		}
	}

	if (_game._trigger >= 70 && _game._trigger <= 73) {
		switch (_game._trigger) {
		case 70:
			_game._player._visible = true;
			_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
			_scene->_sequences.addTimer(15, 71);
			break;

		case 71:
			_game._player.walk(Common::Point(92, 130), FACING_SOUTH);
			_scene->_sequences.addTimer(30, 72);
			break;

		case 72:
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 73);
			break;

		case 73:
			_game._player._stepEnabled = true;
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			break;

		default:
			break;
		}
	}
}

void Scene501::preActions() {
	if (_action.isAction(VERB_WALK_DOWN) && (_action.isObject(NOUN_STREET_TO_EAST) || _action.isObject(NOUN_SIDEWALK_TO_EAST)))
		_game._player._walkOffScreenSceneId = 551;
}

void Scene501::actions() {
	if (_action.isAction(VERB_GET_INTO, NOUN_CAR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1: {
			int syncIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], syncIdx);
			_scene->_sequences.addTimer(15, 2);
			}
			break;

		case 2:
			_game._player._visible = false;
			if (_globals[kSexOfRex] == REX_MALE)
				_mainSpriteId = _globals._spriteIndexes[6];
			else
				_mainSpriteId = _globals._spriteIndexes[7];

			_mainSequenceId = _scene->_sequences.addSpriteCycle(_mainSpriteId, false, 8, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_mainSequenceId);
			_scene->_sequences.addSubEntry(_mainSequenceId, SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3: {
			int syncIdx = _mainSequenceId;
			_mainSequenceId = _scene->_sequences.startCycle(_mainSpriteId, false, -2);
			_scene->_sequences.setMsgLayout(_mainSequenceId);
			_scene->_sequences.updateTimeout(_mainSequenceId, syncIdx);
			_scene->_sequences.addTimer(30, 4);
			}
			break;

		case 4:
			_scene->_nextSceneId = 504;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PUT, NOUN_SECURITY_CARD, NOUN_CARD_SLOT))
		_vm->_dialogs->show(50113);
	else if (_action.isAction(VERB_PUT, NOUN_FAKE_ID, NOUN_CARD_SLOT)) {
		switch (_game._trigger) {
		case 0:
		case 1:
		case 2:
		case 3:
			handleSlotActions();
			break;

		case 4:
			if (_globals[kSexOfRex] == REX_MALE) {
				_game._player._visible = false;
				_vm->_sound->command(13);
				_scene->loadAnimation(formAnimName('G', 1), 5);
			} else {
				_rexPunched = false;
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 6, 120, _game.getQuote(0x277));
			}
			break;

		case 5:
			_game._player._visible = true;
			_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
			_scene->_sequences.addTimer(30, 6);
			break;

		case 6:
			if (_globals[kSexOfRex] == REX_MALE) {
				if (_rexPunched) {
					_scene->_kernelMessages.reset();
					_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x275));
					_rexPunched = false;
				} else {
					_scene->_kernelMessages.reset();
					_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x276));
				}
			}
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PUT, NOUN_ID_CARD, NOUN_CARD_SLOT)) {
		switch (_game._trigger) {
		case 0:
		case 1:
		case 2:
		case 3:
			handleSlotActions();
			break;

		case 4:
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_scene->_dynamicHotspots.remove(_doorHotspotid);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_vm->_sound->command(11);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 6);
			break;

		case 6:
			_game._player.walk(Common::Point(317, 102), FACING_NORTHEAST);
			_scene->_sequences.addTimer(120, 7);
			break;

		case 7: {
			_vm->_sound->command(12);
			int syncIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
			_vm->_sound->command(12);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 8);
			}
			break;

		case 8: {
			int syncIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
			_scene->_nextSceneId = 503;
			}
			break;

		default:
			break;
		}
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_STREET))
		_vm->_dialogs->show(50121);
	else if (_action.isAction(VERB_LOOK, NOUN_DOOR))
		_vm->_dialogs->show(50110);
	else if (_action.isAction(VERB_LOOK, NOUN_CARD_SLOT))
		_vm->_dialogs->show(50112);
	else if (_action.isAction(VERB_LOOK, NOUN_SIGN))
		_vm->_dialogs->show(50114);
	else if (_action.isAction(VERB_TAKE, NOUN_SIGN))
		_vm->_dialogs->show(50115);
	else if (_action.isAction(VERB_LOOK, NOUN_SIDEWALK_TO_EAST))
		_vm->_dialogs->show(50118);
	else if (_action.isAction(VERB_LOOK, NOUN_SIDEWALK_TO_WEST) || _action.isAction(VERB_LOOK, NOUN_STREET_TO_WEST)
		|| _action.isAction(VERB_WALK_DOWN, NOUN_SIDEWALK_TO_WEST) || _action.isAction(VERB_WALK_DOWN, NOUN_STREET_TO_WEST))
		_vm->_dialogs->show(50119);
	else if (_action.isAction(VERB_LOOK, NOUN_BUILDING))
		_vm->_dialogs->show(50120);
	else if (_action.isAction(VERB_OPEN, NOUN_DOOR))
		_vm->_dialogs->show(50122);
	else if (_action.isAction(VERB_LOOK, NOUN_FIRE_HYDRANT))
		_vm->_dialogs->show(50123);
	else if (_action.isAction(VERB_OPEN, NOUN_FIRE_HYDRANT))
		_vm->_dialogs->show(50124);
	else if (_action.isAction(VERB_LOOK, NOUN_EQUIPMENT_OVERHEAD))
		_vm->_dialogs->show(50125);
	else if (_action.isAction(VERB_LOOK, NOUN_PIPES) || _action.isAction(VERB_LOOK, NOUN_PIPE))
		_vm->_dialogs->show(50126);
	else if (_action.isAction(VERB_LOOK, NOUN_CAR)) {
		if (!_game._visitedScenes.exists(504))
			_vm->_dialogs->show(50116);
		else
			_vm->_dialogs->show(50117);
	} else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene502::setup() {
	_game._player._spritesPrefix = "";
	// The original is using Scene5xx_setAAName()
	_game._aaName = Resources::formatAAName(5);
}

void Scene502::enter() {
	if (_globals[kSexOfRex] == REX_MALE)
		_handSpriteId = _scene->_sprites.addSprites("*REXHAND");
	else
		_handSpriteId = _scene->_sprites.addSprites("*ROXHAND");

	teleporterEnter();

	// The original uses scene5xx_sceneEntrySound
	if (!_vm->_musicFlag)
		_vm->_sound->command(2);
	else if (_scene->_priorSceneId == 503)
		_vm->_sound->command(38);
	else
		_vm->_sound->command(29);
}

void Scene502::step() {
	teleporterStep();
}

void Scene502::actions() {
	if (teleporterActions()) {
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_VIEWPORT) || _action.isAction(VERB_PEER_THROUGH, NOUN_VIEWPORT))
		_vm->_dialogs->show(50210);
	else if (_action.isAction(VERB_LOOK, NOUN_KEYPAD))
		_vm->_dialogs->show(50211);
	else if (_action.isAction(VERB_LOOK, NOUN_DISPLAY))
		_vm->_dialogs->show(50212);
	else if (_action.isAction(VERB_LOOK, NOUN_0_KEY) || _action.isAction(VERB_LOOK, NOUN_1_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_2_KEY) || _action.isAction(VERB_LOOK, NOUN_3_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_4_KEY) || _action.isAction(VERB_LOOK, NOUN_5_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_6_KEY) || _action.isAction(VERB_LOOK, NOUN_7_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_8_KEY) || _action.isAction(VERB_LOOK, NOUN_9_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_SMILE_KEY) || _action.isAction(VERB_LOOK, NOUN_ENTER_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_FROWN_KEY))
		_vm->_dialogs->show(50213);
	else if (_action.isAction(VERB_LOOK, NOUN_DEVICE) || _action._lookFlag)
		_vm->_dialogs->show(50214);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene503::Scene503(MADSEngine *vm) : Scene5xx(vm) {
	_detonatorHotspotId = -1;
}

void Scene503::synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsSint16LE(_detonatorHotspotId);
}

void Scene503::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_DETONATORS);
	_scene->addActiveVocab(VERB_WALKTO);
}

void Scene503::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', -1));

	if (_globals[kSexOfRex] == REX_MALE)
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMBD_2");
	else
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*ROXBD_2");

	if (_game._objects[OBJ_DETONATORS]._roomNumber == _scene->_currentSceneId) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
		_detonatorHotspotId = _scene->_dynamicHotspots.add(NOUN_DETONATORS, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_detonatorHotspotId, Common::Point(254, 135), FACING_SOUTH);
	}

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(191, 152);
		_game._player._facing = FACING_NORTHWEST;
	}

	sceneEntrySound();
}

void Scene503::actions() {
	if (_action.isAction(VERB_WALK, NOUN_OUTSIDE))
		_scene->_nextSceneId = 501;
	else if (_action.isAction(VERB_TAKE, NOUN_DETONATORS)) {
		if ( _game._trigger || !_game._objects.isInInventory(OBJ_DETONATORS)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				if (_globals[kSexOfRex] == REX_MALE) {
					_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 3);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 3, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				} else {
					_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], true, 8, 1, 0, 0);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 4, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				}
				break;

			case 1:
				_vm->_sound->command(9);
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_scene->_dynamicHotspots.remove(_detonatorHotspotId);
				_game._objects.addToInventory(OBJ_DETONATORS);
				_vm->_dialogs->showItem(OBJ_DETONATORS, 50326);
				break;

			case 2:
				if (_globals[kSexOfRex] == REX_MALE)
					_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
				else
					_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);

				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
	} else if (_action._lookFlag)
		_vm->_dialogs->show(50328);
	else if (_action.isAction(VERB_LOOK, NOUN_MONITORING_EQUIPMENT))
		_vm->_dialogs->show(50310);
	else if (_action.isAction(VERB_LOOK, NOUN_PHOTON_RIFLES))
		_vm->_dialogs->show(50311);
	else if (_action.isAction(VERB_TAKE, NOUN_PHOTON_RIFLES) || _action.isAction(VERB_TAKE, NOUN_NUCLEAR_SLINGSHOT))
		_vm->_dialogs->show(50312);
	else if (_action.isAction(VERB_LOOK, NOUN_DISPLAY_CASE))
		_vm->_dialogs->show(50313);
	else if (_action.isAction(VERB_LOOK, NOUN_NUCLEAR_SLINGSHOT))
		_vm->_dialogs->show(50314);
	else if (_action.isAction(VERB_LOOK, NOUN_WATER_COOLER))
		_vm->_dialogs->show(50315);
	else if (_action.isAction(VERB_LOOK, NOUN_STORAGE_BOX))
		_vm->_dialogs->show(50316);
	else if (_action.isAction(VERB_OPEN, NOUN_STORAGE_BOX))
		_vm->_dialogs->show(50317);
	else if (_action.isAction(VERB_LOOK, NOUN_WARNING_LABEL))
		_vm->_dialogs->show(50318);
	else if (_action.isAction(VERB_LOOK, NOUN_DESK))
		_vm->_dialogs->show(50319);
	else if (_action.isAction(VERB_LOOK, NOUN_MONITOR))
		_vm->_dialogs->show(50320);
	else if (_action.isAction(VERB_LOOK, NOUN_FILE_CABINETS))
		_vm->_dialogs->show(50322);
	else if (_action.isAction(VERB_LOOK, NOUN_BOX)) {
		if (_game._objects.isInRoom(OBJ_DETONATORS))
			_vm->_dialogs->show(50323);
		else
			_vm->_dialogs->show(50324);
	} else if (_action.isAction(VERB_LOOK, NOUN_DETONATORS) && (_action._savedFields._mainObjectSource == 4))
		_vm->_dialogs->show(50325);
	else if (_action.isAction(VERB_LOOK, NOUN_WINDOWS))
		_vm->_dialogs->show(50327);
	else if (_action.isAction(VERB_OPEN, NOUN_DISPLAY_CASE))
		_vm->_dialogs->show(50329);
	else if (_action.isAction(VERB_THROW, NOUN_DISPLAY_CASE) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId)))
		_vm->_dialogs->show(50330);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene504::Scene504(MADSEngine *vm) : Scene5xx(vm) {
	_carAnimationMode = -1;
	_carFrame = -1;
}

void Scene504::synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsSint16LE(_carAnimationMode);
	s.syncAsSint16LE(_carFrame);
}

void Scene504::setup() {
	_game._player._spritesPrefix = "";
	setAAName();
}

void Scene504::enter() {
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 2));

	for (int i = 0; i < 4; i++)
		_globals._spriteIndexes[5 + i] = _scene->_sprites.addSprites(formAnimName('m', i));

	if (_globals[kSexOfRex] == REX_MALE)
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 0));
	else {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 1));
		_scene->changeVariant(1);
	}

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 0);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 0, 0, 0);
	_carFrame = -1;

	if ((_scene->_priorSceneId == 505) && (_globals[kHoverCarDestination] != _globals[kHoverCarLocation])){
		_carAnimationMode = 1;
		_scene->loadAnimation(formAnimName('A', -1));
		_vm->_sound->command(14);
		_scene->_sequences.addTimer(1, 70);
		_game._player._stepEnabled = false;
	} else {
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 3));
		_carAnimationMode = 1;
		_scene->loadAnimation(formAnimName('A', -1));
		if ((_scene->_priorSceneId != RETURNING_FROM_DIALOG) && (_scene->_priorSceneId != 505))
			_globals[kHoverCarLocation] = _scene->_priorSceneId;

		_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, 1);
	}

	if (_globals[kTimebombTimer] > 10500)
		_globals[kTimebombTimer] = 10500;

	sceneEntrySound();
}

void Scene504::step() {
	if ((_carAnimationMode == 1) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame;

			if (_carFrame == 1)
				nextFrame = 0;
			else
				nextFrame = -1;

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				_carFrame = nextFrame;
			}
		}
	}


	if (_game._trigger >= 70) {
		switch (_game._trigger) {
		case 70:
			if (_globals[kHoverCarDestination] != -1) {
				_game._player._stepEnabled = false;
				_scene->freeAnimation();
				_carAnimationMode = 2;
				if (((_globals[kHoverCarLocation] >= 500 && _globals[kHoverCarLocation] <= 599) &&
					(_globals[kHoverCarDestination] >= 500 && _globals[kHoverCarDestination] <= 599)) ||
					((_globals[kHoverCarLocation] >= 600 && _globals[kHoverCarLocation] <= 699) &&
					(_globals[kHoverCarDestination] >= 600 && _globals[kHoverCarDestination] <= 699))) {
						_scene->loadAnimation(formAnimName('A', -1), 71);
				} else if (_globals[kHoverCarLocation] > _globals[kHoverCarDestination])
					_scene->loadAnimation(formAnimName('C', -1), 71);
				else
					_scene->loadAnimation(formAnimName('B', -1), 71);
			}
			break;

		case 71:
			_vm->_sound->command(15);
			_scene->_nextSceneId = _globals[kHoverCarDestination];
			break;

		default:
			break;
		}
	}

	if ((_globals[kTimebombTimer] >= 10800) && (_globals[kTimebombStatus] == TIMEBOMB_ACTIVATED) && (_game._difficulty != 3)) {
		_globals[kTimebombStatus] = TIMEBOMB_DEAD;
		_globals[kTimebombTimer] = 0;
		_globals[kCheckDaemonTimebomb] = false;
		_scene->_nextSceneId = 620;
	}
}

void Scene504::preActions() {
	_game._player._needToWalk = false;
}

void Scene504::actions() {
	if (_action.isAction(VERB_EXIT_FROM, NOUN_CAR)) {
		_vm->_sound->command(15);
		_scene->_nextSceneId = _globals[kHoverCarLocation];
	} else if (_action.isAction(VERB_ACTIVATE, NOUN_CAR_CONTROLS)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_vm->_sound->command(39);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 13);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_scene->_sequences.remove(_globals._sequenceIndexes[7]);
			_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 18, 0, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 14);
			break;

		case 1: {
			int syncIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 13);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 6);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
			}
			break;

		case 2:
			_scene->_sequences.addTimer(10, 3);
			break;

		case 3:
			_scene->_sequences.remove(_globals._sequenceIndexes[5]);
			if (_globals[kSexOfRex] == REX_MALE) {
				_vm->_sound->command(34);
				_scene->_sequences.addTimer(60, 4);
				_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 14);
			} else {
				_vm->_sound->command(40);
				_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 18, 0, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 14);
				_scene->_sequences.addTimer(120, 5);
			}
			break;

		case 4:
			_game._player._stepEnabled = true;
			_globals[kHoverCarDestination] = _globals[kHoverCarLocation];
			_scene->_nextSceneId = 505;
			break;

		case 5:
			_game._player._stepEnabled = true;
			_scene->_sequences.remove(_globals._sequenceIndexes[8]);
			_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, 1);
			_vm->_dialogs->show(50421);
			break;

		default:
			break;
		}
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_INTERIOR_OF_CAR))
		_vm->_dialogs->show(50412);
	else if (_action.isAction(VERB_LOOK, NOUN_GLOVE_COMPARTMENT))
		_vm->_dialogs->show(50410);
	else if (_action.isAction(VERB_LOOK, NOUN_CAR_CONTROLS) || _action.isAction(VERB_LOOK, NOUN_DASHBOARD))
		_vm->_dialogs->show(50411);
	else if (_action.isAction(VERB_LOOK, NOUN_SCENT_PACKET))
		_vm->_dialogs->show(50413);
	else if (_action.isAction(VERB_LOOK, NOUN_SODA_CANS))
		_vm->_dialogs->show(50414);
	else if (_action.isAction(VERB_LOOK, NOUN_KITTY))
		_vm->_dialogs->show(50415);
	else if (_action.isAction(VERB_LOOK, NOUN_WINDSHIELD) || _action.isAction(VERB_LOOK_THROUGH, NOUN_WINDSHIELD))
		_vm->_dialogs->show(50416);
	else if (_action.isAction(VERB_LOOK, NOUN_REARVIEW_MIRROR))
		_vm->_dialogs->show(50417);
	else if (_action.isAction(VERB_TAKE, NOUN_REARVIEW_MIRROR))
		_vm->_dialogs->show(50418);
	else if (_action.isAction(VERB_LOOK, NOUN_MOLDY_SOCK))
		_vm->_dialogs->show(50419);
	else if (_action.isAction(VERB_TAKE, NOUN_MOLDY_SOCK))
		_vm->_dialogs->show(50420);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene505::Scene505(MADSEngine *vm) : Scene5xx(vm) {
	_frame = -1;
	_nextButtonId = -1;
	_homeSelectedId = -1;
	_selectedId = -1;
	_activeCars = -1;

	for (int i = 0; i < 9; i++)
		_carLocations[i] = -1;
}

void Scene505::synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsSint16LE(_frame);
	s.syncAsSint16LE(_nextButtonId);
	s.syncAsSint16LE(_homeSelectedId);
	s.syncAsSint16LE(_selectedId);
	s.syncAsSint16LE(_activeCars);

	for (int i = 0; i < 9; i++)
		s.syncAsSint16LE(_carLocations[i]);
}

void Scene505::setup() {
	_game._player._spritesPrefix = "";
	setAAName();
}

void Scene505::enter() {
	for (int i = 0; i < 9; i++)
		_globals._spriteIndexes[i] = _scene->_sprites.addSprites(formAnimName('a', i + 1));

	_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('b', 1));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('g', 1));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('g', 0));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('t', -1));
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('e', -1));

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_globals._sequenceIndexes[12] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[12], false, 6, 1, 0, 0);

	_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[13], false, 6, 1, 120, 0);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 60);
	_scene->_sequences.addTimer(30, 62);

	_carLocations[0] = 501;
	_carLocations[1] = 506;
	_carLocations[2] = 511;
	_carLocations[3] = 513;
	_carLocations[4] = 601;
	_carLocations[5] = 604;
	_carLocations[6] = 607;
	_carLocations[7] = 609;
	_carLocations[8] = 612;

	_activeCars = false;

	for (int i = 0; i < 9; i++) {
		if (_globals[kHoverCarLocation] == _carLocations[i]) {
			_homeSelectedId = i;
			if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
				_selectedId = i;
		}
	}

	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_frame = -1;
	_scene->loadAnimation(formAnimName('a', -1));
	_scene->_animation[0]->setCurrentFrame(86);

	sceneEntrySound();
	_vm->_sound->command(16);
}

void Scene505::step() {
	if (_frame != _scene->_animation[0]->getCurrentFrame()) {
		_frame = _scene->_animation[0]->getCurrentFrame();
		int resetFrame = -1;

		switch (_frame) {
		case 4:
		case 24:
		case 33:
		case 53:
		case 62:
		case 82:
			if (_nextButtonId == 0x38A)
				resetFrame = 4;
			else if (_nextButtonId == 0x38B)
				resetFrame = 33;
			else if (_nextButtonId == 0x2DE)
				resetFrame = 62;

			break;

		case 15:
		case 44:
		case 73: {
			int this_button;
			int old_select;
			_vm->_sound->command(17);
			old_select = _selectedId;
			if (_frame == 15) {
				this_button = 0x38A;
				_selectedId = (_selectedId + 1) % 9;
			} else if (_frame == 44) {
				this_button = 0x38B;
				_selectedId--;
				if (_selectedId < 0)
					_selectedId = 8;
			} else {
				this_button = 0x2DE;
				if ((_globals[kTimebombStatus] == TIMEBOMB_ACTIVATED) && (_carLocations[_selectedId] == 501))
					_vm->_dialogs->show(431);
				else if (_selectedId != _homeSelectedId) {
					_nextButtonId = 0;
					_activeCars = true;
					_game._player._stepEnabled = false;
					_scene->_sequences.remove(_globals._sequenceIndexes[1]);
					_scene->_sequences.remove(_globals._sequenceIndexes[0]);
					_scene->_sequences.remove(_globals._sequenceIndexes[13]);
					_globals._sequenceIndexes[13] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[13], false, 6, 1, 0, 0);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 63);
					_vm->_sound->command(18);
				}
			}

			if (_nextButtonId == this_button)
				_nextButtonId = 0;

			if (old_select != _selectedId) {
				_scene->_sequences.remove(_globals._sequenceIndexes[11]);
				_globals._sequenceIndexes[11] = _scene->_sequences.startCycle(_globals._spriteIndexes[11], false, _selectedId + 1);
				if (old_select != _homeSelectedId)
					_scene->_sequences.remove(_globals._sequenceIndexes[0]);

				if (_selectedId != _homeSelectedId) {
					_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0 + _selectedId], false, 24, 0, 0, 0);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 1);
				}
			}
			break;
			}

		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
			if (_nextButtonId == 0x38A)
				resetFrame = 29 - _frame;

			break;

		case 26:
		case 55:
		case 84:
			if (_nextButtonId != 0)
				resetFrame = 3;

			break;

		case 27:
		case 56:
		case 85:
			if (_nextButtonId != 0)
				resetFrame = 2;

			break;

		case 29:
		case 58:
		case 87:
			if (_activeCars)
				_globals[kHoverCarDestination] = _carLocations[_selectedId];

			if (_nextButtonId == 0x38A)
				resetFrame = 0;
			else if (_nextButtonId == 0x38B)
				resetFrame = 29;
			else if (_nextButtonId == 0x2DE)
				resetFrame = 58;
			else
				resetFrame = 86;
			break;

		case 47:
		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
			if (_nextButtonId == 0x38B)
				resetFrame = 87 - _frame;

			break;

		case 76:
		case 77:
		case 78:
		case 79:
		case 80:
		case 81:
			if (_nextButtonId == 0x2DE)
				resetFrame = 145 - _frame;

			break;

		default:
			break;
		}

		if ((resetFrame >= 0) && (resetFrame != _scene->_animation[0]->getCurrentFrame())) {
			_scene->_animation[0]->setCurrentFrame(resetFrame);
			_frame = resetFrame;
		}
	}

	switch (_game._trigger) {
	case 60: {
		_game._player._stepEnabled = true;
		int syncIdx = _globals._sequenceIndexes[13];
		_globals._sequenceIndexes[13] = _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[13], syncIdx);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[_homeSelectedId], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_globals._sequenceIndexes[11] = _scene->_sequences.startCycle(_globals._spriteIndexes[11], false, _selectedId + 1);

		if (_selectedId != _homeSelectedId) {
			_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0 + _selectedId], false, 24, 0, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 1);
		}
		break;
		}

	case 61:
		_globals._sequenceIndexes[10] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[10], false, 8, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 8);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[10], _globals._sequenceIndexes[9]);
		break;

	case 62:
		_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 8, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
		break;

	case 63:
		_globals[kHoverCarDestination] = _carLocations[_selectedId];
		_scene->_nextSceneId = 504;
		break;

	default:
		break;
	}
}

void Scene505::actions() {
	if (_action.isAction(VERB_PRESS))
		_nextButtonId = _action._activeAction._objectNameId;
	else if (_action.isAction(VERB_RETURN_TO, NOUN_INSIDE_OF_CAR))
		_scene->_nextSceneId = 504;
	else if (_action.isAction(VERB_LOOK, NOUN_VIEW_SCREEN))
		_vm->_dialogs->show(50510);
	else if (_action.isAction(VERB_LOOK, NOUN_CONTROL_PANEL))
		_vm->_dialogs->show(50511);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene506::Scene506(MADSEngine *vm) : Scene5xx(vm), _doorPos(0, 0) {
	_heroFacing = FACING_DUMMY;

	_doorDepth = -1;
	_doorSpriteIdx = -1;
	_doorSequenceIdx = -1;
	_doorWord = -1;

	_labDoorFl = false;
	_firstDoorFl = false;
	_actionFl = false;
}

void Scene506::synchronize(Common::Serializer &s) {
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

void Scene506::enter() {
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

	case 83: {
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

void Scene506::actions() {
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

		case 1: {
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

/*------------------------------------------------------------------------*/

Scene507::Scene507(MADSEngine *vm) : Scene5xx(vm) {
	_penlightHotspotId = -1;
}

void Scene507::synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsSint16LE(_penlightHotspotId);
}

void Scene507::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_PENLIGHT);
	_scene->addActiveVocab(VERB_WALKTO);
}

void Scene507::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('p', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMRD_3");

	if ((_game._difficulty != DIFFICULTY_EASY) && (_game._objects[OBJ_PENLIGHT]._roomNumber == _scene->_currentSceneId)) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
		_penlightHotspotId = _scene->_dynamicHotspots.add(NOUN_PENLIGHT, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_penlightHotspotId, Common::Point(233, 152), FACING_SOUTHEAST);
	}

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(121, 147);
		_game._player._facing = FACING_NORTH;
	}

	sceneEntrySound();
}
void Scene507::actions() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ENTRANCE))
		_scene->_nextSceneId = 506;
	else if (_action.isAction(VERB_TAKE, NOUN_PENLIGHT)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_PENLIGHT)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 5);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_scene->_dynamicHotspots.remove(_penlightHotspotId);
				_vm->_sound->command(27);
				_game._objects.addToInventory(OBJ_PENLIGHT);
				_vm->_dialogs->showItem(OBJ_PENLIGHT, 50730);
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
	} else if (_action._lookFlag)
		_vm->_dialogs->show(50722);
	else if (_action.isAction(VERB_LOOK, NOUN_SWIRLING_LIGHT))
		_vm->_dialogs->show(50710);
	else if (_action.isAction(VERB_TAKE, NOUN_SWIRLING_LIGHT))
		_vm->_dialogs->show(50711);
	else if (_action.isAction(VERB_LOOK, NOUN_OLD_SOFTWARE))
		_vm->_dialogs->show(50712);
	else if (_action.isAction(VERB_TAKE, NOUN_OLD_SOFTWARE))
		_vm->_dialogs->show(50713);
	else if (_action.isAction(VERB_LOOK, NOUN_ADVERTISEMENT))
		_vm->_dialogs->show(50714);
	else if (_action.isAction(VERB_LOOK, NOUN_ADVERTISING_POSTER))
		_vm->_dialogs->show(50715);
	else if (_action.isAction(VERB_LOOK, NOUN_SIGN)) {
		if (_scene->_customDest.x < 100)
			_vm->_dialogs->show(50726);
		else
			_vm->_dialogs->show(50716);
	} else if (_action.isAction(VERB_LOOK, NOUN_HOTTEST_SOFTWARE))
		_vm->_dialogs->show(50717);
	else if (_action.isAction(VERB_LOOK, NOUN_SOFTWARE_SHELF))
		_vm->_dialogs->show(50718);
	else if (_action.isAction(VERB_LOOK, NOUN_SENSOR))
		_vm->_dialogs->show(50719);
	else if (_action.isAction(VERB_LOOK, NOUN_CASH_REGISTER))
		_vm->_dialogs->show(50720);
	else if (_action.isAction(VERB_LOOK, NOUN_PAD_OF_PAPER))
		_vm->_dialogs->show(50721);
	else if (_action.isAction(VERB_OPEN, NOUN_CASH_REGISTER))
		_vm->_dialogs->show(50723);
	else if (_action.isAction(VERB_LOOK, NOUN_BARGAIN_VAT))
		_vm->_dialogs->show(50724);
	else if (_action.isAction(VERB_LOOK, NOUN_WINDOW))
		_vm->_dialogs->show(50725);
	else if (_action.isAction(VERB_WALK_BEHIND, NOUN_COUNTER)) {
		// WORKAROUND: Empty handling to prevent default "can't do that" dialogs showing
	} else if (_action.isAction(VERB_LOOK, NOUN_COUNTER)) {
		if (_game._objects.isInRoom(OBJ_PENLIGHT))
			_vm->_dialogs->show(50728);
		else
			_vm->_dialogs->show(50727);
	} else if (_action.isAction(VERB_LOOK, NOUN_PENLIGHT) && !_game._objects.isInInventory(OBJ_PENLIGHT)) {
		if (_game._objects.isInRoom(OBJ_PENLIGHT))
			_vm->_dialogs->show(50729);
	} else if (_action.isAction(VERB_LOOK, NOUN_EMERGENCY_LIGHT))
		_vm->_dialogs->show(50731);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene508::Scene508(MADSEngine *vm) : Scene5xx(vm) {
	_chosenObject = -1;
}

void Scene508::synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsSint16LE(_chosenObject);
}

void Scene508::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_SPINACH_PATCH_DOLL);
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_LASER_BEAM);
}

void Scene508::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('m', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('h', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('l', 2));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('t', 0));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*RXMRC_9");
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('l', 3));

	if (!_game._visitedScenes._sceneRevisited) {
		_globals[kLaserOn] = false;
		_chosenObject = 0;
	}

	if (!_globals[kLaserOn]) {
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, -2);
		int idx = _scene->_dynamicHotspots.add(NOUN_SPINACH_PATCH_DOLL, VERB_WALKTO, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(57, 116), FACING_NORTHEAST);
		_scene->_hotspots.activate(NOUN_HOLE, false);
		_scene->_hotspots.activate(NOUN_LASER_BEAM, false);
	} else {
		_scene->changeVariant(1);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 11);
		int idx = _scene->_dynamicHotspots.add(NOUN_LASER_BEAM, VERB_WALKTO, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(57, 116), FACING_NORTHEAST);
		_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 15, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 6, 8);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 6);
		if (_globals[kLaserHoleIsThere]) {
			_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, -2);
			_scene->_hotspots.activate(NOUN_HOLE, true);
			_scene->_hotspots.activate(NOUN_LASER_BEAM, true);
		}
		_vm->_sound->command(21);
	}
	_vm->_sound->command(20);

	if (_scene->_priorSceneId == 515) {
		_game._player._playerPos = Common::Point(57, 116);
		_game._player._facing = FACING_NORTHEAST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(289, 139);
		_game._player._facing = FACING_WEST;
	}

	sceneEntrySound();
	_game.loadQuoteSet(0x273, 0);

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_COMPACT_CASE);
		_game._objects.addToInventory(OBJ_REARVIEW_MIRROR);
	}
}

void Scene508::preActions() {
	if (_action.isAction(VERB_WALK, NOUN_OUTSIDE))
		_game._player._walkOffScreenSceneId = 506;
}

void Scene508::handlePedestral() {
	if (!_globals[kLaserOn])
		_vm->_dialogs->show(50835);

	if (_globals[kLaserHoleIsThere])
		_vm->_dialogs->show(50836);

	if (_globals[kLaserOn] && !_globals[kLaserHoleIsThere]) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[6] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[6], false, 9, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 4);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[6]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 1:
			if (_chosenObject == 2)
				_game._objects.removeFromInventory(OBJ_COMPACT_CASE, 1);
			else
				_game._objects.removeFromInventory(OBJ_REARVIEW_MIRROR, 1);

			_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, -2);
			_scene->_hotspots.activate(NOUN_HOLE, true);
			_scene->_hotspots.activate(NOUN_LASER_BEAM, true);
			break;

		case 3:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[6]);
			_game._player._visible = true;
			_scene->_sequences.addTimer(120, 4);
			break;

		case 4:
			_vm->_dialogs->show(50834);
			_globals[kLaserHoleIsThere] = true;
			_scene->_nextSceneId = 515;
			break;

		default:
			break;
		}
	}
}

void Scene508::actions() {
	if (_action.isAction(VERB_PULL, NOUN_LEVER)) {
		if (!_globals[kLaserOn]) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 2, 120, _game.getQuote(0x273));
				break;

			case 2:
				_game._player._visible = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[3]);
				_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 10, 1, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 7);
				_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], -1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				_vm->_sound->command(19);
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 15, 1, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 6);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
				_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[1]);
				_game._player._visible = true;
				_scene->_sequences.addTimer(15, 5);
				break;

			case 4:
				_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 15, 0, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 6, 8);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 6);
				break;

			case 5:
				_scene->_sequences.remove(_globals._sequenceIndexes[5]);
				_scene->loadAnimation(formAnimName('B', 1), 6);
				break;

			case 6: {
				_vm->_sound->command(22);
				_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 11);
				int idx = _scene->_dynamicHotspots.add(NOUN_LASER_BEAM, VERB_WALKTO, _globals._sequenceIndexes[4], Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots.setPosition(idx, Common::Point(57, 116), FACING_NORTHEAST);
				_scene->_kernelMessages.reset();
				_scene->changeVariant(1);
				_scene->_sequences.addTimer(30, 7);
				}
				break;

			case 7:
				_globals[kLaserOn] = true;
				_vm->_dialogs->show(50833);
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		} else {
			_vm->_dialogs->show(50837);
		}
	} else if (_action.isAction(VERB_REFLECT, NOUN_REARVIEW_MIRROR, NOUN_LASER_BEAM) || _action.isAction(VERB_PUT, NOUN_REARVIEW_MIRROR, NOUN_PEDESTAL) || _action.isAction(VERB_PUT, NOUN_REARVIEW_MIRROR, NOUN_LASER_BEAM)) {
		_chosenObject = 1;
		handlePedestral();
	} else if (_action.isAction(VERB_PUT, NOUN_COMPACT_CASE, NOUN_PEDESTAL) || _action.isAction(VERB_PUT, NOUN_COMPACT_CASE, NOUN_LASER_BEAM) || _action.isAction(VERB_REFLECT, NOUN_COMPACT_CASE, NOUN_LASER_BEAM)) {
		_chosenObject = 2;
		handlePedestral();
	} else if (_action._lookFlag)
		_vm->_dialogs->show(50822);
	else if (_action.isAction(VERB_LOOK, NOUN_TARGET_AREA))
		_vm->_dialogs->show(50810);
	else if (_action.isAction(VERB_LOOK, NOUN_SPINACH_PATCH_DOLL))
		_vm->_dialogs->show(50811);
	else if (_action.isAction(VERB_TAKE, NOUN_SPINACH_PATCH_DOLL))
		_vm->_dialogs->show(50812);
	else if (_action.isAction(VERB_LOOK, NOUN_SAND_BAGS))
		_vm->_dialogs->show(50816);
	else if (_action.isAction(VERB_TAKE, NOUN_SAND_BAGS))
		_vm->_dialogs->show(50817);
	else if (_action.isAction(VERB_LOOK, NOUN_CONTROL_STATION))
		_vm->_dialogs->show(50818);
	else if (_action.isAction(VERB_LOOK, NOUN_MONITOR)) {
		if (_globals[kLaserOn])
			_vm->_dialogs->show(50820);
		else
			_vm->_dialogs->show(50819);
	} else if (_action.isAction(VERB_LOOK, NOUN_LASER_CANNON)) {
		if (_globals[kLaserOn])
			_vm->_dialogs->show(50822);
		else
			_vm->_dialogs->show(50821);
	} else if (_action.isAction(VERB_TAKE, NOUN_LASER_CANNON))
		_vm->_dialogs->show(50823);
	else if (_action.isAction(VERB_LOOK, NOUN_LEVER)) {
		if (_globals[kLaserOn])
			_vm->_dialogs->show(50825);
		else
			_vm->_dialogs->show(50824);
	} else if (_action.isAction(VERB_PUSH, NOUN_LEVER))
		_vm->_dialogs->show(50826);
	else if (_action.isAction(VERB_LOOK, NOUN_LASER_BEAM)) {
		if (_globals[kLaserHoleIsThere])
			_vm->_dialogs->show(50828);
		else
			_vm->_dialogs->show(50827);
	} else if (_action.isAction(VERB_TAKE, NOUN_LASER_BEAM))
		_vm->_dialogs->show(50829);
	else if (_action.isAction(VERB_LOOK, NOUN_CEILING)) {
		if (_globals[kLaserHoleIsThere])
			_vm->_dialogs->show(50831);
		else
			_vm->_dialogs->show(50830);
	} else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(50832);
	else if (_action.isAction(VERB_LOOK, NOUN_PEDESTAL)) {
		if (!_globals[kLaserOn])
			_vm->_dialogs->show(50813);
		else if (!_globals[kLaserHoleIsThere])
			_vm->_dialogs->show(50814);
		else
			_vm->_dialogs->show(50815);
	} else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene511::Scene511(MADSEngine *vm) : Scene5xx(vm) {
	_handingLine = false;
	_lineMoving = false;

	_lineAnimationMode = -1;
	_lineFrame = -1;
	_lineAnimationPosition = -1;
}

void Scene511::synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsByte(_handingLine);
	s.syncAsByte(_lineMoving);

	s.syncAsSint16LE(_lineAnimationMode);
	s.syncAsSint16LE(_lineFrame);
	s.syncAsSint16LE(_lineAnimationPosition);
}

void Scene511::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_BOAT);
	_scene->addActiveVocab(NOUN_FISHING_LINE);
	_scene->addActiveVocab(VERB_WALKTO);
}

void Scene511::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXCD_6");

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_handingLine = false;

	if (_globals[kBoatRaised]) {
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('b', 0));
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 3);
		_scene->_hotspots.activate(NOUN_BOAT, false);
		int idx = _scene->_dynamicHotspots.add(NOUN_BOAT, VERB_WALKTO, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(75, 124), FACING_NORTH);
		_scene->_hotspots.activate(NOUN_ROPE, false);
	} else {
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('b', 2));
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('b', 3));
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('b', 1));

		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 1, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 5);

		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 4);

		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6],5);

		_scene->_hotspots.activate(NOUN_ROPE, true);
		_scene->_hotspots.activate(NOUN_BOAT, true);
		_scene->changeVariant(1);
	}

	int frame = 0;
	if (_globals[kLineStatus] == 2)
		frame = -1;
	else if (_globals[kLineStatus] == 3)
		frame = -2;

	if (_globals[kLineStatus] == 2 || _globals[kLineStatus] == 3) {
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('b', 4));
		_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, frame);
		int idx = _scene->_dynamicHotspots.add(NOUN_FISHING_LINE, VERB_WALKTO, _globals._sequenceIndexes[7], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(26, 153), FACING_NORTHEAST);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 3);
		if (_globals[kBoatRaised])
			_scene->changeVariant(2);
	}

	_lineFrame = -1;
	_lineMoving = false;

	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

	if (_scene->_priorSceneId == 512) {
		_game._player._playerPos = Common::Point(60, 112);
		_game._player._facing = FACING_SOUTHEAST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(55, 152);
		_game._player._facing = FACING_NORTHWEST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_scene->loadAnimation(formAnimName('R', 1), 70);
	} else if (_handingLine) {
		_game._player._visible = false;
		_lineAnimationMode = 1;
		_lineAnimationPosition = 1;
		_scene->loadAnimation(formAnimName('R', -1));
		_lineFrame = 2;
	}
	sceneEntrySound();
}

void Scene511::step() {
	if ((_lineAnimationMode == 1) && _scene->_animation[0]) {
		if (_lineFrame != _scene->_animation[0]->getCurrentFrame()) {
			_lineFrame = _scene->_animation[0]->getCurrentFrame();
			int resetFrame = -1;

			if ((_lineAnimationPosition == 2) && (_lineFrame == 14))
				_lineMoving = false;

			if (_lineAnimationPosition == 1) {
				if (_lineFrame == 3) {
					_lineMoving = false;
					resetFrame = 2;
				}

				if (_handingLine)
					resetFrame = 2;
			}

			if ((resetFrame >= 0) && (resetFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(resetFrame);
				_lineFrame = resetFrame;
			}
		}
	}

	switch (_game._trigger) {
	case 70:
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
		_scene->_sequences.addTimer(6, 71);
		break;

	case 71:
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

void Scene511::preActions() {
	if (!_handingLine)
		return;

	if (_action.isAction(VERB_LOOK) || _action.isObject(NOUN_FISHING_LINE) || _action.isAction(VERB_TALKTO))
		_game._player._needToWalk = false;

	if ((!_action.isAction(VERB_TIE, NOUN_FISHING_LINE, NOUN_BOAT) || !_action.isAction(VERB_ATTACH, NOUN_FISHING_LINE, NOUN_BOAT)) && _game._player._needToWalk) {
		if (_game._trigger == 0) {
			_game._player._readyToWalk = false;
			_game._player._stepEnabled = false;
			_scene->freeAnimation ();
			_lineAnimationMode = 2;
			_scene->loadAnimation(formAnimName('R',2), 1);
		} else if (_game._trigger == 1) {
			_game._player._visible = true;
			_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
			_game._objects.setRoom(OBJ_FISHING_LINE, 1);
			_handingLine = false;
			_game._player._stepEnabled = true;
			_game._player._readyToWalk = true;
		}
	}
}

void Scene511::actions() {
	if (_action.isAction(VERB_WALK_INTO, NOUN_RESTAURANT))
		_scene->_nextSceneId = 512;
	else if (_action.isAction(VERB_GET_INTO, NOUN_CAR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1: {
			int syncIdx = _globals._sequenceIndexes[1];
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], syncIdx);
			_scene->_sequences.addTimer(6, 2);
			}
			break;

		case 2:
			_game._player._visible = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 8, 1, 0, 0);
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
	} else 	if (_action.isAction(VERB_TAKE, NOUN_FISHING_LINE)) {
		if (!_globals[kBoatRaised]) {
			if (_globals[kLineStatus] == 2) {
				if (_globals[kLineStatus] != 3) {
					if (_game._trigger == 0) {
						_game._player._stepEnabled = false;
						_game._player._visible = false;
						_game._player.update();
						_lineAnimationMode = 1;
						_lineAnimationPosition = 1;
						_lineMoving = true;
						_scene->loadAnimation(formAnimName('R', -1));
						_scene->_sequences.addTimer(1, 1);
					} else if (_game._trigger == 1) {
						if (_lineMoving) {
							_scene->_sequences.addTimer(1, 1);
						} else {
							_game._objects.addToInventory(OBJ_FISHING_LINE);
							_lineMoving = true;
							_handingLine = true;
							_game._player._stepEnabled = true;
						}
					}
				} else
					_vm->_dialogs->show(51129);
			} else
				return;
		} else {
			_vm->_dialogs->show(51130);
		}
	} else if (_action.isAction(VERB_TIE, NOUN_FISHING_LINE, NOUN_BOAT) ||
			_action.isAction(VERB_ATTACH, NOUN_FISHING_LINE, NOUN_BOAT)) {
		if (_globals[kBoatRaised])
			_vm->_dialogs->show(51131);
		else if (_globals[kLineStatus] == 1)
			_vm->_dialogs->show(51130);
		else if (!_globals[kBoatRaised] && _handingLine) {
			if (_globals[kLineStatus] != 3) {
				if (_game._trigger == 0) {
					_game._player._stepEnabled = false;
					_scene->_sequences.remove(_globals._sequenceIndexes[7]);
					_lineMoving = true;
					_lineAnimationPosition = 2;
					_scene->_sequences.addTimer(1, 1);
				} else if (_game._trigger == 1) {
					if (_lineMoving)
						_scene->_sequences.addTimer(1, 1);
					else {
						_game._player._visible = true;
						_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, -2);
						_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 4);
						int idx = _scene->_dynamicHotspots.add(NOUN_FISHING_LINE, VERB_WALKTO, _globals._sequenceIndexes[7], Common::Rect(0, 0, 0, 0));
						_scene->_dynamicHotspots.setPosition(idx, Common::Point(26, 153), FACING_NORTHEAST);
						_game._objects.removeFromInventory(OBJ_FISHING_LINE, 1);
						_handingLine = false;
						_lineMoving = true;
						_globals[kLineStatus] = 3;
						_game._player._stepEnabled = true;

						if (_scene->_animation[0])
							_scene->_animation[0]->eraseSprites();
						_game._player.update();
					}
				}
			}
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_STREET) || _action._lookFlag) {
		if (_globals[kLineStatus] == 2)
			_vm->_dialogs->show(51110);
		else {
			if (_globals[kLineStatus] == 3)
				_vm->_dialogs->show(51111);
			else
				_vm->_dialogs->show(51112);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_CAR))
		_vm->_dialogs->show(51113);
	else if (_action.isAction(VERB_LOOK, NOUN_SIDEWALK))
		_vm->_dialogs->show(51114);
	else if (_action.isAction(VERB_WALK_DOWN, NOUN_SIDEWALK_TO_EAST) || _action.isAction(VERB_WALK_DOWN, NOUN_SIDEWALK_TO_WEST) || _action.isAction(VERB_WALK_DOWN, NOUN_STREET_TO_EAST))
		_vm->_dialogs->show(51115);
	else if (_action.isAction(VERB_LOOK, NOUN_PLEASURE_DOME))
		_vm->_dialogs->show(51116);
	else if (_action.isAction(VERB_LOOK, NOUN_TICKET_BOOTH))
		_vm->_dialogs->show(51117);
	else if (_action.isAction(VERB_LOOK, NOUN_DOME_ENTRANCE))
		_vm->_dialogs->show(51118);
	else if (_action.isAction(VERB_UNLOCK, NOUN_PADLOCK_KEY, NOUN_DOME_ENTRANCE) || _action.isAction(VERB_UNLOCK, NOUN_DOOR_KEY, NOUN_DOME_ENTRANCE))
		_vm->_dialogs->show(51119);
	else if ( (_action.isAction(VERB_PUT) || _action.isAction(VERB_THROW))
		 && (_action.isObject(NOUN_TIMEBOMB) || _action.isObject(NOUN_BOMB) || _action.isObject(NOUN_BOMBS))
		 && _action.isObject(NOUN_DOME_ENTRANCE))
		_vm->_dialogs->show(51120);
	else if (_action.isAction(VERB_LOOK, NOUN_RESTAURANT)) {
		if (_globals[kBoatRaised])
			_vm->_dialogs->show(51121);
		else
			_vm->_dialogs->show(51128);
	} else if (_action.isAction(VERB_LOOK, NOUN_PORTHOLE))
		_vm->_dialogs->show(51122);
	else if (_action.isAction(VERB_LOOK, NOUN_FISHING_LINE) && (_action._mainObjectSource == CAT_HOTSPOT) && (_globals[kLineStatus] == 2))
		_vm->_dialogs->show(51126);
	else if (_action.isAction(VERB_LOOK, NOUN_FISHING_LINE) && (_action._mainObjectSource == CAT_HOTSPOT) && (_globals[kLineStatus] == 3))
		_vm->_dialogs->show(51133);
	else if (_action.isAction(VERB_LOOK, NOUN_STATUE))
		_vm->_dialogs->show(51127);
	else if (_action.isAction(VERB_LOOK, NOUN_BOAT))
		if (_globals[kBoatRaised])
			_vm->_dialogs->show(51123);
		else if (_globals[kLineStatus] != 3)
			_vm->_dialogs->show(51124);
		else
			_vm->_dialogs->show(51125);
	else if (_action.isAction(VERB_LOOK, NOUN_FISHING_LINE) && (_globals[kLineStatus] == 3))
		_vm->_dialogs->show(51125);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene512::Scene512(MADSEngine *vm) : Scene5xx(vm) {
	_fishingRodHotspotId = -1;
	_keyHotspotId = -1;
}

void Scene512::synchronize(Common::Serializer &s) {
	Scene5xx::synchronize(s);

	s.syncAsSint16LE(_fishingRodHotspotId);
	s.syncAsSint16LE(_keyHotspotId);
}

void Scene512::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_FISHING_ROD);
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_PADLOCK_KEY);
	_scene->addActiveVocab(NOUN_REGISTER_DRAWER);
}

void Scene512::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('r', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMRC_9");
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites("*RXMRC_8");
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('x', 3));

	if (_game._objects[OBJ_FISHING_ROD]._roomNumber == _scene->_currentSceneId) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
		_fishingRodHotspotId = _scene->_dynamicHotspots.add(NOUN_FISHING_ROD, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_fishingRodHotspotId, Common::Point(199, 101), FACING_NORTHEAST);
	}

	if (!_game._visitedScenes._sceneRevisited)
		_globals[kRegisterOpen] = false;

	_scene->_hotspots.activate(NOUN_PADLOCK_KEY, false);
	if (_game._difficulty == DIFFICULTY_EASY) {
		if (_game._objects[OBJ_PADLOCK_KEY]._roomNumber == _scene->_currentSceneId) {
			_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 10, 0, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 3);
			_keyHotspotId = _scene->_dynamicHotspots.add(NOUN_PADLOCK_KEY, VERB_WALKTO, _globals._sequenceIndexes[6], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(_keyHotspotId, Common::Point(218, 152), FACING_NORTHEAST);
		}
		if (_globals[kRegisterOpen]) {
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
			_scene->_hotspots.activate(NOUN_PADLOCK_KEY, false);
		}
	} else if (_globals[kRegisterOpen]) {
		if (_game._objects[OBJ_PADLOCK_KEY]._roomNumber == _scene->_currentSceneId) {
			_scene->_hotspots.activate(NOUN_PADLOCK_KEY, true);
			_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 3);
		} else {
			_scene->_hotspots.activate(NOUN_PADLOCK_KEY, false);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		}
	} else
		_scene->_hotspots.activate(NOUN_PADLOCK_KEY, false);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(144, 152);
		_game._player._facing = FACING_NORTHEAST;
	}

	sceneEntrySound();
}

void Scene512::actions() {
	if (_action.isAction(VERB_WALK, NOUN_OUTSIDE))
		_scene->_nextSceneId = 511;
	else if (_action.isAction(VERB_TAKE, NOUN_FISHING_ROD)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_FISHING_ROD)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				_vm->_sound->command(9);
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_scene->_dynamicHotspots.remove(_fishingRodHotspotId);
				_game._objects.addToInventory(OBJ_FISHING_ROD);
				_vm->_dialogs->showItem(OBJ_FISHING_ROD, 51217);
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
	} else if (_action.isAction(VERB_OPEN, NOUN_CASH_REGISTER)) {
		if (!_globals[kRegisterOpen]) {
			switch (_game._trigger) {
			case 0:
				_vm->_dialogs->show(51236);
				_game._player._stepEnabled = false;
				_game._player._facing = FACING_NORTH;
				_scene->_sequences.addTimer(15, 1);
				break;

			case 1:
				_game._player._visible = false;
				_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], false, 9, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 1, 3);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[8]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2:
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[8]);
				_game._player._visible = true;
				_scene->_sequences.addTimer(30, 3);
				break;

			case 3:
				_game._player._facing = FACING_NORTHEAST;
				if (!_game._objects.isInRoom(OBJ_PADLOCK_KEY) || (_game._difficulty == DIFFICULTY_EASY)) {
					_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
				} else {
					_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 3);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
				}
				_vm->_sound->command(23);
				break;

			case 4:
				_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
				_scene->_sequences.addTimer(60, 6);
				break;

			case 5:
				_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], false, 14, 0, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 3);
				_scene->_hotspots.activate(NOUN_PADLOCK_KEY, true);
				_scene->_sequences.addTimer(60, 6);
				break;

			case 6:
				_globals[kRegisterOpen] = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		} else
			_vm->_dialogs->show(51239);
	} else if (_action.isAction(VERB_CLOSE, NOUN_CASH_REGISTER) && _globals[kRegisterOpen]) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 10, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
			_game._player._visible = true;
			if (!_game._objects.isInRoom(OBJ_PADLOCK_KEY) || _game._difficulty == DIFFICULTY_EASY) {
				_scene->_sequences.remove(_globals._sequenceIndexes[3]);
				_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			} else {
				_scene->_sequences.remove(_globals._sequenceIndexes[5]);
				_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 3);
				_scene->_hotspots.activate(NOUN_PADLOCK_KEY, false);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

		case 2:
			_globals[kRegisterOpen] = false;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_PADLOCK_KEY)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_PADLOCK_KEY)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;

				int endVal;
				if (_game._player._playerPos == Common::Point(218, 152))
					endVal = 3;
				else
					endVal = 2;

				_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 10, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, endVal);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, endVal, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				if (_game._player._playerPos == Common::Point(218, 152)) {
					_scene->_sequences.remove(_globals._sequenceIndexes[6]);
					_scene->_dynamicHotspots.remove(_keyHotspotId);
				} else {
					_scene->_sequences.remove(_globals._sequenceIndexes[5]);
					_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
					_scene->_hotspots.activate(NOUN_PADLOCK_KEY, false);
				}
				_vm->_sound->command(9);
				_game._objects.addToInventory(OBJ_PADLOCK_KEY);
				_vm->_dialogs->showItem(OBJ_PADLOCK_KEY, 51226);
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
	} else if (_action._lookFlag)
		_vm->_dialogs->show(51225);
	else if (_action.isAction(VERB_LOOK, NOUN_PADLOCK_KEY) && _game._objects.isInRoom(OBJ_PADLOCK_KEY))
		_vm->_dialogs->show(51215);
	else if (_action.isAction(VERB_LOOK, NOUN_FISHING_ROD) && (!_scene->_animation[0] ||
			_scene->_animation[0]->getCurrentFrame() == 4))
		_vm->_dialogs->show(51216);
	else if (_action.isAction(VERB_LOOK, NOUN_SHIPS_WHEEL))
		_vm->_dialogs->show(51218);
	else if (_action.isAction(VERB_TAKE, NOUN_SHIPS_WHEEL))
		_vm->_dialogs->show(51219);
	else if (_action.isAction(VERB_LOOK, NOUN_PORTHOLE) || _action.isAction(VERB_PEER_THROUGH, NOUN_PORTHOLE))
		_vm->_dialogs->show(51220);
	else if (_action.isAction(VERB_LOOK, NOUN_TABLE))
		_vm->_dialogs->show(51221);
	else if (_action.isAction(VERB_LOOK, NOUN_STARFISH))
		_vm->_dialogs->show(51222);
	else if (_action.isAction(VERB_TAKE, NOUN_STARFISH))
		_vm->_dialogs->show(51223);
	else if (_action.isAction(VERB_LOOK, NOUN_OUTSIDE))
		_vm->_dialogs->show(51224);
	else if (_action.isAction(VERB_LOOK, NOUN_POSTER))
		_vm->_dialogs->show(51227);
	else if (_action.isAction(VERB_TAKE, NOUN_POSTER))
		_vm->_dialogs->show(51228);
	else if (_action.isAction(VERB_LOOK, NOUN_TROPHY)) {
		if (_game._visitedScenes.exists(604))
			_vm->_dialogs->show(51229);
		else
			_vm->_dialogs->show(51230);
	} else if (_action.isAction(VERB_LOOK, NOUN_CHAIR))
		_vm->_dialogs->show(51231);
	else if (_action.isAction(VERB_LOOK, NOUN_ROPE))
		_vm->_dialogs->show(51232);
	else if (_action.isAction(VERB_TAKE, NOUN_ROPE))
		_vm->_dialogs->show(51233);
	else if (_action.isAction(VERB_LOOK, NOUN_LAMP))
		_vm->_dialogs->show(51234);
	else if (_action.isAction(VERB_WALK_BEHIND, NOUN_COUNTER)) {
		// WORKAROUND: Empty handling to prevent default "can't do that" dialogs showing
	} else if (_action.isAction(VERB_LOOK, NOUN_COUNTER))
		_vm->_dialogs->show(51235);
	else if (_action.isAction(VERB_LOOK, NOUN_ICE_CHESTS))
		_vm->_dialogs->show(51237);
	else if (_action.isAction(VERB_OPEN, NOUN_ICE_CHESTS))
		_vm->_dialogs->show(51238);
	else if (_action.isAction(VERB_LOOK, NOUN_CASH_REGISTER)) {
		if (!_globals[kRegisterOpen])
			_vm->_dialogs->show(51212);
		else if (_game._objects.isInRoom(OBJ_PADLOCK_KEY))
			_vm->_dialogs->show(51214);
		else
			_vm->_dialogs->show(51213);
	} else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene513::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_ELEVATOR_DOOR);
	_scene->addActiveVocab(VERB_WALKTO);
}

void Scene513::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXCD_9");
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXMRC_9");

	_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);

	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

	if ((_scene->_priorSceneId == 751) || (_scene->_priorSceneId == 701)) {
		_game._player._playerPos = Common::Point(296, 147);
		_game._player._facing = FACING_WEST;
		_game._player._stepEnabled = false;
		_scene->_sequences.addTimer(15, 80);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(63, 149);
		_game._player._facing = FACING_NORTHEAST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_scene->loadAnimation(formAnimName('R', 1), 70);
	}

	sceneEntrySound();

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_SECURITY_CARD);

	_game.loadQuoteSet(0x278, 0);
}

void Scene513::step() {
	switch (_game._trigger) {
	case 80:
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
		_vm->_sound->command(24);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
		break;

	case 81:
		_game._player.walk(Common::Point(265, 152), FACING_WEST);
		_scene->_sequences.addTimer(120, 82);
		break;

	case 82:
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
		_vm->_sound->command(25);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 83);
		break;

	case 83:
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
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
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

void Scene513::actions() {
	if (_action.isAction(VERB_GET_INTO, NOUN_CAR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1: {
			int syncIdx = _globals._sequenceIndexes[1];
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], syncIdx);
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
	} else if (_action.isAction(VERB_PUT, NOUN_ID_CARD, NOUN_CARD_SLOT) || _action.isAction(VERB_PUT, NOUN_FAKE_ID, NOUN_CARD_SLOT)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 7, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[4]);
			_game._player._visible = true;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
			_vm->_sound->command(24);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x278));
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			_game._player.walk(Common::Point(296, 147), FACING_WEST);
			_scene->_sequences.addTimer(120, 3);
			break;

		case 3:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
			_vm->_sound->command(25);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
			break;

		case 4:
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
			_game._player._stepEnabled = true;
			if (_globals[kCityFlooded])
				_scene->_nextSceneId = 701;
			else
				_scene->_nextSceneId = 751;

			break;

		default:
			break;
		}
	} else if ((_action._lookFlag) || _action.isAction(VERB_LOOK, NOUN_STREET))
		_vm->_dialogs->show(51318);
	else if (_action.isAction(VERB_LOOK, NOUN_ELEVATOR))
		_vm->_dialogs->show(51310);
	else if (_action.isAction(VERB_LOOK, NOUN_ELEVATOR_DOOR))
		_vm->_dialogs->show(51311);
	else if (_action.isAction(VERB_LOOK, NOUN_CARD_SLOT))
		_vm->_dialogs->show(51312);
	else if (_action.isAction(VERB_LOOK, NOUN_HANDICAP_SIGN))
		_vm->_dialogs->show(51313);
	else if (_action.isAction(VERB_LOOK, NOUN_BIKE_RACK))
		_vm->_dialogs->show(51314);
	else if (_action.isAction(VERB_LOOK, NOUN_BUILDING))
		_vm->_dialogs->show(51315);
	else if (_action.isAction(VERB_LOOK, NOUN_SIGN))
		_vm->_dialogs->show(51316);
	else if (_action.isAction(VERB_LOOK, NOUN_STREET_TO_WEST) || _action.isAction(VERB_WALK_DOWN, NOUN_STREET_TO_WEST))
		_vm->_dialogs->show(51317);
	else if (_action.isAction(VERB_OPEN, NOUN_ELEVATOR_DOOR) || _action.isAction(VERB_OPEN, NOUN_ELEVATOR))
		_vm->_dialogs->show(51319);
	else if (_action.isAction(VERB_LOOK, NOUN_CAR))
		_vm->_dialogs->show(51321);
	else if (_action.isAction(VERB_LOOK, NOUN_BRICK_WALL))
		_vm->_dialogs->show(51322);
	else if (_action.isAction(VERB_PUT, NOUN_SECURITY_CARD, NOUN_CARD_SLOT))
		_vm->_dialogs->show(51320);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene515::setup() {
	_game._player._spritesPrefix = "";
	setAAName();
}

void Scene515::enter() {
	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->_sequences.addTimer(30, 70);

	sceneEntrySound();
}

void Scene515::step() {
	if (_game._trigger == 70)
		_scene->loadAnimation(formAnimName('A', -1), 71);
	else if (_game._trigger == 71)
		_scene->_nextSceneId = 508;
}

/*------------------------------------------------------------------------*/

void Scene551::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene551::enter() {
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

	sceneEntrySound();
}

void Scene551::step() {
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

void Scene551::preActions() {
	if (_action.isAction(VERB_WALK_DOWN) && (_action.isObject(NOUN_STREET_TO_WEST) || _action.isObject(NOUN_SIDEWALK_TO_WEST)))
		_game._player._walkOffScreenSceneId = 501;
}

void Scene551::actions() {
	if (_action.isAction(VERB_STEP_INTO, NOUN_TELEPORTER))
		_scene->_nextSceneId = 502;
	else if ((_action._lookFlag))
		_vm->_dialogs->show(55117);
	else if (_action.isAction(VERB_LOOK, NOUN_SKELETON))
		_vm->_dialogs->show(55110);
	else if (_action.isAction(VERB_LOOK, NOUN_ELEVATOR_SHAFT))
		_vm->_dialogs->show(55111);
	else if (_action.isAction(VERB_WALKTO, NOUN_ELEVATOR_SHAFT))
		_vm->_dialogs->show(55112);
	else if (_action.isAction(VERB_LOOK, NOUN_BUILDING))
		_vm->_dialogs->show(55113);
	else if (_action.isAction(VERB_LOOK, NOUN_TELEPORTER))
		_vm->_dialogs->show(55114);
	else if (_action.isAction(VERB_LOOK, NOUN_SIDEWALK_TO_WEST)) {
		if (_game._visitedScenes.exists(505))
			_vm->_dialogs->show(55116);
		else
			_vm->_dialogs->show(55115);
	} else if (_action.isAction(VERB_LOOK, NOUN_SIDEWALK))
		_vm->_dialogs->show(55118);
	else if (_action.isAction(VERB_LOOK, NOUN_EQUIPMENT_OVERHEAD))
		_vm->_dialogs->show(55119);
	else if (_action.isAction(VERB_LOOK, NOUN_RAILING))
		_vm->_dialogs->show(55120);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
