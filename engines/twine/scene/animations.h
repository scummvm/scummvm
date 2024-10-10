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

#ifndef TWINE_SCENE_ANIMATIONS_H
#define TWINE_SCENE_ANIMATIONS_H

#include "common/scummsys.h"
#include "twine/parser/anim.h"

namespace TwinE {

struct AnimTimerDataStruct;
class BodyData;
class TwinEEngine;

class Animations {
private:
	TwinEEngine *_engine;
	int16 patchInterAngle(int32 deltaTime, int32 keyFrameLength, int16 newAngle1, int16 lastAngle1) const;
	int16 patchInterStep(int32 deltaTime, int32 keyFrameLength, int16 newPos, int16 lastPos) const;

	/**
	 * Verify animation at keyframe
	 * @param keyframeIdx Animation key frame index
	 * @param animData Animation data
	 * @param animTimerDataPtr Animation time data
	 */
	bool setInterDepObjet(int32 keyframeIdx, const AnimData &animData, AnimTimerDataStruct *animTimerDataPtr);

	void copyKeyFrameToState(const KeyFrame *keyframe, BodyData &bodyData, int32 numBones) const;
	void copyStateToKeyFrame(KeyFrame *keyframe, const BodyData &bodyData) const;

	int _animKeyframeBufIdx = 0;
	KeyFrame _animKeyframeBuf[32];

	/** Rotation by anim and not by engine */
	int16 _animMasterRot = 0; // AnimMasterRot
	/** Last rotation angle */
	int16 _animStepBeta = 0; // AnimStepBeta

	/** Current step coordinates */
	IVec3 _currentStep;

public:
	Animations(TwinEEngine *engine);

	/** Current actor anim extra pointer */
	AnimationTypes _currentActorAnimExtraPtr = AnimationTypes::kAnimNone;

	/**
	 * Set animation keyframe
	 * @param keyframIdx Animation keyframe index
	 * @param animData Animation data
	 * @param bodyData Body model data
	 * @param animTimerDataPtr Animation time data
	 */
	void setAnimObjet(int32 keyframeIdx, const AnimData &animData, BodyData &bodyData, AnimTimerDataStruct *animTimerDataPtr);

	/**
	 * Set new body animation
	 * @param keyframeIdx Animation key frame index
	 * @param animData Animation data
	 * @param bodyData Body model data
	 * @param animTimerDataPtr Animation time data
	 */
	bool setInterAnimObjet(int32 keyframeIdx, const AnimData &animData, BodyData &bodyData, AnimTimerDataStruct *animTimerDataPtr);

	/**
	 * Get entity anim index (This is taken from File3D entities)
	 * @param animIdx Entity animation index
	 * @param actorIdx Actor index
	 */
	int32 searchAnim(AnimationTypes animIdx, int32 actorIdx = OWN_ACTOR_SCENE_INDEX);

	/**
	 * Stock animation - copy the next keyFrame from a different buffer
	 * @param bodyData Body model data
	 * @param animTimerDataPtr Animation time data
	 */
	void stockInterAnim(const BodyData &bodyData, AnimTimerDataStruct *animTimerDataPtr);

	/**
	 * Initialize animation
	 * @param newAnim animation to init
	 * @param animType animation type
	 * @param animExtra animation actions extra data
	 * @param actorIdx actor index
	 */
	bool initAnim(AnimationTypes newAnim, AnimType animType, AnimationTypes animExtra, int32 actorIdx); // InitAnim

	/**
	 * Process acotr animation actions
	 * @param actorIdx Actor index
	 */
	void processAnimActions(int32 actorIdx);

	/**
	 * Process main loop actor animations
	 * @param actorIdx Actor index
	 */
	void doAnim(int32 actorIdx);
};

} // namespace TwinE
#endif
