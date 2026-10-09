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


static const uint8 PART_33_ANIM_DATA_4[4] = { 0, 1, 2, 1 };

static const uint8 PART_33_ANIM_DATA_5[5] = { 0, 6, 7, 8, 1 };

void IgorEngine::PART_33_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		igorSay(201, 1, 396);
		break;
	case 102:
		igorSay(202, 1, 397);
		break;
	case 103:
		igorSay(203, 1, 398);
		break;
	case 104:
		igorSay(204, 1, 399);
		break;
	case 105:
		igorSay(205, 1, 400);
		break;
	case 106:
		igorSay(206, 1, 401);
		break;
	case 107:
		igorSay(207, 1, 402);
		break;
	case 108:
		igorSay(208, 1, 403);
		break;
	case 109:
		PART_33_ACTION_109();
		break;
	case 110:
		igorSay(209, 1, 404);
		break;
	case 111:
		PART_33_ACTION_111();
		break;
	case 112:
		igorSay(213, 2, 407);
		break;
	case 113:
		PART_33_ACTION_113();
		break;
	case 114:
		PART_33_ACTION_114();
		break;
	case 115:
		PART_33_ACTION_115();
		break;
	default:
		error("PART_33_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_33_ACTION_109() {
	if (_objectsState[4] == 0) {
		igorSay(201, 1, 396);
		return;
	}
	if (_objectsState[74] == 1) {
		igorSay(222, 1, 413);
		return;
	}
	for (int i = 1; i <= 3; ++i) {
		for (int j = 0; j <= 48; ++j) {
			memcpy(_screenVGA + j * 320 + 23448, _animFramesBuffer + 0x467 * (PART_33_ANIM_DATA_4[i] - 1) + j * 23, 23);
			waitForTimer(120);
		}
	}
	addObjectToInventory(31, 66);
	_objectsState[74] = 1;
	PART_33_HELPER_1(255);
	igorSayAndWait({ { 210, 1, 405 }, { 211, 2, 406 } });
}

void IgorEngine::PART_33_ACTION_111() {
	if (_objectsState[75] == 1) {
		PART_33_HELPER_9();
		PART_33_HELPER_1(255);
		igorSayAndWait(201, 1, 447);
		PART_33_HARRISON_SPEAKS({ { 202, 2, 448 }, { 204, 2, 449 } });
		loadResourceData__ROOM_Library();
		PART_33_HELPER_1(255);
	} else {
		PART_33_HANDLE_DIALOGUE_HARRISON();
		PART_33_HELPER_1(255);
	}
}

void IgorEngine::PART_33_ACTION_113() {
	_walkDataCurrentIndex = 0;
	for (int i = 9; i >= 0; --i) {
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		WalkData *wd = &_walkData[0];
		wd->setPos(207, 143, 3, _walkCurrentFrame);
		WalkData::setNextFrame(3, _walkCurrentFrame);
		wd->clipSkipX = 1;
		wd->clipWidth = 30;
		wd->scaleWidth = i * 3 + 23;
		wd->xPosChanged = 1;
		wd->dxPos = 0;
		wd->yPosChanged = 1;
		wd->dyPos = 3;
		wd->scaleHeight = 50;
		moveIgor(wd->posNum, wd->frameNum);
		waitForTimer(15);
	}
	_currentPart = 241;
}

void IgorEngine::PART_33_ACTION_114() {
	copyArea(_screenVGA, 26279, 320, _animFramesBuffer + 0x5827, 63, 62, 29);
	igorSayAndWait(223, 1, 414);
	PART_33_HARRISON_SPEAKS({ { 224, 1, 415 } });
	for (int i = 2; i <= 5; ++i) {
		copyArea(_screenVGA, 26279, 320, _animFramesBuffer + i * 1827 + 0x5104, 63, 62, 29);
		waitForTimer(60);
	}
	removeObjectFromInventory(54);
	PART_33_HARRISON_SPEAKS({ { 225, 1, 416 } });
	copyArea(_screenVGA, 26279, 320, _animFramesBuffer + 0x5827, 63, 62, 29);
	igorSayAndWait(226, 1, 417);
	PART_33_HARRISON_SPEAKS({ { 227, 1, 418 } });
	igorSayAndWait(228, 1, 419);
	PART_33_HARRISON_SPEAKS({ { 229, 2, 420 }, { 231, 2, 421 } });
	for (int i = 1; i <= 4; ++i) {
		copyArea(_screenVGA, 26279, 320, _animFramesBuffer + PART_33_ANIM_DATA_5[i] * 1827 + 0x5104, 63, 62, 29);
		waitForTimer(60);
	}
	addObjectToInventory(35, 70);
	PART_33_HELPER_1(255);
	PART_33_HARRISON_SPEAKS({ { 233, 2, 422 } });
	_objectsState[75] = 1;
}

