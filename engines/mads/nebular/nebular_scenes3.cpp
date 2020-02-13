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
#include "mads/nebular/nebular_scenes3.h"

namespace MADS {

namespace Nebular {

void Scene3xx::setAAName() {
	_game._aaName = Resources::formatAAName(4);
}

void Scene3xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);

	Common::String oldName = _game._player._spritesPrefix;

	if (_globals[kSexOfRex] == REX_MALE)
		_game._player._spritesPrefix = "RXM";
	else
		_game._player._spritesPrefix = "ROX";

	if ((_scene->_nextSceneId == 313) || (_scene->_nextSceneId == 366)
	|| ((_scene->_nextSceneId >= 301) && (_scene->_nextSceneId <= 303))
	|| ((_scene->_nextSceneId == 304) && (_scene->_currentSceneId == 303))
	|| ((_scene->_nextSceneId == 311) && (_scene->_currentSceneId == 304))
	|| ((_scene->_nextSceneId >= 308) && (_scene->_nextSceneId <= 310))
	|| ((_scene->_nextSceneId >= 319) && (_scene->_nextSceneId <= 322))
	|| ((_scene->_nextSceneId >= 387) && (_scene->_nextSceneId <= 391))) {
		_game._player._spritesPrefix = "";
		_game._player._spritesChanged = true;
	}

	_game._player._scalingVelocity = true;
	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

void Scene3xx::sceneEntrySound() {
	if (!_vm->_musicFlag) {
		_vm->_sound->command(2);
		return;
	}

	switch (_scene->_nextSceneId) {
	case 301:
	case 302:
	case 303:
	case 304:
	case 308:
	case 309:
	case 310:
		_vm->_sound->command(11);
		break;

	case 311:
		if (_scene->_priorSceneId == 304)
			_vm->_sound->command(11);
		else
			_vm->_sound->command(10);
		break;

	case 313:
	case 316:
	case 320:
	case 322:
	case 357:
	case 358:
	case 359:
	case 360:
	case 361:
	case 387:
	case 388:
	case 389:
	case 390:
	case 391:
	case 399:
		_vm->_sound->command(10);
		break;

	case 318:
		if ((_scene->_priorSceneId == 357) || (_scene->_priorSceneId == 407))
			_vm->_sound->command(10);
		else if (_scene->_priorSceneId == 319)
			_vm->_sound->command(16);
		else
			_vm->_sound->command(3);

		_vm->_sound->command(50);
		break;

	case 319:
		_vm->_sound->command(16);
		break;

	case 321:
		_vm->_sound->command(18);
		break;

	default:
		break;
	}
}

void Scene3xx::initForceField(ForceField *force, bool flag) {
	force->_flag = flag;
	force->_vertical = 0;
	force->_horizontal = 0;
	force->_timer = 0;

	for (int count = 0; count < 40; count++)
		force->_seqId[count] = -1;

	if (force->_flag)
		_vm->_sound->command(24);
}

int Scene3xx::computeScale(int low, int high, int id) {
	int diff = high - (low + 2);
	int quotient = diff / 20;
	int remainder = diff % 20;
	int value = low + 2 + (quotient * id) + (remainder / (id + 1));

	return (value);
}

void Scene3xx::handleForceField(ForceField *force, int *sprites) {
	if (_game._trigger >= 150) {
		int id = _game._trigger - 150;
		if (id < 40) {
			if (id < 20)
				force->_vertical--;
			else
				force->_horizontal--;

			force->_seqId[id] = -1;
		}
		return;
	}

	if (!force->_flag || (_scene->_frameStartTime < force->_timer) || (force->_vertical + force->_horizontal >= 5))
		return;

	if (_vm->getRandomNumber(1, 1000) <= (200 + ((40 - (force->_vertical + force->_horizontal)) << 5))) {
		int id = -1;
		for (int i = 0; i < 100; i++) {
			int randIdx = _vm->getRandomNumber(0, 39);
			if (force->_seqId[randIdx] < 0) {
				id = randIdx;
				break;
			}
		}

		if (id < 0) {
			for (int i = 0; i < 40; i++) {
				if (force->_seqId[i] < 0) {
					id = i;
					break;
				}
			}
		}

		int speedX, speedY;
		int posX, posY;
		int randVal = _vm->getRandomNumber(1, 100);
		int spriteId;
		bool mirror;

		if (id >= 20) {
			spriteId = 2;
			mirror = (randVal <= 50);
			posX = mirror ? 315 : 5;
			posY = computeScale(15, 119, id - 20);
			speedX = 1000 * (mirror ? -1 : 1);
			speedY = 0;
		} else if (randVal <= 50) {
			spriteId = 1;
			mirror = false;
			posX = computeScale(21, 258, id);
			posY = 0;
			speedX = 0;
			speedY = 600;
		} else {
			spriteId = 0;
			mirror = false;
			posX = computeScale(21, 258, id);
			posY = 155;
			speedX = 0;
			speedY = -600;
		}

		if (id >= 0) {
			force->_seqId[id] = _scene->_sequences.addSpriteCycle(sprites[spriteId], mirror, 2, 0, 0, 0);
			_scene->_sequences.setDepth(force->_seqId[id], 8);
			_scene->_sequences.setPosition(force->_seqId[id], Common::Point(posX, posY));
			_scene->_sequences.setMotion(force->_seqId[id], 2, speedX, speedY);
			_scene->_sequences.addSubEntry(force->_seqId[id], SEQUENCE_TRIGGER_EXPIRE, 0, 150 + id);
			if (spriteId == 2)
				force->_horizontal++;
			else
				force->_vertical++;
		}
	}

	force->_timer = _scene->_frameStartTime + 4;
}

/*------------------------------------------------------------------------*/

void Scene300s::preActions() {
	_game._player._needToWalk = false;
}

/*------------------------------------------------------------------------*/

void Scene301::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene301::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);

	_globals[kMeteorologistStatus] = METEOROLOGIST_GONE;
	_globals[kTeleporterCommand] = TELEPORTER_NONE;

	_game._player._stepEnabled = false;
	_game._player._visible = false;
	_scene->loadAnimation(formAnimName('a', -1), 60);

	sceneEntrySound();
}

void Scene301::step() {
	if (_game._trigger == 60)
		_scene->_nextSceneId = 302;
}

/*------------------------------------------------------------------------*/

Scene302::Scene302(MADSEngine *vm) : Scene3xx(vm) {
	_oldFrame = 0;
}

void Scene302::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	s.syncAsSint32LE(_oldFrame);
}

void Scene302::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene302::enter() {
	_game._player._stepEnabled = false;
	_game._player._visible = false;

	_scene->loadAnimation(formAnimName('a', -1), 71);
	sceneEntrySound();
}

void Scene302::step() {
	if (_game._trigger == 71)
		_scene->_nextSceneId = 303;

	if ((_scene->_animation[0] != nullptr) && (_scene->_animation[0]->getCurrentFrame() != _oldFrame)) {
		_oldFrame = _scene->_animation[0]->getCurrentFrame();
		if (_oldFrame == 147) {
			_game._objects.setRoom(OBJ_POISON_DARTS, 1);
			_game._objects.setRoom(OBJ_BLOWGUN, 1);
			_game._objects.setRoom(OBJ_REBREATHER, 1);
			_game._objects.setRoom(OBJ_STUFFED_FISH, 1);
			_game._objects.setRoom(OBJ_DEAD_FISH, 1);
			_game._objects.setRoom(OBJ_BURGER, 1);

			int count = (int)_game._objects.size();
			for (int idx = 0; idx < count; idx++) {
				if (_game._objects.isInInventory(idx))
					_game._objects.setRoom(idx, 50);
			}
		}
	}
}

/*------------------------------------------------------------------------*/

void Scene303::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene303::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('b', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('b', 1));

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 10, 0, 50, 120);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 10, 0, 0, 0);

	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->loadAnimation(formAnimName('a', -1), 60);

	sceneEntrySound();
}

void Scene303::step() {
	if (_game._trigger == 60)
		_scene->_nextSceneId = 304;
}

/*------------------------------------------------------------------------*/

Scene304::Scene304(MADSEngine *vm) : Scene3xx(vm) {
	_explosionSpriteId = -1;
}

void Scene304::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	s.syncAsSint32LE(_explosionSpriteId);
}

void Scene304::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene304::enter() {
	if (_scene->_priorSceneId == 303) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->loadAnimation(formAnimName('a', -1), 60);
	} else {
		if (_globals[kSexOfRex] == REX_MALE)
			_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 0));
		else
			_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('a', 2));

		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 1));
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('b', 0));

		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 150, 0, 3, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 2);
		_vm->_palette->setEntry(252, 45, 63, 45);
		_vm->_palette->setEntry(253, 20, 45, 20);

		if (_globals[kSexOfRex] == REX_MALE)
			_game._player._playerPos = Common::Point(111, 117);
		else
			_game._player._playerPos = Common::Point(113, 116);

		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 11, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -1);
		_scene->_sequences.addTimer(48, 70);
	}

	sceneEntrySound();
	_game.loadQuoteSet(0xEB, 0xEC, 0);
}

void Scene304::step() {
	if (_game._trigger == 60)
		_scene->_nextSceneId = 311;

	if (_game._trigger >= 70) {
		switch (_game._trigger) {
		case 70: {
			_game._player._visible = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 2, 4);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			if (_globals[kSexOfRex] == REX_MALE)
				_explosionSpriteId = _globals._spriteIndexes[1];
			else
				_explosionSpriteId = _globals._spriteIndexes[4];

			int sprIdx = _scene->_sequences.addSpriteCycle(_explosionSpriteId, false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(sprIdx, -1, 4);
			_scene->_sequences.setDepth(sprIdx, 1);
			_scene->_sequences.addSubEntry(sprIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 74);
			}
			break;

		case 71:
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 60, _game.getQuote(0xEB));
			_scene->_sequences.addTimer(1, 72);
			break;

		case 72: {
			_vm->_sound->command(43);
			int sprIdx = _scene->_sequences.addSpriteCycle(_explosionSpriteId, false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(sprIdx, 5, -2);
			_scene->_sequences.setDepth(sprIdx, 1);
			_scene->_sequences.addSubEntry(sprIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 73);
			if (_game._storyMode == STORYMODE_NICE)
				_scene->_sequences.addSubEntry(sprIdx, SEQUENCE_TRIGGER_SPRITE, 8, 78);
			}
			break;

		case 73: {
			int sprIdx = _scene->_sequences.addSpriteCycle(_explosionSpriteId, false, 8, 0, 0, 0);
			_scene->_sequences.setAnimRange(sprIdx, -2, -2);
			_scene->_sequences.setDepth(sprIdx, 1);
			}
			break;

		case 74:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 5, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
			break;

		case 75:
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 2, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 76);
			break;

		case 76:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 0, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 2, 2);
			_scene->_sequences.addTimer(48, 77);
			break;

		case 77:
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(211, 45), 0xFDFC, 32, 0, 180, _game.getQuote(0xEC));
			_scene->_sequences.addTimer(120, 78);
			break;

		case 78:
			_scene->_nextSceneId = 316;
			break;

		default:
			break;
		}
	}
}

/*------------------------------------------------------------------------*/

Scene307::Scene307(MADSEngine *vm) : Scene3xx(vm) {
	_afterPeeingFl = false;
	_duringPeeingFl = false;
	_grateOpenedFl = false;
	_activePrisonerFl = false;

	_animationMode = -1;
	_prisonerMessageId = -1;
	_fieldCollisionCounter = -1;

	_lastFrameTime = 0;
	_guardTime = 0;
	_prisonerTimer = 0;

	_subQuote2 = "";

	_forceField.init();
}

void Scene307::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	_forceField.synchronize(s);

	s.syncAsByte(_afterPeeingFl);
	s.syncAsByte(_duringPeeingFl);
	s.syncAsByte(_grateOpenedFl);
	s.syncAsByte(_activePrisonerFl);

	s.syncAsSint32LE(_animationMode);
	s.syncAsSint32LE(_prisonerMessageId);
	s.syncAsSint32LE(_fieldCollisionCounter);

	s.syncAsUint32LE(_lastFrameTime);
	s.syncAsUint32LE(_guardTime);
	s.syncAsUint32LE(_prisonerTimer);

	s.syncString(_subQuote2);
}

void Scene307::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_AIR_VENT);
	_scene->addActiveVocab(VERB_CLIMB_INTO);
}

void Scene307::handleRexDialog(int quote) {
	Common::String curQuote = _game.getQuote(quote);
	if (_vm->_font->getWidth(curQuote, _scene->_textSpacing) > 200) {
		Common::String subQuote1;
		_game.splitQuote(curQuote, subQuote1, _subQuote2);
		_scene->_kernelMessages.add(Common::Point(0, -14), 0x1110, 34, 0, 240, subQuote1);
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 1, 180, _subQuote2);
	} else
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 1, 120, curQuote);
}

void Scene307::handlePrisonerEncounter() {
	switch (_action._activeAction._verbId) {
	case 275:
		setDialogNode(5);
		break;

	case 277:
		setDialogNode(4);
		break;

	case 276:
		setDialogNode(6);
		break;

	default:
		break;
	}
}

void Scene307::handlePrisonerSpeech(int firstQuoteId, int number, uint32 timeout) {
	int height = number * 14;
	int posY;

	if (height < 60)
		posY = 65 - height;
	else
		posY = 78 - (height / 2);

	_scene->_kernelMessages.reset();
	_activePrisonerFl = true;

	int quoteId = firstQuoteId;
	for (int count = 0; count < number; count++) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_kernelMessages.add(Common::Point(5, posY), 0xFDFC, 0, 81, timeout, _game.getQuote(quoteId));
		posY += 14;
		quoteId++;
	}
}

void Scene307::setDialogNode(int node) {
	switch (node) {
	case 0:
		handlePrisonerSpeech(0x153, 2, 120);
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	case 1:
		_globals[kMetBuddyBeast] = true;
		handlePrisonerSpeech(0x10F, 2, INDEFINITE_TIMEOUT);
		_dialog1.start();
		break;

	case 2:
		_globals[kMetBuddyBeast] = true;
		handlePrisonerSpeech(0x111, 2, INDEFINITE_TIMEOUT);
		_dialog1.start();
		break;

	case 4:
		handlePrisonerSpeech(0x116, 1, 120);
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	case 5:
		_globals[kKnowsBuddyBeast] = true;
		handlePrisonerSpeech(0x117, 2, INDEFINITE_TIMEOUT);
		_dialog2.start();
		break;

	case 6:
		handlePrisonerSpeech(0x123, 1, 120);
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	case 7:
		_globals[kKnowsBuddyBeast] = true;
		handlePrisonerSpeech(0x124, 10, INDEFINITE_TIMEOUT);
		_dialog2.write(0x11A, false);
		_dialog2.write(0x11B, true);
		_dialog2.write(0x120, true);
		_dialog2.start();
		break;

	case 8:
		handlePrisonerSpeech(0x12E, 6, INDEFINITE_TIMEOUT);
		_dialog2.write(0x11A, false);
		_dialog2.write(0x11B, false);
		_dialog2.write(0x11C, true);
		_dialog2.write(0x11D, true);
		_dialog2.write(0x11F, true);
		_dialog2.start();
		break;

	case 9:
		handlePrisonerSpeech(0x134, 4, INDEFINITE_TIMEOUT);
		_dialog2.write(0x11D, false);
		_dialog2.start();
		break;

	case 10:
		handlePrisonerSpeech(0x138, 6, INDEFINITE_TIMEOUT);
		_dialog2.write(0x11E, false);
		_dialog2.start();
		break;

	case 11:
		handlePrisonerSpeech(0x13E, 6, INDEFINITE_TIMEOUT);
		_dialog2.write(0x11F, false);
		_dialog2.write(0x121, true);
		_dialog2.start();
		break;

	case 12:
		handlePrisonerSpeech(0x144, 4, INDEFINITE_TIMEOUT);
		_dialog2.write(0x11C, false);
		_dialog2.start();
		break;

	case 13:
		handlePrisonerSpeech(0x148, 7, INDEFINITE_TIMEOUT);
		_dialog2.write(0x120, false);
		_dialog2.start();
		break;

	case 14:
		handlePrisonerSpeech(0x14F, 3, INDEFINITE_TIMEOUT);
		_dialog2.write(0x121, false);
		_dialog2.start();
		break;

	case 15:
		handlePrisonerSpeech(0x152, 1, 120);
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	case 16:
		_globals[kKnowsBuddyBeast] = true;
		handlePrisonerSpeech(0x10C, 1, INDEFINITE_TIMEOUT);
		_dialog2.start();
		break;

	default:
		break;
	}
}

void Scene307::handlePrisonerDialog() {
	switch (_action._activeAction._verbId) {
	case 0x11A:
		setDialogNode(7);
		break;

	case 0x11B:
		setDialogNode(8);
		break;

	case 0x11C:
		setDialogNode(12);
		break;

	case 0x11D:
		setDialogNode(9);
		break;

	case 0x11E:
		setDialogNode(10);
		break;

	case 0x11F:
		setDialogNode(11);
		break;

	case 0x120:
		setDialogNode(13);
		break;

	case 0x121:
		setDialogNode(14);
		break;

	case 0x122:
		setDialogNode(15);
		break;

	default:
		break;
	}
}

void Scene307::handleDialog() {
	if (_game._trigger == 0) {
		_scene->_kernelMessages.reset();
		_game._player._stepEnabled = false;
		handleRexDialog(_action._activeAction._verbId);
	} else {
		_game._player._stepEnabled = true;

		if (!_globals[kKnowsBuddyBeast]) {
			handlePrisonerEncounter();
		} else {
			handlePrisonerDialog();
		}
	}
}

