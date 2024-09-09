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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_ACTOR_H
#define SAGA2_ACTOR_H

#include "saga2/objects.h"
#include "saga2/saveload.h"

namespace Saga2 {

class ActorAssignment;
class Band;
class MotionTask;
class TaskStack;

const int   kBandingDist = kSectorSize * 2;
const int   kActorScriptVars = 4;

/* ===================================================================== *
   Actor character attributes
 * ===================================================================== */

//  Defines the colors of MANA

enum ActorSkillID {
	kSkillIDArchery      =  0,
	kSkillIDSwordcraft,
	kSkillIDShieldcraft,
	kSkillIDBludgeon,
	kSkillIDThrowing,    //  gone
	kSkillIDSpellcraft,
	kSkillIDStealth,     //  gone
	kSkillIDAgility,
	kSkillIDBrawn,
	kSkillIDLockpick,    //  gone
	kSkillIDPilfer,      //  gone
	kSkillIDFirstAid,    //  gone
	kSkillIDSpotHidden,  //  gone
	kNumSkills
};

enum ArmorObjectTypes {
	SHIRT_ARMOR = 0,
	BOOT_ARMOR,
	BRACER_ARMOR,
	HELM_ARMOR,
	NECKLACE_ARMOR,
	CLOAK_ARMOR,
	RING_ARMOR,

	ARMOR_COUNT
};

/* ===================================================================== *
   ArmorAttributes struct
 * ===================================================================== */

struct ArmorAttributes {
	uint8       damageAbsorbtion,
	            damageDivider,
	            defenseBonus;
};

/* ===================================================================== *
   ActorAttributes structure
 * ===================================================================== */

// this enum acts as a layer of indirection for the unioned allSkills
// array.


//  This defines the basic skills possessed by an actor
struct ActorAttributes {
	enum skillInfo {
		kSkillBasePercent            = 100,
		kSkillFracPointsPerLevel     = 5,    // this being twenty and levels at 5
		kSkillLevels                 = 20,   // will make the advancement calc quick & easy
		kSkillMaxLevel               = kSkillFracPointsPerLevel * kSkillLevels
	};

	enum vitalityInfo {
		kVitalityLimit               = 256
	};

	//  Automatic skills
	uint8       archery,        //  Accuracy of missile weapons
				swordcraft,     //  Accuracy of bladed melee weapons
				shieldcraft,    //  Actor's ability to use a shield
				bludgeon,       //  Accuracy of non-bladed melee weapons
				throwing,       //  Ability to throw objects accurately
				spellcraft,     //  Accuracy of spell combat
				stealth,        //  Ability to remain unnoticed
				agility,        //  Ability to dodge
				brawn,          //  Ability to lift, and damage of weapons
				lockpick;       //  Ability to pick locks

	//  Manual skills
	uint8       pilfer,         //  Ability to "lift" an item
				firstAid,       //  Ability to heal recent injuries
				spotHidden;     //  Ability to spot hidden objects

	//  Pad byte for alignment
	int8 pad;

	//  Hit-points
	int16 vitality;

	//  Magic energy
	int16       redMana,
				orangeMana,
				yellowMana,
				greenMana,
				blueMana,
				violetMana;

	uint8 &skill(int16 id) {
		switch (id) {
		case kSkillIDArchery: return archery;
		case kSkillIDSwordcraft: return swordcraft;
		case kSkillIDShieldcraft: return shieldcraft;
		case kSkillIDBludgeon: return bludgeon;
		case kSkillIDThrowing: return throwing;
		case kSkillIDSpellcraft: return spellcraft;
		case kSkillIDStealth: return stealth;
		case kSkillIDAgility: return agility;
		case kSkillIDBrawn: return brawn;
		case kSkillIDLockpick: return lockpick;
		case kSkillIDPilfer: return pilfer;
		case kSkillIDFirstAid: return firstAid;
		case kSkillIDSpotHidden: return spotHidden;
		}
		error("Incorrect skill id: %d", id);
	}
	int16 &mana(int16 id) {
		switch (id) {
		case kManaIDRed: return redMana;
		case kManaIDOrange: return orangeMana;
		case kManaIDYellow: return yellowMana;
		case kManaIDGreen: return greenMana;
		case kManaIDBlue: return blueMana;
		case kManaIDViolet: return violetMana;
		}
		error("Incorrect mana id: %d", id);
	}

