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
#include "igor/statics.h"

namespace Igor {


const uint32 kPart10PanelLeft = 0x0000;

void IgorEngine::PART_10_11_DRAW_OBJECT_STATE(int num) {
	// These three original blitters are byte-for-byte duplicates in the two
	// halves of the room

	if ((num == 2 || num == 255) && _objectsState[41] == 1) {
		for (int y = 0; y <= 7; ++y)
			memcpy(_screenLayer1 + 0x7DA1 + y * 320,
					_animFramesBuffer + 0xC786 + y * 37, 37);
	}

	if (num == 3 || num == 255) {
		if (_objectsState[42] == 1) {
			for (int y = 0; y <= 14; ++y)
				memcpy(_screenLayer1 + 0x4E32 + y * 320,
						_animFramesBuffer + 0xC5E2 + y * 14, 14);
		} else if (_objectsState[42] == 2) {
			for (int y = 0; y <= 14; ++y)
				memcpy(_screenLayer1 + 0x4E32 + y * 320,
						_animFramesBuffer + 0xC6B4 + y * 14, 14);
		}
	}
}

void IgorEngine::PART_10_EXEC_ACTION(int action) {
	switch (action) {
	case 101: // look at plaque
		ADD_DIALOGUE_TEXT(201, 1, 626);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 102: //enter door
		_roomObjectAreasTable[7].deltaLum = 3;
		for (int area = 11; area <= 12; ++area)
			_roomObjectAreasTable[area].area = 3;
		--_walkDataLastIndex;
		buildWalkPath(170, 97, 100, 73);
		_walkData[_walkDataLastIndex].frameNum = 0;
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true;
		waitForIgorMove();
		_currentPart = 70;
		break;
	case 103: //look at door
		ADD_DIALOGUE_TEXT(202, 1, 627);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 104: // pick burger
		PART_10_ACTION_104_pickHamburger();
		break;
	case 105: // use anything with paperbin
		ADD_DIALOGUE_TEXT(206, 1, 630);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 106: // walk at zebra crossing
		ADD_DIALOGUE_TEXT(208, 1, 632);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 107: // look at zebra crossing
		ADD_DIALOGUE_TEXT(207, 1, 631);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 108:
		PART_10_ACTION_108_scrollLeft();
		break;
	case 109: // back to map
		_currentPart = 40;
		break;
	default:
		warning("PART_10_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_10_ACTION_104_pickHamburger() {
	if (_objectsState[38] == 1) { // already picked hamburger
		ADD_DIALOGUE_TEXT(205, 1, 629);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		return;
	}

	ADD_DIALOGUE_TEXT(203, 2, 628);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();

	static const uint8 frameSelectors[] = { 0, 1, 0, 2 };
	for (int frame = 0; frame < 4; ++frame) {
		for (int y = 0; y < 49; ++y) {
			memcpy(_screenVGA + 0x595F + y * 320,
					_animFramesBuffer + kPart10_11_Frm1 + frameSelectors[frame] * 0x682 + y * 34,
					34);
		}
		waitForTimer(31);
	}

	addObjectToInventory(17, 52);
	_objectsState[38] = 1;
}

void IgorEngine::PART_10_ACTION_108_scrollLeft() {
	uint8 *walkTable = loadData(WLK_DecanatoLeft);
	int xPos = 329;
	int yPos = 0;
	int i = 1;
	do {
		if (compareGameTick(1, 16)) {
			for (int y = 0; y <= 143; ++y) {
				memcpy(_screenLayer2 + y * 320 + i * 8, _screenLayer1 + y * 320, 320 - i * 8);
				memcpy(_screenLayer2 + y * 320, _animFramesBuffer + kPart10PanelLeft + y * 320 + 320 - i * 8, i * 8);
			}
			if (i < 9) {
				xPos -= _walkScaleTable[0x8F9 + _walkCurrentFrame];
				assert(xPos >= 260);
				yPos = walkTable[xPos - 260];
				WalkData::setNextFrame(kFacingPositionLeft, _walkCurrentFrame);
			} else {
				_walkCurrentFrame = 0;
			}
			int dstOffset = (yPos - 50) * 320 + xPos - 330 + i * 8;
			for (int row = 0; row <= 49; ++row) {
				dstOffset += 320;
				for (int col = 0; col <= 29; ++col) {
					const uint8 color = _facingIgorFrames[kFacingPositionLeft - 1][_walkCurrentFrame * 1500 + row * 30 + col];
					if (color != 0)
						_screenLayer2[dstOffset + col] = color;
				}
			}
			memcpy(_screenVGA, _screenLayer2, 46080);
			++i;
		}
		waitForTimer();
	} while (i != 41);
	free(walkTable);
	_walkData[0].setPos(xPos + 5, yPos, kFacingPositionLeft, 0);
	_walkData[0].setDefaultScale();
	_currentPart = 110;
}

void IgorEngine::PART_10() {
	_gameState.enableLight = 1;

	loadActionData(DAT_Decanato);
	loadRoomData(PAL_DecanatoLeft, IMG_DecanatoLeft, BOX_DecanatoLeft, MSK_DecanatoLeft, TXT_DecanatoLeft);
	static const int frames1[] = { FRM_Decanato1, 0 };
	static const int frames2[] = { FRM_Decanato2, 0 };
	static const int frames3[] = { FRM_Decanato3, 0 };
	static const int frames4[] = { FRM_Decanato4, 0 };
	static const int frames5[] = { FRM_Decanato5, 0 };
	loadAnimData(frames1, kPart10_11_Frm1);
	loadAnimData(frames2, kPart10_11_Frm2);
	loadAnimData(frames3, kPart10_11_Frm3);
	loadAnimData(frames4, kPart10_11_Frm4);
	loadAnimData(frames5, kPart10_11_Frm5);

	PART_10_11_DRAW_OBJECT_STATE(255);

	// Preserve the complete left panel at ANM+0, then make the right panel
	// active. The original load order is unconditional.
	memcpy(_animFramesBuffer + kPart10PanelLeft, _screenLayer1, 46080);


	loadRoomData(PAL_DecanatoRight, IMG_DecanatoRight, BOX_DecanatoRight, MSK_DecanatoRight, TXT_DecanatoRight);
	static const int hamburgerAnimation[] = { ANM_DecanatoHamburger, 0 };
	loadAnimData(hamburgerAnimation, kPart10_11_Frm1);

	SET_PAL_240_48_1();
	SET_PAL_208_96_1();

	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_10_EXEC_ACTION);
	_roomDataOffsets = PART_10_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);

	// A normal entry presents the freshly loaded right panel. On state 102
	// handoff, the original jumps directly to the walk-index setup instead,
	// preserving the completed pan (including Igor) already in screen VGA.

	if (_currentPart != 102 && !_gameStateLoaded) {
		memcpy(_screenVGA, _screenLayer1, 46080);
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
	}
	if (!restoreRoomAfterLoad()) {
		_walkDataLastIndex = 1;
		_walkDataCurrentIndex = 1;
		if (_currentPart == 100) {
			// Enter at the right edge and walk left into the room.
			_walkData[0].setPos(319, 79, kFacingPositionLeft, 0);
			_walkData[0].setDefaultScale();
			_walkData[0].clipWidth = 15;
			_walkDataLastIndex = 0;
			buildWalkPath(319, 79, 288, 84);
			_walkData[_walkDataLastIndex].frameNum = 0;
			_walkDataCurrentIndex = 1;
			_gameState.igorMoving = true;
			waitForIgorMove();
		} else if (_currentPart == 101) {
			// Enter from the adjacent panel and walk right into the room.
			_walkData[0].setPos(136, 86, kFacingPositionRight, 0);
			_walkData[0].setDefaultScale();
			_walkDataLastIndex = 0;
			buildWalkPath(136, 86, 171, 97);
			_walkData[_walkDataLastIndex].frameNum = 0;
			_walkDataCurrentIndex = 1;
			_gameState.igorMoving = true;
			waitForIgorMove();
		}
	}
	_roomObjectAreasTable[7].deltaLum = 0;
	for (int area = 11; area <= 12; ++area) {
		_roomObjectAreasTable[area].area = 0;
	}

	enterPartLoop();
	while (_currentPart >= 100 && _currentPart <= 102 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (_currentPart != 110 && !_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
