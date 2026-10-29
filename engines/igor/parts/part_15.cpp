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


static const uint8 PART_15_ANIM_DATA_1[8] = { 5, 6, 0, 3, 3, 0, 0, 1 };

static const uint8 PART_15_ANIM_DATA_2[8] = { 0, 1, 2, 3, 4, 3, 5, 6 };

static const uint8 PART_15_ANIM_DATA_3[12] = { 0, 0, 1, 1, 1, 2, 1, 1, 1, 2, 1, 3 };

static const uint8 PART_15_ANIM_DATA_4[8] = { 0, 7, 8, 3, 4, 3, 5, 6 };

static const uint8 PART_15_ANIM_DATA_5[12] = { 0, 0, 1, 1, 1, 2, 1, 1, 1, 2, 1, 3 };

void IgorEngine::part15ExecuteAction(int action) {
	switch (action) {
	case 101:
		part15LeaveRoom();
		break;
	case 102: // look at door
		igorSay(204, 1, 324);
		break;
	case 103: // take clock
		igorSay(205, 2, 325);
		break;
	case 104: // look at clock
		igorSay(207, 1, 326);
		break;
	case 105: // open clock
		igorSay(205, 2, 325);
		break;
	case 106: // close clock
		igorSay(205, 2, 325);
		break;
	case 107:
		part15TalkToTobias();
		break;
	case 108: // Take tobias
		igorSay(208, 1, 327);
		break;
	case 109: // look at Tobias
		igorSay({ { 201, 1, 322 }, { 202, 2, 323 } });
		break;
	case 110: // use Tobias
		igorSay(234, 1, 345);
		break;
	case 111: // take keys
		igorSay({ { 235, 1, 346 }, { 236, 1, 347 } });
		break;
	case 112: // look at keys
		igorSay(237, 2, 348);
		break;
	case 113:
		igorSay(239, 1, 349);
		break;
	case 114:
		igorSay(240, 1, 350);
		break;
	case 115:
		part15GiveProjectToTobias();
		break;
	case 116:
		part15GiveMoneyToTobias();
		break;
	default:
		error("part15ExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part15LeaveRoom() {
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[34560]].area = 1;
	buildWalkPathSimple(34, 108, 0, 108);
	_roomObjectAreasTable[_screenLayer2[34560]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 271;
}

void IgorEngine::part15TalkToTobias() {
	part15WaitForCuckooClock();
	part15DrawIgorAndTobiasScene(6);
	part15HandleTobiasDialogue();
}

void IgorEngine::part15GiveProjectToTobias() {
	part15WaitForCuckooClock();
	part15DrawIgorAndTobiasScene(6);
	igorSayAndWait({ { 209, 1, 328 }, { 210, 1, 329 } });
	cutsceneSayWithCallback(133, 67, 0, 63, 19, 212, 1, 331, &IgorEngine::part15UpdateTobiasDialogue);
	igorSayAndWait(213, 1, 332);
	cutsceneSayWithCallback(133, 67, 0, 63, 19, 214, 1, 333, &IgorEngine::part15UpdateTobiasDialogue);
	for (int i = 1; i <= 11; ++i) {
		part15DrawTobiasIdleFrame(PART_15_ANIM_DATA_3[i]);
		waitForTimer(60);
	}
	part15DrawIgorAndTobiasScene(6);
	if (_objectsState[kPersonalFileChangedStateIndex] == 0) {
		ADD_DIALOGUE_TEXT(215, 2, 334);
		ADD_DIALOGUE_TEXT(217, 1, 335);
	} else {
		ADD_DIALOGUE_TEXT(218, 2, 336);
		ADD_DIALOGUE_TEXT(220, 1, 337);
	}
	SET_DIALOGUE_TEXT(1, 2);
	startCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = &IgorEngine::part15UpdateTobiasDialogue;
	waitForEndOfCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = 0;
	if (_objectsState[kPersonalFileChangedStateIndex] == 0) {
		return;
	}
	for (int i = 1; i <= 7; ++i) {
		part15DrawIgorAndTobiasScene(PART_15_ANIM_DATA_2[i]);
		waitForTimer(40);
	}
	removeObjectFromInventory(70);
	_objectsState[kProjectHandedToTobiasStateIndex] = 1;
	if (_objectsState[kMoneyHandedToTobiasStateIndex] == 0) {
		cutsceneSayWithCallback(133, 67, 0, 63, 19, 221, 2, 338, &IgorEngine::part15UpdateTobiasDialogue);
		return;
	}
	cutsceneSayWithCallback(133, 67, 0, 63, 19, 226, 2, 341, &IgorEngine::part15UpdateTobiasDialogue);
	igorSayAndWait(228, 1, 342);
	cutsceneSayWithCallback(133, 67, 0, 63, 19, 229, 1, 343, &IgorEngine::part15UpdateTobiasDialogue);
	igorSayAndWait(230, 1, 344);
	_currentPart = 780;
}

void IgorEngine::part15GiveMoneyToTobias() {
	part15WaitForCuckooClock();
	part15DrawIgorAndTobiasScene(6);
	igorSayAndWait({ { 209, 1, 328 }, { 211, 1, 330 } });
	cutsceneSayWithCallback(133, 67, 0, 63, 19, 212, 1, 331, &IgorEngine::part15UpdateTobiasDialogue);
	igorSayAndWait(213, 1, 332);
	cutsceneSayWithCallback(133, 67, 0, 63, 19, 214, 1, 333, &IgorEngine::part15UpdateTobiasDialogue);
	for (int i = 1; i <= 11; ++i) {
		part15DrawTobiasIdleFrame(PART_15_ANIM_DATA_5[i]);
		waitForTimer(60);
	}
	part15DrawIgorAndTobiasScene(6);
	if (_objectsState[kPersonalFileChangedStateIndex] == 0) {
		ADD_DIALOGUE_TEXT(215, 2, 334);
		ADD_DIALOGUE_TEXT(217, 1, 335);
	} else {
		ADD_DIALOGUE_TEXT(218, 2, 336);
		ADD_DIALOGUE_TEXT(223, 1, 339);
	}
	SET_DIALOGUE_TEXT(1, 2);
	startCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = &IgorEngine::part15UpdateTobiasDialogue;
	waitForEndOfCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = 0;
	if (_objectsState[kPersonalFileChangedStateIndex] == 0) {
		return;
	}
	for (int i = 1; i <= 7; ++i) {
		part15DrawIgorAndTobiasScene(PART_15_ANIM_DATA_4[i]);
		waitForTimer(40);
	}
	removeObjectFromInventory(60);
	_objectsState[kMoneyHandedToTobiasStateIndex] = 1;
	if (_objectsState[kProjectHandedToTobiasStateIndex] == 0) {
		cutsceneSayWithCallback(133, 67, 0, 63, 19, 224, 2, 340, &IgorEngine::part15UpdateTobiasDialogue);
		return;
	}
	cutsceneSayWithCallback(133, 67, 0, 63, 19, 226, 2, 341, &IgorEngine::part15UpdateTobiasDialogue);
	igorSayAndWait(228, 1, 342);
	cutsceneSayWithCallback(133, 67, 0, 63, 19, 229, 1, 343, &IgorEngine::part15UpdateTobiasDialogue);
	igorSayAndWait(230, 1, 344);
	_currentPart = 780;
}

void IgorEngine::part15UpdateBackground() {
	part15AnimateTobiasIdle();
	if (compareGameTick(38) || compareGameTick(60)) {
		if (_objectsState[kCuckooClockTriggeredStateIndex] != 1 && getRandomNumber(199) == 0 && _gameState.counter[3] == 0) {
			_gameState.counter[3] = 1;
		}
	}
	part15UpdateCuckooClock();
}

void IgorEngine::part15UpdateTobiasDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part15DrawTobiasTalking(0);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part15DrawTobiasTalking(getRandomNumber(4));
		break;
	case kUpdateDialogueAnimStanding:
		part15DrawTobiasTalking(0);
		break;
	}
}

