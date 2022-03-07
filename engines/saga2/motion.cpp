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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/dispnode.h"
#include "saga2/tile.h"
#include "saga2/motion.h"
#include "saga2/tilemode.h"
#include "saga2/magic.h"
#include "saga2/spellbuk.h"
#include "saga2/contain.h"
#include "saga2/intrface.h"

namespace Saga2 {

//  Turns on visual debugging aids
#define VISUAL1     0

/* ===================================================================== *
   Globals
 * ===================================================================== */

bool    interruptableMotionsPaused;

/* ===================================================================== *
   Test Functions
 * ===================================================================== */

bool unstickObject(GameObject *obj);
int32 currentGamePerformance();

/* ===================================================================== *
   Functions
 * ===================================================================== */

/*  Different motion types we want to simulate:

    Transient motions
    ~~~~~~~~~~~~~~~~~
x       Walk to Point
        Walk to Object (moving)
x       Run
        Ballistic Motion
            Jump up (vertical motion only)
            Leap to X distance
            Running Leap
x           Fall off of cliff or height
            Object Ballistic Motion
        Climb
            Climb up ladder
            Climb up Rope
            Climb up Ledge
        Talk & Gesture
        Give Item (requires cooperation)
        Stoop to pick up item
        Stoop to dodge
        Fight
            Cast Magic spell
            Swing High
            Swing Low
            Parry
            Lunge
            Shoot Bow
        Consume Food
x       Cycle through arbitrary frames
x           Forward
x           Backward
x           Random
x           Ping-Pong
x           Looped
x           Single
        Die

    Persistent Motions
    ~~~~~~~~~~~~~~~~~~
        Wait cycle / Twitch (not for FTA, but eventually)
        Be Dead
        Sleep
        Sit
*/


/* ===================================================================== *
    Motion Constants
 * ===================================================================== */

const StaticTilePoint dirTable[8] = {
	{ 2,  2, 0},
	{ 0,  3, 0},
	{-2,  2, 0},
	{-3,  0, 0},
	{-2, -2, 0},
	{ 0, -3, 0},
	{ 2, -2, 0},
	{ 3,  0, 0}
};

//  Incremental direction table
const StaticTilePoint incDirTable[8] = {
	{ 1,  1, 0},
	{ 0,  1, 0},
	{-1,  1, 0},
	{-1,  0, 0},
	{-1, -1, 0},
	{ 0, -1, 0},
	{ 1, -1, 0},
	{ 1,  0, 0}
};

extern uint16   uMaxMasks[4],
       uMinMasks[4],
       vMaxMasks[4],
       vMinMasks[4];

extern SpellStuff *spellBook;

/* ===================================================================== *
   PathMinder
 * ===================================================================== */

int32 getPathFindIQ(GameObject *obj) {
	int32 pfIQ = 50;
	if (isActor(obj)) {
		Actor   *a = (Actor *)obj;

		if (a == getCenterActor())
			pfIQ = 400;
		else if (isPlayerActor(a))
			pfIQ = 300;
		else {
			if (objRoofRipped(obj))
				pfIQ = 75;
			else if (a->_disposition == 1)
				pfIQ = 250;
			else
				pfIQ = 100;
			if (g_vm->_rnd->getRandomNumber(9) == 5)
				pfIQ += 200;

		}
		int32 p = clamp(50, currentGamePerformance(), 200);
		pfIQ = (pfIQ * p) / 200;
	}



	return pfIQ;
}

/* ===================================================================== *
   Utility functions
 * ===================================================================== */

//  This subroutine detects if the actor has landed on an active
//  tile, and checks to see if the active tile's script should
//  be triggered.

void setObjectSurface(GameObject *obj, StandingTileInfo &sti) {
	ActiveItemID    tagID = sti.surfaceTAG != nullptr
	                        ?   sti.surfaceTAG->thisID()
	                        :   NoActiveItem;

	if (!(sti.surfaceRef.flags & trTileSensitive))
		tagID = NoActiveItem;

	if (obj->_data.currentTAG != tagID) {
		ObjectID    objID = obj->thisID(),
		            enactorID = isActor(objID) ? objID : Nothing;

		if (obj->_data.currentTAG != NoActiveItem) {
			ActiveItem  *oldTAG =
			    ActiveItem::activeItemAddress(obj->_data.currentTAG);

			oldTAG->release(enactorID, objID);

			obj->_data.currentTAG = NoActiveItem;
		}

		if (tagID != NoActiveItem) {
			if (sti.surfaceTAG->trigger(enactorID, objID))
				obj->_data.currentTAG = tagID;
		}
	}
}

inline int16 spinLeft(int16 dir, int16 amt = 1) {
	return (dir + amt) & 7;
}
inline int16 spinRight(int16 dir, int16 amt = 1) {
	return (dir - amt) & 7;
}

//  Special code to avoid actors sticking in walls, which occasionally
//  happens due to the point-sampled nature of the environment.

bool unstickObject(GameObject *obj) {
	assert(isObject(obj) || isActor(obj));

	TilePoint       pos;
	int16           mapNum;
	bool            outside;

	mapNum = obj->getMapNum();
	outside = objRoofID(obj, mapNum, obj->getLocation()) == 0;

	if (checkBlocked(obj, obj->getLocation()) == blockageNone)
		return false;

#if 1
#if DEBUG
	WriteStatusF(9, "Unsticking");
#endif
	//  A stochastic unsticker, written by Talin
	//  Basically, it tightens the constraints each time a solution
	//  is found.
	int32           radius = 256;
	int16           objZ = obj->getLocation().z;
	TilePoint       bestPos;

	for (int tries = 128; tries >= 0; tries--) {
		int32       dx = g_vm->_rnd->getRandomNumber(radius * 2) - radius,
		            dy = g_vm->_rnd->getRandomNumber(radius * 2) - radius,
		            dz = g_vm->_rnd->getRandomNumber(radius * 2) - radius;
		int16       tHeight;

		//  Compute the actual _data.location of the new point
		pos = obj->getLocation() + TilePoint(dx, dy, dz);

		//  Get the surface height at that point
		tHeight = tileSlopeHeight(pos, obj);

		//  If the surface height is too far away from the sample
		//  height, then ignore it.
		if (tHeight > pos.z + kMaxStepHeight
		        ||  tHeight < pos.z - kMaxStepHeight * 4) continue;

		//  Recompute the coordinate
		dz = tHeight - objZ;

		//  If under the same roof, and no blockages...

		if (outside == (objRoofID(obj, mapNum, pos) == 0)
		        &&  checkBlocked(obj, pos) == blockageNone) {
			int32   newRadius;

			//  Then this is the best one found so far.

			//  Set new radius to maximum of abs of the 3 coords, minus 1
			//  (Because we want solution to converge faster)
			newRadius = MAX(MAX(ABS(dx), ABS(dy)), ABS(dz)) - 1;
			if (newRadius < radius) {
				radius = newRadius;

				//  Each time radius gets reduced, we try a few more times
				//  to find a better solution.
				tries = radius * 2 + 8;
			}

			pos.z = tHeight;
			bestPos = pos;
		}
	}

	if (radius < 128) {
#if DEBUG
		WriteStatusF(9, "Unstick Dist: %d", radius);
#endif
		obj->move(bestPos);
		return true;
	}

#else
	for (dist = 4; dist < 64; dist += 4) {
		int         level;

		for (level = 0; level < dist / 2; level++) {
			bool    up = (level & 1) == 0;

			height = 8 * (up ? level >> 1 : -1 - (level >> 1));

			for (dir = 0; dir < 8; dir++) {
				pos = obj->getLocation() + (dirTable[dir] * dist);
				pos.z += height;

				if (outside == (objRoofID(obj, mapNum, pos) == 0)
				        &&  checkBlocked(obj, pos) == blockageNone) {
					int16       tHeight;

					tHeight = tileSlopeHeight(pos, obj);
					if (tHeight <= pos.z + kMaxStepHeight
					        &&  tHeight >= pos.z - kMaxStepHeight * 4) {
						pos.z = tHeight;
						obj->move(pos);
						return true;
					}
				}
			}
		}
	}
#endif
#if DEBUG
	WriteStatusF(9, "Unstick Failed!");
#endif
	return true;
}

//  Calculates the direction of a missile based upon the velocity vector
uint8 missileDir(const TilePoint &vector) {
	return (((ptToAngle(vector.u, vector.v) + 8) >> 4) - 2) & 0xF;
}

//  Computes the frames needed to turn from one direction to another
uint8 computeTurnFrames(Direction fromDir, Direction toDir) {
	Direction       relDir = (toDir - fromDir) & 0x7;

	return relDir <= 4 ? relDir : 8 - relDir;
}

/* ===================================================================== *
   MotionTaskList member functions
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Initialize the MotionTaskList

MotionTaskList::MotionTaskList() {
	_nextMT = _list.end();
}

MotionTaskList::MotionTaskList(Common::SeekableReadStream *stream) {
	read(stream);
	_nextMT = _list.end();
}


void MotionTaskList::read(Common::InSaveFile *in) {
	int16 motionTaskCount;
	//  Retrieve the motion task count
	motionTaskCount = in->readSint16LE();

	for (int i = 0; i < motionTaskCount; i++) {
		MotionTask *mt;

		mt = new MotionTask;
		_list.push_back(mt);

		mt->read(in);
	}
}

//-----------------------------------------------------------------------
//	Return the number of bytes needed to archive the motion tasks
//	in a buffer

int32 MotionTaskList::archiveSize() {
	//  Initilialize with sizeof motion task count
	int32       size = sizeof(int16);

	for (Common::List<MotionTask *>::iterator it = _list.begin(); it != _list.end(); ++it)
		size += (*it)->archiveSize();

	return size;
}

void MotionTaskList::write(Common::MemoryWriteStreamDynamic *out) {
	int16 motionTaskCount = _list.size();

	//  Store the motion task count
	out->writeSint16LE(motionTaskCount);

	//  Archive the active motion tasks
	for (Common::List<MotionTask *>::iterator it = _list.begin(); it != _list.end(); ++it)
		(*it)->write(out);
}

//-----------------------------------------------------------------------
//	Cleanup the motion tasks

void MotionTaskList::cleanup() {
	for (Common::List<MotionTask *>::iterator it = _list.begin(); it != _list.end(); ++it) {
		abortPathFind(*it);
		(*it)->pathFindTask = nullptr;

		delete *it;
	}

	_list.clear();
}

//-----------------------------------------------------------------------
//	Get a new motion task, if there is one available, and initialize it.

MotionTask *MotionTaskList::newTask(GameObject *obj) {
	MotionTask *mt = nullptr;

	//  Check see if there's already motion associated with this object.
	for (Common::List<MotionTask *>::iterator it = _list.begin(); it != _list.end(); ++it) {

		if ((*it)->object == obj) {
			mt = *it;
			wakeUpThread(mt->thread, motionInterrupted);
			mt->thread = NoThread;

			break;
		}
	}

	if (mt == nullptr) {
		mt = new MotionTask;

		mt->object = obj;
		mt->motionType = mt->prevMotionType = MotionTask::motionTypeNone;
		mt->pathFindTask = nullptr;
		mt->pathCount = -1;
		mt->flags = 0;
		mt->velocity = TilePoint(0, 0, 0);
		mt->immediateLocation = mt->finalTarget = obj->getLocation();
		mt->thread = NoThread;

		mt->targetObj = nullptr;
		mt->targetTAG = nullptr;
		mt->spellObj = nullptr;

		_list.push_back(mt);

		if (isActor(obj))
			((Actor *)obj)->_moveTask = mt;
	}

	obj->_data.objectFlags |= objectMoving;

	return mt;
}

/* ===================================================================== *
   MotionTask member functions
 * ===================================================================== */

void MotionTask::read(Common::InSaveFile *in) {
	ObjectID    objectID;

	//  Restore the motion type and previous motion type
	motionType = in->readByte();
	prevMotionType = in->readByte();

	//  Restore the thread ID
	thread = in->readSint16LE();

	//  Restore the motion flags
	flags = in->readUint16LE();

	//  Get the object ID
	objectID = in->readUint16LE();

	//  Convert the object ID to and object address
	object =    objectID != Nothing
	            ?   GameObject::objectAddress(objectID)
	            :   nullptr;

	//  If the object is an actor, plug this motion task into the actor
	if (object && isActor(object))
		((Actor *)object)->_moveTask = this;

	if (motionType == motionTypeWalk
	        ||  prevMotionType == motionTypeWalk) {
		//  Restore the target _data.locations
		immediateLocation.load(in);
		finalTarget.load(in);

		//  If there is a tether restore it
		if (flags & tethered) {
			tetherMinU = in->readSint16LE();
			tetherMinV = in->readSint16LE();
			tetherMaxU = in->readSint16LE();
			tetherMaxV = in->readSint16LE();
		}

		//  Restore the direction
		direction = in->readByte();

		//  Restore the path index and path count
		pathIndex = in->readSint16LE();
		pathCount = in->readSint16LE();
		runCount = in->readSint16LE();

		//  Restore the action counter if needed
		if (flags & agitated)
			actionCounter = in->readSint16LE();

		//  If there were valid path way points, restore those
		if (pathIndex >= 0 && pathIndex < pathCount) {
			int16 wayPointIndex = pathIndex;

			while (wayPointIndex < pathCount) {
				pathList[wayPointIndex].load(in);

				wayPointIndex++;
			}
		}

		//  If this motion task previously had a path finding request
		//  it must be restarted
		pathFindTask = nullptr;
	}

	if (motionType == motionTypeThrown || motionType == motionTypeShot) {
		//  Restore the velocity
		velocity.load(in);

		//  Restore other ballistic motion variables
		steps = in->readSint16LE();
		uFrac = in->readSint16LE();
		vFrac = in->readSint16LE();
		uErrorTerm = in->readSint16LE();
		vErrorTerm = in->readSint16LE();

		if (motionType == motionTypeShot) {
			ObjectID targetObjID,
			         enactorID;

			targetObjID = in->readUint16LE();

			targetObj = targetObjID
			            ?   GameObject::objectAddress(targetObjID)
			            :   nullptr;

			enactorID = in->readUint16LE();

			o.enactor = enactorID != Nothing
			            ? (Actor *)GameObject::objectAddress(enactorID)
			            :   nullptr;
		}
	} else if (motionType == motionTypeClimbUp
	           ||  motionType == motionTypeClimbDown) {
		immediateLocation.load(in);
	} else if (motionType == motionTypeJump) {
		velocity.load(in);
	} else if (motionType == motionTypeTurn) {
		direction = in->readByte();
	} else if (motionType == motionTypeGive) {
		ObjectID id = in->readUint16LE();
		targetObj = id != Nothing
		            ?   GameObject::objectAddress(id)
		            :   nullptr;
	} else if (motionType == motionTypeWait) {
		actionCounter = in->readSint16LE();
	} else if (motionType == motionTypeUseObject
	           ||  motionType == motionTypeUseObjectOnObject
	           ||  motionType == motionTypeUseObjectOnTAI
	           ||  motionType == motionTypeUseObjectOnLocation
	           ||  motionType == motionTypeDropObject
	           ||  motionType == motionTypeDropObjectOnObject
	           ||  motionType == motionTypeDropObjectOnTAI) {
	    ObjectID directObjID = in->readUint16LE();
		o.directObject = directObjID != Nothing
		                ?   GameObject::objectAddress(directObjID)
		                :   nullptr;

		direction = in->readByte();

		if (motionType == motionTypeUseObjectOnObject
		        ||  motionType == motionTypeDropObjectOnObject) {
		    ObjectID indirectObjID = in->readUint16LE();
			o.indirectObject =  indirectObjID != Nothing
			                    ?   GameObject::objectAddress(indirectObjID)
			                    :   nullptr;
		} else {
			if (motionType == motionTypeUseObjectOnTAI
			        ||  motionType == motionTypeDropObjectOnTAI) {
			    ActiveItemID tai(in->readSint16LE());
				o.TAI = tai != NoActiveItem
				        ?   ActiveItem::activeItemAddress(tai)
				        :   nullptr;
			}

			if (motionType == motionTypeUseObjectOnLocation
			        ||  motionType == motionTypeDropObject
			        ||  motionType == motionTypeDropObjectOnTAI) {
				targetLoc.load(in);
			}
		}
	} else if (motionType == motionTypeUseTAI) {
		ActiveItemID tai(in->readSint16LE());
		o.TAI = tai != NoActiveItem
		        ?   ActiveItem::activeItemAddress(tai)
		        :   nullptr;

		direction = in->readByte();
	} else if (motionType == motionTypeTwoHandedSwing
	           ||  motionType == motionTypeOneHandedSwing
	           ||  motionType == motionTypeFireBow
	           ||  motionType == motionTypeCastSpell
	           ||  motionType == motionTypeUseWand) {
		ObjectID    targetObjID;

		//  Restore the direction
		direction = in->readByte();

		//  Restore the combat motion type
		combatMotionType = in->readByte();

		//  Get the target object ID
		targetObjID = in->readUint16LE();

		//  Convert the target object ID to a pointer
		targetObj = targetObjID != Nothing
		            ?   GameObject::objectAddress(targetObjID)
		            :   nullptr;

		if (motionType == motionTypeCastSpell) {
			SpellID sid       ;
			ObjectID toid     ;
			ActiveItemID ttaid;

			//  restore the spell prototype
			warning("MotionTask::read: Check SpellID size");
			sid = (SpellID)in->readUint32LE();
			spellObj = sid != nullSpell
			           ? skillProtoFromID(sid)
			           : nullptr;

			//  restore object target
			toid = in->readUint16LE();
			targetObj = toid != Nothing
			            ?   GameObject::objectAddress(toid)
			            :   nullptr;

			//  restore TAG target
			ttaid = in->readSint16LE();
			targetTAG = ttaid != NoActiveItem
			            ?  ActiveItem::activeItemAddress(ttaid)
			            :  nullptr;

			//  restore _data.location target
			targetLoc.load(in);
		}

		//  Restore the action counter
		actionCounter = in->readSint16LE();
	} else if (motionType == motionTypeTwoHandedParry
	           ||  motionType == motionTypeOneHandedParry
	           ||  motionType == motionTypeShieldParry) {
		ObjectID attackerID,
		         defensiveObjID;

		//  Restore the direction
		direction = in->readByte();

		//  Get the attacker's and defensive object's IDs
		attackerID = in->readByte();
		defensiveObjID = in->readByte();

		//  Convert IDs to pointers
		d.attacker = attackerID != Nothing
		            ? (Actor *)GameObject::objectAddress(attackerID)
		            :   nullptr;

		d.defensiveObj = defensiveObjID != Nothing
		                ?   GameObject::objectAddress(defensiveObjID)
		                :   nullptr;

		//  Restore the defense flags
		d.defenseFlags = in->readByte();

		//  Restore the action counter
		actionCounter = in->readSint16LE();

		if (motionType == motionTypeOneHandedParry) {
			//  Restore the combat sub-motion type
			combatMotionType = in->readByte();
		}
	} else if (motionType == motionTypeDodge
	           ||  motionType == motionTypeAcceptHit
	           ||  motionType == motionTypeFallDown) {
		ObjectID        attackerID;

		//  Get the attacker's ID
		attackerID = in->readUint16LE();

		//  Convert ID to pointer
		d.attacker = attackerID != Nothing
		            ? (Actor *)GameObject::objectAddress(attackerID)
		            :   nullptr;

		//  Restore the action counter
		actionCounter = in->readSint16LE();
	}
}

//-----------------------------------------------------------------------
//	Return the number of bytes needed to archive this MotionTask

int32 MotionTask::archiveSize() {
	int32       size = 0;

	size =      sizeof(motionType)
	            +   sizeof(prevMotionType)
	            +   sizeof(thread)
	            +   sizeof(flags)
	            +   sizeof(ObjectID);            //  object

	if (motionType == motionTypeWalk
	        ||  prevMotionType == motionTypeWalk) {
		size +=     sizeof(immediateLocation)
		            +   sizeof(finalTarget);

		if (flags & tethered) {
			size +=     sizeof(tetherMinU)
			            +   sizeof(tetherMinV)
			            +   sizeof(tetherMaxU)
			            +   sizeof(tetherMaxV);
		}

		size +=     sizeof(direction)
		            +   sizeof(pathIndex)
		            +   sizeof(pathCount)
		            +   sizeof(runCount);

		if (flags & agitated)
			size += sizeof(actionCounter);

		if (pathIndex >= 0 && pathIndex < pathCount)
			size += sizeof(TilePoint) * (pathCount - pathIndex);
	}

	if (motionType == motionTypeThrown || motionType == motionTypeShot) {
		size +=     sizeof(velocity)
		            +   sizeof(steps)
		            +   sizeof(uFrac)
		            +   sizeof(vFrac)
		            +   sizeof(uErrorTerm)
		            +   sizeof(vErrorTerm);

		if (motionType == motionTypeShot) {
			size +=     sizeof(ObjectID)         //  targetObj ID
			            +   sizeof(ObjectID);        //  enactor ID
		}
	} else if (motionType == motionTypeClimbUp
	           ||  motionType == motionTypeClimbDown) {
		size += sizeof(immediateLocation);
	} else if (motionType == motionTypeJump) {
		size += sizeof(velocity);
	} else if (motionType == motionTypeTurn) {
		size += sizeof(direction);
	} else if (motionType == motionTypeGive) {
		size += sizeof(ObjectID);        //  targetObj ID
	} else if (motionType == motionTypeUseObject
	           ||  motionType == motionTypeUseObjectOnObject
	           ||  motionType == motionTypeUseObjectOnTAI
	           ||  motionType == motionTypeUseObjectOnLocation
	           ||  motionType == motionTypeDropObject
	           ||  motionType == motionTypeDropObjectOnObject
	           ||  motionType == motionTypeDropObjectOnTAI) {
		size +=     sizeof(ObjectID)
		            +   sizeof(direction);

		if (motionType == motionTypeUseObjectOnObject
		        ||  motionType == motionTypeDropObjectOnObject) {
			size += sizeof(ObjectID);
		} else {
			if (motionType == motionTypeUseObjectOnTAI
			        ||  motionType == motionTypeDropObjectOnTAI) {
				size += sizeof(ActiveItemID);
			}

			if (motionType == motionTypeUseObjectOnLocation
			        ||  motionType == motionTypeDropObject
			        ||  motionType == motionTypeDropObjectOnTAI) {
				size += sizeof(targetLoc);
			}
		}
	} else if (motionType == motionTypeUseTAI) {
		size +=     sizeof(ActiveItemID)
		            +   sizeof(direction);
	} else if (motionType == motionTypeTwoHandedSwing
	           ||  motionType == motionTypeOneHandedSwing
	           ||  motionType == motionTypeFireBow
	           ||  motionType == motionTypeCastSpell
	           ||  motionType == motionTypeUseWand) {
		size +=     sizeof(direction)
		            +   sizeof(combatMotionType)
		            +   sizeof(ObjectID);            //  targetObj

		if (motionType == motionTypeCastSpell) {
			size += sizeof(SpellID);             //  spellObj
			size += sizeof(ObjectID);            //  targetObj
			size += sizeof(ActiveItemID);        //  targetTAG
			size += sizeof(targetLoc);           //  targetLoc
		}

		size +=     sizeof(actionCounter);

	} else if (motionType == motionTypeTwoHandedParry
	           ||  motionType == motionTypeOneHandedParry
	           ||  motionType == motionTypeShieldParry) {
		size +=     sizeof(direction)
		            +   sizeof(ObjectID)             //  attacker ID
		            +   sizeof(ObjectID)             //  defensiveObj ID
		            +   sizeof(d.defenseFlags)
		            +   sizeof(actionCounter);

		if (motionType == motionTypeOneHandedParry)
			size += sizeof(combatMotionType);
	} else if (motionType == motionTypeDodge
	           ||  motionType == motionTypeAcceptHit
	           ||  motionType == motionTypeFallDown) {
		size +=     sizeof(ObjectID)             //  attacker ID
		            +   sizeof(actionCounter);
	}

	return size;
}

void MotionTask::write(Common::MemoryWriteStreamDynamic *out) {
	ObjectID    objectID;

	//  Store the motion type and previous motion type
	out->writeByte(motionType);
	out->writeByte(prevMotionType);

	//  Store the thread ID
	out->writeSint16LE(thread);

	//  Store the motion flags
	out->writeUint16LE(flags);

	//  Convert the object pointer to an object ID
	objectID = object != nullptr ? object->thisID() : Nothing;

	//  Store the object ID
	out->writeUint16LE(objectID);

	if (motionType == motionTypeWalk
	        ||  prevMotionType == motionTypeWalk) {
		//  Store the target _data.locations
		immediateLocation.write(out);
		finalTarget.write(out);

		//  If there is a tether store it
		if (flags & tethered) {
			out->writeSint16LE(tetherMinU);
			out->writeSint16LE(tetherMinV);
			out->writeSint16LE(tetherMaxU);
			out->writeSint16LE(tetherMaxV);
		}

		//  Store the direction
		out->writeByte(direction);

		//  Store the path index and path count
		out->writeSint16LE(pathIndex);
		out->writeSint16LE(pathCount);
		out->writeSint16LE(runCount);

		//  Store the action counter if needed
		if (flags & agitated)
			out->writeSint16LE(actionCounter);

		//  If there are valid path way points, store them
		if (pathIndex >= 0 && pathIndex < pathCount) {
			int16   wayPointIndex = pathIndex;

			while (wayPointIndex < pathCount) {
				pathList[wayPointIndex].write(out);

				wayPointIndex++;
			}
		}
	}

	if (motionType == motionTypeThrown || motionType == motionTypeShot) {
		//  Store the velocity
		velocity.write(out);

		//  Store other ballistic motion variables
		out->writeSint16LE(steps);
		out->writeSint16LE(uFrac);
		out->writeSint16LE(vFrac);
		out->writeSint16LE(uErrorTerm);
		out->writeSint16LE(vErrorTerm);

		if (motionType == motionTypeShot) {
			ObjectID        targetObjID,
			                enactorID;

			targetObjID =   targetObj != nullptr
			                ?   targetObj->thisID()
			                :   Nothing;

			out->writeUint16LE(targetObjID);

			enactorID = o.enactor != nullptr
			            ?   o.enactor->thisID()
			            :   Nothing;

			out->writeUint16LE(enactorID);
		}
	} else if (motionType == motionTypeClimbUp
	           ||  motionType == motionTypeClimbDown) {
		immediateLocation.write(out);
	} else if (motionType == motionTypeJump) {
		velocity.write(out);
	} else if (motionType == motionTypeTurn) {
		out->writeByte(direction);
	} else if (motionType == motionTypeGive) {
		if (targetObj != nullptr)
			out->writeUint16LE(targetObj->thisID());
		else
			out->writeUint16LE(Nothing);
	} else if (motionType == motionTypeUseObject
	           ||  motionType == motionTypeUseObjectOnObject
	           ||  motionType == motionTypeUseObjectOnTAI
	           ||  motionType == motionTypeUseObjectOnLocation
	           ||  motionType == motionTypeDropObject
	           ||  motionType == motionTypeDropObjectOnObject
	           ||  motionType == motionTypeDropObjectOnTAI) {
		if (o.directObject != nullptr)
			out->writeUint16LE(o.directObject->thisID());
		else
			out->writeUint16LE(Nothing);

		out->writeByte(direction);

		if (motionType == motionTypeUseObjectOnObject
		        ||  motionType == motionTypeDropObjectOnObject) {
			if (o.indirectObject != nullptr)
				out->writeUint16LE(o.indirectObject->thisID());
			else
				out->writeUint16LE(Nothing);
		} else {
			if (motionType == motionTypeUseObjectOnTAI
			        ||  motionType == motionTypeDropObjectOnTAI) {
				if (o.TAI != nullptr)
					out->writeSint16LE(o.TAI->thisID());
				else
					out->writeSint16LE(NoActiveItem.val);
			}

			if (motionType == motionTypeUseObjectOnLocation
			        ||  motionType == motionTypeDropObject
			        ||  motionType == motionTypeDropObjectOnTAI) {
				targetLoc.write(out);
			}
		}
	} else if (motionType == motionTypeUseTAI) {
		if (o.TAI != nullptr)
			out->writeSint16LE(o.TAI->thisID());
		else
			out->writeSint16LE(NoActiveItem.val);

		out->writeByte(direction);
	} else if (motionType == motionTypeTwoHandedSwing
	           ||  motionType == motionTypeOneHandedSwing
	           ||  motionType == motionTypeFireBow
	           ||  motionType == motionTypeCastSpell
	           ||  motionType == motionTypeUseWand) {
		ObjectID    targetObjID;

		//  Store the direction
		out->writeByte(direction);

		//  Store the combat motion type
		out->writeByte(combatMotionType);

		//  Convert the target object pointer to an ID
		targetObjID = targetObj != nullptr ? targetObj->thisID() : Nothing;

		//  Store the target object ID
		out->writeUint16LE(targetObjID);

		if (motionType == motionTypeCastSpell) {
			//  Convert the spell object pointer to an ID

			SpellID sid         = spellObj != nullptr
			                      ? spellObj->getSpellID()
			                      : nullSpell;

			ObjectID toid       = targetObj != nullptr
			                      ? targetObj->thisID()
			                      : Nothing;

			ActiveItemID ttaid  = targetTAG != nullptr
			                      ? targetTAG->thisID()
			                      : NoActiveItem;

			//  Store the spell prototype
			warning("MotionTask::write: Check SpellID size");
			out->writeUint32LE(sid);

			//  Store object target
			out->writeUint16LE(toid);

			//  Store TAG target
			out->writeSint16LE(ttaid.val);

			//  Store _data.location target
			targetLoc.write(out);
		}

		//  Store the action counter
		out->writeSint16LE(actionCounter);

	} else if (motionType == motionTypeTwoHandedParry
	           ||  motionType == motionTypeOneHandedParry
	           ||  motionType == motionTypeShieldParry) {
		ObjectID        attackerID,
		                defensiveObjID;

		//  Store the direction
		out->writeByte(direction);

		attackerID = d.attacker != nullptr ? d.attacker->thisID() : Nothing;
		defensiveObjID = d.defensiveObj != nullptr ? d.defensiveObj->thisID() : Nothing;

		//  Store the attacker's and defensive object's IDs
		out->writeUint16LE(attackerID);
		out->writeUint16LE(defensiveObjID);

		//  Store the defense flags
		out->writeByte(d.defenseFlags);

		//  Store the action counter
		out->writeSint16LE(actionCounter);

		if (motionType == motionTypeOneHandedParry) {
			//  Store the combat sub-motion type
			out->writeByte(combatMotionType);
		}
	} else if (motionType == motionTypeDodge
	           ||  motionType == motionTypeAcceptHit
	           ||  motionType == motionTypeFallDown) {
		ObjectID        attackerID;

		attackerID = d.attacker != nullptr ? d.attacker->thisID() : Nothing;

		//  Store the attacker's ID
		out->writeUint16LE(attackerID);

		//  Store the action counter
		out->writeSint16LE(actionCounter);
	}
}

//-----------------------------------------------------------------------
//	When a motion task is finished, call this function to delete it.

void MotionTask::remove(int16 returnVal) {
	if (g_vm->_mTaskList->_nextMT != g_vm->_mTaskList->_list.end() && *(g_vm->_mTaskList->_nextMT) == this)
		++g_vm->_mTaskList->_nextMT;

	object->_data.objectFlags &= ~objectMoving;
	if (objObscured(object))
		object->_data.objectFlags |= objectObscured;
	else
		object->_data.objectFlags &= ~objectObscured;

	if (isActor(object)) {
		Actor   *a = (Actor *)object;

		a->_moveTask = nullptr;
		a->_cycleCount = g_vm->_rnd->getRandomNumber(19);

		//  Make sure the actor is not left in a permanently
		//  uninterruptable state with no motion task to reset it
		if (a->isPermanentlyUninterruptable())
			a->setInterruptablity(true);
	}

	g_vm->_mTaskList->_list.remove(this);

	abortPathFind(this);
	pathFindTask = nullptr;

	wakeUpThread(thread, returnVal);
}

//-----------------------------------------------------------------------
//	Determine the immediate target _data.location

TilePoint MotionTask::getImmediateTarget() {
	if (immediateLocation != Nowhere)
		return immediateLocation;

	Direction       dir;

	//  If the wandering then simply go in the direction the actor is
	//  facing, else if avoiding a block go in the previously selected
	//  random direction
	if (flags & agitated)
		dir = direction;
	else
		dir = ((Actor *)object)->_currentFacing;

	return  object->_data.location
	        +   incDirTable[dir] * kTileUVSize;
}

//-----------------------------------------------------------------------
//	This calculates the velocity for a ballistic motion

void MotionTask::calcVelocity(const TilePoint &vector,  int16 turns) {
	TilePoint   veloc;

	// Here is the formula for calculating the velocity Z vector

	// Vz = - 1/2gt + 1/t(Dz - Sz)

	// Vz = Velocity Z Coords
	// g  = gravity
	// t  = turns
	// Dz = Destination Z Coords
	// Sz = Source Z Coords

	veloc.u = vector.u / turns;
	veloc.v = vector.v / turns;

	//  This is used in ballistic motion to make up for rounding

	steps = turns;
	uFrac = vector.u % turns;
	vFrac = vector.v % turns;
	uErrorTerm = 0;
	vErrorTerm = 0;

	veloc.z = ((gravity * turns) >> 1) + vector.z / turns;
	velocity = veloc;
}

//-----------------------------------------------------------------------
//	This initiates a motion task for turning an actor

void MotionTask::turn(Actor &obj, Direction dir) {
	assert(dir < 8);

	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&obj)) != nullptr) {
		mt->direction = dir;
		mt->motionType = motionTypeTurn;
		mt->flags = reset;
	}
}

