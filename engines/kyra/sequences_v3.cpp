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

#include "kyra/kyra_v3.h"
#include "kyra/resource.h"

namespace Kyra {

void KyraEngine_v3::showBadConscience() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::showBadConscience()");
	if (_badConscienceShown)
		return;

	_badConscienceShown = true;
	_badConscienceAnim = _rnd.getRandomNumberRng(0, 2);
	if (_currentChapter == 2)
		_badConscienceAnim = 5;
	else if (_currentChapter == 3)
		_badConscienceAnim = 3;
	else if (_currentChapter == 4 && _rnd.getRandomNumberRng(1, 100) <= 25)
		_badConscienceAnim = 6;
	else if (_currentChapter == 5 && _rnd.getRandomNumberRng(1, 100) <= 25)
		_badConscienceAnim = 7;
	else if (_malcolmShapes == 9)
		_badConscienceAnim = 4;

	_badConsciencePosition = (_mainCharacter.x1 <= 160);

	if (_goodConscienceShown)
		_badConsciencePosition = !_goodConsciencePosition;
	
	int anim = _badConscienceAnim + (_badConsciencePosition ? 0 : 8);
	TalkObject &talkObject = _talkObjectList[1];

	if (_badConsciencePosition)
		talkObject.x = 290;
	else
		talkObject.x = 30;
	talkObject.y = 30;

	static const char *animFilenames[] = {
		"GUNFL00.WSA", "GUNFL01.WSA", "GUNFL02.WSA", "GUNFL03.WSA", "GUNFL04.WSA", "GUNFL05.WSA", "GUNFL06.WSA", "GUNFL07.WSA",
		"GUNFR00.WSA", "GUNFR01.WSA", "GUNFR02.WSA", "GUNFR03.WSA", "GUNFR04.WSA", "GUNFR05.WSA", "GUNFR06.WSA", "GUNFR07.WSA"
	};

	setupSceneAnimObject(0x0E, 9, 0, 187, -1, -1, -1, -1, 0, 0, 0, -1, animFilenames[anim]);
	for (uint i = 0; i <= _badConscienceFrameTable[_badConscienceAnim]; ++i) {
		if (i == 8)
			snd_playSoundEffect(0x1B, 0xC8);
		updateSceneAnim(0x0E, i);
		delay(3*_tickLength, true);
	}

	if (_mainCharacter.animFrame < 50 || _mainCharacter.animFrame > 87)
		return;

	if (_mainCharacter.y1 == -1 || (_mainCharacter.x1 != -1 && _mainCharacter.animFrame == 87) || _mainCharacter.animFrame == 87) {
		_mainCharacter.animFrame = 87;
	} else {
		if (_badConsciencePosition)
			_mainCharacter.facing = 3;
		else
			_mainCharacter.facing = 5;
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	}

	updateCharacterAnim(0);
	refreshAnimObjectsIfNeed();
}

void KyraEngine_v3::hideBadConscience() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::hideBadConscience()");
	if (!_badConscienceShown)
		return;

	_badConscienceShown = false;
	for (int frame = _badConscienceFrameTable[_badConscienceAnim+8]; frame >= 0; --frame) {
		if (frame == 15)
			snd_playSoundEffect(0x31, 0xC8);
		updateSceneAnim(0x0E, frame);
		delay(1*_tickLength, true);
	}

	updateSceneAnim(0x0E, -1);
	update();
	removeSceneAnimObject(0x0E, 1);
	setNextIdleAnimTimer();
}

void KyraEngine_v3::showGoodConscience() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::showGoodConscience()");

	if (_goodConscienceShown)
		return;

	_goodConscienceShown = true;
	++_goodConscienceAnim;
	_goodConscienceAnim %= 5;
	
	setNextIdleAnimTimer();
	_goodConsciencePosition = (_mainCharacter.x1 <= 160);

	if (_badConscienceShown)
		_goodConsciencePosition = !_badConsciencePosition;

	int anim = _goodConscienceAnim + (_goodConsciencePosition ? 0 : 5);
	TalkObject &talkObject = _talkObjectList[87];

	if (_goodConsciencePosition)
		talkObject.x = 290;
	else
		talkObject.x = 30;
	talkObject.y = 30;

	static const char *animFilenames[] = {
		"STUFL00.WSA", "STUFL02.WSA", "STUFL04.WSA", "STUFL03.WSA", "STUFL01.WSA",
		"STUFR00.WSA", "STUFR02.WSA", "STUFR04.WSA", "STUFR03.WSA", "STUFR01.WSA"
	};

	setupSceneAnimObject(0x0F, 9, 0, 187, -1, -1, -1, -1, 0, 0, 0, -1, animFilenames[anim]);
	for (uint i = 0; i <= _goodConscienceFrameTable[_goodConscienceAnim]; ++i) {
		if (i == 10)
			snd_playSoundEffect(0x7F, 0xC8);
		updateSceneAnim(0x0F, i);
		delay(2*_tickLength, true);
	}

	if (_mainCharacter.animFrame < 50 || _mainCharacter.animFrame > 87)
		return;

	if (_mainCharacter.y1 == -1 || (_mainCharacter.x1 != -1 && _mainCharacter.animFrame == 87) || _mainCharacter.animFrame == 87) {
		_mainCharacter.animFrame = 87;
	} else {
		if (_goodConsciencePosition)
			_mainCharacter.facing = 3;
		else
			_mainCharacter.facing = 5;
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	}

	updateCharacterAnim(0);
	refreshAnimObjectsIfNeed();
}

