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

#include "igor/igor.h"

namespace Igor {

static int VAR_CURRENT_TALKING_ACTOR;

void IgorEngine::PART_17_EXEC_ACTION(int action) {
	debugC(9, kDebugGame, "PART_17_EXEC_ACTION %d", action);
	switch (action) {
	case 101:
		PART_17_ACTION_101();
		break;
	case 102:
		ADD_DIALOGUE_TEXT(225, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 103:
		PART_17_ACTION_103();
		break;
	case 104:
		ADD_DIALOGUE_TEXT(220, 3);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 105:
		PART_17_ACTION_105();
		break;
	case 106:
		PART_17_ACTION_106();
		break;
	default:
		error("PART_17_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_17_ACTION_101() {
	_roomObjectAreasTable[_screenLayer2[22034]].area = 8;
	_roomObjectAreasTable[_screenLayer2[23923]].area = 8;
	--_walkDataLastIndex;
	buildWalkPath(243, 77, 243, 74);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	buildWalkPath(243, 74, 274, 68);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 302;
}

void IgorEngine::PART_17_ACTION_103() {
	PART_17_HELPER_8(0);
	if (_gameState.counter[4] == 1) {
		ADD_DIALOGUE_TEXT(201, 2);
	} else if (_gameState.counter[4] == 2) {
		ADD_DIALOGUE_TEXT(206, 2);
	} else if (_gameState.counter[4] == 3) {
		ADD_DIALOGUE_TEXT(210, 2);
	}
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(156, 32, 35, 38, 47);
	_updateDialogue = &IgorEngine::PART_17_UPDATE_DIALOGUE_PHILIP;
	waitForEndOfCutsceneDialogue(156, 32, 35, 38, 47);
	_updateDialogue = 0;
	if (_gameState.counter[4] == 1) {
		ADD_DIALOGUE_TEXT(203, 1);
		ADD_DIALOGUE_TEXT(204, 2);
		SET_DIALOGUE_TEXT(1, 2);
	} else if (_gameState.counter[4] == 2) {
		ADD_DIALOGUE_TEXT(208, 2);
		SET_DIALOGUE_TEXT(1, 1);
	} else if (_gameState.counter[4] == 3) {
		ADD_DIALOGUE_TEXT(212, 2);
		ADD_DIALOGUE_TEXT(214, 1);
		SET_DIALOGUE_TEXT(1, 2);
	}
	startCutsceneDialogue(135, 33, 63, 63, 0);
	VAR_CURRENT_TALKING_ACTOR = 0;
	_updateDialogue = &IgorEngine::PART_17_UPDATE_DIALOGUE_PHILIP_JIMMY;
	waitForEndOfCutsceneDialogue(135, 33, 63, 63, 0);
	_updateDialogue = 0;
	PART_17_HELPER_11(0);
	waitForTimer(255);
	ADD_DIALOGUE_TEXT(215 + getRandomNumber(4), 1);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(135, 33, 63, 63, 0);
	VAR_CURRENT_TALKING_ACTOR = 1;
	_updateDialogue = &IgorEngine::PART_17_UPDATE_DIALOGUE_PHILIP_JIMMY;
	waitForEndOfCutsceneDialogue(135, 33, 63, 63, 0);
	_updateDialogue = 0;
	PART_17_HANDLE_DIALOGUE_PHILIP();
	PART_17_HELPER_1(255);
	PART_17_HELPER_8(0);
	--_walkDataLastIndex;
	buildWalkPath(104, 87, 143, 123);
	_walkDataCurrentIndex = 1;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
	ADD_DIALOGUE_TEXT(219, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	if (_gameState.counter[4] == 3) {
		_gameState.counter[4] = 1;
	} else {
		++_gameState.counter[4];
	}
}

void IgorEngine::PART_17_ACTION_105() {
	_roomObjectAreasTable[5].area = 1;
	_roomObjectAreasTable[10].area = 2;
	_roomObjectAreasTable[11].area = 2;
	_roomObjectAreasTable[13].area = 2;
	_roomObjectAreasTable[16].area = 3;
	_roomObjectAreasTable[17].area = 4;
	_roomObjectAreasTable[26].area = 4;
	_roomObjectAreasTable[28].area = 4;
	--_walkDataLastIndex;
	buildWalkPath(45, 95, 41, 86);
	_walkDataCurrentIndex = 1;
	_walkData[_walkDataLastIndex].frameNum = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	PART_17_HELPER_5(143);
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[44836]].area = 4;
	buildWalkPath(41, 86, 36, 143);
	_roomObjectAreasTable[_screenLayer2[44836]].area = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	for (int i = _walkDataCurrentIndex; i <= _walkDataLastIndex; ++i) {
		_walkData[i].posNum = kFacingPositionBack;
	}
	waitForIgorMove();
	for (int i = 17; i <= 85; ++i) {
		_walkYScaleRoom[i] = ((i - 13) / 4) + 5;
	}
	for (int i = 86; i <= 135; ++i) {
		_walkYScaleRoom[i] = 23;
	}
	_walkData[0].setPos(65, 115, 1, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 14;
	_walkData[0].scaleWidth = 23;
	_walkData[0].scaleHeight = 23;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[36865]].area = 4;
	buildWalkPath(65, 115, 21, 17);
	_roomObjectAreasTable[_screenLayer2[36865]].area = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 40;
}

void IgorEngine::PART_17_ACTION_106() {
	if (_objectsState[55] == 1) {
		ADD_DIALOGUE_TEXT(226, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		return;
	}
	playSound(24, 1);
	for (int i = 0; i <= 8; ++i) {
		int offset = 12575;
		for (int j = 0; j <= 27; ++j) {
			memcpy(_screenVGA + offset + i * 320, _animFramesBuffer + 0x1E6E + j * 40 + i * 1120, 40);
		}
		waitForTimer(30);
	}
	int offset = 12575;
	for (int i = 0; i <= 27; ++i) {
		memcpy(_screenVGA + offset + i * 320, _animFramesBuffer + 0x1E6E + i * 40, 40);
	}
	_objectsState[55] = 1;
	_objectsState[3] = 1;
	PART_17_HELPER_1(255);
	UPDATE_OBJECT_STATE(4);
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
	--_walkDataLastIndex;
	buildWalkPath(104, 87, 143, 123);
	_walkDataCurrentIndex = 1;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
	ADD_DIALOGUE_TEXT(223, 2);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
}

void IgorEngine::PART_17_HANDLE_DIALOGUE_PHILIP() {
	loadDialogueData(DLG_OutsideCollege);
	warning("PART_17_HANDLE_DIALOGUE_PHILIP unimplemented");
}

void IgorEngine::PART_17_UPDATE_DIALOGUE_PHILIP(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_17_HELPER_9(0);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_17_HELPER_9(getRandomNumber(7));
		break;
	}
}

void IgorEngine::PART_17_UPDATE_DIALOGUE_PHILIP_JIMMY(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		if (VAR_CURRENT_TALKING_ACTOR == 0) {
			PART_17_HELPER_8(0);
		} else {
			PART_17_HELPER_11(0);
		}
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		if (VAR_CURRENT_TALKING_ACTOR == 0) {
			PART_17_HELPER_8(getRandomNumber(7));
		} else {
			PART_17_HELPER_11(getRandomNumber(6));
		}
		break;
	}
}

void IgorEngine::PART_17_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(3, 32) && _gameState.unkF) {
		switch (_gameState.unk10 / 16) {
		case 0:
			PART_17_HELPER_8(0);
			PART_17_HELPER_9(getRandomNumber(7));
			break;
		case 1:
			PART_17_HELPER_8(getRandomNumber(7));
			PART_17_HELPER_9(0);
		}
		if (_gameState.unk10 == 31) {
			_gameState.unk10 = 0;
		} else {
			++_gameState.unk10;
		}
	}
}

void IgorEngine::PART_17_HELPER_1(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[54] == 0) {
			_gameState.unkF = false;
		} else {
			PART_17_HELPER_3(0);
			_gameState.unkF = true;
		}
	}
	if (num == 2 || num == 255) {
		if (_objectsState[55] == 1) {
			_roomActionsTable[2698] = 0;
		}
	}
	if (num == 3 || num == 255) {
		if (_objectsState[56] == 1) {
			_gameState.unkF = false;
			PART_17_HELPER_3(6);
		}
	}
}

