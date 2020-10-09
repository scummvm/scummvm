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

Skeleton::Skeleton(const Common::String &filename, Common::SeekableReadStream *data) :
		_numJoints(0), _joints(nullptr), _animLayers(nullptr) {
	loadSkeleton(data);
}

Skeleton::~Skeleton() {
	for (int i = 0; i < MAX_ANIMATION_LAYERS; ++i) {
		delete[] _animLayers[i]._jointAnims;
	}
	delete[] _animLayers;
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

		_joints[i]._parentIndex = findJointIndex(_joints[i]._parent);

		_jointsMap[_joints[i]._name] = i;
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

	_animLayers = new AnimationLayer[MAX_ANIMATION_LAYERS];
	for (int i = 0; i < MAX_ANIMATION_LAYERS; ++i) {
		_animLayers[i]._jointAnims = new JointAnimation[_numJoints];
	}
}

void Skeleton::resetAnim() {
	for (int i = 0; i < MAX_ANIMATION_LAYERS; ++i) {
		AnimationLayer &layer = _animLayers[i];
		for (int j = 0; j < _numJoints; ++j) {
			JointAnimation &jointAnim = layer._jointAnims[j];
			jointAnim._pos.set(0.f, 0.f, 0.f);
			jointAnim._quat.set(0.f, 0.f, 0.f, 1.f);
			jointAnim._transWeight = 0.0f;
			jointAnim._rotWeight = 0.0f;
		}
	}
	for (int i = 0; i < _numJoints; ++i) {
		_joints[i]._animMatrix = _joints[i]._relMatrix;
		_joints[i]._animQuat = _joints[i]._quat;
	}
}

void Skeleton::animate() {
	resetAnim();

	// This first pass over the animations calculates bone-specific sums of blend weights for all
	// animation layers. The sums must be pre-computed in order to be able to normalize the blend
	// weights properly in the next step.
	for (Common::List<AnimationStateEmi*>::iterator j = _activeAnims.begin(); j != _activeAnims.end(); ++j) {
		(*j)->computeWeights();
	}

	// Now make a second pass over the animations to actually accumulate animation to layers.
	for (Common::List<AnimationStateEmi*>::iterator j = _activeAnims.begin(); j != _activeAnims.end(); ++j) {
		(*j)->animate();
	}

	// Blend the layers together in priority order to produce the final result. Highest priority
	// layer will get as much weight as it wants, while the next highest priority will get the
	// amount that remains and so on.
	for (int i = 0; i < _numJoints; ++i) {
		float remainingTransWeight = 1.0f;
		float remainingRotWeight = 1.0f;

		for (int j = MAX_ANIMATION_LAYERS - 1; j >= 0; --j) {
			AnimationLayer &layer = _animLayers[j];
			JointAnimation &jointAnim = layer._jointAnims[i];

			if (remainingRotWeight > 0.0f && jointAnim._rotWeight != 0.0f) {
				Math::Vector3d pos = _joints[i]._animMatrix.getPosition();
				_joints[i]._animQuat = _joints[i]._animQuat.slerpQuat(_joints[i]._animQuat * jointAnim._quat, remainingRotWeight);
				_joints[i]._animQuat.toMatrix(_joints[i]._animMatrix);
				_joints[i]._animMatrix.setPosition(pos);

				remainingRotWeight *= 1.0f - jointAnim._rotWeight;
			}

			if (remainingTransWeight > 0.0f && jointAnim._transWeight != 0.0f) {
				Math::Vector3d pos = _joints[i]._animMatrix.getPosition();
				Math::Vector3d delta = jointAnim._pos;
				_joints[i]._animMatrix.setPosition(pos + delta * remainingTransWeight);

				remainingTransWeight *= 1.0f - jointAnim._transWeight;
			}

			if (remainingRotWeight <= 0.0f && remainingTransWeight <= 0.0f)
				break;
		}
	}

	commitAnim();
}

void Skeleton::addAnimation(AnimationStateEmi *anim) {
	_activeAnims.push_back(anim);
}
void Skeleton::removeAnimation(AnimationStateEmi *anim) {
	_activeAnims.remove(anim);
}

void Skeleton::commitAnim() {
	for (int m = 0; m < _numJoints; ++m) {
		const Joint *parent = getParentJoint(&_joints[m]);
		if (parent) {
			_joints[m]._finalMatrix = parent->_finalMatrix * _joints[m]._animMatrix;
			_joints[m]._finalQuat = parent->_finalQuat * _joints[m]._animQuat;
		} else {
			_joints[m]._finalMatrix = _joints[m]._animMatrix;
			_joints[m]._finalQuat = _joints[m]._animQuat;
		}
	}
}

int Skeleton::findJointIndex(const Common::String &name) const {
	JointMap::const_iterator it = _jointsMap.find(name);
	if (it != _jointsMap.end())
		return it->_value;
	return -1;
}

bool Skeleton::hasJoint(const Common::String &name) const {
	return name.empty() || findJointIndex(name) >= 0;
}

Joint *Skeleton::getJointNamed(const Common::String &name) const {
	int idx = findJointIndex(name);
	if (name.empty()) {
		return & _joints[0];
	} else if (idx == -1) {
		warning("Skeleton has no joint named '%s'!", name.c_str());
		return nullptr;
	} else {
		return & _joints[idx];
	}
}

Joint *Skeleton::getParentJoint(const Joint *j) const {
	assert(j);
	if (j->_parentIndex == -1)
		return nullptr;
	return &_joints[j->_parentIndex];
}

int Skeleton::getJointIndex(const Joint *j) const {
	int idx = j - _joints;
	assert(idx >= 0 && idx < _numJoints);
	return idx;
}

AnimationLayer* Skeleton::getLayer(int priority) const {
	assert(priority >= 0 && priority < MAX_ANIMATION_LAYERS);
	return &_animLayers[priority];
}


} // end of namespace Grim
