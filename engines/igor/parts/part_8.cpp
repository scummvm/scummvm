/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "igor/igor.h"

namespace Igor {


void IgorEngine::PART_08_APPLY_OBJECT_STATE(int num) {
	if (num == 1 || num == 255) {
		const uint32 srcOffset = _objectsState[26] == 0 ? 0 : 0x4C8;
		for (int y = 0; y <= 50; ++y)
			memcpy(_screenLayer1 + 0x572C + y * 320, _animFramesBuffer + srcOffset + y * 24, 24);
		_roomActionsTable[150] = _objectsState[26] == 0 ? 6 : 7;
	}
	if (num == 2 || num == 255) {
		if (_objectsState[111] == 1) {
			_roomObjectAreasTable[7].object = 0;
			_roomObjectAreasTable[8].object = 0;
			_roomActionsTable[146] = 4;
			return;
		}
		uint32 srcOffset;
		int dstOffset;
		int rows;
		int width;
		if (_objectsState[29] == 0) {
			srcOffset = 0x208A;
			dstOffset = 0x6BC3;
			rows = 26;
			width = 34;
			_roomObjectAreasTable[8].object = 0;
		} else {
			srcOffset = _objectsState[31] == 0 ? 0x9992 : 0x985C;
			dstOffset = 0x7FC8;
			rows = 10;
			width = 31;
			_roomObjectAreasTable[8].object = _objectsState[31] == 0 ? 3 : 0;
		}
		for (int y = 0; y < rows; ++y) {
			const uint8 *src = _animFramesBuffer + srcOffset + y * width;
			memcpy(_screenVGA + dstOffset + y * 320, src, width);
			memcpy(_screenLayer1 + dstOffset + y * 320, src, width);
		}
		_roomActionsTable[146] = 4;
	}
}

void IgorEngine::drawDoor(bool open) {
	if ((_objectsState[26] != 0) == open) {
		const int text = open ? 19 : 23;
		ADD_DIALOGUE_TEXT(text, 1, text);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		return;
	}

	const int lastFrame = open ? 2 : 0;
	const int step = open ? 1 : -1;
	for (int frame = 1;; frame += step) {
		for (int y = 0; y <= 52; ++y)
			memcpy(_screenVGA + 0x5729 + y * 320,
					_animFramesBuffer + 0x3DA6 + frame * 0x597 + y * 27, 27);
		if ((open && frame == 2) || (!open && frame == 1))
			playSound(open ? 13 : 14, 1);
		if (frame == lastFrame)
			break;
		waitForTimer(127);
	}
	_objectsState[26] = open ? 1 : 0;
	PART_08_APPLY_OBJECT_STATE(1);
}

void IgorEngine::drawDean() {
	for (int y = 0; y <= 25; ++y) {
		const uint8 *src = _animFramesBuffer + 0x23FE + y * 34;
		memcpy(_screenVGA + 0x6BC3 + y * 320, src, 34);
		memcpy(_screenLayer1 + 0x6BC3 + y * 320, src, 34);
	}
}

void IgorEngine::drawDeanTalkingFrame(int frame) {
	for (int y = 0; y <= 21; ++y) {
		const uint8 *src = _animFramesBuffer + 0x4E6B + frame * 0x226 + y * 25;
		memcpy(_screenVGA + 0x6BC6 + y * 320, src, 25);
		memcpy(_screenLayer1 + 0x6BC6 + y * 320, src, 25);
	}
}

void IgorEngine::PART_08_UPDATE_DIALOGUE_DEAN(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		drawDeanTalkingFrame(0);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		drawDeanTalkingFrame(getRandomNumber(6));
		break;
	}
}

void IgorEngine::PART_08_ACTION_105() {
	for (int frame = 0; frame <= 1; ++frame) {
		for (int y = 0; y <= 48; ++y) {
			const uint8 *src = _animFramesBuffer + 0x125E + frame * 0x55C + y * 28;
			memcpy(_screenVGA + 0x66E3 + y * 320, src, 28);
		}
		if (frame == 0)
			waitForTimer(127);
	}
	addObjectToInventory(15, 50);
	_objectsState[31] = 1;
	PART_08_APPLY_OBJECT_STATE(255);
}

