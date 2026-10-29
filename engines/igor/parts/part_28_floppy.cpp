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

static const int kCarolineDoorStateIndex = 86;

static const int kCarolineTalkX = 259;
static const int kCarolineTalkY = 76;
static const int kCarolineTalkR = 55;
static const int kCarolineTalkG = 39;
static const int kCarolineTalkB = 51;

static const int kCarolineOffset = 25211;
static const int kCarolineFrameSize = 16 * 42;
static const int kCarolineRestFrameOffset = 0xD20;

static const int kDoorPictureOffset = 19030;
static const int kClosedDoorPicture = 15587;
static const int kOpenDoorPicture = kClosedDoorPicture + 28 * 58;
static const int kFolderPictureOffset = 31260;
static const int kFolderPicture = 15431;
static const int kTakeFolderOffset = 24202;
static const int kTakeFolderFrames = 0x1451;
static const int kTakeFolderFrameSize = 0x5EF;

void IgorEngine::part28FloppyExecuteAction(int action) {
	switch (action) {
	case 101: // read plaque
		igorSay(201, 1, 0);
		break;
	case 102: // look at door
		igorSay(202, 1, 0);
		break;
	case 103: // open door
		if (_objectsState[kCarolineDoorStateIndex] == 1) {
			partMainExecuteAction(11);
		} else {
			igorSay({ { 228, 1, 0 }, { 229, 1, 0 } });
		}
		break;
	case 104: // close door
		// The door never gets opened, so it is never open here
		if (_objectsState[kCarolineDoorStateIndex] == 0) {
			partMainExecuteAction(14);
		}
		break;
	case 105:
		part28FloppyTakeFolder();
		break;
	case 106: // look at the folder
		igorSay(203, 1, 0);
		break;
	case 107:
		part28FloppyTalkToCaroline();
		break;
	case 108: // look at Caroline
		igorSay(204, 1, 0);
		break;
	case 109: // look at the message board
		if (_objectsState[kExamCompletedStateIndex] == 0) {
			igorSay({ { 205, 1, 0 }, { 206, 1, 0 } });
		} else {
			igorSay(234, 1, 0);
		}
		break;
	case 110: // exit right
		_currentPart = 230;
		break;
	case 111: // give the folder to Caroline
		if (_objectsState[kFolderSwappedStateIndex] == 0) {
			igorSay(226, 1, 0);
		} else {
			igorSay(227, 1, 0);
		}
		break;
	default:
		error("part28FloppyExecuteAction unhandled action %d", action);
		break;
	}
}

// Caroline's idle animation is only driven by the room loop and while Igor speaks;
// her own talking loops never run it.
void IgorEngine::part28FloppyTalkToCaroline() {
	const UpdateRoomBackgroundProc updateRoomBackground = _updateRoomBackground;
	if (_objectsState[kExamCompletedStateIndex] == 1) {
		igorSayAndWait(218, 1, 0);
		_updateRoomBackground = 0;
		cutsceneSayWithCallback(kCarolineTalkX, kCarolineTalkY, kCarolineTalkR, kCarolineTalkG, kCarolineTalkB,
				{ { 211, 1, 0 }, { 212, 1, 0 }, { 213, 2, 0 }, { 215, 1, 0 }, { 216, 1, 0 } },
				&IgorEngine::part28FloppyUpdateCarolineDialogue);
		cutsceneSayWithCallback(kCarolineTalkX, kCarolineTalkY, kCarolineTalkR, kCarolineTalkG, kCarolineTalkB,
				217, 1, 0, &IgorEngine::part28FloppyUpdateCarolineDialogue);
		_updateRoomBackground = updateRoomBackground;
		--_walkDataLastIndex;
		buildWalkPath(288, 122, 160, 140);
		_walkDataCurrentIndex = 1;
		_walkData[_walkDataLastIndex].frameNum = 0;
		_walkData[_walkDataLastIndex].posNum = 3;
		_gameState.igorMoving = true;
		waitForIgorMove();
		igorSay(222, 2, 0);
		return;
	}
	if (_objectsState[kCarolineFolderStateIndex] < 2) {
		igorSayAndWait(207, 1, 0);
	} else {
		igorSayAndWait(218, 1, 0);
	}
	_updateRoomBackground = 0;
	if (_objectsState[kCarolineFolderStateIndex] < 2) {
		cutsceneSayWithCallback(kCarolineTalkX, kCarolineTalkY, kCarolineTalkR, kCarolineTalkG, kCarolineTalkB,
				208, 1, 0, &IgorEngine::part28FloppyUpdateCarolineDialogue);
	} else {
		cutsceneSayWithCallback(kCarolineTalkX, kCarolineTalkY, kCarolineTalkR, kCarolineTalkG, kCarolineTalkB,
				{ { 219, 2, 0 }, { 221, 1, 0 } }, &IgorEngine::part28FloppyUpdateCarolineDialogue);
	}
	_updateRoomBackground = updateRoomBackground;
}

