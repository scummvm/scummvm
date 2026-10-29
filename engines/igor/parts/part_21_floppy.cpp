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

// Part 21 (college corridor with Margaret) as in the Spanish floppy version.

const int kMargaretTalkX = 81;
const int kMargaretTalkY = 76;
const int kMargaretTalkR = 38;
const int kMargaretTalkG = 48;
const int kMargaretTalkB = 59;

const int kMargaretBodyOffset = 25990;
const int kMargaretAnimOffset = 25029;
const int kMargaretHeadOffset = 25675;
const int kMargaretNoteOffset = 25670;
const int kPhysicsDoorOffset = 18409;
const int kPhysicsDoorAnimOffset = 18724;
const int kWritingOffset = 24751;

const int kMargaretBodyFrame = 0x4ECD;
const int kMargaretFrames = 0x20B3;
const int kMargaretHeadFrames = 0x7DEF;
const int kMargaretNoteFrames = 0x51A9;
const int kMargaretNoteHeadFrames = 0x6FA5;
const int kPhysicsDoorOpenFrame = 0x6CC;
const int kPhysicsDoorAnimFrames = 0x59A;
const int kWritingFrames = 0x3B1D;

// Frames of the last part of the sandwiches action
const uint8 kWritingEndFrames[5] = { 0, 1, 2, 6, 1 };
const int kPhysicsDoorCell = 114 * 320 + 181;

bool IN_ACTION_111;

