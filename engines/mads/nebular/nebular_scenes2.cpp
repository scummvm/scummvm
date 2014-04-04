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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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
#include "mads/nebular/nebular_scenes2.h"

namespace MADS {

namespace Nebular {

void Scene2xx::setAAName() {
	int idx = (_scene->_nextSceneId == 216) ? 4 : 2;
	_game._aaName = Resources::formatAAName(idx);
}

void Scene2xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);
	Common::String oldName = _game._player._spritesPrefix;

	switch(_scene->_nextSceneId) {
	case 213:
	case 216:
		_game._player._spritesPrefix = "";
		break;
	default:
		if (_globals[kSexOfRex] != SEX_MALE) {
			_game._player._spritesPrefix = "ROX";
		} else {
			_game._player._spritesPrefix = "RXM";
		}
		break;
	}

	_game._player._scalingVelocity = (_scene->_nextSceneId <= 212);

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;
	
	if ((_scene->_nextSceneId == 203 || _scene->_nextSceneId == 204) && _globals[kRhotundaStatus])
		_game._player._loadsFirst = false;

	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

void Scene2xx::sceneEntrySound() {
	if (_vm->_musicFlag) {
		switch (_scene->_nextSceneId) {
		case 201:
			if ((_globals[kTeleporterCommand] == 2) || (_globals[kTeleporterCommand] == 4) || (_globals[kMeteorologistStatus] != 1))
				_vm->_sound->command(9);
			else
				_vm->_sound->command(17);
			break;
		case 202:
		case 203:
		case 204:
		case 205:
		case 208:
		case 209:
		case 212:
			_vm->_sound->command(9);
			break;
		case 206:
		case 211:
		case 215:
			_vm->_sound->command(10);
			break;
		case 207:
		case 214:
			_vm->_sound->command(11);
			break;
		case 210:
			if (_globals[kTwinklesStatus] == 0)
				_vm->_sound->command(15);
			else
				_vm->_sound->command(10);
			break;
		case 213:
			if (_globals[kMeteorologistWatch] == 0)
				_vm->_sound->command(1);
			else
				_vm->_sound->command(9);
				break;
		case 216:
			_vm->_sound->command(16);
			break;
		default:
			_vm->_sound->command(10);
			break;
		}
	} else
		_vm->_sound->command(2);
}

/*------------------------------------------------------------------------*/

void Scene201::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	Scene &scene = _vm->_game->_scene;
	scene.addActiveVocab(NOUN_15F);
	scene.addActiveVocab(NOUN_487);
	scene.addActiveVocab(NOUN_D);
}

void Scene201::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0), 0);
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 1), 0);
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('m', -1), 0);
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('b', -1), 0);
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*SC002Z1");
	_globals._spriteIndexes[16] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 0, 1, 0);
	_globals._spriteIndexes[17] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 15, 0, 0, 50);
	_globals._spriteIndexes[18] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 4, 0, 0, 0);
	_globals._spriteIndexes[19] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 0, 0);
	_scene->_sequences.setDepth(_globals._spriteIndexes[19], 8);
	_scene->_sequences.setMsgPosition(_globals._spriteIndexes[19], Common::Point(185, 46));

	int idx = _scene->_dynamicHotspots.add(1159, 209, _globals._spriteIndexes[19], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, 186, 81, FACING_NORTH);

	if ((_scene->_priorSceneId == 202) || (_scene->_priorSceneId == -1)) {
		_game._player._playerPos = Common::Point(165, 152);
	} else {
		_game._player._playerPos = Common::Point(223, 149);
		_game._player._facing = FACING_SOUTH;
	}

	if (_globals[kTeleporterCommand]) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		int sepChar = (_globals[kSexOfRex] == SEX_UNKNOWN) ? 't' : 'u';
		// Guess values. What is the default value used by the compiler?
		int suffixNum = -1;
		int abortTimers = -1; 
		switch(_globals[kTeleporterCommand]) {
		case 1:
			suffixNum = 3;
			abortTimers = 76;
			_globals[kTeleporterUnderstood] = true;
			break;
		case 2:
			suffixNum = 1;
			abortTimers = 77;
			break;
		case 3:
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			suffixNum = -1;
			break;
		case 4:
			suffixNum = 2;
			abortTimers = 78;
			break;
		}
		_globals[kTeleporterCommand] = 0;
		if (suffixNum >= 0)
			_scene->loadAnimation(formAnimName(sepChar, suffixNum), abortTimers);
	}
	
	if ((_scene->_priorSceneId == 202) && (_globals[kMeteorologistStatus] == 1) && !_scene->_roomChanged) {
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('a', 0), 0);
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('a', 1), 0);
		_game.loadQuoteSet(90, 91, 0);
		_game._player._stepEnabled = false;
		_globals._spriteIndexes[21] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[21], -1, 12);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[21], SM_FRAME_INDEX, 12, 70);
		_scene->_sequences.setDepth(_globals._spriteIndexes[21], 1);
		_pterodactylFlag = false;
		_game._player.walk(Common::Point(157, 143), FACING_NORTH);
		_vm->_palette->setEntry(252, 45, 63, 45);
		_vm->_palette->setEntry(253, 20, 45, 20);
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 2, 0, 120, _game.getQuote(90));
	} else
		_pterodactylFlag = true;

	if (_globals[kTeleporterUnderstood])
		_scene->_hotspots.activate(438, false);

	sceneEntrySound();
}

void Scene201::step() {
	if (_pterodactylFlag && (_vm->getRandomNumber(5000) == 9)) {
		_globals._spriteIndexes[20] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 5, 1, 6, 0);
		int idx = _scene->_dynamicHotspots.add(351, 13, _globals._spriteIndexes[20], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, 270, 80, FACING_EAST);
		_scene->_sequences.setDepth(_globals._spriteIndexes[20], 8);
		_vm->_sound->command(14);
		_pterodactylFlag = false;
	}

	if (_game._trigger == 70) {
		_globals._spriteIndexes[21] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 9, 1, 0, 0);
		_game._player._visible = false;
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[21], 12, 16);
		_globals._spriteIndexes[22] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 9, 1, 0, 0);
		_vm->_sound->command(42);
		_scene->_sequences.setDepth(_globals._spriteIndexes[21], 1);
		_scene->_sequences.setDepth(_globals._spriteIndexes[22], 1);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[22], SM_FRAME_INDEX, 3, 81);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[22], SM_0, 0, 71);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[21], SM_0, 0, 73);
	}

	if (_game._trigger == 81) {
		_scene->_kernelMessages.reset();
	}

	if (_game._trigger == 71) {
		_globals._spriteIndexes[22] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 9, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[22], -2, -2);
		_scene->_sequences.setDepth(_globals._spriteIndexes[22], 1);
	}

	if (_game._trigger == 73) {
		_globals._spriteIndexes[21] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 9, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[21], 17, -2);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[21], SM_0, 0, 74);
		_scene->_sequences.setDepth(_globals._spriteIndexes[21], 1);
	}

	if (_game._trigger == 74) {
		_vm->_sound->command(40);

		_scene->_kernelMessages.add(Common::Point(125, 56), 0xFDFC, 32, 82, 180, _game.getQuote(91));
		_globals._spriteIndexes[21] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 9, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._spriteIndexes[21], 1);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[21], -2, -2);
		_scene->_sequences.addTimer(180, 75);
	}

	if (_game._trigger == 75) {
		_globals[kMeteorologistEverSeen] = 0;
		_scene->_nextSceneId = 202;
	}

	if (_game._trigger == 76) {
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
	}

	if (_game._trigger == 77) {
		_globals[kTeleporterCommand] = 1;
		_scene->_nextSceneId = _globals[kTeleporterDestination];
		_scene->_reloadSceneFlag = true;
	}

	if (_game._trigger == 78) {
		_vm->_sound->command(40);
		_vm->_dialogs->show(0x4E92);
		_scene->_reloadSceneFlag = true;
	}
}

