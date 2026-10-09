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
#include "igor/statics.h"

namespace Igor {

const uint32 kPart11PanelRight = 0x0000;

void IgorEngine::PART_11_APPLY_OBJECT_STATE(int num) {
	PART_10_11_DRAW_OBJECT_STATE(num);

	if (num == 1 || num == 255)
		_roomObjectAreasTable[13].object = _objectsState[40] == 0 ? 4 : 3;

	if (num == 2 || num == 255) {
		if (_objectsState[41] == 1) {
			// make area selectable
			for (int area = 10; area <= 11; ++area)
				_roomObjectAreasTable[area].object = 7;
		} else {
			// make area not selectable
			for (int area = 10; area <= 11; ++area)
				_roomObjectAreasTable[area].object = 0;
		}
	}

	if (num == 3 || num == 255) {
		if (_objectsState[42] == 0) {
			for (int area = 7; area <= 8; ++area)
				_roomObjectAreasTable[area].object = 0;
		} else if (_objectsState[42] == 1) {
			for (int area = 7; area <= 8; ++area)
				_roomObjectAreasTable[area].object = 6;
		} else if (_objectsState[42] == 2) {
			_roomObjectAreasTable[7].object = 6;
			_roomObjectAreasTable[8].object = 8;
		}
	}
}

void IgorEngine::PART_11_ACTION_105() {
	for (int frame = 0; frame <= 1; ++frame) {
		for (int y = 0; y <= 48; ++y)
			memcpy(_screenVGA + 0x6167 + y * 320, _animFramesBuffer + kPart10_11_Frm1 + frame * 0x405 + y * 21, 21);
		waitForTimer(127);
	}
	addObjectToInventory(18, 53);
	_objectsState[40] = 1;
	PART_11_APPLY_OBJECT_STATE(255);
}

void IgorEngine::PART_11_ACTION_107() {
	for (int frame = 0; frame <= 2; ++frame) {
		for (int y = 0; y <= 27; ++y)
			memcpy(_screenVGA + 0x47E3 + y * 320, _animFramesBuffer + kPart10_11_Frm5 + frame * 0x348 + y * 30, 30);
		waitForTimer(61);
	}
	removeObjectFromInventory(42);
	_objectsState[42] = 1;
	PART_11_APPLY_OBJECT_STATE(255);
}

void IgorEngine::PART_11_ACTION_108_scrollRight() {
	uint8 *walkTable = loadData(WLK_DecanatoRight);
	int xPos = 310;
	int yPos = 0;
	int i = 1;
	do {
		if (compareGameTick(1, 16)) {
			for (int y = 0; y <= 143; ++y) {
				memcpy(_screenLayer2 + y * 320, _screenLayer1 + y * 320 + i * 8, 320 - i * 8);
				memcpy(_screenLayer2 + y * 320 + 320 - i * 8, _animFramesBuffer + kPart11PanelRight + y * 320, i * 8);
			}
			if (i < 9) {
				xPos += _walkScaleTable[0x8F9 + _walkCurrentFrame];
				assert(xPos >= 260);
				yPos = walkTable[xPos - 260];
				WalkData::setNextFrame(kFacingPositionRight, _walkCurrentFrame);
			} else {
				_walkCurrentFrame = 0;
			}
			int dstOffset = (yPos - 50) * 320 + xPos - 10 - i * 8;
			for (int row = 0; row <= 49; ++row) {
				dstOffset += 320;
				for (int col = 0; col <= 29; ++col) {
					const uint8 color = _facingIgorFrames[kFacingPositionRight - 1][_walkCurrentFrame * 1500 + row * 30 + col];
					if (color != 0)
						_screenLayer2[dstOffset + col] = color;
				}
			}
			memcpy(_screenVGA, _screenLayer2, 46080);
			++i;
		}
		waitForTimer();
	} while (i != 41);
	free(walkTable);
	_walkData[0].setPos(xPos - 315, yPos, kFacingPositionRight, 0);
	_walkData[0].setDefaultScale();
	_currentPart = 102;
}

void IgorEngine::PART_11_ACTION_112() {
	for (int frame = 0; frame <= 2; ++frame) {
		for (int y = 0; y <= 27; ++y)
			memcpy(_screenVGA + 0x47E3 + y * 320, _animFramesBuffer + kPart10_11_Frm2 + frame * 0x348 + y * 30, 30);
		waitForTimer(61);
	}
	addObjectToInventory(23, 58);
	_objectsState[42] = 1;
	PART_11_APPLY_OBJECT_STATE(3);
	_objectsState[0] = 1;
	UPDATE_OBJECT_STATE(1);
}

void IgorEngine::PART_11_EXEC_ACTION(int action) {
	switch (action) {
	case 101: // look at window
		ADD_DIALOGUE_TEXT(201, 2, 616);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 102:
		igorSayAndWait(29, 1, 29);
		break;
	case 103: // look at pipe
		igorSayAndWait(_objectsState[40] == 0 ? 203 : 204, 1, _objectsState[40] == 0 ? 617 : 618);
		break;
	case 104: // look at snail
		igorSayAndWait({ { 205, 1, 619 }, { 206, 1, 620 } });
		break;
	case 105:
		PART_11_ACTION_105();
		break;
	case 106:
		igorSayAndWait(73, 1, 50);
		break;
	case 107:
		PART_11_ACTION_107();
		break;
	case 108:
		PART_11_ACTION_108_scrollRight();
		break;
	case 109: // Look at hole
		igorSayAndWait(207, 1, 621);
		break;
	case 110: // Look at butterfly net
		igorSayAndWait(_objectsState[42] == 1 ? 208 : 209, 1, _objectsState[42] == 1 ? 622 : 623);
		break;
	case 111: // look at glass shards
		igorSayAndWait(210, 1, 624);
		break;
	case 112:
		PART_11_ACTION_112();
		break;
	case 113:
		igorSayAndWait(211, 1, 625);
		break;
	case 114:
		igorSayAndWait(29, 1, 29);
		break;
	default:
		warning("PART_11_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_11() {
	playMusic(1);
	_gameState.enableLight = 1;
	loadActionData(DAT_DecanatoPart11);
	loadRoomData(PAL_DecanatoRight, IMG_DecanatoRight, BOX_DecanatoRight, MSK_DecanatoRight, TXT_DecanatoRight);
	memcpy(_animFramesBuffer + kPart11PanelRight, _screenLayer1, 46080);
	loadRoomData(PAL_DecanatoLeft, IMG_DecanatoLeft, BOX_DecanatoLeft, MSK_DecanatoLeft, TXT_DecanatoLeft);

	static const int frames1[] = { FRM_Decanato1, 0 };
	static const int frames2[] = { FRM_Decanato2, 0 };
	static const int frames3[] = { FRM_Decanato3, 0 };
	static const int frames4[] = { FRM_Decanato4, 0 };
	static const int frames5[] = { FRM_Decanato5, 0 };
	loadAnimData(frames1, kPart10_11_Frm1);
	loadAnimData(frames2, kPart10_11_Frm2);
	loadAnimData(frames3, kPart10_11_Frm3);
	loadAnimData(frames4, kPart10_11_Frm4);
	loadAnimData(frames5, kPart10_11_Frm5);

	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_11_EXEC_ACTION);
	_roomDataOffsets = PART_11_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	PART_11_APPLY_OBJECT_STATE(255);
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	if (!restoreRoomAfterLoad()) {
		_walkDataLastIndex = 1;
		_walkDataCurrentIndex = 1;
	}

	enterPartLoop();
	while (_currentPart == 110 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
}

} // End of namespace Igor
