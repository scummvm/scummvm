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
		(*it)->_pathFindTask = nullptr;

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

		if ((*it)->_object == obj) {
			mt = *it;
			wakeUpThread(mt->_thread, motionInterrupted);
			mt->_thread = NoThread;

			break;
		}
	}

	if (mt == nullptr) {
		mt = new MotionTask;

		mt->_object = obj;
		mt->_motionType = mt->_prevMotionType = MotionTask::motionTypeNone;
		mt->_pathFindTask = nullptr;
		mt->_pathCount = -1;
		mt->_flags = 0;
		mt->_velocity = TilePoint(0, 0, 0);
		mt->_immediateLocation = mt->_finalTarget = obj->getLocation();
		mt->_thread = NoThread;

		mt->_targetObj = nullptr;
		mt->_targetTAG = nullptr;
		mt->_spellObj = nullptr;

		_list.push_back(mt);

		if (isActor(obj))
			((Actor *)obj)->_moveTask = mt;
	}

	obj->_data.objectFlags |= kObjectMoving;

	return mt;
}

/* ===================================================================== *
   MotionTask member functions
 * ===================================================================== */

void MotionTask::read(Common::InSaveFile *in) {
	ObjectID    objectID;

	//  Restore the motion type and previous motion type
	_motionType = in->readByte();
	_prevMotionType = in->readByte();

	//  Restore the thread ID
	_thread = in->readSint16LE();

	//  Restore the motion flags
	_flags = in->readUint16LE();

	//  Get the object ID
	objectID = in->readUint16LE();

	//  Convert the object ID to and object address
	_object =    objectID != Nothing
	            ?   GameObject::objectAddress(objectID)
	            :   nullptr;

	//  If the object is an actor, plug this motion task into the actor
	if (_object && isActor(_object))
		((Actor *)_object)->_moveTask = this;

	if (_motionType == motionTypeWalk
	        ||  _prevMotionType == motionTypeWalk) {
		//  Restore the target _data.locations
		_immediateLocation.load(in);
		_finalTarget.load(in);

		//  If there is a tether restore it
		if (_flags & tethered) {
			_tetherMinU = in->readSint16LE();
			_tetherMinV = in->readSint16LE();
			_tetherMaxU = in->readSint16LE();
			_tetherMaxV = in->readSint16LE();
		}

		//  Restore the direction
		_direction = in->readByte();

		//  Restore the path index and path count
		_pathIndex = in->readSint16LE();
		_pathCount = in->readSint16LE();
		_runCount = in->readSint16LE();

		//  Restore the action counter if needed
		if (_flags & agitated)
			actionCounter = in->readSint16LE();

		//  If there were valid path way points, restore those
		if (_pathIndex >= 0 && _pathIndex < _pathCount) {
			int16 wayPointIndex = _pathIndex;

			while (wayPointIndex < _pathCount) {
				_pathList[wayPointIndex].load(in);

				wayPointIndex++;
			}
		}

		//  If this motion task previously had a path finding request
		//  it must be restarted
		_pathFindTask = nullptr;
	}

	if (_motionType == motionTypeThrown || _motionType == motionTypeShot) {
		//  Restore the velocity
		_velocity.load(in);

		//  Restore other ballistic motion variables
		_steps = in->readSint16LE();
		_uFrac = in->readSint16LE();
		_vFrac = in->readSint16LE();
		_uErrorTerm = in->readSint16LE();
		_vErrorTerm = in->readSint16LE();

		if (_motionType == motionTypeShot) {
			ObjectID _targetObjID,
			         enactorID;

			_targetObjID = in->readUint16LE();

			_targetObj = _targetObjID
			            ?   GameObject::objectAddress(_targetObjID)
			            :   nullptr;

			enactorID = in->readUint16LE();

			_o.enactor = enactorID != Nothing
			            ? (Actor *)GameObject::objectAddress(enactorID)
			            :   nullptr;
		}
	} else if (_motionType == motionTypeClimbUp
	           ||  _motionType == motionTypeClimbDown) {
		_immediateLocation.load(in);
	} else if (_motionType == motionTypeJump) {
		_velocity.load(in);
	} else if (_motionType == motionTypeTurn) {
		_direction = in->readByte();
	} else if (_motionType == motionTypeGive) {
		ObjectID id = in->readUint16LE();
		_targetObj = id != Nothing
		            ?   GameObject::objectAddress(id)
		            :   nullptr;
	} else if (_motionType == motionTypeWait) {
		actionCounter = in->readSint16LE();
	} else if (_motionType == motionTypeUseObject
	           ||  _motionType == motionTypeUseObjectOnObject
	           ||  _motionType == motionTypeUseObjectOnTAI
	           ||  _motionType == motionTypeUseObjectOnLocation
	           ||  _motionType == motionTypeDropObject
	           ||  _motionType == motionTypeDropObjectOnObject
	           ||  _motionType == motionTypeDropObjectOnTAI) {
	    ObjectID directObjID = in->readUint16LE();
		_o.directObject = directObjID != Nothing
		                ?   GameObject::objectAddress(directObjID)
		                :   nullptr;

		_direction = in->readByte();

		if (_motionType == motionTypeUseObjectOnObject
		        ||  _motionType == motionTypeDropObjectOnObject) {
		    ObjectID indirectObjID = in->readUint16LE();
			_o.indirectObject =  indirectObjID != Nothing
			                    ?   GameObject::objectAddress(indirectObjID)
			                    :   nullptr;
		} else {
			if (_motionType == motionTypeUseObjectOnTAI
			        ||  _motionType == motionTypeDropObjectOnTAI) {
			    ActiveItemID tai(in->readSint16LE());
				_o.TAI = tai != NoActiveItem
				        ?   ActiveItem::activeItemAddress(tai)
				        :   nullptr;
			}

			if (_motionType == motionTypeUseObjectOnLocation
			        ||  _motionType == motionTypeDropObject
			        ||  _motionType == motionTypeDropObjectOnTAI) {
				_targetLoc.load(in);
			}
		}
	} else if (_motionType == motionTypeUseTAI) {
		ActiveItemID tai(in->readSint16LE());
		_o.TAI = tai != NoActiveItem
		        ?   ActiveItem::activeItemAddress(tai)
		        :   nullptr;

		_direction = in->readByte();
	} else if (_motionType == motionTypeTwoHandedSwing
	           ||  _motionType == motionTypeOneHandedSwing
	           ||  _motionType == motionTypeFireBow
	           ||  _motionType == motionTypeCastSpell
	           ||  _motionType == motionTypeUseWand) {
		ObjectID    _targetObjID;

		//  Restore the direction
		_direction = in->readByte();

		//  Restore the combat motion type
		_combatMotionType = in->readByte();

		//  Get the target object ID
		_targetObjID = in->readUint16LE();

		//  Convert the target object ID to a pointer
		_targetObj = _targetObjID != Nothing
		            ?   GameObject::objectAddress(_targetObjID)
		            :   nullptr;

		if (_motionType == motionTypeCastSpell) {
			SpellID sid       ;
			ObjectID toid     ;
			ActiveItemID ttaid;

			//  restore the spell prototype
			warning("MotionTask::read: Check SpellID size");
			sid = (SpellID)in->readUint32LE();
			_spellObj = sid != nullSpell
			           ? skillProtoFromID(sid)
			           : nullptr;

			//  restore object target
			toid = in->readUint16LE();
			_targetObj = toid != Nothing
			            ?   GameObject::objectAddress(toid)
			            :   nullptr;

			//  restore TAG target
			ttaid = in->readSint16LE();
			_targetTAG = ttaid != NoActiveItem
			            ?  ActiveItem::activeItemAddress(ttaid)
			            :  nullptr;

			//  restore _data.location target
			_targetLoc.load(in);
		}

		//  Restore the action counter
		actionCounter = in->readSint16LE();
	} else if (_motionType == motionTypeTwoHandedParry
	           ||  _motionType == motionTypeOneHandedParry
	           ||  _motionType == motionTypeShieldParry) {
		ObjectID attackerID,
		         defensiveObjID;

		//  Restore the direction
		_direction = in->readByte();

		//  Get the attacker's and defensive object's IDs
		attackerID = in->readByte();
		defensiveObjID = in->readByte();

		//  Convert IDs to pointers
		_d.attacker = attackerID != Nothing
		            ? (Actor *)GameObject::objectAddress(attackerID)
		            :   nullptr;

		_d.defensiveObj = defensiveObjID != Nothing
		                ?   GameObject::objectAddress(defensiveObjID)
		                :   nullptr;

		//  Restore the defense flags
		_d.defenseFlags = in->readByte();

		//  Restore the action counter
		actionCounter = in->readSint16LE();

		if (_motionType == motionTypeOneHandedParry) {
			//  Restore the combat sub-motion type
			_combatMotionType = in->readByte();
		}
	} else if (_motionType == motionTypeDodge
	           ||  _motionType == motionTypeAcceptHit
	           ||  _motionType == motionTypeFallDown) {
		ObjectID        attackerID;

		//  Get the attacker's ID
		attackerID = in->readUint16LE();

		//  Convert ID to pointer
		_d.attacker = attackerID != Nothing
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

	size =      sizeof(_motionType)
	            +   sizeof(_prevMotionType)
	            +   sizeof(_thread)
	            +   sizeof(_flags)
	            +   sizeof(ObjectID);            //  object

	if (_motionType == motionTypeWalk
	        ||  _prevMotionType == motionTypeWalk) {
		size +=     sizeof(_immediateLocation)
		            +   sizeof(_finalTarget);

		if (_flags & tethered) {
			size +=     sizeof(_tetherMinU)
			            +   sizeof(_tetherMinV)
			            +   sizeof(_tetherMaxU)
			            +   sizeof(_tetherMaxV);
		}

		size +=     sizeof(_direction)
		            +   sizeof(_pathIndex)
		            +   sizeof(_pathCount)
		            +   sizeof(_runCount);

		if (_flags & agitated)
			size += sizeof(actionCounter);

		if (_pathIndex >= 0 && _pathIndex < _pathCount)
			size += sizeof(TilePoint) * (_pathCount - _pathIndex);
	}

	if (_motionType == motionTypeThrown || _motionType == motionTypeShot) {
		size +=     sizeof(_velocity)
		            +   sizeof(_steps)
		            +   sizeof(_uFrac)
		            +   sizeof(_vFrac)
		            +   sizeof(_uErrorTerm)
		            +   sizeof(_vErrorTerm);

		if (_motionType == motionTypeShot) {
			size +=     sizeof(ObjectID)         //  _targetObj ID
			            +   sizeof(ObjectID);        //  enactor ID
		}
	} else if (_motionType == motionTypeClimbUp
	           ||  _motionType == motionTypeClimbDown) {
		size += sizeof(_immediateLocation);
	} else if (_motionType == motionTypeJump) {
		size += sizeof(_velocity);
	} else if (_motionType == motionTypeTurn) {
		size += sizeof(_direction);
	} else if (_motionType == motionTypeGive) {
		size += sizeof(ObjectID);        //  _targetObj ID
	} else if (_motionType == motionTypeUseObject
	           ||  _motionType == motionTypeUseObjectOnObject
	           ||  _motionType == motionTypeUseObjectOnTAI
	           ||  _motionType == motionTypeUseObjectOnLocation
	           ||  _motionType == motionTypeDropObject
	           ||  _motionType == motionTypeDropObjectOnObject
	           ||  _motionType == motionTypeDropObjectOnTAI) {
		size +=     sizeof(ObjectID)
		            +   sizeof(_direction);

		if (_motionType == motionTypeUseObjectOnObject
		        ||  _motionType == motionTypeDropObjectOnObject) {
			size += sizeof(ObjectID);
		} else {
			if (_motionType == motionTypeUseObjectOnTAI
			        ||  _motionType == motionTypeDropObjectOnTAI) {
				size += sizeof(ActiveItemID);
			}

			if (_motionType == motionTypeUseObjectOnLocation
			        ||  _motionType == motionTypeDropObject
			        ||  _motionType == motionTypeDropObjectOnTAI) {
				size += sizeof(_targetLoc);
			}
		}
	} else if (_motionType == motionTypeUseTAI) {
		size +=     sizeof(ActiveItemID)
		            +   sizeof(_direction);
	} else if (_motionType == motionTypeTwoHandedSwing
	           ||  _motionType == motionTypeOneHandedSwing
	           ||  _motionType == motionTypeFireBow
	           ||  _motionType == motionTypeCastSpell
	           ||  _motionType == motionTypeUseWand) {
		size +=     sizeof(_direction)
		            +   sizeof(_combatMotionType)
		            +   sizeof(ObjectID);            //  _targetObj

		if (_motionType == motionTypeCastSpell) {
			size += sizeof(SpellID);             //  _spellObj
			size += sizeof(ObjectID);            //  _targetObj
			size += sizeof(ActiveItemID);        //  _targetTAG
			size += sizeof(_targetLoc);           //  _targetLoc
		}

		size +=     sizeof(actionCounter);

	} else if (_motionType == motionTypeTwoHandedParry
	           ||  _motionType == motionTypeOneHandedParry
	           ||  _motionType == motionTypeShieldParry) {
		size +=     sizeof(_direction)
		            +   sizeof(ObjectID)             //  attacker ID
		            +   sizeof(ObjectID)             //  defensiveObj ID
		            +   sizeof(_d.defenseFlags)
		            +   sizeof(actionCounter);

		if (_motionType == motionTypeOneHandedParry)
			size += sizeof(_combatMotionType);
	} else if (_motionType == motionTypeDodge
	           ||  _motionType == motionTypeAcceptHit
	           ||  _motionType == motionTypeFallDown) {
		size +=     sizeof(ObjectID)             //  attacker ID
		            +   sizeof(actionCounter);
	}

	return size;
}

void MotionTask::write(Common::MemoryWriteStreamDynamic *out) {
	ObjectID    objectID;

	//  Store the motion type and previous motion type
	out->writeByte(_motionType);
	out->writeByte(_prevMotionType);

	//  Store the thread ID
	out->writeSint16LE(_thread);

	//  Store the motion flags
	out->writeUint16LE(_flags);

	//  Convert the object pointer to an object ID
	objectID = _object != nullptr ? _object->thisID() : Nothing;

	//  Store the object ID
	out->writeUint16LE(objectID);

	if (_motionType == motionTypeWalk
	        ||  _prevMotionType == motionTypeWalk) {
		//  Store the target _data.locations
		_immediateLocation.write(out);
		_finalTarget.write(out);

		//  If there is a tether store it
		if (_flags & tethered) {
			out->writeSint16LE(_tetherMinU);
			out->writeSint16LE(_tetherMinV);
			out->writeSint16LE(_tetherMaxU);
			out->writeSint16LE(_tetherMaxV);
		}

		//  Store the direction
		out->writeByte(_direction);

		//  Store the path index and path count
		out->writeSint16LE(_pathIndex);
		out->writeSint16LE(_pathCount);
		out->writeSint16LE(_runCount);

		//  Store the action counter if needed
		if (_flags & agitated)
			out->writeSint16LE(actionCounter);

		//  If there are valid path way points, store them
		if (_pathIndex >= 0 && _pathIndex < _pathCount) {
			int16   wayPointIndex = _pathIndex;

			while (wayPointIndex < _pathCount) {
				_pathList[wayPointIndex].write(out);

				wayPointIndex++;
			}
		}
	}

	if (_motionType == motionTypeThrown || _motionType == motionTypeShot) {
		//  Store the velocity
		_velocity.write(out);

		//  Store other ballistic motion variables
		out->writeSint16LE(_steps);
		out->writeSint16LE(_uFrac);
		out->writeSint16LE(_vFrac);
		out->writeSint16LE(_uErrorTerm);
		out->writeSint16LE(_vErrorTerm);

		if (_motionType == motionTypeShot) {
			ObjectID        _targetObjID,
			                enactorID;

			_targetObjID =   _targetObj != nullptr
			                ?   _targetObj->thisID()
			                :   Nothing;

			out->writeUint16LE(_targetObjID);

			enactorID = _o.enactor != nullptr
			            ?   _o.enactor->thisID()
			            :   Nothing;

			out->writeUint16LE(enactorID);
		}
	} else if (_motionType == motionTypeClimbUp
	           ||  _motionType == motionTypeClimbDown) {
		_immediateLocation.write(out);
	} else if (_motionType == motionTypeJump) {
		_velocity.write(out);
	} else if (_motionType == motionTypeTurn) {
		out->writeByte(_direction);
	} else if (_motionType == motionTypeGive) {
		if (_targetObj != nullptr)
			out->writeUint16LE(_targetObj->thisID());
		else
			out->writeUint16LE(Nothing);
	} else if (_motionType == motionTypeUseObject
	           ||  _motionType == motionTypeUseObjectOnObject
	           ||  _motionType == motionTypeUseObjectOnTAI
	           ||  _motionType == motionTypeUseObjectOnLocation
	           ||  _motionType == motionTypeDropObject
	           ||  _motionType == motionTypeDropObjectOnObject
	           ||  _motionType == motionTypeDropObjectOnTAI) {
		if (_o.directObject != nullptr)
			out->writeUint16LE(_o.directObject->thisID());
		else
			out->writeUint16LE(Nothing);

		out->writeByte(_direction);

		if (_motionType == motionTypeUseObjectOnObject
		        ||  _motionType == motionTypeDropObjectOnObject) {
			if (_o.indirectObject != nullptr)
				out->writeUint16LE(_o.indirectObject->thisID());
			else
				out->writeUint16LE(Nothing);
		} else {
			if (_motionType == motionTypeUseObjectOnTAI
			        ||  _motionType == motionTypeDropObjectOnTAI) {
				if (_o.TAI != nullptr)
					out->writeSint16LE(_o.TAI->thisID());
				else
					out->writeSint16LE(NoActiveItem.val);
			}

			if (_motionType == motionTypeUseObjectOnLocation
			        ||  _motionType == motionTypeDropObject
			        ||  _motionType == motionTypeDropObjectOnTAI) {
				_targetLoc.write(out);
			}
		}
	} else if (_motionType == motionTypeUseTAI) {
		if (_o.TAI != nullptr)
			out->writeSint16LE(_o.TAI->thisID());
		else
			out->writeSint16LE(NoActiveItem.val);

		out->writeByte(_direction);
	} else if (_motionType == motionTypeTwoHandedSwing
	           ||  _motionType == motionTypeOneHandedSwing
	           ||  _motionType == motionTypeFireBow
	           ||  _motionType == motionTypeCastSpell
	           ||  _motionType == motionTypeUseWand) {
		ObjectID    _targetObjID;

		//  Store the direction
		out->writeByte(_direction);

		//  Store the combat motion type
		out->writeByte(_combatMotionType);

		//  Convert the target object pointer to an ID
		_targetObjID = _targetObj != nullptr ? _targetObj->thisID() : Nothing;

		//  Store the target object ID
		out->writeUint16LE(_targetObjID);

		if (_motionType == motionTypeCastSpell) {
			//  Convert the spell object pointer to an ID

			SpellID sid         = _spellObj != nullptr
			                      ? _spellObj->getSpellID()
			                      : nullSpell;

			ObjectID toid       = _targetObj != nullptr
			                      ? _targetObj->thisID()
			                      : Nothing;

			ActiveItemID ttaid  = _targetTAG != nullptr
			                      ? _targetTAG->thisID()
			                      : NoActiveItem;

			//  Store the spell prototype
			warning("MotionTask::write: Check SpellID size");
			out->writeUint32LE(sid);

			//  Store object target
			out->writeUint16LE(toid);

			//  Store TAG target
			out->writeSint16LE(ttaid.val);

			//  Store _data.location target
			_targetLoc.write(out);
		}

		//  Store the action counter
		out->writeSint16LE(actionCounter);

	} else if (_motionType == motionTypeTwoHandedParry
	           ||  _motionType == motionTypeOneHandedParry
	           ||  _motionType == motionTypeShieldParry) {
		ObjectID        attackerID,
		                defensiveObjID;

		//  Store the direction
		out->writeByte(_direction);

		attackerID = _d.attacker != nullptr ? _d.attacker->thisID() : Nothing;
		defensiveObjID = _d.defensiveObj != nullptr ? _d.defensiveObj->thisID() : Nothing;

		//  Store the attacker's and defensive object's IDs
		out->writeUint16LE(attackerID);
		out->writeUint16LE(defensiveObjID);

		//  Store the defense flags
		out->writeByte(_d.defenseFlags);

		//  Store the action counter
		out->writeSint16LE(actionCounter);

		if (_motionType == motionTypeOneHandedParry) {
			//  Store the combat sub-motion type
			out->writeByte(_combatMotionType);
		}
	} else if (_motionType == motionTypeDodge
	           ||  _motionType == motionTypeAcceptHit
	           ||  _motionType == motionTypeFallDown) {
		ObjectID        attackerID;

		attackerID = _d.attacker != nullptr ? _d.attacker->thisID() : Nothing;

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

	_object->_data.objectFlags &= ~kObjectMoving;
	if (objObscured(_object))
		_object->_data.objectFlags |= kObjectObscured;
	else
		_object->_data.objectFlags &= ~kObjectObscured;

	if (isActor(_object)) {
		Actor   *a = (Actor *)_object;

		a->_moveTask = nullptr;
		a->_cycleCount = g_vm->_rnd->getRandomNumber(19);

		//  Make sure the actor is not left in a permanently
		//  uninterruptable state with no motion task to reset it
		if (a->isPermanentlyUninterruptable())
			a->setInterruptablity(true);
	}

	g_vm->_mTaskList->_list.remove(this);

	abortPathFind(this);
	_pathFindTask = nullptr;

	wakeUpThread(_thread, returnVal);
}

//-----------------------------------------------------------------------
//	Determine the immediate target _data.location

TilePoint MotionTask::getImmediateTarget() {
	if (_immediateLocation != Nowhere)
		return _immediateLocation;

	Direction       dir;

	//  If the wandering then simply go in the direction the actor is
	//  facing, else if avoiding a block go in the previously selected
	//  random direction
	if (_flags & agitated)
		dir = _direction;
	else
		dir = ((Actor *)_object)->_currentFacing;

	return  _object->_data.location
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

	_steps = turns;
	_uFrac = vector.u % turns;
	_vFrac = vector.v % turns;
	_uErrorTerm = 0;
	_vErrorTerm = 0;

	veloc.z = ((gravity * turns) >> 1) + vector.z / turns;
	_velocity = veloc;
}

//-----------------------------------------------------------------------
//	This initiates a motion task for turning an actor

void MotionTask::turn(Actor &obj, Direction dir) {
	assert(dir < 8);

	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&obj)) != nullptr) {
		mt->_direction = dir;
		mt->_motionType = motionTypeTurn;
		mt->_flags = reset;
	}
}