void IgorEngine::PART_17_HELPER_2() {
	for (int i = 17; i <= 85; ++i) {
		_walkYScaleRoom[i] = ((i - 13) / 4) + 5;
	}
	for (int i = 86; i <= 135; ++i) {
		_walkYScaleRoom[i] = 23;
	}
	WalkData *wd = &_walkData[0];
	wd->setPos(21, 17, 3, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 4;
	wd->scaleWidth = 6;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 6;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[36865]].area = 4;
	buildWalkPath(21, 17, 65, 115);
	_roomObjectAreasTable[_screenLayer2[36865]].area = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	for (int i = 17; i <= 135; ++i) {
		_walkYScaleRoom[i] = 50;
	}
	_walkData[0].setPos(36, 140, 3, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 30;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[44836]].area = 4;
	buildWalkPath(36, 143, 41, 86);
	_roomObjectAreasTable[_screenLayer2[44836]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	for (int i = _walkDataCurrentIndex; i <= _walkDataLastIndex; ++i) {
		_walkData[i].posNum = kFacingPositionFront;
	}
	waitForIgorMove();
	PART_17_HELPER_5(0);
	--_walkDataLastIndex;
	buildWalkPath(41, 86, 45, 95);
	_walkDataCurrentIndex = 2;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_roomObjectAreasTable[5].area = 0;
	_roomObjectAreasTable[10].area = 0;
	_roomObjectAreasTable[11].area = 0;
	_roomObjectAreasTable[13].area = 0;
	_roomObjectAreasTable[16].area = 0;
	_roomObjectAreasTable[17].area = 0;
	_roomObjectAreasTable[26].area = 0;
	_roomObjectAreasTable[28].area = 0;
}

void IgorEngine::PART_17_HELPER_3(int lum) {
	_roomObjectAreasTable[18].y1Lum = lum;
	_roomObjectAreasTable[19].y1Lum = lum;
	_roomObjectAreasTable[20].y1Lum = lum;
}

void IgorEngine::PART_17_HELPER_4() {
	int offset = 11642;
	for (int i = 0; i <= 48; ++i) {
		memcpy(_screenLayer1 + offset +  i * 320, _animFramesBuffer + i * 44, 44);
		memcpy(_screenVGA + offset +  i * 320, _animFramesBuffer + i * 44, 44);
	}
}

void IgorEngine::PART_17_HELPER_5(int lum) {
	_roomObjectAreasTable[23].y1Lum = lum;
	_roomObjectAreasTable[26].y1Lum = lum;
	_roomObjectAreasTable[27].y1Lum = lum;
	_roomObjectAreasTable[32].y1Lum = lum;
}

void IgorEngine::PART_17_HELPER_6() {
	playMusic(2);
	PART_17_HELPER_1(255);
	fadeInPalette(768);
	_walkData[0].setPos(274, 68, 4, 1);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 30;
	_walkCurrentFrame = 1;
	_walkDataCurrentIndex = 0;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[22034]].area = 8;
	_roomObjectAreasTable[_screenLayer2[23923]].area = 8;
	buildWalkPath(274, 68, 243, 74);
	_roomObjectAreasTable[_screenLayer2[22034]].area = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	--_walkDataLastIndex;
	buildWalkPath(243, 74, 205, 124);
	_roomObjectAreasTable[_screenLayer2[23923]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	PART_17_HELPER_5(0);
	_roomObjectAreasTable[5].area = 0;
	_roomObjectAreasTable[10].area = 0;
	_roomObjectAreasTable[11].area = 0;
	_roomObjectAreasTable[13].area = 0;
	_roomObjectAreasTable[16].area = 0;
	_roomObjectAreasTable[17].area = 0;
	_roomObjectAreasTable[26].area = 0;
	_roomObjectAreasTable[28].area = 0;
}

void IgorEngine::PART_17_HELPER_8(int num) {
	for (int i = 0; i <= 8; ++i) {
		for (int j = 0; j <= 10; ++j) {
			int offset = (36 + i) * 320 + 130 + j;
			uint8 color = _screenVGA[offset];
			if (color >= 0xF0 && color <= 0xF1) {
				_screenTempLayer[i * 100 + j] = _screenVGA[offset];
			} else {
				_screenTempLayer[i * 100 + j] = _animFramesBuffer[num * 99 + i * 11 + j + 0x1967];
			}
		}
	}
	int offset = 11650;
	for (int i = 0; i <= 8; ++i) {
		memcpy(_screenVGA + i * 320 + offset, _screenTempLayer + i * 100, 11);
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + num * 99 + i * 11 + 0x1967, 11);
	}
}