	uint8 getSkillLevel(int16 id) {
		return skill(id) / kSkillFracPointsPerLevel + 1;
	}

	void read(Common::InSaveFile *in) {
		archery = in->readByte();
		swordcraft = in->readByte();
		shieldcraft = in->readByte();
		bludgeon = in->readByte();
		throwing = in->readByte();
		spellcraft = in->readByte();
		stealth = in->readByte();
		agility = in->readByte();
		brawn = in->readByte();
		lockpick = in->readByte();
		pilfer = in->readByte();
		firstAid = in->readByte();
		spotHidden = in->readByte();
		pad = in->readSByte();
		vitality = in->readSint16LE();
		redMana = in->readSint16LE();
		orangeMana = in->readSint16LE();
		yellowMana = in->readSint16LE();
		greenMana = in->readSint16LE();
		blueMana = in->readSint16LE();
		violetMana = in->readSint16LE();
	}

	void write(Common::MemoryWriteStreamDynamic *out) {
		out->writeByte(archery);
		out->writeByte(swordcraft);
		out->writeByte(shieldcraft);
		out->writeByte(bludgeon);
		out->writeByte(throwing);
		out->writeByte(spellcraft);
		out->writeByte(stealth);
		out->writeByte(agility);
		out->writeByte(brawn);
		out->writeByte(lockpick);
		out->writeByte(pilfer);
		out->writeByte(firstAid);
		out->writeByte(spotHidden);
		out->writeSByte(pad);
		out->writeSint16LE(vitality);
		out->writeSint16LE(redMana);
		out->writeSint16LE(orangeMana);
		out->writeSint16LE(yellowMana);
		out->writeSint16LE(greenMana);
		out->writeSint16LE(blueMana);
		out->writeSint16LE(violetMana);
	}
};  // 28 bytes


const int kBaseCarryingCapacity = 100;
const int kCarryingCapacityBonusPerBrawn = 200 / ActorAttributes::kSkillLevels;

/* ===================================================================== *
   ResourceActorProtoExtension structure
 * ===================================================================== */

enum combatBehaviorTypes {
	kBehaviorHungry,
	kBehaviorCowardly,
	kBehaviorBerserk,
	kBehaviorSmart
};

//  This defines the additional data fields needed for actor prototypes
struct ResourceActorProtoExtension {

	ActorAttributes     baseStats;  //  Base stats for non-player actors

	//  Defines behavior for combat tactics.
	uint8               combatBehavior;
	uint8               gruntStyle;

	uint32              baseEffectFlags;    // special effects, see EFFECTS.H

	//  Default constructor -- do nothing
	ResourceActorProtoExtension() {
		memset(&baseStats, 0, sizeof(baseStats));

		combatBehavior = 0;
		gruntStyle = 0;
		baseEffectFlags = 0;
	}

	//  Copy constructor
	ResourceActorProtoExtension(ResourceActorProtoExtension &ext) {
		memcpy(this, &ext, sizeof(ResourceActorProtoExtension));
	}

	void load(Common::SeekableReadStream *stream) {
		baseStats.read(stream);
		combatBehavior = stream->readByte();
		gruntStyle = stream->readByte();
		baseEffectFlags = stream->readUint32LE();
	}
};  // 28 bytes

/* ===================================================================== *
   ResourceActorPrototype structure
 * ===================================================================== */

//  Defines the actor prototype data as read from the resource file
struct ResourceActorPrototype {
	ResourceObjectPrototype         proto;  //  Standard prototype data
	ResourceActorProtoExtension     ext;    //  Extended actor data

	void load(Common::SeekableReadStream *stream) {
		proto.load(stream);
		ext.load(stream);
	}
};

/* ===================================================================== *
   ActorProto prototype behavior for Actors
 * ===================================================================== */

class ActorProto : public ProtoObj, public ResourceActorProtoExtension {

private:
	enum {
		kViewableRows    = 3,
		kViewableCols    = 3,
		kMaxRows         = 3,
		kMaxCols         = 3
	};

public:
	ActorProto(ResourceActorPrototype &a) :
		ProtoObj(a.proto),
		ResourceActorProtoExtension(a.ext) {
	}

