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
void updateActorTasks(void);

void pauseActorTasks(void);
void resumeActorTasks(void);

//  Allocate a new task stack
void newTaskStack(TaskStack *p);

//  Dispose of a previously allocated task stack
void deleteTaskStack(TaskStack *p);

//  Return the ID number of a specified task stack
TaskStackID getTaskStackID(TaskStack *ts);

//  Return a pointer to a TaskStack given a TaskStackID
TaskStack *getTaskStackAddress(TaskStackID id);

//  Initialize the task stack list
void initTaskStacks(void);

//  Save the task stack list to a save file
void saveTaskStacks(SaveFileConstructor &saveGame);

//  Load the task stack list from a save file
void loadTaskStacks(SaveFileReader &saveGame);

//  Cleanup the task stacks
void cleanupTaskStacks(void);


void newTask(Task *t);
void newTask(Task *t, TaskID id);

//  Dispose of a previously allocated task
void deleteTask(Task *p);

//  Return a task's ID number
TaskID getTaskID(Task *t);

//  Return a pointer to a Task given a TaskID
Task *getTaskAddress(TaskID id);

//  Initialize the task list
void initTasks(void);

//  Save the task list to a save file
void saveTasks(SaveFileConstructor &saveGame);

//  Load the task list from a save file
void loadTasks(SaveFileReader &saveGame);

//  Cleanup the task list
void cleanupTasks(void);

/* ===================================================================== *
   Task Class
 * ===================================================================== */

//  This is the base class for all Task classes
class Task {
	friend class TaskStack;

protected:
	//  A pointer to this task's stack
	TaskStack   *stack;

public:
	Common::String _type;

	//  Constructor -- initial construction
	Task(TaskStack *ts) : stack(ts) {
		newTask(this);
	}

	Task(TaskStack *ts, TaskID id) : stack(ts) {
		newTask(this, id);
	}

	//  Constructor -- reconstruct from archive buffer
	Task(void **buf, TaskID id);

	//  Virtual destructor -- do nothing
	virtual ~Task(void) {
		deleteTask(this);
	}

	//  Fixup any subtask pointers
	virtual void fixup(void);

	//  Return the number of bytes necessary to archive this Task
	//  in a buffer
	virtual int32 archiveSize(void) const;

	//  Create an archive of this task in a buffer
	virtual void *archive(void *buf) const;

	//  Return an integer representing the type of this task
	virtual int16 getType(void) const = 0;

	virtual void abortTask(void) = 0;
	virtual TaskResult evaluate(void) = 0;
	virtual TaskResult update(void) = 0;

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

	//  Constructor -- reconstruct from archive buffer
	WanderTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	void abortTask();
	TaskResult evaluate(void);
	TaskResult update(void);

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

protected:
	//  Update function used while task is not paused
	virtual TaskResult handleWander(void);

	//  Update function used while task is paused
	TaskResult handlePaused(void) {
		return taskNotDone;
	}

	//  Set this task into the paused state
	void pause(void);

	//  Set this task into the wander state
	void wander(void);
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
		gotoTether(NULL) {
		debugC(2, kDebugTasks, " - TetheredWanderTask");
		_type = "TetheredWanderTask";
	}

	//  Constructor -- reconstruct from archive buffer
	TetheredWanderTask(void **buf, TaskID id);

	//  Fixup the subtask pointers
	void fixup(void);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

#if DEBUG
	//  Debugging function used to mark this task and any sub tasks as
	//  being used.  This is used to find task leaks.
	void mark(void);
#endif

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	void abortTask(void);

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

	//  Update function used while task is not paused
	TaskResult handleWander(void);
};

/* ===================================================================== *
   GotoTask Class
 * ===================================================================== */

class GotoTask : public Task {
	WanderTask  *wander;
	bool        prevRunState;

public:
	//  Constructor -- initial construction
	GotoTask(TaskStack *ts) :
		Task(ts),
		wander(NULL),
		prevRunState(false) {
		debugC(2, kDebugTasks, " - GotoTask");
		_type = "GotoTask";
	}

	//  Constructor -- reconstruct from archive buffer
	GotoTask(void **buf, TaskID id);

	//  Fixup the subtask pointer
	void fixup(void);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

#if DEBUG
	//  Debugging function used to mark this task and any sub tasks as
	//  being used.  This is used to find task leaks.
	void mark(void);
#endif

	void abortTask(void);
	TaskResult evaluate(void);
	TaskResult update(void);

private:
	virtual TilePoint destination(void) = 0;
	virtual TilePoint intermediateDest(void) = 0;
	virtual bool lineOfSight(void) = 0;
	virtual bool run(void) = 0;
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