void IgorEngine::part15HandleTobiasDialogue() {
	loadDialogueData(DLG_TobiasOffice);
	_updateDialogue = &IgorEngine::part15UpdateTobiasDialogue;
	handleDialogue(133, 67, 0, 63, 19);
	_updateDialogue = 0;
}

void IgorEngine::part15ApplyObjectState(int num) {
}

void IgorEngine::part15WalkIn() {
	_walkData[0].setPos(0, 108, 2, 0);
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
	_roomObjectAreasTable[_screenLayer2[34560]].area = 1;
	buildWalkPathSimple(0, 108, 34, 108);
	_roomObjectAreasTable[_screenLayer2[34560]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part15UpdateCuckooClock() {
	if (compareGameTick(38) || compareGameTick(60)) {
		if (_objectsState[kCuckooClockTriggeredStateIndex] != 1 && _gameState.counter[3] == 1) {
			if (_gameState.unk11 == 0) {
				_objectsState[kCuckooClockTriggeredStateIndex] = 1;
				_objectsState[kCuckooAnimationControlStateIndex] = 1;
				_gameState.counter[3] = 0;
			} else {
				part15DrawCuckooFrame(_gameState.unk11);
			}
			if (_gameState.unk11 == 7) {
				_objectsState[kCuckooAnimationControlStateIndex] = 2;
			}
			if (_objectsState[kCuckooAnimationControlStateIndex] == 2) {
				if (_gameState.counter[4] == 0) {
					playSound(54, 1);
				}
				++_gameState.counter[4];
				if (_gameState.unk11 == 7) {
					_gameState.unk11	= 6;
				} else {
					_gameState.unk11 = 7;
				}
				if (_gameState.counter[4] > 15) {
					_objectsState[kCuckooAnimationControlStateIndex] = 3;
					_gameState.unk11	= 6;
				}
			} else if (_objectsState[kCuckooAnimationControlStateIndex] == 1) {
				++_gameState.unk11;
			} else {
				--_gameState.unk11;
			}
		}
	}
}

void IgorEngine::part15WaitForCuckooClock() {
	if (_gameState.counter[3] == 1) {
		do {
			part15UpdateCuckooClock();
			waitForTimer();
		} while (_gameState.counter[3] != 0);
	}
}

void IgorEngine::part15AnimateTobiasIdle() {
	if (compareGameTick(7) || compareGameTick(29) || compareGameTick(61)) {
		if (_gameState.unk10 >= 3) {
			part15DrawTobiasIdleFrame(PART_15_ANIM_DATA_1[_gameState.unk10]);
			++_gameState.unk10;
			if (_gameState.unk10 == 8) {
				_gameState.unk10 = 1;
			}
		}
	}
	if (compareGameTick(5)) {
		if (_gameState.unk10 >= 1 && _gameState.unk10 <= 2) {
			part15DrawTobiasIdleFrame(_gameState.unk10);
			if (_gameState.unk10 == 1 && getRandomNumber(9) == 0) {
				_gameState.unk10 = 2;
			} else {
				_gameState.unk10 = 1;
			}
			if (getRandomNumber(29) == 0) {
				_gameState.unk10 = 3;
			}
		}
	}
}

void IgorEngine::part15DrawCuckooFrame(int frame) {
	_roomCursorOn = false;
	for (int i = 0; i <= 17; ++i) {
		for (int j = 0; j <= 52; ++j) {
			int offset = (i + 23) * 320 + j + 18;
			uint8 color = _screenVGA[offset];
			if (color < 0xF0 || color > 0xF1) {
				color = _animFramesBuffer[0x4B8C + frame * 954 + i * 53 + j];
			}
			_screenTempLayer[i * 100 + j] = color;
		}
	}
	int offset = 7378;
	copyArea(_screenVGA, offset, 320, _screenTempLayer, 100, 53, 18);
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + 0x4B8C + frame * 954, 53, 53, 18);
	if (_gameState.dialogueTextRunning) {
		memcpy(_screenTextLayer + 23040, _screenLayer1 + _dialogueDirtyRectY, _dialogueDirtyRectSize);
	}
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::part15DrawIgorAndTobiasScene(int frame) {
	int offset = 20887;
	drawAnimRect(offset, 0x49A + frame * 2124, 59, 36);
}

void IgorEngine::part15DrawTobiasTalking(int frame) {
	int offset = 22847;
	drawAnimRect(offset, 0x958 + frame * 182, 14, 13);
}

void IgorEngine::part15DrawTobiasIdleFrame(int frame) {
	int offset = 22835;
	drawAnimRect(offset, frame * 598, 26, 23);
}

void IgorEngine::part15() {
	if (isCDVersion())
		playMusic(2);
	_gameState.enableLight = 2;
	loadRoomData(PAL_TobiasOffice, IMG_TobiasOffice, BOX_TobiasOffice, MSK_TobiasOffice, TXT_TobiasOffice);
	static const int anm[] = { ANM_TobiasOffice1, AOF_TobiasOffice1, ANM_TobiasOffice2, AOF_TobiasOffice2, 0 };
	loadAnimData(anm);
	loadActionData(DAT_TobiasOffice);
	_roomDataOffsets = PART_15_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(28, 0, 96, 143);
	partMainSetExecuteAction(1, &IgorEngine::part15ExecuteAction);
	_updateRoomBackground = &IgorEngine::part15UpdateBackground;
	part15ApplyObjectState(255);
	setVerbPanelColors();
	setInventoryPanelColors();
	if (!restoreRoomAfterLoad()) {
		copyLayerToScreen(_screenLayer1Surface);
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		_gameState.unk10 = 1;
		_gameState.unk11 = 2;
		_gameState.counter[3] = 0;
		_gameState.counter[4] = 0;
		part15WalkIn();
		enterPartLoop();
	}
	if (takeResumeDialogue(1)) { // loaded while talking to tobias
		part15HandleTobiasDialogue();
		part15ApplyObjectState(255);
		finishResumedDialogue();
	}
	while (_currentPart == 150 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (_objectsState[kCuckooClockTriggeredStateIndex] == 1) {
		_objectsState[kCuckooClockTriggeredStateIndex] = 2;
	}
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