void Scene201::actions() {
	MADSAction *action = _game._player._action;
	if (action->_lookFlag == false) {
		if (action->isAction(0x18C, 0x83))
			_scene->_nextSceneId = 202;
		else if ((action->isAction(0x50, 0x156)) || (action->isAction(0x188, 0x16C)) || (action->isAction(0x188, 0x1B6))) {
			if (_game._trigger == 0) { 
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				int sepChar = (_globals[kSexOfRex] == SEX_UNKNOWN) ? 't' : 'u';
				_scene->loadAnimation(formAnimName(sepChar, 0), 1);
			} else if (_game._trigger == 1) {
				_scene->_nextSceneId = 213;
			}
		} else if (action->isAction(VERB_LOOK, 0x1A6)) {
			_vm->_dialogs->show(0x4E85);
		} else if (action->isAction(VERB_LOOK, 0x129)) {
			_vm->_dialogs->show(0x4E86);
		} else if (action->isAction(VERB_LOOK, 0x16F)) {
			_vm->_dialogs->show(0x4E87);
		} else if (action->isAction(VERB_LOOK, 0x142)) {
			_vm->_dialogs->show(0x4E88);
		} else if (action->isAction(VERB_LOOK, 0x18F)) {
			_vm->_dialogs->show(0x4E89);
		} else if (action->isAction(VERB_LOOK, 0x1B9)) {
			_vm->_dialogs->show(0x4E8A);
		} else if (action->isAction(VERB_LOOK, 0x192)) {
			_vm->_dialogs->show(0x4E8B);
		} else if (action->isAction(VERB_LOOK, 0x1BA)) {
			_vm->_dialogs->show(0x4E8C);
		} else if (action->isAction(VERB_LOOK, 0x83)) {
			_vm->_dialogs->show(0x4E8E);
		} else if (action->isAction(VERB_LOOK, 0x1B6)) {
			if (_globals[kMeteorologistEverSeen])
				_vm->_dialogs->show(0x4E90);
			else
				_vm->_dialogs->show(0x4E8D);
		} else if (action->isAction(VERB_LOOK, 0x16C)) {
			_vm->_dialogs->show(0x4E91);
		} else
			return;
	} else {
		_vm->_dialogs->show(0x4E8F);
	}
	action->_inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene202::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	Scene &scene = _vm->_game->_scene;
	scene.addActiveVocab(NOUN_C7);
	scene.addActiveVocab(NOUN_4E);
	scene.addActiveVocab(NOUN_D);
	scene.addActiveVocab(NOUN_2C);
	scene.addActiveVocab(NOUN_140);
	scene.addActiveVocab(NOUN_1C9);
}

void Scene202::enter() {
	_game._player._visible3 = true;
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('b', 0), 0);
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('b', 1), 0);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('b', 2), 0);
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 0), 0);
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('l', -1), 0);
	if (_globals[kSexOfRex] != SEX_MALE) {
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites("*ROXBD_2");
	} else {
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites("*RXMBD_2");
	}
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('a', 0), 0);
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('a', 1), 0);
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('a', 2), 0);

	_globals._spriteIndexes[18] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 0, 0);
	_globals._spriteIndexes[17] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 0, 0, 0);
	_scene->_sequences.setMsgPosition(_globals._spriteIndexes[17], Common::Point(149, 113));
	_scene->_sequences.setDepth(_globals._spriteIndexes[17], 10);
	int idx = _scene->_dynamicHotspots.add(320, 13, _globals._spriteIndexes[17], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, 153, 97, FACING_SOUTH);

	if (!(_globals[kBone202Status] & 1)) {
		_globals._spriteIndexes[16] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 0, 0, 0);
		_scene->_sequences.setMsgPosition(_globals._spriteIndexes[16], Common::Point(130, 108));
		_scene->_sequences.setDepth(_globals._spriteIndexes[16], 10);
		idx = _scene->_dynamicHotspots.add(44, 13, _globals._spriteIndexes[16], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, 132, 97, FACING_SOUTH);
	}

	if (!(_globals[kBone202Status] & 2)) {
		_globals._spriteIndexes[21] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 6, 0, 0, 0);
		_scene->_sequences.setMsgPosition(_globals._spriteIndexes[21], Common::Point(166, 110));
		_scene->_sequences.setDepth(_globals._spriteIndexes[21], 10);
		idx = _scene->_dynamicHotspots.add(44, 13, _globals._spriteIndexes[21], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, 165, 99, FACING_SOUTH);
	}

	if (_globals[kBone202Status])
		_scene->changeDepthSurface(_globals[kBone202Status]);

	if (_scene->_priorSceneId == 201) {
		_game._player._playerPos = Common::Point(190, 91);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId != -2) {
		_game._player._playerPos = Common::Point(178, 152);
		_game._player._facing = FACING_NORTH;
	}

	if (_globals[kLadderBroken]) {
		_globals._spriteIndexes[20] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 6, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._spriteIndexes[20], 6);
		_scene->_hotspots.activate(199, false);
		idx = _scene->_dynamicHotspots.add(457, 13, _globals._spriteIndexes[20], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, 246, 124, FACING_NORTH);
	}

	_game.loadQuoteSet(0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x62, 0x63, 0x64, 0x65, 0x66, 0x61, 0);
	_activeMsgFl = false;

	if (_scene->_priorSceneId == -2) {
		if (_waitingMeteoFl) {
			_globals._spriteIndexes[24] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 1);
			_game._player._visible = false;
		}
	} else {
		_waitingMeteoFl = false;
		_ladderTopFl = false;
	}

	_meteoClock1 = _meteoClock2 = _scene->_frameStartTime;

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_NONE);
	
	if (_globals[kMeteorologistWatch]) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		if (_globals[kMeteorologistWatch] == 2)
			_ladderTopFl = true;
		else
			_ladderTopFl = false;

		if (_globals[kMeteorologistWatch] < 1)
			_globals._spriteIndexes[25] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 6);
		else
			_globals._spriteIndexes[25] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 8);
		_scene->_sequences.setDepth(_globals._spriteIndexes[25], 1);

		if (_ladderTopFl) {
			_scene->_sequences.setMsgPosition(_globals._spriteIndexes[25], Common::Point(247, 82));
			_game._player._playerPos = Common::Point(246, 124);
			_game._player._facing = FACING_NORTH;
			_globals[kTeleporterUnderstood] = true;
		} else {
			_scene->_sequences.setMsgPosition(_globals._spriteIndexes[25], Common::Point(172, 123));
			_game._player._playerPos = Common::Point(171, 122);
			_game._player._facing = FACING_NORTH;
		}

		_scene->loadAnimation(formAnimName('M', -1), 71);
		_scene->_activeAnimation->setCurrentFrame(200);
	} else {
		_game._player._visible = false;
		_scene->_sequences.startCycle(_globals._spriteIndexes[24], true, 1);
		_scene->_sequences.setDepth(_globals._spriteIndexes[24], 1);
		_scene->_sequences.setMsgPosition(_globals._spriteIndexes[24], Common::Point(247, 82));
		_game._player._playerPos = Common::Point(246, 124);
		_game._player._facing = FACING_NORTH;
	}
	_globals._abortVal = 0;
}

void Scene202::setRandomKernelMessage() {
	int vocabId = _vm->getRandomNumber(92, 96);
	_scene->_kernelMessages.reset();
	_game._triggerSetupMode = KERNEL_TRIGGER_DAEMON;
	_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 70, 120, _game.getQuote(vocabId));
	_activeMsgFl = true;
}

