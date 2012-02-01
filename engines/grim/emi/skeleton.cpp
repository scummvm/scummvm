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
		_joints[i]._animIndex = -1;
	}
	
	for(int i = 0; i < _anim->_numBones; i++) {
		int index = findJointIndex(_anim->_bones[i]->_boneName, _numJoints);
		_joints[index]._animIndex = i;
	}
	resetAnim();
}

int Skeleton::findJointIndex(Common::String name, int max) {
	if (_numJoints > 0) {
		for(int i = 0; i < max; i++) {
			if(_joints[i]._name == name) {
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

	int curJoint = 0;
	int animIdx = 0;
	int prevKeyfIdx = 0;
	int keyfIdx = 0;
	int curKeyFrame = 0;
	float timeDelta = 0.0f;
	float interpVal = 0.0f;
	
	Math::Matrix4 relFinal;
	Math::Vector3d vec;
	
	for (curJoint = 0; curJoint < _numJoints; curJoint++) {
		animIdx = _joints[curJoint]._animIndex;
		
		if (animIdx >= 0) {
			prevKeyfIdx = -1;
			keyfIdx = 0;

			Bone *_curBone = _anim->_bones[animIdx];
			// Find the right keyframe
			for (curKeyFrame = 0; curKeyFrame < _curBone->_count; curKeyFrame++) {
				if (_curBone->_operation == ROTATE_OP) {
					if (_curBone->_rotations[curKeyFrame]->_time >= _time) {
						keyfIdx = curKeyFrame;
						break;
					}
				} else if (_curBone->_operation == TRANSLATE_OP) {
					if (_curBone->_translations[curKeyFrame]->_time >= _time) {
						keyfIdx = curKeyFrame;
						break;
					}
					
				}
			}

			relFinal = _joints[curJoint]._relMatrix;
			Math::Quaternion quat;

			if (_curBone->_operation == ROTATE_OP) {
				if (keyfIdx == 0) {
					quat = _curBone->_rotations[keyfIdx]->_quat;
				} else if (keyfIdx == _curBone->_count - 1) {
					quat = _curBone->_rotations[keyfIdx-1]->_quat;
				} else {
					timeDelta = _curBone->_rotations[keyfIdx-1]->_time - _curBone->_rotations[keyfIdx]->_time;
					interpVal = (_time - _curBone->_rotations[keyfIdx]->_time) / timeDelta;
					
					// Might be the other way around (keyfIdx - 1 slerped against keyfIdx)
					quat = _curBone->_rotations[keyfIdx]->_quat.slerpQuat(_curBone->_rotations[keyfIdx - 1]->_quat, interpVal);
				}
				quat.toMatrix(relFinal);
			} else if (_curBone->_operation == TRANSLATE_OP) {
				if (keyfIdx == 0) {
					vec = _curBone->_translations[keyfIdx]->_vec;
				} else if (keyfIdx == _curBone->_count - 1) {
					vec = _curBone->_translations[keyfIdx-1]->_vec;
				} else {
					timeDelta = _curBone->_translations[keyfIdx-1]->_time - _curBone->_translations[keyfIdx]->_time;
					interpVal = (_time - _curBone->_translations[keyfIdx]->_time) / timeDelta;
					
					vec.x() = _curBone->_translations[keyfIdx-1]->_vec.x() +
					(_curBone->_translations[keyfIdx]->_vec.x() - _curBone->_translations[keyfIdx-1]->_vec.x()) * interpVal;
					
					vec.y() = _curBone->_translations[keyfIdx-1]->_vec.y() +
					(_curBone->_translations[keyfIdx]->_vec.y() - _curBone->_translations[keyfIdx-1]->_vec.y()) * interpVal;
					
					vec.z() = _curBone->_translations[keyfIdx-1]->_vec.z() +
					(_curBone->_translations[keyfIdx]->_vec.z() - _curBone->_translations[keyfIdx-1]->_vec.z()) * interpVal;
				}
				relFinal.setPosition(vec);
			} else {
				error("Skeleton::Animate, invalid operation %d", _curBone->_operation);
			}
		} else {
			relFinal = _joints[curJoint]._relMatrix;
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
