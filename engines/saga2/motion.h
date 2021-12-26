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

#ifndef SAGA2_MOTION_H
#define SAGA2_MOTION_H

#include "saga2/actor.h"

namespace Saga2 {

class PathRequest;
struct StandingTileInfo;

extern const StaticTilePoint dirTable[];
extern const StaticTilePoint incDirTable[];


const int   gravity         = 2;
const int   walkSpeed       = 4;
const int   slowWalkSpeed   = 2;
const int   runSpeed        = 8;
const int   walkSpeedDiag   = 3;
const int   runSpeedDiag    = 6;
const int   angleThresh     = 24;

enum MotionThreadReturnValues {
	motionInterrupted,              //  MotionTask has been rudely
	//  interrupted and recycled for a new
	//  motion.

	motionStarted,                  //  The actor started moving.
	motionCompleted,                //  The actor motion completed
	//  successfully.
	motionWalkBlocked               //  The walk motion failed.
};

enum {
	moveWait            = (1 << 0),
	moveRun             = (1 << 1)
};

/* ===================================================================== *
   Utility Motion Functions
 * ===================================================================== */

void setObjectSurface(GameObject *obj, StandingTileInfo &sti);

/* ===================================================================== *
   Motion Task Class
 * ===================================================================== */

//  This class handles the movement of objects. It includes things like
//  thrown rocks, cast spells, and actors walking.
//
//  Since most things in the game aren't moving at a given point, the
//  variables for simulating motion don't need to always be present.

//  Also need to handle explosion on impact for projectiles
//  Or other special action.
//      an object can hit a regular surface
//      can hit a trip plate
//      can hit another object
//      can hit an actor
//
//      can damage actor
//      can break
//      can set off trap
//      can explode

class MotionTask {
	friend class    MotionTaskList;
	friend class    PathRequest;
	friend class    DestinationPathRequest;
	friend class    WanderPathRequest;
	friend class    Actor;
	friend void     RequestPath(MotionTask *mTask, int16 smartness);
	friend void     RequestWanderPath(MotionTask *mTask, int16 smartness);
	friend void     abortPathFind(MotionTask *mTask);

	GameObject      *object;                // the object to move
	TilePoint       velocity;               // object velocity for ballistic flight
	TilePoint       immediateLocation,      // where we are trying to get to
	                finalTarget;            // where we eventually want to get to
	int16           tetherMinU,
	                tetherMinV,
	                tetherMaxU,
	                tetherMaxV;
	uint8           motionType,             // thrown or shot.
	                prevMotionType;         // motion type before interruption

	ThreadID        thread;                 // SAGA thread to wake up when
	// motion is done

	uint16          flags;                  // various flags

	enum motionFlags {
		pathFind        = (1 << 0),         // walk is using path finding
		finalPath       = (1 << 1),         // current path is final
		inWater         = (1 << 2),         // handle motion as in water
		reset           = (1 << 3),         // target has been reset
		blocked         = (1 << 4),         // target was blocked
		requestRun      = (1 << 5),         // caller has requested running
		wandering       = (1 << 6),         // wander
		tethered        = (1 << 7),         // stay within tether
		nextAnim        = (1 << 8),         // update animation for this frame
		turnDelay       = (1 << 9),         // Delay While Turning
		TAGTarg         = (1 << 10),        // The target is not an object but a TAG
		LocTarg         = (1 << 11),        // The target is not an object but a TAG
		agitated        = (1 << 12),        // Walking around blockage
		agitatable      = (1 << 13),        // Will agitate when blocked
		onStairs        = (1 << 14),        // actor is climbing stairs
		privledged      = (1 << 15)         // don't let AI interrupt this
	};

	Direction       direction;              // direction of movement
	TilePoint       pathList[16];         // intermediate motion targets
	int16           pathCount,              // number of points in path
	                pathIndex,              // number of points so far
	                runCount;               // used for run requests.
	PathRequest     *pathFindTask;          // request to find the path
	int16           steps,                  // number of steps in ballistic motion
	                uFrac,                  // remainder in U direction
	                vFrac,                  // remainder in V direction
	                uErrorTerm,             // used to adjust for rounding errors
	                vErrorTerm;             // used to adjust for rounding errors

	//  Data used in combat motion
	uint8           combatMotionType;       // combat sub motion type

	// Spell casting stuff
	GameObject      *targetObj;             // target of attack or defense (object)
	ActiveItem      *targetTAG;             // target of attack or defense (TAG)
	Location        targetLoc;              // target of attack or defense (Location)
	SkillProto      *spellObj;              // spell being cast

	union {
		int16           actionCounter;      // counter used in some motion
		int16           moveCount;          // counter used when moving mergeable objects around
	};

