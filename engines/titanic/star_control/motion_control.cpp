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

#include "titanic/star_control/motion_control.h"
#include "titanic/star_control/base_stars.h"
#include "titanic/star_control/error_code.h"
#include "titanic/star_control/camera.h"
#include "titanic/support/simple_file.h"

namespace Titanic {

void CCallbackHandler::apply() {
	_owner->addLockedStar(_vector);
}

/*------------------------------------------------------------------------*/

CMotionControl::CMotionControl(const CNavigationInfo *src) {
	_lockCounter = 0;
	_callback = nullptr;

	if (src) {
		setMotion(src);
	} else {
		reset();
	}
}

CMotionControl::~CMotionControl() {
	clear();
}

void CMotionControl::clear() {
	if (_callback) {
		delete _callback;
		_callback = nullptr;
	}
}

void CMotionControl::reset() {
	_currVelocity = 0.0;
	_incVelocity = 0.0;
	_incAcceleration = 20.0;
	_minVelocity = 0.0;
	_maxVelocity = 50000.0;
	_rotationX = 1.0;
	_rotationY = 1.0;
	_rotationZ = 0.0;
}

void CMotionControl::setCallback(CCallbackHandler *callback) {
	clear();
	_callback = callback;
}

void CMotionControl::setMotion(const CNavigationInfo *src) {
	_currVelocity = src->_initialVelocity;
	_minVelocity = src->_minVelocity;
	_maxVelocity = src->_maxVelocity;
	_incVelocity = src->_velocity;
	_incAcceleration = src->_acceleration;
	_rotationX = src->_rotationX;
	_rotationY = src->_rotationY;
	_rotationZ = src->_rotationZ;
}

void CMotionControl::getMotion(CNavigationInfo *dest) {
	dest->_initialVelocity = _currVelocity;
	dest->_minVelocity = _minVelocity;
	dest->_maxVelocity = _maxVelocity;
	dest->_velocity = _incVelocity;
	dest->_acceleration = _incAcceleration;
	dest->_rotationX = _rotationX;
	dest->_rotationY = _rotationY;
	dest->_rotationZ = _rotationZ;
}

void CMotionControl::accelerate() {
	if (!isLocked() && _currVelocity < _maxVelocity) {
		_incVelocity += _incAcceleration;
		_currVelocity += ABS(_incVelocity);
	}
}

void CMotionControl::deccelerate() {
	if (!isLocked() && _currVelocity > -_maxVelocity) {
		_incVelocity -= _incAcceleration;
		_currVelocity -= ABS(_incVelocity);
	}
}

void CMotionControl::fullSpeed() {
	if (!isLocked())
		_currVelocity = _maxVelocity;
}

void CMotionControl::stop() {
	if (!isLocked()) {
		_currVelocity = 0.0;
		_incVelocity = 0.0;
	}
}

// TODO: this is confusing to negate the val value
void CMotionControl::load(SimpleFile *file, int version) {
	if (version == 0) {
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

void CMotionControl::save(SimpleFile *file, int indent) {
	file->writeFloatLine(_currVelocity, indent);
	file->writeFloatLine(_incVelocity, indent);
	file->writeFloatLine(_incAcceleration, indent);
	file->writeFloatLine(_minVelocity, indent);
	file->writeFloatLine(_maxVelocity, indent);
	file->writeFloatLine(_rotationX, indent);
	file->writeFloatLine(_rotationY, indent);
	file->writeFloatLine(_rotationZ, indent);
}

void CMotionControl::incLockCount() {
	if (_lockCounter < 3)
	++_lockCounter;
}

void CMotionControl::decLockCount() {
	if (_lockCounter > 0)
		--_lockCounter;
}

} // End of namespace Titanic
