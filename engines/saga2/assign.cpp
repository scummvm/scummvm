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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

/* ===================================================================== *
   Includes
 * ===================================================================== */

#include "saga2/std.h"
#include "saga2/assign.h"
#include "saga2/patrol.h"

namespace Saga2 {

/* ===================================================================== *
   ActorAssignment member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- reconstruct from archive buffer

ActorAssignment::ActorAssignment(void **buf) {
	uint16  *a = (uint16 *)*buf;

	startFrame = *a++;
	endFrame = *a++;

	*buf = a;
}

//----------------------------------------------------------------------
//	ActorAssignment destructor

ActorAssignment::~ActorAssignment(void) {
	Actor *a = getActor();

	//  Determine if the actor has a task initiated by this assignment
	if (a->currentGoal == actorGoalFollowAssignment
	        &&  a->curTask != NULL) {
		//  If so, abort it
		a->curTask->abort();
		delete a->curTask;
		a->curTask = NULL;
	}
}

//----------------------------------------------------------------------
//	Return the number of bytes need to archive the data in this
//	assignment

inline int32 ActorAssignment::archiveSize(void) const {
	return sizeof(startFrame) + sizeof(endFrame);
}

//----------------------------------------------------------------------
//	Save the data in this object to a buffer

void *ActorAssignment::archive(void *buf) const {
	uint16  *a = (uint16 *)buf;

	*a++ = startFrame;
	*a++ = endFrame;

	return a;
}

//----------------------------------------------------------------------
//	Return a pointer to the specified actor's reserved memory for
//	assignments if it is not already being used.

void *ActorAssignment::operator new (
    size_t bytes,
    Actor *a) {
	return a->allocAssignment(bytes);
}

//----------------------------------------------------------------------
//	Simply mark the actor's assignment as not being used.

void ActorAssignment::operator delete (void *p) {
	((ActorAssignment *)p)->getActor()->freeAssignment();
}

//----------------------------------------------------------------------
//	Determine if the time limit for this assignment has been exceeded

bool ActorAssignment::isValid(void) {
	uint16  frame = calender.frameInDay();

	return      frame < endFrame
	            || (startFrame >= endFrame && frame >= startFrame);
}

//----------------------------------------------------------------------
//	Create a TaskStack for this actor and plug in the assignment's Task.

TaskStack *ActorAssignment::createTask(void) {
	if (!taskNeeded()) return NULL;

	Actor       *a = getActor();
	TaskStack   *ts = NULL;

	if ((ts = new TaskStack(a)) != NULL) {
		Task    *task = getTask(ts);

		if (task != NULL)
			ts->setTask(task);
		else {
			delete ts;
			ts = NULL;
		}
	}

	return ts;
}

//----------------------------------------------------------------------
//	This function is called to notify the assignment of the completion
//	of a task which the assignment had created.

void ActorAssignment::handleTaskCompletion(TaskResult) {
	//  Do nothing
}

//----------------------------------------------------------------------
//	Plug a new task into the actor, if the actor is currently following
//	his assignment

void ActorAssignment::startTask(void) {
	Actor   *a = getActor();

	if (a->currentGoal == actorGoalFollowAssignment)
		a->curTask = createTask();
}

//----------------------------------------------------------------------
//	Determine if this assignment needs to create a task at this time

bool ActorAssignment::taskNeeded(void) {
	return TRUE;
}

/* ===================================================================== *
   PatrolRouteAssignment member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- initial object construction

PatrolRouteAssignment::PatrolRouteAssignment(
    uint16  until,
    int16   rteNo,
    uint8   patrolFlags,
    int16   start,
    int16   end) :
	ActorAssignment(until),
	routeNo(rteNo),
	startingWayPoint(start),
	endingWayPoint(end),
	routeFlags(patrolFlags),
	flags(0) {
}

//----------------------------------------------------------------------
//	Restore the data for this object from a buffer

PatrolRouteAssignment::PatrolRouteAssignment(void **buf) :
	ActorAssignment(buf) {
	void        *bufferPtr = *buf;

	//  Restore route number
	routeNo             = *((int16 *)bufferPtr);
	//  Restore the starting way point
	startingWayPoint    = *((int16 *)bufferPtr + 1);
	//  Restore the ending way point
	endingWayPoint      = *((int16 *)bufferPtr + 2);
	bufferPtr = (int16 *)bufferPtr + 3;

	//  Restore the route flags
	routeFlags = *((uint8 *)bufferPtr);
	//  Restore the assignment flags
	flags = *((uint8 *)bufferPtr + 1);
	bufferPtr = (uint8 *)bufferPtr + 2;

	*buf = bufferPtr;
}

//----------------------------------------------------------------------
//	Return the number of bytes need to archive the data in this
//	assignment

inline int32 PatrolRouteAssignment::archiveSize(void) const {
	return      ActorAssignment::archiveSize()
	            +   sizeof(routeNo)
	            +   sizeof(startingWayPoint)
	            +   sizeof(endingWayPoint)
	            +   sizeof(routeFlags)
	            +   sizeof(flags);
}

//----------------------------------------------------------------------
//	Save the data in this object to a buffer

void *PatrolRouteAssignment::archive(void *buf) const {
	//  Let the base class write its data to the buffer
	buf = ActorAssignment::archive(buf);

	//  Store the route number
	*((int16 *)buf)        = routeNo;
	//  Store the starting way point
	*((int16 *)buf + 1)    = startingWayPoint;
	//  Store the ending way point
	*((int16 *)buf + 2)    = endingWayPoint;
	buf = (int16 *)buf + 3;

	//  Store the route flags
	*((uint8 *)buf)        = routeFlags;
	//  Store the assignment flags
	*((uint8 *)buf + 1)   = flags;
	buf = (uint8 *)buf + 2;

	return buf;
}

//----------------------------------------------------------------------
//	Return an integer representing the class of this object for archival
//	reasons.

int16 PatrolRouteAssignment::type(void) const {
	return patrolRouteAssignment;
}

//----------------------------------------------------------------------
//	This function is called to notify the assignment of the completion
//	of a task which the assignment had created.

void PatrolRouteAssignment::handleTaskCompletion(TaskResult result) {
	if (result == taskSucceeded) flags |= routeCompleted;
}

//----------------------------------------------------------------------
//	Determine if assignment is still valid

bool PatrolRouteAssignment::isValid(void) {
	//  If the route has already been completed, then the assignment is
	//  no longer valid
	if (flags & routeCompleted) return FALSE;

	return ActorAssignment::isValid();
}

//----------------------------------------------------------------------
//	Determine if this assignment needs to create a task at this time

bool PatrolRouteAssignment::taskNeeded(void) {
	//  If the route has already been completed, then no task is needed
	return !(flags & routeCompleted);
}

//----------------------------------------------------------------------
//	Construct a Task for this assignment

Task *PatrolRouteAssignment::getTask(TaskStack *ts) {
	int16       startPoint = startingWayPoint;
	uint8       mapNum = getActor()->getMapNum();

	startingWayPoint = -1;

	if (startPoint == -1) {
		int16               i;
		uint16              bestDist = maxuint16;
		const PatrolRoute   &route = patrolRouteList[ mapNum ][ routeNo ];
		TilePoint           actorLoc = getActor()->getLocation();

		for (i = 0; i < route.vertices(); i++) {
			uint16  dist = lineDist(route[ i ], route[(i + 1) % route.vertices() ], actorLoc);

			if (dist < bestDist) {
				bestDist = dist;
				startPoint = (routeFlags & patrolRouteReverse) ? i : (i + 1) % route.vertices();
			}
		}
	}

	//  Construct a patrol route iterator.
	PatrolRouteIterator
	iter =  PatrolRouteIterator(
	            mapNum,
	            routeNo,
	            routeFlags,
	            startPoint);

	//  Construct a FollowPatrolRouteTask
	return  endingWayPoint != -1
	        ?   NEW_TASK FollowPatrolRouteTask(ts, iter, endingWayPoint)
	        :   NEW_TASK FollowPatrolRouteTask(ts, iter);
}

/* ===================================================================== *
   HuntToBeNearLocationAssignment member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	An initialization function which provides a common ground for
//	the initial constructors.

void HuntToBeNearLocationAssignment::initialize(
    const Target    &targ,
    uint16          r) {
	ASSERT(targ.size() <= sizeof(targetMem));
	//  Make a copy of the target
	targ.clone(targetMem);

	range = r;
}

//----------------------------------------------------------------------
//	Constructor -- constructs from archive buffer

HuntToBeNearLocationAssignment::HuntToBeNearLocationAssignment(void **buf) :
	ActorAssignment(buf) {
	void    *bufferPtr = *buf;

	//  Restore the target
	bufferPtr = constructTarget(targetMem, bufferPtr);

	//  Restore the range
	range = *((uint16 *)bufferPtr);

	*buf = (uint16 *)bufferPtr + 1;
}

//----------------------------------------------------------------------
//	Return the number of bytes need to archive the data in this
//	assignment

inline int32 HuntToBeNearLocationAssignment::archiveSize(void) const {
	return      ActorAssignment::archiveSize()
	            +   targetArchiveSize(getTarget())
	            +   sizeof(range);
}

//----------------------------------------------------------------------
//	Write the data from this assignment object to a buffer in order
//	to save it on disk

void *HuntToBeNearLocationAssignment::archive(void *buf) const {
	//  Let the base class archive its data
	buf = ActorAssignment::archive(buf);

	//  Store the target
	buf = archiveTarget(getTarget(), buf);

	//  Store the range
	*((uint16 *)buf) = range;

	return (uint16 *)buf + 1;
}

//----------------------------------------------------------------------
//	Return an integer representing the class of this object for archival
//	reasons.

int16 HuntToBeNearLocationAssignment::type(void) const {
	return huntToBeNearLocationAssignment;
}

//----------------------------------------------------------------------
//	Determine if this assignment needs to create a task at this time

bool HuntToBeNearLocationAssignment::taskNeeded(void) {
	Actor       *a  = getActor();
	TilePoint   actorLoc = a->getLocation();

	return !a->inRange(getTarget()->where(a->world(), actorLoc), range);
}

//----------------------------------------------------------------------
//	Construct a Task for this assignment

Task *HuntToBeNearLocationAssignment::getTask(TaskStack *ts) {
	return NEW_TASK HuntToBeNearLocationTask(ts, *getTarget(), range);
}

/* ===================================================================== *
   HuntToBeNearActorAssignment member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	An initialization function which provides a common ground for the
//	initial constructors.

void HuntToBeNearActorAssignment::initialize(
    const ActorTarget   &at,
    uint16              r,
    bool                trackFlag) {
	ASSERT(at.size() <= sizeof(targetMem));

	//  Copy the target
	at.clone(targetMem);

	range = r;
	flags = trackFlag ? track : 0;
}

//----------------------------------------------------------------------
//	Constructor -- constructs from archive buffer

HuntToBeNearActorAssignment::HuntToBeNearActorAssignment(void **buf) :
	ActorAssignment(buf) {
	void    *bufferPtr = *buf;

	//  Restore the target
	bufferPtr = constructTarget(targetMem, bufferPtr);

	//  Restore the range
	range = *((uint16 *)bufferPtr);
	bufferPtr = (uint16 *)bufferPtr + 1;

	//  Restore the flags
	flags = *((uint8 *)bufferPtr);
	bufferPtr = (uint8 *)bufferPtr + 1;

	*buf = bufferPtr;
}

//----------------------------------------------------------------------
//	Return the number of bytes need to archive the data in this
//	assignment

inline int32 HuntToBeNearActorAssignment::archiveSize(void) const {
	return      ActorAssignment::archiveSize()
	            +   targetArchiveSize(getTarget())
	            +   sizeof(range)
	            +   sizeof(flags);
}

//----------------------------------------------------------------------
//	Write the data from this assignment object to a buffer in order
//	to save it on disk

void *HuntToBeNearActorAssignment::archive(void *buf) const {
	//  Let the base class archive its data
	buf = ActorAssignment::archive(buf);

	//  Store the target
	buf = archiveTarget(getTarget(), buf);

	//  Store the range
	*((uint16 *)buf) = range;
	buf = (uint16 *)buf + 1;

	//  Store the flags
	*((uint8 *)buf) = flags;
	buf = (uint8 *)buf + 1;

	return buf;
}

//----------------------------------------------------------------------
//	Return an integer representing the class of this object for archival
//	reasons.

int16 HuntToBeNearActorAssignment::type(void) const {
	return huntToBeNearActorAssignment;
}

//----------------------------------------------------------------------
//	Determine if this assignment needs to create a task at this time

bool HuntToBeNearActorAssignment::taskNeeded(void) {
	Actor       *a  = getActor();
	TilePoint   actorLoc = a->getLocation(),
	            targetLoc = getTarget()->where(a->world(), actorLoc);

	return      !a->inRange(targetLoc, range)
	            ||  a->inRange(targetLoc, HuntToBeNearActorTask::tooClose);
}

//----------------------------------------------------------------------
//	Construct a Task for this assignment

Task *HuntToBeNearActorAssignment::getTask(TaskStack *ts) {
	return NEW_TASK HuntToBeNearActorTask(
	           ts,
	           *getTarget(),
	           range,
	           (flags & track) != FALSE);
}

/* ===================================================================== *
   HuntToKillAssignment member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	An initialization function which provides a common ground for the
//	initial constructors.

void HuntToKillAssignment::initialize(
    const ActorTarget   &at,
    bool                trackFlag,
    bool                specificActorFlag) {
	ASSERT(at.size() <= sizeof(targetMem));

	//  Copy the target
	at.clone(targetMem);

	flags = (trackFlag ? track : 0)
	        | (specificActorFlag ? specificActor : 0);
}

//----------------------------------------------------------------------
//	Constructor -- constructs from archive buffer

HuntToKillAssignment::HuntToKillAssignment(void **buf) :
	ActorAssignment(buf) {
	void    *bufferPtr = *buf;

	//  Restore the target
	bufferPtr = constructTarget(targetMem, bufferPtr);

	//  Restore the flags
	flags = *((uint8 *)bufferPtr);
	bufferPtr = (uint8 *)bufferPtr + 1;

	*buf = bufferPtr;
}

//----------------------------------------------------------------------
//	Return the number of bytes need to archive the data in this
//	assignment

inline int32 HuntToKillAssignment::archiveSize(void) const {
	return      ActorAssignment::archiveSize()
	            +   targetArchiveSize(getTarget())
	            +   sizeof(flags);
}

//----------------------------------------------------------------------
//	Write the data from this assignment object to a buffer in order
//	to save it on disk

void *HuntToKillAssignment::archive(void *buf) const {
	//  Let the base class archive its data
	buf = ActorAssignment::archive(buf);

	//  Store the target
	buf = archiveTarget(getTarget(), buf);

	//  Store the flags
	*((uint8 *)buf) = flags;
	buf = (uint8 *)buf + 1;

	return buf;
}

//----------------------------------------------------------------------
//	Determine if this assignment is still valid

bool HuntToKillAssignment::isValid(void) {
	//  If the target actor is already dead, then this is not a valid
	//  assignment
	if (flags & specificActor) {
		SpecificActorTarget     *sat = (SpecificActorTarget *)getTarget();

		if (sat->getTargetActor()->isDead()) return FALSE;
	}

	//  Otherwise, determine if the base class thinks this is a valid
	//  assignment
	return ActorAssignment::isValid();
}

//----------------------------------------------------------------------
//	Return an integer representing the class of this object for archival
//	reasons.

int16 HuntToKillAssignment::type(void) const {
	return huntToKillAssignment;
}

//----------------------------------------------------------------------
//	Determine if this assignment needs to create a task at this time

bool HuntToKillAssignment::taskNeeded(void) {
	//  If we're hunting a specific actor, we only need a task if that
	//  actor is still alive.
	if (flags & specificActor) {
		SpecificActorTarget     *sat = (SpecificActorTarget *)getTarget();

		return !sat->getTargetActor()->isDead();
	}

	//  Otherwise, we'll always want to create a task
	return TRUE;
}

//----------------------------------------------------------------------
//	Construct a Task for this assignment

Task *HuntToKillAssignment::getTask(TaskStack *ts) {
	return NEW_TASK HuntToKillTask(
	           ts,
	           *getTarget(),
	           (flags & track) != FALSE);
}

/* ===================================================================== *
   TetheredAssignment member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- constructs from archive buffer

TetheredAssignment::TetheredAssignment(void **buf) : ActorAssignment(buf) {
	int16   *a = (int16 *)*buf;

	//  Read data from buffer
	minU = *a++;
	minV = *a++;
	maxU = *a++;
	maxV = *a++;

	*buf = a;
}

//----------------------------------------------------------------------
//	Return the number of bytes need to archive the data in this
//	assignment

inline int32 TetheredAssignment::archiveSize(void) const {
	return      ActorAssignment::archiveSize()
	            +   sizeof(minU)
	            +   sizeof(minV)
	            +   sizeof(maxU)
	            +   sizeof(maxV);
}

//----------------------------------------------------------------------
//	Write the data from this assignment object to a buffer in order
//	to save it on disk

void *TetheredAssignment::archive(void *buf) const {
	//  Let the base class archive its data
	buf = ActorAssignment::archive(buf);

	int16   *a = (int16 *)buf;

	//  Copy data to buffer
	*a++ = minU;
	*a++ = minV;
	*a++ = maxU;
	*a++ = maxV;

	return a;
}

/* ===================================================================== *
   TetheredWanderAssignment member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- initial assignment construction

TetheredWanderAssignment::TetheredWanderAssignment(
    uint16 until,
    const TileRegion &reg) :
	TetheredAssignment(until, reg) {
}

//----------------------------------------------------------------------
//	Return an integer representing the class of this object for archival
//	reasons.

int16 TetheredWanderAssignment::type(void) const {
	return tetheredWanderAssignment;
}

//----------------------------------------------------------------------
//	Construct a Task for this assignment

Task *TetheredWanderAssignment::getTask(TaskStack *ts) {
	return NEW_TASK TetheredWanderTask(ts, minU, minV, maxU, maxV);
}

/* ===================================================================== *
   AttendAssignment member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- initial assignment construction

AttendAssignment::AttendAssignment(uint16 until, GameObject *o) :
	ActorAssignment(until),
	obj(o) {
}

//----------------------------------------------------------------------
//	Constructor -- constructs from archive buffer

AttendAssignment::AttendAssignment(void **buf) : ActorAssignment(buf) {
	ObjectID    *bufferPtr = (ObjectID *)*buf;
	ObjectID    objID;

	//  Get the object ID
	objID = *bufferPtr++;

	//  Convert the object ID to an object pointer
	obj = objID != Nothing ? GameObject::objectAddress(objID) : NULL;

	*buf = bufferPtr;
}

//----------------------------------------------------------------------
//	Return the number of bytes need to archive the data in this
//	assignment

inline int32 AttendAssignment::archiveSize(void) const {
	return      ActorAssignment::archiveSize()
	            +   sizeof(ObjectID);
}

//----------------------------------------------------------------------
//	Write the data from this assignment object to a buffer in order
//	to save it on disk

void *AttendAssignment::archive(void *buf) const {
	//  Let the base class write its data to the buffer
	buf = ActorAssignment::archive(buf);

	ObjectID    objID;

	//  Convert the object pointer to an object ID
	objID = obj != NULL ? obj->thisID() : Nothing;

	//  Store the object ID
	*((ObjectID *)buf) = objID;

	return (ObjectID *)buf + 1;
}

//----------------------------------------------------------------------
//	Return an integer representing the class of this object for archival
//	reasons.

int16 AttendAssignment::type(void) const {
	return attendAssignment;
}

//----------------------------------------------------------------------
//	Construct a Task for this assignment

Task *AttendAssignment::getTask(TaskStack *ts) {
	return NEW_TASK AttendTask(ts, obj);
}

/* ===================================================================== *
   Misc functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Reconstruct the ActorAssignment object from the archive buffer

void *constructAssignment(Actor *a, void *buf) {
	//  Get the type which is the first word in the archive buffer
	int16   type = *((int16 *)buf);

	buf = (int16 *)buf + 1;

	//  Based upon the type, call the correct constructor
	switch (type) {
	case patrolRouteAssignment:
		new (a) PatrolRouteAssignment(&buf);
		break;

	case huntToBeNearActorAssignment:
		new (a) HuntToBeNearActorAssignment(&buf);
		break;

	case huntToBeNearLocationAssignment:
		new (a) HuntToBeNearLocationAssignment(&buf);
		break;

	case tetheredWanderAssignment:
		new (a) TetheredWanderAssignment(&buf);
		break;

	case attendAssignment:
		new (a) AttendAssignment(&buf);
		break;
	}

	return buf;
}

//----------------------------------------------------------------------
//	Return the number of bytes necessary to archive this actor's
//	assignment in an archive buffer

int32 assignmentArchiveSize(Actor *a) {
	ActorAssignment     *assign = a->getAssignment();

	return assign != NULL ? sizeof(int16) + assign->archiveSize() : 0;
}

//----------------------------------------------------------------------
//	Write the specified actor's assignment to an archive buffer

void *archiveAssignment(Actor *a, void *buf) {
	ActorAssignment     *assign = a->getAssignment();

	if (assign != NULL) {
		//  Store the type in the buffer and increment the pointer
		*((int16 *)buf) = assign->type();
		buf = (int16 *)buf + 1;

		//  Have the assignment archive itself in the buffer
		buf = assign->archive(buf);
	}

	return buf;
}

}
