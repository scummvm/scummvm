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

#ifndef TWINE_ACTOR_H
#define TWINE_ACTOR_H

#include "common/scummsys.h"

namespace TwinE {

/** Total number of sprites allowed in the game */
#define NUM_SPRITES 200

/** Total number of bodies allowed in the game */
#define NUM_BODIES 200

/** Hero behaviour
 * <li> NORMAL: Talk / Read / Search / Use
 * <li> ATHLETIC: Jump
 * <li> AGGRESSIVE:
 * Auto mode   : Fight
 * Manual mode : While holding the spacebar down
 * 			UP / RIGHT / LEFT will manually select
 * 			different punch/kick options
 * <li> DISCREET: Kneel down to hide
 *
 * @note The values must match the @c TextId indices
 */
enum HeroBehaviourType {
	kNormal = 0,
	kAthletic = 1,
	kAggressive = 2,
	kDiscrete = 3,
	kProtoPack = 4
};

/** Actors move structure */
struct ActorMoveStruct {
	int16 from = 0;
	int16 to = 0;
	int16 numOfStep = 0;
	int32 timeOfChange = 0;
};

/** Actors zone volumique points structure */
struct ZVPoint {
	int16 bottomLeft = 0;
	int16 topRight = 0;
};

/** Actors zone volumique box structure */
struct ZVBox {
	ZVPoint x;
	ZVPoint y;
	ZVPoint z;
};

/** Actors animation timer structure */
struct AnimTimerDataStruct {
	uint8 *ptr = nullptr;
	int32 time = 0;
};

enum AnimationTypes {
	kAnimNone = -1,
	kStanding = 0,
	kForward = 1,
	kBackward = 2,
	kTurnLeft = 3,
	kTurnRight = 4,
	kHit = 5,
	kBigHit = 6,
	kFall = 7,
	kLanding = 8,
	kLandingHit = 9,
	kLandDeath = 10,
	kAction = 11,
	kClimbLadder = 12,
	kTopLadder = 13,
	kJump = 14,
	kThrowBall = 15,
	kHide = 16,
	kKick = 17,
	kRightPunch = 18,
	kLeftPunch = 19,
	kFoundItem = 20,
	kDrawn = 21,
	kHit2 = 22,
	kSabreAttack = 23,
	kSabreUnknown = 24,
};

/** Actors static flags structure */
struct StaticFlagsStruct {
	uint16 bComputeCollisionWithObj : 1;    // 0x0001
	uint16 bComputeCollisionWithBricks : 1; // 0x0002
	uint16 bIsZonable : 1;                  // 0x0004
	uint16 bUsesClipping : 1;               // 0x0008
	uint16 bCanBePushed : 1;                // 0x0010
	uint16 bComputeLowCollision : 1;        // 0x0020
	uint16 bCanDrown : 1;                   // 0x0040
	uint16 bUnk80 : 1;                      // 0x0080
	uint16 bUnk0100 : 1;                    // 0x0100
	uint16 bIsHidden : 1;                   // 0x0200
	uint16 bIsSpriteActor : 1;              // 0x0400
	uint16 bCanFall : 1;                    // 0x0800
	uint16 bDoesntCastShadow : 1;           // 0x1000
	uint16 bIsBackgrounded : 1;             // 0x2000
	uint16 bIsCarrierActor : 1;             // 0x4000
	uint16 bUseMiniZv : 1;                  // 0x8000
};

/** Actors dynamic flags structure */
struct DynamicFlagsStruct {
	uint16 bWaitHitFrame : 1;     // 0x0001 wait for hit frame
	uint16 bIsHitting : 1;        // 0x0002 hit frame anim
	uint16 bAnimEnded : 1;        // 0x0004 anim ended in the current loop (will be looped in the next engine loop)
	uint16 bAnimFrameReached : 1; // 0x0008 new frame anim reached
	uint16 bIsVisible : 1;        // 0x0010 actor has been drawn in this loop
	uint16 bIsDead : 1;           // 0x0020 is dead
	uint16 bIsSpriteMoving : 1;   // 0x0040 door is opening or closing (wait to reach the destination position)
	uint16 bIsRotationByAnim : 1; // 0x0080 actor rotation is managed by its animaation not by the engine
	uint16 bIsFalling : 1;        // 0x0100 is falling on scene
	uint16 bUnk0200 : 1;          // 0x0200 unused
	uint16 bUnk0400 : 1;          // 0x0400 unused
	uint16 bUnk0800 : 1;          // 0x0800 unused
	uint16 bUnk1000 : 1;          // 0x1000 unused
	uint16 bUnk2000 : 1;          // 0x2000 unused
	uint16 bUnk4000 : 1;          // 0x4000 unused
	uint16 bUnk8000 : 1;          // 0x8000 unused
};

/** Control mode types */
enum ControlMode {
	kNoMove = 0,
	kManual = 1,
	kFollow = 2,
	kTrack = 3,
	kFollow2 = 4,
	kTrackAttack = 5,
	kSameXZ = 6,
	kRandom = 7
};

/** Actors structure */
struct ActorStruct {
	StaticFlagsStruct staticFlags;
	DynamicFlagsStruct dynamicFlags;

