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

#include "saga2/saga2.h"
#include "saga2/actor.h"
#include "saga2/assign.h"
#include "saga2/calender.h"
#include "saga2/task.h"
#include "saga2/tile.h"

namespace Saga2 {

const uint16 kIndefiniteTime = CalenderTime::kFramesPerDay;

/* ===================================================================== *
   ActorAssignment member functions
 * ===================================================================== */

//  Constructor
ActorAssignment::ActorAssignment(Actor *a, uint16 until) :
	_startFrame(g_vm->_calender->frameInDay()),
	_endFrame(until) {
	_actor = a;
	debugC(2, kDebugActors, "New assignment for %p (%s) from %d until %d: %p",
	      (void *)a, a->objName(), _startFrame, _endFrame, (void *)this);
	a->_assignment = this;
	a->_flags |= hasAssignment;
}

ActorAssignment::ActorAssignment(Actor *ac, Common::SeekableReadStream *stream) {
	_startFrame = stream->readUint16LE();
	_endFrame = stream->readUint16LE();

	_actor = ac;
	ac->_assignment = this;
	ac->_flags |= hasAssignment;
}

//----------------------------------------------------------------------
//	ActorAssignment destructor

ActorAssignment::~ActorAssignment() {
	Actor *a = getActor();
	debugC(2, kDebugActors, "Ending assignment for %p (%s): %p",
	      (void *)a, a->objName(), (void *)this);

	//  Determine if the actor has a task initiated by this assignment
	if (a->_currentGoal == actorGoalFollowAssignment
	        &&  a->_curTask != nullptr) {
		//  If so, abort it
		a->_curTask->abortTask();
		delete a->_curTask;
		a->_curTask = nullptr;
	}

	a->_flags &= ~hasAssignment;
}

//----------------------------------------------------------------------
//	Return the number of bytes need to archive the data in this
//	assignment

inline int32 ActorAssignment::archiveSize() const {
	return sizeof(_startFrame) + sizeof(_endFrame);
}

void ActorAssignment::write(Common::MemoryWriteStreamDynamic *out) const {
	out->writeUint16LE(_startFrame);
	out->writeUint16LE(_endFrame);
}

//----------------------------------------------------------------------
//	Determine if the time limit for this assignment has been exceeded

bool ActorAssignment::isValid() {
	uint16  frame = g_vm->_calender->frameInDay();

	return      frame < _endFrame
	            || (_startFrame >= _endFrame && frame >= _startFrame);
}

//----------------------------------------------------------------------
//	Create a TaskStack for this actor and plug in the assignment's Task.

TaskStack *ActorAssignment::createTask() {
	if (!taskNeeded()) return nullptr;

	Actor       *a = getActor();
	TaskStack   *ts = nullptr;

	if ((ts = newTaskStack(a)) != nullptr) {
		Task    *task = getTask(ts);

		if (task != nullptr)
			ts->setTask(task);
		else {
			delete ts;
			ts = nullptr;
		}
	}

	return ts;
}

Actor *ActorAssignment::getActor() const {
	return _actor;
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

void ActorAssignment::startTask() {
	Actor   *a = getActor();

	if (a->_currentGoal == actorGoalFollowAssignment)
		a->_curTask = createTask();
}

//----------------------------------------------------------------------
//	Determine if this assignment needs to create a task at this time

bool ActorAssignment::taskNeeded() {
	return true;
}

/* ===================================================================== *
   PatrolRouteAssignment member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- initial object construction

PatrolRouteAssignment::PatrolRouteAssignment(
	Actor *a,
    uint16  until,
    int16   rteNo,
    uint8   patrolFlags,
    int16   start,
    int16   end) :
	ActorAssignment(a, until),
	_routeNo(rteNo),
	_startingWayPoint(start),
	_endingWayPoint(end),
	_routeFlags(patrolFlags),
	_flags(0) {
}

PatrolRouteAssignment::PatrolRouteAssignment(Actor *a, Common::SeekableReadStream *stream) :
	ActorAssignment(a, stream) {
	debugC(4, kDebugSaveload, "... Loading PatrolRouteAssignment");

	//  Restore route number
	_routeNo = stream->readSint16LE();
	//  Restore the starting way point
	_startingWayPoint = stream->readSint16LE();
	//  Restore the ending way point
	_endingWayPoint = stream->readSint16LE();

	//  Restore the route flags
	_routeFlags = stream->readByte();
	//  Restore the assignment flags
	_flags = stream->readByte();
}

//----------------------------------------------------------------------
//	Return the number of bytes need to archive the data in this
//	assignment

inline int32 PatrolRouteAssignment::archiveSize() const {
	return      ActorAssignment::archiveSize()
	            +   sizeof(_routeNo)
	            +   sizeof(_startingWayPoint)
	            +   sizeof(_endingWayPoint)
	            +   sizeof(_routeFlags)
	            +   sizeof(_flags);
}

void PatrolRouteAssignment::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving PatrolRouteAssignment");

	//  Let the base class write its data to the buffer
	ActorAssignment::write(out);

	//  Store the route number
	out->writeSint16LE(_routeNo);
	out->writeSint16LE(_startingWayPoint);
	out->writeSint16LE(_endingWayPoint);

	//  Store the route flags
	out->writeByte(_routeFlags);
	//  Store the assignment flags
	out->writeByte(_flags);
}

//----------------------------------------------------------------------
//	Return an integer representing the class of this object for archival
//	reasons.

int16 PatrolRouteAssignment::type() const {
	return patrolRouteAssignment;
}

//----------------------------------------------------------------------
//	This function is called to notify the assignment of the completion
//	of a task which the assignment had created.

void PatrolRouteAssignment::handleTaskCompletion(TaskResult result) {
	if (result == taskSucceeded) _flags |= routeCompleted;
}

//----------------------------------------------------------------------
//	Determine if assignment is still valid

bool PatrolRouteAssignment::isValid() {
	//  If the route has already been completed, then the assignment is
	//  no longer valid
	if (_flags & routeCompleted) return false;

	return ActorAssignment::isValid();
}

//----------------------------------------------------------------------
//	Determine if this assignment needs to create a task at this time

bool PatrolRouteAssignment::taskNeeded() {
	//  If the route has already been completed, then no task is needed
	return !(_flags & routeCompleted);
}

//----------------------------------------------------------------------
//	Construct a Task for this assignment

Task *PatrolRouteAssignment::getTask(TaskStack *ts) {
	int16       startPoint = _startingWayPoint;
	uint8       mapNum = getActor()->getMapNum();

	_startingWayPoint = -1;

	if (startPoint == -1) {
		int16               i;
		uint16              bestDist = maxuint16;
		const PatrolRoute   &route = patrolRouteList[mapNum]->getRoute(_routeNo);
		TilePoint           actorLoc = getActor()->getLocation();

		for (i = 0; i < route.vertices(); i++) {
			uint16  dist = lineDist(route[i], route[(i + 1) % route.vertices()], actorLoc);

			if (dist < bestDist) {
				bestDist = dist;
				startPoint = (_routeFlags & patrolRouteReverse) ? i : (i + 1) % route.vertices();
			}
		}
	}

	//  Construct a patrol route iterator.
	PatrolRouteIterator
	iter =  PatrolRouteIterator(
	            mapNum,
	            _routeNo,
	            _routeFlags,
	            startPoint);

	//  Construct a FollowPatrolRouteTask
	return  _endingWayPoint != -1
	        ?   new FollowPatrolRouteTask(ts, iter, _endingWayPoint)
	        :   new FollowPatrolRouteTask(ts, iter);
}

/* ===================================================================== *
   HuntToBeNearLocationAssignment member functions
 * ===================================================================== */

HuntToBeNearLocationAssignment::HuntToBeNearLocationAssignment(Actor *a, const TilePoint &tp, uint16 r) :
	ActorAssignment(a, kIndefiniteTime) {
	initialize(LocationTarget(tp), r);
}

//  Construct with no time limit and an abstract target
HuntToBeNearLocationAssignment::HuntToBeNearLocationAssignment(Actor *a, const Target &targ, uint16 r) :
	ActorAssignment(a, kIndefiniteTime) {
	initialize(targ, r);
}

//----------------------------------------------------------------------
//	An initialization function which provides a common ground for
//	the initial constructors.

void HuntToBeNearLocationAssignment::initialize(
    const Target    &targ,
    uint16          r) {
	assert(targ.size() <= sizeof(_targetMem));
	//  Make a copy of the target
	targ.clone(_targetMem);

	_range = r;
}

HuntToBeNearLocationAssignment::HuntToBeNearLocationAssignment(Actor *a, Common::SeekableReadStream *stream) :
	ActorAssignment(a, stream) {
	debugC(4, kDebugSaveload, "... Loading HuntToBeNearLocationAssignment");

	//  Restore the target
	readTarget(_targetMem, stream);

	//  Restore the range
	_range = stream->readUint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes need to archive the data in this
//	assignment

inline int32 HuntToBeNearLocationAssignment::archiveSize() const {
	return      ActorAssignment::archiveSize()
	            +   targetArchiveSize(getTarget())
	            +   sizeof(_range);
}

void HuntToBeNearLocationAssignment::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToBeNearLocationAssignment");

	//  Let the base class archive its data
	ActorAssignment::write(out);

	//  Store the target
	writeTarget(getTarget(), out);

	//  Store the range
	out->writeUint16LE(_range);
}

//----------------------------------------------------------------------
//	Return an integer representing the class of this object for archival
//	reasons.

int16 HuntToBeNearLocationAssignment::type() const {
	return huntToBeNearLocationAssignment;
}

//----------------------------------------------------------------------
//	Determine if this assignment needs to create a task at this time

bool HuntToBeNearLocationAssignment::taskNeeded() {
	Actor       *a  = getActor();
	TilePoint   actorLoc = a->getLocation();

	return !a->inRange(getTarget()->where(a->world(), actorLoc), _range);
}

//----------------------------------------------------------------------
//	Construct a Task for this assignment

Task *HuntToBeNearLocationAssignment::getTask(TaskStack *ts) {
	return new HuntToBeNearLocationTask(ts, *getTarget(), _range);
}

/* ===================================================================== *
   HuntToBeNearActorAssignment member functions
 * ===================================================================== */

//  Construct with no time limit and specific actor
HuntToBeNearActorAssignment::HuntToBeNearActorAssignment(
	Actor               *a,
	uint16              r,
	bool                trackFlag) :
	ActorAssignment(a, kIndefiniteTime) {
	assert(isActor(a) && a != getActor());
	initialize(SpecificActorTarget(a), r, trackFlag);
}

//  Construct with no time limit and abstract actor target
HuntToBeNearActorAssignment::HuntToBeNearActorAssignment(
	Actor *a,
	const ActorTarget   &at,
	uint16              r,
	bool                trackFlag) :
	ActorAssignment(a, kIndefiniteTime) {
	initialize(at, r, trackFlag);
}

//----------------------------------------------------------------------
//	An initialization function which provides a common ground for the
//	initial constructors.

void HuntToBeNearActorAssignment::initialize(
    const ActorTarget   &at,
    uint16              r,
    bool                trackFlag) {
	assert(at.size() <= sizeof(_targetMem));

	//  Copy the target
	at.clone(_targetMem);

	_range = r;
	_flags = trackFlag ? track : 0;
}

HuntToBeNearActorAssignment::HuntToBeNearActorAssignment(Actor *a, Common::SeekableReadStream *stream) :
	ActorAssignment(a, stream) {
	debugC(4, kDebugSaveload, "... Loading HuntToBeNearActorAssignment");

	readTarget(_targetMem, stream);

	//  Restore the range
	_range = stream->readUint16LE();

	//  Restore the flags
	_flags = stream->readByte();
}

//----------------------------------------------------------------------
//	Return the number of bytes need to archive the data in this
//	assignment

inline int32 HuntToBeNearActorAssignment::archiveSize() const {
	return      ActorAssignment::archiveSize()
	            +   targetArchiveSize(getTarget())
	            +   sizeof(_range)
	            +   sizeof(_flags);
}

void HuntToBeNearActorAssignment::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToBeNearActorAssignment");

	//  Let the base class archive its data
	ActorAssignment::write(out);

	//  Store the target
	writeTarget(getTarget(), out);

	//  Store the range
	out->writeUint16LE(_range);

	//  Store the flags
	out->writeByte(_flags);
}

//----------------------------------------------------------------------
//	Return an integer representing the class of this object for archival
//	reasons.

int16 HuntToBeNearActorAssignment::type() const {
	return huntToBeNearActorAssignment;
}

//----------------------------------------------------------------------
//	Determine if this assignment needs to create a task at this time

bool HuntToBeNearActorAssignment::taskNeeded() {
	Actor       *a  = getActor();
	TilePoint   actorLoc = a->getLocation(),
	            targetLoc = getTarget()->where(a->world(), actorLoc);

	return      !a->inRange(targetLoc, _range)
	            ||  a->inRange(targetLoc, HuntToBeNearActorTask::tooClose);
}

//----------------------------------------------------------------------
//	Construct a Task for this assignment

Task *HuntToBeNearActorAssignment::getTask(TaskStack *ts) {
	return new HuntToBeNearActorTask(
	           ts,
	           *getTarget(),
	           _range,
	           (_flags & track) != false);
}

/* ===================================================================== *
   HuntToKillAssignment member functions
 * ===================================================================== */

//  Construct with no time limit and specific actor
HuntToKillAssignment::HuntToKillAssignment(Actor *a, bool trackFlag) :
	ActorAssignment(a, kIndefiniteTime) {
	assert(isActor(a) && a != getActor());
	initialize(SpecificActorTarget(a), trackFlag, true);
}

//  Construct with no time limit and abstract actor target
HuntToKillAssignment::HuntToKillAssignment(
	Actor *a,
	const ActorTarget   &at,
	bool                trackFlag) :
	ActorAssignment(a, kIndefiniteTime) {
	initialize(at, trackFlag, false);
}


//----------------------------------------------------------------------
//	An initialization function which provides a common ground for the
//	initial constructors.

void HuntToKillAssignment::initialize(
    const ActorTarget   &at,
    bool                trackFlag,
    bool                specificActorFlag) {
	assert(at.size() <= sizeof(_targetMem));

	//  Copy the target
	at.clone(_targetMem);

	_flags = (trackFlag ? track : 0)
	        | (specificActorFlag ? specificActor : 0);
}

//----------------------------------------------------------------------
//	Return the number of bytes need to archive the data in this
//	assignment

inline int32 HuntToKillAssignment::archiveSize() const {
	return      ActorAssignment::archiveSize()
	            +   targetArchiveSize(getTarget())
	            +   sizeof(_flags);
}

void HuntToKillAssignment::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToKillAssignment");

