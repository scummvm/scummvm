/** @file animations.h
	@brief
	This file contains 3D actors animations routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include "sys.h"
#include "actor.h"

/** Total number of animations allowed in the game */
#define NUM_ANIMS 600

enum AnimationTypes {
	kStanding			= 0,
	kForward			= 1,
	kBackward			= 2,
	kTurnLeft			= 3,
	kTurnRight			= 4,
	kHit				= 5,
	kBigHit				= 6,
	kFall				= 7,
	kLanding			= 8,
	kLandingHit			= 9,
	kLandDeath			= 10,
	kAction				= 11,
	kClimbLadder		= 12,
	kTopLadder			= 13,
	kJump				= 14,
	kThrowBall			= 15,
	kHide				= 16,
	kKick				= 17,
	kRightPunch			= 18,
	kLeftPunch			= 19,
	kFoundItem			= 20,
	kDrawn				= 21,
	kHit2				= 22,
	kSabreAttack		= 23
};


/** Table with all loaded animations */
uint8* animTable[NUM_ANIMS];
/** Table with all loaded animations sizes */
uint32 animSizeTable[NUM_ANIMS];

/** Rotation by anim and not by engine */
int16 processRotationByAnim;    // processActorVar5
/** Last rotation angle */
int16 processLastRotationAngle; // processActorVar6
/** Current process actor index */
int16 currentlyProcessedActorIdx;

/** Current step X coornidate */
int16 currentStepX;
/** Current step Y coornidate */
int16 currentStepY;
/** Current step Z coornidate */
int16 currentStepZ;
/** Current actor anim extra pointer */
uint8 *currentActorAnimExtraPtr;

/** Pointer to current animation keyframe */
uint8 *keyFramePtr;
/** Pointer to last animation keyframe */
uint8 *lastKeyFramePtr;

uint8 *animBuffer1;
uint8 *animBuffer2;

/** Set animation keyframe
	@param keyframIdx Animation keyframe index
	@param animPtr Pointer to animation
	@param bodyPtr Body model poitner
	@param animTimerDataPtr Animation time data */
int32 setAnimAtKeyframe(int32 keyframeIdx, uint8 *animPtr, uint8 *bodyPtr, AnimTimerDataStruct* animTimerDataPtr);

/** Get total number of keyframes in animation
	@param animPtr Pointer to animation */
int32 getNumKeyframes(uint8 *animPtr);

/** Get first keyframes in animation
	@param animPtr Pointer to animation */
int32 getStartKeyframe(uint8 *animPtr);

/** Set new body animation
	@param animIdx Animation index
	@param animPtr Animation pointer
	@param bodyPtr Body model poitner
	@param animTimerDataPtr Animation time data */
int32 setModelAnimation(int32 animIdx, uint8 *animPtr, uint8 *bodyPtr, AnimTimerDataStruct* animTimerDataPtr);

/** Get entity anim index (This is taken from File3D entities)
	@param animIdx Entity animation index
	@param actorIdx Actor index */
int32 getBodyAnimIndex(int32 animIdx, int32 actorIdx);

/** Stock animation - copy the next keyFrame from a different buffer
	@param animPtr Animation pointer
	@param bodyPtr Body model poitner
	@param animTimerDataPtr Animation time data */
int32 stockAnimation(uint8 *animPtr, uint8 *bodyPtr, AnimTimerDataStruct* animTimerDataPtr);

/** Verify animation at keyframe
	@param animIdx Animation index
	@param animPtr Animation pointer
	@param bodyPtr Body model poitner
	@param animTimerDataPtr Animation time data */
int32 verifyAnimAtKeyframe(int32 animPos, uint8 *animPtr, uint8 *bodyPtr, AnimTimerDataStruct* animTimerDataPtr);

/** Initialize animation
	@param newAnim animation to init
	@param animType animation type
	@param animExtra animation actions extra data
	@param actorIdx actor index */
int32 initAnim(int32 newAnim, int16 animType, uint8 animExtra, int32 actorIdx);

/** Process acotr animation actions
	@param actorIdx Actor index */
void processAnimActions(int32 actorIdx);

/** Process main loop actor animations
	@param actorIdx Actor index */
void processActorAnimations(int32 actorIdx);

#endif
