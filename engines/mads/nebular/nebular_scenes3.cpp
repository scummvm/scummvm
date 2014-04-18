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
			_scene->_sequences.setMsgPosition(force->_seqId[id], Common::Point(posX, posY));
			_scene->_sequences.sub70C52(force->_seqId[id], 2, speedX, speedY);
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

	if ((_scene->_activeAnimation != nullptr) && (_scene->_activeAnimation->getCurrentFrame() != _oldFrame)) {
		_oldFrame = _scene->_activeAnimation->getCurrentFrame();
		if (_oldFrame == 147) {
			_game._objects.setRoom(OBJ_POISON_DARTS, 1);
			_game._objects.setRoom(OBJ_BLOWGUN, 1);
			_game._objects.setRoom(OBJ_REBREATHER, 1);
			_game._objects.setRoom(OBJ_STUFFED_FISH, 1);
			_game._objects.setRoom(OBJ_DEAD_FISH, 1);
			_game._objects.setRoom(OBJ_BURGER, 1);

			int count = (int)_game._objects._inventoryList.size();
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

		_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 150, 0, 3, 0);
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

void Scene307::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(0x11);
	_scene->addActiveVocab(0x2F7);
}

void Scene307::handleRexDialog(int quote) {
	Common::String curQuote = _game.getQuote(_action._activeAction._verbId);
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
	}
}

void Scene307::handlePrisonerSpeech(int firstQuoteId, int number, long time) {
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
		_scene->_kernelMessages.add(Common::Point(5, posY), 0xFDFC, 0, 81, time, _game.getQuote(quoteId));
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
		handlePrisonerSpeech(0x10F, 2, 9999999);
		warning("TODO: talk_init(&dialog1);");
		break;

	case 2:
		_globals[kMetBuddyBeast] = true;
		handlePrisonerSpeech(0x111, 2, 9999999);
		warning("TODO: talk_init(&dialog1);");
		break;

	case 4:
		handlePrisonerSpeech(0x116, 1, 120);
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	case 5:
		_globals[kKnowsBuddyBeast] = true;
		handlePrisonerSpeech(0x117, 2, 9999999);
		warning("TODO: talk_init(&dialog2);");
		break;

	case 6:
		handlePrisonerSpeech(0x123, 1, 120);
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	case 7:
		_globals[kKnowsBuddyBeast] = true;
		handlePrisonerSpeech(0x124, 10, 9999999);
		warning("TODO: sub71B9E(&dialog2, 0x11A, false);");
		warning("TODO: sub71B9E(&dialog2, 0x11B, true);");
		warning("TODO: sub71B9E(&dialog2, 0x120, true);");
		warning("TODO: talk_init(&dialog2);");
		break;

	case 8:
		handlePrisonerSpeech(0x12E, 6, 9999999);
		warning("TODO: sub71B9E(&dialog2, 0x11A, false);");
		warning("TODO: sub71B9E(&dialog2, 0x11B, false);");
		warning("TODO: sub71B9E(&dialog2, 0x11C, true);");
		warning("TODO: sub71B9E(&dialog2, 0x11D, true);");
		warning("TODO: sub71B9E(&dialog2, 0x11F, true);");
		warning("TODO: talk_init(&dialog2);");
		break;

	case 9:
		handlePrisonerSpeech(0x134, 4, 9999999);
		warning("TODO: sub71B9E(&dialog2, 0x11D, false);");
		warning("TODO: talk_init(&dialog2);");
		break;

	case 10:
		handlePrisonerSpeech(0x138, 6, 9999999);
		warning("TODO: sub71B9E(&dialog2, 0x11E, false);");
		warning("TODO: talk_init(&dialog2);");
		break;

	case 11:
		handlePrisonerSpeech(0x13E, 6, 9999999);
		warning("TODO: sub71B9E(&dialog2, 0x11F, false);");
		warning("TODO: sub71B9E(&dialog2, 0x121, true);");
		warning("TODO: talk_init(&dialog2);");
		break;

	case 12:
		handlePrisonerSpeech(0x144, 4, 9999999);
		warning("TODO: sub71B9E(&dialog2, 0x11C, false);");
		warning("TODO: talk_init(&dialog2);");
		break;

	case 13:
		handlePrisonerSpeech(0x148, 7, 9999999);
		warning("TODO: sub71B9E(&dialog2, 0x120, false);");
		warning("TODO: talk_init(&dialog2);");
		break;

	case 14:
		handlePrisonerSpeech(0x14F, 3, 9999999);
		warning("TODO: sub71B9E(&dialog2, 0x121, false);");
		warning("TODO: talk_init(&dialog2);");
		break;

	case 15:
		handlePrisonerSpeech(0x152, 1, 120);
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	case 16:
		_globals[kKnowsBuddyBeast] = true;
		handlePrisonerSpeech(0x10C, 1, 9999999);
		warning("TODO: talk_init(&dialog2);");
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
	_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
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

	warning("TODO: sub71A50(&dialog1, 0x3F, 0x113, 0x114, 0x115, -1);");
	warning("TODO: sub71A50(&dialog2, 0x40, 0x11A, 0x11B, 0x11C, 0x11D, 0x11E, 0x11F, 0x120, 0x121, 0x122, 0);");

	if (!_game._visitedScenes._sceneRevisited)
		warning("TODO: sub71B18(&dialog2, 0x11A, 0x122, 0);");
	else if (_scene->_priorSceneId == 318)
		warning("TODO: sub71B9E(&dialog2, 0x11E, -1);");


	if (_scene->_priorSceneId == -2) {
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

		int idx = _scene->_dynamicHotspots.add(17, 0x2F7, -1, Common::Rect(117, 67, 117 + 19, 67 + 13));
		int hotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(129, 104), FACING_NORTH);
		_scene->_dynamicHotspots.setCursor(hotspotId, CURSOR_GO_UP);

		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 2);
		_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 15);
	}

	_vm->_palette->setEntry(252, 63, 30, 20);
	_vm->_palette->setEntry(253, 45, 15, 12);

	sceneEntrySound();

	if ((_scene->_priorSceneId == 318) || (_scene->_priorSceneId == 387))
		_scene->_kernelMessages.addQuote(0xF3, 120, 0);
}

