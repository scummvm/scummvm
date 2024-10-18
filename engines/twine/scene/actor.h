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

#ifndef TWINE_SCENE_ACTOR_H
#define TWINE_SCENE_ACTOR_H

#include "common/scummsys.h"
#include "twine/parser/anim.h"
#include "twine/parser/body.h"
#include "twine/parser/entity.h"
#include "twine/shared.h"

namespace TwinE {

/** Total number of sprites allowed in the game */
#define NUM_SPRITES 425 // 200 for lba1

/** Total number of bodies allowed in the game */
#define NUM_BODIES 469 // 131 for lba1

/** Actors move structure */
struct RealValue {
	int16 startValue = 0;
	int16 endValue = 0;
	int16 timeValue = 0;
	int32 memoTicks = 0;

	/**
	 * Get actor real angle
	 * @param time engine time used for interpolation
	 */
	int16 getRealValueFromTime(int32 time);
	int16 getRealAngle(int32 time);
};

/** Actors animation timer structure */
struct AnimTimerDataStruct {
	const KeyFrame *ptr = nullptr;
	int32 time = 0;
};

/** Actors static flags structure */
struct StaticFlagsStruct {
	uint32 bComputeCollisionWithObj : 1;    // 0x000001 CHECK_OBJ_COL
	uint32 bComputeCollisionWithBricks : 1; // 0x000002 CHECK_BRICK_COL
	uint32 bCheckZone : 1;                  // 0x000004 CHECK_ZONE - testing of scenaric areas
	uint32 bSpriteClip : 1;                 // 0x000008 SPRITE_CLIP - (doors) fixed clip area
	uint32 bCanBePushed : 1;                // 0x000010 PUSHABLE
	uint32 bComputeLowCollision : 1;        // 0x000020 COL_BASSE
	uint32 bCanDrown : 1;                   // 0x000040 CHECK_CODE_JEU
	uint32 bComputeCollisionWithFloor : 1;  // 0x000080 CHECK_WATER_COL
	uint32 bUnk0100 : 1;                    // 0x000100
	uint32 bIsInvisible : 1;                // 0x000200 INVISIBLE - not drawn but all computed
	uint32 bSprite3D : 1;                   // 0x000400 SPRITE_3D - a sprite not a 3D object
	uint32 bObjFallable : 1;                // 0x000800 OBJ_FALLABLE
	uint32 bNoShadow : 1;                   // 0x001000 NO_SHADOW - no auto shadow
	uint32 bIsBackgrounded : 1;             // 0x002000 OBJ_BACKGROUND - is embedded in the decor the 1st time
	uint32 bIsCarrierActor : 1;             // 0x004000 OBJ_CARRIER - can carry and move an obj
	// take smaller value for bound, or if not set take average for bound
	uint32 bUseMiniZv : 1;                  // 0x008000 MINI_ZV - square on smaller dimension (if 3D object)
	uint32 bHasInvalidPosition : 1;         // 0x010000 POS_INVALIDE - carrier considered as an invalid position
	uint32 bNoElectricShock : 1;            // 0x020000 NO_CHOC - does not trigger electric shock animation
	uint32 bHasSpriteAnim3D : 1;            // 0x040000 ANIM_3DS - 3DS animation (extension of 3D sprite)
	uint32 bNoPreClipping : 1;              // 0x080000 NO_PRE_CLIP - does not pre-clip the object (for large objects)
	uint32 bHasZBuffer : 1;                 // 0x100000 OBJ_ZBUFFER - displays object in ZBuffer (exterior only!)
	uint32 bHasZBufferInWater : 1;          // 0x200000 OBJ_IN_WATER - displays object in ZBuffer in water (exterior only!)
};

/** Actors dynamic flags structure */
struct DynamicFlagsStruct {
	uint32 bWaitHitFrame : 1;            // 0x0001 WAIT_HIT_FRAME - wait for hit frame
	uint32 bIsHitting : 1;               // 0x0002 OK_HIT - hit frame anim
	uint32 bAnimEnded : 1;               // 0x0004 ANIM_END - anim ended in the current loop (will be looped in the next engine loop)
	uint32 bAnimNewFrame : 1;            // 0x0008 NEW_FRAME - new frame anim reached
	uint32 bWasDrawn : 1;                // 0x0010 WAS_DRAWN - actor has been drawn in this loop
	uint32 bIsDead : 1;                  // 0x0020 OBJ_DEAD - is dead
	uint32 bIsSpriteMoving : 1;          // 0x0040 AUTO_STOP_DOOR - door is opening or closing (wait to reach the destination position)
	uint32 bIsRotationByAnim : 1;        // 0x0080 ANIM_MASTER_ROT - actor rotation is managed by its animation not by the engine
	uint32 bIsFalling : 1;               // 0x0100 FALLING - is falling on scene
	uint32 bIsTargetable : 1;            // 0x0200 IS_TARGETABLE (lba1) OK_SUPER_HIT (lba2)
	uint32 bIsBlinking : 1;              // 0x0400 IS_BLINKING (lba1) FRAME_SHIELD (lba2)
	uint32 bWasWalkingBeforeFalling : 1; // 0x0800 DRAW_SHADOW (lba2) - bWasWalkingBeforeFalling in lba1
	uint32 bANIM_MASTER_GRAVITY : 1;     // 0x1000 ANIM_MASTER_GRAVITY (lba2)
	uint32 bSKATING : 1;                 // 0x2000 SKATING (lba2) Ouch! I slip in a forbidden collision
	uint32 bOK_RENVOIE : 1;              // 0x4000 OK_RENVOIE (lba2) ready to send back a projectile
	uint32 bLEFT_JUMP : 1;               // 0x8000 LEFT_JUMP (lba2) ready to jump from the left foot
	uint32 bRIGHT_JUMP : 1;              // RIGHT_JUMP          (1<<16) // (lba2) ready to jump from the right foot
	uint32 bWAIT_SUPER_HIT : 1;          // WAIT_SUPER_HIT      (1<<17) // (lba2) waiting for the end of the animation before giving another super hit
	uint32 bTRACK_MASTER_ROT : 1;        // TRACK_MASTER_ROT    (1<<18) // (lba2) it's the track that manages the direction
	uint32 bFLY_JETPACK : 1;             // FLY_JETPACK         (1<<19) // (lba2) flying with the Jetpack
	uint32 bDONT_PICK_CODE_JEU : 1;      // DONT_PICK_CODE_JEU  (1<<20) // (lba2) Cheat - Conveyor Belt Zones
	uint32 bMANUAL_INTER_FRAME : 1;      // MANUAL_INTER_FRAME  (1<<21) // (lba2) Manually performs the ObjectSetInterFrame()
	uint32 bWAIT_COORD : 1;              // WAIT_COORD          (1<<22) // (lba2) waiting to have been displayed to pass the coordinates from one point to an extra
	uint32 bCHECK_FALLING : 1;           // CHECK_FALLING       (1<<23) // (lba2) forces object to test FALLING during a frame
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
	uint16 givenNothing : 1;
	uint16 unk2 : 1; // unused in lba1
	uint16 unk3 : 1; // unused in lba1
	uint16 unk4 : 1; // unused in lba1
	uint16 kashes : 1;
	uint16 lifepoints : 1;
	uint16 magicpoints : 1;
	uint16 key : 1;
	uint16 cloverleaf : 1;
	uint16 unused : 7;
};

/**
 * Actors structure
 *
 * Such as characters, doors, moving platforms, invisible actors, ...
 */
class ActorStruct { // T_OBJET
private:
	ShapeType _col = ShapeType::kNone; // collision
	bool _brickCausesDamage = false;
	int32 _maxLife;

public:
	ActorStruct(int maxLife = 0) : _lifePoint(maxLife), _maxLife(maxLife) {}
	StaticFlagsStruct _flags;
	DynamicFlagsStruct _workFlags;

