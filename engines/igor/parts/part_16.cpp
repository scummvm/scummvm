/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "igor/igor.h"

namespace Igor {


static int VAR_CURRENT_TALKING_ACTOR;

void IgorEngine::part16ExecuteAction(int action) {
	debugC(9, kDebugGame, "part16ExecuteAction %d", action);
	switch (action) {
	case 101: // pick up things
		part16TakeFlask();
		break;
	case 102: // look at things
		igorSay(208, 2, 730);
		break;
	case 103: // exit
		_currentPart = 261;
		break;
	default:
		error("part16ExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part16TakeFlask() {
	if (_objectsState[kFlaskTakenStateIndex] == 1) {
		igorSay(207, 1, 729);
		return;
	}
	igorSayAndWait({ { 201, 1, 726 }, { 202, 2, 727 } });
	for (int i = 1; i <= 2; ++i) {
		int offset = 20332;
		drawAnimRect(offset, (i - 1) * 1127, 23, 49);
		waitForTimer(120);
	}
	addObjectToInventory(kInventoryObjectFlask, 55);
	_objectsState[kFlaskTakenStateIndex] = 1;
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
	igorSay(204, 3, 728);
}

void IgorEngine::part16UpdateMargaretHarrisonDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part16DrawTalkingFrame((VAR_CURRENT_TALKING_ACTOR == 0) ? 34 : 48);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part16DrawTalkingFrame((VAR_CURRENT_TALKING_ACTOR == 0) ? getRandomNumber(4) + 30 : getRandomNumber(4) + 44);
		break;
	}
}

void IgorEngine::part16UpdateMargaretDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part16DrawTalkingFrame(49);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part16DrawTalkingFrame(getRandomNumber(3) + 49);
		break;
	}
}

void IgorEngine::part16ApplyObjectState(int num) {
}

