/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/gfx/xanimation.h"
#include "engines/wintermute/base/gfx/xanimation_set.h"
#include "engines/wintermute/base/gfx/xframe_node.h"
#include "engines/wintermute/base/gfx/xmodel.h"
#include "engines/wintermute/base/gfx/xfile_loader.h"
#include "engines/wintermute/dcgf.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
Animation::Animation(BaseGame *inGame) : BaseClass(inGame) {
	_targetFrame = nullptr;
}

//////////////////////////////////////////////////////////////////////////
Animation::~Animation() {
	for (uint32 i = 0; i < _posKeys.size(); i++) {
		delete _posKeys[i];
	}
	_posKeys.clear();

	for (uint32 i = 0; i < _rotKeys.size(); i++) {
		delete _rotKeys[i];
	}
	_rotKeys.clear();

	for (uint32 i = 0; i < _scaleKeys.size(); i++) {
		delete _scaleKeys[i];
	}
	_scaleKeys.clear();
}

//////////////////////////////////////////////////////////////////////////
bool Animation::findBone(FrameNode *rootFrame) {
	if (_targetName != "") {
		_targetFrame = rootFrame->findFrame(_targetName.c_str());
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Animation::load(XFileData *xobj, AnimationSet *parentAnimSet) {
	bool result;
	XClassType objectType;

	if (xobj->isReference()) {
		// The original data is found
		result = xobj->getType(objectType);
		if (!result)
			return result;

		// The object must be a frame
		if (objectType == kXClassFrame) {
			// The frame is found, get its name
			// The name will be used later by the findBone function to get
			// a pointer to the target frame
			if (_targetFrame) {
				BaseEngine::LOG(0, "Animation frame name reference duplicated");
				return false;
			}

			// get name
			result = XModel::loadName(_targetName, xobj);
			if (!result) {
				BaseEngine::LOG(0, "Error retrieving frame name while loading animation");
				return false;
			}
		}
	} else {
		// a data object is found, get its type
		result = xobj->getType(objectType);
		if (!result)
			return false;

		if (objectType == kXClassAnimationKey) {
			// an animation key is found, load the data
			XAnimationKeyObject *animationKey = xobj->getXAnimationKeyObject();
			if (!animationKey)
				return false;
			result = loadAnimationKeyData(animationKey);
			if (!result)
				return false;
		} else if (objectType == kXClassAnimationOptions) {
			XAnimationOptionsObject *animationOptions = xobj->getXAnimationOptionsObject();
			if (!animationOptions)
				return false;
			result = loadAnimationOptionData(animationOptions, parentAnimSet);
			if (!result)
				return false;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Animation::loadAnimationOptionData(XAnimationOptionsObject *animationOptionData, AnimationSet *parentAnimSet) {
	if (animationOptionData->_openclosed && parentAnimSet)
		parentAnimSet->_looping = true;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Animation::loadAnimationKeyData(XAnimationKeyObject *animationKey) {
	// get the type and count of the key
	uint32 keyType = animationKey->_keyType;
	uint32 numKeys = animationKey->_numKeys;

	if (keyType == 0) { // rotation key
		if (_rotKeys.size() != 0) {
			BaseEngine::LOG(0, "Rotation key duplicated");
			return false;
		}

		for (uint32 key = 0; key < numKeys; key++) {
			const XTimedFloatKeys *fileRotKey = &animationKey->_keys[key];
			assert(fileRotKey->_numTfkeys == 4);

			BoneRotationKey *rotKey = new BoneRotationKey;
			rotKey->_time         = fileRotKey->_time;
			// NOTE x files are w x y z and QUATERNIONS are x y z w
			rotKey->_rotation.w() = fileRotKey->_tfkeys[0];
			rotKey->_rotation.x() = fileRotKey->_tfkeys[1];
			rotKey->_rotation.y() = fileRotKey->_tfkeys[2];
			// mirror z component
			rotKey->_rotation.z() = -fileRotKey->_tfkeys[3];

			_rotKeys.push_back(rotKey);
		}
	} else if (keyType == 1) { // scale key
		if (_scaleKeys.size() != 0) {
			BaseEngine::LOG(0, "Scale key duplicated");
			return false;
		}

		for (uint32 key = 0; key < numKeys; key++) {
			const XTimedFloatKeys *fileScaleKey = &animationKey->_keys[key];
			assert(fileScaleKey->_numTfkeys == 3);

			BoneScaleKey *scaleKey = new BoneScaleKey;
			scaleKey->_time  = fileScaleKey->_time;
			for (uint i = 0; i < fileScaleKey->_numTfkeys; ++i) {
				scaleKey->_scale.getData()[i] = fileScaleKey->_tfkeys[i];
			}

			_scaleKeys.push_back(scaleKey);
		}
	} else if (keyType == 2) { // position key
		if (_posKeys.size() != 0) {
			BaseEngine::LOG(0, "Position key duplicated");
			return false;
		}

		for (uint32 key = 0; key < numKeys; key++) {
			const XTimedFloatKeys *filePosKey = &animationKey->_keys[key];
			assert(filePosKey->_numTfkeys == 3);

			BonePositionKey *posKey = new BonePositionKey;
			posKey->_time = filePosKey->_time;
			for (uint i = 0; i < filePosKey->_numTfkeys; ++i) {
				posKey->_pos.getData()[i] = filePosKey->_tfkeys[i];
			}

			// mirror Z
			posKey->_pos.getData()[2] *= -1.0f;

			_posKeys.push_back(posKey);
		}
	} else if (keyType == 4) { // matrix key
		if (_rotKeys.size() != 0 || _scaleKeys.size() != 0 || _posKeys.size() != 0) {
			BaseEngine::LOG(0, "Matrix key duplicated");
			return false;
		}

		for (uint32 key = 0; key < numKeys; key++) {
			const XTimedFloatKeys *fileMatrixKey = &animationKey->_keys[key];
			uint32 time = fileMatrixKey->_time;
			assert(fileMatrixKey->_numTfkeys == 16);

			Math::Matrix4 keyData;
			for (int r = 0; r < 4; ++r) {
				for (int c = 0; c < 4; ++c) {
					keyData(c, r) = fileMatrixKey->_tfkeys[r * 4 + c];
				}
			}

			// mirror at orign
			keyData(2, 3) *= -1.0f;

			// mirror base vectors
			keyData(2, 0) *= -1.0f;
			keyData(2, 1) *= -1.0f;

			// change handedness
			keyData(0, 2) *= -1.0f;
			keyData(1, 2) *= -1.0f;

			Math::Vector3d translation = keyData.getPosition();

			Math::Vector3d scale;
			scale.x() = keyData(0, 0) * keyData(0, 0) + keyData(1, 0) * keyData(1, 0) + keyData(2, 0) * keyData(2, 0);
			scale.x() = sqrtf(scale.x());
			scale.y() = keyData(0, 1) * keyData(0, 1) + keyData(1, 1) * keyData(1, 1) + keyData(2, 1) * keyData(2, 1);
			scale.y() = sqrtf(scale.y());
			scale.z() = keyData(0, 2) * keyData(0, 2) + keyData(1, 2) * keyData(1, 2) + keyData(2, 2) * keyData(2, 2);
			scale.z() = sqrtf(scale.z());

			Math::Quaternion rotation;
			rotation.fromMatrix(keyData.getRotation());

			BonePositionKey *positionKey = new BonePositionKey;
			BoneScaleKey *scaleKey = new BoneScaleKey;
			BoneRotationKey *rotationKey = new BoneRotationKey;

			positionKey->_time = time;
			scaleKey->_time = time;
			rotationKey->_time = time;

			positionKey->_pos = translation;
			scaleKey->_scale = scale;
			rotationKey->_rotation = rotation;

			_posKeys.push_back(positionKey);
			_scaleKeys.push_back(scaleKey);
			_rotKeys.push_back(rotationKey);
		}
	} else {
		// the type is unknown, report the error
		BaseEngine::LOG(0, "Unexpected animation key type (%d)", keyType);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Animation::update(int slot, uint32 localTime, float animLerpValue) {
	// no target frame = no animation keys
	if (!_targetFrame) {
		return true;
	}

	Math::Vector3d resultPos(0.0f, 0.0f, 0.0f);
	Math::Vector3d resultScale(1.0f, 1.0f, 1.0f);
	Math::Quaternion resultRot(0.0f, 0.0f, 0.0f, 1.0f);

	int keyIndex1, keyIndex2;
	uint32 time1, time2;
	float lerpValue;

	bool animate = false;

	// scale keys
	if (_scaleKeys.size() > 0) {
		keyIndex1 = keyIndex2 = 0;

		// get the two keys between which the time is currently in
		for (uint32 key = 0; key < _scaleKeys.size(); key++) {
			if (_scaleKeys[key]->_time > localTime) {
				keyIndex2 = key;

				if (key > 0) {
					keyIndex1 = key - 1;
				} else { // when ikey == 0, then dwp2 == 0
					keyIndex1 = key;
				}

				break;
			}
		}

		time1 = _scaleKeys[keyIndex1]->_time;
		time2 = _scaleKeys[keyIndex2]->_time;

		// get the lerp value
		if ((time2 - time1) == 0) {
			lerpValue = 0;
		} else {
			lerpValue = float(localTime - time1) / float(time2 - time1);
		}

		resultScale = (1 - lerpValue) * _scaleKeys[keyIndex1]->_scale + lerpValue * _scaleKeys[keyIndex2]->_scale;

		animate = true;
	}

	// rotation keys
	if (_rotKeys.size() > 0) {
		keyIndex1 = keyIndex2 = 0;

		// get the two keys surrounding the current time value
		for (uint32 key = 0; key < _rotKeys.size(); key++) {
			if (_rotKeys[key]->_time > localTime) {
				keyIndex2 = key;
				if (key > 0) {
					keyIndex1 = key - 1;
				} else { // when ikey == 0, then dwp2 == 0
					keyIndex1 = key;
				}

				break;
			}
		}
		time1 = _rotKeys[keyIndex1]->_time;
		time2 = _rotKeys[keyIndex2]->_time;

		// get the lerp value
		if ((time2 - time1) == 0) {
			lerpValue = 0;
		} else {
			lerpValue = float(localTime - time1) / float(time2 - time1);
		}

		resultRot = _rotKeys[keyIndex1]->_rotation.slerpQuat(_rotKeys[keyIndex2]->_rotation, lerpValue);

		animate = true;
	}

	// position keys
	if (_posKeys.size() > 0) {
		keyIndex1 = keyIndex2 = 0;

		// get the two keys surrounding the time value
		for (uint32 key = 0; key < _posKeys.size(); key++) {
			if (_posKeys[key]->_time > localTime) {
				keyIndex2 = key;
				if (key > 0) {
					keyIndex1 = key - 1;
				} else { // when ikey == 0, then dwp2 == 0
					keyIndex1 = key;
				}

				break;
			}
		}
		time1 = _posKeys[keyIndex1]->_time;
		time2 = _posKeys[keyIndex2]->_time;

		// get the lerp value
		if (time2 - time1 == 0)
			lerpValue = 0;
		else
			lerpValue = float(localTime - time1) / float(time2 - time1);

		resultPos = (1 - lerpValue) * _posKeys[keyIndex1]->_pos + lerpValue * _posKeys[keyIndex2]->_pos;

		animate = true;
	}

	if (animate) {
		_targetFrame->setTransformation(slot, resultPos, resultScale, resultRot, animLerpValue);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
int Animation::getFrameTime() {
	uint32 frameTime = 0;
	uint32 prevTime;

	// get the shortest frame time
	prevTime = 0;
	for (uint32 i = 0; i < _rotKeys.size(); i++) {
		if (frameTime == 0 || _rotKeys[i]->_time - prevTime < frameTime)
			frameTime = _rotKeys[i]->_time - prevTime;

		prevTime = _rotKeys[i]->_time;
	}

	prevTime = 0;
	for (uint32 i = 0; i < _posKeys.size(); i++) {
		if (frameTime == 0 || _posKeys[i]->_time - prevTime < frameTime)
			frameTime = _posKeys[i]->_time - prevTime;

		prevTime = _posKeys[i]->_time;
	}

	prevTime = 0;
	for (uint32 i = 0; i < _scaleKeys.size(); i++) {
		if (frameTime == 0 || _scaleKeys[i]->_time - prevTime < frameTime)
			frameTime = _scaleKeys[i]->_time - prevTime;

		prevTime = _scaleKeys[i]->_time;
	}

	return frameTime;
}

//////////////////////////////////////////////////////////////////////////
uint32 Animation::getTotalTime() {
	uint32 totalTime = 0;
	if (_rotKeys.size() > 0) {
		totalTime = MAX(totalTime, _rotKeys[_rotKeys.size() - 1]->_time);
	}

	if (_posKeys.size() > 0) {
		totalTime = MAX(totalTime, _posKeys[_posKeys.size() - 1]->_time);
	}

	if (_scaleKeys.size() > 0) {
		totalTime = MAX(totalTime, _scaleKeys[_scaleKeys.size() - 1]->_time);
	}

	return totalTime;
}

} // namespace Wintermute
