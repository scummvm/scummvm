/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "igor/igor.h"

namespace Igor {

void IgorEngine::part9DrawSecretaryFrame(int frame, bool background) {
	drawAnimRect(0x7839, frame * 0x34E, 18, 47, background);
}

void IgorEngine::part9ApplyObjectState(int num) {
	if (num == 1 || num == 255) {
		const uint32 srcOffset = _objectsState[kSecretaryDoorOpenStateIndex] == 0 ? 0x5D0B : 0x61A3;
		copyArea(_screenLayer1, 0x46C2, 320, _animFramesBuffer + srcOffset, 24, 24, 49);
		_roomActionsTable[150] = _objectsState[kSecretaryDoorOpenStateIndex] == 0 ? 6 : 7;
	}
	if (num == 2 || num == 255) {
		const uint32 srcOffset = _objectsState[kSecretaryClosetOpenStateIndex] == 0 ? 0x663B : 0x6A23;
		copyArea(_screenLayer1, 0x48E6, 320, _animFramesBuffer + srcOffset, 20, 20, 50);
		_roomObjectAreasTable[4].object = (_objectsState[kSecretaryClosetOpenStateIndex] == 0 || _objectsState[kSecretaryClosetStateIndex] != 0) ? 3 : 5;
	}
	if (num == 3 || num == 255) {
		if (_objectsState[kSecretaryDismissedStateIndex] == 0) {
			part9DrawSecretaryFrame(1, true);
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

void IgorEngine::part9AnimateDoor(bool open) {
	if ((_objectsState[kSecretaryDoorOpenStateIndex] != 0) == open) {
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
	_objectsState[kSecretaryDoorOpenStateIndex] = open ? 1 : 0;
	part9ApplyObjectState(1);
}

void IgorEngine::part9DrawOpenClosetFrame(int frame) {
	drawAnimRect(0x48E6, 0x6E0B + frame * 0x639, 27, 59);
}

void IgorEngine::part9OpenCloset(bool hairpin) {
	if (_objectsState[kSecretaryDismissedStateIndex] == 0) { // if secretary present, she will prevent searching the closet
		ADD_DIALOGUE_TEXT(214, 2, 243);
	} else if (_objectsState[kSecretaryClosetOpenStateIndex] == 1) {
		executeAction(10);
		return;
	} else if (hairpin) {
		static const uint8 frames[] = { 1, 2, 1, 2, 1, 2, 3 };
		for (uint i = 0; i < ARRAYSIZE(frames); ++i) {
			part9DrawOpenClosetFrame(frames[i]);
			waitForTimer(61);
		}
		_objectsState[kSecretaryClosetOpenStateIndex] = 1;
		part9ApplyObjectState(2);
		ADD_DIALOGUE_TEXT(212, 1, 241);
	} else { // igor tries to open
		static const uint8 frames[] = { 1, 2, 1, 2, 1, 2, 1, 2, 0 };
		for (uint i = 0; i < ARRAYSIZE(frames); ++i) {
			part9DrawOpenClosetFrame(frames[i]);
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

void IgorEngine::part9TakeCostumeFromCloset() {
	for (int frame = 1; frame >= 0; --frame) {
		drawAnimRect(0x48E6, 0x9BA3 + frame * 0x438, 27, 40, false, kBlendLitSprite);
		waitForTimer(127);
	}
	addObjectToInventory(kInventoryObjectMonkeySuit, 49);
	_objectsState[kSecretaryClosetStateIndex] = 1;
	part9ApplyObjectState(255);
}

void IgorEngine::part9OpenFileCabinet() {
	if (_objectsState[kSecretaryDismissedStateIndex] == 0) { // secretary dismissed
		ADD_DIALOGUE_TEXT(220, 1, 248);
	} else if (_objectsState[kPersonalFileChangedStateIndex] == 1) { // already changed file
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
		_objectsState[kPersonalFileChangedStateIndex] = 1;
		return;
	}
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
}

void IgorEngine::part9DrawSecretaryTalkingFrame(int frame) {
	drawAnimRect(0x783A, 0xD38 + frame * 0xA9, 13, 13, true);
}

void IgorEngine::part9UpdateSecretaryDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part9DrawSecretaryTalkingFrame(0);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part9DrawSecretaryTalkingFrame(getRandomNumber(2));
		break;
	}
}

void IgorEngine::part9SecretarySearchesFile() {
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

void IgorEngine::part9UpdateBackground() {
	if (_objectsState[kSecretaryDismissedStateIndex] == 0 && compareGameTick(61) && getRandomNumber(1) == 0)
		part9DrawSecretaryFrame(getRandomNumber(2), false);
}

void IgorEngine::part9ExecuteAction(int action) {
	switch (action) {
	case 101: // open file cabinet
		part9OpenFileCabinet();
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
			_updateDialogue = &IgorEngine::part9UpdateSecretaryDialogue;
			handleDialogue(63, 59, 0, 90, 65);
			_updateDialogue = 0;
			_updateRoomBackground = updateRoomBackground;
		}
		part9ApplyObjectState(255);
		break;
	case 104: // look at secretary
		igorSayAndWait(203, 2, 235);
		break;
	case 105: { // look at closet
		const int text = _objectsState[kSecretaryClosetOpenStateIndex] == 0 ? 205 : (_objectsState[kSecretaryClosetStateIndex] == 0 ? 218 : 219);
		igorSayAndWait(text, _objectsState[kSecretaryClosetOpenStateIndex] == 0 ? 2 : 1, _objectsState[kSecretaryClosetOpenStateIndex] == 0 ? 236 : text + 28);
		break;
	}
	case 106:
		part9OpenCloset(false);
		break;
	case 107: // close closet
		if (_objectsState[kSecretaryClosetOpenStateIndex] == 0) {
			executeAction(13);
		} else {
			igorSayAndWait(217, 1, 245);
		}
		break;
	case 108: // look at intercom
		igorSayAndWait({ { 207, 1, 237 }, { 208, 1, 238 } });
		break;
	case 109: //user intercom
		igorSayAndWait(_objectsState[kSecretaryDismissedStateIndex] == 0 ? 216 : 224, 1, _objectsState[kSecretaryDismissedStateIndex] == 0 ? 244 : 252);
		break;
	case 110:
		part9TakeCostumeFromCloset();
		break;
	case 112: // look at door
		igorSayAndWait(211, 1, 240);
		break;
	case 113: // open door
		part9AnimateDoor(true);
		break;
	case 114: // close door
		part9AnimateDoor(false);
		break;
	case 115: // use hairpin
		part9OpenCloset(true);
		break;
	case 116: //leave
		if (_objectsState[kSecretaryDoorOpenStateIndex] != 0) {
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
		warning("part9ExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part9() {
	playMusic(2);
	_gameState.enableLight = 1;
	loadActionData(DAT_AdministrationSecretaryRoom);
	loadRoomData(PAL_AdministrationSecretaryRoom, IMG_AdministrationSecretaryRoom,
		BOX_AdministrationSecretaryRoom, MSK_AdministrationSecretaryRoom,
		TXT_AdministrationSecretaryRoom);
	if (_currentPart == 91) {
		// This sequence uses the animation frames retained from the dean's room.
		memcpy(_screenVGA, _screenLayer1, 46080);
		part8DrawSecretaryTalkingFrame(1);
		memcpy(_screenLayer1, _screenVGA, 46080);
		fadeIn(624);

		cutsceneSayWithCallback(65, 70, 0, 59, 63, { { 226, 1, 254 } }, &IgorEngine::part8UpdateSecretaryDialogue);

		for (int frame = 5; frame <= 19; ++frame) {
			part8DrawSecretaryTalkingFrame(frame);
			if (frame < 19)
				waitForTimer(31);
			else
				playSound(14, 1);
		}
		waitForTimer(255);
		fadeOut(624);
		_objectsState[kSecretaryDoorOpenStateIndex] = 0;
		_objectsState[kSecretaryDismissedStateIndex] = 1;
		return;
	}
	static const int anim[] = {
		ANM_AdministrationSecretaryRoom1, ANM_AdministrationSecretaryRoom2,
		ANM_AdministrationSecretaryRoom3, ANM_AdministrationSecretaryRoom4,
		ANM_AdministrationSecretaryRoom5, ANM_AdministrationSecretaryRoom6,
		ANM_AdministrationSecretaryRoom7, ANM_AdministrationSecretaryRoom8,
		ANM_AdministrationSecretaryRoom9, 0
	};
	loadAnimData(anim);
	partMainSetExecuteAction(1, &IgorEngine::part9ExecuteAction);
	_roomDataOffsets = PART_09_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	part9ApplyObjectState(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	_updateRoomBackground = &IgorEngine::part9UpdateBackground;
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