void IgorEngine::part16EnterScene() {
	WalkData *wd = &_walkData[0];
	wd->setPos(0, 135, 2, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 30;
	wd->scaleWidth = 50;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 50;
	_walkDataLastIndex = 0;
	buildWalkPath(0, 135, 90, 135);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part16PhotoCutscene() {
	fillScreenRows(144, 56, 0);
	igorSayAndWait({ { 210, 1, 731 }, { 211, 1, 732 }, { 212, 1, 733 }, { 213, 2, 734 } });
	--_walkDataLastIndex;
	buildWalkPath(90, 135, 200, 143);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_walkDataCurrentIndex = 0;
	for (int i = 15; i >= 0; --i) {
		if (i == 15) {
			_walkCurrentFrame = 0;
		}
		_walkData[0].setPos(200, 143, 3, _walkCurrentFrame);
		WalkData::setNextFrame(3, _walkCurrentFrame);
		_walkData[0].clipSkipX = 1;
		_walkData[0].clipWidth = 30;
		_walkData[0].scaleWidth = i * 3 + 5;
		_walkData[0].xPosChanged = 1;
		_walkData[0].dxPos = 0;
		_walkData[0].yPosChanged = 1;
		_walkData[0].dyPos = 3;
		_walkData[0].scaleHeight = 50;
		moveIgor(_walkData[0].posNum, _walkData[0].frameNum);
		waitForTimer(15);
	}
	fadeOut(768);
	if (isCDVersion())
		playMusic(8);
	_currentPart = 331;
	part33();
	memset(_currentPalette, 0, 768);
	setPaletteRange(208, 255);
	do {
		partMargaret();
	} while (_objectsState[kMargaretSceneStateIndex] != 9);
	setupDefaultPalette();
	setVerbPanelColors();
	setInventoryPanelColors();
	part16LoadRoom();
	part16LoadAnimation();
	copyLayerToScreen(_screenLayer1Surface);
	part16ApplyObjectState(255);
	fadeIn(768);
	waitForTimer(255);
	for (int i = 1; i <= 29; ++i) {
		decodeAnimFrame(_animFramesBuffer + 0x8CE + READ_LE_UINT16(_animFramesBuffer + 0x6F36 + i * 2) - 1, _screenVGA, true);
		waitForTimer(25);
	}
	VAR_CURRENT_TALKING_ACTOR = 0;
	cutsceneSayWithCallback(175, 78, 0, 58, 40, { { 215, 1, 735 }, { 216, 1, 736 }, { 217, 1, 737 } }, &IgorEngine::part16UpdateMargaretHarrisonDialogue);
	for (int i = 35; i <= 41; ++i) {
		decodeAnimFrame(_animFramesBuffer + 0x8CE + READ_LE_UINT16(_animFramesBuffer + 0x6F36 + i * 2) - 1, _screenVGA, true);
		waitForTimer(35);
	}
	cutsceneSayWithCallback(88, 69, 63, 32, 0, 218, 2, 738, &IgorEngine::part16UpdateMargaretDialogue);
	for (int i = 42; i <= 43; ++i) {
		decodeAnimFrame(_animFramesBuffer + 0x8CE + READ_LE_UINT16(_animFramesBuffer + 0x6F36 + i * 2) - 1, _screenVGA, true);
		waitForTimer(35);
	}
	VAR_CURRENT_TALKING_ACTOR = 1;
	cutsceneSayWithCallback(175, 78, 0, 58, 40, { { 220, 1, 739 }, { 221, 1, 740 } }, &IgorEngine::part16UpdateMargaretHarrisonDialogue);
	cutsceneSayWithCallback(88, 69, 63, 32, 0, 222, 2, 741, &IgorEngine::part16UpdateMargaretDialogue);
	playSound(26, 1);
	for (int i = 53; i <= 61; ++i) {
		decodeAnimFrame(_animFramesBuffer + 0x8CE + READ_LE_UINT16(_animFramesBuffer + 0x6F36 + i * 2) - 1, _screenVGA, true);
		waitForTimer(35);
	}
	playSound(25, 1);
	for (int i = 1; i <= 20; ++i) {
		int frame = getRandomNumber(3) + 61;
		decodeAnimFrame(_animFramesBuffer + 0x8CE + READ_LE_UINT16(_animFramesBuffer + 0x6F36 + frame * 2) - 1, _screenVGA, true);
		waitForTimer(15);
	}
	playSound(27, 1);
	part16DisplayPhoto();
	for (int i = 1; i <= 3; ++i) {
		waitForTimer(255);
	}
	part16ApplyObjectState(255);
	// Data-only inventory update: the panel is redrawn later together with the verbs
	++_inventoryInfo[kInventoryCount];
	_inventoryInfo[_inventoryInfo[kInventoryCount] - 1] = kInventoryObjectPhoto;
	_inventoryInfo[54] = _inventoryInfo[kInventoryCount];
	_inventoryInfo[kInventoryFirstVisible] = _inventoryOffsetTable[(_inventoryInfo[kInventoryCount] - 1) / 7];
	_objectsState[kPhotoCutsceneSeenStateIndex] = 1;
	_objectsState[kMargaretStateIndex] = kMargaretStatePhotoTaken;
	if (!isCDVersion() && _game.version != kIdSpaFloppy)
		playMusic(3, true);
	_currentPart = 261;
}

void IgorEngine::part16DisplayPhoto() {
	memset(&_currentPalette[3], 63, 621);
	setPaletteRange(1, 207);
	loadData(IMG_PhotoHarrisonMargaret, _screenVGA);
	loadData(PAL_PhotoHarrisonMargaret, _paletteBuffer);
	for (int m = 1; m <= 63; ++m) {
		for (int i = 3; i <= 207 * 3 + 2; ++i) {
			if (_paletteBuffer[i] <= m && _currentPalette[i] > _paletteBuffer[i]) {
				--_currentPalette[i];
			}
		}
		setPaletteRange(1, 207);
		waitForTimer();
	}
}

void IgorEngine::part16DrawTalkingFrame(int frame) {
	const uint8 *src = _animFramesBuffer + 0x8CE + READ_LE_UINT16(_animFramesBuffer + 0x6F36 + frame * 2) - 1;
	decodeAnimFrame(src, _screenVGA, true);
}

void IgorEngine::part16() {
	if (isCDVersion())
		playMusic(2);
	_gameState.enableLight = 1;
	part16LoadRoom();
	part16LoadAnimation();
	loadActionData(DAT_Laboratory);
	_roomDataOffsets = PART_16_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 248, 143);
	partMainSetExecuteAction(1, &IgorEngine::part16ExecuteAction);
	part16ApplyObjectState(255);
	if (!restoreRoomAfterLoad()) {
		copyLayerToScreen(_screenLayer1Surface);
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		part16EnterScene();
		if (_objectsState[kMargaretStateIndex] == kMargaretStateReadNote && _inventoryInfo[71] != 0 && _objectsState[kHarrisonNoteShownStateIndex] == 1) {
			part16PhotoCutscene();
		}
		enterPartLoop();
	}
	while (_currentPart == 160 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

void IgorEngine::part16LoadRoom() {
	loadRoomData(PAL_Laboratory, IMG_Laboratory, BOX_Laboratory, MSK_Laboratory, TXT_Laboratory);
}

void IgorEngine::part16LoadAnimation() {
	static const int anm[] = { FRM_Laboratory1, FRM_Laboratory2, FRM_Laboratory3, 0 };
	loadAnimData(anm);
}

} // End of namespace Igor