	int32 entity = 0; // costumeIndex
	int32 body = 0;
	AnimationTypes anim = kAnimNone;
	int32 animExtra = 0;  //field_2
	int32 brickShape = 0; // field_3
	const uint8 *animExtraPtr = nullptr;
	int32 sprite = 0; // field_8
	uint8 *entityDataPtr = nullptr;
	int32 entityDataSize = 0;

	int32 x = 0;
	int32 y = 0;
	int32 z = 0;
	int32 strengthOfHit = 0; // field_66
	int32 hitBy = 0;
	int32 bonusParameter = 0; // field_10
	int32 angle = 0;
	int32 speed = 0;
	ControlMode controlMode = ControlMode::kNoMove;
	int32 info0 = 0;         // cropLeft
	int32 info1 = 0;         // cropTop
	int32 info2 = 0;         // cropRight
	int32 info3 = 0;         // cropBottom
	int32 followedActor = 0; // same as info3
	int32 bonusAmount = 0;   // field_12
	int32 talkColor = 0;
	int32 armor = 0; // field_14
	int32 life = 0;

	int32 collisionX = 0; // field_20
	int32 collisionY = 0; // field_22
	int32 collisionZ = 0; // field_24

	int32 positionInMoveScript = 0;
	uint8 *moveScript = nullptr;
	int32 moveScriptSize = 0;

	int32 positionInLifeScript = 0;
	uint8 *lifeScript = nullptr;
	int32 lifeScriptSize = 0;

	int32 labelIdx = 0;        // script label index
	int32 currentLabelPtr = 0; // pointer to LABEL offset
	int32 pausedTrackPtr = 0;

	//int costumeIndex;
	int32 collision = 0;
	int32 standPosition = 0;
	int32 standOn = 0;
	int32 zone = 0;

	int32 lastRotationAngle = 0;
	int32 lastX = 0;
	int32 lastZ = 0;
	int32 lastY = 0;
	int32 previousAnimIdx = 0;
	int32 doorStatus = 0;
	int32 animPosition = 0;
	int32 animType = 0;   // field_78
	int32 brickSound = 0; // field_7A

	ZVBox boudingBox;
	ActorMoveStruct move;
	AnimTimerDataStruct animTimerData;
};

class TwinEEngine;

class Actor {
private:
	TwinEEngine* _engine;

	/** Hero 3D entity for normal behaviour */
	uint8 *heroEntityNORMAL = nullptr; // file3D0
	int32 heroEntityNORMALSize = 0;
	/** Hero 3D entity for athletic behaviour */
	uint8 *heroEntityATHLETIC = nullptr; // file3D1
	int32 heroEntityATHLETICSize = 0;
	/** Hero 3D entity for aggressive behaviour */
	uint8 *heroEntityAGGRESSIVE = nullptr; // file3D2
	int32 heroEntityAGGRESSIVESize = 0;
	/** Hero 3D entity for discrete behaviour */
	uint8 *heroEntityDISCRETE = nullptr; // file3D3
	int32 heroEntityDISCRETESize = 0;
	/** Hero 3D entity for protopack behaviour */
	uint8 *heroEntityPROTOPACK = nullptr; // file3D4
	int32 heroEntityPROTOPACKSize = 0;

