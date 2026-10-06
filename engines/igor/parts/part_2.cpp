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
	for (int frame = 0; frame <= 2; ++frame) {
		PART_00_animateRaw(0x3C0 + frame * 0x715, 0, 0, 0x715, 37, 49, 0x608C, 0, -1, 0);
		if (frame > 0)
			waitForTimer(127);
	}
}

void IgorEngine::PART_02_DRAW_FUSE_SPARK(int frame) {
	// Four 12x12 spark frames are stored at animation offset 0x7FCB and are
	// composited over the fuse at screen offset 0x5996. cseg203:1339-1443
	const uint8 *src = _animFramesBuffer + 0x7FCB + frame * 0x90;
	for (int y = 0; y < 12; ++y) {
		for (int x = 0; x < 12; ++x) {
			const int dstOffset = 0x5996 + y * 320 + x;
			const uint8 color = src[y * 12 + x];
			if (_screenVGA[dstOffset] >= 0xA0 && _screenVGA[dstOffset] <= 0xAF)
				_screenVGA[dstOffset] = 0xA0;
			else
				_screenVGA[dstOffset] = color == 0 ? _screenLayer1[dstOffset] : color;
		}
	}
}

void IgorEngine::PART_02_UPDATE_FUSE() {
	// The DOS loop chooses a new spark every eight timer units. The engine's
	// regular update step is exactly kTimerTicksCount (8) units.
	PART_02_DRAW_FUSE_SPARK(getRandomNumber(3)); // DOS random(4);
}

void IgorEngine::PART_02_WALK_WITH_FUSE(int srcX, int srcY, int dstX, int dstY) {
	--_walkDataLastIndex;
	buildWalkPath(srcX, srcY, dstX, dstY);
	_walkDataCurrentIndex = 1;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	_gameTicks = 0;
	do {
		if (compareGameTick(1, 16)) {
			if (_walkDataCurrentIndex > _walkDataLastIndex) {
				_gameState.igorMoving = false;
				_walkDataLastIndex = _walkDataCurrentIndex;
			}
			if (_gameState.igorMoving) {
				moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
				++_walkDataCurrentIndex;
			}
		}
		PART_02_UPDATE_FUSE();
		waitForTimer();
	} while (_gameState.igorMoving);
}

void IgorEngine::PART_02_APPLY_OBJECT_STATE(int num) {
	if (num == 1 || num == 255) {
		// redraws the closed/open trunk into the persistent room background.
		const int srcOffset = _objectsState[16] == 0 ? 0x2E3E : 0x30FC;
		for (int y = 0; y < 18; ++y)
			memcpy(_screenLayer1 + 0x748A + y * 320, _animFramesBuffer + srcOffset + y * 39, 39);
		_roomActionsTable[252] = _objectsState[16] == 0 ? 6 : 4;
	}
	if ((num == 2 || num == 255) && _objectsState[17] == 1) {
		_roomObjectAreasTable[17].object = 4;
		_roomObjectAreasTable[19].object = 4;
	}
	if (num == 3 || num == 255) {
		if (_objectsState[18] == 1) {
			for (int y = 0; y < 50; ++y)
				memcpy(_screenLayer1 + 0x50D6 + y * 320, _animFramesBuffer + y * 14, 14);
		}
		_roomObjectAreasTable[9].object = _objectsState[18] == 0 ? 0 : 6;
	}
	if (num == 4 || num == 255) {
		if (_objectsState[19] == 0) {
			_roomObjectAreasTable[12].object = _objectsState[18] == 1 ? 6 : 0;
		} else {
			for (int y = 0; y < 22; ++y)
				memcpy(_screenLayer1 + 0x5357 + y * 320,_animFramesBuffer + 0x2BC + y * 10, 10);
			_roomObjectAreasTable[12].object = 7;
		}
	}
	if (num == 5 || num == 255) {
		if (_objectsState[20] == 0) {
			_roomObjectAreasTable[13].object = _objectsState[19] == 1 ? 7 : (_objectsState[18] == 1 ? 6 : 0);
		} else {
			for (int y = 0; y < 10; ++y)
				memcpy(_screenLayer1 + 0x599B + y * 320, _animFramesBuffer + 0x398 + y * 4, 4);
			_roomObjectAreasTable[13].object = 8;
		}
	}
	if (num == 6 || num == 255) {
		if (_objectsState[21] == 0)
			_roomObjectAreasTable[14].object = _objectsState[20] == 1 ? 8 : (_objectsState[19] == 1 ? 7 : (_objectsState[18] == 1 ? 6 : 0));
	}
	if ((num == 8 || num == 255) && _objectsState[23] == 1) {
		_roomObjectAreasTable[13].object = 8;
		// Reuse the already-loaded
		// localized inventory-object name for dynamite.
		Common::strlcpy(_roomObjectNames[8], _globalObjectNames[11], sizeof(_roomObjectNames[8]));
		_roomActionsTable[457] = 0x20;
	}
}

