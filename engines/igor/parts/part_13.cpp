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

void IgorEngine::part13ExecuteAction(int action) {
	debugC(9, kDebugGame, "part13ExecuteAction %d", action);
	switch (action) {
	case 101: // go to platform
		part13GoToPlatform();
		break;
	case 102: // look at stained glass
		igorSay(201, 2, 858);
		break;
	case 103: // look at platform
		part13GoToPlatform();
		break;
	case 104:
		part13ExitChurch();
		break;
	case 105: // go to church puzzle
		_currentPart = 140;
		break;
	default:
		error("part13ExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part13GoToPlatform() {
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
	igorSayAndWait(203, 1, 859);
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

void IgorEngine::part13ExitChurch() {
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

void IgorEngine::part13ApplyObjectState(int num) {
}

void IgorEngine::part13EnterFromRight() {
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

void IgorEngine::part13EnterFromBelow() {
	if (!isCDVersion())
		playMusic(4, true);
	part13ApplyObjectState(255);
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

void IgorEngine::part13() {
	if (isCDVersion())
		playMusic(3);
	_gameState.enableLight = 1;
	loadRoomData(PAL_InsideChurch, IMG_InsideChurch, BOX_InsideChurch, MSK_InsideChurch, TXT_InsideChurch);
	loadActionData(DAT_InsideChurch);
	_roomDataOffsets = PART_13_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(92, 0, 288, 143);
	partMainSetExecuteAction(1, &IgorEngine::part13ExecuteAction);
	part13ApplyObjectState(255);
	if (!restoreRoomAfterLoad()) {
		memcpy(_screenVGA, _screenLayer1, 46080);
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 130) {
			part13EnterFromBelow();
		} else {
			part13EnterFromRight();
		}
		enterPartLoop();
	}
	while (_currentPart >= 130 && _currentPart <= 131 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