	enum defenseMotionFlags {
		blocking    = (1 << 0)             // actor is blocking an attack
	};

	union {
		//  Object interaction stuff
		struct {
			GameObject      *directObject,  // object directly being acted
			                // upon.
			                *indirectObject;    // object idirectly being acted
			// upon.
			Actor           *enactor;
			ActiveItem      *TAI;           // TAI involved in interation
		} o;

		//  Defensive motion stuff
		struct {
			Actor           *attacker;      // attacking actor
			GameObject      *defensiveObj;  // shield or parrying weapon
			uint8           defenseFlags;   // various combat flags
		} d;
	};

public:
	//  Combat specific motion sub-types
	enum TwoHandedSwingTypes {
		twoHandedSwingHigh,
		twoHandedSwingLow,
		twoHandedSwingLeftHigh,
		twoHandedSwingLeftLow,
		twoHandedSwingRightHigh,
		twoHandedSwingRightLow
	};

	enum OneHandedSwingTypes {
		oneHandedSwingHigh,
		oneHandedSwingLow,
		oneHandedThrust
	};

	enum OneHandedParryTypes {
		oneHandedParryHigh,
		oneHandedParryLow
	};

private:

	enum motionTypes {
		motionTypeNone,                     // no motion

		motionTypeThrown,                   // thrown in an arc
		motionTypeShot,                     // shot in very shallow arc w/ cheat
		motionTypeFall,                     // fall from a height
		motionTypeWalk,                     // walk to a point
		motionTypeStagger,                  // stagger to a point
		motionTypeClimbUp,                  // climb up ladder to a point
		motionTypeClimbDown,                // climb dowb ladder
		motionTypeTalk,                     // talk and gesture
		motionTypeLand,                     // land after falling
		motionTypeLandBadly,                // land badly after falling
		motionTypeJump,                     // get ready for jump
		motionTypeTurn,                     // Turn Object
		motionTypeGive,                     // Extend arm to give object
		motionTypeRise,                     // Rise slowly in water
		motionTypeHit,                      // For simple animations

		//  Immobile motions
		motionTypeWait,                     // Don't move, simply eat some time
		motionTypeUseObject,                // Use an object
		motionTypeUseObjectOnObject,        // Use one object on another
		motionTypeUseObjectOnTAI,           // Use an object on a TAI
		motionTypeUseObjectOnLocation,      // Use an object on a TilePoint
		motionTypeUseTAI,                   // Use a TAI
		motionTypeDropObject,               // Drop an object at a location
		motionTypeDropObjectOnObject,       // Drop one object on another
		motionTypeDropObjectOnTAI,          // Drop an object on a TAI

		//  Offensive combat actions
		motionTypeTwoHandedSwing,           // swing two-handed weapon
		motionTypeOneHandedSwing,           // swing one-handed weapon
		motionTypeFireBow,                  // fire bow
		motionTypeCastSpell,                // cast spell
		motionTypeUseWand,                  // cast spell with wand

		//  Defensive combat actions
		motionTypeTwoHandedParry,           // parry with two-handed weapon
		motionTypeOneHandedParry,           // parry with one-handed weapon
		motionTypeShieldParry,              // parry with shield
		motionTypeDodge,                    // dodge blow

		//  Other combat actions
		motionTypeAcceptHit,                // show effect of hit
		motionTypeFallDown,                 // be knocked off feet
		motionTypeDie                       // self-explanatory

	};

	void read(Common::InSaveFile *in);

	//  Return the number of bytes needed to archive this MotionTask
	int32 archiveSize();

	void write(Common::MemoryWriteStreamDynamic *out);

	// motion task is finished.
	void remove(int16 returnVal = motionInterrupted);

	TilePoint getImmediateTarget();      // determine immediate target
	// location

	//  Routines to handle updating of specific motion types
	void turnAction() {
		Actor   *a = (Actor *)object;

		if (flags & reset) {
			a->setAction(actionStand, 0);
			flags &= ~reset;
		}

		if (a->_currentFacing != direction)
			a->turn(direction);
		else
			remove(motionCompleted);
	}

	void ballisticAction();
	void walkAction();
	void giveAction();

	void upLadderAction();
	void downLadderAction();

	//  Set up specified animation and run through the frames
	void genericAnimationAction(uint8 actionType);

	//  Offensive combat actions
	void twoHandedSwingAction();
	void oneHandedSwingAction();
	void fireBowAction();
	void castSpellAction();
	void useWandAction();

	//  Defensive combat actions
	void twoHandedParryAction();
	void oneHandedParryAction();
	void shieldParryAction();
	void dodgeAction();

	//  Other combat actions
	void acceptHitAction();
	void fallDownAction();