void IgorEngine::PART_08_ACTION_109() {
	for (int frame = 0; frame <= 1; ++frame) {
		for (int y = 0; y <= 48; ++y) {
			for (int x = 0; x <= 22; ++x) {
				const int dstOffset = 0x4F77 + y * 320 + x;
				uint8 color = _animFramesBuffer[2448 + frame * 1127 + y * 23 + x];
				if (color >= 0xC0 && color <= 0xCF) {
					const RoomObjectArea &area = _roomObjectAreasTable[_screenLayer2[dstOffset]];
					if (area.y1Lum > 0)
						color = _screenLayer1[dstOffset];
					else if (area.y2Lum > 0)
						color -= area.deltaLum;
				}
				_screenTempLayer[y * 100 + x] = color;
			}
		}
		for (int y = 0; y <= 48; ++y)
			memcpy(_screenVGA + 0x4F77 + y * 320, _screenTempLayer + y * 100, 23);
		if (frame == 0)
			waitForTimer(127);
	}

	addObjectToInventory(13, 48);
	PART_08_APPLY_OBJECT_STATE(255);
	_objectsState[28] = 1;
}

void IgorEngine::drawSecretaryTalkingFrame(int frame) {
	const uint16 frameOffset = READ_LE_UINT16(_animFramesBuffer + 0xE240 + frame * 2);
	decodeAnimFrame(_animFramesBuffer + 0x9AC8 + frameOffset - 1, _screenVGA, true);
}

void IgorEngine::PART_08_UPDATE_DIALOGUE_SECRETARY(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		drawSecretaryTalkingFrame(1);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		drawSecretaryTalkingFrame(getRandomNumber(2) + 2);
		break;
	}
}

