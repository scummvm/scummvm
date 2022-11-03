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

#ifndef SAGA2_TASK_H
#define SAGA2_TASK_H

#include "saga2/target.h"
#include "saga2/patrol.h"

namespace Saga2 {

const   int     kDefaultEvalRate = 10;

const   size_t  kMaxTaskSize = 48;

//  Integers representing task types
enum TaskType {
	kWanderTask,
	kTetheredWanderTask,
	kGotoLocationTask,
	kGotoRegionTask,
	kGotoObjectTask,
	kGotoActorTask,
	kGoAwayFromObjectTask,
	kGoAwayFromActorTask,
	kHuntToBeNearLocationTask,
	kHuntToBeNearObjectTask,
	kHuntToPossessTask,
	kHuntToBeNearActorTask,
	kHuntToKillTask,
	kHuntToGiveTask,
	kBandTask,
	kBandAndAvoidEnemiesTask,
	kFollowPatrolRouteTask,
	kAttendTask
};

/* ===================================================================== *
   Function prototypes
 * ===================================================================== */

class Task;
class TaskStack;

//  Run through the active task stacks, updating each
void updateActorTasks();

void pauseActorTasks();
void resumeActorTasks();

//  Allocate a new task stack
TaskStack *newTaskStack(Actor *a);
void newTaskStack(TaskStack *p);

//  Dispose of a previously allocated task stack
void deleteTaskStack(TaskStack *p);

//  Return the ID number of a specified task stack
TaskStackID getTaskStackID(TaskStack *ts);

//  Return a pointer to a TaskStack given a TaskStackID
TaskStack *getTaskStackAddress(TaskStackID id);

//  Initialize the task stack list
void initTaskStacks();

void saveTaskStacks(Common::OutSaveFile *outS);
void loadTaskStacks(Common::InSaveFile *in, int32 chunkSize);

//  Cleanup the task stacks
void cleanupTaskStacks();


void newTask(Task *t);
void newTask(Task *t, TaskID id);

//  Dispose of a previously allocated task
void deleteTask(Task *p);

//  Return a task's ID number
TaskID getTaskID(Task *t);

//  Return a pointer to a Task given a TaskID
Task *getTaskAddress(TaskID id);

//  Initialize the task list
void initTasks();

void saveTasks(Common::OutSaveFile *outS);
void loadTasks(Common::InSaveFile *in, int32 chunkSize);

//  Cleanup the task list
void cleanupTasks();

/* ===================================================================== *
   Task Class
 * ===================================================================== */

//  This is the base class for all Task classes
class Task {
	friend class TaskStack;

protected:
	//  A pointer to this task's stack
	TaskStack   *_stack;
	TaskStackID _stackID;

public:
	Common::String _type;

	//  Constructor -- initial construction
	Task(TaskStack *ts) : _stack(ts), _stackID(NoTaskStack) {
		newTask(this);
	}

	Task(TaskStack *ts, TaskID id) : _stack(ts) {
		newTask(this, id);
	}

	Task(Common::InSaveFile *in, TaskID id);

	//  Virtual destructor -- do nothing
	virtual ~Task() {
		deleteTask(this);
	}

	//	Fixup any subtask pointers
	virtual void fixup();

	//  Return the number of bytes necessary to archive this Task
	//  in a buffer
	virtual int32 archiveSize() const;

	virtual void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	virtual int16 getType() const = 0;

	virtual void abortTask() = 0;
	virtual TaskResult evaluate() = 0;
	virtual TaskResult update() = 0;

	//  Determine if the specified task is equivalent to this task
	virtual bool operator == (const Task &t) const = 0;
	bool operator != (const Task &t) const {
		return !operator == (t);
	}
};

/* ===================================================================== *
   WanderTask Class
 * ===================================================================== */

//  This class is basically a shell around the wander motion task
class WanderTask : public Task {
protected:
	bool            _paused;         //  Flag indicating "paused"ness of this task
	int16           _counter;        //  Counter for tracking pause length

public:
	//  Constructor
	WanderTask(TaskStack *ts) : Task(ts) {
		debugC(2, kDebugTasks, " - WanderTask");
		_type = "WanderTask";
		wander();
	}

	WanderTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	void abortTask();
	TaskResult evaluate();
	TaskResult update();

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

protected:
	//  Update function used while task is not paused
	virtual TaskResult handleWander();

	//  Update function used while task is paused
	TaskResult handlePaused() {
		return kTaskNotDone;
	}

	//  Set this task into the paused state
	void pause();

	//  Set this task into the wander state
	void wander();
};

/* ===================================================================== *
   TetheredWanderTask Class
 * ===================================================================== */

class GotoRegionTask;

//  This class is basically a shell around the tethered wander
//  motion task
class TetheredWanderTask : public WanderTask {
	//  Tether coordinates
	int16           _minU,
	                _minV,
	                _maxU,
	                _maxV;

	//  Pointer to subtask for going to the tether region
	GotoRegionTask  *_gotoTether;
	TaskID _gotoTetherID;

public:
	//  Constructor
	TetheredWanderTask(
	    TaskStack *ts,
	    int16 uMin,
	    int16 vMin,
	    int16 uMax,
	    int16 vMax) :
		WanderTask(ts),
		_minU(uMin),
		_minV(vMin),
		_maxU(uMax),
		_maxV(vMax),
		_gotoTether(NULL),
		_gotoTetherID(NoTask) {
		debugC(2, kDebugTasks, " - TetheredWanderTask");
		_type = "TetheredWanderTask";
	}

	TetheredWanderTask(Common::InSaveFile *in, TaskID id);

	//	Fixup the subtask pointers
	void fixup();

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	void abortTask();

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

	//  Update function used while task is not paused
	TaskResult handleWander();
};

/* ===================================================================== *
   GotoTask Class
 * ===================================================================== */

class GotoTask : public Task {
	WanderTask  *_wander;
	TaskID _wanderID;
	bool        _prevRunState;

public:
	//  Constructor -- initial construction
	GotoTask(TaskStack *ts) :
		Task(ts),
		_wander(NULL),
		_wanderID(NoTask),
		_prevRunState(false) {
		debugC(2, kDebugTasks, " - GotoTask");
		_type = "GotoTask";
	}

	GotoTask(Common::InSaveFile *in, TaskID id);

	//  Fixup the subtask pointer
	void fixup();

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	void abortTask();
	TaskResult evaluate();
	TaskResult update();

private:
	virtual TilePoint destination() = 0;
	virtual TilePoint intermediateDest() = 0;
	virtual bool lineOfSight() = 0;
	virtual bool run() = 0;
};

/* ===================================================================== *
   GotoLocationTask Class
 * ===================================================================== */

class GotoLocationTask : public GotoTask {
	TilePoint   _targetLoc;
	uint8       _runThreshold;

public:
	//  Constructor -- initial construction
	GotoLocationTask(
	    TaskStack       *ts,
	    const TilePoint &tp,
	    uint8           runDist = maxuint8) :
		GotoTask(ts),
		_targetLoc(tp),
		_runThreshold(runDist) {
		debugC(2, kDebugTasks, " - GotoLocationTask");
		_type = "GotoLocationTask";
	}

	GotoLocationTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

	const TilePoint getTarget() const {
		return _targetLoc;
	}

	void changeTarget(const TilePoint &newTarget) {
		_targetLoc = newTarget;
	}

private:
	TilePoint destination();
	TilePoint intermediateDest();
	bool lineOfSight();
	bool run();
};

/* ===================================================================== *
   GotoRegionTask Class
 * ===================================================================== */

class GotoRegionTask : public GotoTask {
	int16       _regionMinU,
	            _regionMinV,
	            _regionMaxU,
	            _regionMaxV;

public:
	//  Constructor -- initial construction
	GotoRegionTask(
	    TaskStack *ts,
	    int16 minU,
	    int16 minV,
	    int16 maxU,
	    int16 maxV) :
		GotoTask(ts),
		_regionMinU(minU),
		_regionMinV(minV),
		_regionMaxU(maxU),
		_regionMaxV(maxV) {
		debugC(2, kDebugTasks, " - GotoRegionTask");
		_type = "GotoRegionTask";
	}

	GotoRegionTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

private:
	TilePoint destination();
	TilePoint intermediateDest();
	bool lineOfSight();
	bool run();
};

/* ===================================================================== *
   GotoObjectTargetTask Class
 * ===================================================================== */

class GotoObjectTargetTask : public GotoTask {
	TilePoint           _lastTestedLoc;
	int16               _sightCtr;

