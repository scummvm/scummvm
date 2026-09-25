/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "igor/igor.h"

namespace Igor {

namespace {

const uint32 kDoor1Closed = 0x2175; // cseg197:007B
const uint32 kDoor1Open = 0x23F1;   // cseg197:002B
const uint32 kDoor2Closed = 0x266D; // cseg197:011B
const uint32 kDoor2Open = 0x28DD;   // cseg197:00CB

} // End of anonymous namespace

void IgorEngine::PART_70_DRAW_DOOR_STATE(int num) {
	_roomActionsTable[297] = 109; // cseg197:0B0C-0B17
	if (num == 1 || num == 255) { // cseg197:0B1D-0B27
		const uint32 srcOffset = _objectsState[52] == 0 ? kDoor1Closed : kDoor1Open; // cseg197:0B29-0B58; s3:0x856
		for (int y = 0; y <= 52; ++y) // cseg197:0052-00A1 and cseg197:0002-0051
			memcpy(_screenLayer1 + 0x4DB8 + y * 320, _animFramesBuffer + srcOffset + y * 12, 12);
		_roomActionsTable[146] = _objectsState[52] == 0 ? 6 : 7; // cseg197:0B35-0B58
	}
	if (num == 2 || num == 255) { // cseg197:0B5E-0B68
		const uint32 srcOffset = _objectsState[53] == 0 ? kDoor2Closed : kDoor2Open; // cseg197:0B6A-0B99; s3:0x857
		for (int y = 0; y <= 51; ++y) // cseg197:00F2-0141 and cseg197:00A2-00F1
			memcpy(_screenLayer1 + 0x4E1A + y * 320, _animFramesBuffer + srcOffset + y * 12, 12);
		_roomActionsTable[147] = _objectsState[53] == 0 ? 6 : 7; // cseg197:0B76-0B99
	}
}

void IgorEngine::PART_70_ANIMATE_DOOR(int door, bool open) {
	const int stateIndex = door == 1 ? 52 : 53; // cseg197:0150 and cseg197:029E; s3:0x856-0x857
	if ((_objectsState[stateIndex] != 0) == open) {
		const int text = open ? 19 : 23; // cseg222:2C5A-2C86 and cseg222:2CF6-2D22
		ADD_DIALOGUE_TEXT(text, 1, text);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		return;
	}

	const int lastFrame = open ? 2 : 0;   // cseg197:01B1-01D9 and cseg197:0258-0280
	const int step = open ? 1 : -1;       // cseg197:0166 and cseg197:020D
	for (int frame = 1;; frame += step) { // cseg197:015E and cseg197:0205
		if (door == 1) {
			for (int y = 0; y <= 52; ++y) // cseg197:016A-01AF and cseg197:0211-0256
				memcpy(_screenVGA + 0x4DB8 + y * 320, _animFramesBuffer + frame * 0x636 + y * 30, 30);
		} else {
			for (int y = 0; y <= 54; ++y) // cseg197:02B8-0301 and cseg197:0363-03AC
				memcpy(_screenVGA + 0x4E0F + y * 320, _animFramesBuffer + 0x12A2 + frame * 0x4F1 + y * 23, 23);
		}
		if ((open && frame == 2) || (!open && frame == 1))
			playSound(open ? 13 : 14, 1); // cseg197:01B1-01BC,0264-026A,0303-030E,03B5-03C0
		if (frame == lastFrame)
			break;
		waitForTimer(127); // cseg197:01C8-01D2,026F-0279,031A-0324,03C5-03CF (wait while tick <= 0x7E)
	}
	_objectsState[stateIndex] = open ? 1 : 0; // cseg197:01DB,0282,032D,03D8
	PART_70_DRAW_DOOR_STATE(door); // cseg197:01E0-01E2,0287-0289,0332-0334,03DD-03DF
}

void IgorEngine::PART_70_DRAW_SCALED_IGOR(int scaleStep, int facing, int frame, int dyPos) {
	WalkData &wd = _walkData[0]; // cseg197:06F6 and cseg197:0884; s3:0xD94A = 0
	wd.setPos(109, 143, facing, frame); // cseg197:070B-071F and cseg197:089A-08BA
	wd.clipSkipX = 1;                   // cseg197:0734 and cseg197:08CF
	wd.clipWidth = 30;                  // cseg197:0739 and cseg197:08D4
	wd.scaleWidth = 23 + scaleStep * 3; // cseg197:073F-074B and cseg197:08DA-08E6
	wd.xPosChanged = 1;                 // cseg197:074E and cseg197:08E9
	wd.dxPos = 0;                       // cseg197:0753-0755 and cseg197:08EE-08F0
	wd.yPosChanged = 1;                 // cseg197:0758 and cseg197:08F3
	wd.dyPos = dyPos;                   // cseg197:075D-075F and cseg197:08F8
	wd.scaleHeight = 50;                // cseg197:0762 and cseg197:08FE
	_walkDataCurrentIndex = 0;           // cseg197:06F6 and cseg197:0884
	moveIgor(facing, frame);             // cseg197:0767-076F and cseg197:0903-090B
}

void IgorEngine::PART_70_ENTER_FROM_OUTSIDE() {
	PART_70_DRAW_DOOR_STATE(255); // cseg197:06EA-06EC
	for (int area = 11; area <= 13; ++area)
		_roomObjectAreasTable[area].area = 0; // cseg197:092E-095D

	int frame = 1; // cseg197:06FB
	for (int step = 0; step <= 9; ++step) { // cseg197:0700-0785
		PART_70_DRAW_SCALED_IGOR(step, kFacingPositionBack, frame, 0);
		frame = frame == 6 ? 1 : frame + 1; // cseg197:0722-0730
		waitForTimer(15); // cseg197:0774-077E
	}

	_walkDataLastIndex = 0; // cseg197:0787-0791
	if (!_part70FirstVisitDone) { // cseg197:0795; s3:0xED2E
		buildWalkPath(109, 143, 180, 129); // cseg197:079F-07D5; DAT+161 = 0xA1F4
		_walkData[_walkDataLastIndex].frameNum = 0; // cseg197:07DA-07E2
		_walkData[_walkDataLastIndex].posNum = kFacingPositionRight; // cseg197:07E7-07FF; DAT+173 = 2
		_walkDataCurrentIndex = 1; // cseg197:0803
		_gameState.igorMoving = true; // cseg197:0808
		waitForIgorMove(); // cseg197:080D
		ADD_DIALOGUE_TEXT(206, 2, 144); // cseg197:0812-081E
		ADD_DIALOGUE_TEXT(208, 4, 145); // cseg197:0824-0830
		SET_DIALOGUE_TEXT(1, 2); // cseg197:0836-083B
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		_part70FirstVisitDone = true; // cseg197:0845; s3:0xED2E
	} else {
		buildWalkPath(109, 143, 129, 138); // cseg197:084C-0857
		_walkData[_walkDataLastIndex].frameNum = 0; // cseg197:085C-0864
		_walkDataCurrentIndex = 1; // cseg197:0869
		_gameState.igorMoving = true; // cseg197:086E
		waitForIgorMove(); // cseg197:0873
	}
}

void IgorEngine::PART_70_EXIT_TO_OUTSIDE() {
	int frame = 1; // cseg197:0889
	for (int step = 9; step >= 0; --step) { // cseg197:088E-0923
		if (step == 9)
			frame = 0; // cseg197:08AB-08B2
		PART_70_DRAW_SCALED_IGOR(step, kFacingPositionFront, frame, 3);
		frame = frame == 6 ? 1 : frame + 1; // cseg197:08BD-08CB
		waitForTimer(15); // cseg197:0910-091A
	}
	_currentPart = 101; // cseg197:0926
}

void IgorEngine::PART_70_EXEC_ACTION(int action) {
	switch (action) { // cseg197:095E-0B01
	case 101:
		ADD_DIALOGUE_TEXT(201, 1, 140); // cseg197:03F0-03FC
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 102:
		ADD_DIALOGUE_TEXT(202, 2, 141); // cseg197:041D-0429
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 103:
		PART_70_ANIMATE_DOOR(1, true); // cseg197:09AC-09CA
		break;
	case 104:
		PART_70_ANIMATE_DOOR(1, false); // cseg197:09CE-09EC
		break;
	case 105:
		PART_70_ANIMATE_DOOR(2, true); // cseg197:09F0-0A0E
		break;
	case 106:
		PART_70_ANIMATE_DOOR(2, false); // cseg197:0A12-0A30
		break;
	case 107:
		ADD_DIALOGUE_TEXT(204, 1, 142); // cseg197:044A-0456
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 108:
		// The active fork's wrapper is inclusive; 2 therefore reproduces the
		// original random(3), whose result is 0..2 (cseg230:1558).
		switch (getRandomNumber(2)) { // cseg197:047B-048B
		case 0:
			ADD_DIALOGUE_TEXT(205, 1, 143); // cseg197:048D-0499
			ADD_DIALOGUE_TEXT(206, 2, 144); // cseg197:049F-04AB
			ADD_DIALOGUE_TEXT(208, 4, 145); // cseg197:04B1-04BD
			SET_DIALOGUE_TEXT(1, 3); // cseg197:04C3-04C8
			break;
		case 1:
			ADD_DIALOGUE_TEXT(213, 2, 147); // cseg197:04D9-04E5
			SET_DIALOGUE_TEXT(1, 1); // cseg197:04EB-04F0
			break;
		case 2:
			ADD_DIALOGUE_TEXT(215, 1, 148); // cseg197:0500-050C
			ADD_DIALOGUE_TEXT(216, 1, 149); // cseg197:0512-051E
			ADD_DIALOGUE_TEXT(217, 1, 150); // cseg197:0524-0530
			ADD_DIALOGUE_TEXT(218, 1, 151); // cseg197:0536-0542
			ADD_DIALOGUE_TEXT(219, 1, 152); // cseg197:0548-0554
			SET_DIALOGUE_TEXT(1, 5); // cseg197:055A-055F
			break;
		}
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 109:
		ADD_DIALOGUE_TEXT(212, 1, 146); // cseg197:0575-0581
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 110:
		PART_70_EXIT_TO_OUTSIDE(); // cseg197:0A9A-0AB8
		break;
	case 111:
		if (_objectsState[52] != 0) { // cseg197:05A2-05A9; s3:0x856
			for (int area = 11; area <= 13; ++area)
				_roomObjectAreasTable[area].area = 3; // cseg197:05AC-05CE
			--_walkDataLastIndex; // cseg197:05D0
			buildWalkPath(77, 114, 46, 111); // cseg197:05D4-05DC
			_walkDataCurrentIndex = 1; // cseg197:05E1
			_gameState.igorMoving = true; // cseg197:05E6
			waitForIgorMove(); // cseg197:05EB-066B
			_currentPart = 80; // cseg197:066D
		}
		break;
	case 112:
		if (_objectsState[53] != 0) { // cseg197:067F-0686; s3:0x857
			WalkData &wd = _walkData[0]; // cseg197:06CA; s3:0xD94A = 0
			wd.setPos(156, 113, kFacingPositionBack, 1); // cseg197:0688-0699
			wd.setDefaultScale(); // cseg197:069E-06C5
			wd.dxPos = 4; // cseg197:06B4-06BA
			wd.yPosChanged = 0; // cseg197:06BA
			wd.dyPos = 3; // cseg197:06BF
			_walkDataCurrentIndex = 0; // cseg197:06CA
			moveIgor(kFacingPositionBack, 1); // cseg197:06CF-06D3
			_currentPart = 90; // cseg197:06D8
		}
		break;
	default:
		warning("PART_70_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_70() {
	_gameState.enableLight = 1; // cseg197:290F
	loadActionData(DAT_AdministrationCorridor); // cseg197:2920-2943
	loadRoomData(PAL_AdministrationCorridor, IMG_AdministrationCorridor,
			BOX_AdministrationCorridor, MSK_AdministrationCorridor, TXT_AdministrationCorridor); // cseg199:0002-06A0
	static const int anim[] = { ANM_AdministrationCorridor, 0 }; // cseg198:0002-00D5
	loadAnimData(anim);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_70_EXEC_ACTION); // cseg197:095E-0B01
	_roomDataOffsets = PART_70_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143); // cseg199 room mask is 320x144
	PART_70_DRAW_DOOR_STATE(255); // cseg197:2957-2959
	memcpy(_screenVGA, _screenLayer1, 46080); // cseg197:295E-2970
	SET_PAL_240_48_1(); // cseg197:29F0-29F4
	SET_PAL_208_96_1(); // cseg197:29F9-29FF
	for (int area = 11; area <= 13; ++area)
		_roomObjectAreasTable[area].area = 0; // cseg197:2A37; sub_197_092E
	_currentAction.verb = kVerbWalk; // cseg197:2A3C-2A4A

	if (_currentPart == 71) {
		_walkData[0].setPos(78, 114, kFacingPositionRight, 0); // cseg197:2A56-2A67
		_walkData[0].setDefaultScale(); // cseg197:2A6C-2A91
		_walkDataCurrentIndex = 0; // cseg197:2A96
		moveIgor(kFacingPositionRight, 0); // cseg197:2A9B-2A9F
	} else if (_currentPart == 72) {
		_walkData[0].setPos(153, 116, kFacingPositionFront, 0); // cseg197:2AB5-2AC6
		_walkData[0].setDefaultScale(); // cseg197:2ACB-2AF0
		_walkDataCurrentIndex = 0; // cseg197:2AF5
		moveIgor(kFacingPositionFront, 0); // cseg197:2AFA-2AFE
	}
	_walkDataLastIndex = 1; // cseg197:2AA4 and cseg197:2B03
	_walkDataCurrentIndex = 1; // cseg197:2AA9 and cseg197:2B08
	fadeIn(768); // cseg197:2B0D-2B10
	if (_currentPart == 70)
		PART_70_ENTER_FROM_OUTSIDE(); // cseg197:2B15-2B1C

	enterPartLoop();
	while (_currentPart >= 70 && _currentPart <= 72) // cseg197:2B4C-2B55
		runPartLoop();
	leavePartLoop();
	fadeOut(624); // cseg197:371E-3721
}

} // End of namespace Igor