void IgorEngine::PART_33_ACTION_115() {
	const int offset = 26279;
	copyArea(_screenVGA, offset, 320, _animFramesBuffer + 0x5827, 63, 62, 29);
	waitForTimer(60);
	igorSayAndWait(215, 2, 408);
	for (int i = 2; i <= 5; ++i) {
		copyArea(_screenVGA, offset, 320, _animFramesBuffer + i * 1827 + 0x5104, 63, 62, 29);
		waitForTimer(60);
	}
	removeObjectFromInventory(67);
	PART_33_HELPER_1(255);
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
	PART_33_HARRISON_SPEAKS({ { 217, 2, 409 } });
	copyArea(_screenVGA, offset, 320, _animFramesBuffer + 0x5827, 63, 62, 29);
	PART_33_HARRISON_SPEAKS({ { 219, 1, 410 } });
	copyArea(_screenVGA, offset, 320, _animFramesBuffer + 0x5827, 63, 62, 29);
	_objectsState[76] = 1;
}

// The room's idle animation of Harrison is only driven by the room loop; the
// blocking dialogue loops never run it, otherwise it fights the talking frames.
void IgorEngine::PART_33_HARRISON_SPEAKS(const Common::Array<DialogueText> &lines) {
	const UpdateRoomBackgroundProc updateRoomBackground = _updateRoomBackground;
	_updateRoomBackground = 0;
	cutsceneSayWithCallback(47, 82, 0, 58, 40, lines, &IgorEngine::PART_33_UPDATE_DIALOGUE_HARRISON_2);
	_updateRoomBackground = updateRoomBackground;
}

void IgorEngine::PART_33_HANDLE_DIALOGUE_HARRISON() {
	const UpdateRoomBackgroundProc updateRoomBackground = _updateRoomBackground;
	_updateRoomBackground = 0;
	loadDialogueData(DLG_Library);
	_updateDialogue = &IgorEngine::PART_33_UPDATE_DIALOGUE_HARRISON_3;
	handleDialogue(47, 82, 0, 58, 40);
	_updateDialogue = 0;
	_updateRoomBackground = updateRoomBackground;
}

void IgorEngine::PART_33_UPDATE_DIALOGUE_HARRISON_1(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_33_HELPER_4(1);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_33_HELPER_4(getRandomNumber(3) + 2);
		break;
	}
}

void IgorEngine::PART_33_UPDATE_DIALOGUE_HARRISON_2(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_33_HELPER_8(4);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_33_HELPER_8(4 + getRandomNumber(2));
		break;
	}
}

void IgorEngine::PART_33_UPDATE_DIALOGUE_HARRISON_3(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_33_HELPER_5(4);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_33_HELPER_5(4 + getRandomNumber(2));
		break;
	case kUpdateDialogueAnimStanding:
		PART_33_HELPER_5(4);
		break;
	}
}

void IgorEngine::PART_33_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(61) && _objectsState[75] != 2) {
		PART_33_HELPER_8(_gameState.unk10);
		_gameState.unk10 = getRandomNumber(3) + 1;
		if (_gameState.unk10 >= 1 && _gameState.unk10 <= 2) {
			_gameState.unk10 = 1;
		} else {
			_gameState.unk10 = 2;
		}
	}
}

void IgorEngine::PART_33_HELPER_1(int num) {
	if (num == 2 || num == 255) {
		if (_objectsState[75] <= 1) {
			PART_33_HELPER_3();
		} else {
			_roomObjectAreasTable[27].object = 0;
		}
	}
}

void IgorEngine::PART_33_HELPER_2() {
	memcpy(_screenLayer2, _screenVGA, 46080);
	memcpy(_screenVGA, _screenLayer1, 46080);
	decodeAnimFrame(_animFramesBuffer + 0x8CE + READ_LE_UINT16(_animFramesBuffer + 0x5255) - 1, _screenVGA, true);
	memcpy(_screenLayer1, _screenVGA, 46080);
	fadeIn(624);
	cutsceneSayWithCallback(92, 79, 0, 58, 40, { { 220, 1, 411 }, { 221, 1, 412 } }, &IgorEngine::PART_33_UPDATE_DIALOGUE_HARRISON_1);
	for (int i = 6; i <= 25; ++i) {
		decodeAnimFrame(_animFramesBuffer + 0x8CE + READ_LE_UINT16(_animFramesBuffer + 0x5253 + i * 2) - 1, _screenVGA, true);
		waitForTimer(30);
	}
	fadeOut(624);
}

