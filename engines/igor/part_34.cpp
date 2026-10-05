/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "igor/igor.h"

namespace Igor {

void IgorEngine::PART_34_EXEC_ACTION(int action) {
	debugC(9, kDebugGame, "PART_34_EXEC_ACTION %d", action);
	switch (action) {
	case 101:
		ADD_DIALOGUE_TEXT(201, 2, 562);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 102:
		ADD_DIALOGUE_TEXT(203, 2, 563);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		_objectsState[82] = 1;
		PART_34_APPLY_OBJECT_STATE(3);
		break;
	case 103:
		// TODO: transcribe the inventory acquisition sequence from cseg101:018D-0284.
		warning("PART_34_EXEC_ACTION action 103 unimplemented");
		break;
	case 104:
		ADD_DIALOGUE_TEXT(205, 1, 564);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 105:
		// TODO: transcribe the park conversation from cseg101:0A9B-0AC3 and cseg094:2498-280D.
		warning("PART_34_EXEC_ACTION action 105 unimplemented");
		break;
	case 106:
		ADD_DIALOGUE_TEXT(206, 2, 565);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 107:
		ADD_DIALOGUE_TEXT(208, 2, 566);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 108:
		// TODO: transcribe the multi-actor park sequence from cseg101:04B7-0A9A.
		warning("PART_34_EXEC_ACTION action 108 unimplemented");
		break;
	case 109:
		PART_34_ACTION_109_SCROLL_RIGHT();
		break;
	default:
		warning("PART_34_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_34_ACTION_109_SCROLL_RIGHT() {
	int xPos = 240;
	const int yPos = 124;
	int step = 1;
	_gameTicks = 8; // original counter 15 normalized to 8-tick engine units; cseg101:0BD1-0BE7
	do {
		if (compareGameTick(1, 16)) {
			for (int y = 0; y <= 143; ++y) {
				memcpy(_screenLayer2 + y * 320, _screenLayer1 + y * 320 + step * 8, 320 - step * 8);
				memcpy(_screenLayer2 + y * 320 + 320 - step * 8, _animFramesBuffer + y * 160, step * 8);
			}
			if (step < 15) {
				xPos += _walkScaleTable[0x8F9 + _walkCurrentFrame];
				WalkData::setNextFrame(kFacingPositionRight, _walkCurrentFrame);
			} else {
				_walkCurrentFrame = 0;
			}
			int dstOffset = (yPos - 50) * 320 + xPos - 15 - step * 8;
			for (int row = 0; row <= 49; ++row) {
				dstOffset += 320;
				for (int col = 0; col <= 29; ++col) {
					const uint8 color = _facingIgorFrames[kFacingPositionRight - 1]
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

	_walkData[0].setPos(xPos - 160, yPos, kFacingPositionRight, 0);
	_walkData[0].setDefaultScale();
	_currentPart = 351;
}

void IgorEngine::PART_34_APPLY_OBJECT_STATE(int num) {
	if (num == 1 || num == 255) {
		if (!(_objectsState[80] == 0 && _objectsState[73] == 1)) {
			_roomObjectAreasTable[5].object = 0;
		} else {
			// TODO: draw the state-dependent frame from FRM_Park1; cseg101:0F4A-0F79.
		}
	}
	if (num == 3 || num == 255) {
		_roomObjectAreasTable[4].object = (_objectsState[82] == 0) ? 2 : 3;
	}
}

void IgorEngine::PART_34() {
	_gameState.enableLight = 1;
	loadActionData(DAT_ParkLeft);
	loadRoomData(PAL_ParkRight, IMG_ParkRight, BOX_ParkRight, MSK_ParkRight, TXT_ParkRight);
	for (int y = 0; y <= 143; ++y) {
		memcpy(_animFramesBuffer + y * 160, _screenLayer1 + y * 320 + 160, 160);
	}
	loadRoomData(PAL_Park, IMG_Park, BOX_Park, MSK_Park, TXT_Park); // active left panel;
	static const int frames[] = { FRM_Park1, FRM_Park2, FRM_Park3, FRM_Park4, 0 };
	loadAnimData(frames, 0x5A00);
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_34_EXEC_ACTION);
	_roomDataOffsets = PART_34_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	PART_34_APPLY_OBJECT_STATE(255);

	if (!restoreRoomAfterLoad()) {
		_walkDataLastIndex = 1;
		_walkDataCurrentIndex = 1;
	}
	enterPartLoop();
	while (_currentPart == 340 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (_currentPart == kInvalidPart && !_gameStateLoaded) {
		fadeOut(768);
	}
}

} // End of namespace Igor
