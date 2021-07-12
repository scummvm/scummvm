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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/fta.h"
#include "saga2/cmisc.h"
#include "saga2/actor.h"
#include "saga2/task.h"
#include "saga2/motion.h"
#include "saga2/band.h"
#include "saga2/sensor.h"
#include "saga2/tilemode.h"
#include "saga2/tile.h"

namespace Saga2 {

bool actorTasksPaused;

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

void readTask(TaskID id, Common::InSaveFile *in);

//  Return the number of bytes necessary to create an archive of the
//  specified Task
int32 taskArchiveSize(Task *t);

void writeTask(Task *t, Common::OutSaveFile *out);

#if DEBUG
//  Debugging function used to check the integrity of the global task
//  list
void checkTaskListIntegrity(void);
#endif

/* ===================================================================== *
   Utility functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Compute a repulsion vector based on an array of repulsor vectors

TilePoint computeRepulsionVector(
    TilePoint   *repulsorVectorArray,
    int16       *repulsorStrengthArray,
    int         numRepulsors) {
	int             i;
	TilePoint       repulsionVector(0, 0, 0);

	for (i = 0; i < numRepulsors; i++) {
		int16       repulsorWeight,
		            repulsorDist;

		repulsorDist =      repulsorVectorArray[i].quickHDistance()
		                    +   ABS(repulsorVectorArray[i].z);
		repulsorWeight =
		    repulsorDist != 0
		    ?   64 * 64 / (repulsorDist * repulsorDist)
		    :   64 * 64;

		repulsionVector +=
		    (-repulsorVectorArray[i]
		     *   repulsorStrengthArray[i]
		     *   repulsorWeight)
		    /   16;
	}

	return repulsionVector;
}

/* ===================================================================== *
   TaskStackList class
 * ===================================================================== */

const int       numTaskStacks = 32;

//  Manages the memory used for the TaskStack's.  There will
//  only be one global instantiation of this class
class TaskStackList {
	TaskStack *_list[numTaskStacks];

public:
	//  Constructor -- initial construction
	TaskStackList(void);

	//  Destructor
	~TaskStackList(void);

	void read(Common::InSaveFile *in);

	//  Return the number of bytes needed to make an archive of the
	//  TaskStackList
	int32 archiveSize(void);

	void write(Common::OutSaveFile *out);

	//  Place a TaskStack from the inactive list into the active
	//  list.
	void newTaskStack(TaskStack *p);

	void newTaskStack(TaskStack *p, TaskID id);

	//  Place a TaskStack back into the inactive list.
	void deleteTaskStack(TaskStack *p);

	//  Return the specified TaskStack's ID
	TaskStackID getTaskStackID(TaskStack *ts) {
		for (int i = 0; i < numTaskStacks; i++)
			if (_list[i] == ts)
				return i;

		error("getTaskStackID(): Unknown stack %p", (void *)ts);
	}

	//  Return a pointer to a TaskStack given a TaskStackID
	TaskStack *getTaskStackAddress(TaskStackID id) {
		assert(id >= 0 && id < numTaskStacks);
		return _list[id];
	}

	//  Run through the TaskStacks in the active list and update
	//  each.
	void updateTaskStacks(void);
};

//----------------------------------------------------------------------
//	TaskStackList constructor -- simply place each element the array in
//	the inactive list

TaskStackList::TaskStackList(void) {
	for (int i = 0; i < numTaskStacks; i++)
		_list[i] = nullptr;
}

//----------------------------------------------------------------------
//	TaskStackList destructor

TaskStackList::~TaskStackList(void) {
	for (int i = 0; i < numTaskStacks; i++) {
		if (_list[i] == nullptr)
			continue;

		_list[i]->actor->curTask = nullptr;
		delete _list[i];
		_list[i] = nullptr;
	}
}

void TaskStackList::read(Common::InSaveFile *in) {
	int16 taskStackCount;

	//  Get the count of task stacks and increment the buffer pointer
	taskStackCount = in->readSint16LE();
	debugC(3, kDebugSaveload, "... taskStackCount = %d", taskStackCount);

	//  Iterate through the archive data, reconstructing the TaskStacks
	for (int i = 0; i < taskStackCount; i++) {
		TaskStackID id;
		TaskStack *ts;

		//  Retreive the TaskStack's id number
		id = in->readSint16LE();
		debugC(3, kDebugSaveload, "Loading Task Stack %d", id);

		ts = new TaskStack;
		newTaskStack(ts, id);

		ts->read(in);

		//  Plug this TaskStack into the Actor
		ts->getActor()->curTask = ts;
	}
}

//----------------------------------------------------------------------
//	Return the number of bytes necessary to archive this TaskStackList

int32 TaskStackList::archiveSize(void) {
	int32 size = sizeof(int16);

	for (int i = 0; i < numTaskStacks; i++) {
		size += sizeof(TaskStackID);

		if (_list[i])
			size +=  _list[i]->archiveSize();
	}

	return size;
}

void TaskStackList::write(Common::OutSaveFile *out) {
	int16 taskStackCount = 0;

	//  Count the active task stacks
	for (int i = 0; i < numTaskStacks; i++)
		if (_list[i])
			taskStackCount++;

	//  Store the task stack count in the archive buffer
	out->writeSint16LE(taskStackCount);
	debugC(3, kDebugSaveload, "... taskStackCount = %d", taskStackCount);

	for (int i = 0; i < numTaskStacks; i++) {
		if (_list[i] == nullptr)
			continue;

		debugC(3, kDebugSaveload, "Saving Task Stack %d", i);

		TaskStack *ts = _list[i];
		out->writeSint16LE(i);
		ts->write(out);
	}
}

//----------------------------------------------------------------------
//	Place a TaskStack into the active list and return its address

void TaskStackList::newTaskStack(TaskStack *p) {
	for (int i = 0; i < numTaskStacks; i++)
		if (!_list[i]) {
			_list[i] = p;

			return;
		}
}

void TaskStackList::newTaskStack(TaskStack *p, TaskID id) {
	if (_list[id])
		error("Task already exists");
	_list[id] = p;
}

//----------------------------------------------------------------------
//	Remove the specified TaskStack from the active list and place it
//	back into the inactive list

void TaskStackList::deleteTaskStack(TaskStack *p) {
	for (int i = 0; i < numTaskStacks; i++)
		if (_list[i] == p) {
			_list[i] = nullptr;
		}
}

//----------------------------------------------------------------------
//	Iterate through all of the TaskStacks in the active list and call
//	their update function

void TaskStackList::updateTaskStacks(void) {
	for (int i = 0; i < numTaskStacks; i++) {
		if (_list[i]) {
			TaskStack *ts = _list[i];
			TaskResult  result;

			//  Update the task stack and delete it if it is done
			if ((result = ts->update()) != taskNotDone) {
				Actor *a = ts->getActor();
				assert(a != NULL);

				a->handleTaskCompletion(result);
			}
		}
	}
}

/* ===================================================================== *
   Global TaskStackList instantiation
 * ===================================================================== */

//	This is a statically allocated buffer large enough to hold a
//	TaskStackList.  The stackList is a TaskStackList reference to this
//	area of memory.  The reason that I did this in this manner is to
//	prevent the TaskStackList constructor from being called until it is
//	expicitly called using an overloaded new call.  The overloaded new
//	call will simply return a pointer to the stackListBuffer in order
//	to construct the TaskStackList in place.

static TaskStackList stackList;

/* ===================================================================== *
   Misc. task stack management functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Simply pass this call to the stackList member function,
//	updateTaskStacks().

void updateActorTasks(void) {
	if (!actorTasksPaused)
		stackList.updateTaskStacks();
}

void pauseActorTasks(void) {
	actorTasksPaused = true;
}
void resumeActorTasks(void) {
	actorTasksPaused = false;
}

//----------------------------------------------------------------------
//	Call the stackList member function newTaskStack() to get a pointer
//	to a new TaskStack

void newTaskStack(TaskStack *p) {
	return stackList.newTaskStack(p);
}

//----------------------------------------------------------------------
//	Call the stackList member function deleteTaskStack() to dispose of
//	a previously allocated TaskStack

void deleteTaskStack(TaskStack *p) {
	stackList.deleteTaskStack(p);
}

//----------------------------------------------------------------------
//	Return the specified TaskStack's ID

TaskStackID getTaskStackID(TaskStack *ts) {
	return stackList.getTaskStackID(ts);
}

//----------------------------------------------------------------------
//	Return a pointer to a TaskStack given a TaskStackID

TaskStack *getTaskStackAddress(TaskStackID id) {
	return stackList.getTaskStackAddress(id);
}

//----------------------------------------------------------------------
//	Initialize the stackList

void initTaskStacks(void) {
}

void saveTaskStacks(Common::OutSaveFile *out) {
	debugC(2, kDebugSaveload, "Saving Task Stacks");

	int32 archiveBufSize;

	archiveBufSize = stackList.archiveSize();

	out->write("TSTK", 4);
	out->writeUint32LE(archiveBufSize);

	stackList.write(out);
}

void loadTaskStacks(Common::InSaveFile *in, int32 chunkSize) {
	debugC(2, kDebugSaveload, "Loading Task Stacks");

	//  If there is no saved data, simply call the default constructor
	if (chunkSize == 0) {
		new (&stackList) TaskStackList;
		return;
	}

	//  Reconstruct stackList from archived data
	new (&stackList) TaskStackList;
	stackList.read(in);
}

//----------------------------------------------------------------------
//	Cleanup the stackList

void cleanupTaskStacks(void) {
	//  Simply call stackList's destructor
	stackList.~TaskStackList();
}

/* ===================================================================== *
   TaskList class
 * ===================================================================== */

const int       numTasks = 64;

//  Manages the memory used for the Task's.  There will only be one
//  global instantiation of this class
class TaskList {

	int _size;
	Task *_list[numTasks];

public:
	//  Constructor -- initial construction
	TaskList(void);

	//  Destructor
	~TaskList(void);

	void read(Common::InSaveFile *in);

	//  Return the number of bytes necessary to archive this task list
	//  in a buffer
	int32 archiveSize(void);

	void write(Common::OutSaveFile *out);

	//  Place a Task from the inactive list into the active
	//  list.
	void newTask(Task *t);
	void newTask(Task *t, TaskID id);

	//  Place a Task back into the inactive list.
	void deleteTask(Task *t);

	//  Return the specified Task's ID
	TaskID getTaskID(Task *t) {
		for (int i = 0; i < numTasks; i++)
			if (_list[i] == t)
				return i;

		error("getTaskID: unknown task %p", (void *)t);
	}