	uint8               _flags;

	enum {
		kTrack       = (1 << 0),
		kInSight     = (1 << 1)
	};

	//  static const doesn't work in Visual C++
	enum {
		kSightRate = 16
	};
//	static const int16  sightRate = 16;

protected:
	TilePoint           _lastKnownLoc;

public:
	//  Constructor -- initial construction
	GotoObjectTargetTask(TaskStack *ts, bool trackFlag) :
		GotoTask(ts),
		_lastTestedLoc(Nowhere),
		_sightCtr(0),
		_flags(trackFlag ? kTrack : 0),
		_lastKnownLoc(Nowhere) {
		debugC(2, kDebugTasks, " - GotoObjectTargetTask");
		_type = "GotoObjectTargetTask";
	}

	GotoObjectTargetTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

private:
	TilePoint destination();
	TilePoint intermediateDest();
	bool lineOfSight();

	virtual GameObject *getObject() = 0;

protected:
	bool tracking() const {
		return (_flags & kTrack) != 0;
	}
	bool isInSight() const {
		return (_flags & kInSight) != 0;
	}
};

//const int16 GotoObjectTargetTask::sightRate = 16;

/* ===================================================================== *
   GotoObjectTask Class
 * ===================================================================== */

class GotoObjectTask : public GotoObjectTargetTask {
	GameObject  *_targetObj;

public:
	//  Constructor -- initial construction
	GotoObjectTask(
	    TaskStack   *ts,
	    GameObject  *obj,
	    bool        trackFlag = false) :
		GotoObjectTargetTask(ts, trackFlag),
		_targetObj(obj) {
		debugC(2, kDebugTasks, " - GotoObjectTask");
		_type = "GotoObjectTask";
	}

	GotoObjectTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

	const GameObject *getTarget() const {
		return _targetObj;
	}

private:
	bool run();
	GameObject *getObject();
};

/* ===================================================================== *
   GotoActorTask Class
 * ===================================================================== */

class GotoActorTask : public GotoObjectTargetTask {
	Actor       *_targetActor;

public:
	//  Constructor -- initial construction
	GotoActorTask(TaskStack *ts, Actor *a, bool trackFlag = false) :
		GotoObjectTargetTask(ts, trackFlag),
		_targetActor(a) {
		debugC(2, kDebugTasks, " - GotoActorTask");
		_type = "GotoActorTask";
	}
	GotoActorTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

	const Actor *getTarget() const {
		return _targetActor;
	}

private:
	bool run();
	GameObject *getObject();
};

/* ===================================================================== *
   GoAwayFromTask Class
 * ===================================================================== */

class GoAwayFromTask : public Task {
	GotoLocationTask        *_goTask;
	TaskID _goTaskID;

	uint8                   _flags;

	enum {
		kRun = (1 << 0)
	};

public:
	//  Constructor -- initial construction
	GoAwayFromTask(TaskStack *ts) :
		Task(ts),
		_goTask(NULL),
		_goTaskID(NoTask),
		_flags(0) {
		debugC(2, kDebugTasks, " - GoAwayFromTask1");
		_type = "GoAwayFromTask";
	}

	GoAwayFromTask(TaskStack *ts, bool runFlag) :
		Task(ts),
		_goTask(NULL),
		_goTaskID(NoTask),
		_flags(runFlag ? kRun : 0) {
		debugC(2, kDebugTasks, " - GoAwayFromTask2");
		_type = "GoAwayFromTask";
	}

	GoAwayFromTask(Common::InSaveFile *in, TaskID id);

	//	Fixup the subtask pointer
	void fixup();

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	void abortTask();
	TaskResult evaluate();
	TaskResult update();

private:
	virtual TilePoint getRepulsionVector() = 0;
};

/* ===================================================================== *
   GoAwayFromObjectTask Class
 * ===================================================================== */

class GoAwayFromObjectTask : public GoAwayFromTask {
	GameObject      *_obj;

public:
	//  Constructor -- initial construction
	GoAwayFromObjectTask(TaskStack *ts, GameObject *object) :
		GoAwayFromTask(ts),
		_obj(object) {
		debugC(2, kDebugTasks, " - GoAwayFromObjectTask");
		_type = "GoAwayFromObjectTask";
	}

	GoAwayFromObjectTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

private:
	TilePoint getRepulsionVector();
};

/* ===================================================================== *
   GoAwayFromActorTask Class
 * ===================================================================== */

class GoAwayFromActorTask : public GoAwayFromTask {
	TargetPlaceHolder   _targetMem;

public:
	//  Constructor -- initial construction
	GoAwayFromActorTask(TaskStack *ts, Actor *a, bool runFlag = false);
	GoAwayFromActorTask(TaskStack *ts, const ActorTarget &at, bool runFlag = false);

	GoAwayFromActorTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

private:
	TilePoint getRepulsionVector();

	const ActorTarget *getTarget() const {
		return (const ActorTarget *)_targetMem;
	}
};

/* ===================================================================== *
   HuntTask Class
 * ===================================================================== */

class HuntTask : public Task {
	Task            *_subTask;   //  This will either be a wander task of a
	TaskID _subTaskID;
	//  goto task
	uint8           _huntFlags;

	enum HuntFlags {
		kHuntWander  = (1 << 0), //  Indicates that subtask is a wander task
		kHuntGoto    = (1 << 1)  //  Indicates that subtask is a goto task
	};

public:
	//  Constructor -- initial construction
	HuntTask(TaskStack *ts) : Task(ts), _huntFlags(0), _subTask(nullptr), _subTaskID(NoTask) {
		debugC(2, kDebugTasks, " - HuntTask");
		_type = "HuntTask";
	}

	HuntTask(Common::InSaveFile *in, TaskID id);

	//	Fixup the subtask pointer
	void fixup();

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	void abortTask();
	TaskResult evaluate();
	TaskResult update();

private:
	void removeWanderTask();
	void removeGotoTask();

protected:
	virtual void evaluateTarget() = 0;

	virtual bool targetHasChanged(GotoTask *gotoTarget) = 0;
	virtual GotoTask *setupGoto() = 0;
	virtual TilePoint currentTargetLoc() = 0;

	virtual bool atTarget() = 0;
	virtual void atTargetabortTask() = 0;
	virtual TaskResult atTargetEvaluate() = 0;
	virtual TaskResult atTargetUpdate() = 0;
};

/* ===================================================================== *
   HuntLocationTask Class
 * ===================================================================== */

class HuntLocationTask : public HuntTask {
	TargetPlaceHolder   _targetMem;

protected:
	TilePoint           _currentTarget;

public:
	//  Constructor -- initial construction
	HuntLocationTask(TaskStack *ts, const Target &t);

	HuntLocationTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

protected:
	bool targetHasChanged(GotoTask *gotoTarget);
	GotoTask *setupGoto();
	TilePoint currentTargetLoc();

	const Target *getTarget() const {
		return (const Target *)_targetMem;
	}
};

/* ===================================================================== *
   HuntToBeNearLocationTask Class
 * ===================================================================== */

class HuntToBeNearLocationTask : public HuntLocationTask {
	uint16              _range;

	uint8               _targetEvaluateCtr;

	//  static const doesn't work in Visual C++
	enum {
		kTargetEvaluateRate = 64
	};
//	static const uint8  kTargetEvaluateRate;

public:
	//  Constructor -- initial construction
	HuntToBeNearLocationTask(TaskStack *ts, const Target &t, uint16 r) :
		HuntLocationTask(ts, t),
		_range(r),
		_targetEvaluateCtr(0) {
		debugC(2, kDebugTasks, " - HuntToBeNearLocationTask");
		_type = "HuntToBeNearLocationTask";
	}

	HuntToBeNearLocationTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

protected:
	void evaluateTarget();

	bool atTarget();

	void atTargetabortTask();
	TaskResult atTargetEvaluate();
	TaskResult atTargetUpdate();

	uint16 getRange() const {
		return _range;
	}
};

//const uint8 HuntToBeNearLocationTask::kTargetEvaluateRate = 64;

/* ===================================================================== *
   HuntObjectTask Class
 * ===================================================================== */

class HuntObjectTask : public HuntTask {
	TargetPlaceHolder   _targetMem;

protected:
	GameObject          *_currentTarget;

public:
	//  Constructor -- initial construction
	HuntObjectTask(TaskStack *ts, const ObjectTarget &ot);

	HuntObjectTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

protected:
	bool targetHasChanged(GotoTask *gotoTarget);
	GotoTask *setupGoto();
	TilePoint currentTargetLoc();

	const ObjectTarget *getTarget() const {
		return (const ObjectTarget *)_targetMem;
	}
};

/* ===================================================================== *
   HuntToBeNearObjectTask Class
 * ===================================================================== */

class HuntToBeNearObjectTask : public HuntObjectTask {
	uint16              _range;

	uint8               _targetEvaluateCtr;

	enum {
		kTargetEvaluateRate = 64
	};
//	static const uint8  kTargetEvaluateRate;

public:
	//  Constructor -- initial construction
	HuntToBeNearObjectTask(
	    TaskStack *ts,
	    const ObjectTarget &ot,
	    uint16 r) :
		HuntObjectTask(ts, ot),
		_range(r),
		_targetEvaluateCtr(0) {
		debugC(2, kDebugTasks, " - HuntToBeNearObjectTask");
		_type = "HuntToBeNearObjectTask";
	}

	HuntToBeNearObjectTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

protected:
	void evaluateTarget();

	bool atTarget();

	void atTargetabortTask();
	TaskResult atTargetEvaluate();
	TaskResult atTargetUpdate();

	uint16 getRange() const {
		return _range;
	}
};

//const uint8   HuntToBeNearObjectTask::kTargetEvaluateRate = 64;

/* ===================================================================== *
   HuntToPossessTask Class
 * ===================================================================== */

class HuntToPossessTask : public HuntObjectTask {
	uint8               _targetEvaluateCtr;

	enum {
		kTargetEvaluateRate = 64
	};
//	static const uint8  kTargetEvaluateRate;

	bool                _grabFlag;

public:
	//  Constructor -- initial construction
	HuntToPossessTask(TaskStack *ts, const ObjectTarget &ot) :
		HuntObjectTask(ts, ot),
		_targetEvaluateCtr(0),
		_grabFlag(false) {
		debugC(2, kDebugTasks, " - HuntToPossessTask");
		_type = "HuntToPossessTask";
	}

	HuntToPossessTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

protected:
	void evaluateTarget();
	bool atTarget();

	void atTargetabortTask();
	TaskResult atTargetEvaluate();
	TaskResult atTargetUpdate();
};

//const uint8 HuntToPossessTask::kTargetEvaluateRate = 16;

/* ===================================================================== *
   HuntActorTask Class
 * ===================================================================== */

class HuntActorTask : public HuntTask {
	TargetPlaceHolder   _targetMem;
	uint8               _flags;

	enum {
		kTrack   = (1 << 0)
	};

protected:
	Actor               *_currentTarget;

public:
	//  Constructor -- initial construction
	HuntActorTask(
	    TaskStack           *ts,
	    const ActorTarget   &at,
	    bool                trackFlag);

	HuntActorTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

protected:
	bool targetHasChanged(GotoTask *gotoTarget);
	GotoTask *setupGoto();
	TilePoint currentTargetLoc();

	const ActorTarget *getTarget() const {
		return (const ActorTarget *)_targetMem;
	}

	bool tracking() const {
		return (_flags & kTrack) != 0;
	}
};

/* ===================================================================== *
   HuntToBeNearActorTask Class
 * ===================================================================== */

class HuntToBeNearActorTask : public HuntActorTask {
	GoAwayFromObjectTask    *_goAway;    //  The 'go away' sub task pointer
	TaskID _goAwayID;
	uint16                  _range;      //  Maximum range

	uint8                   _targetEvaluateCtr;

	enum {
		kTargetEvaluateRate = 16
	};
//	static const uint8  kTargetEvaluateRate;

public:

	enum {
		kTooClose = 12
	};

	//  Constructor -- initial construction
	HuntToBeNearActorTask(
	    TaskStack           *ts,
	    const ActorTarget   &at,
	    uint16              r,
	    bool                trackFlag = false) :
		HuntActorTask(ts, at, trackFlag),
		_goAway(NULL),
		_goAwayID(NoTask),
		_range(MAX<uint16>(r, 16)),
		_targetEvaluateCtr(0) {
		debugC(2, kDebugTasks, " - HuntToBeNearActorTask");
		_type = "HuntToBeNearActorTask";
	}