void Scene307::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*SC003x0");
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*SC003x1");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*SC003x2");
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 0));

	initForceField(&_forceField, true);

	_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 15);

	_animationMode = 0;
	_fieldCollisionCounter = 0;

	_scene->changeVariant(1);

	_game.loadQuoteSet(0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0x10C, 0x104, 0x106, 0x107, 0x108, 0x105,
		0x109, 0x10A, 0x10B, 0x10D, 0x10E, 0x10F, 0x110, 0x111, 0x112, 0x113, 0x114, 0x115, 0x116, 0x117,
		0x118, 0x119, 0x11A, 0x11B, 0x11C, 0x11D, 0x11E, 0x11F, 0x120, 0x121, 0x122, 0x123, 0x124, 0x125,
		0x126, 0x127, 0x128, 0x129, 0x12A, 0x12B, 0x12C, 0x12D, 0x12E, 0x12F, 0x130, 0x131, 0x132, 0x133,
		0x134, 0x135, 0x136, 0x137, 0x138, 0x139, 0x13A, 0x13B, 0x13C, 0x13D, 0x13E, 0x13F, 0x140, 0x141,
		0x142, 0x143, 0x144, 0x145, 0x146, 0x147, 0x148, 0x149, 0x14A, 0x14B, 0x14C, 0x14D, 0x14E, 0x14F,
		0x150, 0x151, 0x152, 0x153, 0);

	_dialog1.setup(0x3F, 0x113, 0x114, 0x115, -1);
	_dialog2.setup(0x40, 0x11A, 0x11B, 0x11C, 0x11D, 0x11E, 0x11F, 0x120, 0x121, 0x122, 0);

	if (!_game._visitedScenes._sceneRevisited)
		_dialog2.set(0x11A, 0x122, 0);
	else if (_scene->_priorSceneId == 318)
		_dialog2.write(0x11E, true);


	if (_scene->_priorSceneId == RETURNING_FROM_DIALOG) {
		if (_grateOpenedFl)
			_vm->_sound->command(10);
		else
			_vm->_sound->command(3);
	} else {
		_afterPeeingFl = false;
		_duringPeeingFl = false;
		_guardTime = 0;
		_grateOpenedFl = false;
		_activePrisonerFl = false;
		_prisonerTimer = 0;
		_prisonerMessageId = 0x104;

		if (_scene->_priorSceneId == 308) {
			_game._player._visible = false;
			_game._player._stepEnabled = false;
			_game._player._playerPos = Common::Point(156, 113);
			_game._player._facing = FACING_NORTH;
			_animationMode = 1;
			_vm->_sound->command(11);
			_scene->loadAnimation(formAnimName('a', -1), 60);
		} else if (_scene->_priorSceneId == 387) {
			_game._player._playerPos = Common::Point(129, 108);
			_game._player._facing = FACING_NORTH;
			_vm->_sound->command(3);
			_grateOpenedFl = true;
		} else {
			_game._player._playerPos = Common::Point(159, 109);
			_game._player._facing = FACING_SOUTH;
			_vm->_sound->command(3);
		}
	}

	if (_grateOpenedFl) {
		_scene->_hotspots.activate(17, false);

		int idx = _scene->_dynamicHotspots.add(17, VERB_CLIMB_INTO, -1, Common::Rect(117, 67, 117 + 19, 67 + 13));
		int hotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(129, 104), FACING_NORTH);
		_scene->_dynamicHotspots.setCursor(hotspotId, CURSOR_GO_UP);

		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 2);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 15);
	}

	_vm->_palette->setEntry(252, 63, 30, 20);
	_vm->_palette->setEntry(253, 45, 15, 12);

	sceneEntrySound();

	if ((_scene->_priorSceneId == 318) || (_scene->_priorSceneId == 387))
		_scene->_kernelMessages.addQuote(0xF3, 0, 120);
}

void Scene307::step() {
	handleForceField(&_forceField, &_globals._spriteIndexes[0]);

	if ((_animationMode == 1) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() == 126) {
			_forceField._flag = false;
			_vm->_sound->command(5);
		}

		if (_scene->_animation[0]->getCurrentFrame() == 194) {
			_forceField._flag = true;
			_vm->_sound->command(24);
		}
	}

	if ((_animationMode == 2) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() == 54)
			_forceField._flag = false;

		if (_scene->_animation[0]->getCurrentFrame() == 150) {
			_game._player._visible = false;
			_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		}
	}

	if (_game._trigger == 60) {
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_animationMode = 0;
		_vm->_sound->command(9);
	}

	if ((_lastFrameTime != _scene->_frameStartTime) && !_duringPeeingFl) {
		int32 elapsedTime = _lastFrameTime - _scene->_frameStartTime;
		if ((elapsedTime > 0) && (elapsedTime <= 4)) {
			_guardTime += elapsedTime;
			_prisonerTimer += elapsedTime;
		} else {
			_guardTime++;
			_prisonerTimer++;
		}
		_lastFrameTime = _scene->_frameStartTime;

		if ((_guardTime > 3000) && !_duringPeeingFl && (_scene->_animation[0] == nullptr)
		&& (_game._screenObjects._inputMode != kInputConversation) && _globals[kMetBuddyBeast] && !_activePrisonerFl) {
			if (!_game._objects.isInInventory(OBJ_SCALPEL) && !_grateOpenedFl) {
				_game._player._stepEnabled = false;
				_game._player.walk(Common::Point(151, 119), FACING_SOUTHEAST);
				_animationMode = 2;
				_vm->_sound->command(11);
				_scene->loadAnimation(formAnimName('b', -1), 70);
			}
			_guardTime = 0;
		} else if ((_prisonerTimer > 300) && (_game._screenObjects._inputMode != kInputConversation) && (_scene->_animation[0] == nullptr) && !_activePrisonerFl) {
			if (!_globals[kMetBuddyBeast]) {
				if (_prisonerMessageId == -1)
					_prisonerMessageId = 0x104;

				int idx = _scene->_kernelMessages.add(Common::Point(5, 51), 0xFDFC, 0, 81, 120, _game.getQuote(_prisonerMessageId));
				_scene->_kernelMessages.setQuoted(idx, 4, true);
				_prisonerMessageId++;
				if (_prisonerMessageId > 0x10A)
					_prisonerMessageId = 0x104;
			} else if (_globals[kKnowsBuddyBeast] && (_dialog2.read(0) > 1) && (_vm->getRandomNumber(1, 3) == 1)) {
				int idx = _scene->_kernelMessages.add(Common::Point(5, 51), 0xFDFC, 0, 81, 120, _game.getQuote(267));
				_scene->_kernelMessages.setQuoted(idx, 4, true);
			}
			_prisonerTimer = 0;
		}
	}

	if (_game._trigger == 70)
		_scene->_nextSceneId = 318;

	if (_game._trigger == 81) {
		_prisonerTimer = 0;
		if (_activePrisonerFl && (_guardTime > 2600))
			_guardTime = 3000 - _vm->getRandomNumber(1, 800);

		_activePrisonerFl = false;
	}
}

void Scene307::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(30715);
	else if (_game._screenObjects._inputMode == kInputConversation)
		handleDialog();
	else if (_action.isAction(VERB_TALKTO, NOUN_CELL_WALL) || _action.isAction(VERB_TALKTO, NOUN_WALL) || _action.isAction(VERB_TALKTO, NOUN_TOILET)) {
		int node, say;
		if (_globals[kKnowsBuddyBeast]) {
			say = 0x10E;
			node = 16;
		} else if (_globals[kMetBuddyBeast]) {
			say = 0x10E;
			node = 2;
		} else {
			say = 0x10D;
			node = 1;
		}

		switch (_game._trigger) {
		case 0:
			handleRexDialog(say);
			break;

		case 1:
			setDialogNode(node);
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PRY, NOUN_SCALPEL, NOUN_AIR_VENT)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(239));
			_scene->_sequences.addTimer(120, 1);
			break;

		case 1:
			_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RXCL_8");
			_game._player._visible = false;
			_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, 3);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2: {
			int oldIdx = _globals._sequenceIndexes[5];
			_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], false, 12, 6, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 2, 3);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], oldIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			}
			break;

		case 3: {
			int oldIdx = _globals._sequenceIndexes[5];
			_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], oldIdx);
			_scene->_sequences.addTimer(48, 4);
			}
			break;

		case 4:
			_vm->_sound->command(26);
			_scene->_sequences.remove(_globals._sequenceIndexes[4]);
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 15);
			_scene->_sequences.addTimer(90, 5);
			break;

		case 5:
			_vm->_sound->command(10);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(241));
			_scene->_sequences.addTimer(120, 6);
			break;

		case 6: {
			_game._player._visible = true;
			_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
			_scene->_sequences.remove(_globals._sequenceIndexes[5]);
			_grateOpenedFl = true;
			_scene->_hotspots.activate(17, false);
			int idx = _scene->_dynamicHotspots.add(17, VERB_CLIMB_INTO, -1, Common::Rect(117, 67, 117 + 19, 67 + 13));
			int hotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(129, 104), FACING_NORTH);
			_scene->_dynamicHotspots.setCursor(hotspotId, CURSOR_GO_UP);
			_game._objects.removeFromInventory(OBJ_SCALPEL, NOWHERE);
			_scene->_kernelMessages.addQuote(0xF2, 7, 120);
			}
			break;

		case 7:
			_scene->_sprites.remove(_globals._spriteIndexes[5]);
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	}else if (_action.isAction(VERB_CLIMB_INTO, NOUN_AIR_VENT)) {
		if (_grateOpenedFl) {
			switch (_game._trigger) {
			case 0:
				_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RXCL_8");
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[4]);
				_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 60, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 3, -2);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 15);
				_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 18, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, 4);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 15);
				break;

			case 2: {
				int oldIdx = _globals._sequenceIndexes[5];
				_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 12, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 4, 10);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
				_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], oldIdx);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				}
				break;

			case 3:
				_scene->_sequences.remove(_globals._sequenceIndexes[4]);
				_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 3);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
				_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 11);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[5], Common::Point(129, 102));
				_scene->_sequences.addTimer(48, 4);
				break;

			case 4:
				_scene->_sequences.remove(_globals._sequenceIndexes[4]);
				_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 2);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
				_scene->_sequences.remove(_globals._sequenceIndexes[5]);
				_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 12, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 12, 14);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[5], Common::Point(129, 102));
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
				break;

			case 5:
				_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 15);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[5], Common::Point(129, 102));
				_scene->_sequences.addTimer(48, 6);
				break;

			case 6:
				_scene->_sequences.remove(_globals._sequenceIndexes[5]);
				_scene->_sequences.remove(_globals._sequenceIndexes[4]);
				_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
				_scene->_sequences.addTimer(48, 7);
				break;

			case 7:
				_scene->_nextSceneId = 313;
				break;

			default:
				break;
			}
		}
	} else if (_action.isAction(VERB_USE, NOUN_TOILET) && (_game._storyMode != STORYMODE_NAUGHTY))
		_vm->_dialogs->show(30723);
	else if (_action.isAction(VERB_USE, NOUN_TOILET)) {
		if (!_afterPeeingFl) {
			switch (_game._trigger) {
			case 0:
				_vm->_sound->command(25);
				_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 0));
				_duringPeeingFl = true;
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, 2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 9, 5, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 3, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2:
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);
				_game._player._visible = true;
				_scene->_sequences.addTimer(48, 3);
				break;

			case 3: {
				_scene->_sprites.remove(_globals._spriteIndexes[3]);
				_scene->_kernelMessages.reset();
				int idx = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 4, 120, _game.getQuote(237));
				_scene->_kernelMessages.setQuoted(idx, 4, true);
				}
				break;

			case 4:
				_game._player._stepEnabled = true;
				_duringPeeingFl = false;
				_afterPeeingFl = true;
				break;

			default:
				break;
			}
		} else {
			_scene->_kernelMessages.reset();
			int idx = _scene->_kernelMessages.add(Common::Point(85, 39), 0x1110, 0, 0, 180, _game.getQuote(238));
			_scene->_kernelMessages.setQuoted(idx, 4, true);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_AIR_VENT)) {
		if (!_grateOpenedFl)
			_vm->_dialogs->show(30710);
		else
			_vm->_dialogs->show(30711);
	} else if (_action.isAction(VERB_LOOK, NOUN_BED))
		_vm->_dialogs->show(30712);
	else if (_action.isAction(VERB_LOOK, NOUN_SINK))
		_vm->_dialogs->show(30713);
	else if (_action.isAction(VERB_LOOK, NOUN_TOILET))
		_vm->_dialogs->show(30714);
	else if (_action.isAction(VERB_SHARPEN, NOUN_SCALPEL))
		_vm->_dialogs->show(30716);
	else if (_action.isAction(VERB_LOOK, NOUN_CELL_WALL))
		_vm->_dialogs->show(30717);
	else if (_action.isAction(VERB_LOOK, NOUN_LIGHT))
		_vm->_dialogs->show(30718);
	else if (_action.isAction(VERB_WALK_INTO, NOUN_CORRIDOR)) {
		switch (_fieldCollisionCounter) {
		case 0:
			_vm->_dialogs->show(30719);
			_fieldCollisionCounter = 1;
			break;

		case 1:
			_vm->_dialogs->show(30720);
			_fieldCollisionCounter = 2;
			break;

		case 2:
			_vm->_dialogs->show(30721);
			_fieldCollisionCounter = 3;
			break;

		case 3:
			_vm->_dialogs->show(30722);
			break;

		default:
			break;
		}
	} else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene308::Scene308(MADSEngine *vm) : Scene3xx(vm) {
	_forceField.init();
}

void Scene308::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	_forceField.synchronize(s);
}


void Scene308::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene308::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*SC003x0");
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*SC003x1");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*SC003x2");

	initForceField(&_forceField, true);

	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('b', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));

	_vm->_palette->setEntry(252, 63, 30, 20);
	_vm->_palette->setEntry(253, 45, 15, 12);

	_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 15);
	_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
	_scene->_sequences.addTimer(48, 70);

	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->loadAnimation(formAnimName('a', -1), 60);

	sceneEntrySound();
	_game.loadQuoteSet(0xF4, 0xF5, 0xF6, 0);
}

void Scene308::step() {
	handleForceField(&_forceField, &_globals._spriteIndexes[0]);

	if (_game._trigger == 60)
		_scene->_nextSceneId = 307;

	if (_game._trigger < 70)
		return;

	switch (_game._trigger) {
	case 70: {
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 18, 9, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 2, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
		_scene->_kernelMessages.reset();
		int idx = _scene->_kernelMessages.add(Common::Point(171, 21), 0xFDFC, 0, 0, 120, _game.getQuote(244));
		_scene->_kernelMessages.setQuoted(idx, 2, true);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		}
		break;

	case 71: {
		int seqIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 4);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], seqIdx);
		_scene->_sequences.addTimer(48, 72);
		}
		break;

	case 72:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 20, 5, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 3, 4);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
		_scene->_kernelMessages.reset();
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 73);
		break;

	case 73: {
		int seqIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], seqIdx);
		_scene->_sequences.addTimer(48, 74);
		}
		break;

	case 74: {
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 20, 8, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 6, 7);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
		_scene->_kernelMessages.reset();
		int idx = _scene->_kernelMessages.add(Common::Point(171, 21), 0xFDFC, 0, 0, 120, _game.getQuote(245));
		_scene->_kernelMessages.setQuoted(idx, 2, true);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
		}
		break;

	case 75: {
		int seqIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 23, 5, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 8, 10);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], seqIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 76);
		}
		break;

	case 76: {
		int seqIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 26, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 2, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
		_scene->_kernelMessages.reset();
		int idx = _scene->_kernelMessages.add(Common::Point(171, 21), 0xFDFC, 0, 0, 120, _game.getQuote(246));
		_scene->_kernelMessages.setQuoted(idx, 2, true);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], seqIdx);
		}
		break;

	default:
		break;
	}
}

/*------------------------------------------------------------------------*/

Scene309::Scene309(MADSEngine *vm) : Scene3xx(vm) {
	for (int i = 0; i < 3; i++) {
		_characterSpriteIndexes[i] = -1;
		_messagesIndexes[i] = -1;
	}

	_lastFrame = -1;
	_forceField.init();
}

void Scene309::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	_forceField.synchronize(s);

	for (int i = 0; i < 3; ++i)
		s.syncAsSint32LE(_characterSpriteIndexes[i]);
	for (int i = 0; i < 3; ++i)
		s.syncAsSint32LE(_messagesIndexes[i]);
	s.syncAsSint32LE(_lastFrame);
}

void Scene309::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene309::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*SC003x0");
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*SC003x1");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*SC003x2");

	initForceField(&_forceField, true);

	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));

	_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 15);

	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, 3);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 3, 70);

	_vm->_palette->setEntry(252, 63, 37, 26);
	_vm->_palette->setEntry(253, 45, 24, 17);
	_vm->_palette->setEntry(16, 63, 63, 63);
	_vm->_palette->setEntry(17, 45, 45, 45);
	_vm->_palette->setEntry(250, 63, 20, 20);
	_vm->_palette->setEntry(251, 45, 10, 10);

	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->loadAnimation(formAnimName('a', -1), 60);

	_characterSpriteIndexes[0] = _scene->_animation[0]->_spriteListIndexes[2];
	_characterSpriteIndexes[1] = _scene->_animation[0]->_spriteListIndexes[2];
	_characterSpriteIndexes[2] = _scene->_animation[0]->_spriteListIndexes[1];

	_messagesIndexes[0] = -1;
	_messagesIndexes[1] = -1;
	_messagesIndexes[2] = -1;

	sceneEntrySound();

	_game.loadQuoteSet(0xF7, 0xF8, 0xF9, 0x15C, 0x15D, 0x15E, 0);
}

void Scene309::step() {
	handleForceField(&_forceField, &_globals._spriteIndexes[0]);

	if (_game._trigger == 61) {
		_messagesIndexes[0] = -1;
		_messagesIndexes[1] = -1;
	}

	if (_game._trigger == 62)
		_messagesIndexes[2] = -1;

	if (_scene->_animation[0] != nullptr) {
		if (_lastFrame != _scene->_animation[0]->getCurrentFrame()) {
			_lastFrame = _scene->_animation[0]->getCurrentFrame();
			if (_lastFrame == 39) {
				_messagesIndexes[0] = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 32, 61, 210, _game.getQuote(348));
				_messagesIndexes[1] = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 32, 0, 210, _game.getQuote(349));
			}

			if (_lastFrame == 97)
				_messagesIndexes[2] = _scene->_kernelMessages.add(Common::Point(0, 0), 0xFBFA, 32, 62, 180, _game.getQuote(350));

			for (int charIdx = 0; charIdx < 3; charIdx++) {
				if (_messagesIndexes[charIdx] >= 0) {
					bool match = false;
					int j = -1;
					for (j = _scene->_animation[0]->_oldFrameEntry; j < _scene->_animation[0]->_header._frameEntriesCount; j++) {
						if (_scene->_animation[0]->_frameEntries[j]._spriteSlot._spritesIndex == _characterSpriteIndexes[charIdx]) {
							match = true;
							break;
						}
					}

					if (match) {
						SpriteSlotSubset *curSpriteSlot = &_scene->_animation[0]->_frameEntries[j]._spriteSlot;
						_scene->_kernelMessages._entries[_messagesIndexes[charIdx]]._position.x = curSpriteSlot->_position.x;
						_scene->_kernelMessages._entries[_messagesIndexes[charIdx]]._position.y = curSpriteSlot->_position.y - (50 + (14 * ((charIdx == 0) ? 2 : 1)));
					}
				}
			}
		}
	}

	if (_game._trigger >= 70) {
		switch (_game._trigger) {
		case 70: {
			int idx = _scene->_dynamicHotspots.add(689, 690, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(142, 146), FACING_NORTHEAST);
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 7, 4, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 2, 3);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			}
			break;

		case 71: {
			int _oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 4, 7);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], _oldIdx);
			int idx = _scene->_kernelMessages.add(Common::Point(85, 37), 0xFDFC, 0, 0, 120, _game.getQuote(248));
			_scene->_kernelMessages.setQuoted(idx, 2, true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
			}
			break;

		case 72: {
			int _oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 7, 8, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 8, 11);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], _oldIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 73);
			}
			break;

		case 73: {
			int _oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 7, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 12, 20);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], _oldIdx);
			int idx = _scene->_kernelMessages.add(Common::Point(170, 49), 0xFDFC, 0, 0, 120, _game.getQuote(249));
			_scene->_kernelMessages.setQuoted(idx, 2, true);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 74);
			}
			break;

		case 74: {
			int _oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 7, 6, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 21, 23);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], _oldIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
			}
			break;

		case 75: {
			int _oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 12, 6, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 24, 25);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], _oldIdx);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 76);
			}
			break;

		case 76: {
			int _oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 26, 28);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], _oldIdx);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 77);
			}
			break;

		case 77: {
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 90, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 29, 30);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			int idx = _scene->_kernelMessages.add(Common::Point(15, 46), 0xFDFC, 0, 0, 120, _game.getQuote(247));
			_scene->_kernelMessages.setQuoted(idx, 2, true);
			_scene->_sequences.addTimer(120, 78);
			}
			break;

		default:
			break;
		}
	}

	if (_game._trigger == 60)
		_scene->_nextSceneId = 308;
}

