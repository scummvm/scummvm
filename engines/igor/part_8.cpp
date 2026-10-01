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
		const uint32 srcOffset = _objectsState[52] == 0 ? 0 : 0x4C8;
		for (int y = 0; y <= 50; ++y)
			memcpy(_screenLayer1 + 0x572C + y * 320, _animFramesBuffer + srcOffset + y * 24, 24);
		_roomActionsTable[150] = _objectsState[52] == 0 ? 6 : 7;
	}
	if (num == 2 || num == 255) {
		// TODO: map the independent s3:0x8AB flag used by cseg189:16EF.
		uint32 srcOffset;
		int dstOffset;
		int rows;
		int width;
		if (_objectsState[55] == 0) {
			srcOffset = 0x208A;
			dstOffset = 0x6BC3;
			rows = 26;
			width = 34;
			_roomObjectAreasTable[8].object = 0;
		} else {
			srcOffset = _objectsState[57] == 0 ? 0x9992 : 0x985C;
			dstOffset = 0x7FC8;
			rows = 10;
			width = 31;
			_roomObjectAreasTable[8].object = _objectsState[57] == 0 ? 3 : 0;
		}
		for (int y = 0; y < rows; ++y) {
			const uint8 *src = _animFramesBuffer + srcOffset + y * width;
			memcpy(_screenVGA + dstOffset + y * 320, src, width);
			memcpy(_screenLayer1 + dstOffset + y * 320, src, width);
		}
		_roomActionsTable[146] = 4;
	}
}

void IgorEngine::PART_08_ANIMATE_DOOR(bool open) {
	if ((_objectsState[52] != 0) == open) {
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
	_objectsState[52] = open ? 1 : 0;
	PART_08_APPLY_OBJECT_STATE(1);
}

void IgorEngine::PART_08_DRAW_DEAN() {
	for (int y = 0; y <= 25; ++y) { // cseg189:08D3-093B
		const uint8 *src = _animFramesBuffer + 0x23FE + y * 34; // cseg189:08E3-08EE
		memcpy(_screenVGA + 0x6BC3 + y * 320, src, 34); // cseg189:08D3-0908
		memcpy(_screenLayer1 + 0x6BC3 + y * 320, src, 34); // cseg189:090D-0931
	}
}

void IgorEngine::PART_08_DRAW_DEAN_DIALOGUE_FRAME(int frame) {
	for (int y = 0; y <= 21; ++y) { // cseg189:093F-09CF
		const uint8 *src = _animFramesBuffer + 0x4E6B + frame * 0x226 + y * 25; // cseg189:095D-0975
		memcpy(_screenVGA + 0x6BC6 + y * 320, src, 25); // cseg189:094D-098F
		memcpy(_screenLayer1 + 0x6BC6 + y * 320, src, 25); // cseg189:0994-09C5
	}
}

void IgorEngine::PART_08_UPDATE_DIALOGUE_DEAN(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		PART_08_DRAW_DEAN_DIALOGUE_FRAME(0); // cseg194:0B8F-0BAF
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_08_DRAW_DEAN_DIALOGUE_FRAME(getRandomNumber(6)); // cseg194:0C33-0C3B
		break;
	}
}

void IgorEngine::PART_08_ACTION_105() {
	for (int frame = 0; frame <= 1; ++frame) { // cseg189:062A-0698
		for (int y = 0; y <= 48; ++y) { // cseg189:0635-067E
			const uint8 *src = _animFramesBuffer + 0x125E + frame * 0x55C + y * 28; // cseg189:0645-065A
			memcpy(_screenVGA + 0x66E3 + y * 320, src, 28); // cseg189:0635-0674
		}
		if (frame == 0)
			waitForTimer(127); // cseg189:0680-0691
	}
	addObjectToInventory(15, 50); // cseg189:069A-06D4; inventory object 15 is slot 50
	_objectsState[57] = 1; // cseg189:06DE
	PART_08_APPLY_OBJECT_STATE(255); // cseg189:06D9-06E5
}

void IgorEngine::PART_08_HANDLE_DIALOGUE_DEAN() {
	loadDialogueData(DLG_DeanPepperOffice); // cseg194:0CD4-0CF7
	_updateDialogue = &IgorEngine::PART_08_UPDATE_DIALOGUE_DEAN;
	handleDialogue(78, 75, 26, 58, 0); // cseg194:0AD5-0AF0
	_updateDialogue = 0;
}

