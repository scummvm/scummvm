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

#ifndef SAGA2_ASSIGN_H
#define SAGA2_ASSIGN_H

#include "saga2/target.h"

namespace Saga2 {

class Actor;
class Task;
class TaskStack;

//  Constants representing the non-virtual ActorAssignment classes
enum AssignmentTypes {
	patrolRouteAssignment,
	huntToBeNearLocationAssignment,
	huntToBeNearActorAssignment,
	huntToKillAssignment,
	tetheredWanderAssignment,
	attendAssignment
};

/* ===================================================================== *
   ActorAssignment class
 * ===================================================================== */

class ActorAssignment {

	enum {
		hasAssignment   = (1 << 3)
	};

	uint16  _startFrame,     //  Time in day when this was constructed
	        _endFrame;       //  End time of the assignment

	Actor *_actor;

public:
	//  Constructor
	ActorAssignment(Actor *a, uint16 until);

	ActorAssignment(Actor *a, Common::SeekableReadStream *stream);

	//  Destructor
	virtual ~ActorAssignment();

	//  Return the number of bytes need to archive the data in this
	//  assignment
	virtual int32 archiveSize() const;

	virtual void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Construct a TaskStack for this assignment
	TaskStack *createTask();

	//  This function is called to notify the assignment of the
	//  completion of a task which the assignment had created.
	virtual void handleTaskCompletion(TaskResult result);

	//  Determine if assignment's time limit is up
	virtual bool isValid();

	//  Return a pointer to the actor to which this assignment belongs
	Actor *getActor() const;

	//  Return an integer representing the class of this assignment
	virtual int16 type() const = 0;

protected:
	void startTask();

	//  Determine if this assignment needs to create a task at this time
	virtual bool taskNeeded();

	//  Create a Task for this assignment
	virtual Task *getTask(TaskStack *ts) = 0;
};

/* ===================================================================== *
   PatrolRouteAssignment class
 * ===================================================================== */

class PatrolRouteAssignment : public ActorAssignment {
	int16   _routeNo,            //  Patrol route number
	        _startingWayPoint,   //  Way point at which to start (-1 = default)
	        _endingWayPoint;     //  Way point at which to end (-1 = default)
	uint8   _routeFlags,         //  Flags indicating how patrol route should
	        //  be followed
	        _flags;              //  Flags representing the state of this
	//  assignment

	enum {
		routeCompleted  = (1 << 0)
	};

public:
	//  Constructor -- initial object construction
	PatrolRouteAssignment(
		Actor *a,
	    uint16 until,
	    int16 rteNo,
	    uint8 patrolFlags,
	    int16 start = -1,
	    int16 end = -1);

	PatrolRouteAssignment(Actor *a, Common::SeekableReadStream *stream);

	//  Return the number of bytes need to archive the data in this
	//  assignment
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this assignment
	int16 type() const;

	//  This function is called to notify the assignment of the
	//  completion of a task which the assignment had created.
	void handleTaskCompletion(TaskResult result);

	//  Determine if assignment is still valid
	bool isValid();

protected:
	//  Determine if this assignment needs to create a task at this time
	bool taskNeeded();

	//  Construct a Task for this assignment
	Task *getTask(TaskStack *ts);
};

/* ===================================================================== *
   HuntToBeNearLocationAssignment class
 * ===================================================================== */

class HuntToBeNearLocationAssignment : public ActorAssignment {
	TargetPlaceHolder   _targetMem;
	uint16              _range;

	//  An initialization function which provides a common ground for
	//  the initial constructors.
	void initialize(const Target &targ, uint16 r);

public:
	//  Constructors -- initial assignment construction

	//  Construct with no time limit and a specific TilePoint
	HuntToBeNearLocationAssignment(Actor *a, const TilePoint &tp, uint16 r);

	//  Construct with time limit and a specific TilePoint
	HuntToBeNearLocationAssignment(
		Actor *a,
	    uint16          until,
	    const TilePoint &tp,
	    uint16          r) :
		ActorAssignment(a, until) {
		initialize(LocationTarget(tp), r);
	}

	//  Construct with no time limit and an abstract target
	HuntToBeNearLocationAssignment(Actor *a, const Target &targ, uint16 r);

	//  Construct with time limit and an abstract target
	HuntToBeNearLocationAssignment(
		Actor *a,
	    uint16          until,
	    const Target    &targ,
	    uint16          r) :
		ActorAssignment(a, until) {
		initialize(targ, r);
	}


	HuntToBeNearLocationAssignment(Actor *a, Common::SeekableReadStream *stream);

	//  Return the number of bytes need to archive the data in this
	//  assignment
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	int16 type() const;

protected:
	bool taskNeeded();

	Task *getTask(TaskStack *ts);

	const Target *getTarget() const {
		return (const Target *)_targetMem;
	}
};

/* ===================================================================== *
   HuntToBeNearActorAssignment class
 * ===================================================================== */

class HuntToBeNearActorAssignment : public ActorAssignment {
	TargetPlaceHolder   _targetMem;
	uint16              _range;
	uint8               _flags;

	enum {
		track           = (1 << 0)  //  This hunt is a track.
	};

	//  An initialization function which provides a common ground for
	//  the initial constructors.
	void initialize(
	    const ActorTarget   &at,
	    uint16              r,
	    bool                trackFlag);

public:
	//  Constructors -- initial assignment construction

	//  Construct with no time limit and specific actor
	HuntToBeNearActorAssignment(
	    Actor               *a,
	    uint16              r,
	    bool                trackFlag = false);

