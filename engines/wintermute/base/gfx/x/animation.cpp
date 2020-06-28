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
#include "engines/wintermute/base/gfx/x/loader_x.h"
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
bool Animation::loadFromX(XFileLexer &lexer, AnimationSet *parentAnimSet) {
	lexer.advanceToNextToken(); // skip name
	lexer.advanceOnOpenBraces();

	bool ret = true;

	while (!lexer.eof()) {
		if (lexer.tokenIsIdentifier("AnimationKey")) {
			lexer.advanceToNextToken(); // skip name
			lexer.advanceOnOpenBraces();

			int keyType = readInt(lexer);
			int keyCount = readInt(lexer);

			switch (keyType) {
			case 0:
				loadRotationKeyData(lexer, keyCount);
				break;
			case 1:
				loadScaleKeyData(lexer, keyCount);
				break;
			case 2:
				loadPositionKeyData(lexer, keyCount);
				break;
			case 3:
				loadMatrixKeyData(lexer, keyCount);
				break;
			default:
				warning("Animation::loadFromX unknown key type encountered");
			}

			lexer.advanceToNextToken(); // skip closed braces

		} else if (lexer.tokenIsIdentifier("AnimationOptions")) {
			lexer.advanceToNextToken();
			lexer.advanceOnOpenBraces();

			// I think we can ignore these for the moment)
			int openClosed = readInt(lexer);
			int positionQuality = readInt(lexer);
			lexer.advanceToNextToken(); // skip closed braces
		} else if (lexer.tokenIsOfType(OPEN_BRACES)) {
			// this is a reference to a frame/bone, given as an identifier
			lexer.advanceToNextToken();
			_targetName = readString(lexer);
		} else if (lexer.reachedClosedBraces()) {
			lexer.advanceToNextToken(); // skip closed braces
			break;
		} else {
			warning("Animation::loadFromX unexpected token encounterd");
			ret = false;
			break;
		}
	}

	return ret;
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

// try to put these functions into a template?
bool Animation::loadRotationKeyData(XFileLexer &lexer, int count) {
	for (int keyIndex = 0; keyIndex < count; ++keyIndex) {
		BoneRotationKey *key = new BoneRotationKey;
		key->_time = readInt(lexer);

		int floatCount = readInt(lexer);
		assert(floatCount == 4);

		// .X file format puts the w coordinate first
		key->_rotation.w() = readFloat(lexer);
		key->_rotation.x() = readFloat(lexer);
		key->_rotation.y() = readFloat(lexer);
		// mirror z component
		key->_rotation.z() = -readFloat(lexer);

		lexer.advanceToNextToken(); // skip semicolon
		lexer.advanceToNextToken(); // skip closed braces

		_rotKeys.push_back(key);
	}

	return true;
}

bool Animation::loadScaleKeyData(XFileLexer &lexer, int count) {
	for (int keyIndex = 0; keyIndex < count; ++keyIndex) {
		BoneScaleKey *key = new BoneScaleKey;
		key->_time = readInt(lexer);

		int floatCount = readInt(lexer);
		assert(floatCount == 3);

		for (int i = 0; i < floatCount; ++i) {
			key->_scale.getData()[i] = readFloat(lexer);
		}

		lexer.advanceToNextToken(); // skip semicolon
		lexer.advanceToNextToken(); // skip closed braces

		_scaleKeys.push_back(key);
	}

	return true;
}

bool Animation::loadPositionKeyData(XFileLexer &lexer, int count) {
	for (int keyIndex = 0; keyIndex < count; ++keyIndex) {
		BonePositionKey *key = new BonePositionKey;
		key->_time = readInt(lexer);

		int floatCount = readInt(lexer);
		assert(floatCount == 3);

		for (int i = 0; i < floatCount; ++i) {
			key->_pos.getData()[i] = readFloat(lexer);
		}

		key->_pos.getData()[2] *= -1.0f;

		lexer.advanceToNextToken(); // skip semicolon
		lexer.advanceToNextToken(); // skip closed braces

		_posKeys.push_back(key);
	}

	return true;
}

bool Animation::loadMatrixKeyData(XFileLexer &lexer, int count) {
	warning("Animation::loadMatrixKeyData not implemented yet");

	for (int keyIndex = 0; keyIndex < count; ++keyIndex) {
		// this would be the time
		lexer.advanceToNextToken();
		lexer.advanceToNextToken(); // skip semicolon

		int floatCount = readInt(lexer);

		for (int i = 0; i < floatCount; ++i) {
			lexer.advanceToNextToken(); // matrix entry
			lexer.advanceToNextToken(); // semicolon
		}

		lexer.advanceToNextToken(); // skip semicolon
		lexer.advanceToNextToken(); // skip closed braces

		// TODO: Also store matrix keys
	}

	return true;
}

} // namespace Wintermute
