/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "igor/igor.h"

namespace Igor {

void IgorEngine::PART_09_APPLY_OBJECT_STATE(int num) {
	if (num == 1 || num == 255) {
		const uint32 srcOffset = _objectsState[27] == 0 ? 0x5D0B : 0x61A3;
		for (int y = 0; y <= 48; ++y)
			memcpy(_screenLayer1 + 0x46C2 + y * 320, _animFramesBuffer + srcOffset + y * 24, 24);
		_roomActionsTable[150] = _objectsState[27] == 0 ? 6 : 7;
	}
	if (num == 2 || num == 255) {
		const uint32 srcOffset = _objectsState[34] == 0 ? 0x663B : 0x6A23;
		for (int y = 0; y <= 49; ++y)
			memcpy(_screenLayer1 + 0x48E6 + y * 320, _animFramesBuffer + srcOffset + y * 20, 20);
		_roomObjectAreasTable[4].object = (_objectsState[34] == 0 || _objectsState[36] != 0) ? 3 : 5;
	}
}

void IgorEngine::PART_09_ANIMATE_DOOR(bool open) {
	if ((_objectsState[27] != 0) == open) {
		const int text = open ? 19 : 23;
		ADD_DIALOGUE_TEXT(text, 1, text);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		return;
	}

	static const uint8 openFrames[] = { 3, 0 };

	for (int i = 0; i < 2; ++i) {
		const int frame = open ? openFrames[i] : i + 1;
		for (int y = 0; y <= 52; ++y)
			memcpy(_screenVGA + 0x46C2 + y * 320,
					_animFramesBuffer + 0x86EF + frame * 0x52D + y * 25, 25);
		if (i == 1)
			playSound(open ? 13 : 14, 1);
		waitForTimer(127);
	}
	_objectsState[27] = open ? 1 : 0;
	PART_09_APPLY_OBJECT_STATE(1);
}

void IgorEngine::PART_09_EXEC_ACTION(int action) {
	switch (action) {
	case 102:
		ADD_DIALOGUE_TEXT(201, 2, 234);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 104:
		ADD_DIALOGUE_TEXT(203, 2, 235);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 108:
		ADD_DIALOGUE_TEXT(207, 1, 237);
		ADD_DIALOGUE_TEXT(208, 1, 238);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 109:
		if (_objectsState[35] == 0)
			ADD_DIALOGUE_TEXT(216, 1, 244);
		else
			ADD_DIALOGUE_TEXT(224, 1, 252);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 111:
		ADD_DIALOGUE_TEXT(211, 1, 240);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 115:
		if (_objectsState[27] != 0) {
			_walkData[0].setPos(207, 106, kFacingPositionBack, 1);
			_walkData[0].setDefaultScale();
			_walkDataCurrentIndex = 0;
			moveIgor(kFacingPositionBack, 1);
			_currentPart = 72;
		}
		break;
	case 112:
		PART_09_ANIMATE_DOOR(true);
		break;
	case 113:
		PART_09_ANIMATE_DOOR(false);
		break;
	case 101:
	case 103:
	case 105:
	case 106:
	case 107:
	case 110:
	case 114:
		// TODO: translate the corresponding cseg190 action routine exactly.
		break;
	default:
		warning("PART_09_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_09() {
	_gameState.enableLight = 1;
	loadActionData(DAT_AdministrationSecretaryRoom);
	loadRoomData(PAL_AdministrationSecretaryRoom, IMG_AdministrationSecretaryRoom,
			BOX_AdministrationSecretaryRoom, MSK_AdministrationSecretaryRoom,
			TXT_AdministrationSecretaryRoom);
	static const int anim[] = {
		ANM_AdministrationSecretaryRoom1, ANM_AdministrationSecretaryRoom2,
		ANM_AdministrationSecretaryRoom3, ANM_AdministrationSecretaryRoom4,
		ANM_AdministrationSecretaryRoom5, ANM_AdministrationSecretaryRoom6,
		ANM_AdministrationSecretaryRoom7, ANM_AdministrationSecretaryRoom8,
		ANM_AdministrationSecretaryRoom9, 0
	};
	loadAnimData(anim);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_09_EXEC_ACTION);
	_roomDataOffsets = PART_09_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	PART_09_APPLY_OBJECT_STATE(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		_walkData[0].setPos(206, 109, kFacingPositionFront, 0);
		_walkData[0].setDefaultScale();
		_walkDataCurrentIndex = 0;
		moveIgor(kFacingPositionFront, 0);
		_walkDataLastIndex = 1;
		_walkDataCurrentIndex = 1;
		fadeIn(768);
	}

	enterPartLoop();
	while (_currentPart == 90 && !_gameStateLoaded)
		runPartLoop();
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