void IgorEngine::part21FloppyExecuteAction(int action) {
	switch (action) {
	case 101:
		part21FloppyLeaveLeft();
		break;
	case 102:
		part21FloppyTalkToMargaret();
		break;
	case 103: // Look at margaret
		igorSay(201, 1, 0);
		break;
	case 104: // take sandwiches
		igorSay(202, 1, 0);
		break;
	case 105: // look at sandwiches
		igorSay(203, 1, 0);
		break;
	case 106: // look at physics door
		igorSay(204, 1, 0);
		break;
	case 107:
		part21FloppyOpenPhysicsDoor();
		break;
	case 108:
		part21FloppyClosePhysicsDoor();
		break;
	case 109: // look at plaque
		igorSay(205, 1, 0);
		break;
	case 110:
		part21FloppyLeaveRight();
		break;
	case 111:
		part21FloppyUseNoteWithSandwiches();
		break;
	case 112: // give anything to Margaret
		igorSay(222, 1, 0);
		break;
	case 113:
		part21FloppyEnterPhysicsClass();
		break;
	default:
		error("part21FloppyExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part21FloppyLeaveLeft() {
	if (_objectsState[kMargaretStateIndex] == 1) {
		const int offset = 26880;
		copyArea(_screenVGA, offset, 320, _screenLayer1 + offset, 320, 20, 51);
		part21FloppyMargaretTakesNote();
	}
	_currentPart = 303;
}

void IgorEngine::part21FloppyLeaveRight() {
	if (_objectsState[kMargaretStateIndex] == 1) {
		const int offset = 27180;
		copyArea(_screenVGA, offset, 320, _screenLayer1 + offset, 320, 20, 51);
		part21FloppyMargaretTakesNote();
	}
	_currentPart = 270;
}

void IgorEngine::part21FloppyTalkToMargaret() {
	part21FloppyDrawMargaretFrameComposited(1);
	part21FloppyDrawMargaretHead(1);
	igorSayAndWait(226, 1, 0);
	cutsceneSayStart(kMargaretTalkX, kMargaretTalkY, kMargaretTalkR, kMargaretTalkG, kMargaretTalkB, 227, 1, 0);
	_updateDialogue = &IgorEngine::part21FloppyUpdateMargaretDialogue;
	// The cutscene dialogue wait does not run Margaret's idle animation
	_updateRoomBackground = 0;
	waitForEndOfCutsceneDialogue(kMargaretTalkX, kMargaretTalkY, kMargaretTalkR, kMargaretTalkG, kMargaretTalkB);
	_updateRoomBackground = &IgorEngine::part21FloppyUpdateBackground;
	_updateDialogue = 0;
	part21FloppyHandleMargaretDialogue();
	part21FloppyApplyObjectState(255);
}

void IgorEngine::part21FloppyOpenPhysicsDoor() {
	if (_objectsState[kPhysicsClassDoorStateIndex] == 1) {
		partMainExecuteAction(11);
		return;
	}
	for (int i = 2; i <= 3; ++i) {
		drawAnimRect(kPhysicsDoorAnimOffset, kPhysicsDoorAnimFrames + i * 0x7FE, 33, 62);
		if (i == 3) {
			playSound(13, 1);
		}
		waitForTimer(127);
	}
	_objectsState[kPhysicsClassDoorStateIndex] = 1;
	part21FloppyApplyObjectState(1);
}

void IgorEngine::part21FloppyClosePhysicsDoor() {
	if (_objectsState[kPhysicsClassDoorStateIndex] == 0) {
		partMainExecuteAction(14);
		return;
	}
	for (int i = 2; i >= 1; --i) {
		drawAnimRect(kPhysicsDoorAnimOffset, kPhysicsDoorAnimFrames + i * 0x7FE, 33, 62);
		if (i == 2) {
			playSound(14, 1);
		}
		waitForTimer(127);
	}
	_objectsState[kPhysicsClassDoorStateIndex] = 0;
	part21FloppyApplyObjectState(1);
}

void IgorEngine::part21FloppyUseNoteWithSandwiches() {
	if (_inventoryInfo[65] == 0) {
		igorSay({ { 223, 2, 0 }, { 225, 1, 0 } });
		return;
	}
	IN_ACTION_111 = true;
	igorSay({ { 208, 1, 0 }, { 209, 1, 0 } });
	// Igor's talking head is not animated here
	waitForEndOfIgorDialogue(false);
	IN_ACTION_111 = false;
	int k = 1;
	do {
		if (compareGameTick(61) && k <= 4) {
			drawAnimRect(kWritingOffset, kWritingFrames + kWritingEndFrames[k] * 720, 24, 30);
			++k;
		}
		part21FloppyUpdateBackground();
		waitForTimer();
	} while (k != 5);
	removeInventoryEntry(56);
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	_objectsState[kMargaretStateIndex] = 1;
	part21FloppyApplyObjectState(255);
	igorSay(210, 2, 0);
}

void IgorEngine::part21FloppyEnterPhysicsClass() {
	if (_objectsState[kPhysicsClassDoorStateIndex] == 0) {
		return;
	}
	_roomObjectAreasTable[_screenLayer2[kPhysicsDoorCell]].area = 2;
	--_walkDataLastIndex;
	buildWalkPath(176, 119, 181, 114);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	if (_objectsState[kMargaretStateIndex] == 1) {
		drawAnimRect(kPhysicsDoorOffset, 0, 29, 60);
		part21FloppyMargaretTakesNote();
	}
	_currentPart = 370;
}

void IgorEngine::part21FloppyUpdateMargaretDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part21FloppyDrawMargaretHead(1);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part21FloppyDrawMargaretHead(getRandomNumber(4) + 1);
		break;
	case kUpdateDialogueAnimStanding:
		part21FloppyDrawMargaretHead(1);
		break;
	}
}

void IgorEngine::part21FloppyUpdateMargaretNoteDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part21FloppyDrawMargaretNoteHead(1);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part21FloppyDrawMargaretNoteHead(getRandomNumber(3) + 2);
		break;
	}
}

void IgorEngine::part21FloppyHandleMargaretDialogue() {
	loadDialogueData(DLG_CollegeCorridorMargaret);
	_updateDialogue = &IgorEngine::part21FloppyUpdateMargaretDialogue;
	handleDialogue(kMargaretTalkX, kMargaretTalkY, kMargaretTalkR, kMargaretTalkG, kMargaretTalkB);
	_updateDialogue = 0;
}

