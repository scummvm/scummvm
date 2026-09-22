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

const uint32 kPart100PanelLeft = 0x0000; // cseg175:2809-2818
const uint32 kPart100Frm1 = 0xB400;      // cseg177:0002
const uint32 kPart100Frm2 = 0xBC0A;      // cseg177:0002
const uint32 kPart100Frm3 = 0xC5E2;      // cseg177:0002
const uint32 kPart100Frm4 = 0xC786;      // cseg177:0002
const uint32 kPart100Frm5 = 0xC8AE;      // cseg177:0002

} // End of anonymous namespace

void IgorEngine::PART_100_EXEC_ACTION(int action) {
	// The action-to-function table is constructed at cseg175:08B2-09EA;
	// its order differs from the disassembler's function listing order.
	switch (action) {
	case 101: // cseg175:08BC-08DA -> sub_175_00F2
		ADD_DIALOGUE_TEXT(201, 1, 626);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 102:
		_roomObjectAreasTable[7].deltaLum = 3; // cseg175:077A; s3:0xDC7E
		for (int area = 11; area <= 12; ++area)
			_roomObjectAreasTable[area].area = 3; // cseg175:077F-07A1; s3:[area*5-9130]
		--_walkDataLastIndex; // cseg175:07CA
		buildWalkPath(170, 97, 100, 73); // cseg175:07CE-07D7
		_walkData[_walkDataLastIndex].frameNum = 0; // cseg175:07DC-07E7
		_walkDataCurrentIndex = 1; // cseg175:07E9
		_gameState.igorMoving = true; // cseg175:07EE
		waitForIgorMove(); // cseg175:07F3-0873
		_currentPart = 70; // cseg175:0875
		break;
	case 103: // cseg175:0900-091E -> sub_175_011F
		ADD_DIALOGUE_TEXT(202, 1, 627);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 104:
		PART_100_ACTION_104(); // cseg175:0922-0940 -> sub_175_014C
		break;
	case 105: // cseg175:0944-0962 -> sub_175_026F
		ADD_DIALOGUE_TEXT(206, 1, 630);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 106: // cseg175:0966-0984 -> sub_175_02C9
		ADD_DIALOGUE_TEXT(208, 1, 632);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 107: // cseg175:0988-09A6 -> sub_175_029C
		ADD_DIALOGUE_TEXT(207, 1, 631);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 108: // cseg175:09AA-09C8 -> sub_175_02F6
		PART_100_ACTION_108();
		break;
	case 109: // cseg175:09CC-09EA -> sub_175_055B
		_currentPart = 40;
		break;
	default:
		warning("PART_100_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_100_ACTION_104() {
	if (_objectsState[64] == 1) { // cseg175:015A; s3:0x862
		ADD_DIALOGUE_TEXT(205, 1, 629); // cseg175:0161-017D
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		return;
	}

	ADD_DIALOGUE_TEXT(203, 2, 628); // cseg175:0185-01A1
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue(); // cseg175:01A6

	static const uint8 frameSelectors[] = { 0, 1, 0, 2 }; // cseg175:01CE-01D8; s3:0x1D2-0x1D5
	for (int frame = 0; frame < 4; ++frame) { // cseg175:01AB-021B
		for (int y = 0; y < 49; ++y) { // cseg175:01BC-0208
			memcpy(_screenVGA + 0x595F + y * 320, // cseg175:01B7-01F7
					_animFramesBuffer + kPart100Frm1 + frameSelectors[frame] * 0x682 + y * 34, // cseg175:01C7-01E4
					34); // cseg175:01FC-01FE
		}
		waitForTimer(31); // cseg175:020A-0214
	}

	// TODO: add inventory object 17 at slot s3:0x90E; the fork has no
	// inventory-state API (cseg175:021D-024E).
	playSound(51, 1); // cseg175:0253-0257
	_objectsState[64] = 1; // cseg175:0261; s3:0x862
}


// Scroll to the LEFT: the left panel (PART100_bg) slides in from the left edge,
// the active right panel (C1) exits right. Mirrors PART_06_ACTION_102
// (part_6.cpp); the incoming strip is read straight from the stashed left panel
// in the ANM buffer (row stride 320). layer1 is untouched by the pan, so it
// still holds the outgoing right panel afterwards and can be kept for a later
// scroll back.
// void IgorEngine::PART_100_SCROLL_LEFT_ACTION() {
// 	for (int i = 1; i < 29; ++i) {
// 		if (compareGameTick(1, 16)) {
// 			for (int y = 0; y <= 143; ++y) {
// 				memcpy(_screenLayer2 + y * 320 + i * 8, _screenLayer1 + y * 320, 320 - i * 8);
// 				memcpy(_screenLayer2 + y * 320, _animFramesBuffer + ANM_PANEL_A + y * 320, i * 8);
// 			}
// 			memcpy(_screenVGA, _screenLayer2, 46080);
// 		}
// 		// PART_100_UPDATE_ROOM_BACKGROUND();
// 		waitForTimer();
// 	}
// 	// Cut over: the left panel becomes the active room (sub_175_066B position).
// 	PART_100_STASH_FAR_PANEL();			// keep the outgoing right panel for the way back
// 	_currentPart = 101;
// 	loadRoomData(PAL_OutsideAdministrationBuildingA, IMG_OutsideAdministrationBuildingA,
// 				 BOX_OutsideAdministrationBuildingA, MSK_OutsideAdministrationBuildingA, 0);
// 	_walkDataCurrentIndex = 1;
// 	_walkDataLastIndex = 1;
// 	_walkData[1].setPos(136, 86, kFacingPositionRight, 0);
// 	_walkData[1].setDefaultScale();
// 	memcpy(_screenVGA, _screenLayer1, 46080);
// 	PART_100_DRAW_IGOR();
// }

// // Scroll to the RIGHT (back to C1 / the arrival panel): the active left panel
// // slides left while the right panel's strip enters from the right edge (mirrors
// // PART_05_ACTION_102, part_5.cpp), sourced from the stashed far panel.
// void IgorEngine::PART_100_SCROLL_RIGHT_ACTION() {
// 	for (int i = 1; i < 29; ++i) {
// 		if (compareGameTick(1, 16)) {
// 			for (int y = 0; y <= 143; ++y) {
// 				memcpy(_screenLayer2 + y * 320, _screenLayer1 + y * 320 + i * 8, 320 - i * 8);
// 				memcpy(_screenLayer2 + y * 320 + 320 - i * 8, g_part100FarPanel + y * 320 + 320 - i * 8, i * 8);
// 			}
// 			memcpy(_screenVGA, _screenLayer2, 46080);
// 		}
// 		PART_100_UPDATE_ROOM_BACKGROUND();
// 		waitForTimer();
// 	}
// 	// Cut over: back on the arrival panel (sub_175_056D position).
// 	_currentPart = 100;
// 	loadRoomData(PAL_OutsideAdministrationBuildingB, IMG_OutsideAdministrationBuildingB,
// 				 BOX_OutsideAdministrationBuildingB, MSK_OutsideAdministrationBuildingB, 0);
// 	_walkDataCurrentIndex = 1;
// 	_walkDataLastIndex = 1;
// 	_walkData[1].setPos(319, 79, kFacingPositionLeft, 0);
// 	_walkData[1].setDefaultScale();
// 	memcpy(_screenVGA, _screenLayer1, 46080);
// 	PART_100_DRAW_IGOR();
// }
void IgorEngine::PART_100_ACTION_108() {
	uint8 *walkTable = loadData(WLK_DecanatoA); // cseg175:03E7-0409
	int xPos = 329; // cseg175:030D
	int yPos = 0;
	int i = 1; // cseg175:0312
	do {
		if (compareGameTick(1, 16)) { // cseg175:0316-032C
			for (int y = 0; y <= 143; ++y) { // cseg175:032F-03C4
				memcpy(_screenLayer2 + y * 320 + i * 8, _screenLayer1 + y * 320, 320 - i * 8);
				memcpy(_screenLayer2 + y * 320, _animFramesBuffer + kPart100PanelLeft + y * 320 + 320 - i * 8, i * 8);
			}
			if (i < 9) { // cseg175:03C7-0421
				xPos -= _walkScaleTable[0x8F9 + _walkCurrentFrame];
				assert(xPos >= 260);
				yPos = walkTable[xPos - 260];
				WalkData::setNextFrame(kFacingPositionLeft, _walkCurrentFrame);
			} else {
				_walkCurrentFrame = 0; // cseg175:0427
			}
			int dstOffset = (yPos - 50) * 320 + xPos - 330 + i * 8; // cseg175:042C-0448
			for (int row = 0; row <= 49; ++row) { // cseg175:044B-04CB
				dstOffset += 320;
				for (int col = 0; col <= 29; ++col) {
					const uint8 color = _facingIgorFrames[kFacingPositionLeft - 1][_walkCurrentFrame * 1500 + row * 30 + col];
					if (color != 0)
						_screenLayer2[dstOffset + col] = color;
				}
			}
			memcpy(_screenVGA, _screenLayer2, 46080); // cseg175:04CD-04E2
			++i;
		}
		waitForTimer(); // cseg175:04E6-0505
	} while (i != 41); // cseg175:0507-050D
	free(walkTable);
	_walkData[0].setPos(xPos + 5, yPos, kFacingPositionLeft, 0); // cseg175:0510-053A
	_walkData[0].setDefaultScale();
	_currentPart = 110; // cseg175:0553
}

void IgorEngine::PART_100() {
	_gameState.enableLight = 1;

	// cseg175:27C3-27D6 copies the 0x18C9-byte DAT; cseg175:27DB then
	// loads the left panel, and cseg175:27E0 loads all five frame blobs.
	loadActionData(DAT_Decanato);
	loadRoomData(PAL_DecanatoA, IMG_DecanatoA, BOX_DecanatoA, MSK_DecanatoA, TXT_DecanatoA);
	static const int frames1[] = { FRM_Decanato1, 0 };
	static const int frames2[] = { FRM_Decanato2, 0 };
	static const int frames3[] = { FRM_Decanato3, 0 };
	static const int frames4[] = { FRM_Decanato4, 0 };
	static const int frames5[] = { FRM_Decanato5, 0 };
	loadAnimData(frames1, kPart100Frm1);
	loadAnimData(frames2, kPart100Frm2);
	loadAnimData(frames3, kPart100Frm3);
	loadAnimData(frames4, kPart100Frm4);
	loadAnimData(frames5, kPart100Frm5);
	if (_objectsState[67] == 1) { // cseg175:27E5-27EC; s3:0x865
		for (int y = 0; y <= 7; ++y)
			memcpy(_screenLayer1 + 0x7DA1 + y * 320, _animFramesBuffer + kPart100Frm4 + y * 37, 37); // cseg175:0002-0051
	}
	if (_objectsState[68] == 1) { // cseg175:27F1-27F8; s3:0x866
		for (int y = 0; y <= 14; ++y)
			memcpy(_screenLayer1 + 0x4E32 + y * 320, _animFramesBuffer + kPart100Frm3 + y * 14, 14); // cseg175:0052-00A1
	} else if (_objectsState[68] == 2) { // cseg175:27FD-2804
		for (int y = 0; y <= 14; ++y)
			memcpy(_screenLayer1 + 0x4E32 + y * 320, _animFramesBuffer + kPart100Frm3 + 210 + y * 14, 14); // cseg175:00A2-00F1
	}

	// Preserve the complete left panel at ANM+0, then make the right panel
	// active. This load order is unconditional in cseg175:27DB-2845.
	memcpy(_animFramesBuffer + kPart100PanelLeft, _screenLayer1, 46080); // cseg175:2809-2818
	loadRoomData(PAL_DecanatoB, IMG_DecanatoB, BOX_DecanatoB, MSK_DecanatoB, TXT_DecanatoB);

	SET_PAL_240_48_1();
	SET_PAL_208_96_1();

	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_100_EXEC_ACTION);
	_roomDataOffsets = PART_100_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143); // cseg175 room mask is 320x144

	// A normal entry presents the freshly loaded right panel. On the part 102
	// handoff, the original jumps directly to the walk-index setup instead,
	// preserving the completed pan (including Igor) already in screen VGA.
	// cseg175:285B-2869, cseg175:295F.
	if (_currentPart != 102) {
		memcpy(_screenVGA, _screenLayer1, 46080); // cseg175:28EA-28F9
		_currentAction.verb = kVerbWalk;           // cseg175:2944-2952
		fadeIn(768);                              // cseg175:2957-295A
	}
	_walkDataLastIndex = 1;                   // cseg175:295F
	_walkDataCurrentIndex = 1;                // cseg175:2964

	if (_currentPart == 100) {
		// sub_175_056D: enter at (319,79), facing left, walk to (288,84).
		_walkData[0].setPos(319, 79, kFacingPositionLeft, 0); // cseg175:0580-05B2
		_walkData[0].setDefaultScale();
		_walkDataLastIndex = 0;
		buildWalkPath(319, 79, 288, 84); // cseg175:05C1-05CB
		_walkData[_walkDataLastIndex].frameNum = 0; // cseg175:05D0-05DB
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true; // cseg175:05E2
		waitForIgorMove();
	} else if (_currentPart == 101) {
		// sub_175_066B: enter at (136,86), facing right, walk to (171,97).
		_walkData[0].setPos(136, 86, kFacingPositionRight, 0); // cseg175:0686-06B7
		_walkData[0].setDefaultScale();
		_walkDataLastIndex = 0;
		buildWalkPath(136, 86, 171, 97); // cseg175:06C6-06D0
		_walkData[_walkDataLastIndex].frameNum = 0; // cseg175:06D5-06E0
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true; // cseg175:06E7
		waitForIgorMove();
	}
	// Part 102 deliberately performs no entry redraw: the completed pan has
	// already drawn Igor and stored his state in walk record 0.
	// cseg175:2969-2981; cseg176:06AD-06F0.

	_roomObjectAreasTable[7].deltaLum = 0; // cseg175:087D-0887; s3:0xDC7E
	for (int area = 11; area <= 12; ++area)
		_roomObjectAreasTable[area].area = 0; // cseg175:088C-08AE; s3:[area*5-9130]

	enterPartLoop();
	while (_currentPart >= 100 && _currentPart <= 102) { // cseg175:29AA-29B5
		runPartLoop();
	}
	leavePartLoop();
}

} // End of namespace Igor
