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
		_name(""), _duration(0.0f), _numBones(0), _bones(nullptr) {
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

	_duration = 1000 * data->readFloatLE();
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

	if (_operation == 3) { // Translation
		_translations = new AnimTranslation[_count];
		for (int j = 0; j < _count; j++) {
			_translations[j]._vec.readFromStream(data);
			_translations[j]._time = 1000 * data->readFloatLE();
		}
	} else if (_operation == 4) { // Rotation
		_rotations = new AnimRotation[_count];
		for (int j = 0; j < _count; j++) {
			_rotations[j]._quat.readFromStream(data);
			_rotations[j]._time = 1000 * data->readFloatLE();
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
		_skel(nullptr), _looping(false), _active(false), _paused(false),
		_fadeMode(Animation::None), _fade(1.0f), _fadeLength(0), _time(-1), _startFade(1.0f),
		_boneJoints(nullptr) {
	_anim = g_resourceloader->getAnimationEmi(anim);
	if (_anim)
		_boneJoints = new int[_anim->_numBones];
}

AnimationStateEmi::~AnimationStateEmi() {
	deactivate();
	delete[] _boneJoints;
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
		int durationMs = (int)_anim->_duration;
		if (_time >= durationMs) {
			if (_looping) {
				_time = _time % durationMs;
			} else {
				if (_fadeMode != Animation::FadeOut)
					deactivate();
			}
		}
		if (_time < 0) {
			_time = 0;
		} else {
			_time += time;
		}
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

void AnimationStateEmi::computeWeights() {
	if (_fade <= 0.0f)
		return;

	for (int bone = 0; bone < _anim->_numBones; ++bone) {
		Bone &curBone = _anim->_bones[bone];
		int jointIndex = _boneJoints[bone];
		if (jointIndex == -1)
			continue;

		AnimationLayer *layer = _skel->getLayer(curBone._priority);
		JointAnimation &jointAnim = layer->_jointAnims[jointIndex];

		if (curBone._rotations) {
			jointAnim._rotWeight += _fade;
		}
		if (curBone._translations) {
			jointAnim._transWeight += _fade;
		}
	}
}

void AnimationStateEmi::animate() {
	if (_fade <= 0.0f)
		return;

	if (_time < 0)
		return;

	for (int bone = 0; bone < _anim->_numBones; ++bone) {
		Bone &curBone = _anim->_bones[bone];
		int jointIndex = _boneJoints[bone];
		if (jointIndex == -1)
			continue;

		Joint *target = &_skel->_joints[jointIndex];
		AnimationLayer *layer = _skel->getLayer(curBone._priority);
		JointAnimation &jointAnim = layer->_jointAnims[jointIndex];

		if (curBone._rotations) {
			int keyfIdx = -1;
			Math::Quaternion quat;

			// Normalize the weight so that the sum of applied weights will equal 1.
			float normalizedRotWeight = _fade;
			if (jointAnim._rotWeight > 1.0f) {
				// Note: Division by unnormalized sum of weights.
				normalizedRotWeight = _fade / jointAnim._rotWeight;
			}

			for (int curKeyFrame = 0; curKeyFrame < curBone._count; curKeyFrame++) {
				if (curBone._rotations[curKeyFrame]._time >= _time) {
					keyfIdx = curKeyFrame;
					break;
				}
			}

			if (keyfIdx == 0) {
				quat = curBone._rotations[0]._quat;
			}
			else if (keyfIdx != -1) {
				float timeDelta = curBone._rotations[keyfIdx]._time - curBone._rotations[keyfIdx - 1]._time;
				float interpVal = (_time - curBone._rotations[keyfIdx - 1]._time) / timeDelta;

				quat = curBone._rotations[keyfIdx - 1]._quat.slerpQuat(curBone._rotations[keyfIdx]._quat, interpVal);
			}
			else {
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
			float normalizedTransWeight = _fade;
			if (jointAnim._transWeight > 1.0f) {
				// Note: Division by unnormalized sum of weights.
				normalizedTransWeight = _fade / jointAnim._transWeight;
			}

			for (int curKeyFrame = 0; curKeyFrame < curBone._count; curKeyFrame++) {
				if (curBone._translations[curKeyFrame]._time >= _time) {
					keyfIdx = curKeyFrame;
					break;
				}
			}

			if (keyfIdx == 0) {
				vec = curBone._translations[0]._vec;
			}
			else if (keyfIdx != -1) {
				float timeDelta = curBone._translations[keyfIdx]._time - curBone._translations[keyfIdx - 1]._time;
				float interpVal = (_time - curBone._translations[keyfIdx - 1]._time) / timeDelta;

				vec = curBone._translations[keyfIdx - 1]._vec +
					(curBone._translations[keyfIdx]._vec - curBone._translations[keyfIdx - 1]._vec) * interpVal;
			}
			else {
				vec = curBone._translations[curBone._count - 1]._vec;
			}

			Math::Vector3d &posFinal = jointAnim._pos;
			vec = vec - target->_relMatrix.getPosition();
			posFinal = posFinal + vec * normalizedTransWeight;
		}
	}
}

void AnimationStateEmi::play() {
	if (!_active) {
		_time = -1;
		if (_fadeMode == Animation::FadeOut)
			_fadeMode = Animation::None;
		if (_fadeMode == Animation::FadeIn || _fade > 0.f)
			activate();
	}
	_paused = false;
}

void AnimationStateEmi::stop() {
	_fadeMode = Animation::None;
	_time = -1;
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

		if (_anim) {
			for (int i = 0; i < _anim->_numBones; ++i) {
				_boneJoints[i] = skel->findJointIndex(_anim->_bones[i]._boneName);
			}
		}
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
	if (_time >= 0) {
		_time += msecs;
	} else {
		_time = msecs;
	}
}

void AnimationStateEmi::saveState(SaveGame *state) {
	state->writeBool(_looping);
	state->writeBool(_active);
	state->writeBool(_paused);
	state->writeLESint32(_time);
	state->writeFloat(_fade);
	state->writeFloat(_startFade);
	state->writeLESint32((int)_fadeMode);
	state->writeLESint32(_fadeLength);
}

void AnimationStateEmi::restoreState(SaveGame *state) {
	if (state->saveMinorVersion() >= 10) {
		_looping = state->readBool();
		bool active = state->readBool();
		_paused = state->readBool();
		if (state->saveMinorVersion() < 22) {
			_time = (uint)state->readFloat();
		} else {
			_time = state->readLESint32();
		}
		_fade = state->readFloat();
		_startFade = state->readFloat();
		_fadeMode = (Animation::FadeMode)state->readLESint32();
		_fadeLength = state->readLESint32();

		if (active)
			activate();
	}
}

} // end of namespace Grim
