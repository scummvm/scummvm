/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "igor/igor.h"

namespace Igor {

void IgorEngine::PART_35_EXEC_ACTION(int action) {
	debugC(9, kDebugGame, "PART_35_EXEC_ACTION %d", action);
	switch (action) {
	case 101:
		ADD_DIALOGUE_TEXT(201, 2, 611);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		_objectsState[82] = 1;
		PART_35_APPLY_OBJECT_STATE(3);
		break;
	case 102:
		// TODO: transcribe the inventory acquisition sequence from cseg100:0102-01F9.
		warning("PART_35_EXEC_ACTION action 102 unimplemented");
		break;
	case 103:
		ADD_DIALOGUE_TEXT(203, 1, 612);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 104:
		ADD_DIALOGUE_TEXT(204, 1, 613);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 105:
		ADD_DIALOGUE_TEXT(205, 1, 614);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 106:
		PART_35_ACTION_106_EXIT_TO_MAP();
		break;
	case 107:
		PART_35_ACTION_107_SCROLL_LEFT();
		break;
	default:
		warning("PART_35_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_35_ACTION_107_SCROLL_LEFT() {
	int xPos = 183;
	const int yPos = 124;
	int step = 1;
	_gameTicks = 8; // original counter 15 normalized to 8-tick engine units; cseg100:021A-0230
	do {
		if (compareGameTick(1, 16)) {
			for (int y = 0; y <= 143; ++y) {
				memcpy(_screenLayer2 + y * 320 + step * 8,
						_screenLayer1 + y * 320, 320 - step * 8);
				memcpy(_screenLayer2 + y * 320,
						_animFramesBuffer + y * 160 + 160 - step * 8, step * 8);
			}
			if (step < 5) {
				xPos -= _walkScaleTable[0x8F9 + _walkCurrentFrame]; // s3:4637; cseg100:02D1-02E5
				WalkData::setNextFrame(kFacingPositionLeft, _walkCurrentFrame);
			} else {
				_walkCurrentFrame = 0;
			}
			int dstOffset = (yPos - 50) * 320 + xPos - 175 + step * 8;
			for (int row = 0; row <= 49; ++row) {
				dstOffset += 320;
				for (int col = 0; col <= 29; ++col) {
					const uint8 color = _facingIgorFrames[kFacingPositionLeft - 1]
						[_walkCurrentFrame * 1500 + row * 30 + col];
					if (color != 0) {
						_screenLayer2[dstOffset + col] = color;
					}
				}
			}
			memcpy(_screenVGA, _screenLayer2, 46080);
			++step;
		}
		waitForTimer();
	} while (step != 21);

	_walkData[0].setPos(xPos, yPos, kFacingPositionLeft, 0);
	_walkData[0].setDefaultScale();
	_currentPart = 340;
}

void IgorEngine::PART_35_ACTION_106_EXIT_TO_MAP() {
	--_walkDataLastIndex;
	const uint8 area = _screenLayer2[10879]; // (319, 33); cseg100:0440-0455
	_roomObjectAreasTable[area].area = 1;
	buildWalkPath(242, 111, 319, 33);
	_roomObjectAreasTable[area].area = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 40;
}

void IgorEngine::PART_35_APPLY_OBJECT_STATE(int num) {
	if (num == 3 || num == 255) {
		_roomObjectAreasTable[7].object = (_objectsState[82] == 0) ? 1 : 2; // s3:0xDC7A; cseg100:0634-0653
	}
}

void IgorEngine::PART_35() {
	_gameState.enableLight = 1;
	loadActionData(DAT_ParkRight); // resource 100
	loadRoomData(PAL_Park, IMG_Park, BOX_Park, MSK_Park, TXT_Park); // left panel; cseg106:0002-06A1
	static const int frames[] = { FRM_Park1, FRM_Park2, FRM_Park3, FRM_Park4, 0 };
	loadAnimData(frames, 0x5A00);
	for (int y = 0; y <= 143; ++y) {
		memcpy(_animFramesBuffer + y * 160, _screenLayer1 + y * 320, 160);
	}
	loadRoomData(PAL_ParkRight, IMG_ParkRight, BOX_ParkRight, MSK_ParkRight,
			TXT_ParkRight); // active right panel; cseg100:1714-1739; cseg104:0002-0D20
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_35_EXEC_ACTION);
	_roomDataOffsets = PART_35_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 281, 143); // rightmost walkable column is 281
	PART_35_APPLY_OBJECT_STATE(255);

	if (_currentPart != 351) {
		memcpy(_screenVGA, _screenLayer1, 46080);
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
	}
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;

	if (_currentPart == 350) {
		WalkData *wd = &_walkData[0];
		wd->setPos(319, 33, kFacingPositionFront, 1);
		wd->clipSkipX = 1;
		wd->clipWidth = 10;
		wd->scaleWidth = 34;
		wd->xPosChanged = 1;
		wd->dxPos = 0;
		wd->yPosChanged = 1;
		wd->dyPos = 0;
		wd->scaleHeight = 34;
		_walkDataLastIndex = 0;
		const uint8 area = _screenLayer2[10879]; // (319, 33); cseg100:04CF-04E4
		_roomObjectAreasTable[area].area = 1;
		buildWalkPath(319, 33, 242, 111);
		_roomObjectAreasTable[area].area = 0;
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true;
		waitForIgorMove();
	}

	enterPartLoop();
	while (_currentPart == 350 || _currentPart == 351) {
		runPartLoop();
	}
	leavePartLoop();
	if (_currentPart == kInvalidPart) {
		fadeOut(768);
	} else if (_currentPart != 340) {
		fadeOut(624);
	}
}

} // End of namespace Igor