	// returns the containment type flags for this object
	virtual uint16 containmentSet();

	//  returns true if this object can contain another object
	virtual bool canContain(ObjectID dObj, ObjectID item);

	//  Determine if this object can contain another object at a
	//  specified slot
	virtual bool canContainAt(
	    ObjectID dObj,
	    ObjectID item,
	    const TilePoint &where);

	weaponID getWeaponID();

	//  use this actor
	bool useAction(ObjectID dObj, ObjectID enactor);

	//  open this actor
	bool canOpen(ObjectID dObj, ObjectID enactor);
	bool openAction(ObjectID dObj, ObjectID enactor);

	//  close this actor
	bool closeAction(ObjectID dObj, ObjectID enactor);

	bool strikeAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID item);

	bool damageAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID target);

	//  drop another object onto this actor.
	bool acceptDropAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID droppedObj,
	    int count);

	//  cause damage directly
	bool acceptDamageAction(
	    ObjectID            dObj,
	    ObjectID            enactor,
	    int8                absDamage,
	    effectDamageTypes   dType,
	    int8                dice,
	    uint8               sides,
	    int8                perDieMod);

	//  cause healing directly
	bool acceptHealingAction(ObjectID dObj, ObjectID enactor, int8 healing);

	//  Accept strike from an object (allows this actor to cause
	//  damage to the striking object).
	bool acceptStrikeAction(
	    ObjectID            dObj,
	    ObjectID            enactor,
	    ObjectID            strikingObj,
	    uint8               skillIndex);

	//  Handle the results of an object being inserted into this object
	//  at the specified slot
	bool acceptInsertionAtAction(
	    ObjectID        dObj,
	    ObjectID        enactor,
	    ObjectID        item,
	    const TilePoint &where,
	    int16           num = 1);

	//  Initiate an attack using this type of object
	virtual void initiateAttack(ObjectID attacker, ObjectID target);

	//  Given an object sound effect record, which sound should be made
	//  when this object is damaged
	virtual uint8 getDamageSound(const ObjectSoundFXs &soundFXs);

	//  Do the background processing, if needed, for this object.
	void doBackgroundUpdate(GameObject *obj);

	//  Cause the user's associated skill to grow
	void applySkillGrowth(ObjectID enactor, uint8 points = 1);

	bool greetActor(
	    ObjectID        dObj,                   // object dropped on
	    ObjectID        enactor);               // person doing dropping

public:
	virtual uint16 getViewableRows() {
		return kViewableRows;
	}
	virtual uint16 getViewableCols() {
		return kViewableCols;
	}
	virtual uint16 getMaxRows() {
		return kMaxRows;
	}
	virtual uint16 getMaxCols() {
		return kMaxCols;
	}

	virtual bool canFitBulkwise(GameObject *container, GameObject *obj);
	virtual bool canFitMasswise(GameObject *container, GameObject *obj);

	virtual uint16 massCapacity(GameObject *container);
	virtual uint16 bulkCapacity(GameObject *container);
};

/* ============================================================================ *
   Actor: Describes an instance of a character
 * ============================================================================ */

enum actorCreationFlags {
	kActorPermanent = (1 << 0)
};

enum DispositionType {
	kDispositionFriendly,
	kDispositionEnemy,

	kDispositionPlayer
};

enum actionSequenceOptions {

	//  Flags set by call to setAction
	kAnimateRepeat    = (1 << 0),            // repeat animation when done
	kAnimateReverse   = (1 << 1),            // animate in reverse direction
	kAnimateAlternate = (1 << 2),            // both directions, back & forth
	kAnimateRandom    = (1 << 3),            // pick a random frame
	kAnimateNoRestart = (1 << 4),            // don't reset from start

	//  This flag is set if the animation has been put on hold until
	//  the actor's appearance is reloaded.
	kAnimateOnHold   = (1 << 5),

	//  This flag is set if the final frame of the animation has
	//  been reached.
	kAnimateFinished = (1 << 6),

	//  This flag gets set if the sprite could not be displayed
	//  because it's bank hasn't been loaded yet.
	kAnimateNotLoaded = (1 << 7)
};

