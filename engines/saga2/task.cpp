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

void writeTask(Task *t, Common::MemoryWriteStreamDynamic *out);

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
	TaskStackList();

	//  Destructor
	~TaskStackList();

	void read(Common::InSaveFile *in);

	//  Return the number of bytes needed to make an archive of the
	//  TaskStackList
	int32 archiveSize();

	void write(Common::MemoryWriteStreamDynamic *out);

	//  Place a TaskStack from the inactive list into the active
	//  list.
	TaskStack *newTaskStack(Actor *a);

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
	void updateTaskStacks();
};

//----------------------------------------------------------------------
//	TaskStackList constructor -- simply place each element the array in
//	the inactive list

TaskStackList::TaskStackList() {
	for (int i = 0; i < numTaskStacks; i++)
		_list[i] = nullptr;
}

//----------------------------------------------------------------------
//	TaskStackList destructor

TaskStackList::~TaskStackList() {
	for (int i = 0; i < numTaskStacks; i++) {
		if (_list[i] == nullptr)
			continue;

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
		ts->getActor()->_curTask = ts;
	}
}

//----------------------------------------------------------------------
//	Return the number of bytes necessary to archive this TaskStackList

int32 TaskStackList::archiveSize() {
	int32 size = sizeof(int16);

	for (int i = 0; i < numTaskStacks; i++) {
		size += sizeof(TaskStackID);

		if (_list[i])
			size +=  _list[i]->archiveSize();
	}

	return size;
}