	//  Let the base class archive its data
	ActorAssignment::write(out);

	//  Store the target
	writeTarget(getTarget(), out);

	//  Store the flags
	out->writeByte(_flags);
}

//----------------------------------------------------------------------
//	Determine if this assignment is still valid

bool HuntToKillAssignment::isValid() {
	//  If the target actor is already dead, then this is not a valid
	//  assignment
	if (_flags & specificActor) {
		const SpecificActorTarget *sat = (const SpecificActorTarget *)getTarget();

		if (sat->getTargetActor()->isDead()) return false;
	}

	//  Otherwise, determine if the base class thinks this is a valid
	//  assignment
	return ActorAssignment::isValid();
}

//----------------------------------------------------------------------
//	Return an integer representing the class of this object for archival
//	reasons.

int16 HuntToKillAssignment::type() const {
	return huntToKillAssignment;
}

//----------------------------------------------------------------------
//	Determine if this assignment needs to create a task at this time

bool HuntToKillAssignment::taskNeeded() {
	//  If we're hunting a specific actor, we only need a task if that
	//  actor is still alive.
	if (_flags & specificActor) {
		const SpecificActorTarget *sat = (const SpecificActorTarget *)getTarget();

		return !sat->getTargetActor()->isDead();
	}

	//  Otherwise, we'll always want to create a task
	return true;
}