//  Various types of action sequences


enum ActorAnimationTypes {

	//  Various types of stands
	kActionStand = 0,                        // standing still
	kActionWaitAgressive,                    // an agigressive wait cycle
	kActionWaitImpatient,                    // an impatient wait cycle
	kActionWaitFriendly,                     // a a friendly wait cycle

	//  Walking and running
	kActionWalk,                             // walking motion
	kActionRun,                              // running motion

	//  Squatting
	kActionDuck,                             // stoop to dodge sword
	kActionStoop,                            // stoop to pick up object

	//  Jumping
	kActionFreeFall,                         // how he looks in ballistic
	kActionFreeFallRunning,                  // a running leap (free fall)
	kActionJumpUp,                           // begin jump straight up
	kActionJumpFwd,                          // begin jump forward
	kActionJumpBack,                         // begin jump back in surprise
	kActionLand,                             // land after jump
	kActionFallBadly,                        // after a very long fall

	//  Climbing
	kActionClimbLadder,                      // climb a ladder (2 directions)

	//  Talking & interacting
	kActionTalk,                             // talking
	kActionGesture,                          // gesture with hands or body
	kActionGiveItem,                         // give or take item

	//  Two-handed weapon use
	kActionTwoHandSwingHigh,                 // full overhead swing aim high
	kActionTwoHandSwingLow,                  // full overhead swing aim low
	kActionTwoHandSwingLeftHigh,             // partial swing on left (high)
	kActionTwoHandSwingLeftLow,              // partial swing on left (low)
	kActionTwoHandSwingRightHigh,            // partial swing on rgt (high)
	kActionTwoHandSwingRightLow,             // partial swing on rgt (low)
	kActionTwoHandParry,                     // hold sword up to parry

	//  One-handed weapon use
	kActionSwingHigh,                        // one-handed swing (high)
	kActionSwingLow,                         // one-handed swing (low)
	kActionParryHigh,                        // one-handed parry (high)
	kActionParryLow,                         // one-handed parry (low)
	kActionShieldParry,                      // parry with shield

	kActionThrowObject,                      // throw

	//  Other combat actions
	kActionFireBow,                          // fire an arrow
	kActionCastSpell,                        // cast a magic spell
	kActionUseWand,                          // cast a magic spell w/wand
	kActionUseStaff,                         // cast a magic spell w/staff
	kActionHit,                              // show impact of blow
	kActionKnockedDown,                      // knocked down by opponent
	kActionDie,                              // death agony

	//  Passive actions
	kActionSleep,                            // sleeping
	kActionDead,                             // dead body on ground
	kActionSit,                              // sitting at table

	//  Misc actions built from other frames
	kActionListenAtDoor,                     // listening at doors
	kActionShoveDoor,                        // try to force a door open
	kActionSpecial1,                         // special Action
	kActionSpecial2,                         // special Action
	kActionSpecial3,                         // special Action
	kActionSpecial4,                         // special Action
	kActionSpecial5,                         // special Action
	kActionSpecial6,                         // special Action
	kActionSpecial7,                         // special Action
	kActionSpecial8                          // special Action
};

enum ActorGoalTypes {
	kActorGoalFollowAssignment,
	kActorGoalPreserveSelf,
	kActorGoalAttackEnemy,
	kActorGoalFollowLeader,
	kActorGoalAvoidEnemies
};

//  The actor structure will be divided into two parts.  The
//  ResourceActor structure defines the data as it is stored in the
//  resource file.  The Actor structure has a copy of all of the
//  ResourceActor data members, plus data members which will be
//  initialized and used during run time.

struct ResourceActor : public ResourceGameObject {

	//  Social loyalty
	uint8           faction;                // actor's faction

	//  Appearance attribute
	uint8           colorScheme;            // indirect color map

	int32           appearanceID;           // appearnce of this actor

	//  Personality attributes
	int8            attitude,               // cooperativeness
	                mood;                   // happiness
	uint8           disposition;            // actor disposition
	// 0 = friendly, 1 = enemy,
	// 2 = Julian, 3 = Philip,
	// 4 = Kevin

	//  Character orientation
	Direction       currentFacing;          // current facing direction