	//  Constructor -- reconstruct from archive buffer
	GotoLocationTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

	const TilePoint getTarget(void) const {
		return targetLoc;
	}

	void changeTarget(const TilePoint &newTarget) {
		targetLoc = newTarget;
	}

private:
	TilePoint destination(void);
	TilePoint intermediateDest(void);
	bool lineOfSight(void);
	bool run(void);
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

	//  Constructor -- reconstruct from archive buffer
	GotoRegionTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

private:
	TilePoint destination(void);
	TilePoint intermediateDest(void);
	bool lineOfSight(void);
	bool run(void);
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

	//  Constructor -- reconstruct from archive buffer
	GotoObjectTargetTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

private:
	TilePoint destination(void);
	TilePoint intermediateDest(void);
	bool lineOfSight(void);

	virtual GameObject *getObject(void) = 0;

protected:
	bool tracking(void) const {
		return (flags & track) != 0;
	}
	bool isInSight(void) const {
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

	//  Constructor -- reconstruct from archive buffer
	GotoObjectTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

	const GameObject *getTarget(void) const {
		return targetObj;
	}

private:
	bool run(void);
	GameObject *getObject(void);
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
	//  Constructor -- reconstruct from archive buffer
	GotoActorTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

	const Actor *getTarget(void) const {
		return targetActor;
	}

private:
	bool run(void);
	GameObject *getObject(void);
};

/* ===================================================================== *
   GoAwayFromTask Class
 * ===================================================================== */

class GoAwayFromTask : public Task {
	GotoLocationTask        *goTask;

	uint8                   flags;

	enum {
		run = (1 << 0)
	};

public:
	//  Constructor -- initial construction
	GoAwayFromTask(TaskStack *ts) :
		Task(ts),
		goTask(NULL),
		flags(0) {
		debugC(2, kDebugTasks, " - GoAwayFromTask1");
		_type = "GoAwayFromTask";
	}

	GoAwayFromTask(TaskStack *ts, bool runFlag) :
		Task(ts),
		goTask(NULL),
		flags(runFlag ? run : 0) {
		debugC(2, kDebugTasks, " - GoAwayFromTask2");
		_type = "GoAwayFromTask";
	}

	//  Constructor -- reconstruct from archive buffer
	GoAwayFromTask(void **buf, TaskID id);

	//  Fixup the subtask pointer
	void fixup(void);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

#if DEBUG
	//  Debugging function used to mark this task and any sub tasks as
	//  being used.  This is used to find task leaks.
	void mark(void);
#endif

	void abortTask(void);
	TaskResult evaluate(void);
	TaskResult update(void);

private:
	virtual TilePoint getRepulsionVector(void) = 0;
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

	//  Constructor -- reconstruct from archive buffer
	GoAwayFromObjectTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

private:
	TilePoint getRepulsionVector(void);
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

	//  Constructor -- reconstruct from archive buffer
	GoAwayFromActorTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

private:
	TilePoint getRepulsionVector(void);

	const ActorTarget *getTarget(void) const {
		return (const ActorTarget *)targetMem;
	}
};

/* ===================================================================== *
   HuntTask Class
 * ===================================================================== */

class HuntTask : public Task {
	Task            *subTask;   //  This will either be a wander task of a
	//  goto task
	uint8           huntFlags;

	enum HuntFlags {
		huntWander  = (1 << 0), //  Indicates that subtask is a wander task
		huntGoto    = (1 << 1)  //  Indicates that subtask is a goto task
	};

public:
	//  Constructor -- initial construction
	HuntTask(TaskStack *ts) :
		Task(ts),
		huntFlags(0) {
		debugC(2, kDebugTasks, " - HuntTask");
		_type = "HuntTask";
	}

	//  Constructor -- reconstruct from archive buffer
	HuntTask(void **buf, TaskID id);

	//  Fixup the subtask pointer
	void fixup(void);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

#if DEBUG
	//  Debugging function used to mark this task and any sub tasks as
	//  being used.  This is used to find task leaks.
	void mark(void);
#endif

	void abortTask(void);
	TaskResult evaluate(void);
	TaskResult update(void);

private:
	void removeWanderTask(void);
	void removeGotoTask(void);

protected:
	virtual void evaluateTarget(void) = 0;

	virtual bool targetHasChanged(GotoTask *gotoTarget) = 0;
	virtual GotoTask *setupGoto(void) = 0;
	virtual TilePoint currentTargetLoc(void) = 0;

