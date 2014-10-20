/* ResidualVM - A 3D game interpreter
*
* ResidualVM is the legal property of its developers, whose names
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

#include "engines/grim/grim.h"
#include "engines/grim/emi/costume/emihead.h"
#include "engines/grim/emi/costumeemi.h"
#include "engines/grim/emi/skeleton.h"

namespace Grim {

EMIHead::EMIHead(EMICostume *costume) : _yawRange(80.0f), _minPitch(-30.0f), _maxPitch(30.0f) {
	_cost = costume;
}

void EMIHead::setJoint(const char *joint, const Math::Vector3d &offset) {
	_jointName = joint;
	_offset = offset;
}

void EMIHead::setLimits(float yawRange, float maxPitch, float minPitch) {
	_yawRange = yawRange;
	_maxPitch = maxPitch;
	_minPitch = minPitch;
}

void EMIHead::lookAt(bool entering, const Math::Vector3d &point, float rate, const Math::Matrix4 &matrix) {
	if (!_cost->_emiSkel || !_cost->_emiSkel->_obj)
		return;

	if (_jointName.empty())
		return;

	Joint *joint = _cost->_emiSkel->_obj->getJointNamed(_jointName);
	if (!joint)
		return;

	Math::Quaternion lookAtQuat; // Note: Identity if not looking at anything.

	if (entering) {
		Math::Matrix4 jointToWorld = _cost->getOwner()->getFinalMatrix() * joint->_finalMatrix;
		Math::Vector3d jointWorldPos = jointToWorld.getPosition();
		Math::Matrix4 worldToJoint = jointToWorld;
		worldToJoint.invertAffineOrthonormal();

		Math::Vector3d targetDir = (point + _offset) - jointWorldPos;
		targetDir.normalize();

		const Math::Vector3d worldUp(0, 1, 0);
		Math::Vector3d frontDir = Math::Vector3d(worldToJoint(0, 1), worldToJoint(1, 1), worldToJoint(2, 1)); // Look straight ahead. (+Y)
		Math::Vector3d modelFront(0, 0, 1);
		Math::Vector3d modelUp(0, 1, 0);

		joint->_absMatrix.inverseRotate(&modelFront);
		joint->_absMatrix.inverseRotate(&modelUp);

		// Generate a world-space look at matrix.
		Math::Matrix4 lookAtTM;
		lookAtTM.setToIdentity();

		if (Math::Vector3d::dotProduct(targetDir, worldUp) >= 0.98f) // Avoid singularity if trying to look straight up.
			lookAtTM.buildFromTargetDir(modelFront, targetDir, modelUp, -frontDir); // Instead of orienting head towards scene up, orient head towards character "back",
		else if (Math::Vector3d::dotProduct(targetDir, worldUp) <= -0.98f) // Avoid singularity if trying to look straight down.
			lookAtTM.buildFromTargetDir(modelFront, targetDir, modelUp, frontDir); // Instead of orienting head towards scene down, orient head towards character "front",
		else
			lookAtTM.buildFromTargetDir(modelFront, targetDir, modelUp, worldUp);

		// Convert from world-space to joint-space.
		lookAtTM = worldToJoint * lookAtTM;

		// Apply angle limits.
		Math::Angle p, y, r;
		lookAtTM.getEuler(&y, &p, &r, Math::EO_ZXY);

		y.clampDegrees(_yawRange);
		p.clampDegrees(_minPitch, _maxPitch);
		r.clampDegrees(30.0f);

		lookAtTM.buildFromEuler(y, p, r, Math::EO_ZXY);

		lookAtQuat.fromMatrix(lookAtTM.getRotation());
	}

	if (_headRot != lookAtQuat) {
		Math::Quaternion diff = _headRot.inverse() * lookAtQuat;
		diff.normalize();
		float angle = 2 * acos(fminf(fmaxf(diff.w(), -1.0f), 1.0f));
		if (diff.w() < 0.0f) {
			angle = 2 * (float)M_PI - angle;
		}

		float turnAmount = g_grim->getPerSecond(rate * ((float)M_PI / 180.0f));
		if (turnAmount < angle)
			_headRot = _headRot.slerpQuat(lookAtQuat, turnAmount / angle);
		else
			_headRot = lookAtQuat;
	}

	if (_headRot != Math::Quaternion()) { // If not identity..
		joint->_animMatrix = joint->_animMatrix * _headRot.toMatrix();
		joint->_animQuat = joint->_animQuat * _headRot;
		_cost->_emiSkel->_obj->commitAnim();
	}
}

void EMIHead::saveState(SaveGame *state) const {
	state->writeString(_jointName);
	state->writeVector3d(_offset);
	state->writeFloat(_headRot.x());
	state->writeFloat(_headRot.y());
	state->writeFloat(_headRot.z());
	state->writeFloat(_headRot.w());
	state->writeFloat(_yawRange);
	state->writeFloat(_minPitch);
	state->writeFloat(_maxPitch);
}

void EMIHead::restoreState(SaveGame *state) {
	if (state->saveMinorVersion() >= 15) {
		_jointName = state->readString();
		_offset = state->readVector3d();
		_headRot.x() = state->readFloat();
		_headRot.y() = state->readFloat();
		_headRot.z() = state->readFloat();
		_headRot.w() = state->readFloat();
		if (state->saveMinorVersion() >= 16) {
			_yawRange = state->readFloat();
			_minPitch = state->readFloat();
			_maxPitch = state->readFloat();
		}
	} else {
		state->readLESint32();
		state->readLESint32();
		state->readLESint32();
		state->readFloat();
		state->readFloat();
		state->readFloat();
		if (state->saveMinorVersion() < 2) {
			state->readFloat();
			state->readFloat();
		} else {
			for (int i = 0; i < 3; ++i) {
				state->readFloat();
				state->readFloat();
				state->readFloat();
			}
		}
	}
}

} // end of namespace Grim
