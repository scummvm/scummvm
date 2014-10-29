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

#include "illusions/illusions.h"
#include "illusions/camera.h"
#include "illusions/backgroundresource.h"
#include "illusions/fixedpoint.h"
#include "illusions/time.h"

namespace Illusions {

Camera::Camera(IllusionsEngine *vm)
	: _vm(vm) {
	_activeState._cameraMode = 6;
	_activeState._paused = false;
	_activeState._panStartTime = getCurrentTime();
	_activeState._panSpeed = 1;
	_activeState._bounds._topLeft.x = 320;
	_activeState._bounds._topLeft.y = 240;
	_activeState._bounds._bottomRight.x = 320;
	_activeState._bounds._bottomRight.y = 240;
	_activeState._currPan.x = 320;
	_activeState._currPan.y = 240;
	_activeState._panXShl = 320 << 16;
	_activeState._panYShl = 240 << 16;
	_activeState._panTargetPoint.x = 320;
	_activeState._panTargetPoint.y = 240;
	_activeState._panToPositionPtr = 0;
	_activeState._panNotifyId = 0;
	_activeState._trackingLimits.x = 0;
	_activeState._trackingLimits.y = 0;
	_activeState._centerPt.x = 320;
	_activeState._centerPt.y = 240;
	_activeState._pointFlags = 0;
}

void Camera::clearStack() {
	_stack.clear();
}

void Camera::set(Common::Point &panPoint, WidthHeight &dimensions) {
	_activeState._cameraMode = 6;
	_activeState._paused = false;
	_activeState._panStartTime = getCurrentTime();
	_activeState._panSpeed = 1;
	_activeState._bounds._topLeft.x = 320;
	_activeState._bounds._topLeft.y = 240;
	_activeState._bounds._bottomRight.x = MAX(0, dimensions._width - 640) + 320;
	_activeState._bounds._bottomRight.y = MAX(0, dimensions._height - 480) + 240;
	_activeState._panTargetPoint = panPoint;
	clipPanTargetPoint();
	_activeState._currPan = _activeState._panTargetPoint;
	_activeState._panXShl = _activeState._currPan.x << 16;
	_activeState._panYShl = _activeState._currPan.y << 16;
	_vm->_backgroundItems->refreshPan();
	_activeState._panToPositionPtr = 0;
	_activeState._panObjectId = 0;
	_activeState._panNotifyId = 0;
	_activeState._trackingLimits.x = 0;
	_activeState._trackingLimits.y = 0;
	_activeState._pointFlags = 0;
	_activeState._centerPt.x = 320;
	_activeState._centerPt.y = 240;
}

void Camera::panCenterObject(uint32 objectId, int16 panSpeed) {
	Common::Point *actorPosition = _vm->getObjectActorPositionPtr(objectId);
	_activeState._cameraMode = 1;
	_activeState._panSpeed = panSpeed;
	_activeState._trackingLimits.x = 8;
	_activeState._trackingLimits.y = 8;
	_activeState._pointFlags = 0;
	_activeState._panToPositionPtr = actorPosition;
	_activeState._panObjectId = objectId;
	_activeState._panTargetPoint = *actorPosition;
	_activeState._panNotifyId = 0;
	clipPanTargetPoint();
	_activeState._panStartTime = getCurrentTime();
	recalcPan(_activeState._panStartTime);
}

void Camera::panTrackObject(uint32 objectId) {
	Common::Point *actorPosition = _vm->getObjectActorPositionPtr(objectId);
	_activeState._cameraMode = 3;
	_activeState._panObjectId = objectId;
	_activeState._trackingLimits.x = 160;
	_activeState._trackingLimits.y = 120;
	_activeState._pointFlags = 0;
	_activeState._panSpeed = 710;
	_activeState._panToPositionPtr = actorPosition;
	_activeState._panNotifyId = 0;
	_activeState._panTargetPoint = *actorPosition;
	clipPanTargetPoint();
	_activeState._panStartTime = getCurrentTime();
	recalcPan(_activeState._panStartTime);
}

void Camera::panToPoint(Common::Point pt, int16 panSpeed, uint32 panNotifyId) {

	_vm->notifyThreadId(_activeState._panNotifyId);

	_activeState._panTargetPoint = getPtOffset(pt);
	clipPanTargetPoint();
	
	if (panSpeed) {
		_activeState._cameraMode = 5;
		_activeState._panSpeed = panSpeed;
		_activeState._trackingLimits.x = 0;
		_activeState._trackingLimits.y = 0;
		_activeState._pointFlags = 0;
		_activeState._panToPositionPtr = 0;
		_activeState._panNotifyId = panNotifyId;
		_activeState._panStartTime = getCurrentTime();
		recalcPan(_activeState._panStartTime);
	} else {
		_activeState._currPan = _activeState._panTargetPoint;
		stopPan();
		_vm->notifyThreadId(panNotifyId);
	}
}

void Camera::panEdgeFollow(uint32 objectId, int16 panSpeed) {
	Common::Point *actorPosition = _vm->getObjectActorPositionPtr(objectId);
	_activeState._cameraMode = 2;
	_activeState._trackingLimits.x = 318;
	_activeState._trackingLimits.y = 238;
	_activeState._pointFlags = 0;
	_activeState._panSpeed = panSpeed;
	_activeState._panToPositionPtr = actorPosition;
	_activeState._panObjectId = objectId;
	_activeState._panTargetPoint = _activeState._currPan;
	_activeState._panNotifyId = 0;
	clipPanTargetPoint();
	_activeState._panStartTime = getCurrentTime();
	recalcPan(_activeState._panStartTime);
}

void Camera::stopPan() {
	_activeState._cameraMode = 6;
	_activeState._panTargetPoint = _activeState._currPan;
	_activeState._panSpeed = 1;
	_activeState._panXShl = _activeState._currPan.x << 16;
	_activeState._panYShl = _activeState._currPan.y << 16;
	_activeState._panToPositionPtr = 0;
	_activeState._panObjectId = 0;
	_activeState._panNotifyId = 0;
	_activeState._pointFlags = 0;
	_vm->_backgroundItems->refreshPan();
}

void Camera::pause() {
	_activeState._pauseStartTime = getCurrentTime();
	_activeState._paused = true;
}

void Camera::unpause() {
	_activeState._paused = false;
	uint32 pauseDuration = getCurrentTime() - _activeState._pauseStartTime;
	_activeState._time28 += pauseDuration;
	_activeState._panStartTime += pauseDuration;
}

void Camera::pushCameraMode() {
	CameraModeStackItem item;
	item._cameraMode = _activeState._cameraMode;
	item._panSpeed = _activeState._panSpeed;
	item._panObjectId = 0;
	item._panNotifyId = 0;
	item._panTargetPoint.x = 0;
	item._panTargetPoint.y = 0;
	switch (_activeState._cameraMode) {
	case 1:
	case 2:
	case 3:
		item._panObjectId = _activeState._panObjectId;
		break;
	case 4:
		item._cameraMode = 3;
		item._panObjectId = _activeState._panObjectId;
		break;
	case 5:
		item._panTargetPoint = _activeState._panTargetPoint;
		item._panNotifyId = _activeState._panNotifyId;
		break;
	}
	_stack.push(item);
}

void Camera::popCameraMode() {
	CameraModeStackItem item = _stack.pop();

	if (item._panObjectId && !_vm->getObjectActorPositionPtr(item._panObjectId)) {
		// Tracking object doesn't exist any longer
		stopPan();
		return;
	}

	switch (item._cameraMode) {
	case 1:
		panCenterObject(item._panObjectId, item._panSpeed);
		break;
	case 2:
		panEdgeFollow(item._panObjectId, item._panSpeed);
		break;
	case 3:
		panTrackObject(item._panObjectId);
		break;
	case 5:
		panToPoint(item._panTargetPoint, item._panSpeed, item._panNotifyId);
		break;
	case 6:
		stopPan();
		break;
	}

}

void Camera::clearCameraModeStack() {
	_stack.clear();
}

void Camera::update(uint32 currTime) {

	if (_activeState._paused)
		return;

	//debug("_activeState._cameraMode = %d", _activeState._cameraMode);

	switch (_activeState._cameraMode) {
	case 1:
		updateMode1(currTime);
		break;
	case 2:
		updateMode2(currTime);
		break;
	case 3:
		updateMode3(currTime);
		break;
	}
	
	if (_activeState._cameraMode != 6) {

		if (!isPanFinished() &&	updatePan(currTime)) {
			/* Unused
			if (_activeState._cameraMode == 1 || _activeState._cameraMode == 5)
				nullsub_2();
			*/
			_vm->_backgroundItems->refreshPan();
		}

		if (isPanFinished()) {
			if (_activeState._cameraMode == 5) {
				// Notify a thread that the camera panning has finished
				_vm->notifyThreadId(_activeState._panNotifyId);
				_activeState._cameraMode = 6;
			} else if (_activeState._cameraMode == 4) {
				_activeState._cameraMode = 3;
			}
		}

	}

}

void Camera::setBounds(Common::Point &minPt, Common::Point &maxPt) {
	_activeState._bounds._topLeft = minPt;
	_activeState._bounds._bottomRight = maxPt;
}

void Camera::setBoundsToDimensions(WidthHeight &dimensions) {
	// NOTE For the caller dimensions = artdispGetMasterBGDimensions();
	_activeState._bounds._topLeft.x = 320;
	_activeState._bounds._topLeft.y = 240;
	_activeState._bounds._bottomRight.x = MAX(0, dimensions._width - 640) + 320;
	_activeState._bounds._bottomRight.y = MAX(0, dimensions._height - 480) + 240;
	clipPanTargetPoint();
}

Common::Point Camera::getCurrentPan() {
	return _activeState._currPan;
}

Common::Point Camera::getScreenOffset() {
	Common::Point screenOffs = getCurrentPan();
	screenOffs.x -= 320;
	screenOffs.y -= 240;
	return screenOffs;
}

void Camera::updateMode1(uint32 currTime) {
	Common::Point ptOffs = getPtOffset(*_activeState._panToPositionPtr);
	int deltaX = ptOffs.x - _activeState._currPan.x + 320 - _activeState._centerPt.x;
	int deltaY = ptOffs.y - _activeState._currPan.y + 240 - _activeState._centerPt.y;
	int deltaXAbs = ABS(deltaX);
	int deltaYAbs = ABS(deltaY);

	if (deltaXAbs > _activeState._trackingLimits.x)
		_activeState._panTargetPoint.x = _activeState._currPan.x + ABS(deltaXAbs - _activeState._trackingLimits.x) * (deltaX >= 0 ? 1 : -1);
	else
		_activeState._panTargetPoint.x = _activeState._currPan.x;

	if (deltaYAbs > _activeState._trackingLimits.y)
		_activeState._panTargetPoint.y = _activeState._currPan.y + ABS(deltaYAbs - _activeState._trackingLimits.y) * (deltaY >= 0 ? 1 : -1);
	else
		_activeState._panTargetPoint.y = _activeState._currPan.y;

	clipPanTargetPoint();

	if (!isPanFinished()) {
		uint32 oldPanTime = _activeState._panStartTime;
		_activeState._panStartTime = _activeState._time28;
		recalcPan(oldPanTime);
	}
	
}

void Camera::updateMode2(uint32 currTime) {
	Common::Point panToPosition = *_activeState._panToPositionPtr;
	uint pointFlags = 0;
	WRect rect;

	rect._topLeft.x = 320 - _activeState._trackingLimits.x;
	rect._topLeft.y = 240 - _activeState._trackingLimits.y;
	rect._bottomRight.x = 320 + _activeState._trackingLimits.x;
	rect._bottomRight.y = 240 + _activeState._trackingLimits.y;

	if (calcPointFlags(panToPosition, rect, pointFlags)) {
		if (pointFlags != _activeState._pointFlags) {
			_activeState._pointFlags = pointFlags;
			if (pointFlags & 1)
				_activeState._panTargetPoint.x = _activeState._bounds._topLeft.x;
			else if (pointFlags & 2)
				_activeState._panTargetPoint.x = _activeState._bounds._bottomRight.x;
			else
				_activeState._panTargetPoint.x = _activeState._currPan.x;
			if (pointFlags & 4)
				_activeState._panTargetPoint.y = _activeState._bounds._topLeft.y;
			else if (pointFlags & 8)
				_activeState._panTargetPoint.y = _activeState._bounds._bottomRight.y;
			else
				_activeState._panTargetPoint.y = _activeState._currPan.y;
			clipPanTargetPoint();
			_activeState._panStartTime = currTime;
			recalcPan(currTime);
		}
	} else if (_activeState._pointFlags) {
		_activeState._pointFlags = 0;
		_activeState._panTargetPoint = _activeState._currPan;
  	}

}

void Camera::updateMode3(uint32 currTime) {
	Common::Point panToPosition = *_activeState._panToPositionPtr;
	int deltaX = panToPosition.x - _activeState._currPan.x;
	int deltaY = panToPosition.y - _activeState._currPan.y;

	if (ABS(deltaX) > _activeState._trackingLimits.x)
		_activeState._panTargetPoint.x = _activeState._currPan.x + 2 * _activeState._trackingLimits.x * (deltaX >= 0 ? 1 : -1);
	else
		_activeState._panTargetPoint.x = _activeState._currPan.x;

	if (ABS(deltaY) > _activeState._trackingLimits.y)
		_activeState._panTargetPoint.y = _activeState._currPan.y + 2 * _activeState._trackingLimits.y * (deltaY >= 0 ? 1 : -1);
	else
		_activeState._panTargetPoint.y = _activeState._currPan.y;

	clipPanTargetPoint();

	if (!isPanFinished()) {
		_activeState._panStartTime = currTime;
		recalcPan(currTime);
		_activeState._cameraMode = 4;
	}
	
}

bool Camera::updatePan(uint32 currTime) {
	if (currTime - _activeState._time28 >= _activeState._time2E) {
		_activeState._panXShl = _activeState._panTargetPoint.x << 16;
		_activeState._panYShl = _activeState._panTargetPoint.y << 16;
	} else {
		_activeState._panXShl += fixedMul(_activeState._someX, (currTime - _activeState._panStartTime) << 16);
		_activeState._panYShl += fixedMul(_activeState._someY, (currTime - _activeState._panStartTime) << 16);
	}
	_activeState._panStartTime = currTime;
	Common::Point newPan(_activeState._panXShl >> 16, _activeState._panYShl >> 16);
	if (_activeState._currPan.x != newPan.x || _activeState._currPan.y != newPan.y) {
		_activeState._currPan = newPan;
		return true;
	}
	return false;
}

bool Camera::isPanFinished() {
	return _activeState._currPan.x == _activeState._panTargetPoint.x && _activeState._currPan.y == _activeState._panTargetPoint.y;
}

Common::Point Camera::getPtOffset(Common::Point pt) {
	pt.x = pt.x - _activeState._centerPt.x + 320;
	pt.y = pt.y - _activeState._centerPt.y + 240;
	return pt;
}

void Camera::recalcPan(uint32 currTime) {
	_activeState._currPan2 = getCurrentPan();
	_activeState._time28 = currTime;

	if (_activeState._panSpeed == 0) {
		_activeState._time2E = 0;
	} else {
		FP16 x1 = _activeState._currPan2.x << 16;
		FP16 y1 = _activeState._currPan2.y << 16;
		FP16 x2 = _activeState._panTargetPoint.x << 16;
		FP16 y2 = _activeState._panTargetPoint.y << 16;
		FP16 distance = fixedDistance(x1, y1, x2, y2);
		_activeState._time2E = 60 * fixedTrunc(distance) / _activeState._panSpeed;
	}

	if (_activeState._time2E != 0) {
		_activeState._someX = fixedDiv((_activeState._panTargetPoint.x - _activeState._currPan2.x) << 16, _activeState._time2E << 16);
		_activeState._someY = fixedDiv((_activeState._panTargetPoint.y - _activeState._currPan2.y) << 16, _activeState._time2E << 16);
	} else {
		_activeState._someX = (_activeState._panTargetPoint.x - _activeState._currPan2.x) << 16;
		_activeState._someY = (_activeState._panTargetPoint.y - _activeState._currPan2.y) << 16;
	}

}

bool Camera::calcPointFlags(Common::Point &pt, WRect &rect, uint &outFlags) {
	bool result = false;
	if (pt.x < rect._topLeft.x) {
		outFlags |= 1;
		result = true;
	} else if (pt.x > rect._bottomRight.x) {
		outFlags |= 2;
		result = true;
	}
	if (pt.y < rect._topLeft.y) {
		outFlags |= 4;
		result = true;
	} else if (pt.y > rect._bottomRight.y) {
		outFlags |= 8;
		result = true;
	}
	return result;
}

void Camera::clipPanTargetPoint() {
	_activeState._panTargetPoint.x = CLIP(_activeState._panTargetPoint.x,
		_activeState._bounds._topLeft.x, _activeState._bounds._bottomRight.x);
	_activeState._panTargetPoint.y = CLIP(_activeState._panTargetPoint.y,
		_activeState._bounds._topLeft.y, _activeState._bounds._bottomRight.y);
}

} // End of namespace Illusions
