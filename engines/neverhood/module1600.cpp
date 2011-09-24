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
 */

#include "neverhood/module1600.h"
#include "neverhood/gamemodule.h"
#include "neverhood/module1200.h"
#include "neverhood/module2200.h"

namespace Neverhood {

Module1600::Module1600(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	if (which < 0) {
		createScene(_vm->gameState().sceneNum, -1);
	} else if (which == 1) {
		createScene(4, 1);
	} else if (which == 2) {
		createScene(5, 0);
	} else if (which == 3) {
		createScene(6, 1);
	} else if (which == 4) {
		createScene(1, 0);
	} else {
		createScene(0, 0);
	}

	// TODO Sound1ChList_addSoundResources(0x1A008D8, dword_4B3BB0, true);
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B3BB0, true, 50, 600, 5, 150);
	// TODO Sound1ChList_sub_407C70(0x1A008D8, 0x41861371, 0x43A2507F, 0);

}

Module1600::~Module1600() {
	// TODO Sound1ChList_sub_407A50(0x1A008D8);
}

void Module1600::createScene(int sceneNum, int which) {
	debug("Module1600::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		createNavigationScene(0x004B39D0, which);
		break;
	case 1:
		createNavigationScene(0x004B3A30, which);
		break;
	case 2:
		createNavigationScene(0x004B3A60, which);
		break;
	case 3:
		createNavigationScene(0x004B3A90, which);
		break;
	case 4:
		createNavigationScene(0x004B3B20, which);
		break;
	case 5:
		createNavigationScene(0x004B3B50, which);
		break;
	case 6:
		createNavigationScene(0x004B3B80, which);
		break;
	case 7:
		_childObject = new Scene1608(_vm, this, which);
		break;
	case 8:
		_childObject = new Scene1609(_vm, this, which);
		break;
	case 1001:
		if (getGlobalVar(0xA0808898) == 1) {
			createSmackerScene(0x80050200, true, true, false);
		} else if (getGlobalVar(0xA0808898) == 2) {
			createSmackerScene(0x80090200, true, true, false);
		} else {
			createSmackerScene(0x80000200, true, true, false);
		}
		if (getGlobalVar(0xA0808898) >= 2)
			setGlobalVar(0xA0808898, 0);
		else
			incGlobalVar(0xA0808898, +1);			
		break;
	}
	SetUpdateHandler(&Module1600::updateScene);
	_childObject->handleUpdate();
}

void Module1600::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			if (_moduleResult == 0)
				createScene(2, 0);
			else if (_moduleResult == 1)
				createScene(1, 0);
			else if (_moduleResult == 2)
				leaveModule(4);
			break;
		case 1:
			if (_moduleResult == 0)
				createScene(1001, -1);
			else if (_moduleResult == 1)
				createScene(0, 3);
			break;
		case 2:
			if (_moduleResult == 0)
				createScene(3, 0);
			else if (_moduleResult == 1)
				createScene(0, 2);
			break;
		case 3:
			if (_moduleResult == 0)
				createScene(5, 0);
			else if (_moduleResult == 2)
				createScene(6, 0);
			else if (_moduleResult == 3)
				createScene(2, 1);
			else if (_moduleResult == 4)
				createScene(4, 0);
			break;
		case 4:
			if (_moduleResult == 0)
				leaveModule(1);
			else if (_moduleResult == 1)
				createScene(3, 1);
			break;
		case 5:
			if (_moduleResult == 0)
				leaveModule(2);
			else if (_moduleResult == 1)
				createScene(3, 3);
			break;
		case 6:
			if (_moduleResult == 0)
				createScene(8, -1);
			else if (_moduleResult == 1)
				createScene(3, 5);
			break;
		case 7:
			createScene(6, 1);
			break;
		case 8:
			if (_moduleResult == 0)
				createScene(6, 0);
			else
				createScene(7, 0);
			break;
		case 1001:
			createScene(1, 0);
			break;
		}
	}
}