/*------------------------------------------------------------------------*/

Scene310::Scene310(MADSEngine *vm) : Scene3xx(vm) {
	_forceField.init();
}

void Scene310::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	_forceField.synchronize(s);
}

void Scene310::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene310::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*SC003x0");
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*SC003x1");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*SC003x2");

	initForceField(&_forceField, true);

	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));
	_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 15);

	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->loadAnimation(formAnimName('a', -1), 70);

	sceneEntrySound();
}

void Scene310::step() {
	handleForceField(&_forceField, &_globals._spriteIndexes[0]);

	if (_game._trigger == 70)
		_scene->_nextSceneId = 309;
}

/*------------------------------------------------------------------------*/

Scene311::Scene311(MADSEngine *vm) : Scene3xx(vm) {
	_checkGuardFl = false;
}

void Scene311::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	s.syncAsByte(_checkGuardFl);
}

void Scene311::setup() {
	if (_scene->_currentSceneId == 391)
		_globals[kSexOfRex] = REX_MALE;

	setPlayerSpritesPrefix();
	setAAName();

	if (_scene->_currentSceneId == 304)
		_game._player._spritesPrefix = "";
}

void Scene311::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXCL_8");
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXCL_2");

	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

	_checkGuardFl = false;
	_game.loadQuoteSet(0xFA, 0);

	if (_scene->_priorSceneId == 391) {
		_globals[kSexOfRex] = REX_MALE;
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_game._player._facing = FACING_SOUTH;
		_game._player._playerPos = Common::Point(166, 101);
		_scene->_sequences.addTimer(120, 71);
	} else if (_scene->_priorSceneId == 310)
		_game._player._playerPos = Common::Point(302, 145);
	else if (_scene->_priorSceneId == 320) {
		_game._player._playerPos = Common::Point(129, 113);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->loadAnimation(formAnimName('a', -1), 70);
	}

	sceneEntrySound();
}

void Scene311::step() {
	if (_game._trigger == 70)
		_scene->_nextSceneId = 310;

	if (_game._trigger >= 71) {
		switch (_game._trigger) {
		case 71:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 72);
			break;

		case 72:
			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 73);
			break;

		case 73:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 3);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 3);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 74);
			break;

		case 74:
			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 4, 5);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
			break;

		case 75: {
			int oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 6);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
			_scene->_sequences.addTimer(15, 76);
			}
			break;

		case 76:
			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 7);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 77);
			break;

		case 77:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 8);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 78);
			break;

		case 78:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 9);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 79);
			break;

		case 79:
			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 10, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
			break;

		case 80:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);
			_game._player._stepEnabled = true;
			_game._player._visible = true;
			break;

		default:
			break;
		}
	}

	if (_game._player._moving && (_scene->_rails.getNext() > 0)) {
		int x = _game._player._prepareWalkPos.x;
		if (x < 75)
			x = 75;
		if (x > 207)
			x = 207;

		_checkGuardFl = true;
		_game._player.startWalking(Common::Point(x, 122), FACING_SOUTH);
		_scene->_rails.resetNext();
	}
}

void Scene311::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(31119);
	else if (_checkGuardFl) {
		_checkGuardFl = false;
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.addQuote(250, 0, 240);
	} else if (_action.isAction(VERB_SIT_AT, NOUN_DESK))
		_scene->_nextSceneId = 320;
	else if (_action.isAction(VERB_CLIMB_INTO, NOUN_AIR_VENT)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 50, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 3, -2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 15, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, 4);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1: {
			int oldIdx = _globals._sequenceIndexes[1];
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], oldIdx);
			}
			break;

		case 2: {
			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 4, 10);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			}
			break;

		case 3: {
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 3);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 11);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(167, 100));
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
			_scene->_sequences.addTimer(15, 4);
			}
			break;

		case 4:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 12, 14);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(167, 100));
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
			break;

		case 5: {
			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 15);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(167, 100));
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
			_scene->_sequences.addTimer(15, 6);
			}
			break;

		case 6:
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
			_scene->_sequences.addTimer(15, 7);
			break;

		case 7:
			_scene->_nextSceneId = 313;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_DESK))
		_vm->_dialogs->show(31110);
	else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(31111);
	else if (_action.isAction(VERB_LOOK, NOUN_LIGHTING_FIXTURE) || _action.isAction(VERB_STARE_AT, NOUN_LIGHTING_FIXTURE))
		_vm->_dialogs->show(31112);
	else if (_action.isAction(VERB_LOOK, NOUN_LIGHTS) || _action.isAction(VERB_STARE_AT, NOUN_LIGHTS))
		_vm->_dialogs->show(31113);
	else if (_action.isAction(VERB_TAKE, NOUN_LIGHTS))
		_vm->_dialogs->show(31114);
	else if (_action.isAction(VERB_LOOK, NOUN_LIGHT) || _action.isAction(VERB_STARE_AT, NOUN_LIGHT))
		_vm->_dialogs->show(31115);
	else if (_action.isAction(VERB_TAKE, NOUN_LIGHT))
		_vm->_dialogs->show(31116);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_WEST))
		_vm->_dialogs->show(31117);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_EAST))
		_vm->_dialogs->show(31118);
	else if (_action.isAction(VERB_LOOK, NOUN_AIR_VENT))
		_vm->_dialogs->show(31120);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene313::setup() {
	setPlayerSpritesPrefix();
	_game._player._spritesPrefix = "RM313A";
	setAAName();
}

void Scene313::enter() {
	_scene->_userInterface.setup(kInputLimitedSentences);

	if ((_scene->_priorSceneId == 366) || (_scene->_priorSceneId == 316)) {
		_game._player._playerPos = Common::Point(30, 80);
		_game._player._facing = FACING_NORTH;
	} else if ((_scene->_priorSceneId == 311) || (_scene->_priorSceneId == 361) || (_scene->_priorSceneId == 391)) {
		_game._player._playerPos = Common::Point(90, 70);
		_game._player._facing = FACING_EAST;
	} else if (_scene->_priorSceneId == 390) {
		_game._player._playerPos = Common::Point(126, 70);
		_game._player._facing = FACING_EAST;
	} else if ((_scene->_priorSceneId == 389) || (_scene->_priorSceneId == 399)) {
		_game._player._playerPos = Common::Point(163, 70);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId == 388) {
		_game._player._playerPos = Common::Point(199, 70);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(234, 70);
		_game._player._facing = FACING_WEST;
	}

	if (_globals[kAfterHavoc]) {
		for (uint16 i = 0; i < _scene->_paletteCycles.size(); i++) {
			int palIdx = _scene->_paletteCycles[i]._firstColorIndex;
			int size = _scene->_paletteCycles[i]._colorCount * 3;
			memset(&_vm->_palette->_cyclingPalette[palIdx], 0, size);
			memset(&_vm->_palette->_mainPalette[palIdx], 0, size);
		}
	}

	sceneEntrySound();
}

void Scene313::actions() {
	if (_action.isAction(VERB_CRAWL_TO, NOUN_FOURTH_CELL))
		_scene->_nextSceneId = 387;
	else if (_action.isAction(VERB_CRAWL_TO, NOUN_THIRD_CELL))
		_scene->_nextSceneId = 388;
	else if (_action.isAction(VERB_CRAWL_TO, NOUN_SECOND_CELL)) {
		if (_globals[kAfterHavoc])
			_scene->_nextSceneId = 399;
		else
			_scene->_nextSceneId = 389;
	} else if (_action.isAction(VERB_CRAWL_TO, NOUN_FIRST_CELL))
		_scene->_nextSceneId = 390;
	else if (_action.isAction(VERB_CRAWL_TO, NOUN_SECURITY_STATION)) {
		if (_globals[kSexOfRex] == REX_FEMALE) {
			_globals[kSexOfRex] = REX_MALE;
			_vm->_dialogs->show(31301);
		}
		_scene->_nextSceneId = 391;
	} else if (_action.isAction(VERB_CRAWL_TO, NOUN_EQUIPMENT_ROOM)) {
		if (_globals[kSexOfRex] == REX_FEMALE) {
			_globals[kSexOfRex] = REX_MALE;
			_vm->_dialogs->show(31301);
		}
		_scene->_nextSceneId = 366;
	} else if (!_action.isAction(VERB_CRAWL_DOWN, NOUN_AIR_SHAFT))
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene316::setup() {
	if (_scene->_currentSceneId == 366)
		_globals[kSexOfRex] = REX_MALE;

	setPlayerSpritesPrefix();
	setAAName();
}

void Scene316::handleRexInGrate() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.addTimer(15, 1);
		break;

	case 1:
		_scene->_sequences.setDone(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 12, 3, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 2, 3);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		break;

	case 2: {
		int oldIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 4, 8);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], oldIdx);

		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 2, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
		}
		break;

	case 3: {
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		}
		break;

	case 4: {
		int oldIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 10, 11);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], oldIdx);

		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 5, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
		}
		break;

	case 5: {
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);

		oldIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 12);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], oldIdx);
		_scene->_sequences.addTimer(15, 6);
		}
		break;

	case 6:
		_scene->_sequences.setDone(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 13, 14);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);

		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 2, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 7);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 8);
		break;

	case 7:
		_scene->_sequences.setDone(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 15);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], -1);
		break;

	case 8: {
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addTimer(15, 9);
		}
		break;

	case 9:
		_scene->_sequences.setDone(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.addTimer(48, 10);
		break;

	case 10:
		_scene->_nextSceneId = 313;
		break;

	default:
		break;
	}
}

void Scene316::handleRoxInGrate() {
	int temp;
	int temp1;

	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.addTimer(15, 1);
		break;

	case 1:
		_scene->_sequences.setDone(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], false, 17, 3, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 2, 3);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		break;

	case 2:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 17, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 2, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);

		temp = _globals._sequenceIndexes[5];
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 17, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 4, 8);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], temp);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
		break;

	case 3:
		temp1 = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], temp1);
		break;

	case 4:
		temp = _globals._sequenceIndexes[5];
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 17, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 10, 11);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], temp);

		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 17, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 5, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
		break;

	case 5:
		temp = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], temp);

		temp = _globals._sequenceIndexes[5];
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 12);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], temp);
		_scene->_sequences.addTimer(20, 6);
		break;

	case 6:
		_scene->_sequences.setDone(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 17, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 13, 15);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);

		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 17, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 2, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 7);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 8);
		break;

	case 7:
		_scene->_sequences.setDone(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 16);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], -1);
		break;

	case 8:
		temp = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], temp);
		_scene->_sequences.addTimer(20, 9);
		break;

	case 9:
		_scene->_sequences.setDone(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
		_scene->_sequences.addTimer(48, 10);
		break;

	case 10:
		_scene->_nextSceneId = 313;
		break;

	default:
		break;
	}
}

void Scene316::enter() {
	if (_globals[kSexOfRex] == REX_MALE) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('g', -1));
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXCL_8");
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*RXCL_2");
	} else {
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('g', 0));
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*ROXCL_8");
	}

	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('v', 0));
	_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);

	if (_scene->_priorSceneId == 366) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_game._player._playerPos = Common::Point(78, 87);
		_game._player._facing = FACING_SOUTH;
		_scene->_sequences.addTimer(48, 70);
	} else if (_scene->_priorSceneId == 321) {
		_game._player._playerPos = Common::Point(153, 102);
		_game._player._facing = FACING_SOUTH;
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_vm->_sound->command(44);
		int spriteIdx = (_globals[kSexOfRex] == REX_MALE) ? 1 : 2;
		_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[spriteIdx], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 60);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(291, 126);

	sceneEntrySound();
	_game.loadQuoteSet(0xFD, 0);
}

void Scene316::step() {
	if (_game._trigger == 60) {
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[1]);
		_game._player._visible = true;
		_game._player._stepEnabled = true;
	}

	if (_game._trigger >= 70) {
		switch (_game._trigger) {
		case 70:
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 2, 6);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);

			_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 5);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[6]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
			break;

		case 71: {
			int synxIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 3);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], synxIdx);
			}
			break;

		case 72: {
			int synxIdx = _globals._sequenceIndexes[6];
			_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 6, 9);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[6]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[6], synxIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 73);
			}
			break;

		case 73:
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 5);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[6]);

			_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 10, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[6]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 74);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
			break;

		case 74: {
			int synxIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 12);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], synxIdx);
			}
			break;

		case 75:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[6]);
			_game._player._stepEnabled = true;
			_game._player._visible = true;
			break;

		default:
			break;
		}
	}
}

void Scene316::preActions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_EAST)) {
		if (_globals[kAfterHavoc])
			_game._player._walkOffScreenSceneId = 354;
		else
			_game._player._walkOffScreenSceneId = 304;
	}
}

void Scene316::actions() {
	if (_action.isAction(VERB_CLIMB_INTO, NOUN_AIR_VENT)) {
		if (_globals[kSexOfRex] == REX_FEMALE)
			handleRoxInGrate();
		else
			handleRexInGrate();
	} else if (_action.isAction(VERB_WALK_UP, NOUN_RAMP) || _action.isAction(VERB_WALK_ONTO, NOUN_PLATFORM)) {
		switch (_game._trigger) {
		case 0:
			if (_globals[kCityFlooded]) {
				_vm->_dialogs->show(31623);
			} else {
				_vm->_sound->command(45);
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				if (_globals[kSexOfRex] == REX_MALE) {
					_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 7, 1, 0, 0);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -1, 7);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				} else {
					_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 1, 0, 0);
					_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				}
			}
			break;

		case 1:
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 7, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 8, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
			_scene->_kernelMessages.reset();
			if (!_game._visitedScenes.exists(321))
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(253));

			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 7, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], -2, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
			_scene->_sequences.addTimer(48, 4);
			break;

		case 3:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 7, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -2, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 2);
			_scene->_sequences.addTimer(48, 4);
			break;

		case 4:
			_scene->_nextSceneId = 321;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_PLATFORM))
		_vm->_dialogs->show(31610);
	else if (_action.isAction(VERB_LOOK, NOUN_STRANGE_DEVICE)) {
		if (_game._visitedScenes.exists(321))
			_vm->_dialogs->show(31612);
		else
			_vm->_dialogs->show(31611);
	} else if (_action.isAction(VERB_LOOK, NOUN_CONTROLS))
		_vm->_dialogs->show(31613);
	else if (_action.isAction(VERB_LOOK, NOUN_EQUIPMENT))
		_vm->_dialogs->show(31614);
	else if (_action.isAction(VERB_LOOK, NOUN_PANEL))
		_vm->_dialogs->show(31615);
	else if (_action.isAction(VERB_LOOK, NOUN_MONITOR))
		_vm->_dialogs->show(31616);
	else if (_action.isAction(VERB_LOOK, NOUN_RAMP))
		_vm->_dialogs->show(31617);
	else if (_action.isAction(VERB_LOOK, NOUN_AIR_VENT))
		_vm->_dialogs->show(31618);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_EAST)) {
		if (!_globals[kAfterHavoc]) {
			if (_game._difficulty != DIFFICULTY_EASY)
				_vm->_dialogs->show(31620);
			else
				_vm->_dialogs->show(31619);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_FLOOR))
		_vm->_dialogs->show(31621);
	else if (_action.isAction(VERB_LOOK, NOUN_SUPPORT))
		_vm->_dialogs->show(31622);
	else
		return;

	_action._inProgress = false;
}


/*------------------------------------------------------------------------*/

Scene318::Scene318(MADSEngine *vm) : Scene3xx(vm) {
	_dropTimer = 0;

	_lastFrame = -1;
	_animMode = -1;
	_internCounter = -1;
	_counter = -1;

	_dialogFl = false;
	_internTalkingFl = false;
	_internWalkingFl = false;
	_internVisibleFl = false;
	_explosionFl = false;

	_lastFrameCounter = 0;

	_subQuote2 = "";
}

void Scene318::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	s.syncAsUint32LE(_dropTimer);

	s.syncAsSint32LE(_lastFrame);
	s.syncAsSint32LE(_animMode);
	s.syncAsSint32LE(_internCounter);
	s.syncAsSint32LE(_counter);

	s.syncAsByte(_dialogFl);
	s.syncAsByte(_internTalkingFl);
	s.syncAsByte(_internWalkingFl);
	s.syncAsByte(_internVisibleFl);
	s.syncAsByte(_explosionFl);

	s.syncAsUint32LE(_lastFrameCounter);

	s.syncString(_subQuote2);
}