void IgorEngine::PART_33_HELPER_3() {
	const int offset = 29479;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + 0x5287, 16, 16, 15);
}

void IgorEngine::PART_33_HELPER_4(int frame) {
	decodeAnimFrame(_animFramesBuffer + 0x8CE + READ_LE_UINT16(_animFramesBuffer + 0x5253 + frame * 2) - 1, _screenVGA, true);
}

void IgorEngine::PART_33_HELPER_5(int frame) {
	_roomCursorOn = false;
	for (int i = 0; i <= 14; ++i) {
		for (int j = 0; j <= 15; ++j) {
			int offset = (i + 92) * 320 + j + 39;
			uint8 color = _screenVGA[offset];
			if (color < 192 || (color > 207 && color != 240 && color != 241)) {
				color = _animFramesBuffer[0x5197 + frame * 240 + i * 16 + j];
			}
			_screenTempLayer[100 * i + j] = color;
		}
	}
	for (int i = 0; i <= 14; ++i) {
		const int offset = i * 320 + 29479;
		memcpy(_screenVGA + offset, _screenTempLayer + i * 100, 16);
		memcpy(_screenLayer1 + offset, _animFramesBuffer + 0x5197 + frame * 240 + i * 16, 16);
	}
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::PART_33_HELPER_7() {
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	_walkCurrentPos = 1;
	for (int i = 0; i <= 9; ++i) {
		WalkData *wd = &_walkData[0];
		wd->setPos(207, 143, 1, _walkCurrentFrame);
		WalkData::setNextFrame(1, _walkCurrentFrame);
		wd->clipSkipX = 1;
		wd->clipWidth = 30;
		wd->scaleWidth = 23 + i * 3;
		wd->xPosChanged = 1;
		wd->dxPos = 0;
		wd->yPosChanged = 1;
		wd->dyPos = 3;
		wd->scaleHeight = 50;
		moveIgor(wd->posNum, wd->frameNum);
		waitForTimer(15);
	}
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;
	--_walkDataLastIndex;
	buildWalkPath(207, 143, 187, 138);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_33_HELPER_8(int frame) {
	_roomCursorOn = false;
	for (int i = 0; i <= 14; ++i) {
		for (int j = 0; j <= 15; ++j) {
			int offset = (i + 92) * 320 + j + 39;
			uint8 color = _screenVGA[offset];
			if (color < 192 || (color > 207 && color != 240 && color != 241)) {
				color = _animFramesBuffer[0x5197 + frame * 240 + i * 16 + j];
			}
			_screenTempLayer[100 * i + j] = color;
		}
	}
	for (int i = 0; i <= 14; ++i) {
		const int offset = i * 320 + 29479;
		memcpy(_screenVGA + offset, _screenTempLayer + i * 100, 16);
		memcpy(_screenLayer1 + offset, _animFramesBuffer + 0x5197 + frame * 240 + i * 16, 16);
	}
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::PART_33_HELPER_9() {
	uint8 *p = loadData(TXT_Library2);
	decodeRoomStrings(p, true);
	free(p);
}

void IgorEngine::PART_33() {
	if (_currentPart != 331) {
		playMusic(2);
	}
	_gameState.enableLight = 1;
	_gameState.unk10 = 1;
	loadResourceData__ROOM_Library();
	loadResourceData__ANIM_Library();
	loadActionData(DAT_Library);
	_roomDataOffsets = PART_33_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 276, 143);
	if (_currentPart == 331) {
		PART_33_HELPER_2();
		return;
	}
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_33_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_33_UPDATE_ROOM_BACKGROUND;
	PART_33_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	fadeIn(768);
	PART_33_HELPER_7();
	enterPartLoop();
	while (_currentPart == 330) {
		runPartLoop();
	}
	leavePartLoop();
	if (_objectsState[75] == 1) {
		_objectsState[75] = 2;
	}
	fadeOut(624);
	_updateRoomBackground = 0;
}

void IgorEngine::loadResourceData__ROOM_Library() {
	loadRoomData(PAL_Library, IMG_Library, BOX_Library, MSK_Library, TXT_Library);
}

void IgorEngine::loadResourceData__ANIM_Library() {
	static const int anm[] = { FRM_Library1, FRM_Library2, FRM_Library3, FRM_Library4, FRM_Library5, 0 };
	loadAnimData(anm);
}

} // End of namespace Igor