void Scene202::step() {
	if (!_activeMsgFl && (_game._player._playerPos == Common::Point(77, 105)) && (_game._player._facing == FACING_NORTH) && (_vm->getRandomNumber(999) == 0)) {
		_scene->_kernelMessages.reset();
		_activeMsgFl = false;
		if (_vm->getRandomNumber(4) == 0)
			setRandomKernelMessage();
	}

	if (_game._trigger == 70)
		_activeMsgFl = false;

	if (_game._trigger == 71) {
		_vm->_sound->command(3);
		_vm->_sound->command(9);
		_meteoClock1 = 900 + _scene->_frameStartTime;
		Common::Point msgPos;
		int msgFlag;
		if (!_ladderTopFl) {
			msgPos = Common::Point(0, 0);
			msgFlag = 2;
		} else {
			msgPos = Common::Point(248, 15);
			msgFlag = 0;
		}
		int msgIndex = _scene->_kernelMessages.add(msgPos, 0x1110, msgFlag | 32, 0, 120, _game.getQuote(102));
		_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
		
		if (_globals[kMeteorologistWatch] == 1) {
			MADSAction *action = _game._player._action;
			action->_activeAction._verbId = VERB_LOOK;
			action->_activeAction._objectNameId = 39;
			action->_activeAction._indirectObjectId = 438;
			_game._triggerSetupMode = KERNEL_TRIGGER_PARSER;
			_scene->_sequences.addTimer(120, 2);
			_globals._abortVal = -1;
		} else if (_globals[kMeteorologistWatch] == 2) {
			_scene->_sequences.addTimer(120, 90);
		}
		_globals[kMeteorologistWatch] = 0;
	}

	switch (_game._trigger) {
	case 90:
		_vm->_sound->command(41);
		_scene->_sequences.remove(_globals._spriteIndexes[25]);
		_globals._spriteIndexes[24] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[9], true, 6, 1, 0, 0); 
		_scene->_sequences.setMsgPosition(_globals._spriteIndexes[24], Common::Point(247, 82));
		_scene->_sequences.setDepth(_globals._spriteIndexes[24], 1);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[24], SM_0, 0, 91);
		break;
	case 91:
		_globals._spriteIndexes[24] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], true, 1);
		_scene->_sequences.setMsgPosition(_globals._spriteIndexes[24], Common::Point(247, 82));
		_scene->_sequences.setDepth(_globals._spriteIndexes[24], 1);
		_scene->_sequences.addTimer(60, 92);
		break;
	case 92: {
		_scene->_sequences.remove(_globals._spriteIndexes[24]);
		_globals._spriteIndexes[26] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._spriteIndexes[26], 1);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[26], SM_0, 0, 93);
		_scene->_kernelMessages.reset();
		int msgIndex = _scene->_kernelMessages.add(Common::Point(0, -65), 0x1110, 32, 0, 60, _game.getQuote(98));
		_scene->_kernelMessages.setSeqIndex(msgIndex, _globals._spriteIndexes[26]);
		}
		break;
	case 93: {
		_globals[kLadderBroken] = -1;
		_globals._spriteIndexes[20] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 6, 0, 0, 0);
		_scene->_hotspots.activate(199, false);
		int idx = _scene->_dynamicHotspots.add(457, 13, _globals._spriteIndexes[20], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, 246, 124, FACING_NORTH);
		_scene->_sequences.updateTimeout(_globals._spriteIndexes[26], _globals._spriteIndexes[20]);
		_scene->_sequences.updateTimeout(-1, _globals._spriteIndexes[26]);
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_ladderTopFl = false;
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(99));
		}
		break;
	default:
		break;
	}
	
	if (!_scene->_activeAnimation && (_globals[kMeteorologistStatus] != 2) && (_meteoClock2 <= _scene->_frameStartTime) && (_meteoClock1 <= _scene->_frameStartTime)) {
		int randVal = _vm->getRandomNumber(1, 500);
		int threshold = 1;
		if (_ladderTopFl)
			threshold += 25;
		if (!_globals[kMeteorologistEverSeen])
			threshold += 25;
		if (threshold >= randVal) {
			_vm->_sound->command(17);
			_scene->loadAnimation(formAnimName('M', -1), 71);
			_toStationFl = true;
			_toTeleportFl = false;
			_globals[kMeteorologistEverSeen] = true;
			_lastRoute = 0;
			_stationCounter = 0;
			_meteoClock2 = _scene->_frameStartTime + 2;
		}
	}
	
	if (!_scene->_activeAnimation)
		return;

	if (_waitingMeteoFl) {
		if (_scene->_activeAnimation->getCurrentFrame() >= 200) {
			if ((_globals[kMeteorologistWatch] == 2) || _globals[kLadderBroken]) {
				_scene->_nextSceneId = 213;
			} else {
				_vm->_dialogs->show(0x4EE9);
				_scene->_reloadSceneFlag = true;
			}
		}

		if ((_scene->_activeAnimation->getCurrentFrame() == 160) && (_meteoFrame != _scene->_activeAnimation->getCurrentFrame())) {
			Common::Point msgPos;
			int msgFlag;
			if (!_ladderTopFl) {
				msgPos = Common::Point(0, 0);
				msgFlag = 2;
			} else {
				msgPos = Common::Point(248, 15);
				msgFlag = 0;
			}
			int msgIndex = _scene->_kernelMessages.add(msgPos, 0x1110, msgFlag | 32, 0, 120, _game.getQuote(101));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
		}
	}

	if (_meteoClock2 + 7200 <= _scene->_frameStartTime) {
		_toTeleportFl = true;
	}

	if (_scene->_activeAnimation->getCurrentFrame() == _meteoFrame) {
		return;
	}
	
	_meteoFrame = _scene->_activeAnimation->getCurrentFrame();
	int randVal = _vm->getRandomNumber(1, 1000);
	int frameStep = -1;
	
	switch (_scene->_activeAnimation->getCurrentFrame()) {
	case 42:
	case 77:
	case 96:
		frameStep = subStep1(randVal);
		break;
	case 51:
	case 74:
		frameStep = subStep2(randVal);
		break;
	case 27:
	case 119:
	case 159:
		frameStep = subStep3(randVal);
		break;
	case 176:
		frameStep = subStep4(randVal);
		break;
	case 59:
		_lastRoute = 3;
		++_stationCounter;
		if (randVal <= 800)
			frameStep = 55;
		break;
	case 89:
		_lastRoute = 1;
		if (randVal <= 700)
			frameStep = 83;
		break;
	case 137:
		_lastRoute = 2;
		if (randVal <= 700)
			frameStep = 126;
		break;
	}

	if (frameStep < 0)
		return;

	int nextFrame = 1 + _scene->_activeAnimation->getCurrentFrame() - frameStep;
	if (nextFrame) {
		_scene->_activeAnimation->setCurrentFrame(nextFrame);
		_meteoFrame = nextFrame;
	}
}

int Scene202::subStep1(int randVal) {
	_stationCounter = 0;

	if ((randVal <= 100) || _toStationFl)
		return 42;

	if ((randVal <= 200) || _toTeleportFl)
		return 96;

	if ((randVal <= 300) && (_lastRoute != 1))
		return 77;

	return 76;
}

int Scene202::subStep2(int randVal) {
	_toStationFl = false;

	if ((randVal <= 150) && (_stationCounter < 5))
		return 51;

	if ((randVal <= 300) || _toTeleportFl)
		return 74;

	if (randVal <= 400)
		return 64;

	return 44;
}

int Scene202::subStep3(int randVal) {
	if ((randVal <= 100) || _toStationFl)
		return 27;

	if ((randVal <= 200) || _toTeleportFl)
		return 159;

	if ((randVal <= 300) && (_lastRoute != 2))
		return 119;

	return 110;
}

int Scene202::subStep4(int randVal) {
	if ((randVal <= 100) || _toTeleportFl)
		return 176;

	if (randVal <= 200)
		return 19;

	return 166;	
}