void Scene318::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene318::handleDialog() {
	if (!_game._trigger) {
		_game._player._stepEnabled = false;
		handleRexDialogs(_action._activeAction._verbId);
	} else if (_game._trigger == 2) {
		int synxIdx = _globals._sequenceIndexes[2];
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(142, 121));
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], synxIdx);
		_vm->_sound->command(3);
		_scene->_userInterface.setup(kInputBuildingSentences);
		_game._player._stepEnabled = true;
	} else {
		if (_action._activeAction._verbId < 0x19C)
			_dialog1.write(_action._activeAction._verbId, false);

		switch (_action._activeAction._verbId) {
		case 0x191:
			handleInternDialog(0x19E, 2, INDEFINITE_TIMEOUT);
			_dialog1.write(0x192, true);
			break;

		case 0x192:
			handleInternDialog(0x1A0, 5, INDEFINITE_TIMEOUT);
			_dialog1.write(0x193, true);
			break;

		case 0x193:
			handleInternDialog(0x1A5, 4, INDEFINITE_TIMEOUT);
			_dialog1.write(0x194, true);
			break;

		case 0x194:
			handleInternDialog(0x1A9, 6, INDEFINITE_TIMEOUT);
			_dialog1.write(0x195, true);
			_dialog1.write(0x196, true);
			_dialog1.write(0x19D, false);
			break;

		case 0x195:
			handleInternDialog(0x1AF, 7, INDEFINITE_TIMEOUT);
			if (!_dialog1.read(0x196))
				_dialog1.write(0x197, true);
			break;

		case 0x196:
			handleInternDialog(0x1B6, 5, INDEFINITE_TIMEOUT);
			if (!_dialog1.read(0x195))
				_dialog1.write(0x197, true);
			break;

		case 0x197:
			handleInternDialog(0x1BB, 5, INDEFINITE_TIMEOUT);
			break;

		case 0x198:
			handleInternDialog(0x1C0, 5, INDEFINITE_TIMEOUT);
			_dialog1.write(0x19A, true);
			break;

		case 0x199:
			handleInternDialog(0x1C5, 3, INDEFINITE_TIMEOUT);
			break;

		case 0x19A:
			handleInternDialog(0x1C8, 5, INDEFINITE_TIMEOUT);
			_dialog1.write(0x19B, true);
			break;

		case 0x19B:
			handleInternDialog(0x1CD, 3, INDEFINITE_TIMEOUT);
			break;

		case 0x19C:
		case 0x19D:
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(142, 121));
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 6, 8);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);

			_dialogFl = false;
			handleInternDialog(0x1D0, 1, 120);
			if (_dialog1.read(0) || (_action._activeAction._verbId == 0x19D)) {
				_explosionFl = true;
				_internCounter = 3420;
			}
			break;

		default:
			break;
		}

		if (_action._activeAction._verbId < 0x19C) {
			_dialog1.start();
			_game._player._stepEnabled = true;
		}

	}
}

void Scene318::handleRexDialogs(int quote) {
	_scene->_kernelMessages.reset();

	Common::String curQuote = _game.getQuote(quote);
	if (_vm->_font->getWidth(curQuote, _scene->_textSpacing) > 200) {
		Common::String subQuote1;
		_game.splitQuote(curQuote, subQuote1, _subQuote2);
		_scene->_kernelMessages.add(Common::Point(138, 59), 0x1110, 32, 0, 240, subQuote1);
		_scene->_kernelMessages.add(Common::Point(138, 73), 0x1110, 32, 1, 180, _subQuote2);
	} else
		_scene->_kernelMessages.add(Common::Point(138, 73), 0x1110, 32, 1, 120, curQuote);
}

void Scene318::handleInternDialog(int quoteId, int quoteNum, uint32 timeout) {
	int height = quoteNum * 14;
	int posY;
	if (height < 85)
		posY = 87 - height;
	else
		posY = 2;

	int curQuoteId= quoteId;

	int maxWidth = 0;
	for (int i = 0; i < quoteNum; i++) {
		maxWidth = MAX(maxWidth, _vm->_font->getWidth(_game.getQuote(curQuoteId), -1));
		curQuoteId++;
	}

	int posX = MIN(319 - maxWidth, 178 - (maxWidth >> 1));
	curQuoteId = quoteId;

	_scene->_kernelMessages.reset();
	_internTalkingFl = true;

	// WORKAROUND: In case the player launches multiple talk selections with the
	// intern before previous ones have finished, take care of removing any
	int seqIndex;
	while ((seqIndex = _scene->_sequences.findByTrigger(63)) != -1)
		_scene->_sequences.remove(seqIndex);

	for (int i = 0; i < quoteNum; i++) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(180, 63);
		_scene->_kernelMessages.add(Common::Point(posX, posY), 0xFDFC, 0, 0, timeout, _game.getQuote(curQuoteId));
		posY += 14;
		curQuoteId++;
	}
}

void Scene318::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('k', -1));

	if (_globals[kAfterHavoc]) {
		_scene->loadAnimation(formAnimName('f', -1));
		_scene->_animation[0]->_resetFlag = true;
	} else if (!_globals[kHasSeenProfPyro]) {
		_scene->_hotspots.activate(NOUN_PROFESSORS_GURNEY, false);
		_scene->_hotspots.activate(NOUN_PROFESSOR, false);
		_scene->_hotspots.activate(NOUN_TAPE_PLAYER, false);
	}

	if (_game._objects.isInRoom(OBJ_SCALPEL)) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 0, 120);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
		_scene->_dynamicHotspots.add(NOUN_SCALPEL, VERB_TAKE, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
	}

	if (_scene->_priorSceneId == 357)
		_game._player._playerPos = Common::Point(15, 110);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(214, 152);

	_dialog1.setup(0x47, 0x191, 0x192, 0x193, 0x194, 0x195, 0x196, 0x197, 0x198, 0x199, 0x19A, 0x19B, 0x19C, 0x19D, 0);

	if (!_game._visitedScenes._sceneRevisited) {
		_dialog1.set(0x191, 0x198, 0x199, 0x19C, 0);
		if (_game._widepipeCtr >= 2)
			_dialog1.write(0x19D, true);
	}

	if (_scene->_priorSceneId == 307) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->loadAnimation(formAnimName('a', -1), 60);
		_animMode = 1;
	}

	_lastFrame = 0;
	_scene->_hotspots.activate(NOUN_INTERN, false);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_dialogFl = false;
		_internWalkingFl = false;
		_counter= 0;
		_internCounter= 0;
		_internVisibleFl = true;
		_explosionFl = false;
	}

	_game.loadQuoteSet(0x18C, 0x18D, 0x18E, 0x18F, 0x191, 0x192, 0x193, 0x194, 0x195, 0x196,
		0x197, 0x198, 0x199, 0x19A, 0x19B, 0x19C, 0x19E, 0x19E, 0x1A0, 0x1A1, 0x1A2, 0x1A3,
		0x1A4, 0x1A5, 0x1A6, 0x1A7, 0x1A8, 0x1A9, 0x1AA, 0x1AB, 0x1AC, 0x1AD, 0x1AE, 0x1AF,
		0x1B0, 0x1B1, 0x1B2, 0x1B3, 0x1B4, 0x1B5, 0x1B6, 0x1B7, 0x1B8, 0x1B9, 0x1BA, 0x1BB,
		0x1BC, 0x1BD, 0x1BE, 0x1BF, 0x1C0, 0x1C1, 0x1C2, 0x1C3, 0x1C4, 0x1C5, 0x1C6, 0x1C7,
		0x1C8, 0x1C9, 0x1CA, 0x1CB, 0x1CC, 0x1CD, 0x1CE, 0x1CF, 0x1D0, 0x1D1, 0x1D2, 0x1D3,
		0x190, 0x19D, 0);

	if ((_scene->_priorSceneId == RETURNING_FROM_DIALOG) || (((_scene->_priorSceneId == 318) ||
			(_scene->_priorSceneId == RETURNING_FROM_LOADING)) && (!_globals[kAfterHavoc]))) {
		if (!_globals[kAfterHavoc]) {
			_game._player._visible = false;
			_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('g', -1));
			_animMode = 2;

			if (_game._visitedScenes.exists(319) || !_internVisibleFl) {
				_internVisibleFl = false;
				_dialogFl = false;
			} else {
				_scene->loadAnimation(formAnimName('b', -1), 61);
				_scene->_hotspots.activate(NOUN_INTERN, true);
			}

			if (_dialogFl) {
				_dialog1.start();
				_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 8);
			} else
				_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);

			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(142, 121));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
		}
	}

	if (_scene->_priorSceneId == 319) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_animMode = 4;
		if (!_globals[kHasSeenProfPyro]) {
			_scene->loadAnimation(formAnimName('d', -1), 64);
			_globals[kHasSeenProfPyro] = true;
		} else {
			_scene->loadAnimation(formAnimName('e', -1), 64);
		}
	}

	_internTalkingFl = false;
	_vm->_palette->setEntry(252, 63, 63, 10);
	_vm->_palette->setEntry(253, 45, 45, 05);

	_dropTimer = _vm->_game->_scene._frameStartTime;
	sceneEntrySound();

	if (_dialogFl)
		_vm->_sound->command(15);
}

void Scene318::step() {
	if ((_scene->_animation[0] != nullptr) && (_animMode == 2)) {
		if (_lastFrame != _scene->_animation[0]->getCurrentFrame()) {
			_lastFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			switch (_lastFrame) {
			case 20:
			case 30:
			case 40:
			case 50:
			case 60:
			case 70:
			case 80:
			case 90:
			case 100:
			case 110:
			case 120:
			case 130:
			case 140:
			case 149:
				if (_internWalkingFl) {
					nextFrame = 149;
				} else if (_internTalkingFl) {
					nextFrame = 149;
				} else if (_lastFrame == 149) {
					nextFrame = 4;
				}
				break;

			case 151:
				if (_internWalkingFl)
					nextFrame = 183;
				break;

			case 167:
			case 184:
				if (_internWalkingFl) {
					nextFrame = 184;
				} else if (!_internTalkingFl) {
					nextFrame = 0;
				} else if (_vm->getRandomNumber(1, 100) <= 50) {
					nextFrame = 151;
				} else {
					nextFrame = 167;
				}

				if (nextFrame == 184) {
					handleInternDialog(0x1D1, 3, 240);
					_scene->_hotspots.activate(NOUN_INTERN, false);
					_internVisibleFl = false;
				}
				break;

			default:
				break;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				_lastFrame = nextFrame;
			}
		}
	}

	switch (_game._trigger) {
	case 60:
		_vm->_sound->command(3);
		_animMode = 2;
		_scene->_reloadSceneFlag = true;
		break;

	case 61:
		_counter = 0;
		break;

	case 62:
		_scene->_nextSceneId = 319;
		break;

	case 63:
		_internTalkingFl = false;
		break;

	case 64:
		_vm->_sound->command(3);
		_scene->_nextSceneId = 307;
		break;

	default:
		break;
	}

	uint32 tmpFrame = _vm->_events->getFrameCounter();
	long diffFrame = tmpFrame - _lastFrameCounter;
	_lastFrameCounter = tmpFrame;

	if ((_animMode == 2) && !_internVisibleFl && _game._player._stepEnabled) {
		if ((diffFrame >= 0) && (diffFrame <= 4))
			_counter += diffFrame;
		else
			_counter++;

		int extraCounter = _game._objects.isInInventory(OBJ_SCALPEL) ? 900 : 0;

		if (_counter + extraCounter >= 1800) {
			_scene->freeAnimation();
			_game._player._stepEnabled = false;
			_scene->loadAnimation(formAnimName('c', -1), 62);
			_animMode = 3;
		}
	} else if ((_animMode == 2) && _explosionFl && _internVisibleFl && !_dialogFl
			&& !_internWalkingFl && (_game._screenObjects._inputMode != kInputConversation)) {
		if ((diffFrame >= 0) && (diffFrame <= 4))
			_internCounter += diffFrame;
		else
			_internCounter++;

		if (_internCounter >= 3600) {
			_vm->_sound->command(59);
			_vm->_screen->_shakeCountdown = 20;
			_internWalkingFl = true;
		}
	}

	if ((_vm->_game->_scene._frameStartTime - _dropTimer) > 600) {
		_vm->_sound->command(51);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 14, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
		_dropTimer = _vm->_game->_scene._frameStartTime;
	}
}

void Scene318::preActions() {
	if (_game._player._needToWalk)
		_game._player._needToWalk = _game._player._visible;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_WEST))
		_game._player._walkOffScreenSceneId = 357;
}

void Scene318::actions() {
	if (_game._screenObjects._inputMode == kInputConversation) {
		handleDialog();
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TALKTO, NOUN_INTERN)) {
		switch (_game._trigger) {
		case 0: {
			_dialogFl = true;
			_vm->_sound->command(15);
			_game._player._stepEnabled = false;
			handleRexDialogs(_vm->getRandomNumber(0x18C, 0x18E));

			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 80);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(142, 121));
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 6, 8);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

		case 1:
			_game._player._stepEnabled = true;
			handleInternDialog(0x18F, 1, INDEFINITE_TIMEOUT);
			_dialog1.start();
			break;

		case 2: {
			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 8);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(142, 121));
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
			}
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_SCALPEL) && (_game._objects.isInRoom(OBJ_SCALPEL) || _game._trigger)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 8, 2, 0, 80);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(142, 121));
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 2, 5);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 5, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			if (_internVisibleFl)
				handleInternDialog(0x190, 1, 120);
			else {
				_game._objects.addToInventory(OBJ_SCALPEL);
				_vm->_dialogs->showItem(OBJ_SCALPEL, 0x7C5D);
				_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			}
			break;

		case 2: {
			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(142, 121));
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
			_scene->_sequences.addTimer(60, 3);
			}
			break;

		case 3:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_game._player._visible) {
		if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_SOUTH)) {
			_scene->_nextSceneId = 407;
			_action._inProgress = false;
			return;
		}

		if (_action.isAction(VERB_TAKE, NOUN_TAPE_PLAYER)) {
			if (_game._objects.isInRoom(OBJ_AUDIO_TAPE)) {
				_vm->_dialogs->showItem(OBJ_AUDIO_TAPE, 0x7C5B);
				_game._objects.addToInventory(OBJ_AUDIO_TAPE);
			} else
				_vm->_dialogs->show(31834);

			_action._inProgress = false;
			return;
		}

		if (_action.isAction(VERB_LOOK, NOUN_TAPE_PLAYER)) {
			if (_game._objects.isInRoom(OBJ_AUDIO_TAPE))
				_vm->_dialogs->show(31833);
			else
				_vm->_dialogs->show(31834);

			_action._inProgress = false;
			return;
		}

		if (_action.isAction(VERB_WALK_INTO, NOUN_DOCTORS_OFFICE)) {
			_vm->_dialogs->show(31831);
			_action._inProgress = false;
			return;
		}

		if (_action.isAction(VERB_LOOK, NOUN_GURNEY)) {
			_vm->_dialogs->show(31823);
			_action._inProgress = false;
			return;
		}

		if (_action.isAction(VERB_LOOK, NOUN_INSTRUMENT_TABLE)) {
			_vm->_dialogs->show(31825);
			_action._inProgress = false;
			return;
		}
	} else { // Not visible
		if (_action.isAction(VERB_LOOK, NOUN_GURNEY)) {
			_vm->_dialogs->show(31822);
			_action._inProgress = false;
			return;
		}

		if (_action.isAction(VERB_LOOK, NOUN_INSTRUMENT_TABLE)) {
			_vm->_dialogs->show(31824);
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(31810);
	else if (_action.isAction(VERB_LOOK, NOUN_FLOOR))
		_vm->_dialogs->show(31811);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_WEST))
		_vm->_dialogs->show(31812);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_SOUTH))
		_vm->_dialogs->show(31813);
	else if (_action.isAction(VERB_LOOK, NOUN_FAUCET))
		_vm->_dialogs->show(31814);
	else if (_action.isAction(VERB_LOOK, NOUN_SINK))
		_vm->_dialogs->show(31815);
	else if (_action.isAction(VERB_LOOK, NOUN_CONVEYOR_BELT))
		_vm->_dialogs->show(31816);
	else if (_action.isAction(VERB_LOOK, NOUN_LARGE_BLADE))
		_vm->_dialogs->show(31817);
	else if (_action.isAction(VERB_LOOK, NOUN_MONITOR))
		_vm->_dialogs->show(31818);
	else if (_action.isAction(VERB_LOOK, NOUN_CABINETS))
		_vm->_dialogs->show(31819);
	else if (_action.isAction(VERB_LOOK, NOUN_EQUIPMENT))
		_vm->_dialogs->show(31820);
	else if (_action.isAction(VERB_LOOK, NOUN_SHELF))
		_vm->_dialogs->show(31821);
	else if (_action.isAction(VERB_OPEN, NOUN_CABINETS))
		_vm->_dialogs->show(31829);
	else if (_action.isAction(VERB_LOOK, NOUN_INTERN))
		_vm->_dialogs->show(31830);
	else if (_action.isAction(VERB_LOOK, NOUN_PROFESSOR))
		_vm->_dialogs->show(31832);
	else if (_action.isAction(VERB_LOOK, NOUN_PROFESSORS_GURNEY))
		_vm->_dialogs->show(31836);
	else if (_action._lookFlag) {
		if (_game._player._visible || _game._objects.isInInventory(OBJ_SCALPEL))
			_vm->_dialogs->show(31828);
		else if (_internVisibleFl)
			_vm->_dialogs->show(31826);
		else
			_vm->_dialogs->show(31827);
	} else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene319::Scene319(MADSEngine *vm) : Scene3xx(vm) {
	_animMode = -1;
	_animFrame = -1;
	_nextAction1 = -1;
	_nextAction2 = -1;
	_slacheMode = -1;
	_slacheTopic = -1;
	_slachePosY = -1;

	_slacheTalkingFl = false;
	_slacheReady = false;
	_slacheInitFl = false;

	_subQuote2 = "";
}

void Scene319::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	s.syncAsUint32LE(_animMode);
	s.syncAsUint32LE(_animFrame);
	s.syncAsUint32LE(_nextAction1);
	s.syncAsUint32LE(_nextAction2);
	s.syncAsUint32LE(_slacheMode);
	s.syncAsUint32LE(_slacheTopic);
	s.syncAsUint32LE(_slachePosY);

	s.syncAsByte(_slacheTalkingFl);
	s.syncAsByte(_slacheReady);
	s.syncAsByte(_slacheInitFl);

	s.syncString(_subQuote2);
}

void Scene319::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene319::handleRexDialogues(int quote) {
	_scene->_kernelMessages.reset();

	Common::String curQuote = _game.getQuote(quote);
	if (_vm->_font->getWidth(curQuote, _scene->_textSpacing) > 200) {
		Common::String subQuote1;
		_game.splitQuote(curQuote, subQuote1, _subQuote2);
		_scene->_kernelMessages.add(Common::Point(160, 106), 0x1110, 32, 0, 120, subQuote1);
		_scene->_kernelMessages.add(Common::Point(160, 120), 0x1110, 32, 1, 120, _subQuote2);
	} else
		_scene->_kernelMessages.add(Common::Point(160, 120), 0x1110, 32, 1, 120, curQuote);
}

void Scene319::handleSlacheDialogs(int quoteId, int counter, uint32 timer) {
	int curQuote = quoteId;
	int posY = 5 + (_slachePosY * 14);

	for (int count = 0; count < counter; count++, curQuote++) {
		_scene->_kernelMessages.add(Common::Point(8, posY), 0xFDFC, 0, 0, timer, _game.getQuote(curQuote));
		posY += 14;
	}
}

