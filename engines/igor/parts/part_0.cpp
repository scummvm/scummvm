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

void IgorEngine::PART_00_APPLY_OBJECT_STATE(int num) {
	if (num == 1 || num == 255) {
		const bool open = _objectsState[8] != 0;
		const int src = open ? 0x4CFA : 0x4C6A; // open or closed bedside table depending on the flag
		copyArea(_screenLayer1, 0x8EBD, 320, _animFramesBuffer + src, 16, 16, 9);
		_roomActionsTable[74] = open ? 7 : 6;
		_roomObjectAreasTable[13].object = open ? (_objectsState[9] == 0 ? 3 : 2) : 2;
	}
	if (num == 3 || num == 255) {
		const bool open = _objectsState[10] != 0;
		const int src = open ? 0x3900 : 0x3238; // open or closed door frame depending on the flag
		copyArea(_screenLayer1, 0x5572, 320, _animFramesBuffer + src, 31, 31, 56);
		_roomActionsTable[76] = open ? 7 : 6;
		_roomObjectAreasTable[3].object = (_objectsState[11] == 0) ? 4 : 5;
	}
	if (num == 6 || num == 255) {

		const int state = _objectsState[13];
		const int src = state == 1 ? 0x143C : 0x1200; // open or closed window depending on the flag
		copyArea(_screenLayer1, 0x6DB7, 320, _animFramesBuffer + src, 26, 26, 22);
		_roomActionsTable[79] = state == 1 ? 7 : (state == 2 ? 6 : 4);
	}
}

void IgorEngine::PART_00_EXEC_ACTION(int action) {
	debugC(9, kDebugGame, "PART_00_EXEC_ACTION %d", action);
	switch (action) {
	case 101: // open window
		if (_objectsState[13] != 1) {

			animateLitAnimFrames(0, 1, 3, 0x480, 36, 32, 0x6B2C, 62, 2, 1);
			_objectsState[13] = 1;
			PART_00_APPLY_OBJECT_STATE(6);
		}
		break;
	case 102: // close window
		if (_objectsState[13] == 1) {
			animateLitAnimFrames(0, 2, 0, 0x480, 36, 32, 0x6B2C, 62, 2, 2);
			_objectsState[13] = 2;
			PART_00_APPLY_OBJECT_STATE(6);
		}
		break;
	case 103: // look at window
		igorSay(201, 3, 95);
		break;
	case 104: // go to window
		if (_objectsState[13] != 1)
			break;
		if (_objectsState[15] != 0) {
			igorSay(212, 1, 101);
			break;
		}
		animateLitAnimFrames(0x4D8A, 0, 1, 0x5DC, 30, 50, 0x6B2C, 62, -1, 0);
		_objectsState[13] = 2;
		_objectsState[15] = 1;
		_currentPart = 10;
		break;
	case 105: // look at bed
		igorSay(204, 1, 96);
		break;
	case 106: // open bedside table
		if (_objectsState[8] == 0) {
			animateLitAnimFrames(0x3FC8, 0, 2, 0x436, 22, 49, 0x693C, 126, 2, 6);
			_objectsState[8] = 1;
			PART_00_APPLY_OBJECT_STATE(1);
		}
		break;
	case 107: // close bedside table
		if (_objectsState[8] != 0) {
			animateLitAnimFrames(0x3FC8, 2, 0, 0x436, 22, 49, 0x693C, 126, 1, 6);
			_objectsState[8] = 0;
			PART_00_APPLY_OBJECT_STATE(1);
		}
		break;
	case 108: // look bedside table
		if (_objectsState[8] == 0)
			ADD_DIALOGUE_TEXT(205, 1, 97);
		else if (_objectsState[9] == 0)
			ADD_DIALOGUE_TEXT(217, 1, 104);
		else
			ADD_DIALOGUE_TEXT(218, 1, 105);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 109: // take alarm clock
		if (_objectsState[9] == 0) {
			animateLitAnimFrames(0x8410, 0, 1, 0x405, 21, 49, 0x693D, 126, -1, 0);
			addObjectToInventory(6, 41); // s3:0903; cseg206:113C-1176
			_objectsState[9] = 1;
			PART_00_APPLY_OBJECT_STATE(1);
		}
		break;
	case 110: // open closet
		if (_objectsState[10] == 0) {
			animateLitAnimFrames(0x1678, 0, 2, 0x940, 37, 64, 0x556C, 126, 2, 3);
			_objectsState[10] = 1;
			PART_00_APPLY_OBJECT_STATE(3);
		}
		break;
	case 111: // close closet
		if (_objectsState[10] != 0) {
			animateLitAnimFrames(0x1678, 2, 0, 0x940, 37, 64, 0x556C, 126, 1, 4);
			_objectsState[10] = 0;
			PART_00_APPLY_OBJECT_STATE(3);
		}
		break;
	case 112: // look at closet
		igorSay(206, 1, 98);
		break;
	case 113: // look at hole
		igorSay(207, 2, 99);
		break;
	case 114: // go through hole
		if (_objectsState[10] == 0)
			PART_00_EXEC_ACTION(110);
		animateLitAnimFrames(0x5942, 0, 2, 0x44C, 22, 50, 0x66EE, 30, -1, 0);
		_currentPart = 24;
		break;
	case 115: // look at records
		igorSay(209, 3, 100);
		break;
	case 116: // take records
		if (_objectsState[12] == 0) {
			animateLitAnimFrames(0x7C06, 0, 1, 0x405, 21, 49, 0x7146, 126, -1, 0);
			addObjectToInventory(5, 40); // s3:0902; cseg206:0F75-0FAF
			_objectsState[12] = 1;
			PART_00_APPLY_OBJECT_STATE(255);
		} else {
			igorSay(213, 2, 102);
		}
		break;
	case 117: // exit to map
		{
			_walkDataCurrentIndex = 0;
			int walkFrame = 1;
			for (int i = 9; i >= 0; --i) {
				WalkData *wd = &_walkData[0];
				wd->setPos(145, 143, kFacingPositionFront, i == 9 ? 0 : walkFrame);
				if (i != 9)
					walkFrame = walkFrame == 6 ? 1 : walkFrame + 1;
				wd->clipSkipX = 1;
				wd->clipWidth = 30;
				wd->scaleWidth = i * 3 + 23;
				wd->xPosChanged = 1;
				wd->dxPos = 0;
				wd->yPosChanged = 1;
				wd->dyPos = 3;
				wd->scaleHeight = 50;
				moveIgor(wd->posNum, wd->frameNum);
				waitForTimer(15);
			}
		}
		_currentPart = 40;
		break;
	case 118: // use bed
		igorSay({ { 219, 1, 106 }, { 220, 1, 107 } });
		break;
	default:
		error("PART_00_EXEC_ACTION unhandled action %d", action);
	}
}

