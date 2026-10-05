/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "igor/igor.h"

namespace Igor {

void IgorEngine::PART_02_START_DIALOGUE(int text, int count, int sound) {
	ADD_DIALOGUE_TEXT(text, count, sound);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
}

void IgorEngine::PART_02_SEARCH_TRUNK() {
	for (int frame = 0; frame <= 2; ++frame) { // cseg203:04E8-0653
		PART_00_ANIMATE_RAW(0x3C0 + frame * 0x715, 0, 0, 0x715,
				37, 49, 0x608C, 0, -1, 0); // cseg203:0509-0637
		if (frame > 0)
			waitForTimer(127); // cseg203:0639-064A
	}
}

void IgorEngine::PART_02_APPLY_OBJECT_STATE(int num) {
	if (num == 1 || num == 255) {
		// s3:0x084C maps to _objectsState[16]. cseg203:22CF-2303
		// redraws the closed/open trunk into the persistent room background.
		const int srcOffset = _objectsState[16] == 0 ? 0x2E3E : 0x30FC;
		for (int y = 0; y < 18; ++y) // cseg203:00EF-018E
			memcpy(_screenLayer1 + 0x748A + y * 320,
					_animFramesBuffer + srcOffset + y * 39, 39);
		_roomActionsTable[252] = _objectsState[16] == 0 ? 6 : 4; // cseg203:22DB-2303
	}
	if ((num == 2 || num == 255) && _objectsState[17] == 1) {
		_roomObjectAreasTable[17].object = 4; // s3:0xDCAC; cseg203:2317
		_roomObjectAreasTable[19].object = 4; // s3:0xDCB6; cseg203:231C
	}
	if (num == 3 || num == 255) {
		if (_objectsState[18] == 1) { // s3:0x084E; cseg203:232D-2340
			for (int y = 0; y < 50; ++y) // cseg203:0002-004D
				memcpy(_screenLayer1 + 0x50D6 + y * 320,
						_animFramesBuffer + y * 14, 14);
		}
		_roomObjectAreasTable[9].object = _objectsState[18] == 0 ? 0 : 6; // s3:0xDC84
	}
	if (num == 4 || num == 255) {
		if (_objectsState[19] == 0) { // s3:0x084F; cseg203:2351-2372
			_roomObjectAreasTable[12].object = _objectsState[18] == 1 ? 6 : 0; // s3:0xDC93
		} else {
			for (int y = 0; y < 22; ++y) // cseg203:004E-009D
				memcpy(_screenLayer1 + 0x5357 + y * 320,
						_animFramesBuffer + 0x2BC + y * 10, 10);
			_roomObjectAreasTable[12].object = 7; // s3:0xDC93
		}
	}
	if (num == 5 || num == 255) {
		if (_objectsState[20] == 0) {
			_roomObjectAreasTable[13].object = _objectsState[19] == 1 ? 7 : (_objectsState[18] == 1 ? 6 : 0);
		} else {
			for (int y = 0; y < 10; ++y) // cseg203:009E-00EE
				memcpy(_screenLayer1 + 0x599B + y * 320,
						_animFramesBuffer + 0x398 + y * 4, 4);
			_roomObjectAreasTable[13].object = 8; // s3:0xDC98; cseg203:23A9-23AE
		}
	}
	if (num == 6 || num == 255) {
		if (_objectsState[21] == 0)
			_roomObjectAreasTable[14].object = _objectsState[20] == 1 ? 8 : (_objectsState[19] == 1 ? 7 : (_objectsState[18] == 1 ? 6 : 0));
	}
	if ((num == 8 || num == 255) && _objectsState[23] == 1)
		_roomObjectAreasTable[13].object = 8;
}

void IgorEngine::PART_02_EXEC_ACTION(int action) {
	debugC(9, kDebugGame, "PART_02_EXEC_ACTION %d", action);
	switch (action) {
	case 101: // open trunk; cseg203:018F-0337
		if (_objectsState[16] == 1) { // s3:0x084C; cseg203:019D-01A8
			PART_02_START_DIALOGUE(201, 1, 108); // indirect s3:0x5964
			break;
		}
		PART_00_ANIMATE_RAW(0x18FF, 1, 2, 0x715, 37, 49, 0x608C,
				127, 1, 7); // cseg203:01AB-0327
		_objectsState[16] = 1; // s3:0x084C; cseg203:032A
		PART_02_APPLY_OBJECT_STATE(1); // cseg203:032F-0331
		break;
	case 102: // close trunk; cseg203:0338-04D9
		if (_objectsState[16] == 0) { // s3:0x084C; cseg203:0346-0351
			PART_02_START_DIALOGUE(201, 1, 108); // indirect s3:0x5970
			break;
		}
		playSound(8, 1); // cseg203:0354-0358
		PART_00_ANIMATE_RAW(0x18FF, 1, 0, 0x715, 37, 49, 0x608C,
				127, -1, 0); // cseg203:035D-04C9
		_objectsState[16] = 0; // s3:0x084C; cseg203:04CC
		PART_02_APPLY_OBJECT_STATE(1); // cseg203:04D1-04D3
		break;
	case 103: { // hammer the nail; cseg203:0658-0837
		// The selector bytes at dseg231:0x0234-0x0248 alternate frames
		// 0/1 and finish on frame 2 (indexed from s3:0x0233).
		for (int step = 1; step <= 21; ++step) { // cseg203:0666-07FA
			const int frame = step == 21 ? 2 : ((step + 1) & 1);
			PART_00_ANIMATE_RAW(0x33BA + frame * 0x4B0, 0, 0, 0x4B0,
					24, 50, 0x5989, 0, -1, 0); // cseg203:0688-07BE
			if ((step & 1) != 0 && step < 21)
				playSound(9, 1); // cseg203:07C0-07E2
			waitForTimer(63); // cseg203:07E7-07F1
		}
		_objectsState[20] = 1; // s3:0x0850; cseg203:07FD
		_objectsState[21] = 0; // s3:0x0851; cseg203:0802
		PART_02_APPLY_OBJECT_STATE(5); // cseg203:0807-0809
		PART_02_APPLY_OBJECT_STATE(6); // cseg203:080E-0810
		PART_02_START_DIALOGUE(217, 1, 121); // cseg203:0815-0831
		break;
	}
	case 104: { // use the pick on the crack; cseg203:0838-0A2C
		if (_objectsState[23] == 1) { // s3:0x0853; cseg203:0846-0851
			EXEC_MAIN_ACTION(2); // s3:0x5944 = cseg222:2A76
			break;
		}
		// The selector bytes at dseg231:0x024A-0x025E alternate frames
		// 0/1 and finish on frame 2 (indexed from s3:0x0249).
		for (int step = 1; step <= 21; ++step) { // cseg203:0854-09E8
			const int frame = step == 21 ? 2 : ((step + 1) & 1);
			PART_00_ANIMATE_RAW(0x41CA + frame * 0x658, 0, 0, 0x658,
					29, 56, 0x5349, 0, -1, 0); // cseg203:0876-09AC
			if ((step & 1) != 0 && step < 21)
				playSound(10, 1); // cseg203:09AE-09D0
			waitForTimer(63); // cseg203:09D5-09DF
		}
		_objectsState[19] = 1; // s3:0x084F; cseg203:09EB
		_objectsState[20] = 0; // s3:0x0850; cseg203:09F0
		PART_02_APPLY_OBJECT_STATE(4); // cseg203:09F5-09F7
		PART_02_APPLY_OBJECT_STATE(5); // cseg203:09FC-09FE
		PART_02_APPLY_OBJECT_STATE(6); // cseg203:0A03-0A05
		PART_02_START_DIALOGUE(218, 1, 122); // cseg203:0A0A-0A26
		break;
	}
	case 105: // look at nail
		PART_02_START_DIALOGUE(209, 1, 115);
		_objectsState[22] = 1;
		break;
	case 106: // look in trunk; cseg203:0A87-0D70
		if (_objectsState[16] == 0) { // s3:0x084C; cseg203:0A91-0AB9
			PART_02_START_DIALOGUE(201, 1, 108);
			break;
		}
		if (_inventoryInfo[44] == 0 && _inventoryInfo[45] == 0) { // s3:0x0906/0907
			if (_objectsState[22] == 0) { // s3:0x0852; cseg203:0AD0-0B0A
				PART_02_START_DIALOGUE(202, 2, 109);
			} else {
				PART_02_SEARCH_TRUNK(); // cseg203:04DA-0657
				PART_02_START_DIALOGUE(204, 1, 111); // cseg203:0B14-0B30
				addObjectToInventory(9, 44); // s3:0x0906; cseg203:0B35-0B6F
				PART_02_APPLY_OBJECT_STATE(255); // cseg203:0B74-0B7B
			}
			break;
		}
		if (_inventoryInfo[44] != 0 && _inventoryInfo[45] == 0) { // cseg203:0B83-0C71
			if (_objectsState[20] == 0) { // s3:0x0850
				PART_02_START_DIALOGUE(202, 2, 109);
			} else {
				PART_02_SEARCH_TRUNK(); // cseg203:0BD7
				PART_02_START_DIALOGUE(205, 1, 112); // cseg203:0BDC-0BF8
				removeObjectFromInventory(44); // s3:0x0906; cseg203:0BFD-0C17
				playSound(63, 1); // cseg203:0C13-0C17
				addObjectToInventory(10, 45); // s3:0x0907; cseg203:0C1C-0C60
				PART_02_APPLY_OBJECT_STATE(255); // cseg203:0C65-0C6C
			}
			break;
		}
		if (_inventoryInfo[44] == 0 && _inventoryInfo[45] != 0 &&
				_inventoryInfo[46] == 0 && _objectsState[23] == 0 &&
				_objectsState[19] != 0) { // cseg203:0C74-0CD9; s3:0x0908/0853/084F
			PART_02_SEARCH_TRUNK(); // cseg203:0CDB
			PART_02_START_DIALOGUE(206, 1, 113); // cseg203:0CE0-0CFC
			addObjectToInventory(11, 46); // s3:0x0908; cseg203:0D01-0D3B
			PART_02_APPLY_OBJECT_STATE(255); // cseg203:0D40-0D47
		} else if (_inventoryInfo[44] == 0 && _inventoryInfo[45] != 0 &&
				_inventoryInfo[46] == 0 && _objectsState[23] == 0) {
			PART_02_START_DIALOGUE(202, 2, 109); // cseg203:0CA3-0CD6
		} else {
			PART_02_START_DIALOGUE(207, 2, 114); // cseg203:0D4E-0D6A
		}
		break;
	case 107: // take butterfly net; cseg203:0D71-0F3E
		PART_00_ANIMATE_RAW(0x54D2, 0, 1, 0x405, 21, 49, 0x5A6D,
				127, -1, 0); // cseg203:0D7F-0EE3
		waitForTimer(127); // cseg203:0ED0-0EDA
		addObjectToInventory(7, 42); // s3:0x0904; cseg203:0EE6-0F20
		_objectsState[17] = 1; // s3:0x084D; cseg203:0F31
		PART_02_APPLY_OBJECT_STATE(2); // cseg203:0F36-0F38
		break;
	case 108: // Look at old junk
		PART_02_START_DIALOGUE(210, 1, 116);
		break;
	case 109: // look at portrait
		ADD_DIALOGUE_TEXT(225, 4, 128);
		ADD_DIALOGUE_TEXT(229, 1, 129);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 110: // look at hole
		PART_02_START_DIALOGUE(213, 1, 118);
		break;
	case 111:
		PART_02_START_DIALOGUE(214, 2, 119);
		break;
	case 112:
		PART_02_START_DIALOGUE(216, 1, 120);
		break;
	case 113: // place dynamite in the crack; cseg203:0FEB-11C1
		PART_00_ANIMATE_RAW(0x5CDC, 0, 2, 0x44C, 22, 50, 0x5989,
				127, -1, 0); // cseg203:0FF9-115D
		waitForTimer(127); // cseg203:114A-1154
		_objectsState[23] = 1; // s3:0x0853; cseg203:1160
		PART_02_APPLY_OBJECT_STATE(8); // cseg203:1165-1167
		removeObjectFromInventory(46); // s3:0x0908; cseg203:116C-11A6
		playSound(63, 1); // cseg203:11AB-11AF
		PART_02_APPLY_OBJECT_STATE(255); // cseg203:11B4-11BB
		break;
	case 114:
		PART_02_START_DIALOGUE(219, 1, 123);
		break;
	case 115:
		PART_02_START_DIALOGUE(217, 1, 121);
		break;
	case 116:
		PART_02_START_DIALOGUE(218, 1, 122);
		break;
	case 118: // exit through window
		if (_objectsState[24] == 1) { // s3:0854; cseg203:1989-19AC
			PART_02_START_DIALOGUE(220, 1, 124);
		} else {
			_objectsState[24] = 1;
			_currentPart = 11;
		}
		break;
	case 119:
		PART_00_ANIMATE_RAW(0xB488, 0, 1, 0x4B0, 24, 50, 0x5988,
				64, -1, 0); // cseg203:1CF8-1E68
		waitForTimer(64); // cseg203:1E57-1E61
		_currentPart = 1; // cseg203:1E6D
		break;
	case 120:
		_currentPart = 30;
		break;
	case 121: // look at shelf / take matches; cseg203:0F3F-0FEA
		if (_objectsState[25] == 1) { // s3:0x0855; cseg203:0F49
			PART_02_START_DIALOGUE(230, 1, 130);
		} else {
			PART_02_START_DIALOGUE(211, 2, 117); // cseg203:0F73-0F8F
			addObjectToInventory(8, 43); // s3:0x0905; cseg203:0F94-0FD3
			_objectsState[25] = 1; // s3:0x0855; cseg203:0FE4
		}
		break;
	case 117: // light the dynamite; cseg203:1447-197E
		if (_objectsState[23] == 0) { // s3:0x0853; cseg203:1455-1460
			EXEC_MAIN_ACTION(2); // s3:0x5944 = cseg222:2A76
		} else {
			// TODO: transcribe the explosion sequence from cseg203:1463-197E.
			// Keeping it disabled avoids inventing its scrolling, palette, and
			// state-12 handoff behavior.
			warning("PART_02_EXEC_ACTION action 117 explosion is not yet transcribed");
		}
		break;
	default:
		warning("PART_02_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_02() {
	_gameState.enableLight = 1;
	loadActionData(DAT_StudentDormitoryAttic);
	loadRoomData(PAL_StudentDormitoryAttic, IMG_StudentDormitoryAttic,
			BOX_StudentDormitoryAttic, MSK_StudentDormitoryAttic,
			TXT_StudentDormitoryAttic);
	static const int animFrames[] = {
		ANM_StudentDormitoryAttic1, ANM_StudentDormitoryAttic2,
		ANM_StudentDormitoryAttic3, ANM_StudentDormitoryAttic4,
		ANM_StudentDormitoryAttic5, ANM_StudentDormitoryAttic6,
		ANM_StudentDormitoryAttic7, ANM_StudentDormitoryAttic8,
		ANM_StudentDormitoryAttic9, ANM_StudentDormitoryAttic10,
		ANM_StudentDormitoryAttic11, ANM_StudentDormitoryAttic12,
		ANM_StudentDormitoryAttic13, ANM_StudentDormitoryAttic14, 0
	};
	loadAnimData(animFrames);
	_roomDataOffsets = PART_02_ROOM_DATA_OFFSETS;
	// clamps clicks to x 41..253 and y <= 143; clicks past the
	// horizontal edges also clamp to y >= 141/138.
	setRoomWalkBounds(41, 0, 253, 143, 141, 138);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_02_EXEC_ACTION);
	PART_02_APPLY_OBJECT_STATE(255);
	memcpy(_screenVGA, _screenLayer1, 46080);

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		switch (_currentPart) {
		case 20:
			fadeIn(768); // cseg203:45AB-45AE
			// Igor climbs through the window on the first attic entrance. Three
			// 33x57 frames at animation offset 0x69C0 are drawn at 0x4B26.
			PART_00_ANIMATE_RAW(0x69C0, 0, 2, 0x759, 33, 57, 0x4B26,
					31, -1, 0); // cseg203:11C2-1338; called at cseg203:45C4
			waitForTimer(31); // cseg203:1321-132B
			_walkData[0].setPos(57, 117, kFacingPositionRight, 0);
			break;
		case 21:
			fadeIn(768);
			_walkData[0].setPos(300, 136, kFacingPositionLeft, 0);
			break;
		case 22:
			// cseg203:4319-4332 restores the live palette from the saved one
			// before the walk setup below, the same copy state 11 performs at
			// cseg200:1964. Without it the part-1 fadeOut leaves the attic black.
			memcpy(_currentPalette, _paletteBuffer, 768); // cseg203:4320-432D
			// cseg221:0x21DA is setPalette(), as identified by the original-runtime
			// trap table in reference/cyxx/igor/game.cpp; cseg203:4332.
			updatePalette(768);
			_walkData[0].setPos(49, 118, kFacingPositionBack, 0); // cseg203:4337-4348
			break;
		case 23:
			_walkData[0].setPos(215, 134, kFacingPositionRight, 0);
			break;
		case 24:
			_walkData[0].setPos(211, 120, kFacingPositionFront, 0);
			fadeIn(768);
			break;
		}
		_walkData[0].clipSkipX = 1;
		_walkData[0].clipWidth = 30;
		_walkData[0].scaleWidth = 50;
		_walkData[0].xPosChanged = 1;
		_walkData[0].dxPos = 0;
		_walkData[0].yPosChanged = 1;
		_walkData[0].dyPos = 0;
		_walkData[0].scaleHeight = 50;
		_walkDataCurrentIndex = 0;
		moveIgor(_walkData[0].posNum, _walkData[0].frameNum);
		_walkDataLastIndex = 1;
		_walkDataCurrentIndex = 1;
		if (_currentPart == 22)
			PART_02_EXEC_ACTION(118); // cseg203:438B
		// cseg222:0x2264 is showCursor() according to the original-runtime
		// trap table in reference/cyxx/igor/game.cpp. enterPartLoop() below
		// performs the equivalent shared-engine operation.
	}

	enterPartLoop();
	while (_currentPart >= 20 && _currentPart <= 24 && !_gameStateLoaded)
		runPartLoop();
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(_currentPart == kInvalidPart ? 768 : 624);
}

} // End of namespace Igor