	//  Tether info
	int16           tetherLocU;             // tether U coordinate
	int16           tetherLocV;             // tether V coordinate
	int16           tetherDist;             // length of tether

	//  Held objects
	ObjectID        leftHandObject,         // object held in left hand.
	                rightHandObject;        // object held in right hand.

	//  Knowledge packets
	uint16          knowledge[16];

	//  Schedule script ID
	uint16          schedule;

	//  Pad bytes
	uint8           reserved[18];

	ResourceActor(Common::SeekableReadStream *stream);
};

class Actor : public GameObject {
	friend class    ActorProto;
	friend class    MotionTask;
	friend class    Task;
	friend class    TaskStack;

public:

	//  Resource fields

	//  Social loyalty
	uint8           _faction;                // actor's faction

	//  Appearance attribute
	uint8           _colorScheme;            // indirect color map

	int32           _appearanceID;           // appearnce of this actor

	//  Personality attributes
	int8            _attitude,               // cooperativeness
	                _mood;                   // happiness
	uint8           _disposition;            // actor disposition
	// 0 = friendly, 1 = enemy,
	// 2 = Julian, 3 = Philip,
	// 4 = Kevin

	//  Character orientation
	Direction       _currentFacing;          // current facing direction

	//  Tether info
	int16           _tetherLocU;             // tether U coordinate
	int16           _tetherLocV;             // tether V coordinate
	int16           _tetherDist;             // length of tether

	//  Held objects
	ObjectID        _leftHandObject,         // object held in left hand.
	                _rightHandObject;        // object held in right hand.

	//  Knowledge packets
	uint16          _knowledge[16];

	//  Schedule script ID
	uint16          _schedule;

	//  Run-time fields

	uint8           _conversationMemory[4];// last things talked about

	//  Sprite animation variables
	uint8           _currentAnimation,       // current action sequence
	                _currentPose,            // current pose in sequence
	                _animationFlags;         // current posing flags

	//  Various actor flags
	enum {
		kAFLobotomized     = (1 << 0),
		kAFTemporary       = (1 << 1),
		kAFAfraid          = (1 << 2),
		kAFHasAssignment   = (1 << 3),
		kAFSpecialAttack   = (1 << 4),
		kAFFightStance     = (1 << 5)
	};

	uint8           _flags;

	//  Contains sprite index and positioning info for the current
	//  actor state.
	ActorPose       _poseInfo;               // current animation state

	//  Pointer to the appearance record (sprite array) for this actor.
	ActorAppearance *_appearance;            // appearance structs

	int16           _cycleCount;             // misc counter for actions
	int16           _kludgeCount;            // another misc counter

	uint32          _enchantmentFlags;       // flags indicating racial
	// abilities and enchantments

	//  Movement attributes
	MotionTask      *_moveTask;

	//  Current task
	TaskStack       *_curTask;

	//  Current goal type
	uint8           _currentGoal;

	//  Used for deltayed deactivation (and also to word-align struct)
	uint8           _deactivationCounter;

	//  Assignment
	ActorAssignment *_assignment;
	// assignments

	//  Current effective stats
	ActorAttributes _effectiveStats;

	uint8           _actionCounter;          //  coordinate moves in combat

	uint16          _effectiveResistance;    // resistances (see EFFECTS.H)
	uint16          _effectiveImmunity;      // immunities  (see EFFECTS.H)

	int16           _recPointsPerUpdate;     // fractional vitality recovery

	int16           _currentRecoveryPoints;  // fraction left from last recovery

	enum vitalityRecovery {
		kRecPointsPerVitality        = 10
	};


	Actor           *_leader;                // This actor's leader
	ObjectID        _leaderID;
	Band            *_followers;             // This actor's band of followers
	BandID          _followersID;

	ObjectID        _armorObjects[ARMOR_COUNT];    //  armor objects being worn

	GameObject      *_currentTarget;
	ObjectID        _currentTargetID;

	int16           _scriptVar[kActorScriptVars];   //  scratch variables for scripter use

	//  Member functions

private:
	//  Initialize actor record
	void init(
	    int16   protoIndex,
	    uint16  nameIndex,
	    uint16  scriptIndex,
	    int32   appearanceNum,
	    uint8   colorSchemeIndex,
	    uint8   factionNum,
	    uint8   initFlags);

public:
	//  Default constructor
	Actor();