void IgorEngine::PART_08_ACTION_103_TALK_TO_DEAN() {
	if (_objectsState[55] == 1) { // cseg189:128F-12B7
		ADD_DIALOGUE_TEXT(223, 1, 169); // cseg189:1296-12A2
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		return;
	}

	if (_objectsState[52] == 1) { // cseg189:12BA-12C4
		PART_08_DRAW_DEAN(); // cseg189:12C4
		ADD_DIALOGUE_TEXT(220, 1, 166); // cseg189:12C9-1328
		ADD_DIALOGUE_TEXT(221, 1, 167); // cseg189:12C9-1328
		SET_DIALOGUE_TEXT(1, 2); // cseg189:132A-132F
		_updateDialogue = &IgorEngine::PART_08_UPDATE_DIALOGUE_DEAN;
		startCutsceneDialogue(78, 75, 26, 58, 0); // cseg189:1334-1343
		waitForEndOfCutsceneDialogue(78, 75, 26, 58, 0); // cseg189:1348-14BA
		_updateDialogue = 0;
		return; // cseg189:14BD jumps to the function epilogue at 14D0
	}

	PART_08_HANDLE_DIALOGUE_DEAN(); // cseg189:14BF
	PART_08_APPLY_OBJECT_STATE(255); // cseg189:14C4-14CB
}

void IgorEngine::PART_08_ACTION_114() {
	if (_objectsState[55] == 1) { // cseg189:09E3-0A0B
		ADD_DIALOGUE_TEXT(227, 1, 172); // cseg189:09EA-09F6
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		return;
	}
	if (_objectsState[26] == 1) { // cseg189:0A0E-0A36; s3:0x83C maps to object state 26
		ADD_DIALOGUE_TEXT(226, 1, 171); // cseg189:0A15-0A21
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		return;
	}

	PART_08_DRAW_DEAN(); // cseg189:0A39
	ADD_DIALOGUE_TEXT(210, 2, 158); // cseg189:0A3E-0A4A
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();

	static const uint8 deanFrames[] = { 1, 2, 3, 0 }; // dseg231:0x01DE,0x01E0,0x01E2,0x01E4
	static const uint8 igorFrames[] = { 0, 1, 2, 3 }; // dseg231:0x01DF,0x01E1,0x01E3,0x01E5
	for (int step = 0; step < 4; ++step) { // cseg189:0A64-0B48
		if (deanFrames[step] != 0) {
			for (int y = 0; y <= 48; ++y) { // cseg189:0A7D-0AD1
				const uint8 *src = _animFramesBuffer + 0x5755 + deanFrames[step] * 0x620 + y * 32; // cseg189:0A8D-0AAD
				memcpy(_screenVGA + 0x6463 + y * 320, src, 32); // cseg189:0A7D-0AC7
			}
		}
		if (igorFrames[step] != 0) {
			for (int y = 0; y <= 24; ++y) { // cseg189:0AE0-0B33
				const uint8 *src = _animFramesBuffer + 0x6D7D + igorFrames[step] * 0x258 + y * 24; // cseg189:0AF0-0B0F
				memcpy(_screenVGA + 0x6D06 + y * 320, src, 24); // cseg189:0AE0-0B29
			}
		}
		waitForTimer(41); // cseg189:0B35-0B3F
	}

	// The original removes object slot 58 here and plays sound 63 only after
	// the conversation has completed. cseg189:0B4B-0B7C,0D86-0D95.
	if (_inventoryInfo[58] != 0) {
		_inventoryInfo[_inventoryInfo[58] - 1] = 0;
		_inventoryInfo[58] = 0;
		packInventory();
		if (_inventoryInfo[72] > _inventoryInfo[73])
			_inventoryInfo[72] = _inventoryOffsetTable[(_inventoryInfo[73] - 1) / 7];
		drawInventory(_inventoryInfo[72], 0);
	}

	ADD_DIALOGUE_TEXT(216, 1, 162); // cseg189:0B7F-0BDE
	ADD_DIALOGUE_TEXT(217, 1, 163); // cseg189:0B7F-0BDE
	ADD_DIALOGUE_TEXT(218, 1, 164); // cseg189:0B7F-0BDE
	ADD_DIALOGUE_TEXT(219, 1, 165); // cseg189:0B7F-0BDE
	SET_DIALOGUE_TEXT(1, 4); // cseg189:0BE0-0BE5
	_updateDialogue = &IgorEngine::PART_08_UPDATE_DIALOGUE_DEAN;
	startCutsceneDialogue(78, 75, 26, 58, 0); // cseg189:0BEA-0BF9
	waitForEndOfCutsceneDialogue(78, 75, 26, 58, 0); // cseg189:0BFE-0D70
	_updateDialogue = 0;
	_objectsState[56] = _objectsState[26] < 2 ? 1 : 2; // cseg189:0D73-0D81
	drawInventory(_inventoryInfo[72], 0); // cseg189:0D86-0D8C
	playSound(63, 1); // cseg189:0D91-0D95
	PART_08_APPLY_OBJECT_STATE(255); // cseg189:0D9A-0DA1
}

