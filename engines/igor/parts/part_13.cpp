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

void IgorEngine::PART_13_EXEC_ACTION(int action) {
	debugC(9, kDebugGame, "PART_13_EXEC_ACTION %d", action);
	switch (action) {
	case 101:
		PART_13_ACTION_101_103();
		break;
	case 102:
		ADD_DIALOGUE_TEXT(201, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 103:
		PART_13_ACTION_101_103();
		break;
	case 104:
		PART_13_ACTION_104();
		break;
	case 105:
		_currentPart = 140;
		break;
	default:
		error("PART_13_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_13_ACTION_101_103() {
	waitForTimer(255);
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	for (int i = 0; i <= 4; ++i) {
		WalkData *wd = &_walkData[0];
		if (i == 4) {
			_walkCurrentFrame = 0;
		}
		wd->setPos(52 + i * 5, 88, 2, _walkCurrentFrame);
		WalkData::setNextFrame(2, _walkCurrentFrame);
		wd->clipSkipX = 1;
		wd->clipWidth = 24;
		wd->scaleWidth = 40;
		wd->xPosChanged = 1;
		wd->dxPos = 5;
		wd->yPosChanged = 1;
		wd->dyPos = 0;
		wd->scaleHeight = 40;
		moveIgor(wd->posNum, wd->frameNum);
		waitForTimer(30);
	}
	_walkDataLastIndex = 1;
	ADD_DIALOGUE_TEXT(203, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	_walkData[0].posNum = 3;
	moveIgor(_walkData[0].posNum, _walkData[0].frameNum);
	waitForTimer(30);
	_walkCurrentFrame = 1;
	for (int i = 4; i >= 0; --i) {
		WalkData *wd = &_walkData[0];
		if (i == 0) {
			_walkCurrentFrame = 0;
		}
		wd->setPos(52 + i * 5, 88, 4, _walkCurrentFrame);
		WalkData::setNextFrame(4, _walkCurrentFrame);
		wd->clipSkipX = 1;
		wd->clipWidth = 24;
		wd->scaleWidth = 40;
		wd->xPosChanged = 1;
		wd->dxPos = 5;
		wd->yPosChanged = 1;
		wd->dyPos = 0;
		wd->scaleHeight = 40;
		moveIgor(wd->posNum, wd->frameNum);
		waitForTimer(30);
	}
	waitForTimer(255);
	WalkData *wd = &_walkData[0];
	wd->setPos(70, 127, 2, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 13;
	wd->scaleWidth = 22;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 22;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPath(70, 127, 92, 127);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	_gameState.unk10 = 1;
	_gameTicks = 0;
	do {
		if (compareGameTick(1, 16)) {
			if (_walkDataCurrentIndex > _walkDataLastIndex) {
				_gameState.igorMoving = false;
				_walkDataLastIndex = _walkDataCurrentIndex;
			}
			if (_gameState.igorMoving) {
				moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
				++_walkDataCurrentIndex;
			}
		}
		waitForTimer();
	} while (_gameState.igorMoving);
}

void IgorEngine::PART_13_ACTION_104() {
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	for (int i = 9; i >= 0; --i) {
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		_walkData[0].setPos(189, 143, 3, _walkCurrentFrame);
		WalkData::setNextFrame(3, _walkCurrentFrame);
		_walkData[0].clipSkipX = 1;
		_walkData[0].clipWidth = 13;
		_walkData[0].scaleWidth = 13 + i;
		_walkData[0].xPosChanged = 1;
		_walkData[0].dxPos = 0;
		_walkData[0].yPosChanged = 1;
		_walkData[0].dyPos = 3;
		_walkData[0].scaleHeight = 22;
		moveIgor(_walkData[0].posNum, _walkData[0].frameNum);
		waitForTimer(15);
	}
	_currentPart = 121;
}

void IgorEngine::PART_13_HELPER_1(int num) {
}

void IgorEngine::PART_13_HELPER_2() {
	WalkData *wd = &_walkData[0];
	wd->setPos(314, 127, 4, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 13;
	wd->scaleWidth = 22;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 22;
	_walkDataLastIndex = 0;
	buildWalkPath(314, 127, 288, 127);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_13_HELPER_3() {
	playMusic(4);
	PART_13_HELPER_1(255);
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	for (int i = 0; i <= 9; ++i) {
		WalkData *wd = &_walkData[0];
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		wd->setPos(189, 143, 1, _walkCurrentFrame);
		WalkData::setNextFrame(1, _walkCurrentFrame);
		wd->clipSkipX = 1;
		wd->clipWidth = 13;
		wd->scaleWidth = i + 13;
		wd->xPosChanged = 1;
		wd->dxPos = 0;
		wd->yPosChanged = 1;
		wd->dyPos = 0;
		wd->scaleHeight = 22;
		moveIgor(wd->posNum, wd->frameNum);
		waitForTimer(15);
	}
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;
}

void IgorEngine::PART_13() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_InsideChurch, IMG_InsideChurch, BOX_InsideChurch, MSK_InsideChurch, TXT_InsideChurch);
	loadActionData(DAT_InsideChurch);
	_roomDataOffsets = PART_13_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(92, 0, 288, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_13_EXEC_ACTION);
	PART_13_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	fadeInPalette(768);
	if (_currentPart == 130) {
		PART_13_HELPER_3();
	} else {
		PART_13_HELPER_2();
	}
	enterPartLoop();
	while (_currentPart >= 130 && _currentPart <= 131) {
		runPartLoop();
	}
	leavePartLoop();
	fadeOutPalette(624);
}

} // namespace Igor
