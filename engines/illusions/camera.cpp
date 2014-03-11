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

#include "illusions/camera.h"
#include "illusions/time.h"

namespace Illusions {

Camera::Camera() {
	_activeState._cameraMode = 6;
	_activeState._paused = 0;
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
	_activeState._paused = 0;
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
	// TODO largeObj_backgroundItem_refreshPan();
	_activeState._panToPositionPtr = 0;
	_activeState._panObjectId = 0;
	_activeState._panNotifyId = 0;
	_activeState._trackingLimits.x = 0;
	_activeState._trackingLimits.y = 0;
	_activeState._pointFlags = 0;
	_activeState._pt.x = 320;
	_activeState._pt.y = 240;
}

Common::Point Camera::getCurrentPan() {
	return _activeState._currPan;
}

} // End of namespace Illusions