void IgorEngine::PART_00_ENTRY_ANIMATION() {
	waitForTimer(62);
	if (_objectsState[10] == 0) {
		drawAnimRect(0x5572, 0x3900, 31, 56);
		// leave closet open
		_objectsState[10] = 1;
		PART_00_APPLY_OBJECT_STATE(3);
		playSound(3, 1);
		waitForTimer(30);
	}
	animateLitAnimFrames(0x6626, 0, 3, 0x578, 25, 56, 0x5F6E, 30, -1, 0);
}

void IgorEngine::PART_00_WALK_IN_FROM_CLOSET() {
	PART_00_ENTRY_ANIMATION();
	WalkData *wd = &_walkData[0];
	wd->setPos(120, 131, kFacingPositionFront, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 30;
	wd->scaleWidth = 50;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 50;
	_walkDataLastIndex = 0;
	buildWalkPath(120, 131, 120, 136);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	if (_objectsState[14] == 0) {
		igorSay(215, 2, 103);
		_objectsState[14] = 1;
	} else {
		PART_00_ENTER_FROM_BELOW();
	}
}

void IgorEngine::PART_00_ENTER_FROM_BELOW() {
	PART_00_APPLY_OBJECT_STATE(255);
	_walkDataCurrentIndex = 0;
	int walkFrame = 1;
	for (int i = 0; i <= 9; ++i) {
		WalkData *entry = &_walkData[0];
		entry->setPos(145, 143, kFacingPositionBack, walkFrame);
		walkFrame = walkFrame == 6 ? 1 : walkFrame + 1;
		entry->clipSkipX = 1;
		entry->clipWidth = 30;
		entry->scaleWidth = i * 3 + 23;
		entry->xPosChanged = 1;
		entry->dxPos = 0;
		entry->yPosChanged = 1;
		entry->dyPos = 0;
		entry->scaleHeight = 50;
		moveIgor(entry->posNum, entry->frameNum);
		waitForTimer(15);
	}
	_walkDataLastIndex = 0;
	buildWalkPath(145, 143, 125, 138);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_00() {
	playMusic(2);
	_gameState.enableLight = 1;
	loadActionData(DAT_StudentDormitoryRoom);
	loadRoomData(PAL_StudentDormitoryRoom, IMG_StudentDormitoryRoom, BOX_StudentDormitoryRoom, MSK_StudentDormitoryRoom, TXT_StudentDormitoryRoom);
	static const int animFrames[] = {
		ANM_StudentDormitoryRoom1, ANM_StudentDormitoryRoom2,
		ANM_StudentDormitoryRoom3, ANM_StudentDormitoryRoom4,
		ANM_StudentDormitoryRoom5, ANM_StudentDormitoryRoom6,
		ANM_StudentDormitoryRoom7, ANM_StudentDormitoryRoom8,
		ANM_StudentDormitoryRoom9, ANM_StudentDormitoryRoom10,
		ANM_StudentDormitoryRoom11, 0
	};
	loadAnimData(animFrames);
	_roomDataOffsets = PART_00_ROOM_DATA_OFFSETS;

	// clamps clicks to x 41..253 and y <= 143; clicks past the
	// horizontal edges also clamp to y >= 141/138
	setRoomWalkBounds(41, 0, 253, 143, 141, 138);

	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_00_EXEC_ACTION);
	PART_00_APPLY_OBJECT_STATE(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		_walkDataLastIndex = 1;
		_walkDataCurrentIndex = 1;
		if (_currentPart == 1)
			PART_00_WALK_IN_FROM_CLOSET();
		else
			PART_00_ENTER_FROM_BELOW();
	}

	enterPartLoop();
	while ((_currentPart == 0 || _currentPart == 1) && !_gameStateLoaded)
		runPartLoop();
	leavePartLoop();
	if (!_gameStateLoaded) {
		if (_currentPart == kInvalidPart)
			fadeOut(768);
		else
			fadeOut(624);
	}
}

} // End of namespace Igor