	//  Generic offensive melee code.  Called by twoHandedSwingAction()
	//  and oneHandedSwingAction
	void offensiveMeleeAction();

	//  Generic magic weapon code.  Called by useWandAction() and
	//  useStaffAction()
	void useMagicWeaponAction();

	//  Generic defensive melee code.  Called by twoHandedParryAction(),
	//  oneHandedParryAction() and shieldParryAction().
	void defensiveMeleeAction();

	//  Retrieve the next waypoint from the path list.
	bool nextWayPoint();
	bool checkWalk(int16, int16, int16, TilePoint &);

	//  Determine the velocity for a ballistic motion
	void calcVelocity(const TilePoint &vector, int16 turns);

public:

	//  Functions to create a new motion task. If the game object is
	//  an actor, then it will cancel any existing motion task
	static void throwObject(GameObject &obj, const TilePoint &velocity);
	static void throwObjectTo(GameObject &obj, const TilePoint &where);
	static void shootObject(
	    GameObject  &obj,
	    Actor       &doer,
	    GameObject  &target,
	    int16       speed);
	static void walkTo(
	    Actor           &obj,
	    const TilePoint &target,
	    bool            run = false,
	    bool            canAgitate = true);
	static void walkTo(
	    ThreadID        th,
	    Actor           &obj,
	    const TilePoint &target,
	    bool            run = false,
	    bool            canAgitate = true);
	static void walkToDirect(
	    Actor           &obj,
	    const TilePoint &target,
	    bool            run = false,
	    bool            canAgitate = true);
	static void walkToDirect(
	    ThreadID        th,
	    Actor           &obj,
	    const TilePoint &target,
	    bool            run = false,
	    bool            canAgitate = true);
	static void wander(Actor &obj, bool run = false);
	static void tetheredWander(Actor &obj, const TileRegion &tether, bool run = false);
//	static void runTo( GameObject &obj, const TilePoint &target );
	static void staggerTo(Actor &obj, const TilePoint &target);
	static void upLadder(Actor &obj);
	static void downLadder(Actor &obj);
	static void talk(Actor &obj);
	static void jump(Actor &obj);
	static void turn(Actor &obj, Direction dir);
	static void turn(ThreadID th, Actor &obj, Direction dir);
	static void turnTowards(Actor &obj, const TilePoint &where);
	static void turnTowards(ThreadID th, Actor &obj, const TilePoint &where);
	static void give(Actor &obj, Actor &givee);
	static void give(ThreadID th, Actor &obj, Actor &givee);

	// Imobile motions
	static void wait(Actor &a);
	static void useObject(Actor &a, GameObject &dObj);
	static void useObjectOnObject(
	    Actor       &a,
	    GameObject  &dObj,
	    GameObject  &target);
	static void useObjectOnTAI(
	    Actor       &a,
	    GameObject  &dObj,
	    ActiveItem  &target);
	static void useObjectOnLocation(
	    Actor           &a,
	    GameObject      &dObj,
	    const Location &target);
	static void useTAI(Actor &a, ActiveItem &dTAI);
	static void dropObject(
	    Actor           &a,
	    GameObject      &dObj,
	    const Location  &loc,
	    int16           num = 1);

	static void dropObjectOnObject(
	    Actor       &a,
	    GameObject  &dObj,
	    GameObject  &target,
	    int16       num = 1);

	static void dropObjectOnTAI(
	    Actor           &a,
	    GameObject      &dObj,
	    ActiveItem      &target,
	    const Location  &loc);

	//  Offensive combat actions
	static void twoHandedSwing(Actor &obj, GameObject &target);
	static void oneHandedSwing(Actor &obj, GameObject &target);
	static void fireBow(Actor &obj, GameObject &target);
	static void castSpell(Actor &obj, SkillProto &spell, GameObject &target);
	static void castSpell(Actor &obj, SkillProto &spell, Location &target);
	static void castSpell(Actor &obj, SkillProto &spell, ActiveItem &target);
	static void useWand(Actor &obj, GameObject &target);

	//  Defensive combat actions
	static void twoHandedParry(
	    Actor       &obj,
	    GameObject  &weapon,
	    Actor       &opponent);
	static void oneHandedParry(
	    Actor       &obj,
	    GameObject  &weapon,
	    Actor       &opponent);
	static void shieldParry(
	    Actor       &obj,
	    GameObject  &shield,
	    Actor       &opponent);
	static void dodge(Actor &obj, Actor &opponent);

	//  Other combat actions
	static void acceptHit(Actor &obj, Actor &opponent);
	static void fallDown(Actor &obj, Actor &opponent);
	static void die(Actor &obj);

	static void updatePositions();

	int16 testCollision(GameObject &obstacle);