void IgorEngine::PART_08_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		if (_objectsState[52] == 0) { // cseg189:10D6-10DD,1229
			_currentPart = 71;
			break;
		}
		if (_objectsState[52] != 0) {
			for (int area = 10; area <= 11; ++area)
				_roomObjectAreasTable[area].area = 4;
			for (int area = 13; area <= 14; ++area)
				_roomObjectAreasTable[area].area = 4;
			--_walkDataLastIndex;
			buildWalkPath(242, 121, 270, 108);
			_walkDataCurrentIndex = 1;
			_gameState.igorMoving = true;
			waitForIgorMove();
			if (_objectsState[56] != 0) {
				// TODO: translate cseg189:11C7-1224 before enabling the Dean's post-exit sequence.
				break;
			}
			_currentPart = 71;
		}
		break;
	case 102:
		ADD_DIALOGUE_TEXT(202, 1, 153);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 104:
		if (_objectsState[55] == 1)
			ADD_DIALOGUE_TEXT(222, 1, 168);
		else
			ADD_DIALOGUE_TEXT(203, 2, 154);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 106:
	case 107: // Action 107 maps to sub_189_0E2C. cseg189:1586-15A4
		ADD_DIALOGUE_TEXT(205, 2, 155);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 110:
		ADD_DIALOGUE_TEXT(207, 2, 156);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 111:
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
		if (_objectsState[55] == 0) { // cseg189:0704-072C
			ADD_DIALOGUE_TEXT(212, 1, 159); // cseg189:070B-0717
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
			waitForEndOfIgorDialogue();
		} else if (_objectsState[58] == 1) { // cseg189:072F-0757; s3:0x85C maps to object state 58
			ADD_DIALOGUE_TEXT(224, 2, 170); // cseg189:0736-0742
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
			waitForEndOfIgorDialogue();
		} else {
			// TODO: translate cseg189:075A-08BE (secretary-room transition sequence).
		}
		break;
	case 109:
		if (_objectsState[55] == 0) { // cseg189:0406-042E
			ADD_DIALOGUE_TEXT(213, 2, 160); // cseg189:040D-0419
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
			waitForEndOfIgorDialogue();
		} else if (_objectsState[54] == 1) { // cseg189:0431-0459; s3:0x858 maps to object state 54
			ADD_DIALOGUE_TEXT(215, 1, 161); // cseg189:0438-0444
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
			waitForEndOfIgorDialogue();
		} else {
			// TODO: translate cseg189:045C-0615 (palette-aware pickup animation).
		}
		break;
	case 114:
		PART_08_ACTION_114();
		break;
	case 112:
		PART_08_ANIMATE_DOOR(true);
		break;
	case 113:
		PART_08_ANIMATE_DOOR(false);
		break;
	default:
		warning("PART_08_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_08() {
	_gameState.enableLight = 1;
	loadActionData(DAT_DeanPepperOffice); // static resource: cseg189:34D7-34FA
	loadRoomData(PAL_DeanPepperOffice, IMG_DeanPepperOffice,
			BOX_DeanPepperOffice, MSK_DeanPepperOffice, TXT_DeanPepperOffice); // static resources: cseg195/cseg196
	static const int anim[] = {
		FRM_DeanPepperOffice1, FRM_DeanPepperOffice2,
		FRM_DeanPepperOffice3, FRM_DeanPepperOffice4,
		FRM_DeanPepperOffice5, FRM_DeanPepperOffice6,
		FRM_DeanPepperOffice7, FRM_DeanPepperOffice8,
		FRM_DeanPepperOffice9, FRM_DeanPepperOffice10,
		FRM_DeanPepperOffice11, FRM_DeanPepperOffice12,
		FRM_DeanPepperOffice13, FRM_DeanPepperOffice14,
		FRM_DeanPepperOffice15, FRM_DeanPepperOffice16, 0
	}; // static resources: cseg195:0002-0337
	loadAnimData(anim);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_08_EXEC_ACTION);
	_roomDataOffsets = PART_08_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	PART_08_APPLY_OBJECT_STATE(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	_currentAction.verb = kVerbWalk;
	_walkData[0].setPos(241, 121, kFacingPositionLeft, 0);
	_walkData[0].setDefaultScale();
	_walkDataCurrentIndex = 0;
	moveIgor(kFacingPositionLeft, 0);
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;
	fadeIn(768);

	enterPartLoop();
	while (_currentPart == 80)
		runPartLoop();
	leavePartLoop();
	fadeOut(624);
}

} // End of namespace Igor
