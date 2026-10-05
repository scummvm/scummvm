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
	static const uint8 windowFrames[] = { 0, 1, 2, 1 };
	for (int i = 0; i < 4; ++i) {
		PART_00_DRAW_RAW_FRAME(216, windowFrames[i], 1204, 28, 43, 0x5919);
		if (i == 0)
			playSound(2, 1);
		if (i != 3) {
			// ANM offset 0 contains four 9x6 frames.
			const int backgroundFrame = getRandomNumber(3);
			const uint8 *src = _animFramesBuffer + backgroundFrame * 54;
			for (int y = 0; y < 6; ++y)
				memcpy(_screenVGA + 0x8EA4 + y * 320, src + y * 9, 9);
			waitForTimer(94);
		}
	}

	ADD_DIALOGUE_TEXT(203, 1, 132);
	ADD_DIALOGUE_TEXT(204, 1, 133);
	SET_DIALOGUE_TEXT(1, 2);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
}

void IgorEngine::PART_01_EXEC_ACTION(int action) {
	debugC(9, kDebugGame, "PART_01_EXEC_ACTION %d", action);
	switch (action) {
	case 101: // walk past pigeon
		ADD_DIALOGUE_TEXT(201, 2, 131);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 102:
		//  draws two 24x41 roof-crossing frames at screen
		// waiting for the video update after each one, then
		// changes to part 20.
		for (int frame = 0; frame <= 1; ++frame) {
			PART_00_DRAW_RAW_FRAME(0x0EF4, frame, 0x3D8, 24, 41, 0x51F4);
			waitForTimer(); // video-update wait
		}
		_currentPart = 20;
		break;
	case 103: // look at pigeon
		ADD_DIALOGUE_TEXT(205, 1, 134);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 104: // look at closed window
		ADD_DIALOGUE_TEXT(206, 2, 135);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 105: // look at other window
		ADD_DIALOGUE_TEXT(208, 1, 136);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 106: // close other window
		ADD_DIALOGUE_TEXT(209, 1, 137);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 107: // open window
		ADD_DIALOGUE_TEXT(210, 1, 138);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 108: // pick up pigeon
		ADD_DIALOGUE_TEXT(211, 2, 139);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	default:
		warning("PART_01_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_01() {
	_gameState.enableLight = 1;
	loadActionData(DAT_OutsideStudentDormitory);
	loadRoomData(PAL_OutsideStudentDormitory, IMG_OutsideStudentDormitory, BOX_OutsideStudentDormitory, MSK_OutsideStudentDormitory, TXT_OutsideStudentDormitory);
	static const int animFrames[] = {
		ANM_OutsideStudentDormitory1, ANM_OutsideStudentDormitory2,
		ANM_OutsideStudentDormitory3, ANM_OutsideStudentDormitory4,
		ANM_OutsideStudentDormitory5, ANM_OutsideStudentDormitory6,
		ANM_OutsideStudentDormitory7, ANM_OutsideStudentDormitory8, 0
	};
	loadAnimData(animFrames);

	_roomDataOffsets = PART_01_ROOM_DATA_OFFSETS;
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_01_EXEC_ACTION);
	memcpy(_screenVGA, _screenLayer1, 46080);

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		if (_currentPart == 10) {
			fadeIn(768);
			_walkData[0].setPos(108, 113, kFacingPositionRight, 0);
			_walkData[0].clipSkipX = 1;
			_walkData[0].clipWidth = 24;
			_walkData[0].scaleWidth = 40;
			_walkData[0].xPosChanged = 1;
			_walkData[0].dxPos = 0;
			_walkData[0].yPosChanged = 1;
			_walkData[0].dyPos = 0;
			_walkData[0].scaleHeight = 40;
			_walkDataCurrentIndex = 0;
			moveIgor(_walkData[0].posNum, _walkData[0].frameNum);
			_walkDataLastIndex = 1;
			_walkDataCurrentIndex = 1;
			PART_01_CLOSE_WINDOW();
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
