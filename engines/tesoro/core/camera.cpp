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

#include "common/textconsole.h"

#include "tesoro/core/camera.h"

#include "tesoro/core/object.h"
#include "tesoro/core/wrap.h"

namespace Tesoro {

static const int32 kHalfWidth = 320;
static const int32 kHalfHeight = 240;
static const int32 kDeadZoneLeft = 75; // how far the target may stand from the goal before the camera scrolls
static const int32 kDeadZoneRight = 75;
static const int32 kScrollStep = 100;
static const int32 kScrollTime = 750;

void Camera::init() {
	_target = nullptr;
	_startX = _startY = 0;
	_startTime = 0;
	_duration = 0;
	_goalX = _goalY = 0;
	_boundLeft = _boundTop = _boundRight = _boundBottom = 0;
	_x = _y = 0;
	_scaledX = _scaledY = 0;
}

void Camera::setBounds(int32 left, int32 top, int32 right, int32 bottom) {
	_boundLeft = left;
	_boundTop = top;
	_boundRight = right;
	_boundBottom = bottom;
}

bool Camera::setPositionClamped(int32 x, int32 y) {
	_x = x;
	_y = y;

	const bool leftOk = _boundLeft <= wrapSub(x, kHalfWidth);
	if (!leftOk)
		_x = wrapAdd(_boundLeft, kHalfWidth);

	const bool bottomOk = _boundBottom <= wrapSub(y, kHalfHeight);
	if (!bottomOk)
		_y = wrapAdd(_boundBottom, kHalfHeight);

	const bool rightOk = wrapAdd(_x, kHalfWidth) <= _boundRight;
	if (!rightOk)
		_x = wrapSub(_boundRight, kHalfWidth);

	const bool topOk = wrapAdd(_y, kHalfHeight) <= _boundTop;
	if (!topOk)
		_y = wrapSub(_boundTop, kHalfHeight);

	return leftOk && bottomOk && rightOk && topOk;
}

void Camera::moveTo(int32 x, int32 y, int32 duration, int32 now) {
	if (duration != 0) {
		_duration = duration;
		_startTime = now;
		_startX = _x;
		_startY = _y;
		_goalX = x;
		_goalY = y;
		return;
	}

	setPositionClamped(x, y);
	_goalX = x;
	_goalY = y;
}

void Camera::updateToward(int32 divisor, int32 now) {
	if (_target) {
		const int32 targetX = _target->x();

		if (targetX < wrapSub(_goalX, kDeadZoneLeft) && _boundLeft < wrapSub(_goalX, kHalfWidth)) {
			_duration = kScrollTime;
			_startTime = now;
			_goalX = wrapSub(_goalX, kScrollStep);
			_startX = _x;
			_startY = _y;
		}

		if (wrapAdd(_goalX, kDeadZoneRight) < targetX && wrapAdd(_goalX, kHalfWidth - 1) < _boundRight) {
			_duration = kScrollTime;
			_startTime = now;
			_goalX = wrapAdd(_goalX, kScrollStep);
			_startX = _x;
			_startY = _y;
		}
	}

	if (_duration != 0) {
		const int32 elapsed = wrapSub(now, _startTime);
		if (_duration < elapsed) {
			_duration = 0;
			setPositionClamped(_goalX, _goalY);
			_goalX = _x;
			_goalY = _y;
		} else {
			const int32 x = wrapAdd(wrapMul(wrapSub(_goalX, _startX), elapsed) / _duration, _startX);
			const int32 y = wrapAdd(wrapMul(wrapSub(_goalY, _startY), elapsed) / _duration, _startY);
			setPositionClamped(x, y);
		}
	}

	if (divisor == 0) {
		warning("Camera: divisor 0");
		return;
	}
	_scaledX = wrapMul(_x, 300) / divisor;
	_scaledY = wrapMul(_y, 300) / divisor;
}

} // End of namespace Tesoro