	//  Construct with time limit and specific actor
	HuntToBeNearActorAssignment(
		Actor *ac,
	    uint16              until,
	    Actor               *a,
	    uint16              r,
	    bool                trackFlag = false) :
		ActorAssignment(ac, until) {
		assert(isActor(a) && a != getActor());
		initialize(SpecificActorTarget(a), r, trackFlag);
	}

	//  Construct with no time limit and abstract actor target
	HuntToBeNearActorAssignment(
		Actor *a,
	    const ActorTarget   &at,
	    uint16              r,
	    bool                trackFlag = false);

	//  Construct with time limit and abstract actor target
	HuntToBeNearActorAssignment(
		Actor *a,
	    uint16              until,
	    const ActorTarget   &at,
	    uint16              r,
	    bool                trackFlag = false) :
		ActorAssignment(a, until) {
		initialize(at, r, trackFlag);
	}

	HuntToBeNearActorAssignment(Actor *a, Common::SeekableReadStream *stream);

	//  Return the number of bytes need to archive the data in this
	//  assignment
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	int16 type() const;

protected:
	bool taskNeeded();

	Task *getTask(TaskStack *ts);

	const ActorTarget *getTarget() const {
		return (const ActorTarget *)_targetMem;
	}
};

/* ===================================================================== *
   HuntToKillAssignment class
 * ===================================================================== */

class HuntToKillAssignment : public ActorAssignment {
	TargetPlaceHolder   _targetMem;
	uint8               _flags;

	enum {
		track           = (1 << 0), //  This hunt is a track.
		specificActor   = (1 << 1)  //  The actor target is a specific actor
	};

	//  An initialization function which provides a common ground for
	//  the initial constructors.
	void initialize(
	    const ActorTarget   &at,
	    bool                trackFlag,
	    bool                specificActorFlag);

public:
	//  Constructors -- initial assignment construction

	//  Construct with no time limit and specific actor
	HuntToKillAssignment(Actor *a, bool trackFlag = false);

	//  Construct with time limit and specific actor
	HuntToKillAssignment(
		Actor *ac,
	    uint16              until,
	    Actor               *a,
	    bool                trackFlag = false) :
		ActorAssignment(ac, until) {
		assert(isActor(a) && a != getActor());
		initialize(SpecificActorTarget(a), trackFlag, true);
	}

	//  Construct with no time limit and abstract actor target
	HuntToKillAssignment(
		Actor *a,
	    const ActorTarget   &at,
	    bool                trackFlag = false);

	//  Construct with time limit and abstract actor target
	HuntToKillAssignment(
		Actor *a,
	    uint16              until,
	    const ActorTarget   &at,
	    bool                trackFlag = false) :
		ActorAssignment(a, until) {
		initialize(at, trackFlag, false);
	}

	//  Return the number of bytes need to archive the data in this
	//  assignment
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Determine if assignment's time limit is up or if the actor is
	//  already dead
	bool isValid();

	int16 type() const;

protected:
	bool taskNeeded();

	Task *getTask(TaskStack *ts);

	const ActorTarget *getTarget() const {
		return (const ActorTarget *)_targetMem;
	}
};

/* ===================================================================== *
   TetheredAssignment class
 * ===================================================================== */

class TetheredAssignment : public ActorAssignment {
protected:
	//  Tether region
	int16       _minU,   //  Minimum U coordinate in tether
	            _minV,   //  Minimum V coordinate in tether
	            _maxU,   //  Maximum U coordinate in tether
	            _maxV;   //  Maximum V coordinate in tether

public:
	//  Constructor -- initial assignment construction
	TetheredAssignment(Actor *a, uint16 until, const TileRegion &reg) :
		ActorAssignment(a, until),
		_minU(reg.min.u),
		_minV(reg.min.v),
		_maxU(reg.max.u),
		_maxV(reg.max.v) {
	}

	TetheredAssignment(Actor *a, Common::SeekableReadStream *stream);

	//  Return the number of bytes need to archive the data in this
	//  assignment
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;
};

/* ===================================================================== *
   TetheredWanderAssignment class
 * ===================================================================== */

class TetheredWanderAssignment : public TetheredAssignment {
public:
	//  Constructor -- initial assignment construction
	TetheredWanderAssignment(Actor *a, uint16 until, const TileRegion &reg);

	TetheredWanderAssignment(Actor *a, Common::SeekableReadStream *stream) : TetheredAssignment(a, stream) {}

	//  Return an integer representing the type of this assignment
	int16 type() const;

protected:
	//  Construct a Task for this assignment
	Task *getTask(TaskStack *ts);
};

/* ===================================================================== *
   AttendAssignment class
 * ===================================================================== */

class AttendAssignment : public ActorAssignment {
	GameObject      *_obj;   //  Object to which to attend

public:
	//  Constructor -- initial assignment construction
	AttendAssignment(Actor *a, uint16 until, GameObject *o);

	AttendAssignment(Actor *a, Common::SeekableReadStream *stream);

	//  Return the number of bytes need to archive the data in this
	//  assignment
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this assignment
	int16 type() const;

protected:
	//  Construct a Task for this assignment
	Task *getTask(TaskStack *ts);
};

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

//  Return the number of bytes necessary to archive this actor's
//  assignment in an archive buffer
int32 assignmentArchiveSize(Actor *a);

void writeAssignment(Actor *a, Common::MemoryWriteStreamDynamic *out);
void readAssignment(Actor *a, Common::InSaveFile *in);

}

#endif
