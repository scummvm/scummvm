/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "igor/igor.h"

namespace Igor {

static const uint8 PART_22_ANIM_DATA_1[4] = { 0, 1, 2, 1 };

void IgorEngine::PART_22_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		PART_22_ACTION_101();
		break;
	case 102:
		PART_22_ACTION_102();
		break;
	default:
		error("PART_22_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_22_ACTION_101() {
	if (_objectsState[78] == 1 && _inventoryInfo[64] == 0) {
		ADD_DIALOGUE_TEXT(203, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		for (int i = 1; i <= 3; ++i) {
			const int offset = 24141;
			for (int j = 0; j <= 29; ++j) {
				const uint8 *src = _animFramesBuffer + 0xA0E6 + PART_22_ANIM_DATA_1[i] * 1050 + j * 35;
				memcpy(_screenVGA + j * 320 + offset, src, 35);
			}
			waitForTimer(60);
		}
		addObjectToInventory(29, 64);
		PART_22_HELPER_1(255);
		ADD_DIALOGUE_TEXT(205, 1);
		ADD_DIALOGUE_TEXT(206, 1);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		_objectsState[105] = 1;
	} else {
		ADD_DIALOGUE_TEXT(201, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
	}
}

void IgorEngine::PART_22_ACTION_102() {
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	int i = 0;

	for (i = 9; i >= 0; --i) {
		WalkData *wd = &_walkData[0];
		wd->setPos(138, 123, 1, _walkCurrentFrame);
		WalkData::setNextFrame(1, _walkCurrentFrame);
		wd->clipSkipX = 1;
		wd->clipWidth = 30;
		wd->scaleWidth = 23 + i * 3;
		wd->xPosChanged = 1;
		wd->dxPos = 0;
		wd->yPosChanged = 1;
		wd->dyPos = 3;
		wd->scaleHeight = 50;
		moveIgor(wd->posNum, wd->frameNum);
		waitForTimer(15);
	}
	i = 16;
	do {
		if (compareGameTick(1, 16)) {
			memcpy(_screenTextLayer + (i * 8 + 16) * 320, _screenLayer1 + (128 - i * 8) * 320, (i * 8 + 16) * 320);
			memcpy(_screenTextLayer + (i * 8 + 16) * 320, _animFramesBuffer, (128 - i * 8) * 320);
			memcpy(_screenVGA, _screenTextLayer, 46080);
			if (i == 0) {
				i = 255;
			} else {
				--i;
			}
		}
		waitForTimer();
	} while (i != 255);
	_currentPart = 141;
}

void IgorEngine::PART_22_HELPER_1(int num) {
}

void IgorEngine::PART_22_HELPER_2() {
	int i = 16;
	do {
		if (compareGameTick(1, 16)) {
			memcpy(_screenTextLayer, _screenLayer1 + (128 - i * 8) * 320, (i * 8 + 16) * 320);
			memcpy(_screenTextLayer + (128 - i * 8) * 320, _animFramesBuffer, (i * 8 + 16) * 320);
			memcpy(_screenVGA, _screenTextLayer, 46080);
			++i;
		}
		waitForTimer();
	} while (i != 17);
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	for ( i = 0; i <= 9; ++i) {
		WalkData *wd = &_walkData[0];
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		wd->setPos(138, 123, 3, _walkCurrentFrame);
		WalkData::setNextFrame(3, _walkCurrentFrame);
		wd->clipSkipX = 1;
		wd->clipWidth = 30;
		wd->scaleWidth = 23 + i * 3;
		wd->xPosChanged = 1;
		wd->dxPos = 0;
		wd->yPosChanged = 1;
		wd->dyPos = 3;
		wd->scaleHeight = 50;
		moveIgor(3, wd->frameNum);
		waitForTimer(15);
	}
	_walkDataLastIndex = 0;
	buildWalkPathSimple(138, 123, 150, 123);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_22() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_BellChurch, IMG_BellChurch, BOX_BellChurch, MSK_BellChurch, TXT_BellChurch);
	static const int anm[] = { FRM_BellChurch1, FRM_BellChurch2, 0 };
	loadAnimData(anm);
	loadActionData(DAT_BellChurch);
	_roomDataOffsets = PART_22_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(126, 123, 193, 123);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_22_EXEC_ACTION);
	PART_22_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	fadeInPalette(768);
	PART_22_HELPER_2();
	enterPartLoop();
	while (_currentPart == 220) {
		runPartLoop();
	}
	leavePartLoop();
	fadeOutPalette(624);
}

} // namespace Igor
