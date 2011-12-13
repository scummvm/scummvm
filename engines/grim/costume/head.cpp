/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/model.h"
#include "engines/grim/grim.h"
#include "engines/grim/costume/head.h"

namespace Grim {

Head::Head() :
	_maxPitch(0),
	_joint1(-1), _joint2(-1), _joint3(-1),
	_joint1Node(NULL), _joint2Node(NULL), _joint3Node(NULL),
	_headYaw(0), _headPitch(0) {

}

void Head::setJoints(int joint1, int joint2, int joint3) {
	_joint1 = joint1;
	_joint2 = joint2;
	_joint3 = joint3;
}

void Head::loadJoints(ModelNode *nodes) {
	if (_joint1 >= 0 && _joint2 >= 0 && _joint3 >= 0 && nodes) {
		_joint1Node = nodes + _joint1;
		_joint2Node = nodes + _joint2;
		_joint3Node = nodes + _joint3;
	}
}

void Head::setMaxAngles(float maxPitch, float maxYaw, float maxRoll) {
	_maxRoll = maxRoll;
	_maxPitch = maxPitch;
	_maxYaw = maxYaw;
}

void Head::lookAt(bool entering, const Math::Vector3d &point, float rate, const Math::Matrix4 &matrix) {
	if (_joint1Node) {
		float step = g_grim->getPerSecond(rate);
		float yawStep = step;
		float pitchStep = step / 3.f;
		if (!entering) {
			//animate yaw
			if (_headYaw > yawStep) {
				_headYaw -= yawStep;
			} else if (_headYaw < -yawStep) {
				_headYaw += yawStep;
			} else {
				_headYaw = 0;
			}
			//animate pitch
			if (_headPitch > pitchStep) {
				_headPitch -= pitchStep;
			} else if (_headPitch < -pitchStep) {
				_headPitch += pitchStep;
			} else {
				_headPitch = 0;
			}
			_joint1Node->_animYaw = _headYaw;
			Math::Angle pi = _headPitch / 3.f;
			_joint1Node->_animPitch += pi;
			_joint2Node->_animPitch += pi;
			_joint3Node->_animPitch += pi;
			_joint1Node->_animRoll = (_joint1Node->_animYaw.getDegrees() / 20.f) *
			_headPitch.getDegrees() / -5.f;

			if (_joint1Node->_animRoll > _maxRoll)
				_joint1Node->_animRoll = _maxRoll;
			if (_joint1Node->_animRoll < -_maxRoll)
				_joint1Node->_animRoll = -_maxRoll;
			return;
		}

		ModelNode *p = _joint3Node;
		while (p->_parent) {
			p = p->_parent;
		}
		p->setMatrix(matrix);
		p->update();

		Math::Vector3d v = point - _joint3Node->_matrix.getPosition();
		if (v.isZero()) {
			return;
		}

		float magnitude = sqrt(v.x() * v.x() + v.y() * v.y());
		float a = v.x() / magnitude;
		float b = v.y() / magnitude;
		float yaw;
		yaw = acos(a) * (180.0f / LOCAL_PI);
		if (b < 0.0f)
			yaw = 360.0f - yaw;

		Math::Angle bodyYaw = matrix.getYaw();
		p = _joint1Node->_parent;
		while (p) {
			bodyYaw += p->_yaw + p->_animYaw;
			p = p->_parent;
		}

		_joint1Node->_animYaw = (- 90 + yaw - bodyYaw);
		if (_joint1Node->_animYaw < -180.) {
			_joint1Node->_animYaw += 360;
		}
		if (_joint1Node->_animYaw > 180.) {
			_joint1Node->_animYaw -= 360;
		}

		if (_joint1Node->_animYaw > _maxYaw)
			_joint1Node->_animYaw = _maxYaw;
		if (_joint1Node->_animYaw < -_maxYaw)
			_joint1Node->_animYaw = -_maxYaw;

		float sqLenght = v.x() * v.x() + v.y() * v.y();
		float h;
		if (sqLenght > 0) {
			h = sqrt(sqLenght);
		} else {
			h = -sqrt(sqLenght);
		}
		magnitude = sqrt(v.z() * v.z() + h * h);
		a = h / magnitude;
		b = v.z() / magnitude;
		Math::Angle pitch;
		pitch = acos(a) * (180.0f / LOCAL_PI);

		if (b < 0.0f)
			pitch = 360.0f - pitch;

		if (pitch > 180)
			pitch -= 360;

		if (pitch > _maxPitch)
			pitch = _maxPitch;
		if (pitch < -_maxPitch)
			pitch = -_maxPitch;

		if ((_joint1Node->_animYaw > 0 && pitch < 0) || (_joint1Node->_animYaw < 0 && pitch > 0)) {
			pitch += _joint1Node->_animYaw / 10.f;
		} else {
			pitch -= _joint1Node->_animYaw / 10.f;
		}

		//animate pitch
		if (pitch - _headPitch > pitchStep)
			pitch = _headPitch + pitchStep;
		if (_headPitch - pitch > pitchStep)
			pitch = _headPitch - pitchStep;

		Math::Angle pi = pitch / 3.f;
		_joint1Node->_animPitch += pi;
		_joint2Node->_animPitch += pi;
		_joint3Node->_animPitch += pi;

		//animate yaw
		if (_joint1Node->_animYaw - _headYaw > yawStep)
			_joint1Node->_animYaw = _headYaw + yawStep;
		if (_headYaw - _joint1Node->_animYaw > yawStep)
			_joint1Node->_animYaw = _headYaw - yawStep;

		_joint1Node->_animRoll = (_joint1Node->_animYaw.getDegrees() / 20.f) *
		pitch.getDegrees() / -5.f;

		if (_joint1Node->_animRoll > _maxRoll)
			_joint1Node->_animRoll = _maxRoll;
		if (_joint1Node->_animRoll < -_maxRoll)
			_joint1Node->_animRoll = -_maxRoll;

		_headPitch = pitch;
		_headYaw = _joint1Node->_animYaw;
	}
}

void Head::saveState(SaveGame *state) const {
	state->writeLESint32(_joint1);
	state->writeLESint32(_joint2);
	state->writeLESint32(_joint3);
	state->writeFloat(_maxPitch);
	state->writeFloat(_maxYaw);
	state->writeFloat(_maxRoll);
	state->writeFloat(_headPitch.getDegrees());
	state->writeFloat(_headYaw.getDegrees());
}

void Head::restoreState(SaveGame *state) {
	_joint1 = state->readLESint32();
	_joint2 = state->readLESint32();
	_joint3 = state->readLESint32();
	_maxPitch = state->readFloat();
	_maxYaw = state->readFloat();
	_maxRoll = state->readFloat();

	_headPitch = state->readFloat();
	_headYaw = state->readFloat();
}

} // end of namespace Grim