	HuntToBeNearActorTask(Common::InSaveFile *in, TaskID id);

	//	Fixup the subtask pointer
	void fixup();

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

protected:
	void evaluateTarget();

	bool atTarget();

	void atTargetabortTask();
	TaskResult atTargetEvaluate();
	TaskResult atTargetUpdate();

	uint16 getRange() const {
		return _range;
	}
};

//const uint8   HuntToBeNearActorTask::kTargetEvaluateRate = 64;

/* ===================================================================== *
   HuntToKillTask Class
 * ===================================================================== */

class HuntToKillTask : public HuntActorTask {
	uint8               _targetEvaluateCtr;
	uint8               _specialAttackCtr;

	enum {
		kTargetEvaluateRate = 16
	};

	enum {
		kCurrentWeaponBonus = 1
	};

	uint8               _flags;

	enum {
		kEvalWeapon      = (1 << 0)
	};
//	static const uint8  kTargetEvaluateRate;

public:
	//  Constructor -- initial construction
	HuntToKillTask(
	    TaskStack           *ts,
	    const ActorTarget   &at,
	    bool                trackFlag = false);

	HuntToKillTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

	void abortTask();
	TaskResult update();

protected:
	void evaluateTarget();
	bool atTarget();

	void atTargetabortTask();
	TaskResult atTargetEvaluate();
	TaskResult atTargetUpdate();

private:
	void evaluateWeapon();
};

//const uint8 HuntToKillTask::kTargetEvaluateRate = 16;

//  Utility function used for combat target selection
inline int16 closenessScore(int16 dist) {
	return 128 / dist;
}

/* ===================================================================== *
   HuntToGiveTask Class
 * ===================================================================== */

class HuntToGiveTask : public HuntActorTask {
	GameObject      *_objToGive;

public:
	//  Constructor -- initial construction
	HuntToGiveTask(
	    TaskStack           *ts,
	    const ActorTarget   &at,
	    GameObject          *obj,
	    bool                trackFlag = false) :
		HuntActorTask(ts, at, trackFlag),
		_objToGive(obj) {
		debugC(2, kDebugTasks, " - HuntToGiveTask");
		_type = "HuntToGiveTask";
	}

	HuntToGiveTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

protected:
	void evaluateTarget();
	bool atTarget();

	void atTargetabortTask();
	TaskResult atTargetEvaluate();
	TaskResult atTargetUpdate();
};

/* ===================================================================== *
   BandTask Class
 * ===================================================================== */

class AttendTask;

class BandTask : public HuntTask {
	AttendTask          *_attend;
	TaskID _attendID;

	TilePoint           _currentTarget;
	uint8               _targetEvaluateCtr;

	enum {
		kTargetEvaluateRate = 2
	};

public:

	class RepulsorIterator {
	public:
		virtual ~RepulsorIterator() {}

		virtual bool first(
		    TilePoint   &repulsorVector,
		    int16       &repulsorStrength) = 0;

		virtual bool next(
		    TilePoint   &repulsorVector,
		    int16       &repulsorStrength) = 0;
	};

	class BandingRepulsorIterator : public RepulsorIterator {
	protected:
		Actor           *_a;

	private:
		Band            *_band;
		int             _bandIndex;

	public:
		BandingRepulsorIterator(Actor *actor) : _a(actor), _band(nullptr), _bandIndex(0) {}

		bool first(
		    TilePoint   &repulsorVector,
		    int16       &repulsorStrength);

		bool next(
		    TilePoint   &repulsorVector,
		    int16       &repulsorStrength);
	};


	//  This class should be nested in the BandAndAvoidEnemiesTask class
	//  but Visual C++ 4.0 is lame and won't let the
	//  BandAndAvoidEnemiesTask inherit the BandingRepulsorIterator class
	//  even though it is explicitly declared protected and not private.
	//  Watcom C++, however, works correctly.
	class BandAndAvoidEnemiesRepulsorIterator : public BandingRepulsorIterator {
		Actor       *_actorArray[6];
		int         _numActors,
		            _actorIndex;
		bool        _iteratingThruEnemies;