Class521::Class521(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: AnimatedSprite(vm, 1000), _parentScene(parentScene) {
	
	SetUpdateHandler(&Class521::update);
	SetMessageHandler(&Class521::handleMessage);
	SetSpriteCallback(NULL);

	// TODO createSurface2(200, dword_4AF4C0);
	createSurface(200, 640, 480); //TODO: Remove once the line above is done
	_x = x;
	_y = y;
	
	_field100 = 0;
	_exitDirection = 0;
	_currPointIndex = 0;
	_againDestPtFlag = 0;
	_stepError = 0;
	_againDestPointFlag = 0;
	_steps = 0;
	_flag10E = 0;
	_moreY = 0;
	_flag10F = 0;
	_flag113 = 0;
	_flag114 = 1;
	_flag11A = 0;
	_newDeltaXType = -1;
	_field11E = 0;
	_pathPoints = NULL;
	_rectList = NULL;
	
	setFileHash(0xD4220027, 0, -1);
	setDoDeltaX(getGlobalVar(0x21E60190));

}

Class521::~Class521() {
	if (_callback1Cb == AnimationCallback(&Class521::sub45D620)) {
		setGlobalVar(0x21E60190, !getGlobalVar(0x21E60190));
	}
}

void Class521::setPathPoints(NPointArray *pathPoints) {
	_pathPoints = pathPoints;
}

void Class521::update() {
	if (_newDeltaXType >= 0) {
		setDoDeltaX(_newDeltaXType);
		_newDeltaXType = -1;
	}
	AnimatedSprite::update();
	if (_againDestPtFlag && _moreY == 0 && !_flag10F) {
		_againDestPtFlag = 0;
		_againDestPointFlag = 0;
		sendPointMessage(this, 0x2004, _againDestPt);
	} else if (_againDestPointFlag && _moreY == 0 && !_flag10F) {
		_againDestPointFlag = 0;
		sendMessage(this, 0x2003, _againDestPointIndex);
	}
	sub45CE10();
	sub45E0A0();
}

void Class521::update45C790() {
	Class521::update();
	if (++_idleCounter >= _idleCounterMax)
		sub45D050();
	sub45E0A0();
}

uint32 Class521::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1019:
		SetSpriteCallback(NULL);
		break;
	/* NOTE: Implemented in setPathPoints
	case 0x2000:
	case 0x2001:
	*/
	case 0x2002:
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
			if (_moreY <= 0 && !_flag10F) {
				_someX = pathPoint(newPointIndex).x;
				_someY = pathPoint(newPointIndex).y;
				if (_currPointIndex < newPointIndex) {
					moveToNextPoint();
				} else if (_currPointIndex == newPointIndex && _stepError == 0) {
					if (_currPointIndex == 0) {
						_moreY = 0;
						sendMessage(_parentScene, 0x2005, 0);
					} else if (_currPointIndex == (int)_pathPoints->size()) {
						_moreY = 0;
						sendMessage(_parentScene, 0x2006, 0);
					}
				} else {
					moveToPrevPoint();
				}
			} else {
				_againDestPointFlag = 1;
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
			if (_moreY <= 0 && !_flag10F) {
				// Check if we're already exiting (or something)
				if ((pt.x <= 20 && _exitDirection == 1) ||
					(pt.x >= 620 && _exitDirection == 3) ||
					(pt.y <= 20 && _exitDirection == 2) ||
					(pt.y >= 460 && _exitDirection == 4))
					break;
				_someX = pt.x;
				_someY = pt.y;
				minMatchDistance = calcDistance(_someX, _someY, _x, _y) + 1;
				for (int i = _currPointIndex + 1; i < (int)_pathPoints->size(); i++) {
					distance = calcDistance(_someX, _someY, pathPoint(i).x, pathPoint(i).y);
					if (distance >= minMatchDistance)
						break;
					minMatchDistance = distance;
					minMatchIndex = i;
				}
				for (int i = _currPointIndex; i >= 0; i--) {
					distance = calcDistance(_someX, _someY, pathPoint(i).x, pathPoint(i).y);
					if (distance >= minMatchDistance)
						break;
					minMatchDistance = distance;
					minMatchIndex = i;
				}
				if (minMatchIndex == -1) {
					if (_currPointIndex == 0) {
						moveToPrevPoint();
					} else {
						SetSpriteCallback(NULL);
					}
				} else {
					if (minMatchIndex > _currPointIndex) {
						moveToNextPoint();
					} else {
						moveToPrevPoint();
					}
				}
			} else {
				_againDestPtFlag = 1;
				_againDestPt = pt;
			}
		}
		break;
	case 0x2007:
		_moreY = param.asInteger();
		_steps = 0;
		_flag10E = 0;
		SetSpriteCallback(&Class521::suMoveToPrevPoint);
		_lastDistance = 640;
		break;
	case 0x2008:
		_moreY = param.asInteger();
		_steps = 0;
		_flag10E = 0;
		SetSpriteCallback(&Class521::suMoveToNextPoint);
		_lastDistance = 640;
		break;
	case 0x2009:
		sub45CF80();
		break;
	case 0x200A:
		sub45CFB0();
		break;
	/* NOTE: Implemented in setRectList
	case 0x200B:
	case 0x200C:
	*/
	case 0x200E:
		sub45D180();
		break;
	case 0x200F:
		sub45CD00();
		_newDeltaXType = param.asInteger();
		break;
	}
	return messageResult;
}

uint32 Class521::handleMessage45CC30(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Class521::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (_flag10F && param.asInteger() == 0x025424A2) {
			removeCallbacks();
		}
		break;
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

uint32 Class521::handleMessage45CCA0(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0x2009:
		sub45CF80();
		break;
	case 0x3002:
		sendMessage(_parentScene, 0x200A, 0);
		SetMessageHandler(&Class521::handleMessage);
		break;
	}
	return 0;
}

void Class521::sub45CD00() {
	bool doDeltaX = _doDeltaX;
	SetSpriteCallback(NULL);
	_againDestPtFlag = 0;
	_againDestPointFlag = 0;
	_flag10E = 0;
	_flag10F = 0;
	_flag113 = 0;
	_flag114 = 0;
	_flag11A = 0;
	_rectList = NULL;
	SetMessageHandler(&Class521::handleMessage45CC30);
	SetAnimationCallback3(&Class521::sub45CFE0);
	setFileHash(0x35698F78, 0, -1);
	SetMessageHandler(&Class521::handleMessage45CC30);
	SetUpdateHandler(&Class521::update45C790);
	setCallback1(AnimationCallback(&Class521::sub45D040));
	setDoDeltaX(doDeltaX ? 1 : 0);
	_currMoveDirection = 0;
	_newMoveDirection = 0;
	_steps = 0;
	_idleCounter = 0;
	_idleCounterMax = _vm->_rnd->getRandomNumber(64 - 1) + 24;
}

