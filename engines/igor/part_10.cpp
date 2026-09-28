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

// Ground truth: code/175_2767.asm, cseg175:0002-3595, cseg177:0002 and cseg134:21AF.
namespace {

const uint32 kPart10PanelLeft = 0x0000;
const uint32 kPart10Frm1 = 0xB400;
const uint32 kPart10Frm2 = 0xBC0A;
const uint32 kPart10Frm3 = 0xC5E2;
const uint32 kPart10Frm4 = 0xC786;
const uint32 kPart10Frm5 = 0xC8AE;

} // End of anonymous namespace

void IgorEngine::PART_10_EXEC_ACTION(int action) {
	// The action table order differs from the function listing order.
	switch (action) {
	case 101:
		ADD_DIALOGUE_TEXT(201, 1, 626);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 102:
		_roomObjectAreasTable[7].deltaLum = 3;
		for (int area = 11; area <= 12; ++area)
			_roomObjectAreasTable[area].area = 3;
		--_walkDataLastIndex;
		buildWalkPath(170, 97, 100, 73);
		_walkData[_walkDataLastIndex].frameNum = 0;
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true;
		waitForIgorMove();
		_currentPart = 70;
		break;
	case 103:
		ADD_DIALOGUE_TEXT(202, 1, 627);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 104:
		PART_10_ACTION_104_pickHamburger();
		break;
	case 105:
		ADD_DIALOGUE_TEXT(206, 1, 630);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 106:
		ADD_DIALOGUE_TEXT(208, 1, 632);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 107:
		ADD_DIALOGUE_TEXT(207, 1, 631);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 108:
		PART_10_ACTION_108();
		break;
	case 109:
		_currentPart = 40;
		break;
	default:
		warning("PART_10_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_10_ACTION_104_pickHamburger() {
	if (_objectsState[64] == 1) {
		ADD_DIALOGUE_TEXT(205, 1, 629);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		return;
	}

	ADD_DIALOGUE_TEXT(203, 2, 628);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();

	static const uint8 frameSelectors[] = { 0, 1, 0, 2 };
	for (int frame = 0; frame < 4; ++frame) {
		for (int y = 0; y < 49; ++y) {
			memcpy(_screenVGA + 0x595F + y * 320,
					_animFramesBuffer + kPart10Frm1 + frameSelectors[frame] * 0x682 + y * 34,
					34);
		}
		waitForTimer(31);
	}

	addObjectToInventory(17, 52);
	_objectsState[64] = 1;
}

void IgorEngine::PART_10_ACTION_108() {
	uint8 *walkTable = loadData(WLK_DecanatoA);
	int xPos = 329;
	int yPos = 0;
	int i = 1;
	do {
		if (compareGameTick(1, 16)) {
			for (int y = 0; y <= 143; ++y) {
				memcpy(_screenLayer2 + y * 320 + i * 8, _screenLayer1 + y * 320, 320 - i * 8);
				memcpy(_screenLayer2 + y * 320, _animFramesBuffer + kPart10PanelLeft + y * 320 + 320 - i * 8, i * 8);
			}
			if (i < 9) {
				xPos -= _walkScaleTable[0x8F9 + _walkCurrentFrame];
				assert(xPos >= 260);
				yPos = walkTable[xPos - 260];
				WalkData::setNextFrame(kFacingPositionLeft, _walkCurrentFrame);
			} else {
				_walkCurrentFrame = 0;
			}
			int dstOffset = (yPos - 50) * 320 + xPos - 330 + i * 8;
			for (int row = 0; row <= 49; ++row) {
				dstOffset += 320;
				for (int col = 0; col <= 29; ++col) {
					const uint8 color = _facingIgorFrames[kFacingPositionLeft - 1][_walkCurrentFrame * 1500 + row * 30 + col];
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
	_walkData[0].setPos(xPos + 5, yPos, kFacingPositionLeft, 0);
	_walkData[0].setDefaultScale();
	_currentPart = 110;
}

void IgorEngine::PART_10() {
	_gameState.enableLight = 1;

	loadActionData(DAT_Decanato);
	loadRoomData(PAL_DecanatoA, IMG_DecanatoA, BOX_DecanatoA, MSK_DecanatoA, TXT_DecanatoA);
	static const int frames1[] = { FRM_Decanato1, 0 };
	static const int frames2[] = { FRM_Decanato2, 0 };
	static const int frames3[] = { FRM_Decanato3, 0 };
	static const int frames4[] = { FRM_Decanato4, 0 };
	static const int frames5[] = { FRM_Decanato5, 0 };
	loadAnimData(frames1, kPart10Frm1);
	loadAnimData(frames2, kPart10Frm2);
	loadAnimData(frames3, kPart10Frm3);
	loadAnimData(frames4, kPart10Frm4);
	loadAnimData(frames5, kPart10Frm5);
	if (_objectsState[67] == 1) {
		for (int y = 0; y <= 7; ++y)
			memcpy(_screenLayer1 + 0x7DA1 + y * 320, _animFramesBuffer + kPart10Frm4 + y * 37, 37);
	}
	if (_objectsState[68] == 1) {
		for (int y = 0; y <= 14; ++y)
			memcpy(_screenLayer1 + 0x4E32 + y * 320, _animFramesBuffer + kPart10Frm3 + y * 14, 14);
	} else if (_objectsState[68] == 2) {
		for (int y = 0; y <= 14; ++y)
			memcpy(_screenLayer1 + 0x4E32 + y * 320, _animFramesBuffer + kPart10Frm3 + 210 + y * 14, 14);
	}

	// Preserve the complete left panel at ANM+0, then make the right panel
	// active. The original load order is unconditional.
	memcpy(_animFramesBuffer + kPart10PanelLeft, _screenLayer1, 46080);
	loadRoomData(PAL_DecanatoB, IMG_DecanatoB, BOX_DecanatoB, MSK_DecanatoB, TXT_DecanatoB);
	static const int hamburgerAnimation[] = { ANM_DecanatoHamburger, 0 };
	loadAnimData(hamburgerAnimation, kPart10Frm1);

	SET_PAL_240_48_1();
	SET_PAL_208_96_1();

	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_10_EXEC_ACTION);
	_roomDataOffsets = PART_10_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);

	// A normal entry presents the freshly loaded right panel. On state 102
	// handoff, the original jumps directly to the walk-index setup instead,
	// preserving the completed pan (including Igor) already in screen VGA.

	if (_currentPart != 102) {
		memcpy(_screenVGA, _screenLayer1, 46080);
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
	}
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;

	if (_currentPart == 100) {
		// Enter at the right edge and walk left into the room.
		_walkData[0].setPos(319, 79, kFacingPositionLeft, 0);
		_walkData[0].setDefaultScale();
		_walkData[0].clipWidth = 15;
		_walkDataLastIndex = 0;
		buildWalkPath(319, 79, 288, 84);
		_walkData[_walkDataLastIndex].frameNum = 0;
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true;
		waitForIgorMove();
	} else if (_currentPart == 101) {
		// Enter from the adjacent panel and walk right into the room.
		_walkData[0].setPos(136, 86, kFacingPositionRight, 0);
		_walkData[0].setDefaultScale();
		_walkDataLastIndex = 0;
		buildWalkPath(136, 86, 171, 97);
		_walkData[_walkDataLastIndex].frameNum = 0;
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true;
		waitForIgorMove();
	}
	// State 102 deliberately performs no entry redraw: the completed pan has
	// already drawn Igor and stored his state in walk record 0.

	_roomObjectAreasTable[7].deltaLum = 0;
	for (int area = 11; area <= 12; ++area)
		_roomObjectAreasTable[area].area = 0;

	enterPartLoop();
	while (_currentPart >= 100 && _currentPart <= 102) {
		runPartLoop();
	}
	leavePartLoop();
	if (_currentPart != 110)
		fadeOut(624);
}

} // End of namespace Igor