//----------------------------------------------------------------------
//	Construct a Task for this assignment

Task *HuntToKillAssignment::getTask(TaskStack *ts) {
	return new HuntToKillTask(
	           ts,
	           *getTarget(),
	           (_flags & track) != false);
}

/* ===================================================================== *
   TetheredAssignment member functions
 * ===================================================================== */

TetheredAssignment::TetheredAssignment(Actor *ac, Common::SeekableReadStream *stream) : ActorAssignment(ac, stream) {
	debugC(4, kDebugSaveload, "... Loading TetheredAssignment");

	//  Read data from buffer
	_minU = stream->readSint16LE();
	_minV = stream->readSint16LE();
	_maxU = stream->readSint16LE();
	_maxV = stream->readSint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes need to archive the data in this
//	assignment

inline int32 TetheredAssignment::archiveSize() const {
	return      ActorAssignment::archiveSize()
	            +   sizeof(_minU)
	            +   sizeof(_minV)
	            +   sizeof(_maxU)
	            +   sizeof(_maxV);
}

void TetheredAssignment::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving TetheredAssignment");

	//  Let the base class archive its data
	ActorAssignment::write(out);

	//  Copy data to buffer
	out->writeSint16LE(_minU);
	out->writeSint16LE(_minV);
	out->writeSint16LE(_maxU);
	out->writeSint16LE(_maxV);
}