	//  Constructor - initial actor construction
	Actor(const ResourceActor &res);

	Actor(Common::InSaveFile *in);

	//  Destructor
	~Actor();

	//  Return the number of bytes needed to archive this actor
	int32 archiveSize();

	void write(Common::MemoryWriteStreamDynamic *out);

	static Actor *newActor(
	    int16   protoNum,
	    uint16  nameIndex,
	    uint16  scriptIndex,
	    int32   appearanceNum,
	    uint8   colorSchemeIndex,
	    uint8   factionNum,
	    uint8   initFlags);

	//  Delete this actor
	void deleteActor();

private:
	//  Turn incrementally
	void turn(Direction targetDir) {
		Direction   relativeDir = (targetDir - _currentFacing) & 0x7;

		_currentFacing =
		    (relativeDir < 4
		     ?   _currentFacing + 1
		     :   _currentFacing - 1)
		    &   0x7;
	}
public:

	//  Cause the actor to stop his current motion task is he is
	//  interruptable
	void stopMoving();

	//  Cause this actor to die
	void die();

	//  Cause this actor to return from the dead
	void imNotQuiteDead();

	// makes the actor do a vitality change test
	void vitalityUpdate();

	//  Perform actor specific activation tasks
	void activateActor();

	//  Perform actor specific deactivation tasks
	void deactivateActor();

	//  De-lobotomize this actor
	void delobotomize();

	//  Lobotomize this actor
	void lobotomize();

	//  Return a pointer to the actor's current assignment
	ActorAssignment *getAssignment() {
		return  _flags & kAFHasAssignment
		        ? _assignment
		        : nullptr;
	}

	//  determine whether this actor has a specified property
	bool hasProperty(const ActorProperty &actorProp) {
		//  The function call operator is used explicitly because
		//  Visual C++ 4.0 doesn't like it otherwise.
		return actorProp.operator()(this);
	}

	//  Determine if specified point is within actor's arms' reach
	bool inReach(const TilePoint &tp);

	//  Determine if specified point is within an objects use range
	bool inUseRange(const TilePoint &tp, GameObject *obj);

	//  Determine if actor is dead
	bool isDead() {
		return _effectiveStats.vitality <= 0;
	}

	//  Determine if actor is immobile (i.e. can't walk)
	bool isImmobile();

	//  Return a pointer to this actor's currently readied offensive
	//  object
	GameObject *offensiveObject();

	//  Returns pointers to this actor's readied primary defensive object
	//  and optionally their scondary defensive object
	void defensiveObject(GameObject **priPtr, GameObject **secPtr = NULL);

	//  Returns a pointer to the object with which this actor is
	//  currently blocking, if any
	GameObject *blockingObject(Actor *attacker);

	//  Return the total used armor attributes
	void totalArmorAttributes(ArmorAttributes &armorAttribs);

	//  Determine if specified point is within actor's attack range
	bool inAttackRange(const TilePoint &tp);

	//  Attack the specified object with the currently selected weapon
	void attack(GameObject *obj);

	//  Stop any attack on the specified object
	void stopAttack(GameObject *obj);

	//  Determine if this actor can block an attack with objects
	//  currently being held
	bool canDefend();

	//  Return a numeric value which roughly estimates this actor's
	//  offensive strength
	int16 offenseScore();

	//  Return a numeric value which roughly estimates this actor's
	//  defensive strength
	int16 defenseScore();

	//  Handle the effect of a successful hit on an opponent in combat
	void handleSuccessfulStrike(GameObject *weapon) {
		weapon->proto()->applySkillGrowth(thisID());
	}

	//  Return the value of this actor's disposition
	int16 getDisposition() {
		return _disposition;
	}

	//  Give the actor a new disposition
	int16 setDisposition(int16 newDisp) {
		int16   oldDisp = _disposition;
		if (newDisp < kDispositionPlayer)
			_disposition = newDisp;
		return oldDisp;
	}

	//  Return a pointer to the effective stats
	ActorAttributes *getStats() {
		return &_effectiveStats;
	}