void Scene319::enter() {
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('e', 0));
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 3));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('k', -1));

	if (!_game._objects.isInInventory(OBJ_SCALPEL)) {
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
	}

	_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 18, 0, 0, 300);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 67, 0, 0, 377);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 173, 0, 0, 233);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

	_globals._sequenceIndexes[0] = _scene->_sequences.startCycle(_globals._spriteIndexes[0], false, 1);
	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);

	_dialog1.setup(0x43, 0x165, 0x166, 0x167, 0x168, 0x169, 0x16A, 0);
	_dialog2.setup(0x44, 0x171, 0x172, 0x173, 0x174, 0x175, 0x176, 0);
	_dialog3.setup(0x45, 0x17D, 0x17E, 0x17F, 0x180, 0x181, 0x182, 0x183, 0);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_dialog1.set(0x165, 0x166, 0x167, 0x168, 0);
		_dialog2.set(0x171, 0x172, 0x173, 0x174, 0);
		_dialog3.set(0x17D, 0x17E, 0x17F, 0x180, 0);
	}

	_game.loadQuoteSet(0x15F, 0x160, 0x161, 0x162, 0x163, 0x164, 0x16B, 0x16C, 0x16D,
		0x16E, 0x16F, 0x170, 0x177, 0x178, 0x178, 0x17A, 0x17B, 0x17C, 0x165, 0x166,
		0x167, 0x168, 0x169, 0x16A, 0x171, 0x172, 0x173, 0x174, 0x175, 0x176, 0x17D,
		0x17E, 0x17F, 0x180, 0x181, 0x182, 0x183, 0x184, 0x185, 0x186, 0x187, 0x188,
		0x189, 0x18A, 0x18B, 0);

	_vm->_palette->setEntry(252, 63, 30, 2);
	_vm->_palette->setEntry(253, 45, 15, 1);

	_slachePosY = 0;
	_slacheInitFl = false;
	_slacheTalkingFl = false;
	_slacheReady = false;
	_animFrame = 0;

	_scene->loadAnimation(formAnimName('b', 0));

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_animMode = 1;
		_nextAction1 = 2;
		_nextAction2 = 2;
		_slacheMode = 1;
		_slacheTopic = 1;
		_slacheInitFl = true;

		if (_globals[kRexHasMetSlache]) {
			handleSlacheDialogs(VERB_WALK_OUTSIDE, 2, INDEFINITE_TIMEOUT);
			_slachePosY = 3;
		} else {
			handleSlacheDialogs(0x186, 4, INDEFINITE_TIMEOUT);
			_slachePosY = 5;
		}
	}

	switch (_slacheTopic) {
	case 1:
		handleSlacheDialogs(0x15F, 2, INDEFINITE_TIMEOUT);
		_dialog1.start();
		break;

	case 2:
		handleSlacheDialogs(0x16B, 2, INDEFINITE_TIMEOUT);
		_dialog2.start();
		break;

	case 3:
		handleSlacheDialogs(0x177, 2, INDEFINITE_TIMEOUT);
		_dialog3.start();
		break;

	default:
		break;
	}

	_slachePosY = 0;
	sceneEntrySound();
}

void Scene319::step() {
	if (_scene->_animation[0] == nullptr)
		return;

	if (_animFrame != _scene->_animation[0]->getCurrentFrame()) {
		_animFrame = _scene->_animation[0]->getCurrentFrame();
		int nextFrame = -1;
		if (_animMode == 1) {
			switch (_animFrame) {
			case 6:
				_slacheTalkingFl = true;
				break;

			case 29:
				_slacheReady = true;
				break;

			case 39:
				if (_slacheInitFl) {
					_slacheInitFl = false;
					if (_nextAction1 == 2)
						nextFrame = 0;
				} else if (_nextAction1 == 2)
					_nextAction1 = 1;
				break;

			case 50:
			case 60:
			case 70:
			case 85:
				if (_nextAction1 == 2)
					nextFrame = 0;
				else if (_nextAction1 == 3) {
					nextFrame = 85;
					_slacheTalkingFl = true;
				} else if (_animFrame == 85) {
					if (!_game._player._stepEnabled)
						_slacheTalkingFl = true;
					nextFrame = 40;
				}
				break;

			case 115:
				_slacheReady = true;
				break;

			case 129:
				if (_nextAction1 == 3) {
					nextFrame = 115;
					if (!_game._player._stepEnabled)
						_slacheTalkingFl = true;
				}
				break;

			case 145:
				nextFrame = 40;
				break;

			default:
				break;
			}

			if ((_animFrame > 40) && (_animFrame < 85) && (nextFrame < 0)) {
				switch (_nextAction1) {
				case 4:
					_animFrame = 0;
					_scene->freeAnimation();
					_animMode = 2;
					_scene->loadAnimation(formAnimName('b', 3), 70);
					break;

				case 5:
					_animFrame = 0;
					_scene->freeAnimation();
					_animMode = 3;
					_scene->loadAnimation(formAnimName('b', 4), 71);
					break;

				case 6:
					_animFrame = 0;
					_scene->freeAnimation();
					_animMode = 4;
					_scene->loadAnimation(formAnimName('b', 5), 72);
					break;

				default:
					break;
				}

				if (!_animFrame) {
					_scene->_sequences.remove(_globals._sequenceIndexes[0]);
					_scene->_sequences.remove(_globals._sequenceIndexes[1]);

					for (int i = 0; i <= 1; i++) {
						_globals._sequenceIndexes[i] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[i], false, 8, 1, 0, 0);
						_scene->_sequences.setAnimRange(_globals._sequenceIndexes[i], 1, 7);
					}
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 73);
				}
			}
		}

		if (_animMode == 2) {
			if (_animFrame == 13)
				_vm->_screen->_shakeCountdown = 40;

			if (_animFrame == 16)
				_vm->_screen->_shakeCountdown = 1;
		}

		if (_animMode == 3) {
			if (_animFrame == 11)
				_vm->_screen->_shakeCountdown = 60;

			if (_animFrame == 18)
				_vm->_screen->_shakeCountdown = 1;
		}

		if ((_animMode == 4) && (_animFrame == 16))
			_vm->_screen->_shakeCountdown = 80;

		if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
			_scene->_animation[0]->setCurrentFrame(nextFrame);
			_animFrame = nextFrame;
		}
	}

	switch (_game._trigger) {
	case 70:
	case 71: {
		_animMode = 1;
		_nextAction1 = _nextAction2;
		_animFrame = 0;
		_scene->freeAnimation();
		_scene->loadAnimation(formAnimName('b', 0));
		if (_nextAction1 == 3)
			_scene->_animation[0]->setCurrentFrame(85);
		else if (_nextAction1 == 1)
			_scene->_animation[0]->setCurrentFrame(40);

		_animFrame = _scene->_animation[0]->getCurrentFrame();
		_slacheTalkingFl = true;
		_vm->_screen->_shakeCountdown = 1;

		for (int i = 0; i <= 1; i++) {
			int oldIdx = _globals._sequenceIndexes[i];
			_scene->_sequences.remove(_globals._sequenceIndexes[i]);
			_globals._sequenceIndexes[i] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[i], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[i], 8, 13);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[i], oldIdx);
		}
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 74);

		// WORKAROUND: This fixes the game sometimes going into an endless waiting
		// loop even after the doctor has finished hitting Rex. Note sure if it's due
		// to a bug in room script or in the engine, but this at least fixes it
		int seqIndex = _scene->_sequences.findByTrigger(2);
		_scene->_sequences[seqIndex]._doneFlag = false;
		break;
	}

	case 72:
		_vm->_palette->setColorFlags(0xFF, 0, 0);
		_vm->_palette->setColorValues(0, 0, 0);
		_vm->_palette->fadeOut(_vm->_palette->_mainPalette, nullptr, 18, 228,
			248, 0, 1, 16);
		_vm->_screen->_shakeCountdown = 1;
		_scene->_reloadSceneFlag = true;
		break;

	case 73:
		for (int i = 0; i <= 1; i++) {
			int oldIdx = _globals._sequenceIndexes[i];
			_scene->_sequences.remove(_globals._sequenceIndexes[i]);
			_globals._sequenceIndexes[i] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[i], false, 8, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[i], 6, 7);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[i], oldIdx);
		}
		break;

	case 74:
		for (int i = 0; i <= 1; i++) {
			int oldIdx = _globals._sequenceIndexes[i];
			_scene->_sequences.remove(_globals._sequenceIndexes[i]);
			_globals._sequenceIndexes[i] = _scene->_sequences.startCycle(_globals._spriteIndexes[i], false, 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[i], oldIdx);
		}
		break;

	default:
		break;
	}
}

void Scene319::actions() {
	if (_game._trigger == 0) {
		_game._player._stepEnabled = false;
		handleRexDialogues(_action._activeAction._verbId);
	} else {
		if ((_action._activeAction._verbId == 0x165) || (_action._activeAction._verbId == 0x166)) {
			if (_game._trigger == 1) {
				_nextAction1 = 3;
				_slacheTalkingFl = false;
				_slacheMode = 1;
				_slacheTopic = 2;
			}

			if (!_slacheTalkingFl) {
				_scene->_sequences.addTimer(4, 2);
			} else {
				handleSlacheDialogs(0x16B, 2, INDEFINITE_TIMEOUT);
				_dialog2.start();
				_game._player._stepEnabled = true;
			}
		}

		if ((_action._activeAction._verbId == 0x171) || (_action._activeAction._verbId == 0x172)) {
			if (_game._trigger == 1) {
				_nextAction1 = 2;
				_slacheTalkingFl = false;
				_slacheMode = 1;
				_slacheTopic = 3;
			}

			if (!_slacheTalkingFl) {
				_scene->_sequences.addTimer(4, 2);
			} else {
				handleSlacheDialogs(0x177, 2, INDEFINITE_TIMEOUT);
				_dialog3.start();
				_game._player._stepEnabled = true;
			}
		}

		if ((_action._activeAction._verbId == 0x17D) || (_action._activeAction._verbId == 0x17E)) {
			if (_game._trigger == 1) {
				_nextAction1 = 3;
				_slacheTalkingFl = false;
				_slacheReady = false;
				_slacheMode = 1;
				_slacheTopic = 1;
			}

			if (!_slacheTalkingFl) {
				_scene->_sequences.addTimer(4, 2);
			} else {
				if (_game._trigger == 2)
					handleSlacheDialogs(0x184, 2, 180);

				if (!_slacheReady) {
					_scene->_sequences.addTimer(120, 3);
				} else {
					_globals[kRexHasMetSlache] = true;
					_scene->_nextSceneId = 318;
				}
			}
		}

		if ((_action._activeAction._verbId == 0x168) || (_action._activeAction._verbId == 0x174) ||
			(_action._activeAction._verbId == 0x180) || (_action._activeAction._verbId == 0x169) ||
			(_action._activeAction._verbId == 0x175) || (_action._activeAction._verbId == 0x181) ||
			(_action._activeAction._verbId == 0x16A) || (_action._activeAction._verbId == 0x176) ||
			(_action._activeAction._verbId == 0x182) || (_action._activeAction._verbId == 0x183) ||
			(_action._activeAction._verbId == 0x167) || (_action._activeAction._verbId == 0x173) ||
			(_action._activeAction._verbId == 0x17F)) {

			bool addDialogLine = !((_action._activeAction._verbId == 0x167) || (_action._activeAction._verbId == 0x173) ||
					(_action._activeAction._verbId == 0x17F) || (_action._activeAction._verbId == 0x16A) ||
					(_action._activeAction._verbId == 0x176) || (_action._activeAction._verbId == 0x182) ||
					(_action._activeAction._verbId == 0x183));

			int addVerbId = _action._activeAction._verbId + 1;
			if ((addVerbId == 0x182) && (_game._storyMode != STORYMODE_NAUGHTY))
				addVerbId = 0x183;

			if (_slacheMode == 1) {
				if (_game._trigger == 1) {
					_nextAction2 = _nextAction1;
					_nextAction1 = 4;
				}

				if (_nextAction1 != _nextAction2) {
					_scene->_sequences.addTimer(4, 2);
				} else {
					Conversation *curDialog;
					int nextDocQuote;
					if ((_action._activeAction._verbId == 0x168) || (_action._activeAction._verbId == 0x167)) {
						curDialog = &_dialog1;
						nextDocQuote = 0x161;
					} else if ((_action._activeAction._verbId == 0x174) || (_action._activeAction._verbId == 0x1753)) {
						nextDocQuote = 0x16D;
						curDialog = &_dialog2;
					} else {
						nextDocQuote = 0x179;
						curDialog = &_dialog3;
					}

					handleSlacheDialogs(nextDocQuote, 2, INDEFINITE_TIMEOUT);
					if (addDialogLine) {
						curDialog->write(_action._activeAction._verbId, false);
						curDialog->write(addVerbId, true);
					}

					curDialog->start();
					_game._player._stepEnabled = true;
					_slacheMode = 2;
				}
			} else if (_slacheMode == 2) {
				if (_game._trigger == 1) {
					_nextAction2 = _nextAction1;
					_nextAction1 = 5;
				}

				if (_nextAction1 != _nextAction2) {
					_scene->_sequences.addTimer(4, 2);
				} else {
					Conversation *curDialog;
					int nextDocQuote;
					if ((_action._activeAction._verbId == 0x168) || (_action._activeAction._verbId == 0x169) || (_action._activeAction._verbId == 0x167)) {
						curDialog = &_dialog1;
						nextDocQuote = 0x163;
					} else if ((_action._activeAction._verbId == 0x174) || (_action._activeAction._verbId == 0x175) || (_action._activeAction._verbId == 0x173)) {
						nextDocQuote = 0x16F;
						curDialog = &_dialog2;
					} else {
						nextDocQuote = 0x17B;
						curDialog = &_dialog3;
					}

					handleSlacheDialogs(nextDocQuote, 2, INDEFINITE_TIMEOUT);
					if (addDialogLine) {
						curDialog->write(_action._activeAction._verbId, false);
						curDialog->write(addVerbId, true);
					}

					curDialog->start();
					_game._player._stepEnabled = true;
					_slacheMode = 3;
				}
			} else {
				_nextAction2 = _nextAction1;
				_nextAction1 = 6;
			}
		}
	}

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene320::Scene320(MADSEngine *vm) : Scene300s(vm) {
	_blinkFl = false;
	_flippedFl = false;

	_buttonId = -1;
	_lastFrame = -1;
	_leftItemId = -1;
	_posX = -1;
	_rightItemId = -1;
}

void Scene320::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	s.syncAsByte(_blinkFl);
	s.syncAsByte(_flippedFl);

	s.syncAsSint32LE(_buttonId);
	s.syncAsSint32LE(_lastFrame);
	s.syncAsSint32LE(_leftItemId);
	s.syncAsSint32LE(_posX);
	s.syncAsSint32LE(_rightItemId);
}

void Scene320::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene320::setRightView(int view) {
	if (_rightItemId < 8) _scene->_sequences.remove(_globals._sequenceIndexes[10]);

	int spriteNum;
	switch (view) {
	case 0:
		spriteNum = 16;
		break;

	case 1:
		spriteNum = 14;
		break;

	case 2:
		spriteNum = 17;
		break;

	case 3:
		spriteNum = 15;
		break;

	default:
		spriteNum = view + 6;
		break;
	}

	if (view != 8) {
		_globals._sequenceIndexes[10] = _scene->_sequences.startCycle(_globals._spriteIndexes[spriteNum], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 0);
	}

	_globals[kRightView320] = _rightItemId = view;
}

void Scene320::setLeftView(int view) {
	if (_leftItemId < 10)
		_scene->_sequences.remove(_globals._sequenceIndexes[0]);

	if (view != 10) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[view], false, 6, 0, 0, 18);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 0);
		if (!_blinkFl)
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], 2, 2);
	}

	_leftItemId = view;
}

void Scene320::handleButtons() {
	switch(_action._activeAction._objectNameId) {
	case 0x2DD:
		_buttonId = 5;
		break;

	case 0x2DE:
		_buttonId = 4;
		break;

	case 0x2E0:
		_buttonId = 6;
		break;

	case 0x2E1:
		_buttonId = 7;
		break;

	case 0x2E2:
		_buttonId = 8;
		break;

	case 0x2E3:
		_buttonId = 9;
		break;

	case 0x2E4:
		_buttonId = 10;
		break;

	case 0x2E5:
		_buttonId = 11;
		break;

	case 0x2E6:
		_buttonId = 12;
		break;

	case 0x2E7:
		_buttonId = 13;
		break;

	case 0x2E8:
		_buttonId = 0;
		break;

	case 0x2E9:
		_buttonId = 1;
		break;

	case 0x2EA:
		_buttonId = 2;
		break;

	case 0x2EB:
		_buttonId = 3;
		break;

	default:
		break;
	}

	if (_buttonId <= 3) {
		_posX = (8 * _buttonId) - 2;
		_flippedFl = true;
	} else if (_buttonId <= 5) {
		_posX = (13 * _buttonId) - 14;
		_flippedFl = true;
	} else {
		_posX = (8 * _buttonId) + 98;
		_flippedFl = false;
	}
}

void Scene320::enter() {
	_blinkFl = true;
	_rightItemId = 8;
	_leftItemId = 10;
	_lastFrame = 0;

	for (int i = 0; i < 10; i++)
		_globals._spriteIndexes[i] = _scene->_sprites.addSprites(formAnimName('M', i));

	for (int i = 0; i < 8; i++)
		_globals._spriteIndexes[10 + i] = _scene->_sprites.addSprites(formAnimName('N', i));

	_globals._spriteIndexes[18] = _scene->_sprites.addSprites("*REXHAND");
	_game._player._visible = false;

	setRightView(_globals[kRightView320]);
	setLeftView(0);

	_vm->_palette->setEntry(252, 63, 30, 20);
	_vm->_palette->setEntry(253, 45, 15, 10);

	sceneEntrySound();
}

void Scene320::step() {
	if (_scene->_animation[0] != nullptr) {
		if (_lastFrame != _scene->_animation[0]->getCurrentFrame()) {
			_lastFrame = _scene->_animation[0]->getCurrentFrame();
			switch (_lastFrame) {
			case 95:
				_blinkFl = true;
				setLeftView(9);
				_vm->_sound->command(41);
				break;

			case 139:
				_blinkFl = false;
				setLeftView(9);
				break;

			case 191:
				_scene->_kernelMessages.add(Common::Point(1, 1), 0xFDFC, 0, 0, 60, _game.getQuote(0xFE));
				break;

			case 417:
			case 457:
				_vm->_screen->_shakeCountdown = 40;
				_vm->_sound->command(59);
				break;

			case 430:
				_blinkFl = true;
				setLeftView(4);
				break;

			default:
				break;
			}
		}
	}

	if (_game._trigger == 70) {
		_globals[kAfterHavoc] = true;
		_globals[kTeleporterRoom + 1] = 351;
		_scene->_nextSceneId = 361;
	}
}