void Class521::sub45CDC0() {
	if (_value112 == 1) {
		_lastDistance = 640;
		_flag113 = 0;
		_flag10E = 0;
		SetSpriteCallback(&Class521::suMoveToNextPoint);
	} else if (_value112 == 2) {
		_lastDistance = 640;
		_flag113 = 0;
		_flag10E = 0;
		SetSpriteCallback(&Class521::suMoveToPrevPoint);
	}
}

void Class521::sub45CE10() {
	if (_flag10E && !_flag113 && !_flag10F) {
		removeCallbacks();
		_flag114 = 0;
		_flag113 = 1;
		setFileHash(0x192ADD30, 0, -1);
		SetMessageHandler(&Class521::handleMessage45CC30);
		SetUpdateHandler(&Class521::update);
		SetAnimationCallback3(&Class521::sub45CFE0);
	} else if (!_flag10E && _steps && _flag113) {
		removeCallbacks();
		_flag113 = 0;
		setFileHash(0x9966B138, 0, -1);
		SetMessageHandler(&Class521::handleMessage45CC30);
		SetUpdateHandler(&Class521::update);
		SetAnimationCallback3(&Class521::sub45D100);
	} else {
		bool flag = false;
		uint index = 0;
		if (_rectList && _rectList->size() > 0) {
			while (index < _rectList->size()) {
				NRect r = (*_rectList)[index];
				if (_x >= r.x1 && _x <= r.x2 && _y >= r.y1 && _y <= r.y2)
					break;
			}
			if (index < _rectList->size() && !_flag11A)
				flag = true;
			_flag11A = index < _rectList->size();
		}
		if (flag) {
			removeCallbacks();
			sub45D0A0();
		} else if (_newMoveDirection != _currMoveDirection && _flag114 && !_flag10F) {
			removeCallbacks();
			_currMoveDirection = _newMoveDirection;
			sub45D100();
		}
	}
}

void Class521::sub45CF80() {
	setFileHash(0xA86A9538, 0, -1);
	SetMessageHandler(&Class521::handleMessage45CC30);
	SetUpdateHandler(&Class521::update);
	SetAnimationCallback3(&Class521::sub45CFE0);
}

void Class521::sub45CFB0() {
	setFileHash(0xA86A9538, -1, -1);
	_playBackwards = true;
	SetMessageHandler(&Class521::handleMessage45CCA0);
	SetUpdateHandler(&Class521::update);
}

void Class521::sub45CFE0() {
	setFileHash(0x35698F78, 0, -1);
	SetMessageHandler(&Class521::handleMessage);
	SetUpdateHandler(&Class521::update45C790);
	setCallback1(AnimationCallback(&Class521::sub45D040));
	_idleCounter = 0;
	_currMoveDirection = 0;
	_newMoveDirection = 0;
	_steps = 0;
	_idleCounterMax = _vm->_rnd->getRandomNumber(64 - 1) + 24;
}

void Class521::sub45D040() {
	SetUpdateHandler(&Class521::update);
}

void Class521::sub45D050() {
	setFileHash(0xB579A77C, 0, -1);
	SetMessageHandler(&Class521::handleMessage45CC30);
	SetUpdateHandler(&Class521::update);
	SetAnimationCallback3(&Class521::sub45CFE0);
	_idleCounter = 0;
	_idleCounterMax = _vm->_rnd->getRandomNumber(64 - 1) + 24;
}

void Class521::sub45D0A0() {
	_flag10F = 1;
	removeCallbacks();
	setFileHash(0x9C220DA4, 0, -1);
	SetMessageHandler(&Class521::handleMessage45CC30);
	SetUpdateHandler(&Class521::update);
	setCallback1(AnimationCallback(&Class521::sub45D0E0));
}

void Class521::sub45D0E0() {
	_flag10F = 0;
	_newMoveDirection = 0;
	sub45D100();
}

void Class521::sub45D100() {
	_flag114 = 1;
	if (_currMoveDirection == 1) {
		setFileHash(0xD4AA03A4, 0, -1);
	} else if (_currMoveDirection == 3) {
		setFileHash(0xD00A1364, 0, -1);
	} else if ((_currMoveDirection == 2 && _doDeltaX) || (_currMoveDirection == 4 && !_doDeltaX)) {
		sub45D180();
	} else {
		setFileHash(0xD4220027, 0, -1);
	}
	setGlobalVar(0x21E60190, _doDeltaX ? 1 : 0);
}

void Class521::sub45D180() {
	_flag10F = 1;
	removeCallbacks();
	setFileHash(0xF46A0324, 0, -1);
	_value112 = 0;
	SetMessageHandler(&Class521::handleMessage45CC30);
	SetUpdateHandler(&Class521::update);
	setCallback1(AnimationCallback(&Class521::sub45D620));
	sub45CDC0();
}

