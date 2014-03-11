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
	_activeState._pt.x = 320;
	_activeState._pt.y = 240;
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
	// TODO camera_clipPanTargetPoint();
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
	_activeState._pt.x = 320;
	_activeState._pt.y = 240;
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
				if (_activeState._panNotifyId) {
					// TODO scrmgrNotifyID(_activeState._panNotifyId);
					_activeState._panNotifyId = 0;
				}
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
	// TODO camera_clipPanTargetPoint();
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
	int deltaX = ptOffs.x - _activeState._currPan.x + 320 - _activeState._pt.x;
	int deltaY = ptOffs.y - _activeState._currPan.y + 240 - _activeState._pt.y;
	int deltaXAbs = ABS(deltaX);
	int deltaYAbs = ABS(deltaY);

	if (deltaXAbs > _activeState._trackingLimits.x) {
		_activeState._panTargetPoint.x = _activeState._currPan.x + ABS(deltaXAbs - _activeState._trackingLimits.x) * (deltaX >= 0 ? 1 : -1);
	} else {
		_activeState._panTargetPoint.x = _activeState._currPan.x;
	}

	if (deltaYAbs > _activeState._trackingLimits.y) {
		_activeState._panTargetPoint.y = _activeState._currPan.y + ABS(deltaYAbs - _activeState._trackingLimits.y) * (deltaY >= 0 ? 1 : -1);
	} else {
		_activeState._panTargetPoint.y = _activeState._currPan.y;
	}

	// TODO Camera_clipPanTargetPoint();

	if (!isPanFinished()) {
		uint32 oldPanTime = _activeState._panStartTime;
		_activeState._panStartTime = _activeState._time28;
		// TODO Camera_recalcPan(oldPanTime);
	}
	
}

void Camera::updateMode2(uint32 currTime) {
	// TODO
}

void Camera::updateMode3(uint32 currTime) {
	// TODO
}

bool Camera::updatePan(uint32 currTime) {
	// TODO
	return false;
}

bool Camera::isPanFinished() {
	return _activeState._currPan.x == _activeState._panTargetPoint.x && _activeState._currPan.y == _activeState._panTargetPoint.y;
}

Common::Point Camera::getPtOffset(Common::Point pt) {
	pt.x = pt.x - _activeState._pt.x + 320;
	pt.y = pt.y - _activeState._pt.y + 240;
	return pt;
}

} // End of namespace Illusions
