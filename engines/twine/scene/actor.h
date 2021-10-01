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

#ifndef TWINE_SCENE_ACTOR_H
#define TWINE_SCENE_ACTOR_H

#include "common/scummsys.h"
#include "twine/parser/anim.h"
#include "twine/parser/entity.h"
#include "twine/shared.h"

namespace TwinE {

/** Total number of sprites allowed in the game */
#define NUM_SPRITES 425 // 200 for lba1

/** Total number of bodies allowed in the game */
#define NUM_BODIES 469 // 131 for lba1

/** Actors move structure */
struct ActorMoveStruct {
	int16 from = 0;
	int16 to = 0;
	int16 numOfStep = 0;
	int32 timeOfChange = 0;

	/**
	 * Get actor real angle
	 * @param time engine time used for interpolation
	 */
	int32 getRealAngle(int32 time);

	/**
	 * Get actor step
	 * @param time engine time used for interpolation
	 */
	int32 getRealValue(int32 time);
};

/** Actors animation timer structure */
struct AnimTimerDataStruct {
	const KeyFrame *ptr = nullptr;
	int32 time = 0;
};

/** Actors static flags structure */
struct StaticFlagsStruct {
	uint32 bComputeCollisionWithObj : 1;    // 0x000001
	uint32 bComputeCollisionWithBricks : 1; // 0x000002
	uint32 bIsZonable : 1;                  // 0x000004
	uint32 bUsesClipping : 1;               // 0x000008
	uint32 bCanBePushed : 1;                // 0x000010
	uint32 bComputeLowCollision : 1;        // 0x000020
	uint32 bCanDrown : 1;                   // 0x000040
	uint32 bComputeCollisionWithFloor : 1;  // 0x000080
	uint32 bUnk0100 : 1;                    // 0x000100
	uint32 bIsHidden : 1;                   // 0x000200
	uint32 bIsSpriteActor : 1;              // 0x000400
	uint32 bCanFall : 1;                    // 0x000800
	uint32 bDoesntCastShadow : 1;           // 0x001000
	uint32 bIsBackgrounded : 1;             // 0x002000
	uint32 bIsCarrierActor : 1;             // 0x004000
	// take smaller value for bound, or if not set take average for bound
	uint32 bUseMiniZv : 1;                  // 0x008000
	uint32 bHasInvalidPosition : 1;         // 0x010000
	uint32 bNoElectricShock : 1;            // 0x020000
	uint32 bHasSpriteAnim3D : 1;            // 0x040000
	uint32 bNoPreClipping : 1;              // 0x080000
	uint32 bHasZBuffer : 1;                 // 0x100000
	uint32 bHasZBufferInWater : 1;          // 0x200000
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

/**
 * Bonus type flags - a bitfield value, of which the bits mean:
 * bit 8: clover leaf,
 * bit 7: small key,
 * bit 6: magic,
 * bit 5: life,
 * bit 4: money,
 * If more than one type of bonus is selected, the actual type of bonus
 * will be chosen randomly each time player uses Action.
 */
struct BonusParameter {
	uint16 unk1 : 1;
	uint16 unk2 : 1;
	uint16 unk3 : 1;
	uint16 unk4 : 1;
	uint16 kashes : 1;
	uint16 lifepoints : 1;
	uint16 magicpoints : 1;
	uint16 key : 1;
	uint16 cloverleaf : 1;
	uint16 unused : 7;
};

#define kActorMaxLife 50

/**
 * Actors structure
 *
 * Such as characters, doors, moving plataforms, invisible actors, ...
 */
class ActorStruct {
private:
	ShapeType _brickShape = ShapeType::kNone; // field_3
	bool _brickCausesDamage = false;

	EntityData _entityData;
public:
	StaticFlagsStruct _staticFlags;
	DynamicFlagsStruct _dynamicFlags;

	inline ShapeType brickShape() const { return _brickShape; }
	inline void setBrickShape(ShapeType shapeType) {
		_brickShape = shapeType;
		_brickCausesDamage = false;
	}
	inline void setBrickCausesDamage() { _brickCausesDamage = true; }
	inline bool brickCausesDamage() { return _brickCausesDamage; }
	void loadModel(int32 modelIndex, bool lba1);

	void addLife(int32 val);

	void setLife(int32 val);

	bool isAttackWeaponAnimationActive() const;
	bool isAttackAnimationActive() const;
	bool isJumpAnimationActive() const;

	const IVec3 &pos() const;

	int32 _entity = 0; // costumeIndex - index into bodyTable
	BodyType _body = BodyType::btNormal;
	AnimationTypes _anim = AnimationTypes::kAnimNone;
	AnimationTypes _animExtra = AnimationTypes::kStanding;
	AnimationTypes _animExtraPtr = AnimationTypes::kAnimNone;
	int32 _sprite = 0;
	EntityData *_entityDataPtr = nullptr;

