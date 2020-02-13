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
#include "illusions/fixedpoint.h"
#include "illusions/resources/backgroundresource.h"
#include "illusions/time.h"
#include "illusions/actor.h"

namespace Illusions {

Camera::Camera(IllusionsEngine *vm)
	: _vm(vm) {
	init();
	_activeState._cameraMode = 6;
	_activeState._paused = false;
	_activeState._panStartTime = getCurrentTime();
	_activeState._panSpeed = 1;
	_activeState._bounds._topLeft.x = _screenMidX;
	_activeState._bounds._topLeft.y = _screenMidY;
	_activeState._bounds._bottomRight.x = _screenMidX;
	_activeState._bounds._bottomRight.y = _screenMidY;
	_activeState._currPan.x = _screenMidX;
	_activeState._currPan.y = _screenMidY;
	_activeState._panXShl = _screenMidX << 16;
	_activeState._panYShl = _screenMidY << 16;
	_activeState._panTargetPoint.x = _screenMidX;
	_activeState._panTargetPoint.y = 240;
	_activeState._panToPositionPtr = 0;
	_activeState._panNotifyId = 0;
	_activeState._trackingLimits.x = 0;
	_activeState._trackingLimits.y = 0;
	_activeState._centerPt.x = _screenMidX;
	_activeState._centerPt.y = _screenMidY;
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
	_activeState._bounds._topLeft.x = _screenMidX;
	_activeState._bounds._topLeft.y = _screenMidY;
	_activeState._bounds._bottomRight.x = MAX(0, dimensions._width - _screenWidth) + _screenMidX;
	_activeState._bounds._bottomRight.y = MAX(0, dimensions._height - _screenHeight) + _screenMidY;
	_activeState._panTargetPoint = panPoint;
	clipPanTargetPoint();
	_activeState._currPan = _activeState._panTargetPoint;
	_activeState._panXShl = _activeState._currPan.x << 16;
	_activeState._panYShl = _activeState._currPan.y << 16;
	_vm->_backgroundInstances->refreshPan();
	_activeState._panToPositionPtr = 0;
	_activeState._panObjectId = 0;
	_activeState._panNotifyId = 0;
	_activeState._trackingLimits.x = 0;
	_activeState._trackingLimits.y = 0;
	_activeState._pointFlags = 0;
	_activeState._centerPt.x = _screenMidX;
	_activeState._centerPt.y = _screenMidY;
}

void Camera::panCenterObject(uint32 objectId, int16 panSpeed) {
	Common::Point *actorPosition = _vm->getObjectActorPositionPtr(objectId);
	if (_vm->getGameId() == kGameIdDuckman) {
		if (objectId == Illusions::CURSOR_OBJECT_ID) {
			_activeState._cameraMode = 2;
			_activeState._trackingLimits.x = 156;
			_activeState._trackingLimits.y = 96;
		} else {
			_activeState._cameraMode = 1;
			_activeState._trackingLimits.x = 4;
			_activeState._trackingLimits.y = 4;
		}
	} else if (_vm->getGameId() == kGameIdBBDOU) {
		_activeState._cameraMode = 1;
		_activeState._trackingLimits = _centerObjectTrackingLimits;
	}
	_activeState._panSpeed = panSpeed;
	_activeState._pointFlags = 0;
	_activeState._panObjectId = objectId;
	_activeState._panNotifyId = 0;
	_activeState._panToPositionPtr = actorPosition;
	_activeState._panTargetPoint = *actorPosition;
	clipPanTargetPoint();
	_activeState._panStartTime = getCurrentTime();
	recalcPan(_activeState._panStartTime);
}

void Camera::panTrackObject(uint32 objectId) {
	Common::Point *actorPosition = _vm->getObjectActorPositionPtr(objectId);
	_activeState._cameraMode = 3;
	_activeState._panObjectId = objectId;
	_activeState._trackingLimits = _trackObjectTrackingLimits;
	_activeState._panSpeed = _trackObjectTrackingLimitsPanSpeed;
	_activeState._pointFlags = 0;
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
	_vm->_backgroundInstances->refreshPan();
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
	default:
		break;
	}
	_stack.push(item);
}

void Camera::popCameraMode() {
	if (_stack.empty())
		return;

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
	default:
		break;
	}

}

void Camera::clearCameraModeStack() {
	_stack.clear();
}