void Scene320::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(32011);
	else if ((_action.isAction(VERB_PRESS) || _action.isAction(VERB_PUSH)) &&
		(_action.isObject(NOUN_LEFT_1_KEY) || _action.isObject(NOUN_LEFT_2_KEY) || _action.isObject(NOUN_LEFT_3_KEY) || _action.isObject(NOUN_LEFT_4_KEY) ||
		 _action.isObject(NOUN_GREEN_BUTTON) || _action.isObject(NOUN_RED_BUTTON) || _action.isObject(NOUN_RIGHT_1_KEY) || _action.isObject(NOUN_RIGHT_2_KEY) ||
		 _action.isObject(NOUN_RIGHT_3_KEY) || _action.isObject(NOUN_RIGHT_4_KEY) || _action.isObject(NOUN_RIGHT_5_KEY) || _action.isObject(NOUN_RIGHT_6_KEY) ||
		 _action.isObject(NOUN_RIGHT_7_KEY) || _action.isObject(NOUN_RIGHT_8_KEY)
		)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			handleButtons();
			_globals._sequenceIndexes[18] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[18], _flippedFl, 4, 2, 0, 0);
			_scene->_sequences.setScale(_globals._sequenceIndexes[18], 60);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[18], Common::Point(_posX, 170));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[18], 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[18], SEQUENCE_TRIGGER_LOOP, 0, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[18], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			if (_buttonId >= 6) {
				_vm->_sound->command(60);
				setRightView(_buttonId - 6);
			}
			if (_buttonId == 4) {
				_vm->_sound->command(38);
				if (_leftItemId == 3)
					setLeftView(0);
				else
					setLeftView(3);
			}
			if (_buttonId == 5) {
				_vm->_sound->command(38);
				if (_leftItemId == 1)
					setLeftView(2);
				else
					setLeftView(1);
			}
			if (_buttonId <= 3) {
				_vm->_sound->command(60);
				setLeftView(_buttonId + 5);
			}
			break;

		case 2:
			_game._player._stepEnabled = true;
			if (_buttonId == 5) {
				if (_leftItemId == 2) {
					_game._player._stepEnabled = false;
					setRightView(8);
					setLeftView(10);
					_scene->_kernelMessages.reset();
					_scene->resetScene();
					_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('m', 2));
					_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('m', 4));
					_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('m', 9));
					_blinkFl = false;
					setLeftView(2);
					_game.loadQuoteSet(0xFE, 0);
					_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
					_scene->loadAnimation(formAnimName('a', -1), 70);
					_vm->_sound->command(17);
				}
			}
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_LEAVE, NOUN_SECURITY_STATION))
		_scene->_nextSceneId = 311;
	else if (_action.isAction(VERB_LOOK, NOUN_RIGHT_MONITOR))
		_vm->_dialogs->show(32001);
	else if (_action.isAction(VERB_LOOK, NOUN_LEFT_MONITOR))
		_vm->_dialogs->show(32002);
	else if (_action.isAction(VERB_LOOK, NOUN_DESK))
		_vm->_dialogs->show(32003);
	else if (_action.isAction(VERB_LOOK, NOUN_SECURITY_STATION))
		_vm->_dialogs->show(32004);
	else if (_action.isAction(VERB_LOOK, NOUN_MUG))
		_vm->_dialogs->show(32005);
	else if (_action.isAction(VERB_LOOK, NOUN_DOUGHNUT))
		_vm->_dialogs->show(32006);
	else if (_action.isAction(VERB_LOOK, NOUN_MAGAZINE))
		_vm->_dialogs->show(32007);
	else if (_action.isAction(VERB_LOOK, NOUN_PAPER_FOOTBALL))
		_vm->_dialogs->show(32008);
	else if (_action.isAction(VERB_LOOK, NOUN_NEWSPAPER))
		_vm->_dialogs->show(32009);
	else if (_action.isAction(VERB_LOOK, NOUN_CLIPBOARD))
		_vm->_dialogs->show(32010);
	else if (_action.isAction(VERB_TAKE, NOUN_MUG))
		_vm->_dialogs->show(32012);
	else if (_action.isAction(VERB_TAKE, NOUN_CLIPBOARD))
		_vm->_dialogs->show(32013);
	else if (_action.isAction(VERB_TAKE, NOUN_DOUGHNUT) || _action.isAction(VERB_EAT, NOUN_DOUGHNUT))
		_vm->_dialogs->show(32014);
	else if (_action.isAction(VERB_TAKE, NOUN_PAPER_FOOTBALL))
		_vm->_dialogs->show(32015);
	else if (_action.isAction(VERB_TAKE, NOUN_MAGAZINE))
		_vm->_dialogs->show(32016);
	else if (_action.isAction(VERB_TAKE, NOUN_NEWSPAPER))
		_vm->_dialogs->show(32017);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene321::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene321::enter() {
	_game._player._visible = false;
	_game._player._stepEnabled = false;

	_scene->_userInterface.emptyConversationList();
	_scene->_userInterface.setup(kInputConversation);

	int suffixNum;
	if (_globals[kSexOfRex] == REX_FEMALE) {
		_globals[kSexOfRex] = REX_MALE;
		suffixNum = 1;
	} else {
		_globals[kSexOfRex] = REX_FEMALE;
		suffixNum = _game._visitedScenes._sceneRevisited ? 2 : 0;
	}

	_scene->loadAnimation(formAnimName('g', suffixNum), 60);
	sceneEntrySound();
}

void Scene321::step() {
	if (_scene->_animation[0] != nullptr) {
		if ((_scene->_animation[0]->getCurrentFrame() >= 260) && (_globals[kSexOfRex] == REX_MALE) && (_game._storyMode >= STORYMODE_NICE))
			_scene->_nextSceneId = 316;
	}

	if (_game._trigger == 60)
		_scene->_nextSceneId = 316;
}

/*------------------------------------------------------------------------*/

void Scene322::setup() {
	_game._player._spritesPrefix = "";
	// The original is calling scene3xx_setAAName()
	_game._aaName = Resources::formatAAName(4);
}

void Scene322::enter() {
	if (_globals[kSexOfRex] == REX_MALE)
		_handSpriteId = _scene->_sprites.addSprites("*REXHAND");
	else
		_handSpriteId = _scene->_sprites.addSprites("*ROXHAND");

	teleporterEnter();

	// The original is using scene3xx_sceneEntrySound()
	if (!_vm->_musicFlag)
		_vm->_sound->command(2);
	else
		_vm->_sound->command(10);
}

void Scene322::step() {
	teleporterStep();
}

void Scene322::actions() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(32214);
		_action._inProgress = false;
		return;
	}

	if (teleporterActions()) {
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_VIEWPORT) || _action.isAction(VERB_PEER_THROUGH, NOUN_VIEWPORT))
		_vm->_dialogs->show(32210);
	else if (_action.isAction(VERB_LOOK, NOUN_KEYPAD))
		_vm->_dialogs->show(32211);
	else if (_action.isAction(VERB_LOOK, NOUN_DISPLAY))
		_vm->_dialogs->show(32212);
	else if (_action.isAction(VERB_LOOK, NOUN_0_KEY) || _action.isAction(VERB_LOOK, NOUN_1_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_2_KEY) || _action.isAction(VERB_LOOK, NOUN_3_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_4_KEY) || _action.isAction(VERB_LOOK, NOUN_5_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_6_KEY) || _action.isAction(VERB_LOOK, NOUN_7_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_8_KEY) || _action.isAction(VERB_LOOK, NOUN_9_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_SMILE_KEY) || _action.isAction(VERB_LOOK, NOUN_ENTER_KEY)
	 || _action.isAction(VERB_LOOK, NOUN_FROWN_KEY))
		_vm->_dialogs->show(32213);
	else if (_action.isAction(VERB_LOOK, NOUN_DEVICE))
		_vm->_dialogs->show(32214);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene351::setup() {
	if (_scene->_currentSceneId == 391)
		_globals[kSexOfRex] = REX_MALE;

	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(VERB_WALKTO);
}

void Scene351::enter() {
	_globals[kAfterHavoc] = -1;
	_globals[kTeleporterRoom + 1] = 351;

	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*ROXRC_7");
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXRD_7");

	if (_game._objects.isInRoom(OBJ_CREDIT_CHIP)) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 4);
	} else
		_scene->_hotspots.activate(NOUN_CREDIT_CHIP, false);

	if (_scene->_priorSceneId == 352)
		_game._player._playerPos = Common::Point(148, 152);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(207, 81);
		_game._player._facing = FACING_NORTH;
	}

	if (_globals[kTeleporterCommand]) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;

		char sepChar = 'a';
		if (_globals[kSexOfRex] != REX_MALE)
			sepChar = 'b';

		int suffixNum = -1;
		int trigger = 0;

		switch (_globals[kTeleporterCommand]) {
		case 1:
			suffixNum = 0;
			trigger = 60;
			_globals[kTeleporterCommand] = true;
			break;

		case 2:
			suffixNum = 1;
			trigger = 61;
			break;

		case 3:
		case 4:
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_game._player._turnToFacing = FACING_SOUTH;
			suffixNum = -1;
			break;

		default:
			break;
		}

		_globals[kTeleporterCommand] = 0;

		if (suffixNum >= 0)
			_scene->loadAnimation(formAnimName(sepChar, suffixNum), trigger);
	}

	sceneEntrySound();
}

void Scene351::step() {
	if (_game._trigger == 60) {
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_game._player._turnToFacing = FACING_SOUTH;
	}

	if (_game._trigger == 61) {
		_globals[kTeleporterCommand] = 1;
		_scene->_nextSceneId = _globals[kTeleporterDestination];
		_scene->_reloadSceneFlag = true;
	}
}

void Scene351::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(35121);
	else if (_action.isAction(VERB_STEP_INTO, NOUN_TELEPORTER))
		_scene->_nextSceneId = 322;
	else if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_SOUTH))
		_scene->_nextSceneId = 352;
	else if (_action.isAction(VERB_TAKE, NOUN_CREDIT_CHIP)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_CREDIT_CHIP)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				if (_globals[kSexOfRex] == REX_FEMALE) {
					_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 5, 2, 0, 0);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 5, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				} else {
					_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 5, 2, 0, 0);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 6, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				}
				break;

			case 1:
				_scene->_hotspots.activate(NOUN_CREDIT_CHIP, false);
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_game._objects.addToInventory(OBJ_CREDIT_CHIP);
				break;

			case 2:
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				_vm->_dialogs->showItem(OBJ_CREDIT_CHIP, 0x32F);
				break;

			default:
				break;
			}
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_VIEW_SCREEN))
		_vm->_dialogs->show(35110);
	else if (_action.isAction(VERB_LOOK, NOUN_RIP_IN_FLOOR))
		_vm->_dialogs->show(35111);
	else if (_action.isAction(VERB_LOOK, NOUN_FIRE_HYDRANT))
		_vm->_dialogs->show(35112);
	else if (_action.isAction(VERB_LOOK, NOUN_GUARD)) {
		if (_game._objects[0xF]._roomNumber == 351)
			_vm->_dialogs->show(35114);
		else
			_vm->_dialogs->show(35113);
	} else if (_action.isAction(VERB_LOOK, NOUN_EQUIPMENT))
		_vm->_dialogs->show(35115);
	else if (_action.isAction(VERB_LOOK, NOUN_DESK))
		_vm->_dialogs->show(35116);
	else if (_action.isAction(VERB_LOOK, NOUN_MACHINE))
		_vm->_dialogs->show(35117);
	else if (_action.isAction(VERB_LOOK, NOUN_TELEPORTER))
		_vm->_dialogs->show(35118);
	else if (_action.isAction(VERB_LOOK, NOUN_CONTROL_PANEL))
		_vm->_dialogs->show(35119);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_SOUTH))
		_vm->_dialogs->show(35120);
	else if (_action.isAction(VERB_LOOK, NOUN_POLE))
		_vm->_dialogs->show(35122);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene352::Scene352(MADSEngine *vm) : Scene3xx(vm) {
	_vaultOpenFl = false;
	_mustPutArmDownFl = false;
	_leaveRoomFl = false;

	_tapePlayerHotspotIdx = -1;
	_hotspot1Idx = -1;
	_hotspot2Idx = -1;
	_lampHostpotIdx = -1;
	_commonSequenceIdx = -1;
	_commonSpriteIndex = -1;
}

void Scene352::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	s.syncAsByte(_vaultOpenFl);
	s.syncAsByte(_mustPutArmDownFl);
	s.syncAsByte(_leaveRoomFl);

	s.syncAsSint32LE(_tapePlayerHotspotIdx);
	s.syncAsSint32LE(_hotspot1Idx);
	s.syncAsSint32LE(_hotspot2Idx);
	s.syncAsSint32LE(_lampHostpotIdx);
	s.syncAsSint32LE(_commonSequenceIdx);
	s.syncAsSint32LE(_commonSpriteIndex);
}

void Scene352::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_YOUR_STUFF);
	_scene->addActiveVocab(NOUN_OTHER_STUFF);
	_scene->addActiveVocab(NOUN_LAMP);
}

void Scene352::putArmDown(bool corridorExit, bool doorwayExit) {
	switch (_game._trigger) {
	case 0:
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0xFF));
		_scene->_sequences.addTimer(48, 1);
		break;

	case 1:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		if (_globals[kSexOfRex] == REX_FEMALE) {
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 5, 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		} else {
			_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 6, 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		}
		break;

	case 2: {
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 0, 0, 0);
		int idx = _scene->_dynamicHotspots.add(NOUN_GUARDS_ARM2, VERB_WALKTO, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(230, 117), FACING_NORTHWEST);
		_scene->changeVariant(0);
		}
		break;

	case 3:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x100));
		_game._objects.setRoom(OBJ_GUARDS_ARM, _scene->_currentSceneId);
		_game._player._visible = true;
		if (corridorExit)
			_scene->_sequences.addTimer(48, 6);
		else if (doorwayExit)
			_scene->_sequences.addTimer(48, 4);
		else {
			_mustPutArmDownFl = false;
			_game._player._stepEnabled = true;
		}
		break;

	case 4:
		_game._player.walk(Common::Point(116, 107), FACING_NORTH);
		_mustPutArmDownFl = false;
		_scene->_sequences.addTimer(180, 5);
		_leaveRoomFl = true;
		break;

	case 5:
		if (_leaveRoomFl)
			_scene->_nextSceneId = 351;

		break;

	case 6:
		_game._player.walk(Common::Point(171, 152), FACING_SOUTH);
		_game._player._stepEnabled = true;
		_mustPutArmDownFl = false;
		_scene->_sequences.addTimer(180, 7);
		_leaveRoomFl = true;
		break;

	case 7:
		if (_leaveRoomFl)
			_scene->_nextSceneId = 353;

		break;

	default:
		break;
	}
}

void Scene352::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*RM302x0");
	_globals._spriteIndexes[13] = _scene->_sprites.addSprites("*RM302x2");
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites("*RM302x3");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('g', -1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('b', -1));


	if (_globals[kSexOfRex] == REX_FEMALE) {
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*ROXRC_7");
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites("*ROXRC_6");
		_globals._spriteIndexes[15] = _scene->_sprites.addSprites("*ROXRC_9");
		_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('a', 3));
		_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('a', 2));
	} else {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXRD_7");
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites("*RXRC_6");
		_globals._spriteIndexes[14] = _scene->_sprites.addSprites("*RXMRC_9");
		_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('a', 1));
		_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('a', 0));
	}

	_leaveRoomFl = false;

	if (_game._objects.isInRoom(OBJ_TAPE_PLAYER)) {
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 12, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 5);
		int idx = _scene->_dynamicHotspots.add(NOUN_TAPE_PLAYER, VERB_WALKTO, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		_tapePlayerHotspotIdx = _scene->_dynamicHotspots.setPosition(idx, Common::Point(84, 145), FACING_WEST);
	}

	_vaultOpenFl = false;

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_mustPutArmDownFl = false;
		if (!_game._visitedScenes._sceneRevisited)
			_globals[kHaveYourStuff] = false;
	}

	if (_game._objects.isInRoom(OBJ_GUARDS_ARM)) {
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 0, 0, 0);
		int idx = _scene->_dynamicHotspots.add(NOUN_GUARDS_ARM2, VERB_WALKTO, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(230, 117), FACING_NORTHWEST);
	} else
		_mustPutArmDownFl = true;

	if (_scene->_priorSceneId == 353)
		_game._player._playerPos = Common::Point(171, 155);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(116, 107);

	sceneEntrySound();

	_game.loadQuoteSet(0xFF, 0x100, 0x101, 0x102, 0x103, 0);
}

void Scene352::preActions() {
	_leaveRoomFl = false;

	if (_action.isAction(VERB_OPEN, NOUN_VAULT))
		_game._player.walk(Common::Point(266, 111), FACING_NORTHEAST);

	if (_vaultOpenFl && !_action.isObject(NOUN_VAULT) && !_action.isObject(NOUN_LAMP) && !_action.isObject(NOUN_OTHER_STUFF) && !_action.isObject(NOUN_YOUR_STUFF)) {
		if (_globals[kHaveYourStuff]) {
			_commonSpriteIndex = _globals._spriteIndexes[13];
			_commonSequenceIdx = _globals._sequenceIndexes[13];
		} else {
			_commonSpriteIndex = _globals._spriteIndexes[1];
			_commonSequenceIdx = _globals._sequenceIndexes[1];
		}

		switch (_game._trigger) {
		case 0:
			if (_game._player._needToWalk) {
				_game._player._stepEnabled = false;
				_scene->_sequences.remove(_commonSequenceIdx);
				_vm->_sound->command(20);
				_commonSequenceIdx = _scene->_sequences.addReverseSpriteCycle(_commonSpriteIndex, false, 6, 1, 0, 0);
				_scene->_sequences.addSubEntry(_commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_scene->_sequences.setDepth(_commonSequenceIdx, 15);
			}
			break;

		case 1:
			if (!_globals[kHaveYourStuff])
				_scene->_dynamicHotspots.remove(_hotspot2Idx);

			_scene->_dynamicHotspots.remove(_hotspot1Idx);
			_scene->_dynamicHotspots.remove(_lampHostpotIdx);
			_vaultOpenFl = false;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	}

	if (_action.isAction(VERB_PUT, NOUN_GUARDS_ARM2, NOUN_SCANNER)) {
		if (_globals[kSexOfRex] == REX_MALE)
			_game._player.walk(Common::Point(269, 111), FACING_NORTHEAST);
		else
			_game._player.walk(Common::Point(271, 111), FACING_NORTHEAST);
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOORWAY) || _action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_SOUTH) || _action.isAction(VERB_PUT, NOUN_GUARDS_ARM2, NOUN_FLOOR)) {
		if (_game._objects.isInInventory(OBJ_GUARDS_ARM))
			_game._player.walk(Common::Point(230, 117), FACING_NORTHWEST);
	}
}