//-----------------------------------------------------------------------
//	This initiates a motion task for turning an actor

void MotionTask::turnTowards(Actor &obj, const TilePoint &where) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&obj)) != nullptr) {
		mt->direction = (where - obj.getLocation()).quickDir();
		mt->motionType = motionTypeTurn;
		mt->flags = reset;
	}
}

//-----------------------------------------------------------------------
//	This initiates a motion task for going through the motions of giving
//	an object to another actor

void MotionTask::give(Actor &actor, Actor &givee) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&actor)) != nullptr) {
		mt->targetObj = &givee;
		mt->motionType = motionTypeGive;
		mt->flags = reset;
	}
}

//-----------------------------------------------------------------------
//	This initiates a motion task for throwing an object

void MotionTask::throwObject(GameObject &obj, const TilePoint &velocity) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&obj)) != nullptr) {
		if (obj.isMissile()) obj._data.missileFacing = missileNoFacing;
		mt->velocity = velocity;
		mt->motionType = motionTypeThrown;
	}
}

//-----------------------------------------------------------------------
//	This function is intended to allow the character to throw an object
//	to a specific point. It is in no way functional yet.

//  REM: we need to know if we are indoors or outdoors!
//  REM: we need to know celing height!!!

void MotionTask::throwObjectTo(GameObject &obj, const TilePoint &where) {
	MotionTask      *mt;
	const int16     turns = 15;

	if ((mt = g_vm->_mTaskList->newTask(&obj)) != nullptr) {
		if (obj.isMissile()) obj._data.missileFacing = missileNoFacing;
		mt->calcVelocity(where - obj.getLocation(), turns);
		mt->motionType = motionTypeThrown;
	}
}

