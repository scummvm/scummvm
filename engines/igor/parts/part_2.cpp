/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "igor/igor.h"

namespace Igor {

void IgorEngine::PART_02_SEARCH_TRUNK() {
	for (int frame = 0; frame <= 2; ++frame) {
		animateLitAnimFrames(0x3C0 + frame * 0x715, 0, 0, 0x715, 37, 49, 0x608C, 0, -1, 0);
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
	PART_02_DRAW_FUSE_SPARK(getRandomNumber(3));
}

void IgorEngine::PART_02_WALK_WHILE_FUSE_BURNS(int srcX, int srcY, int dstX, int dstY) {
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
		copyArea(_screenLayer1, 0x748A, 320, _animFramesBuffer + srcOffset, 39, 39, 18);
		_roomActionsTable[252] = _objectsState[16] == 0 ? 6 : 4;
	}
	if ((num == 2 || num == 255) && _objectsState[17] == 1) {
		_roomObjectAreasTable[17].object = 4;
		_roomObjectAreasTable[19].object = 4;
	}
	if (num == 3 || num == 255) {
		if (_objectsState[18] == 1) {
			copyArea(_screenLayer1, 0x50D6, 320, _animFramesBuffer, 14, 14, 50);
		}
		_roomObjectAreasTable[9].object = _objectsState[18] == 0 ? 0 : 6;
	}
	if (num == 4 || num == 255) {
		if (_objectsState[19] == 0) {
			_roomObjectAreasTable[12].object = _objectsState[18] == 1 ? 6 : 0;
		} else {
			copyArea(_screenLayer1, 0x5357, 320, _animFramesBuffer + 0x2BC, 10, 10, 22);
			_roomObjectAreasTable[12].object = 7;
		}
	}
	if (num == 5 || num == 255) {
		if (_objectsState[20] == 0) {
			_roomObjectAreasTable[13].object = _objectsState[19] == 1 ? 7 : (_objectsState[18] == 1 ? 6 : 0);
		} else {
			copyArea(_screenLayer1, 0x599B, 320, _animFramesBuffer + 0x398, 4, 4, 10);
			_roomObjectAreasTable[13].object = 8;
		}
	}
	if (num == 6 || num == 255) {
		if (_objectsState[21] == 0) // changes action on hammer after using it the first time
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
			igorSay(201, 1, 108);
			break;
		}
		animateLitAnimFrames(0x18FF, 1, 2, 0x715, 37, 49, 0x608C, 127, 1, 7);
		_objectsState[16] = 1;
		PART_02_APPLY_OBJECT_STATE(1);
		break;
	case 102: // close trunk
		if (_objectsState[16] == 0) {
			igorSay(201, 1, 108);
			break;
		}
		playSound(8, 1);
		animateLitAnimFrames(0x18FF, 1, 0, 0x715, 37, 49, 0x608C, 127, -1, 0);
		_objectsState[16] = 0;
		PART_02_APPLY_OBJECT_STATE(1);
		break;
	case 103: { // hammer the nail
		for (int step = 1; step <= 21; ++step) {
			const int frame = step == 21 ? 2 : ((step + 1) & 1);
			animateLitAnimFrames(0x33BA + frame * 0x4B0, 0, 0, 0x4B0,
					24, 50, 0x5989, 0, -1, 0);
			if ((step & 1) != 0 && step < 21)
				playSound(9, 1);
			waitForTimer(63);
		}
		_objectsState[20] = 1;
		_objectsState[21] = 0;
		PART_02_APPLY_OBJECT_STATE(5);
		PART_02_APPLY_OBJECT_STATE(6);
		igorSay(217, 1, 121);
		break;
	}
	case 104: { // use the pick on the crack; cseg203:0838-0A2C
		if (_objectsState[23] == 1) {
			EXEC_MAIN_ACTION(2);
			break;
		}

		for (int step = 1; step <= 21; ++step) {
			const int frame = step == 21 ? 2 : ((step + 1) & 1);
			animateLitAnimFrames(0x41CA + frame * 0x658, 0, 0, 0x658,
					29, 56, 0x5349, 0, -1, 0);
			if ((step & 1) != 0 && step < 21)
				playSound(10, 1);
			waitForTimer(63);
		}
		_objectsState[19] = 1;
		_objectsState[20] = 0;
		PART_02_APPLY_OBJECT_STATE(4);
		PART_02_APPLY_OBJECT_STATE(5);
		PART_02_APPLY_OBJECT_STATE(6);
		igorSay(218, 1, 122);
		break;
	}
	case 105: // look at nail
		igorSay(209, 1, 115);
		_objectsState[22] = 1;
		break;
	case 106: // look in trunk
		if (_objectsState[16] == 0) {
			igorSay(201, 1, 108);
			break;
		}
		if (_inventoryInfo[44] == 0 && _inventoryInfo[45] == 0) {
			if (_objectsState[22] == 0) {
				igorSay(202, 2, 109);
			} else {
				PART_02_SEARCH_TRUNK();
				igorSay(204, 1, 111);
				addObjectToInventory(9, 44);
				PART_02_APPLY_OBJECT_STATE(255);
			}
			break;
		}
		if (_inventoryInfo[44] != 0 && _inventoryInfo[45] == 0) {
			if (_objectsState[20] == 0) {
				igorSay(202, 2, 109);
			} else {
				PART_02_SEARCH_TRUNK();
				igorSay(205, 1, 112);
				removeObjectFromInventory(44);
				playSound(63, 1);
				addObjectToInventory(10, 45);
				PART_02_APPLY_OBJECT_STATE(255);
			}
			break;
		}
		if (_inventoryInfo[44] == 0 && _inventoryInfo[45] != 0 &&
				_inventoryInfo[46] == 0 && _objectsState[23] == 0 &&
				_objectsState[19] != 0) {
			PART_02_SEARCH_TRUNK();
			igorSay(206, 1, 113);
			addObjectToInventory(11, 46);
		} else if (_inventoryInfo[44] == 0 && _inventoryInfo[45] != 0 &&
				_inventoryInfo[46] == 0 && _objectsState[23] == 0) {
			igorSay(202, 2, 109);
		} else {
			igorSay(207, 2, 114);
		}
		break;
	case 107: // take butterfly net
		animateLitAnimFrames(0x54D2, 0, 1, 0x405, 21, 49, 0x5A6D, 127, -1, 0);
		waitForTimer(127);
		addObjectToInventory(7, 42);
		_objectsState[17] = 1;
		PART_02_APPLY_OBJECT_STATE(2);
		break;
	case 108: // Look at old junk
		igorSay(210, 1, 116);
		break;
	case 109: // look at portrait
		igorSay({ { 225, 4, 128 }, { 229, 1, 129 } });
		break;
	case 110: // look at hole
		igorSay(213, 1, 118);
		break;
	case 111: // look at big crack
		igorSay(214, 2, 119);
		break;
	case 112:
		igorSay(216, 1, 120);
		break;
	case 113: // place dynamite in the crack
		animateLitAnimFrames(0x5CDC, 0, 2, 0x44C, 22, 50, 0x5989, 127, -1, 0);
		waitForTimer(127);
		_objectsState[23] = 1;
		PART_02_APPLY_OBJECT_STATE(8);
		removeObjectFromInventory(46);
		playSound(63, 1);
		PART_02_APPLY_OBJECT_STATE(255);
		break;
	case 114:
		igorSay(219, 1, 123);
		break;
	case 115:
		igorSay(217, 1, 121);
		break;
	case 116:
		igorSay(218, 1, 122);
		break;
	case 118: // exit through window
		if (_objectsState[24] == 1) {
			igorSay(220, 1, 124);
		} else {
			_objectsState[24] = 1;
			_currentPart = 11;
		}
		break;
	case 119:
		animateLitAnimFrames(0xB488, 0, 1, 0x4B0, 24, 50, 0x5988, 64, -1, 0);
		waitForTimer(64);
		_currentPart = 1;
		break;
	case 120:
		_currentPart = 30;
		break;
	case 121: // look at shelf / take matches
		if (_objectsState[25] == 1) {
			igorSay(230, 1, 130);
		} else {
			igorSay(211, 2, 117);
			addObjectToInventory(8, 43);
			_objectsState[25] = 1;
		}
		break;
	case 117: // light the dynamite
		if (_objectsState[23] == 0) {
			EXEC_MAIN_ACTION(2);
		} else {
			lightUpDynamite();
		}
		break;
	default:
		warning("PART_02_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::lightUpDynamite() {
	memset(_screenVGA + 0xB400, 0, 0x4BFF);
	playSound(68, 1);
	animateLitAnimFrames(0x5CDC, 0, 2, 0x44C, 22, 50, 0x5989, 127, -1, 0);
	waitForTimer(127);

	igorSay(221, 1, 125);
	if (_gameState.talkMode == kTalkModeTextOnly)
		playSound(11, 1);
	_updateRoomBackground = &IgorEngine::PART_02_UPDATE_FUSE;
	waitForEndOfIgorDialogue();
	_updateRoomBackground = 0;

	memcpy(_screenVGA + _dialogueDirtyRectY, _screenTextLayer + 320 * 72, _dialogueDirtyRectSize);
	if (_gameState.talkMode != kTalkModeTextOnly)
		playSound(11, 1);

	PART_02_WALK_WHILE_FUSE_BURNS(211, 120, 0, 143);
	for (int elapsed = 0; elapsed < 255; elapsed += kTimerTicksCount) {
		PART_02_UPDATE_FUSE();
		waitForTimer();
	}
	copyArea(_screenVGA, 0x5996, 320, _screenLayer1 + 0x5996, 320, 12, 12);
	stopSound();
	waitForTimer(5 * 255);

	igorSay(222, 2, 126);
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

void IgorEngine::PART_02() {
	playMusic(2);
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
		case 20:  // Igor climbs through the window on the first attic entrance.
			fadeIn(768);
			animateLitAnimFrames(0x69C0, 0, 2, 0x759, 33, 57, 0x4B26, 31, -1, 0);
			waitForTimer(31);
			_walkData[0].setPos(57, 117, kFacingPositionRight, 0);
			break;
		case 21:
			fadeIn(768);
			_walkData[0].setPos(300, 136, kFacingPositionLeft, 0);
			break;
		case 22: // back after pigeons cutscene
			memcpy(_currentPalette, _paletteBuffer, 768);
			updatePalette(768);
			_walkData[0].setPos(49, 118, kFacingPositionBack, 0);
			break;
		case 23: // back after outdoor explosion scene
			memset(_screenVGA + 0xB400, 0, 0x4600);
			memcpy(_currentPalette, _paletteBuffer, 768);
			drawAnimRect(0x6C3B, 0x820B + 0 * 0xA19, 47, 55, false, kBlendLitSprite);
			updatePalette(768);

			// Igor lies stunned, stands up, and shakes his head.
			static const uint8 postExplosionFrames[] = {
				0, 1, 2, 1, 2, 1, 2, 1, 2, 1,
				2, 1, 2, 1, 2, 1, 2, 1, 3, 4
			};
			for (uint i = 0; i < ARRAYSIZE(postExplosionFrames); ++i) {
				const int frame = postExplosionFrames[i];
				drawAnimRect(0x6C3B, 0x820B + frame * 0xA19, 47, 55, false, kBlendLitSprite);
				if (frame == 0)
					waitForTimer(2 * 255);
				else if (frame == 1 || frame == 2)
					waitForTimer(31);
				else
					waitForTimer(127);
			}
			igorSay(224, 1, 127);

			// Rebuild the panels erased above.
			drawVerbsPanel();
			drawInventory(_inventoryInfo[72], 0);
			_walkData[0].setPos(215, 134, kFacingPositionRight, 0);
			break;
		case 24: // From igor's room
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