void IgorEngine::PART_17_HELPER_9(int num) {
	for (int i = 0; i <= 26; ++i) {
		for (int j = 0; j <= 22; ++j) {
			int offset = (36 + i) * 320 + 143 + j;
			uint8 color = _screenVGA[offset];
			if ((color >= 0xC0 && color <= 0xCF) || (color >= 0xF0 && color <= 0xF1)) {
				_screenTempLayer[i * 100 + j] = _screenVGA[offset];
			} else {
				_screenTempLayer[i * 100 + j] = _animFramesBuffer[num * 621 + i * 23 + j + 0x86C];
			}
		}
	}
	int offset = 11663;
	for (int i = 0; i <= 26; ++i) {
		memcpy(_screenVGA + i * 320 + offset, _screenTempLayer + i * 100, 23);
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + num * 621 + i * 23 + 0x86C, 23);
	}
}

void IgorEngine::PART_17_HELPER_11(int frame) {
	for (int i = 0; i <= 8; ++i) {
		for (int j = 0; j <= 10; ++j) {
			int offset = (i + 36) * 320 + j + 130;
			uint8 color = _screenVGA[offset];
			if (color != 0 && color != 240) {
				color = _animFramesBuffer[0x1C1C + frame * 99 + i * 11 + j];
			}
			_screenTempLayer[100 * i + j] = color;
		}
	}
	for (int i = 0; i <= 8; ++i) {
		const int offset = i * 320 + 11650;
		memcpy(_screenVGA + offset, _screenTempLayer + i * 100, 11);
		memcpy(_screenLayer1 + offset, _animFramesBuffer + 0x1C1C + frame * 99 + i * 11, 11);
	}
}