	EntityData _entityData;
	inline ShapeType brickShape() const { return _col; }
	inline void setCollision(ShapeType shapeType) {
		_col = shapeType;
		_brickCausesDamage = false;
	}
	inline void setBrickCausesDamage() { _brickCausesDamage = true; }
	inline bool brickCausesDamage() { return _brickCausesDamage; }

	void addLife(int32 val);

	void setLife(int32 val);

	bool isAttackWeaponAnimationActive() const;
	bool isAttackAnimationActive() const;
	bool isJumpAnimationActive() const;

	const IVec3 &posObj() const;

	int32 _body = -1; // costumeIndex - index into bodyTable
	BodyType _genBody = BodyType::btNormal;
	BodyType _saveGenBody = BodyType::btNormal; // lba2
	AnimationTypes _genAnim = AnimationTypes::kAnimNone;
	AnimationTypes _nextGenAnim = AnimationTypes::kStanding;
	AnimationTypes _ptrAnimAction = AnimationTypes::kAnimNone;
	int32 _sprite = 0;
	EntityData *_entityDataPtr = nullptr;

	int16 _actorIdx = 0; // own actor index
	IVec3 _posObj; // PosObjX, PosObjY, PosObjZ

	// T_ANIM_3DS - Coord.A3DS
	struct A3DSAnim {
		int32 Num;
		int32 Deb;
		int32 Fin;
	} A3DS;

	int32 _strengthOfHit = 0;
	int32 _hitBy = -1;
	BonusParameter _bonusParameter;
	int32 _beta = 0; // facing angle of actor. Minumum is 0 (SW). Going counter clock wise
	int32 _srot = 40; // speed of rotation
	ControlMode _move = ControlMode::kNoMove; // Move
	int32 _delayInMillis = 0; // Info
	int32 _cropLeft = 0;      // Info
	int32 _cropTop = 0;       // Info1
	int32 _cropRight = 0;     // Info2
	int32 _cropBottom = 0;    // Info3
	int32 _followedActor = 0; // same as Info3
	int32 _bonusAmount = 0;
	int32 _talkColor = COLOR_BLACK;
	int32 _armor = 1;
	int32 _lifePoint = 0;

