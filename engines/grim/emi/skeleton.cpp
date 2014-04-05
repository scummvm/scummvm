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

#include "common/stream.h"
#include "math/vector3d.h"
#include "math/vector4d.h"
#include "math/quat.h"
#include "engines/grim/debug.h"
#include "engines/grim/emi/animationemi.h"
#include "engines/grim/emi/skeleton.h"

namespace Grim {

#define ROTATE_OP 4
#define TRANSLATE_OP 3

Skeleton::Skeleton(const Common::String &filename, Common::SeekableReadStream *data) {
	loadSkeleton(data);
}

Skeleton::~Skeleton() {
	delete[] _joints;
}

void Skeleton::loadSkeleton(Common::SeekableReadStream *data) {
	_numJoints = data->readUint32LE();
	_joints = new Joint[_numJoints];

	char inString[32];

	for (int i = 0; i < _numJoints; i++) {
		data->read(inString, 32);
		_joints[i]._name = inString;
		data->read(inString, 32);
		_joints[i]._parent = inString;

		_joints[i]._trans.readFromStream(data);
		_joints[i]._quat.readFromStream(data);

		_joints[i]._parentIndex = findJointIndex(_joints[i]._parent, i);
	}
	initBones();
	resetAnim();
}

void Skeleton::initBone(int index) {
	// The matrix should have identity at this point.
	_joints[index]._quat.toMatrix(_joints[index]._relMatrix);
	// Might need to be translate instead.
	_joints[index]._relMatrix.setPosition(_joints[index]._trans);
	if (_joints[index]._parentIndex == -1) {
		_joints[index]._absMatrix = _joints[index]._relMatrix;
	} else {
		_joints[index]._absMatrix = _joints[_joints[index]._parentIndex]._absMatrix;

		// Might be the other way around.
		_joints[index]._absMatrix =  _joints[index]._absMatrix * _joints[index]._relMatrix;
	}
}

void Skeleton::initBones() {
	for (int i = 0; i < _numJoints; i++) {
		initBone(i);
	}
}

void Skeleton::resetAnim() {
	for (int i = 0; i < _numJoints; ++i) {
		_joints[i]._finalMatrix = _joints[i]._relMatrix;
		_joints[i]._finalQuat = _joints[i]._quat;
	}
}

void Skeleton::commitAnim() {
	for (int m = 0; m < _numJoints; ++m) {
		const Joint *parent = getParentJoint(&_joints[m]);
		if (parent) {
			_joints[m]._finalMatrix = parent->_finalMatrix * _joints[m]._finalMatrix;
			_joints[m]._finalQuat = parent->_finalQuat * _joints[m]._finalQuat;
		}
	}
}

int Skeleton::findJointIndex(const Common::String &name, int max) const {
	if (_numJoints > 0) {
		for (int i = 0; i < max; i++) {
			if (!_joints[i]._name.compareToIgnoreCase(name)) {
				return i;
			}
		}
	}
	return -1;
}

bool Skeleton::hasJoint(const Common::String &name) const {
	return name.empty() || findJointIndex(name, _numJoints) >= 0;
}

Joint *Skeleton::getJointNamed(const Common::String &name) const {
	int idx = findJointIndex(name, _numJoints);
	if (name.empty()) {
		return & _joints[0];
	} else if (idx == -1) {
		warning("Skeleton has no joint named '%s'!", name.c_str());
		return NULL;
	} else {
		return & _joints[idx];
	}
}

Joint *Skeleton::getParentJoint(const Joint *j) const {
	assert(j);
	if (j->_parentIndex == -1)
		return NULL;
	return &_joints[j->_parentIndex];
}

int Skeleton::getJointIndex(const Joint *j) const {
	int idx = j - _joints;
	assert(idx >= 0 && idx < _numJoints);
	return idx;
}


} // end of namespace Grim
