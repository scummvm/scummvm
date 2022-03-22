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

const   int     defaultEvalRate = 10;

const   size_t  maxTaskSize = 48;

//  Integers representing task types
enum TaskType {
	wanderTask,
	tetheredWanderTask,
	gotoLocationTask,
	gotoRegionTask,
	gotoObjectTask,
	gotoActorTask,
	goAwayFromObjectTask,
	goAwayFromActorTask,
	huntToBeNearLocationTask,
	huntToBeNearObjectTask,
	huntToPossessTask,
	huntToBeNearActorTask,
	huntToKillTask,
	huntToGiveTask,
	bandTask,
	bandAndAvoidEnemiesTask,
	followPatrolRouteTask,
	attendTask
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
	TaskStack   *stack;
	TaskStackID _stackID;

public:
	Common::String _type;

	//  Constructor -- initial construction
	Task(TaskStack *ts) : stack(ts), _stackID(NoTaskStack) {
		newTask(this);
	}

	Task(TaskStack *ts, TaskID id) : stack(ts) {
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
	bool            paused;         //  Flag indicating "paused"ness of this task
	int16           counter;        //  Counter for tracking pause length

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
		return taskNotDone;
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
	int16           minU,
	                minV,
	                maxU,
	                maxV;

	//  Pointer to subtask for going to the tether region
	GotoRegionTask  *gotoTether;
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
		minU(uMin),
		minV(vMin),
		maxU(uMax),
		maxV(vMax),
		gotoTether(NULL),
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
	WanderTask  *wander;
	TaskID _wanderID;
	bool        prevRunState;

public:
	//  Constructor -- initial construction
	GotoTask(TaskStack *ts) :
		Task(ts),
		wander(NULL),
		_wanderID(NoTask),
		prevRunState(false) {
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
	TilePoint   targetLoc;
	uint8       runThreshold;

public:
	//  Constructor -- initial construction
	GotoLocationTask(
	    TaskStack       *ts,
	    const TilePoint &tp,
	    uint8           runDist = maxuint8) :
		GotoTask(ts),
		targetLoc(tp),
		runThreshold(runDist) {
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
		return targetLoc;
	}

	void changeTarget(const TilePoint &newTarget) {
		targetLoc = newTarget;
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
	int16       regionMinU,
	            regionMinV,
	            regionMaxU,
	            regionMaxV;

public:
	//  Constructor -- initial construction
	GotoRegionTask(
	    TaskStack *ts,
	    int16 minU,
	    int16 minV,
	    int16 maxU,
	    int16 maxV) :
		GotoTask(ts),
		regionMinU(minU),
		regionMinV(minV),
		regionMaxU(maxU),
		regionMaxV(maxV) {
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
	TilePoint           lastTestedLoc;
	int16               sightCtr;

	uint8               flags;

	enum {
		track       = (1 << 0),
		inSight     = (1 << 1)
	};

	//  static const doesn't work in Visual C++
	enum {
		sightRate = 16
	};
//	static const int16  sightRate = 16;

protected:
	TilePoint           lastKnownLoc;

public:
	//  Constructor -- initial construction
	GotoObjectTargetTask(TaskStack *ts, bool trackFlag) :
		GotoTask(ts),
		lastTestedLoc(Nowhere),
		sightCtr(0),
		flags(trackFlag ? track : 0),
		lastKnownLoc(Nowhere) {
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
		return (flags & track) != 0;
	}
	bool isInSight() const {
		return (flags & inSight) != 0;
	}
};

//const int16 GotoObjectTargetTask::sightRate = 16;

/* ===================================================================== *
   GotoObjectTask Class
 * ===================================================================== */

class GotoObjectTask : public GotoObjectTargetTask {
	GameObject  *targetObj;

public:
	//  Constructor -- initial construction
	GotoObjectTask(
	    TaskStack   *ts,
	    GameObject  *obj,
	    bool        trackFlag = false) :
		GotoObjectTargetTask(ts, trackFlag),
		targetObj(obj) {
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
		return targetObj;
	}

private:
	bool run();
	GameObject *getObject();
};

/* ===================================================================== *
   GotoActorTask Class
 * ===================================================================== */

class GotoActorTask : public GotoObjectTargetTask {
	Actor       *targetActor;

public:
	//  Constructor -- initial construction
	GotoActorTask(TaskStack *ts, Actor *a, bool trackFlag = false) :
		GotoObjectTargetTask(ts, trackFlag),
		targetActor(a) {
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
		return targetActor;
	}

private:
	bool run();
	GameObject *getObject();
};

/* ===================================================================== *
   GoAwayFromTask Class
 * ===================================================================== */

class GoAwayFromTask : public Task {
	GotoLocationTask        *goTask;
	TaskID _goTaskID;

	uint8                   flags;

	enum {
		run = (1 << 0)
	};

public:
	//  Constructor -- initial construction
	GoAwayFromTask(TaskStack *ts) :
		Task(ts),
		goTask(NULL),
		_goTaskID(NoTask),
		flags(0) {
		debugC(2, kDebugTasks, " - GoAwayFromTask1");
		_type = "GoAwayFromTask";
	}

	GoAwayFromTask(TaskStack *ts, bool runFlag) :
		Task(ts),
		goTask(NULL),
		_goTaskID(NoTask),
		flags(runFlag ? run : 0) {
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
	GameObject      *obj;

public:
	//  Constructor -- initial construction
	GoAwayFromObjectTask(TaskStack *ts, GameObject *object) :
		GoAwayFromTask(ts),
		obj(object) {
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
	TargetPlaceHolder   targetMem;

public:
	//  Constructor -- initial construction
	GoAwayFromActorTask(
	    TaskStack   *ts,
	    Actor       *a,
	    bool        runFlag = false);
	GoAwayFromActorTask(
	    TaskStack           *ts,
	    const ActorTarget   &at,
	    bool                runFlag = false);

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
		return (const ActorTarget *)targetMem;
	}
};

/* ===================================================================== *
   HuntTask Class
 * ===================================================================== */

class HuntTask : public Task {
	Task            *subTask;   //  This will either be a wander task of a
	TaskID _subTaskID;
	//  goto task
	uint8           huntFlags;

	enum HuntFlags {
		huntWander  = (1 << 0), //  Indicates that subtask is a wander task
		huntGoto    = (1 << 1)  //  Indicates that subtask is a goto task
	};

public:
	//  Constructor -- initial construction
	HuntTask(TaskStack *ts) : Task(ts), huntFlags(0), subTask(nullptr), _subTaskID(NoTask) {
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
	TargetPlaceHolder   targetMem;

protected:
	TilePoint           currentTarget;

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
		return (const Target *)targetMem;
	}
};

/* ===================================================================== *
   HuntToBeNearLocationTask Class
 * ===================================================================== */

class HuntToBeNearLocationTask : public HuntLocationTask {
	uint16              range;

	uint8               targetEvaluateCtr;

	//  static const doesn't work in Visual C++
	enum {
		targetEvaluateRate = 64
	};
//	static const uint8  targetEvaluateRate;

public:
	//  Constructor -- initial construction
	HuntToBeNearLocationTask(TaskStack *ts, const Target &t, uint16 r) :
		HuntLocationTask(ts, t),
		range(r),
		targetEvaluateCtr(0) {
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
		return range;
	}
};

//const uint8 HuntToBeNearLocationTask::targetEvaluateRate = 64;

/* ===================================================================== *
   HuntObjectTask Class
 * ===================================================================== */

class HuntObjectTask : public HuntTask {
	TargetPlaceHolder   targetMem;

protected:
	GameObject          *currentTarget;

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
		return (const ObjectTarget *)targetMem;
	}
};

/* ===================================================================== *
   HuntToBeNearObjectTask Class
 * ===================================================================== */

class HuntToBeNearObjectTask : public HuntObjectTask {
	uint16              range;

	uint8               targetEvaluateCtr;

	enum {
		targetEvaluateRate = 64
	};
//	static const uint8  targetEvaluateRate;

public:
	//  Constructor -- initial construction
	HuntToBeNearObjectTask(
	    TaskStack *ts,
	    const ObjectTarget &ot,
	    uint16 r) :
		HuntObjectTask(ts, ot),
		range(r),
		targetEvaluateCtr(0) {
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
		return range;
	}
};

//const uint8   HuntToBeNearObjectTask::targetEvaluateRate = 64;

/* ===================================================================== *
   HuntToPossessTask Class
 * ===================================================================== */

class HuntToPossessTask : public HuntObjectTask {
	uint8               targetEvaluateCtr;

	enum {
		targetEvaluateRate = 64
	};
//	static const uint8  targetEvaluateRate;

	bool                grabFlag;

public:
	//  Constructor -- initial construction
	HuntToPossessTask(TaskStack *ts, const ObjectTarget &ot) :
		HuntObjectTask(ts, ot),
		targetEvaluateCtr(0),
		grabFlag(false) {
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

//const uint8 HuntToPossessTask::targetEvaluateRate = 16;

/* ===================================================================== *
   HuntActorTask Class
 * ===================================================================== */

class HuntActorTask : public HuntTask {
	TargetPlaceHolder   targetMem;
	uint8               flags;

	enum {
		track   = (1 << 0)
	};

protected:
	Actor               *currentTarget;

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
		return (const ActorTarget *)targetMem;
	}

	bool tracking() const {
		return (flags & track) != 0;
	}
};

/* ===================================================================== *
   HuntToBeNearActorTask Class
 * ===================================================================== */

class HuntToBeNearActorTask : public HuntActorTask {
	GoAwayFromObjectTask    *goAway;    //  The 'go away' sub task pointer
	TaskID _goAwayID;
	uint16                  range;      //  Maximum range

	uint8                   targetEvaluateCtr;

	enum {
		targetEvaluateRate = 16
	};
//	static const uint8  targetEvaluateRate;

public:

	enum {
		tooClose = 12
	};

	//  Constructor -- initial construction
	HuntToBeNearActorTask(
	    TaskStack           *ts,
	    const ActorTarget   &at,
	    uint16              r,
	    bool                trackFlag = false) :
		HuntActorTask(ts, at, trackFlag),
		goAway(NULL),
		_goAwayID(NoTask),
		range(MAX<uint16>(r, 16)),
		targetEvaluateCtr(0) {
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
		return range;
	}
};

//const uint8   HuntToBeNearActorTask::targetEvaluateRate = 64;

/* ===================================================================== *
   HuntToKillTask Class
 * ===================================================================== */

class HuntToKillTask : public HuntActorTask {
	uint8               targetEvaluateCtr;
	uint8               specialAttackCtr;

	enum {
		targetEvaluateRate = 16
	};

	enum {
		currentWeaponBonus = 1
	};

	uint8               flags;

	enum {
		evalWeapon      = (1 << 0)
	};
//	static const uint8  targetEvaluateRate;

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

//const uint8 HuntToKillTask::targetEvaluateRate = 16;

//  Utility function used for combat target selection
inline int16 closenessScore(int16 dist) {
	return 128 / dist;
}

/* ===================================================================== *
   HuntToGiveTask Class
 * ===================================================================== */

class HuntToGiveTask : public HuntActorTask {
	GameObject      *objToGive;

public:
	//  Constructor -- initial construction
	HuntToGiveTask(
	    TaskStack           *ts,
	    const ActorTarget   &at,
	    GameObject          *obj,
	    bool                trackFlag = false) :
		HuntActorTask(ts, at, trackFlag),
		objToGive(obj) {
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
	AttendTask          *attend;
	TaskID _attendID;

	TilePoint           currentTarget;
	uint8               targetEvaluateCtr;

	enum {
		targetEvaluateRate = 2
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
		Actor           *a;

	private:
		Band            *band;
		int             bandIndex;

	public:
		BandingRepulsorIterator(Actor *actor) : a(actor), band(nullptr), bandIndex(0) {}

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
		Actor       *actorArray[6];
		int         numActors,
		            actorIndex;
		bool        iteratingThruEnemies;

	public:
		BandAndAvoidEnemiesRepulsorIterator(Actor *actor) :
				BandingRepulsorIterator(actor), numActors(0), actorIndex(0), iteratingThruEnemies(false) {
			for (int i = 0; i < 6; i++)
				actorArray[i] = 0;
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
		attend(NULL),
		_attendID(NoTask),
		currentTarget(Nowhere),
		targetEvaluateCtr(0) {
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
	        Actor       *actorArray[6];
	        int         numActors,
	                    actorIndex;
	        bool        iteratingThruEnemies;

	    public:
	        BandAndAvoidEnemiesRepulsorIterator( Actor *actor ) :
	            BandingRepulsorIterator( actor )
	        {
	        }

	    private:
	        bool firstEnemyRepulsor(
	            TilePoint   &repulsorVector,
	            int16       &repulsorStrength );

	        bool nextEnemyRepulsor(
	            TilePoint   &repulsorVector,
	            int16       &repulsorStrength );

	    public:
	        bool first(
	            TilePoint   &repulsorVector,
	            int16       &repulsorStrength );

	        bool next(
	            TilePoint   &repulsorVector,
	            int16       &repulsorStrength );
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
	GotoLocationTask        *gotoWayPoint;  //  A goto waypoint sub task
	TaskID _gotoWayPointID;
	//  pointer.
	PatrolRouteIterator     patrolIter;     //  The patrol route iterator.
	int16                   lastWayPointNum;    //  Waypoint at which to end
	//  this task.
	bool                    paused;         //  Flag indicating "paused"ness
	//  of this task
	int16                   counter;        //  Counter for tracking pause
	//  length

public:
	//  Constructor -- initial construction
	FollowPatrolRouteTask(
	    TaskStack           *ts,
	    PatrolRouteIterator iter,
	    int16               stopAt = -1) :
		Task(ts),
		gotoWayPoint(NULL),
		_gotoWayPointID(NoTask),
		patrolIter(iter),
		lastWayPointNum(stopAt), counter(0) {
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
		paused = false;
	}
};

/* ===================================================================== *
   AttendTask Class
 * ===================================================================== */

class AttendTask : public Task {
	GameObject  *obj;

public:
	//  Constructor -- initial construction
	AttendTask(TaskStack *ts, GameObject *o) : Task(ts), obj(o) {
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
	Actor       *attacker;

	Task        *subTask;

public:
	//  Constructor -- initial construction
	DefendTask(TaskStack *ts, Actor *a) :
		Task(ts),
		attacker(a),
		subTask(NULL) {
	}

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
	Actor               *attacker;
	GameObject          *defenseObj;

	uint8               flags;

	enum {
		motionStarted   = (1 << 0),
		blockStarted    = (1 << 1)
	};

public:
	//  Constructor -- initial construction
	ParryTask(TaskStack *ts, Actor *a, GameObject *obj) :
		Task(ts),
		attacker(a),
		defenseObj(obj),
		flags(0) {
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

	TaskID          stackBottomID;  //  Bottom task in stack

	int16           evalCount,  //  Counter for automatic task re-evaluation
	                evalRate;   //  Rate of automatic task re-evalutation
public:
	Actor           *actor;     //  Pointer to actor performing tasks

	//  Constructor
	TaskStack() :
		stackBottomID(0),
		evalCount(0),
		evalRate(0),
		actor(nullptr) {}

	TaskStack(Actor *a) :
		stackBottomID(NoTask),
		actor(a),
		evalCount(defaultEvalRate),
		evalRate(defaultEvalRate) {

		newTaskStack(this);
	}

	//  Destructor
	~TaskStack() {
		if (actor)
			actor->_curTask = nullptr;
		deleteTaskStack(this);
	}

	//  Return the number of bytes necessary to archive this TaskStack
	//  in a buffer
	int32 archiveSize() {
		return      sizeof(ObjectID)     //  actor's id
		            +   sizeof(stackBottomID)
		            +   sizeof(evalCount)
		            +   sizeof(evalRate);
	}

	void write(Common::MemoryWriteStreamDynamic *out);

	void read(Common::InSaveFile *in);

	//  Set the bottom task of this task stack
	void setTask(Task *t);

	//  Return a pointer to the bottom task in this task stack
	const Task *getTask() {
		return  stackBottomID != NoTask
		        ?   getTaskAddress(stackBottomID)
		        :   NULL;
	}

	Actor *getActor() {
		return actor;
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
