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
		setMotion(src);
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
	_currVelocity = 0.0;
	_incVelocity = 0.0;
	_incAcceleration = 20.0;
	_minVelocity = 0.0;
	_maxVelocity = 50000.0;
	_rotationX = 1.0;
	_rotationY = 1.0;
	_rotationZ = 0.0;
}

void CCameraMover::setVector(CStarVector *sv) {
	clear();
	_starVector = sv;
}

void CCameraMover::setMotion(const CNavigationInfo *src) {
	_currVelocity = src->_initialVelocity;
	_minVelocity = src->_minVelocity;
	_maxVelocity = src->_maxVelocity;
	_incVelocity = src->_velocity;
	_incAcceleration = src->_acceleration;
	_rotationX = src->_rotationX;
	_rotationY = src->_rotationY;
	_rotationZ = src->_rotationZ;
}

void CCameraMover::getMotion(CNavigationInfo *dest) {
	dest->_initialVelocity = _currVelocity;
	dest->_minVelocity = _minVelocity;
	dest->_maxVelocity = _maxVelocity;
	dest->_velocity = _incVelocity;
	dest->_acceleration = _incAcceleration;
	dest->_rotationX = _rotationX;
	dest->_rotationY = _rotationY;
	dest->_rotationZ = _rotationZ;
}

void CCameraMover::accelerate() {
	if (!isLocked() && _currVelocity < _maxVelocity) {
		_incVelocity += _incAcceleration;
		_currVelocity += ABS(_incVelocity);
	}
}

void CCameraMover::deccelerate() {
	if (!isLocked() && _currVelocity > -_maxVelocity) {
		_incVelocity -= _incAcceleration;
		_currVelocity -= ABS(_incVelocity);
	}
}

void CCameraMover::fullSpeed() {
	if (!isLocked())
		_currVelocity = _maxVelocity;
}

void CCameraMover::stop() {
	if (!isLocked()) {
		_currVelocity = 0.0;
		_incVelocity = 0.0;
	}
}

// TODO: this is confusing to negate the val value
void CCameraMover::load(SimpleFile *file, int val) {
	if (!val) {
		_currVelocity = file->readFloat();
		_incVelocity = file->readFloat();
		_incAcceleration = file->readFloat();
		_minVelocity = file->readFloat();
		_maxVelocity = file->readFloat();
		_rotationX = file->readFloat();
		_rotationY = file->readFloat();
		_rotationZ = file->readFloat();
	}
}

void CCameraMover::save(SimpleFile *file, int indent) {
	file->writeFloatLine(_currVelocity, indent);
	file->writeFloatLine(_incVelocity, indent);
	file->writeFloatLine(_incAcceleration, indent);
	file->writeFloatLine(_minVelocity, indent);
	file->writeFloatLine(_maxVelocity, indent);
	file->writeFloatLine(_rotationX, indent);
	file->writeFloatLine(_rotationY, indent);
	file->writeFloatLine(_rotationZ, indent);
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
