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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "neverhood/modules/module1600_sprites.h"

namespace Neverhood {

AsCommonCar::AsCommonCar(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: AnimatedSprite(vm, 1000), _parentScene(parentScene) {

	createSurface(200, 556, 328);
	_x = x;
	_y = y;
	_destX = x;
	_destY = y;

	_inMainArea = false;
	_exitDirection = 0;
	_currPointIndex = 0;
	_hasAgainDestPoint = false;
	_stepError = 0;
	_hasAgainDestPointIndex = false;
	_steps = 0;
	_isBraking = false;
	_yMoveTotalSteps = 0;
	_isBusy = false;
	_isIdle = false;
	_isMoving = true;
	_rectFlag = false;
	_newDeltaXType = -1;
	_soundCounter = 0;
	_pathPoints = NULL;
	_currMoveDirection = 0;
	_newMoveDirection = 0;

	startAnimation(0xD4220027, 0, -1);
	setDoDeltaX(getGlobalVar(V_CAR_DELTA_X));

	SetUpdateHandler(&AsCommonCar::update);
	SetMessageHandler(&AsCommonCar::handleMessage);
	SetSpriteUpdate(NULL);
}

AsCommonCar::~AsCommonCar() {
	if (_finalizeStateCb == AnimationCallback(&AsCommonCar::evTurnCarDone))
		setGlobalVar(V_CAR_DELTA_X, !getGlobalVar(V_CAR_DELTA_X));
}

void AsCommonCar::setPathPoints(NPointArray *pathPoints) {
	_pathPoints = pathPoints;
}

void AsCommonCar::update() {
	if (_newDeltaXType >= 0) {
		setDoDeltaX(_newDeltaXType);
		_newDeltaXType = -1;
	}
	AnimatedSprite::update();
	if (_hasAgainDestPoint && _yMoveTotalSteps == 0 && !_isBusy) {
		_hasAgainDestPoint = false;
		_hasAgainDestPointIndex = false;
		sendPointMessage(this, 0x2004, _againDestPoint);
	} else if (_hasAgainDestPointIndex && _yMoveTotalSteps == 0 && !_isBusy) {
		_hasAgainDestPointIndex = false;
		sendMessage(this, 0x2003, _againDestPointIndex);
	}
	updateMovement();
	updateSound();
}

void AsCommonCar::upIdle() {
	update();
	if (++_idleCounter >= _idleCounterMax)
		stIdleBlink();
	updateSound();
}

uint32 AsCommonCar::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_SCENE_LEAVE:
		SetSpriteUpdate(NULL);
		break;
	case NM_POSITION_CHANGE:
		// Set the current position without moving
		_currPointIndex = param.asInteger();
		_stepError = 0;
		_x = pathPoint(_currPointIndex).x;
		_y = pathPoint(_currPointIndex).y;
		break;
	case 0x2003:
		// Move to a point by its index
		{
			int newPointIndex = param.asInteger();
			if (_yMoveTotalSteps <= 0 && !_isBusy) {
				_destX = pathPoint(newPointIndex).x;
				_destY = pathPoint(newPointIndex).y;
				if (_currPointIndex < newPointIndex) {
					moveToNextPoint();
				} else if (_currPointIndex == newPointIndex && _stepError == 0) {
					if (_currPointIndex == 0) {
						_yMoveTotalSteps = 0;
						sendMessage(_parentScene, NM_KLAYMEN_CLIMB_LADDER, 0);
					} else if (_currPointIndex == (int)_pathPoints->size()) {
						_yMoveTotalSteps = 0;
						sendMessage(_parentScene, NM_KLAYMEN_STOP_CLIMBING, 0);
					}
				} else {
					moveToPrevPoint();
				}
			} else {
				_hasAgainDestPointIndex = true;
				_againDestPointIndex = newPointIndex;
			}
		}
		break;
	case 0x2004:
		// Move to the point closest to the parameter point
		{
			int minMatchIndex = -1;
			int minMatchDistance, distance;
			NPoint pt = param.asPoint();
			if (_yMoveTotalSteps <= 0 && !_isBusy) {
				// Check if we're already exiting (or something)
				if ((pt.x <= 20 && _exitDirection == 1) ||
					(pt.x >= 620 && _exitDirection == 3) ||
					(pt.y <= 20 && _exitDirection == 2) ||
					(pt.y >= 460 && _exitDirection == 4))
					break;
				_destX = pt.x;
				_destY = pt.y;
				minMatchDistance = calcDistance(_destX, _destY, _x, _y) + 1;
				for (int i = _currPointIndex + 1; i < (int)_pathPoints->size(); i++) {
					distance = calcDistance(_destX, _destY, pathPoint(i).x, pathPoint(i).y);
					if (distance >= minMatchDistance)
						break;
					minMatchDistance = distance;
					minMatchIndex = i;
				}
				for (int i = _currPointIndex; i >= 0; i--) {
					distance = calcDistance(_destX, _destY, pathPoint(i).x, pathPoint(i).y);
					if (distance >= minMatchDistance)
						break;
					minMatchDistance = distance;
					minMatchIndex = i;
				}
				if (minMatchIndex == -1) {
					if (_currPointIndex == 0)
						moveToPrevPoint();
					else
						SetSpriteUpdate(NULL);
				} else {
					if (minMatchIndex > _currPointIndex)
						moveToNextPoint();
					else
						moveToPrevPoint();
				}
			} else {
				_hasAgainDestPoint = true;
				_againDestPoint = pt;
			}
		}
		break;
	case NM_CAR_MOVE_TO_PREV_POINT:
		_yMoveTotalSteps = param.asInteger();
		_steps = 0;
		_isBraking = false;
		_lastDistance = 640;
		SetSpriteUpdate(&AsCommonCar::suMoveToPrevPoint);
		break;
	case NM_CAR_MOVE_TO_NEXT_POINT:
		_yMoveTotalSteps = param.asInteger();
		_steps = 0;
		_isBraking = false;
		_lastDistance = 640;
		SetSpriteUpdate(&AsCommonCar::suMoveToNextPoint);
		break;
	case NM_CAR_ENTER:
		stEnterCar();
		break;
	case NM_CAR_LEAVE:
		stLeaveCar();
		break;
	case NM_CAR_TURN:
		stTurnCar();
		break;
	case NM_CAR_AT_HOME:
		stCarAtHome();
		_newDeltaXType = param.asInteger();
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 AsCommonCar::hmAnimation(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = AsCommonCar::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (_isBusy && param.asInteger() == 0x025424A2)
			gotoNextState();
		break;
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 AsCommonCar::hmLeaveCar(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case NM_CAR_ENTER:
		stEnterCar();
		break;
	case NM_ANIMATION_STOP:
		sendMessage(_parentScene, NM_CAR_LEAVE, 0);
		SetMessageHandler(&AsCommonCar::handleMessage);
		break;
	default:
		break;
	}
	return 0;
}

void AsCommonCar::stCarAtHome() {
	bool doDeltaX = _doDeltaX;
	SetSpriteUpdate(NULL);
	_hasAgainDestPoint = false;
	_hasAgainDestPointIndex = false;
	_isBraking = false;
	_isBusy = false;
	_isIdle = false;
	_isMoving = false;
	_rectFlag = false;
	NextState(&AsCommonCar::stLeanForwardIdle);
	startAnimation(0x35698F78, 0, -1);
	setDoDeltaX(doDeltaX ? 1 : 0);
	_currMoveDirection = 0;
	_newMoveDirection = 0;
	_steps = 0;
	_idleCounter = 0;
	_idleCounterMax = _vm->_rnd->getRandomNumber(64 - 1) + 24;
	SetUpdateHandler(&AsCommonCar::upIdle);
	SetMessageHandler(&AsCommonCar::handleMessage);
	FinalizeState(&AsCommonCar::evIdleDone);
}

void AsCommonCar::updateTurnMovement() {
	if (_turnMoveStatus == 1) {
		_lastDistance = 640;
		_isIdle = false;
		_isBraking = false;
		SetSpriteUpdate(&AsCommonCar::suMoveToNextPoint);
	} else if (_turnMoveStatus == 2) {
		_lastDistance = 640;
		_isIdle = false;
		_isBraking = false;
		SetSpriteUpdate(&AsCommonCar::suMoveToPrevPoint);
	}
}

void AsCommonCar::updateMovement() {
	if (_isBraking && !_isIdle && !_isBusy) {
		gotoNextState();
		_isMoving = false;
		_isIdle = true;
		startAnimation(0x192ADD30, 0, -1);
		SetUpdateHandler(&AsCommonCar::update);
		SetMessageHandler(&AsCommonCar::hmAnimation);
		NextState(&AsCommonCar::stLeanForwardIdle);
	} else if (!_isBraking && _steps && _isIdle) {
		gotoNextState();
		_isIdle = false;
		startAnimation(0x9966B138, 0, -1);
		SetUpdateHandler(&AsCommonCar::update);
		SetMessageHandler(&AsCommonCar::hmAnimation);
		NextState(&AsCommonCar::stUpdateMoveDirection);
	} else if (_newMoveDirection != _currMoveDirection && _isMoving && !_isBusy) {
		gotoNextState();
		_currMoveDirection = _newMoveDirection;
		stUpdateMoveDirection();
	}
}

void AsCommonCar::stEnterCar() {
	startAnimation(0xA86A9538, 0, -1);
	SetUpdateHandler(&AsCommonCar::update);
	SetMessageHandler(&AsCommonCar::hmAnimation);
	NextState(&AsCommonCar::stLeanForwardIdle);
}

void AsCommonCar::stLeaveCar() {
	startAnimation(0xA86A9538, -1, -1);
	_playBackwards = true;
	SetUpdateHandler(&AsCommonCar::update);
	SetMessageHandler(&AsCommonCar::hmLeaveCar);
}

void AsCommonCar::stLeanForwardIdle() {
	startAnimation(0x35698F78, 0, -1);
	_currMoveDirection = 0;
	_newMoveDirection = 0;
	_steps = 0;
	_idleCounter = 0;
	_idleCounterMax = _vm->_rnd->getRandomNumber(64 - 1) + 24;
	SetUpdateHandler(&AsCommonCar::upIdle);
	SetMessageHandler(&AsCommonCar::handleMessage);
	FinalizeState(&AsCommonCar::evIdleDone);
}

void AsCommonCar::evIdleDone() {
	SetUpdateHandler(&AsCommonCar::update);
}

void AsCommonCar::stIdleBlink() {
	startAnimation(0xB579A77C, 0, -1);
	_idleCounter = 0;
	_idleCounterMax = _vm->_rnd->getRandomNumber(64 - 1) + 24;
	SetUpdateHandler(&AsCommonCar::update);
	SetMessageHandler(&AsCommonCar::hmAnimation);
	NextState(&AsCommonCar::stLeanForwardIdle);
}

void AsCommonCar::stUpdateMoveDirection() {
	_isMoving = true;
	if (_currMoveDirection == 1)
		startAnimation(0xD4AA03A4, 0, -1);
	else if (_currMoveDirection == 3)
		startAnimation(0xD00A1364, 0, -1);
	else if ((_currMoveDirection == 2 && _doDeltaX) || (_currMoveDirection == 4 && !_doDeltaX))
		stTurnCar();
	else
		startAnimation(0xD4220027, 0, -1);
	setGlobalVar(V_CAR_DELTA_X, _doDeltaX ? 1 : 0);
}

void AsCommonCar::moveToNextPoint() {
	if (_currPointIndex >= (int)_pathPoints->size() - 1) {
		_yMoveTotalSteps = 0;
		sendMessage(this, NM_SCENE_LEAVE, 0);
		sendMessage(_parentScene, NM_KLAYMEN_STOP_CLIMBING, 0);
	} else {
		NPoint nextPt = pathPoint(_currPointIndex + 1);
		NPoint currPt = pathPoint(_currPointIndex);
		if (ABS(nextPt.y - currPt.y) <= ABS(nextPt.x - currPt.x) &&
			((_currMoveDirection == 2 && nextPt.x < currPt.x) ||
			(_currMoveDirection == 4 && nextPt.x >= currPt.x))) {
			if (_currMoveDirection == 2)
				_currMoveDirection = 4;
			else if (_currMoveDirection == 4)
				_currMoveDirection = 2;
			if (_isIdle)
				stTurnCarMoveToNextPoint();
			else
				stBrakeMoveToNextPoint();
		} else {
			if (_steps == 0) {
				gotoNextState();
				_isIdle = false;
				startAnimation(0x9966B138, 0, -1);
				SetMessageHandler(&AsCommonCar::hmAnimation);
				SetUpdateHandler(&AsCommonCar::update);
				NextState(&AsCommonCar::stUpdateMoveDirection);
			}
			_isBraking = false;
			SetSpriteUpdate(&AsCommonCar::suMoveToNextPoint);
			_lastDistance = 640;
		}
	}
}

void AsCommonCar::stBrakeMoveToNextPoint() {
	gotoNextState();
	_isBusy = true;
	_isBraking = true;
	startAnimation(0x192ADD30, 0, -1);
	SetUpdateHandler(&AsCommonCar::update);
	SetMessageHandler(&AsCommonCar::hmAnimation);
	NextState(&AsCommonCar::stTurnCarMoveToNextPoint);
}

void AsCommonCar::stTurnCar() {
	// Turn to left/right #1
	gotoNextState();
	_isBusy = true;
	startAnimation(0xF46A0324, 0, -1);
	SetUpdateHandler(&AsCommonCar::update);
	SetMessageHandler(&AsCommonCar::hmAnimation);
	FinalizeState(&AsCommonCar::evTurnCarDone);
	_turnMoveStatus = 0;
	updateTurnMovement();
}

void AsCommonCar::stTurnCarMoveToNextPoint() {
	// Turn to left/right #2
	gotoNextState();
	_isBusy = true;
	startAnimation(0xF46A0324, 0, -1);
	SetUpdateHandler(&AsCommonCar::update);
	SetMessageHandler(&AsCommonCar::hmAnimation);
	FinalizeState(&AsCommonCar::evTurnCarDone);
	_turnMoveStatus = 1;
	updateTurnMovement();
}

void AsCommonCar::stTurnCarMoveToPrevPoint() {
	// Turn to left/right #3
	FinalizeState(NULL);
	_isBusy = true;
	startAnimation(0xF46A0324, 0, -1);
	SetUpdateHandler(&AsCommonCar::update);
	SetMessageHandler(&AsCommonCar::hmAnimation);
	FinalizeState(&AsCommonCar::evTurnCarDone);
	_turnMoveStatus = 2;
	updateTurnMovement();
}

void AsCommonCar::moveToPrevPoint() {
	if (_currPointIndex == 0 && _stepError == 0) {
		_yMoveTotalSteps = 0;
		sendMessage(this, NM_SCENE_LEAVE, 0);
		sendMessage(_parentScene, NM_KLAYMEN_CLIMB_LADDER, 0);
	} else {
		NPoint prevPt;
		NPoint currPt;
		if (_stepError == 0) {
			prevPt = pathPoint(_currPointIndex - 1);
			currPt = pathPoint(_currPointIndex);
		} else {
			prevPt = pathPoint(_currPointIndex);
			currPt = pathPoint(_currPointIndex + 1);
		}
		if (ABS(prevPt.y - currPt.y) <= ABS(prevPt.x - currPt.x) &&
			((_currMoveDirection == 2 && prevPt.x < currPt.x) ||
			(_currMoveDirection == 4 && prevPt.x >= currPt.x))) {
			if (_currMoveDirection == 2)
				_currMoveDirection = 4;
			else if (_currMoveDirection == 4)
				_currMoveDirection = 2;
			if (_isIdle)
				stTurnCarMoveToPrevPoint();
			else
				stBrakeMoveToPrevPoint();
		} else {
			if (_steps == 0) {
				gotoNextState();
				_isIdle = false;
				startAnimation(0x9966B138, 0, -1);
				SetMessageHandler(&AsCommonCar::hmAnimation);
				SetUpdateHandler(&AsCommonCar::update);
				NextState(&AsCommonCar::stUpdateMoveDirection);
			}
			_isBraking = false;
			SetSpriteUpdate(&AsCommonCar::suMoveToPrevPoint);
			_lastDistance = 640;
		}
	}
}

void AsCommonCar::stBrakeMoveToPrevPoint() {
	FinalizeState(NULL);
	_isBusy = true;
	_isBraking = true;
	startAnimation(0x192ADD30, 0, -1);
	SetUpdateHandler(&AsCommonCar::update);
	SetMessageHandler(&AsCommonCar::hmAnimation);
	NextState(&AsCommonCar::stTurnCarMoveToPrevPoint);
}

void AsCommonCar::evTurnCarDone() {
	_isBusy = false;
	setDoDeltaX(2);
	_newMoveDirection = 0;
	stUpdateMoveDirection();
}

void AsCommonCar::suMoveToNextPoint() {
	int16 newX = _x, newY = _y;

	if (_currPointIndex >= (int)_pathPoints->size()) {
		_yMoveTotalSteps = 0;
		sendMessage(this, NM_SCENE_LEAVE, 0);
		sendMessage(_parentScene, NM_KLAYMEN_STOP_CLIMBING, 0);
		return;
	}

	if (_isBraking) {
		if (_steps <= 0) {
			sendMessage(this, NM_SCENE_LEAVE, 0);
			return;
		} else
			_steps--;
	} else if (_steps < 11)
		_steps++;

	bool firstTime = true;
	_ySteps = _steps;
	int stepsCtr = _steps;

	while (stepsCtr > 0) {
		NPoint pt1;
		NPoint pt2 = pathPoint(_currPointIndex);
		if (_currPointIndex + 1 >= (int)_pathPoints->size())
			pt1 = pathPoint(0);
		else
			pt1 = pathPoint(_currPointIndex + 1);
		int16 deltaX = ABS(pt1.x - pt2.x);
		int16 deltaY = ABS(pt1.y - pt2.y);
		if (deltaX >= deltaY) {
			_newMoveDirection = 2;
			if (pt1.x < pt2.x)
				_newMoveDirection = 4;
			if (stepsCtr + _stepError >= deltaX) {
				stepsCtr -= deltaX;
				stepsCtr += _stepError;
				_stepError = 0;
				_currPointIndex++;
				if (_currPointIndex == (int)_pathPoints->size() - 1)
					stepsCtr = 0;
				newX = pathPoint(_currPointIndex).x;
				newY = pathPoint(_currPointIndex).y;
			} else {
				_stepError += stepsCtr;
				if (pt1.x >= pt2.x)
					newX += stepsCtr;
				else
					newX -= stepsCtr;
				if (pt1.y >= pt2.y)
					newY = pt2.y + (deltaY * _stepError) / deltaX;
				else
					newY = pt2.y - (deltaY * _stepError) / deltaX;
				stepsCtr = 0;
			}
		} else {
			_newMoveDirection = 3;
			if (pt1.y < pt2.y)
				_newMoveDirection = 1;
			if (firstTime) {
				if (pt1.y >= pt2.y)
					stepsCtr += 7;
				else {
					stepsCtr -= 4;
					if (stepsCtr < 0)
						stepsCtr = 0;
				}
				_ySteps = stepsCtr;
			}
			if (stepsCtr + _stepError >= deltaY) {
				stepsCtr -= deltaY;
				stepsCtr += _stepError;
				_stepError = 0;
				_currPointIndex++;
				if (_currPointIndex == (int)_pathPoints->size() - 1)
					stepsCtr = 0;
				newX = pathPoint(_currPointIndex).x;
				newY = pathPoint(_currPointIndex).y;
			} else {
				_stepError += stepsCtr;
				if (pt1.x >= pt2.x)
					newX = pt2.x + (deltaX * _stepError) / deltaY;
				else
					newX = pt2.x - (deltaX * _stepError) / deltaY;
				if (pt1.y >= pt2.y)
					newY += stepsCtr;
				else
					newY -= stepsCtr;
				stepsCtr = 0;
			}
		}
		firstTime = false;
	}

	if (_yMoveTotalSteps != 0) {
		_x = newX;
		_y = newY;
		_yMoveTotalSteps -= _ySteps;
		if (_yMoveTotalSteps <= 0) {
			_isBraking = true;
			_yMoveTotalSteps = 0;
		}
	} else {
		int distance = calcDistance(_destX, _destY, _x, _y);
		_x = newX;
		_y = newY;
		if (newX > 20 && newX < 620 && newY > 20 && newY < 460) {
			_exitDirection = 0;
			_inMainArea = true;
		} else if (_inMainArea) {
			_destX = pathPoint(_pathPoints->size() - 1).x;
			_destY = pathPoint(_pathPoints->size() - 1).y;
			_inMainArea = false;
			if (_x <= 20)
				_exitDirection = 1;
			else if (_x >= 620)
				_exitDirection = 3;
			else if (_y <= 20)
				_exitDirection = 2;
			else if (_y >= 460)
				_exitDirection = 4;
			if (_exitDirection != 0 && _isBraking) {
				_isBraking = false;
				_steps = 11;
			}
		}
		if ((distance < 20 && _exitDirection == 0 && _lastDistance < distance) ||
			(_exitDirection == 0 && _lastDistance + 20 < distance))
			_isBraking = true;
		if (distance < _lastDistance)
			_lastDistance = distance;
		if (_currPointIndex == (int)_pathPoints->size() - 1) {
			_isBraking = true;
			_yMoveTotalSteps = 0;
			sendMessage(this, NM_SCENE_LEAVE, 0);
			sendMessage(_parentScene, NM_KLAYMEN_STOP_CLIMBING, 0);
		}
	}

}

void AsCommonCar::suMoveToPrevPoint() {
	int16 newX = _x, newY = _y;

	if (_currPointIndex == 0 && _stepError == 0) {
		_yMoveTotalSteps = 0;
		sendMessage(this, NM_SCENE_LEAVE, 0);
		sendMessage(_parentScene, NM_KLAYMEN_CLIMB_LADDER, 0);
		return;
	}

	if (_isBraking) {
		if (_steps <= 0) {
			sendMessage(this, NM_SCENE_LEAVE, 0);
			return;
		} else
			_steps--;
	} else if (_steps < 11)
		_steps++;

	bool firstTime = true;
	_ySteps = _steps;
	int stepsCtr = _steps;

	while (stepsCtr > 0) {
		if (_stepError == 0)
			_currPointIndex--;
		NPoint pt1;
		NPoint pt2 = pathPoint(_currPointIndex);
		if (_currPointIndex + 1 >= (int)_pathPoints->size())
			pt1 = pathPoint(0);
		else
			pt1 = pathPoint(_currPointIndex + 1);
		int16 deltaX = ABS(pt1.x - pt2.x);
		int16 deltaY = ABS(pt1.y - pt2.y);
		if (deltaX >= deltaY) {
			_newMoveDirection = 4;
			if (pt1.x < pt2.x)
				_newMoveDirection = 2;
			if (_stepError == 0)
				_stepError = deltaX;
			if (stepsCtr > _stepError) {
				stepsCtr -= _stepError;
				_stepError = 0;
				if (_currPointIndex == 0)
					stepsCtr = 0;
				newX = pathPoint(_currPointIndex).x;
				newY = pathPoint(_currPointIndex).y;
			} else {
				_stepError -= stepsCtr;
				if (pt1.x >= pt2.x)
					newX -= stepsCtr;
				else
					newX += stepsCtr;
				if (pt1.y >= pt2.y)
					newY = pt2.y + (deltaY * _stepError) / deltaX;
				else
					newY = pt2.y - (deltaY * _stepError) / deltaX;
				stepsCtr = 0;
			}
		} else {
			_newMoveDirection = 1;
			if (pt1.y < pt2.y)
				_newMoveDirection = 3;
			if (firstTime) {
				if (pt1.y >= pt2.y) {
					stepsCtr -= 4;
					if (stepsCtr < 0)
						stepsCtr = 0;
				} else {
					stepsCtr += 7;
				}
				_ySteps = stepsCtr;
			}
			if (_stepError == 0)
				_stepError = deltaY;
			if (stepsCtr > _stepError) {
				stepsCtr -= _stepError;
				_stepError = 0;
				if (_currPointIndex == 0)
					stepsCtr = 0;
				newX = pathPoint(_currPointIndex).x;
				newY = pathPoint(_currPointIndex).y;
			} else {
				_stepError -= stepsCtr;
				if (pt1.x >= pt2.x)
					newX = pt2.x + (deltaX * _stepError) / deltaY;
				else
					newX = pt2.x - (deltaX * _stepError) / deltaY;
				if (pt1.y >= pt2.y)
					newY -= stepsCtr;
				else
					newY += stepsCtr;
				stepsCtr = 0;
			}
		}
		firstTime = false;
	}

	if (_yMoveTotalSteps != 0) {
		_x = newX;
		_y = newY;
		_yMoveTotalSteps -= _ySteps;
		if (_yMoveTotalSteps <= 0) {
			_isBraking = true;
			_yMoveTotalSteps = 0;
		}
	} else {
		int distance = calcDistance(_destX, _destY, _x, _y);
		_x = newX;
		_y = newY;
		if (newX > 20 && newX < 620 && newY > 20 && newY < 460) {
			_exitDirection = 0;
			_inMainArea = true;
		} else if (_inMainArea) {
			_destX = pathPoint(0).x;
			_destY = pathPoint(0).y;
			_inMainArea = false;
			if (_x <= 20)
				_exitDirection = 1;
			else if (_x >= 620)
				_exitDirection = 3;
			else if (_y <= 20)
				_exitDirection = 2;
			else if (_y >= 460)
				_exitDirection = 4;
			if (_exitDirection != 0 && _isBraking) {
				_isBraking = false;
				_steps = 11;
			}
		}
		if ((distance < 20 && _exitDirection == 0 && _lastDistance < distance) ||
			(_exitDirection == 0 && _lastDistance + 20 < distance))
			_isBraking = true;
		if (distance < _lastDistance)
			_lastDistance = distance;
		if (_currPointIndex == 0 && _stepError == 0) {
			_isBraking = true;
			_yMoveTotalSteps = 0;
			sendMessage(this, NM_SCENE_LEAVE, 0);
			sendMessage(_parentScene, NM_KLAYMEN_CLIMB_LADDER, 0);
		}
	}

}

void AsCommonCar::updateSound() {
	int maxSoundCounter = 0;
	_soundCounter++;
	if (_steps != 0 && !_isIdle) {
		if (_currMoveDirection == 1)
			maxSoundCounter = 18 - _steps;
		else if (_currMoveDirection == 3) {
			maxSoundCounter = 5 - _steps;
			if (maxSoundCounter < 1)
				maxSoundCounter = 1;
		} else
			maxSoundCounter = 14 - _steps;
	} else
		maxSoundCounter = 21;
	if (_soundCounter >= maxSoundCounter) {
		sendMessage(_parentScene, 0x200D, 0);
		_soundCounter = 0;
	}
}

AsCommonIdleCarLower::AsCommonIdleCarLower(NeverhoodEngine *vm, int16 x, int16 y)
	: AnimatedSprite(vm, 0x1209E09F, 1100, x, y) {

	setDoDeltaX(1);
	startAnimation(0x1209E09F, 1, -1);
	_newStickFrameIndex = 1;
}

AsCommonIdleCarFull::AsCommonIdleCarFull(NeverhoodEngine *vm, int16 x, int16 y)
	: AnimatedSprite(vm, 0x1209E09F, 100, x, y) {

	setDoDeltaX(1);
	_newStickFrameIndex = 0;
}

AsCommonCarConnector::AsCommonCarConnector(NeverhoodEngine *vm, AsCommonCar *asCar)
	: AnimatedSprite(vm, 1100), _asCar(asCar) {

	createSurface1(0x60281C10, 150);
	startAnimation(0x60281C10, -1, -1);
	_newStickFrameIndex = STICK_LAST_FRAME;
	SetUpdateHandler(&AsCommonCarConnector::update);
}

void AsCommonCarConnector::update() {
	_x = _asCar->getX();
	_y = _asCar->getY();
	AnimatedSprite::update();
}

void Tracks::findTrackPoint(NPoint pt, int &minMatchTrackIndex, int &minMatchDistance,
	DataResource &dataResource) {
	const uint trackCount = size();
	minMatchTrackIndex = -1;
	minMatchDistance = 640;
	for (uint trackIndex = 0; trackIndex < trackCount; trackIndex++) {
		NPointArray *pointList = dataResource.getPointArray((*this)[trackIndex]->trackPointsName);
		for (uint pointIndex = 0; pointIndex < pointList->size(); pointIndex++) {
			NPoint testPt = (*pointList)[pointIndex];
			int distance = calcDistance(testPt.x, testPt.y, pt.x, pt.y);
			if (distance < minMatchDistance) {
				minMatchTrackIndex = trackIndex;
				minMatchDistance = distance;
			}
		}
	}
}

KmScene1608::KmScene1608(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene1608::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x2032:
		_isSittingInTeleporter = param.asInteger() != 0;
		messageResult = 1;
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stSitIdleTeleporter);
		else
			GotoState(&Klaymen::stTryStandIdle);
		break;
	case NM_KLAYMEN_PICKUP:
		if (param.asInteger() == 2)
			GotoState(&Klaymen::stPickUpNeedle);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stPickUpTube);
		else
			GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case NM_KLAYMEN_TURN_TO_USE:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stTurnToUseInTeleporter);
		break;
	case NM_KLAYMEN_RETURN_FROM_USE:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stReturnFromUseInTeleporter);
		break;
	case 0x481F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else
			GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x4834:
		GotoState(&Klaymen::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2032, 1);
		_isSittingInTeleporter = true;
		GotoState(&Klaymen::stSitInTeleporter);
		break;
	case 0x4836:
		sendMessage(_parentScene, 0x2032, 0);
		_isSittingInTeleporter = false;
		GotoState(&Klaymen::stGetUpFromTeleporter);
		break;
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	default:
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
