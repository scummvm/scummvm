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

void Scene501::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(0x6E);
	_scene->addActiveVocab(0x18B);
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

		_mainSequenceId = _scene->_sequences.startReverseCycle(_mainSpriteId, false, numTicks, 1, 0, 0);
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
	int idx = _scene->_dynamicHotspots.add(0x6E, 0x18B, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
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
	} else if (_scene->_priorSceneId != -2)
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
			_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 7);
			_vm->_sound->command(12);
			_doorHotspotid = _scene->_dynamicHotspots.add(0x6E, 0x18B, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
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
			_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
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
	if (_action.isAction(0x1AD) && (_action.isAction(0x321) || _action.isAction(0x322)))
		_game._player._walkOffScreenSceneId = 551;
}

void Scene501::actions() {
	if (_action.isAction(0x36A, 0x324)) {
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
	} else if (_action.isAction(VERB_PUT, 0x131, 0x251))
		_vm->_dialogs->show(50113);
	else if (_action.isAction(VERB_PUT, 0x305, 0x251)) {
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
	} else if (_action.isAction(VERB_PUT, 0xB3, 0x251)) {
		switch (_game._trigger) {
		case 0:
		case 1:
		case 2:
		case 3:
			handleSlotActions();

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
			_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
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
	} else if ((_action._lookFlag) || _action.isAction(VERB_LOOK, 0x31E))
		_vm->_dialogs->show(50121);
	else if (_action.isAction(VERB_LOOK, 0x6E))
		_vm->_dialogs->show(50110);
	else if (_action.isAction(VERB_LOOK, 0x251))
		_vm->_dialogs->show(50112);
	else if (_action.isAction(VERB_LOOK, 0x244))
		_vm->_dialogs->show(50114);
	else if (_action.isAction(VERB_TAKE, 0x244))
		_vm->_dialogs->show(50115);
	else if (_action.isAction(VERB_LOOK, 0x322))
		_vm->_dialogs->show(50118);
	else if (_action.isAction(VERB_LOOK, 0x361) || _action.isAction(VERB_LOOK, 0x360)
		|| _action.isAction(0x1AD, 0x361) || _action.isAction(0x1AD, 0x360))
		_vm->_dialogs->show(50119);
	else if (_action.isAction(VERB_LOOK, 0x323))
		_vm->_dialogs->show(50120);
	else if (_action.isAction(VERB_OPEN, 0x6E))
		_vm->_dialogs->show(50122);
	else if (_action.isAction(VERB_LOOK, 0x1E6))
		_vm->_dialogs->show(50123);
	else if (_action.isAction(VERB_OPEN, 0x1E6))
		_vm->_dialogs->show(50124);
	else if (_action.isAction(VERB_LOOK, 0x369))
		_vm->_dialogs->show(50125);
	else if (_action.isAction(VERB_LOOK, 0x1FA) || _action.isAction(VERB_LOOK, 0x10C))
		_vm->_dialogs->show(50126);
	else if (_action.isAction(VERB_LOOK, 0x324)) {
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

	if (_action.isAction(VERB_LOOK, 0x181) || _action.isAction(0x103, 0x181))
		_vm->_dialogs->show(50210);
	else if (_action.isAction(VERB_LOOK, 0xC4))
		_vm->_dialogs->show(50211);
	else if (_action.isAction(VERB_LOOK, 0x1CC))
		_vm->_dialogs->show(50212);
	else if (_action.isAction(VERB_LOOK, 0x1D0) || _action.isAction(VERB_LOOK, 0x1D1)
	 || _action.isAction(VERB_LOOK, 0x1D2) || _action.isAction(VERB_LOOK, 0x1D3)
	 || _action.isAction(VERB_LOOK, 0x1D4) || _action.isAction(VERB_LOOK, 0x1D5)
	 || _action.isAction(VERB_LOOK, 0x1D6) || _action.isAction(VERB_LOOK, 0x1D7)
	 || _action.isAction(VERB_LOOK, 0x1D8) || _action.isAction(VERB_LOOK, 0x1D9)
	 || _action.isAction(VERB_LOOK, 0x1DB) || _action.isAction(VERB_LOOK, 0x7A)
	 || _action.isAction(VERB_LOOK, 0x1DA))
		_vm->_dialogs->show(50213);
	else if (_action.isAction(VERB_LOOK, 0x1CF) || _action._lookFlag)
		_vm->_dialogs->show(50214);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene503::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(0x6A);
	_scene->addActiveVocab(0xD);
}

void Scene503::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', -1));

	if (_globals[kSexOfRex] == REX_MALE)
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMBD_2");
	else
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*ROXBD_2");

	if (_game._objects[OBJ_DETONATORS]._roomNumber == _scene->_currentSceneId) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
		_detonatorHotspotId = _scene->_dynamicHotspots.add(0x6A, 0xD, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_detonatorHotspotId, Common::Point(254, 135), FACING_SOUTH);
	}

	if (_scene->_priorSceneId != -2) {
		_game._player._playerPos = Common::Point(191, 152);
		_game._player._facing = FACING_NORTHWEST;
	}

	sceneEntrySound();
}