void Class521::moveToNextPoint() {
	if (_currPointIndex >= (int)_pathPoints->size() - 1) {
		_moreY = 0;
		sendMessage(this, 0x1019, 0);
		sendMessage(_parentScene, 0x2006, 0);
	} else {
		NPoint nextPt = pathPoint(_currPointIndex + 1);
		NPoint currPt = pathPoint(_currPointIndex);
		if (ABS(nextPt.y - currPt.y) <= ABS(nextPt.x - currPt.x) && nextPt.x >= currPt.x &&
			(_currMoveDirection == 4 || _currMoveDirection == 2)) {
			if (_currMoveDirection == 4) 
				_currMoveDirection = 2;
			else if (_currMoveDirection == 2)
				_currMoveDirection = 4;
			if (_flag113)
				sub45D390();
			else
				sub45D350();
		} else {
			if (_steps == 0) {
				removeCallbacks();
				_flag113 = 0;
				setFileHash(0x9966B138, 0, -1);
				SetMessageHandler(&Class521::handleMessage45CC30);
				SetUpdateHandler(&Class521::update);
				SetAnimationCallback3(&Class521::sub45D100);
			}
			_flag10E = 0;
			SetSpriteCallback(&Class521::suMoveToNextPoint);
			_lastDistance = 640;
		}
	}
}

void Class521::sub45D350() {
	removeCallbacks();
	_flag10F = 1;
	_flag10E = 1;
	setFileHash(0x192ADD30, 0, -1);
	SetMessageHandler(&Class521::handleMessage45CC30);
	SetUpdateHandler(&Class521::update);
	SetAnimationCallback3(&Class521::sub45D390);
}

void Class521::sub45D390() {
	removeCallbacks();
	_flag10F = 1;
	setFileHash(0xF46A0324, 0, -1);
	SetMessageHandler(&Class521::handleMessage45CC30);
	SetUpdateHandler(&Class521::update);
	setCallback1(AnimationCallback(&Class521::sub45D620));
	_value112 = 1;
	sub45CDC0();
}

void Class521::moveToPrevPoint() {
	if (_currPointIndex == 0 && _stepError == 0) {
		_moreY = 0;
		sendMessage(this, 0x1019, 0);
		sendMessage(_parentScene, 0x2005, 0);
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
		if (ABS(prevPt.y - currPt.y) <= ABS(prevPt.x - currPt.x) && currPt.x >= prevPt.x &&
			(_currMoveDirection == 2 || _currMoveDirection == 4)) {
			if (_currMoveDirection == 2) 
				_currMoveDirection = 4;
			else if (_currMoveDirection == 4)
				_currMoveDirection = 2;
			if (_flag113)
				sub45D5D0();
			else
				sub45D580();
		} else {
			if (_steps == 0) {
				removeCallbacks();
				_flag113 = 0;
				setFileHash(0x9966B138, 0, -1);
				SetMessageHandler(&Class521::handleMessage45CC30);
				SetUpdateHandler(&Class521::update);
				SetAnimationCallback3(&Class521::sub45D100);
			}
			_flag10E = 0;
			SetSpriteCallback(&Class521::suMoveToPrevPoint);
			_lastDistance = 640;
		}
	}
}

void Class521::sub45D580() {
	_flag10F = 1;
	_flag10E = 1;
	setCallback1(NULL);
	setFileHash(0x192ADD30, 0, -1);
	SetMessageHandler(&Class521::handleMessage45CC30);
	SetUpdateHandler(&Class521::update);
	SetAnimationCallback3(&Class521::sub45D5D0);
}

void Class521::sub45D5D0() {
	_flag10F = 1;
	setCallback1(NULL);
	setFileHash(0xF46A0324, 0, -1);
	SetMessageHandler(&Class521::handleMessage45CC30);
	SetUpdateHandler(&Class521::update);
	setCallback1(AnimationCallback(&Class521::sub45D620));
	_value112 = 2;
	sub45CDC0();
}

void Class521::sub45D620() {
	_flag10F = 0;
	_newMoveDirection = 0;
	setDoDeltaX(2);
	sub45D100();
}