void IgorEngine::PART_02_EXEC_ACTION(int action) {
	debugC(9, kDebugGame, "PART_02_EXEC_ACTION %d", action);
	switch (action) {
	case 101: // open trunk
		if (_objectsState[16] == 1) {
			PART_02_START_DIALOGUE(201, 1, 108);
			break;
		}
		PART_00_animateRaw(0x18FF, 1, 2, 0x715, 37, 49, 0x608C, 127, 1, 7);
		_objectsState[16] = 1;
		PART_02_APPLY_OBJECT_STATE(1);
		break;
	case 102: // close trunk
		if (_objectsState[16] == 0) {
			PART_02_START_DIALOGUE(201, 1, 108);
			break;
		}
		playSound(8, 1);
		PART_00_animateRaw(0x18FF, 1, 0, 0x715, 37, 49, 0x608C, 127, -1, 0);
		_objectsState[16] = 0;
		PART_02_APPLY_OBJECT_STATE(1);
		break;
	case 103: { // hammer the nail
		for (int step = 1; step <= 21; ++step) {
			const int frame = step == 21 ? 2 : ((step + 1) & 1);
			PART_00_animateRaw(0x33BA + frame * 0x4B0, 0, 0, 0x4B0,
					24, 50, 0x5989, 0, -1, 0);
			if ((step & 1) != 0 && step < 21)
				playSound(9, 1);
			waitForTimer(63);
		}
		_objectsState[20] = 1;
		_objectsState[21] = 0;
		PART_02_APPLY_OBJECT_STATE(5);
		PART_02_APPLY_OBJECT_STATE(6);
		PART_02_START_DIALOGUE(217, 1, 121);
		break;
	}
	case 104: { // use the pick on the crack; cseg203:0838-0A2C
		if (_objectsState[23] == 1) { // s3:0x0853; cseg203:0846-0851
			EXEC_MAIN_ACTION(2); // s3:0x5944 = cseg222:2A76
			break;
		}
		// The selector bytes at dseg231:0x024A-0x025E alternate frames
		// 0/1 and finish on frame 2 (indexed from s3:0x0249).
		for (int step = 1; step <= 21; ++step) {
			const int frame = step == 21 ? 2 : ((step + 1) & 1);
			PART_00_animateRaw(0x41CA + frame * 0x658, 0, 0, 0x658,
					29, 56, 0x5349, 0, -1, 0);
			if ((step & 1) != 0 && step < 21)
				playSound(10, 1);
			waitForTimer(63);
		}
		_objectsState[19] = 1; // s3:0x084F; cseg203:09EB
		_objectsState[20] = 0; // s3:0x0850; cseg203:09F0
		PART_02_APPLY_OBJECT_STATE(4);
		PART_02_APPLY_OBJECT_STATE(5);
		PART_02_APPLY_OBJECT_STATE(6);
		PART_02_START_DIALOGUE(218, 1, 122);
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
				PART_02_SEARCH_TRUNK();
				PART_02_START_DIALOGUE(204, 1, 111);
				addObjectToInventory(9, 44); // s3:0x0906; cseg203:0B35-0B6F
				PART_02_APPLY_OBJECT_STATE(255);
			}
			break;
		}
		if (_inventoryInfo[44] != 0 && _inventoryInfo[45] == 0) {
			if (_objectsState[20] == 0) { // s3:0x0850
				PART_02_START_DIALOGUE(202, 2, 109);
			} else {
				PART_02_SEARCH_TRUNK();
				PART_02_START_DIALOGUE(205, 1, 112);
				removeObjectFromInventory(44); // s3:0x0906; cseg203:0BFD-0C17
				playSound(63, 1);
				addObjectToInventory(10, 45); // s3:0x0907; cseg203:0C1C-0C60
				PART_02_APPLY_OBJECT_STATE(255);
			}
			break;
		}
		if (_inventoryInfo[44] == 0 && _inventoryInfo[45] != 0 &&
				_inventoryInfo[46] == 0 && _objectsState[23] == 0 &&
				_objectsState[19] != 0) {
			PART_02_SEARCH_TRUNK();
			PART_02_START_DIALOGUE(206, 1, 113);
			addObjectToInventory(11, 46); // s3:0x0908; cseg203:0D01-0D3B
			PART_02_APPLY_OBJECT_STATE(255);
		} else if (_inventoryInfo[44] == 0 && _inventoryInfo[45] != 0 &&
				_inventoryInfo[46] == 0 && _objectsState[23] == 0) {
			PART_02_START_DIALOGUE(202, 2, 109);
		} else {
			PART_02_START_DIALOGUE(207, 2, 114);
		}
		break;
	case 107: // take butterfly net; cseg203:0D71-0F3E
		PART_00_animateRaw(0x54D2, 0, 1, 0x405, 21, 49, 0x5A6D, 127, -1, 0);
		waitForTimer(127);
		addObjectToInventory(7, 42);
		_objectsState[17] = 1;
		PART_02_APPLY_OBJECT_STATE(2);
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
	case 113: // place dynamite in the crack
		PART_00_animateRaw(0x5CDC, 0, 2, 0x44C, 22, 50, 0x5989, 127, -1, 0);
		waitForTimer(127);
		_objectsState[23] = 1;
		PART_02_APPLY_OBJECT_STATE(8);
		removeObjectFromInventory(46);
		playSound(63, 1);
		PART_02_APPLY_OBJECT_STATE(255);
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
		if (_objectsState[24] == 1) {
			PART_02_START_DIALOGUE(220, 1, 124);
		} else {
			_objectsState[24] = 1;
			_currentPart = 11;
		}
		break;
	case 119:
		PART_00_animateRaw(0xB488, 0, 1, 0x4B0, 24, 50, 0x5988, 64, -1, 0);
		waitForTimer(64);
		_currentPart = 1;
		break;
	case 120:
		_currentPart = 30;
		break;
	case 121: // look at shelf / take matches
		if (_objectsState[25] == 1) { // s3:0x0855; cseg203:0F49
			PART_02_START_DIALOGUE(230, 1, 130);
		} else {
			PART_02_START_DIALOGUE(211, 2, 117);
			addObjectToInventory(8, 43);
			_objectsState[25] = 1;
		}
		break;
	case 117: // light the dynamite
		if (_objectsState[23] == 0) {
			EXEC_MAIN_ACTION(2);
		} else {
			memset(_screenVGA + 0xB400, 0, 0x4BFF);
			playSound(68, 1);
			PART_00_animateRaw(0x5CDC, 0, 2, 0x44C, 22, 50, 0x5989, 127, -1, 0);
			waitForTimer(127);


			PART_02_START_DIALOGUE(221, 1, 125);
			if (_gameState.talkMode == kTalkModeTextOnly)
				playSound(11, 1);
			_updateRoomBackground = &IgorEngine::PART_02_UPDATE_FUSE;
			waitForEndOfIgorDialogue();
			_updateRoomBackground = 0;

			memcpy(_screenVGA + _dialogueDirtyRectY, _screenTextLayer + 320 * 72, _dialogueDirtyRectSize);
			if (_gameState.talkMode != kTalkModeTextOnly)
				playSound(11, 1);

			PART_02_WALK_WITH_FUSE(211, 120, 0, 143);
			for (int elapsed = 0; elapsed < 255; elapsed += kTimerTicksCount) {
				PART_02_UPDATE_FUSE();
				waitForTimer();
			}
			for (int y = 0; y < 12; ++y) {
				memcpy(_screenVGA + 0x5996 + y * 320, _screenLayer1 + 0x5996 + y * 320, 12);
			}
			stopSound();
			waitForTimer(5 * 255);

			PART_02_START_DIALOGUE(222, 2, 126);
			waitForEndOfIgorDialogue();
			waitForTimer(2 * 255);
			stopDialogueSpeech();

			_gameState.dialogueTextRunning = false;

			// Return from the left edge to the dynamite wall.
			--_walkDataLastIndex;
			buildWalkPath(0, 143, 211, 120);
			_walkDataCurrentIndex = 1;
			_walkData[_walkDataLastIndex].frameNum = 0;
			_gameState.igorMoving = true;
			waitForIgorMove();

			_objectsState[18] = 1;
			_objectsState[19] = 0;
			_objectsState[23] = 0;
			_objectsState[11] = 1;
			PART_02_APPLY_OBJECT_STATE(3);
			PART_02_APPLY_OBJECT_STATE(4);
			PART_02_APPLY_OBJECT_STATE(5);
			memset(_currentPalette + 240 * 3, 0, 16 * 3);
			setPaletteRange(240, 255);
			_currentPart = 12;
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
	loadRoomData(PAL_StudentDormitoryAttic, IMG_StudentDormitoryAttic, BOX_StudentDormitoryAttic, MSK_StudentDormitoryAttic, TXT_StudentDormitoryAttic);
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
			fadeIn(768);
			// Igor climbs through the window on the first attic entrance.
			PART_00_animateRaw(0x69C0, 0, 2, 0x759, 33, 57, 0x4B26, 31, -1, 0);
			waitForTimer(31);
			_walkData[0].setPos(57, 117, kFacingPositionRight, 0);
			break;
		case 21:
			fadeIn(768);
			_walkData[0].setPos(300, 136, kFacingPositionLeft, 0);
			break;
		case 22:
			// restores the live palette from the saved one
			// before the walk setup below, the same copy state 11 happens
			// Without it the part-1 fadeOut leaves the attic black.
			memcpy(_currentPalette, _paletteBuffer, 768);
			updatePalette(768);
			_walkData[0].setPos(49, 118, kFacingPositionBack, 0);
			break;
		case 23:
			// State 12 fades the outside scene to black before returning here.
			// The original state-23 entrance clears the text/UI area and restores
			// the attic palette before drawing its post-explosion frame.
			memset(_screenVGA + 0xB400, 0, 0x4600);
			memcpy(_currentPalette, _paletteBuffer, 768);
			part_00_drawRawFrame(0x820B, 0, 0xA19, 47, 55, 0x6C3B);
			updatePalette(768);

			// Igor lies stunned, stands up, and shakes his head.
			static const uint8 postExplosionFrames[] = {
				0, 1, 2, 1, 2, 1, 2, 1, 2, 1,
				2, 1, 2, 1, 2, 1, 2, 1, 3, 4
			};
			for (uint i = 0; i < ARRAYSIZE(postExplosionFrames); ++i) {
				const int frame = postExplosionFrames[i];
				part_00_drawRawFrame(0x820B, frame, 0xA19, 47, 55, 0x6C3B);
				if (frame == 0)
					waitForTimer(2 * 255);
				else if (frame == 1 || frame == 2)
					waitForTimer(31);
				else
					waitForTimer(127);
			}
			PART_02_START_DIALOGUE(224, 1, 127);

			// Rebuild the panels erased above.
			drawVerbsPanel();
			drawInventory(_inventoryInfo[72], 0);
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
			PART_02_EXEC_ACTION(118);
	}

	enterPartLoop();
	while (_currentPart >= 20 && _currentPart <= 24 && !_gameStateLoaded)
		runPartLoop();
	leavePartLoop();
	debug("Exiting part loop, currentPart=%d", _currentPart);
	if (!_gameStateLoaded && !(_currentPart >= 10 && _currentPart <= 19)) {
		fadeOut(_currentPart == kInvalidPart ? 768 : 624);
	}
}

} // End of namespace Igor