	public:
		BandAndAvoidEnemiesRepulsorIterator(Actor *actor) :
				BandingRepulsorIterator(actor), _numActors(0), _actorIndex(0), _iteratingThruEnemies(false) {
			for (int i = 0; i < 6; i++)
				_actorArray[i] = 0;
		}

	private:
		bool firstEnemyRepulsor(
		    TilePoint   &repulsorVector,
		    int16       &repulsorStrength);

		bool nextEnemyRepulsor(
		    TilePoint   &repulsorVector,
		    int16       &repulsorStrength);

	public:
		bool first(
		    TilePoint   &repulsorVector,
		    int16       &repulsorStrength);

		bool next(
		    TilePoint   &repulsorVector,
		    int16       &repulsorStrength);
	};

public:
	//  Constructor -- initial construction
	BandTask(TaskStack *ts) :
		HuntTask(ts),
		_attend(NULL),
		_attendID(NoTask),
		_currentTarget(Nowhere),
		_targetEvaluateCtr(0) {
		debugC(2, kDebugTasks, " - BandTask");
		_type = "BandTask";
	}

	BandTask(Common::InSaveFile *in, TaskID id);

	//	Fixup the subtask pointer
	void fixup();

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

protected:
	void evaluateTarget();

	bool targetHasChanged(GotoTask *gotoTarget);
	GotoTask *setupGoto();
	TilePoint currentTargetLoc();

	bool atTarget();

	void atTargetabortTask();
	TaskResult atTargetEvaluate();
	TaskResult atTargetUpdate();

	virtual int16 getRunThreshold();
	virtual RepulsorIterator *getNewRepulsorIterator();
};

/* ===================================================================== *
   BandAndAvoidEnemiesTask Class
 * ===================================================================== */

class BandAndAvoidEnemiesTask : public BandTask {
protected:
	//  I had to move this nested class up to the BandTask class because
	//  Visual C++ is lame.
	/*  class BandAndAvoidEnemiesRepulsorIterator : public BandingRepulsorIterator {
	        Actor       *_actorArray[6];
	        int         _numActors,
	                    _actorIndex;
	        bool        _iteratingThruEnemies;

	    public:
	        BandAndAvoidEnemiesRepulsorIterator(Actor *actor) :
	            BandingRepulsorIterator(actor) {}

	    private:
	        bool firstEnemyRepulsor(
	            TilePoint   &repulsorVector,
	            int16       &repulsorStrength);

	        bool nextEnemyRepulsor(
	            TilePoint   &repulsorVector,
	            int16       &repulsorStrength);

	    public:
	        bool first(
	            TilePoint   &repulsorVector,
	            int16       &repulsorStrength);

	        bool next(
	            TilePoint   &repulsorVector,
	            int16       &repulsorStrength);
	    };
	*/
public:
	//  Constructor -- initial constructor
	BandAndAvoidEnemiesTask(TaskStack *ts) : BandTask(ts) {}

	BandAndAvoidEnemiesTask(Common::InSaveFile *in, TaskID id) : BandTask(in, id) {}

	//  Return an integer representing the type of this task
	int16 getType() const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

protected:
	int16 getRunThreshold();
	RepulsorIterator *getNewRepulsorIterator();
};

/* ===================================================================== *
   FollowPatrolRouteTask Class
 * ===================================================================== */

class FollowPatrolRouteTask : public Task {
	GotoLocationTask        *_gotoWayPoint;  //  A goto waypoint sub task
	TaskID _gotoWayPointID;
	//  pointer.
	PatrolRouteIterator     _patrolIter;     //  The patrol route iterator.
	int16                   _lastWayPointNum;    //  Waypoint at which to end
	//  this task.
	bool                    _paused;         //  Flag indicating "paused"ness
	//  of this task
	int16                   _counter;        //  Counter for tracking pause
	//  length

public:
	//  Constructor -- initial construction
	FollowPatrolRouteTask(
	    TaskStack           *ts,
	    PatrolRouteIterator iter,
	    int16               stopAt = -1) :
		Task(ts),
		_gotoWayPoint(NULL),
		_gotoWayPointID(NoTask),
		_patrolIter(iter),
		_lastWayPointNum(stopAt), _counter(0) {
		debugC(2, kDebugTasks, " - FollowPatrolRouteTask");
		_type = "FollowPatrolRouteTask";
		followPatrolRoute();
	}

