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

void IgorEngine::PART_02_APPLY_OBJECT_STATE(int num) {
	if (num == 1 || num == 255)
		_roomActionsTable[252] = _objectsState[16] == 0 ? 6 : 4; // cseg203:22CF-2303; s3:084C
	if ((num == 2 || num == 255) && _objectsState[17] == 1) { // cseg203:2310-231C; s3:084D
		_roomObjectAreasTable[17].object = 4; // s3:DCAC; (DCAC-DC56)/5 = 17, field 1
		_roomObjectAreasTable[19].object = 4; // s3:DCB6; (DCB6-DC56)/5 = 19, field 1
	}
	if (num == 3 || num == 255)
		_roomObjectAreasTable[9].object = _objectsState[18] == 0 ? 0 : 6; // cseg203:232D-2340; s3:084E/DC84
	if (num == 4 || num == 255) {
		if (_objectsState[19] == 0) // cseg203:2351-2372; s3:084F
			_roomObjectAreasTable[12].object = _objectsState[18] == 1 ? 6 : 0; // s3:DC93
		else
			_roomObjectAreasTable[12].object = 7; // s3:DC93
	}
	if (num == 5 || num == 255) {
		if (_objectsState[20] == 0) { // cseg203:2383-23AE; s3:0850
			_roomObjectAreasTable[13].object = _objectsState[19] == 1 ? 7 : (_objectsState[18] == 1 ? 6 : 0); // s3:DC98
		} else {
			_roomObjectAreasTable[13].object = 8; // s3:DC98
		}
	}
	if (num == 6 || num == 255) {
		if (_objectsState[21] == 0) // cseg203:23B3-23E5; s3:0851
			_roomObjectAreasTable[14].object = _objectsState[20] == 1 ? 8 : (_objectsState[19] == 1 ? 7 : (_objectsState[18] == 1 ? 6 : 0)); // s3:DC9D
	}
	if ((num == 8 || num == 255) && _objectsState[23] == 1)
		_roomObjectAreasTable[13].object = 8; // cseg203:23EA-23FD; s3:0853/DC98
}