void Scene503::actions() {
	if (_action.isAction(0x32F, 0xF9))
		_scene->_nextSceneId = 501;
	else if (_action.isAction(VERB_TAKE, 0x6A)) {
		if ( _game._trigger || !_game._objects.isInInventory(OBJ_DETONATORS)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				if (_globals[kSexOfRex] == REX_MALE) {
					_globals._sequenceIndexes[2] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 3);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 3, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				} else {
					_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], true, 8, 1, 0, 0);
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
	else if (_action.isAction(VERB_LOOK, 0x36F))
		_vm->_dialogs->show(50310);
	else if (_action.isAction(VERB_LOOK, 0x36E))
		_vm->_dialogs->show(50311);
	else if (_action.isAction(VERB_TAKE, 0x36E) || _action.isAction(VERB_TAKE, 0x36C))
		_vm->_dialogs->show(50312);
	else if (_action.isAction(VERB_LOOK, 0x36D))
		_vm->_dialogs->show(50313);
	else if (_action.isAction(VERB_LOOK, 0x36C))
		_vm->_dialogs->show(50314);
	else if (_action.isAction(VERB_LOOK, 0x331))
		_vm->_dialogs->show(50315);
	else if (_action.isAction(VERB_LOOK, 0x330))
		_vm->_dialogs->show(50316);
	else if (_action.isAction(VERB_OPEN, 0x330))
		_vm->_dialogs->show(50317);
	else if (_action.isAction(VERB_LOOK, 0x36B))
		_vm->_dialogs->show(50318);
	else if (_action.isAction(VERB_LOOK, 0x1E4))
		_vm->_dialogs->show(50319);
	else if (_action.isAction(VERB_LOOK, 0xE2))
		_vm->_dialogs->show(50320);
	else if (_action.isAction(VERB_LOOK, 0x481))
		_vm->_dialogs->show(50322);
	else if (_action.isAction(VERB_LOOK, 0x332)) {
		if (_game._objects.isInRoom(OBJ_DETONATORS))
			_vm->_dialogs->show(50323);
		else
			_vm->_dialogs->show(50324);
	} else if (_action.isAction(VERB_LOOK, 0x6A) && (_action._savedFields._mainObjectSource == 4))
		_vm->_dialogs->show(50325);
	else if (_action.isAction(VERB_LOOK, 0x32E))
		_vm->_dialogs->show(50327);
	else if (_action.isAction(VERB_OPEN, 0x36D))
		_vm->_dialogs->show(50329);
	else if (_action.isAction(0xC, 0x36D) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId)))
		_vm->_dialogs->show(50330);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

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
		if ((_scene->_priorSceneId != -2) && (_scene->_priorSceneId != 505))
			_globals[kHoverCarLocation] = _scene->_priorSceneId;

		_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, 1);
	}

	if (_globals[kTimebombTimer] > 10500)
		_globals[kTimebombTimer] = 10500;

	sceneEntrySound();
}

