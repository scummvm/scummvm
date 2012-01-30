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

Skeleton::Skeleton(const Common::String &filename, Common::SeekableReadStream *data) {
	loadSkeleton(data);
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
	// TODO: Fill in math
}

void Skeleton::initBones() {
	for (int i = 0; i < _numJoints; i++) {
		initBone(i);
	}	
}

void Skeleton::resetAnim() {
	for (int i = 0; i < _numJoints; i++) {
		_joints[i]._finalMatrix = _joints[i]._absMatrix;
	}
}

void Skeleton::setAnim(AnimationEmi *anim) {
	_anim = anim;
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

Skeleton::~Skeleton() {
	delete[] _joints;
}

} // end of namespace Grim
