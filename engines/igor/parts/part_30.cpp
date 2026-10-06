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


void IgorEngine::PART_30_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		_currentPart = 231;
		break;
	case 102:
		PART_30_ACTION_102_goUpstairs();
		break;
	case 103:
		ADD_DIALOGUE_TEXT(201, 1, 450);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 104:
		PART_30_ACTION_104_goDownstairs();
		break;
	case 105:
		ADD_DIALOGUE_TEXT(202, 1, 451);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 106:
		_currentPart = 210;
		break;
	default:
		error("PART_30_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_30_ACTION_102_goUpstairs() {
	_roomObjectAreasTable[_screenLayer2[24108]].area = 2;
	--_walkDataLastIndex;
	buildWalkPath(99, 119, 108, 75);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_roomObjectAreasTable[_screenLayer2[24170]].area = 0;
	--_walkDataLastIndex;
	buildWalkPath(108, 75, 170, 75);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 312;
}

void IgorEngine::PART_30_ACTION_104_goDownstairs() {
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[36318]].area = 2;
	buildWalkPath(158, 119, 158, 113);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_walkDataCurrentIndex = 0;
	for (int i = 9; i >= 0; --i) {
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		_walkData[0].setPos(158, 113, 1, _walkCurrentFrame);
		WalkData::setNextFrame(1, _walkCurrentFrame);
		_walkData[0].setDefaultScale();
		_walkData[0].clipSkipX = 1;
		_walkData[0].clipWidth = 30;
		_walkData[0].dyPos = 3;
		_walkData[0].scaleWidth = i * 3 + 23;
		moveIgor(_walkData[0].posNum, _walkData[0].frameNum);
		waitForTimer(15);
	}
	_currentPart = (_objectsState[111] == 0) ? 171 : 771;
}

void IgorEngine::PART_30_UPDATE_DIALOGUE_LAURA(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_30_HELPER_9_setLauraFrame(42);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_30_HELPER_9_setLauraFrame(getRandomNumber(8) + 42);
		break;
	case kUpdateDialogueAnimStanding:
		PART_30_HELPER_9_setLauraFrame(33);
		break;
	}
}

void IgorEngine::PART_30_HANDLE_DIALOGUE_LAURA() {
	loadDialogueData(DLG_CollegeStairsFirstFloor);
	_updateDialogue = &IgorEngine::PART_30_UPDATE_DIALOGUE_LAURA;
	handleDialogue(201, 85, 63, 0, 38, false);
	_updateDialogue = 0;
}

void IgorEngine::PART_30_HELPER_1(int num) {
}

