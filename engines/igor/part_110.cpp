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
 */

#include "igor/igor.h"

namespace Igor {

namespace {

const uint32 kPart110PanelRight = 0x0000; // cseg176:288C-289B
const uint32 kPart110Frm1 = 0xB400;       // cseg177:0002
const uint32 kPart110Frm2 = 0xBC0A;       // cseg177:0002
const uint32 kPart110Frm3 = 0xC5E2;       // cseg177:0002
const uint32 kPart110Frm4 = 0xC786;       // cseg177:0002
const uint32 kPart110Frm5 = 0xC8AE;       // cseg177:0002

} // End of anonymous namespace

void IgorEngine::PART_110_APPLY_OBJECT_STATE(int num) {
	if (num == 1 || num == 255) // cseg176:09B3-09BD
		_roomObjectAreasTable[13].object = _objectsState[66] == 0 ? 4 : 3; // cseg176:09BF-09D2; s3:0xDC98 = object field 13

	if (num == 2 || num == 255) { // cseg176:09D2-09DC
		if (_objectsState[67] == 1) { // cseg176:09DE-0A0E; s3:0x865
			for (int y = 0; y <= 7; ++y)
				memcpy(_screenLayer1 + 0x7DA1 + y * 320, _animFramesBuffer + kPart110Frm4 + y * 37, 37); // cseg176:0002-0051
			_roomObjectAreasTable[11].object = 7;
		} else {
			_roomObjectAreasTable[11].object = 0;
		}
	}

	if (num == 3 || num == 255) { // cseg176:0A34-0A3E
		if (_objectsState[68] == 1) { // cseg176:0A40-0AAA; s3:0x866
			for (int y = 0; y <= 14; ++y)
				memcpy(_screenLayer1 + 0x4E32 + y * 320, _animFramesBuffer + kPart110Frm3 + y * 14, 14); // cseg176:0052-00A1
			_roomObjectAreasTable[8].object = 6;
		} else if (_objectsState[68] == 2) {
			for (int y = 0; y <= 14; ++y)
				memcpy(_screenLayer1 + 0x4E32 + y * 320, _animFramesBuffer + kPart110Frm3 + 210 + y * 14, 14); // cseg176:00A2-00F1
		} else {
			_roomObjectAreasTable[8].object = 0;
		}
		_roomObjectAreasTable[7].object = 6; // cseg176:0AA5; s3:0xDC7A = object field 7
		_roomObjectAreasTable[8].object = 8; // cseg176:0AAA; s3:0xDC7F = object field 8
	}
}

void IgorEngine::PART_110_ACTION_105() {
	for (int frame = 0; frame <= 1; ++frame) { // cseg176:06F8-076D
		for (int y = 0; y <= 48; ++y)
			memcpy(_screenVGA + 0x6167 + y * 320, _animFramesBuffer + kPart110Frm1 + frame * 0x405 + y * 21, 21);
		waitForTimer(127); // cseg176:075C-0766
	}
	// TODO: add inventory object 18 at slot 0x90F; the fork has no inventory-state API (cseg176:076F-07A9).
	playSound(51, 1); // cseg176:07A5-07A9
	_objectsState[66] = 1; // cseg176:07B3; s3:0x864
	PART_110_APPLY_OBJECT_STATE(255); // cseg176:07B8-07BA
}

void IgorEngine::PART_110_ACTION_107() {
	for (int frame = 0; frame <= 2; ++frame) { // cseg176:01C7-023C
		for (int y = 0; y <= 27; ++y)
			memcpy(_screenVGA + 0x47E3 + y * 320, _animFramesBuffer + kPart110Frm5 + frame * 0x348 + y * 30, 30);
		waitForTimer(61); // cseg176:022B-0235
	}
	// TODO: remove the inventory object indexed by s3:0x904; the fork has no inventory-state API (cseg176:023E-0281).
	playSound(63, 1); // cseg176:027D-0281
	_objectsState[68] = 1; // cseg176:0286; s3:0x866
	PART_110_APPLY_OBJECT_STATE(255); // cseg176:0290-0292
}

void IgorEngine::PART_110_ACTION_108() {
	uint8 *walkTable = loadData(WLK_DecanatoB); // cseg176:0584-05A6
	int xPos = 310; // cseg176:04AE
	int yPos = 0;
	int i = 1; // cseg176:04B3
	do {
		if (compareGameTick(1, 16)) { // cseg176:04B7-04CD
			for (int y = 0; y <= 143; ++y) { // cseg176:04D0-0565
				memcpy(_screenLayer2 + y * 320, _screenLayer1 + y * 320 + i * 8, 320 - i * 8);
				memcpy(_screenLayer2 + y * 320 + 320 - i * 8, _animFramesBuffer + kPart110PanelRight + y * 320, i * 8);
			}
			if (i < 9) { // cseg176:0568-05BE
				xPos += _walkScaleTable[0x8F9 + _walkCurrentFrame];
				assert(xPos >= 260);
				yPos = walkTable[xPos - 260];
				WalkData::setNextFrame(kFacingPositionRight, _walkCurrentFrame);
			} else {
				_walkCurrentFrame = 0; // cseg176:05C4
			}
			int dstOffset = (yPos - 50) * 320 + xPos - 10 - i * 8; // cseg176:05C9-05E5
			for (int row = 0; row <= 49; ++row) { // cseg176:05E8-0668
				dstOffset += 320;
				for (int col = 0; col <= 29; ++col) {
					const uint8 color = _facingIgorFrames[kFacingPositionRight - 1][_walkCurrentFrame * 1500 + row * 30 + col];
					if (color != 0)
						_screenLayer2[dstOffset + col] = color;
				}
			}
			memcpy(_screenVGA, _screenLayer2, 46080); // cseg176:066A-067F
			++i;
		}
		waitForTimer(); // cseg176:0683-06A2
	} while (i != 41); // cseg176:06A4-06AA
	free(walkTable);
	_walkData[0].setPos(xPos - 315, yPos, kFacingPositionRight, 0); // cseg176:06AD-06EB
	_walkData[0].setDefaultScale();
	_currentPart = 102; // cseg176:06F0
}

void IgorEngine::PART_110_ACTION_112() {
	for (int frame = 0; frame <= 2; ++frame) { // cseg176:00F2-0167
		for (int y = 0; y <= 27; ++y)
			memcpy(_screenVGA + 0x47E3 + y * 320, _animFramesBuffer + kPart110Frm2 + frame * 0x348 + y * 30, 30);
		waitForTimer(61); // cseg176:0156-0160
	}
	// TODO: add inventory object 23 at slot 0x914; the fork has no inventory-state API (cseg176:0169-01A3).
	playSound(51, 1); // cseg176:019F-01A3
	_objectsState[68] = 1; // cseg176:01AD; s3:0x866
	_objectsState[26] = 1; // cseg176:01B9; s3:0x83C
	PART_110_APPLY_OBJECT_STATE(3); // cseg176:01B2-01B4
}

void IgorEngine::PART_110_EXEC_ACTION(int action) {
	switch (action) { // cseg176:07C1-09A8
	case 101: // sub_176_0362
		ADD_DIALOGUE_TEXT(201, 2, 616); // cseg176:036C-0388
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 102: // sub_176_038F
		ADD_DIALOGUE_TEXT(29, 1, 29); // cseg176:0399-03B5
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 103: // sub_176_03BC
		ADD_DIALOGUE_TEXT(_objectsState[66] == 0 ? 203 : 204, 1, _objectsState[66] == 0 ? 617 : 618); // cseg176:03C6-03F7
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 104: // sub_176_03FE
		ADD_DIALOGUE_TEXT(205, 1, 619); // cseg176:0408-0436
		ADD_DIALOGUE_TEXT(206, 1, 620);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 105:
		PART_110_ACTION_105();
		break;
	case 106: // sub_176_043D
		ADD_DIALOGUE_TEXT(73, 1, 50); // cseg176:0447-0463
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 107:
		PART_110_ACTION_107();
		break;
	case 108:
		PART_110_ACTION_108();
		break;
	case 109: // sub_176_046A
		ADD_DIALOGUE_TEXT(207, 1, 621); // cseg176:0474-0490
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 110: // sub_176_0299
		ADD_DIALOGUE_TEXT(_objectsState[68] == 1 ? 208 : 209, 1, _objectsState[68] == 1 ? 622 : 623); // cseg176:02A3-02D4
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 111: // sub_176_0308
		ADD_DIALOGUE_TEXT(210, 1, 624); // cseg176:0312-032E
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 112:
		PART_110_ACTION_112();
		break;
	case 113: // sub_176_0335
		ADD_DIALOGUE_TEXT(211, 1, 625); // cseg176:033F-035B
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 114: // sub_176_02DB
		ADD_DIALOGUE_TEXT(29, 1, 29); // cseg176:02E5-0301
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	default:
		warning("PART_110_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_110() {
	_gameState.enableLight = 1;
	loadActionData(DAT_DecanatoPart110); // cseg176:286F-2882
	loadRoomData(PAL_DecanatoB, IMG_DecanatoB, BOX_DecanatoB, MSK_DecanatoB, TXT_DecanatoB); // cseg176:2887
	memcpy(_animFramesBuffer + kPart110PanelRight, _screenLayer1, 46080); // cseg176:288C-289B
	loadRoomData(PAL_DecanatoA, IMG_DecanatoA, BOX_DecanatoA, MSK_DecanatoA, TXT_DecanatoA); // cseg176:28C8

	static const int frames1[] = { FRM_Decanato1, 0 };
	static const int frames2[] = { FRM_Decanato2, 0 };
	static const int frames3[] = { FRM_Decanato3, 0 };
	static const int frames4[] = { FRM_Decanato4, 0 };
	static const int frames5[] = { FRM_Decanato5, 0 };
	loadAnimData(frames1, kPart110Frm1); // cseg176:28CD; cseg177:0002
	loadAnimData(frames2, kPart110Frm2);
	loadAnimData(frames3, kPart110Frm3);
	loadAnimData(frames4, kPart110Frm4);
	loadAnimData(frames5, kPart110Frm5);

	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_110_EXEC_ACTION); // cseg176:28D2; sub_176_07C1
	_roomDataOffsets = PART_110_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143); // cseg176 room mask is 320x144
	PART_110_APPLY_OBJECT_STATE(255); // cseg176:28D7-28D9; sub_176_09A9(-1)
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	_walkDataLastIndex = 1; // cseg176:29B6
	_walkDataCurrentIndex = 1; // cseg176:29BB

	enterPartLoop();
	while (_currentPart == 110) { // cseg176:29DF-29E6
		runPartLoop();
	}
	leavePartLoop();
}

} // End of namespace Igor