//-----------------------------------------------------------------------
//	This function initiates a ballistic motion towards a specified target
//	_data.location at a specified horizontal speed.

void MotionTask::shootObject(
    GameObject &obj,
    Actor &doer,
    GameObject &target,
    int16 speed) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&obj)) != nullptr) {
		TilePoint   targetLoc = target.getLocation();

		targetLoc.z += target.proto()->height / 2;

		TilePoint   vector = targetLoc - obj.getLocation();
		int16       turns = MAX(vector.quickHDistance() / speed, 1);

		if (isActor(&target)) {
			Actor       *targetActor = (Actor *)&target;

			if (targetActor->_moveTask != nullptr) {
				MotionTask      *targetMotion = targetActor->_moveTask;

				if (targetMotion->motionType == motionTypeWalk)
					vector += targetMotion->velocity * turns;
			}
		}

		mt->calcVelocity(vector, turns);

		if (obj.isMissile())
			obj._data.missileFacing = missileDir(mt->velocity);

		mt->motionType = motionTypeShot;
		mt->o.enactor = &doer;
		mt->targetObj = &target;
	}
}

//-----------------------------------------------------------------------
//	Walk to a specific point, using pathfinding.

void MotionTask::walkTo(
    Actor           &actor,
    const TilePoint &target,
    bool            run,
    bool            canAgitate) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&actor)) != nullptr) {
		if (!mt->isReflex() && !actor.isImmobile()) {
			unstickObject(&actor);
			mt->finalTarget = mt->immediateLocation = target;
			mt->motionType = mt->prevMotionType = motionTypeWalk;
			mt->pathCount = mt->pathIndex = 0;
			mt->flags = pathFind | reset;
			mt->runCount = 12;          // # of frames until we can run

			if (run && actor.isActionAvailable(actionRun))
				mt->flags |= requestRun;
			if (canAgitate)
				mt->flags |= agitatable;

			RequestPath(mt, getPathFindIQ(&actor));
		}
	}
}

//-----------------------------------------------------------------------
//	Walk to a specific point without pathfinding

void MotionTask::walkToDirect(
    Actor           &actor,
    const TilePoint &target,
    bool            run,
    bool            canAgitate) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&actor)) != nullptr) {
		if (!mt->isReflex() && !actor.isImmobile()) {
			//  Abort any pending path finding task
			abortPathFind(mt);
			mt->pathFindTask = nullptr;

			unstickObject(&actor);
			mt->motionType = mt->prevMotionType = motionTypeWalk;
			mt->finalTarget = mt->immediateLocation = target;
			mt->pathCount = mt->pathIndex = 0;
			mt->flags = reset;
			mt->runCount = 12;

			if (run && actor.isActionAvailable(actionRun))
				mt->flags |= requestRun;
			if (canAgitate)
				mt->flags |= agitatable;
		}
	}
}

//-----------------------------------------------------------------------
//	Wander around

void MotionTask::wander(
    Actor       &actor,
    bool        run) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&actor)) != nullptr) {
		if (!mt->isReflex() && !actor.isImmobile()) {
			//  Abort any pending path finding task
			abortPathFind(mt);
			mt->pathFindTask = nullptr;

			unstickObject(&actor);
			mt->motionType = mt->prevMotionType = motionTypeWalk;
			mt->immediateLocation = Nowhere;
			mt->pathCount = mt->pathIndex = 0;
			mt->flags = reset | wandering;
			mt->runCount = 12;

			if (run && actor.isActionAvailable(actionRun))
				mt->flags |= requestRun;

			RequestWanderPath(mt, getPathFindIQ(&actor));
		}
	}
}

//-----------------------------------------------------------------------
//	Wander around within a tether region

void MotionTask::tetheredWander(
    Actor               &actor,
    const TileRegion    &tetherReg,
    bool                run) {
	MotionTask          *mt;

	if ((mt = g_vm->_mTaskList->newTask(&actor)) != nullptr) {
		if (!mt->isReflex() && !actor.isImmobile()) {
			//  Abort any pending path finding task
			abortPathFind(mt);
			mt->pathFindTask = nullptr;

			unstickObject(&actor);
			mt->motionType = mt->prevMotionType = motionTypeWalk;
			mt->immediateLocation = Nowhere;
			mt->tetherMinU = tetherReg.min.u;
			mt->tetherMinV = tetherReg.min.v;
			mt->tetherMaxU = tetherReg.max.u;
			mt->tetherMaxV = tetherReg.max.v;
			mt->pathCount = mt->pathIndex = 0;
			mt->flags = reset | wandering | tethered;
			mt->runCount = 12;

			if (run && actor.isActionAvailable(actionRun))
				mt->flags |= requestRun;

			RequestWanderPath(mt, getPathFindIQ(&actor));
		}
	}
}

//-----------------------------------------------------------------------
//	Create a climb up ladder motion task.

void MotionTask::upLadder(Actor &actor) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&actor)) != nullptr) {
		if (mt->motionType != motionTypeClimbUp) {
			mt->motionType = motionTypeClimbUp;
			mt->flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Create a climb down ladder motion task.

void MotionTask::downLadder(Actor &actor) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&actor)) != nullptr) {
		if (mt->motionType != motionTypeClimbDown) {
			mt->motionType = motionTypeClimbDown;
			mt->flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Create a talk motion task.

void MotionTask::talk(Actor &actor) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&actor)) != nullptr) {
		if (mt->motionType != motionTypeTalk) {
			mt->motionType = motionTypeTalk;
			mt->flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Begin a jump. REM: This should probably have a parameter for jumping
//	forward, backward, etc.

void MotionTask::jump(Actor &actor) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&actor)) != nullptr) {
		if (mt->motionType != motionTypeThrown) {
			mt->velocity.z = 10;
			mt->motionType = motionTypeJump;
			mt->flags = reset;
		}
	}
}


//-----------------------------------------------------------------------
//	Don't move -- simply eat some time

void MotionTask::wait(Actor &a) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeWait) {
			mt->motionType = motionTypeWait;
			mt->flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Use an object

void MotionTask::useObject(Actor &a, GameObject &dObj) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeUseObject) {
			mt->motionType = motionTypeUseObject;
			mt->o.directObject = &dObj;
			mt->flags = reset;
			if (isPlayerActor(&a)) mt->flags |= privledged;
		}
	}
}

//-----------------------------------------------------------------------
//	Use one object on another

void MotionTask::useObjectOnObject(
    Actor       &a,
    GameObject  &dObj,
    GameObject  &target) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeUseObjectOnObject) {
			mt->motionType = motionTypeUseObjectOnObject;
			mt->o.directObject = &dObj;
			mt->o.indirectObject = &target;
			mt->flags = reset;
			if (isPlayerActor(&a)) mt->flags |= privledged;
		}
	}
}

//-----------------------------------------------------------------------
//	Use an object on a TAI

void MotionTask::useObjectOnTAI(
    Actor       &a,
    GameObject  &dObj,
    ActiveItem  &target) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeUseObjectOnTAI) {
			mt->motionType = motionTypeUseObjectOnTAI;
			mt->o.directObject = &dObj;
			mt->o.TAI = &target;
			mt->flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Use on object on a TilePoint

void MotionTask::useObjectOnLocation(
    Actor           &a,
    GameObject      &dObj,
    const Location  &target) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeUseObjectOnLocation) {
			mt->motionType = motionTypeUseObjectOnLocation;
			mt->o.directObject = &dObj;
			mt->targetLoc = target;
			mt->flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Use a TAI

void MotionTask::useTAI(Actor &a, ActiveItem &dTAI) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeUseTAI) {
			mt->motionType = motionTypeUseTAI;
			mt->o.TAI = &dTAI;
			mt->flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Drop an object

void MotionTask::dropObject(Actor       &a,
                            GameObject &dObj,
                            const      Location &loc,
                            int16      num) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeDropObject) {
			mt->motionType = motionTypeDropObject;
			mt->o.directObject = &dObj;
			mt->targetLoc = loc;
			mt->flags = reset;
			mt->moveCount = num;
		}
	}
}

//-----------------------------------------------------------------------
//	Drop one object on another

void MotionTask::dropObjectOnObject(
    Actor       &a,
    GameObject  &dObj,
    GameObject  &target,
    int16       num) {
	MotionTask      *mt;

	//  If actor is dropping object on himself, and object is already
	//  in actor's container then consider it a "use" (if the object
	//  is of the correct type).

	if (isActor(&target)
	        &&  isPlayerActor((Actor *)&target)
	        &&  dObj.IDParent() == target.thisID()
	        &&  !(dObj.proto()->containmentSet() & ProtoObj::isContainer)) {
		useObject(a, dObj);
		return;
	}

	//  Otherwise, drop it on the object

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeDropObjectOnObject) {
			mt->motionType = motionTypeDropObjectOnObject;
			mt->o.directObject = &dObj;
			mt->o.indirectObject = &target;
			mt->flags = reset;
			mt->moveCount = num;
		}
	}
}

//-----------------------------------------------------------------------
//	Drop an object on a TAI

void MotionTask::dropObjectOnTAI(
    Actor           &a,
    GameObject      &dObj,
    ActiveItem      &target,
    const Location  &loc) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeDropObjectOnTAI) {
			mt->motionType = motionTypeDropObjectOnTAI;
			mt->o.directObject = &dObj;
			mt->o.TAI = &target;
			mt->targetLoc = loc;
			mt->flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Determine if this MotionTask is a reflex ( motion over which an actor
//	has no control )

bool MotionTask::isReflex() {
	return      motionType == motionTypeThrown
	            ||  motionType == motionTypeFall
	            ||  motionType == motionTypeLand
	            ||  motionType == motionTypeAcceptHit
	            ||  motionType == motionTypeFallDown
	            ||  motionType == motionTypeDie;
}

//  Offensive combat actions

//-----------------------------------------------------------------------
//	Initiate a two-handed swing

void MotionTask::twoHandedSwing(Actor &a, GameObject &target) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeTwoHandedSwing) {
			mt->motionType = motionTypeTwoHandedSwing;
			mt->targetObj = &target;
			mt->flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Initiate a one-handed swing

void MotionTask::oneHandedSwing(Actor &a, GameObject &target) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeOneHandedSwing) {
			mt->motionType = motionTypeOneHandedSwing;
			mt->targetObj = &target;
			mt->flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Initiate a fire bow motion

void MotionTask::fireBow(Actor &a, GameObject &target) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeFireBow) {
			mt->motionType = motionTypeFireBow;
			mt->targetObj = &target;
			mt->flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Initiate a cast spell motion

void MotionTask::castSpell(Actor &a, SkillProto &spell, GameObject &target) {
	MotionTask      *mt;
	motionTypes     type =
	    (spellBook[spell.getSpellID()].getManaType() == sManaIDSkill) ?
	    motionTypeGive :
	    motionTypeCastSpell;


	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != type) {
			mt->motionType = type;
			mt->spellObj = &spell;
			mt->targetObj = &target;
			mt->flags = reset;
			mt->direction = (mt->targetObj->getLocation() - a.getLocation()).quickDir();
			if (isPlayerActor(&a)) mt->flags |= privledged;
		}
	}
}

void MotionTask::castSpell(Actor &a, SkillProto &spell, Location &target) {
	MotionTask      *mt;
	motionTypes     type =
	    (spellBook[spell.getSpellID()].getManaType() == sManaIDSkill) ?
	    motionTypeGive :
	    motionTypeCastSpell;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != type) {
			mt->motionType = type;
			mt->spellObj = &spell;
			mt->targetLoc = target; //target;
			mt->flags = reset | LocTarg;
			mt->direction = (target - a.getLocation()).quickDir();
			if (isPlayerActor(&a)) mt->flags |= privledged;
		}
	}
}

void MotionTask::castSpell(Actor &a, SkillProto &spell, ActiveItem &target) {
	MotionTask      *mt;
	motionTypes     type =
	    (spellBook[spell.getSpellID()].getManaType() == sManaIDSkill) ?
	    motionTypeGive :
	    motionTypeCastSpell;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != type) {
			Location loc;
			assert(target._data.itemType == activeTypeInstance);
			mt->motionType = type;
			mt->spellObj = &spell;
			mt->targetTAG = &target;
			loc = Location(
			          target._data.instance.u << kTileUVShift,
			          target._data.instance.v << kTileUVShift,
			          target._data.instance.h,
			          a.world()->thisID());
			mt->targetLoc = loc; //target;
			mt->flags = reset | TAGTarg;
			mt->direction = (loc - a.getLocation()).quickDir();
			if (isPlayerActor(&a)) mt->flags |= privledged;
		}
	}
}

//-----------------------------------------------------------------------
//	Initiate a use wand motion

void MotionTask::useWand(Actor &a, GameObject &target) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeUseWand) {
			mt->motionType = motionTypeUseWand;
			mt->targetObj = &target;
			mt->flags = reset;
		}
	}
}

//  Defensive combat actions

//-----------------------------------------------------------------------
//	Initiate a two-handed parry

void MotionTask::twoHandedParry(
    Actor       &a,
    GameObject  &weapon,
    Actor       &opponent) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeTwoHandedParry) {
			mt->motionType = motionTypeTwoHandedParry;
			mt->d.attacker = &opponent;
			mt->d.defensiveObj = &weapon;
		}
		mt->flags = reset;
		mt->d.defenseFlags = 0;
	}
}

//-----------------------------------------------------------------------
//	Initiate a one-handed parry

void MotionTask::oneHandedParry(
    Actor       &a,
    GameObject  &weapon,
    Actor       &opponent) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeOneHandedParry) {
			mt->motionType = motionTypeOneHandedParry;
			mt->d.attacker = &opponent;
			mt->d.defensiveObj = &weapon;
		}
		mt->flags = reset;
		mt->d.defenseFlags = 0;
	}
}

//-----------------------------------------------------------------------
//	Initiate a shield parry

void MotionTask::shieldParry(
    Actor       &a,
    GameObject  &shield,
    Actor       &opponent) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeShieldParry) {
			mt->motionType = motionTypeShieldParry;
			mt->d.attacker = &opponent;
			mt->d.defensiveObj = &shield;
		}
		mt->flags = reset;
		mt->d.defenseFlags = 0;
	}
}

//-----------------------------------------------------------------------
//	Initiate a dodge

void MotionTask::dodge(Actor &a, Actor &opponent) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeDodge) {
			mt->motionType = motionTypeDodge;
			mt->d.attacker = &opponent;
		}
		mt->flags = reset;
		mt->d.defenseFlags = 0;
	}
}

//  Other combat actions

//-----------------------------------------------------------------------
//	Initiate an accept hit motion