void Scene504::step() {
	if ((_carAnimationMode == 1) && (_scene->_activeAnimation != nullptr)) {
		if (_scene->_activeAnimation->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_activeAnimation->getCurrentFrame();
			int nextFrame;

			if (_carFrame == 1)
				nextFrame = 0;
			else
				nextFrame = -1;

			if ((nextFrame >= 0) && (nextFrame != _scene->_activeAnimation->getCurrentFrame())) {
				_scene->_activeAnimation->setCurrentFrame(nextFrame);
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
	if (_action.isAction(0x1CE, 0x324)) {
		_vm->_sound->command(15);
		_scene->_nextSceneId = _globals[kHoverCarLocation];
	} else if (_action.isAction(0xE, 0x380)) {
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
			_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
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
	} else if ((_action._lookFlag) || _action.isAction(VERB_LOOK, 0x388))
		_vm->_dialogs->show(50412);
	else if (_action.isAction(VERB_LOOK, 0x383))
		_vm->_dialogs->show(50410);
	else if (_action.isAction(VERB_LOOK, 0x380) || _action.isAction(VERB_LOOK, 0x387))
		_vm->_dialogs->show(50411);
	else if (_action.isAction(VERB_LOOK, 0x381))
		_vm->_dialogs->show(50413);
	else if (_action.isAction(VERB_LOOK, 0x385))
		_vm->_dialogs->show(50414);
	else if (_action.isAction(VERB_LOOK, 0x382))
		_vm->_dialogs->show(50415);
	else if (_action.isAction(VERB_LOOK, 0x386) || _action.isAction(0xD3, 0x386))
		_vm->_dialogs->show(50416);
	else if (_action.isAction(VERB_LOOK, 0x120))
		_vm->_dialogs->show(50417);
	else if (_action.isAction(VERB_TAKE, 0x120))
		_vm->_dialogs->show(50418);
	else if (_action.isAction(VERB_LOOK, 0x384))
		_vm->_dialogs->show(50419);
	else if (_action.isAction(VERB_TAKE, 0x384))
		_vm->_dialogs->show(50420);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

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

	if (_scene->_priorSceneId != -2)
		_globals._sequenceIndexes[12] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[12], false, 6, 1, 0, 0);

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
			if (_scene->_priorSceneId != -2)
				_selectedId = i;
		}
	}

	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_frame = -1;
	_scene->loadAnimation(formAnimName('a', -1));
	_scene->_activeAnimation->setCurrentFrame(86);

	sceneEntrySound();
	_vm->_sound->command(16);
}

void Scene505::step() {
	int resetFrame;
	if (_frame != _scene->_activeAnimation->getCurrentFrame()) {
		_frame = _scene->_activeAnimation->getCurrentFrame();
		resetFrame = -1;

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
					_globals._sequenceIndexes[13] = _scene->_sequences.startReverseCycle (_globals._spriteIndexes[13], false, 6, 1, 0, 0);
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
					_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle (_globals._spriteIndexes[0 + _selectedId], false, 24, 0, 0, 0);
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

		if ((resetFrame >= 0) && (resetFrame != _scene->_activeAnimation->getCurrentFrame())) {
			_scene->_activeAnimation->setCurrentFrame(resetFrame);
			_frame = resetFrame;
		}
	}

	switch (_game._trigger) {
	case 60: {
		_game._player._stepEnabled = true;
		int syncIdx = _globals._sequenceIndexes[13];
		_globals._sequenceIndexes[13] = _scene->_sequences.startCycle (_globals._spriteIndexes[13], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.updateTimeout (_globals._sequenceIndexes[13], syncIdx);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[_homeSelectedId], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_globals._sequenceIndexes[11] = _scene->_sequences.startCycle(_globals._spriteIndexes[11], false, _selectedId + 1);

		if (_selectedId != _homeSelectedId) {
			_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle (_globals._spriteIndexes[0 + _selectedId], false, 24, 0, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 1);
		}
		break;
		}

	case 61:
		_globals._sequenceIndexes[10] = _scene->_sequences.addSpriteCycle (_globals._spriteIndexes[10], false, 8, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 8);
		_scene->_sequences.updateTimeout (_globals._sequenceIndexes[10], _globals._sequenceIndexes[9]);
		break;

	case 62:
		_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle (_globals._spriteIndexes[9], false, 8, 1, 0, 0);
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
	if (_action.isAction(0x11A))
		_nextButtonId = _action._activeAction._objectNameId;
	else if (_action.isAction(0x2D5, 0x38C))
		_scene->_nextSceneId = 504;
	else if (_action.isAction(VERB_LOOK, 0x180))
		_vm->_dialogs->show(50510);
	else if (_action.isAction(VERB_LOOK, 0x59))
		_vm->_dialogs->show(50511);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
