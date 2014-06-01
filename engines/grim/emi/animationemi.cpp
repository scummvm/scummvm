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
#include "math/quat.h"
#include "engines/grim/resource.h"
#include "engines/grim/emi/animationemi.h"
#include "common/textconsole.h"

namespace Grim {

AnimationEmi::AnimationEmi(const Common::String &filename, Common::SeekableReadStream *data) :
		_name(""), _duration(0.0f), _numBones(0), _bones(NULL) {
	_fname = filename;
	loadAnimation(data);
}

// Use modelemi's solution for the LA-strings.
void AnimationEmi::loadAnimation(Common::SeekableReadStream *data) {
	int len = data->readUint32LE();
	char *inString = new char[len];
	data->read(inString, len);
	_name = inString;
	delete[] inString;

	char temp[4];
	data->read(temp, 4);
	_duration = 1000 * get_float(temp);
	_numBones = data->readUint32LE();

	_bones = new Bone[_numBones];
	for (int i = 0; i < _numBones; i++) {
		_bones[i].loadBinary(data);
	}
}

AnimationEmi::~AnimationEmi() {
	g_resourceloader->uncacheAnimationEmi(this);
	delete[] _bones;
}

void AnimationEmi::computeWeights(const Skeleton *skel, float weight) {
	if (weight <= 0.0f)
		return;

	for (int bone = 0; bone < _numBones; ++bone) {
		Bone &curBone = _bones[bone];
		Joint *target = skel->getJointNamed(curBone._boneName);
		if (!target) {
			continue;
		}

		AnimationLayer *layer = skel->getLayer(curBone._priority);
		int jointIndex = skel->getJointIndex(target);
		JointAnimation &jointAnim = layer->_jointAnims[jointIndex];

		if (curBone._rotations) {
			jointAnim._rotWeight += weight;
		}
		if (curBone._translations) {
			jointAnim._transWeight += weight;
		}
	}
}

void AnimationEmi::animate(const Skeleton *skel, float time, bool loop, float weight) {
	if (weight <= 0.0f)
		return;

	for (int bone = 0; bone < _numBones; ++bone) {
		Bone &curBone = _bones[bone];
		Joint *target = skel->getJointNamed(curBone._boneName);
		if (!target) {
			continue;
		}

		AnimationLayer *layer = skel->getLayer(curBone._priority);
		int jointIndex = skel->getJointIndex(target);
		JointAnimation &jointAnim = layer->_jointAnims[jointIndex];

		if (curBone._rotations) {
			int keyfIdx = -1;
			Math::Quaternion quat;

			// Normalize the weight so that the sum of applied weights will equal 1.
			float normalizedRotWeight = weight;
			if (jointAnim._rotWeight > 1.0f) {
				// Note: Division by unnormalized sum of weights.
				normalizedRotWeight = weight / jointAnim._rotWeight;
			}

			for (int curKeyFrame = 0; curKeyFrame < curBone._count; curKeyFrame++) {
				if (curBone._rotations[curKeyFrame]._time >= time) {
					keyfIdx = curKeyFrame;
					break;
				}
			}

			if (keyfIdx == 0) {
				quat = curBone._rotations[0]._quat;
			} else if (keyfIdx != -1) {
				float timeDelta = curBone._rotations[keyfIdx]._time - curBone._rotations[keyfIdx - 1]._time;
				float interpVal = (time - curBone._rotations[keyfIdx - 1]._time) / timeDelta;

				quat = curBone._rotations[keyfIdx - 1]._quat.slerpQuat(curBone._rotations[keyfIdx]._quat, interpVal);
			} else {
				quat = curBone._rotations[curBone._count - 1]._quat;
			}

			Math::Quaternion &quatFinal = jointAnim._quat;
			quat = target->_quat.inverse() * quat;
			quat = quatFinal * quat;
			quatFinal = quatFinal.slerpQuat(quat, normalizedRotWeight);
		}

		if (curBone._translations) {
			int keyfIdx = -1;
			Math::Vector3d vec;

			// Normalize the weight so that the sum of applied weights will equal 1.
			float normalizedTransWeight = weight;
			if (jointAnim._rotWeight > 1.0f) {
				// Note: Division by unnormalized sum of weights.
				normalizedTransWeight = weight / jointAnim._transWeight;
			}

			for (int curKeyFrame = 0; curKeyFrame < curBone._count; curKeyFrame++) {
				if (curBone._translations[curKeyFrame]._time >= time) {
					keyfIdx = curKeyFrame;
					break;
				}
			}
			
			if (keyfIdx == 0) {
				vec = curBone._translations[0]._vec;
			} else if (keyfIdx != -1) {
				float timeDelta = curBone._translations[keyfIdx]._time - curBone._translations[keyfIdx - 1]._time;
				float interpVal = (time - curBone._translations[keyfIdx - 1]._time) / timeDelta;

				vec = curBone._translations[keyfIdx - 1]._vec +
					(curBone._translations[keyfIdx]._vec - curBone._translations[keyfIdx - 1]._vec) * interpVal;
			} else {
				vec = curBone._translations[curBone._count - 1]._vec;
			}

			Math::Vector3d &posFinal = jointAnim._pos;
			vec = vec - target->_relMatrix.getPosition();
			posFinal = posFinal + vec * normalizedTransWeight;
		}
	}
}

void Bone::loadBinary(Common::SeekableReadStream *data) {
	uint32 len = data->readUint32LE();
	char *inString = new char[len];
	data->read(inString, len);
	_boneName = inString;
	delete[] inString;
	_operation = data->readUint32LE();
	_priority = data->readUint32LE();
	_c = data->readUint32LE();
	_count = data->readUint32LE();

	char temp[4];
	if (_operation == 3) { // Translation
		_translations = new AnimTranslation[_count];
		for (int j = 0; j < _count; j++) {
			_translations[j]._vec.readFromStream(data);
			data->read(temp, 4);
			_translations[j]._time = 1000 * get_float(temp);
		}
	} else if (_operation == 4) { // Rotation
		_rotations = new AnimRotation[_count];
		for (int j = 0; j < _count; j++) {
			_rotations[j]._quat.readFromStream(data);
			data->read(temp, 4);
			_rotations[j]._time = 1000 * get_float(temp);
		}
	} else {
		error("Unknown animation-operation %d", _operation);
	}
}

Bone::~Bone() {
	if (_operation == 3) {
		delete[] _translations;
	} else if (_operation == 4) {
		delete[] _rotations;
	}
}

AnimationStateEmi::AnimationStateEmi(const Common::String &anim) :
		_skel(NULL), _looping(false), _active(false),
		_fadeMode(Animation::None), _fade(1.0f), _fadeLength(0), _time(0.0f), _startFade(1.0f) {
	_anim = g_resourceloader->getAnimationEmi(anim);
}

AnimationStateEmi::~AnimationStateEmi() {
	deactivate();
}

void AnimationStateEmi::activate() {
	if (!_active) {
		_active = true;
		if (_skel)
			_skel->addAnimation(this);
	}
}

void AnimationStateEmi::deactivate() {
	if (_active) {
		_active = false;
		if (_skel)
			_skel->removeAnimation(this);
	}
}

void AnimationStateEmi::update(uint time) {
	if (!_active)
		return;

	if (!_anim) {
		deactivate();
		return;
	}

	if (!_paused) {
		if (_time > _anim->_duration) {
			if (_looping) {
				_time = 0.0f;
			} else {
				if (_fadeMode != Animation::FadeOut)
					deactivate();
			}
		}
		_time += time;
	}

	if (_fadeMode != Animation::None) {
		if (_fadeMode == Animation::FadeIn) {
			_fade += (float)time * (1.0f - _startFade) / _fadeLength;
			if (_fade >= 1.f) {
				_fade = 1.f;
				_fadeMode = Animation::None;
			}
		} else {
			_fade -= (float)time * _startFade / _fadeLength;
			if (_fade <= 0.f) {
				_fade = 0.f;
				// Don't reset the _fadeMode here. This way if fadeOut() was called
				// on a looping chore its keyframe animations will remain faded out
				// when it calls play() again.
				deactivate();
				return;
			}
		}
	}
}

void AnimationStateEmi::play() {
	if (!_active) {
		_time = 0.f;
		if (_fadeMode == Animation::FadeOut)
			_fadeMode = Animation::None;
		if (_fadeMode == Animation::FadeIn || _fade > 0.f)
			activate();
	}
	_paused = false;
}

void AnimationStateEmi::stop() {
	_fadeMode = Animation::None;
	_time = 0.f;
	deactivate();
}

void AnimationStateEmi::setPaused(bool paused) {
	_paused = paused;
}

void AnimationStateEmi::setLooping(bool loop) {
	_looping = loop;
}

void AnimationStateEmi::setSkeleton(Skeleton *skel) {
	if (skel != _skel) {
		if (_skel)
			_skel->removeAnimation(this);
		_skel = skel;
		if (_active)
			skel->addAnimation(this);
	}
}

void AnimationStateEmi::fade(Animation::FadeMode mode, int fadeLength) {
	if (mode == Animation::None) {
		_fade = 1.f;
	} else if (_fadeMode != Animation::FadeOut && mode == Animation::FadeIn) {
		_fade = 0.f;
	}
	_startFade = _fade;
	_fadeMode = mode;
	_fadeLength = fadeLength;
}

void AnimationStateEmi::advance(uint msecs) {
	_time += msecs;
}

} // end of namespace Grim