	/** Process actor coordinate Nxw, Nyw, Nzw */
	IVec3 _processActor;
	IVec3 _oldPos; // OldPosX, OldPosY, OldPosZ

	int32 _offsetTrack = -1;
	uint8 *_ptrTrack = nullptr;
	int32 _moveScriptSize = 0;

	int32 _offsetLife = 0;
	int32 _saveOffsetLife = 0; // lba2
	uint8 *_lifeScript = nullptr;
	int32 _lifeScriptSize = 0;

	int32 _labelTrack = 0;       // script label index
	int32 _offsetLabelTrack = 0; // pointer to LABEL offset
	int32 _memoLabelTrack = 0;

	/**
	 * colliding actor id
	 */
	int32 _objCol = -1;
	/**
	 * actor id we are standing on
	 */
	int32 _carryBy = -1;
	int32 _zoneSce = -1;

	int32 _animStepBeta = 0;
	IVec3 _animStep;
	int32 _anim = -1;
	int32 _doorWidth = 0;
	int32 _frame = 0;
	AnimType _flagAnim = AnimType::kAnimationTypeRepeat;
	int32 _spriteActorRotation = 0;
	uint8 _brickSound = 0U; // CodeJeu
	int32 SampleAlways = 0; // lba2
	uint8 SampleVolume = 0; // lba2
	// SizeSHit contains the number of the brick under the wagon - hack
	int16 SizeSHit; // lba2 - always square

	// T_OBJ_3D Obj; // lba2
	// T_GROUP_INFO CurrentFrame[30]; // lba2

	BoundingBox _boundingBox; // Xmin, YMin, Zmin, Xmax, Ymax, Zmax
	RealValue realAngle;
	AnimTimerDataStruct _animTimerData;
};

inline const IVec3 &ActorStruct::posObj() const {
	return _posObj;
}

inline void ActorStruct::addLife(int32 val) {
	setLife(_lifePoint + val);
}

inline void ActorStruct::setLife(int32 val) {
	_lifePoint = val;
	if (_lifePoint > _maxLife) {
		_lifePoint = _maxLife;
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

	/**
	 * Initialize 3D actor body
	 * @param bodyIdx 3D actor body index
	 * @param actorIdx 3D actor index
	 */
	int32 searchBody(BodyType bodyIdx, int32 actorIdx, ActorBoundingBox &actorBoundingBox);

	void loadBehaviourEntity(ActorStruct *actor, EntityData &entityData, int16 &bodyAnimIndex, int32 index);

	void copyInterAnim(const BodyData &src, BodyData &dest);

public:
	Actor(TwinEEngine *engine);

	HeroBehaviourType _heroBehaviour = HeroBehaviourType::kNormal; // Comportement
	HeroBehaviourType _saveHeroBehaviour = HeroBehaviourType::kNormal; // SaveComportementHero (lba2)
	/** Hero auto aggressive mode */
	bool _combatAuto = true;
	/** Previous Hero behaviour */
	HeroBehaviourType _previousHeroBehaviour = HeroBehaviourType::kNormal;
	/** Previous Hero angle */
	int16 _previousHeroAngle = 0;

	int16 _cropBottomScreen = 0; // TODO: usage differ in original sources

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

	void initSprite(int32 spriteNum, int32 actorIdx);
	void setFrame(int32 actorIdx, uint32 frame);

	/** Restart hero variables while opening new scenes */
	void restartPerso();

	/** Load hero 3D body and animations */
	void loadHeroEntities();

	TextId getTextIdForBehaviour() const;

	/**
	 * Set hero behaviour
	 * @param behaviour behaviour value to set
	 */
	void setBehaviour(HeroBehaviourType behaviour); // SetComportement

	/**
	 * Initialize 3D actor
	 * @param bodyIdx 3D actor body index
	 * @param actorIdx 3D actor index
	 */
	void initBody(BodyType bodyIdx, int16 actorIdx);

	/**
	 * Initialize actors
	 * @param actorIdx actor index to init
	 */
	void startInitObj(int16 actorIdx);

	/**
	 * Reset actor
	 * @param actorIdx actor index to init
	 */
	void initObject(int16 actorIdx);

	/**
	 * Process hit actor
	 * @param actorIdx actor hitting index
	 * @param actorIdxAttacked actor attacked index
	 * @param strengthOfHit actor hitting strength of hit
	 * @param angle angle of actor hitting
	 */
	void hitObj(int32 actorIdx, int32 actorIdxAttacked, int32 strengthOfHit, int32 angle);

	/** Process actor carrier */
	void checkCarrier(int32 actorIdx);

	/** Process actor extra bonus */
	void giveExtraBonus(int32 actorIdx);

	// Lba2
	void posObjectAroundAnother(uint8 numsrc, uint8 numtopos); // PosObjetAroundAnother
};

} // namespace TwinE

#endif