void IgorEngine::part21FloppyApplyObjectState(int num) {
	if (num == 1 || num == 255) {
		// physics class door
		if (_objectsState[kPhysicsClassDoorStateIndex] == 0) {
			copyArea(_screenLayer1, kPhysicsDoorOffset, 320, _animFramesBuffer, 29, 29, 60);
			_roomActionsTable[32] = 6;
		} else {
			copyArea(_screenLayer1, kPhysicsDoorOffset, 320, _animFramesBuffer + kPhysicsDoorOpenFrame, 29, 29, 60);
			_roomActionsTable[32] = 7;
		}
	}
	if (num == 2 || num == 255) {
		// is margaret present?
		if (_objectsState[kMargaretStateIndex] <= 1) {
			copyArea(_screenLayer1, kMargaretBodyOffset, 320, _animFramesBuffer + kMargaretBodyFrame, 48, 48, 40);
		}
		if (_objectsState[kMargaretStateIndex] == 2 || _objectsState[kMargaretStateIndex] == 4) {
			_roomObjectAreasTable[5].object = 0;
			_roomObjectAreasTable[6].object = 0;
		}
	}
}

void IgorEngine::part21FloppyEnterFromLeft() {
	_walkData[0].setPos(0, 134, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 15;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	buildWalkPath(0, 134, 30, 134);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part21FloppyEnterFromRight() {
	_walkData[0].setPos(319, 134, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	buildWalkPath(319, 134, 289, 134);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part21FloppyEnterFromPhysicsClass() {
	_walkData[0].setPos(181, 114, 3, 0);
	_walkData[0].setDefaultScale();
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[kPhysicsDoorCell]].area = 2;
	buildWalkPath(181, 114, 181, 125);
	_roomObjectAreasTable[_screenLayer2[kPhysicsDoorCell]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part21FloppyDrawMargaretFrameComposited(int frame) {
	_roomCursorOn = false;
	drawAnimRect(kMargaretAnimOffset, kMargaretFrames + frame * 0x4DF, 29, 29, true, kBlendBehindIgorAndText);
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::part21FloppyDrawMargaretHead(int frame) {
	drawAnimRect(kMargaretHeadOffset, kMargaretHeadFrames + frame * 0xB6, 14, 13);
}

void IgorEngine::part21FloppyDrawMargaretNoteHead(int frame) {
	drawAnimRect(kMargaretNoteOffset, kMargaretNoteHeadFrames + frame * 0x280, 32, 20);
}

void IgorEngine::part21FloppyMargaretTakesNote() {
	part21FloppyDrawMargaretFrameComposited(1);
	fillScreenRows(144, 56, 0);
	drawAnimRect(kMargaretBodyOffset, kMargaretBodyFrame, 48, 40);
	waitForTimer(121);
	for (int i = 1; i <= 6; ++i) {
		drawAnimRect(kMargaretNoteOffset, kMargaretNoteFrames + i * 0x4A4, 44, 27);
		if (i < 6) {
			waitForTimer(121);
		}
	}
	cutsceneSayStart(kMargaretTalkX, kMargaretTalkY, kMargaretTalkR, kMargaretTalkG, kMargaretTalkB,
			{ { 212, 1, 0 }, { 213, 1, 0 }, { 214, 1, 0 }, { 215, 2, 0 }, { 217, 2, 0 } });
	_updateDialogue = &IgorEngine::part21FloppyUpdateMargaretNoteDialogue;
	_updateRoomBackground = 0;
	waitForEndOfCutsceneDialogue(kMargaretTalkX, kMargaretTalkY, kMargaretTalkR, kMargaretTalkG, kMargaretTalkB);
	_updateRoomBackground = &IgorEngine::part21FloppyUpdateBackground;
	_updateDialogue = 0;
	cutsceneSayStart(kMargaretTalkX, kMargaretTalkY, kMargaretTalkR, kMargaretTalkG, kMargaretTalkB,
			{ { 219, 1, 0 }, { 220, 1, 0 }, { 221, 1, 0 } });
	_updateDialogue = &IgorEngine::part21FloppyUpdateMargaretNoteDialogue;
	_updateRoomBackground = 0;
	waitForEndOfCutsceneDialogue(kMargaretTalkX, kMargaretTalkY, kMargaretTalkR, kMargaretTalkG, kMargaretTalkB);
	_updateRoomBackground = &IgorEngine::part21FloppyUpdateBackground;
	_updateDialogue = 0;
	_objectsState[kMargaretStateIndex] = 2;
	drawVerbsPanel();
	_currentAction.verb = kVerbWalk;
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	part21FloppyApplyObjectState(255);
	_objectsState[kMargaretSceneStateIndex] = 1;
}

void IgorEngine::part21FloppyUpdateBackground() {
	if (IN_ACTION_111 && compareGameTick(61)) {
		const int i = getRandomNumber(2) + 3;
		drawAnimRect(kWritingOffset, kWritingFrames + i * 720, 24, 30);
	}
	if (compareGameTick(61) && _objectsState[kMargaretStateIndex] <= 1) {
		if (_gameState.unk10 > 2) {
			part21FloppyDrawMargaretFrameComposited(_gameState.unk10);
			++_gameState.unk10;
			if (_gameState.unk10 == 6) {
				++_gameState.counter[4];
				if (_gameState.counter[4] < 2) {
					_gameState.unk10 = 4;
				} else {
					_gameState.counter[4] = 0;
					_gameState.unk10 = 1;
				}
			}
		} else {
			part21FloppyDrawMargaretFrameComposited(_gameState.unk10);
			++_gameState.unk10;
			if (_gameState.unk10 == 3 && getRandomNumber(5) != 0) {
				_gameState.unk10 = 1;
			}
		}
	}
}

void IgorEngine::part21Floppy() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_CollegeCorridorMargaret, IMG_CollegeCorridorMargaret, BOX_CollegeCorridorMargaret, MSK_CollegeCorridorMargaret, TXT_CollegeCorridorMargaret);
	static const int anm[] = {
		FRM_CollegeCorridorMargaretFloppy1, FRM_CollegeCorridorMargaretFloppy2,
		FRM_CollegeCorridorMargaretFloppy3, FRM_CollegeCorridorMargaretFloppy4,
		FRM_CollegeCorridorMargaretFloppy5, FRM_CollegeCorridorMargaretFloppy6,
		FRM_CollegeCorridorMargaretFloppy7, FRM_CollegeCorridorMargaretFloppy8, 0
	};
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorMargaret);
	_roomDataOffsets = PART_21_FLOPPY_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 248, 143);
	partMainSetExecuteAction(1, &IgorEngine::part21FloppyExecuteAction);
	_updateRoomBackground = &IgorEngine::part21FloppyUpdateBackground;
	part21FloppyApplyObjectState(255);
	copyLayerToScreen(_screenLayer1Surface);
	setVerbPanelColors();
	setInventoryPanelColors();
	if (!restoreRoomAfterLoad()) {
		_gameState.unk10 = 1;
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 210) {
			part21FloppyEnterFromLeft();
		} else if (_currentPart == 211) {
			part21FloppyEnterFromRight();
		} else if (_currentPart == 212) {
			part21FloppyEnterFromPhysicsClass();
		}
	}
	enterPartLoop();
	if (takeResumeDialogue(1)) { // loaded while talking to margaret
		part21FloppyHandleMargaretDialogue();
		part21FloppyApplyObjectState(255);
		finishResumedDialogue();
	}
	while (_currentPart >= 210 && _currentPart <= 212 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