	//  Return a pointer to a Task given a TaskID
	Task *getTaskAddress(TaskID id) {
		assert(id >= 0 && id < numTasks);
		return _list[id];
	}
};

//----------------------------------------------------------------------
//	TaskList constructor -- simply place each element of the array in
//	the inactive list

TaskList::TaskList(void) {
	_size = 0;
	for (int i = 0; i < numTasks; i++)
		_list[i] = nullptr;
}

//----------------------------------------------------------------------
//	TaskList destructor

TaskList::~TaskList(void) {
	for (int i = 0; i < numTasks; i++) {
		if (_list[i] == nullptr)
			continue;

		delete _list[i];
		_list[i] = nullptr;
	}
}

void TaskList::read(Common::InSaveFile *in) {
	int16 taskCount;

	//  Get the count of tasks and increment the buffer pointer
	taskCount = in->readSint16LE();
	debugC(3, kDebugSaveload, "... taskCount = %d", taskCount);

	//  Iterate through the archive data, reconstructing the Tasks
	for (int i = 0; i < taskCount; i++) {
		TaskID id;

		//  Retreive the Task's id number
		id = in->readSint16LE();
		debugC(3, kDebugSaveload, "Loading Task %d", i);

		readTask(id, in);
	}
}

//----------------------------------------------------------------------
//	Return the number of bytes necessary to archive this TaskList

int32 TaskList::archiveSize(void) {
	int32 size = sizeof(int16);

	for (int i = 0; i < numTasks; i++) {
		size += sizeof(TaskID);

		if (_list[i])
			size += taskArchiveSize(_list[i]);
	}

	return size;
}

void TaskList::write(Common::OutSaveFile *out) {
	int16 taskCount = 0;

	//  Count the active tasks
	for (int i = 0 ; i < numTasks; ++i)
		if (_list[i])
			taskCount++;

	//  Store the task count in the archive buffer
	out->writeSint16LE(taskCount);
	debugC(3, kDebugSaveload, "... taskCount = %d", taskCount);

	for (int i = 0; i < numTasks; ++i) {
		if (_list[i] == nullptr)
			continue;

		debugC(3, kDebugSaveload, "Saving Task %d", i);

		out->writeSint16LE(i);
		writeTask(_list[i], out);
	}
}

void TaskList::newTask(Task *t) {
	debugC(1, kDebugTasks, "List: %p Adding task %p (total %d)", (void *)this, (void *)t, ++_size);
	for (int i = 0; i < numTasks; i++)
		if (!_list[i]) {
			_list[i] = t;

			return;
		}

	for (int i = 0; i < numTasks; i++)
		debug("%d: %p (%s)", i, (void *)_list[i], _list[i]->_type.c_str());
	error("Too many tasks in the list, > %d", numTasks);
}

void TaskList::newTask(Task *t, TaskID id) {
	if (_list[id])
		error("Task already exists");
	_list[id] = t;
}

//----------------------------------------------------------------------
//	Remove the specified Task from the active list and place it back
//	into the inactive list

void TaskList::deleteTask(Task *p) {
	debugC(1, kDebugTasks, "List: %p Deleting task %p (%s) (total %d)", (void *)this, (void *)p, p->_type.c_str(), --_size);
	for (int i = 0; i < numTasks; i++)
		if (_list[i] == p) {
			_list[i] = nullptr;
		}
}

/* ===================================================================== *
   Global TaskList instantiation
 * ===================================================================== */

static TaskList taskList;

/* ===================================================================== *
   Misc. task management functions
 * ===================================================================== */

void newTask(Task *t) {
	return taskList.newTask(t);
}

void newTask(Task *t, TaskID id) {
	return taskList.newTask(t, id);
}

//----------------------------------------------------------------------
//	Call the taskList member function deleteTask() to dispose of a
//	previously allocated TaskStack

void deleteTask(Task *p) {
	taskList.deleteTask(p);
}

//----------------------------------------------------------------------
//	Return the specified Task's ID

TaskID getTaskID(Task *t) {
	return taskList.getTaskID(t);
}

//----------------------------------------------------------------------
//	Return a pointer to a Task given a TaskID

Task *getTaskAddress(TaskID id) {
	return taskList.getTaskAddress(id);
}

//----------------------------------------------------------------------
//	Initialize the taskList

void initTasks(void) {
	//  Simply call the default constructor for the task list
	new (&taskList) TaskList;
}

void saveTasks(Common::OutSaveFile *out) {
	debugC(2, kDebugSaveload, "Saving Tasks");

	int32 archiveBufSize;

	archiveBufSize = taskList.archiveSize();

	out->write("TASK", 4);
	out->writeUint32LE(archiveBufSize);

	taskList.write(out);
}

void loadTasks(Common::InSaveFile *in, int32 chunkSize) {
	debugC(2, kDebugSaveload, "Loading Tasks");

	//  If there is no saved data, simply call the default constructor
	if (chunkSize == 0) {
		return;
	}

	//  Reconstruct taskList from archived data
	taskList.read(in);
}

//----------------------------------------------------------------------
//	Cleanup the taskList

void cleanupTasks(void) {
	//  Simply call the taskList's destructor
	taskList.~TaskList();
}

void readTask(TaskID id, Common::InSaveFile *in) {
	int16 type;

	//  Get the Task type
	type = in->readSint16LE();

	//  Reconstruct the Task based upon the type
	switch (type) {
	case wanderTask:
		new WanderTask(in, id);
		break;

	case tetheredWanderTask:
		new TetheredWanderTask(in, id);
		break;

	case gotoLocationTask:
		new GotoLocationTask(in, id);
		break;

	case gotoRegionTask:
		new GotoRegionTask(in, id);
		break;

	case gotoObjectTask:
		new GotoObjectTask(in, id);
		break;

	case gotoActorTask:
		new GotoActorTask(in, id);
		break;

	case goAwayFromObjectTask:
		new GoAwayFromObjectTask(in, id);
		break;

	case goAwayFromActorTask:
		new GoAwayFromActorTask(in, id);
		break;

	case huntToBeNearLocationTask:
		new HuntToBeNearLocationTask(in, id);
		break;

	case huntToBeNearObjectTask:
		new HuntToBeNearObjectTask(in, id);
		break;

	case huntToPossessTask:
		new HuntToPossessTask(in, id);
		break;

	case huntToBeNearActorTask:
		new HuntToBeNearActorTask(in, id);
		break;

	case huntToKillTask:
		new HuntToKillTask(in, id);
		break;

	case huntToGiveTask:
		new HuntToGiveTask(in, id);
		break;

	case bandTask:
		new BandTask(in, id);
		break;

	case bandAndAvoidEnemiesTask:
		new BandAndAvoidEnemiesTask(in, id);
		break;

	case followPatrolRouteTask:
		new FollowPatrolRouteTask(in, id);
		break;

	case attendTask:
		new AttendTask(in, id);
		break;
	}
}

//----------------------------------------------------------------------
//	Return the number of bytes necessary to create an archive of the
//	specified Task

int32 taskArchiveSize(Task *t) {
	return      sizeof(int16)        //  Task type
	            +   t->archiveSize();
}

void writeTask(Task *t, Common::OutSaveFile *out) {
	//  Store the task's type
	out->writeSint16LE(t->getType());

	//  Store the task
	t->write(out);
}

/* ===================================================================== *
   Task member functions
 * ===================================================================== */

Task::Task(Common::InSaveFile *in, TaskID id) {
	//  Place the stack ID into the stack pointer field
	int16 stackID = in->readSint16LE();
	stack = getTaskStackAddress(stackID);
	newTask(this, id);
}

//----------------------------------------------------------------------
//	Return the number of bytes necessary to create an archive of this
//	object's data

inline int32 Task::archiveSize(void) const {
	return sizeof(TaskStackID);      //  stack's ID
}

void Task::write(Common::OutSaveFile *out) const {
	out->writeSint16LE(getTaskStackID(stack));
}

/* ===================================================================== *
   WanderTask member functions
 * ===================================================================== */

WanderTask::WanderTask(Common::InSaveFile *in, TaskID id) : Task(in, id) {
	//  Restore the paused flag
	paused = in->readByte();

	//  Restore the counter
	counter = in->readSint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

int32 WanderTask::archiveSize(void) const {
	return      Task::archiveSize()
	            +   sizeof(paused)
	            +   sizeof(counter);
}

void WanderTask::write(Common::OutSaveFile *out) const {
	//  Let the base class archive its data
	Task::write(out);

	//  Store the paused flag
	out->writeByte(paused);

	//  Store the counter
	out->writeSint16LE(counter);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 WanderTask::getType(void) const {
	return wanderTask;
}

//----------------------------------------------------------------------

void WanderTask::abortTask(void) {
	//  if the actor has a wander motion, abort it
	MotionTask *actorMotion = stack->getActor()->moveTask;

	if (actorMotion && actorMotion->isWander()) actorMotion->finishWalk();
}

//----------------------------------------------------------------------

TaskResult WanderTask::evaluate(void) {
	//  Wandering is never done.  It must be stopped manually.
	return taskNotDone;
}

//----------------------------------------------------------------------

TaskResult WanderTask::update(void) {
	if (counter == 0) {
		if (!paused)
			pause();
		else
			wander();
	} else
		counter--;

	return !paused ? handleWander() : handlePaused();
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool WanderTask::operator == (const Task &t) const {
	return t.getType() == wanderTask;
}

//----------------------------------------------------------------------
//	Update function used when task is not paused

TaskResult WanderTask::handleWander(void) {
	MotionTask  *actorMotion = stack->getActor()->moveTask;

	//  If the actor is not already wandering, start a wander motion
	//  task
	if (!actorMotion
	        ||  !actorMotion->isWander())
		MotionTask::wander(*stack->getActor());

	return taskNotDone;
}

//----------------------------------------------------------------------
//	Set this task into the paused state

void WanderTask::pause(void) {
	//  Call abort to stop the wandering motion
	abortTask();

	paused = true;
	counter = (g_vm->_rnd->getRandomNumber(63) + g_vm->_rnd->getRandomNumber(63)) / 2;
}

//----------------------------------------------------------------------
//	Set this task into the wander state

void WanderTask::wander(void) {
	paused = false;
	counter = (g_vm->_rnd->getRandomNumber(255) + g_vm->_rnd->getRandomNumber(255)) / 2;
}

/* ===================================================================== *
   TetheredWanderTask member functions
 * ===================================================================== */

TetheredWanderTask::TetheredWanderTask(Common::InSaveFile *in, TaskID id) : WanderTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading TetheredWanderTask");

	//  Restore the tether coordinates
	minU = in->readSint16LE();
	minV = in->readSint16LE();
	maxU = in->readSint16LE();
	maxV = in->readSint16LE();

	//  Put the gotoTether ID into the gotoTether pointer field
	int gotoTetherID = in->readSint16LE();
	gotoTether = gotoTetherID != NoTask
	             ? (GotoRegionTask *)getTaskAddress(gotoTetherID)
	             :   NULL;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

inline int32 TetheredWanderTask::archiveSize(void) const {
	return      WanderTask::archiveSize()
	            +   sizeof(minU)
	            +   sizeof(minU)
	            +   sizeof(minU)
	            +   sizeof(minU)
	            +   sizeof(TaskID);      //  gotoTether ID
}

void TetheredWanderTask::write(Common::OutSaveFile *out) const {
	debugC(3, kDebugSaveload, "... Saving TetheredWanderTask");

	//  Let the base class archive its data
	WanderTask::write(out);

	//  Archive tether coordinates
	out->writeSint16LE(minU);
	out->writeSint16LE(minV);
	out->writeSint16LE(maxU);
	out->writeSint16LE(maxV);

	//  Archive gotoTether ID
	if (gotoTether != NULL)
		out->writeSint16LE(getTaskID(gotoTether));
	else
		out->writeSint16LE(NoTask);
}

#if DEBUG
//----------------------------------------------------------------------
//	Debugging function used to mark this task and any sub tasks as being
//	used.  This is used to find task leaks.

void TetheredWanderTask::mark(void) {
	WanderTask::mark();
	if (gotoTether != NULL)
		gotoTether->mark();
}
#endif

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 TetheredWanderTask::getType(void) const {
	return tetheredWanderTask;
}

//----------------------------------------------------------------------

void TetheredWanderTask::abortTask(void) {
	if (gotoTether != NULL) {
		gotoTether->abortTask();
		delete gotoTether;
		gotoTether = NULL;
	} else {
		MotionTask *actorMotion = stack->getActor()->moveTask;

		//  if the actor has a tethered wander motion, abort it
		if (actorMotion && actorMotion->isTethered())
			actorMotion->finishWalk();
	}
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool TetheredWanderTask::operator == (const Task &t) const {
	if (t.getType() != tetheredWanderTask) return false;

	const TetheredWanderTask *taskPtr = (const TetheredWanderTask *)&t;

	return      minU == taskPtr->minU && minV == taskPtr->minV
	            &&  maxU == taskPtr->maxU && maxV == taskPtr->maxV;
}

//----------------------------------------------------------------------
//	Update function used when task is not paused

TaskResult TetheredWanderTask::handleWander(void) {
	Actor       *a = stack->getActor();
	TilePoint   actorLoc = a->getLocation();

	if (actorLoc.u < minU || actorLoc.u >= maxU
	        ||  actorLoc.v < minV || actorLoc.v >= maxV) {
		if (gotoTether != NULL)
			gotoTether->update();
		else {
			gotoTether = new GotoRegionTask(stack, minU, minV, maxU, maxV);
			if (gotoTether != NULL) gotoTether->update();
		}
	} else {
		if (gotoTether != NULL) {
			gotoTether->abortTask();
			delete gotoTether;
			gotoTether = NULL;
		}

		bool            startWander = false;
		TileRegion      motionTether;

		MotionTask  *actorMotion = a->moveTask;

		if (actorMotion) {
			TileRegion  motionTeth = actorMotion->getTether();
			startWander     = ((!actorMotion->isWander())
			                   ||  motionTeth.min.u != minU
			                   ||  motionTeth.min.v != minV
			                   ||  motionTeth.max.u != maxU
			                   ||  motionTeth.max.v != maxV);

		} else
			startWander = true;

		//  If the actor is not already wandering, start a wander motion
		//  task

		// JeffL - prevent null pointer reference
		/*
		if (    !actorMotion
		    ||  !actorMotion->isWander()
		    ||  motionTether.min.u != minU
		    ||  motionTether.min.v != minV
		    ||  motionTether.max.u != maxU
		    ||  motionTether.max.v != maxV )
		*/
		if (startWander) {
			TileRegion  reg;

			reg.min = TilePoint(minU, minV, 0);
			reg.max = TilePoint(maxU, maxV, 0);
			MotionTask::tetheredWander(*stack->getActor(), reg);
		}
	}

	return taskNotDone;
}

/* ===================================================================== *
   GotoTask member functions
 * ===================================================================== */

GotoTask::GotoTask(Common::InSaveFile *in, TaskID id) : Task(in, id) {
	//  Get the wander TaskID
	TaskID wanderID = in->readSint16LE();
	wander = wanderID != NoTask
	         ? (WanderTask *)getTaskAddress(wanderID)
	         :   NULL;

	//  Restore prevRunState
	prevRunState = in->readByte();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 GotoTask::archiveSize(void) const {
	return      Task::archiveSize()
	            +   sizeof(TaskID)       //  wander ID
	            +   sizeof(prevRunState);
}

void GotoTask::write(Common::OutSaveFile *out) const {
	//  Let the base class archive its data
	Task::write(out);

	//  Convert the wander Task pointer to a TaskID and store it
	//  in the buffer
	if (wander != NULL)
		out->writeSint16LE(getTaskID(wander));
	else
		out->writeSint16LE(NoTask);

	//  Store prevRunState
	out->writeByte(prevRunState);
}

#if DEBUG
//----------------------------------------------------------------------
//	Debugging function used to mark this task and any sub tasks as being
//	used.  This is used to find task leaks.

void GotoTask::mark(void) {
	Task::mark();
	if (wander != NULL)
		wander->mark();
}
#endif

//----------------------------------------------------------------------

void GotoTask::abortTask(void) {
	//  If there is a wander subtask, delete it.
	if (wander) {
		wander->abortTask();
		delete wander;
		wander = NULL;
	} else {
		MotionTask  *actorMotion = stack->getActor()->moveTask;

		if (actorMotion && actorMotion->isWalk()) actorMotion->finishWalk();
	}
}

//----------------------------------------------------------------------

TaskResult GotoTask::evaluate(void) {
	//  Determine if we have reach the target.
	if (stack->getActor()->getLocation() == destination()) {
		abortTask();
		return taskSucceeded;
	}

	return taskNotDone;
}

//----------------------------------------------------------------------

TaskResult GotoTask::update(void) {
	//  Check to see if we have reached the target
	{
		TaskResult  result = evaluate();
		if (result != taskNotDone) return result;
	}

	Actor *const   a = stack->getActor();
	//  Compute the immediate destination based upon wether or not the
	//  actor has a line of sight to the target.
	TilePoint       immediateDest = lineOfSight()
	                                ?   destination()
	                                :   intermediateDest();

	//  If we have a destination, walk there, else wander
	if (immediateDest != Nowhere) {
		//  If wandering, cut it out
		if (wander != NULL) {
			delete wander;
			wander = NULL;
		}

		//  Determine if there is aready a motion task, and if so,
		//  wether or not it needs to be modified.
		MotionTask  *actorMotion = a->moveTask;
		TilePoint   actorLoc = a->getLocation();

		if (actorMotion != NULL && actorMotion->isWalkToDest()) {
			bool        runState = run();
			TilePoint   motionTarget = actorMotion->getTarget();

			if ((actorLoc.u >> kTileUVShift)
			        == (immediateDest.u >> kTileUVShift)
			        && (actorLoc.v >> kTileUVShift)
			        == (immediateDest.v >> kTileUVShift)) {
				if (motionTarget != immediateDest
				        ||  runState != prevRunState)
					actorMotion->changeDirectTarget(
					    immediateDest,
					    prevRunState = runState);
			} else {
				if ((motionTarget.u >> kTileUVShift)
				        != (immediateDest.u >> kTileUVShift)
				        || (motionTarget.v >> kTileUVShift)
				        != (immediateDest.v >> kTileUVShift)
				        ||  ABS(motionTarget.z - immediateDest.z) > 16
				        ||  runState != prevRunState)
					actorMotion->changeTarget(
					    immediateDest,
					    prevRunState = runState);
			}
		} else {
			if ((actorLoc.u >> kTileUVShift)
			        == (immediateDest.u >> kTileUVShift)
			        && (actorLoc.v >> kTileUVShift)
			        == (immediateDest.v >> kTileUVShift)) {
				MotionTask::walkToDirect(
				    *a,
				    immediateDest,
				    prevRunState = run());
			} else
				MotionTask::walkTo(*a, immediateDest, prevRunState = run());
		}
	} else {
		//  If wandering, update the wander task else set up a new
		//  wander task
		if (wander != NULL)
			wander->update();
		else {
			wander = new WanderTask(stack);
			if (wander != NULL) wander->update();
		}

		return taskNotDone;
	}

	return taskNotDone;
}

/* ===================================================================== *
   GotoLocationTask member functions
 * ===================================================================== */

GotoLocationTask::GotoLocationTask(Common::InSaveFile *in, TaskID id) : GotoTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading GotoLocationTask");

	//  Restore the target location
	targetLoc.load(in);

	//  Restore the runThreshold
	runThreshold = in->readByte();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 GotoLocationTask::archiveSize(void) const {
	return      GotoTask::archiveSize()
	            +   sizeof(targetLoc)
	            +   sizeof(runThreshold);
}

void GotoLocationTask::write(Common::OutSaveFile *out) const {
	debugC(3, kDebugSaveload, "... Saving GotoLocationTask");

	//  Let the base class archive its data
	GotoTask::write(out);

	//  Archive the target location
	targetLoc.write(out);

	//  Archive the run threshold
	out->writeByte(runThreshold);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 GotoLocationTask::getType(void) const {
	return gotoLocationTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool GotoLocationTask::operator == (const Task &t) const {
	if (t.getType() != gotoLocationTask) return false;

	const GotoLocationTask *taskPtr = (const GotoLocationTask *)&t;

	return      targetLoc == taskPtr->targetLoc
	            &&  runThreshold == taskPtr->runThreshold;
}

//----------------------------------------------------------------------

TilePoint GotoLocationTask::destination(void) {
	//  Simply return the target location
	return targetLoc;
}

//----------------------------------------------------------------------

TilePoint GotoLocationTask::intermediateDest(void) {
	//  GotoLocationTask's never have an intermediate destination
	return targetLoc;
}

//----------------------------------------------------------------------

bool GotoLocationTask::lineOfSight(void) {
	//  Let's pretend that there is always a line of sight to the
	//  target location
	return true;
}

//----------------------------------------------------------------------

bool GotoLocationTask::run(void) {
	TilePoint       actorLoc = stack->getActor()->getLocation();

	return  runThreshold != maxuint8
	        ? (targetLoc - actorLoc).quickHDistance() > runThreshold
	        ||  ABS(targetLoc.z - actorLoc.z) > runThreshold
	        :   false;
}

/* ===================================================================== *
   GotoRegionTask member functions
 * ===================================================================== */

GotoRegionTask::GotoRegionTask(Common::InSaveFile *in, TaskID id) : GotoTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading GotoRegionTask");

	//  Restore the region coordinates
	regionMinU = in->readSint16LE();
	regionMinV = in->readSint16LE();
	regionMaxU = in->readSint16LE();
	regionMaxV = in->readSint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 GotoRegionTask::archiveSize(void) const {
	return      GotoTask::archiveSize()
	            +   sizeof(regionMinU)
	            +   sizeof(regionMinV)
	            +   sizeof(regionMaxU)
	            +   sizeof(regionMaxV);
}

void GotoRegionTask::write(Common::OutSaveFile *out) const {
	debugC(3, kDebugSaveload, "... Saving GotoRegionTask");

	//  Let the base class archive its data
	GotoTask::write(out);
	//  Archive the region coordinates
	out->writeSint16LE(regionMinU);
	out->writeSint16LE(regionMinV);
	out->writeSint16LE(regionMaxU);
	out->writeSint16LE(regionMaxV);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 GotoRegionTask::getType(void) const {
	return gotoRegionTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool GotoRegionTask::operator == (const Task &t) const {
	if (t.getType() != gotoRegionTask) return false;

	const GotoRegionTask *taskPtr = (const GotoRegionTask *)&t;

	return      regionMinU == taskPtr->regionMinU
	            &&  regionMinV == taskPtr->regionMinV
	            &&  regionMaxU == taskPtr->regionMaxU
	            &&  regionMaxV == taskPtr->regionMaxV;
}

TilePoint GotoRegionTask::destination(void) {
	TilePoint   actorLoc = stack->getActor()->getLocation();

	return  TilePoint(
	            clamp(regionMinU, actorLoc.u, regionMaxU - 1),
	            clamp(regionMinV, actorLoc.v, regionMaxV - 1),
	            actorLoc.z);
}

//----------------------------------------------------------------------

TilePoint GotoRegionTask::intermediateDest(void) {
	return destination();
}

//----------------------------------------------------------------------

bool GotoRegionTask::lineOfSight(void) {
	return true;
}

//----------------------------------------------------------------------

bool GotoRegionTask::run(void) {
	return false;
}

/* ===================================================================== *
   GotoObjectTargetTask member functions
 * ===================================================================== */

GotoObjectTargetTask::GotoObjectTargetTask(Common::InSaveFile *in, TaskID id) : GotoTask(in, id) {
	//  Restore lastTestedLoc and increment pointer
	lastTestedLoc.load(in);

	//  Restore sightCtr and increment pointer
	sightCtr = in->readSint16LE();

	//  Restore the flags and increment pointer
	flags = in->readByte();

	//  Restore lastKnownLoc
	lastKnownLoc.load(in);
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 GotoObjectTargetTask::archiveSize(void) const {
	return      GotoTask::archiveSize()
	            +   sizeof(lastTestedLoc)
	            +   sizeof(sightCtr)
	            +   sizeof(flags)
	            +   sizeof(lastKnownLoc);
}

void GotoObjectTargetTask::write(Common::OutSaveFile *out) const {
	//  Let the base class archive its data
	GotoTask::write(out);

	//  Archive lastTestedLoc and increment pointer
	lastTestedLoc.write(out);

	//  Archive sightCtr and increment pointer
	out->writeSint16LE(sightCtr);

	//  Archive the flags and increment pointer
	out->writeByte(flags);

	//  Archive lastKnownLoc
	lastKnownLoc.write(out);
}

//----------------------------------------------------------------------

TilePoint GotoObjectTargetTask::destination(void) {
	//  Return the object's true location
	return getObject()->getLocation();
}

//----------------------------------------------------------------------

TilePoint GotoObjectTargetTask::intermediateDest(void) {
	//  Return the last known location
	return lastKnownLoc;
}

//----------------------------------------------------------------------

bool GotoObjectTargetTask::lineOfSight(void) {
	if (flags & track) {
		flags |= inSight;
		lastKnownLoc = getObject()->getLocation();
	} else {
		Actor       *a = stack->getActor();
		GameObject  *target = getObject();
		ObjectID    targetID = target->thisID();
		TilePoint   targetLoc = target->getLocation();
		SenseInfo   info;

		//  Determine if we need to retest the line of sight
		if (flags & inSight) {
			//  If the object was previously in sight, retest the line of
			//  sight if the target has moved beyond a certain range from
			//  the last location it was tested at.
			if ((targetLoc - lastTestedLoc).quickHDistance() > 25
			        ||  ABS(targetLoc.z - lastTestedLoc.z) > 25) {
				if (a->canSenseSpecificObject(
				            info,
				            maxSenseRange,
				            targetID)
				        ||  a->canSenseSpecificObjectIndirectly(
				            info,
				            maxSenseRange,
				            targetID))
					flags |= inSight;
				else
					flags &= ~inSight;
				lastTestedLoc = targetLoc;
			}
		} else {
			//  If the object was not privously in sight, retest the line
			//  of sight periodically
			if (sightCtr == 0) {
				sightCtr = sightRate;
				if (a->canSenseSpecificObject(
				            info,
				            maxSenseRange,
				            targetID)
				        ||  a->canSenseSpecificObjectIndirectly(
				            info,
				            maxSenseRange,
				            targetID))
					flags |= inSight;
				else
					flags &= ~inSight;
				lastTestedLoc = targetLoc;
			}
			sightCtr--;
		}

		if (flags & inSight) {
			//  If the target is in sight, the last known location is the
			//  objects current location.
			lastKnownLoc = targetLoc;
		} else {
			//  If the target is not in sight, determine if we've already
			//  reached the last know location and if so set the last
			//  known location to Nowhere
			if (lastKnownLoc != Nowhere
			        && (lastKnownLoc - a->getLocation()).quickHDistance() <= 4)
				lastKnownLoc = Nowhere;
		}
	}

	return flags & inSight;
}

/* ===================================================================== *
   GotoObjectTask member functions
 * ===================================================================== */

GotoObjectTask::GotoObjectTask(Common::InSaveFile *in, TaskID id) :
	GotoObjectTargetTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading GotoObjectTask");

	ObjectID targetID = in->readUint16LE();

	//  Restore the targetObj pointer
	targetObj = targetID != Nothing
	            ?   GameObject::objectAddress(targetID)
	            :   NULL;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 GotoObjectTask::archiveSize(void) const {
	return GotoObjectTargetTask::archiveSize() + sizeof(ObjectID);
}

void GotoObjectTask::write(Common::OutSaveFile *out) const {
	debugC(3, kDebugSaveload, "... Saving GotoObjectTask");

	//  Let the base class archive its data
	GotoObjectTargetTask::write(out);

	if (targetObj != NULL)
		out->writeUint16LE(targetObj->thisID());
	else
		out->writeUint16LE(Nothing);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 GotoObjectTask::getType(void) const {
	return gotoObjectTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool GotoObjectTask::operator == (const Task &t) const {
	if (t.getType() != gotoObjectTask) return false;

	const GotoObjectTask *taskPtr = (const GotoObjectTask *)&t;

	return      tracking() == taskPtr->tracking()
	            &&  targetObj == taskPtr->targetObj;
}

//----------------------------------------------------------------------

GameObject *GotoObjectTask::getObject(void) {
	//  Simply return the pointer to the target object
	return targetObj;
}

//----------------------------------------------------------------------

bool GotoObjectTask::run(void) {
	//  Running after objects has not been implemented yet
	return false;
}

/* ===================================================================== *
   GotoActorTask member functions
 * ===================================================================== */

GotoActorTask::GotoActorTask(Common::InSaveFile *in, TaskID id) :
	GotoObjectTargetTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading GotoActorTask");
	//  Restore the targetObj pointer
	ObjectID targetID = in->readUint16LE();
	targetActor =   targetID != Nothing
	                ? (Actor *)GameObject::objectAddress(targetID)
	                :   NULL;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 GotoActorTask::archiveSize(void) const {
	return GotoObjectTargetTask::archiveSize() + sizeof(ObjectID);
}

void GotoActorTask::write(Common::OutSaveFile *out) const {
	debugC(3, kDebugSaveload, "... Saving GotoActorTask");

	//  Let the base class archive its data
	GotoObjectTargetTask::write(out);

	if (targetActor != NULL)
		out->writeUint16LE(targetActor->thisID());
	else
		out->writeUint16LE(Nothing);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 GotoActorTask::getType(void) const {
	return gotoActorTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool GotoActorTask::operator == (const Task &t) const {
	if (t.getType() != gotoActorTask) return false;

	const GotoActorTask *taskPtr = (const GotoActorTask *)&t;

	return      tracking() == taskPtr->tracking()
	            &&  targetActor == taskPtr->targetActor;
}

//----------------------------------------------------------------------

GameObject *GotoActorTask::getObject(void) {
	//  Simply return the pointer to the target actor
	return (GameObject *)targetActor;
}

//----------------------------------------------------------------------

bool GotoActorTask::run(void) {
	if (isInSight()) {
		TilePoint       actorLoc = stack->getActor()->getLocation(),
		                targetLoc = getTarget()->getLocation();

		return (actorLoc - targetLoc).quickHDistance() >= kTileUVSize * 4;
	} else
		return lastKnownLoc != Nowhere;
}

GoAwayFromTask::GoAwayFromTask(Common::InSaveFile *in, TaskID id) : Task(in, id) {
	//  Get the subtask ID
	TaskID goTaskID = in->readSint16LE();
	goTask = goTaskID != NoTask
	         ? (GotoLocationTask *)getTaskAddress(goTaskID)
	         :   NULL;

	//  Restore the flags
	flags = in->readByte();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 GoAwayFromTask::archiveSize(void) const {
	return Task::archiveSize() + sizeof(TaskID) + sizeof(flags);
}

void GoAwayFromTask::write(Common::OutSaveFile *out) const {
	//  Let the base class archive its data
	Task::write(out);

	//  Store the subTask's ID
	if (goTask != NULL)
		out->writeSint16LE(getTaskID(goTask));
	else
		out->writeSint16LE(NoTask);

	//  Store the flags
	out->writeByte(flags);
}

#if DEBUG
//----------------------------------------------------------------------
//	Debugging function used to mark this task and any sub tasks as being
//	used.  This is used to find task leaks.

void GoAwayFromTask::mark(void) {
	Task::mark();
	if (goTask != NULL)
		goTask->mark();
}
#endif

//----------------------------------------------------------------------
//	Abort this task

void GoAwayFromTask::abortTask(void) {
	if (goTask != NULL) {
		goTask->abortTask();
		delete goTask;
		goTask = NULL;
	}
}

//----------------------------------------------------------------------
//	Evaluate this task

TaskResult GoAwayFromTask::evaluate(void) {
	//  Going away is never done, it must be stopped manually
	return taskNotDone;
}

//----------------------------------------------------------------------
//	Update this task

TaskResult GoAwayFromTask::update(void) {
	static const TilePoint dirTable_[] = {
		TilePoint(64,  64, 0),
		TilePoint(0,  64, 0),
		TilePoint(-64,  64, 0),
		TilePoint(-64,   0, 0),
		TilePoint(-64, -64, 0),
		TilePoint(0, -64, 0),
		TilePoint(64, -64, 0),
		TilePoint(64,   0, 0),
	};

	Actor           *a = stack->getActor();
	TilePoint       actorLoc = a->getLocation(),
	                repulsionVector = getRepulsionVector(),
	                dest;
	int16           repulsionDist = repulsionVector.quickHDistance();

	//  Compute a point for the actor to walk toward
	if (repulsionDist != 0) {
		dest.u = actorLoc.u + ((int32)repulsionVector.u * 64 / repulsionDist);
		dest.v = actorLoc.v + ((int32)repulsionVector.v * 64 / repulsionDist);
		dest.z = actorLoc.z;
	} else
		dest = actorLoc + dirTable_[a->currentFacing];

	if (goTask != NULL) {
		if (goTask->getTarget() != dest)
			goTask->changeTarget(dest);

		goTask->update();
	} else {
		if ((goTask =   flags & run
		                ?   new GotoLocationTask(stack, dest, 0)
		                :   new GotoLocationTask(stack, dest))
		        !=  NULL)
			goTask->update();
	}

	return taskNotDone;
}

/* ===================================================================== *
   GoAwayFromObjectTask member functions
 * ===================================================================== */

GoAwayFromObjectTask::GoAwayFromObjectTask(Common::InSaveFile *in, TaskID id) :
	GoAwayFromTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading GoAwayFromObjectTask");

	//  Get the object's ID
	ObjectID objectID = in->readUint16LE();

	//  Convert the ID to an object pointer
	obj = objectID != Nothing
		? GameObject::objectAddress(objectID)
		: NULL;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

int32 GoAwayFromObjectTask::archiveSize(void) const {
	return GoAwayFromTask::archiveSize() + sizeof(ObjectID);
}

void GoAwayFromObjectTask::write(Common::OutSaveFile *out) const {
	debugC(3, kDebugSaveload, "... Saving GoAwayFromObjectTask");

	//  Let the base class archive its data
	GoAwayFromTask::write(out);

	//  Store the object's ID
	if (obj != NULL)
		out->writeUint16LE(obj->thisID());
	else
		out->writeUint16LE(Nothing);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 GoAwayFromObjectTask::getType(void) const {
	return goAwayFromObjectTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool GoAwayFromObjectTask::operator == (const Task &t) const {
	if (t.getType() != goAwayFromObjectTask) return false;

	const GoAwayFromObjectTask *taskPtr = (const GoAwayFromObjectTask *)&t;

	return obj == taskPtr->obj;
}

//----------------------------------------------------------------------
//	Simply return the object's location

TilePoint GoAwayFromObjectTask::getRepulsionVector(void) {
	return stack->getActor()->getLocation() - obj->getLocation();
}

/* ===================================================================== *
   GoAwayFromActorTask member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- initial construction

GoAwayFromActorTask::GoAwayFromActorTask(
    TaskStack   *ts,
    Actor       *a,
    bool        runFlag) :
	GoAwayFromTask(ts, runFlag) {
	debugC(2, kDebugTasks, " - GoAwayFromActorTask1");
	SpecificActorTarget(a).clone(targetMem);
}

GoAwayFromActorTask::GoAwayFromActorTask(
    TaskStack           *ts,
    const ActorTarget   &at,
    bool                runFlag) :
	GoAwayFromTask(ts, runFlag) {
	assert(at.size() <= sizeof(targetMem));
	debugC(2, kDebugTasks, " - GoAwayFromActorTask2");
	//  Copy the target to the target buffer
	at.clone(targetMem);
}


GoAwayFromActorTask::GoAwayFromActorTask(Common::InSaveFile *in, TaskID id) : GoAwayFromTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading GoAwayFromActorTask");

	//  Restore the target
	readTarget(targetMem, in);
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

int32 GoAwayFromActorTask::archiveSize(void) const {
	return GoAwayFromTask::archiveSize() + targetArchiveSize(getTarget());
}

void GoAwayFromActorTask::write(Common::OutSaveFile *out) const {
	debugC(3, kDebugSaveload, "... Saving GoAwayFromActorTask");

	//  Let the base class archive its data
	GoAwayFromTask::write(out);

	//  Store the target
	writeTarget(getTarget(), out);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 GoAwayFromActorTask::getType(void) const {
	return goAwayFromActorTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool GoAwayFromActorTask::operator == (const Task &t) const {
	if (t.getType() != goAwayFromActorTask) return false;

	const GoAwayFromActorTask *taskPtr = (const GoAwayFromActorTask *)&t;

	return *getTarget() == *taskPtr->getTarget();
}

//----------------------------------------------------------------------

TilePoint GoAwayFromActorTask::getRepulsionVector(void) {
	Actor               *a = stack->getActor();
	TilePoint           actorLoc = a->getLocation(),
	                    repulsionVector;
	int16               i;
	TilePoint           locArray[6];
	int16               strengthArray[ARRAYSIZE(locArray)] =
	{ 1, 1, 1, 1, 1, 1 };
	int16               distArray[ARRAYSIZE(locArray)];
	TargetLocationArray tla(
	    ARRAYSIZE(locArray),
	    locArray,
	    distArray);

	getTarget()->where(a->world(), actorLoc, tla);

	if (tla.locs == 0) return TilePoint(0, 0, 0);

	for (i = 0; i < tla.locs; i++)
		locArray[i] -= actorLoc;

	repulsionVector = computeRepulsionVector(locArray, strengthArray, tla.locs);

	return  repulsionVector.quickHDistance() > 0
	        ?   repulsionVector
	        :   -locArray[0];
}

/* ===================================================================== *
   HuntTask member functions
 * ===================================================================== */

HuntTask::HuntTask(Common::InSaveFile *in, TaskID id) : Task(in, id) {
	//  Restore the flags
	huntFlags = in->readByte();

	//  If the flags say we have a sub task, restore it too
	if (huntFlags & (huntGoto | huntWander)) {
		TaskID subTaskID = in->readSint16LE();
		subTask = getTaskAddress(subTaskID);
	}
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntTask::archiveSize(void) const {
	int32       size = 0;

	size += Task::archiveSize() + sizeof(huntFlags);
	if (huntFlags & (huntGoto | huntWander)) size += sizeof(TaskID);

	return size;
}

void HuntTask::write(Common::OutSaveFile *out) const {
	//  Let the base class archive its data
	Task::write(out);

	//  Store the flags
	out->writeByte(huntFlags);

	//  If the flags say we have a sub task, store it too
	if (huntFlags & (huntGoto | huntWander))
		out->writeSint16LE(getTaskID(subTask));
}

#if DEBUG
//----------------------------------------------------------------------
//	Debugging function used to mark this task and any sub tasks as being
//	used.  This is used to find task leaks.

void HuntTask::mark(void) {
	Task::mark();
	if (huntFlags & (huntGoto | huntWander))
		subTask->mark();
}
#endif

//----------------------------------------------------------------------

void HuntTask::abortTask(void) {
	if (huntFlags & (huntWander | huntGoto)) {
		subTask->abortTask();
		delete subTask;
	}

	//  If we've reached the target call the atTargetabortTask() function
	if (atTarget()) atTargetabortTask();
}

//----------------------------------------------------------------------

TaskResult HuntTask::evaluate(void) {
	if (atTarget()) {
		//  If we've reached the target abort any sub tasks
		if (huntFlags & huntWander)
			removeWanderTask();
		else if (huntFlags & huntGoto)
			removeGotoTask();

		return atTargetEvaluate();
	} else
		//  If we haven't reached the target, we know we're not done
		return taskNotDone;
}

//----------------------------------------------------------------------

TaskResult HuntTask::update(void) {
	Actor       *a = stack->getActor();

	if (a->moveTask && a->moveTask->isPrivledged()) return taskNotDone;

	//  Reevaluate the target
	evaluateTarget();

	//  Determine if we have reached the target
	if (atTarget()) {
		//  If we've reached the target abort any sub tasks
		if (huntFlags & huntWander)
			removeWanderTask();
		else if (huntFlags & huntGoto)
			removeGotoTask();

		return atTargetUpdate();
	} else {
		//  If we are going to a target, determine if the goto task
		//  is still valid.  If not, abort it.
		if ((huntFlags & huntGoto)
		        &&  targetHasChanged((GotoTask *)subTask))
			removeGotoTask();

		//  Determine if there is a goto subtask
		if (!(huntFlags & huntGoto)) {
			GotoTask    *gotoResult;

			//  Try to set up a goto subtask
			if ((gotoResult = setupGoto()) != NULL) {
				if (huntFlags & huntWander) removeWanderTask();

				subTask = gotoResult;
				huntFlags |= huntGoto;
			} else {
				//  If we couldn't setup a goto task, setup a wander task
				if (!(huntFlags & huntWander)) {
					if ((subTask = new WanderTask(stack)) != NULL)
						huntFlags |= huntWander;
				}
			}
		}

		//  If there is a subtask, update it
		if (huntFlags & (huntGoto | huntWander)) subTask->update();

		//  If we're not at the target, we know the hunt task is not
		//  done
		return taskNotDone;
	}
}

//----------------------------------------------------------------------

void HuntTask::removeWanderTask(void) {
	subTask->abortTask();
	delete subTask;
	huntFlags &= ~huntWander;
}

//----------------------------------------------------------------------

void HuntTask::removeGotoTask(void) {
	subTask->abortTask();
	delete subTask;
	huntFlags &= ~huntGoto;
}

/* ===================================================================== *
   HuntLocationTask member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- initial construction

HuntLocationTask::HuntLocationTask(TaskStack *ts, const Target &t) :
	HuntTask(ts),
	currentTarget(Nowhere) {
	assert(t.size() <= sizeof(targetMem));
	debugC(2, kDebugTasks, " - HuntLocationTask");
	//  Copy the target to the target buffer
	t.clone(targetMem);
}

HuntLocationTask::HuntLocationTask(Common::InSaveFile *in, TaskID id) : HuntTask(in, id) {
	//  Restore the currentTarget location
	currentTarget.load(in);

	//  Restore the target
	readTarget(targetMem, in);
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntLocationTask::archiveSize(void) const {
	return      HuntTask::archiveSize()
	            +   sizeof(currentTarget)
	            +   targetArchiveSize(getTarget());
}

void HuntLocationTask::write(Common::OutSaveFile *out) const {
	//  Let the base class archive its data
	HuntTask::write(out);

	//  Store the current target location
	currentTarget.write(out);

	//  Store the target
	writeTarget(getTarget(), out);
}

//----------------------------------------------------------------------

bool HuntLocationTask::targetHasChanged(GotoTask *gotoTarget) {
	//  Determine if the specified goto task is going to the current
	//  target.
	GotoLocationTask    *gotoLoc = (GotoLocationTask *)gotoTarget;
	return gotoLoc->getTarget() != currentTarget;
}

//----------------------------------------------------------------------

GotoTask *HuntLocationTask::setupGoto(void) {
	//  If there is somewhere to go, setup a goto task, else return NULL
	return  currentTarget != Nowhere
	        ?   new GotoLocationTask(stack, currentTarget)
	        :   NULL;
}

//----------------------------------------------------------------------

TilePoint HuntLocationTask::currentTargetLoc(void) {
	return currentTarget;
}

/* ===================================================================== *
   HuntToBeNearLocationTask member functions
 * ===================================================================== */

HuntToBeNearLocationTask::HuntToBeNearLocationTask(Common::InSaveFile *in, TaskID id) :
	HuntLocationTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading HuntToBeNearLocationTask");

	//  Restore the range
	range = in->readUint16LE();

	//  Restore the evaluation counter
	targetEvaluateCtr = in->readByte();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntToBeNearLocationTask::archiveSize(void) const {
	return      HuntLocationTask::archiveSize()
	            +   sizeof(range)
	            +   sizeof(targetEvaluateCtr);
}

void HuntToBeNearLocationTask::write(Common::OutSaveFile *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToBeNearLocationTask");

	//  Let the base class archive its data
	HuntLocationTask::write(out);

	//  Store the range
	out->writeUint16LE(range);

	//  Store the evaluation counter
	out->writeByte(targetEvaluateCtr);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 HuntToBeNearLocationTask::getType(void) const {
	return huntToBeNearLocationTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool HuntToBeNearLocationTask::operator == (const Task &t) const {
	if (t.getType() != huntToBeNearLocationTask) return false;

	const HuntToBeNearLocationTask *taskPtr = (const HuntToBeNearLocationTask *)&t;

	return      *getTarget() == *taskPtr->getTarget()
	            &&  range == taskPtr->range;
}

//----------------------------------------------------------------------

void HuntToBeNearLocationTask::evaluateTarget(void) {
	//  If its time to reevaluate the target, simply get the nearest
	//  target location from the LocationTarget
	if (targetEvaluateCtr == 0) {
		Actor   *a = stack->getActor();

		currentTarget =
		    getTarget()->where(a->world(), a->getLocation());

		targetEvaluateCtr = targetEvaluateRate;
	}
	targetEvaluateCtr--;
}

//----------------------------------------------------------------------

bool HuntToBeNearLocationTask::atTarget(void) {
	TilePoint   targetLoc = currentTargetLoc();

	//  Determine if we are within the specified range of the target
	return  targetLoc != Nowhere
	        &&  stack->getActor()->inRange(targetLoc, range);
}

//----------------------------------------------------------------------

void HuntToBeNearLocationTask::atTargetabortTask(void) {}

//----------------------------------------------------------------------

TaskResult HuntToBeNearLocationTask::atTargetEvaluate(void) {
	//  If we're at the target, we're done
	return taskSucceeded;
}

//----------------------------------------------------------------------

TaskResult HuntToBeNearLocationTask::atTargetUpdate(void) {
	//  If we're at the target, we're done
	return taskSucceeded;
}

/* ===================================================================== *
   HuntObjectTask member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- initial construction

HuntObjectTask::HuntObjectTask(TaskStack *ts, const ObjectTarget &ot) :
	HuntTask(ts),
	currentTarget(NULL) {
	assert(ot.size() <= sizeof(targetMem));
	debugC(2, kDebugTasks, " - HuntObjectTask");
	//  Copy the target to the target buffer
	ot.clone(targetMem);
}

HuntObjectTask::HuntObjectTask(Common::InSaveFile *in, TaskID id) : HuntTask(in, id) {
	//  Restore the current target ID
	ObjectID currentTargetID = in->readUint16LE();

	//  Convert the ID to a GameObject pointer
	currentTarget = currentTargetID != Nothing
	                ?   GameObject::objectAddress(currentTargetID)
	                :   NULL;

	//  Reconstruct the object target
	readTarget(targetMem, in);
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntObjectTask::archiveSize(void) const {
	return      HuntTask::archiveSize()
	            +   sizeof(ObjectID)
	            +   targetArchiveSize(getTarget());
}

void HuntObjectTask::write(Common::OutSaveFile *out) const {
	//  Let the base class archive its data
	HuntTask::write(out);

	//  Store the ID
	if (currentTarget != NULL)
		out->writeByte(currentTarget->thisID());
	else
		out->writeByte(Nothing);

	//  Store the object target
	writeTarget(getTarget(), out);
}

//----------------------------------------------------------------------

bool HuntObjectTask::targetHasChanged(GotoTask *gotoTarget) {
	//  Determine if the specified goto task's destination is the
	//  current target object
	GotoObjectTask  *gotoObj = (GotoObjectTask *)gotoTarget;
	return gotoObj->getTarget() != currentTarget;
}

//----------------------------------------------------------------------

GotoTask *HuntObjectTask::setupGoto(void) {
	//  If there is an object to goto, setup a GotoObjectTask, else
	//  return NULL
	return  currentTarget
	        ?   new GotoObjectTask(stack, currentTarget)
	        :   NULL;
}

//----------------------------------------------------------------------

TilePoint HuntObjectTask::currentTargetLoc(void) {
	//  If there is a current target object, return its locatio, else
	//  return Nowhere
	return currentTarget ? currentTarget->getLocation() : Nowhere;
}

/* ===================================================================== *
   HuntToBeNearObjectTask member functions
 * ===================================================================== */

HuntToBeNearObjectTask::HuntToBeNearObjectTask(Common::InSaveFile *in, TaskID id) :
	HuntObjectTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading HuntToBeNearObjectTask");

	//  Restore the range
	range = in->readUint16LE();

	//  Restore the evaluation counter
	targetEvaluateCtr = in->readByte();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntToBeNearObjectTask::archiveSize(void) const {
	return      HuntObjectTask::archiveSize()
	            +   sizeof(range)
	            +   sizeof(targetEvaluateCtr);
}

void HuntToBeNearObjectTask::write(Common::OutSaveFile *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToBeNearObjectTask");

	//  Let the base class archive its data
	HuntObjectTask::write(out);

	//  Store the range
	out->writeUint16LE(range);

	//  Store the evaluation counter
	out->writeByte(targetEvaluateCtr);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 HuntToBeNearObjectTask::getType(void) const {
	return huntToBeNearObjectTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool HuntToBeNearObjectTask::operator == (const Task &t) const {
	if (t.getType() != huntToBeNearObjectTask) return false;

	const HuntToBeNearObjectTask *taskPtr = (const HuntToBeNearObjectTask *)&t;

	return      *getTarget() == *taskPtr->getTarget()
	            &&  range == taskPtr->range;
}

//----------------------------------------------------------------------

void HuntToBeNearObjectTask::evaluateTarget(void) {
	//  Determine if it is time to reevaluate the target object
	if (targetEvaluateCtr == 0) {
		Actor               *a = stack->getActor();
		int16               i;
		GameObject          *objArray[16];
		int16               distArray[ARRAYSIZE(objArray)];
		TargetObjectArray   toa(
		    ARRAYSIZE(objArray),
		    objArray,
		    distArray);
		SenseInfo           info;

		//  Get an array of objects from the ObjectTarget
		getTarget()->object(a->world(), a->getLocation(), toa);

		//  Iterate through each object in the array and determine if
		//  there is a line of sight to that object
		for (i = 0; i < toa.objs; i++) {
			ObjectID    objID = objArray[i]->thisID();

			if (a->canSenseSpecificObject(
			            info,
			            maxSenseRange,
			            objID)
			        ||  a->canSenseSpecificObjectIndirectly(
			            info,
			            maxSenseRange,
			            objID)) {
				currentTarget = objArray[i];
				break;
			}
		}

		targetEvaluateCtr = targetEvaluateRate;
	}

	//  Decrement the target reevaluate counter
	targetEvaluateCtr--;
}

//----------------------------------------------------------------------

bool HuntToBeNearObjectTask::atTarget(void) {
	TilePoint   targetLoc = currentTargetLoc();

	//  Determine if we are within the specified range of the current
	//  target
	return      targetLoc != Nowhere
	            &&  stack->getActor()->inRange(targetLoc, range);
}

//----------------------------------------------------------------------

void HuntToBeNearObjectTask::atTargetabortTask(void) {}

//----------------------------------------------------------------------

TaskResult HuntToBeNearObjectTask::atTargetEvaluate(void) {
	//  If we're at the target, we're done
	return taskSucceeded;
}

//----------------------------------------------------------------------

TaskResult HuntToBeNearObjectTask::atTargetUpdate(void) {
	//  If we're at the target, we're done
	return taskSucceeded;
}

/* ===================================================================== *
   HuntToPossessTask member functions
 * ===================================================================== */

//  Hunt to possess in not fully implemented yet

HuntToPossessTask::HuntToPossessTask(Common::InSaveFile *in, TaskID id) : HuntObjectTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading HuntToPossessTask");

	//  Restore evaluation counter
	targetEvaluateCtr = in->readByte();

	//  Restore grab flag
	grabFlag = in->readByte();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntToPossessTask::archiveSize(void) const {
	return      HuntObjectTask::archiveSize()
	            +   sizeof(targetEvaluateCtr)
	            +   sizeof(grabFlag);
}

void HuntToPossessTask::write(Common::OutSaveFile *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToPossessTask");

	//  Let the base class archive its data
	HuntObjectTask::write(out);

	//  Store the evaluation counter
	out->writeByte(targetEvaluateCtr);

	//  Store the grab flag
	out->writeByte(grabFlag);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 HuntToPossessTask::getType(void) const {
	return huntToPossessTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool HuntToPossessTask::operator == (const Task &t) const {
	if (t.getType() != huntToPossessTask) return false;

	const HuntToPossessTask *taskPtr = (const HuntToPossessTask *)&t;

	return *getTarget() == *taskPtr->getTarget();
}

//----------------------------------------------------------------------

void HuntToPossessTask::evaluateTarget(void) {
	//  Determine if it is time to reevaluate the target object
	if (targetEvaluateCtr == 0) {
		Actor               *a = stack->getActor();
		int16               i;
		GameObject          *objArray[16];
		int16               distArray[ARRAYSIZE(objArray)];
		TargetObjectArray   toa(
		    ARRAYSIZE(objArray),
		    objArray,
		    distArray);
		SenseInfo           info;

		//  Get an array of objects from the ObjectTarget
		getTarget()->object(a->world(), a->getLocation(), toa);

		//  Iterate through each object in the array and determine if
		//  there is a line of sight to that object
		for (i = 0; i < toa.objs; i++) {
			ObjectID    objID = objArray[i]->thisID();

			if (a->canSenseSpecificObject(
			            info,
			            maxSenseRange,
			            objID)
			        ||  a->canSenseSpecificObjectIndirectly(
			            info,
			            maxSenseRange,
			            objID)) {
				currentTarget = objArray[i];
				break;
			}
		}

		targetEvaluateCtr = targetEvaluateRate;
	}

	//  Decrement the target reevaluate counter
	targetEvaluateCtr--;
}

//----------------------------------------------------------------------

bool HuntToPossessTask::atTarget(void) {
	Actor   *a = stack->getActor();

	return  currentTarget
	        && (a->inReach(currentTarget->getLocation())
	            || (grabFlag
	                &&  a->isContaining(currentTarget)));
}

//----------------------------------------------------------------------

void HuntToPossessTask::atTargetabortTask(void) {}

//----------------------------------------------------------------------

TaskResult HuntToPossessTask::atTargetEvaluate(void) {
	if (currentTarget && stack->getActor()->isContaining(currentTarget))
		return taskSucceeded;

	return taskNotDone;
}

//----------------------------------------------------------------------

TaskResult HuntToPossessTask::atTargetUpdate(void) {
	//  Hunt to possess in not implemented yet
	return taskNotDone;
}

/* ===================================================================== *
   HuntActorTask member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- initial construction

HuntActorTask::HuntActorTask(
    TaskStack           *ts,
    const ActorTarget   &at,
    bool                trackFlag) :
	HuntTask(ts),
	flags(trackFlag ? track : 0),
	currentTarget(NULL) {
	assert(at.size() <= sizeof(targetMem));
	debugC(2, kDebugTasks, " - HuntActorTask");
	//  Copy the target to the target buffer
	at.clone(targetMem);
}

HuntActorTask::HuntActorTask(Common::InSaveFile *in, TaskID id) : HuntTask(in, id) {
	//  Restore the flags
	flags = in->readByte();

	//  Restore the current target ID
	ObjectID currentTargetID = in->readUint16LE();

	//  Convert the ID to a GameObject pointer
	currentTarget = currentTargetID != Nothing
	                ? (Actor *)GameObject::objectAddress(currentTargetID)
	                :   NULL;

	//  Reconstruct the object target
	readTarget(targetMem, in);
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntActorTask::archiveSize(void) const {
	return      HuntTask::archiveSize()
	            +   sizeof(flags)
	            +   sizeof(ObjectID)
	            +   targetArchiveSize(getTarget());
}

void HuntActorTask::write(Common::OutSaveFile *out) const {
	//  Let the base class archive its data
	HuntTask::write(out);

	//  Store the flags
	out->writeByte(flags);

	//  Store the ID
	if (currentTarget != NULL)
		out->writeUint16LE(currentTarget->thisID());
	else
		out->writeUint16LE(Nothing);

	//  Store the object target
	writeTarget(getTarget(), out);
}

//----------------------------------------------------------------------

bool HuntActorTask::targetHasChanged(GotoTask *gotoTarget) {
	//  Determine if the specified goto task's destination is the
	//  current target actor
	GotoActorTask   *gotoActor = (GotoActorTask *)gotoTarget;
	return gotoActor->getTarget() != currentTarget;
}

//----------------------------------------------------------------------

GotoTask *HuntActorTask::setupGoto(void) {
	//  If there is an actor to goto, setup a GotoActorTask, else
	//  return NULL
	/*  return  currentTarget
	            ?   new GotoActorTask( stack, currentTarget, flags & track )
	            :   NULL;
	*/
	if (currentTarget != NULL) {
		return new GotoActorTask(
		           stack,
		           currentTarget,
		           flags & track);
	}

	return NULL;
}

//----------------------------------------------------------------------

TilePoint HuntActorTask::currentTargetLoc(void) {
	//  If there is a current target actor, return its location, else
	//  return Nowhere
	return currentTarget ? currentTarget->getLocation() : Nowhere;
}

/* ===================================================================== *
   HuntToBeNearActorTask member functions
 * ===================================================================== */

HuntToBeNearActorTask::HuntToBeNearActorTask(Common::InSaveFile *in, TaskID id) :
	HuntActorTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading HuntToBeNearActorTask");

	//  Get the goAway task ID
	TaskID goAwayID = in->readSint16LE();

	//  Convert the task ID to a task pointer
	goAway = goAwayID != NoTask
	         ? (GoAwayFromObjectTask *)getTaskAddress(goAwayID)
	         :   NULL;

	//  Restore the range
	range = in->readUint16LE();

	//  Restore the evaluation counter
	targetEvaluateCtr = in->readByte();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntToBeNearActorTask::archiveSize(void) const {
	return      HuntActorTask::archiveSize()
	            +   sizeof(TaskID)               //  goAway ID
	            +   sizeof(range)
	            +   sizeof(targetEvaluateCtr);
}

void HuntToBeNearActorTask::write(Common::OutSaveFile *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToBeNearActorTask");

	//  Let the base class archive its data
	HuntActorTask::write(out);

	//  Store the task ID
	if (goAway != NULL)
		out->writeSint16LE(getTaskID(goAway));
	else
		out->writeSint16LE(NoTask);

	//  Store the range
	out->writeUint16LE(range);

	//  Store the evaluation counter
	out->writeByte(targetEvaluateCtr);
}

#if DEBUG
//----------------------------------------------------------------------
//	Debugging function used to mark this task and any sub tasks as being
//	used.  This is used to find task leaks.

void HuntToBeNearActorTask::mark(void) {
	HuntActorTask::mark();
	if (goAway != NULL)
		goAway->mark();
}
#endif

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 HuntToBeNearActorTask::getType(void) const {
	return huntToBeNearActorTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool HuntToBeNearActorTask::operator == (const Task &t) const {
	if (t.getType() != huntToBeNearActorTask) return false;

	const HuntToBeNearActorTask *taskPtr = (const HuntToBeNearActorTask *)&t;

	return      *getTarget() == *taskPtr->getTarget()
	            &&  tracking() ? taskPtr->tracking() : !taskPtr->tracking()
	            &&  range == taskPtr->range;
}

//----------------------------------------------------------------------

void HuntToBeNearActorTask::evaluateTarget(void) {
	//  Determine if its time to reevaluate the current target actor
	if (targetEvaluateCtr == 0) {
		Actor               *a = stack->getActor();
		int16               i;
		Actor               *actorArray[16];
		int16               distArray[ARRAYSIZE(actorArray)];
		TargetActorArray    taa(
		    ARRAYSIZE(actorArray),
		    actorArray,
		    distArray);
		SenseInfo           info;

		//  Get an array of actor pointers from the ActorTarget
		getTarget()->actor(a->world(), a->getLocation(), taa);

		//  Iterate through each actor in the array and determine if
		//  there is a line of sight to that actor
		for (i = 0; i < taa.actors; i++) {
			if (tracking()
			        ||  a->canSenseSpecificActor(
			            info,
			            maxSenseRange,
			            actorArray[i])
			        ||  a->canSenseSpecificActorIndirectly(
			            info,
			            maxSenseRange,
			            actorArray[i])) {
				if (currentTarget != actorArray[i]) {
					if (atTarget()) atTargetabortTask();
					currentTarget = actorArray[i];
				}

				break;
			}
		}

		targetEvaluateCtr = targetEvaluateRate;
	}

	//  Decrement the target reevaluation counter.
	targetEvaluateCtr--;
}

//----------------------------------------------------------------------

bool HuntToBeNearActorTask::atTarget(void) {
	TilePoint   targetLoc = currentTargetLoc();

	//  Determine if we're within the specified range of the current
	//  target actor
	if (targetLoc != Nowhere
	        &&  stack->getActor()->inRange(targetLoc, range))
		return true;
	else {
		if (goAway != NULL) {
			goAway->abortTask();
			delete goAway;
			goAway = NULL;
		}

		return false;
	}
}

//----------------------------------------------------------------------

void HuntToBeNearActorTask::atTargetabortTask(void) {
	if (goAway != NULL) {
		goAway->abortTask();
		delete goAway;
		goAway = NULL;
	}
}

//----------------------------------------------------------------------

TaskResult HuntToBeNearActorTask::atTargetEvaluate(void) {
	TilePoint   targetLoc = currentTargetLoc();

	//  If we're not TOO close, we're done
	if (stack->getActor()->inRange(targetLoc, tooClose))
		return taskNotDone;

	if (goAway != NULL) {
		goAway->abortTask();
		delete goAway;
		goAway = NULL;
	}

	return taskSucceeded;
}

//----------------------------------------------------------------------

TaskResult HuntToBeNearActorTask::atTargetUpdate(void) {
	Actor       *a = stack->getActor();
	TilePoint   targetLoc = currentTargetLoc();

	//  Determine if we're TOO close
	if (a->inRange(targetLoc, tooClose)) {
		//  Setup a go away task if necessary and update it
		if (goAway == NULL) {
			goAway = new GoAwayFromObjectTask(stack, currentTarget);
			if (goAway != NULL) goAway->update();
		} else
			goAway->update();

		return taskNotDone;
	}

	//  Delete the go away task if it exists
	if (goAway != NULL) {
		goAway->abortTask();
		delete goAway;
		goAway = NULL;
	}

	return taskSucceeded;
}

/* ===================================================================== *
   HuntToKillTask member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- initial construction

HuntToKillTask::HuntToKillTask(
    TaskStack           *ts,
    const ActorTarget   &at,
    bool                trackFlag) :
	HuntActorTask(ts, at, trackFlag),
	targetEvaluateCtr(0),
	specialAttackCtr(10),
	flags(evalWeapon) {
	debugC(2, kDebugTasks, " - HuntToKillTask");
	Actor       *a = stack->getActor();

	if (isActor(a->currentTarget))
		currentTarget = (Actor *)a->currentTarget;

	a->setFightStance(true);
}

HuntToKillTask::HuntToKillTask(Common::InSaveFile *in, TaskID id) : HuntActorTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading HuntToKillTask");

	//  Restore the evaluation counter
	targetEvaluateCtr = in->readByte();
	specialAttackCtr = in->readByte();
	flags = in->readByte();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntToKillTask::archiveSize(void) const {
	return      HuntActorTask::archiveSize()
	            +   sizeof(targetEvaluateCtr)
	            +   sizeof(specialAttackCtr)
	            +   sizeof(flags);
}

void HuntToKillTask::write(Common::OutSaveFile *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToKillTask");

	//  Let the base class archive its data
	HuntActorTask::write(out);

	//  Store the evaluation counter
	out->writeByte(targetEvaluateCtr);
	out->writeByte(specialAttackCtr);
	out->writeByte(flags);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 HuntToKillTask::getType(void) const {
	return huntToKillTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool HuntToKillTask::operator == (const Task &t) const {
	if (t.getType() != huntToKillTask) return false;

	const HuntToKillTask *taskPtr = (const HuntToKillTask *)&t;

	return      *getTarget() == *taskPtr->getTarget()
	            &&  tracking() ? taskPtr->tracking() : !taskPtr->tracking();
}

//----------------------------------------------------------------------

void HuntToKillTask::abortTask(void) {
	HuntActorTask::abortTask();

	Actor       *a = stack->getActor();

	a->flags &= ~Actor::specialAttack;

	a->setFightStance(false);
}

//----------------------------------------------------------------------

TaskResult HuntToKillTask::update(void) {
	if (specialAttackCtr == 0) {
		stack->getActor()->flags |= Actor::specialAttack;
		//  A little hack to make monsters with 99 spellcraft cast spells more often
		if (stack->getActor()->getStats()->spellcraft >= 99)
			specialAttackCtr = 3;
		else specialAttackCtr = 10;
	} else
		specialAttackCtr--;

	return HuntActorTask::update();
}

//----------------------------------------------------------------------

void HuntToKillTask::evaluateTarget(void) {
	Actor               *a = stack->getActor();

	if (flags & evalWeapon
	        &&  a->isInterruptable()) {
		evaluateWeapon();
		flags &= ~evalWeapon;
	}


	//  Determine if its time to reevaluate the current target actor
	if (targetEvaluateCtr == 0
	        || (currentTarget != NULL
	            &&  currentTarget->isDead())) {
		Actor               *bestTarget = NULL;
		ActorProto          *proto = (ActorProto *)a->proto();
		int16               i;
		Actor               *actorArray[16];
		int16               distArray[ARRAYSIZE(actorArray)];
		TargetActorArray    taa(
		    ARRAYSIZE(actorArray),
		    actorArray,
		    distArray);
		SenseInfo           info;

		//  Get an array of actor pointers from the ActorTarget
		getTarget()->actor(a->world(), a->getLocation(), taa);

		switch (proto->combatBehavior) {
		case behaviorHungry:
			//  Iterate through each actor in the array and determine if
			//  there is a line of sight to that actor
			for (i = 0; i < taa.actors; i++) {
				if (actorArray[i]->isDead()) continue;

				if (tracking()
				        ||  a->canSenseSpecificActor(
				            info,
				            maxSenseRange,
				            actorArray[i])
				        ||  a->canSenseSpecificActorIndirectly(
				            info,
				            maxSenseRange,
				            actorArray[i])) {
					bestTarget = actorArray[i];
					break;
				}
			}
			break;

		case behaviorCowardly: {
			int16       bestScore = 0;

			for (i = 0; i < taa.actors; i++) {
				if (actorArray[i]->isDead()) continue;

				if (tracking()
				        ||  a->canSenseSpecificActor(
				            info,
				            maxSenseRange,
				            actorArray[i])
				        ||  a->canSenseSpecificActorIndirectly(
				            info,
				            maxSenseRange,
				            actorArray[i])) {
					int16   score;

					score =     closenessScore(distArray[i]) * 16
					            /   actorArray[i]->defenseScore();

					if (score > bestScore || bestTarget == NULL) {
						bestScore = score;
						bestTarget = actorArray[i];
					}
				}
			}
		}
		break;

		case behaviorBerserk: {
			int16       bestScore = 0;

			for (i = 0; i < taa.actors; i++) {
				if (actorArray[i]->isDead()) continue;

				if (tracking()
				        ||  a->canSenseSpecificActor(
				            info,
				            maxSenseRange,
				            actorArray[i])
				        ||  a->canSenseSpecificActorIndirectly(
				            info,
				            maxSenseRange,
				            actorArray[i])) {
					int16   score;

					score =     closenessScore(distArray[i])
					            *   actorArray[i]->offenseScore();

					if (score > bestScore || bestTarget == NULL) {
						bestScore = score;
						bestTarget = actorArray[i];
					}
				}
			}
		}
		break;

		case behaviorSmart: {
			int16       bestScore = 0;

			for (i = 0; i < taa.actors; i++) {
				if (actorArray[i]->isDead()) continue;

				if (tracking()
				        ||  a->canSenseSpecificActor(
				            info,
				            maxSenseRange,
				            actorArray[i])
				        ||  a->canSenseSpecificActorIndirectly(
				            info,
				            maxSenseRange,
				            actorArray[i])) {
					int16   score;

					score =     closenessScore(distArray[i])
					            *   actorArray[i]->offenseScore()
					            /   actorArray[i]->defenseScore();

					if (score > bestScore || bestTarget == NULL) {
						bestScore = score;
						bestTarget = actorArray[i];
					}
				}
			}
		}
		break;
		}

		if (bestTarget != currentTarget) {
			//  If the current target has changed, abort any
			//  action currently taking place
			if (atTarget()) atTargetabortTask();
			currentTarget = bestTarget;
			a->currentTarget = currentTarget;
		}

		flags |= evalWeapon;

		targetEvaluateCtr = targetEvaluateRate;
	}

	//  Decrement the target reevaluation counter
	targetEvaluateCtr--;
}

//----------------------------------------------------------------------

bool HuntToKillTask::atTarget(void) {
	//  Determine if we're in attack range of the current target
	return      currentTarget != NULL
	            &&  stack->getActor()->inAttackRange(
	                currentTarget->getLocation());
}

//----------------------------------------------------------------------

void HuntToKillTask::atTargetabortTask(void) {
	//  If the task is aborted while at the target actor, abort any
	//  attack currently taking place
	stack->getActor()->stopAttack(currentTarget);
}

//----------------------------------------------------------------------

TaskResult HuntToKillTask::atTargetEvaluate(void) {
	//  This task is never done and must be aborted manually
	return taskNotDone;
}

//----------------------------------------------------------------------

TaskResult HuntToKillTask::atTargetUpdate(void) {
	assert(isActor(currentTarget));

	Actor   *a = stack->getActor();

	//  If we're ready to attack, attack
	if (a->isInterruptable() && g_vm->_rnd->getRandomNumber(6) == 0) {
		a->attack(currentTarget);
		flags |= evalWeapon;
	}

	return taskNotDone;
}

//----------------------------------------------------------------------

void HuntToKillTask::evaluateWeapon(void) {
	Actor               *a = stack->getActor();
	ObjectID            actorID = a->thisID();
	GameObject          *obj,
	                    *bestWeapon,
	                    *currentWeapon;
	int                 bestWeaponRating;
	ContainerIterator   iter(a);

	bestWeapon = NULL;
	bestWeaponRating = 0;
	currentWeapon = a->offensiveObject();
	//  If the current offensive object is the actor himself then there
	//  is no current weapon.
	if (currentWeapon == a) currentWeapon = NULL;

	if (!isAutoWeaponSet() && isPlayerActor(a)) {
		WeaponProto     *weaponProto =  currentWeapon != NULL
		                                ? (WeaponProto *)currentWeapon->proto()
		                                :   NULL;

		if (currentWeapon == NULL
		        ||      weaponProto->weaponRating(
		            a->thisID(),
		            actorID,
		            currentTarget->thisID())
		        !=  0)
			return;
	}

	while (iter.next(&obj) != Nothing) {
		ProtoObj        *proto = obj->proto();
		uint16          cSet = proto->containmentSet();

		//  Simply use all armor objects
		if (!isPlayerActor(a) && (cSet & ProtoObj::isArmor)) {
			if (proto->useSlotAvailable(obj, a))
				obj->use(actorID);
			continue;
		}

		if (cSet & ProtoObj::isWeapon) {
			WeaponProto     *weaponProto = (WeaponProto *)proto;
			int             weaponRating;

			weaponRating =  weaponProto->weaponRating(
			                    obj->thisID(),
			                    actorID,
			                    currentTarget->thisID());

			//  a rating of zero means this weapon is useless
			if (weaponRating == 0) continue;

			if (obj == currentWeapon)
				weaponRating += currentWeaponBonus;

			if (weaponRating > bestWeaponRating) {
				bestWeaponRating = weaponRating;
				bestWeapon = obj;
			}
		}
	}

	if (bestWeapon != NULL) {
		if (bestWeapon != currentWeapon)
			bestWeapon->use(actorID);
	}
	//  If there is no useful best weapon and the actor is currently
	//  wielding a weapon, un-wield the weapon
	else if (currentWeapon != NULL)
		currentWeapon->use(actorID);
}

/* ===================================================================== *
   HuntToGiveTask member functions
 * ===================================================================== */

//	Hunt to give is not implemented yet

HuntToGiveTask::HuntToGiveTask(Common::InSaveFile *in, TaskID id) : HuntActorTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading HuntToGiveTask");

	//  Get the object ID
	ObjectID objToGiveID = in->readUint16LE();

	//  Convert the object ID to a pointer
	objToGive = objToGiveID != Nothing
	            ?   GameObject::objectAddress(objToGiveID)
	            :   NULL;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntToGiveTask::archiveSize(void) const {
	return      HuntActorTask::archiveSize()
	            +   sizeof(ObjectID);                //  objToGive ID
}

void HuntToGiveTask::write(Common::OutSaveFile *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToGiveTask");

	//  Let base class archive its data
	HuntActorTask::write(out);

	//  Store the ID
	if (objToGive != NULL)
		out->writeUint16LE(objToGive->thisID());
	else
		out->writeUint16LE(Nothing);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 HuntToGiveTask::getType(void) const {
	return huntToGiveTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool HuntToGiveTask::operator == (const Task &t) const {
	if (t.getType() != huntToGiveTask) return false;

	const HuntToGiveTask *taskPtr = (const HuntToGiveTask *)&t;

	return      *getTarget() == *taskPtr->getTarget()
	            &&  tracking() ? taskPtr->tracking() : !taskPtr->tracking()
	            &&  objToGive == taskPtr->objToGive;
}

//----------------------------------------------------------------------

void HuntToGiveTask::evaluateTarget(void) {}

//----------------------------------------------------------------------

bool HuntToGiveTask::atTarget(void) {
	return false;
}

//----------------------------------------------------------------------

void HuntToGiveTask::atTargetabortTask(void) {}

//----------------------------------------------------------------------

TaskResult HuntToGiveTask::atTargetEvaluate(void) {
	return taskNotDone;
}

//----------------------------------------------------------------------

TaskResult HuntToGiveTask::atTargetUpdate(void) {
	return taskNotDone;
}

/* ===================================================================== *
   BandTask member functions
 * ===================================================================== */

//----------------------------------------------------------------------

bool BandTask::BandingRepulsorIterator::first(
    TilePoint   &repulsorVector,
    int16       &repulsorStrength) {
	assert(a->leader != NULL && a->leader->followers != NULL);

	band = a->leader->followers;
	bandIndex = 0;

	while (bandIndex < band->size()) {
		Actor       *bandMember = (*band)[bandIndex];

		if (bandMember != a) {
			repulsorVector = bandMember->getLocation() - a->getLocation();
			repulsorStrength = 1;

			return true;
		}

		bandIndex++;
	}

	return false;
}

//----------------------------------------------------------------------

bool BandTask::BandingRepulsorIterator::next(
    TilePoint   &repulsorVector,
    int16       &repulsorStrength) {
	assert(a->leader != NULL && a->leader->followers != NULL);
	assert(band == a->leader->followers);
	assert(bandIndex < band->size());

	bandIndex++;
	while (bandIndex < band->size()) {
		Actor       *bandMember = (*band)[bandIndex];

		if (bandMember != a) {
			repulsorVector = bandMember->getLocation() - a->getLocation();
			repulsorStrength = 1;

			return true;
		}

		bandIndex++;
	}

	return false;
}

BandTask::BandTask(Common::InSaveFile *in, TaskID id) : HuntTask(in, id) {
	TaskID attendID = in->readSint16LE();
	debugC(3, kDebugSaveload, "... Loading BandTask");


	//  Convert the TaskID to a Task pointer
	attend = attendID != NoTask
	         ? (AttendTask *)getTaskAddress(attendID)
	         :   NULL;

	//  Restore the current target location
	currentTarget.load(in);

	//  Restore the target evaluation counter
	targetEvaluateCtr = in->readByte();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 BandTask::archiveSize(void) const {
	return      HuntTask::archiveSize()
	            +   sizeof(TaskID)           //  attend ID
	            +   sizeof(currentTarget)
	            +   sizeof(targetEvaluateCtr);
}

void BandTask::write(Common::OutSaveFile *out) const {
	debugC(3, kDebugSaveload, "... Saving BandTask");

	//  Let the base class archive its data
	HuntTask::write(out);

	//  Store the attend task ID
	if (attend != NULL)
		out->writeSint16LE(getTaskID(attend));
	else
		out->writeSint16LE(NoTask);

	//  Store the current target location
	currentTarget.write(out);

	//  Store the target evaluation counter
	out->writeByte(targetEvaluateCtr);
}

#if DEBUG
//----------------------------------------------------------------------
//	Debugging function used to mark this task and any sub tasks as being
//	used.  This is used to find task leaks.

void BandTask::mark(void) {
	HuntTask::mark();
	if (attend != NULL)
		attend->mark();
}
#endif

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 BandTask::getType(void) const {
	return bandTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool BandTask::operator == (const Task &t) const {
	return t.getType() == bandTask;
}

//----------------------------------------------------------------------

void BandTask::evaluateTarget(void) {
	if (targetEvaluateCtr == 0) {
		Actor           *leader = stack->getActor()->leader;
		TilePoint       actorLoc = stack->getActor()->getLocation(),
		                movementVector;
		TilePoint       repulsorVector;
		int16           repulsorStrength;
		TilePoint       repulsorVectorArray[6];
		int16           repulsorStrengthArray[ARRAYSIZE(repulsorVectorArray)];
		int16           repulsorDistArray[ARRAYSIZE(repulsorVectorArray)];
		int16           repulsorCount;
		bool            repulsorFlag;

		RepulsorIterator    *repulsorIter = getNewRepulsorIterator();

		if (repulsorIter == NULL) return;

		//  Count the leader as two band members to double his
		//  repulsion
		repulsorVectorArray[0] = leader->getLocation() - actorLoc;
		repulsorStrengthArray[0] = 3;
		repulsorDistArray[0] = repulsorVectorArray[0].quickHDistance();
		repulsorCount = 1;

		//  Iterate through the band members, adding their locations
		//  to the repulsor array sorted by distance.
		for (repulsorFlag = repulsorIter->first(
		                        repulsorVector,
		                        repulsorStrength);
		        repulsorFlag;
		        repulsorFlag =  repulsorIter->next(
		                            repulsorVector,
		                            repulsorStrength)) {
			int16           repulsorDist = repulsorVector.quickHDistance();
			int16           j = repulsorCount;

			if (repulsorDist < repulsorDistArray[j - 1]) {
				if (repulsorCount < (long)ARRAYSIZE(repulsorVectorArray)) {
					repulsorDistArray[j] = repulsorDistArray[j - 1];
					repulsorVectorArray[j] = repulsorVectorArray[j - 1];
					repulsorStrengthArray[j] = repulsorStrengthArray[j - 1];
				}
				j--;
			}

			while (j > 0 && repulsorDist < repulsorDistArray[j - 1]) {
				repulsorDistArray[j] = repulsorDistArray[j - 1];
				repulsorVectorArray[j] = repulsorVectorArray[j - 1];
				repulsorStrengthArray[j] = repulsorStrengthArray[j - 1];
				j--;
			}

			if (j < (long)ARRAYSIZE(repulsorVectorArray)) {
				if (repulsorCount < (long)ARRAYSIZE(repulsorVectorArray))
					repulsorCount++;
				repulsorDistArray[j] = repulsorDist;
				repulsorVectorArray[j] = repulsorVector;
				repulsorStrengthArray[j] = repulsorStrength;
			}
		}

		delete repulsorIter;

		//  Compute the target location
		movementVector = (leader->getLocation() - actorLoc)
		                 +   computeRepulsionVector(
		                     repulsorVectorArray,
		                     repulsorStrengthArray,
		                     repulsorCount);

		currentTarget = actorLoc + movementVector;
		currentTarget.z = leader->getLocation().z;

		targetEvaluateCtr = targetEvaluateRate;
	}

	targetEvaluateCtr--;
}

//----------------------------------------------------------------------

bool BandTask::targetHasChanged(GotoTask *gotoTarget) {
	GotoLocationTask    *gotoLocation = (GotoLocationTask *)gotoTarget;
	TilePoint           actorLoc = stack->getActor()->getLocation(),
	                    oldTarget = gotoLocation->getTarget();
	int16               slop;

	slop = ((currentTarget - actorLoc).quickHDistance()
	        +   ABS(currentTarget.z - actorLoc.z))
	       /   2;

	if ((currentTarget - oldTarget).quickHDistance()
	        +   ABS(currentTarget.z - oldTarget.z)
	        >   slop)
		gotoLocation->changeTarget(currentTarget);

	return false;
}

//----------------------------------------------------------------------

GotoTask *BandTask::setupGoto(void) {
	return new GotoLocationTask(stack, currentTarget, getRunThreshold());
}

//----------------------------------------------------------------------

TilePoint BandTask::currentTargetLoc(void) {
	return currentTarget;
}

//----------------------------------------------------------------------

bool BandTask::atTarget(void) {
	TilePoint       actorLoc = stack->getActor()->getLocation();

	if ((actorLoc - currentTarget).quickHDistance() > 6
	        ||  ABS(actorLoc.z - currentTarget.z) > kMaxStepHeight) {
		if (attend != NULL) {
			attend->abortTask();
			delete attend;
			attend = NULL;
		}

		return false;
	}

	return true;
}

//----------------------------------------------------------------------

void BandTask::atTargetabortTask(void) {
	if (attend != NULL) {
		attend->abortTask();
		delete attend;
		attend = NULL;
	}
}

//----------------------------------------------------------------------

TaskResult BandTask::atTargetEvaluate(void) {
	return taskNotDone;
}

//----------------------------------------------------------------------

TaskResult BandTask::atTargetUpdate(void) {
	Actor       *a = stack->getActor();

	if (attend != NULL)
		attend->update();
	else {
		attend = new AttendTask(stack, a->leader);
		if (attend != NULL)
			attend->update();
	}

	return taskNotDone;
}

//----------------------------------------------------------------------

int16 BandTask::getRunThreshold(void) {
	return kTileUVSize * 3;
}

//----------------------------------------------------------------------

BandTask::RepulsorIterator *BandTask::getNewRepulsorIterator(void) {
	return new BandingRepulsorIterator(stack->getActor());
}

/* ===================================================================== *
   BandAndAvoidEnemiesTask member functions
 * ===================================================================== */

//----------------------------------------------------------------------

//bool BandAndAvoidEnemiesTask::BandAndAvoidEnemiesRepulsorIterator::firstEnemyRepulsor(
bool BandTask::BandAndAvoidEnemiesRepulsorIterator::firstEnemyRepulsor(
    TilePoint   &repulsorVector,
    int16       &repulsorStrength) {
	assert(iteratingThruEnemies);

	int16                   actorDistArray[ARRAYSIZE(actorArray)];
	TargetActorArray        taa(ARRAYSIZE(actorArray), actorArray, actorDistArray);
	ActorPropertyTarget     target(actorPropIDEnemy);

	numActors = target.actor(a->world(), a->getLocation(), taa);

	assert(numActors == taa.actors);

	actorIndex = 0;

	if (actorIndex < numActors) {
		repulsorVector =
		    actorArray[actorIndex]->getLocation() - a->getLocation();
		repulsorStrength = 6;

		return true;
	}

	return false;
}

//----------------------------------------------------------------------

//bool BandAndAvoidEnemiesTask::BandAndAvoidEnemiesRepulsorIterator::nextEnemyRepulsor(
bool BandTask::BandAndAvoidEnemiesRepulsorIterator::nextEnemyRepulsor(
    TilePoint   &repulsorVector,
    int16       &repulsorStrength) {
	assert(iteratingThruEnemies);

	actorIndex++;

	if (actorIndex < numActors) {
		repulsorVector =
		    actorArray[actorIndex]->getLocation() - a->getLocation();
		repulsorStrength = 6;

		return true;
	}

	return false;
}

//----------------------------------------------------------------------

//bool BandAndAvoidEnemiesTask::BandAndAvoidEnemiesRepulsorIterator::first(
bool BandTask::BandAndAvoidEnemiesRepulsorIterator::first(
    TilePoint   &repulsorVector,
    int16       &repulsorStrength) {
	iteratingThruEnemies = false;

	if (BandingRepulsorIterator::first(repulsorVector, repulsorStrength))
		return true;

	iteratingThruEnemies = true;
	return firstEnemyRepulsor(repulsorVector, repulsorStrength);
}

//----------------------------------------------------------------------

//bool BandAndAvoidEnemiesTask::BandAndAvoidEnemiesRepulsorIterator::first(
bool BandTask::BandAndAvoidEnemiesRepulsorIterator::next(
    TilePoint   &repulsorVector,
    int16       &repulsorStrength) {
	if (!iteratingThruEnemies) {
		if (BandingRepulsorIterator::next(repulsorVector, repulsorStrength))
			return true;

		iteratingThruEnemies = true;
		return firstEnemyRepulsor(repulsorVector, repulsorStrength);
	}

	return nextEnemyRepulsor(repulsorVector, repulsorStrength);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 BandAndAvoidEnemiesTask::getType(void) const {
	return bandAndAvoidEnemiesTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool BandAndAvoidEnemiesTask::operator == (const Task &t) const {
	return t.getType() == bandAndAvoidEnemiesTask;
}

//----------------------------------------------------------------------

int16 BandAndAvoidEnemiesTask::getRunThreshold(void) {
	return 0;
}

//----------------------------------------------------------------------

BandTask::RepulsorIterator *BandAndAvoidEnemiesTask::getNewRepulsorIterator(void) {
	return new BandAndAvoidEnemiesRepulsorIterator(stack->getActor());
}

/* ===================================================================== *
   FollowPatrolRouteTask member functions
 * ===================================================================== */

FollowPatrolRouteTask::FollowPatrolRouteTask(Common::InSaveFile *in, TaskID id) : Task(in, id) {
	debugC(3, kDebugSaveload, "... Loading FollowPatrolRouteTask");

	//  Get the gotoWayPoint TaskID
	TaskID gotoWayPointID = in->readSint16LE();

	//  Convert the TaskID to a Task pointer
	gotoWayPoint = gotoWayPointID != NoTask
	               ? (GotoLocationTask *)getTaskAddress(gotoWayPointID)
	               :   NULL;

	//  Restore the patrol route iterator
	patrolIter.read(in);

	//  Restore the last waypoint number
	lastWayPointNum = in->readSint16LE();

	//  Restore the paused flag
	paused = in->readByte();

	//  Restore the paused counter
	counter = in->readSint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 FollowPatrolRouteTask::archiveSize(void) const {
	return      Task::archiveSize()
	            +   sizeof(TaskID)   //  gotoWayPoint ID
	            +   sizeof(patrolIter)
	            +   sizeof(lastWayPointNum)
	            +   sizeof(paused)
	            +   sizeof(counter);
}

void FollowPatrolRouteTask::write(Common::OutSaveFile *out) const {
	debugC(3, kDebugSaveload, "... Saving FollowPatrolRouteTask");

	//  Let the base class archive its data
	Task::write(out);

	//  Store the gotoWayPoint ID
	if (gotoWayPoint != NULL)
		out->writeSint16LE(getTaskID(gotoWayPoint));
	else
		out->writeSint16LE(NoTask);

	//  Store the PatrolRouteIterator
	patrolIter.write(out);

	//  Store the last waypoint number
	out->writeSint16LE(lastWayPointNum);

	//  Store the paused flag
	out->writeByte(paused);

	//  Store the paused counter
	out->writeSint16LE(counter);
}

#if DEBUG
//----------------------------------------------------------------------
//	Debugging function used to mark this task and any sub tasks as being
//	used.  This is used to find task leaks.

void FollowPatrolRouteTask::mark(void) {
	Task::mark();
	if (gotoWayPoint != NULL)
		gotoWayPoint->mark();
}
#endif

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 FollowPatrolRouteTask::getType(void) const {
	return followPatrolRouteTask;
}

//----------------------------------------------------------------------

void FollowPatrolRouteTask::abortTask(void) {
	//  If there is a subtask, get rid of it
	if (gotoWayPoint) {
		gotoWayPoint->abortTask();
		delete gotoWayPoint;
		gotoWayPoint = NULL;
	}
}

//----------------------------------------------------------------------

TaskResult FollowPatrolRouteTask::evaluate(void) {
	//  Simply check the patrol iterator to determine if there are
	//  any more waypoints
	return *patrolIter == Nowhere ? taskSucceeded : taskNotDone;
}

//----------------------------------------------------------------------

TaskResult FollowPatrolRouteTask::update(void) {
	return !paused ? handleFollowPatrolRoute() : handlePaused();
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool FollowPatrolRouteTask::operator == (const Task &t) const {
	if (t.getType() != followPatrolRouteTask) return false;

	const FollowPatrolRouteTask *taskPtr = (const FollowPatrolRouteTask *)&t;

	return      patrolIter == taskPtr->patrolIter
	            &&  lastWayPointNum == taskPtr->lastWayPointNum;
}

//----------------------------------------------------------------------
//	Update function used if this task is not paused

TaskResult FollowPatrolRouteTask::handleFollowPatrolRoute(void) {
	TilePoint   currentWayPoint = *patrolIter,
	            actorLoc = stack->getActor()->getLocation();

	if (currentWayPoint == Nowhere) return taskSucceeded;

	//  Determine if the actor has reached the waypoint tile position
	if ((actorLoc.u >> kTileUVShift)
	        == (currentWayPoint.u >> kTileUVShift)
	        && (actorLoc.v >> kTileUVShift)
	        == (currentWayPoint.v >> kTileUVShift)
	        &&  ABS(actorLoc.z - currentWayPoint.z) <= kMaxStepHeight) {
		//  Delete the gotoWayPoint task
		if (gotoWayPoint != NULL) {
			gotoWayPoint->abortTask();
			delete gotoWayPoint;
			gotoWayPoint = NULL;
		}

		//  If this way point is the specified last way point,
		//  return success
		if (lastWayPointNum != -1
		        &&  patrolIter.wayPointNum() == lastWayPointNum)
			return taskSucceeded;

		//  If there are no more way points in the patrol route, return
		//  success
		if ((currentWayPoint = *++patrolIter) == Nowhere)
			return taskSucceeded;

		//  We are at a way point so randomly determine if we should
		//  pause for a while.
		if (g_vm->_rnd->getRandomNumber(3) == 0) {
			pause();
			return taskNotDone;
		}
	}

	//  Setup a gotoWayPoint task if one doesn't already exist and
	//  update it
	if (gotoWayPoint != NULL)
		gotoWayPoint->update();
	else {
		gotoWayPoint = new GotoLocationTask(stack, currentWayPoint);
		if (gotoWayPoint != NULL) gotoWayPoint->update();
	}

	return taskNotDone;
}

//----------------------------------------------------------------------
//	Update function used if this task is paused

TaskResult FollowPatrolRouteTask::handlePaused(void) {
	TaskResult      result;

	if ((result = evaluate()) == taskNotDone) {
		if (counter == 0)
			followPatrolRoute();
		else
			counter--;
	}

	return result;
}

//----------------------------------------------------------------------
//	Set this task into the paused state

void FollowPatrolRouteTask::pause(void) {
	paused = true;
	counter = (g_vm->_rnd->getRandomNumber(63) + g_vm->_rnd->getRandomNumber(63)) / 2;
}

/* ===================================================================== *
   AttendTask member functions
 * ===================================================================== */

AttendTask::AttendTask(Common::InSaveFile *in, TaskID id) : Task(in, id) {
	debugC(3, kDebugSaveload, "... Loading AttendTask");

	//  Get the object ID
	ObjectID objID = in->readUint16LE();

	//  Convert the object ID to a pointer
	obj = objID != Nothing
		? GameObject::objectAddress(objID)
		: NULL;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 AttendTask::archiveSize(void) const {
	return Task::archiveSize() + sizeof(ObjectID);
}


void AttendTask::write(Common::OutSaveFile *out) const {
	debugC(3, kDebugSaveload, "... Saving AttendTask");

	//  Let the base class archive its data
	Task::write(out);

	//  Store the object ID
	if (obj != NULL)
		out->writeUint16LE(obj->thisID());
	else
		out->writeUint16LE(Nothing);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 AttendTask::getType(void) const {
	return attendTask;
}

//----------------------------------------------------------------------

void AttendTask::abortTask(void) {
	MotionTask  *actorMotion = stack->getActor()->moveTask;

	//  Determine if we need to abort the actor motion
	if (actorMotion != NULL && actorMotion->isTurn())
		actorMotion->finishTurn();
}

//----------------------------------------------------------------------

TaskResult AttendTask::evaluate(void) {
	//  Attending must be stopped manually
	return taskNotDone;
}

//----------------------------------------------------------------------

TaskResult AttendTask::update(void) {
	Actor       *a = stack->getActor();
	TilePoint   attendLoc = obj->getWorldLocation();

	//  Determine if we are facing the object
	if (a->currentFacing != (attendLoc - a->getLocation()).quickDir()) {
		//  If not, turn
		if (!a->moveTask || !a->moveTask->isTurn())
			MotionTask::turnTowards(*a, attendLoc);
	}

	return taskNotDone;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool AttendTask::operator == (const Task &t) const {
	if (t.getType() != attendTask) return false;

	const AttendTask *taskPtr = (const AttendTask *)&t;

	return obj == taskPtr->obj;
}

/* ===================================================================== *
   TaskStack member functions
 * ===================================================================== */

void TaskStack::write(Common::OutSaveFile *out) {
	//  Store the stack bottom TaskID
	out->writeSint16LE(stackBottomID);

	//  Store the actor's id
	out->writeUint16LE(actor->thisID());

	//  Store the evalCount and evalRate
	out->writeSint16LE(evalCount);

	out->writeSint16LE(evalRate);

	debugC(4, kDebugSaveload, "...... stackBottomID = %d", stackBottomID);
	debugC(4, kDebugSaveload, "...... actorID = %d", actor->thisID());
	debugC(4, kDebugSaveload, "...... evalCount = %d", evalCount);
	debugC(4, kDebugSaveload, "...... evalRate = %d", evalRate);
}

void TaskStack::read(Common::InSaveFile *in) {
	ObjectID actorID;

	//  Restore the stack bottom pointer
	stackBottomID = in->readSint16LE();

	//  Restore the actor pointer
	actorID = in->readUint16LE();
	actor = (Actor *)GameObject::objectAddress(actorID);

	//  Restore the evaluation count
	evalCount = in->readSint16LE();

	//  Restore the evaluation rate
	evalRate = in->readSint16LE();

	debugC(4, kDebugSaveload, "...... stackBottomID = %d", stackBottomID);
	debugC(4, kDebugSaveload, "...... actorID = %d", actorID);
	debugC(4, kDebugSaveload, "...... evalCount = %d", evalCount);
	debugC(4, kDebugSaveload, "...... evalRate = %d", evalRate);
}

#if DEBUG
//----------------------------------------------------------------------
//	Debugging function used to mark this task and any sub tasks as being
//	used.  This is used to find task leaks.

void TaskStack::mark(void) {
	if (stackBottomID != NoTask) {
		Task    *stackBottom = getTaskAddress(stackBottomID);

		stackBottom->mark();
	}
}
#endif

//----------------------------------------------------------------------
//	Set the bottom task of this task stack

void TaskStack::setTask(Task *t) {
	assert(stackBottomID == NoTask);

	if (t->stack == this) {
		TaskID      id = getTaskID(t);

		stackBottomID = id;
	}
}

//----------------------------------------------------------------------
//  Abort all tasks in stack

void TaskStack::abortTask(void) {
	if (stackBottomID != NoTask) {
		Task    *stackBottom = getTaskAddress(stackBottomID);

		stackBottom->abortTask();
		delete stackBottom;
	}
}

//----------------------------------------------------------------------
//  Re-evaluate tasks in stack

TaskResult TaskStack::evaluate(void) {
	if (stackBottomID != -1) {
		Task    *stackBottom = getTaskAddress(stackBottomID);

		return stackBottom->evaluate();
	} else
		return taskNotDone;
}

//----------------------------------------------------------------------
//  Update the state of the tasks in stack

TaskResult TaskStack::update(void) {
	TaskResult  result;

	//  If the actor is currently uniterruptable then this task is paused
	if (!actor->isInterruptable()) return taskNotDone;

	if (stackBottomID != NoTask) {
		Task    *stackBottom = getTaskAddress(stackBottomID);

		//  Determine if it is time to reevaluate the tasks
		if (--evalCount == 0) {
			if ((result = stackBottom->evaluate()) != taskNotDone) {
				delete stackBottom;
				stackBottomID = NoTask;

				return result;
			}
			evalCount = evalRate;
		}

		//  Update the tasks
		if ((result = stackBottom->update()) != taskNotDone) {
			delete stackBottom;
			stackBottomID = NoTask;

			return result;
		}
	} else
		return taskFailed;

	return taskNotDone;
}

} // end of namespace Saga2