void IgorEngine::PART_02_EXEC_ACTION(int action) {
	// Action-to-function mapping: cseg203:1FD9-22AE.
	debugC(9, kDebugGame, "PART_02_EXEC_ACTION %d", action);
	switch (action) {
	case 105:
		PART_02_START_DIALOGUE(209, 1, 115); // cseg203:1E87-1EB2
		_objectsState[22] = 1; // s3:0852; cseg203:1EB2
		break;
	case 108:
		PART_02_START_DIALOGUE(210, 1, 116); // cseg203:1EB9-1EE4
		break;
	case 109:
		ADD_DIALOGUE_TEXT(225, 4, 128); // cseg203:1EE6-1F1E
		ADD_DIALOGUE_TEXT(229, 1, 129); // cseg203:1F02-1F0E
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 110:
		PART_02_START_DIALOGUE(213, 1, 118); // cseg203:1F25-1F50
		break;
	case 111:
		PART_02_START_DIALOGUE(214, 2, 119); // cseg203:1F52-1F7D
		break;
	case 112:
		PART_02_START_DIALOGUE(216, 1, 120); // cseg203:1F7F-1FAA
		break;
	case 114:
		PART_02_START_DIALOGUE(219, 1, 123); // cseg203:1FAC-1FD7
		break;
	case 115:
		PART_02_START_DIALOGUE(217, 1, 121); // cseg203:0A2D-0A58
		break;
	case 116:
		PART_02_START_DIALOGUE(218, 1, 122); // cseg203:0A5A-0A85
		break;
	case 118:
		if (_objectsState[24] == 1) { // s3:0854; cseg203:1989-19AC
			PART_02_START_DIALOGUE(220, 1, 124);
		} else {
			_objectsState[24] = 1; // cseg203:19B3
			_currentPart = 11; // cseg203:19B8
		}
		break;
	case 119:
		// TODO: transcribe the exit animation from cseg203:1CF8-1E74.
		// It ends in state 1 at cseg203:1E6D; do not skip observable behavior.
		warning("PART_02_EXEC_ACTION action 119 is not yet transcribed");
		break;
	case 120:
		_currentPart = 30; // cseg203:1E75-1E85
		break;
	case 101: case 102: case 103: case 104: case 106: case 107:
	case 113: case 117: case 121:
		// TODO: transcribe the corresponding function selected by
		// cseg203:1FD9-22AE. Keeping these actions disabled avoids inventing
		// animation/state behavior.
		warning("PART_02_EXEC_ACTION action %d is not yet transcribed", action);
		break;
	default:
		warning("PART_02_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_02() {
	_gameState.enableLight = 1; // cseg203:41D0
	loadActionData(DAT_StudentDormitoryAttic); // cseg203:41ED-4204
	loadRoomData(PAL_StudentDormitoryAttic, IMG_StudentDormitoryAttic,
			BOX_StudentDormitoryAttic, MSK_StudentDormitoryAttic,
			TXT_StudentDormitoryAttic); // cseg205:0002
	static const int animFrames[] = {
		ANM_StudentDormitoryAttic1, ANM_StudentDormitoryAttic2,
		ANM_StudentDormitoryAttic3, ANM_StudentDormitoryAttic4,
		ANM_StudentDormitoryAttic5, ANM_StudentDormitoryAttic6,
		ANM_StudentDormitoryAttic7, ANM_StudentDormitoryAttic8,
		ANM_StudentDormitoryAttic9, ANM_StudentDormitoryAttic10,
		ANM_StudentDormitoryAttic11, ANM_StudentDormitoryAttic12,
		ANM_StudentDormitoryAttic13, ANM_StudentDormitoryAttic14, 0
	};
	loadAnimData(animFrames); // cseg204:0002-02CC
	_roomDataOffsets = PART_02_ROOM_DATA_OFFSETS;
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_02_EXEC_ACTION); // cseg203:1FD9-22AE
	PART_02_APPLY_OBJECT_STATE(255); // cseg203:4213-421A
	memcpy(_screenVGA, _screenLayer1, 46080); // cseg203:421F-4231

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		switch (_currentPart) {
		case 20:
			fadeIn(768); // cseg203:45AB-45AE
			_walkData[0].setPos(57, 117, kFacingPositionRight, 0); // cseg203:45C9-45DA
			break;
		case 21:
			fadeIn(768); // cseg203:463F-4642
			_walkData[0].setPos(300, 136, kFacingPositionLeft, 0); // cseg203:4647-4658
			break;
		case 22:
			_walkData[0].setPos(49, 118, kFacingPositionBack, 0); // cseg203:4337-4348
			break;
		case 23:
			_walkData[0].setPos(215, 134, kFacingPositionRight, 0); // cseg203:4437-445E
			break;
		case 24:
			_walkData[0].setPos(211, 120, kFacingPositionFront, 0); // cseg203:450E-451F
			fadeIn(768); // cseg203:455C-455F
			break;
		}
		_walkData[0].clipSkipX = 1; // cseg203:4524,45DF,465D
		_walkData[0].clipWidth = 30; // cseg203:4529,45E4,4662
		_walkData[0].scaleWidth = 50; // cseg203:452F,45EA,4668
		_walkData[0].xPosChanged = 1; // cseg203:4535,45F0,466E
		_walkData[0].dxPos = 0; // cseg203:453A-453C,45F5-45F7,4673-4675
		_walkData[0].yPosChanged = 1; // cseg203:453F,45FA,4678
		_walkData[0].dyPos = 0; // cseg203:4544-4546,45FF-4601,467D-467F
		_walkData[0].scaleHeight = 50; // cseg203:4549,4604,4682
		_walkDataCurrentIndex = 0;
		moveIgor(_walkData[0].posNum, _walkData[0].frameNum);
		_walkDataLastIndex = 1; // cseg203:4575,4609,4698
		_walkDataCurrentIndex = 1; // cseg203:457A,460E,469D
		if (_currentPart == 22)
			PART_02_EXEC_ACTION(118); // cseg203:438B
		// TODO: transcribe state-specific entrance animations from
		// cseg203:42F8-46B0. Positions above are their exact final walk state.
	}

	enterPartLoop();
	while (_currentPart >= 20 && _currentPart <= 24 && !_gameStateLoaded)
		runPartLoop();
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(_currentPart == kInvalidPart ? 768 : 624);
}

} // End of namespace Igor
