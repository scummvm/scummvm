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

#ifndef WINTERMUTE_ANIMATION_H
#define WINTERMUTE_ANIMATION_H

#include "engines/wintermute/base/base.h"
#include "engines/wintermute/coll_templ.h"

namespace Wintermute {

class FrameNode;
class AnimationSet;
class XFileData;
struct XAnimationKeyObject;
struct XAnimationOptionsObject;

class Animation : public BaseClass {
public:
	Animation(BaseGame *inGame);
	virtual ~Animation();

	bool load(XFileData *xobj, AnimationSet *parentAnimSet);

	bool findBone(FrameNode *rootFrame);
	bool update(int slot, uint32 localTime, float animLerpValue);

	int getFrameTime();
	uint32 getTotalTime();

	// data types
protected:
	struct BonePositionKey {
		uint32 _time;
		DXVector3 _pos;
	};

	struct BoneScaleKey {
		uint32 _time;
		DXVector3 _scale;
	};

	struct BoneRotationKey {
		uint32 _time;
		DXQuaternion _rotation;
	};

protected:
	Common::String _targetName;
	FrameNode *_targetFrame;

	BaseArray<BonePositionKey *> _posKeys;
	BaseArray<BoneRotationKey *> _rotKeys;
	BaseArray<BoneScaleKey *> _scaleKeys;

private:
	bool loadAnimationKeyData(XAnimationKeyObject *animationKey);
	bool loadAnimationOptionData(XAnimationOptionsObject *animationSet, AnimationSet *parentAnimSet);
};

} // namespace Wintermute

#endif