/* ===================================================================== *
   TetheredWanderAssignment member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- initial assignment construction

TetheredWanderAssignment::TetheredWanderAssignment(
	Actor *a,
    uint16 until,
    const TileRegion &reg) :
	TetheredAssignment(a, until, reg) {
}

//----------------------------------------------------------------------
//	Return an integer representing the class of this object for archival
//	reasons.

int16 TetheredWanderAssignment::type() const {
	return tetheredWanderAssignment;
}

//----------------------------------------------------------------------
//	Construct a Task for this assignment

Task *TetheredWanderAssignment::getTask(TaskStack *ts) {
	return new TetheredWanderTask(ts, _minU, _minV, _maxU, _maxV);
}

/* ===================================================================== *
   AttendAssignment member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- initial assignment construction

AttendAssignment::AttendAssignment(Actor *a, uint16 until, GameObject *o) :
	ActorAssignment(a, until),
	_obj(o) {
}

AttendAssignment::AttendAssignment(Actor *a, Common::SeekableReadStream *stream) : ActorAssignment(a, stream) {
	debugC(4, kDebugSaveload, "... Loading AttendAssignment");

	ObjectID    objID;

	//  Get the object ID
	objID = stream->readUint16LE();

	//  Convert the object ID to an object pointer
	_obj = objID != Nothing ? GameObject::objectAddress(objID) : nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes need to archive the data in this
//	assignment

inline int32 AttendAssignment::archiveSize() const {
	return      ActorAssignment::archiveSize()
	            +   sizeof(ObjectID);
}

void AttendAssignment::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving AttendAssignment");

	//  Let the base class write its data to the buffer
	ActorAssignment::write(out);

	ObjectID    objID;

	//  Convert the object pointer to an object ID
	objID = _obj != nullptr ? _obj->thisID() : Nothing;

	//  Store the object ID
	out->writeUint16LE(objID);
}

//----------------------------------------------------------------------
//	Return an integer representing the class of this object for archival
//	reasons.

int16 AttendAssignment::type() const {
	return attendAssignment;
}

//----------------------------------------------------------------------
//	Construct a Task for this assignment

Task *AttendAssignment::getTask(TaskStack *ts) {
	return new AttendTask(ts, _obj);
}

/* ===================================================================== *
   Misc functions
 * ===================================================================== */