	bool freeFall(TilePoint &newPos, StandingTileInfo &sti);

	//  Determine if the motion task is a walk motion
	bool isWalk();

	//  Determine if the motion task is walking to a destination
	bool isWalkToDest() {
		return isWalk() && !(flags & wandering);
	}

	//  Determine if the motion task is a wandering motion
	bool isWander() {
		return isWalk() && (flags & wandering);
	}

	//  Determine if the motion task is tethered
	bool isTethered() {
		return isWander() && (flags & tethered);
	}

	bool isRunning() {
		return (flags & requestRun) && runCount == 0;
	}

	bool isTurn() {
		return motionType == motionTypeTurn;
	}

	//  Return the wandering tether region
	TileRegion getTether();

	//  Return the final target location
	TilePoint getTarget() {
		return finalTarget;
	}

	//  Update to a new final target
	void changeTarget(const TilePoint &newPos, bool run = false);
	void changeDirectTarget(const TilePoint &newPos, bool run = false);
	void finishWalk();                   // stop walking
	void finishTurn() {
		if (isTurn()) remove();
	}
	void finishTalking();                    // stop talking motion

	//  Determine if this MotionTask is a reflexive motion
	bool isReflex();

	//  Determine if this MotionTask is a defensive motion
	bool isDefense();

	//  End the defensive motion task
	void finishDefense() {
		if (isDefense()) remove();
	}

	//  Determine if this MotionTask is an offensive motion
	bool isAttack();

	//  Determine if this MotionTask is an offensive melee motion
	bool isMeleeAttack();

	//  Compute the number of frames before the actual strike in an
	//  offensive melee motion
	uint16 framesUntilStrike();

	//  End the offensive motion
	void finishAttack() {
		if (isAttack()) remove();
	}

	//  Returns a pointer to the blocking object if it applicable to
	//  this motion task
	GameObject *blockingObject(Actor *thisAttacker);

	//  Determine if this motion is a dodge motion
	bool isDodging(Actor *thisAttacker) {
		return motionType == motionTypeDodge && thisAttacker == d.attacker;
	}

	static void initMotionTasks();

	bool isPrivledged() {
		return flags & privledged;
	}
};

class MotionTaskList {
	friend class    MotionTask;

	Common::List<MotionTask *> _list;
	Common::List<MotionTask *>::iterator _nextMT;

public:
	//  Default constructor
	MotionTaskList();

	MotionTaskList(Common::SeekableReadStream *stream);

	void read(Common::InSaveFile *in);

	//  Return the number of bytes needed to archive the motion tasks
	//  in a buffer
	int32 archiveSize();

	void write(Common::MemoryWriteStreamDynamic *out);

	//  Cleanup the motion tasks
	void cleanup();

	MotionTask *newTask(GameObject *obj);    // get new motion task
};

/* ===================================================================== *
    MotionTask inline member functions
 * ===================================================================== */

inline void MotionTask::walkTo(
    ThreadID        th,
    Actor           &actor,
    const TilePoint &target,
    bool            run,
    bool            canAgitate) {
	walkTo(actor, target, run, canAgitate);
	if (actor._moveTask != NULL)
		actor._moveTask->thread = th;
}

inline void MotionTask::walkToDirect(
    ThreadID        th,
    Actor           &actor,
    const TilePoint &target,
    bool            run,
    bool            canAgitate) {
	walkToDirect(actor, target, run, canAgitate);
	if (actor._moveTask != NULL)
		actor._moveTask->thread = th;
}

inline void MotionTask::turn(ThreadID th, Actor &actor, Direction dir) {
	turn(actor, dir);
	if (actor._moveTask != NULL)
		actor._moveTask->thread = th;
}

inline void MotionTask::turnTowards(
    ThreadID        th,
    Actor           &actor,
    const TilePoint &where) {
	turnTowards(actor, where);
	if (actor._moveTask != NULL)
		actor._moveTask->thread = th;
}

inline void MotionTask::give(ThreadID th, Actor &actor, Actor &givee) {
	give(actor, givee);
	if (actor._moveTask != NULL)
		actor._moveTask->thread = th;
}

/* ===================================================================== *
   Utility functions
 * ===================================================================== */

//  Initiate ladder climbing
bool checkLadder(Actor *a, const TilePoint &tp);

void pauseInterruptableMotions();
void resumeInterruptableMotions();

/* ===================================================================== *
   MotionTask list management functions
 * ===================================================================== */

//  Initialize the motion task list
void initMotionTasks();

void saveMotionTasks(Common::OutSaveFile *out);
void loadMotionTasks(Common::InSaveFile *in, int32 chunkSize);

//  Cleanup the motion task list
void cleanupMotionTasks();

} // end of namespace Saga2

#endif