void KyraEngine_v3::hideGoodConscience() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::hideGoodConscience()");
	if (!_goodConscienceShown)
		return;

	_goodConscienceShown = false;
	for (int frame = _goodConscienceFrameTable[_goodConscienceAnim+5]; frame >= 0; --frame) {
		if (frame == 17)
			snd_playSoundEffect(0x31, 0xC8);
		updateSceneAnim(0x0F, frame);
		delay(1*_tickLength, true);
	}

	updateSceneAnim(0x0F, -1);
	update();
	removeSceneAnimObject(0x0F, 1);
	setNextIdleAnimTimer();
}

void KyraEngine_v3::runTemporaryScript(const char *filename, int allowSkip, int resetChar, int newShapes, int shapeUnload) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::runTemporaryScript('%s', %d, %d, %d, %d)", filename, allowSkip, resetChar, newShapes, shapeUnload);
	memset(&_temporaryScriptData, 0, sizeof(_temporaryScriptData));
	memset(&_temporaryScriptState, 0, sizeof(_temporaryScriptState));

	if (!_emc->load(filename, &_temporaryScriptData, &_opcodesTemporary))
		error("Couldn't load temporary script '%s'", filename);

	_emc->init(&_temporaryScriptState, &_temporaryScriptData);
	_emc->start(&_temporaryScriptState, 0);

	_newShapeFlag = -1;

	if (_newShapeFiledata && newShapes) {
		resetNewShapes(_newShapeCount, _newShapeFiledata);
		_newShapeFiledata = 0;
		_newShapeCount = 0;
	}

	while (_emc->isValid(&_temporaryScriptState))
		_emc->run(&_temporaryScriptState);

	uint8 *fileData = 0;

	if (newShapes)
		_newShapeFiledata = _res->fileData(_newShapeFilename, 0);

	fileData = _newShapeFiledata;

	if (!fileData) {
		_emc->unload(&_temporaryScriptData);
		return;
	}

	if (newShapes)
		_newShapeCount = initNewShapes(fileData);

	processNewShapes(allowSkip, resetChar);

	if (shapeUnload) {
		resetNewShapes(_newShapeCount, fileData);
		_newShapeCount = 0;
		_newShapeFiledata = 0;
	}

	_emc->unload(&_temporaryScriptData);
}

void KyraEngine_v3::eelScript() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::eelScript()");
	if (_chatText)
		return;
	_screen->hideMouse();

	if (_inventoryState)
		hideInventory();
	removeHandItem();

	objectChat((const char*)getTableEntry(_cCodeFile, 35), 0, 204, 35);
	objectChat((const char*)getTableEntry(_cCodeFile, 40), 0, 204, 40);

	setGameFlag(0xD1);

	snd_playSoundEffect(0x2A, 0xC8);

	setGameFlag(0x171);

	switch (_malcolmShapes-1) {
	case 0:
		runTemporaryScript("EELS01.EMC", 0, 0, 1, 1);
		break;

	case 1:
		runTemporaryScript("EELS02.EMC", 0, 0, 1, 1);
		break;

	case 2:
		runTemporaryScript("EELS03.EMC", 0, 0, 1, 1);
		break;
	
	case 3:
		runTemporaryScript("EELS04.EMC", 0, 0, 1, 1);
		break;

	default:
		resetGameFlag(0x171);
		runTemporaryScript("EELS00.EMC", 0, 0, 1, 1);
		break;
	}

	changeChapter(2, 29, 0, 4);
	_screen->showMouse();
}

int KyraEngine_v3::initNewShapes(uint8 *filedata) {
	debugC(9, kDebugLevelAnimator, "KyraEngine_v3::initNewShapes(%p)", (const void*)filedata);
	const int lastEntry = MIN(_newShapeLastEntry, 41);
	for (int i = 0; i < lastEntry; ++i)
		_gameShapes[9+i] = _screen->getPtrToShape(filedata, i);
	return lastEntry;
}

void KyraEngine_v3::processNewShapes(int allowSkip, int resetChar) {
	debugC(9, kDebugLevelAnimator, "KyraEngine_v3::processNewShapes(%d, %d)", allowSkip, resetChar);
	setCharacterAnimDim(_newShapeWidth, _newShapeHeight);

	_emc->init(&_temporaryScriptState, &_temporaryScriptData);
	_emc->start(&_temporaryScriptState, 1);

	resetSkipFlag();

	while (_emc->isValid(&_temporaryScriptState)) {
		_temporaryScriptExecBit = false;
		while (_emc->isValid(&_temporaryScriptState) && !_temporaryScriptExecBit)
			_emc->run(&_temporaryScriptState);

		if (_newShapeAnimFrame < 0)
			continue;

		_mainCharacter.animFrame = _newShapeAnimFrame + 9;
		updateCharacterAnim(0);
		if (_chatText)
			updateWithText();
		else
			update();

		uint32 delayEnd = _system->getMillis() + _newShapeDelay * _tickLength;

		while ((!skipFlag() || !allowSkip) && _system->getMillis() < delayEnd) {
			if (_chatText)
				updateWithText();
			else
				update();

			delay(10);
		}

		if (skipFlag())
			resetSkipFlag();
	}

	if (resetChar) {
		if (_newShapeFlag >= 0) {
			_mainCharacter.animFrame = _newShapeFlag + 9;
			updateCharacterAnim(0);
			if (_chatText)
				updateWithText();
			else
				update();
		}

		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
		updateCharacterAnim(0);
	}

	_newShapeFlag = -1;
	resetCharacterAnimDim();
}

void KyraEngine_v3::resetNewShapes(int count, uint8 *filedata) {
	debugC(9, kDebugLevelAnimator, "KyraEngine_v3::resetNewShapes(%d, %p)", count, (const void*)filedata);
	for (int i = 0; i < count; ++i)
		_gameShapes[9+i] = 0;
	delete [] filedata;
	setNextIdleAnimTimer();
}

} // end of namespace Kyra