	struct ActorBoundingBox {
		/** Bottom left X coordinate */
		int16 bottomLeftX = 0;
		/** Bottom left Y coordinate */
		int16 bottomLeftY = 0;
		/** Bottom left Z coordinate */
		int16 bottomLeftZ = 0;
		/** Top left X coordinate */
		int16 topRightX = 0;
		/** Top left Y coordinate */
		int16 topRightY = 0;
		/** Top left Z coordinate */
		int16 topRightZ = 0;
		bool hasBoundingBox = false;
	};

	void initSpriteActor(int32 actorIdx);

	/**
	 * Initialize 3D actor body
	 * @param bodyIdx 3D actor body index
	 * @param actorIdx 3D actor index
	 */
	int32 initBody(int32 bodyIdx, int32 actorIdx, ActorBoundingBox &actorBoundingBox);

public:
	Actor(TwinEEngine* engine);
	~Actor();

	ActorStruct *processActorPtr = nullptr; // processActorVar1

	/** Actor shadow X coordinate */
	int32 shadowX = 0;
	/** Actor shadow Y coordinate */
	int32 shadowY = 0;
	/** Actor shadow Z coordinate */
	int32 shadowZ = 0;
	/** Actor shadow collition type - brick shape */
	int8 shadowCollisionType = 0; // shadowVar

	HeroBehaviourType heroBehaviour = kNormal;
	/** Hero auto agressive mode */
	bool autoAgressive = true;
	/** Previous Hero behaviour */
	HeroBehaviourType previousHeroBehaviour = kNormal;
	/** Previous Hero angle */
	int16 previousHeroAngle = 0;

	int16 cropBottomScreen = 0;

	/** Hero current anim for normal behaviour */
	int16 heroAnimIdxNORMAL = 0; // TCos0Init
	/** Hero current anim for athletic behaviour */
	int16 heroAnimIdxATHLETIC = 0; // TCos1Init
	/** Hero current anim for aggressive behaviour */
	int16 heroAnimIdxAGGRESSIVE = 0; // TCos2Init
	/** Hero current anim for discrete behaviour */
	int16 heroAnimIdxDISCRETE = 0; // TCos3Init
	/** Hero current anim for protopack behaviour */
	int16 heroAnimIdxPROTOPACK = 0; // TCos4Init

	/** Hero anim for behaviour menu */
	int16 heroAnimIdx[4]; // TCOS

	/** Actors 3D body table - size of NUM_BODIES */
	uint8 *bodyTable[NUM_BODIES]{nullptr};
	int32 bodyTableSize[NUM_BODIES]{0};

	/** Current position in body table */
	int32 currentPositionInBodyPtrTab;

	/** Restart hero variables while opening new scenes */
	void restartHeroScene();

	/** Load hero 3D body and animations */
	void loadHeroEntities();

	int32 getTextIdForBehaviour() const;

	/**
	 * Set hero behaviour
	 * @param behaviour behaviour value to set
	 */
	void setBehaviour(int32 behaviour);

	/** Preload all sprites */
	void preloadSprites();

	/**
	 * Initialize 3D actor
	 * @param bodyIdx 3D actor body index
	 * @param actorIdx 3D actor index
	 */
	void initModelActor(int32 bodyIdx, int16 actorIdx);

	/**
	 * Initialize actors
	 * @param actorIdx actor index to init
	 */
	void initActor(int16 actorIdx);

	/**
	 * Reset actor
	 * @param actorIdx actor index to init
	 */
	void resetActor(int16 actorIdx);

	/**
	 * Process hit actor
	 * @param actorIdx actor hitting index
	 * @param actorIdxAttacked actor attacked index
	 * @param strengthOfHit actor hitting strength of hit
	 * @param angle angle of actor hitting
	 */
	void hitActor(int32 actorIdx, int32 actorIdxAttacked, int32 strengthOfHit, int32 angle);

	/** Process actor carrier */
	void processActorCarrier(int32 actorIdx);

	/** Process actor extra bonus */
	void processActorExtraBonus(int32 actorIdx);
};

} // namespace TwinE

#endif