void Scene307::step() {
	handleForceField(&_forceField, &_globals._spriteIndexes[0]);

	if ((_animationMode == 1) && (_scene->_activeAnimation != nullptr)) {
		if (_scene->_activeAnimation->getCurrentFrame() == 126) {
			_forceField._flag = false;
			_vm->_sound->command(5);
		}

		if (_scene->_activeAnimation->getCurrentFrame() == 194) {
			_forceField._flag = true;
			_vm->_sound->command(24);
		}
	}

	if ((_animationMode == 2) && (_scene->_activeAnimation != nullptr)) {
		if (_scene->_activeAnimation->getCurrentFrame() == 54)
			_forceField._flag = false;

		if (_scene->_activeAnimation->getCurrentFrame() == 150) {
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

		if ((_guardTime > 3000) && !_duringPeeingFl && (_scene->_activeAnimation == nullptr)
		&& (_game._screenObjects._inputMode != 1) && _globals[kMetBuddyBeast] && !_activePrisonerFl) {
			if (!_game._objects.isInInventory(OBJ_SCALPEL) && !_grateOpenedFl) {
				_game._player._stepEnabled = false;
				_game._player.walk(Common::Point(151, 119), FACING_SOUTHEAST);
				_animationMode = 2;
				_vm->_sound->command(11);
				_scene->loadAnimation(formAnimName('b', -1), 70);
			}
			_guardTime = 0;
		} else if ((_prisonerTimer > 300) && (_game._screenObjects._inputMode != 1) && (_scene->_activeAnimation == nullptr) && !_activePrisonerFl) {
			if (!_globals[kMetBuddyBeast]) {
				int idx = _scene->_kernelMessages.add(Common::Point(5, 51), 0xFDFC, 0, 81, 120, _game.getQuote(_prisonerMessageId));
				_scene->_kernelMessages.setQuoted(idx, 4, true);
				_prisonerMessageId++;
				if (_prisonerMessageId > 0x10A)
					_prisonerMessageId = 0x104;
			} else if (_globals[kKnowsBuddyBeast] && (_vm->getRandomNumber(1, 3) == 1)) {
				warning("TODO: if (_globals[kKnowsBuddyBeast] && (sub71C16(&dialog2, 0) > 1) && (_vm->getRandomNumber(1, 3) == 1)) {");
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
		_vm->_dialogs->show(0x77FB);
	else if (_game._screenObjects._inputMode == 1)
		handleDialog();
	else if (_action.isAction(VERB_TALKTO, 0x207) || _action.isAction(VERB_TALKTO, 0x18D) || _action.isAction(VERB_TALKTO, 0x21B)) {
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
		}
	} else if (_action.isAction(0x11C, 0x12C, 0x11)) {
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
			_globals._sequenceIndexes[5] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[5], false, 12, 6, 0, 0);
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
			_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
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
			int idx = _scene->_dynamicHotspots.add(17, 0x2F7, -1, Common::Rect(117, 67, 117 + 19, 67 + 13));
			int hotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(129, 104), FACING_NORTH);
			_scene->_dynamicHotspots.setCursor(hotspotId, CURSOR_GO_UP);
			_game._objects.removeFromInventory(OBJ_SCALPEL, NOWHERE);
			_scene->_kernelMessages.addQuote(0xF2, 120, 7);
			}
			break;

		case 7:
			_scene->_sprites.remove(_globals._spriteIndexes[5]);
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	}else if (_action.isAction(0x2F7, 0x11)) {
		if (_grateOpenedFl) {
			switch (_game._trigger) {
			case 0:
				_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RXCL_8");
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[4]);
				_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 60, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 3, -2);
				_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 15);
				_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 18, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, 4);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
				_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
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
				_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
				_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 11);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
				_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[5], Common::Point(129, 102));
				_scene->_sequences.addTimer(48, 4);
				break;

			case 4:
				_scene->_sequences.remove(_globals._sequenceIndexes[4]);
				_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 2);
				_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
				_scene->_sequences.remove(_globals._sequenceIndexes[5]);
				_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 12, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 12, 14);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
				_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[5], Common::Point(129, 102));
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
				break;

			case 5:
				_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 15);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
				_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[5], Common::Point(129, 102));
				_scene->_sequences.addTimer(48, 6);
				break;

			case 6:
				_scene->_sequences.remove(_globals._sequenceIndexes[5]);
				_scene->_sequences.remove(_globals._sequenceIndexes[4]);
				_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
				_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
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
	} else if (_action.isAction(0x20C, 0x21B) && (_game._storyMode != STORYMODE_NAUGHTY))
		_vm->_dialogs->show(0x7803);
	else if (_action.isAction(0x20C, 0x21B)) {
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
	} else if (_action.isAction(VERB_LOOK, 0x11)) {
		if (!_grateOpenedFl)
			_vm->_dialogs->show(0x77F6);
		else
			_vm->_dialogs->show(0x77F7);
	} else if (_action.isAction(VERB_LOOK, 0x1CB))
		_vm->_dialogs->show(0x77F8);
	else if (_action.isAction(VERB_LOOK, 0x216))
		_vm->_dialogs->show(0x77F9);
	else if (_action.isAction(VERB_LOOK, 0x21B))
		_vm->_dialogs->show(0x77FA);
	else if (_action.isAction(0x134, 0x12C))
		_vm->_dialogs->show(0x77FC);
	else if (_action.isAction(VERB_LOOK, 0x207))
		_vm->_dialogs->show(0x77FD);
	else if (_action.isAction(VERB_LOOK, 0xCD))
		_vm->_dialogs->show(0x77FE);
	else if (_action.isAction(0x242, 0x204)) {
		switch (_fieldCollisionCounter) {
		case 0:
			_vm->_dialogs->show(0x77FF);
			_fieldCollisionCounter = 1;
			break;

		case 1:
			_vm->_dialogs->show(0x7800);
			_fieldCollisionCounter = 2;
			break;

		case 2:
			_vm->_dialogs->show(0x7801);
			_fieldCollisionCounter = 3;
			break;

		case 3:
			_vm->_dialogs->show(0x7802);
			break;
		}
	} else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

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
	_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
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
		_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 18, 9, 0, 0);
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
		_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 20, 5, 0, 0);
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
		_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 20, 8, 0, 0);
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
		_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 26, 0, 0, 0);
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
	_scene->_sequences.setMsgPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
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

	_characterSpriteIndexes[0] = _scene->_activeAnimation->_spriteListIndexes[2];
	_characterSpriteIndexes[1] = _scene->_activeAnimation->_spriteListIndexes[2];
	_characterSpriteIndexes[2] = _scene->_activeAnimation->_spriteListIndexes[1];

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

	if (_scene->_activeAnimation != nullptr) {
		if (_lastFrame != _scene->_activeAnimation->getCurrentFrame()) {
			_lastFrame = _scene->_activeAnimation->getCurrentFrame();
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
					for (j = _scene->_activeAnimation->_oldFrameEntry; j < _scene->_activeAnimation->_header._frameEntriesCount; j++) {
						if (_scene->_activeAnimation->_frameEntries[j]._spriteSlot._spritesIndex == _characterSpriteIndexes[charIdx]) {
							match = true;
							break;
						}
					}

					if (match) {
						SpriteSlotSubset *curSpriteSlot = &_scene->_activeAnimation->_frameEntries[j]._spriteSlot;
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
			_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 7, 4, 0, 0);
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
			_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 7, 8, 0, 0);
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
			_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 7, 6, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 21, 23);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], _oldIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
			}
			break;

		case 75: {
			int _oldIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 12, 6, 0, 0);
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
			_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 90, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 29, 30);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 11);
			int idx = _scene->_kernelMessages.add(Common::Point(15, 46), 0xFDFC, 0, 0, 120, _game.getQuote(247));
			_scene->_kernelMessages.setQuoted(idx, 2, true);
			_scene->_sequences.addTimer(120, 78);
			}
			break;
		}
	}

	if (_game._trigger == 60)
		_scene->_nextSceneId = 308;
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
