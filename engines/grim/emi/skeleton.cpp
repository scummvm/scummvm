/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

Skeleton::Skeleton(const Common::String &filename, Common::SeekableReadStream *data) : _anim(NULL), _time(0) {
	loadSkeleton(data);
}

Skeleton::~Skeleton() {
	delete[] _joints;
}

void Skeleton::loadSkeleton(Common::SeekableReadStream *data) {
	_numJoints = data->readUint32LE();
	_joints = new Joint[_numJoints];
	
	char inString[32];
	
	for(int i = 0;i < _numJoints; i++) {
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
	if(_joints[index]._parentIndex == -1) {
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
	_time = 0;
	for (int i = 0; i < _numJoints; i++) {
		_joints[i]._finalMatrix = _joints[i]._absMatrix;
	}
}

void Skeleton::setAnim(AnimationEmi *anim) {
	if (_anim == anim) {
		return;
	}
	_anim = anim;
	if (!_anim) {
		return;
	}
	for (int i = 0; i < _numJoints; i++) {
		_joints[i]._animIndex[0] = -1;
		_joints[i]._animIndex[1] = -1;
	}
	
	for(int i = 0; i < _anim->_numBones; i++) {
		int index = findJointIndex(_anim->_bones[i]->_boneName, _numJoints);
		if (_anim->_bones[i]->_operation == 3) {
			_joints[index]._animIndex[0] = i;
		} else {
			_joints[index]._animIndex[1] = i;
		}
	}
	resetAnim();
}

int Skeleton::findJointIndex(Common::String name, int max) {
	if (_numJoints > 0) {
		for (int i = 0; i < max; i++) {
			if (_joints[i]._name == name) {
				return i;
			}
		}
	}
	return -1;
}

void Skeleton::animate(float delta) {
	if (_anim == NULL)
		return;
	_time += delta;
	if (_time > _anim->_duration) {
		resetAnim();
	}
	
	Math::Vector3d vec;

	for (int curJoint = 0; curJoint < _numJoints; curJoint++) {
		int transIdx = _joints[curJoint]._animIndex[0];
		int rotIdx = _joints[curJoint]._animIndex[1];

		float timeDelta = 0.0f;
		float interpVal = 0.0f;

		Math::Matrix4 relFinal = _joints[curJoint]._relMatrix;
		
		if (rotIdx >= 0) {
			int keyfIdx = 0;
			Math::Quaternion quat;
			Bone *curBone = _anim->_bones[rotIdx];
			Math::Vector3d relPos = relFinal.getPosition();

			// Find the right keyframe
			for (int curKeyFrame = 0; curKeyFrame < curBone->_count; curKeyFrame++) {
				if (curBone->_rotations[curKeyFrame]->_time >= _time) {
					keyfIdx = curKeyFrame;
					break;
				}
			}

			if (keyfIdx == 0) {
				quat = curBone->_rotations[keyfIdx]->_quat;
			} else if (keyfIdx == curBone->_count - 1) {
				quat = curBone->_rotations[keyfIdx-1]->_quat;
			} else {
				timeDelta = curBone->_rotations[keyfIdx-1]->_time - curBone->_rotations[keyfIdx]->_time;
				interpVal = (_time - curBone->_rotations[keyfIdx]->_time) / timeDelta;

				// Might be the other way around (keyfIdx - 1 slerped against keyfIdx)
				quat = curBone->_rotations[keyfIdx]->_quat.slerpQuat(curBone->_rotations[keyfIdx - 1]->_quat, interpVal);
			}
			quat.toMatrix(relFinal);
			relFinal.setPosition(relPos);
		}

		if (transIdx >= 0) {
			int keyfIdx = 0;
			Bone *curBone = _anim->_bones[transIdx];
			// Find the right keyframe
			for (int curKeyFrame = 0; curKeyFrame < curBone->_count; curKeyFrame++) {
				if (curBone->_translations[curKeyFrame]->_time >= _time) {
					keyfIdx = curKeyFrame;
					break;
				}
			}

			if (keyfIdx == 0) {
				vec = curBone->_translations[keyfIdx]->_vec;
			} else if (keyfIdx == curBone->_count - 1) {
				vec = curBone->_translations[keyfIdx-1]->_vec;
			} else {
				timeDelta = curBone->_translations[keyfIdx-1]->_time - curBone->_translations[keyfIdx]->_time;
				interpVal = (_time - curBone->_translations[keyfIdx]->_time) / timeDelta;

				vec.x() = curBone->_translations[keyfIdx-1]->_vec.x() +
					(curBone->_translations[keyfIdx]->_vec.x() - curBone->_translations[keyfIdx-1]->_vec.x()) * interpVal;

				vec.y() = curBone->_translations[keyfIdx-1]->_vec.y() +
					(curBone->_translations[keyfIdx]->_vec.y() - curBone->_translations[keyfIdx-1]->_vec.y()) * interpVal;

				vec.z() = curBone->_translations[keyfIdx-1]->_vec.z() +
					(curBone->_translations[keyfIdx]->_vec.z() - curBone->_translations[keyfIdx-1]->_vec.z()) * interpVal;
			}
			relFinal.setPosition(vec);
		}
		
		if (_joints[curJoint]._parentIndex == -1) {
			_joints[curJoint]._finalMatrix = relFinal;
		} else {
			_joints[curJoint]._finalMatrix = _joints[_joints[curJoint]._parentIndex]._finalMatrix;
			_joints[curJoint]._finalMatrix = _joints[curJoint]._finalMatrix * relFinal;
		}
	} // end for
}

} // end of namespace Grim