void Class521::suMoveToNextPoint() {
	int16 newX = _x, newY = _y;

	if (_currPointIndex >= (int)_pathPoints->size()) {
		_moreY = 0;
		sendMessage(this, 0x1019, 0);
		sendMessage(_parentScene, 0x2006, 0);
		return;
	}

	if (_flag10E) {
		if (_steps <= 0) {
			sendMessage(this, 0x1019, 0);
			return;
		} else {
			_steps--;
		}
	} else if (_steps < 11) {
		_steps++;
	}

	bool firstTime = true;
	_anotherY = _steps;
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
				if (pt1.y >= pt2.y) {
					stepsCtr += 7;
				} else {
					stepsCtr -= 4;
					if (stepsCtr < 0)
						stepsCtr = 0;
				}
				_anotherY = stepsCtr;
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

	if (_moreY != 0) {
		_x = newX;
		_y = newY;
		_moreY -= _anotherY;
		if (_moreY <= 0) {
			_flag10E = 1;
			_moreY = 0;
		}
	} else {
		int distance = calcDistance(_someX, _someY, _x, _y);
		_x = newX;
		_y = newY;
		if (newX > 20 && newX < 620 && newY > 20 && newY < 460) {
			_exitDirection = 0;
			_field100 = 1;
		} else if (_field100) {
			_someX = pathPoint(_pathPoints->size() - 1).x;
			_someY = pathPoint(_pathPoints->size() - 1).y;
			_field100 = 0;
			if (_x <= 20)
				_exitDirection = 1;
			else if (_x >= 620)
				_exitDirection = 3;
			else if (_y <= 20)
				_exitDirection = 2;
			else if (_y >= 460)
				_exitDirection = 4;
			if (_exitDirection != 0 && _flag10E) {
				_flag10E = 0;
				_steps = 11;
			}
		}
		if ((distance < 20 && _exitDirection == 0 && _lastDistance < distance) ||
			(_exitDirection == 0 && _lastDistance + 20 < distance))
			_flag10E = 1;
		if (distance < _lastDistance)
			_lastDistance = distance;
		if (_currPointIndex == (int)_pathPoints->size() - 1) {
			_flag10E = 1;
			_moreY = 0;
			sendMessage(this, 0x1019, 0);
			sendMessage(_parentScene, 0x2006, 0);
		}
	}

}

void Class521::suMoveToPrevPoint() {
	int16 newX = _x, newY = _y;

	if (_currPointIndex == 0 && _stepError == 0) {
		_moreY = 0;
		sendMessage(this, 0x1019, 0);
		sendMessage(_parentScene, 0x2005, 0);
		return;
	}

	if (_flag10E) {
		if (_steps <= 0) {
			sendMessage(this, 0x1019, 0);
			return;
		} else {
			_steps--;
		}
	} else if (_steps < 11) {
		_steps++;
	}

	bool firstTime = true;
	_anotherY = _steps;
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
				_anotherY = stepsCtr;
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

	if (_moreY != 0) {
		_x = newX;
		_y = newY;
		_moreY -= _anotherY;
		if (_moreY <= 0) {
			_flag10E = 1;
			_moreY = 0;
		}
	} else {
		int distance = calcDistance(_someX, _someY, _x, _y);
		_x = newX;
		_y = newY;
		if (newX > 20 && newX < 620 && newY > 20 && newY < 460) {
			_exitDirection = 0;
			_field100 = 1;
		} else if (_field100) {
			_someX = pathPoint(0).x;
			_someY = pathPoint(0).y;
			_field100 = 0;
			if (_x <= 20)
				_exitDirection = 1;
			else if (_x >= 620)
				_exitDirection = 3;
			else if (_y <= 20)
				_exitDirection = 2;
			else if (_y >= 460)
				_exitDirection = 4;
			if (_exitDirection != 0 && _flag10E) {
				_flag10E = 0;
				_steps = 11;
			}
		}
		if ((distance < 20 && _exitDirection == 0 && _lastDistance < distance) ||
			(_exitDirection == 0 && _lastDistance + 20 < distance))
			_flag10E = 1;
		if (distance < _lastDistance)
			_lastDistance = distance;
		if (_currPointIndex == 0 && _stepError == 0) {
			_flag10E = 1;
			_moreY = 0;
			sendMessage(this, 0x1019, 0);
			sendMessage(_parentScene, 0x2005, 0);
		}
	}

}

void Class521::sub45E0A0() {
	// TODO
}

int Class521::calcDistance(int16 x1, int16 y1, int16 x2, int16 y2) {
	int16 deltaX = ABS(x1 - x2);
	int16 deltaY = ABS(y1 - y2);
	return sqrt(deltaX * deltaX + deltaY * deltaY);
}

Class546::Class546(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 0x08C80144, 900, 320, 240), _soundResource(vm),
	_parentScene(parentScene) {

	setVisible(false);
	SetMessageHandler(&Class546::handleMessage);
	setFileHash1();
}

uint32 Class546::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
		break;
	case 0x4808:
		sub44D710();
		break; 
	case 0x4809:
		sub44D790();
		break; 
	}
	return messageResult;
}

void Class546::sub44D710() {
	setFileHash(0x08C80144, 0, -1);
	setVisible(true);
	SetAnimationCallback3(&Class546::sub44D760);
	_soundResource.play(calcHash("fxDoorOpen23"));
}

void Class546::sub44D760() {
	sendMessage(_parentScene, 0x2033, 0);
	setFileHash1();
	setVisible(false);
}

void Class546::sub44D790() {
	setFileHash(0x08C80144, -1, -1);
	setVisible(true);
	SetAnimationCallback3(&Class546::sub44D7F0);
	_soundResource.play(calcHash("fxDoorClose23"));
}

void Class546::sub44D7F0() {
	sendMessage(_parentScene, 0x2034, 0);
	setFileHash1();
}

Class547::Class547(NeverhoodEngine *vm, int16 x, int16 y)
	: AnimatedSprite(vm, 0x1209E09F, 1100, x, y) {

	setDoDeltaX(1);
	setFileHash(0x1209E09F, 1, -1);
	_newHashListIndex = 1;
}