void IgorEngine::PART_08_ACTION_108_deanCallsSecretary() {
	const int currentPart = _currentPart;
	const int actionFrameOffset = _objectsState[31] == 0 ? 0x2773 : 0x328D;
	playSound(60, 1);

	for (int frame = 0; frame <= 1; ++frame) {
		for (int y = 0; y <= 48; ++y)
			memcpy(_screenVGA + 0x6D1E + y * 320, _animFramesBuffer + actionFrameOffset - 1 + frame * 0x58D + y * 29, 29);
		if (frame == 0)
			waitForTimer(127);
	}

	_currentPart = 91;
	fadeOut(768);
	loadActionData(DAT_AdministrationSecretaryRoom);
	loadRoomData(PAL_AdministrationSecretaryRoom, IMG_AdministrationSecretaryRoom, BOX_AdministrationSecretaryRoom, MSK_AdministrationSecretaryRoom, TXT_AdministrationSecretaryRoom);
	memcpy(_screenVGA, _screenLayer1, 46080);
	drawSecretaryTalkingFrame(1);
	memcpy(_screenLayer1, _screenVGA, 46080);
	fadeIn(624);

	ADD_DIALOGUE_TEXT(226, 1, 254);
	SET_DIALOGUE_TEXT(1, 1);
	_updateDialogue = &IgorEngine::PART_08_UPDATE_DIALOGUE_SECRETARY;
	startCutsceneDialogue(65, 70, 0, 59, 63);
	waitForEndOfCutsceneDialogue(65, 70, 0, 59, 63);
	_updateDialogue = 0;

	for (int frame = 5; frame <= 19; ++frame) {
		drawSecretaryTalkingFrame(frame);
		if (frame < 19)
			waitForTimer(31);
		else
			playSound(14, 1);
	}
	waitForTimer(255);
	fadeOut(624);

	loadActionData(DAT_DeanPepperOffice);
	loadRoomData(PAL_DeanPepperOffice, IMG_DeanPepperOffice, BOX_DeanPepperOffice, MSK_DeanPepperOffice, TXT_DeanPepperOffice);
	_roomDataOffsets = PART_08_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	PART_08_APPLY_OBJECT_STATE(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	drawInventory(_inventoryInfo[72], 0);
	fadeIn(768);
	_currentPart = currentPart;
	_objectsState[32] = 1;
}

void IgorEngine::PART_08_DEAN_PASSES_OUT() {
	static const uint8 frames[] = {
		3, 1, 2, 3, 4, 5, 4, 5, 4, 5, 4, 5, 4, 5,
		4, 5, 4, 5, 4, 5, 4, 5, 4, 5, 6, 7, 8, 9
	};
	drawDean();
	for (uint i = 0; i < ARRAYSIZE(frames); ++i) {
		decodeAnimFrame(getAnimFrame(0x76DD, 0xD98, frames[i]), _screenVGA, true);
		waitForTimer(31);
	}
}

void IgorEngine::PART_08_DEAN_DRINKS() {
	const uint8 savedTalkMode = _gameState.talkMode;
	_gameState.talkMode = kTalkModeTextOnly;
	waitForTimer(255);

	ADD_DIALOGUE_TEXT(229, 1, 0);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(78, 75, 26, 58, 0);
	playSound(61, 1);

	for (int step = 1; step <= 20; ++step) {
		const int frame = getRandomNumber(5);
		for (int y = 0; y <= 28; ++y)
			memcpy(_screenVGA + 0x6806 + y * 320,
					_animFramesBuffer + 0x8489 + frame * 0x2D5 + y * 25, 25);
		waitForTimer(61);

		if (step == 10) {
			memcpy(_screenVGA + _dialogueDirtyRectY,
					_screenTextLayer + 320 * 72, _dialogueDirtyRectSize);
			ADD_DIALOGUE_TEXT(230, 1, 0);
			SET_DIALOGUE_TEXT(1, 1);
			startCutsceneDialogue(78, 75, 26, 58, 0);
		}
	}

	memcpy(_screenVGA + _dialogueDirtyRectY,
			_screenTextLayer + 320 * 72, _dialogueDirtyRectSize);
	_gameState.dialogueTextRunning = false;
	playSound(15, 1);
	for (int y = 0; y <= 28; ++y)
		memcpy(_screenVGA + 0x6806 + y * 320,
				_animFramesBuffer + 0x9587 + y * 25, 25);
	waitForTimer(255);
	_gameState.talkMode = savedTalkMode;
}

void IgorEngine::PART_08_HANDLE_DIALOGUE_DEAN() {
	loadDialogueData(DLG_DeanPepperOffice);
	_updateDialogue = &IgorEngine::PART_08_UPDATE_DIALOGUE_DEAN;
	handleDialogue(78, 75, 26, 58, 0);
	_updateDialogue = 0;
}

void IgorEngine::PART_08_ACTION_103_TALK_TO_DEAN() {
	if (_objectsState[29] == 1) {
		ADD_DIALOGUE_TEXT(223, 1, 169);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		return;
	}

	if (_objectsState[26] == 1) { // door open
		drawDean();
		ADD_DIALOGUE_TEXT(220, 1, 166);
		ADD_DIALOGUE_TEXT(221, 1, 167);
		SET_DIALOGUE_TEXT(1, 2);
		_updateDialogue = &IgorEngine::PART_08_UPDATE_DIALOGUE_DEAN;
		startCutsceneDialogue(78, 75, 26, 58, 0);
		waitForEndOfCutsceneDialogue(78, 75, 26, 58, 0);
		_updateDialogue = 0;
		return;
	}

	PART_08_HANDLE_DIALOGUE_DEAN();
	PART_08_APPLY_OBJECT_STATE(255);
}

void IgorEngine::giveBottleToDean() {

	if (_objectsState[29] == 1) {
		ADD_DIALOGUE_TEXT(227, 1, 172);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		return;
	}
	if (_objectsState[0] == 1) {
		ADD_DIALOGUE_TEXT(226, 1, 171);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		return;
	}

	drawDean();
	ADD_DIALOGUE_TEXT(210, 2, 158);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();

	static const uint8 deanFrames[] = { 1, 2, 3, 0 };
	static const uint8 igorFrames[] = { 0, 1, 2, 3 };

	for (int step = 0; step < 4; ++step) {
		if (deanFrames[step] != 0) {
			for (int y = 0; y <= 48; ++y) {
				const uint8 *src = _animFramesBuffer + 0x5755 + deanFrames[step] * 0x620 + y * 32;
				memcpy(_screenVGA + 0x6463 + y * 320, src, 32);
			}
		}
		if (igorFrames[step] != 0) {
			for (int y = 0; y <= 24; ++y) {
				const uint8 *src = _animFramesBuffer + 0x6D7D + igorFrames[step] * 0x258 + y * 24;
				memcpy(_screenVGA + 0x6D06 + y * 320, src, 24);
			}
		}
		waitForTimer(41);
	}


	if (_inventoryInfo[58] != 0) {
		_inventoryInfo[_inventoryInfo[58] - 1] = 0;
		_inventoryInfo[58] = 0;
		packInventory();
		if (_inventoryInfo[72] > _inventoryInfo[73])
			_inventoryInfo[72] = _inventoryOffsetTable[(_inventoryInfo[73] - 1) / 7];
		drawInventory(_inventoryInfo[72], 0);
	}

	ADD_DIALOGUE_TEXT(216, 1, 162);
	ADD_DIALOGUE_TEXT(217, 1, 163);
	ADD_DIALOGUE_TEXT(218, 1, 164);
	ADD_DIALOGUE_TEXT(219, 1, 165);
	SET_DIALOGUE_TEXT(1, 4);
	_updateDialogue = &IgorEngine::PART_08_UPDATE_DIALOGUE_DEAN;
	startCutsceneDialogue(78, 75, 26, 58, 0);
	waitForEndOfCutsceneDialogue(78, 75, 26, 58, 0);
	_updateDialogue = 0;
	_objectsState[30] = _objectsState[0] < 2 ? 1 : 2;
	drawInventory(_inventoryInfo[72], 0);
	playSound(63, 1);
	PART_08_APPLY_OBJECT_STATE(255);
}

void IgorEngine::PART_08_EXEC_ACTION(int action) {
	switch (action) {
	case 101: // exit office
		if (_objectsState[26] == 0) {
			_currentPart = 71;
			break;
		}
		if (_objectsState[26] != 0) {
			for (int area = 10; area <= 11; ++area)
				_roomObjectAreasTable[area].area = 4;
			for (int area = 13; area <= 14; ++area)
				_roomObjectAreasTable[area].area = 4;
			--_walkDataLastIndex;
			buildWalkPath(242, 121, 270, 108);
			_walkDataCurrentIndex = 1;
			_gameState.igorMoving = true;
			waitForIgorMove();
			if (_objectsState[30] != 0) {
				waitForTimer(251);
				PART_08_DEAN_PASSES_OUT();
				if (_objectsState[42] == 1)
					_objectsState[42] = 2;
				else
					_objectsState[41] = 1;
				waitForTimer(101);
				playSound(16, 1);
				if (_objectsState[30] == 2) {
					PART_08_DEAN_DRINKS();
					_objectsState[29] = 1;
				}
				_objectsState[30] = 0;
			}
			_currentPart = 71;
		}
		break;
	case 102: // Look at window
		ADD_DIALOGUE_TEXT(202, 1, 153);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 104: // Look at dean
		if (_objectsState[29] == 1)
			ADD_DIALOGUE_TEXT(222, 1, 168);
		else
			ADD_DIALOGUE_TEXT(203, 2, 154);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 107: // look at intercom
		ADD_DIALOGUE_TEXT(205, 2, 155);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 110: // look at bookcase
		ADD_DIALOGUE_TEXT(207, 2, 156);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 111: // look at door
		ADD_DIALOGUE_TEXT(209, 1, 157);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 105:
		PART_08_ACTION_105();
		break;
	case 103:
		PART_08_ACTION_103_TALK_TO_DEAN();
		break;
	case 108:
		if (_objectsState[29] == 0) {
			ADD_DIALOGUE_TEXT(212, 1, 159);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
			waitForEndOfIgorDialogue();
		} else if (_objectsState[32] == 1) {
			ADD_DIALOGUE_TEXT(224, 2, 170);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
			waitForEndOfIgorDialogue();
		} else {
			PART_08_ACTION_108_deanCallsSecretary();
		}
		break;
	case 109:
		if (_objectsState[29] == 0) {
			ADD_DIALOGUE_TEXT(213, 2, 160);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
			waitForEndOfIgorDialogue();
		} else if (_objectsState[28] == 1) {
			ADD_DIALOGUE_TEXT(215, 1, 161);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
			waitForEndOfIgorDialogue();
		} else {
			PART_08_ACTION_109();
		}
		break;
	case 114:
		giveBottleToDean();
		break;
	case 112:
		drawDoor(true);
		break;
	case 113:
		drawDoor(false);
		break;
	default:
		warning("PART_08_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_08() {
	_gameState.enableLight = 1;
	loadActionData(DAT_DeanPepperOffice);
	loadRoomData(PAL_DeanPepperOffice, IMG_DeanPepperOffice, BOX_DeanPepperOffice, MSK_DeanPepperOffice, TXT_DeanPepperOffice);
	static const int anim[] = {
		FRM_DeanPepperOffice1, FRM_DeanPepperOffice2,
		FRM_DeanPepperOffice3, FRM_DeanPepperOffice4,
		FRM_DeanPepperOffice5, FRM_DeanPepperOffice6,
		FRM_DeanPepperOffice7, FRM_DeanPepperOffice8,
		FRM_DeanPepperOffice9, FRM_DeanPepperOffice10,
		FRM_DeanPepperOffice11, FRM_DeanPepperOffice12,
		FRM_DeanPepperOffice13, FRM_DeanPepperOffice14,
		FRM_DeanPepperOffice15, FRM_DeanPepperOffice16, 0
	};
	loadAnimData(anim);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_08_EXEC_ACTION);
	_roomDataOffsets = PART_08_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	PART_08_APPLY_OBJECT_STATE(255);

	memcpy(_screenVGA, _screenLayer1, 46080);
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		_walkData[0].setPos(241, 121, kFacingPositionLeft, 0);
		_walkData[0].setDefaultScale();
		_walkDataCurrentIndex = 0;
		moveIgor(kFacingPositionLeft, 0);
		_walkDataLastIndex = 1;
		_walkDataCurrentIndex = 1;
		fadeIn(768);
	}

	enterPartLoop();
	while (_currentPart == 80 && !_gameStateLoaded)
		runPartLoop();
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