	virtual bool atTarget(void) = 0;
	virtual void atTargetabortTask(void) = 0;
	virtual TaskResult atTargetEvaluate(void) = 0;
	virtual TaskResult atTargetUpdate(void) = 0;
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

	//  Constructor -- reconstruct from archive buffer
	HuntLocationTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

protected:
	bool targetHasChanged(GotoTask *gotoTarget);
	GotoTask *setupGoto(void);
	TilePoint currentTargetLoc(void);

	const Target *getTarget(void) const {
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

	//  Constructor -- reconstruct from archive buffer
	HuntToBeNearLocationTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

protected:
	void evaluateTarget(void);

	bool atTarget(void);

	void atTargetabortTask(void);
	TaskResult atTargetEvaluate(void);
	TaskResult atTargetUpdate(void);

	uint16 getRange(void) const {
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

	//  Constructor -- reconstruct from archive buffer
	HuntObjectTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

protected:
	bool targetHasChanged(GotoTask *gotoTarget);
	GotoTask *setupGoto(void);
	TilePoint currentTargetLoc(void);

	const ObjectTarget *getTarget(void) const {
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

	//  Constructor -- reconstruct from archive buffer
	HuntToBeNearObjectTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

protected:
	void evaluateTarget(void);

	bool atTarget(void);

	void atTargetabortTask(void);
	TaskResult atTargetEvaluate(void);
	TaskResult atTargetUpdate(void);

	uint16 getRange(void) const {
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

	//  Constructor -- reconstruct from archive buffer
	HuntToPossessTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

protected:
	void evaluateTarget(void);
	bool atTarget(void);

	void atTargetabortTask(void);
	TaskResult atTargetEvaluate(void);
	TaskResult atTargetUpdate(void);
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

	//  Constructor -- reconstruct from archive buffer
	HuntActorTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

protected:
	bool targetHasChanged(GotoTask *gotoTarget);
	GotoTask *setupGoto(void);
	TilePoint currentTargetLoc(void);

	const ActorTarget *getTarget(void) const {
		return (const ActorTarget *)targetMem;
	}

	bool tracking(void) const {
		return (flags & track) != 0;
	}
};

/* ===================================================================== *
   HuntToBeNearActorTask Class
 * ===================================================================== */

class HuntToBeNearActorTask : public HuntActorTask {
	GoAwayFromObjectTask    *goAway;    //  The 'go away' sub task pointer
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
		range(MAX<uint16>(r, 16)),
		targetEvaluateCtr(0) {
		debugC(2, kDebugTasks, " - HuntToBeNearActorTask");
		_type = "HuntToBeNearActorTask";
	}

	//  Constructor -- reconstruct from archive buffer
	HuntToBeNearActorTask(void **buf, TaskID id);

	//  Fixup the subtask pointer
	void fixup(void);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

#if DEBUG
	//  Debugging function used to mark this task and any sub tasks as
	//  being used.  This is used to find task leaks.
	void mark(void);
#endif

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

protected:
	void evaluateTarget(void);

	bool atTarget(void);

	void atTargetabortTask(void);
	TaskResult atTargetEvaluate(void);
	TaskResult atTargetUpdate(void);

	uint16 getRange(void) const {
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

	//  Constructor -- reconstruct from archive buffer
	HuntToKillTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

	void abortTask(void);
	TaskResult update(void);

protected:
	void evaluateTarget(void);
	bool atTarget(void);

	void atTargetabortTask(void);
	TaskResult atTargetEvaluate(void);
	TaskResult atTargetUpdate(void);

private:
	void evaluateWeapon(void);
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

	//  Constructor -- reconstruct from archive buffer
	HuntToGiveTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

protected:
	void evaluateTarget(void);
	bool atTarget(void);

	void atTargetabortTask(void);
	TaskResult atTargetEvaluate(void);
	TaskResult atTargetUpdate(void);
};

/* ===================================================================== *
   BandTask Class
 * ===================================================================== */

class AttendTask;

class BandTask : public HuntTask {
	AttendTask          *attend;

	TilePoint           currentTarget;
	uint8               targetEvaluateCtr;

	enum {
		targetEvaluateRate = 2
	};

public:

	class RepulsorIterator {
	public:
		virtual ~RepulsorIterator(void) {}

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
		BandingRepulsorIterator(Actor *actor) : a(actor) {}

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
			BandingRepulsorIterator(actor) {
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
		currentTarget(Nowhere),
		targetEvaluateCtr(0) {
		debugC(2, kDebugTasks, " - BandTask");
		_type = "BandTask";
	}

	//  Constructor -- reconstruct from archive buffer
	BandTask(void **buf, TaskID id);

	//  Fixup the subtask pointer
	void fixup(void);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

#if DEBUG
	//  Debugging function used to mark this task and any sub tasks as
	//  being used.  This is used to find task leaks.
	void mark(void);
#endif

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

protected:
	void evaluateTarget(void);

	bool targetHasChanged(GotoTask *gotoTarget);
	GotoTask *setupGoto(void);
	TilePoint currentTargetLoc(void);

	bool atTarget(void);

	void atTargetabortTask(void);
	TaskResult atTargetEvaluate(void);
	TaskResult atTargetUpdate(void);

	virtual int16 getRunThreshold(void);
	virtual RepulsorIterator *getNewRepulsorIterator(void);
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

	//  Constructor -- reconstruct from archive buffer
	BandAndAvoidEnemiesTask(void **buf, TaskID id) : BandTask(buf, id) {}

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

protected:
	int16 getRunThreshold(void);
	RepulsorIterator *getNewRepulsorIterator(void);
};

/* ===================================================================== *
   FollowPatrolRouteTask Class
 * ===================================================================== */

class FollowPatrolRouteTask : public Task {
	GotoLocationTask        *gotoWayPoint;  //  A goto waypoint sub task
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
		patrolIter(iter),
		lastWayPointNum(stopAt) {
		debugC(2, kDebugTasks, " - FollowPatrolRouteTask");
		_type = "FollowPatrolRouteTask";
		followPatrolRoute();
	}

	//  Constructor -- reconstruct from archive buffer
	FollowPatrolRouteTask(void **buf, TaskID id);

	//  Fixup the subtask pointer
	void fixup(void);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

#if DEBUG
	//  Debugging function used to mark this task and any sub tasks as
	//  being used.  This is used to find task leaks.
	void mark(void);
#endif

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	void abortTask(void);
	TaskResult evaluate(void);
	TaskResult update(void);

	//  Determine if the specified task is equivalent to this task
	bool operator == (const Task &t) const;

	//  Update function used if this task is not paused
	TaskResult handleFollowPatrolRoute(void);

	//  Update function used if this task is paused
	TaskResult handlePaused(void);

	//  Set this task into the paused state
	void pause(void);

	//  Set this task into the unpaused state
	void followPatrolRoute(void) {
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

	//  Constructor -- reconstruct from archive buffer
	AttendTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	void abortTask(void);
	TaskResult evaluate(void);
	TaskResult update(void);

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

	//  Constructor -- reconstruct from archive buffer
	DefendTask(void **buf, TaskID id);

	//  Fixup the subtask pointer
	void fixup(void);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	void abortTask(void);
	TaskResult evaluate(void);
	TaskResult update(void);

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

	//  Constructor -- reconstruct from archive buffer
	ParryTask(void **buf, TaskID id);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void) const;

	//  Create an archive of this object in a buffer
	void *archive(void *buf) const;

	//  Return an integer representing the type of this task
	int16 getType(void) const;

	void abortTask(void);
	TaskResult evaluate(void);
	TaskResult update(void);

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

	Actor           *actor;     //  Pointer to actor performing tasks
	int16           evalCount,  //  Counter for automatic task re-evaluation
	                evalRate;   //  Rate of automatic task re-evalutation
public:
	//  Constructor
	TaskStack(Actor *a) :
		stackBottomID(NoTask),
		actor(a),
		evalCount(defaultEvalRate),
		evalRate(defaultEvalRate) {

		newTaskStack(this);
	}

	//  Constructor -- reconstruct from archive buffer
	TaskStack(void **buf);

	//  Destructor
	~TaskStack(void) {
		deleteTaskStack(this);
	}

	//  Return the number of bytes necessary to archive this TaskStack
	//  in a buffer
	int32 archiveSize(void) {
		return      sizeof(ObjectID)     //  actor's id
		            +   sizeof(stackBottomID)
		            +   sizeof(evalCount)
		            +   sizeof(evalRate);
	}

	//  Create an archive of this TaskStack in a buffer
	void *archive(void *buf);

	//  Set the bottom task of this task stack
	void setTask(Task *t);

	//  Return a pointer to the bottom task in this task stack
	const Task *getTask(void) {
		return  stackBottomID != NoTask
		        ?   getTaskAddress(stackBottomID)
		        :   NULL;
	}

	Actor *getActor(void) {
		return actor;
	}

	//  Abort all tasks in stack
	void abortTask(void);
	//  Re-evaluate tasks in stack
	TaskResult evaluate(void);
	//  Update the state of the tasks in stack
	TaskResult update(void);
};

} // end of namespace Saga2

#endif
