/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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

#ifndef TWINE_SCENE_ANIMATIONS_H
#define TWINE_SCENE_ANIMATIONS_H

#include "common/scummsys.h"
#include "twine/scene/actor.h"
#include "twine/scene/scene.h"

namespace TwinE {

class TwinEEngine;

class Animations {
private:
	TwinEEngine *_engine;
	int16 applyAnimStepRotation(int32 deltaTime, int32 keyFrameLength, int16 newAngle1, int16 lastAngle1) const;
	int16 applyAnimStepTranslation(int32 deltaTime, int32 keyFrameLength, int16 newPos, int16 lastPos) const;

	/**
	 * Verify animation at keyframe
	 * @param keyframeIdx Animation key frame index
	 * @param animData Animation data
	 * @param animTimerDataPtr Animation time data
	 */
	bool verifyAnimAtKeyframe(int32 keyframeIdx, const AnimData &animData, AnimTimerDataStruct *animTimerDataPtr);

	void copyKeyFrameToState(const KeyFrame *keyframe, BodyData &bodyData, int32 numBones) const;
	void copyStateToKeyFrame(KeyFrame *keyframe, const BodyData &bodyData) const;

	int animKeyframeBufIdx = 0;
	KeyFrame animKeyframeBuf[32];

	/** Rotation by anim and not by engine */
	int16 processRotationByAnim = 0; // processActorVar5
	/** Last rotation angle */
	int16 processLastRotationAngle = ANGLE_0; // processActorVar6

	/** Current step coordinates */
	IVec3 currentStep;

public:
	Animations(TwinEEngine *engine);

	/** Current process actor index */
	int16 currentlyProcessedActorIdx = 0;
	/** Current actor anim extra pointer */
	AnimationTypes currentActorAnimExtraPtr = AnimationTypes::kAnimNone;

	/**
	 * Set animation keyframe
	 * @param keyframIdx Animation keyframe index
	 * @param animData Animation data
	 * @param bodyData Body model data
	 * @param animTimerDataPtr Animation time data
	 */
	void setAnimAtKeyframe(int32 keyframeIdx, const AnimData &animData, BodyData &bodyData, AnimTimerDataStruct *animTimerDataPtr);

	/**
	 * Set new body animation
	 * @param keyframeIdx Animation key frame index
	 * @param animData Animation data
	 * @param bodyData Body model data
	 * @param animTimerDataPtr Animation time data
	 */
	bool setModelAnimation(int32 keyframeIdx, const AnimData &animData, BodyData &bodyData, AnimTimerDataStruct *animTimerDataPtr);

	/**
	 * Get entity anim index (This is taken from File3D entities)
	 * @param animIdx Entity animation index
	 * @param actorIdx Actor index
	 */
	int32 getBodyAnimIndex(AnimationTypes animIdx, int32 actorIdx = OWN_ACTOR_SCENE_INDEX);

	/**
	 * Stock animation - copy the next keyFrame from a different buffer
	 * @param bodyData Body model data
	 * @param animTimerDataPtr Animation time data
	 */
	void stockAnimation(const BodyData &bodyData, AnimTimerDataStruct *animTimerDataPtr);

	/**
	 * Initialize animation
	 * @param newAnim animation to init
	 * @param animType animation type
	 * @param animExtra animation actions extra data
	 * @param actorIdx actor index
	 */
	bool initAnim(AnimationTypes newAnim, AnimType animType, AnimationTypes animExtra, int32 actorIdx);

	/**
	 * Process acotr animation actions
	 * @param actorIdx Actor index
	 */
	void processAnimActions(int32 actorIdx);

	/**
	 * Process main loop actor animations
	 * @param actorIdx Actor index
	 */
	void processActorAnimations(int32 actorIdx);
};

} // namespace TwinE
#endif
