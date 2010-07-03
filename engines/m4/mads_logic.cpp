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
 * $URL$
 * $Id$
 *
 */

#include "m4/m4.h"
#include "m4/mads_logic.h"
#include "m4/scene.h"

namespace M4 {

/*--------------------------------------------------------------------------*/

const char *MadsSceneLogic::formAnimName(char sepChar, int16 suffixNum) {
	return MADSResourceManager::getResourceName(sepChar, _sceneNumber, EXTTYPE_NONE, NULL, suffixNum);
}

void MadsSceneLogic::getSceneSpriteSet() {
	char prefix[100];

	// Room change sound
	_madsVm->_sound->playSound(5);

	// Set up sprite set prefix to use
	if ((_sceneNumber <= 103) || (_sceneNumber == 111)) {
		if (_madsVm->globals()->_globals[0] == SEX_FEMALE)
			strcpy(prefix, "ROX");
		else
			strcpy(prefix, "RXM");
	} else if (_sceneNumber <= 110) {
		strcpy(prefix, "RXSW");
		_madsVm->globals()->_globals[0] = SEX_UNKNOWN;
	} else if (_sceneNumber == 112)
		strcpy(prefix, "");

	_madsVm->globals()->playerSpriteChanged = true;
	_madsVm->_player.loadSprites(prefix);

//	if ((_sceneNumber == 105) ((_sceneNumber == 109) && (word_84800 != 0)))
//		_madsVm->globals()->playerSpriteChanged = true;

	_vm->_palette->setEntry(16, 0x38, 0xFF, 0xFF);
	_vm->_palette->setEntry(17, 0x38, 0xb4, 0xb4);
}

void MadsSceneLogic::getAnimName() {
	const char *newName = MADSResourceManager::getAAName(
		((_sceneNumber <= 103) || (_sceneNumber > 111)) ? 0 : 1);
	strcpy(_madsVm->scene()->_aaName, newName);
}

IntStorage &MadsSceneLogic::dataMap() {
	return _madsVm->globals()->_dataMap;
}

/*--------------------------------------------------------------------------*/

uint16 MadsSceneLogic::loadSpriteSet(uint16 suffixNum, uint16 sepChar) {
	assert(sepChar < 256);
	const char *resName = formAnimName((char)sepChar, (int16)suffixNum);
	return _madsVm->scene()->loadSceneSpriteSet(resName);
}

uint16 MadsSceneLogic::startReversibleSpriteSequence(uint16 srcSpriteIdx, int v0, int numTicks, int triggerCountdown, int timeoutTicks, int extraTicks) {
	M4Sprite *spriteFrame = _madsVm->scene()->_spriteSlots.getSprite(srcSpriteIdx).getFrame(0);
	uint8 depth = _madsVm->_rails->getDepth(Common::Point(spriteFrame->x + (spriteFrame->width() / 2),
		spriteFrame->y + (spriteFrame->height() / 2)));

	return _madsVm->scene()->_sequenceList.add(srcSpriteIdx, v0, 1, triggerCountdown, timeoutTicks, extraTicks, numTicks, 0, 0, 
		true, 100, depth - 1, 1, ANIMTYPE_REVERSIBLE, 0, 0);
}

uint16 MadsSceneLogic::startCycledSpriteSequence(uint16 srcSpriteIdx, int v0, int numTicks, int triggerCountdown, int timeoutTicks, int extraTicks) {
	M4Sprite *spriteFrame = _madsVm->scene()->_spriteSlots.getSprite(srcSpriteIdx).getFrame(0);
	uint8 depth = _madsVm->_rails->getDepth(Common::Point(spriteFrame->x + (spriteFrame->width() / 2),
		spriteFrame->y + (spriteFrame->height() / 2)));

	return _madsVm->scene()->_sequenceList.add(srcSpriteIdx, v0, 1, triggerCountdown, timeoutTicks, extraTicks, numTicks, 0, 0, 
		true, 100, depth - 1, 1, ANIMTYPE_CYCLED, 0, 0);
}

uint16 MadsSceneLogic::startSpriteSequence3(uint16 srcSpriteIdx, int v0, int numTicks, int triggerCountdown, int timeoutTicks, int extraTicks) {
	M4Sprite *spriteFrame = _madsVm->scene()->_spriteSlots.getSprite(srcSpriteIdx).getFrame(0);
	uint8 depth = _madsVm->_rails->getDepth(Common::Point(spriteFrame->x + (spriteFrame->width() / 2),
		spriteFrame->y + (spriteFrame->height() / 2)));

	return _madsVm->scene()->_sequenceList.add(srcSpriteIdx, v0, 1, triggerCountdown, timeoutTicks, extraTicks, numTicks, 0, 0, 
		true, 100, depth - 1, -1, ANIMTYPE_CYCLED, 0, 0);
}

void MadsSceneLogic::activateHotspot(int idx, bool active) {
	// TODO:
}

void MadsSceneLogic::lowRoomsEntrySound() {
	if (!_madsVm->globals()->_config.musicFlag) {
		_madsVm->_sound->playSound(2);
	} else {
		// Play different sounds for each of the rooms
		switch (_madsVm->globals()->sceneNumber) {
		case 101:
			_madsVm->_sound->playSound(11);
			break;
		case 102:
			_madsVm->_sound->playSound(12);
			break;
		case 103:
			_madsVm->_sound->playSound(3);
			_madsVm->_sound->playSound(25);
			break;
		case 104:
			_madsVm->_sound->playSound(10);
			break;
		case 105:
			if ((_madsVm->globals()->previousScene < 104) || (_madsVm->globals()->previousScene > 108))
				_madsVm->_sound->playSound(10);
			break;
		case 106:
			_madsVm->_sound->playSound(13);
			break;
		case 107:
			_madsVm->_sound->playSound(3);
			break;
		case 108:
			_madsVm->_sound->playSound(15);
			break;
		default:
			break;
		}
	}
}

void MadsSceneLogic::getPlayerSpritesPrefix() {
	_madsVm->_sound->playSound(5);

	char oldName[80];
	strcpy(oldName, _madsVm->_player._spritesPrefix);

	if ((_madsVm->globals()->_nextSceneId <= 103) || (_madsVm->globals()->_nextSceneId == 111))
		strcpy(_madsVm->_player._spritesPrefix, (_madsVm->globals()->_globals[0] == SEX_FEMALE) ? "ROX" : "RXM");
	else if (_madsVm->globals()->_nextSceneId <= 110)
		strcpy(_madsVm->_player._spritesPrefix, "RXSM");
	else if (_madsVm->globals()->_nextSceneId == 112)
		strcpy(_madsVm->_player._spritesPrefix, "");
		
	if (strcmp(oldName, _madsVm->_player._spritesPrefix) != 0)
		_madsVm->_player._spritesChanged = true;

	if ((_madsVm->globals()->_nextSceneId == 105) || 
		((_madsVm->globals()->_nextSceneId == 109) && (_madsVm->globals()->_globals[15] != 0))) {
		// TODO: unknown flag setting
		_madsVm->_player._spritesChanged = true;
	}

	_madsVm->_palette->setEntry(16, 40, 255, 255);
	_madsVm->_palette->setEntry(17, 40, 180, 180);

}

void MadsSceneLogic::getPlayerSpritesPrefix2() {
	_madsVm->_sound->playSound(5);

	char oldName[80];
	strcpy(oldName, _madsVm->_player._spritesPrefix);

	if ((_madsVm->globals()->_nextSceneId == 213) || (_madsVm->globals()->_nextSceneId == 216))
		strcpy(_madsVm->_player._spritesPrefix, "");
	else if (_madsVm->globals()->_globals[0] == SEX_MALE)
		strcpy(_madsVm->_player._spritesPrefix, "RXM");
	else
		strcpy(_madsVm->_player._spritesPrefix, "ROX");

	// TODO: unknown flag setting for next scene Id > 212

	if (strcmp(oldName, _madsVm->_player._spritesPrefix) != 0)
		_madsVm->_player._spritesChanged = true;

/*	if ((_madsVm->globals()->_nextSceneId == 203) && (_madsVm->globals()->_nextSceneId == 204) &&
		(_madsVm->globals()->_globals[0x22] == 0))
		// TODO: unknown flag set
*/
	_madsVm->_palette->setEntry(16, 40, 255, 255);
	_madsVm->_palette->setEntry(17, 40, 180, 180);
}


/*--------------------------------------------------------------------------*/

/**
 * FIXME:
 * Currently I'm only working at providing manual implementation of the first Rex Nebular scene.
 * It will make more sense to convert the remaining game logic from the games into some
 * kind of bytecode scripts
 */

void MadsSceneLogic::selectScene(int sceneNum) {
	assert(sceneNum == 101);
	_sceneNumber = sceneNum;

	Common::set_to(&_spriteIndexes[0], &_spriteIndexes[50], 0);
}

void MadsSceneLogic::setupScene() {
	// FIXME: This is the hardcoded logic for Rex scene 101 only
	const char *animName = formAnimName('A', -1);
	warning("anim - %s", animName);

//	sub_1e754(animName, 3);

	if ((_sceneNumber >= 101) && (_sceneNumber <= 112))
		getPlayerSpritesPrefix();
	else
		getPlayerSpritesPrefix2();

	getAnimName();
}

void MadsSceneLogic::enterScene() {
	for (int i = 1; i <= 7; ++i)
		_spriteIndexes[i - 1] = loadSpriteSet(i, 'x');
	_spriteIndexes[7] = loadSpriteSet(0xFFFF, 'm');
	_spriteIndexes[8] = loadSpriteSet(1, 'b');
	_spriteIndexes[9] = loadSpriteSet(2, 'b');
	_spriteIndexes[10] = loadSpriteSet(0, 'a');
	_spriteIndexes[11] = loadSpriteSet(1, 'a');
	_spriteIndexes[12] = loadSpriteSet(8, 'x');
	_spriteIndexes[13] = loadSpriteSet(0, 'x');

	_spriteIndexes[15] = startCycledSpriteSequence(_spriteIndexes[0], 0, 5, 0, 0, 25);

	_spriteIndexes[16] = startCycledSpriteSequence(_spriteIndexes[1], 0, 4, 0, 1, 0);
	_spriteIndexes[17] = startCycledSpriteSequence(_spriteIndexes[2], 0, 4, 0, 1, 0);

	_madsVm->scene()->_sequenceList.addSubEntry(_spriteIndexes[17], SM_FRAME_INDEX, 7, 70);

	_spriteIndexes[18] = startReversibleSpriteSequence(_spriteIndexes[3], 0, 10, 0, 0, 60);
	_spriteIndexes[19] = startCycledSpriteSequence(_spriteIndexes[4], 0, 5, 0, 1, 0);
	_spriteIndexes[20] = startCycledSpriteSequence(_spriteIndexes[5], 0, 10, 0, 2, 0);
	_spriteIndexes[21] = startCycledSpriteSequence(_spriteIndexes[6], 0, 6, 0, 0, 0);

	_spriteIndexes[23] = startCycledSpriteSequence(_spriteIndexes[8], 0, 6, 0, 10, 4);
	_spriteIndexes[24] = startCycledSpriteSequence(_spriteIndexes[9], 0, 6, 0, 32, 47);

	activateHotspot(0x137, false);		// SHIELD MODULATOR
	// shield_panel_opened = 0;

	if (_madsVm->globals()->previousScene != -1)
		_madsVm->globals()->_globals[10] = 0;
	if (_madsVm->globals()->previousScene != -2) {
		_madsVm->_player._playerPos = Common::Point(100, 152);
	}
	
	if ((_madsVm->globals()->previousScene == 112) || 
		((_madsVm->globals()->previousScene != -2) && (_spriteIndexes[29] != 0))) {
		// Returning from probe cutscene?
		_spriteIndexes[29] = -1;
		_madsVm->_player._playerPos = Common::Point(161, 123);
		_madsVm->_player._direction = 9;

		// TODO: Extra flags setting
		_spriteIndexes[25] = startCycledSpriteSequence(_spriteIndexes[10], 0, 3, 0, 0, 0);
		_madsVm->scene()->_sequenceList.setAnimRange(_spriteIndexes[25], 17, 17);
		activateHotspot(0x47, false);	// CHAIR
		/*timer_unk1 = */_madsVm->scene()->_dynamicHotspots.add(0x47, 0x13F /*SIT_IN*/, -1,
			Common::Rect(159, 84, 159+33, 84+36));
		
		//if (_madsVm->globals()->previousScene == 112)
		//	room101Check();
	} else {
		_spriteIndexes[26] = startCycledSpriteSequence(_spriteIndexes[11], 0, 6, 0, 0, 0);
	}

	_madsVm->globals()->loadQuoteSet(0x31, 0x32, 0x37, 0x38, 0x39, -1);

	if (_madsVm->globals()->_globals[10]) {
		const char *animName = MADSResourceManager::getResourceName('S', 'e', EXTTYPE_AA, NULL, -1);
		_madsVm->scene()->loadAnimation(animName, 71);

		_madsVm->_player._playerPos = Common::Point(68, 140);
		_madsVm->_player._direction = 4;
		_madsVm->_player._visible = false;

		dataMap()[0x56FC] = 0;
		dataMap()[0x5482] = 0;
		dataMap()[0x5484] = 30;
	}

	_madsVm->globals()->_dataMap[0x5486] = 0;
	lowRoomsEntrySound();
}

void MadsSceneLogic::doAction() {

}

void MadsSceneLogic::sceneStep() {
	// TODO: Sound handling
	
	switch (_madsVm->scene()->_abortTimers) {
	case 70:
		_madsVm->_sound->playSound(9);
		break;
	case 71:
		_madsVm->globals()->_globals[10] = 0;
		_madsVm->_player._visible = true;
		dataMap()[0x56FC] = 0;

		_madsVm->scene()->_newTimeout = _madsVm->_currentTimer - _madsVm->scene()->_ticksAmount;
		break;
	case 72:
	case 73:
		// TODO: Method that should be scripted
		break;

	default:
		break;
	}

	// Wake up message sequence
	Animation *anim = _madsVm->scene()->activeAnimation();
	if (anim) {
		if ((anim->getCurrentFrame() == 6) && (dataMap()[0x5482] == 0)) {
			dataMap()[0x5482]++;
			_madsVm->scene()->_kernelMessages.add(Common::Point(63, dataMap()[0x5484]), 
				0x1110, 0, 0, 240, _madsVm->globals()->getQuote(49));
			dataMap()[0x5484] += 14;
		}

		if ((anim->getCurrentFrame() == 7) && (dataMap()[0x5482] == 1)) {
			dataMap()[0x5482]++;
			_madsVm->scene()->_kernelMessages.add(Common::Point(63, dataMap()[0x5484]), 
				0x1110, 0, 0, 240, _madsVm->globals()->getQuote(54));
			dataMap()[0x5484] += 14;
		}

		if ((anim->getCurrentFrame() == 10) && (dataMap()[0x5482] == 2)) {
			dataMap()[0x5482]++;
			_madsVm->scene()->_kernelMessages.add(Common::Point(63, dataMap()[0x5484]), 
				0x1110, 0, 0, 240, _madsVm->globals()->getQuote(55));
			dataMap()[0x5484] += 14;
		}

		if ((anim->getCurrentFrame() == 17) && (dataMap()[0x5482] == 3)) {
			dataMap()[0x5482]++;
			_madsVm->scene()->_kernelMessages.add(Common::Point(63, dataMap()[0x5484]), 
				0x1110, 0, 0, 240, _madsVm->globals()->getQuote(56));
			dataMap()[0x5484] += 14;
		}

		if ((anim->getCurrentFrame() == 20) && (dataMap()[0x5482] == 4)) {
			dataMap()[0x5482]++;
			_madsVm->scene()->_kernelMessages.add(Common::Point(63, dataMap()[0x5484]), 
				0x1110, 0, 0, 240, _madsVm->globals()->getQuote(50));
			dataMap()[0x5484] += 14;
		}
	}		
}

}
