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
#include "titanic/star_control/base_stars.h" // includes class CStarVector
#include "titanic/star_control/error_code.h"
#include "titanic/support/simple_file.h"
// Not currently being used: #include "common/textconsole.h"

namespace Titanic {

CCameraMover::CCameraMover(const CNavigationInfo *src) {
	_lockCounter = 0;
	_starVector = nullptr;

	if (src) {
		copyFrom(src);
	} else {
		reset();
	}
}

CCameraMover::~CCameraMover() {
	clear();
}

void CCameraMover::clear() {
	if (_starVector) {
		delete _starVector;
		_starVector = nullptr;
	}
}

void CCameraMover::reset() {
	_speed = 0.0;
	_speedChangeCtr = 0.0;
	_speedChangeInc = 20.0;
	_unused = 0.0;
	_maxSpeed = 50000.0;
	_unusedX = 1.0;
	_unusedY = 1.0;
	_unusedZ = 0.0;
}

void CCameraMover::setVector(CStarVector *sv) {
	clear();
	_starVector = sv;
}

void CCameraMover::copyFrom(const CNavigationInfo *src) {
	_speed = src->_speed;
	_unused = src->_speedChangeCtr;
	_maxSpeed = src->_speedChangeInc;
	_speedChangeCtr = src->_unused;
	_speedChangeInc = src->_maxSpeed;
	_unusedX = src->_unusedX;
	_unusedY = src->_unusedY;
	_unusedZ = src->_unusedZ;
}

void CCameraMover::copyTo(CNavigationInfo *dest) {
	dest->_speed = _speed;
	dest->_speedChangeCtr = _unused;
	dest->_speedChangeInc = _maxSpeed;
	dest->_unused = _speedChangeCtr;
	dest->_maxSpeed = _speedChangeInc;
	dest->_unusedX = _unusedX;
	dest->_unusedY = _unusedY;
	dest->_unusedZ = _unusedZ;
}

void CCameraMover::increaseForwardSpeed() {
	if (!isLocked() && _speed < _maxSpeed) {
		_speedChangeCtr += _speedChangeInc;
		_speed += ABS(_speedChangeCtr);
	}
}

void CCameraMover::increaseBackwardSpeed() {
	if (!isLocked() && _speed > -_maxSpeed) {
		_speedChangeCtr -= _speedChangeInc;
		_speed -= ABS(_speedChangeCtr);
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

// TODO: this is confusing to negate the val value
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
	if (_lockCounter < 3)
	++_lockCounter;
}

void CCameraMover::decLockCount() {
	if (_lockCounter > 0)
		--_lockCounter;
}

} // End of namespace Titanic