	//  Return a pointer to this actor's base stats
	ActorAttributes *getBaseStats();

	//  Return the color remapping table
	void getColorTranslation(ColorTable map);

	//  Determine if this actor is interruptable
	bool isInterruptable() {
		return _actionCounter == 0;
	}

	//  Determine if this actor is permanently uninterruptable
	bool isPermanentlyUninterruptable() {
		return _actionCounter == maxuint8;
	}

	//  Set the inturruptability for this actor
	void setInterruptablity(bool val) {
		_actionCounter = val ? 0 : maxuint8;
	}

	//  Set action time counter for this actor
	//  REM: the action points will eventually need to be scaled based
	//  upon enchantments and abilities
	void setActionPoints(uint8 points) {
		_actionCounter = points;
	}

	//  Drop the all of the actor's inventory
	void dropInventory();

	//  Place an object into this actor's right or left hand
	void holdInRightHand(ObjectID objID);
	void holdInLeftHand(ObjectID objID);

	//  Wear a piece of armor
	void wear(ObjectID objID, uint8 where);

	//  Update the appearance of an actor with no motion task.
	void updateAppearance(int32 deltaTime);

	//  Used To Find Wait State When Preffered Not Available
	bool setAvailableAction(int16 action1, int16 action2, int16 action3, int16 actiondefault);

	//  Set the current animation sequence that the actor is doing.
	//  Returns the number of poses in the sequence, or 0 if there
	//  are no poses in the sequence.
	int16 setAction(int16 newState, int16 flags);

	//  returns true if the action is available in the current
	//  direction.
	bool isActionAvailable(int16 newState, bool anyDir = false);

	//  Return the number of animation frames in the specified action
	//  for the specified direction
	int16 animationFrames(int16 actionType, Direction dir);

	//  Update the current animation sequence to the next frame
	bool nextAnimationFrame();

	//  calculate which sprite frames to show. Return false if
	//  sprite frames are not loaded.
	bool calcSpriteFrames();

	//  Calculate the frame list entry, given the current actor's
	//  body state, and facing direction.
//	FrameListEntry   *calcFrameState( int16 bodyState );

	//  Returns 0 if not moving, 1 if path being calculated,
	//  2 if path being followed.
	int pathFindState();

	//  High level actor behavior functions
private:
	void setGoal(uint8 newGoal);

public:
	void evaluateNeeds();

	//  Called every frame to update the state of this actor
	void updateState();

	void handleTaskCompletion(TaskResult result);
	void handleOffensiveAct(Actor *attacker);
	void handleDamageTaken(uint8 damage);
	void handleSuccessfulStrike(Actor *target, int8 damage);
	void handleSuccessfulKill(Actor *target);

private:
	static bool canBlockWith(GameObject *defenseObj, Direction relativeDir);
public:
	void evaluateMeleeAttack(Actor *attacker);

	//  Banding related functions
	void bandWith(Actor *newLeader);
	void disband();

	bool inBandingRange() {
		assert(_leader != NULL);

		return      _leader->IDParent() == IDParent()
		            && (_leader->getLocation() - getLocation()).quickHDistance()
		            <=  kBandingDist;
	}

private:
	bool addFollower(Actor *newBandMember);
	void removeFollower(Actor *bandMember);

	TaskStack *createFollowerTask(Actor *bandMember);
	uint8 evaluateFollowerNeeds(Actor *follower);

public:
	//  Knowledge-related member functions
	bool addKnowledge(uint16 kID);
	bool removeKnowledge(uint16 kID);
	void clearKnowledge();
	void useKnowledge(scriptCallFrame &scf);

	bool canSenseProtaganistIndirectly(SenseInfo &info, int16 range);
	bool canSenseSpecificActorIndirectly(
	    SenseInfo           &info,
	    int16               range,
	    Actor               *a);
	bool canSenseSpecificObjectIndirectly(
	    SenseInfo           &info,
	    int16               range,
	    ObjectID            obj);
	bool canSenseActorPropertyIndirectly(
	    SenseInfo           &info,
	    int16               range,
	    ActorPropertyID     prop);
	bool canSenseObjectPropertyIndirectly(
	    SenseInfo           &info,
	    int16               range,
	    ObjectPropertyID    prop);