	int16 _actorIdx = 0; // own actor index
	IVec3 _pos;
	int32 _strengthOfHit = 0;
	int32 _hitBy = 0;
	BonusParameter _bonusParameter;
	int32 _angle = 0; // facing angle of actor. Minumum is 0 (SW). Going counter clock wise
	int32 _speed = 0;
	ControlMode _controlMode = ControlMode::kNoMove;
	int32 _delayInMillis = 0;
	int32 _cropLeft = 0;
	int32 _cropTop = 0;
	int32 _cropRight = 0;
	int32 _cropBottom = 0;
	int32 _followedActor = 0; // same as info3
	int32 _bonusAmount = 0;
	int32 _talkColor = COLOR_BLACK;
	int32 _armor = 0;
	int32 _life = 0;

	IVec3 _collisionPos;

	int32 _positionInMoveScript = 0;
	uint8 *_moveScript = nullptr;
	int32 _moveScriptSize = 0;

	int32 _positionInLifeScript = 0;
	uint8 *_lifeScript = nullptr;
	int32 _lifeScriptSize = 0;

	int32 _labelIdx = 0;        // script label index
	int32 _currentLabelPtr = 0; // pointer to LABEL offset
	int32 _pausedTrackPtr = 0;

	/**
	 * colliding actor id
	 */
	int32 _collision = 0;
	/**
	 * actor id we are standing on
	 */
	int32 _standOn = 0;
	int32 _zone = 0;

	int32 _lastRotationAngle = ANGLE_0;
	IVec3 _lastPos;
	int32 _previousAnimIdx = 0;
	int32 _doorStatus = 0;
	int32 _animPosition = 0;
	AnimType _animType = AnimType::kAnimationTypeLoop;
	int32 _spriteActorRotation = 0;
	uint8 _brickSound = 0U;

	BoundingBox _boudingBox;
	ActorMoveStruct _move;
	AnimTimerDataStruct _animTimerData;
};

inline const IVec3 &ActorStruct::pos() const {
	return _pos;
}

inline void ActorStruct::addLife(int32 val) {
	setLife(_life + val);
}

inline void ActorStruct::setLife(int32 val) {
	_life = val;
	if (_life > kActorMaxLife) {
		_life = kActorMaxLife;
	}
}

class TwinEEngine;

class Actor {
private:
	TwinEEngine *_engine;

	/** Hero 3D entity for normal behaviour */
	EntityData _heroEntityNORMAL;
	/** Hero 3D entity for athletic behaviour */
	EntityData _heroEntityATHLETIC;
	/** Hero 3D entity for aggressive behaviour */
	EntityData _heroEntityAGGRESSIVE;
	/** Hero 3D entity for discrete behaviour */
	EntityData _heroEntityDISCRETE;
	/** Hero 3D entity for protopack behaviour */
	EntityData _heroEntityPROTOPACK;

	void initSpriteActor(int32 actorIdx);

	/**
	 * Initialize 3D actor body
	 * @param bodyIdx 3D actor body index
	 * @param actorIdx 3D actor index
	 */
	int32 initBody(BodyType bodyIdx, int32 actorIdx, ActorBoundingBox &actorBoundingBox);

	void loadBehaviourEntity(ActorStruct *actor, EntityData &entityData, int16 &bodyAnimIndex, int32 index);

public:
	Actor(TwinEEngine *engine);

	ActorStruct *_processActorPtr = nullptr;

	/** Actor shadow coordinate */
	IVec3 _shadowCoord;

	HeroBehaviourType _heroBehaviour = HeroBehaviourType::kNormal;
	/** Hero auto aggressive mode */
	bool _autoAggressive = true;
	/** Previous Hero behaviour */
	HeroBehaviourType _previousHeroBehaviour = HeroBehaviourType::kNormal;
	/** Previous Hero angle */
	int16 _previousHeroAngle = 0;

	int16 _cropBottomScreen = 0;

	/** Hero current anim for normal behaviour */
	int16 _heroAnimIdxNORMAL = 0;
	/** Hero current anim for athletic behaviour */
	int16 _heroAnimIdxATHLETIC = 0;
	/** Hero current anim for aggressive behaviour */
	int16 _heroAnimIdxAGGRESSIVE = 0;
	/** Hero current anim for discrete behaviour */
	int16 _heroAnimIdxDISCRETE = 0;
	/** Hero current anim for protopack behaviour */
	int16 _heroAnimIdxPROTOPACK = 0;

	/** Hero anim for behaviour menu */
	int16 _heroAnimIdx[4];

	/** Restart hero variables while opening new scenes */
	void restartHeroScene();

	/** Load hero 3D body and animations */
	void loadHeroEntities();

	TextId getTextIdForBehaviour() const;

	/**
	 * Set hero behaviour
	 * @param behaviour behaviour value to set
	 */
	void setBehaviour(HeroBehaviourType behaviour);

	/**
	 * Initialize 3D actor
	 * @param bodyIdx 3D actor body index
	 * @param actorIdx 3D actor index
	 */
	void initModelActor(BodyType bodyIdx, int16 actorIdx);

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
