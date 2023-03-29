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
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_ANIMATION_TRACK_H
#define HPL_ANIMATION_TRACK_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class cAnimation;
class cNode3D;

class cAnimationTrack {
public:
	cAnimationTrack(const tString &asName, tAnimTransformFlag aTransformFlags, cAnimation *apParent);
	~cAnimationTrack();

	void ResizeKeyFrames(int alSize);

	/**
	 * Creates a new key frame. These should be added in sequential order.
	 * \param afTime the time for the key frame.
	 */
	cKeyFrame *CreateKeyFrame(float afTime);

	inline cKeyFrame *GetKeyFrame(int alIndex) { return mvKeyFrames[alIndex]; }
	inline int GetKeyFrameNum() { return (int)mvKeyFrames.size(); }

	inline tAnimTransformFlag GetTransformFlags() { return mTransformFlags; }

	/**
	 * Apply the animation to a node. The method uses Node->AddXXX() so Update matrix must be called
	 * for the transformation to be applied.
	 * \param apNode The node with it's base pose
	 * \param afTime The time at which to apply the animation
	 * \param afWeight The weight of the animation, a value from 0 to 1.
	 */
	void ApplyToNode(cNode3D *apNode, float afTime, float afWeight);

	/**
	 * Get a KeyFrame that contains an interpolated value.
	 * \param afTime The time from wihcih to create the keyframe.
	 */
	cKeyFrame GetInterpolatedKeyFrame(float afTime);

	/**
	 * Gets key frames between for a specific time.
	 * \param afTime The time
	 * \param &apKeyFrameA The frame that is equal to or before time
	 * \param &apKeyFrameB The frame that is after time.
	 * \return Weight of the different frames. 0 = 100% A, 1 = 100% B 0.5 = 50% A and 50% B
	 */
	float GetKeyFramesAtTime(float afTime, cKeyFrame **apKeyFrameA, cKeyFrame **apKeyFrameB);

	const char *GetName() { return msName.c_str(); }

	void SetNodeIndex(int alIndex) { mlNodeIdx = alIndex; }
	int GetNodeIndex() { return mlNodeIdx; }

private:
	tString msName;

	int mlNodeIdx;

	tKeyFramePtrVec mvKeyFrames;
	tAnimTransformFlag mTransformFlags;

	float mfMaxFrameTime;

	cAnimation *mpParent;
};

} // namespace hpl

#endif // HPL_ANIMATION_TRACK_H