void Camera::update(uint32 currTime) {

	if (_activeState._paused)
		return;

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
	default:
		break;
	}

	if (_activeState._cameraMode != 6) {

		if (!isPanFinished() &&	updatePan(currTime)) {
			/* Unused
			if (_activeState._cameraMode == 1 || _activeState._cameraMode == 5)
				nullsub_2();
			*/
			_vm->_backgroundInstances->refreshPan();
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

void Camera::setBounds(Common::Point minPt, Common::Point maxPt) {
	_activeState._bounds._topLeft = minPt;
	_activeState._bounds._bottomRight = maxPt;
}

void Camera::setBoundsToDimensions(WidthHeight &dimensions) {
	// NOTE For the caller dimensions = artdispGetMasterBGDimensions();
	_activeState._bounds._topLeft.x = _screenMidX;
	_activeState._bounds._topLeft.y = _screenMidY;
	_activeState._bounds._bottomRight.x = MAX(0, dimensions._width - _screenWidth) + _screenMidX;
	_activeState._bounds._bottomRight.y = MAX(0, dimensions._height - _screenHeight) + _screenMidY;
	clipPanTargetPoint();
}

Common::Point Camera::getCurrentPan() {
	return _activeState._currPan;
}

Common::Point Camera::getScreenOffset() {
	Common::Point screenOffs = getCurrentPan();
	screenOffs.x -= _screenMidX;
	screenOffs.y -= _screenMidY;
	return screenOffs;
}

Common::Point Camera::getTrackingLimits() {
	return _activeState._trackingLimits;
}

bool Camera::isAtPanLimit(int limitNum) {
	switch (limitNum) {
	case 1:
		return _activeState._currPan.y <= _activeState._bounds._topLeft.y;
	case 2:
		return _activeState._currPan.y >= _activeState._bounds._bottomRight.y;
	case 3:
		return _activeState._currPan.x <= _activeState._bounds._topLeft.x;
	case 4:
		return _activeState._currPan.x >= _activeState._bounds._bottomRight.x;
	default:
		break;
	}
	return false;
}

void Camera::setActiveState(CameraState &state) {
	_activeState = state;
	_activeState._panStartTime = getCurrentTime();
}

void Camera::getActiveState(CameraState &state) {
	state = _activeState;
}

void Camera::refreshPan(BackgroundInstance *backgroundItem, WidthHeight &dimensions) {
	Common::Point screenOffs = getScreenOffset();
	int x = dimensions._width - _screenWidth;
	int y = dimensions._height - _screenHeight;
	for (uint i = 0; i < backgroundItem->_bgRes->_bgInfosCount; ++i) {
		const BgInfo &bgInfo = backgroundItem->_bgRes->_bgInfos[i];
		if (bgInfo._flags & 1) {
			backgroundItem->_panPoints[i] = screenOffs;
		} else {
			Common::Point newOffs(0, 0);
			if (x > 0 && bgInfo._surfInfo._dimensions._width - _screenWidth > 0)
				newOffs.x = screenOffs.x * (bgInfo._surfInfo._dimensions._width - _screenWidth) / x;
			if (y > 0 && bgInfo._surfInfo._dimensions._height - _screenHeight > 0)
				newOffs.y = screenOffs.y * (bgInfo._surfInfo._dimensions._height - _screenHeight) / y;
			backgroundItem->_panPoints[i] = newOffs;
		}
	}
}

void Camera::updateMode1(uint32 currTime) {
	Common::Point ptOffs = getPtOffset(*_activeState._panToPositionPtr);
	int deltaX = ptOffs.x - _activeState._currPan.x + _screenMidX - _activeState._centerPt.x;
	int deltaY = ptOffs.y - _activeState._currPan.y + _screenMidY - _activeState._centerPt.y;
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
	// TOOD CHECKME Bigger differences in Duckman
	Common::Point panToPosition = *_activeState._panToPositionPtr;
	uint pointFlags = 0;
	WRect rect;

	rect._topLeft.x = _screenMidX - _activeState._trackingLimits.x;
	rect._topLeft.y = _screenMidY - _activeState._trackingLimits.y;
	rect._bottomRight.x = _screenMidX + _activeState._trackingLimits.x;
	rect._bottomRight.y = _screenMidY + _activeState._trackingLimits.y;

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
	pt.x = pt.x - _activeState._centerPt.x + _screenMidX;
	pt.y = pt.y - _activeState._centerPt.y + _screenMidY;
	return pt;
}

void Camera::recalcPan(uint32 currTime) {
	_activeState._currPan2 = getCurrentPan();
	_activeState._time28 = currTime;

	if (_activeState._panSpeed == 0) {
		_activeState._time2E = 0;
	} else {
		FixedPoint16 x1 = _activeState._currPan2.x << 16;
		FixedPoint16 y1 = _activeState._currPan2.y << 16;
		FixedPoint16 x2 = _activeState._panTargetPoint.x << 16;
		FixedPoint16 y2 = _activeState._panTargetPoint.y << 16;
		FixedPoint16 distance = fixedDistance(x1, y1, x2, y2);
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

void Camera::init() {
	switch (_vm->getGameId()) {
	case kGameIdDuckman:
		initDuckman();
		break;
	case kGameIdBBDOU:
		initBBDOU();
		break;
	default:
		break;
	}
}

void Camera::initDuckman() {
	_centerObjectTrackingLimits.x = 4;
	_centerObjectTrackingLimits.y = 4;
	_screenWidth = 320;
	_screenHeight = 200;
	_screenMidX = 160;
	_screenMidY = 100;
	_trackObjectTrackingLimits.x = 80;
	_trackObjectTrackingLimits.y = 50;
	_trackObjectTrackingLimitsPanSpeed = 353;
}

void Camera::initBBDOU() {
	_centerObjectTrackingLimits.x = 8;
	_centerObjectTrackingLimits.y = 8;
	_screenWidth = 640;
	_screenHeight = 480;
	_screenMidX = 320;
	_screenMidY = 240;
	_trackObjectTrackingLimits.x = 160;
	_trackObjectTrackingLimits.y = 120;
	_trackObjectTrackingLimitsPanSpeed = 710;
}

} // End of namespace Illusions