void Scene202::preActions() {
	Player &player = _vm->_game->_player;
	MADSAction *action = player._action;

	if (player._readyToWalk)
		_scene->_kernelMessages.reset();

	if (!_ladderTopFl && (action->isAction(0x4E, 0xC7) || !player._readyToWalk)) {
		if (_game._trigger == 0) {
			_vm->_sound->command(29);
			player._readyToWalk = false;
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._spriteIndexes[24]);
			_globals._spriteIndexes[23] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[8], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._spriteIndexes[23], 1);
			_scene->_sequences.addSubEntry(_globals._spriteIndexes[23], SM_0, 0, 1);
		} else if (_game._trigger == 1) {
			_scene->_sequences.updateTimeout(-1, _globals._spriteIndexes[23]);
			_scene->_dynamicHotspots.remove(_ladderHotspotId);
			_game._player._visible = true;
			player._readyToWalk = true;
			_game._player._stepEnabled = true;
			_ladderTopFl = false;
		}		
	}

	if (action->isAction(VERB_LOOK, 0x27) && action->_activeAction._indirectObjectId > 0) {
		if (!player._readyToWalk || _ladderTopFl)
			_game._player._needToWalk = false;
		else
			_game._player._needToWalk = true;

		if (!_ladderTopFl)
			_game._player.walk(Common::Point(171, 122), FACING_NORTH);
	}
}

void Scene202::actions() {
	MADSAction *action = _game._player._action;
	if (action->_lookFlag == false) {
		if (action->isAction(0x4E, 0xC7)) {
			action->_inProgress = false;
			return;
		} else if (action->isAction(0x18C, 0x83)) {
			_scene->_nextSceneId = 203;
		} else if (action->isAction(0x18C, 0x82)) {
			if (_globals[kMeteorologistStatus] != 2) {
				if (_scene->_activeAnimation)
					_globals[kMeteorologistStatus] = 1;
				else
					_globals[kMeteorologistStatus] = 0;
			}
			_scene->_nextSceneId = 201;
		} else if (action->isAction(VERB_TAKE, 0x2C)) {
			if (action->_mainObjectSource == 4) {
				if (_game._trigger == 0) {
					if (_game._objects.isInInventory(OBJ_BONES)) {
						_vm->_dialogs->show(0x4EFB);
					} else {
						_game._player._stepEnabled = false;
						_game._player._visible = false;
						_globals._spriteIndexes[22] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[6], false, 3, 2, 0, 0);
						_scene->_sequences.setMsgLayout(_globals._spriteIndexes[22]);
						_scene->_sequences.addSubEntry(_globals._spriteIndexes[22], SM_FRAME_INDEX, 6, 1);
						_scene->_sequences.addSubEntry(_globals._spriteIndexes[22], SM_0, 0, 2);
					}
				} else if (_game._trigger == 1) {
					if ((_game._player._playerPos.x == 132) && (_game._player._playerPos.y == 97)) {
						_scene->_sequences.remove(_globals._spriteIndexes[16]);
						_globals[kBone202Status] |= 1;
					} else {
						_scene->_sequences.remove(_globals._spriteIndexes[21]);
						_globals[kBone202Status] |= 2;
					}
				} else if (_game._trigger == 2) {
					if (_game._objects.isInInventory(OBJ_BONE)) {
						_game._objects.removeFromInventory(OBJ_BONE, 1);
						_game._objects.addToInventory(OBJ_BONES);
						_vm->_dialogs->showPicture(OBJ_BONES, 0x4EFA, 0);
					} else {
						_game._objects.addToInventory(OBJ_BONE);
						_vm->_dialogs->showPicture(OBJ_BONE, 0x4EFA, 0);
					}
					_scene->changeDepthSurface(_globals[kBone202Status]);
					_game._player._stepEnabled = true;
					_game._player._visible = true;
				} else {
					action->_inProgress = false;
					return;
				}
			}
		} else if ((action->isAction(0x50, 0xC7)) && (_globals[kLadderBroken] == 0)) {
			if (_game._trigger == 0) {
				_vm->_sound->command(29);
				_meteoClock1 = _scene->_frameStartTime;
				_game._player._visible = false;
				_game._player._stepEnabled = false;

				_ladderHotspotId = _scene->_dynamicHotspots.add(199, 79, -1, Common::Rect(241, 68, 12, 54));
				_scene->_dynamicHotspots.setPosition(_ladderHotspotId, 246, 124, FACING_NORTH);
				_globals._spriteIndexes[23] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 6, 1, 0, 0);
				_scene->_sequences.setDepth(_globals._spriteIndexes[23], 1);
				_scene->_sequences.addSubEntry(_globals._spriteIndexes[23], SM_0, 0, 1);
			} else if (_game._trigger == 1) {
				_globals._spriteIndexes[24] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], true, 1);
				_scene->_sequences.setMsgPosition(_globals._spriteIndexes[24], Common::Point(247, 82));
				_scene->_sequences.setDepth(_globals._spriteIndexes[24], 1);
				_scene->_sequences.updateTimeout(_globals._spriteIndexes[23], _globals._spriteIndexes[24]);
				_ladderTopFl = true;
				_game._player._stepEnabled = true;
				int msgIndex = _scene->_kernelMessages.add(Common::Point(248, 15), 0x1110, 32, 0, 60, _game.getQuote(97));
				_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
			} else {
				action->_inProgress = false;
				return;
			}
		} else if (((action->isAction(VERB_LOOK, 0x27, 0x82)) || (action->isAction(VERB_LOOK, 0x27, 0x1B6))) && (_globals[kSexOfRex] == SEX_MALE)) {
			if (!_ladderTopFl) {
				if (_game._trigger == 0) {
					_game._player._stepEnabled = false;
					_game._player._visible= false;
					_globals._spriteIndexes[25] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 6, 1, 0, 0);
					_scene->_sequences.setAnimRange(_globals._spriteIndexes[25], 1, 6);
					_scene->_sequences.setMsgPosition(_globals._spriteIndexes[25], Common::Point(172, 123));
					_scene->_sequences.setDepth(_globals._spriteIndexes[25], 1);
					_scene->_sequences.updateTimeout(-1, _globals._spriteIndexes[25]);
					_scene->_sequences.addSubEntry(_globals._spriteIndexes[25], SM_0, 0, 1);
				} else if (_game._trigger == 1) {
					_globals._spriteIndexes[25] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 6);
					_scene->_sequences.setDepth(_globals._spriteIndexes[25], 1);
					_scene->_sequences.setMsgPosition(_globals._spriteIndexes[25], Common::Point(172, 123));
					if (_scene->_activeAnimation) {
						_waitingMeteoFl = true;
						_globals[kMeteorologistWatch] = 1;
					} else {
						_scene->_sequences.addTimer(120, 2);
					}
				} else if (_game._trigger == 2) {
					if (!_scene->_activeAnimation && (_globals._abortVal == 0)) {
						_vm->_dialogs->show(0x4EFE);
					}
					_scene->_sequences.remove(_globals._spriteIndexes[25]);
					_globals._spriteIndexes[25] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[9], false, 6, 1, 0, 0);
					_scene->_sequences.setDepth(_globals._spriteIndexes[25], 1);
					_scene->_sequences.setAnimRange(_globals._spriteIndexes[25], 1, 6);
					_scene->_sequences.setMsgPosition(_globals._spriteIndexes[25], Common::Point(172, 123));
					_scene->_sequences.addSubEntry(_globals._spriteIndexes[25], SM_0, 0, 3);
				} else if (_game._trigger == 3) {
					_scene->_sequences.updateTimeout(-1, _globals._spriteIndexes[25]);
					_game._player._stepEnabled = true;
					_game._player._visible = true;
				} else {
					action->_inProgress = false;
					return;
				}
			} else {
				if (_game._trigger == 0) {
					_toTeleportFl = true;
					_game._player._stepEnabled = false;
					_scene->_sequences.remove(_globals._spriteIndexes[24]);
					_globals._spriteIndexes[24] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], true, 6, 1, 0, 0);
					_scene->_sequences.setMsgPosition(_globals._spriteIndexes[24], Common::Point(247, 82));
					_scene->_sequences.setDepth(_globals._spriteIndexes[24], 1);
					_scene->_sequences.addSubEntry(_globals._spriteIndexes[24], SM_0, 0, 1);
				} else if (_game._trigger == 1) {
					_globals._spriteIndexes[25] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], true, -2);
					_scene->_sequences.setMsgPosition(_globals._spriteIndexes[25], Common::Point(247, 82));
					_scene->_sequences.setDepth(_globals._spriteIndexes[25], 1);
					if (_scene->_activeAnimation) {
						if (_scene->_activeAnimation->getCurrentFrame() > 200) {
							_scene->_sequences.addTimer(120, 2);
						} else {
							_waitingMeteoFl = true;
							_globals[kMeteorologistWatch] = 2;
							if ((_scene->_activeAnimation->getCurrentFrame() >= 44) && (_scene->_activeAnimation->getCurrentFrame() <= 75)) {
								_scene->_kernelMessages.reset();
								int msgIndex = _scene->_kernelMessages.add(Common::Point(248, 15), 0x1110, 32, 0, 60, _game.getQuote(100));
								_scene->_kernelMessages.setQuoted(msgIndex, 4, false);
							} else {
								action->_inProgress = false;
								return;
							}
						}
					} else {
						_scene->_sequences.addTimer(120, 2);
					}
				} else if (_game._trigger == 2) {
					if (!_scene->_activeAnimation)
						_vm->_dialogs->show(0x4EFE);
					_globals._abortVal = 0;
					_scene->_sequences.remove(_globals._spriteIndexes[25]);
					_globals._spriteIndexes[24] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[9], false, 6, 1, 0, 0);
					_scene->_sequences.setMsgPosition(_globals._spriteIndexes[24], Common::Point(247, 82));
					_scene->_sequences.setDepth(_globals._spriteIndexes[24], 1);
					_scene->_sequences.addSubEntry(_globals._spriteIndexes[24], SM_0, 0, 3);
				} else if (_game._trigger == 3) {
					_globals._spriteIndexes[24] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], true, 1);
					_scene->_sequences.setMsgPosition(_globals._spriteIndexes[24], Common::Point(247, 82));
					_scene->_sequences.setDepth(_globals._spriteIndexes[24], 1);
					_game._player._stepEnabled = true;
				} else {
					action->_inProgress = false;
					return;
				}
			}
		} else if (action->isAction(0x188, 0xAA)) {
			setRandomKernelMessage();
		} else if (action->isAction(VERB_LOOK, 0x129)) {
			_vm->_dialogs->show(0x4EEA);
		} else if (action->isAction(VERB_LOOK, 0x86)) {
			_vm->_dialogs->show(0x4EEB);
		} else if (action->isAction(VERB_LOOK, 0x19C)) {
			_vm->_dialogs->show(0x4EEC);
		} else if (action->isAction(VERB_LOOK, 0x82)) {
			if ((_globals[kMeteorologistStatus] == 0) || (_globals[kMeteorologistStatus] == 2)) {
				_vm->_dialogs->show(0x4EED);
			} else if (_globals[kMeteorologistStatus] == 1) {
				_vm->_dialogs->show(0x4EFC);
			} else {
				action->_inProgress = false;
				return;
			} 
		} else if (action->isAction(VERB_LOOK, 0x18E)) {
			_vm->_dialogs->show(0x4EEE);
		} else if (action->isAction(VERB_LOOK, 0x164)) {
			_vm->_dialogs->show(0x4EEF);
		} else if (action->isAction(VERB_LOOK, 0x175)) {
			_vm->_dialogs->show(0x4EF0);
		} else if (action->isAction(VERB_LOOK, 0x174)) {
			_vm->_dialogs->show(0x4EF1);
		} else if (action->isAction(VERB_LOOK, 0x142)) {
			_vm->_dialogs->show(0x4EF2);
		} else if (action->isAction(VERB_LOOK, 0xAA)) {
			if ((_game._player._playerPos == Common::Point(77, 105)) && (_game._player._facing == FACING_NORTH))
				_vm->_dialogs->show(0x4EF4);
			else
				_vm->_dialogs->show(0x4EF3);
		} else if (action->isAction(VERB_LOOK, 0x186)) {
			_vm->_dialogs->show(0x4EF5);
		} else if (action->isAction(VERB_LOOK, 0x1B5)) {
			_vm->_dialogs->show(0x4EF6);
		} else if (action->isAction(VERB_LOOK, 0x140)) {
			_vm->_dialogs->show(0x4EF7);
		} else if (action->isAction(VERB_TAKE, 0x140)) {
			_vm->_dialogs->show(0x4EF8);
		} else if (action->isAction(VERB_LOOK, 0x2D)) {
			if (action->_commandSource == 4)
				_vm->_dialogs->show(0x4EF9);
			else
				return;
		} else {
			return;			
		}
	} else {
		_vm->_dialogs->show(0x4EFB);
	}
	action->_inProgress = false;
}