void Scene352::actions() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(35225);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_VAULT)) {
		if (!_vaultOpenFl) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				if (_globals[kSexOfRex] == REX_FEMALE)
					_commonSpriteIndex = _globals._spriteIndexes[9];
				else
					_commonSpriteIndex = _globals._spriteIndexes[8];

				_commonSequenceIdx = _scene->_sequences.addSpriteCycle(_commonSpriteIndex, false, 8, 1, 0, 0);
				_scene->_sequences.updateTimeout(_commonSequenceIdx, -1);
				_scene->_sequences.addSubEntry(_commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1: {
				_vm->_sound->command(21);
				_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 7, 2, 20, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[12], FACING_NORTH);
				int oldIdx = _commonSequenceIdx;
				_commonSequenceIdx = _scene->_sequences.startCycle(_commonSpriteIndex, false, -2);
				_scene->_sequences.updateTimeout(_commonSequenceIdx, oldIdx);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				}
				break;

			case 2:
				_vm->_sound->command(22);
				_scene->_sequences.remove(_commonSequenceIdx);
				_commonSequenceIdx = _scene->_sequences.startPingPongCycle(_commonSpriteIndex, false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_commonSequenceIdx, 1, 3);
				_scene->_sequences.addSubEntry(_commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				_scene->_sequences.updateTimeout(-1, _commonSequenceIdx);
				_game._player._visible = true;
				_scene->_sequences.addTimer(60, 4);
				break;

			case 4:
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x101));
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (_game._objects.isInInventory(OBJ_GUARDS_ARM)) {
		_mustPutArmDownFl = true;
	}

	bool exit_corridor = false;
	bool exit_doorway = false;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_SOUTH)) {
		exit_corridor = true;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOORWAY)) {
		exit_doorway = true;
	}

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_SOUTH) || _action.isAction(VERB_WALK_THROUGH, NOUN_DOORWAY) || _action.isAction(VERB_PUT, NOUN_GUARDS_ARM2, NOUN_FLOOR)) {
		if (_mustPutArmDownFl)
			putArmDown(exit_corridor, exit_doorway);
		else if (exit_corridor)
			_scene->_nextSceneId = 353;
		else
			_scene->_nextSceneId = 351;

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_GUARDS_ARM2)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_GUARDS_ARM)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				if (_globals[kSexOfRex] == REX_FEMALE) {
					_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 5, 2, 0, 0);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 5, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				} else {
					_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 5, 2, 0, 0);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 6, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				}
				break;

			case 1:
				_scene->_sequences.remove(_globals._sequenceIndexes[2]);
				_game._objects.addToInventory(OBJ_GUARDS_ARM);
				_scene->changeVariant(1);
				break;

			case 2:
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				_vm->_dialogs->showItem(OBJ_GUARDS_ARM, 0x899C);
				break;

			default:
				break;
			}
			_action._inProgress = false;
			return;
		}
	}

	if (_action.isAction(VERB_PUT, NOUN_GUARDS_ARM2, NOUN_SCANNER)) {
		if (!_vaultOpenFl) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				if (_globals[kSexOfRex] == REX_FEMALE)
					_commonSpriteIndex = _globals._spriteIndexes[11];
				else
					_commonSpriteIndex = _globals._spriteIndexes[10];

				_commonSequenceIdx = _scene->_sequences.addSpriteCycle(_commonSpriteIndex, false, 8, 1, 0, 0);
				_scene->_sequences.updateTimeout(_commonSequenceIdx, -1);
				_scene->_sequences.addSubEntry(_commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1: {
				_vm->_sound->command(21);
				_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 7, 2, 20, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 8);
				int oldIdx = _commonSequenceIdx;
				_commonSequenceIdx = _scene->_sequences.startCycle(_commonSpriteIndex, false, -2);
				_scene->_sequences.updateTimeout(_commonSequenceIdx, oldIdx);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				}
				break;

			case 2:
				_vm->_sound->command(23);
				_scene->_sequences.remove(_commonSequenceIdx);
				_commonSequenceIdx = _scene->_sequences.addReverseSpriteCycle(_commonSpriteIndex, false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_commonSequenceIdx, 1, 4);
				_scene->_sequences.addSubEntry(_commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;


			case 3:
				_scene->_sequences.updateTimeout(-1, _commonSequenceIdx);
				_game._player._visible = true;
				if (_globals[kHaveYourStuff])
					_commonSpriteIndex = _globals._spriteIndexes[13];
				else
					_commonSpriteIndex = _globals._spriteIndexes[1];

				_vm->_sound->command(20);
				_commonSequenceIdx = _scene->_sequences.addSpriteCycle(_commonSpriteIndex, false, 6, 1, 0, 0);
				_scene->_sequences.setDepth(_commonSequenceIdx, 15);
				_scene->_sequences.addSubEntry(_commonSequenceIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 4);
				break;

			case 4:
				_commonSequenceIdx = _scene->_sequences.addSpriteCycle(_commonSpriteIndex, false, 6, 0, 0, 0);
				_scene->_sequences.setAnimRange(_commonSequenceIdx, -2, -2);
				_scene->_sequences.setDepth(_commonSequenceIdx, 15);
				_scene->_sequences.addTimer(60, 5);
				break;

			case 5: {
				_vaultOpenFl = true;
				int idx;
				if (!_globals[kHaveYourStuff]) {
					idx = _scene->_dynamicHotspots.add(NOUN_YOUR_STUFF, VERB_WALKTO, -1, Common::Rect(282, 87, 282 + 13, 87 + 7));
					_hotspot2Idx = _scene->_dynamicHotspots.setPosition(idx, Common::Point(280, 111), FACING_NORTHEAST);
					_globals._sequenceIndexes[1] = _commonSequenceIdx;
					_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x102));
				} else {
					_globals._sequenceIndexes[13] = _commonSequenceIdx;
					_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x103));
				}

				idx = _scene->_dynamicHotspots.add(NOUN_OTHER_STUFF, VERB_WALKTO, -1, Common::Rect(282, 48, 282 + 36, 48 + 27));
				_hotspot1Idx = _scene->_dynamicHotspots.setPosition(idx, Common::Point(287, 115), FACING_NORTHEAST);
				idx = _scene->_dynamicHotspots.add(NOUN_LAMP, VERB_WALKTO, -1, Common::Rect(296, 76, 296 + 11, 76 + 17));
				_lampHostpotIdx = _scene->_dynamicHotspots.setPosition(idx, Common::Point(287, 115), FACING_NORTHEAST);
				_game._player._stepEnabled = true;
				}
				break;

			default:
				break;
			}
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_YOUR_STUFF)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			if (_globals[kSexOfRex] == REX_MALE) {
				_globals._sequenceIndexes[14] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[14], false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[14], 1, 2);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[14]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[14], SEQUENCE_TRIGGER_SPRITE, 2, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[14], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			} else {
				_globals._sequenceIndexes[15] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[15], false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[15], 1, 2);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[15]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[15], SEQUENCE_TRIGGER_SPRITE, 2, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[15], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

		case 1:
			_scene->_dynamicHotspots.remove(_hotspot2Idx);
			_globals[kHaveYourStuff] = true;

			for (uint16 i = 0; i < _game._objects.size(); i++) {
				if (_game._objects[i]._roomNumber == 50)
					_game._objects.addToInventory(i);
			}

			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[13] = _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 15);
			break;

		case 2:
			if (_globals[kSexOfRex] == REX_MALE)
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[14]);
			else
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[15]);

			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_TAPE_PLAYER) && !_game._objects.isInInventory(OBJ_TAPE_PLAYER)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			if (_globals[kSexOfRex] == REX_MALE) {
				_globals._sequenceIndexes[6] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[6], true, 6, 2, 0, 0);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[6]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_SPRITE, 6, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			} else {
				_globals._sequenceIndexes[7] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[7], true, 6, 2, 0, 0);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[7]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_SPRITE, 6, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			}
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[5]);
			_scene->_dynamicHotspots.remove(_tapePlayerHotspotIdx);
			break;

		case 2:
			_game._objects.addToInventory(OBJ_TAPE_PLAYER);
			if (_globals[kSexOfRex] == REX_MALE)
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[6]);
			else
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[7]);

			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_vm->_dialogs->showItem(OBJ_TAPE_PLAYER, 35227);
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_SCANNER))
		_vm->_dialogs->show(35210);
	else if (_action.isAction(VERB_LOOK, NOUN_MONITOR)) {
		if (_game._storyMode == STORYMODE_NAUGHTY)
			_vm->_dialogs->show(35211);
		else
			_vm->_dialogs->show(35212);
	} else if (_action.isAction(VERB_LOOK, NOUN_DISPLAY))
		_vm->_dialogs->show(35213);
	else if (_action.isAction(VERB_LOOK, NOUN_STATUE))
		_vm->_dialogs->show(35214);
	else if (_action.isAction(VERB_LOOK, NOUN_TAPE_PLAYER) && (_action._savedFields._mainObjectSource == 4))
		_vm->_dialogs->show(35215);
	else if (_action.isAction(VERB_LOOK, NOUN_AIR_VENT))
		_vm->_dialogs->show(35216);
	else if (_action.isAction(VERB_LOOK, NOUN_GUARDS_ARM2) && (_action._savedFields._mainObjectSource == 4))
		_vm->_dialogs->show(35217);
	else if (_action.isAction(VERB_LOOK, NOUN_IRONING_BOARD))
		_vm->_dialogs->show(35218);
	else if (_action.isAction(VERB_LOOK, NOUN_CLOCK))
		_vm->_dialogs->show(35219);
	else if (_action.isAction(VERB_LOOK, NOUN_GAUGE))
		_vm->_dialogs->show(35220);
	else if (_action.isAction(VERB_LOOK, NOUN_VAULT)) {
		if (!_vaultOpenFl)
			_vm->_dialogs->show(35221);
	} else if (_action.isAction(VERB_LOOK, NOUN_YOUR_STUFF))
		_vm->_dialogs->show(35222);
	else if (_action.isAction(VERB_LOOK, NOUN_OTHER_STUFF))
		_vm->_dialogs->show(35223);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_SOUTH))
		_vm->_dialogs->show(35224);
	else if (_action.isAction(VERB_TAKE, NOUN_OTHER_STUFF))
		_vm->_dialogs->show(35226);
	else if (_action.isAction(VERB_LOOK, NOUN_DESK))
		_vm->_dialogs->show(35229);
	else if (_action.isAction(VERB_LOOK, NOUN_GUARD))
		_vm->_dialogs->show(35230);
	else if (_action.isAction(VERB_LOOK, NOUN_DOORWAY))
		_vm->_dialogs->show(35231);
	else if (_action.isAction(VERB_LOOK, NOUN_TABLE))
		_vm->_dialogs->show(35232);
	else if (_action.isAction(VERB_LOOK, NOUN_PROJECTOR))
		_vm->_dialogs->show(35233);
	else if (_action.isAction(VERB_LOOK, NOUN_SUPPORT))
		_vm->_dialogs->show(35234);
	else if (_action.isAction(VERB_LOOK, NOUN_SECURITY_MONITOR))
		_vm->_dialogs->show(35235);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene353::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene353::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(Resources::formatName(303, 'B', 0, EXT_SS, ""));
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 0, 5, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

	if (_scene->_priorSceneId == 352)
		_game._player._playerPos = Common::Point(144, 95);
	else
		_game._player._playerPos = Common::Point(139, 155);

	sceneEntrySound();
}

void Scene353::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(35315);
	else if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOORWAY))
		_scene->_nextSceneId = 352;
	else if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_SOUTH))
		_scene->_nextSceneId = 354;
	else if (_action.isAction(VERB_LOOK, NOUN_ROCK_CHUNK))
		_vm->_dialogs->show(35310);
	else if (_action.isAction(VERB_LOOK, NOUN_PIPES) || _action.isAction(VERB_LOOK, NOUN_PIPE))
		_vm->_dialogs->show(35311);
	else if (_action.isAction(VERB_LOOK, NOUN_BROKEN_BEAM))
		_vm->_dialogs->show(35312);
	else if (_action.isAction(VERB_LOOK, NOUN_DOORWAY))
		_vm->_dialogs->show(35313);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_SOUTH))
		_vm->_dialogs->show(35314);
	else if (_action.isAction(VERB_LOOK, NOUN_FLOOR))
		_vm->_dialogs->show(35316);
	else if (_action.isAction(VERB_LOOK, NOUN_CEILING))
		_vm->_dialogs->show(35317);
	else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(35318);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene354::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene354::enter() {
	_globals[kAfterHavoc] = true;
	_globals[kTeleporterRoom + 1] = 351;

	if (_scene->_priorSceneId == 361)
		_game._player._playerPos = Common::Point(231, 110);
	else if (_scene->_priorSceneId == 401) {
		_game._player._playerPos = Common::Point(106, 152);
		_game._player._facing = FACING_NORTH;
	} else if (_scene->_priorSceneId == 316)
		_game._player._playerPos = Common::Point(71, 107);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(167, 57);

	sceneEntrySound();
}

void Scene354::preActions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_SOUTH))
		_game._player._walkOffScreenSceneId = 401;
}

void Scene354::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(35414);
	else if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_NORTH)) {
		_game._player.startWalking(Common::Point(208, 0), FACING_NORTHEAST);
		_game._player._walkOffScreenSceneId = 353;
	} else if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_EAST))
		_scene->_nextSceneId = 361;
	else if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_WEST))
		_scene->_nextSceneId = 316;
	else if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_SOUTH))
		_scene->_nextSceneId = 401;
	else if (_action.isAction(VERB_LOOK, NOUN_CONTROLS))
		_vm->_dialogs->show(35410);
	else if (_action.isAction(VERB_LOOK, NOUN_SIGNAL))
		_vm->_dialogs->show(35411);
	else if (_action.isAction(VERB_LOOK, NOUN_CATWALK))
		_vm->_dialogs->show(35412);
	else if (_action.isAction(VERB_LOOK, NOUN_AIR_DUCT))
		_vm->_dialogs->show(35413);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_NORTH))
		_vm->_dialogs->show(35415);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_SOUTH))
		_vm->_dialogs->show(35416);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_EAST))
		_vm->_dialogs->show(35417);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_WEST))
		_vm->_dialogs->show(35418);
	else if (_action.isAction(VERB_LOOK, NOUN_DEBRIS))
		_vm->_dialogs->show(35419);
	else if (_action.isAction(VERB_LOOK, NOUN_GUARD))
		_vm->_dialogs->show(35420);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene357::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene357::enter() {
	_globals[kAfterHavoc] = true;
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));
	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

	if (_scene->_priorSceneId == 318)
		_game._player._playerPos = Common::Point(298, 142);
	else if (_scene->_priorSceneId == 313)
		_game._player._playerPos = Common::Point(127, 101);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(15, 148);

	sceneEntrySound();
}

void Scene357::preActions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_EAST))
		_game._player._walkOffScreenSceneId = 318;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_WEST))
		_game._player._walkOffScreenSceneId = 358;
}

void Scene357::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(35715);
	else if (_action.isAction(VERB_LOOK, NOUN_AIR_VENT))
		_vm->_dialogs->show(35710);
	else if (_action.isAction(VERB_CLIMB_INTO, NOUN_AIR_VENT))
		_vm->_dialogs->show(35711);
	else if (_action.isAction(VERB_LOOK, NOUN_BED))
		_vm->_dialogs->show(35712);
	else if (_action.isAction(VERB_LOOK, NOUN_SINK))
		_vm->_dialogs->show(35713);
	else if (_action.isAction(VERB_LOOK, NOUN_TOILET))
		_vm->_dialogs->show(35714);
	else if (_action.isAction(VERB_LOOK, NOUN_CELL_WALL))
		_vm->_dialogs->show(35716);
	else if (_action.isAction(VERB_LOOK, NOUN_LIGHT))
		_vm->_dialogs->show(35717);
	else if (_action.isAction(VERB_LOOK, NOUN_RIP_IN_FLOOR))
		_vm->_dialogs->show(35718);
	else if (_action.isAction(VERB_LOOK, NOUN_DEBRIS))
		_vm->_dialogs->show(35719);
	else if (_action.isAction(VERB_TAKE, NOUN_DEBRIS))
		_vm->_dialogs->show(35720);
	else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(35721);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_EAST))
		_vm->_dialogs->show(35722);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_WEST))
		_vm->_dialogs->show(35723);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene358::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene358::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));
	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

	if (_scene->_priorSceneId == 357)
		_game._player._playerPos = Common::Point(305, 142);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(12, 141);

	sceneEntrySound();
}

void Scene358::preActions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_EAST))
		_game._player._walkOffScreenSceneId = 357;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_WEST))
		_game._player._walkOffScreenSceneId = 359;
}

void Scene358::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(35815);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_WEST))
		_vm->_dialogs->show(35810);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_EAST))
		_vm->_dialogs->show(35811);
	else if (_action.isAction(VERB_LOOK, NOUN_BED))
		_vm->_dialogs->show(35812);
	else if (_action.isAction(VERB_LOOK, NOUN_SINK))
		_vm->_dialogs->show(35813);
	else if (_action.isAction(VERB_LOOK, NOUN_TOILET))
		_vm->_dialogs->show(35814);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR) || _action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(35816);
	else if (_action.isAction(VERB_LOOK, NOUN_AIR_VENT))
		_vm->_dialogs->show(35817);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene359::Scene359(MADSEngine *vm) : Scene3xx(vm) {
	_cardHotspotId = -1;
}

void Scene359::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	s.syncAsSint32LE(_cardHotspotId);
}

void Scene359::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(VERB_WALKTO);
}

void Scene359::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('b', -1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));

	if (_globals[kSexOfRex] == REX_MALE)
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMBD_2");
	else
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*ROXBD_2");

	_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[3], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 15);

	if (_game._objects.isInRoom(OBJ_SECURITY_CARD)) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
		_cardHotspotId = _scene->_dynamicHotspots.add(NOUN_SECURITY_CARD, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_cardHotspotId, Common::Point(107, 107), FACING_SOUTH);
	}

	if (_scene->_priorSceneId == 358)
		_game._player._playerPos = Common::Point(301, 141);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(15, 148);

	sceneEntrySound();
}

void Scene359::preActions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_EAST))
		_game._player._walkOffScreenSceneId = 358;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_WEST))
		_game._player._walkOffScreenSceneId = 360;
}