void IgorEngine::PART_30_HELPER_2_walkInFromLeft() {
	_walkData[0].setPos(0, 138, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPath(0, 138, 30, 138);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_30_HELPER_3_walkInFromUpstairs() {
	_roomObjectAreasTable[_screenLayer2[24170]].area = 2;
	_walkData[0].setPos(170, 75, 4, 0);
	_walkData[0].setDefaultScale();
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[24108]].area = 2;
	buildWalkPath(170, 75, 108, 75);
	_roomObjectAreasTable[_screenLayer2[24170]].area = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	--_walkDataLastIndex;
	buildWalkPath(108, 75, 99, 119);
	_roomObjectAreasTable[_screenLayer2[24108]].area = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	--_walkDataLastIndex;
	buildWalkPath(99, 119, 128, 123);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_30_HELPER_2_walkInFromOutside() {
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	_walkCurrentPos = 3;
	for (int i = 0; i <= 9; ++i) {
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		_walkData[0].setPos(158, 113, 3, _walkCurrentFrame);
		WalkData::setNextFrame(3, _walkCurrentFrame);
		_walkData[0].setDefaultScale();
		_walkData[0].scaleWidth = i * 3 + 23;
		moveIgor(_walkData[0].posNum, _walkData[0].frameNum);
		waitForTimer(15);
	}
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;
	_roomObjectAreasTable[_screenLayer2[36318]].area = 2;
	--_walkDataLastIndex;
	buildWalkPath(158, 113, 128, 123);
	_roomObjectAreasTable[_screenLayer2[36318]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_30_HELPER_5_walkInFromRight() {
	_walkData[0].setPos(319, 138, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPath(319, 138, 289, 138);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_30_HELPER_8_LauraCutscene() {
	playMusic(3);
	memset(_screenVGA + 46080, 0, 17920);
	fadeIn(768);
    lauraAndIgorBumpIntoEachOther();
	_walkData[0].setPos(172, 134, 2, 0);
	_walkData[0].setDefaultScale();
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;
	_updateDialogue = &IgorEngine::PART_30_UPDATE_DIALOGUE_LAURA;
	ADD_DIALOGUE_TEXT(203, 1, 452);
	ADD_DIALOGUE_TEXT(204, 1, 453);
	ADD_DIALOGUE_TEXT(205, 1, 454);
	SET_DIALOGUE_TEXT(1, 3);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	ADD_DIALOGUE_TEXT(206, 1, 455);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(201, 85, 63, 0, 38);
	waitForEndOfCutsceneDialogue(201, 85, 63, 0, 38);
    // igor crosses arms
	igorCrossesArms();
	// real dialogue

	PART_30_HANDLE_DIALOGUE_LAURA();
    // "Nunca me habian descrito tan bien"
	ADD_DIALOGUE_TEXT(207, 1, 456);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
    // at least you're funny
	ADD_DIALOGUE_TEXT(208, 1, 457);
	ADD_DIALOGUE_TEXT(209, 1, 458);
	SET_DIALOGUE_TEXT(1, 2);
	startCutsceneDialogue(201, 85, 63, 0, 38);
	waitForEndOfCutsceneDialogue(201, 85, 63, 0, 38);
	_updateDialogue = 0;
    // Laura leaves
	lauraLeaves();
	_walkData[0].setPos(164, 135, 3, 0);
	_walkData[0].setDefaultScale();
	_walkDataLastIndex = 1;
	drawVerbsPanel();
	drawInventory(_inventoryInfo[72], 0);
    // Que caracter
	ADD_DIALOGUE_TEXT(210, 1, 459);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	memset(_screenVGA + 46080, 0, 17920);
	drawVerbsPanel();
	drawInventory(_inventoryInfo[72], 0);
	_currentAction.verb = kVerbWalk;
	_objectsState[73] = 1;
	PART_30_HELPER_1(255);
}

void IgorEngine::lauraAndIgorBumpIntoEachOther() {
// they both walk in
	for (int i = 1; i <= 40; ++i) {
		const uint8 *src = _animFramesBuffer + READ_LE_UINT16(_animFramesBuffer + 0xBE94 + i * 2) - 1;
		decodeAnimFrame(src, _screenVGA, true);
		if (i < 34) {
			waitForTimer(25);
		} else {
			waitForTimer(15);
		}
	}
}

void IgorEngine::igorCrossesArms() {
    decodeAnimFrame(_animFramesBuffer + READ_LE_UINT16(_animFramesBuffer + 0xBEE6) - 1, _screenVGA, true);
	for (int i = 1; i <= 41; ++i) {
		const uint8 *src = _animFramesBuffer + READ_LE_UINT16(_animFramesBuffer + 0xBE94 + i * 2) - 1;
		decodeAnimFrame(src, _screenVGA, true);
	}
}

void IgorEngine::lauraLeaves() {
	for (int i = 51; i <= 70; ++i) {
		const uint8 *src = _animFramesBuffer + READ_LE_UINT16(_animFramesBuffer + 0xBE94 + i * 2) - 1;
		decodeAnimFrame(src, _screenVGA, true);
		if (i < 68) {
			waitForTimer(25);
		}
	}
}

void IgorEngine::PART_30_HELPER_9_setLauraFrame(int frame) {
	const uint8 *src = _animFramesBuffer + READ_LE_UINT16(_animFramesBuffer + 0xBE94 + frame * 2) - 1;
	decodeAnimFrame(src, _screenVGA, true);
}

void IgorEngine::PART_30() {
	_gameState.enableLight = 2;
	loadRoomData(PAL_CollegeStairsFirstFloor, IMG_CollegeStairsFirstFloor, BOX_CollegeStairsFirstFloor, MSK_CollegeStairsFirstFloor, TXT_CollegeStairsFirstFloor);
	static const int anm[] = { FRM_CollegeStairsFirstFloor1, FRM_CollegeStairsFirstFloor2, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeStairsFirstFloor);
	_roomDataOffsets = PART_30_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_30_EXEC_ACTION);
	PART_30_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		if (_objectsState[73] == 0) {
			PART_30_HELPER_8_LauraCutscene();
		} else {
			if (_currentPart != 302) {
				fadeIn(768);
			}
			if (_currentPart == 300) { // enter from outside
				PART_30_HELPER_2_walkInFromLeft();
			} else if (_currentPart == 301) {
				PART_30_HELPER_3_walkInFromUpstairs();
			} else if (_currentPart == 302) {
				playMusic(3);
				fadeIn(768);
				PART_30_HELPER_2_walkInFromOutside();
			} else if (_currentPart == 303) {
				PART_30_HELPER_5_walkInFromRight();
			}
		}
	}
	enterPartLoop();
	while (_currentPart >= 300 && _currentPart <= 303 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