/*****************************************************************************/

void Scene203::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(477);
}

void Scene203::enter() {
	if (_scene->_priorSceneId == 202) {
		_game._player._playerPos = Common::Point(187, 99);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId == 209) {
		_game._player._playerPos = Common::Point(308, 117);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId != -2) {
		_game._player._playerPos = Common::Point(155, 152);
		_game._player._facing = FACING_NORTH;
	}

	_rhotundaEatFl = false;
	_rhotundaEat2Fl = false;

	if ((_globals[kRhotundaStatus] == 0) && (!_scene->_roomChanged)) {
		_rhotundaEatFl = true;
		_game._player.walk(Common::Point(158, 135), FACING_SOUTH);
		int idx = _scene->_dynamicHotspots.add(131, 396, 0, Common::Rect(0, 0, 320, 156));
		_scene->_dynamicHotspots.setPosition(idx, 155, 152, FACING_SOUTH);
		_scene->_dynamicHotspots.setCursor(idx, CURSOR_GO_DOWN);
	}

	if (!_rhotundaEatFl) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('b', -1), 0);
		if (_vm->getRandomNumber(1, 3) == 2) {
			_globals._spriteIndexes[15] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 9, 1, 0, 0);
			int idx = _scene->_dynamicHotspots.add(477, 209, _globals._spriteIndexes[15], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, -2, 0, FACING_NONE);
			_vm->_sound->command(14);
		}
	}

	_game.loadQuoteSet(0x67, 0x68, 0x69, 0x6A, 0x5A, 0);

	if (_rhotundaEatFl) {
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(_vm->getRandomNumber(103, 106)));
	}

	sceneEntrySound();
}

void Scene203::step() {
	if (!_rhotundaEatFl)
		return;

	if ((_game._trigger == 0) && _rhotundaEat2Fl)
		return;

	if ((_game._player._playerPos != Common::Point(158, 136)) || (_game._player._facing != FACING_SOUTH))
		return;

	_rhotundaEat2Fl = true;

	if (_game._trigger == 0) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_vm->_palette->lock();
		_scene->_kernelMessages.reset();
		_scene->resetScene();
		_vm->_events->setCursor2(CURSOR_WAIT);
		_scene->loadAnimation(Resources::formatName(203, 'a', -1, EXT_AA, ""), 81);
	} else if (_game._trigger == 81) {
		_scene->_nextSceneId = 208;
		_scene->_reloadSceneFlag = true;
	} 
}

void Scene203::preActions() {
	if (_rhotundaEatFl && !_action.isAction(0x18C, 0x83)) {
		_game._player.walk(Common::Point(158, 136), FACING_SOUTH);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALKTO, 0xF3))
		_game._player._walkOffScreenSceneId = 209;
}