void Scene359::actions() {
	if (_action._lookFlag) {
		if ((_game._difficulty != DIFFICULTY_HARD) && (_game._objects[OBJ_SECURITY_CARD]._roomNumber == 359))
			_vm->_dialogs->show(35914);
		else
			_vm->_dialogs->show(35915);
	} else if (_action.isAction(VERB_TAKE, NOUN_SECURITY_CARD)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_SECURITY_CARD)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_vm->_dialogs->show(35920);
				if (_globals[kSexOfRex] == REX_MALE) {
					_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 4, 2, 0, 0);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 6, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				} else {
					_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], true, 7, 2, 0, 0);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
					_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(106, 110));
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 6, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				}
				break;

			case 1:
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_scene->_dynamicHotspots.remove(_cardHotspotId);
				_vm->_sound->command(57);
				_game._objects.addToInventory(OBJ_SECURITY_CARD);
				_vm->_dialogs->showItem(OBJ_SECURITY_CARD, 0x330);
				_scene->changeVariant(1);
				break;

			case 2:
				if (_globals[kSexOfRex] == REX_MALE)
					_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
				else
					_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[4]);

				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_BLOODY_CELL_WALL))
		_vm->_dialogs->show(35910);
	else if (_action.isAction(VERB_LOOK, NOUN_BED))
		_vm->_dialogs->show(35911);
	else if (_action.isAction(VERB_LOOK, NOUN_SINK))
		_vm->_dialogs->show(35912);
	else if (_action.isAction(VERB_LOOK, NOUN_TOILET))
		_vm->_dialogs->show(35913);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_EAST))
		_vm->_dialogs->show(35916);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_WEST))
		_vm->_dialogs->show(35917);
	else if (_action.isAction(VERB_LOOK, NOUN_LIMB))
		_vm->_dialogs->show(35918);
	else if (_action.isAction(VERB_TAKE, NOUN_LIMB))
		_vm->_dialogs->show(35919);
	else if (_action.isAction(VERB_LOOK, NOUN_SECURITY_CARD) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(35921);
	else if (_action.isAction(VERB_LOOK, NOUN_BLOOD_STAIN)) {
		if ((_game._difficulty != DIFFICULTY_HARD) && (_game._objects[OBJ_SECURITY_CARD]._roomNumber == 359))
			_vm->_dialogs->show(35922);
		else
			_vm->_dialogs->show(35923);
	} else if (_action.isAction(VERB_LOOK, NOUN_WALL_BOARD))
		_vm->_dialogs->show(35924);
	else if (_action.isAction(VERB_TAKE, NOUN_WALL_BOARD))
		_vm->_dialogs->show(35925);
	else if (_action.isAction(VERB_LOOK, NOUN_RIP_IN_FLOOR))
		_vm->_dialogs->show(35926);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR))
		_vm->_dialogs->show(35927);
	else if (_action.isAction(VERB_LOOK, NOUN_FLOOR)) {
		if ((_game._difficulty != DIFFICULTY_HARD) && (_game._objects[OBJ_SECURITY_CARD]._roomNumber == 359))
			_vm->_dialogs->show(35928);
		else
			_vm->_dialogs->show(35929);
	} else if (_action.isAction(VERB_OPEN, NOUN_AIR_VENT) || _action.isAction(VERB_LOOK, NOUN_AIR_VENT))
		_vm->_dialogs->show(36016);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene360::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene360::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));
	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

	if (_scene->_priorSceneId == 359)
		_game._player._playerPos = Common::Point(304, 143);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(13, 141);

	sceneEntrySound();
}

void Scene360::preActions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_EAST))
		_game._player._walkOffScreenSceneId = 359;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_WEST))
		_game._player._walkOffScreenSceneId = 361;
}

void Scene360::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(36015);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_WEST))
		_vm->_dialogs->show(36010);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_EAST))
		_vm->_dialogs->show(36011);
	else if (_action.isAction(VERB_LOOK, NOUN_BED))
		_vm->_dialogs->show(36012);
	else if (_action.isAction(VERB_LOOK, NOUN_SINK))
		_vm->_dialogs->show(36013);
	else if (_action.isAction(VERB_LOOK, NOUN_TOILET))
		_vm->_dialogs->show(36014);
	else if (_action.isAction(VERB_LOOK, NOUN_AIR_VENT))
		_vm->_dialogs->show(36016);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR))
		_vm->_dialogs->show(36017);
	else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(36018);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene361::setup() {
	if (_scene->_currentSceneId == 391)
		_globals[kSexOfRex] = REX_MALE;

	setPlayerSpritesPrefix();
	setAAName();
}

void Scene361::handleRexAction() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 50, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 3, -2);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 15, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, 4);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		break;

	case 1: {
		int seqIdx = _globals._sequenceIndexes[1];
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 4);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], seqIdx);
		}
		break;

	case 2: {
		int seqIdx = _globals._sequenceIndexes[2];
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 4, 10);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], seqIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		}
		break;

	case 3: {
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 3);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

		int seqIdx = _globals._sequenceIndexes[2];
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 11);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(167, 100));
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], seqIdx);
		_scene->_sequences.addTimer(15, 4);
		}
		break;

	case 4:
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 2);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

		_scene->_sequences.setDone(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 12, 14);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(167, 100));
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
		break;

	case 5: {
		int seqIdx = _globals._sequenceIndexes[2];
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 15);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(167, 100));
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], seqIdx);
		_scene->_sequences.addTimer(15, 6);
		}
		break;

	case 6:
		_scene->_sequences.setDone(_globals._sequenceIndexes[2]);
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_scene->_sequences.addTimer(48, 7);
		break;

	case 7:
		_scene->_nextSceneId = 313;
		break;

	default:
		break;
	}
}

void Scene361::handleRoxAction() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 18, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 2, 4);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 18, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -1, 3);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		break;

	case 1: {
		int tmpIdx = _globals._sequenceIndexes[1];
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 4);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], tmpIdx);
		}
		break;

	case 2: {
		int tmpIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 4, 8);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], tmpIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		}
		break;

	case 3: {
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 3);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

		int tmpIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 9, 10);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(167, 100));
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], tmpIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
		}
		break;

	case 4: {
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 2);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

		int tmpIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 11, 15);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(167, 100));
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], tmpIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
		}
		break;

	case 5: {
		int tmpIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 16);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(167, 100));
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], tmpIdx);
		_scene->_sequences.addTimer(48, 6);
		}
		break;

	case 6:
		_scene->_sequences.setDone(_globals._sequenceIndexes[4]);

		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		_scene->_sequences.addTimer(48, 7);
		break;

	case 7:
		_scene->_nextSceneId = 313;
		break;

	default:
		break;
	}
}

void Scene361::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(Resources::formatName(307, 'X', 0, EXT_SS, ""));

	if (_globals[kSexOfRex] == REX_MALE) {
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXCL_8");
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXCL_2");
	} else
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*ROXCL_8");

	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

	if (_scene->_priorSceneId == 391) {
		_globals[kSexOfRex] = REX_MALE;
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_game._player._facing = FACING_SOUTH;
		_game._player._playerPos = Common::Point(166, 101);
		_scene->_sequences.addTimer(120, 70);
	} else if (_scene->_priorSceneId == 360)
		_game._player._playerPos = Common::Point(302, 145);
	else if (_scene->_priorSceneId == 320) {
		_game._player._playerPos = Common::Point(129, 113);
		_game._player._facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(13, 145);

	_game.loadQuoteSet(0xFB, 0xFC, 0);

	if (_scene->_priorSceneId == 320)
		_scene->_kernelMessages.setQuoted(_scene->_kernelMessages.addQuote(0xFB, 0, 0x78), 4, true);

	sceneEntrySound();
}

void Scene361::step() {
	if (_game._trigger >= 70) {
		switch (_game._trigger) {
		case 70:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 71);
			break;

		case 71:
			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 72);
			break;

		case 72:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 3);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 3);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 73);
			break;

		case 73:
			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 4, 5);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 74);
			break;

		case 74: {
			int seqIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 6);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], seqIdx);
			_scene->_sequences.addTimer(15, 75);
			}
			break;

		case 75:
			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 7);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 76);
			break;

		case 76:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);

			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 8);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 77);
			break;

		case 77:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(165, 76));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 9);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addTimer(15, 78);
			break;

		case 78:
			_scene->_sequences.setDone(_globals._sequenceIndexes[3]);
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 10, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 79);
			break;

		case 79:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);
			_game._player._stepEnabled = true;
			_game._player._visible = true;
			break;

		default:
			break;
		}
	}
}

void Scene361::preActions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_EAST))
		_game._player._walkOffScreenSceneId = 360;

	if (_action.isAction(VERB_WALK_DOWN, NOUN_CORRIDOR_TO_WEST))
		_game._player._walkOffScreenSceneId = 354;
}

void Scene361::actions() {
	if (_action._lookFlag)
		_vm->_dialogs->show(36119);
	else if (_action.isAction(VERB_SIT_AT, NOUN_DESK)) {
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.addQuote(252, 0, 120);
	} else if (_action.isAction(VERB_CLIMB_INTO, NOUN_AIR_VENT)) {
		if (_globals[kSexOfRex] == REX_FEMALE)
			handleRoxAction();
		else
			handleRexAction();
	} else if (_action.isAction(VERB_LOOK, NOUN_DESK))
		_vm->_dialogs->show(36110);
	else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(36111);
	else if (_action.isAction(VERB_LOOK, NOUN_LIGHTING_FIXTURE) || _action.isAction(VERB_STARE_AT, NOUN_LIGHTING_FIXTURE))
		_vm->_dialogs->show(36112);
	else if (_action.isAction(VERB_LOOK, NOUN_LIGHTS) || _action.isAction(VERB_STARE_AT, NOUN_LIGHTS))
		_vm->_dialogs->show(36113);
	else if (_action.isAction(VERB_TAKE, NOUN_LIGHTS))
		_vm->_dialogs->show(36114);
	else if (_action.isAction(VERB_LOOK, NOUN_LIGHT_BULB) || _action.isAction(VERB_STARE_AT, NOUN_LIGHT_BULB))
		_vm->_dialogs->show(36115);
	else if (_action.isAction(VERB_TAKE, NOUN_LIGHT_BULB))
		_vm->_dialogs->show(36116);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_WEST))
		_vm->_dialogs->show(36117);
	else if (_action.isAction(VERB_LOOK, NOUN_CORRIDOR_TO_EAST))
		_vm->_dialogs->show(36118);
	else if (_action.isAction(VERB_LOOK, NOUN_AIR_VENT))
		_vm->_dialogs->show(36120);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene366::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene366::enter() {
	_scene->_userInterface.setup(kInputLimitedSentences);
	_game._player._visible = false;
	sceneEntrySound();
}

void Scene366::actions() {
	if (_action.isAction(VERB_RETURN_TO, NOUN_AIR_SHAFT))
		_scene->_nextSceneId = 302;
	else if (_action.isAction(VERB_OPEN, NOUN_GRATE)) {
		if (_game._visitedScenes.exists(316))
			_vm->_dialogs->show(36612);
		else
			_vm->_dialogs->show(36613);
		_scene->_nextSceneId = 316;
	} else if (_action.isAction(VERB_LOOK_THROUGH, NOUN_GRATE)) {
		if (_game._visitedScenes.exists(321))
			_vm->_dialogs->show(36611);
		else
			_vm->_dialogs->show(36610);
	} else {
		return;
	}

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene387::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene387::enter() {
	_scene->_userInterface.setup(kInputLimitedSentences);
	_game._player._visible = false;

	sceneEntrySound();
}

void Scene387::actions() {
	if (_action.isAction(VERB_RETURN_TO, NOUN_AIR_SHAFT))
		_scene->_nextSceneId = 313;
	else if (_action.isAction(VERB_LOOK_THROUGH, NOUN_GRATE))
		_vm->_dialogs->show(38710);
	else if (_action.isAction(VERB_OPEN, NOUN_GRATE))
		_vm->_dialogs->show(38711);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene388::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene388::enter() {
	_scene->_userInterface.setup(kInputLimitedSentences);

	if (_globals[kAfterHavoc])
		_scene->_hotspots.activate(NOUN_SAUROPOD, false);
	else {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('b', 0));
		_globals._sequenceIndexes[0] = _scene->_sequences.startCycle(_globals._spriteIndexes[0], false, 1);
	}

	_game._player._visible = false;
	_vm->_palette->setEntry(252, 63, 30, 20);
	_vm->_palette->setEntry(253, 45, 15, 12);
	_game.loadQuoteSet(0x154, 0x155, 0x156, 0x157, 0x158, 0);

	sceneEntrySound();
}

void Scene388::actions() {
	if (_action.isAction(VERB_RETURN_TO, NOUN_AIR_SHAFT))
		_scene->_nextSceneId = 313;
	else if (_action.isAction(VERB_TALKTO, NOUN_SAUROPOD)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(160, 136), 0x1110, 32, 1, 120, _game.getQuote(0x154));
			break;

		case 1:
			_scene->_kernelMessages.add(Common::Point(82, 38), 0xFDFC, 0, 0, 300, _game.getQuote(0x156));
			_scene->_kernelMessages.add(Common::Point(82, 52), 0xFDFC, 0, 0, 300, _game.getQuote(0x157));
			_scene->_kernelMessages.add(Common::Point(82, 66), 0xFDFC, 0, 2, 300, _game.getQuote(0x158));
			break;

		case 2:
			_game._player._stepEnabled = true;
			_scene->_kernelMessages.add(Common::Point(160, 136), 0x1110, 32, 0, 120, _game.getQuote(0x155));
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_LOOK_THROUGH, NOUN_GRATE)) {
		if (_globals[kAfterHavoc])
			_vm->_dialogs->show(38811);
		else
			_vm->_dialogs->show(38810);
	} else if (_action.isAction(VERB_OPEN, NOUN_GRATE))
		_vm->_dialogs->show(38812);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

Scene389::Scene389(MADSEngine *vm) : Scene300s(vm) {
	_monsterTime = 0;
	_circularQuoteId = -1;
}

void Scene389::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	s.syncAsUint32LE(_monsterTime);
	s.syncAsSint32LE(_circularQuoteId);
}

void Scene389::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene389::enter() {
	_scene->_userInterface.setup(kInputLimitedSentences);
	_monsterTime = 0;
	_circularQuoteId = 0x159;

	if (_globals[kAfterHavoc])
		_scene->_hotspots.activate(NOUN_MONSTER, false);
	else {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('m', -1));
		_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6, 0, 0, 0);
		_scene->_kernelMessages.initRandomMessages(1,
			Common::Rect(88, 19, 177, 77), 13, 2, 0xFDFC, 60,
			247, 248, 249, 0);
	}

	_vm->_palette->setEntry(252, 63, 37, 26);
	_vm->_palette->setEntry(253, 45, 24, 17);
	_game._player._visible = false;
	_game.loadQuoteSet(0xF7, 0xF8, 0xF9, 0x159, 0x15A, 0x15B, 0);

	sceneEntrySound();
}

void Scene389::step() {
	_scene->_kernelMessages.randomServer();
	if (_scene->_frameStartTime >= _monsterTime) {
		int chanceMinor = _scene->_kernelMessages.checkRandom() * 4 + 1;
		_scene->_kernelMessages.generateRandom(20, chanceMinor);
		_monsterTime = _scene->_frameStartTime + 2;
	}
}

void Scene389::actions() {
	if (_action.isAction(VERB_RETURN_TO, NOUN_AIR_SHAFT))
		_scene->_nextSceneId = 313;
	else if (_action.isAction(VERB_TALKTO, NOUN_MONSTER)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_kernelMessages.add(Common::Point(160, 136), 0x1110, 32, 1, 120, _game.getQuote(_circularQuoteId));
			_circularQuoteId++;
			if (_circularQuoteId > 0x15B)
				_circularQuoteId = 0x159;

			break;

		case 1:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_LOOK_THROUGH, NOUN_GRATE)) {
		if (_globals[kAfterHavoc]) {
			if ((_game._difficulty != DIFFICULTY_HARD) && (_game._objects[OBJ_SECURITY_CARD]._roomNumber == 359))
				_vm->_dialogs->show(38911);
			else
				_vm->_dialogs->show(38912);
		} else
			_vm->_dialogs->show(38910);
	} else if (_action.isAction(VERB_OPEN, NOUN_GRATE)) {
		if (_globals[kAfterHavoc])
			_vm->_dialogs->show(38914);
		else
			_vm->_dialogs->show(38913);
	} else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene390::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene390::enter() {
	_scene->_userInterface.setup(kInputLimitedSentences);
	_game._player._visible = false;

	sceneEntrySound();
}

void Scene390::actions() {
	if (_action.isAction(VERB_RETURN_TO, NOUN_AIR_SHAFT))
		_scene->_nextSceneId = 313;
	else if (_action.isAction(VERB_LOOK_THROUGH, NOUN_GRATE))
		_vm->_dialogs->show(39010);
	else if (_action.isAction(VERB_OPEN, NOUN_GRATE))
		_vm->_dialogs->show(39011);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene391::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene391::enter() {
	_scene->_userInterface.setup(kInputLimitedSentences);
	_game._player._visible = false;
	sceneEntrySound();
}

void Scene391::actions() {
	if (_action.isAction(VERB_RETURN_TO, NOUN_AIR_SHAFT))
		_scene->_nextSceneId = 313;
	else if (_action.isAction(VERB_OPEN, NOUN_GRATE)) {
		if (_globals[kKickedIn391Grate])
			_vm->_dialogs->show(39113);
		else {
			_vm->_dialogs->show(39112);
			_globals[kKickedIn391Grate] = true;
		}

		if (_globals[kAfterHavoc])
			_scene->_nextSceneId = 361;
		else
			_scene->_nextSceneId = 311;
	} else if (_action.isAction(VERB_LOOK_THROUGH, NOUN_GRATE)) {
		if (_globals[kAfterHavoc])
			_vm->_dialogs->show(39111);
		else
			_vm->_dialogs->show(39110);
	} else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene399::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene399::enter() {
	_scene->_userInterface.setup(kInputLimitedSentences);
	_game._player._visible = false;
	sceneEntrySound();
}

void Scene399::actions() {
	if (_action.isAction(VERB_RETURN_TO, NOUN_AIR_SHAFT))
		_scene->_nextSceneId = 313;
	else if (_action.isAction(VERB_LOOK_THROUGH, NOUN_GRATE)) {
		if (_globals[kAfterHavoc]) {
			if ((_game._difficulty != DIFFICULTY_HARD) && (_game._objects[OBJ_SECURITY_CARD]._roomNumber == 359))
				_vm->_dialogs->show(38911);
			else
				_vm->_dialogs->show(38912);
		} else
			_vm->_dialogs->show(38910);
	} else if (_action.isAction(VERB_OPEN, NOUN_GRATE)) {
		if (_globals[kAfterHavoc])
			_vm->_dialogs->show(38914);
		else
			_vm->_dialogs->show(38913);
	} else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
