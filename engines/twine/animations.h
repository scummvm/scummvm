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

#ifndef TWINE_ANIMATIONS_H
#define TWINE_ANIMATIONS_H

#include "common/scummsys.h"
#include "twine/actor.h"

namespace TwinE {

/** Total number of animations allowed in the game */
#define NUM_ANIMS 600

class TwinEEngine;

class Animations {
private:
	TwinEEngine *_engine;
	void applyAnimStepRotation(uint8 **ptr, int32 bp, int32 bx);
	int32 getAnimMode(uint8 **ptr);
	void applyAnimStep(uint8 **ptr, int32 bp, int32 bx);

public:
	Animations(TwinEEngine *engine);
	/** Table with all loaded animations */
	uint8 *animTable[NUM_ANIMS]{nullptr};
	/** Table with all loaded animations sizes */
	uint32 animSizeTable[NUM_ANIMS]{0};

	/** Rotation by anim and not by engine */
	int16 processRotationByAnim = 0; // processActorVar5
	/** Last rotation angle */
	int16 processLastRotationAngle = 0; // processActorVar6
	/** Current process actor index */
	int16 currentlyProcessedActorIdx = 0;

	/** Current step X coornidate */
	int16 currentStepX = 0;
	/** Current step Y coornidate */
	int16 currentStepY = 0;
	/** Current step Z coornidate */
	int16 currentStepZ = 0;
	/** Current actor anim extra pointer */
	uint8 *currentActorAnimExtraPtr = nullptr;

	/** Pointer to current animation keyframe */
	uint8 *keyFramePtr = nullptr;
	/** Pointer to last animation keyframe */
	uint8 *lastKeyFramePtr = nullptr;

	uint8 *animBuffer1 = nullptr;
	uint8 *animBuffer2 = nullptr;

	/**
	 * Set animation keyframe
	 * @param keyframIdx Animation keyframe index
	 * @param animPtr Pointer to animation
	 * @param bodyPtr Body model poitner
	 * @param animTimerDataPtr Animation time data
	 */
	int32 setAnimAtKeyframe(int32 keyframeIdx, uint8 *animPtr, uint8 *bodyPtr, AnimTimerDataStruct *animTimerDataPtr);

	/**
	 * Get total number of keyframes in animation
	 * @param animPtr Pointer to animation
	 */
	int32 getNumKeyframes(uint8 *animPtr);

	/**
	 * Get first keyframes in animation
	 * @param animPtr Pointer to animation
	 */
	int32 getStartKeyframe(uint8 *animPtr);

	/**
	 * Set new body animation
	 * @param animIdx Animation index
	 * @param animPtr Animation pointer
	 * @param bodyPtr Body model poitner
	 * @param animTimerDataPtr Animation time data
	 */
	int32 setModelAnimation(int32 animIdx, uint8 *animPtr, uint8 *bodyPtr, AnimTimerDataStruct *animTimerDataPtr);

	/**
	 * Get entity anim index (This is taken from File3D entities)
	 * @param animIdx Entity animation index
	 * @param actorIdx Actor index
	 */
	int32 getBodyAnimIndex(int32 animIdx, int32 actorIdx);

	/**
	 * Stock animation - copy the next keyFrame from a different buffer
	 * @param animPtr Animation pointer
	 * @param bodyPtr Body model poitner
	 * @param animTimerDataPtr Animation time data
	 */
	int32 stockAnimation(uint8 *animPtr, uint8 *bodyPtr, AnimTimerDataStruct *animTimerDataPtr);

	/**
	 * Verify animation at keyframe
	 * @param animIdx Animation index
	 * @param animPtr Animation pointer
	 * @param bodyPtr Body model poitner
	 * @param animTimerDataPtr Animation time data
	 */
	int32 verifyAnimAtKeyframe(int32 animPos, uint8 *animPtr, uint8 *bodyPtr, AnimTimerDataStruct *animTimerDataPtr);

	/**
	 * Initialize animation
	 * @param newAnim animation to init
	 * @param animType animation type
	 * @param animExtra animation actions extra data
	 * @param actorIdx actor index
	 */
	int32 initAnim(AnimationTypes newAnim, int16 animType, uint8 animExtra, int32 actorIdx);

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