void Scene203::actions() {
	if (_action._savedFields._lookFlag) {
		_vm->_dialogs->show(0x4F53);
	} else if (_action.isAction(0x18C, 0x83)) {
		_scene->_nextSceneId = 208;
	} else if (_action.isAction(0x18C, 0x82)) {
		_scene->_nextSceneId = 202;
	} else if (_action.isAction(VERB_LOOK, 0x142)) {
		_vm->_dialogs->show(0x4F4D);
	} else if (_action.isAction(VERB_LOOK, 0x4D)) {
		_vm->_dialogs->show(0x4F4E);
	} else if (_action.isAction(VERB_LOOK, 0x100)) {
		_vm->_dialogs->show(0x4F4F);
	} else if (_action.isAction(VERB_LOOK, 0x82)) {
		_vm->_dialogs->show(0x4F50);
	} else if (_action.isAction(VERB_LOOK, 0x1A6)) {
		_vm->_dialogs->show(0x4F51);
	} else if (_action.isAction(VERB_LOOK, 0x30)) {
		_vm->_dialogs->show(0x4F51);
	} else
		return;

	_action._inProgress = false;
}

/*****************************************************************************/

void Scene205::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_D);
	_scene->addActiveVocab(NOUN_49);
	_scene->addActiveVocab(NOUN_10D);
}

void Scene205::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0), 0);
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 1), 0);
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 2), 0);
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('f', -1), 0);
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('c', -1), 0);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('p', -1), 0);
	
	if (_globals[kSexOfRex] == SEX_MALE)
		_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('a', 1), 0);

	_globals._spriteIndexes[16] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 10, 0, 0, 3);
	int idx = _scene->_dynamicHotspots.add(73, 13, _globals._spriteIndexes[16], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, 162, 120, FACING_NORTHEAST);

	_globals._spriteIndexes[17] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 15, 0, 0, 0);
	idx = _scene->_dynamicHotspots.add(73, 13, _globals._spriteIndexes[16], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, 162, 120, FACING_NORTHEAST);

	_globals._spriteIndexes[18] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 9, 0, 0, 0);
	_globals._spriteIndexes[20] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 6, 0, 0, 0);
	_scene->_sequences.setDepth(_globals._spriteIndexes[20], 11);

	if (!_game._visitedScenes._sceneRevisited) {
		_lastFishTime = _scene->_frameStartTime;
		_globals._spriteIndexes[21] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 7, 1, 0, 0);
		idx = _scene->_dynamicHotspots.add(269, 13, _globals._spriteIndexes[21], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, 49, 86, FACING_NORTH);
	}

	if (_game._objects[12]._roomNumber == 205) {
		_globals._spriteIndexes[19] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 7, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._spriteIndexes[19], 11);
	} else {
		_scene->_hotspots.activate(450, false);
	}

	_globals._frameTime &= 0xFFFF;
	_game.loadQuoteSet(0x6B, 0x70, 0x71, 0x72, 0x5A, 0x74, 0x75, 0x76, 0x77, 0x78, 0x73, 0x79, 0x7A, 0x7B, 0x7C,
		0x7D, 0x7E, 0x7F, 0x80, 0xAC, 0xAD, 0xAE, 0x6C, 0x6D, 0x6E, 0x6F, 0x2, 0);
	warning("TODO: sub71A50(_globals._v4, 0x5A, 0x78, 0x74, 0x75, 0x76, 0x77, 0);");

	if (!_game._visitedScenes._sceneRevisited)
		warning("TODO: sub71B18(_globals._v4, 0x5A, 0x74, 0x75, 0x77, 0);");


	_vm->_palette->setEntry(250, 63, 50, 20);
	_vm->_palette->setEntry(251, 50, 40, 15);
	_vm->_palette->setEntry(252, 63, 63, 40);
	_vm->_palette->setEntry(253, 50, 50, 30);

	_chickenTime = _globals._frameTime;

	if (_globals[kSexOfRex] == SEX_FEMALE)
		warning("sub71704(0x3, 0xC3, 0x108, 0x63, 0x86, 0xD, 2, 0xFDFC, 0x3C, 0x6C, 0x6C, 0x6D, 0x6D, 0x6E, 0x6E, 0x6F, 0x6C, 0);");

	if (_scene->_priorSceneId != -2)
		_game._player._playerPos = Common::Point(99, 152);	

	if (_globals[kSexOfRex] != SEX_MALE) {
		_scene->loadAnimation(formAnimName('a', -1), 0);
		_scene->_activeAnimation->_resetFlag = false;
	} else {
		_globals._frameTime |= 0xFFFF0000;
		_globals._spriteIndexes[23] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8, 1, 0, 0);
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->_sequences.setDepth(_globals._spriteIndexes[23], 2);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[23], SM_FRAME_INDEX, 6, 73);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[23], SM_FRAME_INDEX, 11, 74);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[23], SM_0, 0, 0);
	}
	sceneEntrySound();
}

void Scene205::step() {
	if (_globals[kSexOfRex] == SEX_FEMALE) {
		warning("TODO: sub7178C()");

		if (_globals._frameTime >= _chickenTime) {
			warning("minVal = 1 + sub7176C();");
			int minVal = 1;

			warning("TODO: sub717B2(100, minVal);");
//			if (sub717B2(100, minVal)) {
				_vm->_sound->command(28);
//			}
			_chickenTime = _globals._frameTime + 2;
		}
	}

	if (_globals._frameTime - _lastFishTime > 1300) {
		_globals._spriteIndexes[21] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 5, 1, 0, 0);
		int idx = _scene->_dynamicHotspots.add(269, 13, _globals._spriteIndexes[21], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, 49, 86, FACING_NORTH);
		_lastFishTime = _globals._frameTime;
	}

	if (_game._trigger == 73) {
		_scene->_kernelMessages.add(Common::Point(160, 68), 0xFBFA, 32, 0, 60, _game.getQuote(112));
	}

	if (_game._trigger == 74) {
		_vm->_sound->command(26);
		_scene->_kernelMessages.add(Common::Point(106, 90), 0x1110, 32, 0, 60, _game.getQuote(113));
	}

	if (_game._trigger == 71) {
		_globals._spriteIndexes[23] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 6, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._spriteIndexes[23], 2);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[23], -2, -2);
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(160, 68), 0xFBFA, 32, 72, 180, _game.getQuote(114));
	}

	if (_game._trigger == 72) {
		_scene->_nextSceneId = 211;
	}
}

void Scene205::actions() {
	warning("TODO: Scene205::actions");
}

/*****************************************************************************/

void Scene207::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(0x185);
	_scene->addActiveVocab(NOUN_D);
	_scene->addActiveVocab(0x14D);
	_scene->addActiveVocab(NOUN_D);
}

void Scene207::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('h', 0), 0);
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('h', 1), 0);
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('c', -1), 0);
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('e', 0), 0);
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('e', 1), 0);
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('g', 1), 0);
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('g', 0), 0);
	_globals._spriteIndexes[20] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 7, 0, 0, 0);
	_scene->_sequences.setDepth(_globals._spriteIndexes[20], 7);

	int var2;
	if (!_game._visitedScenes._sceneRevisited) {
		var2 = 1;
	} else {
		var2 = _vm->getRandomNumber(4) + 1;
	}

	if (var2 > 2)
		_vultureFl = false;
	else
		_vultureFl = true;

	_spiderFl = (var2 & 1);
	
	if (_vultureFl) {
		_globals._spriteIndexes[16] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[1], false, 30, 0, 0, 400);
		_vultureTime = _game._player._priorTimer;
		_vultureHotspotId = _scene->_dynamicHotspots.add(389, 13, _globals._spriteIndexes[16], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_vultureHotspotId, 254, 94, FACING_WEST);
	}

	if (_spiderFl) {
		_globals._spriteIndexes[19] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[19], -1, -1);
		_spiderTime = _game._player._priorTimer;
		_spiderHotspotId = _scene->_dynamicHotspots.add(333, 13, _globals._spriteIndexes[19], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_spiderHotspotId, 59, 132, FACING_SOUTH);
	}

	_eyeFl = false;
	if (_scene->_priorSceneId == 211) {
		_game._player._playerPos = Common::Point(13, 105);
		_game._player._facing = FACING_EAST;
	} else if (_scene->_priorSceneId == 214) {
		_game._player._playerPos = Common::Point(164, 117);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId != -2) {
		_game._player._playerPos = Common::Point(305, 131);
	}

	_globals._spriteIndexes[21] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 10, 1, 0, 0);
	_scene->_sequences.setAnimRange(_globals._spriteIndexes[21], 1, 22);
	_scene->_sequences.setDepth(_globals._spriteIndexes[21], 6);
	_scene->_sequences.addSubEntry(_globals._spriteIndexes[21], SM_0, 0, 70);
}