void IgorEngine::PART_17() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_OutsideCollege, IMG_OutsideCollege, BOX_OutsideCollege, MSK_OutsideCollege, TXT_OutsideCollege);
	static const int anm[] = { FRM_OutsideCollege1, FRM_OutsideCollege2, FRM_OutsideCollege3, FRM_OutsideCollege4, FRM_OutsideCollege5, 0 };
	loadAnimData(anm);
	loadActionData(DAT_OutsideCollege);
	_roomDataOffsets = PART_17_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 286, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_17_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_17_UPDATE_ROOM_BACKGROUND;
	PART_17_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	if (_objectsState[56] == 1) {
		_gameState.unkF = false;
		PART_17_HELPER_3(6);
		_roomObjectAreasTable[14].object = 0;
		_roomObjectAreasTable[15].object = 0;
		_roomObjectAreasTable[19].object = 0;
	} else if (_gameState.unkF) {
		PART_17_HELPER_4();
		_gameState.counter[4] = 1;
	} else {
		_roomObjectAreasTable[14].object = 0;
		_roomObjectAreasTable[15].object = 0;
		_roomObjectAreasTable[19].object = 0;
	}
	_currentAction.verb = kVerbWalk;
	if (_currentPart == 170) {
		fadeInPalette(768);
		PART_17_HELPER_2();
	} else if (_currentPart == 171) {
		PART_17_HELPER_6();
	}
	enterPartLoop();
	while (_currentPart >= 170 && _currentPart <= 171) {
		runPartLoop();
	}
	leavePartLoop();
	if (_objectsState[55] == 1) {
		_objectsState[56] = 1;
	}
	if (_objectsState[54] == 0) {
		_objectsState[54] = 1;
	}
	fadeOutPalette(624);
}

} // namespace Igor
