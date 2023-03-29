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

#include "hpl1/engine/graphics/AnimationTrack.h"

#include "hpl1/engine/graphics/Animation.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/scene/Node3D.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cAnimationTrack::cAnimationTrack(const tString &asName, tAnimTransformFlag aTransformFlags,
								 cAnimation *apParent) {
	msName = asName;
	mTransformFlags = aTransformFlags;
	mpParent = apParent;

	mfMaxFrameTime = 0;

	mlNodeIdx = -1;
}

//-----------------------------------------------------------------------

cAnimationTrack::~cAnimationTrack() {
	STLDeleteAll(mvKeyFrames);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cAnimationTrack::ResizeKeyFrames(int alSize) {
	mvKeyFrames.reserve(alSize);
}

//-----------------------------------------------------------------------

cKeyFrame *cAnimationTrack::CreateKeyFrame(float afTime) {
	cKeyFrame *pFrame = hplNew(cKeyFrame, ());
	pFrame->time = afTime;

	// Check so that this is the first
	if (afTime > mfMaxFrameTime || mvKeyFrames.empty()) {
		mvKeyFrames.push_back(pFrame);
		mfMaxFrameTime = afTime;
	} else {
		tKeyFramePtrVecIt it = mvKeyFrames.begin();
		for (; it != mvKeyFrames.end(); it++) {
			if (afTime < (*it)->time) {
				break;
			}
		}
		mvKeyFrames.insert(it, pFrame);
	}

	return pFrame;
}

//-----------------------------------------------------------------------

void cAnimationTrack::ApplyToNode(cNode3D *apNode, float afTime, float afWeight) {
	cKeyFrame Frame = GetInterpolatedKeyFrame(afTime);

	// Scale
	// Skip this for now...
	/*cVector3f vOne(1,1,1);
	cVector3f vScale = (Frame.scale - vOne)*afWeight + vOne;
	apNode->AddScale(vScale);*/

	// Rotation
	cQuaternion qRot = cMath::QuaternionSlerp(afWeight, cQuaternion::Identity, Frame.rotation, true);
	apNode->AddRotation(qRot);

	// Translation
	cVector3f vTrans = Frame.trans * afWeight;
	apNode->AddTranslation(vTrans);
}

//-----------------------------------------------------------------------

cKeyFrame cAnimationTrack::GetInterpolatedKeyFrame(float afTime) {
	cKeyFrame ResultKeyFrame;
	ResultKeyFrame.time = afTime;

	cKeyFrame *pKeyFrameA = NULL;
	cKeyFrame *pKeyFrameB = NULL;

	float fT = GetKeyFramesAtTime(afTime, &pKeyFrameA, &pKeyFrameB);

	if (fT == 0.0f) {
		ResultKeyFrame.rotation = pKeyFrameA->rotation;
		ResultKeyFrame.scale = pKeyFrameA->scale;
		ResultKeyFrame.trans = pKeyFrameA->trans;
	} else {
		// Do a linear interpolation
		// This should include spline stuff later on.

		ResultKeyFrame.rotation = cMath::QuaternionSlerp(fT, pKeyFrameA->rotation,
														 pKeyFrameB->rotation, true);

		ResultKeyFrame.scale = pKeyFrameA->scale * (1 - fT) + pKeyFrameB->scale * fT;
		ResultKeyFrame.trans = pKeyFrameA->trans * (1 - fT) + pKeyFrameB->trans * fT;
	}

	return ResultKeyFrame;
}

//-----------------------------------------------------------------------

float cAnimationTrack::GetKeyFramesAtTime(float afTime, cKeyFrame **apKeyFrameA, cKeyFrame **apKeyFrameB) {
	float fTotalAnimLength = mpParent->GetLength();

	// Wrap time
	// Not sure it is a good idea to clamp the length.
	// But wrapping screws loop mode up.
	// Wrap(..., totalLength + kEpislon), migh work though.
	afTime = cMath::Clamp(afTime, 0, fTotalAnimLength);

	// If longer than max time return last frame and first
	if (afTime >= mfMaxFrameTime) {
		*apKeyFrameA = mvKeyFrames[mvKeyFrames.size() - 1];
		*apKeyFrameB = mvKeyFrames[0];

		// Get T between end to start again. (the last frame doesn't mean the anim is over.
		//  In that case wrap to the first frame).
		// float fDeltaT = fTotalAnimLength - (*apKeyFrameA)->time;

		// If animation time is >= max time might as well just return the last frame.
		// Not sure if this is good for some looping anims, in that case check the code.
		return 0.0f; //(afTime - (*apKeyFrameA)->time) / fDeltaT;
	}

	// Get the number of frames
	const int lSize = (int)mvKeyFrames.size();

	// Find the second frame.
	int lIdxB = -1;
	for (int i = 0; i < lSize; i++) {
		if (afTime <= mvKeyFrames[i]->time) {
			lIdxB = i;
			break;
		}
	}

	// If first frame was found, the lowest time is not 0.
	// If so return the first frame only.
	if (lIdxB == 0) {
		*apKeyFrameA = mvKeyFrames[0];
		*apKeyFrameB = mvKeyFrames[0];
		return 0.0f;
	}

	// Get the frames
	*apKeyFrameA = mvKeyFrames[lIdxB - 1];
	*apKeyFrameB = mvKeyFrames[lIdxB];

	float fDeltaT = (*apKeyFrameB)->time - (*apKeyFrameA)->time;

	return (afTime - (*apKeyFrameA)->time) / fDeltaT;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
} // namespace hpl
