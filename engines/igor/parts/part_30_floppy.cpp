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

// Part 30 in the Spanish floppy version was redone for the English release and
// all subsequent versions.

const int kLauraTalkX = 219;
const int kLauraTalkY = 69;
const int kLauraTalkR = 63;
const int kLauraTalkG = 40;
const int kLauraTalkB = 31;

const int kLauraFramesTable = 0x9FB0;

const int kUpstairsFirstCell = 67 * 320 + 141;
const int kUpstairsSecondCell = 67 * 320 + 187;
const int kDownstairsCell = 107 * 320 + 189;

void IgorEngine::part30FloppyExecuteAction(int action) {
	switch (action) {
	case 101:
		_currentPart = 231;
		break;
	case 102:
		part30FloppyGoUpstairs();
		break;
	case 103: // look at upward stairs
		igorSay(201, 1, 0);
		break;
	case 104:
		part30FloppyGoDownstairs();
		break;
	case 105: // look at downward stairs
		igorSay(202, 1, 0);
		break;
	case 106:
		_currentPart = 210;
		break;
	default:
		error("part30FloppyExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part30FloppyGoUpstairs() {
	_roomObjectAreasTable[_screenLayer2[kUpstairsFirstCell]].area = 1;
	--_walkDataLastIndex;
	buildWalkPathSimple(124, 120, 141, 67);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_roomObjectAreasTable[_screenLayer2[kUpstairsSecondCell]].area = 1;
	--_walkDataLastIndex;
	buildWalkPathSimple(141, 67, 187, 67);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 312;
}

void IgorEngine::part30FloppyGoDownstairs() {
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[kDownstairsCell]].area = 1;
	buildWalkPathSimple(189, 120, 189, 107);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_walkDataCurrentIndex = 0;
	for (int i = 9; i >= 0; --i) {
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		_walkData[0].setPos(189, 107, 1, _walkCurrentFrame);
		WalkData::setNextFrame(1, _walkCurrentFrame);
		_walkData[0].setDefaultScale();
		_walkData[0].clipSkipX = 1;
		_walkData[0].clipWidth = 30;
		_walkData[0].dyPos = 3;
		_walkData[0].scaleWidth = i * 3 + 23;
		moveIgor(_walkData[0].posNum, _walkData[0].frameNum);
		waitForTimer(15);
	}
	// The room it leads to exists only in the Spanish floppy version and is not ported yet
	_currentPart = 151;
}

void IgorEngine::part30FloppyDrawCutsceneFrame(int frame, bool keepTextColors) {
	const uint8 *src = _animFramesBuffer + READ_LE_UINT16(_animFramesBuffer + kLauraFramesTable + frame * 2) - 1;
	decodeAnimFrame(src, _screenVGA, keepTextColors);
}

void IgorEngine::part30FloppyUpdateLauraDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part30FloppyDrawCutsceneFrame(33, true);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part30FloppyDrawCutsceneFrame(getRandomNumber(5) + 33, true);
		break;
	case kUpdateDialogueAnimStanding:
		part30FloppyDrawCutsceneFrame(33, true);
		break;
	}
}

void IgorEngine::part30FloppyHandleLauraDialogue() {
	loadDialogueData(DLG_CollegeStairsFirstFloor);
	_updateDialogue = &IgorEngine::part30FloppyUpdateLauraDialogue;
	handleDialogue(kLauraTalkX, kLauraTalkY, kLauraTalkR, kLauraTalkG, kLauraTalkB, false);
	_updateDialogue = 0;
}

void IgorEngine::part30FloppyApplyObjectState(int num) {
}

