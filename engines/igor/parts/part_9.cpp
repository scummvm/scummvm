/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "igor/igor.h"

namespace Igor {

void IgorEngine::PART_09_DRAW_SECRETARY_FRAME(int frame, bool background) {
	drawAnimRect(0x7839, frame * 0x34E, 18, 47, background);
}

void IgorEngine::PART_09_APPLY_OBJECT_STATE(int num) {
	if (num == 1 || num == 255) {
		const uint32 srcOffset = _objectsState[27] == 0 ? 0x5D0B : 0x61A3;
		copyArea(_screenLayer1, 0x46C2, 320, _animFramesBuffer + srcOffset, 24, 24, 49);
		_roomActionsTable[150] = _objectsState[27] == 0 ? 6 : 7;
	}
	if (num == 2 || num == 255) {
		const uint32 srcOffset = _objectsState[34] == 0 ? 0x663B : 0x6A23;
		copyArea(_screenLayer1, 0x48E6, 320, _animFramesBuffer + srcOffset, 20, 20, 50);
		_roomObjectAreasTable[4].object = (_objectsState[34] == 0 || _objectsState[36] != 0) ? 3 : 5;
	}
	if (num == 3 || num == 255) {
		if (_objectsState[35] == 0) {
			PART_09_DRAW_SECRETARY_FRAME(1, true);
			_roomObjectAreasTable[6].object = 2;
			_roomObjectAreasTable[15].object = 2;
			_roomObjectAreasTable[15].area = 0;
			_roomObjectAreasTable[16].area = 0;
		} else {
			_roomObjectAreasTable[6].object = 0;
			_roomObjectAreasTable[15].object = 0;
			_roomActionsTable[151] = 0xB5;
			_roomActionsTable[152] = 0xB1;
			_roomActionsTable[189] = 1;
			_roomActionsTable[201] = 13;
			_roomActionsTable[205] = 35;
			_roomActionsTable[207] = 35;
		}
	}
}

void IgorEngine::PART_09_ANIMATE_DOOR(bool open) {
	if ((_objectsState[27] != 0) == open) {
		executeAction(open ? 11 : 14);
		return;
	}

	static const uint8 openFrames[] = { 3, 0 };

	for (int i = 0; i < 2; ++i) {
		const int frame = open ? openFrames[i] : i + 1;
		drawAnimRect(0x46C2, 0x86EF + frame * 0x52D, 25, 53);
		if (i == 1)
			playSound(open ? 13 : 14, 1);
		waitForTimer(127);
	}
	_objectsState[27] = open ? 1 : 0;
	PART_09_APPLY_OBJECT_STATE(1);
}

void IgorEngine::PART_09_DRAW_DRAWER_FRAME(int frame) {
	drawAnimRect(0x48E6, 0x6E0B + frame * 0x639, 27, 59);
}

void IgorEngine::PART_09_ACTION_106(bool search) {
	if (_objectsState[35] == 0) {
		ADD_DIALOGUE_TEXT(214, 2, 243);
	} else if (_objectsState[34] == 1) {
		executeAction(10);
		return;
	} else if (search) {
		static const uint8 frames[] = { 1, 2, 1, 2, 1, 2, 3 };
		for (uint i = 0; i < ARRAYSIZE(frames); ++i) {
			PART_09_DRAW_DRAWER_FRAME(frames[i]);
			waitForTimer(61);
		}
		_objectsState[34] = 1;
		PART_09_APPLY_OBJECT_STATE(2);
		ADD_DIALOGUE_TEXT(212, 1, 241);
	} else {
		static const uint8 frames[] = { 1, 2, 1, 2, 1, 2, 1, 2, 0 };
		for (uint i = 0; i < ARRAYSIZE(frames); ++i) {
			PART_09_DRAW_DRAWER_FRAME(frames[i]);
			if (i == 0)
				playSound(49, 1);
			if (i + 1 < ARRAYSIZE(frames))
				waitForTimer(31);
		}
		ADD_DIALOGUE_TEXT(213, 1, 242);
	}
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
}

void IgorEngine::PART_09_ACTION_110() {
	for (int frame = 1; frame >= 0; --frame) {
		drawAnimRect(0x48E6, 0x9BA3 + frame * 0x438, 27, 40, false, kBlendLitSprite);
		waitForTimer(127);
	}
	addObjectToInventory(14, 49);
	_objectsState[36] = 1;
	PART_09_APPLY_OBJECT_STATE(255);
}

void IgorEngine::PART_09_ACTION_101_openFileCabinet() {
	if (_objectsState[35] == 0) { // secretary dismissed
		ADD_DIALOGUE_TEXT(220, 1, 248);
	} else if (_objectsState[37] == 1) { // already changed file
		ADD_DIALOGUE_TEXT(225, 1, 253);
	} else {
		static const uint8 frames[] = { 1, 2, 3, 4, 5, 4, 5, 4, 5, 4, 5, 4, 5, 4, 5, 2, 1, 6 };
		for (uint i = 0; i < ARRAYSIZE(frames); ++i) {
			drawAnimRect(0x7595, 0x9BD8 + frames[i] * 0x83B, 43, 49);
			waitForTimer(i >= 3 && i <= 14 ? 31 : 61);
		}
		_walkData[_walkDataLastIndex - 1].posNum = kFacingPositionFront;
		igorSayAndWait({ { 221, 1, 249 }, { 222, 1, 250 }, { 223, 1, 251 } });
		for (int frame = 7; frame >= 6; --frame) {
			drawAnimRect(0x7595, 0x9BD8 + frame * 0x83B, 43, 49);
			if (frame == 7)
				waitForTimer(255);
		}
		_objectsState[37] = 1;
		return;
	}
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
}

void IgorEngine::PART_09_DRAW_SECRETARY_MOUTH(int frame) {
	drawAnimRect(0x783A, 0xD38 + frame * 0xA9, 13, 13, true);
}

void IgorEngine::PART_09_UPDATE_DIALOGUE_SECRETARY(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_09_DRAW_SECRETARY_MOUTH(0);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_09_DRAW_SECRETARY_MOUTH(getRandomNumber(2));
		break;
	}
}

