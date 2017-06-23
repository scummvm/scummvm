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

#include "titanic/star_control/camera_mover.h"
#include "common/textconsole.h"

namespace Titanic {

CCameraMover::CCameraMover(const CNavigationInfo *src) {
	_lockCounter = 0;
	_starVector = nullptr;

	if (src) {
		copyFrom(src);
	} else {
		_speed = 0.0;
		_speedChangeCtr = 0.0;
		_speedChangeInc = 20.0;
		_unused = 0.0;
		_maxSpeed = 50000.0;
		_unusedX = 1.0;
		_unusedY = 1.0;
		_unusedZ = 0.0;
	}
}

CCameraMover::~CCameraMover() {
	clear();
}

void CCameraMover::copyFrom(const CNavigationInfo *src) {
	*((CNavigationInfo *)this) = *src;
}

void CCameraMover::copyTo(CNavigationInfo *dest) {
	*dest = *((CNavigationInfo *)this);
}

void CCameraMover::increaseSpeed() {
	if (!isLocked() && _speed < _maxSpeed) {
		_speedChangeCtr += _speedChangeInc;
		if (_speedChangeCtr > _speed)
			_speed -= _speedChangeCtr;
		else
			_speed += _speedChangeCtr;
	}
}

void CCameraMover::decreaseSpeed() {
	if (!isLocked()) {
		_speedChangeCtr -= _speedChangeInc;
		if (_speedChangeCtr > _speed)
			_speed -= _speedChangeCtr;
		else
			_speed += _speedChangeCtr;

		if (_speedChangeCtr < 0.0)
			_speedChangeCtr = 0.0;
	}
}

void CCameraMover::fullSpeed() {
	if (!isLocked())
		_speed = _maxSpeed;
}

void CCameraMover::stop() {
	if (!isLocked()) {
		_speed = 0.0;
		_speedChangeCtr = 0.0;
	}
}

void CCameraMover::updatePosition(CErrorCode &errorCode, FVector &pos, FMatrix &orientation) {
	if (_speed > 0.0) {
		pos._x += orientation._row3._x * _speed;
		pos._y += orientation._row3._y * _speed;
		pos._z += orientation._row3._z * _speed;

		errorCode.set();
	}
}

void CCameraMover::setVector(CStarVector *sv) {
	clear();
	_starVector = sv;
}

void CCameraMover::clear() {
	if (_starVector) {
		delete _starVector;
		_starVector = nullptr;
	}
}

void CCameraMover::load(SimpleFile *file, int val) {
	if (!val) {
		_speed = file->readFloat();
		_speedChangeCtr = file->readFloat();
		_speedChangeInc = file->readFloat();
		_unused = file->readFloat();
		_maxSpeed = file->readFloat();
		_unusedX = file->readFloat();
		_unusedY = file->readFloat();
		_unusedZ = file->readFloat();
	}
}

void CCameraMover::save(SimpleFile *file, int indent) {
	file->writeFloatLine(_speed, indent);
	file->writeFloatLine(_speedChangeCtr, indent);
	file->writeFloatLine(_speedChangeInc, indent);
	file->writeFloatLine(_unused, indent);
	file->writeFloatLine(_maxSpeed, indent);
	file->writeFloatLine(_unusedX, indent);
	file->writeFloatLine(_unusedY, indent);
	file->writeFloatLine(_unusedZ, indent);
}

void CCameraMover::incLockCount() {
	++_lockCounter;
}

void CCameraMover::decLockCount() {
	if (_lockCounter > 0)
		--_lockCounter;
}

} // End of namespace Titanic
