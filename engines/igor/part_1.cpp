/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "igor/igor.h"

namespace Igor {

void IgorEngine::PART_01_CLOSE_WINDOW() {
	// s3:0230-0233 (IGOR.EXE:0x8B0230); selected at cseg200:0231-0249.
	static const uint8 windowFrames[] = { 0, 1, 2, 1 };
	for (int i = 0; i < 4; ++i) {
		// cseg200:0226-035C: 28x43 raw frames at ANM offset 216,
		// frame stride 1204, drawn at screen offset 0x5919.
		PART_00_DRAW_RAW_FRAME(216, windowFrames[i], 1204, 28, 43, 0x5919);
		if (i == 0)
			playSound(2, 1); // cseg200:035E-0369
		if (i != 3) {
			// cseg200:0375-037D; ANM offset 0 contains four 9x6 frames.
			const int backgroundFrame = getRandomNumber(3);
			const uint8 *src = _animFramesBuffer + backgroundFrame * 54;
			for (int y = 0; y < 6; ++y)
				memcpy(_screenVGA + 0x8EA4 + y * 320, src + y * 9, 9);
			waitForTimer(94); // cseg200:0382-038C
		}
	}

	ADD_DIALOGUE_TEXT(203, 1, 132); // cseg200:0398-03AA
	ADD_DIALOGUE_TEXT(204, 1, 133); // cseg200:03AA-03BC
	SET_DIALOGUE_TEXT(1, 2); // cseg200:03BC-03C1
	startIgorDialogue(); // cseg200:03C6
	waitForEndOfIgorDialogue(); // cseg200:03CB
}

void IgorEngine::PART_01_EXEC_ACTION(int action) {
	// Action-to-function mapping: cseg200:068A-079F.
	debugC(9, kDebugGame, "PART_01_EXEC_ACTION %d", action);
	switch (action) {
	case 101:
		ADD_DIALOGUE_TEXT(201, 2, 131); // cseg200:054F-0575
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 102:
		// TODO: transcribe the roof-crossing animation from cseg200:03D2-054E.
		// It ends in state 20 at cseg200:0547; do not skip observable behavior.
		warning("PART_01_EXEC_ACTION action 102 is not yet transcribed");
		break;
	case 103:
		ADD_DIALOGUE_TEXT(205, 1, 134); // cseg200:057C-05A2
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 104:
		ADD_DIALOGUE_TEXT(206, 2, 135); // cseg200:05A9-05CF
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 105:
		ADD_DIALOGUE_TEXT(208, 1, 136); // cseg200:05D6-05FC
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 106:
		ADD_DIALOGUE_TEXT(209, 1, 137); // cseg200:0603-0629
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 107:
		ADD_DIALOGUE_TEXT(210, 1, 138); // cseg200:0630-0656
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 108:
		ADD_DIALOGUE_TEXT(211, 2, 139); // cseg200:065D-0683
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	default:
		warning("PART_01_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_01() {
	_gameState.enableLight = 1; // cseg200:1836
	loadActionData(DAT_OutsideStudentDormitory); // cseg200:1853-186A
	loadRoomData(PAL_OutsideStudentDormitory, IMG_OutsideStudentDormitory,
			BOX_OutsideStudentDormitory, MSK_OutsideStudentDormitory,
			TXT_OutsideStudentDormitory); // cseg202:0002
	static const int animFrames[] = {
		ANM_OutsideStudentDormitory1, ANM_OutsideStudentDormitory2,
		ANM_OutsideStudentDormitory3, ANM_OutsideStudentDormitory4,
		ANM_OutsideStudentDormitory5, ANM_OutsideStudentDormitory6,
		ANM_OutsideStudentDormitory7, ANM_OutsideStudentDormitory8, 0
	};
	loadAnimData(animFrames); // cseg201:0002-019F
	_roomDataOffsets = PART_01_ROOM_DATA_OFFSETS;
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_01_EXEC_ACTION); // cseg200:068A-079F
	memcpy(_screenVGA, _screenLayer1, 46080); // cseg200:187E-1890

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		if (_currentPart == 10) {
			fadeIn(768); // cseg200:1AD2-1AD5
			_walkData[0].setPos(108, 113, kFacingPositionRight, 0); // cseg200:1ADA-1AE6
			_walkData[0].clipSkipX = 1; // cseg200:1AF0
			_walkData[0].clipWidth = 24; // cseg200:1AF5
			_walkData[0].scaleWidth = 40; // cseg200:1AFB
			_walkData[0].xPosChanged = 1; // cseg200:1B01
			_walkData[0].dxPos = 0; // cseg200:1B06-1B08
			_walkData[0].yPosChanged = 1; // cseg200:1B0B
			_walkData[0].dyPos = 0; // cseg200:1B10-1B12
			_walkData[0].scaleHeight = 40; // cseg200:1B15
			_walkDataCurrentIndex = 0;
			moveIgor(_walkData[0].posNum, _walkData[0].frameNum);
			_walkDataLastIndex = 1; // cseg200:1B1A
			_walkDataCurrentIndex = 1; // cseg200:1B1F
			PART_01_CLOSE_WINDOW(); // cseg200:1B24
		} else {
			// TODO: transcribe state 11 and 12 transition cutscenes from
			// cseg200:1950-1AB0 before enabling their state changes to 22/23.
			warning("PART_01 state %d entrance is not yet transcribed", _currentPart);
		}
	}

	enterPartLoop();
	while (_currentPart >= 10 && _currentPart <= 12 && !_gameStateLoaded)
		runPartLoop();
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(_currentPart == kInvalidPart ? 768 : 624);
}

} // End of namespace Igor
