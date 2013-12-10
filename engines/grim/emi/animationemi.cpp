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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/stream.h"
#include "math/vector3d.h"
#include "math/quat.h"
#include "engines/grim/emi/animationemi.h"
#include "common/textconsole.h"

namespace Grim {

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
	delete[] _bones;
}

void AnimationEmi::reset() {
	_time = 0.0f;
}

void AnimationEmi::animate(const Skeleton *skel, float delta) {
	_time += delta;
	if (_time > _duration) {
		reset();
	}

	for (int bone = 0; bone < _numBones; ++bone) {
		Bone &curBone = _bones[bone];
		curBone._target = skel->getJointNamed(curBone._boneName);

		Math::Matrix4 &relFinal = curBone._target->_finalMatrix;
		Math::Quaternion &quatFinal = curBone._target->_finalQuat;

		if (curBone._rotations) {
			int keyfIdx = 0;
			Math::Quaternion quat;
			Math::Vector3d relPos = relFinal.getPosition();

			for (int curKeyFrame = 0; curKeyFrame < curBone._count; curKeyFrame++) {
				if (curBone._rotations[curKeyFrame]._time >= _time) {
					keyfIdx = curKeyFrame;
					break;
				}
			}

			if (keyfIdx == 0) {
				quat = curBone._rotations[keyfIdx]._quat;
			} else if (keyfIdx == curBone._count - 1) {
				quat = curBone._rotations[keyfIdx - 1]._quat;
			} else {
				float timeDelta = curBone._rotations[keyfIdx - 1]._time - curBone._rotations[keyfIdx]._time;
				float interpVal = (_time - curBone._rotations[keyfIdx]._time) / timeDelta;

				// Might be the other way around (keyfIdx - 1 slerped against keyfIdx)
				quat = curBone._rotations[keyfIdx]._quat.slerpQuat(curBone._rotations[keyfIdx - 1]._quat, interpVal);
			}
			quat.toMatrix(relFinal);
			quatFinal = quat;
			relFinal.setPosition(relPos);
		}

		if (curBone._translations) {
			int keyfIdx = 0;
			Math::Vector3d vec;

			for (int curKeyFrame = 0; curKeyFrame < curBone._count; curKeyFrame++) {
				if (curBone._translations[curKeyFrame]._time >= _time) {
					keyfIdx = curKeyFrame;
					break;
				}
			}

			if (keyfIdx == 0) {
				vec = curBone._translations[keyfIdx]._vec;
			} else if (keyfIdx == curBone._count - 1) {
				vec = curBone._translations[keyfIdx - 1]._vec;
			} else {
				float timeDelta = curBone._translations[keyfIdx]._time - curBone._translations[keyfIdx - 1]._time;
				float interpVal = (_time - curBone._translations[keyfIdx]._time) / timeDelta;

				vec.x() = curBone._translations[keyfIdx - 1]._vec.x() +
						  (curBone._translations[keyfIdx]._vec.x() - curBone._translations[keyfIdx - 1]._vec.x()) * interpVal;

				vec.y() = curBone._translations[keyfIdx - 1]._vec.y() +
						  (curBone._translations[keyfIdx]._vec.y() - curBone._translations[keyfIdx - 1]._vec.y()) * interpVal;

				vec.z() = curBone._translations[keyfIdx - 1]._vec.z() +
						  (curBone._translations[keyfIdx]._vec.z() - curBone._translations[keyfIdx - 1]._vec.z()) * interpVal;
			}
			relFinal.setPosition(vec);
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
	_b = data->readUint32LE();
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

} // end of namespace Grim
