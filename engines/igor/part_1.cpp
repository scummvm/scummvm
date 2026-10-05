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

void IgorEngine::PART_01_STATE_11_BLIT_blitIgor() {
	for (int i = 0; i < 13; ++i)
		memcpy(_screenVGA + 0x5476 + i * 320, _animFramesBuffer + 0x1859 + i * 16, 16);
}

void IgorEngine::PART_01_STATE_11_BLIT_00A5() {
	for (int i = 0; i < 40; ++i)
		memcpy(_screenVGA + 0x118F + i * 320, _animFramesBuffer + 0xA7F9 + i * 0xDA, 0xDA);
}

//
void IgorEngine::PART_01_STATE_11_BLIT_drawIgorsEyes(int frame) {
	for (int i = 0; i < 4; ++i)
		memcpy(_screenVGA + 0x55BB + i * 320,
				_animFramesBuffer + 0xA7A9 + frame * 20 + i * 5, 5);
}

//
void IgorEngine::PART_01_STATE_11_DRAW_drawPigeons(int index) {
	memcpy(_screenVGA + 0x6F40, _animFramesBuffer + 0x1929 + index * 0x2F80, 0x2F80);
}

// 11: the window-pigeons cutscene It blits two regions,
// restores the current palette from the saved one, then runs a 1000-tick
// animation loop before handing over to state 22.
void IgorEngine::PART_01_STATE_11() {
	PART_01_STATE_11_BLIT_blitIgor();
	PART_01_STATE_11_BLIT_00A5();

	memcpy(_currentPalette, _paletteBuffer, 768);
	updatePalette(768);

	uint8 dosTimerPhase = 0;
	int pendingTicks = 0;
	// for 1000 ticks
	for (int k = 0; k < 1000; ++k) {
		const bool drawIgorsEyes = dosTimerPhase == 0x3D;
		const bool drawPigeons = ((dosTimerPhase + 1) % 0x20) == 0;
		if ((drawIgorsEyes || drawPigeons) && pendingTicks != 0) {
			waitForTimer(pendingTicks);
			pendingTicks = 0;
		}
		if (drawIgorsEyes) {
			PART_01_STATE_11_BLIT_drawIgorsEyes(getRandomNumber(3)); // cseg200:1992-199A
		}
		if (drawPigeons) {
			PART_01_STATE_11_DRAW_drawPigeons(getRandomNumber(2));
		}

		++pendingTicks;

		if (dosTimerPhase == 0x3F)
			dosTimerPhase = 0;
		else
			++dosTimerPhase;
	}
	if (pendingTicks != 0)
		waitForTimer(pendingTicks);
	_currentPart = 22;
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
	// clamps clicks to x 60..276 and y <= 143 before consulting
	// the room mask
	setRoomWalkBounds(60, 0, 276, 143);
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
			if (_currentPart == 11) // cseg200:1950-195A
				PART_01_STATE_11();
			// TODO: transcribe the state 12 cutscene, cseg200:19FD-1AB0.
			// It exits to 0x17 (23) at cseg200:1AD8 and ends in state 0x6C
			// (108) at cseg200:1ADA. Its body repeats the same s3:0xE15E ->
			// s3:0xE45E palette copy as state 11 (cseg200:19FD-1A0A).
			else if (_currentPart == 12) // cseg200:19F3
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