void readAssignment(Actor *a, Common::InSaveFile *in) {
	//  Get the type which is the first word in the archive buffer
	int16 type = in->readSint16LE();

	//  Based upon the type, call the correct constructor
	switch (type) {
	case patrolRouteAssignment:
		new PatrolRouteAssignment(a, in);
		break;

	case huntToBeNearActorAssignment:
		new HuntToBeNearActorAssignment(a, in);
		break;

	case huntToBeNearLocationAssignment:
		new HuntToBeNearLocationAssignment(a, in);
		break;

	case tetheredWanderAssignment:
		new TetheredWanderAssignment(a, in);
		break;

	case attendAssignment:
		new AttendAssignment(a, in);
		break;
	}
}

//----------------------------------------------------------------------
//	Return the number of bytes necessary to archive this actor's
//	assignment in an archive buffer

int32 assignmentArchiveSize(Actor *a) {
	ActorAssignment     *assign = a->getAssignment();

	return assign != nullptr ? sizeof(int16) + assign->archiveSize() : 0;
}

void writeAssignment(Actor *a, Common::MemoryWriteStreamDynamic *out) {
	ActorAssignment *assign = a->getAssignment();

	if (assign != nullptr) {
		out->writeSint16LE(assign->type());

		assign->write(out);
	}
}

}
