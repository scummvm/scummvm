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


void IgorEngine::part28ExecuteAction(int action) {
	switch (action) {
	case 101: // leave
		_currentPart = 230;
		break;
	case 102: // read plaque
		igorSay(201, 1, 1226);
		break;
	case 103: // look at door
		igorSay(202, 1, 1227);
		break;
	case 104: // look at file
		igorSay(203, 1, 1228);
		break;
	case 105: // look at Caroline
		igorSay(204, 1, 1229);
		break;
	case 106: // look at message board
		if (_objectsState[kExamCompletedStateIndex] == 0) {
			igorSay({ { 205, 1, 1230 }, { 206, 1, 1231 } });
		} else {
			igorSay(234, 1, 1253);
		}
		break;
	case 107: // look at plant
		igorSay(237, 1, 1256);
		break;
	case 108:
		part28TalkToCaroline();
		break;
	case 109:
		part28TakeFolder();
		break;
	case 110: // open door
		igorSay({ { 228, 1, 1248 }, { 229, 1, 1249 } });
		break;
	case 111: // give folder to caroline
		if (_objectsState[kFolderSwappedStateIndex] == 0) {
			igorSay(226, 1, 1246);
		} else {
			igorSay(227, 1, 1247);
		}
		break;
	case 112: // use caroline
		igorSay(235, 1, 1254);
		break;
	case 113:
		igorSay(236, 1, 1255);
		break;
	default:
		error("part28ExecuteAction unhandled action %d", action);
		break;
	}
}

// Caroline's idle animation is only driven by the room loop and while Igor speaks;
// her own talking loops never run it, otherwise it fights the talking frames.
void IgorEngine::part28TalkToCaroline() {
	const UpdateRoomBackgroundProc updateRoomBackground = _updateRoomBackground;
	if (_objectsState[kExamCompletedStateIndex] == 1) {
		igorSayAndWait(218, 1, 1241);
		_updateRoomBackground = 0;
		cutsceneSayStart(182, 81, 63, 17, 17, { { 211, 1, 1235 }, { 212, 1, 1236 }, { 213, 2, 1237 }, { 215, 1, 1238 }, { 216, 1, 1239 } });
		_updateDialogue = &IgorEngine::part28UpdateCarolineDialogue;
		waitForEndOfCutsceneDialogue(182, 81, 63, 17, 17);
		ADD_DIALOGUE_TEXT(217, 1, 1240);
		SET_DIALOGUE_TEXT(1, 5);
		startCutsceneDialogue(182, 81, 63, 17, 17);
		waitForEndOfCutsceneDialogue(182, 81, 63, 17, 17);
		_updateDialogue = 0;
		_updateRoomBackground = updateRoomBackground;
		--_walkDataLastIndex;
		buildWalkPath(213, 131, 160, 140);
		_walkDataCurrentIndex = 1;
		_walkData[_walkDataLastIndex].frameNum = 0;
		_walkData[_walkDataLastIndex].posNum = 3;
		_gameState.igorMoving = true;
		waitForIgorMove();
		igorSay(222, 2, 1244);
		return;
	}
	if (_objectsState[kCarolineFolderStateIndex] < 2) {
		igorSayAndWait(207, 1, 1232);
	} else {
		igorSayAndWait(218, 1, 1241);
	}
	_updateRoomBackground = 0;
	if (_objectsState[kCarolineFolderStateIndex] < 2) {
		cutsceneSayWithCallback(182, 81, 63, 17, 17, 208, 1, 1233, &IgorEngine::part28UpdateCarolineDialogue);
	} else {
		cutsceneSayWithCallback(182, 81, 63, 17, 17, { { 219, 2, 1242 }, { 221, 1, 1243 } }, &IgorEngine::part28UpdateCarolineDialogue);
	}
	_updateRoomBackground = updateRoomBackground;
}

void IgorEngine::part28TakeFolder() {
	for (int i = 1; i <= 7; ++i) {
		const int offset = 25067;
		drawAnimRect(offset, i * 1085 + 0x5EC9, 31, 35);
		if (i < 7) {
			waitForTimer(45);
		}
	}
	addObjectToInventory(kInventoryObjectFolder, 57);
	_objectsState[kCarolineFolderStateIndex] = 1;
	part28ApplyObjectState(255);
	igorSay(230, 1, 1250);
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
}