void TaskStackList::write(Common::MemoryWriteStreamDynamic *out) {
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

TaskStack *TaskStackList::newTaskStack(Actor *a) {
	for (int i = 0; i < numTaskStacks; i++)
		if (!_list[i]) {
			_list[i] = new TaskStack(a);

			return _list[i];
		}

	warning("Too many task stacks in the list, > %d", numTaskStacks);

	return nullptr;
}

void TaskStackList::newTaskStack(TaskStack *p) {
	for (int i = 0; i < numTaskStacks; i++) {
		if (_list[i] == p) {
			warning("TaskStack %d (%p) already added", i, (void *)p);

			return;
		}
	}

	debugC(1, kDebugTasks, "List: %p Adding task stack %p", (void *)this, (void *)p);
	for (int i = 0; i < numTaskStacks; i++) {
		if (!_list[i]) {
			_list[i] = p;

			return;
		}
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
	debugC(1, kDebugTasks, "List: %p Deleting task stack %p", (void *)this, (void *)p);
	for (int i = 0; i < numTaskStacks; i++) {
		if (_list[i] == p) {
			_list[i] = nullptr;
		}
	}
}

//----------------------------------------------------------------------
//	Iterate through all of the TaskStacks in the active list and call
//	their update function

void TaskStackList::updateTaskStacks() {
	for (int i = 0; i < numTaskStacks; i++) {
		if (_list[i]) {
			TaskStack *ts = _list[i];
			TaskResult  result;

			//  Update the task stack and delete it if it is done
			if ((result = ts->update()) != taskNotDone) {
				Actor *a = ts->getActor();
				assert(a != nullptr);

				a->handleTaskCompletion(result);
			}
		}
	}
}

/* ===================================================================== *
   Misc. task stack management functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Simply pass this call to the stackList member function,
//	updateTaskStacks().

void updateActorTasks() {
	if (!actorTasksPaused)
		g_vm->_stackList->updateTaskStacks();
}

void pauseActorTasks() {
	actorTasksPaused = true;
}
void resumeActorTasks() {
	actorTasksPaused = false;
}

//----------------------------------------------------------------------
//	Call the stackList member function newTaskStack() to get a pointer
//	to a new TaskStack

TaskStack *newTaskStack(Actor *a) {
	return g_vm->_stackList->newTaskStack(a);
}

void newTaskStack(TaskStack *p) {
	return g_vm->_stackList->newTaskStack(p);
}

//----------------------------------------------------------------------
//	Call the stackList member function deleteTaskStack() to dispose of
//	a previously allocated TaskStack

void deleteTaskStack(TaskStack *p) {
	g_vm->_stackList->deleteTaskStack(p);
}

//----------------------------------------------------------------------
//	Return the specified TaskStack's ID

TaskStackID getTaskStackID(TaskStack *ts) {
	return g_vm->_stackList->getTaskStackID(ts);
}

//----------------------------------------------------------------------
//	Return a pointer to a TaskStack given a TaskStackID

TaskStack *getTaskStackAddress(TaskStackID id) {
	return g_vm->_stackList->getTaskStackAddress(id);
}

//----------------------------------------------------------------------
//	Initialize the stackList

void initTaskStacks() {
	g_vm->_stackList = new TaskStackList;
}

void saveTaskStacks(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving Task Stacks");

	outS->write("TSTK", 4);
	CHUNK_BEGIN;
	g_vm->_stackList->write(out);
	CHUNK_END;
}

void loadTaskStacks(Common::InSaveFile *in, int32 chunkSize) {
	debugC(2, kDebugSaveload, "Loading Task Stacks");

	//  If there is no saved data, simply call the default constructor
	if (chunkSize == 0) {
		g_vm->_stackList = new TaskStackList;
		return;
	}

	//  Reconstruct stackList from archived data
	g_vm->_stackList = new TaskStackList;
	g_vm->_stackList->read(in);
}

//----------------------------------------------------------------------
//	Cleanup the stackList

void cleanupTaskStacks() {
	//  Simply call stackList's destructor
	delete g_vm->_stackList;
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
	TaskList();

	//  Destructor
	~TaskList();

	void read(Common::InSaveFile *in);

	//  Return the number of bytes necessary to archive this task list
	//  in a buffer
	int32 archiveSize();

	void write(Common::MemoryWriteStreamDynamic *out);

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

TaskList::TaskList() {
	_size = 0;
	for (int i = 0; i < numTasks; i++)
		_list[i] = nullptr;
}

//----------------------------------------------------------------------
//	TaskList destructor

TaskList::~TaskList() {
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
		debugC(3, kDebugSaveload, "Loading Task %d (%d)", i, id);

		readTask(id, in);
	}

	//	Iterate through the Tasks to fixup the subtask pointers
	for (int i = 0; i < numTasks; ++i) {
		if (_list[i] == nullptr)
			continue;

		_list[i]->fixup();
	}
}

//----------------------------------------------------------------------
//	Return the number of bytes necessary to archive this TaskList

int32 TaskList::archiveSize() {
	int32 size = sizeof(int16);

	for (int i = 0; i < numTasks; i++) {
		size += sizeof(TaskID);

		if (_list[i])
			size += taskArchiveSize(_list[i]);
	}

	return size;
}

void TaskList::write(Common::MemoryWriteStreamDynamic *out) {
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
   Misc. task management functions
 * ===================================================================== */

void newTask(Task *t) {
	return g_vm->_taskList->newTask(t);
}

void newTask(Task *t, TaskID id) {
	return g_vm->_taskList->newTask(t, id);
}

//----------------------------------------------------------------------
//	Call the taskList member function deleteTask() to dispose of a
//	previously allocated TaskStack

void deleteTask(Task *p) {
	g_vm->_taskList->deleteTask(p);
}

//----------------------------------------------------------------------
//	Return the specified Task's ID

TaskID getTaskID(Task *t) {
	return g_vm->_taskList->getTaskID(t);
}

//----------------------------------------------------------------------
//	Return a pointer to a Task given a TaskID

Task *getTaskAddress(TaskID id) {
	return g_vm->_taskList->getTaskAddress(id);
}

//----------------------------------------------------------------------
//	Initialize the taskList

void initTasks() {
	//  Simply call the default constructor for the task list
	g_vm->_taskList = new TaskList;
}

void saveTasks(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving Tasks");

	outS->write("TASK", 4);
	CHUNK_BEGIN;
	g_vm->_taskList->write(out);
	CHUNK_END;
}

void loadTasks(Common::InSaveFile *in, int32 chunkSize) {
	debugC(2, kDebugSaveload, "Loading Tasks");

	//  If there is no saved data, simply call the default constructor
	if (chunkSize == 0) {
		g_vm->_taskList = new TaskList;
		return;
	}

	//  Reconstruct taskList from archived data
	g_vm->_taskList = new TaskList;
	g_vm->_taskList->read(in);
}

//----------------------------------------------------------------------
//	Cleanup the taskList

void cleanupTasks() {
	//  Simply call the taskList's destructor
	delete g_vm->_taskList;
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

void writeTask(Task *t, Common::MemoryWriteStreamDynamic *out) {
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
	_stackID = in->readSint16LE();
	stack = nullptr;
	newTask(this, id);
}

//----------------------------------------------------------------------
//	Fixup the Task pointers

void Task::fixup() {
	//	Convert the stack ID to a stack pointer
	stack = getTaskStackAddress(_stackID);
}

//----------------------------------------------------------------------
//	Return the number of bytes necessary to create an archive of this
//	object's data

inline int32 Task::archiveSize() const {
	return sizeof(TaskStackID);      //  stack's ID
}

void Task::write(Common::MemoryWriteStreamDynamic *out) const {
	out->writeSint16LE(getTaskStackID(stack));
}

/* ===================================================================== *
   WanderTask member functions
 * ===================================================================== */

WanderTask::WanderTask(Common::InSaveFile *in, TaskID id) : Task(in, id) {
	//  Restore the paused flag
	paused = in->readUint16LE();

	//  Restore the counter
	counter = in->readSint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

int32 WanderTask::archiveSize() const {
	return      Task::archiveSize()
	            +   sizeof(paused)
	            +   sizeof(counter);
}

void WanderTask::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Let the base class archive its data
	Task::write(out);

	//  Store the paused flag
	out->writeUint16LE(paused);

	//  Store the counter
	out->writeSint16LE(counter);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 WanderTask::getType() const {
	return wanderTask;
}

//----------------------------------------------------------------------

void WanderTask::abortTask() {
	//  if the actor has a wander motion, abort it
	MotionTask *actorMotion = stack->getActor()->_moveTask;

	if (actorMotion && actorMotion->isWander()) actorMotion->finishWalk();
}

//----------------------------------------------------------------------

TaskResult WanderTask::evaluate() {
	//  Wandering is never done.  It must be stopped manually.
	return taskNotDone;
}

//----------------------------------------------------------------------

TaskResult WanderTask::update() {
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

TaskResult WanderTask::handleWander() {
	MotionTask  *actorMotion = stack->getActor()->_moveTask;

	//  If the actor is not already wandering, start a wander motion
	//  task
	if (!actorMotion
	        ||  !actorMotion->isWander())
		MotionTask::wander(*stack->getActor());

	return taskNotDone;
}

//----------------------------------------------------------------------
//	Set this task into the paused state

void WanderTask::pause() {
	//  Call abort to stop the wandering motion
	abortTask();

	paused = true;
	counter = (g_vm->_rnd->getRandomNumber(63) + g_vm->_rnd->getRandomNumber(63)) / 2;
}

//----------------------------------------------------------------------
//	Set this task into the wander state

void WanderTask::wander() {
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
	_gotoTetherID = in->readSint16LE();
	gotoTether = nullptr;
}

//----------------------------------------------------------------------
//	Fixup the subtask pointers

void TetheredWanderTask::fixup() {
	//	Let the base class fixup it's pointers
	WanderTask::fixup();

	//	Restore the gotoTether pointer
	gotoTether = _gotoTetherID != NoTask
				 ?	(GotoRegionTask *)getTaskAddress(_gotoTetherID)
				 :	nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

inline int32 TetheredWanderTask::archiveSize() const {
	return      WanderTask::archiveSize()
	            +   sizeof(minU)
	            +   sizeof(minU)
	            +   sizeof(minU)
	            +   sizeof(minU)
	            +   sizeof(TaskID);      //  gotoTether ID
}

void TetheredWanderTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving TetheredWanderTask");

	//  Let the base class archive its data
	WanderTask::write(out);

	//  Archive tether coordinates
	out->writeSint16LE(minU);
	out->writeSint16LE(minV);
	out->writeSint16LE(maxU);
	out->writeSint16LE(maxV);

	//  Archive gotoTether ID
	if (gotoTether != nullptr)
		out->writeSint16LE(getTaskID(gotoTether));
	else
		out->writeSint16LE(NoTask);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 TetheredWanderTask::getType() const {
	return tetheredWanderTask;
}

//----------------------------------------------------------------------

void TetheredWanderTask::abortTask() {
	if (gotoTether != nullptr) {
		gotoTether->abortTask();
		delete gotoTether;
		gotoTether = nullptr;
	} else {
		MotionTask *actorMotion = stack->getActor()->_moveTask;

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

TaskResult TetheredWanderTask::handleWander() {
	Actor       *a = stack->getActor();
	TilePoint   actorLoc = a->getLocation();

	if (actorLoc.u < minU || actorLoc.u >= maxU
	        ||  actorLoc.v < minV || actorLoc.v >= maxV) {
		if (gotoTether != nullptr)
			gotoTether->update();
		else {
			gotoTether = new GotoRegionTask(stack, minU, minV, maxU, maxV);
			if (gotoTether != nullptr) gotoTether->update();
		}
	} else {
		if (gotoTether != nullptr) {
			gotoTether->abortTask();
			delete gotoTether;
			gotoTether = nullptr;
		}

		bool            startWander = false;
		TileRegion      motionTether;

		MotionTask  *actorMotion = a->_moveTask;

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
	_wanderID = in->readSint16LE();
	wander = nullptr;

	//  Restore prevRunState
	prevRunState = in->readUint16LE();
}

//----------------------------------------------------------------------
//	Fixup the subtask pointers

void GotoTask::fixup() {
	//	Let the base class fixup its pointers
	Task::fixup();

	//	Convert wanderID to a Task pointer
	wander = _wanderID != NoTask
	         ? (WanderTask *)getTaskAddress(_wanderID)
	         :   nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 GotoTask::archiveSize() const {
	return      Task::archiveSize()
	            +   sizeof(TaskID)       //  wander ID
	            +   sizeof(prevRunState);
}

void GotoTask::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Let the base class archive its data
	Task::write(out);

	//  Convert the wander Task pointer to a TaskID and store it
	//  in the buffer
	if (wander != nullptr)
		out->writeSint16LE(getTaskID(wander));
	else
		out->writeSint16LE(NoTask);

	//  Store prevRunState
	out->writeUint16LE(prevRunState);
}

//----------------------------------------------------------------------

void GotoTask::abortTask() {
	//  If there is a wander subtask, delete it.
	if (wander) {
		wander->abortTask();
		delete wander;
		wander = nullptr;
	} else {
		MotionTask  *actorMotion = stack->getActor()->_moveTask;

		if (actorMotion && actorMotion->isWalk()) actorMotion->finishWalk();
	}
}

//----------------------------------------------------------------------

TaskResult GotoTask::evaluate() {
	//  Determine if we have reach the target.
	if (stack->getActor()->getLocation() == destination()) {
		abortTask();
		return taskSucceeded;
	}

	return taskNotDone;
}

//----------------------------------------------------------------------

TaskResult GotoTask::update() {
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
		if (wander != nullptr) {
			delete wander;
			wander = nullptr;
		}

		//  Determine if there is aready a motion task, and if so,
		//  wether or not it needs to be modified.
		MotionTask  *actorMotion = a->_moveTask;
		TilePoint   actorLoc = a->getLocation();

		if (actorMotion != nullptr && actorMotion->isWalkToDest()) {
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
		if (wander != nullptr)
			wander->update();
		else {
			wander = new WanderTask(stack);
			if (wander != nullptr) wander->update();
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

inline int32 GotoLocationTask::archiveSize() const {
	return      GotoTask::archiveSize()
	            +   sizeof(targetLoc)
	            +   sizeof(runThreshold);
}

void GotoLocationTask::write(Common::MemoryWriteStreamDynamic *out) const {
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

int16 GotoLocationTask::getType() const {
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

TilePoint GotoLocationTask::destination() {
	//  Simply return the target location
	return targetLoc;
}

//----------------------------------------------------------------------

TilePoint GotoLocationTask::intermediateDest() {
	//  GotoLocationTask's never have an intermediate destination
	return targetLoc;
}

//----------------------------------------------------------------------

bool GotoLocationTask::lineOfSight() {
	//  Let's pretend that there is always a line of sight to the
	//  target location
	return true;
}

//----------------------------------------------------------------------

bool GotoLocationTask::run() {
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

inline int32 GotoRegionTask::archiveSize() const {
	return      GotoTask::archiveSize()
	            +   sizeof(regionMinU)
	            +   sizeof(regionMinV)
	            +   sizeof(regionMaxU)
	            +   sizeof(regionMaxV);
}

void GotoRegionTask::write(Common::MemoryWriteStreamDynamic *out) const {
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

int16 GotoRegionTask::getType() const {
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

TilePoint GotoRegionTask::destination() {
	TilePoint   actorLoc = stack->getActor()->getLocation();

	return  TilePoint(
	            clamp(regionMinU, actorLoc.u, regionMaxU - 1),
	            clamp(regionMinV, actorLoc.v, regionMaxV - 1),
	            actorLoc.z);
}

//----------------------------------------------------------------------

TilePoint GotoRegionTask::intermediateDest() {
	return destination();
}

//----------------------------------------------------------------------

bool GotoRegionTask::lineOfSight() {
	return true;
}

//----------------------------------------------------------------------

bool GotoRegionTask::run() {
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

inline int32 GotoObjectTargetTask::archiveSize() const {
	return      GotoTask::archiveSize()
	            +   sizeof(lastTestedLoc)
	            +   sizeof(sightCtr)
	            +   sizeof(flags)
	            +   sizeof(lastKnownLoc);
}

void GotoObjectTargetTask::write(Common::MemoryWriteStreamDynamic *out) const {
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

TilePoint GotoObjectTargetTask::destination() {
	//  Return the object's true location
	return getObject()->getLocation();
}

//----------------------------------------------------------------------

TilePoint GotoObjectTargetTask::intermediateDest() {
	//  Return the last known location
	return lastKnownLoc;
}

//----------------------------------------------------------------------

bool GotoObjectTargetTask::lineOfSight() {
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
	            :   nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 GotoObjectTask::archiveSize() const {
	return GotoObjectTargetTask::archiveSize() + sizeof(ObjectID);
}

void GotoObjectTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving GotoObjectTask");

	//  Let the base class archive its data
	GotoObjectTargetTask::write(out);

	if (targetObj != nullptr)
		out->writeUint16LE(targetObj->thisID());
	else
		out->writeUint16LE(Nothing);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 GotoObjectTask::getType() const {
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

GameObject *GotoObjectTask::getObject() {
	//  Simply return the pointer to the target object
	return targetObj;
}

//----------------------------------------------------------------------

bool GotoObjectTask::run() {
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
	                :   nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 GotoActorTask::archiveSize() const {
	return GotoObjectTargetTask::archiveSize() + sizeof(ObjectID);
}

void GotoActorTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving GotoActorTask");

	//  Let the base class archive its data
	GotoObjectTargetTask::write(out);

	if (targetActor != nullptr)
		out->writeUint16LE(targetActor->thisID());
	else
		out->writeUint16LE(Nothing);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 GotoActorTask::getType() const {
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

GameObject *GotoActorTask::getObject() {
	//  Simply return the pointer to the target actor
	return (GameObject *)targetActor;
}

//----------------------------------------------------------------------

bool GotoActorTask::run() {
	if (isInSight()) {
		TilePoint       actorLoc = stack->getActor()->getLocation(),
		                targetLoc = getTarget()->getLocation();

		return (actorLoc - targetLoc).quickHDistance() >= kTileUVSize * 4;
	} else
		return lastKnownLoc != Nowhere;
}

GoAwayFromTask::GoAwayFromTask(Common::InSaveFile *in, TaskID id) : Task(in, id) {
	//  Get the subtask ID
	_goTaskID = in->readSint16LE();
	goTask = nullptr;

	//  Restore the flags
	flags = in->readByte();
}

//----------------------------------------------------------------------
//	Fixup the subtask pointer

void GoAwayFromTask::fixup() {
		//	Let the base class fixup its pointers
	Task::fixup();

	goTask = _goTaskID != NoTask
	         ? (GotoLocationTask *)getTaskAddress(_goTaskID)
	         :   nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 GoAwayFromTask::archiveSize() const {
	return Task::archiveSize() + sizeof(TaskID) + sizeof(flags);
}

void GoAwayFromTask::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Let the base class archive its data
	Task::write(out);

	//  Store the subTask's ID
	if (goTask != nullptr)
		out->writeSint16LE(getTaskID(goTask));
	else
		out->writeSint16LE(NoTask);

	//  Store the flags
	out->writeByte(flags);
}

//----------------------------------------------------------------------
//	Abort this task

void GoAwayFromTask::abortTask() {
	if (goTask != nullptr) {
		goTask->abortTask();
		delete goTask;
		goTask = nullptr;
	}
}

//----------------------------------------------------------------------
//	Evaluate this task

TaskResult GoAwayFromTask::evaluate() {
	//  Going away is never done, it must be stopped manually
	return taskNotDone;
}

//----------------------------------------------------------------------
//	Update this task

TaskResult GoAwayFromTask::update() {
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
		dest = actorLoc + dirTable_[a->_currentFacing];

	if (goTask != nullptr) {
		if (goTask->getTarget() != dest)
			goTask->changeTarget(dest);

		goTask->update();
	} else {
		if ((goTask =   flags & run
		                ?   new GotoLocationTask(stack, dest, 0)
		                :   new GotoLocationTask(stack, dest))
		        !=  nullptr)
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
		: nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

int32 GoAwayFromObjectTask::archiveSize() const {
	return GoAwayFromTask::archiveSize() + sizeof(ObjectID);
}

void GoAwayFromObjectTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving GoAwayFromObjectTask");

	//  Let the base class archive its data
	GoAwayFromTask::write(out);

	//  Store the object's ID
	if (obj != nullptr)
		out->writeUint16LE(obj->thisID());
	else
		out->writeUint16LE(Nothing);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 GoAwayFromObjectTask::getType() const {
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

TilePoint GoAwayFromObjectTask::getRepulsionVector() {
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

int32 GoAwayFromActorTask::archiveSize() const {
	return GoAwayFromTask::archiveSize() + targetArchiveSize(getTarget());
}

void GoAwayFromActorTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving GoAwayFromActorTask");

	//  Let the base class archive its data
	GoAwayFromTask::write(out);

	//  Store the target
	writeTarget(getTarget(), out);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 GoAwayFromActorTask::getType() const {
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

TilePoint GoAwayFromActorTask::getRepulsionVector() {
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
	subTask = nullptr;

	//  If the flags say we have a sub task, restore it too
	if (huntFlags & (huntGoto | huntWander))
		_subTaskID = in->readSint16LE();
	else
		_subTaskID = NoTask;
}

//----------------------------------------------------------------------
//	Fixup the subtask pointers

void HuntTask::fixup( void ) {
	//	Let the base class fixup its pointers
	Task::fixup();

	if (huntFlags & (huntGoto | huntWander))
		subTask = getTaskAddress(_subTaskID);
	else
		subTask = nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntTask::archiveSize() const {
	int32       size = 0;

	size += Task::archiveSize() + sizeof(huntFlags);
	if (huntFlags & (huntGoto | huntWander)) size += sizeof(TaskID);

	return size;
}

void HuntTask::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Let the base class archive its data
	Task::write(out);

	//  Store the flags
	out->writeByte(huntFlags);

	//  If the flags say we have a sub task, store it too
	if (huntFlags & (huntGoto | huntWander))
		out->writeSint16LE(getTaskID(subTask));
}

//----------------------------------------------------------------------

void HuntTask::abortTask() {
	if (huntFlags & (huntWander | huntGoto)) {
		subTask->abortTask();
		delete subTask;
	}

	//  If we've reached the target call the atTargetabortTask() function
	if (atTarget()) atTargetabortTask();
}

//----------------------------------------------------------------------

TaskResult HuntTask::evaluate() {
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

TaskResult HuntTask::update() {
	Actor       *a = stack->getActor();

	if (a->_moveTask && a->_moveTask->isPrivledged()) return taskNotDone;

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
			if ((gotoResult = setupGoto()) != nullptr) {
				if (huntFlags & huntWander) removeWanderTask();

				subTask = gotoResult;
				huntFlags |= huntGoto;
			} else {
				//  If we couldn't setup a goto task, setup a wander task
				if (!(huntFlags & huntWander)) {
					if ((subTask = new WanderTask(stack)) != nullptr)
						huntFlags |= huntWander;
				}
			}
		}

		//  If there is a subtask, update it
		if ((huntFlags & (huntGoto | huntWander)) && subTask)
			subTask->update();

		//  If we're not at the target, we know the hunt task is not
		//  done
		return taskNotDone;
	}
}

//----------------------------------------------------------------------

void HuntTask::removeWanderTask() {
	subTask->abortTask();
	delete subTask;
	huntFlags &= ~huntWander;
}

//----------------------------------------------------------------------

void HuntTask::removeGotoTask() {
	subTask->abortTask();
	delete subTask;
	subTask = nullptr;
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

inline int32 HuntLocationTask::archiveSize() const {
	return      HuntTask::archiveSize()
	            +   sizeof(currentTarget)
	            +   targetArchiveSize(getTarget());
}

void HuntLocationTask::write(Common::MemoryWriteStreamDynamic *out) const {
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

GotoTask *HuntLocationTask::setupGoto() {
	//  If there is somewhere to go, setup a goto task, else return NULL
	return  currentTarget != Nowhere
	        ?   new GotoLocationTask(stack, currentTarget)
	        :   nullptr;
}

//----------------------------------------------------------------------

TilePoint HuntLocationTask::currentTargetLoc() {
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

inline int32 HuntToBeNearLocationTask::archiveSize() const {
	return      HuntLocationTask::archiveSize()
	            +   sizeof(range)
	            +   sizeof(targetEvaluateCtr);
}

void HuntToBeNearLocationTask::write(Common::MemoryWriteStreamDynamic *out) const {
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

int16 HuntToBeNearLocationTask::getType() const {
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

void HuntToBeNearLocationTask::evaluateTarget() {
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

bool HuntToBeNearLocationTask::atTarget() {
	TilePoint   targetLoc = currentTargetLoc();

	//  Determine if we are within the specified range of the target
	return  targetLoc != Nowhere
	        &&  stack->getActor()->inRange(targetLoc, range);
}

//----------------------------------------------------------------------

void HuntToBeNearLocationTask::atTargetabortTask() {}

//----------------------------------------------------------------------

TaskResult HuntToBeNearLocationTask::atTargetEvaluate() {
	//  If we're at the target, we're done
	return taskSucceeded;
}

//----------------------------------------------------------------------

TaskResult HuntToBeNearLocationTask::atTargetUpdate() {
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
	currentTarget(nullptr) {
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
	                :   nullptr;

	//  Reconstruct the object target
	readTarget(targetMem, in);
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntObjectTask::archiveSize() const {
	return      HuntTask::archiveSize()
	            +   sizeof(ObjectID)
	            +   targetArchiveSize(getTarget());
}

void HuntObjectTask::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Let the base class archive its data
	HuntTask::write(out);

	//  Store the ID
	if (currentTarget != nullptr)
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

GotoTask *HuntObjectTask::setupGoto() {
	//  If there is an object to goto, setup a GotoObjectTask, else
	//  return NULL
	return  currentTarget
	        ?   new GotoObjectTask(stack, currentTarget)
	        :   nullptr;
}

//----------------------------------------------------------------------

TilePoint HuntObjectTask::currentTargetLoc() {
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

inline int32 HuntToBeNearObjectTask::archiveSize() const {
	return      HuntObjectTask::archiveSize()
	            +   sizeof(range)
	            +   sizeof(targetEvaluateCtr);
}

void HuntToBeNearObjectTask::write(Common::MemoryWriteStreamDynamic *out) const {
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

int16 HuntToBeNearObjectTask::getType() const {
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

void HuntToBeNearObjectTask::evaluateTarget() {
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

bool HuntToBeNearObjectTask::atTarget() {
	TilePoint   targetLoc = currentTargetLoc();

	//  Determine if we are within the specified range of the current
	//  target
	return      targetLoc != Nowhere
	            &&  stack->getActor()->inRange(targetLoc, range);
}

//----------------------------------------------------------------------

void HuntToBeNearObjectTask::atTargetabortTask() {}

//----------------------------------------------------------------------

TaskResult HuntToBeNearObjectTask::atTargetEvaluate() {
	//  If we're at the target, we're done
	return taskSucceeded;
}

//----------------------------------------------------------------------

TaskResult HuntToBeNearObjectTask::atTargetUpdate() {
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
	grabFlag = in->readUint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntToPossessTask::archiveSize() const {
	return      HuntObjectTask::archiveSize()
	            +   sizeof(targetEvaluateCtr)
	            +   sizeof(grabFlag);
}

void HuntToPossessTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToPossessTask");

	//  Let the base class archive its data
	HuntObjectTask::write(out);

	//  Store the evaluation counter
	out->writeByte(targetEvaluateCtr);

	//  Store the grab flag
	out->writeUint16LE(grabFlag);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 HuntToPossessTask::getType() const {
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

void HuntToPossessTask::evaluateTarget() {
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

bool HuntToPossessTask::atTarget() {
	Actor   *a = stack->getActor();

	return  currentTarget
	        && (a->inReach(currentTarget->getLocation())
	            || (grabFlag
	                &&  a->isContaining(currentTarget)));
}

//----------------------------------------------------------------------

void HuntToPossessTask::atTargetabortTask() {}

//----------------------------------------------------------------------

TaskResult HuntToPossessTask::atTargetEvaluate() {
	if (currentTarget && stack->getActor()->isContaining(currentTarget))
		return taskSucceeded;

	return taskNotDone;
}

//----------------------------------------------------------------------

TaskResult HuntToPossessTask::atTargetUpdate() {
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
	currentTarget(nullptr) {
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
	                :   nullptr;

	//  Reconstruct the object target
	readTarget(targetMem, in);
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntActorTask::archiveSize() const {
	return      HuntTask::archiveSize()
	            +   sizeof(flags)
	            +   sizeof(ObjectID)
	            +   targetArchiveSize(getTarget());
}

void HuntActorTask::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Let the base class archive its data
	HuntTask::write(out);

	//  Store the flags
	out->writeByte(flags);

	//  Store the ID
	if (currentTarget != nullptr)
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

GotoTask *HuntActorTask::setupGoto() {
	//  If there is an actor to goto, setup a GotoActorTask, else
	//  return NULL
	/*  return  currentTarget
	            ?   new GotoActorTask( stack, currentTarget, flags & track )
	            :   NULL;
	*/
	if (currentTarget != nullptr) {
		return new GotoActorTask(
		           stack,
		           currentTarget,
		           flags & track);
	}

	return nullptr;
}

//----------------------------------------------------------------------

TilePoint HuntActorTask::currentTargetLoc() {
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
	_goAwayID = in->readSint16LE();
	goAway = nullptr;

	//  Restore the range
	range = in->readUint16LE();

	//  Restore the evaluation counter
	targetEvaluateCtr = in->readByte();
}

//----------------------------------------------------------------------
//	Fixup the subtask pointers

void HuntToBeNearActorTask::fixup() {
		//	Let the base class fixup its pointers
	HuntActorTask::fixup();

	//  Convert the task ID to a task pointer
	goAway = _goAwayID != NoTask
	         ? (GoAwayFromObjectTask *)getTaskAddress(_goAwayID)
	         :   nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntToBeNearActorTask::archiveSize() const {
	return      HuntActorTask::archiveSize()
	            +   sizeof(TaskID)               //  goAway ID
	            +   sizeof(range)
	            +   sizeof(targetEvaluateCtr);
}

void HuntToBeNearActorTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToBeNearActorTask");

	//  Let the base class archive its data
	HuntActorTask::write(out);

	//  Store the task ID
	if (goAway != nullptr)
		out->writeSint16LE(getTaskID(goAway));
	else
		out->writeSint16LE(NoTask);

	//  Store the range
	out->writeUint16LE(range);

	//  Store the evaluation counter
	out->writeByte(targetEvaluateCtr);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 HuntToBeNearActorTask::getType() const {
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

void HuntToBeNearActorTask::evaluateTarget() {
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

bool HuntToBeNearActorTask::atTarget() {
	TilePoint   targetLoc = currentTargetLoc();

	//  Determine if we're within the specified range of the current
	//  target actor
	if (targetLoc != Nowhere
	        &&  stack->getActor()->inRange(targetLoc, range))
		return true;
	else {
		if (goAway != nullptr) {
			goAway->abortTask();
			delete goAway;
			goAway = nullptr;
		}

		return false;
	}
}

//----------------------------------------------------------------------

void HuntToBeNearActorTask::atTargetabortTask() {
	if (goAway != nullptr) {
		goAway->abortTask();
		delete goAway;
		goAway = nullptr;
	}
}

//----------------------------------------------------------------------

TaskResult HuntToBeNearActorTask::atTargetEvaluate() {
	TilePoint   targetLoc = currentTargetLoc();

	//  If we're not TOO close, we're done
	if (stack->getActor()->inRange(targetLoc, tooClose))
		return taskNotDone;

	if (goAway != nullptr) {
		goAway->abortTask();
		delete goAway;
		goAway = nullptr;
	}

	return taskSucceeded;
}

//----------------------------------------------------------------------

TaskResult HuntToBeNearActorTask::atTargetUpdate() {
	Actor       *a = stack->getActor();
	TilePoint   targetLoc = currentTargetLoc();

	//  Determine if we're TOO close
	if (a->inRange(targetLoc, tooClose)) {
		//  Setup a go away task if necessary and update it
		if (goAway == nullptr) {
			goAway = new GoAwayFromObjectTask(stack, currentTarget);
			if (goAway != nullptr) goAway->update();
		} else
			goAway->update();

		return taskNotDone;
	}

	//  Delete the go away task if it exists
	if (goAway != nullptr) {
		goAway->abortTask();
		delete goAway;
		goAway = nullptr;
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

	if (isActor(a->_currentTarget))
		currentTarget = (Actor *)a->_currentTarget;

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

inline int32 HuntToKillTask::archiveSize() const {
	return      HuntActorTask::archiveSize()
	            +   sizeof(targetEvaluateCtr)
	            +   sizeof(specialAttackCtr)
	            +   sizeof(flags);
}

void HuntToKillTask::write(Common::MemoryWriteStreamDynamic *out) const {
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

int16 HuntToKillTask::getType() const {
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

void HuntToKillTask::abortTask() {
	HuntActorTask::abortTask();

	Actor       *a = stack->getActor();

	a->_flags &= ~Actor::specialAttack;

	a->setFightStance(false);
}

//----------------------------------------------------------------------

TaskResult HuntToKillTask::update() {
	if (specialAttackCtr == 0) {
		stack->getActor()->_flags |= Actor::specialAttack;
		//  A little hack to make monsters with 99 spellcraft cast spells more often
		if (stack->getActor()->getStats()->spellcraft >= 99)
			specialAttackCtr = 3;
		else specialAttackCtr = 10;
	} else
		specialAttackCtr--;

	return HuntActorTask::update();
}

//----------------------------------------------------------------------

void HuntToKillTask::evaluateTarget() {
	Actor               *a = stack->getActor();

	if (flags & evalWeapon && a->isInterruptable()) {
		evaluateWeapon();
		flags &= ~evalWeapon;
	}


	//  Determine if its time to reevaluate the current target actor
	if (targetEvaluateCtr == 0
	        || (currentTarget != nullptr
	            &&  currentTarget->isDead())) {
		Actor               *bestTarget = nullptr;
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

					if (score > bestScore || bestTarget == nullptr) {
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

					if (score > bestScore || bestTarget == nullptr) {
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

					if (score > bestScore || bestTarget == nullptr) {
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
			a->_currentTarget = currentTarget;
		}

		flags |= evalWeapon;

		targetEvaluateCtr = targetEvaluateRate;
	}

	//  Decrement the target reevaluation counter
	targetEvaluateCtr--;
}

//----------------------------------------------------------------------

bool HuntToKillTask::atTarget() {
	//  Determine if we're in attack range of the current target
	return      currentTarget != nullptr
	            &&  stack->getActor()->inAttackRange(
	                currentTarget->getLocation());
}

//----------------------------------------------------------------------

void HuntToKillTask::atTargetabortTask() {
	//  If the task is aborted while at the target actor, abort any
	//  attack currently taking place
	stack->getActor()->stopAttack(currentTarget);
}

//----------------------------------------------------------------------

TaskResult HuntToKillTask::atTargetEvaluate() {
	//  This task is never done and must be aborted manually
	return taskNotDone;
}

//----------------------------------------------------------------------

TaskResult HuntToKillTask::atTargetUpdate() {
	assert(isActor(currentTarget));

	Actor   *a = stack->getActor();

	//  If we're ready to attack, attack
	if (a->isInterruptable() && g_vm->_rnd->getRandomNumber(7) == 0) {
		a->attack(currentTarget);
		flags |= evalWeapon;
	}

	return taskNotDone;
}

//----------------------------------------------------------------------

void HuntToKillTask::evaluateWeapon() {
	Actor               *a = stack->getActor();
	ObjectID            actorID = a->thisID();
	GameObject          *obj,
	                    *bestWeapon,
	                    *currentWeapon;
	int                 bestWeaponRating;
	ContainerIterator   iter(a);

	bestWeapon = nullptr;
	bestWeaponRating = 0;
	currentWeapon = a->offensiveObject();
	//  If the current offensive object is the actor himself then there
	//  is no current weapon.
	if (currentWeapon == a) currentWeapon = nullptr;

	if (!isAutoWeaponSet() && isPlayerActor(a)) {
		WeaponProto     *weaponProto =  currentWeapon != nullptr
		                                ? (WeaponProto *)currentWeapon->proto()
		                                :   nullptr;

		if (currentTarget == nullptr) {
			warning("%s: currentTarget = NULL (return)", a->objName());
			return;
		}

		if (currentWeapon == nullptr
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

			if (currentTarget) {
				warning("%s: currentTarget = NULL (weaponRating = 0)", a->objName());
				weaponRating =  weaponProto->weaponRating(obj->thisID(),
				                                          actorID,
				                                          currentTarget->thisID());
			} else
				weaponRating = 0;

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

	if (bestWeapon != nullptr) {
		if (bestWeapon != currentWeapon)
			bestWeapon->use(actorID);
	}
	//  If there is no useful best weapon and the actor is currently
	//  wielding a weapon, un-wield the weapon
	else if (currentWeapon != nullptr)
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
	            :   nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntToGiveTask::archiveSize() const {
	return      HuntActorTask::archiveSize()
	            +   sizeof(ObjectID);                //  objToGive ID
}

void HuntToGiveTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToGiveTask");

	//  Let base class archive its data
	HuntActorTask::write(out);

	//  Store the ID
	if (objToGive != nullptr)
		out->writeUint16LE(objToGive->thisID());
	else
		out->writeUint16LE(Nothing);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 HuntToGiveTask::getType() const {
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

void HuntToGiveTask::evaluateTarget() {}

//----------------------------------------------------------------------

bool HuntToGiveTask::atTarget() {
	return false;
}

//----------------------------------------------------------------------

void HuntToGiveTask::atTargetabortTask() {}

//----------------------------------------------------------------------

TaskResult HuntToGiveTask::atTargetEvaluate() {
	return taskNotDone;
}

//----------------------------------------------------------------------

TaskResult HuntToGiveTask::atTargetUpdate() {
	return taskNotDone;
}

/* ===================================================================== *
   BandTask member functions
 * ===================================================================== */

//----------------------------------------------------------------------

bool BandTask::BandingRepulsorIterator::first(
    TilePoint   &repulsorVector,
    int16       &repulsorStrength) {
	assert(a->_leader != nullptr && a->_leader->_followers != nullptr);

	band = a->_leader->_followers;
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
	assert(a->_leader != nullptr && a->_leader->_followers != nullptr);
	assert(band == a->_leader->_followers);
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
	debugC(3, kDebugSaveload, "... Loading BandTask");

	_attendID = in->readSint16LE();
	attend = nullptr;

	//  Restore the current target location
	currentTarget.load(in);

	//  Restore the target evaluation counter
	targetEvaluateCtr = in->readByte();
}

//----------------------------------------------------------------------
//	Fixup the subtask pointers

void BandTask::fixup() {
	//	Let the base class fixup its pointers
	HuntTask::fixup();

	//  Convert the TaskID to a Task pointer
	attend = _attendID != NoTask
	         ? (AttendTask *)getTaskAddress(_attendID)
	         :   nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 BandTask::archiveSize() const {
	return      HuntTask::archiveSize()
	            +   sizeof(TaskID)           //  attend ID
	            +   sizeof(currentTarget)
	            +   sizeof(targetEvaluateCtr);
}

void BandTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving BandTask");

	//  Let the base class archive its data
	HuntTask::write(out);

	//  Store the attend task ID
	if (attend != nullptr)
		out->writeSint16LE(getTaskID(attend));
	else
		out->writeSint16LE(NoTask);

	//  Store the current target location
	currentTarget.write(out);

	//  Store the target evaluation counter
	out->writeByte(targetEvaluateCtr);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 BandTask::getType() const {
	return bandTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool BandTask::operator == (const Task &t) const {
	return t.getType() == bandTask;
}

//----------------------------------------------------------------------

void BandTask::evaluateTarget() {
	if (targetEvaluateCtr == 0) {
		Actor           *leader = stack->getActor()->_leader;
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

		if (repulsorIter == nullptr) return;

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

GotoTask *BandTask::setupGoto() {
	return new GotoLocationTask(stack, currentTarget, getRunThreshold());
}

//----------------------------------------------------------------------

TilePoint BandTask::currentTargetLoc() {
	return currentTarget;
}

//----------------------------------------------------------------------

bool BandTask::atTarget() {
	TilePoint       actorLoc = stack->getActor()->getLocation();

	if ((actorLoc - currentTarget).quickHDistance() > 6
	        ||  ABS(actorLoc.z - currentTarget.z) > kMaxStepHeight) {
		if (attend != nullptr) {
			attend->abortTask();
			delete attend;
			attend = nullptr;
		}

		return false;
	}

	return true;
}

//----------------------------------------------------------------------

void BandTask::atTargetabortTask() {
	if (attend != nullptr) {
		attend->abortTask();
		delete attend;
		attend = nullptr;
	}
}

//----------------------------------------------------------------------

TaskResult BandTask::atTargetEvaluate() {
	return taskNotDone;
}

//----------------------------------------------------------------------

TaskResult BandTask::atTargetUpdate() {
	Actor       *a = stack->getActor();

	if (attend != nullptr)
		attend->update();
	else {
		attend = new AttendTask(stack, a->_leader);
		if (attend != nullptr)
			attend->update();
	}

	return taskNotDone;
}

//----------------------------------------------------------------------

int16 BandTask::getRunThreshold() {
	return kTileUVSize * 3;
}

//----------------------------------------------------------------------

BandTask::RepulsorIterator *BandTask::getNewRepulsorIterator() {
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

int16 BandAndAvoidEnemiesTask::getType() const {
	return bandAndAvoidEnemiesTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool BandAndAvoidEnemiesTask::operator == (const Task &t) const {
	return t.getType() == bandAndAvoidEnemiesTask;
}

//----------------------------------------------------------------------

int16 BandAndAvoidEnemiesTask::getRunThreshold() {
	return 0;
}

//----------------------------------------------------------------------

BandTask::RepulsorIterator *BandAndAvoidEnemiesTask::getNewRepulsorIterator() {
	return new BandAndAvoidEnemiesRepulsorIterator(stack->getActor());
}

/* ===================================================================== *
   FollowPatrolRouteTask member functions
 * ===================================================================== */

FollowPatrolRouteTask::FollowPatrolRouteTask(Common::InSaveFile *in, TaskID id) : Task(in, id) {
	debugC(3, kDebugSaveload, "... Loading FollowPatrolRouteTask");

	//  Get the gotoWayPoint TaskID
	_gotoWayPointID = in->readSint16LE();
	gotoWayPoint = nullptr;

	//  Restore the patrol route iterator
	patrolIter.read(in);

	//  Restore the last waypoint number
	lastWayPointNum = in->readSint16LE();

	//  Restore the paused flag
	paused = in->readUint16LE();

	//  Restore the paused counter
	counter = in->readSint16LE();
}

//----------------------------------------------------------------------
//	Fixup the subtask pointers

void FollowPatrolRouteTask::fixup() {
	//	Let the base class fixup its pointers
	Task::fixup();

	//  Convert the TaskID to a Task pointer
	gotoWayPoint = _gotoWayPointID != NoTask
	               ? (GotoLocationTask *)getTaskAddress(_gotoWayPointID)
	               :   nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 FollowPatrolRouteTask::archiveSize() const {
	return      Task::archiveSize()
	            +   sizeof(TaskID)   //  gotoWayPoint ID
	            +   sizeof(patrolIter)
	            +   sizeof(lastWayPointNum)
	            +   sizeof(paused)
	            +   sizeof(counter);
}

void FollowPatrolRouteTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving FollowPatrolRouteTask");

	//  Let the base class archive its data
	Task::write(out);

	//  Store the gotoWayPoint ID
	if (gotoWayPoint != nullptr)
		out->writeSint16LE(getTaskID(gotoWayPoint));
	else
		out->writeSint16LE(NoTask);

	//  Store the PatrolRouteIterator
	patrolIter.write(out);

	//  Store the last waypoint number
	out->writeSint16LE(lastWayPointNum);

	//  Store the paused flag
	out->writeUint16LE(paused);

	//  Store the paused counter
	out->writeSint16LE(counter);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 FollowPatrolRouteTask::getType() const {
	return followPatrolRouteTask;
}

//----------------------------------------------------------------------

void FollowPatrolRouteTask::abortTask() {
	//  If there is a subtask, get rid of it
	if (gotoWayPoint) {
		gotoWayPoint->abortTask();
		delete gotoWayPoint;
		gotoWayPoint = nullptr;
	}
}

//----------------------------------------------------------------------

TaskResult FollowPatrolRouteTask::evaluate() {
	//  Simply check the patrol iterator to determine if there are
	//  any more waypoints
	return *patrolIter == Nowhere ? taskSucceeded : taskNotDone;
}

//----------------------------------------------------------------------

TaskResult FollowPatrolRouteTask::update() {
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

TaskResult FollowPatrolRouteTask::handleFollowPatrolRoute() {
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
		if (gotoWayPoint != nullptr) {
			gotoWayPoint->abortTask();
			delete gotoWayPoint;
			gotoWayPoint = nullptr;
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
	if (gotoWayPoint != nullptr)
		gotoWayPoint->update();
	else {
		gotoWayPoint = new GotoLocationTask(stack, currentWayPoint);
		if (gotoWayPoint != nullptr) gotoWayPoint->update();
	}

	return taskNotDone;
}

//----------------------------------------------------------------------
//	Update function used if this task is paused

TaskResult FollowPatrolRouteTask::handlePaused() {
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

void FollowPatrolRouteTask::pause() {
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
		: nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 AttendTask::archiveSize() const {
	return Task::archiveSize() + sizeof(ObjectID);
}


void AttendTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving AttendTask");

	//  Let the base class archive its data
	Task::write(out);

	//  Store the object ID
	if (obj != nullptr)
		out->writeUint16LE(obj->thisID());
	else
		out->writeUint16LE(Nothing);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 AttendTask::getType() const {
	return attendTask;
}

//----------------------------------------------------------------------

void AttendTask::abortTask() {
	MotionTask  *actorMotion = stack->getActor()->_moveTask;

	//  Determine if we need to abort the actor motion
	if (actorMotion != nullptr && actorMotion->isTurn())
		actorMotion->finishTurn();
}

//----------------------------------------------------------------------

TaskResult AttendTask::evaluate() {
	//  Attending must be stopped manually
	return taskNotDone;
}

//----------------------------------------------------------------------

TaskResult AttendTask::update() {
	Actor       *a = stack->getActor();
	TilePoint   attendLoc = obj->getWorldLocation();

	//  Determine if we are facing the object
	if (a->_currentFacing != (attendLoc - a->getLocation()).quickDir()) {
		//  If not, turn
		if (!a->_moveTask || !a->_moveTask->isTurn())
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

void TaskStack::write(Common::MemoryWriteStreamDynamic *out) {
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

void TaskStack::abortTask() {
	if (stackBottomID != NoTask) {
		Task    *stackBottom = getTaskAddress(stackBottomID);

		stackBottom->abortTask();
		delete stackBottom;
	}
}

//----------------------------------------------------------------------
//  Re-evaluate tasks in stack

TaskResult TaskStack::evaluate() {
	if (stackBottomID != -1) {
		Task    *stackBottom = getTaskAddress(stackBottomID);

		return stackBottom->evaluate();
	} else
		return taskNotDone;
}

//----------------------------------------------------------------------
//  Update the state of the tasks in stack

TaskResult TaskStack::update() {
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