void IgorEngine::part28FloppyTakeFolder() {
	igorSay(230, 1, 0);
	for (int i = 1; i <= 2; ++i) {
		drawAnimRect(kTakeFolderOffset, kTakeFolderFrames + i * kTakeFolderFrameSize, 31, 49);
		if (i < 2) {
			waitForTimer(91);
		}
	}

	++_inventoryInfo[kInventoryCount];
	_inventoryInfo[_inventoryInfo[kInventoryCount] - 1] = kInventoryObjectFolder;
	_inventoryInfo[57] = _inventoryInfo[kInventoryCount];
	if (_inventoryInfo[kInventoryCount] > 8) {
		_inventoryInfo[kInventoryFirstVisible] = _inventoryInfo[kInventoryCount] - 7;
	}
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	_objectsState[kCarolineFolderStateIndex] = 1;
	part28FloppyApplyObjectState(255);
}

void IgorEngine::part28FloppyUpdateCarolineDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part28FloppyDrawCarolineFrame(6);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part28FloppyDrawCarolineFrame(getRandomNumber(4) + 6);
		break;
	}
}

void IgorEngine::part28FloppyUpdateBackground() {
	if (compareGameTick(61)) {
		if (_gameState.unk10 > 3) {
			part28FloppyDrawCarolineFrame(_gameState.unk10);
			if (_gameState.unk10 == 4) {
				++_gameState.unk10;
			} else {
				_gameState.unk10 = 4;
			}
			if (getRandomNumber(7) == 0) {
				_gameState.unk10 = 1;
			}
		} else {
			part28FloppyDrawCarolineFrame(_gameState.unk10);
			++_gameState.unk10;
		}
	}
}

void IgorEngine::part28FloppyApplyObjectState(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[kCarolineDoorStateIndex] == 0) {
			copyArea(_screenLayer1, kDoorPictureOffset, 320, _animFramesBuffer + kClosedDoorPicture, 28, 28, 58);
			_roomActionsTable[30] = 6;
		} else {
			copyArea(_screenLayer1, kDoorPictureOffset, 320, _animFramesBuffer + kOpenDoorPicture, 28, 28, 58);
			_roomActionsTable[30] = 7;
		}
	}
	if (num == 2 || num == 255) {
		if (_objectsState[kCarolineFolderStateIndex] == 0) {
			copyArea(_screenLayer1, kFolderPictureOffset, 320, _animFramesBuffer + kFolderPicture, 13, 13, 12);
		} else {
			_roomObjectAreasTable[6].object = 0;
		}
	}
}

void IgorEngine::part28FloppyDrawCaroline() {
	copyArea(_screenLayer1, kCarolineOffset, 320, _animFramesBuffer + kCarolineRestFrameOffset, 16, 16, 42);
}

void IgorEngine::part28FloppyDrawCarolineFrame(int frame) {
	_roomCursorOn = false;
	drawAnimRect(kCarolineOffset, (frame - 1) * kCarolineFrameSize, 16, 42, true, kBlendBehindIgorAndText);
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::part28FloppyEnterFromRight() {
	_walkData[0].setPos(319, 134, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	buildWalkPath(319, 134, 289, 134);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part28FloppyEnterAndCheckGrades() {
	setupDefaultPalette();
	setVerbPanelColors();
	setInventoryPanelColors();
	drawVerbsPanel();
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	part28FloppyApplyObjectState(255);
	fadeIn(768);
	_walkData[0].setPos(319, 134, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPath(319, 134, 288, 122);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkData[_walkDataLastIndex].posNum = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	igorSayAndWait(209, 2, 0);
	waitForTimer(255);
	igorSayAndWait({ { 231, 1, 0 }, { 224, 2, 0 }, { 232, 2, 0 } });
}

void IgorEngine::part28Floppy() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_CollegeCorridorCaroline, IMG_CollegeCorridorCaroline, BOX_CollegeCorridorCaroline, MSK_CollegeCorridorCaroline, TXT_CollegeCorridorCaroline);
	memcpy(_paletteBuffer + 192 * 3, _igorPalette, 48);
	static const int anm[] = {
		FRM_CollegeCorridorCarolineFloppy1, FRM_CollegeCorridorCarolineFloppy2, FRM_CollegeCorridorCarolineFloppy3,
		FRM_CollegeCorridorCarolineFloppy4, FRM_CollegeCorridorCarolineFloppy5, 0
	};
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorCaroline);
	_roomDataOffsets = PART_28_FLOPPY_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(23, 0, 319, 143);
	partMainSetExecuteAction(1, &IgorEngine::part28FloppyExecuteAction);
	_updateRoomBackground = &IgorEngine::part28FloppyUpdateBackground;
	part28FloppyDrawCaroline();
	part28FloppyApplyObjectState(255);
	copyLayerToScreen(_screenLayer1Surface);
	if (!restoreRoomAfterLoad()) {
		_gameState.unk10 = 4;
		_currentAction.verb = kVerbWalk;
		if (_currentPart == 280) {
			fadeIn(768);
			part28FloppyEnterFromRight();
		} else if (_currentPart == 281) {
			part28FloppyEnterAndCheckGrades();
		}
	}
	enterPartLoop();
	while ((_currentPart == 280 || _currentPart == 281) && !_gameStateLoaded) {
		runPartLoop();
	}
	if (!_gameStateLoaded && _objectsState[kCarolineFolderStateIndex] == 1) {
		_objectsState[kCarolineFolderStateIndex] = 2;
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
