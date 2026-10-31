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

// Part 15 (the porter Tobias Jennings) as in the Spanish floppy version.

const int kTobiasTalkX = 50;
const int kTobiasTalkY = 85;
const int kTobiasTalkR = 53;
const int kTobiasTalkG = 31;
const int kTobiasTalkB = 23;

const int kStreetDoorCell = 97 * 320 + 309;

const int kHandOverOffset = 29808;

void IgorEngine::part15FloppyExecuteAction(int action) {
	switch (action) {
	case 101: // talk to Tobias
		igorSay(207, 1, 0);
		break;
	case 102: // look at Tobias
		igorSay({ { 201, 1, 0 }, { 202, 2, 0 } });
		break;
	case 103: // look at the lodge door
		igorSay(204, 2, 0);
		break;
	case 104: // look at the crest
		igorSay(206, 1, 0);
		break;
	case 105:
		part15FloppyLeaveToStreet();
		break;
	case 106: // look at the street door
		igorSay(208, 1, 0);
		break;
	case 107:
		part15FloppyGoUpstairs();
		break;
	case 108:
		part15FloppyGiveProjectToTobias();
		break;
	case 109:
		part15FloppyGiveMoneyToTobias();
		break;
	default:
		error("part15FloppyExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part15FloppyLeaveToStreet() {
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[kStreetDoorCell]].area = 1;
	buildWalkPathSimple(309, 105, 309, 97);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	for (int i = 9; i >= 0; --i) {
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		_walkData[0].setPos(309, 97, 1, _walkCurrentFrame);
		WalkData::setNextFrame(1, _walkCurrentFrame);
		_walkData[0].clipSkipX = 1;
		_walkData[0].clipWidth = 15;
		_walkData[0].scaleWidth = i + 16;
		_walkData[0].xPosChanged = 1;
		_walkData[0].dxPos = 0;
		_walkData[0].yPosChanged = 1;
		_walkData[0].dyPos = 3;
		_walkData[0].scaleHeight = 25;
		moveIgor(_walkData[0].posNum, _walkData[0].frameNum);
		waitForTimer(15);
	}
	_currentPart = (_objectsState[kExamCompletedStateIndex] == 0) ? 171 : 771;
}

void IgorEngine::part15FloppyGoUpstairs() {
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	for (int i = 9; i >= 0; --i) {
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		_walkData[0].setPos(230, 143, 3, _walkCurrentFrame);
		WalkData::setNextFrame(3, _walkCurrentFrame);
		_walkData[0].clipSkipX = 1;
		_walkData[0].clipWidth = 15;
		_walkData[0].scaleWidth = i * 2 + 7;
		_walkData[0].xPosChanged = 1;
		_walkData[0].dxPos = 0;
		_walkData[0].yPosChanged = 1;
		_walkData[0].dyPos = 3;
		_walkData[0].scaleHeight = 25;
		moveIgor(_walkData[0].posNum, _walkData[0].frameNum);
		waitForTimer(15);
	}
	_currentPart = 302;
}

void IgorEngine::part15FloppyHandOver() {
	for (int i = 1; i >= 0; --i) {
		drawAnimRect(kHandOverOffset, i * 0x168, 15, 24);
		waitForTimer(127);
	}
}

void IgorEngine::part15FloppyGiveProjectToTobias() {
	igorSayAndWait({ { 209, 1, 0 }, { 210, 1, 0 } });
	cutsceneSay(kTobiasTalkX, kTobiasTalkY, kTobiasTalkR, kTobiasTalkG, kTobiasTalkB, 212, 1, 0);
	igorSayAndWait(213, 1, 0);
	if (_objectsState[kPersonalFileChangedStateIndex] == 0) {
		cutsceneSay(kTobiasTalkX, kTobiasTalkY, kTobiasTalkR, kTobiasTalkG, kTobiasTalkB,
				{ { 214, 1, 0 }, { 215, 2, 0 }, { 217, 1, 0 } });
		return;
	}
	cutsceneSay(kTobiasTalkX, kTobiasTalkY, kTobiasTalkR, kTobiasTalkG, kTobiasTalkB,
			{ { 214, 1, 0 }, { 218, 2, 0 }, { 220, 1, 0 } });
	part15FloppyHandOver();
	removeInventoryEntry(70);
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	_objectsState[kProjectHandedToTobiasStateIndex] = 1;
	if (_objectsState[kMoneyHandedToTobiasStateIndex] == 0) {
		cutsceneSay(kTobiasTalkX, kTobiasTalkY, kTobiasTalkR, kTobiasTalkG, kTobiasTalkB, 221, 2, 0);
		return;
	}
	cutsceneSay(kTobiasTalkX, kTobiasTalkY, kTobiasTalkR, kTobiasTalkG, kTobiasTalkB, 226, 2, 0);
	igorSayAndWait(228, 1, 0);
	cutsceneSay(kTobiasTalkX, kTobiasTalkY, kTobiasTalkR, kTobiasTalkG, kTobiasTalkB, 229, 1, 0);
	igorSayAndWait(230, 1, 0);
	_currentPart = 780;
}

void IgorEngine::part15FloppyGiveMoneyToTobias() {
	igorSayAndWait({ { 209, 1, 0 }, { 211, 1, 0 } });
	cutsceneSay(kTobiasTalkX, kTobiasTalkY, kTobiasTalkR, kTobiasTalkG, kTobiasTalkB, 212, 1, 0);
	igorSayAndWait(213, 1, 0);
	if (_objectsState[kPersonalFileChangedStateIndex] == 0) {
		cutsceneSay(kTobiasTalkX, kTobiasTalkY, kTobiasTalkR, kTobiasTalkG, kTobiasTalkB,
				{ { 214, 1, 0 }, { 215, 2, 0 }, { 217, 1, 0 } });
		return;
	}
	cutsceneSay(kTobiasTalkX, kTobiasTalkY, kTobiasTalkR, kTobiasTalkG, kTobiasTalkB,
			{ { 214, 1, 0 }, { 218, 2, 0 }, { 223, 1, 0 } });
	part15FloppyHandOver();
	removeInventoryEntry(60);
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	_objectsState[kMoneyHandedToTobiasStateIndex] = 1;
	if (_objectsState[kProjectHandedToTobiasStateIndex] == 0) {
		cutsceneSay(kTobiasTalkX, kTobiasTalkY, kTobiasTalkR, kTobiasTalkG, kTobiasTalkB, 224, 2, 0);
		return;
	}
	cutsceneSay(kTobiasTalkX, kTobiasTalkY, kTobiasTalkR, kTobiasTalkG, kTobiasTalkB, 226, 2, 0);
	igorSayAndWait(228, 1, 0);
	cutsceneSay(kTobiasTalkX, kTobiasTalkY, kTobiasTalkR, kTobiasTalkG, kTobiasTalkB, 229, 1, 0);
	igorSayAndWait(230, 1, 0);
	_currentPart = 780;
}

void IgorEngine::part15FloppyDrawFigure(int figure, int frame) {
	_roomCursorOn = false;
	switch (figure) {
	case 1:
		drawAnimRect(24510, 0x2D0 + frame * 0x380, 32, 28, true, kBlendBehindIgorAndText);
		break;
	case 2:
		drawAnimRect(23124, 0x10D0 + frame * 0x201, 19, 27, true, kBlendBehindIgorAndText);
		break;
	case 3:
		drawAnimRect(23799, 0x18D4 + frame * 0x396, 34, 27, true, kBlendBehindIgorAndText);
		break;
	}
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::part15FloppyUpdateBackground() {
	if (compareGameTick(61) && _gameState.unk10 >= 1 && _gameState.unk10 <= 3) {
		part15FloppyDrawFigure(_gameState.unk10, getRandomNumber(3));
	}
}

void IgorEngine::part15FloppyApplyObjectState(int num) {
}

void IgorEngine::part15FloppyEnterFromStreet() {
	playMusic(3, true);
	_walkData[0].setPos(309, 97, 3, 0);
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 15;
	_walkData[0].scaleWidth = 25;
	_walkData[0].xPosChanged = 1;
	_walkData[0].dxPos = 0;
	_walkData[0].yPosChanged = 1;
	_walkData[0].dyPos = 0;
	_walkData[0].scaleHeight = 25;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	_roomObjectAreasTable[_screenLayer2[kStreetDoorCell]].area = 1;
	buildWalkPathSimple(309, 97, 309, 105);
	_roomObjectAreasTable[_screenLayer2[kStreetDoorCell]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part15FloppyEnterFromStairs() {
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	for (int i = 0; i <= 9; ++i) {
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		_walkData[0].setPos(230, 143, 1, _walkCurrentFrame);
		WalkData::setNextFrame(1, _walkCurrentFrame);
		_walkData[0].clipSkipX = 1;
		_walkData[0].clipWidth = 15;
		_walkData[0].scaleWidth = i * 2 + 7;
		_walkData[0].xPosChanged = 1;
		_walkData[0].dxPos = 0;
		_walkData[0].yPosChanged = 1;
		_walkData[0].dyPos = 3;
		_walkData[0].scaleHeight = 25;
		moveIgor(_walkData[0].posNum, _walkData[0].frameNum);
		waitForTimer(15);
	}
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;
}

void IgorEngine::part15Floppy() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_TobiasOffice, IMG_TobiasOffice, BOX_TobiasOffice, MSK_TobiasOffice, TXT_TobiasOffice);
	static const int anm[] = { ANM_TobiasOffice1, AOF_TobiasOffice1, ANM_TobiasOffice2, AOF_TobiasOffice2, 0 };
	loadAnimData(anm);
	loadActionData(DAT_TobiasOffice);
	_roomDataOffsets = PART_15_FLOPPY_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	partMainSetExecuteAction(1, &IgorEngine::part15FloppyExecuteAction);
	_updateRoomBackground = &IgorEngine::part15FloppyUpdateBackground;
	part15FloppyApplyObjectState(255);
	copyLayerToScreen(_screenLayer1Surface);
	// One of three figures in the hall, or none, drawn in its first frame
	_gameState.unk10 = (getRandomNumber(9) + 2) / 3;
	if (_gameState.unk10 != 0) {
		part15FloppyDrawFigure(_gameState.unk10, 0);
	}
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 150) {
			part15FloppyEnterFromStreet();
		} else {
			part15FloppyEnterFromStairs();
		}
	}
	enterPartLoop();
	while ((_currentPart == 150 || _currentPart == 151) && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
