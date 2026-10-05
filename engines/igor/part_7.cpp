/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "igor/igor.h"

namespace Igor {

const uint32 kDoor1Closed = 0x2175;
const uint32 kDoor1Open = 0x23F1;
const uint32 kDoor2Closed = 0x266D;
const uint32 kDoor2Open = 0x28DD;

void IgorEngine::PART_07_DRAW_DOOR_STATE(int num) {
	_roomActionsTable[297] = 109;
	if (num == 1 || num == 255) {
		const uint32 srcOffset = _objectsState[26] == 0 ? kDoor1Closed : kDoor1Open;
		for (int y = 0; y <= 52; ++y)
			memcpy(_screenLayer1 + 0x4DB8 + y * 320, _animFramesBuffer + srcOffset + y * 12, 12);
		_roomActionsTable[146] = _objectsState[26] == 0 ? 6 : 7;
	}
	if (num == 2 || num == 255) {
		const uint32 srcOffset = _objectsState[27] == 0 ? kDoor2Closed : kDoor2Open;
		for (int y = 0; y <= 51; ++y)
			memcpy(_screenLayer1 + 0x4E1A + y * 320, _animFramesBuffer + srcOffset + y * 12, 12);
		_roomActionsTable[147] = _objectsState[27] == 0 ? 6 : 7;
	}
}

void IgorEngine::PART_07_openCloseDoor(int door, bool open) {
	const int stateIndex = door == 1 ? 26 : 27; // s3:0x856/0x857; cseg197:0150-03D8
	if ((_objectsState[stateIndex] != 0) == open) {
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
		if (door == 1) {
			for (int y = 0; y <= 52; ++y)
				memcpy(_screenVGA + 0x4DB8 + y * 320, _animFramesBuffer + frame * 0x636 + y * 30, 30);
		} else {
			for (int y = 0; y <= 54; ++y)
				memcpy(_screenVGA + 0x4E0F + y * 320, _animFramesBuffer + 0x12A2 + frame * 0x4F1 + y * 23, 23);
		}
		if ((open && frame == 2) || (!open && frame == 1))
			playSound(open ? 13 : 14, 1);
		if (frame == lastFrame)
			break;
		waitForTimer(127);
	}
	_objectsState[stateIndex] = open ? 1 : 0;
	PART_07_DRAW_DOOR_STATE(door);
}

void IgorEngine::PART_07_DRAW_SCALED_IGOR(int scaleStep, int facing, int frame, int dyPos) {
	WalkData &wd = _walkData[0];
	wd.setPos(109, 143, facing, frame);
	wd.clipSkipX = 1;
	wd.clipWidth = 30;
	wd.scaleWidth = 23 + scaleStep * 3;
	wd.xPosChanged = 1;
	wd.dxPos = 0;
	wd.yPosChanged = 1;
	wd.dyPos = dyPos;
	wd.scaleHeight = 50;
	_walkDataCurrentIndex = 0;
	moveIgor(facing, frame);
}

void IgorEngine::PART_07_ENTER_FROM_OUTSIDE() {
	PART_07_DRAW_DOOR_STATE(255);
	for (int area = 11; area <= 13; ++area)
		_roomObjectAreasTable[area].area = 0;

	int frame = 1;
	for (int step = 0; step <= 9; ++step) {
		PART_07_DRAW_SCALED_IGOR(step, kFacingPositionBack, frame, 0);
		frame = frame == 6 ? 1 : frame + 1;
		waitForTimer(15);
	}

	_walkDataLastIndex = 0;
	if (!_part07FirstVisitDone) {
		buildWalkPath(109, 143, 180, 129);
		_walkData[_walkDataLastIndex].frameNum = 0;
		_walkData[_walkDataLastIndex].posNum = kFacingPositionRight;
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true;
		waitForIgorMove();
		ADD_DIALOGUE_TEXT(206, 2, 144);
		ADD_DIALOGUE_TEXT(208, 4, 145);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		_part07FirstVisitDone = true;
	} else {
		buildWalkPath(109, 143, 129, 138);
		_walkData[_walkDataLastIndex].frameNum = 0;
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true;
		waitForIgorMove();
	}
}

void IgorEngine::PART_07_EXIT_TO_OUTSIDE() {
	int frame = 1;
	for (int step = 9; step >= 0; --step) {
		if (step == 9)
			frame = 0;
		PART_07_DRAW_SCALED_IGOR(step, kFacingPositionFront, frame, 3);
		frame = frame == 6 ? 1 : frame + 1;
		waitForTimer(15);
	}
	_currentPart = 101;
}

void IgorEngine::PART_07_EXEC_ACTION(int action) {
	switch (action) {
	case 101: // Look at dean's plaque
		ADD_DIALOGUE_TEXT(201, 1, 140);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 102: // Look at dean's door
		ADD_DIALOGUE_TEXT(202, 2, 141);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 103:
		PART_07_openCloseDoor(1, true);
		break;
	case 104:
		PART_07_openCloseDoor(1, false);
		break;
	case 105:
		PART_07_openCloseDoor(2, true);
		break;
	case 106:
		PART_07_openCloseDoor(2, false);
		break;
	case 107: // Secretary's plaque
		ADD_DIALOGUE_TEXT(204, 1, 142);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 108: // Look at messageboard
		switch (getRandomNumber(2)) {
		case 0:
			ADD_DIALOGUE_TEXT(205, 1, 143);
			ADD_DIALOGUE_TEXT(206, 2, 144);
			ADD_DIALOGUE_TEXT(208, 4, 145);
			SET_DIALOGUE_TEXT(1, 3);
			break;
		case 1:
			ADD_DIALOGUE_TEXT(213, 2, 147);
			SET_DIALOGUE_TEXT(1, 1);
			break;
		case 2:
			ADD_DIALOGUE_TEXT(215, 1, 148);
			ADD_DIALOGUE_TEXT(216, 1, 149);
			ADD_DIALOGUE_TEXT(217, 1, 150);
			ADD_DIALOGUE_TEXT(218, 1, 151);
			ADD_DIALOGUE_TEXT(219, 1, 152);
			SET_DIALOGUE_TEXT(1, 5);
			break;
		}
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 109: // Look at stairs
		ADD_DIALOGUE_TEXT(212, 1, 146);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 110:
		PART_07_EXIT_TO_OUTSIDE();
		break;
	case 111: // Go through dean's door
		if (_objectsState[26] != 0) {
			for (int area = 11; area <= 13; ++area) {
				_roomObjectAreasTable[area].area = 3;
			}
			--_walkDataLastIndex;
			buildWalkPath(77, 114, 46, 111);
			_walkDataCurrentIndex = 1;
			_gameState.igorMoving = true;
			waitForIgorMove();
			_currentPart = 80;
		}
		break;
	case 112: // Go through secretary's door
		if (_objectsState[27] != 0) {
			WalkData &wd = _walkData[0];
			wd.setPos(156, 113, kFacingPositionBack, 1);
			wd.setDefaultScale();
			wd.dxPos = 4;
			wd.yPosChanged = 0;
			wd.dyPos = 3;
			_walkDataCurrentIndex = 0;
			moveIgor(kFacingPositionBack, 1);
			_currentPart = 90;
		}
		break;
	default:
		warning("PART_07_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_07() {
	_gameState.enableLight = 1;
	loadActionData(DAT_AdministrationCorridor);
	loadRoomData(PAL_AdministrationCorridor, IMG_AdministrationCorridor,
				 BOX_AdministrationCorridor, MSK_AdministrationCorridor,
				 TXT_AdministrationCorridor);
	static const int anim[] = {ANM_AdministrationCorridor, 0};
	loadAnimData(anim);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_07_EXEC_ACTION);
	_roomDataOffsets = PART_07_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	PART_07_DRAW_DOOR_STATE(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	for (int area = 11; area <= 13; ++area) {
		_roomObjectAreasTable[area].area = 0;
	}
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		if (_currentPart == 71) { // enter from dean's door
			_walkData[0].setPos(78, 114, kFacingPositionRight, 0);
			_walkData[0].setDefaultScale();
			_walkDataCurrentIndex = 0;
			moveIgor(kFacingPositionRight, 0);
		} else if (_currentPart == 72) { // enter from secretary's door
			_walkData[0].setPos(153, 116, kFacingPositionFront, 0);
			_walkData[0].setDefaultScale();
			_walkDataCurrentIndex = 0;
			moveIgor(kFacingPositionFront, 0);
		}

		_walkDataLastIndex = 1;
		_walkDataCurrentIndex = 1;
		fadeIn(768);
		if (_currentPart == 70) { // enter from outside
			PART_07_ENTER_FROM_OUTSIDE();
		}
	}
	enterPartLoop();
	while (_currentPart >= 70 && _currentPart <= 72 && !_gameStateLoaded)
		runPartLoop();
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