Class548::Class548(NeverhoodEngine *vm, int16 x, int16 y)
	: AnimatedSprite(vm, 0x1209E09F, 100, x, y) {

	setDoDeltaX(1);
	_newHashListIndex = 0;
}

Class518::Class518(NeverhoodEngine *vm, Class521 *class521)
	: AnimatedSprite(vm, 1100), _class521(class521) {
	
	SetUpdateHandler(&Class518::update);
	createSurface1(0x60281C10, 150);
	setFileHash(0x60281C10, -1, -1);
	_newHashListIndex = -2;
}

void Class518::update() {
	_x = _class521->getX();
	_y = _class521->getY();
	AnimatedSprite::update();
}

Scene1608::Scene1608(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _class521(NULL), _countdown1(0) {
	
	setGlobalVar(0x21E60190, 1);
	
	_surfaceFlag = true;
	SetMessageHandler(&Scene1608::handleMessage44D2A0);
	
	_class545 = insertSprite<Class545>(this, 1, 1100, 198, 220);
	_vm->_collisionMan->addSprite(_class545);

	if (which < 0) {
		if (_vm->gameState().which == 1)
			which = 1;
		else {
			setRectList(0x004B47D0);
			insertKlayman<KmScene1608>(380, 438);
			_klayman2 = _klayman;
			_flag4 = false;
			_class546 = insertSprite<Class546>(this);
			_sprite1 = insertStaticSprite(0x7D0404E8, 1100);
			setMessageList(0x004B46A8);
			setBackground(0x10080E01);
			setPalette(0x10080E01);
			_asTape = insertSprite<AsScene1201Tape>(this, 13, 1100, 412, 443, 0x9148A011);
			_vm->_collisionMan->addSprite(_asTape);
			_klayman->setClipRect(_sprite1->getDrawRect().x, 0, 640, 480);
			SetUpdateHandler(&Scene1608::update44CE90);
			insertMouse433(0x80E05108);
			insertStaticSprite(0x4B18F868, 1200);
		}
	} else if (which == 0) {
		_vm->gameState().which = 0;
		setRectList(0x004B47D0);
		insertKlayman<KmScene1608>(0, 438);
		_klayman2 = _klayman;
		_flag4 = false;
		setMessageList(0x004B46B0);
		setBackground(0x10080E01);
		setPalette(0x10080E01);
		_asTape = insertSprite<AsScene1201Tape>(this, 13, 1100, 412, 443, 0x9148A011);
		_vm->_collisionMan->addSprite(_asTape);
		insertMouse433(0x80E05108);
		_sprite1 = insertStaticSprite(0x7D0404E8, 1100);
		_class546 = insertSprite<Class546>(this);
		_klayman->setClipRect(_sprite1->getDrawRect().x, 0, 640, 480);
		SetUpdateHandler(&Scene1608::update44CE90);
		sendMessage(_class546, 0x4808, 0);
		insertStaticSprite(0x4B18F868, 1200);
	} else if (which == 2) {
		_vm->gameState().which = 1;
		_dataResource.load(0x003C0492);
		_roomPathPoints = _dataResource.getPointArray(calcHash("meArchroArchRoomPath"));
		setBackground(0x98001604);
		setPalette(0x98001604);
		_palette->addPalette("paPodRed", 65, 31, 65);
		insertMouse433(0x01600988);
		_sprite2 = insertStaticSprite(0x491F38A8, 1100);
		_class521 = createSprite<Class521>(this, 375, 227); // Create but don't add to the sprite list yet
		_class547 = insertSprite<Class547>(375, 227);
		_class548 = insertSprite<Class548>(375, 227);
		_class521->setVisible(false);
		if (getGlobalVar(0xC0418A02)) {
			insertKlayman<KmScene1608>(373, 220);
			_klayman->setDoDeltaX(1);
		} else {
			insertKlayman<KmScene1608>(283, 220);
		}
		_klayman2 = _klayman;
		setMessageList(0x004B47A8);
		SetMessageHandler(&Scene1608::handleMessage44D3C0);
		SetUpdateHandler(&Scene1608::update44CED0);
		// NOTE: Setting the point array was handled by messages 0x2000 (array) and 0x2001 (count) in the original
		_class521->setPathPoints(_roomPathPoints);
		sendMessage(_class521, 0x2002, _roomPathPoints->size() - 1);
		_sprite3 = insertStaticSprite(0xB47026B0, 1100);
		_rect1.set(_sprite3->getDrawRect().x, _sprite3->getDrawRect().y, 640, _sprite2->getDrawRect().y2());
		_rect3.set(_sprite2->getDrawRect().x, _sprite3->getDrawRect().y, 640, _sprite2->getDrawRect().y2());
		_rect2 = _rect1;
		_rect2.y2 = 215;
		_klayman->setClipRect(_rect1);
		_class521->setClipRect(_rect1);
		_class547->setClipRect(_rect1);
		_class548->setClipRect(_rect1);
		_asTape = insertSprite<AsScene1201Tape>(this, 13, 1100, 412, 443, 0x9148A011);
		_vm->_collisionMan->addSprite(_asTape);
		insertSprite<Class518>(_class521)->setClipRect(_rect1);
		_flag4 = false;
		_flag2 = false;
		_flag1 = 0;
		setRectList(0x004B4810);
	} 

	// NOTE: Not in the else because 'which' is set to 1 in the true branch	
	if (which == 1) {
		_vm->gameState().which = 1;
		_dataResource.load(0x003C0492);
		_roomPathPoints = _dataResource.getPointArray(calcHash("meArchroArchRoomPath"));
		setBackground(0x98001604);
		setPalette(0x98001604);
		_palette->addPalette("paPodRed", 65, 31, 65);
		insertMouse433(0x01600988);
		_class521 = insertSprite<Class521>(this, 375, 227);
		_class547 = insertSprite<Class547>(375, 227);
		_class548 = insertSprite<Class548>(375, 227);
		_sprite2 = insertStaticSprite(0x491F38A8, 1100);
		_klayman2 = createSprite<KmScene1608>(this, 439, 220); // Special Klayman handling...
		sendMessage(_klayman2, 0x2032, 1);
		_klayman2->setDoDeltaX(1);
		SetMessageHandler(&Scene1608::handleMessage44D470);
		SetUpdateHandler(&Scene1608::update44D1E0);
		_class547->setVisible(false);
		_class548->setVisible(false);
		// NOTE: Setting the point array was handled by messages 0x2000 (array) and 0x2001 (count) in the original
		_class521->setPathPoints(_roomPathPoints);
		sendMessage(_class521, 0x2002, 0);
		sendMessage(_class521, 0x2008, 90);
		_sprite3 = insertStaticSprite(0xB47026B0, 1100);
		_rect1.set(_sprite3->getDrawRect().x, _sprite3->getDrawRect().y, 640, _sprite2->getDrawRect().y2());
		_rect3.set(_sprite2->getDrawRect().x, _sprite3->getDrawRect().y, 640, _sprite2->getDrawRect().y2());
		_rect2 = _rect1;
		_rect2.y2 = 215;
		_klayman2->setClipRect(_rect1);
		_class521->setClipRect(_rect1);
		_class547->setClipRect(_rect1);
		_class548->setClipRect(_rect1);
		_asTape = insertSprite<AsScene1201Tape>(this, 13, 1100, 412, 443, 0x9148A011);
		// ... _vm->_collisionMan->addSprite(_asTape);
		insertSprite<Class518>(_class521)->setClipRect(_rect1);
		_flag4 = true;
		_flag2 = true;
		_flag1 = 0;
	}
	
	_palette->addPalette("paKlayRed", 0, 64, 0);

}

Scene1608::~Scene1608() {
	setGlobalVar(0xC0418A02, _klayman2->isDoDeltaX() ? 1 : 0);
	// Weird
	if (_flag4) {
		delete _klayman2;
	} else {
		delete _class521;
	}
}

void Scene1608::update44CE90() {
	Scene::update();
	if (_countdown1 != 0 && (--_countdown1 == 0)) {
		leaveScene(0);
	}
}

void Scene1608::update44CED0() {
	Scene::update();
	if (_flag1 == 1) {
		removeSurface(_klayman->getSurface());
		removeEntity(_klayman);
		addSprite(_class521);
		_flag4 = true;
		clearRectList();
		SetUpdateHandler(&Scene1608::update44CFE0);
		SetMessageHandler(&Scene1608::handleMessage44D510);
		_class547->setVisible(false);
		_class548->setVisible(false);
		_class521->setVisible(true);
		sendMessage(_class521, 0x2009, 0);
		_class521->handleUpdate();
		_klayman = NULL;
		_flag1 = 0;
	}
	if (_klayman2->getX() <= 375) {
		_klayman2->setClipRect(_rect1);
	} else {
		_klayman2->setClipRect(_rect2);
	}
}

void Scene1608::update44CFE0() {
	Scene::update();
	if (_mouseClicked) {
		if (_mouseClickPos.x <= 329 && _class521->getX() == 375 && _class521->getY() == 227) {
			sendMessage(_class521, 0x200A, 0);
			SetUpdateHandler(&Scene1608::update44D0C0);
		} else {
			sendPointMessage(_class521, 0x2004, _mouseClickPos);
			SetMessageHandler(&Scene1608::handleMessage44D470);
			SetUpdateHandler(&Scene1608::update44D1E0);
		}
		_mouseClicked = false;
	}
	if (_klayman2->getX() <= 375) {
		_klayman2->setClipRect(_rect1);
	} else {
		_klayman2->setClipRect(_rect2);
	}
}

void Scene1608::update44D0C0() {
	Scene::update();
	if (_flag1 == 2) {
		_klayman = _klayman2;
		removeSurface(_class521->getSurface());
		removeEntity(_class521);
		addSprite(_klayman);
		_flag4 = false;
		SetMessageHandler(&Scene1608::handleMessage44D3C0);
		SetUpdateHandler(&Scene1608::update44CED0);
		setRectList(0x004B4810);
		_class547->setVisible(true);
		_class548->setVisible(true);
		_class521->setVisible(false);
		setMessageList(0x004B4748);
		runMessageList();
		_klayman->handleUpdate();
		_flag1 = 0;
	}
	if (_klayman2->getX() <= 375) {
		_klayman2->setClipRect(_rect1);
	} else {
		_klayman2->setClipRect(_rect2);
	}
}

void Scene1608::update44D1E0() {
	Scene::update();
	if (_mouseClicked) {
		sendPointMessage(_class521, 0x2004, _mouseClickPos);
		_mouseClicked = false;
	}
	if (_class521->getX() < 300) {
		if (_flag2) {
			_flag2 = false;
			_class521->setClipRect(_rect1);
			if (!_class521->isDoDeltaX())
				sendMessage(_class521, 0x200E, 0);
		}
	} else if (!_flag2) {
		_flag2 = true;
		_class521->setClipRect(_rect3);
	}
}

uint32 Scene1608::handleMessage44D2A0(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x20250B1A) {
			clearRectList();
			_klayman->setVisible(false);
			showMouse(false);
			_sprite1->setVisible(false);
			sendMessage(_class546, 0x4809, 0);
			_countdown1 = 28;
		}
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	case 0x4826:
		if (sender == _asTape) {
			sendEntityMessage(_klayman2, 0x1014, _asTape);
			setMessageList(0x004B4770);
		} else if (sender == _class545) {
			setMessageList(0x004B46C8);
		}
		break;
	}
	return 0;
}