//-----------------------------------------------------------------------
//	This initiates a motion task for turning an actor

void MotionTask::turnTowards(Actor &obj, const TilePoint &where) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&obj)) != nullptr) {
		mt->_direction = (where - obj.getLocation()).quickDir();
		mt->_motionType = motionTypeTurn;
		mt->_flags = reset;
	}
}

//-----------------------------------------------------------------------
//	This initiates a motion task for going through the motions of giving
//	an object to another actor

void MotionTask::give(Actor &actor, Actor &givee) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&actor)) != nullptr) {
		mt->_targetObj = &givee;
		mt->_motionType = motionTypeGive;
		mt->_flags = reset;
	}
}

//-----------------------------------------------------------------------
//	This initiates a motion task for throwing an object

void MotionTask::throwObject(GameObject &obj, const TilePoint &velocity) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&obj)) != nullptr) {
		if (obj.isMissile()) obj._data.missileFacing = missileNoFacing;
		mt->_velocity = velocity;
		mt->_motionType = motionTypeThrown;
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
		mt->_motionType = motionTypeThrown;
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
		TilePoint   _targetLoc = target.getLocation();

		_targetLoc.z += target.proto()->height / 2;

		TilePoint   vector = _targetLoc - obj.getLocation();
		int16       turns = MAX(vector.quickHDistance() / speed, 1);

		if (isActor(&target)) {
			Actor       *targetActor = (Actor *)&target;

			if (targetActor->_moveTask != nullptr) {
				MotionTask      *targetMotion = targetActor->_moveTask;

				if (targetMotion->_motionType == motionTypeWalk)
					vector += targetMotion->_velocity * turns;
			}
		}

		mt->calcVelocity(vector, turns);

		if (obj.isMissile())
			obj._data.missileFacing = missileDir(mt->_velocity);

		mt->_motionType = motionTypeShot;
		mt->_o.enactor = &doer;
		mt->_targetObj = &target;
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
			mt->_finalTarget = mt->_immediateLocation = target;
			mt->_motionType = mt->_prevMotionType = motionTypeWalk;
			mt->_pathCount = mt->_pathIndex = 0;
			mt->_flags = pathFind | reset;
			mt->_runCount = 12;          // # of frames until we can run

			if (run && actor.isActionAvailable(kActionRun))
				mt->_flags |= requestRun;
			if (canAgitate)
				mt->_flags |= agitatable;

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
			mt->_pathFindTask = nullptr;

			unstickObject(&actor);
			mt->_motionType = mt->_prevMotionType = motionTypeWalk;
			mt->_finalTarget = mt->_immediateLocation = target;
			mt->_pathCount = mt->_pathIndex = 0;
			mt->_flags = reset;
			mt->_runCount = 12;

			if (run && actor.isActionAvailable(kActionRun))
				mt->_flags |= requestRun;
			if (canAgitate)
				mt->_flags |= agitatable;
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
			mt->_pathFindTask = nullptr;

			unstickObject(&actor);
			mt->_motionType = mt->_prevMotionType = motionTypeWalk;
			mt->_immediateLocation = Nowhere;
			mt->_pathCount = mt->_pathIndex = 0;
			mt->_flags = reset | wandering;
			mt->_runCount = 12;

			if (run && actor.isActionAvailable(kActionRun))
				mt->_flags |= requestRun;

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
			mt->_pathFindTask = nullptr;

			unstickObject(&actor);
			mt->_motionType = mt->_prevMotionType = motionTypeWalk;
			mt->_immediateLocation = Nowhere;
			mt->_tetherMinU = tetherReg.min.u;
			mt->_tetherMinV = tetherReg.min.v;
			mt->_tetherMaxU = tetherReg.max.u;
			mt->_tetherMaxV = tetherReg.max.v;
			mt->_pathCount = mt->_pathIndex = 0;
			mt->_flags = reset | wandering | tethered;
			mt->_runCount = 12;

			if (run && actor.isActionAvailable(kActionRun))
				mt->_flags |= requestRun;

			RequestWanderPath(mt, getPathFindIQ(&actor));
		}
	}
}