	// Take mana from actor's mana pool (if possible)
	bool takeMana(ActorManaID i, int8 dMana);

	bool hasMana(ActorManaID i, int8 dMana);

	uint32 getBaseEnchantmentEffects();
	uint16 getBaseResistance();
	uint16 getBaseImmunity();
	uint16 getBaseRecovery();

	bool resists(effectResistTypes r) {
		return _effectiveResistance & (1 << r);
	}
	bool isImmuneTo(effectImmuneTypes r) {
		return _effectiveImmunity & (1 << r);
	}
	bool hasEffect(effectOthersTypes e) {
		return (_enchantmentFlags & (1 << e)) != 0;
	}

	void setResist(effectResistTypes r, bool on) {
		_effectiveResistance = on ?
		                      _effectiveResistance | (1 << r) :
		                      _effectiveResistance & ~(1 << r);
	}

	void setImmune(effectImmuneTypes r, bool on) {
		_effectiveImmunity = on ?
		                    _effectiveImmunity | (1 << r) :
		                    _effectiveImmunity & ~(1 << r);
	}

	void setEffect(effectOthersTypes e, bool on) {
		_enchantmentFlags = on ?
		                   _enchantmentFlags | (1 << e) :
		                   _enchantmentFlags & ~(1 << e);
	}

	bool makeSavingThrow();

	void setFightStance(bool val) {
		if (val)
			_flags |= kAFFightStance;
		else
			_flags &= ~kAFFightStance;
	}
};

inline bool isPlayerActor(Actor *a) {
	return a->_disposition >= kDispositionPlayer;
}

inline bool isPlayerActor(ObjectID obj) {
	return  isActor(obj)
	        &&  isPlayerActor((Actor *)GameObject::objectAddress(obj));
}

inline bool isEnemy(Actor *a) {
	return !a->isDead() && a->_disposition == kDispositionEnemy;
}

inline bool isEnemy(ObjectID obj) {
	return  isActor(obj)
	        &&  isEnemy((Actor *)GameObject::objectAddress(obj));
}

void updateActorStates();

void pauseActorStates();
void resumeActorStates();

void setCombatBehavior(bool enabled);

//  Determine if the actors are currently initialized
bool areActorsInitialized();

void clearEnchantments(Actor *a);
void addEnchantment(Actor *a, uint16 enchantmentID);

/* ============================================================================ *
   Actor factions table
 * ============================================================================ */

enum factionTallyTypes {
	kFactionNumKills = 0,            // # of times faction member killed by PC
	kFactionNumThefts,               // # of times PC steals from faction member
	kFactionNumFavors,               // accumulated by SAGA script.

	kFactionNumColumns
};

//  Get the attitude a particular faction has for a char.
int16 GetFactionTally(int faction, enum factionTallyTypes act);

//  Increment / Decrement faction attitude
//  Whenever an actor is killed, call:
//      AddFactionAttitude( actor.faction, kFactionNumKills, 1 );
//  Whenever an actor is robbed, call:
//      AddFactionAttitude( actor.faction, kFactionNumThefts, 1 );
int16 AddFactionTally(int faction, enum factionTallyTypes act, int amt);

//  Initialize the faction tally table
void initFactionTallies();

//  Save the faction tallies to a save file
void saveFactionTallies(Common::OutSaveFile *outS);

//  Load the faction tallies from a save file
void loadFactionTallies(Common::InSaveFile *in);

//  Cleanup the faction tally table
inline void cleanupFactionTallies() { /* Nothing to do */ }

class ActorManager {
public:

	enum {
		kEvalRate = 8,
		kEvalRateMask = kEvalRate - 1
	};

	Common::Array<Actor *> _actorList;

	int32 _updatesViaScript;
	int32 _baseActorIndex;
	int16 _factionTable[kMaxFactions][kFactionNumColumns];
	bool _actorStatesPaused;
	bool _combatBehaviorEnabled;

	ActorManager() {
		_updatesViaScript = 0;
		_baseActorIndex = kEvalRateMask;

		memset(_factionTable, 0, sizeof(_factionTable));

		_actorStatesPaused = false;
		_combatBehaviorEnabled = false;
	}
};

} // end of namespace Saga2

#endif