uint32 Scene1608::handleMessage44D3C0(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x60842040) {
			_flag1 = true;
		}
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	case 0x4826:
		if (sender == _class545) {
			sendEntityMessage(_klayman2, 0x1014, _class545);
			setMessageList(0x004B4760);
		}
		break;
	}
	return 0;
}

uint32 Scene1608::handleMessage44D470(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2005:
		leaveScene(1);
		break;
	case 0x2006:
		SetMessageHandler(&Scene1608::handleMessage44D510);
		SetUpdateHandler(&Scene1608::update44CFE0);
		sendMessage(_class521, 0x200F, 1);
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	}
	return 0;
}

uint32 Scene1608::handleMessage44D510(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x200A:
		_flag1 = 2;
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	}
	return 0;
}
	
Scene1609::Scene1609(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource(vm), _countdown1(1),
	_index1(0), _index3(0), _flag5(true), _flag6(false) {

	// TODO _vm->gameModule()->initScene3011Vars();
	_index2 = getGlobalVar(0x2414C2F2);
	
	_surfaceFlag = true;
	SetMessageHandler(&Scene1609::handleMessage);
	SetUpdateHandler(&Scene1609::update);
	
	setBackground(0x92124A14);
	setPalette(0x92124A14);
	
	for (int i = 0; i < 12; i++)
		_asSymbols[i] = insertSprite<AsScene3011Symbol>(i, false);
	
	_ssButton = insertSprite<SsScene3011Button>(this, true);
	_vm->_collisionMan->addSprite(_ssButton);

	insertMouse435(0x24A10929, 20, 620);

	_soundResource.load(0x68E25540);

}

void Scene1609::update() {
	if (!_flag6 && _countdown1 != 0 && (--_countdown1 == 0)) {
		if (_flag5) {
			_index1++;
			if (_index1 >= 12)
				_index1 = 0;
			_asSymbols[_index3]->change(_index1 + 12, _index1 == (int)getSubVar(0x04909A50, _index2));
			_flag5 = false;
			_countdown1 = 36;
		} else {
			_asSymbols[_index3]->hide();
			_flag5 = true;
			_countdown1 = 12;
		}
	}
	if (_flag6 && !_soundResource.isPlaying()) {
		leaveScene(1);
	}
	Scene::update();
}

uint32 Scene1609::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO: Debug stuff
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620)
			leaveScene(0);
		break;
	// TODO: Debug stuff
	case 0x2000:
		if (!_flag6) {
			if (_flag5)
				_asSymbols[_index3]->change(_index1 + 12, false);
			_asSymbols[_index3]->stopSound();
			_index3++;
			if (_index3 >= 12) {
				if (testVars()) {
					_soundResource.play();
					setGlobalVar(0x2C531AF8, 1);
					_flag6 = true;
				} else {
					_index3 = 0;
					for (int i = 0; i < 12; i++)
						_asSymbols[i]->hide();
				}
			}
			_flag5 = true;
			_countdown1 = 1;
		}
		break;
	}
	return 0;
}

bool Scene1609::testVars() {
	int index1 = 0;
	do {
		int cmpIndex = _asSymbols[0]->getIndex();
		if (!_asSymbols[0]->getFlag1())
			cmpIndex -= 12;
		if ((int)getSubVar(0x04909A50, index1) == cmpIndex)
			break;
		index1++;
	} while(1);
	for (int index2 = 0; index2 < 12; index2++) {
		int cmpIndex = _asSymbols[index2]->getIndex();
		if (!_asSymbols[index2]->getFlag1())
			cmpIndex -= 12;
		if ((int)getSubVar(0x04909A50, index1) != cmpIndex)
			return false;
		_index1++;
		if (_index1 >= 12)
			_index1 = 0;
		_index2++;
	}
	return true;
}
	
} // End of namespace Neverhood