//-----------------------------------------------------------------------
//	Create a climb up ladder motion task.

void MotionTask::upLadder(Actor &actor) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&actor)) != nullptr) {
		if (mt->_motionType != motionTypeClimbUp) {
			mt->_motionType = motionTypeClimbUp;
			mt->_flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Create a climb down ladder motion task.

void MotionTask::downLadder(Actor &actor) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&actor)) != nullptr) {
		if (mt->_motionType != motionTypeClimbDown) {
			mt->_motionType = motionTypeClimbDown;
			mt->_flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Create a talk motion task.

void MotionTask::talk(Actor &actor) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&actor)) != nullptr) {
		if (mt->_motionType != motionTypeTalk) {
			mt->_motionType = motionTypeTalk;
			mt->_flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Begin a jump. REM: This should probably have a parameter for jumping
//	forward, backward, etc.

void MotionTask::jump(Actor &actor) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&actor)) != nullptr) {
		if (mt->_motionType != motionTypeThrown) {
			mt->_velocity.z = 10;
			mt->_motionType = motionTypeJump;
			mt->_flags = reset;
		}
	}
}


//-----------------------------------------------------------------------
//	Don't move -- simply eat some time

void MotionTask::wait(Actor &a) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->_motionType != motionTypeWait) {
			mt->_motionType = motionTypeWait;
			mt->_flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Use an object

void MotionTask::useObject(Actor &a, GameObject &dObj) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->_motionType != motionTypeUseObject) {
			mt->_motionType = motionTypeUseObject;
			mt->_o.directObject = &dObj;
			mt->_flags = reset;
			if (isPlayerActor(&a)) mt->_flags |= privledged;
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
		if (mt->_motionType != motionTypeUseObjectOnObject) {
			mt->_motionType = motionTypeUseObjectOnObject;
			mt->_o.directObject = &dObj;
			mt->_o.indirectObject = &target;
			mt->_flags = reset;
			if (isPlayerActor(&a)) mt->_flags |= privledged;
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
		if (mt->_motionType != motionTypeUseObjectOnTAI) {
			mt->_motionType = motionTypeUseObjectOnTAI;
			mt->_o.directObject = &dObj;
			mt->_o.TAI = &target;
			mt->_flags = reset;
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
		if (mt->_motionType != motionTypeUseObjectOnLocation) {
			mt->_motionType = motionTypeUseObjectOnLocation;
			mt->_o.directObject = &dObj;
			mt->_targetLoc = target;
			mt->_flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Use a TAI

void MotionTask::useTAI(Actor &a, ActiveItem &dTAI) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->_motionType != motionTypeUseTAI) {
			mt->_motionType = motionTypeUseTAI;
			mt->_o.TAI = &dTAI;
			mt->_flags = reset;
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
		if (mt->_motionType != motionTypeDropObject) {
			mt->_motionType = motionTypeDropObject;
			mt->_o.directObject = &dObj;
			mt->_targetLoc = loc;
			mt->_flags = reset;
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
		if (mt->_motionType != motionTypeDropObjectOnObject) {
			mt->_motionType = motionTypeDropObjectOnObject;
			mt->_o.directObject = &dObj;
			mt->_o.indirectObject = &target;
			mt->_flags = reset;
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
		if (mt->_motionType != motionTypeDropObjectOnTAI) {
			mt->_motionType = motionTypeDropObjectOnTAI;
			mt->_o.directObject = &dObj;
			mt->_o.TAI = &target;
			mt->_targetLoc = loc;
			mt->_flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Determine if this MotionTask is a reflex ( motion over which an actor
//	has no control )

bool MotionTask::isReflex() {
	return      _motionType == motionTypeThrown
	            ||  _motionType == motionTypeFall
	            ||  _motionType == motionTypeLand
	            ||  _motionType == motionTypeAcceptHit
	            ||  _motionType == motionTypeFallDown
	            ||  _motionType == motionTypeDie;
}

//  Offensive combat actions

//-----------------------------------------------------------------------
//	Initiate a two-handed swing

void MotionTask::twoHandedSwing(Actor &a, GameObject &target) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->_motionType != motionTypeTwoHandedSwing) {
			mt->_motionType = motionTypeTwoHandedSwing;
			mt->_targetObj = &target;
			mt->_flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Initiate a one-handed swing

void MotionTask::oneHandedSwing(Actor &a, GameObject &target) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->_motionType != motionTypeOneHandedSwing) {
			mt->_motionType = motionTypeOneHandedSwing;
			mt->_targetObj = &target;
			mt->_flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Initiate a fire bow motion

void MotionTask::fireBow(Actor &a, GameObject &target) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->_motionType != motionTypeFireBow) {
			mt->_motionType = motionTypeFireBow;
			mt->_targetObj = &target;
			mt->_flags = reset;
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
		if (mt->_motionType != type) {
			mt->_motionType = type;
			mt->_spellObj = &spell;
			mt->_targetObj = &target;
			mt->_flags = reset;
			mt->_direction = (mt->_targetObj->getLocation() - a.getLocation()).quickDir();
			if (isPlayerActor(&a)) mt->_flags |= privledged;
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
		if (mt->_motionType != type) {
			mt->_motionType = type;
			mt->_spellObj = &spell;
			mt->_targetLoc = target; //target;
			mt->_flags = reset | LocTarg;
			mt->_direction = (target - a.getLocation()).quickDir();
			if (isPlayerActor(&a)) mt->_flags |= privledged;
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
		if (mt->_motionType != type) {
			Location loc;
			assert(target._data.itemType == activeTypeInstance);
			mt->_motionType = type;
			mt->_spellObj = &spell;
			mt->_targetTAG = &target;
			loc = Location(
			          target._data.instance.u << kTileUVShift,
			          target._data.instance.v << kTileUVShift,
			          target._data.instance.h,
			          a.world()->thisID());
			mt->_targetLoc = loc; //target;
			mt->_flags = reset | TAGTarg;
			mt->_direction = (loc - a.getLocation()).quickDir();
			if (isPlayerActor(&a)) mt->_flags |= privledged;
		}
	}
}

//-----------------------------------------------------------------------
//	Initiate a use wand motion

void MotionTask::useWand(Actor &a, GameObject &target) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->_motionType != motionTypeUseWand) {
			mt->_motionType = motionTypeUseWand;
			mt->_targetObj = &target;
			mt->_flags = reset;
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
		if (mt->_motionType != motionTypeTwoHandedParry) {
			mt->_motionType = motionTypeTwoHandedParry;
			mt->_d.attacker = &opponent;
			mt->_d.defensiveObj = &weapon;
		}
		mt->_flags = reset;
		mt->_d.defenseFlags = 0;
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
		if (mt->_motionType != motionTypeOneHandedParry) {
			mt->_motionType = motionTypeOneHandedParry;
			mt->_d.attacker = &opponent;
			mt->_d.defensiveObj = &weapon;
		}
		mt->_flags = reset;
		mt->_d.defenseFlags = 0;
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
		if (mt->_motionType != motionTypeShieldParry) {
			mt->_motionType = motionTypeShieldParry;
			mt->_d.attacker = &opponent;
			mt->_d.defensiveObj = &shield;
		}
		mt->_flags = reset;
		mt->_d.defenseFlags = 0;
	}
}

//-----------------------------------------------------------------------
//	Initiate a dodge

void MotionTask::dodge(Actor &a, Actor &opponent) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->_motionType != motionTypeDodge) {
			mt->_motionType = motionTypeDodge;
			mt->_d.attacker = &opponent;
		}
		mt->_flags = reset;
		mt->_d.defenseFlags = 0;
	}
}

//  Other combat actions

//-----------------------------------------------------------------------
//	Initiate an accept hit motion

void MotionTask::acceptHit(Actor &a, Actor &opponent) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->_motionType != motionTypeAcceptHit) {
			mt->_motionType = motionTypeAcceptHit;
			mt->_d.attacker = &opponent;
			mt->_flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Initiate a fall down motion

void MotionTask::fallDown(Actor &a, Actor &opponent) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->_motionType != motionTypeFallDown) {
			mt->_motionType = motionTypeFallDown;
			mt->_d.attacker = &opponent;
			mt->_flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Initiate a die motion

void MotionTask::die(Actor &a) {
	MotionTask      *mt;

	if ((mt = g_vm->_mTaskList->newTask(&a)) != nullptr) {
		if (mt->_motionType != motionTypeDie) {
			mt->_motionType = motionTypeDie;
			mt->_flags = reset;
		}
	}
}

//-----------------------------------------------------------------------
//	Determine if this MotionTask is a defensive motion

bool MotionTask::isDefense() {
	return      _motionType == motionTypeOneHandedParry
	            ||  _motionType == motionTypeTwoHandedParry
	            ||  _motionType == motionTypeShieldParry
	            ||  _motionType == motionTypeDodge;
}

//-----------------------------------------------------------------------
//	Determine if this MotionTask is an offensive motion

bool MotionTask::isAttack() {
	return      isMeleeAttack()
	            ||  _motionType == motionTypeFireBow
	            ||  _motionType == motionTypeCastSpell
	            ||  _motionType == motionTypeUseWand;
}

//-----------------------------------------------------------------------
//	Determine if this MotionTask is an offensive melee motion

bool MotionTask::isMeleeAttack() {
	return      _motionType == motionTypeOneHandedSwing
	            ||  _motionType == motionTypeTwoHandedSwing;
}

//-----------------------------------------------------------------------
//	Determine if this MotionTask is a walk motion

bool MotionTask::isWalk() {
	return _prevMotionType == motionTypeWalk;
}

//-----------------------------------------------------------------------
//	Return the wandering tether region

TileRegion MotionTask::getTether() {
	TileRegion  reg;

	if (_flags & tethered) {
		reg.min = TilePoint(_tetherMinU, _tetherMinV, 0);
		reg.max = TilePoint(_tetherMaxU, _tetherMaxV, 0);
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
	if (_prevMotionType == motionTypeWalk) {
		uint16      oldFlags = _flags;

		abortPathFind(this);

		_finalTarget = _immediateLocation = newPos;
		_pathCount = _pathIndex = 0;

		_flags = pathFind | reset;
		if (oldFlags & agitatable)
			_flags |= agitatable;

		//  Set run flag if requested
		if (run
		        //  Check if actor capable of running...
		        && ((Actor *)_object)->isActionAvailable(kActionRun))

			_flags |= requestRun;
		else
			_flags &= ~requestRun;

		RequestPath(this, getPathFindIQ(_object));
	}
}

//-----------------------------------------------------------------------
//	If the target has changed position since the walk/run started, then
//	call this function.

void MotionTask::changeDirectTarget(const TilePoint &newPos, bool run) {
	if (_prevMotionType == motionTypeWalk) {
		_prevMotionType = motionTypeWalk;

		_finalTarget = _immediateLocation = newPos;

		//  Reset motion task
		_flags |= reset;
		_flags &= ~pathFind;

		//  Set run flag if requested
		if (run
		        //  Check if actor capable of running...
		        && ((Actor *)_object)->isActionAvailable(kActionRun))

			_flags |= requestRun;
		else
			_flags &= ~requestRun;
	}
}

//  Cancel actor movement if walking...
void MotionTask::finishWalk() {
	//  If the actor is in a running state
	if (_motionType == motionTypeWalk) {
		remove();
		//  If there is currently a path finding request, abort it.
		/*      abortPathFind( this );

		            //  Simply set actor's target _data.location to "here".
		        _finalTarget = _immediateLocation = _object->getLocation();
		        _pathList[0] = _finalTarget;
		        flags = reset;
		        _pathCount = 0;
		        _pathIndex = 0;*/
	}
}

//  Cancel actor movement if talking...
void MotionTask::finishTalking() {
	if (_motionType == motionTypeTalk) {
		if (isActor(_object)) {
			Actor   *a = (Actor *)_object;
			if (a->_currentAnimation != kActionStand)
				a->setAction(kActionStand, 0);
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

	GameObject      *obj = _object;
	ProtoObj        *proto = obj->proto();

	if (isActor(obj)) {
		//  Before anything else make sure the actor is in an
		//  uninterruptable state.
		((Actor *)obj)->setInterruptablity(false);
	}


	//  Add the force of gravity to the acceleration.

	if (!(_flags & inWater)) {
		_velocity.z -= gravity;
	} else {
		_velocity.u = _velocity.v = 0;
		_velocity.z = -gravity;
	}
	location = obj->getLocation();

//	WriteStatusF( 6, "%d %d %d", _data.location.u, _data.location.v, _data.location.z );

	//  Because we live in a point-sampled universe, we need to make
	//  sure that objects which are moving extremely fast don't
	//  undersample the terrain. We do this by breaking the velocity
	//  vector into smaller vectors, and handling them individually.

	totalVelocity = _velocity;

	//  Make Up For Rounding Errors In ThrowTo

	if (_uFrac) {
		_uErrorTerm += ABS(_uFrac);

		if (_uErrorTerm >= _steps) {
			_uErrorTerm -= _steps;
			if (_uFrac > 0)
				totalVelocity.u++;
			else
				totalVelocity.u--;
		}
	}

	if (_vFrac) {
		_vErrorTerm += ABS(_vFrac);

		if (_vErrorTerm >= _steps) {
			_vErrorTerm -= _steps;
			if (_vFrac > 0)
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

	if (isActor(obj) && _velocity.magnitude() > 16) {
		Actor       *a = (Actor *)obj;

		if (a->isActionAvailable(kActionFreeFall))
			a->setAction(kActionFreeFall, 0);
	}

	for (int i = 0; i < vectorSteps; i++) {
		int16       stepsLeft = vectorSteps - i;
		GameObject  *collisionObject;

		//  REM: This would be better as a rounded division...

		//  Compute the small velocity vector for this increment,
		//  and then subtract it from the total _velocity.

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
			TilePoint       oldVelocity = _velocity;

			if (_motionType == motionTypeShot && collisionObject != nullptr) {
				//  If this motion is for a shot arrow and we did not
				//  collide with our target object just continue the
				//  motion as if there was no collision.
				if (collisionObject == _targetObj) {
					if (_object->strike(
					            _o.enactor->thisID(),
					            _targetObj->thisID())) {
						//  The arrow struck, so delete the arrow and
						//  end this motion
						remove();
						_object->deleteObject();
						return;
					} else {
						//  If the arrow failed to strike continue the
						//  arrows flight as if there was no collision.
						_targetObj = nullptr;
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
				_velocity = -_velocity / 2;
				totalVelocity = -totalVelocity / 2;
			} else {
				if (probe & (1 << 0)) {     // If struck wall in U direction
					_velocity.u = -_velocity.u / 2;
					totalVelocity.u = -totalVelocity.u / 2;
				} else {
					_velocity.u = (_velocity.u * 2) / 3;
					totalVelocity.u = (totalVelocity.u * 2) / 3;
				}

				if (probe & (1 << 1)) {     // If struck wall in V direction
					_velocity.v = -_velocity.v / 2;
					totalVelocity.v = -totalVelocity.v / 2;
				} else {
					_velocity.v = (_velocity.v * 2) / 3;
					totalVelocity.v = (totalVelocity.v * 2) / 3;
				}

				if (probe & (1 << 2)) {     // If struct wall in Z direction
					_velocity.z = -_velocity.z / 2;
					totalVelocity.z = -totalVelocity.z / 2;
				} else {
					_velocity.z = (_velocity.z * 2) / 3;
					totalVelocity.z = (totalVelocity.z * 2) / 3;
				}
			}
			_uFrac = _vFrac = 0;
			if (_motionType == motionTypeShot && obj->isMissile())
				obj->_data.missileFacing = missileDir(_velocity);

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
						_motionType =    velocityMagnitude <= 16
						                ?   motionTypeLand
						                :   motionTypeLandBadly;
						_flags |= reset;
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
			} else if (_velocity.u < 2 && _velocity.u > -2
			           &&  _velocity.v < 2 && _velocity.v > -2
			           &&  _velocity.z < 2 && _velocity.z > -2) {
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
//	if ( ( _flags & pathFind ) && _pathCount < 0 ) return false;

	//  If there are still waypoints in the path list, then
	//  retrieve the next waypoint.
	if ((_flags & (pathFind | wandering)) && _pathIndex < _pathCount) {
		TilePoint   wayPointVector(0, 0, 0);

		if (_pathIndex > 0)
			wayPointVector = _immediateLocation - _object->_data.location;

		if (wayPointVector.quickHDistance() == 0)
			//  Next vertex in path polyline
			_immediateLocation = _pathList[_pathIndex++];
		else
			return false;
	} else {
		if (_flags & wandering) {
			_immediateLocation = Nowhere;
			if (_pathFindTask == nullptr)
				RequestWanderPath(this, getPathFindIQ(_object));
		} else if (_flags & agitated) {
			_immediateLocation = Nowhere;
		} else {
			//  If we've gone off the end of the path list,
			//  and we're not at the target yet, request more waypoints then
			//  use dumb pathfinding until the pathfinder finishes it's task.

			if ((_finalTarget - _object->_data.location).quickHDistance() > 0
			        ||  ABS(_finalTarget.z - _object->_data.location.z) > kMaxStepHeight) {
				//  If no pathfind in progress
				if ((_flags & pathFind)
				        &&  !(_flags & finalPath)
				        &&  _pathFindTask == nullptr)
					RequestPath(this, getPathFindIQ(_object));

				//  Set the immediate target to the final target,
				_immediateLocation = _finalTarget;
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

	newPos      = _object->_data.location + (dirTable[dir] * speed) / 2;
	newPos.z    = _object->_data.location.z + stepUp;

	if (checkWalkable(_object, newPos)) return false;

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

	assert(isActor(_object));
	a = (Actor *)_object;
	aa = a->_appearance;

	if (a->isImmobile()) {
		remove(motionWalkBlocked);
		return;
	}

	//  Make sure that the actor is interruptable
	a->setInterruptablity(true);

	//  Set the speed of movement based on whether we are walking
	//  or running. Running only occurs after we have accelerated.
	if (_flags & requestRun
	        &&  _runCount == 0
	        &&  !(_flags & (inWater | onStairs))) {
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
		if (!(_flags & onStairs)) {
			if (!(_flags & inWater)) {
				speed = walkSpeed;
				speedScale = 2;
				walkType = walkNormal;
			} else {
				speed = slowWalkSpeed;
				speedScale = 1;
				walkType = walkSlow;

				//  reset run count if actor walking slowly
				_runCount = MAX<int16>(_runCount, 8);
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
			_runCount = MAX<int16>(_runCount, 8);
		}
	}

	if ((_flags & agitated)
	        &&  --actionCounter <= 0) {
		_flags &= ~agitated;
		_flags |= pathFind | reset;
	}

	for (;;) {
		//  The "reset" flag indicates that the final target has
		//  changed since the last time this routine was called.
		if (!(_flags & reset)) {
			//  Compute the vector and distance of the current
			//  position to the next "immediate" target.
			targetVector = immediateTarget - _object->_data.location;
			targetDist = targetVector.quickHDistance();

			//  If we're not already there, then proceed towards
			//  the target.
			if (targetDist > 0 || ABS(targetVector.z) > kMaxStepHeight)
				break;
		}

		if (nextWayPoint() == false) {
			//  If no waypoint could be found and this motion task has
			//  a path find request, then go into "wait" mode.
			if (_pathFindTask)
				moveTaskWaiting = true;
			else moveTaskDone = true;
			break;
		} else {
			_flags &= ~reset;
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

//		TPLine( a->getLocation(), _immediateLocation );
		curPt = a->getLocation();
		wayPt = immediateTarget;

		for (int i = _pathIndex - 1; i < _pathCount;) {
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
			wayPt = _pathList[++i];
		}

		ShowObjectSection(a);
	}
#endif

	moveBlocked = false;

	if (moveTaskDone || moveTaskWaiting) {
		movementDirection = a->_currentFacing;
	} else if (targetDist == 0 && ABS(targetVector.z) > kMaxStepHeight) {
		if (_pathFindTask) {
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
		newPos.z = _object->_data.location.z;

		//  Determine the direction the character must spin
		//  to be at the correct movement angle.
		directionAngle =
		    (((movementDirection - a->_currentFacing) + 4) & 7) - 4;

		//  Test terrain. Note that if the character is spinning more than 1
		//  octant this frame, then they cannot move so a terrain test is unneeded.
		if (directionAngle <= 1 && directionAngle >= -1) {
			//  Test the terrain to see if we can go there.
			if ((blockageType = checkWalkable(_object, newPos)) != false) {
				//  Try stepping up to a higher terrain too.
				newPos.z = _object->_data.location.z + kMaxStepHeight;
				if (checkWalkable(_object, newPos) != blockageNone) {
					//  If there is a path find task pending, put the walk action
					//  on hold until it finishes, else, abort the walk action.
					if (_pathFindTask)
						moveTaskWaiting = true;
					else {
						movementDirection = a->_currentFacing;
						moveBlocked = true;
					}
					/*                  if (!(_flags & pathFind) || nextWayPoint() == false)
					                    {
					                        moveBlocked = true;
					                        _flags |= blocked;
					                        newPos.z = _object->_data.location.z;

					                    }*/
				}
			}
		}
	} else {
		int16       height;
		bool        foundPath = false;

		movementDirection = targetVector.quickDir();

		//  Calculate new object position along direction vector.
		TilePoint   pos = _object->_data.location
		                  + targetVector * speed / targetDist;

#if DEBUG*0
		TPLine(_object->_data.location, pos);
#endif

		//  Check the terrain in various directions.
		//  Check in the forward direction first, at various heights

		for (height = 0; height <= kMaxStepHeight; height += kMaxSmoothStep) {
			//  This code has him move along the exact direction
			//  vector, even if it's not aligned with one of the
			//  cardinal directions.

			pos.z   = _object->_data.location.z + height;

			if (!checkWalkable(_object, pos)) {
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
			if (_pathFindTask)
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
		a->setAction(kActionStand, 0);
		if (_flags & agitatable) {
			if (freeFall(_object->_data.location, sti)) return;

			//  When he starts running again, then have him walk only.
			_runCount = MAX<int16>(_runCount, 8);

			//  We're blocked so we're going to wander in a random
			//  direction for a random duration
			_flags |= agitated | reset;

			_direction = g_vm->_rnd->getRandomNumber(7);
			actionCounter = 8 + g_vm->_rnd->getRandomNumber(7);

			//  Discard the path
			if (_flags & pathFind) {
				_flags &= ~finalPath;
				_pathIndex = _pathCount = 0;
			}
		} else
			remove(motionWalkBlocked);
	} else if (moveTaskWaiting
	           ||  movementDirection != a->_currentFacing) {
		//  When he starts running again, then have him walk only.
		_runCount = MAX<int16>(_runCount, 8);

		a->setAction(kActionStand, 0);
		freeFall(_object->_data.location, sti);
	} else {
		if (a == getCenterActor() && checkLadder(a, newPos)) return;

		int16               tHeight;

		_flags &= ~blocked;

		tHeight = tileSlopeHeight(newPos, _object, &sti);


		//  This is a kludge to keep the character from
		//  "jumping" as he climbs up a small step.

		if (tHeight >= _object->_data.location.z - kMaxSmoothStep
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
			        &&  a->isActionAvailable(kActionSpecial7)) {
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
					newAction = kActionSpecial7;
					_flags |= onStairs;
				} else if (a->_currentFacing == ((stairsDir - 4) & 0x7)) {
					//  walk down stairs
					newAction = kActionSpecial8;
					_flags |= onStairs;
				} else {
					_flags &= ~onStairs;
					if (walkType == walkStairs) walkType = walkNormal;
					newAction = (walkType == walkRun) ? kActionRun : kActionWalk;
				}
			} else {
				_flags &= ~onStairs;
				if (walkType == walkStairs) walkType = walkNormal;
				newAction = (walkType == walkRun) ? kActionRun : kActionWalk;
			}


			_object->move(newPos);

			//  Determine if the new action is running
			//  or walking.

			if (a->_currentAnimation == newAction) {
				//  If we are already doing that action, then
				//  just continue doing it.
				if (walkType != walkSlow)
					a->nextAnimationFrame();
				else {
					if (_flags & nextAnim)
						a->nextAnimationFrame();
					_flags ^= nextAnim;
				}
			} else if (a->_currentAnimation == kActionWalk
			           ||  a->_currentAnimation == kActionRun
			           ||  a->_currentAnimation == kActionSpecial7
			           ||  a->_currentAnimation == kActionSpecial8) {
				//  If we are running instead of walking or
				//  vice versa, then change to the new action
				//  but don't break stride
				a->setAction(newAction,
				             kAnimateRepeat | kAnimateNoRestart);

				if (walkType != walkSlow)
					a->nextAnimationFrame();
				else {
					if (_flags & nextAnim)
						a->nextAnimationFrame();
					_flags ^= nextAnim;
				}
			} else {
				// If we weren't walking or running before, then start
				// walking/running and reset the sequence.
				a->setAction(newAction, kAnimateRepeat);
				if (walkType == walkSlow) _flags |= nextAnim;
			}

			if (_runCount > 0) _runCount--;
			setObjectSurface(_object, sti);
		}
	}
}

//-----------------------------------------------------------------------
//	Climb up a ladder

void MotionTask::upLadderAction() {
	Actor               *a = (Actor *)_object;

	if (_flags & reset) {
		a->setAction(kActionClimbLadder, kAnimateRepeat);
		_flags &= ~reset;
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

		a->setAction(kActionStand, 0);

		remove();
	}
}

//-----------------------------------------------------------------------
//	Climb down a ladder

void MotionTask::downLadderAction() {
	Actor               *a = (Actor *)_object;

	if (_flags & reset) {
		a->setAction(kActionClimbLadder, kAnimateRepeat | kAnimateReverse);
		_flags &= ~reset;
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

		a->setAction(kActionStand, 0);

		remove();
	}
}

//  Go through the giving motions
void MotionTask::giveAction() {
	Actor       *a = (Actor *)_object;
	Direction   targetDir = (_targetObj->getLocation()
	                         -   a->getLocation()).quickDir();

	if (_flags & reset) {
		a->setAction(kActionGiveItem, 0);
		_flags &= ~reset;
	}

	if (a->_currentFacing != targetDir)
		a->turn(targetDir);
	else if (a->nextAnimationFrame())
		remove(motionCompleted);
}


//  Set up specified animation and run through the frames
void MotionTask::genericAnimationAction(uint8 actionType) {
	Actor *const   a = (Actor *)_object;

	if (_flags & reset) {
		a->setAction(actionType, 0);
		_flags &= ~reset;
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
	if (_flags & reset) {
		//  Let the game engine know about this aggressive act
		logAggressiveAct(_object->thisID(), _targetObj->thisID());

		//  Notify the target actor that he is being attacked
		if (isActor(_targetObj))
			((Actor *)_targetObj)->evaluateMeleeAttack((Actor *)_object);

		//  Create an animation type lookup table
		static const uint8  animationTypeArray[] = {
			kActionTwoHandSwingHigh,
			kActionTwoHandSwingLow,
			kActionTwoHandSwingLeftHigh,
			kActionTwoHandSwingLeftLow,
			kActionTwoHandSwingRightHigh,
			kActionTwoHandSwingRightLow,
		};

		Actor               *a = (Actor *)_object;
		uint8               actorAnimation;
		int16               actorMidAltitude,
		                    targetAltitude = _targetObj->getLocation().z;

		const CombatMotionSet   *availableSet;

		//  Calculate the altitude of the actor's mid section
		actorMidAltitude = a->getLocation().z + (a->proto()->height >> 1);

		if (targetAltitude > actorMidAltitude)
			//  The target is higher than the actor's midsection
			availableSet = &twoHandedHighSwingSet;
		else {
			uint8           targetHeight = _targetObj->proto()->height;

			if (targetAltitude + targetHeight < actorMidAltitude)
				//  The target is below the actor's midsection
				availableSet = &twoHandedLowSwingSet;
			else
				//  The target is nearly the same altitude as the actor
				availableSet = &twoHandedSwingSet;
		}

		//  Calculate the direction of the attack
		_direction = (_targetObj->getLocation() - a->getLocation()).quickDir();

		//  Randomly select a combat motion type from the available set
		_combatMotionType = availableSet->selectRandom();
		actorAnimation = animationTypeArray[_combatMotionType];

		if (a->_appearance != nullptr
		        &&  a->isActionAvailable(actorAnimation)) {
			//  Compute the number of frames in the animation before the
			//  actual strike
			actionCounter = a->animationFrames(actorAnimation, _direction) - 2;

			a->setAction(actorAnimation, 0);

			//  Set this flag to indicate that the animation is actually
			//  being played
			_flags |= nextAnim;
		} else {
			actionCounter = 2;

			//  Clear this flag to indicate that the animation is not
			//  being played
			_flags &= ~nextAnim;
		}

		a->setActionPoints(
		    computeTurnFrames(a->_currentFacing, _direction) + 10);

		_flags &= ~reset;
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
	if (_flags & reset) {
		//  Let the game engine know about this aggressive act
		logAggressiveAct(_object->thisID(), _targetObj->thisID());

		//  Notify the target actor that he is being attacked
		if (isActor(_targetObj))
			((Actor *)_targetObj)->evaluateMeleeAttack((Actor *)_object);

		//  Create an animation type lookup table
		static const uint8  animationTypeArray[] = {
			kActionSwingHigh,
			kActionSwingLow,
		};

		Actor *const       a = (Actor *)_object;
		uint8               actorAnimation;
		int16               actorMidAltitude,
		                    targetAltitude = _targetObj->getLocation().z;

		const CombatMotionSet   *availableSet;

		//  Calculate the altitude of the actor's mid section
		actorMidAltitude = a->getLocation().z + (a->proto()->height >> 1);

		if (targetAltitude > actorMidAltitude)
			//  The target is higher than the actor's midsection
			availableSet = &oneHandedHighSwingSet;
		else {
			uint8           targetHeight = _targetObj->proto()->height;

			if (targetAltitude + targetHeight < actorMidAltitude)
				//  The target is below the actor's midsection
				availableSet = &oneHandedLowSwingSet;
			else
				//  The target is nearly the same altitude as the actor
				availableSet = &oneHandedSwingSet;
		}

		//  Calculate the direction of the attack
		_direction = (_targetObj->getLocation() - a->getLocation()).quickDir();

		//  Randomly select a combat motion type from the available set
		_combatMotionType = availableSet->selectRandom();

		/*      if ( _combatMotionType == oneHandedThrust )
		        {
		            //  Initialize the thrust motion
		        }
		        else*/
		{
			actorAnimation = animationTypeArray[_combatMotionType];
			if (a->_appearance != nullptr
			        &&  a->isActionAvailable(actorAnimation)) {
				//  Compute the number of frames in the animation before the
				//  actual strike
				actionCounter = a->animationFrames(actorAnimation, _direction) - 2;

				a->setAction(actorAnimation, 0);

				//  Set this flag to indicate that the animation is actually
				//  being played
				_flags |= nextAnim;
			} else {
				actionCounter = 1;

				//  Clear this flag to indicate that the animation is not
				//  being played
				_flags &= ~nextAnim;
			}

		}

		a->setActionPoints(actionCounter * 2);

		a->setActionPoints(
		    computeTurnFrames(a->_currentFacing, _direction) + 10);

		_flags &= ~reset;
	} else
		//  Call the generic offensive melee function
		offensiveMeleeAction();
}

//-----------------------------------------------------------------------
//	Compute the number of frames before the actual strike in an
//	offensive melee motion

uint16 MotionTask::framesUntilStrike() {
	//  If the melee action has not been initialized, return a safe value
	if (_flags & reset) return maxuint16;

	uint16          turnFrames;

	turnFrames = (_direction - ((Actor *)_object)->_currentFacing) & 0x7;
	if (turnFrames > 4) turnFrames = 8 - turnFrames;

	return turnFrames + actionCounter;
}

//-----------------------------------------------------------------------
//	Returns a pointer to the blocking object if it applicable to
//	this motion task

GameObject *MotionTask::blockingObject(Actor *thisAttacker) {
	return      isDefense()
	            && (_d.defenseFlags & blocking)
	            &&  thisAttacker == _d.attacker
	            ?   _d.defensiveObj
	            :   nullptr;
}

//-----------------------------------------------------------------------
//  Handle bow firing motions

void MotionTask::fireBowAction() {
	Actor       *a = (Actor *)_object;

	assert(a->_leftHandObject != Nothing);

	//  Initialize the bow firing motion
	if (_flags & reset) {
		//  Let the game engine know about this aggressive act
		logAggressiveAct(_object->thisID(), _targetObj->thisID());

		//  Compute the direction to the target
		_direction = (_targetObj->getLocation() - a->getLocation()).quickDir();

		if (a->_appearance != nullptr
		        &&  a->isActionAvailable(kActionFireBow)) {
			//  Calculate the number of frames in the animation before the
			//  projectile is actually fired
			actionCounter = a->animationFrames(kActionFireBow, _direction) - 1;
			a->setAction(kActionFireBow, 0);

			//  Set this flag to indicate that the animation is actually
			//  being played
			_flags |= nextAnim;
		} else {
			actionCounter = 1;

			//  Clear this flag to indicate that the animation is not
			//  being played
			_flags &= ~nextAnim;
		}

		a->setActionPoints(
		    computeTurnFrames(a->_currentFacing, _direction) + 10);

		if (a->_currentFacing != _direction)
			a->turn(_direction);

		_flags &= ~reset;
	} else if (a->_currentFacing != _direction)
		a->turn(_direction);
	else {
		//  If the actors appearance becomes NULL, make sure this action
		//  no longer depends upon the animation
		if ((_flags & nextAnim) && a->_appearance == nullptr)
			_flags &= ~nextAnim;

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
						shootObject(*proj, *a, *_targetObj, 16);
					}
				}
			}
		}

		if (_flags & nextAnim) {
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
	Actor       *a = (Actor *)_object;

	//  Turn until facing the target
	if (a->_currentFacing != _direction)
		a->turn(_direction);
	else {
		if (_flags & reset) {
			if (a->_appearance != nullptr
			        &&  a->isActionAvailable(kActionCastSpell)) {
				//  Calculate the number of frames in the animation before the
				//  spell is case
				actionCounter = a->animationFrames(kActionCastSpell, _direction) - 1;
				a->setAction(kActionCastSpell, 0);

				//  Set this flag to indicate that the animation is actually
				//  being played
				_flags |= nextAnim;
			} else {
				actionCounter = 3;

				//  Clear this flag to indicate that the animation is not
				//  being played
				_flags &= ~nextAnim;
			}

			_flags &= ~reset;
		}

		//  If the actors appearance becomes NULL, make sure this action
		//  no longer depends upon the animation
		if ((_flags & nextAnim) && a->_appearance == nullptr)
			_flags &= ~nextAnim;

		if (actionCounter == 0) {
			if (_spellObj) {
				if (_flags & TAGTarg) {
					assert(_targetTAG->_data.itemType == activeTypeInstance);
					_spellObj->implementAction(_spellObj->getSpellID(), a->thisID(), _targetTAG);
				} else if (_flags & LocTarg) {
					_spellObj->implementAction(_spellObj->getSpellID(), a->thisID(), _targetLoc);
				} else if (_targetObj) {
					_spellObj->implementAction(_spellObj->getSpellID(), a->thisID(), _targetObj->thisID());
				}
			}
		}

		if (_flags & nextAnim) {
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
	if (_flags & reset) {
		//  Let the game engine know about this aggressive act
		logAggressiveAct(_object->thisID(), _targetObj->thisID());

		Actor       *a = (Actor *)_object;

		_direction = (_targetObj->getLocation() - a->getLocation()).quickDir();

		if (a->_appearance != nullptr
		        &&  a->isActionAvailable(kActionUseWand)) {
			actionCounter = a->animationFrames(kActionUseWand, _direction) - 1;
			a->setAction(kActionUseWand, 0);

			//  Set this flag to indicate that the animation is actually
			//  being played
			_flags |= nextAnim;
		} else {
			actionCounter = 3;

			//  Clear this flag to indicate that the animation is not
			//  being played
			_flags &= ~nextAnim;
		}

		a->setActionPoints(
		    computeTurnFrames(a->_currentFacing, _direction) + 10);

		_flags &= ~reset;
	}
	useMagicWeaponAction();
}

//  Defensive combat actions
//-----------------------------------------------------------------------
//	Handle two handed parrying motions

void MotionTask::twoHandedParryAction() {
	if (_flags & reset) {
		Actor       *a = (Actor *)_object;
		int16       animationFrames;

		_direction = (_d.attacker->getLocation() - a->getLocation()).quickDir();

		if (a->_appearance != nullptr
		        &&  a->isActionAvailable(kActionTwoHandParry)) {
			a->setAction(kActionTwoHandParry, 0);
			animationFrames = a->animationFrames(kActionTwoHandParry, _direction);

			//  Set this flag to indicate that the animation is actually
			//  being played
			_flags |= nextAnim;
		} else {
			animationFrames = 2;

			//  Clear this flag to indicate that the animation is not
			//  being played
			_flags &= ~nextAnim;
		}

		a->setActionPoints(
		    computeTurnFrames(a->_currentFacing, _direction)
		    +   animationFrames + 1);

		_flags &= ~reset;
	}
	defensiveMeleeAction();
}

//-----------------------------------------------------------------------
//	Handle one handed parrying motions

void MotionTask::oneHandedParryAction() {
	if (_flags & reset) {
		Actor       *a = (Actor *)_object;
		int16       animationFrames;

		_direction = (_d.attacker->getLocation() - a->getLocation()).quickDir();

		_combatMotionType = oneHandedParryHigh;
		if (a->_appearance != nullptr
		        &&  a->isActionAvailable(kActionParryHigh)) {
			a->setAction(kActionParryHigh, 0);
			animationFrames = a->animationFrames(kActionParryHigh, _direction);

			//  Set this flag to indicate that the animation is actually
			//  being played
			_flags |= nextAnim;
		} else {
			animationFrames = 2;

			//  Clear this flag to indicate that the animation is not
			//  being played
			_flags &= ~nextAnim;
		}

		a->setActionPoints(
		    computeTurnFrames(a->_currentFacing, _direction)
		    +   animationFrames + 1);

		_flags &= ~reset;
	}
	defensiveMeleeAction();
}

//-----------------------------------------------------------------------
//	Handle shield parrying motions

void MotionTask::shieldParryAction() {
	if (_flags & reset) {
		Actor       *a = (Actor *)_object;
		int16       animationFrames;

		_direction = (_d.attacker->getLocation() - a->getLocation()).quickDir();

		if (a->_appearance != nullptr
		        &&  a->isActionAvailable(kActionShieldParry)) {
			a->setAction(kActionShieldParry, 0);
			animationFrames = a->animationFrames(kActionShieldParry, _direction);

			//  Set this flag to indicate that the animation is actually
			//  being played
			_flags |= nextAnim;
		} else {
			animationFrames = 1;

			//  Clear this flag to indicate that the animation is not
			//  being played
			_flags &= ~nextAnim;
		}

		a->setActionPoints(
		    computeTurnFrames(a->_currentFacing, _direction)
		    +   animationFrames + 1);

		_flags &= ~reset;
	}
	defensiveMeleeAction();
}

//-----------------------------------------------------------------------
//	Handle dodging motions

void MotionTask::dodgeAction() {
	Actor           *a = (Actor *)_object;
	MotionTask      *attackerMotion = _d.attacker->_moveTask;

	if (_flags & reset) {
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
			        &&  a->isActionAvailable(kActionJumpUp, a->_currentFacing)) {
				a->setAction(kActionJumpUp, 0);
				animationFrames = a->animationFrames(kActionJumpUp, a->_currentFacing);

				//  Set this flag to indicate that the animation is actually
				//  being played
				_flags |= nextAnim;
			} else {
				animationFrames = 3;

				//  Clear this flag to indicate that the animation is not
				//  being played
				_flags &= ~nextAnim;
			}

			actionCounter = animationFrames - 1;
			a->setActionPoints(animationFrames + 1);

			_flags &= ~reset;
		}
	} else {
		//  If the actors appearance becomes NULL, make sure this action
		//  no longer depends upon the animation
		if ((_flags & nextAnim) && a->_appearance == nullptr)
			_flags &= ~nextAnim;

		if (_flags & nextAnim) {
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
	Actor           *a = (Actor *)_object;

	if (_flags & reset) {
		TilePoint           newLoc = a->getLocation();
		StandingTileInfo    sti;
		int16               animationFrames;

		a->_currentFacing =
		    (_d.attacker->getWorldLocation() - a->getLocation()).quickDir();

		if (a->_appearance != nullptr
		        &&  a->isActionAvailable(kActionHit, a->_currentFacing)) {
			a->setAction(kActionHit, 0);
			animationFrames = a->animationFrames(kActionHit, a->_currentFacing);

			//  Set this flag to indicate that the animation is actually
			//  being played
			_flags |= nextAnim;
		} else {
			animationFrames = 1;

			//  Clear this flag to indicate that the animation is not
			//  being played
			_flags &= ~nextAnim;
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

		_flags &= ~reset;
	} else {
		//  If the actors appearance becomes NULL, make sure this action
		//  no longer depends upon the animation
		if ((_flags & nextAnim) && a->_appearance == nullptr)
			_flags &= ~nextAnim;

		if (_flags & nextAnim) {
			if (a->nextAnimationFrame()) remove();
		} else
			remove();
	}
}

//-----------------------------------------------------------------------
//	Handle fall down motions

void MotionTask::fallDownAction() {
	Actor           *a = (Actor *)_object;

	if (_flags & reset) {
		TilePoint           newLoc = a->getLocation();
		StandingTileInfo    sti;
		int16               animationFrames;

		a->_currentFacing =
		    (_d.attacker->getWorldLocation() - a->getLocation()).quickDir();

		if (a->_appearance != nullptr
		        &&  a->isActionAvailable(kActionKnockedDown, a->_currentFacing)) {
			a->setAction(kActionKnockedDown, 0);
			animationFrames =   a->animationFrames(
			                        kActionKnockedDown,
			                        a->_currentFacing);

			//  Set this flag to indicate that the animation is actually
			//  being played
			_flags |= nextAnim;
		} else {
			animationFrames = 6;

			//  Clear this flag to indicate that the animation is not
			//  being played
			_flags &= ~nextAnim;
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

		_flags &= ~reset;
	} else {
		//  If the actors appearance becomes NULL, make sure this action
		//  no longer depends upon the animation
		if ((_flags & nextAnim) && a->_appearance == nullptr)
			_flags &= ~nextAnim;

		if (_flags & nextAnim) {
			if (a->nextAnimationFrame()) remove();
		} else
			remove();
	}
}

//-----------------------------------------------------------------------
//  Generic offensive melee code.  Called by twoHandedSwingAction()
//	and oneHandedSwingAction()

void MotionTask::offensiveMeleeAction() {
	Actor       *a = (Actor *)_object;

	//  Turn until facing the target
	if (a->_currentFacing != _direction)
		a->turn(_direction);
	else {
		//  If the actors appearance becomes NULL, make sure this action
		//  no longer depends upon the animation
		if ((_flags & nextAnim) && a->_appearance == nullptr)
			_flags &= ~nextAnim;

		//  If the action counter has reached zero, use the weapon on
		//  the target
		if (actionCounter == 0) {
			GameObject  *weapon;

			weapon = a->offensiveObject();
			if (weapon) weapon->strike(a->thisID(), _targetObj->thisID());
		}

		if (_flags & nextAnim) {
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
	Actor       *a = (Actor *)_object;

	//  Turn until facing the target
	if (a->_currentFacing != _direction)
		a->turn(_direction);
	else {
		//  If the actors appearance becomes NULL, make sure this action
		//  no longer depends upon the animation
		if ((_flags & nextAnim) && a->_appearance == nullptr)
			_flags &= ~nextAnim;

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
				    _targetObj->thisID());
			}
		}

		if (_flags & nextAnim) {
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
	Actor           *a = (Actor *)_object;
	MotionTask      *attackerMotion = _d.attacker->_moveTask;

	//  Determine if the blocking action has been initiated
	if (!(_d.defenseFlags & blocking)) {
		//  If the attacker is not attacking, we're done
		if (attackerMotion == nullptr
		        ||  !attackerMotion->isMeleeAttack()) {
			a->setInterruptablity(true);
			remove();
			return;
		}

		//  turn towards attacker
		if (a->_currentFacing != _direction)
			a->turn(_direction);

		//  If the strike is about to land start the blocking motion
		if (attackerMotion->framesUntilStrike() <= 1)
			_d.defenseFlags |= blocking;
	} else {
		//  If the actors appearance becomes NULL, make sure this action
		//  no longer depends upon the animation
		if ((_flags & nextAnim) && a->_appearance == nullptr)
			_flags &= ~nextAnim;

		//  Run through the animation frames
		if (!(_flags & nextAnim) || a->nextAnimationFrame()) {
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
		GameObject  *obj = mt->_object;
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
			mt->_flags |= inWater;
		else
			mt->_flags &= ~inWater;

		switch (mt->_motionType) {
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

			if (mt->_flags & reset) {
				a->setAction(kActionStand, 0);
				a->_cycleCount = g_vm->_rnd->getRandomNumber(3);
				mt->_flags &= ~(reset | nextAnim);
			}
			if (a->_cycleCount == 0) {
				a->setAction(kActionTalk, 0);
				mt->_flags |= nextAnim;
				a->_cycleCount = -1;
			} else if (mt->_flags & nextAnim) {
				if (a->nextAnimationFrame()) {
					a->setAction(kActionStand, 0);
					a->_cycleCount = g_vm->_rnd->getRandomNumber(3);
					mt->_flags &= ~nextAnim;
				}
			} else
				a->_cycleCount--;
			break;

		case motionTypeLand:
		case motionTypeLandBadly:

			if (mt->_flags & reset) {
				int16   newAction = mt->_motionType == motionTypeLand
				                    ?   kActionJumpUp
				                    :   kActionFallBadly;

				if (!a->isActionAvailable(newAction)) {
					if (mt->_prevMotionType == motionTypeWalk) {
						mt->_motionType = mt->_prevMotionType;
						if (mt->_flags & pathFind) {
							mt->changeTarget(
							    mt->_finalTarget,
							    (mt->_flags & requestRun) != 0);
						} else {
							mt->changeDirectTarget(
							    mt->_finalTarget,
							    (mt->_flags & requestRun) != 0);
						}
						g_vm->_mTaskList->_nextMT = it;
					}
				} else {
					a->setAction(newAction, 0);
					a->setInterruptablity(false);
					mt->_flags &= ~reset;
				}
			} else if (a->nextAnimationFrame() || (mt->_flags & inWater)) {
				if (mt->_prevMotionType == motionTypeWalk) {
					mt->_motionType = mt->_prevMotionType;
					if (mt->_flags & pathFind) {
						mt->changeTarget(
						    mt->_finalTarget,
						    (mt->_flags & requestRun) != 0);
					} else {
						mt->changeDirectTarget(
						    mt->_finalTarget,
						    (mt->_flags & requestRun) != 0);
					}
					g_vm->_mTaskList->_nextMT = it;
				} else if (mt->freeFall(obj->_data.location, sti) == false)
					moveTaskDone = true;
			} else {
				//  If actor was running, go through an abreviated
				//  landing sequence by aborting the landing animation
				//  after the first frame.
				if (mt->_prevMotionType == motionTypeWalk
				        &&  mt->_flags & requestRun
				        &&  mt->_runCount == 0
				        &&  !(mt->_flags & inWater)) {
					mt->_motionType = mt->_prevMotionType;
					if (mt->_flags & pathFind) {
						mt->changeTarget(
						    mt->_finalTarget,
						    (mt->_flags & requestRun) != 0);
					} else {
						mt->changeDirectTarget(
						    mt->_finalTarget,
						    (mt->_flags & requestRun) != 0);
					}
					g_vm->_mTaskList->_nextMT = it;
				}
			}
			break;

		case motionTypeJump:

			if (mt->_flags & reset) {
				a->setAction(kActionJumpUp, 0);
				a->setInterruptablity(false);
				mt->_flags &= ~reset;
			} else if (a->nextAnimationFrame()) {
				mt->_motionType = motionTypeThrown;
				a->setAction(kActionFreeFall, 0);
			}
			break;

		case motionTypeTurn:

			mt->turnAction();
			break;

		case motionTypeGive:

			mt->giveAction();
			break;

		case motionTypeRise:

			if (a->_data.location.z < mt->_immediateLocation.z) {
				a->_data.location.z++;
				if (mt->_flags & nextAnim)
					a->nextAnimationFrame();
				mt->_flags ^= nextAnim;
			} else {
				targetVector = mt->_finalTarget - obj->_data.location;
				targetDist = targetVector.quickHDistance();

				if (targetDist > kTileUVSize) {
					mt->_motionType = mt->_prevMotionType;
					mt->_flags |= reset;
					g_vm->_mTaskList->_nextMT = it;
				} else
					moveTaskDone = true;
			}
			break;

		case motionTypeWait:

			if (mt->_flags & reset) {
				mt->actionCounter = 5;
				mt->_flags &= ~reset;
			} else if (--mt->actionCounter == 0)
				moveTaskDone = true;
			break;

		case motionTypeUseObject:

			//  This will be uninterrutable for 2 frames
			a->setActionPoints(2);
			mt->_o.directObject->use(a->thisID());
			//g_vm->_mTaskList->_nextMT=mt;
			moveTaskDone = true;
			break;

		case motionTypeUseObjectOnObject:

			if (isWorld(mt->_o.indirectObject->IDParent())) {
				if (
				    1
#ifdef THIS_SHOULD_BE_IN_TILEMODE
				    a->inUseRange(
				        mt->_o.indirectObject->getLocation(),
				        mt->_o.directObject)
#endif
				) {
					mt->_direction = (mt->_o.indirectObject->getLocation()
					                 -   a->getLocation()).quickDir();
					if (a->_currentFacing != mt->_direction)
						a->turn(mt->_direction);
					else {
						//  The actor will now be uniterruptable
						a->setActionPoints(2);
						mt->_o.directObject->useOn(
						    a->thisID(),
						    mt->_o.indirectObject->thisID());
						if (mt->_motionType == motionTypeUseObjectOnObject)
							moveTaskDone = true;
						else
							g_vm->_mTaskList->_nextMT = it;
					}
				}
			} else {
				//  The actor will now be uniterruptable
				a->setActionPoints(2);
				mt->_o.directObject->useOn(
				    a->thisID(),
				    mt->_o.indirectObject->thisID());
				if (mt->_motionType == motionTypeUseObjectOnObject)
					moveTaskDone = true;
				else
					g_vm->_mTaskList->_nextMT = it;
			}

			break;

		case motionTypeUseObjectOnTAI:

			if (mt->_flags & reset) {
				TilePoint       actorLoc = a->getLocation(),
				                TAILoc;
				TileRegion      TAIReg;
				ActiveItem      *TAG = mt->_o.TAI->getGroup();

				//  Compute in points the region of the TAI
				TAIReg.min.u = mt->_o.TAI->_data.instance.u << kTileUVShift;
				TAIReg.min.v = mt->_o.TAI->_data.instance.v << kTileUVShift;
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
				mt->_direction = (TAILoc - actorLoc).quickDir();
				mt->_flags &= ~reset;
			}

			if (a->_currentFacing != mt->_direction)
				a->turn(mt->_direction);
			else {
				//  The actor will now be uniterruptable
				a->setActionPoints(2);
				mt->_o.directObject->useOn(a->thisID(), mt->_o.TAI);
				if (mt->_motionType == motionTypeUseObjectOnTAI)
					moveTaskDone = true;
				else
					g_vm->_mTaskList->_nextMT = it;
			}
			break;

		case motionTypeUseObjectOnLocation:

			if (mt->_flags & reset) {
				mt->_direction = (mt->_targetLoc - a->getLocation()).quickDir();
				mt->_flags &= ~reset;
			}

			if (a->_currentFacing != mt->_direction)
				a->turn(mt->_direction);
			else {
				//  The actor will now be uniterruptable
				a->setActionPoints(2);
				mt->_o.directObject->useOn(a->thisID(), mt->_targetLoc);
				if (mt->_motionType == motionTypeUseObjectOnLocation)
					moveTaskDone = true;
				else
					g_vm->_mTaskList->_nextMT = it;
			}
			break;

		case motionTypeUseTAI:

			if (mt->_flags & reset) {
				TilePoint       actorLoc = a->getLocation(),
				                TAILoc;
				TileRegion      TAIReg;
				ActiveItem      *TAG = mt->_o.TAI->getGroup();

				//  Compute in points the region of the TAI
				TAIReg.min.u = mt->_o.TAI->_data.instance.u << kTileUVShift;
				TAIReg.min.v = mt->_o.TAI->_data.instance.v << kTileUVShift;
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
				mt->_direction = (TAILoc - actorLoc).quickDir();
				mt->_flags &= ~reset;
			}

			if (a->_currentFacing != mt->_direction)
				a->turn(mt->_direction);
			else {
				//  The actor will now be uniterruptable
				a->setActionPoints(2);
				mt->_o.TAI->use(a->thisID());
				moveTaskDone = true;
			}
			break;

		case motionTypeDropObject:

			if (isWorld(mt->_targetLoc._context)) {
				if (mt->_flags & reset) {
					mt->_direction = (mt->_targetLoc - a->getLocation()).quickDir();
					mt->_flags &= ~reset;
				}

				if (a->_currentFacing != mt->_direction)
					a->turn(mt->_direction);
				else {
					//  The actor will now be uniterruptable
					a->setActionPoints(2);
					mt->_o.directObject->drop(a->thisID(),
					                       mt->_targetLoc,
					                       mt->moveCount);
					if (mt->_motionType == motionTypeDropObject)
						moveTaskDone = true;
					else
						g_vm->_mTaskList->_nextMT = it;
				}
			} else {
				//  The actor will now be uniterruptable
				a->setActionPoints(2);
				mt->_o.directObject->drop(a->thisID(),
				                       mt->_targetLoc,
				                       mt->moveCount);
				if (mt->_motionType == motionTypeDropObject)
					moveTaskDone = true;
				else
					g_vm->_mTaskList->_nextMT = it;
			}

			CMassWeightIndicator::_bRedraw = true;   // tell the mass/weight indicators to refresh

			break;

		case motionTypeDropObjectOnObject:

			if (isWorld(mt->_o.indirectObject->IDParent())) {
				mt->_direction = (mt->_o.indirectObject->getLocation()
				                 -   a->getLocation()).quickDir();
				if (a->_currentFacing != mt->_direction)
					a->turn(mt->_direction);
				else {
					//  The actor will now be uniterruptable
					a->setActionPoints(2);
					mt->_o.directObject->dropOn(
					    a->thisID(),
					    mt->_o.indirectObject->thisID(),
					    mt->moveCount);
					if (mt->_motionType == motionTypeDropObjectOnObject)
						moveTaskDone = true;
					else
						g_vm->_mTaskList->_nextMT = it;
				}
			} else {
				//  The actor will now be uniterruptable
				a->setActionPoints(2);
				mt->_o.directObject->dropOn(
				    a->thisID(),
				    mt->_o.indirectObject->thisID(),
				    mt->moveCount);
				if (mt->_motionType == motionTypeDropObjectOnObject)
					moveTaskDone = true;
				else
					g_vm->_mTaskList->_nextMT = it;
			}

			CMassWeightIndicator::_bRedraw = true;   // tell the mass/weight indicators to refresh

			break;

		case motionTypeDropObjectOnTAI:

			if (mt->_flags & reset) {
				mt->_direction = (mt->_targetLoc - a->getLocation()).quickDir();
				mt->_flags &= ~reset;
			}

			if (a->_currentFacing != mt->_direction)
				a->turn(mt->_direction);
			else {
				//  The actor will now be uniterruptable
				a->setActionPoints(2);
				mt->_o.directObject->dropOn(
				    a->thisID(),
				    mt->_o.TAI,
				    mt->_targetLoc);
				if (mt->_motionType == motionTypeDropObjectOnTAI)
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
			if (mt->_flags & reset) {
				if (a->isActionAvailable(kActionDie)) {
					a->setAction(kActionDie, 0);
					a->setInterruptablity(false);
					mt->_flags &= ~reset;
				} else {
					moveTaskDone = true;
					a->setInterruptablity(true);
					if (!a->hasEffect(kActorDisappearOnDeath)) {
						a->setAction(kActionDead, 0);
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
				if (!a->hasEffect(kActorDisappearOnDeath)) {
					a->setAction(kActionDead, 0);
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

	tHeight = tileSlopeHeight(newPos, _object, &sti);

	if (_object->_data.objectFlags & kObjectFloating) return false;

	_velocity.u = (newPos.u - _object->_data.location.u) * 2 / 3;
	_velocity.v = (newPos.v - _object->_data.location.v) * 2 / 3;
	_velocity.z = (newPos.z - _object->_data.location.z) * 2 / 3;
//	_velocity.z = 0;

	//  If terrain is HIGHER (or even sligtly lower) than we are
	//  currently at, then try climbing it.

	if (tHeight >= newPos.z - gravity * 4) {
supported:
		if (_motionType != motionTypeWalk
		        ||  tHeight <= newPos.z
		        ||  !(_flags & inWater)) {
			if (tHeight > newPos.z + kMaxStepHeight) {
				unstickObject(_object);
				tHeight = tileSlopeHeight(newPos, _object, &sti);
			}
			newPos.z = tHeight;
//			setObjectSurface( _object, sti );
			return false;
		} else {
			_motionType = motionTypeRise;
			_immediateLocation.z = tHeight;
			_object->move(newPos);
			return true;
		}

	}

	//  Otherwise, begin a fall sequence...
	tPos = newPos;

	//  Attempt to solve cases where he gets stuck in falling,
	//  by checking the contact of what he's about to fall on.
	if (tPos.z > tHeight) tPos.z--;
	//  See if we fell on something.
	if (checkContact(_object, tPos) == blockageNone) {
falling:
		if (_motionType != motionTypeWalk
				||  newPos.z > gravity * 4
				||  tHeight >= 0) {
			_motionType = motionTypeThrown;

//			newPos = tPos;
			_object->move(tPos);
			return true;
		} else {
			newPos = tPos;
			return false;
		}
	}

	//  If we fall on something, reduce velocity due to impact.
	//  Try a couple of probes to see if we can fall in
	//  other directions.
	objCrossSection = _object->proto()->crossSection;

	tPos.u += objCrossSection;
	if (!checkBlocked(_object, tPos)
			&&  !checkContact(_object, tPos))
		goto falling;

	tPos.u -= objCrossSection * 2;
	if (!checkBlocked(_object, tPos)
			&&  !checkContact(_object, tPos))
		goto falling;

	tPos.u += objCrossSection;
	tPos.v += objCrossSection;
	if (!checkBlocked(_object, tPos)
			&&  !checkContact(_object, tPos))
		goto falling;

	tPos.v -= objCrossSection * 2;
	if (!checkBlocked(_object, tPos)
			&&  !checkContact(_object, tPos))
		goto falling;

	//  There is no support for the object and there is no place to fall
	//  so cheat and pretend this whole mess never happened.
	tPos = newPos;

	tPos.u += objCrossSection;
	tHeight = tileSlopeHeight(tPos, _object, &sti);
	if (tHeight <= tPos.z + kMaxStepHeight
			&&  tHeight >= tPos.z - gravity * 4) {
		newPos = tPos;
		goto supported;
	}

	tPos.u -= objCrossSection * 2;
	tHeight = tileSlopeHeight(tPos, _object, &sti);
	if (tHeight <= tPos.z + kMaxStepHeight
			&&  tHeight >= tPos.z - gravity * 4) {
		newPos = tPos;
		goto supported;
	}

	tPos.u += objCrossSection;
	tPos.v += objCrossSection;
	tHeight = tileSlopeHeight(tPos, _object, &sti);
	if (tHeight <= tPos.z + kMaxStepHeight
			&&  tHeight >= tPos.z - gravity * 4) {
		newPos = tPos;
		goto supported;
	}

	tPos.v -= objCrossSection * 2;
	tHeight = tileSlopeHeight(tPos, _object, &sti);
	if (tHeight <= tPos.z + kMaxStepHeight
			&&  tHeight >= tPos.z - gravity * 4) {
		newPos = tPos;
		goto supported;
	}

	//  If we STILL cannot find support for the object, change its
	//  position and try again.  This should be very rare.
	newPos.z--;
	_object->move(newPos);
	unstickObject(_object);
	newPos = _object->getLocation();
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
