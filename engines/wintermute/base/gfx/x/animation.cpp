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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/x/animation.h"
#include "engines/wintermute/base/gfx/x/animation_set.h"
#include "engines/wintermute/base/gfx/x/frame_node.h"
#include "engines/wintermute/base/gfx/x/modelx.h"
#include "engines/wintermute/dcgf.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
Animation::Animation(BaseGame *inGame) : BaseClass(inGame), _targetFrame(nullptr) {
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
bool Animation::loadFromX(byte *buffer, AnimationSet *parentAnimSet) {
	bool res;
	//	GUID ObjectType;

	//	// Query the child for it's FileDataReference
	//	if(XObj->IsReference())
	//	{
	//		// The original data is found
	//		res = XObj->GetType(&ObjectType);
	//		if(FAILED(res))
	//		{
	//			_gameRef->LOG(res, "Couldn't retrieve object type while loading animation");
	//			return res;
	//		}

	//		// The object must be a frame
	//		if(ObjectType == TID_D3DRMFrame) {
	//			// The frame is found, get its name
	//			// The name will be used later by the FindBone function to get
	//			// a pointer to the target frame
	//			if(_targetFrame != NULL) {
	//				_gameRef->LOG(0, "Animation frame name reference duplicated");
	//				return E_FAIL;
	//			}

	//			// get name
	//			res = CXModel::LoadName(&_targetName, XObj);
	//			if(FAILED(res)) {
	//				_gameRef->LOG(res, "Error retrieving frame name while loading animation");
	//				return res;
	//			}
	//		}
	//	}
	//	else {
	//		// a data object is found, get its type
	//		res = XObj->GetType(&ObjectType);
	//		if (FAILED(res)) {
	//			_gameRef->LOG(res, "Couldn't retrieve object type");
	//			return res;
	//		}

	//		if(ObjectType == TID_D3DRMAnimationKey) {
	//			// an animation key is found, load the data
	//			uint32 Size;
	//			BYTE* Buffer;
	//			res = XObj->Lock(&Size, (LPCVOID*)&Buffer);
	//			if(FAILED(res)) {
	//				_gameRef->LOG(res, "Error retrieving data");
	//				return res;
	//			}

	//			if(FAILED(res = LoadAnimationKeyData(Buffer))) {
	//				XObj->Unlock();
	//				return res;
	//			}
	//			XObj->Unlock();
	//		}
	//		else if(ObjectType == TID_D3DRMAnimationOptions) {
	//			uint32 Size;
	//			byte* buffer;
	//			res = XObj->Lock(&Size, (LPCVOID *)&Buffer);
	//			if(FAILED(res)) {
	//				_gameRef->LOG(res, "Error retrieving data");
	//				return res;
	//			}

	//			LoadAnimationOptionData(Buffer, ParentAnimSet);
	//			XObj->Unlock();
	//		}
	//	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Animation::loadAnimationOptionData(byte *buffer, AnimationSet *parentAnimSet) {
	// get the type and count of the key
	uint32 openClosed = ((uint32 *)buffer)[0];
	uint32 positionQuality = ((uint32 *)buffer)[1];

	if (openClosed && parentAnimSet)
		parentAnimSet->_looping = true;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Animation::loadAnimationKeyData(byte *buffer) {
	// define datatypes needed by the load function
	struct tmpPOSITIONKEY {
		uint32 dwTime;
		uint32 dwFloatsCount;
		Math::Vector3d Pos;
	} * FilePosKey;

	struct tmpSCALEKEY {
		uint32 dwTime;
		uint32 dwFloatsCount;
		Math::Vector3d Scale;
	} * FileScaleKey;

	struct tmpROTATIONKEY {
		uint32 dwTime;
		uint32 dwFloatsCount;
		//NOTE x files are w x y z and QUATERNIONS are x y z w
		float w;
		float x;
		float y;
		float z;
	} * fileRotKey;

	struct tmpMATRIXKEY {
		uint32 dwTime;
		uint32 dwFloatsCount;
		Math::Matrix4 Mat;
	} * FileMatrixKey;

	// get the type and count of the key
	uint32 keyType = ((uint32 *)buffer)[0];
	uint32 numKeys = ((uint32 *)buffer)[1];

	if (keyType == 0 /*rotation key*/) {
		if (_rotKeys.size() != 0) {
			_gameRef->LOG(0, "Rotation key duplicated");
			return false;
		}

		//NOTE x files are w x y z and QUATERNIONS are x y z w

		fileRotKey = (tmpROTATIONKEY *)(buffer + (sizeof(uint32) * 2));
		for (uint32 Key = 0; Key < numKeys; Key++) {
			BoneRotationKey *RotKey = new BoneRotationKey;
			RotKey->_time = fileRotKey->dwTime;
			RotKey->_rotation.x() = fileRotKey->x;
			RotKey->_rotation.y() = fileRotKey->y;
			RotKey->_rotation.z() = fileRotKey->z;
			RotKey->_rotation.w() = fileRotKey->w;

			_rotKeys.add(RotKey);

			fileRotKey++;
		}
	}

	else if (keyType == 1 /*scale key*/) {
		if (_scaleKeys.size() != 0) {
			_gameRef->LOG(0, "Scale key duplicated");
			return false;
		}

		FileScaleKey = (tmpSCALEKEY *)(buffer + (sizeof(uint32) * 2));
		for (uint32 Key = 0; Key < numKeys; Key++) {
			BoneScaleKey *ScaleKey = new BoneScaleKey;
			ScaleKey->_time = FileScaleKey->dwTime;
			ScaleKey->_scale = FileScaleKey->Scale;

			_scaleKeys.add(ScaleKey);

			FileScaleKey++;
		}
	}

	else if (keyType == 2 /*position key*/) {
		if (_posKeys.size() != 0) {
			_gameRef->LOG(0, "Position key duplicated");
			return false;
		}

		FilePosKey = (tmpPOSITIONKEY *)(buffer + (sizeof(uint32) * 2));
		for (uint32 Key = 0; Key < numKeys; Key++) {
			BonePositionKey *PosKey = new BonePositionKey;
			PosKey->_time = FilePosKey->dwTime;
			PosKey->_pos = FilePosKey->Pos;

			_posKeys.add(PosKey);

			FilePosKey++;
		}
	}

	else if (keyType == 4 /*matrix key*/) {
		if (_rotKeys.size() != 0 || _scaleKeys.size() != 0 || _posKeys.size() != 0) {
			_gameRef->LOG(0, "Matrix key duplicated");
			return false;
		}

		Math::Quaternion QRot;
		Math::Vector3d TransVec;
		Math::Vector3d ScaleVec;

		FileMatrixKey = (tmpMATRIXKEY *)(buffer + (sizeof(uint32) * 2));

		for (uint32 Key = 0; Key < numKeys; Key++) {
			// we always convert matrix keys to T-R-S
			//			C3DUtils::DecomposeMatrixSimple(&FileMatrixKey->Mat, &TransVec, &ScaleVec, &QRot);

			BonePositionKey *PosKey = new BonePositionKey;
			PosKey->_time = FileMatrixKey->dwTime;
			PosKey->_pos = TransVec;
			_posKeys.add(PosKey);

			BoneScaleKey *ScaleKey = new BoneScaleKey;
			ScaleKey->_time = FileMatrixKey->dwTime;
			ScaleKey->_scale = ScaleVec;
			_scaleKeys.add(ScaleKey);

			BoneRotationKey *RotKey = new BoneRotationKey;
			RotKey->_time = FileMatrixKey->dwTime;
			RotKey->_rotation = QRot;

			RotKey->_rotation.x() = -RotKey->_rotation.x();
			RotKey->_rotation.y() = -RotKey->_rotation.y();
			RotKey->_rotation.z() = -RotKey->_rotation.z();
			_rotKeys.add(RotKey);

			FileMatrixKey++;
		}
	} else {
		// the type is unknown, report the error
		_gameRef->LOG(0, "Unexpected animation key type (%d)", keyType);
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

		//apply spherical lerp function
		Math::Quaternion q1, q2;

		q1.x() = -_rotKeys[keyIndex1]->_rotation.x();
		q1.y() = -_rotKeys[keyIndex1]->_rotation.y();
		q1.z() = -_rotKeys[keyIndex1]->_rotation.z();
		q1.w() = _rotKeys[keyIndex1]->_rotation.w();

		q2.x() = -_rotKeys[keyIndex2]->_rotation.x();
		q2.y() = -_rotKeys[keyIndex2]->_rotation.y();
		q2.z() = -_rotKeys[keyIndex2]->_rotation.z();
		q2.w() = _rotKeys[keyIndex2]->_rotation.w();

		resultRot = q1.slerpQuat(q2, lerpValue);

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