void IgorEngine::part30FloppyWalkInFromLeft() {
	_walkData[0].setPos(0, 134, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPathSimple(0, 134, 30, 134);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part30FloppyWalkInFromUpstairs() {
	_roomObjectAreasTable[_screenLayer2[kUpstairsSecondCell]].area = 1;
	_walkData[0].setPos(187, 67, 4, 0);
	_walkData[0].setDefaultScale();
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[kUpstairsFirstCell]].area = 1;
	buildWalkPathSimple(187, 67, 141, 67);
	_roomObjectAreasTable[_screenLayer2[kUpstairsSecondCell]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	--_walkDataLastIndex;
	buildWalkPathSimple(141, 67, 124, 125);
	_roomObjectAreasTable[_screenLayer2[kUpstairsFirstCell]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part30FloppyWalkInFromOutside() {
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	_walkCurrentPos = 3;
	for (int i = 0; i <= 9; ++i) {
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		_walkData[0].setPos(189, 107, 3, _walkCurrentFrame);
		WalkData::setNextFrame(3, _walkCurrentFrame);
		_walkData[0].setDefaultScale();
		_walkData[0].dyPos = 3;
		_walkData[0].scaleWidth = i * 3 + 23;
		moveIgor(_walkData[0].posNum, _walkData[0].frameNum);
		waitForTimer(15);
	}
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;
	_roomObjectAreasTable[_screenLayer2[kDownstairsCell]].area = 1;
	--_walkDataLastIndex;
	buildWalkPathSimple(189, 107, 189, 125);
	_roomObjectAreasTable[_screenLayer2[kDownstairsCell]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part30FloppyWalkInFromRight() {
	_walkData[0].setPos(319, 134, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPathSimple(319, 134, 289, 134);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part30FloppyLauraAndIgorBumpIntoEachOther() {
	// they both walk in
	for (int i = 1; i <= 17; ++i) {
		part30FloppyDrawCutsceneFrame(i, true);
		waitForTimer(26);
	}
	static const int frames[] = { 18, 25, 19, 26, 20, 27, 21, 28, 22, 29, 23, 30, 24, 31 };
	for (int i = 0; i < ARRAYSIZE(frames); ++i) {
		part30FloppyDrawCutsceneFrame(frames[i], true);
		waitForTimer(14);
	}
}

void IgorEngine::part30FloppyIgorCrossesArms() {
	part30FloppyDrawCutsceneFrame(32, true);
	for (int i = 1; i <= 32; ++i) {
		part30FloppyDrawCutsceneFrame(i, false);
	}
}

void IgorEngine::part30FloppyLauraLeaves() {
	for (int i = 39; i <= 52; ++i) {
		part30FloppyDrawCutsceneFrame(i, true);
		if (i >= 43 && i <= 50) {
			waitForTimer(41);
		} else {
			waitForTimer(26);
		}
	}
}

void IgorEngine::part30FloppyLauraCutscene() {
	// The cutscene does not select any music, the current music keeps playing
	const bool resuming = _dialogueResuming;
	fillScreenRows(144, 56, 0);
	if (!resuming) {
		fadeIn(768);
		part30FloppyLauraAndIgorBumpIntoEachOther();
	}
	_walkData[0].setPos(190, 119, 2, 0);
	_walkData[0].setDefaultScale();
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;
	if (!resuming) {
		igorSayAndWait({ { 203, 1, 0 }, { 204, 1, 0 }, { 205, 1, 0 } });
		_updateDialogue = &IgorEngine::part30FloppyUpdateLauraDialogue;
		cutsceneSay(kLauraTalkX, kLauraTalkY, kLauraTalkR, kLauraTalkG, kLauraTalkB, 206, 1, 0);
		_updateDialogue = 0;
	}
	if (!resuming) // when resuming igor has to be drawn before the screen fades in
		part30FloppyIgorCrossesArms();
	part30FloppyHandleLauraDialogue();
	if (_gameStateLoaded || _eventQuitGame || _currentPart == kInvalidPart)
		return;
	igorSayAndWait(207, 1, 0);
	_updateDialogue = &IgorEngine::part30FloppyUpdateLauraDialogue;
	cutsceneSay(kLauraTalkX, kLauraTalkY, kLauraTalkR, kLauraTalkG, kLauraTalkB, { { 208, 1, 0 }, { 209, 1, 0 } });
	_updateDialogue = 0;
	part30FloppyLauraLeaves();
	_walkData[0].setPos(179, 120, 3, 0);
	_walkData[0].setDefaultScale();
	_walkDataLastIndex = 1;
	igorSay(210, 1, 0);
	fillScreenRows(144, 56, 0);
	drawVerbsPanel();
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	_currentAction.verb = kVerbWalk;
	_objectsState[kLauraMetAtCollegeStateIndex] = 1;
	part30FloppyApplyObjectState(255);
}

void IgorEngine::part30Floppy() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_CollegeStairsFirstFloor, IMG_CollegeStairsFirstFloor, BOX_CollegeStairsFirstFloor, MSK_CollegeStairsFirstFloor, TXT_CollegeStairsFirstFloor);
	static const int anm[] = { FRM_CollegeStairsFirstFloor1, FRM_CollegeStairsFirstFloor2, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeStairsFirstFloor);
	_roomDataOffsets = PART_30_FLOPPY_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	partMainSetExecuteAction(1, &IgorEngine::part30FloppyExecuteAction);
	part30FloppyApplyObjectState(255);
	copyLayerToScreen(_screenLayer1Surface);
	setVerbPanelColors();
	setInventoryPanelColors();
	const bool loaded = restoreRoomAfterLoad(true, false);
	if (loaded) {
		// a game saved at laura's dialogue shows the scene of the dialogue from the first frame
		if (_resumeDialogueId == 1)
			part30FloppyIgorCrossesArms();
		fadeIn(768);
	} else {
		_currentAction.verb = kVerbWalk;
		if (_objectsState[kLauraMetAtCollegeStateIndex] == 0) {
			part30FloppyLauraCutscene();
		} else {
			fadeIn(768);
			if (_currentPart == 300) {
				part30FloppyWalkInFromLeft();
			} else if (_currentPart == 301) {
				part30FloppyWalkInFromUpstairs();
			} else if (_currentPart == 302) {
				part30FloppyWalkInFromOutside();
			} else if (_currentPart == 303) {
				part30FloppyWalkInFromRight();
			}
		}
	}
	enterPartLoop();
	if (takeResumeDialogue(1)) { // loaded while talking to laura
		part30FloppyLauraCutscene();
		finishResumedDialogue();
	}
	while (_currentPart >= 300 && _currentPart <= 303 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