void MotionTask::acceptHit(Actor &a, Actor &opponent) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeAcceptHit) {
			mt->motionType = motionTypeAcceptHit;
			mt->d.attacker = &opponent;
			mt->flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Initiate a fall down motion

void MotionTask::fallDown(Actor &a, Actor &opponent) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeFallDown) {
			mt->motionType = motionTypeFallDown;
			mt->d.attacker = &opponent;
			mt->flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Initiate a die motion

void MotionTask::die(Actor &a) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->motionType != motionTypeDie) {
			mt->motionType = motionTypeDie;
			mt->flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Determine if this MotionTask is a defensive motion

bool MotionTask::isDefense() {
	return      motionType == motionTypeOneHandedParry
	            ||  motionType == motionTypeTwoHandedParry
	            ||  motionType == motionTypeShieldParry
	            ||  motionType == motionTypeDodge;
}

//-----------------------------------------------------------------------
//	Determine if this MotionTask is an offensive motion

bool MotionTask::isAttack() {
	return      isMeleeAttack()
	            ||  motionType == motionTypeFireBow
	            ||  motionType == motionTypeCastSpell
	            ||  motionType == motionTypeUseWand;
}

//-----------------------------------------------------------------------
//	Determine if this MotionTask is an offensive melee motion

bool MotionTask::isMeleeAttack() {
	return      motionType == motionTypeOneHandedSwing
	            ||  motionType == motionTypeTwoHandedSwing;
}

//-----------------------------------------------------------------------
//	Determine if this MotionTask is a walk motion

bool MotionTask::isWalk() {
	return prevMotionType == motionTypeWalk;
}

//-----------------------------------------------------------------------
//	Return the wandering tether region

TileRegion MotionTask::getTether() {
	TileRegion  reg;

	if (flags & tethered) {
		reg.min = TilePoint(tetherMinU, tetherMinV, 0);
		reg.max = TilePoint(tetherMaxU, tetherMaxV, 0);
	} else {
		reg.min = Nowhere;
		reg.max = Nowhere;
	}

	return reg;
}

//-----------------------------------------------------------------------
//	If the target has changed position since the last path find started,
//  then call this function.

void MotionTask::changeTarget(const TilePoint &newPos, bool run) {
	if (prevMotionType == motionTypeWalk) {
		uint16      oldFlags = flags;

		abortPathFind(this);

		finalTarget = immediateLocation = newPos;
		pathCount = pathIndex = 0;

		flags = pathFind | reset;
		if (oldFlags & agitatable)
			flags |= agitatable;

		//  Set run flag if requested
		if (run
		        //  Check if actor capable of running...
		        && ((Actor *)object)->isActionAvailable(actionRun))

			flags |= requestRun;
		else
			flags &= ~requestRun;

		RequestPath(this, getPathFindIQ(object));
	}
}

//-----------------------------------------------------------------------
//	If the target has changed position since the walk/run started, then
//	call this function.

void MotionTask::changeDirectTarget(const TilePoint &newPos, bool run) {
	if (prevMotionType == motionTypeWalk) {
		prevMotionType = motionTypeWalk;

		finalTarget = immediateLocation = newPos;

		//  Reset motion task
		flags |= reset;
		flags &= ~pathFind;

		//  Set run flag if requested
		if (run
		        //  Check if actor capable of running...
		        && ((Actor *)object)->isActionAvailable(actionRun))

			flags |= requestRun;
		else
			flags &= ~requestRun;
	}
}

//  Cancel actor movement if walking...
void MotionTask::finishWalk() {
	//  If the actor is in a running state
	if (motionType == motionTypeWalk) {
		remove();
		//  If there is currently a path finding request, abort it.
		/*      abortPathFind( this );

		            //  Simply set actor's target _data.location to "here".
		        finalTarget = immediateLocation = object->getLocation();
		        pathList[0] = finalTarget;
		        flags = reset;
		        pathCount = 0;
		        pathIndex = 0;*/
	}
}

//  Cancel actor movement if talking...
void MotionTask::finishTalking() {
	if (motionType == motionTypeTalk) {
		if (isActor(object)) {
			Actor   *a = (Actor *)object;
			if (a->_currentAnimation != actionStand)
				a->setAction(actionStand, 0);
		}
		remove();
	}
}

//-----------------------------------------------------------------------
//	Handle actions for characters and objects in free-fall

void MotionTask::ballisticAction() {
	TilePoint       totalVelocity,          // total velocity vector
	                stepVelocity,           // sub-velocity vector
	                location,
	                newPos;

	int16           minDim,
	                vectorSteps;

	GameObject      *obj = object;
	ProtoObj        *proto = obj->proto();

	if (isActor(obj)) {
		//  Before anything else make sure the actor is in an
		//  uninterruptable state.
		((Actor *)obj)->setInterruptablity(false);
	}


	//  Add the force of gravity to the acceleration.

	if (!(flags & inWater)) {
		velocity.z -= gravity;
	} else {
		velocity.u = velocity.v = 0;
		velocity.z = -gravity;
	}
	location = obj->getLocation();

//	WriteStatusF( 6, "%d %d %d", _data.location.u, _data.location.v, _data.location.z );

	//  Because we live in a point-sampled universe, we need to make
	//  sure that objects which are moving extremely fast don't
	//  undersample the terrain. We do this by breaking the velocity
	//  vector into smaller vectors, and handling them individually.

	totalVelocity = velocity;

	//  Make Up For Rounding Errors In ThrowTo

	if (uFrac) {
		uErrorTerm += ABS(uFrac);

		if (uErrorTerm >= steps) {
			uErrorTerm -= steps;
			if (uFrac > 0)
				totalVelocity.u++;
			else
				totalVelocity.u--;
		}
	}

	if (vFrac) {
		vErrorTerm += ABS(vFrac);

		if (vErrorTerm >= steps) {
			vErrorTerm -= steps;
			if (vFrac > 0)
				totalVelocity.v++;
			else
				totalVelocity.v--;
		}
	}

	//  Determine which dimension is smaller, width or height.
	minDim = MAX<int16>(MIN<int16>(proto->height, proto->crossSection * 2), 1);

	//  "vectorSteps" is the number of increments we are going to process
	//  this vector.

	vectorSteps = ((totalVelocity.magnitude() - 1) / minDim) + 1;

	if (isActor(obj) && velocity.magnitude() > 16) {
		Actor       *a = (Actor *)obj;

		if (a->isActionAvailable(actionFreeFall))
			a->setAction(actionFreeFall, 0);
	}

	for (int i = 0; i < vectorSteps; i++) {
		int16       stepsLeft = vectorSteps - i;
		GameObject  *collisionObject;

		//  REM: This would be better as a rounded division...

		//  Compute the small velocity vector for this increment,
		//  and then subtract it from the total velocity.

		stepVelocity = totalVelocity / stepsLeft;
		totalVelocity -= stepVelocity;

		//  Compute the new position of the object

		newPos = location + stepVelocity;



		//  See if the object ran into anything. If it didn't, then
		//  update the coord and try again.

		if (isActor(obj)) {
			Actor   *a = (Actor *)obj;

			if (a == getCenterActor() && checkLadder(a, newPos))
				return;
		}

		if (checkContact(obj, newPos, &collisionObject) == false) {
			location = newPos;
		} else {
			TilePoint       oldVelocity = velocity;

			if (motionType == motionTypeShot && collisionObject != nullptr) {
				//  If this motion is for a shot arrow and we did not
				//  collide with our target object just continue the
				//  motion as if there was no collision.
				if (collisionObject == targetObj) {
					if (object->strike(
					            o.enactor->thisID(),
					            targetObj->thisID())) {
						//  The arrow struck, so delete the arrow and
						//  end this motion
						remove();
						object->deleteObject();
						return;
					} else {
						//  If the arrow failed to strike continue the
						//  arrows flight as if there was no collision.
						targetObj = nullptr;
						location = newPos;
						continue;
					}
				} else {
					location = newPos;
					continue;
				}
			}

			if (unstickObject(obj)) return;

			//  "probe" is a bitfield which will indicate which
			//  directions the obstructions lie in.

			int16       probe = 0;

			//  Probe along each of the three coordinate axes

			if (checkBlocked(obj,
			                 TilePoint(newPos.u,
			                           obj->_data.location.v,
			                           obj->_data.location.z))) {
				probe |= (1 << 0);
			}

			if (checkBlocked(obj,
			                 TilePoint(obj->_data.location.u,
			                           newPos.v,
			                           obj->_data.location.z))) {
				probe |= (1 << 1);
			}

			if (checkContact(obj,
			                 TilePoint(obj->_data.location.u,
			                           obj->_data.location.v,
			                           newPos.z))) {
				probe |= (1 << 2);
			}

			//  If there are no obstructions along the orthogonal
			//  directions, then we must have hit a corner. In this
			//  case, we just bounce directly backwards.

			if (probe == 0) {
				velocity = -velocity / 2;
				totalVelocity = -totalVelocity / 2;
			} else {
				if (probe & (1 << 0)) {     // If struck wall in U direction
					velocity.u = -velocity.u / 2;
					totalVelocity.u = -totalVelocity.u / 2;
				} else {
					velocity.u = (velocity.u * 2) / 3;
					totalVelocity.u = (totalVelocity.u * 2) / 3;
				}

				if (probe & (1 << 1)) {     // If struck wall in V direction
					velocity.v = -velocity.v / 2;
					totalVelocity.v = -totalVelocity.v / 2;
				} else {
					velocity.v = (velocity.v * 2) / 3;
					totalVelocity.v = (totalVelocity.v * 2) / 3;
				}

				if (probe & (1 << 2)) {     // If struct wall in Z direction
					velocity.z = -velocity.z / 2;
					totalVelocity.z = -totalVelocity.z / 2;
				} else {
					velocity.z = (velocity.z * 2) / 3;
					totalVelocity.z = (totalVelocity.z * 2) / 3;
				}
			}
			uFrac = vFrac = 0;
			if (motionType == motionTypeShot && obj->isMissile())
				obj->_data.missileFacing = missileDir(velocity);

			//  If the ballistic object is an actor hitting the
			//  ground, then instead of bouncing, we'll just have
			//  them absorb the impact

			if (isActor(obj) && probe & (1 << 2)) {
				StandingTileInfo    sti;

				if (freeFall(location, sti) == false) {
					int16       velocityMagnitude = oldVelocity.magnitude();

					fallingDamage(obj, velocityMagnitude);
					obj->move(location);
					if (!((Actor *)obj)->isDead()) {
						motionType =    velocityMagnitude <= 16
						                ?   motionTypeLand
						                :   motionTypeLandBadly;
						flags |= reset;
						setObjectSurface(obj, sti);
					} else {
						setObjectSurface(obj, sti);
						remove();
					}
					return;
				} else {
					setObjectSurface(obj, sti);
					//  If the object is falling, then
					//  freeFall will have already modified the
					//  object's _data.location
					return;
				}
			} else if (velocity.u < 2 && velocity.u > -2
			           &&  velocity.v < 2 && velocity.v > -2
			           &&  velocity.z < 2 && velocity.z > -2) {
				StandingTileInfo    sti;

				//  If the reduced velocity after impact is
				//  very small, then we'll assume that the object
				//  has come to rest.

				if (freeFall(location, sti) == false) {
					obj->move(location);
					remove();           // delete motion task
					setObjectSurface(obj, sti);
					return;
				}
				setObjectSurface(obj, sti);
				return;
			}
			//  Otherwise, since we struck a wall at high velocity,
			//  we just drop this small velocity vector from our
			//  calculations, and continue with the next iteration
			//  of the loop.
		}
	}

	obj->move(location);
}


//-----------------------------------------------------------------------
//	Get the coordinates of the next waypoint.

bool MotionTask::nextWayPoint() {
	//  If the pathfinder hasn't managed to determine waypoints
	//  yet, then return failure.
//	if ( ( flags & pathFind ) && pathCount < 0 ) return false;

	//  If there are still waypoints in the path list, then
	//  retrieve the next waypoint.
	if ((flags & (pathFind | wandering)) && pathIndex < pathCount) {
		TilePoint   wayPointVector(0, 0, 0);

		if (pathIndex > 0)
			wayPointVector = immediateLocation - object->_data.location;

		if (wayPointVector.quickHDistance() == 0)
			//  Next vertex in path polyline
			immediateLocation = pathList[pathIndex++];
		else
			return false;
	} else {
		if (flags & wandering) {
			immediateLocation = Nowhere;
			if (pathFindTask == nullptr)
				RequestWanderPath(this, getPathFindIQ(object));
		} else if (flags & agitated) {
			immediateLocation = Nowhere;
		} else {
			//  If we've gone off the end of the path list,
			//  and we're not at the target yet, request more waypoints then
			//  use dumb pathfinding until the pathfinder finishes it's task.

			if ((finalTarget - object->_data.location).quickHDistance() > 0
			        ||  ABS(finalTarget.z - object->_data.location.z) > kMaxStepHeight) {
				//  If no pathfind in progress
				if ((flags & pathFind)
				        &&  !(flags & finalPath)
				        &&  pathFindTask == nullptr)
					RequestPath(this, getPathFindIQ(object));

				//  Set the immediate target to the final target,
				immediateLocation = finalTarget;
			}
			//  else we're close enough to call it quits.
			else return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------
//	Test to see if actor can walk in a given direction

bool MotionTask::checkWalk(
    int16           dir,
    int16           speed,
    int16           stepUp,
    TilePoint       &pos) {
	TilePoint       newPos;

	//  Check the terrain in various directions.
	//  Check in the forward direction first, at various heights

	newPos      = object->_data.location + (dirTable[dir] * speed) / 2;
	newPos.z    = object->_data.location.z + stepUp;

	if (checkWalkable(object, newPos)) return false;

//	movementDirection = direction;
	pos = newPos;
	return true;
}

//-----------------------------------------------------------------------
//	Handle actions for characters walking and running

void MotionTask::walkAction() {
	enum WalkType {
		walkNormal  = 0,
		walkSlow,
		walkRun,
		walkStairs
	};

	TilePoint       immediateTarget = getImmediateTarget(),
	                newPos,
	                targetVector;
	int16           targetDist = 0;
	int16           movementDirection,
	                directionAngle;
	int16           moveBlocked,
	                speed = walkSpeed,
	                speedScale = 2;
	Actor           *a;
	ActorAppearance *aa;
	StandingTileInfo sti;

	bool            moveTaskWaiting = false,
	                moveTaskDone = false;
	WalkType        walkType = walkNormal;

	assert(isActor(object));
	a = (Actor *)object;
	aa = a->_appearance;

	if (a->isImmobile()) {
		remove(motionWalkBlocked);
		return;
	}

	//  Make sure that the actor is interruptable
	a->setInterruptablity(true);

	//  Set the speed of movement based on whether we are walking
	//  or running. Running only occurs after we have accelerated.
	if (flags & requestRun
	        &&  runCount == 0
	        &&  !(flags & (inWater | onStairs))) {
		speed = runSpeed;
		speedScale = 4;
		walkType = walkRun;

		//  If we can see this actor, and the actor's run frames
		//  have not been loaded, then downgrade this action to
		//  a walk (but request the run frames).
		if (aa && !aa->isBankLoaded(sprRunBankNum)) {
			walkType = walkNormal;
			aa->requestBank(sprRunBankNum);
		}
	}

	//  If for some reason we cannot run at this time, then
	//  set up for a walk instead.
	if (walkType != walkRun) {
		if (!(flags & onStairs)) {
			if (!(flags & inWater)) {
				speed = walkSpeed;
				speedScale = 2;
				walkType = walkNormal;
			} else {
				speed = slowWalkSpeed;
				speedScale = 1;
				walkType = walkSlow;

				//  reset run count if actor walking slowly
				runCount = MAX<int16>(runCount, 8);
			}

			//  If we can see this actor, and this actor's walk
			//  frames have not been loaded, then downgrade this
			//  action to a stand (i.e. do nothing).
			if (aa && !aa->isBankLoaded(sprWalkBankNum)) {
				aa->requestBank(sprWalkBankNum);
				return;
			}
		} else {
			speed = slowWalkSpeed;
			speedScale = 1;
			walkType = walkStairs;

			//  reset run count if actor walking on stairs
			runCount = MAX<int16>(runCount, 8);
		}
	}

	if ((flags & agitated)
	        &&  --actionCounter <= 0) {
		flags &= ~agitated;
		flags |= pathFind | reset;
	}

	for (;;) {
		//  The "reset" flag indicates that the final target has
		//  changed since the last time this routine was called.
		if (!(flags & reset)) {
			//  Compute the vector and distance of the current
			//  position to the next "immediate" target.
			targetVector = immediateTarget - object->_data.location;
			targetDist = targetVector.quickHDistance();

			//  If we're not already there, then proceed towards
			//  the target.
			if (targetDist > 0 || ABS(targetVector.z) > kMaxStepHeight)
				break;
		}

		if (nextWayPoint() == false) {
			//  If no waypoint could be found and this motion task has
			//  a path find request, then go into "wait" mode.
			if (pathFindTask)
				moveTaskWaiting = true;
			else moveTaskDone = true;
			break;
		} else {
			flags &= ~reset;
			immediateTarget = getImmediateTarget();
		}
	}

#if VISUAL1
	extern void ShowObjectSection(GameObject * obj);
	extern void TPLine(const TilePoint & start, const TilePoint & stop);
	{
		TilePoint   curPt,
		            wayPt,
		            pt1,
		            pt2;

//		TPLine( a->getLocation(), immediateLocation );
		curPt = a->getLocation();
		wayPt = immediateTarget;

		for (int i = pathIndex - 1; i < pathCount;) {
			TPLine(curPt, wayPt);
			pt1 = pt2 = wayPt;
			pt1.u -= 2;
			pt1.v -= 2;
			pt2.u -= 2;
			pt2.v += 2;
			TPLine(pt1, pt2);
			pt1.u += 4;
			pt1.v += 4;
			TPLine(pt1, pt2);
			pt2.u += 4;
			pt2.v -= 4;
			TPLine(pt1, pt2);
			pt1.u -= 4;
			pt1.v -= 4;
			TPLine(pt1, pt2);

			curPt = wayPt;
			wayPt = pathList[++i];
		}

		ShowObjectSection(a);
	}
#endif

	moveBlocked = false;

	if (moveTaskDone || moveTaskWaiting) {
		movementDirection = a->_currentFacing;
	} else if (targetDist == 0 && ABS(targetVector.z) > kMaxStepHeight) {
		if (pathFindTask) {
			movementDirection = a->_currentFacing;
			moveTaskWaiting = true;
		} else {
			movementDirection = a->_currentFacing;
			moveBlocked = true;
		}
	} else if (targetDist <= speed) {
		int16       blockageType;

		//  If we're near the target, then don't bother with
		//  a smooth movement, just jump right there.
		movementDirection = targetVector.quickDir();
//		movementDirection = a->currentFacing;

		//  Set the new _data.location to the character's _data.location.
		newPos.u = immediateTarget.u;
		newPos.v = immediateTarget.v;
		newPos.z = object->_data.location.z;

		//  Determine the direction the character must spin
		//  to be at the correct movement angle.
		directionAngle =
		    (((movementDirection - a->_currentFacing) + 4) & 7) - 4;

		//  Test terrain. Note that if the character is spinning more than 1
		//  octant this frame, then they cannot move so a terrain test is unneeded.
		if (directionAngle <= 1 && directionAngle >= -1) {
			//  Test the terrain to see if we can go there.
			if ((blockageType = checkWalkable(object, newPos)) != false) {
				//  Try stepping up to a higher terrain too.
				newPos.z = object->_data.location.z + kMaxStepHeight;
				if (checkWalkable(object, newPos) != blockageNone) {
					//  If there is a path find task pending, put the walk action
					//  on hold until it finishes, else, abort the walk action.
					if (pathFindTask)
						moveTaskWaiting = true;
					else {
						movementDirection = a->_currentFacing;
						moveBlocked = true;
					}
					/*                  if (!(flags & pathFind) || nextWayPoint() == false)
					                    {
					                        moveBlocked = true;
					                        flags |= blocked;
					                        newPos.z = object->_data.location.z;

					                    }*/
				}
			}
		}
	} else {
		int16       height;
		bool        foundPath = false;

		movementDirection = targetVector.quickDir();

		//  Calculate new object position along direction vector.
		TilePoint   pos = object->_data.location
		                  + targetVector * speed / targetDist;

#if DEBUG*0
		TPLine(object->_data.location, pos);
#endif

		//  Check the terrain in various directions.
		//  Check in the forward direction first, at various heights

		for (height = 0; height <= kMaxStepHeight; height += kMaxSmoothStep) {
			//  This code has him move along the exact direction
			//  vector, even if it's not aligned with one of the
			//  cardinal directions.

			pos.z   = object->_data.location.z + height;

			if (!checkWalkable(object, pos)) {
				newPos = pos;
				foundPath = true;
				break;
			}
		}


		//  Check left and right facings if a path was not found in
		//  the forward direction.

		if (foundPath == false) {
			int16   leftDir = spinLeft(movementDirection),
			        rightDir = spinRight(movementDirection);

			for (height = 0; height <= kMaxStepHeight; height += 8) {
				if (checkWalk(rightDir, speedScale, height, newPos)) {
					movementDirection = rightDir;
					foundPath = true;
					break;
				}

				if (checkWalk(leftDir, speedScale, height, newPos)) {
					movementDirection = leftDir;
					foundPath = true;
					break;
				}
			}
		}

		//  Let's try moving at a right angle to the current path to
		//  get around this annoying obstacle...

		if (foundPath == false) {
			if (targetVector.u > speed / 2
			        &&  checkWalk(dirUpRight, speedScale, 0, newPos)) {
				movementDirection = dirUpRight;
				foundPath = true;
			} else if (-targetVector.u > speed / 2
			           &&  checkWalk(dirDownLeft, speedScale, 0, newPos)) {
				movementDirection = dirDownLeft;
				foundPath = true;
			} else if (targetVector.v > speed / 2
			           &&  checkWalk(dirUpLeft, speedScale, 0, newPos)) {
				movementDirection = dirUpLeft;
				foundPath = true;
			} else if (-targetVector.v > speed / 2
			           &&  checkWalk(dirDownRight, speedScale, 0, newPos)) {
				movementDirection = dirDownRight;
				foundPath = true;
			}
		}

		//  If we just couldn't find a valid path no matter how hard
		//  we tried, then just give up and say that we were blocked.

		if (foundPath == false) {

			//  If there is a path find task pending, put the walk action
			//  on hold until it finishes, else, abort the walk action.
			if (pathFindTask)
				moveTaskWaiting = true;
			else {
				movementDirection = a->_currentFacing;
				moveBlocked = true;
			}
		}
	}

	//  REM: Test the terrain at the new spot.

	if (movementDirection != a->_currentFacing) {
		//  Determine the direction the character must spin
		//  to be at the correct movement angle.
		directionAngle =
		    (((movementDirection - a->_currentFacing) + 4) & 7) - 4;

		//  If the direction is at a right angle or behind
		//  the character, then they cannot move.

		if (directionAngle < 0) {
			a->_currentFacing = spinRight(a->_currentFacing);
		} else {
			a->_currentFacing = spinLeft(a->_currentFacing);
		}
	}

	if (moveTaskDone) {
		remove(motionCompleted);
	} else if (moveBlocked) {
		a->setAction(actionStand, 0);
		if (flags & agitatable) {
			if (freeFall(object->_data.location, sti)) return;

			//  When he starts running again, then have him walk only.
			runCount = MAX<int16>(runCount, 8);

			//  We're blocked so we're going to wander in a random
			//  direction for a random duration
			flags |= agitated | reset;

			direction = g_vm->_rnd->getRandomNumber(7);
			actionCounter = 8 + g_vm->_rnd->getRandomNumber(7);

			//  Discard the path
			if (flags & pathFind) {
				flags &= ~finalPath;
				pathIndex = pathCount = 0;
			}
		} else
			remove(motionWalkBlocked);
	} else if (moveTaskWaiting
	           ||  movementDirection != a->_currentFacing) {
		//  When he starts running again, then have him walk only.
		runCount = MAX<int16>(runCount, 8);

		a->setAction(actionStand, 0);
		freeFall(object->_data.location, sti);
	} else {
		if (a == getCenterActor() && checkLadder(a, newPos)) return;

		int16               tHeight;

		flags &= ~blocked;

		tHeight = tileSlopeHeight(newPos, object, &sti);


		//  This is a kludge to keep the character from
		//  "jumping" as he climbs up a small step.

		if (tHeight >= object->_data.location.z - kMaxSmoothStep
		        * ((sti.surfaceTile != nullptr
		            && (sti.surfaceTile->combinedTerrainMask() & terrainStair))
		           ?   4
		           :   1)
		        &&  tHeight <  newPos.z)
			newPos.z = tHeight;

		if (freeFall(newPos, sti) == false) {
			int16   newAction;

			if (sti.surfaceTile != nullptr
			        && (sti.surfaceTile->combinedTerrainMask() & terrainStair)
			        &&  a->isActionAvailable(actionSpecial7)) {
				Direction   stairsDir;
				uint8       *cornerHeight;

				cornerHeight = sti.surfaceTile->attrs.cornerHeight;

				if (cornerHeight[0] == 0 && cornerHeight[1] == 0)
					stairsDir = 1;
				else if (cornerHeight[1] == 0 && cornerHeight[2] == 0)
					stairsDir = 3;
				else if (cornerHeight[2] == 0 && cornerHeight[3] == 0)
					stairsDir = 5;
				else
					stairsDir = 7;

				if (a->_currentFacing == stairsDir) {
					//  walk up stairs
					newAction = actionSpecial7;
					flags |= onStairs;
				} else if (a->_currentFacing == ((stairsDir - 4) & 0x7)) {
					//  walk down stairs
					newAction = actionSpecial8;
					flags |= onStairs;
				} else {
					flags &= ~onStairs;
					if (walkType == walkStairs) walkType = walkNormal;
					newAction = (walkType == walkRun) ? actionRun : actionWalk;
				}
			} else {
				flags &= ~onStairs;
				if (walkType == walkStairs) walkType = walkNormal;
				newAction = (walkType == walkRun) ? actionRun : actionWalk;
			}


			object->move(newPos);

			//  Determine if the new action is running
			//  or walking.

			if (a->_currentAnimation == newAction) {
				//  If we are already doing that action, then
				//  just continue doing it.
				if (walkType != walkSlow)
					a->nextAnimationFrame();
				else {
					if (flags & nextAnim)
						a->nextAnimationFrame();
					flags ^= nextAnim;
				}
			} else if (a->_currentAnimation == actionWalk
			           ||  a->_currentAnimation == actionRun
			           ||  a->_currentAnimation == actionSpecial7
			           ||  a->_currentAnimation == actionSpecial8) {
				//  If we are running instead of walking or
				//  vice versa, then change to the new action
				//  but don't break stride
				a->setAction(newAction,
				             animateRepeat | animateNoRestart);

				if (walkType != walkSlow)
					a->nextAnimationFrame();
				else {
					if (flags & nextAnim)
						a->nextAnimationFrame();
					flags ^= nextAnim;
				}
			} else {
				// If we weren't walking or running before, then start
				// walking/running and reset the sequence.
				a->setAction(newAction, animateRepeat);
				if (walkType == walkSlow) flags |= nextAnim;
			}

			if (runCount > 0) runCount--;
			setObjectSurface(object, sti);
		}
	}
}

//-----------------------------------------------------------------------
//	Climb up a ladder

void MotionTask::upLadderAction() {
	Actor               *a = (Actor *)object;

	if (flags & reset) {
		a->setAction(actionClimbLadder, animateRepeat);
		flags &= ~reset;
	} else {
		TilePoint           loc = a->getLocation();
		uint8               crossSection = a->proto()->crossSection,
		                    height = a->proto()->height;
		int16               mapNum = a->getMapNum();
		TileRegion          actorTileReg;
		TileInfo            *ti;
		TilePoint           tileLoc;
		StandingTileInfo    sti = {nullptr, nullptr, {0, 0, 0}, 0};

		loc.z += 6;

		//  Determine the tile region which the actor overlays
		actorTileReg.min.u = (loc.u - crossSection) >> kTileUVShift;
		actorTileReg.min.v = (loc.v - crossSection) >> kTileUVShift;
		actorTileReg.max.u =
		    (loc.u + crossSection + kTileUVMask) >> kTileUVShift;
		actorTileReg.max.v =
		    (loc.v + crossSection + kTileUVMask) >> kTileUVShift;
		actorTileReg.min.z = actorTileReg.max.z = 0;

		TileIterator    iter(mapNum, actorTileReg);

		//  Iterate through all the tiles in the actor's tile region
		for (ti = iter.first(&tileLoc, &sti);
		        ti != nullptr;
		        ti = iter.next(&tileLoc, &sti)) {
			if (!(ti->combinedTerrainMask() & terrainLadder)) continue;

			if (sti.surfaceHeight
			        +   ti->attrs.terrainHeight
			        <=      loc.z
			        +   height
			        ||  sti.surfaceHeight > loc.z + height)
				continue;

			uint16      footPrintMask = 0xFFFF,
			            ladderMask;
			TilePoint   subTileLoc(
			    tileLoc.u << kTileSubShift,
			    tileLoc.v << kTileSubShift,
			    0);
			TileRegion  actorSubTileReg;

			actorSubTileReg.min.u = (loc.u - crossSection) >> kSubTileShift;
			actorSubTileReg.min.v = (loc.v - crossSection) >> kSubTileShift;
			actorSubTileReg.max.u =
			    (loc.u + crossSection + kSubTileMask) >> kSubTileShift;
			actorSubTileReg.max.v =
			    (loc.v + crossSection + kSubTileMask) >> kSubTileShift;

			if (actorSubTileReg.min.u >= subTileLoc.u)
				footPrintMask &=
				    uMinMasks[actorSubTileReg.min.u - subTileLoc.u];

			if (actorSubTileReg.min.v >= subTileLoc.v)
				footPrintMask &=
				    vMinMasks[actorSubTileReg.min.v - subTileLoc.v];

			if (actorSubTileReg.max.u < subTileLoc.u + kTileSubSize)
				footPrintMask &=
				    uMaxMasks[actorSubTileReg.max.u - subTileLoc.u];

			if (actorSubTileReg.max.v < subTileLoc.v + kTileSubSize)
				footPrintMask &=
				    vMaxMasks[actorSubTileReg.max.v - subTileLoc.v];

			ladderMask =    ti->attrs.fgdTerrain == terrNumLadder
			                ?   ti->attrs.terrainMask
			                :   ~ti->attrs.terrainMask;

			if (footPrintMask & ladderMask) {
				a->nextAnimationFrame();
				a->move(loc);
				return;
			}
		}

		TilePoint   newLoc;

		newLoc = loc + incDirTable[a->_currentFacing] * crossSection * 2;
		newLoc.z = tileSlopeHeight(newLoc, a);

		if (!checkBlocked(a, newLoc))
			a->move(newLoc);
		else {
			newLoc =    loc
			            +       incDirTable[(a->_currentFacing - 2) & 7]
			            *   crossSection * 2;
			newLoc.z = tileSlopeHeight(newLoc, a);

			if (!checkBlocked(a, newLoc))
				a->move(newLoc);
			else {
				newLoc =    loc
				            +       incDirTable[(a->_currentFacing + 2) & 7]
				            *   crossSection * 2;
				newLoc.z = tileSlopeHeight(newLoc, a);

				if (!checkBlocked(a, newLoc))
					a->move(newLoc);
				else {
					newLoc =    loc
					            +       incDirTable[a->_currentFacing]
					            *   crossSection * 2;
					newLoc.z = tileSlopeHeight(newLoc, a);
					a->move(newLoc);
					unstickObject(a);
				}
			}
		}

		a->setAction(actionStand, 0);

		remove();
	}
}

//-----------------------------------------------------------------------
//	Climb down a ladder

void MotionTask::downLadderAction() {
	Actor               *a = (Actor *)object;

	if (flags & reset) {
		a->setAction(actionClimbLadder, animateRepeat | animateReverse);
		flags &= ~reset;
	} else {
		TilePoint           loc = a->getLocation();
		uint8               crossSection = a->proto()->crossSection;
		int16               mapNum = a->getMapNum();
		TileRegion          actorTileReg;
		TileInfo            *ti;
		TilePoint           tileLoc;
		StandingTileInfo    sti = {nullptr, nullptr, {0, 0, 0}, 0};

		loc.z -= 6;

		actorTileReg.min.u = (loc.u - crossSection) >> kTileUVShift;
		actorTileReg.min.v = (loc.v - crossSection) >> kTileUVShift;
		actorTileReg.max.u =
		    (loc.u + crossSection + kTileUVMask) >> kTileUVShift;
		actorTileReg.max.v =
		    (loc.v + crossSection + kTileUVMask) >> kTileUVShift;
		actorTileReg.min.z = actorTileReg.max.z = 0;

		TileIterator    iter(mapNum, actorTileReg);

		for (ti = iter.first(&tileLoc, &sti);
		        ti != nullptr;
		        ti = iter.next(&tileLoc, &sti)) {
			if (!(ti->combinedTerrainMask() & terrainLadder)) continue;

			if (sti.surfaceHeight + ti->attrs.terrainHeight <= loc.z
			        ||  sti.surfaceHeight > loc.z)
				continue;

			uint16      footPrintMask = 0xFFFF,
			            ladderMask;
			TilePoint   subTileLoc(
			    tileLoc.u << kTileSubShift,
			    tileLoc.v << kTileSubShift,
			    0);
			TileRegion  actorSubTileReg;

			actorSubTileReg.min.u = (loc.u - crossSection) >> kSubTileShift;
			actorSubTileReg.min.v = (loc.v - crossSection) >> kSubTileShift;
			actorSubTileReg.max.u =
			    (loc.u + crossSection + kSubTileMask) >> kSubTileShift;
			actorSubTileReg.max.v =
			    (loc.v + crossSection + kSubTileMask) >> kSubTileShift;

			if (actorSubTileReg.min.u >= subTileLoc.u)
				footPrintMask &=
				    uMinMasks[actorSubTileReg.min.u - subTileLoc.u];

			if (actorSubTileReg.min.v >= subTileLoc.v)
				footPrintMask &=
				    vMinMasks[actorSubTileReg.min.v - subTileLoc.v];

			if (actorSubTileReg.max.u < subTileLoc.u + kTileSubSize)
				footPrintMask &=
				    uMaxMasks[actorSubTileReg.max.u - subTileLoc.u];

			if (actorSubTileReg.max.v < subTileLoc.v + kTileSubSize)
				footPrintMask &=
				    vMaxMasks[actorSubTileReg.max.v - subTileLoc.v];

			ladderMask =    ti->attrs.fgdTerrain == terrNumLadder
			                ?   ti->attrs.terrainMask
			                :   ~ti->attrs.terrainMask;

			if (footPrintMask & ladderMask) {
				a->nextAnimationFrame();
				a->move(loc);
				return;
			}
		}

		TilePoint   newLoc;

		newLoc = loc - incDirTable[a->_currentFacing] * kTileUVSize;
		newLoc.z = tileSlopeHeight(newLoc, a);

		if (!checkBlocked(a, newLoc))
			a->move(newLoc);
		else {
			newLoc =    loc
			            -       incDirTable[(a->_currentFacing - 2) & 7]
			            *   kTileUVSize;
			newLoc.z = tileSlopeHeight(newLoc, a);

			if (!checkBlocked(a, newLoc))
				a->move(newLoc);
			else {
				newLoc =    loc
				            -       incDirTable[(a->_currentFacing + 2) & 7]
				            *   kTileUVSize;
				newLoc.z = tileSlopeHeight(newLoc, a);

				if (!checkBlocked(a, newLoc))
					a->move(newLoc);
				else {
					newLoc =    loc
					            -       incDirTable[a->_currentFacing]
					            *   kTileUVSize;
					newLoc.z = tileSlopeHeight(newLoc, a);
					a->move(newLoc);
					unstickObject(a);
				}
			}
		}

		a->setAction(actionStand, 0);

		remove();
	}
}

//  Go through the giving motions
void MotionTask::giveAction() {
	Actor       *a = (Actor *)object;
	Direction   targetDir = (targetObj->getLocation()
	                         -   a->getLocation()).quickDir();

	if (flags & reset) {
		a->setAction(actionGiveItem, 0);
		flags &= ~reset;
	}

	if (a->_currentFacing != targetDir)
		a->turn(targetDir);
	else if (a->nextAnimationFrame())
		remove(motionCompleted);
}


//  Set up specified animation and run through the frames
void MotionTask::genericAnimationAction(uint8 actionType) {
	Actor *const   a = (Actor *)object;

	if (flags & reset) {
		a->setAction(actionType, 0);
		flags &= ~reset;
	} else if (a->nextAnimationFrame())
		remove(motionCompleted);
}

//  This class is specifically designed to aid in the selection of
//  of a combat motion type from a selected subset
struct CombatMotionSet {
	const uint8 *list;          //  Array of motion types
	uint16      listSize;       //  Size of array

	//  Select randome element from the array
	uint8 selectRandom() const {
		return list[g_vm->_rnd->getRandomNumber(listSize - 1)];
	}
};

//  Offensive combat actions

//  Construct a set of all two handed swing types
const uint8 twoHandedSwingArray[] = {
	MotionTask::twoHandedSwingHigh,
	MotionTask::twoHandedSwingLow,
	MotionTask::twoHandedSwingLeftHigh,
	MotionTask::twoHandedSwingLeftLow,
	MotionTask::twoHandedSwingRightHigh,
	MotionTask::twoHandedSwingRightLow,
};

const CombatMotionSet twoHandedSwingSet = {
	twoHandedSwingArray,
	ARRAYSIZE(twoHandedSwingArray)
};

//  Construct a subset of all high two handed swing types
const uint8 twoHandedHighSwingArray[] = {
	MotionTask::twoHandedSwingHigh,
	MotionTask::twoHandedSwingLeftHigh,
	MotionTask::twoHandedSwingRightHigh,
};

const CombatMotionSet twoHandedHighSwingSet = {
	twoHandedHighSwingArray,
	ARRAYSIZE(twoHandedHighSwingArray)
};

//  Construct a subset of all low two handed swing types
const uint8 twoHandedLowSwingArray[] = {
	MotionTask::twoHandedSwingLow,
	MotionTask::twoHandedSwingLeftLow,
	MotionTask::twoHandedSwingRightLow,
};

const CombatMotionSet twoHandedLowSwingSet = {
	twoHandedLowSwingArray,
	ARRAYSIZE(twoHandedLowSwingArray)
};

//-----------------------------------------------------------------------
//  Handle all two handed swing motions

void MotionTask::twoHandedSwingAction() {
	//  If the reset flag is set, initialize the motion
	if (flags & reset) {
		//  Let the game engine know about this aggressive act
		logAggressiveAct(object->thisID(), targetObj->thisID());

		//  Notify the target actor that he is being attacked
		if (isActor(targetObj))
			((Actor *)targetObj)->evaluateMeleeAttack((Actor *)object);

		//  Create an animation type lookup table
		static const uint8  animationTypeArray[] = {
			actionTwoHandSwingHigh,
			actionTwoHandSwingLow,
			actionTwoHandSwingLeftHigh,
			actionTwoHandSwingLeftLow,
			actionTwoHandSwingRightHigh,
			actionTwoHandSwingRightLow,
		};

		Actor               *a = (Actor *)object;
		uint8               actorAnimation;
		int16               actorMidAltitude,
		                    targetAltitude = targetObj->getLocation().z;

		const CombatMotionSet   *availableSet;

		//  Calculate the altitude of the actor's mid section
		actorMidAltitude = a->getLocation().z + (a->proto()->height >> 1);

		if (targetAltitude > actorMidAltitude)
			//  The target is higher than the actor's midsection
			availableSet = &twoHandedHighSwingSet;
		else {
			uint8           targetHeight = targetObj->proto()->height;

			if (targetAltitude + targetHeight < actorMidAltitude)
				//  The target is below the actor's midsection
				availableSet = &twoHandedLowSwingSet;
			else
				//  The target is nearly the same altitude as the actor
				availableSet = &twoHandedSwingSet;
		}

		//  Calculate the direction of the attack
		direction = (targetObj->getLocation() - a->getLocation()).quickDir();

		//  Randomly select a combat motion type from the available set
		combatMotionType = availableSet->selectRandom();
		actorAnimation = animationTypeArray[combatMotionType];

		if (a->_appearance != nullptr
		        &&  a->isActionAvailable(actorAnimation)) {
			//  Compute the number of frames in the animation before the
			//  actual strike
			actionCounter = a->animationFrames(actorAnimation, direction) - 2;

			a->setAction(actorAnimation, 0);

			//  Set this flag to indicate that the animation is actually
			//  being played
			flags |= nextAnim;
		} else {
			actionCounter = 2;

			//  Clear this flag to indicate that the animation is not
			//  being played
			flags &= ~nextAnim;
		}

		a->setActionPoints(
		    computeTurnFrames(a->_currentFacing, direction) + 10);

		flags &= ~reset;
	} else
		//  Call the generic offensive melee function
		offensiveMeleeAction();
}


//  Construct a set of all one handed swing types
const uint8 oneHandedSwingArray[] = {
	MotionTask::oneHandedSwingHigh,
	MotionTask::oneHandedSwingLow,
//				MotionTask::oneHandedThrust,
};

const CombatMotionSet oneHandedSwingSet = {
	oneHandedSwingArray,
	ARRAYSIZE(oneHandedSwingArray)
};

//  Construct a subset of all high one handed swing types
const uint8 oneHandedHighSwingArray[] = {
	MotionTask::oneHandedSwingHigh,
};

const CombatMotionSet oneHandedHighSwingSet = {
	oneHandedHighSwingArray,
	ARRAYSIZE(oneHandedHighSwingArray)
};

//  Construct a subset of all low one handed swing types
const uint8 oneHandedLowSwingArray[] = {
	MotionTask::oneHandedSwingLow,
};

const CombatMotionSet oneHandedLowSwingSet = {
	oneHandedLowSwingArray,
	ARRAYSIZE(oneHandedLowSwingArray)
};

//-----------------------------------------------------------------------
//  Handle all one handed swing motions

void MotionTask::oneHandedSwingAction() {
	if (flags & reset) {
		//  Let the game engine know about this aggressive act
		logAggressiveAct(object->thisID(), targetObj->thisID());

		//  Notify the target actor that he is being attacked
		if (isActor(targetObj))
			((Actor *)targetObj)->evaluateMeleeAttack((Actor *)object);

		//  Create an animation type lookup table
		static const uint8  animationTypeArray[] = {
			actionSwingHigh,
			actionSwingLow,
		};

		Actor *const       a = (Actor *)object;
		uint8               actorAnimation;
		int16               actorMidAltitude,
		                    targetAltitude = targetObj->getLocation().z;

		const CombatMotionSet   *availableSet;

		//  Calculate the altitude of the actor's mid section
		actorMidAltitude = a->getLocation().z + (a->proto()->height >> 1);

		if (targetAltitude > actorMidAltitude)
			//  The target is higher than the actor's midsection
			availableSet = &oneHandedHighSwingSet;
		else {
			uint8           targetHeight = targetObj->proto()->height;

			if (targetAltitude + targetHeight < actorMidAltitude)
				//  The target is below the actor's midsection
				availableSet = &oneHandedLowSwingSet;
			else
				//  The target is nearly the same altitude as the actor
				availableSet = &oneHandedSwingSet;
		}

		//  Calculate the direction of the attack
		direction = (targetObj->getLocation() - a->getLocation()).quickDir();

		//  Randomly select a combat motion type from the available set
		combatMotionType = availableSet->selectRandom();

		/*      if ( combatMotionType == oneHandedThrust )
		        {
		            //  Initialize the thrust motion
		        }
		        else*/
		{
			actorAnimation = animationTypeArray[combatMotionType];
			if (a->_appearance != nullptr
			        &&  a->isActionAvailable(actorAnimation)) {
				//  Compute the number of frames in the animation before the
				//  actual strike
				actionCounter = a->animationFrames(actorAnimation, direction) - 2;

				a->setAction(actorAnimation, 0);

				//  Set this flag to indicate that the animation is actually
				//  being played
				flags |= nextAnim;
			} else {
				actionCounter = 1;

				//  Clear this flag to indicate that the animation is not
				//  being played
				flags &= ~nextAnim;
			}

		}

		a->setActionPoints(actionCounter * 2);

		a->setActionPoints(
		    computeTurnFrames(a->_currentFacing, direction) + 10);

		flags &= ~reset;
	} else
		//  Call the generic offensive melee function
		offensiveMeleeAction();
}

//-----------------------------------------------------------------------
//	Compute the number of frames before the actual strike in an
//	offensive melee motion

uint16 MotionTask::framesUntilStrike() {
	//  If the melee action has not been initialized, return a safe value
	if (flags & reset) return maxuint16;

	uint16          turnFrames;

	turnFrames = (direction - ((Actor *)object)->_currentFacing) & 0x7;
	if (turnFrames > 4) turnFrames = 8 - turnFrames;

	return turnFrames + actionCounter;
}

//-----------------------------------------------------------------------
//	Returns a pointer to the blocking object if it applicable to
//	this motion task

GameObject *MotionTask::blockingObject(Actor *thisAttacker) {
	return      isDefense()
	            && (d.defenseFlags & blocking)
	            &&  thisAttacker == d.attacker
	            ?   d.defensiveObj
	            :   nullptr;
}

//-----------------------------------------------------------------------
//  Handle bow firing motions

void MotionTask::fireBowAction() {
	Actor       *a = (Actor *)object;

	assert(a->_leftHandObject != Nothing);

	//  Initialize the bow firing motion
	if (flags & reset) {
		//  Let the game engine know about this aggressive act
		logAggressiveAct(object->thisID(), targetObj->thisID());

		//  Compute the direction to the target
		direction = (targetObj->getLocation() - a->getLocation()).quickDir();

		if (a->_appearance != nullptr
		        &&  a->isActionAvailable(actionFireBow)) {
			//  Calculate the number of frames in the animation before the
			//  projectile is actually fired
			actionCounter = a->animationFrames(actionFireBow, direction) - 1;
			a->setAction(actionFireBow, 0);

			//  Set this flag to indicate that the animation is actually
			//  being played
			flags |= nextAnim;
		} else {
			actionCounter = 1;

			//  Clear this flag to indicate that the animation is not
			//  being played
			flags &= ~nextAnim;
		}

		a->setActionPoints(
		    computeTurnFrames(a->_currentFacing, direction) + 10);

		if (a->_currentFacing != direction)
			a->turn(direction);

		flags &= ~reset;
	} else if (a->_currentFacing != direction)
		a->turn(direction);
	else {
		//  If the actors appearance becomes NULL, make sure this action
		//  no longer depends upon the animation
		if ((flags & nextAnim) && a->_appearance == nullptr)
			flags &= ~nextAnim;

		//  If the action counter has reached zero, get a projectile and
		//  fire it
		if (actionCounter == 0) {
			GameObject  *missileWeapon;

			missileWeapon = GameObject::objectAddress(a->_leftHandObject);
			if (missileWeapon != nullptr) {
				GameObject  *proj;

				//  Ask the missile weapon's prototype to get a projectile
				proj =  missileWeapon->proto()->getProjectile(
				            a->_leftHandObject,
				            a->thisID());

				//  Shoot the projectile
				if (proj != nullptr) {
					TilePoint   actorLoc = a->getLocation();
					uint8       actorCrossSection = a->proto()->crossSection,
					            projCrossSection = proj->proto()->crossSection;
					ObjectID    projID;

					actorLoc.u +=       incDirTable[a->_currentFacing].u
					                    * (actorCrossSection + projCrossSection);
					actorLoc.v +=       incDirTable[a->_currentFacing].v
					                    * (actorCrossSection + projCrossSection);
					actorLoc.z += a->proto()->height * 7 / 8;

					if ((projID =   proj->extractMerged(Location(actorLoc, a->IDParent()), 1)) !=  Nothing) {
						g_vm->_cnm->setUpdate(a->thisID());
						proj = GameObject::objectAddress(projID);
						shootObject(*proj, *a, *targetObj, 16);
					}
				}
			}
		}

		if (flags & nextAnim) {
			//  Run through the animation frames
			if (!a->nextAnimationFrame()) {
				if (actionCounter >= 0) actionCounter--;
			} else
				remove();
		} else {
			if (actionCounter > 0)
				actionCounter--;
			else
				remove();
		}
	}
}

//-----------------------------------------------------------------------
//  Handle spell casting motions

void MotionTask::castSpellAction() {
	Actor       *a = (Actor *)object;

	//  Turn until facing the target
	if (a->_currentFacing != direction)
		a->turn(direction);
	else {
		if (flags & reset) {
			if (a->_appearance != nullptr
			        &&  a->isActionAvailable(actionCastSpell)) {
				//  Calculate the number of frames in the animation before the
				//  spell is case
				actionCounter = a->animationFrames(actionCastSpell, direction) - 1;
				a->setAction(actionCastSpell, 0);

				//  Set this flag to indicate that the animation is actually
				//  being played
				flags |= nextAnim;
			} else {
				actionCounter = 3;

				//  Clear this flag to indicate that the animation is not
				//  being played
				flags &= ~nextAnim;
			}

			flags &= ~reset;
		}

		//  If the actors appearance becomes NULL, make sure this action
		//  no longer depends upon the animation
		if ((flags & nextAnim) && a->_appearance == nullptr)
			flags &= ~nextAnim;

		if (actionCounter == 0) {
			if (spellObj) {
				if (flags & TAGTarg) {
					assert(targetTAG->_data.itemType == activeTypeInstance);
					spellObj->implementAction(spellObj->getSpellID(), a->thisID(), targetTAG);
				} else if (flags & LocTarg) {
					spellObj->implementAction(spellObj->getSpellID(), a->thisID(), targetLoc);
				} else if (targetObj) {
					spellObj->implementAction(spellObj->getSpellID(), a->thisID(), targetObj->thisID());
				}
			}
		}

		if (flags & nextAnim) {
			//  Run through the animation frames
			if (!a->nextAnimationFrame()) {
				if (actionCounter >= 0) actionCounter--;
			} else
				remove();
		} else {
			if (actionCounter > 0)
				actionCounter--;
			else
				remove();
		}
	}
}

//-----------------------------------------------------------------------
//  Handle wand using motions

void MotionTask::useWandAction() {
	//  Initialize the wand using motion
	if (flags & reset) {
		//  Let the game engine know about this aggressive act
		logAggressiveAct(object->thisID(), targetObj->thisID());

		Actor       *a = (Actor *)object;

		direction = (targetObj->getLocation() - a->getLocation()).quickDir();

		if (a->_appearance != nullptr
		        &&  a->isActionAvailable(actionUseWand)) {
			actionCounter = a->animationFrames(actionUseWand, direction) - 1;
			a->setAction(actionUseWand, 0);

			//  Set this flag to indicate that the animation is actually
			//  being played
			flags |= nextAnim;
		} else {
			actionCounter = 3;

			//  Clear this flag to indicate that the animation is not
			//  being played
			flags &= ~nextAnim;
		}

		a->setActionPoints(
		    computeTurnFrames(a->_currentFacing, direction) + 10);

		flags &= ~reset;
	}
	useMagicWeaponAction();
}

//  Defensive combat actions
//-----------------------------------------------------------------------
//	Handle two handed parrying motions

void MotionTask::twoHandedParryAction() {
	if (flags & reset) {
		Actor       *a = (Actor *)object;
		int16       animationFrames;

		direction = (d.attacker->getLocation() - a->getLocation()).quickDir();

		if (a->_appearance != nullptr
		        &&  a->isActionAvailable(actionTwoHandParry)) {
			a->setAction(actionTwoHandParry, 0);
			animationFrames = a->animationFrames(actionTwoHandParry, direction);

			//  Set this flag to indicate that the animation is actually
			//  being played
			flags |= nextAnim;
		} else {
			animationFrames = 2;

			//  Clear this flag to indicate that the animation is not
			//  being played
			flags &= ~nextAnim;
		}

		a->setActionPoints(
		    computeTurnFrames(a->_currentFacing, direction)
		    +   animationFrames + 1);

		flags &= ~reset;
	}
	defensiveMeleeAction();
}

//-----------------------------------------------------------------------
//	Handle one handed parrying motions

void MotionTask::oneHandedParryAction() {
	if (flags & reset) {
		Actor       *a = (Actor *)object;
		int16       animationFrames;

		direction = (d.attacker->getLocation() - a->getLocation()).quickDir();

		combatMotionType = oneHandedParryHigh;
		if (a->_appearance != nullptr
		        &&  a->isActionAvailable(actionParryHigh)) {
			a->setAction(actionParryHigh, 0);
			animationFrames = a->animationFrames(actionParryHigh, direction);

			//  Set this flag to indicate that the animation is actually
			//  being played
			flags |= nextAnim;
		} else {
			animationFrames = 2;

			//  Clear this flag to indicate that the animation is not
			//  being played
			flags &= ~nextAnim;
		}

		a->setActionPoints(
		    computeTurnFrames(a->_currentFacing, direction)
		    +   animationFrames + 1);

		flags &= ~reset;
	}
	defensiveMeleeAction();
}

//-----------------------------------------------------------------------
//	Handle shield parrying motions

void MotionTask::shieldParryAction() {
	if (flags & reset) {
		Actor       *a = (Actor *)object;
		int16       animationFrames;

		direction = (d.attacker->getLocation() - a->getLocation()).quickDir();

		if (a->_appearance != nullptr
		        &&  a->isActionAvailable(actionShieldParry)) {
			a->setAction(actionShieldParry, 0);
			animationFrames = a->animationFrames(actionShieldParry, direction);

			//  Set this flag to indicate that the animation is actually
			//  being played
			flags |= nextAnim;
		} else {
			animationFrames = 1;

			//  Clear this flag to indicate that the animation is not
			//  being played
			flags &= ~nextAnim;
		}

		a->setActionPoints(
		    computeTurnFrames(a->_currentFacing, direction)
		    +   animationFrames + 1);

		flags &= ~reset;
	}
	defensiveMeleeAction();
}

//-----------------------------------------------------------------------
//	Handle dodging motions

void MotionTask::dodgeAction() {
	Actor           *a = (Actor *)object;
	MotionTask      *attackerMotion = d.attacker->_moveTask;

	if (flags & reset) {
		//  If the attacker is not attacking, we're done
		if (attackerMotion == nullptr
		        ||  !attackerMotion->isMeleeAttack()) {
			a->setInterruptablity(true);
			remove();
			return;
		}

		//  If the strike is about to land start the dodging motion
		if (attackerMotion->framesUntilStrike() <= 2) {
			int16       animationFrames;

			if (a->_appearance != nullptr
			        &&  a->isActionAvailable(actionJumpUp, a->_currentFacing)) {
				a->setAction(actionJumpUp, 0);
				animationFrames = a->animationFrames(actionJumpUp, a->_currentFacing);

				//  Set this flag to indicate that the animation is actually
				//  being played
				flags |= nextAnim;
			} else {
				animationFrames = 3;

				//  Clear this flag to indicate that the animation is not
				//  being played
				flags &= ~nextAnim;
			}

			actionCounter = animationFrames - 1;
			a->setActionPoints(animationFrames + 1);

			flags &= ~reset;
		}
	} else {
		//  If the actors appearance becomes NULL, make sure this action
		//  no longer depends upon the animation
		if ((flags & nextAnim) && a->_appearance == nullptr)
			flags &= ~nextAnim;

		if (flags & nextAnim) {
			//  Run through the animation frames
			if (!a->nextAnimationFrame()) {
				if (actionCounter > 0) actionCounter--;
			} else
				remove();
		} else {
			if (actionCounter > 0)
				actionCounter--;
			else
				remove();
		}
	}
}

//-----------------------------------------------------------------------
//	Handle accept hit motions

void MotionTask::acceptHitAction() {
	Actor           *a = (Actor *)object;

	if (flags & reset) {
		TilePoint           newLoc = a->getLocation();
		StandingTileInfo    sti;
		int16               animationFrames;

		a->_currentFacing =
		    (d.attacker->getWorldLocation() - a->getLocation()).quickDir();

		if (a->_appearance != nullptr
		        &&  a->isActionAvailable(actionHit, a->_currentFacing)) {
			a->setAction(actionHit, 0);
			animationFrames = a->animationFrames(actionHit, a->_currentFacing);

			//  Set this flag to indicate that the animation is actually
			//  being played
			flags |= nextAnim;
		} else {
			animationFrames = 1;

			//  Clear this flag to indicate that the animation is not
			//  being played
			flags &= ~nextAnim;
		}

		a->setActionPoints(animationFrames + 1);

		if (g_vm->_rnd->getRandomNumber(1)) {
			//  Calculate the new position to knock the actor back to
			newLoc += dirTable[(a->_currentFacing - 4) & 0x7];

			//  If the actor is not blocked, move him back
			if (!checkBlocked(a, newLoc)) {
				newLoc.z = tileSlopeHeight(newLoc, a, &sti);
				a->move(newLoc);
				setObjectSurface(a, sti);
			}
		}

		flags &= ~reset;
	} else {
		//  If the actors appearance becomes NULL, make sure this action
		//  no longer depends upon the animation
		if ((flags & nextAnim) && a->_appearance == nullptr)
			flags &= ~nextAnim;

		if (flags & nextAnim) {
			if (a->nextAnimationFrame()) remove();
		} else
			remove();
	}
}

//-----------------------------------------------------------------------
//	Handle fall down motions

void MotionTask::fallDownAction() {
	Actor           *a = (Actor *)object;

	if (flags & reset) {
		TilePoint           newLoc = a->getLocation();
		StandingTileInfo    sti;
		int16               animationFrames;

		a->_currentFacing =
		    (d.attacker->getWorldLocation() - a->getLocation()).quickDir();

		if (a->_appearance != nullptr
		        &&  a->isActionAvailable(actionKnockedDown, a->_currentFacing)) {
			a->setAction(actionKnockedDown, 0);
			animationFrames =   a->animationFrames(
			                        actionKnockedDown,
			                        a->_currentFacing);

			//  Set this flag to indicate that the animation is actually
			//  being played
			flags |= nextAnim;
		} else {
			animationFrames = 6;

			//  Clear this flag to indicate that the animation is not
			//  being played
			flags &= ~nextAnim;
		}

		a->setActionPoints(animationFrames + 1);

		if (g_vm->_rnd->getRandomNumber(1)) {
			//  Calculate the new position to knock the actor back to
			newLoc += dirTable[(a->_currentFacing - 4) & 0x7];
			newLoc.z = tileSlopeHeight(newLoc, a, &sti);

			//  If the actor is not blocked, move him back
			if (!checkBlocked(a, newLoc)) {
				a->move(newLoc);
				setObjectSurface(a, sti);
			}
		}

		flags &= ~reset;
	} else {
		//  If the actors appearance becomes NULL, make sure this action
		//  no longer depends upon the animation
		if ((flags & nextAnim) && a->_appearance == nullptr)
			flags &= ~nextAnim;

		if (flags & nextAnim) {
			if (a->nextAnimationFrame()) remove();
		} else
			remove();
	}
}

//-----------------------------------------------------------------------
//  Generic offensive melee code.  Called by twoHandedSwingAction()
//	and oneHandedSwingAction()

void MotionTask::offensiveMeleeAction() {
	Actor       *a = (Actor *)object;

	//  Turn until facing the target
	if (a->_currentFacing != direction)
		a->turn(direction);
	else {
		//  If the actors appearance becomes NULL, make sure this action
		//  no longer depends upon the animation
		if ((flags & nextAnim) && a->_appearance == nullptr)
			flags &= ~nextAnim;

		//  If the action counter has reached zero, use the weapon on
		//  the target
		if (actionCounter == 0) {
			GameObject  *weapon;

			weapon = a->offensiveObject();
			if (weapon) weapon->strike(a->thisID(), targetObj->thisID());
		}

		if (flags & nextAnim) {
			//  Run through the animation frames
			if (!a->nextAnimationFrame()) {
				if (actionCounter >= 0) actionCounter--;
			} else
				remove();
		} else {
			if (actionCounter > 0)
				actionCounter--;
			else
				remove();
		}
	}
}

//-----------------------------------------------------------------------
//  Generic magic weapon code.  Called by useWandAction().

void MotionTask::useMagicWeaponAction() {
	Actor       *a = (Actor *)object;

	//  Turn until facing the target
	if (a->_currentFacing != direction)
		a->turn(direction);
	else {
		//  If the actors appearance becomes NULL, make sure this action
		//  no longer depends upon the animation
		if ((flags & nextAnim) && a->_appearance == nullptr)
			flags &= ~nextAnim;

		//  If the action counter has reached zero, get a spell and
		//  use it
		if (actionCounter == 0) {
			GameObject  *magicWeapon;

			magicWeapon = a->offensiveObject();

			if (magicWeapon != nullptr && magicWeapon->IDChild() != Nothing) {
				GameObject      *spell;
				SkillProto      *spellProto;

				spell = GameObject::objectAddress(magicWeapon->IDChild());
				spellProto = (SkillProto *)spell->proto();

				assert(spellProto->containmentSet() & ProtoObj::isSkill);

				//  use the spell
				spellProto->implementAction(
				    spellProto->getSpellID(),
				    magicWeapon->thisID(),
				    targetObj->thisID());
			}
		}

		if (flags & nextAnim) {
			//  Run through the animation frames
			if (!a->nextAnimationFrame()) {
				if (actionCounter >= 0) actionCounter--;
			} else
				remove();
		} else {
			if (actionCounter > 0)
				actionCounter--;
			else
				remove();
		}
	}
}

//-----------------------------------------------------------------------
//	Generic defensive melee code.  Called by twoHandedParryAction(),
//	oneHandedParryAction() and shieldParryAction().

void MotionTask::defensiveMeleeAction() {
	Actor           *a = (Actor *)object;
	MotionTask      *attackerMotion = d.attacker->_moveTask;

	//  Determine if the blocking action has been initiated
	if (!(d.defenseFlags & blocking)) {
		//  If the attacker is not attacking, we're done
		if (attackerMotion == nullptr
		        ||  !attackerMotion->isMeleeAttack()) {
			a->setInterruptablity(true);
			remove();
			return;
		}

		//  turn towards attacker
		if (a->_currentFacing != direction)
			a->turn(direction);

		//  If the strike is about to land start the blocking motion
		if (attackerMotion->framesUntilStrike() <= 1)
			d.defenseFlags |= blocking;
	} else {
		//  If the actors appearance becomes NULL, make sure this action
		//  no longer depends upon the animation
		if ((flags & nextAnim) && a->_appearance == nullptr)
			flags &= ~nextAnim;

		//  Run through the animation frames
		if (!(flags & nextAnim) || a->nextAnimationFrame()) {
			//  Wait for the attacker's attack
			if (attackerMotion == nullptr
			        ||  !attackerMotion->isMeleeAttack()) {
				a->setInterruptablity(true);
				remove();
			}
		}
	}
}

//-----------------------------------------------------------------------
//	Routine to update positions of all moving objects using MotionTasks

void MotionTask::updatePositions() {
	TilePoint       targetVector;
	TilePoint       fallVelocity, terminalVelocity(15, 15, 0);
	TilePoint       curLoc;
	int16           targetDist;
	StandingTileInfo sti;

	for (Common::List<MotionTask *>::iterator it = g_vm->_mTaskList->_list.begin(); it != g_vm->_mTaskList->_list.end(); it = g_vm->_mTaskList->_nextMT) {
		MotionTask *mt = *it;
		GameObject  *obj = mt->object;
		ProtoObj    *proto = obj->proto();
		Actor       *a = (Actor *)obj;
		bool        moveTaskDone = false;

		g_vm->_mTaskList->_nextMT = it;
		g_vm->_mTaskList->_nextMT++;

		if (!isWorld(obj->IDParent())) {
			mt->remove();
			continue;
		}

		//  Determine if this motion should be skipped
		if (interruptableMotionsPaused
		        &&  isActor(obj)
		        &&  a->isInterruptable())
			continue;

		if (obj->_data.location.z < -(proto->height >> 2))
			mt->flags |= inWater;
		else
			mt->flags &= ~inWater;

		switch (mt->motionType) {
		case motionTypeThrown:
		case motionTypeShot:
			mt->ballisticAction();
			break;

		case motionTypeWalk:
			mt->walkAction();
			break;

		case motionTypeClimbUp:
			mt->upLadderAction();
			break;

		case motionTypeClimbDown:
			mt->downLadderAction();
			break;

		case motionTypeTalk:

			if (mt->flags & reset) {
				a->setAction(actionStand, 0);
				a->_cycleCount = g_vm->_rnd->getRandomNumber(3);
				mt->flags &= ~(reset | nextAnim);
			}
			if (a->_cycleCount == 0) {
				a->setAction(actionTalk, 0);
				mt->flags |= nextAnim;
				a->_cycleCount = -1;
			} else if (mt->flags & nextAnim) {
				if (a->nextAnimationFrame()) {
					a->setAction(actionStand, 0);
					a->_cycleCount = g_vm->_rnd->getRandomNumber(3);
					mt->flags &= ~nextAnim;
				}
			} else
				a->_cycleCount--;
			break;

		case motionTypeLand:
		case motionTypeLandBadly:

			if (mt->flags & reset) {
				int16   newAction = mt->motionType == motionTypeLand
				                    ?   actionJumpUp
				                    :   actionFallBadly;

				if (!a->isActionAvailable(newAction)) {
					if (mt->prevMotionType == motionTypeWalk) {
						mt->motionType = mt->prevMotionType;
						if (mt->flags & pathFind) {
							mt->changeTarget(
							    mt->finalTarget,
							    (mt->flags & requestRun) != 0);
						} else {
							mt->changeDirectTarget(
							    mt->finalTarget,
							    (mt->flags & requestRun) != 0);
						}
						g_vm->_mTaskList->_nextMT = it;
					}
				} else {
					a->setAction(newAction, 0);
					a->setInterruptablity(false);
					mt->flags &= ~reset;
				}
			} else if (a->nextAnimationFrame() || (mt->flags & inWater)) {
				if (mt->prevMotionType == motionTypeWalk) {
					mt->motionType = mt->prevMotionType;
					if (mt->flags & pathFind) {
						mt->changeTarget(
						    mt->finalTarget,
						    (mt->flags & requestRun) != 0);
					} else {
						mt->changeDirectTarget(
						    mt->finalTarget,
						    (mt->flags & requestRun) != 0);
					}
					g_vm->_mTaskList->_nextMT = it;
				} else if (mt->freeFall(obj->_data.location, sti) == false)
					moveTaskDone = true;
			} else {
				//  If actor was running, go through an abreviated
				//  landing sequence by aborting the landing animation
				//  after the first frame.
				if (mt->prevMotionType == motionTypeWalk
				        &&  mt->flags & requestRun
				        &&  mt->runCount == 0
				        &&  !(mt->flags & inWater)) {
					mt->motionType = mt->prevMotionType;
					if (mt->flags & pathFind) {
						mt->changeTarget(
						    mt->finalTarget,
						    (mt->flags & requestRun) != 0);
					} else {
						mt->changeDirectTarget(
						    mt->finalTarget,
						    (mt->flags & requestRun) != 0);
					}
					g_vm->_mTaskList->_nextMT = it;
				}
			}
			break;

		case motionTypeJump:

			if (mt->flags & reset) {
				a->setAction(actionJumpUp, 0);
				a->setInterruptablity(false);
				mt->flags &= ~reset;
			} else if (a->nextAnimationFrame()) {
				mt->motionType = motionTypeThrown;
				a->setAction(actionFreeFall, 0);
			}
			break;

		case motionTypeTurn:

			mt->turnAction();
			break;

		case motionTypeGive:

			mt->giveAction();
			break;

		case motionTypeRise:

			if (a->_data.location.z < mt->immediateLocation.z) {
				a->_data.location.z++;
				if (mt->flags & nextAnim)
					a->nextAnimationFrame();
				mt->flags ^= nextAnim;
			} else {
				targetVector = mt->finalTarget - obj->_data.location;
				targetDist = targetVector.quickHDistance();

				if (targetDist > kTileUVSize) {
					mt->motionType = mt->prevMotionType;
					mt->flags |= reset;
					g_vm->_mTaskList->_nextMT = it;
				} else
					moveTaskDone = true;
			}
			break;

		case motionTypeWait:

			if (mt->flags & reset) {
				mt->actionCounter = 5;
				mt->flags &= ~reset;
			} else if (--mt->actionCounter == 0)
				moveTaskDone = true;
			break;

		case motionTypeUseObject:

			//  This will be uninterrutable for 2 frames
			a->setActionPoints(2);
			mt->o.directObject->use(a->thisID());
			//g_vm->_mTaskList->_nextMT=mt;
			moveTaskDone = true;
			break;

		case motionTypeUseObjectOnObject:

			if (isWorld(mt->o.indirectObject->IDParent())) {
				if (
				    1
#ifdef THIS_SHOULD_BE_IN_TILEMODE
				    a->inUseRange(
				        mt->o.indirectObject->getLocation(),
				        mt->o.directObject)
#endif
				) {
					mt->direction = (mt->o.indirectObject->getLocation()
					                 -   a->getLocation()).quickDir();
					if (a->_currentFacing != mt->direction)
						a->turn(mt->direction);
					else {
						//  The actor will now be uniterruptable
						a->setActionPoints(2);
						mt->o.directObject->useOn(
						    a->thisID(),
						    mt->o.indirectObject->thisID());
						if (mt->motionType == motionTypeUseObjectOnObject)
							moveTaskDone = true;
						else
							g_vm->_mTaskList->_nextMT = it;
					}
				}
			} else {
				//  The actor will now be uniterruptable
				a->setActionPoints(2);
				mt->o.directObject->useOn(
				    a->thisID(),
				    mt->o.indirectObject->thisID());
				if (mt->motionType == motionTypeUseObjectOnObject)
					moveTaskDone = true;
				else
					g_vm->_mTaskList->_nextMT = it;
			}

			break;

		case motionTypeUseObjectOnTAI:

			if (mt->flags & reset) {
				TilePoint       actorLoc = a->getLocation(),
				                TAILoc;
				TileRegion      TAIReg;
				ActiveItem      *TAG = mt->o.TAI->getGroup();

				//  Compute in points the region of the TAI
				TAIReg.min.u = mt->o.TAI->_data.instance.u << kTileUVShift;
				TAIReg.min.v = mt->o.TAI->_data.instance.v << kTileUVShift;
				TAIReg.max.u =      TAIReg.min.u
				                    + (TAG->_data.group.uSize << kTileUVShift);
				TAIReg.max.v =      TAIReg.min.v
				                    + (TAG->_data.group.vSize << kTileUVShift);
				TAIReg.min.z = TAIReg.max.z = 0;

				//  Find the point on the TAI closest to the actor
				TAILoc.u = clamp(TAIReg.min.u, actorLoc.u, TAIReg.max.u - 1);
				TAILoc.v = clamp(TAIReg.min.v, actorLoc.v, TAIReg.max.v - 1);
				TAILoc.z = actorLoc.z;

				//  Compute the direction from the actor to the TAI
				mt->direction = (TAILoc - actorLoc).quickDir();
				mt->flags &= ~reset;
			}

			if (a->_currentFacing != mt->direction)
				a->turn(mt->direction);
			else {
				//  The actor will now be uniterruptable
				a->setActionPoints(2);
				mt->o.directObject->useOn(a->thisID(), mt->o.TAI);
				if (mt->motionType == motionTypeUseObjectOnTAI)
					moveTaskDone = true;
				else
					g_vm->_mTaskList->_nextMT = it;
			}
			break;

		case motionTypeUseObjectOnLocation:

			if (mt->flags & reset) {
				mt->direction = (mt->targetLoc - a->getLocation()).quickDir();
				mt->flags &= ~reset;
			}

			if (a->_currentFacing != mt->direction)
				a->turn(mt->direction);
			else {
				//  The actor will now be uniterruptable
				a->setActionPoints(2);
				mt->o.directObject->useOn(a->thisID(), mt->targetLoc);
				if (mt->motionType == motionTypeUseObjectOnLocation)
					moveTaskDone = true;
				else
					g_vm->_mTaskList->_nextMT = it;
			}
			break;

		case motionTypeUseTAI:

			if (mt->flags & reset) {
				TilePoint       actorLoc = a->getLocation(),
				                TAILoc;
				TileRegion      TAIReg;
				ActiveItem      *TAG = mt->o.TAI->getGroup();

				//  Compute in points the region of the TAI
				TAIReg.min.u = mt->o.TAI->_data.instance.u << kTileUVShift;
				TAIReg.min.v = mt->o.TAI->_data.instance.v << kTileUVShift;
				TAIReg.max.u =      TAIReg.min.u
				                    + (TAG->_data.group.uSize << kTileUVShift);
				TAIReg.max.v =      TAIReg.min.v
				                    + (TAG->_data.group.vSize << kTileUVShift);
				TAIReg.min.z = TAIReg.max.z = 0;

				//  Find the point on the TAI closest to the actor
				TAILoc.u = clamp(TAIReg.min.u, actorLoc.u, TAIReg.max.u - 1);
				TAILoc.v = clamp(TAIReg.min.v, actorLoc.v, TAIReg.max.v - 1);
				TAILoc.z = actorLoc.z;

				//  Compute the direction from the actor to the TAI
				mt->direction = (TAILoc - actorLoc).quickDir();
				mt->flags &= ~reset;
			}

			if (a->_currentFacing != mt->direction)
				a->turn(mt->direction);
			else {
				//  The actor will now be uniterruptable
				a->setActionPoints(2);
				mt->o.TAI->use(a->thisID());
				moveTaskDone = true;
			}
			break;

		case motionTypeDropObject:

			if (isWorld(mt->targetLoc.context)) {
				if (mt->flags & reset) {
					mt->direction = (mt->targetLoc - a->getLocation()).quickDir();
					mt->flags &= ~reset;
				}

				if (a->_currentFacing != mt->direction)
					a->turn(mt->direction);
				else {
					//  The actor will now be uniterruptable
					a->setActionPoints(2);
					mt->o.directObject->drop(a->thisID(),
					                       mt->targetLoc,
					                       mt->moveCount);
					if (mt->motionType == motionTypeDropObject)
						moveTaskDone = true;
					else
						g_vm->_mTaskList->_nextMT = it;
				}
			} else {
				//  The actor will now be uniterruptable
				a->setActionPoints(2);
				mt->o.directObject->drop(a->thisID(),
				                       mt->targetLoc,
				                       mt->moveCount);
				if (mt->motionType == motionTypeDropObject)
					moveTaskDone = true;
				else
					g_vm->_mTaskList->_nextMT = it;
			}

			CMassWeightIndicator::bRedraw = true;   // tell the mass/weight indicators to refresh

			break;

		case motionTypeDropObjectOnObject:

			if (isWorld(mt->o.indirectObject->IDParent())) {
				mt->direction = (mt->o.indirectObject->getLocation()
				                 -   a->getLocation()).quickDir();
				if (a->_currentFacing != mt->direction)
					a->turn(mt->direction);
				else {
					//  The actor will now be uniterruptable
					a->setActionPoints(2);
					mt->o.directObject->dropOn(
					    a->thisID(),
					    mt->o.indirectObject->thisID(),
					    mt->moveCount);
					if (mt->motionType == motionTypeDropObjectOnObject)
						moveTaskDone = true;
					else
						g_vm->_mTaskList->_nextMT = it;
				}
			} else {
				//  The actor will now be uniterruptable
				a->setActionPoints(2);
				mt->o.directObject->dropOn(
				    a->thisID(),
				    mt->o.indirectObject->thisID(),
				    mt->moveCount);
				if (mt->motionType == motionTypeDropObjectOnObject)
					moveTaskDone = true;
				else
					g_vm->_mTaskList->_nextMT = it;
			}

			CMassWeightIndicator::bRedraw = true;   // tell the mass/weight indicators to refresh

			break;

		case motionTypeDropObjectOnTAI:

			if (mt->flags & reset) {
				mt->direction = (mt->targetLoc - a->getLocation()).quickDir();
				mt->flags &= ~reset;
			}

			if (a->_currentFacing != mt->direction)
				a->turn(mt->direction);
			else {
				//  The actor will now be uniterruptable
				a->setActionPoints(2);
				mt->o.directObject->dropOn(
				    a->thisID(),
				    mt->o.TAI,
				    mt->targetLoc);
				if (mt->motionType == motionTypeDropObjectOnTAI)
					moveTaskDone = true;
				else
					g_vm->_mTaskList->_nextMT = it;
			}
			break;

		case motionTypeTwoHandedSwing:
			mt->twoHandedSwingAction();
			break;

		case motionTypeOneHandedSwing:
			mt->oneHandedSwingAction();
			break;

		case motionTypeFireBow:
			mt->fireBowAction();
			break;

		case motionTypeCastSpell:
			mt->castSpellAction();
			break;

		case motionTypeUseWand:
			mt->useWandAction();
			break;

		case motionTypeTwoHandedParry:
			mt->twoHandedParryAction();
			break;

		case motionTypeOneHandedParry:
			mt->oneHandedParryAction();
			break;

		case motionTypeShieldParry:
			mt->shieldParryAction();
			break;

		case motionTypeDodge:
			mt->dodgeAction();
			break;

		case motionTypeAcceptHit:
			mt->acceptHitAction();
			break;

		case motionTypeFallDown:
			mt->fallDownAction();
			break;

		case motionTypeDie:
			if (mt->flags & reset) {
				if (a->isActionAvailable(actionDie)) {
					a->setAction(actionDie, 0);
					a->setInterruptablity(false);
					mt->flags &= ~reset;
				} else {
					moveTaskDone = true;
					a->setInterruptablity(true);
					if (!a->hasEffect(actorDisappearOnDeath)) {
						a->setAction(actionDead, 0);
						a->die();
					} else {
						a->die();
						a->dropInventory();
						a->deleteObjectRecursive();
					}
				}
			} else if (a->nextAnimationFrame()) {
				moveTaskDone = true;
				a->setInterruptablity(true);
				if (!a->hasEffect(actorDisappearOnDeath)) {
					a->setAction(actionDead, 0);
					a->die();
				} else {
					a->die();
					a->dropInventory();
					a->deleteObjectRecursive();
				}
			}

			break;
		}
		if (moveTaskDone) mt->remove();
	}
}

//-----------------------------------------------------------------------
//	Manages any object which has no supporting surface.
//	Returns true if object is still falling.

bool MotionTask::freeFall(TilePoint &newPos, StandingTileInfo &sti) {
	int16           tHeight;
	TilePoint       tPos;
	uint8           objCrossSection;

	tHeight = tileSlopeHeight(newPos, object, &sti);

	if (object->_data.objectFlags & objectFloating) return false;

	velocity.u = (newPos.u - object->_data.location.u) * 2 / 3;
	velocity.v = (newPos.v - object->_data.location.v) * 2 / 3;
	velocity.z = (newPos.z - object->_data.location.z) * 2 / 3;
//	velocity.z = 0;

	//  If terrain is HIGHER (or even sligtly lower) than we are
	//  currently at, then try climbing it.

	if (tHeight >= newPos.z - gravity * 4) {
supported:
		if (motionType != motionTypeWalk
		        ||  tHeight <= newPos.z
		        ||  !(flags & inWater)) {
			if (tHeight > newPos.z + kMaxStepHeight) {
				unstickObject(object);
				tHeight = tileSlopeHeight(newPos, object, &sti);
			}
			newPos.z = tHeight;
//			setObjectSurface( object, sti );
			return false;
		} else {
			motionType = motionTypeRise;
			immediateLocation.z = tHeight;
			object->move(newPos);
			return true;
		}

	}

	//  Otherwise, begin a fall sequence...
	tPos = newPos;

	//  Attempt to solve cases where he gets stuck in falling,
	//  by checking the contact of what he's about to fall on.
	if (tPos.z > tHeight) tPos.z--;
	//  See if we fell on something.
	if (checkContact(object, tPos) == blockageNone) {
falling:
		if (motionType != motionTypeWalk
				||  newPos.z > gravity * 4
				||  tHeight >= 0) {
			motionType = motionTypeThrown;

//			newPos = tPos;
			object->move(tPos);
			return true;
		} else {
			newPos = tPos;
			return false;
		}
	}

	//  If we fall on something, reduce velocity due to impact.
	//  Try a couple of probes to see if we can fall in
	//  other directions.
	objCrossSection = object->proto()->crossSection;

	tPos.u += objCrossSection;
	if (!checkBlocked(object, tPos)
			&&  !checkContact(object, tPos))
		goto falling;

	tPos.u -= objCrossSection * 2;
	if (!checkBlocked(object, tPos)
			&&  !checkContact(object, tPos))
		goto falling;

	tPos.u += objCrossSection;
	tPos.v += objCrossSection;
	if (!checkBlocked(object, tPos)
			&&  !checkContact(object, tPos))
		goto falling;

	tPos.v -= objCrossSection * 2;
	if (!checkBlocked(object, tPos)
			&&  !checkContact(object, tPos))
		goto falling;

	//  There is no support for the object and there is no place to fall
	//  so cheat and pretend this whole mess never happened.
	tPos = newPos;

	tPos.u += objCrossSection;
	tHeight = tileSlopeHeight(tPos, object, &sti);
	if (tHeight <= tPos.z + kMaxStepHeight
			&&  tHeight >= tPos.z - gravity * 4) {
		newPos = tPos;
		goto supported;
	}

	tPos.u -= objCrossSection * 2;
	tHeight = tileSlopeHeight(tPos, object, &sti);
	if (tHeight <= tPos.z + kMaxStepHeight
			&&  tHeight >= tPos.z - gravity * 4) {
		newPos = tPos;
		goto supported;
	}

	tPos.u += objCrossSection;
	tPos.v += objCrossSection;
	tHeight = tileSlopeHeight(tPos, object, &sti);
	if (tHeight <= tPos.z + kMaxStepHeight
			&&  tHeight >= tPos.z - gravity * 4) {
		newPos = tPos;
		goto supported;
	}

	tPos.v -= objCrossSection * 2;
	tHeight = tileSlopeHeight(tPos, object, &sti);
	if (tHeight <= tPos.z + kMaxStepHeight
			&&  tHeight >= tPos.z - gravity * 4) {
		newPos = tPos;
		goto supported;
	}

	//  If we STILL cannot find support for the object, change its
	//  position and try again.  This should be very rare.
	newPos.z--;
	object->move(newPos);
	unstickObject(object);
	newPos = object->getLocation();
	return true;
}

//-----------------------------------------------------------------------
//	Calls the handling routine for each active motion task

void moveActors(int32 deltaTime) {
	MotionTask::updatePositions();
}

//-----------------------------------------------------------------------
//	Check the actor's area to see if he is intersecting ladder terrain, and
//	if so, make him climb it.

bool checkLadder(Actor *a, const TilePoint &loc) {
	TileRegion          actorTileReg;
	uint8               crossSection = a->proto()->crossSection,
	                    height = a->proto()->height;
	int16               mapNum = a->getMapNum();
	TileInfo            *ti;
	TilePoint           tileLoc;
	StandingTileInfo    sti = {nullptr, nullptr, {0, 0, 0}, 0};

	actorTileReg.min.u = (loc.u - crossSection) >> kTileUVShift;
	actorTileReg.min.v = (loc.v - crossSection) >> kTileUVShift;
	actorTileReg.max.u = (loc.u + crossSection + kTileUVMask) >> kTileUVShift;
	actorTileReg.max.v = (loc.v + crossSection + kTileUVMask) >> kTileUVShift;
	actorTileReg.min.z = actorTileReg.max.z = 0;

	TileIterator    iter(mapNum, actorTileReg);

	for (ti = iter.first(&tileLoc, &sti);
	        ti != nullptr;
	        ti = iter.next(&tileLoc, &sti)) {
		if (!(ti->combinedTerrainMask() & terrainLadder)) continue;

		if (sti.surfaceHeight + ti->attrs.terrainHeight < loc.z
		        ||  sti.surfaceHeight > loc.z + height)
			continue;

		uint16      footPrintMask = 0xFFFF,
		            ladderMask;
		TilePoint   subTileLoc(
		    tileLoc.u << kTileSubShift,
		    tileLoc.v << kTileSubShift,
		    0);
		TileRegion  actorSubTileReg;

		actorSubTileReg.min.u = (loc.u - crossSection) >> kSubTileShift;
		actorSubTileReg.min.v = (loc.v - crossSection) >> kSubTileShift;
		actorSubTileReg.max.u =
		    (loc.u + crossSection + kSubTileMask) >> kSubTileShift;
		actorSubTileReg.max.v =
		    (loc.v + crossSection + kSubTileMask) >> kSubTileShift;

		if (actorSubTileReg.min.u >= subTileLoc.u)
			footPrintMask &=
			    uMinMasks[actorSubTileReg.min.u - subTileLoc.u];

		if (actorSubTileReg.min.v >= subTileLoc.v)
			footPrintMask &=
			    vMinMasks[actorSubTileReg.min.v - subTileLoc.v];

		if (actorSubTileReg.max.u < subTileLoc.u + kTileSubSize)
			footPrintMask &=
			    uMaxMasks[actorSubTileReg.max.u - subTileLoc.u];

		if (actorSubTileReg.max.v < subTileLoc.v + kTileSubSize)
			footPrintMask &=
			    vMaxMasks[actorSubTileReg.max.v - subTileLoc.v];

		ladderMask =    ti->attrs.fgdTerrain == terrNumLadder
		                ?   ti->attrs.terrainMask
		                :   ~ti->attrs.terrainMask;

		if (footPrintMask & ladderMask) {
			if (!(~ladderMask & 0xF000)) {
				a->_currentFacing = 7;
				a->move(
				    TilePoint(
				        (tileLoc.u << kTileUVShift)
				        +   kTileUVSize
				        -   crossSection,
				        (tileLoc.v << kTileUVShift) + kTileUVSize / 2,
				        loc.z));
			} else if (!(~ladderMask & 0x000F)) {
				a->_currentFacing = 3;
				a->move(
				    TilePoint(
				        (tileLoc.u << kTileUVShift) + crossSection,
				        (tileLoc.v << kTileUVShift) + kTileUVSize / 2,
				        loc.z));
			} else if (!(~ladderMask & 0x8888)) {
				a->_currentFacing = 1;
				a->move(
				    TilePoint(
				        (tileLoc.u << kTileUVShift) + kTileUVSize / 2,
				        (tileLoc.v << kTileUVShift)
				        +   kTileUVSize
				        -   crossSection,
				        loc.z));
			} else {
				a->_currentFacing = 3;
				a->move(
				    TilePoint(
				        (tileLoc.u << kTileUVShift) + kTileUVSize / 2,
				        (tileLoc.v << kTileUVShift) + crossSection,
				        loc.z));
			}

			if (loc.z
			        <   tileSlopeHeight(a->getLocation(), a) + kMaxStepHeight)
				MotionTask::upLadder(*a);
			else
				MotionTask::downLadder(*a);

			return true;
		}
	}

	return false;
}


void pauseInterruptableMotions() {
	interruptableMotionsPaused = true;
}

void resumeInterruptableMotions() {
	interruptableMotionsPaused = false;
}

/* ===================================================================== *
   MotionTask list management functions
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Initialize the motion task list

void initMotionTasks() {
	//  Simply call the default MotionTaskList constructor
	//new (g_vm->_mTaskList) MotionTaskList;
}

void saveMotionTasks(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving MotionTasks");

	outS->write("MOTN", 4);
	CHUNK_BEGIN;
	g_vm->_mTaskList->write(out);
	CHUNK_END;
}

void loadMotionTasks(Common::InSaveFile *in, int32 chunkSize) {
	debugC(2, kDebugSaveload, "Loading MotionTasks");

	//  If there is no saved data, simply call the default constructor
	if (chunkSize == 0) {
		//new (g_vm->_mTaskList) MotionTaskList;
		return;
	}

	//  Reconstruct g_vm->_mTaskList from archived data
	g_vm->_mTaskList->read(in);
}

//-----------------------------------------------------------------------
//	Cleanup the motion task list

void cleanupMotionTasks() {
	//  Simply call stackList's cleanup
	g_vm->_mTaskList->cleanup();
}

} // end of namespace Saga2