void Scene207::moveVulture() {
	_scene->_sequences.remove(_globals._spriteIndexes[16]);
	_globals._spriteIndexes[17] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
	_vm->_sound->command(43);
	_vultureFl = false;
	_vultureTime = _game._player._priorTimer;
	_scene->_dynamicHotspots.remove(_vultureHotspotId);
}

void Scene207::moveSpider() {
	_scene->_sequences.remove(_globals._spriteIndexes[19]);
	_globals._spriteIndexes[19] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 5, 1, 0, 0);
	_spiderFl = false;
	_spiderTime = _game._player._priorTimer;
	_scene->_dynamicHotspots.remove(_spiderHotspotId);
}

void Scene207::step() {
	if (!_vultureFl)
		moveVulture();

	if (_spiderFl)
		moveSpider();

	if (_game._trigger == 70) {
		_globals._spriteIndexes[21] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 10, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[21], 23, 34);
		_scene->_sequences.setDepth(_globals._spriteIndexes[21], 6);
	}

	if (_game._trigger == 71)
		_eyeFl = false;

	if (_eyeFl)
		return;

	if ((_game._player._playerPos.x >= 124) && (_game._player._playerPos.x <= 201)) {
		_globals._spriteIndexes[22] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 10, 1, 0, 0);
		_globals._spriteIndexes[23] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._spriteIndexes[22], 6);
		_scene->_sequences.setDepth(_globals._spriteIndexes[23], 6);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[22], SM_0, 0, 71);
		_eyeFl = true;
	}
}

void Scene207::preActions() {
	if (_action.isAction(0x1AD, 0x1AE))
		_game._player._walkOffScreenSceneId = 211;

	if (_action.isAction(0x18C, 0x1AB))
		_game._player._walkOffScreenSceneId = 208;

	if ((_action.isAction(VERB_WALKTO)) || (_action.isAction(VERB_LOOK))) {
		if (_action.isAction(0x185)) {
			_vultureTime = -9999;
		} else if (_action.isAction(0x14D)) {
			_spiderTime = -9999;
		}
	}
}

void Scene207::actions() {
	if (_action._savedFields._lookFlag) {
		_vm->_dialogs->show(0x50E7);
	} else if (_action.isAction(0x18B, 0x70))
		_scene->_nextSceneId = 214;
	else {
		if ((_game._player._playerPos.x > 150) && (_game._player._playerPos.x < 189) &&
		    (_game._player._playerPos.y > 111) && (_game._player._playerPos.y < 130)) {
			if ((_game._player._playerPos.x <= 162) && (_game._player._playerPos.x >= 181) &&
			    (_game._player._playerPos.y <= 115) && (_game._player._playerPos.y >= 126)) {
				_globals._spriteIndexes[22] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 10, 2, 0, 0);
				_globals._spriteIndexes[23] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8, 2, 0, 0);
				_scene->_sequences.setDepth(_globals._spriteIndexes[22], 6);
				_scene->_sequences.setDepth(_globals._spriteIndexes[23], 6);
			}
		} else if (_eyeFl) {
			_scene->_sequences.remove(_globals._spriteIndexes[22]);
			_scene->_sequences.remove(_globals._spriteIndexes[23]);
			_eyeFl = false;
		}

		if (_action.isAction(VERB_LOOK, 0x69)) {
			_vm->_dialogs->show(0x50DD);
		} else if (_action.isAction(VERB_LOOK, 0x1AF)) {
			_vm->_dialogs->show(0x50DE);
		} else if (_action.isAction(VERB_LOOK, 0x141)) {
			_vm->_dialogs->show(0x50DF);
		} else if (_action.isAction(VERB_LOOK, 0x3E)) {
			_vm->_dialogs->show(0x50E0);
		} else if (_action.isAction(VERB_LOOK, 0x198)) {
			_vm->_dialogs->show(0x50E1);
		} else if (_action.isAction(VERB_LOOK, 0x1AE)) {
			_vm->_dialogs->show(0x50E2);
		} else if (_action.isAction(VERB_LOOK, 0xE8)) {
			_vm->_dialogs->show(0x50E3);
		} else if (_action.isAction(VERB_LOOK, 0x12)) {
			_vm->_dialogs->show(0x50E4);
		} else if (_action.isAction(VERB_LOOK, 0x1AC)) {
			_vm->_dialogs->show(0x50E5);
		} else if (_action.isAction(VERB_LOOK, 0x185)) {
			_vm->_dialogs->show(0x50E6);
		} else if (_action.isAction(VERB_TAKE, 0x141)) {
			_vm->_dialogs->show(0x50E8);
		} else if (_action.isAction(VERB_TAKE, 0x12)) {
			_vm->_dialogs->show(0x50E9);
		} else if (_action.isAction(VERB_LOOK, 0x14D)) {
			_vm->_dialogs->show(0x50EA);
		} else if (_action.isAction(VERB_TAKE, 0x14D)) {
			_vm->_dialogs->show(0x50EB);
		} else
			return;
	}

	_action._inProgress = false;
}

/*****************************************************************************/

void Scene208::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(0x1A8);
	_scene->addActiveVocab(0x1A9);
	_scene->addActiveVocab(0x1AA);
	_scene->addActiveVocab(NOUN_D);
}

void Scene208::updateTrap() {
	if (_globals[kRhotundaStatus] == 1) {
		_globals._spriteIndexes[16] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 0, 0, 24);
		_scene->_sequences.setDepth(_globals._spriteIndexes[16], 5);
		int idx = _scene->_dynamicHotspots.add(424, 13, _globals._spriteIndexes[16], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, 100, 146, FACING_NORTH);
		_scene->_hotspots.activate(414, false);
		return;
	}

	switch (_globals[35]) {
	case 0: {
		_globals._spriteIndexes[17] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._spriteIndexes[17], 15);
		int idx = _scene->_dynamicHotspots.add(426, 13, _globals._spriteIndexes[17], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, 60, 152, FACING_NORTH);
		}
		break;
	case 2: {
		_scene->_sequences.setDepth(_globals._spriteIndexes[18], 15);
		_globals._spriteIndexes[18] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_hotspots.activate(414, false);
		int idx = _scene->_dynamicHotspots.add(425, 13, _globals._spriteIndexes[17], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, 100, 146, FACING_NORTH);
		_scene->_dynamicHotspots[idx]._articleNumber = 5;
		}
		break;
	}
}

void Scene208::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 1), 0);
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0), 0);
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 1), 0);
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 2), 0);
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RXMBD_8");

	updateTrap();

	_rhotundaTurnFl = false;
	_boundingFl = false;
	_scene->_kernelMessages._talkFont = _vm->_font->getFont(FONT_INTERFACE);
	_scene->_textSpacing = 0;

	if (_scene->_priorSceneId == 207) {
		_game._player._playerPos = Common::Point(8, 122);
		_game._player._facing = FACING_EAST;
	} else if (_scene->_priorSceneId == 203) {
		_game._player._playerPos = Common::Point(142, 108);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId == 209) {
		_game._player._playerPos = Common::Point(307, 123);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId != -2) {
		_game._player._playerPos = Common::Point(162, 149);
		_game._player._facing = FACING_NORTH;
	}

	_game.loadQuoteSet(0x81, 0x46, 0);

	if ((_scene->_priorSceneId == 207) && (_globals[kMonkeyStatus] == 1)) {
		int msgIndex = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(129));
		_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
	}

	_vm->_palette->setEntry(16, 0, 0, 63);
	_vm->_palette->setEntry(17, 0, 0, 45);
	sceneEntrySound();
}