	FollowPatrolRouteTask(Common::InSaveFile *in, TaskID id);

	//	Fixup the subtask pointer
	void fixup();

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	void abortTask();
	TaskResult evaluate();
	TaskResult update();

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

	//  Update function used if this task is not paused
	TaskResult handleFollowPatrolRoute();

	//  Update function used if this task is paused
	TaskResult handlePaused();

	//  Set this task into the paused state
	void pause();

	//  Set this task into the unpaused state
	void followPatrolRoute() {
		_paused = false;
	}
};

/* ===================================================================== *
   AttendTask Class
 * ===================================================================== */

class AttendTask : public Task {
	GameObject  *_obj;

public:
	//  Constructor -- initial construction
	AttendTask(TaskStack *ts, GameObject *o) : Task(ts), _obj(o) {
		debugC(2, kDebugTasks, " - AttendTask");
		_type = "AttendTask";
	}

	AttendTask(Common::InSaveFile *in, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	void abortTask();
	TaskResult evaluate();
	TaskResult update();

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;
};

#if 0

//  The defend task is no longer necessary

/* ===================================================================== *
   DefendTask Class
 * ===================================================================== */

class DefendTask : public Task {
	Actor       *_attacker;

	Task        *_subTask;

public:
	//  Constructor -- initial construction
	DefendTask(TaskStack *ts, Actor *a) : Task(ts), _attacker(a), _subTask(NULL) {}

	//  Fixup the subtask pointer
	void fixup();

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	void abortTask();
	TaskResult evaluate();
	TaskResult update();

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;
};

/* ===================================================================== *
   ParryTask Class
 * ===================================================================== */

class ParryTask : public Task {
	Actor               *_attacker;
	GameObject          *_defenseObj;

	uint8               _flags;

	enum {
		kMotionStarted   = (1 << 0),
		kBlockStarted    = (1 << 1)
	};

public:
	//  Constructor -- initial construction
	ParryTask(TaskStack *ts, Actor *a, GameObject *obj) :
		Task(ts),
		_attacker(a),
		_defenseObj(obj),
		_flags(0) {
	}

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	//  Return an integer representing the type of this task
	int16 getType() const;

	void abortTask();
	TaskResult evaluate();
	TaskResult update();

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;
};

#endif

/* ===================================================================== *
   TaskStack Class
 * ===================================================================== */

//  This class contains data common to all task's in an actor task
//  stack.  Also, this class manages the automatic task reevaluation.
class TaskStack {

	TaskID          _stackBottomID;  //  Bottom task in stack

	int16           _evalCount,  //  Counter for automatic task re-evaluation
	                _evalRate;   //  Rate of automatic task re-evalutation
public:
	Actor           *_actor;     //  Pointer to actor performing tasks

	//  Constructor
	TaskStack() :
		_stackBottomID(0),
		_evalCount(0),
		_evalRate(0),
		_actor(nullptr) {}

	TaskStack(Actor *a) :
		_stackBottomID(NoTask),
		_actor(a),
		_evalCount(kDefaultEvalRate),
		_evalRate(kDefaultEvalRate) {

		newTaskStack(this);
	}

	//  Destructor
	~TaskStack() {
		if (_actor)
			_actor->_curTask = nullptr;
		deleteTaskStack(this);
	}

	//  Return the number of bytes necessary to archive this TaskStack
	//  in a buffer
	int32 archiveSize() {
		return      sizeof(ObjectID)     //  actor's id
		            +   sizeof(_stackBottomID)
		            +   sizeof(_evalCount)
		            +   sizeof(_evalRate);
	}

	void write(Common::MemoryWriteStreamDynamic *out);

	void read(Common::InSaveFile *in);

	//  Set the bottom task of this task stack
	void setTask(Task *t);

	//  Return a pointer to the bottom task in this task stack
	const Task *getTask() {
		return  _stackBottomID != NoTask
		        ?   getTaskAddress(_stackBottomID)
		        :   NULL;
	}

	Actor *getActor() {
		return _actor;
	}

	//  Abort all tasks in stack
	void abortTask();
	//  Re-evaluate tasks in stack
	TaskResult evaluate();
	//  Update the state of the tasks in stack
	TaskResult update();
};

} // end of namespace Saga2

#endif