void IgorEngine::PART_09_SECRETARY_GESTURE() {
	static const uint8 beginFrames[] = { 1, 2, 3, 4 };
	static const uint8 endFrames[] = { 3, 2, 1, 8 };
	for (uint step = 0; step < ARRAYSIZE(beginFrames); ++step) {
		drawAnimRect(0x7815, 0x578 + beginFrames[step] * 0x9BB, 53, 47);
		waitForTimer(31);
	}
	for (int step = 0; step < 32; ++step) {
		const int frame = getRandomNumber(3) + 4;
		drawAnimRect(0x7815, 0x578 + frame * 0x9BB, 53, 47);
		waitForTimer(16);
	}
	for (uint step = 0; step < ARRAYSIZE(endFrames); ++step) {
		drawAnimRect(0x7815, 0x578 + endFrames[step] * 0x9BB, 53, 47);
		waitForTimer(31);
	}
}

void IgorEngine::PART_09_UPDATE_ROOM_BACKGROUND() {
	if (_objectsState[35] == 0 && compareGameTick(61) && getRandomNumber(1) == 0)
		PART_09_DRAW_SECRETARY_FRAME(getRandomNumber(2), false);
}

void IgorEngine::PART_09_EXEC_ACTION(int action) {
	switch (action) {
	case 101: // open file cabinet
		PART_09_ACTION_101_openFileCabinet();
		break;
	case 102: // look at file cabinet
		igorSayAndWait(201, 2, 234);
		break;
	case 103: // talk to secretary
		{
			// the blocking dialogue loops never run the room's idle animation, which
			// would otherwise fight the secretary's talking frames
			const UpdateRoomBackgroundProc updateRoomBackground = _updateRoomBackground;
			_updateRoomBackground = 0;
			loadDialogueData(DLG_AdministrationSecretaryRoom);
			_updateDialogue = &IgorEngine::PART_09_UPDATE_DIALOGUE_SECRETARY;
			handleDialogue(63, 59, 0, 90, 65);
			_updateDialogue = 0;
			_updateRoomBackground = updateRoomBackground;
		}
		PART_09_APPLY_OBJECT_STATE(255);
		break;
	case 104: // look at secretary
		igorSayAndWait(203, 2, 235);
		break;
	case 105: { // look at closet
		const int text = _objectsState[34] == 0 ? 205 : (_objectsState[36] == 0 ? 218 : 219);
		igorSayAndWait(text, _objectsState[34] == 0 ? 2 : 1, _objectsState[34] == 0 ? 236 : text + 28);
		break;
	}
	case 106:
		PART_09_ACTION_106(false);
		break;
	case 107:
		if (_objectsState[34] == 0) {
			executeAction(13);
		} else {
			igorSayAndWait(217, 1, 245);
		}
		break;
	case 108: // look at intercom
		igorSayAndWait({ { 207, 1, 237 }, { 208, 1, 238 } });
		break;
	case 109:
		igorSayAndWait(_objectsState[35] == 0 ? 216 : 224, 1, _objectsState[35] == 0 ? 244 : 252);
		break;
	case 110:
		PART_09_ACTION_110();
		break;
	case 112: // look at door
		igorSayAndWait(211, 1, 240);
		break;
	case 113: // open door
		PART_09_ANIMATE_DOOR(true);
		break;
	case 114: // close door
		PART_09_ANIMATE_DOOR(false);
		break;
	case 115:
		PART_09_ACTION_106(true);
		break;
	case 116:
		if (_objectsState[27] != 0) {
			WalkData *wd = &_walkData[0];
			wd->setPos(207, 106, kFacingPositionBack, 1);
			wd->clipSkipX = 1;
			wd->clipWidth = 30;
			wd->scaleWidth = 50;
			wd->xPosChanged = 1;
			wd->dxPos = 1;
			wd->yPosChanged = 0;
			wd->dyPos = 2;
			wd->scaleHeight = 50;
			_walkDataCurrentIndex = 0;
			moveIgor(kFacingPositionBack, 1);
			_currentPart = 72;
		}
		break;
	default:
		warning("PART_09_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_09() {
	playMusic(2);
	_gameState.enableLight = 1;
	loadActionData(DAT_AdministrationSecretaryRoom);
	loadRoomData(PAL_AdministrationSecretaryRoom, IMG_AdministrationSecretaryRoom,BOX_AdministrationSecretaryRoom, MSK_AdministrationSecretaryRoom, TXT_AdministrationSecretaryRoom);
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
	_updateRoomBackground = &IgorEngine::PART_09_UPDATE_ROOM_BACKGROUND;
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
	_updateRoomBackground = 0;
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