void Scene208::step() {
	if (_boundingFl  && (_rhotundaTime <= _scene->_activeAnimation->getCurrentFrame())) {
		_rhotundaTime = _scene->_activeAnimation->getCurrentFrame();

		if (_rhotundaTime == 125)
			_scene->_sequences.remove(_globals._spriteIndexes[19]);
	}

	if (!_rhotundaTurnFl)
		return;

	if ((_game._player._playerPos != Common::Point(20, 148)) || (_game._player._facing != FACING_EAST))
		return;

	if ((_game._trigger == 0) && _boundingFl)
		return;
	
	_boundingFl = true;
	
	switch (_game._trigger) {
	case 0:
		_scene->loadAnimation(formAnimName('A', -1), 81);
		_rhotundaTime = 0;
		break;
	case 81:
		_scene->_sequences.remove(_globals._spriteIndexes[15]);
		_globals[kRhotundaStatus] = 1;
		updateTrap();
		_scene->_sequences.addTimer(90, 82);
		break;
	case 82:
		_game._player._stepEnabled = true;
		break;
	}
}

void Scene208::preActions() {
	Player &player = _vm->_game->_player;

	if (_action.isAction(VERB_LOOK) && player._readyToWalk)
		player._needToWalk = true;

	if (_action.isAction(0x18C, 0x9B))
		player._walkOffScreenSceneId = 209;

	if (_action.isAction(0x18C, 0xF6))
		player._walkOffScreenSceneId = 207;
}

void Scene208::subAction(int mode) {

	switch (_game._trigger) {
	case 0: {
		_game._player._stepEnabled = false;
		_game._player._visible   = false;
		_globals._spriteIndexes[20] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 6, 1, 0, 0);
		_scene->_sequences.setMsgLayout(_globals._spriteIndexes[20]);

		int abortVal;
		if ((mode == 1) || (mode == 2))
			abortVal = 1;
		else
			abortVal = 2;

		_scene->_sequences.addSubEntry(_globals._spriteIndexes[20], SM_0, 0, abortVal);
		}
		break;
	case 1: {
		int oldVal = _globals._spriteIndexes[20];
		_globals._spriteIndexes[20] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 12, 3, 0, 0);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[20], 3, 4);
		_scene->_sequences.setMsgLayout(_globals._spriteIndexes[20]);
		_scene->_sequences.updateTimeout(_globals._spriteIndexes[20], oldVal);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[20], SM_0, 0, 2);
		_vm->_sound->command(20);
		}
		break;
	case 2: {
		switch (mode) {
		case 1:
			_game._objects.addToInventory(6);
			_scene->_sequences.remove(_globals._spriteIndexes[17]);
			_globals[kLeavesStatus] = 1;
			break;
		case 2:
			_game._objects.setRoom(6, 1);
			_globals[kLeavesStatus] = 2;
			updateTrap();
			break;
		case 3:
			_scene->_sequences.remove(_globals._spriteIndexes[18]);
			_globals._spriteIndexes[19] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
			_game._objects.removeFromInventory(10, 1);
			_vm->_sound->command(34);
			break;
		case 4:
			_game._objects.removeFromInventory(1, 1);
			_vm->_sound->command(33);
			break;
		case 5:
			_game._objects.removeFromInventory(2, 1);
			_vm->_sound->command(33);
			break;
		}

		int oldVal = _globals._spriteIndexes[20];
		_globals._spriteIndexes[20] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[5], false, 6, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[20], 1, 3);
		_scene->_sequences.setMsgLayout(_globals._spriteIndexes[20]);
		_scene->_sequences.updateTimeout(_globals._spriteIndexes[20], oldVal);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[20], SM_0, 0, 3);
		}
		break;
	case 3:
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		break;
	}
}

void Scene208::actions() {
	if (_action.isAction(0x18C, 0x19F)) {
		if (_globals[kRhotundaStatus])
			_scene->_nextSceneId = 203;
		else if (_game._trigger == 0) {
			_game._player._stepEnabled = false;
			int msgIndex = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 1, 120, _game.getQuote(70));
			_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
		} else if (_game._trigger == 1) {
			_scene->_nextSceneId = 203;
		}
	} else if (_action.isAction(0x18C, 0x83)) {
		_scene->_nextSceneId = 212;
	} else if (_action.isAction(VERB_TAKE, 0x1AA) && (!_globals[kLeavesStatus] || _game._trigger)) {
		subAction(1);
		if (_game._player._stepEnabled)
			_vm->_dialogs->showPicture(OBJ_BIG_LEAVES, 0x326, 0);
	} else if (_action.isAction(VERB_PUT, 0x23, 0x19E) && (_globals[kLeavesStatus] == 1 || _game._trigger)) {
		subAction(2);
	} else if (_action.isAction(VERB_PUT, 0x17A, 0x1A9)) {
		subAction(3);
		if (_game._player._stepEnabled) {
			_game._player._stepEnabled = false;
			_rhotundaTurnFl = true;
			_game._player.walk(Common::Point(20, 148), FACING_EAST);
		}
	} else if (_action.isAction(VERB_PUT, 0x35, 0x1A9)) {
		subAction(4);
		if (_game._player._stepEnabled) {
			_vm->_dialogs->show(0x514C);
		}
	} else if (_action.isAction(VERB_PUT, 0x65, 0x1A9)) {
		subAction(5);
		if (_game._player._stepEnabled) {
			_vm->_dialogs->show(0x514C);
		}
	} else if (_action.isAction(VERB_LOOK, 0x5D)) {
		_vm->_dialogs->show(0x5141);
	} else if (_action.isAction(VERB_LOOK, 0xF6)) {
		_vm->_dialogs->show(0x5142);
	} else if (_action.isAction(VERB_LOOK, 0x16F)) {
		_vm->_dialogs->show(0x5143);
	} else if (_action.isAction(VERB_LOOK, 0x129)) {
		_vm->_dialogs->show(0x5144);
	} else if (_action.isAction(VERB_LOOK, 0x1A1)) {
		_vm->_dialogs->show(0x5145);
	} else if (_action.isAction(VERB_TAKE, 0x1A1)) {
		_vm->_dialogs->show(0x5146);
	} else if (_action.isAction(VERB_LOOK, 0x9B)) {
		_vm->_dialogs->show(0x5147);
	} else if (_action.isAction(VERB_LOOK, 0x19E)) {
		_vm->_dialogs->show(0x5148);
	} else if (_action.isAction(VERB_LOOK, 0x1AA)) {
		_vm->_dialogs->show(0x5149);
	} else if (_action.isAction(VERB_LOOK, 0x1A9)) {
		if (_game._difficulty == DIFFICULTY_IMPOSSIBLE)
			_vm->_dialogs->show(0x514A);
		else
			_vm->_dialogs->show(0x514B);
	} else if (_action.isAction(VERB_LOOK, 0x174) || _action.isAction(VERB_LOOK, 0x175)) {
		_vm->_dialogs->show(0x514D);
	} else if (_action.isAction(VERB_TAKE, 0x1A9)) {
		_vm->_dialogs->show(0x514E);
	} else if (_action.isAction(VERB_LOOK, 0x1A8)) {
		_vm->_dialogs->show(0x514F);
	} else if (_action.isAction(VERB_TAKE, 0x1A8) || _action.isAction(0xA, 0x1A8)) {
		_vm->_dialogs->show(0x5150);
	} else if (_action._savedFields._lookFlag == 0) {
		return;
	} else if (_globals[kRhotundaStatus] == 1) {
		_vm->_dialogs->show(0x5153);
	} else if (_globals[kLeavesStatus] == 2) {
		_vm->_dialogs->show(0x5152);
	} else {
		_vm->_dialogs->show(0x5151);
	}

	_action._inProgress = false;
}

/*****************************************************************************/

void Scene209::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(0x10F);
}

void Scene209::enter() {
}

void Scene209::step() {
}

void Scene209::preActions() {
}

void Scene209::actions() {
}

} // End of namespace Nebular
} // End of namespace MADS