void IgorEngine::part28UpdateCarolineDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part28DrawCarolineTalkingFrame(7);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part28DrawCarolineTalkingFrame(getRandomNumber(4) + 7);
		break;
	}
}

void IgorEngine::part28UpdateBackground() {
	if (compareGameTick(61)) {
		if (_gameState.unk10 > 2) {
			part28DrawCarolineTalkingFrame(_gameState.unk10);
			if (_gameState.unk10 < 6) {
				++_gameState.unk10;
			} else {
				_gameState.unk10 = 1;
			}
		} else {
			part28DrawCarolineTalkingFrame(_gameState.unk10);
			if (_gameState.unk10 == 1) {
				_gameState.unk10 = 2;
			} else {
				_gameState.unk10 = 1;
			}
			if (getRandomNumber(7) == 0) {
				_gameState.unk10 = 3;
			}
		}
	}
}

void IgorEngine::part28ApplyObjectState(int num) {
	WRITE_LE_UINT16(_roomActionsTable + 0x56, 42133);
	if (num == 2 || num == 255) {
		if (_objectsState[kCarolineFolderStateIndex] == 0) {
			part28DrawFolder(1);
		} else {
			part28DrawFolder(0);
			_roomObjectAreasTable[7].object = 0;
		}
	}
}

void IgorEngine::part28DrawCaroline() {
	const int offset = 27374;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer, 30, 30, 44);
}

void IgorEngine::part28EnterFromRight() {
	_walkData[0].setPos(319, 142, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPathSimple(319, 142, 289, 142);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part28DrawCarolineTalkingFrame(int frame) {
	_roomCursorOn = false;
	drawAnimRect(27374, (frame - 1) * 1320, 30, 44, true, kBlendBehindIgorAndText);
	if (_gameState.dialogueTextRunning) {
		memcpy(_screenTextLayer + 23040, _screenLayer1 + _dialogueDirtyRectY, _dialogueDirtyRectSize);
	}
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::part28EnterAndCheckGrades() {
	setupDefaultPalette();
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	drawVerbsPanel();
	drawInventory(_inventoryInfo[72], 0);
	part28ApplyObjectState(255);
	fadeIn(768);
	_walkData[0].setPos(319, 142, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPath(319, 142, 214, 130);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkData[_walkDataLastIndex].posNum = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	igorSayAndWait(209, 2, 1234);
	waitForTimer(255);
	igorSayAndWait(231, 1, 1251);
	--_walkDataLastIndex;
	buildWalkPath(214, 130, 214, 140);
	_walkDataCurrentIndex = 1;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
	igorSayAndWait({ { 224, 2, 1245 }, { 232, 2, 1252 } });
}

void IgorEngine::part28DrawFolder(int frame) {
	const int offset = 33387;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + frame * 135 + 0x61F8, 15, 15, 9);
}

void IgorEngine::part28() {
	if (isCDVersion())
		playMusic(2);
	_gameState.enableLight = 2;
	loadRoomData(PAL_CollegeCorridorCaroline, IMG_CollegeCorridorCaroline, BOX_CollegeCorridorCaroline, MSK_CollegeCorridorCaroline, TXT_CollegeCorridorCaroline);
	static const int anm[] = { FRM_CollegeCorridorCaroline1, FRM_CollegeCorridorCaroline2, FRM_CollegeCorridorCaroline3, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorCaroline);
	_roomDataOffsets = PART_28_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(15, 0, 319, 143);
	partMainSetExecuteAction(1, &IgorEngine::part28ExecuteAction);
	_updateRoomBackground = &IgorEngine::part28UpdateBackground;
	part28DrawCaroline();
	part28ApplyObjectState(255);
	memcpy(_screenVGA, _screenLayer1, 46080);

	if (!restoreRoomAfterLoad()) {
		_gameState.unk10 = 1;
		_currentAction.verb = kVerbWalk;
		if (_currentPart == 280) {
			fadeIn(768);
			part28EnterFromRight();
		} else if (_currentPart == 281) {
			part28EnterAndCheckGrades();
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
