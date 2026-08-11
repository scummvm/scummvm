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

const int       numTaskStacks = 320;

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

		//  Retrieve the TaskStack's id number
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
			if ((result = ts->update()) != kTaskNotDone) {
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

const int numTasks = 640;

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

		//  Retrieve the Task's id number
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
	case kWanderTask:
		new WanderTask(in, id);
		break;

	case kTetheredWanderTask:
		new TetheredWanderTask(in, id);
		break;

	case kGotoLocationTask:
		new GotoLocationTask(in, id);
		break;

	case kGotoRegionTask:
		new GotoRegionTask(in, id);
		break;

	case kGotoObjectTask:
		new GotoObjectTask(in, id);
		break;

	case kGotoActorTask:
		new GotoActorTask(in, id);
		break;

	case kGoAwayFromObjectTask:
		new GoAwayFromObjectTask(in, id);
		break;

	case kGoAwayFromActorTask:
		new GoAwayFromActorTask(in, id);
		break;

	case kHuntToBeNearLocationTask:
		new HuntToBeNearLocationTask(in, id);
		break;

	case kHuntToBeNearObjectTask:
		new HuntToBeNearObjectTask(in, id);
		break;

	case kHuntToPossessTask:
		new HuntToPossessTask(in, id);
		break;

	case kHuntToBeNearActorTask:
		new HuntToBeNearActorTask(in, id);
		break;

	case kHuntToKillTask:
		new HuntToKillTask(in, id);
		break;

	case kHuntToGiveTask:
		new HuntToGiveTask(in, id);
		break;

	case kBandTask:
		new BandTask(in, id);
		break;

	case kBandAndAvoidEnemiesTask:
		new BandAndAvoidEnemiesTask(in, id);
		break;

	case kFollowPatrolRouteTask:
		new FollowPatrolRouteTask(in, id);
		break;

	case kAttendTask:
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
	_stack = nullptr;
	newTask(this, id);
}

//----------------------------------------------------------------------
//	Fixup the Task pointers

void Task::fixup() {
	//	Convert the stack ID to a stack pointer
	_stack = getTaskStackAddress(_stackID);
}

//----------------------------------------------------------------------
//	Return the number of bytes necessary to create an archive of this
//	object's data

inline int32 Task::archiveSize() const {
	return sizeof(TaskStackID);      //  stack's ID
}

void Task::write(Common::MemoryWriteStreamDynamic *out) const {
	out->writeSint16LE(getTaskStackID(_stack));
}

/* ===================================================================== *
   WanderTask member functions
 * ===================================================================== */

WanderTask::WanderTask(Common::InSaveFile *in, TaskID id) : Task(in, id) {
	//  Restore the _paused flag
	_paused = in->readUint16LE();

	//  Restore the _counter
	_counter = in->readSint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

int32 WanderTask::archiveSize() const {
	return      Task::archiveSize()
	            +   sizeof(_paused)
	            +   sizeof(_counter);
}

void WanderTask::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Let the base class archive its data
	Task::write(out);

	//  Store the _paused flag
	out->writeUint16LE(_paused);

	//  Store the _counter
	out->writeSint16LE(_counter);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 WanderTask::getType() const {
	return kWanderTask;
}

//----------------------------------------------------------------------

void WanderTask::abortTask() {
	//  if the actor has a wander motion, abort it
	MotionTask *actorMotion = _stack->getActor()->_moveTask;

	if (actorMotion && actorMotion->isWander())
		actorMotion->finishWalk();
}

//----------------------------------------------------------------------

TaskResult WanderTask::evaluate() {
	//  Wandering is never done.  It must be stopped manually.
	return kTaskNotDone;
}

//----------------------------------------------------------------------

TaskResult WanderTask::update() {
	if (_counter == 0) {
		if (!_paused)
			pause();
		else
			wander();
	} else
		_counter--;

	return !_paused ? handleWander() : handlePaused();
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool WanderTask::operator == (const Task &t) const {
	return t.getType() == kWanderTask;
}

//----------------------------------------------------------------------
//	Update function used when task is not _paused

TaskResult WanderTask::handleWander() {
	MotionTask  *actorMotion = _stack->getActor()->_moveTask;

	//  If the actor is not already wandering, start a wander motion
	//  task
	if (!actorMotion
	        ||  !actorMotion->isWander())
		MotionTask::wander(*_stack->getActor());

	return kTaskNotDone;
}

//----------------------------------------------------------------------
//	Set this task into the _paused state

void WanderTask::pause() {
	//  Call abort to stop the wandering motion
	abortTask();

	_paused = true;
	_counter = (g_vm->_rnd->getRandomNumber(63) + g_vm->_rnd->getRandomNumber(63)) / 2;
}

//----------------------------------------------------------------------
//	Set this task into the wander state

void WanderTask::wander() {
	_paused = false;
	_counter = (g_vm->_rnd->getRandomNumber(255) + g_vm->_rnd->getRandomNumber(255)) / 2;
}

/* ===================================================================== *
   TetheredWanderTask member functions
 * ===================================================================== */

TetheredWanderTask::TetheredWanderTask(Common::InSaveFile *in, TaskID id) : WanderTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading TetheredWanderTask");

	//  Restore the tether coordinates
	_minU = in->readSint16LE();
	_minV = in->readSint16LE();
	_maxU = in->readSint16LE();
	_maxV = in->readSint16LE();

	//  Put the _gotoTether ID into the _gotoTether pointer field
	_gotoTetherID = in->readSint16LE();
	_gotoTether = nullptr;
}

//----------------------------------------------------------------------
//	Fixup the subtask pointers

void TetheredWanderTask::fixup() {
	//	Let the base class fixup it's pointers
	WanderTask::fixup();

	//	Restore the _gotoTether pointer
	_gotoTether = _gotoTetherID != NoTask
				 ?	(GotoRegionTask *)getTaskAddress(_gotoTetherID)
				 :	nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

inline int32 TetheredWanderTask::archiveSize() const {
	return      WanderTask::archiveSize()
	            +   sizeof(_minU)
	            +   sizeof(_minU)
	            +   sizeof(_minU)
	            +   sizeof(_minU)
	            +   sizeof(TaskID);      //  _gotoTether ID
}

void TetheredWanderTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving TetheredWanderTask");

	//  Let the base class archive its data
	WanderTask::write(out);

	//  Archive tether coordinates
	out->writeSint16LE(_minU);
	out->writeSint16LE(_minV);
	out->writeSint16LE(_maxU);
	out->writeSint16LE(_maxV);

	//  Archive _gotoTether ID
	if (_gotoTether != nullptr)
		out->writeSint16LE(getTaskID(_gotoTether));
	else
		out->writeSint16LE(NoTask);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 TetheredWanderTask::getType() const {
	return kTetheredWanderTask;
}

//----------------------------------------------------------------------

void TetheredWanderTask::abortTask() {
	if (_gotoTether != nullptr) {
		_gotoTether->abortTask();
		delete _gotoTether;
		_gotoTether = nullptr;
	} else {
		MotionTask *actorMotion = _stack->getActor()->_moveTask;

		//  if the actor has a tethered wander motion, abort it
		if (actorMotion && actorMotion->isTethered())
			actorMotion->finishWalk();
	}
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool TetheredWanderTask::operator == (const Task &t) const {
	if (t.getType() != kTetheredWanderTask) return false;

	const TetheredWanderTask *taskPtr = (const TetheredWanderTask *)&t;

	return      _minU == taskPtr->_minU && _minV == taskPtr->_minV
	            &&  _maxU == taskPtr->_maxU && _maxV == taskPtr->_maxV;
}

//----------------------------------------------------------------------
//	Update function used when task is not _paused

TaskResult TetheredWanderTask::handleWander() {
	Actor       *a = _stack->getActor();
	TilePoint   actorLoc = a->getLocation();

	if (actorLoc.u < _minU || actorLoc.u >= _maxU
	        ||  actorLoc.v < _minV || actorLoc.v >= _maxV) {
		if (_gotoTether != nullptr)
			_gotoTether->update();
		else {
			_gotoTether = new GotoRegionTask(_stack, _minU, _minV, _maxU, _maxV);
			if (_gotoTether != nullptr)
				_gotoTether->update();
		}
	} else {
		if (_gotoTether != nullptr) {
			_gotoTether->abortTask();
			delete _gotoTether;
			_gotoTether = nullptr;
		}

		bool            startWander = false;
		TileRegion      motionTether;

		MotionTask  *actorMotion = a->_moveTask;

		if (actorMotion) {
			TileRegion  motionTeth = actorMotion->getTether();
			startWander     = ((!actorMotion->isWander())
			                   ||  motionTeth.min.u != _minU
			                   ||  motionTeth.min.v != _minV
			                   ||  motionTeth.max.u != _maxU
			                   ||  motionTeth.max.v != _maxV);

		} else
			startWander = true;

		//  If the actor is not already wandering, start a wander motion
		//  task

		// JeffL - prevent null pointer reference
		/*
		if (    !actorMotion
		    ||  !actorMotion->isWander()
		    ||  motionTether.min.u != _minU
		    ||  motionTether.min.v != _minV
		    ||  motionTether.max.u != _maxU
		    ||  motionTether.max.v != _maxV )
		*/
		if (startWander) {
			TileRegion  reg;

			reg.min = TilePoint(_minU, _minV, 0);
			reg.max = TilePoint(_maxU, _maxV, 0);
			MotionTask::tetheredWander(*_stack->getActor(), reg);
		}
	}

	return kTaskNotDone;
}

/* ===================================================================== *
   GotoTask member functions
 * ===================================================================== */

GotoTask::GotoTask(Common::InSaveFile *in, TaskID id) : Task(in, id) {
	//  Get the wander TaskID
	_wanderID = in->readSint16LE();
	_wander = nullptr;

	//  Restore prevRunState
	_prevRunState = in->readUint16LE();
}

//----------------------------------------------------------------------
//	Fixup the subtask pointers

void GotoTask::fixup() {
	//	Let the base class fixup its pointers
	Task::fixup();

	//	Convert wanderID to a Task pointer
	_wander = _wanderID != NoTask
	         ? (WanderTask *)getTaskAddress(_wanderID)
	         :   nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 GotoTask::archiveSize() const {
	return      Task::archiveSize()
	            +   sizeof(TaskID)       //  wander ID
	            +   sizeof(_prevRunState);
}

void GotoTask::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Let the base class archive its data
	Task::write(out);

	//  Convert the wander Task pointer to a TaskID and store it
	//  in the buffer
	if (_wander != nullptr)
		out->writeSint16LE(getTaskID( _wander));
	else
		out->writeSint16LE(NoTask);

	//  Store _prevRunState
	out->writeUint16LE(_prevRunState);
}

//----------------------------------------------------------------------

void GotoTask::abortTask() {
	//  If there is a wander subtask, delete it.
	if ( _wander) {
		 _wander->abortTask();
		delete _wander;
		_wander = nullptr;
	} else {
		MotionTask  *actorMotion = _stack->getActor()->_moveTask;

		if (actorMotion && actorMotion->isWalk()) actorMotion->finishWalk();
	}
}

//----------------------------------------------------------------------

TaskResult GotoTask::evaluate() {
	//  Determine if we have reach the target.
	if (_stack->getActor()->getLocation() == destination()) {
		abortTask();
		return kTaskSucceeded;
	}

	return kTaskNotDone;
}

//----------------------------------------------------------------------

TaskResult GotoTask::update() {
	//  Check to see if we have reached the target
	{
		TaskResult  result = evaluate();
		if (result != kTaskNotDone) return result;
	}

	Actor *const   a = _stack->getActor();
	//  Compute the immediate destination based upon whether or not the
	//  actor has a line of sight to the target.
	TilePoint       immediateDest = lineOfSight()
	                                ?   destination()
	                                :   intermediateDest();

	//  If we have a destination, walk there, else wander
	if (immediateDest != Nowhere) {
		//  If wandering, cut it out
		if (_wander != nullptr) {
			delete _wander;
			_wander = nullptr;
		}

		//  Determine if there is already a motion task, and if so,
		//  whether or not it needs to be modified.
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
				        ||  runState != _prevRunState)
					actorMotion->changeDirectTarget(
					    immediateDest,
					    _prevRunState = runState);
			} else {
				if ((motionTarget.u >> kTileUVShift)
				        != (immediateDest.u >> kTileUVShift)
				        || (motionTarget.v >> kTileUVShift)
				        != (immediateDest.v >> kTileUVShift)
				        ||  ABS(motionTarget.z - immediateDest.z) > 16
				        ||  runState != _prevRunState)
					actorMotion->changeTarget(
					    immediateDest,
					    _prevRunState = runState);
			}
		} else {
			if ((actorLoc.u >> kTileUVShift)
			        == (immediateDest.u >> kTileUVShift)
			        && (actorLoc.v >> kTileUVShift)
			        == (immediateDest.v >> kTileUVShift)) {
				MotionTask::walkToDirect(
				    *a,
				    immediateDest,
				    _prevRunState = run());
			} else
				MotionTask::walkTo(*a, immediateDest, _prevRunState = run());
		}
	} else {
		//  If wandering, update the wander task else set up a new
		//  wander task
		if (_wander != nullptr)
			 _wander->update();
		else {
			_wander = new WanderTask(_stack);
			if (_wander != nullptr)  _wander->update();
		}

		return kTaskNotDone;
	}

	return kTaskNotDone;
}

/* ===================================================================== *
   GotoLocationTask member functions
 * ===================================================================== */

GotoLocationTask::GotoLocationTask(Common::InSaveFile *in, TaskID id) : GotoTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading GotoLocationTask");

	//  Restore the target location
	_targetLoc.load(in);

	//  Restore the _runThreshold
	_runThreshold = in->readByte();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 GotoLocationTask::archiveSize() const {
	return      GotoTask::archiveSize()
	            +   sizeof(_targetLoc)
	            +   sizeof(_runThreshold);
}

void GotoLocationTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving GotoLocationTask");

	//  Let the base class archive its data
	GotoTask::write(out);

	//  Archive the target location
	_targetLoc.write(out);

	//  Archive the run threshold
	out->writeByte(_runThreshold);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 GotoLocationTask::getType() const {
	return kGotoLocationTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool GotoLocationTask::operator == (const Task &t) const {
	if (t.getType() != kGotoLocationTask) return false;

	const GotoLocationTask *taskPtr = (const GotoLocationTask *)&t;

	return      _targetLoc == taskPtr->_targetLoc
	            &&  _runThreshold == taskPtr->_runThreshold;
}

//----------------------------------------------------------------------

TilePoint GotoLocationTask::destination() {
	//  Simply return the target location
	return _targetLoc;
}

//----------------------------------------------------------------------

TilePoint GotoLocationTask::intermediateDest() {
	//  GotoLocationTask's never have an intermediate destination
	return _targetLoc;
}

//----------------------------------------------------------------------

bool GotoLocationTask::lineOfSight() {
	//  Let's pretend that there is always a line of sight to the
	//  target location
	return true;
}

//----------------------------------------------------------------------

bool GotoLocationTask::run() {
	TilePoint       actorLoc = _stack->getActor()->getLocation();

	return  _runThreshold != maxuint8
	        ? (_targetLoc - actorLoc).quickHDistance() > _runThreshold
	        ||  ABS(_targetLoc.z - actorLoc.z) > _runThreshold
	        :   false;
}

/* ===================================================================== *
   GotoRegionTask member functions
 * ===================================================================== */

GotoRegionTask::GotoRegionTask(Common::InSaveFile *in, TaskID id) : GotoTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading GotoRegionTask");

	//  Restore the region coordinates
	_regionMinU = in->readSint16LE();
	_regionMinV = in->readSint16LE();
	_regionMaxU = in->readSint16LE();
	_regionMaxV = in->readSint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 GotoRegionTask::archiveSize() const {
	return      GotoTask::archiveSize()
	            +   sizeof(_regionMinU)
	            +   sizeof(_regionMinV)
	            +   sizeof(_regionMaxU)
	            +   sizeof(_regionMaxV);
}

void GotoRegionTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving GotoRegionTask");

	//  Let the base class archive its data
	GotoTask::write(out);
	//  Archive the region coordinates
	out->writeSint16LE(_regionMinU);
	out->writeSint16LE(_regionMinV);
	out->writeSint16LE(_regionMaxU);
	out->writeSint16LE(_regionMaxV);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 GotoRegionTask::getType() const {
	return kGotoRegionTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool GotoRegionTask::operator == (const Task &t) const {
	if (t.getType() != kGotoRegionTask) return false;

	const GotoRegionTask *taskPtr = (const GotoRegionTask *)&t;

	return      _regionMinU == taskPtr->_regionMinU
	            &&  _regionMinV == taskPtr->_regionMinV
	            &&  _regionMaxU == taskPtr->_regionMaxU
	            &&  _regionMaxV == taskPtr->_regionMaxV;
}

TilePoint GotoRegionTask::destination() {
	TilePoint   actorLoc = _stack->getActor()->getLocation();

	return  TilePoint(
	            clamp(_regionMinU, actorLoc.u, _regionMaxU - 1),
	            clamp(_regionMinV, actorLoc.v, _regionMaxV - 1),
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
	//  Restore _lastTestedLoc and increment pointer
	_lastTestedLoc.load(in);

	//  Restore _sightCtr and increment pointer
	_sightCtr = in->readSint16LE();

	//  Restore the flags and increment pointer
	_flags = in->readByte();

	//  Restore _lastKnownLoc
	_lastKnownLoc.load(in);
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 GotoObjectTargetTask::archiveSize() const {
	return      GotoTask::archiveSize()
	            +   sizeof(_lastTestedLoc)
	            +   sizeof(_sightCtr)
	            +   sizeof(_flags)
	            +   sizeof(_lastKnownLoc);
}

void GotoObjectTargetTask::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Let the base class archive its data
	GotoTask::write(out);

	//  Archive _lastTestedLoc and increment pointer
	_lastTestedLoc.write(out);

	//  Archive _sightCtr and increment pointer
	out->writeSint16LE(_sightCtr);

	//  Archive the flags and increment pointer
	out->writeByte(_flags);

	//  Archive _lastKnownLoc
	_lastKnownLoc.write(out);
}

//----------------------------------------------------------------------

TilePoint GotoObjectTargetTask::destination() {
	//  Return the object's true location
	return getObject()->getLocation();
}

//----------------------------------------------------------------------

TilePoint GotoObjectTargetTask::intermediateDest() {
	//  Return the last known location
	return _lastKnownLoc;
}

//----------------------------------------------------------------------

bool GotoObjectTargetTask::lineOfSight() {
	if (_flags & kTrack) {
		_flags |= kInSight;
		_lastKnownLoc = getObject()->getLocation();
	} else {
		Actor       *a = _stack->getActor();
		GameObject  *target = getObject();
		ObjectID    targetID = target->thisID();
		TilePoint   _targetLoc = target->getLocation();
		SenseInfo   info;

		//  Determine if we need to retest the line of sight
		if (_flags & kInSight) {
			//  If the object was previously in sight, retest the line of
			//  sight if the target has moved beyond a certain range from
			//  the last location it was tested at.
			if ((_targetLoc - _lastTestedLoc).quickHDistance() > 25
			        ||  ABS(_targetLoc.z - _lastTestedLoc.z) > 25) {
				if (a->canSenseSpecificObject(
				            info,
				            kMaxSenseRange,
				            targetID)
				        ||  a->canSenseSpecificObjectIndirectly(
				            info,
				            kMaxSenseRange,
				            targetID))
					_flags |= kInSight;
				else
					_flags &= ~kInSight;
				_lastTestedLoc = _targetLoc;
			}
		} else {
			//  If the object was not privously in sight, retest the line
			//  of sight periodically
			if (_sightCtr == 0) {
				_sightCtr = kSightRate;
				if (a->canSenseSpecificObject(
				            info,
				            kMaxSenseRange,
				            targetID)
				        ||  a->canSenseSpecificObjectIndirectly(
				            info,
				            kMaxSenseRange,
				            targetID))
					_flags |= kInSight;
				else
					_flags &= ~kInSight;
				_lastTestedLoc = _targetLoc;
			}
			_sightCtr--;
		}

		if (_flags & kInSight) {
			//  If the target is in sight, the last known location is the
			//  objects current location.
			_lastKnownLoc = _targetLoc;
		} else {
			//  If the target is not in sight, determine if we've already
			//  reached the last know location and if so set the last
			//  known location to Nowhere
			if (_lastKnownLoc != Nowhere
			        && (_lastKnownLoc - a->getLocation()).quickHDistance() <= 4)
				_lastKnownLoc = Nowhere;
		}
	}

	return _flags & kInSight;
}

/* ===================================================================== *
   GotoObjectTask member functions
 * ===================================================================== */

GotoObjectTask::GotoObjectTask(Common::InSaveFile *in, TaskID id) :
	GotoObjectTargetTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading GotoObjectTask");

	ObjectID targetID = in->readUint16LE();

	//  Restore the _targetObj pointer
	_targetObj = targetID != Nothing
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

	if (_targetObj != nullptr)
		out->writeUint16LE(_targetObj->thisID());
	else
		out->writeUint16LE(Nothing);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 GotoObjectTask::getType() const {
	return kGotoObjectTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool GotoObjectTask::operator == (const Task &t) const {
	if (t.getType() != kGotoObjectTask) return false;

	const GotoObjectTask *taskPtr = (const GotoObjectTask *)&t;

	return      tracking() == taskPtr->tracking()
	            &&  _targetObj == taskPtr->_targetObj;
}

//----------------------------------------------------------------------

GameObject *GotoObjectTask::getObject() {
	//  Simply return the pointer to the target object
	return _targetObj;
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
	//  Restore the _targetObj pointer
	ObjectID targetID = in->readUint16LE();
	_targetActor =   targetID != Nothing
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

	if (_targetActor != nullptr)
		out->writeUint16LE(_targetActor->thisID());
	else
		out->writeUint16LE(Nothing);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 GotoActorTask::getType() const {
	return kGotoActorTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool GotoActorTask::operator == (const Task &t) const {
	if (t.getType() != kGotoActorTask) return false;

	const GotoActorTask *taskPtr = (const GotoActorTask *)&t;

	return      tracking() == taskPtr->tracking()
	            &&  _targetActor == taskPtr->_targetActor;
}

//----------------------------------------------------------------------

GameObject *GotoActorTask::getObject() {
	//  Simply return the pointer to the target actor
	return (GameObject *)_targetActor;
}

//----------------------------------------------------------------------

bool GotoActorTask::run() {
	if (isInSight()) {
		TilePoint       actorLoc = _stack->getActor()->getLocation(),
		                _targetLoc = getTarget()->getLocation();

		return (actorLoc - _targetLoc).quickHDistance() >= kTileUVSize * 4;
	} else
		return _lastKnownLoc != Nowhere;
}

GoAwayFromTask::GoAwayFromTask(Common::InSaveFile *in, TaskID id) : Task(in, id) {
	//  Get the subtask ID
	_goTaskID = in->readSint16LE();
	_goTask = nullptr;

	//  Restore the flags
	_flags = in->readByte();
}

//----------------------------------------------------------------------
//	Fixup the subtask pointer

void GoAwayFromTask::fixup() {
		//	Let the base class fixup its pointers
	Task::fixup();

	_goTask = _goTaskID != NoTask
	         ? (GotoLocationTask *)getTaskAddress(_goTaskID)
	         :   nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 GoAwayFromTask::archiveSize() const {
	return Task::archiveSize() + sizeof(TaskID) + sizeof(_flags);
}

void GoAwayFromTask::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Let the base class archive its data
	Task::write(out);

	//  Store the _subTask's ID
	if (_goTask != nullptr)
		out->writeSint16LE(getTaskID(_goTask));
	else
		out->writeSint16LE(NoTask);

	//  Store the flags
	out->writeByte(_flags);
}

//----------------------------------------------------------------------
//	Abort this task

void GoAwayFromTask::abortTask() {
	if (_goTask != nullptr) {
		_goTask->abortTask();
		delete _goTask;
		_goTask = nullptr;
	}
}

//----------------------------------------------------------------------
//	Evaluate this task

TaskResult GoAwayFromTask::evaluate() {
	//  Going away is never done, it must be stopped manually
	return kTaskNotDone;
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

	Actor           *a = _stack->getActor();
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

	if (_goTask != nullptr) {
		if (_goTask->getTarget() != dest)
			_goTask->changeTarget(dest);

		_goTask->update();
	} else {
		if ((_goTask =   _flags & kRun
		                ?   new GotoLocationTask(_stack, dest, 0)
		                :   new GotoLocationTask(_stack, dest))
		        !=  nullptr)
			_goTask->update();
	}

	return kTaskNotDone;
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
	_obj = objectID != Nothing
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
	if (_obj != nullptr)
		out->writeUint16LE(_obj->thisID());
	else
		out->writeUint16LE(Nothing);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 GoAwayFromObjectTask::getType() const {
	return kGoAwayFromObjectTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool GoAwayFromObjectTask::operator == (const Task &t) const {
	if (t.getType() != kGoAwayFromObjectTask) return false;

	const GoAwayFromObjectTask *taskPtr = (const GoAwayFromObjectTask *)&t;

	return _obj == taskPtr->_obj;
}

//----------------------------------------------------------------------
//	Simply return the object's location

TilePoint GoAwayFromObjectTask::getRepulsionVector() {
	return _stack->getActor()->getLocation() - _obj->getLocation();
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
	SpecificActorTarget(a).clone(_targetMem);
}

GoAwayFromActorTask::GoAwayFromActorTask(
    TaskStack           *ts,
    const ActorTarget   &at,
    bool                runFlag) :
	GoAwayFromTask(ts, runFlag) {
	assert(at.size() <= sizeof(_targetMem));
	debugC(2, kDebugTasks, " - GoAwayFromActorTask2");
	//  Copy the target to the target buffer
	at.clone(_targetMem);
}


GoAwayFromActorTask::GoAwayFromActorTask(Common::InSaveFile *in, TaskID id) : GoAwayFromTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading GoAwayFromActorTask");

	//  Restore the target
	readTarget(_targetMem, in);
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
	return kGoAwayFromActorTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool GoAwayFromActorTask::operator == (const Task &t) const {
	if (t.getType() != kGoAwayFromActorTask) return false;

	const GoAwayFromActorTask *taskPtr = (const GoAwayFromActorTask *)&t;

	return *getTarget() == *taskPtr->getTarget();
}

//----------------------------------------------------------------------

TilePoint GoAwayFromActorTask::getRepulsionVector() {
	Actor               *a = _stack->getActor();
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
	_huntFlags = in->readByte();
	_subTask = nullptr;

	//  If the flags say we have a sub task, restore it too
	if (_huntFlags & (kHuntGoto| kHuntWander))
		_subTaskID = in->readSint16LE();
	else
		_subTaskID = NoTask;
}

//----------------------------------------------------------------------
//	Fixup the subtask pointers

void HuntTask::fixup( void ) {
	//	Let the base class fixup its pointers
	Task::fixup();

	if (_huntFlags & (kHuntGoto| kHuntWander))
		_subTask = getTaskAddress(_subTaskID);
	else
		_subTask = nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntTask::archiveSize() const {
	int32       size = 0;

	size += Task::archiveSize() + sizeof(_huntFlags);
	if (_huntFlags & (kHuntGoto| kHuntWander)) size += sizeof(TaskID);

	return size;
}

void HuntTask::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Let the base class archive its data
	Task::write(out);

	//  Store the flags
	out->writeByte(_huntFlags);

	//  If the flags say we have a sub task, store it too
	if (_huntFlags & (kHuntGoto| kHuntWander))
		out->writeSint16LE(getTaskID(_subTask));
}

//----------------------------------------------------------------------

void HuntTask::abortTask() {
	if (_huntFlags & (kHuntWander| kHuntGoto)) {
		_subTask->abortTask();
		delete _subTask;
	}

	//  If we've reached the target call the atTargetabortTask() function
	if (atTarget()) atTargetabortTask();
}

//----------------------------------------------------------------------

TaskResult HuntTask::evaluate() {
	if (atTarget()) {
		//  If we've reached the target abort any sub tasks
		if (_huntFlags & kHuntWander)
			removeWanderTask();
		else if (_huntFlags & kHuntGoto)
			removeGotoTask();

		return atTargetEvaluate();
	} else
		//  If we haven't reached the target, we know we're not done
		return kTaskNotDone;
}

//----------------------------------------------------------------------

TaskResult HuntTask::update() {
	Actor       *a = _stack->getActor();

	if (a->_moveTask && a->_moveTask->isPrivledged()) return kTaskNotDone;

	//  Reevaluate the target
	evaluateTarget();

	//  Determine if we have reached the target
	if (atTarget()) {
		//  If we've reached the target abort any sub tasks
		if (_huntFlags & kHuntWander)
			removeWanderTask();
		else if (_huntFlags & kHuntGoto)
			removeGotoTask();

		return atTargetUpdate();
	} else {
		//  If we are going to a target, determine if the goto task
		//  is still valid.  If not, abort it.
		if ((_huntFlags & kHuntGoto)
		        &&  targetHasChanged((GotoTask *)_subTask))
			removeGotoTask();

		//  Determine if there is a goto subtask
		if (!(_huntFlags & kHuntGoto)) {
			GotoTask    *gotoResult;

			//  Try to set up a goto subtask
			if ((gotoResult = setupGoto()) != nullptr) {
				if (_huntFlags & kHuntWander) removeWanderTask();

				_subTask = gotoResult;
				_huntFlags |= kHuntGoto;
			} else {
				//  If we couldn't setup a goto task, setup a wander task
				if (!(_huntFlags & kHuntWander)) {
					if ((_subTask = new WanderTask(_stack)) != nullptr)
						_huntFlags |= kHuntWander;
				}
			}
		}

		//  If there is a subtask, update it
		if ((_huntFlags & (kHuntGoto| kHuntWander)) && _subTask)
			_subTask->update();

		//  If we're not at the target, we know the hunt task is not
		//  done
		return kTaskNotDone;
	}
}

//----------------------------------------------------------------------

void HuntTask::removeWanderTask() {
	_subTask->abortTask();
	delete _subTask;
	_huntFlags &= ~kHuntWander;
}

//----------------------------------------------------------------------

void HuntTask::removeGotoTask() {
	_subTask->abortTask();
	delete _subTask;
	_subTask = nullptr;
	_huntFlags &= ~kHuntGoto;
}

/* ===================================================================== *
   HuntLocationTask member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- initial construction

HuntLocationTask::HuntLocationTask(TaskStack *ts, const Target &t) :
	HuntTask(ts),
	_currentTarget(Nowhere) {
	assert(t.size() <= sizeof(_targetMem));
	debugC(2, kDebugTasks, " - HuntLocationTask");
	//  Copy the target to the target buffer
	t.clone(_targetMem);
}

HuntLocationTask::HuntLocationTask(Common::InSaveFile *in, TaskID id) : HuntTask(in, id) {
	//  Restore the _currentTarget location
	_currentTarget.load(in);

	//  Restore the target
	readTarget(_targetMem, in);
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntLocationTask::archiveSize() const {
	return      HuntTask::archiveSize()
	            +   sizeof(_currentTarget)
	            +   targetArchiveSize(getTarget());
}

void HuntLocationTask::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Let the base class archive its data
	HuntTask::write(out);

	//  Store the current target location
	_currentTarget.write(out);

	//  Store the target
	writeTarget(getTarget(), out);
}

//----------------------------------------------------------------------

bool HuntLocationTask::targetHasChanged(GotoTask *gotoTarget) {
	//  Determine if the specified goto task is going to the current
	//  target.
	GotoLocationTask    *gotoLoc = (GotoLocationTask *)gotoTarget;
	return gotoLoc->getTarget() != _currentTarget;
}

//----------------------------------------------------------------------

GotoTask *HuntLocationTask::setupGoto() {
	//  If there is somewhere to go, setup a goto task, else return NULL
	return  _currentTarget != Nowhere
	        ?   new GotoLocationTask(_stack, _currentTarget)
	        :   nullptr;
}

//----------------------------------------------------------------------

TilePoint HuntLocationTask::currentTargetLoc() {
	return _currentTarget;
}

/* ===================================================================== *
   HuntToBeNearLocationTask member functions
 * ===================================================================== */

HuntToBeNearLocationTask::HuntToBeNearLocationTask(Common::InSaveFile *in, TaskID id) :
	HuntLocationTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading HuntToBeNearLocationTask");

	//  Restore the range
	_range = in->readUint16LE();

	//  Restore the evaluation _counter
	_targetEvaluateCtr = in->readByte();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntToBeNearLocationTask::archiveSize() const {
	return      HuntLocationTask::archiveSize()
	            +   sizeof(_range)
	            +   sizeof(_targetEvaluateCtr);
}

void HuntToBeNearLocationTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToBeNearLocationTask");

	//  Let the base class archive its data
	HuntLocationTask::write(out);

	//  Store the range
	out->writeUint16LE(_range);

	//  Store the evaluation _counter
	out->writeByte(_targetEvaluateCtr);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 HuntToBeNearLocationTask::getType() const {
	return kHuntToBeNearLocationTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool HuntToBeNearLocationTask::operator == (const Task &t) const {
	if (t.getType() != kHuntToBeNearLocationTask) return false;

	const HuntToBeNearLocationTask *taskPtr = (const HuntToBeNearLocationTask *)&t;

	return      *getTarget() == *taskPtr->getTarget()
	            &&  _range == taskPtr->_range;
}

//----------------------------------------------------------------------

void HuntToBeNearLocationTask::evaluateTarget() {
	//  If its time to reevaluate the target, simply get the nearest
	//  target location from the LocationTarget
	if (_targetEvaluateCtr == 0) {
		Actor   *a = _stack->getActor();

		_currentTarget =
		    getTarget()->where(a->world(), a->getLocation());

		_targetEvaluateCtr = kTargetEvaluateRate;
	}
	_targetEvaluateCtr--;
}

//----------------------------------------------------------------------

bool HuntToBeNearLocationTask::atTarget() {
	TilePoint   _targetLoc = currentTargetLoc();

	//  Determine if we are within the specified range of the target
	return  _targetLoc != Nowhere
	        &&  _stack->getActor()->inRange(_targetLoc, _range);
}

//----------------------------------------------------------------------

void HuntToBeNearLocationTask::atTargetabortTask() {}

//----------------------------------------------------------------------

TaskResult HuntToBeNearLocationTask::atTargetEvaluate() {
	//  If we're at the target, we're done
	return kTaskSucceeded;
}

//----------------------------------------------------------------------

TaskResult HuntToBeNearLocationTask::atTargetUpdate() {
	//  If we're at the target, we're done
	return kTaskSucceeded;
}

/* ===================================================================== *
   HuntObjectTask member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- initial construction

HuntObjectTask::HuntObjectTask(TaskStack *ts, const ObjectTarget &ot) :
	HuntTask(ts),
	_currentTarget(nullptr) {
	assert(ot.size() <= sizeof(_targetMem));
	debugC(2, kDebugTasks, " - HuntObjectTask");
	//  Copy the target to the target buffer
	ot.clone(_targetMem);
}

HuntObjectTask::HuntObjectTask(Common::InSaveFile *in, TaskID id) : HuntTask(in, id) {
	//  Restore the current target ID
	ObjectID _currentTargetID = in->readUint16LE();

	//  Convert the ID to a GameObject pointer
	_currentTarget = _currentTargetID != Nothing
	                ?   GameObject::objectAddress(_currentTargetID)
	                :   nullptr;

	//  Reconstruct the object target
	readTarget(_targetMem, in);
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
	if (_currentTarget != nullptr)
		out->writeByte(_currentTarget->thisID());
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
	return gotoObj->getTarget() != _currentTarget;
}

//----------------------------------------------------------------------

GotoTask *HuntObjectTask::setupGoto() {
	//  If there is an object to goto, setup a GotoObjectTask, else
	//  return NULL
	return  _currentTarget
	        ?   new GotoObjectTask(_stack, _currentTarget)
	        :   nullptr;
}

//----------------------------------------------------------------------

TilePoint HuntObjectTask::currentTargetLoc() {
	//  If there is a current target object, return its locatio, else
	//  return Nowhere
	return _currentTarget ? _currentTarget->getLocation() : Nowhere;
}

/* ===================================================================== *
   HuntToBeNearObjectTask member functions
 * ===================================================================== */

HuntToBeNearObjectTask::HuntToBeNearObjectTask(Common::InSaveFile *in, TaskID id) :
	HuntObjectTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading HuntToBeNearObjectTask");

	//  Restore the range
	_range = in->readUint16LE();

	//  Restore the evaluation _counter
	_targetEvaluateCtr = in->readByte();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntToBeNearObjectTask::archiveSize() const {
	return      HuntObjectTask::archiveSize()
	            +   sizeof(_range)
	            +   sizeof(_targetEvaluateCtr);
}

void HuntToBeNearObjectTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToBeNearObjectTask");

	//  Let the base class archive its data
	HuntObjectTask::write(out);

	//  Store the range
	out->writeUint16LE(_range);

	//  Store the evaluation _counter
	out->writeByte(_targetEvaluateCtr);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 HuntToBeNearObjectTask::getType() const {
	return kHuntToBeNearObjectTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool HuntToBeNearObjectTask::operator == (const Task &t) const {
	if (t.getType() != kHuntToBeNearObjectTask) return false;

	const HuntToBeNearObjectTask *taskPtr = (const HuntToBeNearObjectTask *)&t;

	return      *getTarget() == *taskPtr->getTarget()
	            &&  _range == taskPtr->_range;
}

//----------------------------------------------------------------------

void HuntToBeNearObjectTask::evaluateTarget() {
	//  Determine if it is time to reevaluate the target object
	if (_targetEvaluateCtr == 0) {
		Actor               *a = _stack->getActor();
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
			            kMaxSenseRange,
			            objID)
			        ||  a->canSenseSpecificObjectIndirectly(
			            info,
			            kMaxSenseRange,
			            objID)) {
				_currentTarget = objArray[i];
				break;
			}
		}

		_targetEvaluateCtr = kTargetEvaluateRate;
	}

	//  Decrement the target reevaluate _counter
	_targetEvaluateCtr--;
}

//----------------------------------------------------------------------

bool HuntToBeNearObjectTask::atTarget() {
	TilePoint   _targetLoc = currentTargetLoc();

	//  Determine if we are within the specified range of the current
	//  target
	return      _targetLoc != Nowhere
	            &&  _stack->getActor()->inRange(_targetLoc, _range);
}

//----------------------------------------------------------------------

void HuntToBeNearObjectTask::atTargetabortTask() {}

//----------------------------------------------------------------------

TaskResult HuntToBeNearObjectTask::atTargetEvaluate() {
	//  If we're at the target, we're done
	return kTaskSucceeded;
}

//----------------------------------------------------------------------

TaskResult HuntToBeNearObjectTask::atTargetUpdate() {
	//  If we're at the target, we're done
	return kTaskSucceeded;
}

/* ===================================================================== *
   HuntToPossessTask member functions
 * ===================================================================== */

//  Hunt to possess in not fully implemented yet

HuntToPossessTask::HuntToPossessTask(Common::InSaveFile *in, TaskID id) : HuntObjectTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading HuntToPossessTask");

	//  Restore evaluation _counter
	_targetEvaluateCtr = in->readByte();

	//  Restore grab flag
	_grabFlag = in->readUint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntToPossessTask::archiveSize() const {
	return      HuntObjectTask::archiveSize()
	            +   sizeof(_targetEvaluateCtr)
	            +   sizeof(_grabFlag);
}

void HuntToPossessTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToPossessTask");

	//  Let the base class archive its data
	HuntObjectTask::write(out);

	//  Store the evaluation _counter
	out->writeByte(_targetEvaluateCtr);

	//  Store the grab flag
	out->writeUint16LE(_grabFlag);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 HuntToPossessTask::getType() const {
	return kHuntToPossessTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool HuntToPossessTask::operator == (const Task &t) const {
	if (t.getType() != kHuntToPossessTask) return false;

	const HuntToPossessTask *taskPtr = (const HuntToPossessTask *)&t;

	return *getTarget() == *taskPtr->getTarget();
}

//----------------------------------------------------------------------

void HuntToPossessTask::evaluateTarget() {
	//  Determine if it is time to reevaluate the target object
	if (_targetEvaluateCtr == 0) {
		Actor               *a = _stack->getActor();
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
			            kMaxSenseRange,
			            objID)
			        ||  a->canSenseSpecificObjectIndirectly(
			            info,
			            kMaxSenseRange,
			            objID)) {
				_currentTarget = objArray[i];
				break;
			}
		}

		_targetEvaluateCtr = kTargetEvaluateRate;
	}

	//  Decrement the target reevaluate _counter
	_targetEvaluateCtr--;
}

//----------------------------------------------------------------------

bool HuntToPossessTask::atTarget() {
	Actor   *a = _stack->getActor();

	return  _currentTarget
	        && (a->inReach(_currentTarget->getLocation())
	            || (_grabFlag
	                &&  a->isContaining(_currentTarget)));
}

//----------------------------------------------------------------------

void HuntToPossessTask::atTargetabortTask() {}

//----------------------------------------------------------------------

TaskResult HuntToPossessTask::atTargetEvaluate() {
	if (_currentTarget && _stack->getActor()->isContaining(_currentTarget))
		return kTaskSucceeded;

	return kTaskNotDone;
}

//----------------------------------------------------------------------

TaskResult HuntToPossessTask::atTargetUpdate() {
	//  Hunt to possess in not implemented yet
	return kTaskNotDone;
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
	_flags(trackFlag ? kTrack : 0),
	_currentTarget(nullptr) {
	assert(at.size() <= sizeof(_targetMem));
	debugC(2, kDebugTasks, " - HuntActorTask");
	//  Copy the target to the target buffer
	at.clone(_targetMem);
}

HuntActorTask::HuntActorTask(Common::InSaveFile *in, TaskID id) : HuntTask(in, id) {
	//  Restore the flags
	_flags = in->readByte();

	//  Restore the current target ID
	ObjectID _currentTargetID = in->readUint16LE();

	//  Convert the ID to a GameObject pointer
	_currentTarget = _currentTargetID != Nothing
	                ? (Actor *)GameObject::objectAddress(_currentTargetID)
	                :   nullptr;

	//  Reconstruct the object target
	readTarget(_targetMem, in);
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntActorTask::archiveSize() const {
	return      HuntTask::archiveSize()
	            +   sizeof(_flags)
	            +   sizeof(ObjectID)
	            +   targetArchiveSize(getTarget());
}

void HuntActorTask::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Let the base class archive its data
	HuntTask::write(out);

	//  Store the flags
	out->writeByte(_flags);

	//  Store the ID
	if (_currentTarget != nullptr)
		out->writeUint16LE(_currentTarget->thisID());
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
	return gotoActor->getTarget() != _currentTarget;
}

//----------------------------------------------------------------------

GotoTask *HuntActorTask::setupGoto() {
	//  If there is an actor to goto, setup a GotoActorTask, else
	//  return NULL
	/*  return  _currentTarget
	            ?   new GotoActorTask( stack, _currentTarget, flags & kTrack )
	            :   NULL;
	*/
	if (_currentTarget != nullptr) {
		return new GotoActorTask(
		           _stack,
		           _currentTarget,
		           _flags & kTrack);
	}

	return nullptr;
}

//----------------------------------------------------------------------

TilePoint HuntActorTask::currentTargetLoc() {
	//  If there is a current target actor, return its location, else
	//  return Nowhere
	return _currentTarget ? _currentTarget->getLocation() : Nowhere;
}

/* ===================================================================== *
   HuntToBeNearActorTask member functions
 * ===================================================================== */

HuntToBeNearActorTask::HuntToBeNearActorTask(Common::InSaveFile *in, TaskID id) :
	HuntActorTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading HuntToBeNearActorTask");

	//  Get the _goAway task ID
	_goAwayID = in->readSint16LE();
	_goAway = nullptr;

	//  Restore the range
	_range = in->readUint16LE();

	//  Restore the evaluation _counter
	_targetEvaluateCtr = in->readByte();
}

//----------------------------------------------------------------------
//	Fixup the subtask pointers

void HuntToBeNearActorTask::fixup() {
		//	Let the base class fixup its pointers
	HuntActorTask::fixup();

	//  Convert the task ID to a task pointer
	_goAway = _goAwayID != NoTask
	         ? (GoAwayFromObjectTask *)getTaskAddress(_goAwayID)
	         :   nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntToBeNearActorTask::archiveSize() const {
	return      HuntActorTask::archiveSize()
	            +   sizeof(TaskID)               //  _goAway ID
	            +   sizeof(_range)
	            +   sizeof(_targetEvaluateCtr);
}

void HuntToBeNearActorTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToBeNearActorTask");

	//  Let the base class archive its data
	HuntActorTask::write(out);

	//  Store the task ID
	if (_goAway != nullptr)
		out->writeSint16LE(getTaskID(_goAway));
	else
		out->writeSint16LE(NoTask);

	//  Store the range
	out->writeUint16LE(_range);

	//  Store the evaluation _counter
	out->writeByte(_targetEvaluateCtr);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 HuntToBeNearActorTask::getType() const {
	return kHuntToBeNearActorTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool HuntToBeNearActorTask::operator == (const Task &t) const {
	if (t.getType() != kHuntToBeNearActorTask) return false;

	const HuntToBeNearActorTask *taskPtr = (const HuntToBeNearActorTask *)&t;

	return      *getTarget() == *taskPtr->getTarget()
	            &&  tracking() ? taskPtr->tracking() : !taskPtr->tracking()
	            &&  _range == taskPtr->_range;
}

//----------------------------------------------------------------------

void HuntToBeNearActorTask::evaluateTarget() {
	//  Determine if its time to reevaluate the current target actor
	if (_targetEvaluateCtr == 0) {
		Actor               *a = _stack->getActor();
		int16               i;
		Actor               *_actorArray[16];
		int16               distArray[ARRAYSIZE(_actorArray)];
		TargetActorArray    taa(
		    ARRAYSIZE(_actorArray),
		    _actorArray,
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
			            kMaxSenseRange,
			            _actorArray[i])
			        ||  a->canSenseSpecificActorIndirectly(
			            info,
			            kMaxSenseRange,
			            _actorArray[i])) {
				if (_currentTarget != _actorArray[i]) {
					if (atTarget()) atTargetabortTask();
					_currentTarget = _actorArray[i];
				}

				break;
			}
		}

		_targetEvaluateCtr = kTargetEvaluateRate;
	}

	//  Decrement the target reevaluation _counter.
	_targetEvaluateCtr--;
}

//----------------------------------------------------------------------

bool HuntToBeNearActorTask::atTarget() {
	TilePoint   targetLoc = currentTargetLoc();

	//  Determine if we're within the specified range of the current
	//  target actor
	if (targetLoc != Nowhere
	        &&  _stack->getActor()->inRange(targetLoc, _range))
		return true;
	else {
		if (_goAway != nullptr) {
			_goAway->abortTask();
			delete _goAway;
			_goAway = nullptr;
		}

		return false;
	}
}

//----------------------------------------------------------------------

void HuntToBeNearActorTask::atTargetabortTask() {
	if (_goAway != nullptr) {
		_goAway->abortTask();
		delete _goAway;
		_goAway = nullptr;
	}
}

//----------------------------------------------------------------------

TaskResult HuntToBeNearActorTask::atTargetEvaluate() {
	TilePoint   _targetLoc = currentTargetLoc();

	//  If we're not TOO close, we're done
	if (_stack->getActor()->inRange(_targetLoc, kTooClose))
		return kTaskNotDone;

	if (_goAway != nullptr) {
		_goAway->abortTask();
		delete _goAway;
		_goAway = nullptr;
	}

	return kTaskSucceeded;
}

//----------------------------------------------------------------------

TaskResult HuntToBeNearActorTask::atTargetUpdate() {
	Actor       *a = _stack->getActor();
	TilePoint   _targetLoc = currentTargetLoc();

	//  Determine if we're TOO close
	if (a->inRange(_targetLoc, kTooClose)) {
		//  Setup a go away task if necessary and update it
		if (_goAway == nullptr) {
			_goAway = new GoAwayFromObjectTask(_stack, _currentTarget);
			if (_goAway != nullptr) _goAway->update();
		} else
			_goAway->update();

		return kTaskNotDone;
	}

	//  Delete the go away task if it exists
	if (_goAway != nullptr) {
		_goAway->abortTask();
		delete _goAway;
		_goAway = nullptr;
	}

	return kTaskSucceeded;
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
	_targetEvaluateCtr(0),
	_specialAttackCtr(10),
	_flags(kEvalWeapon) {
	debugC(2, kDebugTasks, " - HuntToKillTask");
	Actor       *a = _stack->getActor();

	if (isActor(a->_currentTarget))
		_currentTarget = (Actor *)a->_currentTarget;

	a->setFightStance(true);
}

HuntToKillTask::HuntToKillTask(Common::InSaveFile *in, TaskID id) : HuntActorTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading HuntToKillTask");

	//  Restore the evaluation _counter
	_targetEvaluateCtr = in->readByte();
	_specialAttackCtr = in->readByte();
	_flags = in->readByte();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntToKillTask::archiveSize() const {
	return      HuntActorTask::archiveSize()
	            +   sizeof(_targetEvaluateCtr)
	            +   sizeof(_specialAttackCtr)
	            +   sizeof(_flags);
}

void HuntToKillTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToKillTask");

	//  Let the base class archive its data
	HuntActorTask::write(out);

	//  Store the evaluation _counter
	out->writeByte(_targetEvaluateCtr);
	out->writeByte(_specialAttackCtr);
	out->writeByte(_flags);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 HuntToKillTask::getType() const {
	return kHuntToKillTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool HuntToKillTask::operator == (const Task &t) const {
	if (t.getType() != kHuntToKillTask) return false;

	const HuntToKillTask *taskPtr = (const HuntToKillTask *)&t;

	return      *getTarget() == *taskPtr->getTarget()
	            &&  tracking() ? taskPtr->tracking() : !taskPtr->tracking();
}

//----------------------------------------------------------------------

void HuntToKillTask::abortTask() {
	HuntActorTask::abortTask();

	Actor       *a = _stack->getActor();

	a->_flags &= ~Actor::kAFSpecialAttack;

	a->setFightStance(false);
}

//----------------------------------------------------------------------

TaskResult HuntToKillTask::update() {
	if (_specialAttackCtr == 0) {
		_stack->getActor()->_flags |= Actor::kAFSpecialAttack;
		//  A little hack to make monsters with 99 spellcraft cast spells more often
		if (_stack->getActor()->getStats()->spellcraft >= 99)
			_specialAttackCtr = 3;
		else _specialAttackCtr = 10;
	} else
		_specialAttackCtr--;

	return HuntActorTask::update();
}

//----------------------------------------------------------------------

void HuntToKillTask::evaluateTarget() {
	Actor               *a = _stack->getActor();

	if (_flags & kEvalWeapon && a->isInterruptable()) {
		evaluateWeapon();
		_flags &= ~kEvalWeapon;
	}


	//  Determine if its time to reevaluate the current target actor
	if (_targetEvaluateCtr == 0
	        || (_currentTarget != nullptr
	            &&  _currentTarget->isDead())) {
		Actor               *bestTarget = nullptr;
		ActorProto          *proto = (ActorProto *)a->proto();
		int16               i;
		Actor               *_actorArray[16];
		int16               distArray[ARRAYSIZE(_actorArray)];
		TargetActorArray    taa(
		    ARRAYSIZE(_actorArray),
		    _actorArray,
		    distArray);
		SenseInfo           info;

		//  Get an array of actor pointers from the ActorTarget
		getTarget()->actor(a->world(), a->getLocation(), taa);

		switch (proto->combatBehavior) {
		case kBehaviorHungry:
			//  Iterate through each actor in the array and determine if
			//  there is a line of sight to that actor
			for (i = 0; i < taa.actors; i++) {
				if (_actorArray[i]->isDead()) continue;

				if (tracking()
				        ||  a->canSenseSpecificActor(
				            info,
				            kMaxSenseRange,
				            _actorArray[i])
				        ||  a->canSenseSpecificActorIndirectly(
				            info,
				            kMaxSenseRange,
				            _actorArray[i])) {
					bestTarget = _actorArray[i];
					break;
				}
			}
			break;

		case kBehaviorCowardly: {
			int16       bestScore = 0;

			for (i = 0; i < taa.actors; i++) {
				if (_actorArray[i]->isDead()) continue;

				if (tracking()
				        ||  a->canSenseSpecificActor(
				            info,
				            kMaxSenseRange,
				            _actorArray[i])
				        ||  a->canSenseSpecificActorIndirectly(
				            info,
				            kMaxSenseRange,
				            _actorArray[i])) {
					int16   score;

					score =     closenessScore(distArray[i]) * 16
					            /   _actorArray[i]->defenseScore();

					if (score > bestScore || bestTarget == nullptr) {
						bestScore = score;
						bestTarget = _actorArray[i];
					}
				}
			}
		}
		break;

		case kBehaviorBerserk: {
			int16       bestScore = 0;

			for (i = 0; i < taa.actors; i++) {
				if (_actorArray[i]->isDead()) continue;

				if (tracking()
				        ||  a->canSenseSpecificActor(
				            info,
				            kMaxSenseRange,
				            _actorArray[i])
				        ||  a->canSenseSpecificActorIndirectly(
				            info,
				            kMaxSenseRange,
				            _actorArray[i])) {
					int16   score;

					score =     closenessScore(distArray[i])
					            *   _actorArray[i]->offenseScore();

					if (score > bestScore || bestTarget == nullptr) {
						bestScore = score;
						bestTarget = _actorArray[i];
					}
				}
			}
		}
		break;

		case kBehaviorSmart: {
			int16       bestScore = 0;

			for (i = 0; i < taa.actors; i++) {
				if (_actorArray[i]->isDead()) continue;

				if (tracking()
				        ||  a->canSenseSpecificActor(
				            info,
				            kMaxSenseRange,
				            _actorArray[i])
				        ||  a->canSenseSpecificActorIndirectly(
				            info,
				            kMaxSenseRange,
				            _actorArray[i])) {
					int16   score;

					score =     closenessScore(distArray[i])
					            *   _actorArray[i]->offenseScore()
					            /   _actorArray[i]->defenseScore();

					if (score > bestScore || bestTarget == nullptr) {
						bestScore = score;
						bestTarget = _actorArray[i];
					}
				}
			}
		}
		break;
		}

		if (bestTarget != _currentTarget) {
			//  If the current target has changed, abort any
			//  action currently taking place
			if (atTarget()) atTargetabortTask();
			_currentTarget = bestTarget;
			a->_currentTarget = _currentTarget;
		}

		_flags |= kEvalWeapon;

		_targetEvaluateCtr = kTargetEvaluateRate;
	}

	//  Decrement the target reevaluation _counter
	_targetEvaluateCtr--;
}

//----------------------------------------------------------------------

bool HuntToKillTask::atTarget() {
	//  Determine if we're in attack range of the current target
	return      _currentTarget != nullptr
	            &&  _stack->getActor()->inAttackRange(
	                _currentTarget->getLocation());
}

//----------------------------------------------------------------------

void HuntToKillTask::atTargetabortTask() {
	//  If the task is aborted while at the target actor, abort any
	//  attack currently taking place
	_stack->getActor()->stopAttack(_currentTarget);
}

//----------------------------------------------------------------------

TaskResult HuntToKillTask::atTargetEvaluate() {
	//  This task is never done and must be aborted manually
	return kTaskNotDone;
}

//----------------------------------------------------------------------

TaskResult HuntToKillTask::atTargetUpdate() {
	assert(isActor(_currentTarget));

	Actor   *a = _stack->getActor();

	//  If we're ready to attack, attack
	if (a->isInterruptable() && g_vm->_rnd->getRandomNumber(7) == 0) {
		a->attack(_currentTarget);
		_flags |= kEvalWeapon;
	}

	return kTaskNotDone;
}

//----------------------------------------------------------------------

void HuntToKillTask::evaluateWeapon() {
	Actor               *a = _stack->getActor();
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

		if (_currentTarget == nullptr) {
			warning("%s: _currentTarget = NULL (return)", a->objName());
			return;
		}

		if (currentWeapon == nullptr
		        ||      weaponProto->weaponRating(
		            a->thisID(),
		            actorID,
		            _currentTarget->thisID())
		        !=  0)
			return;
	}

	while (iter.next(&obj) != Nothing) {
		ProtoObj        *proto = obj->proto();
		uint16          cSet = proto->containmentSet();

		//  Simply use all armor objects
		if (!isPlayerActor(a) && (cSet & ProtoObj::kIsArmor)) {
			if (proto->useSlotAvailable(obj, a))
				obj->use(actorID);
			continue;
		}

		if (cSet & ProtoObj::kIsWeapon) {
			WeaponProto     *weaponProto = (WeaponProto *)proto;
			int             weaponRating;

			if (_currentTarget) {
				warning("%s: _currentTarget = NULL (weaponRating = 0)", a->objName());
				weaponRating =  weaponProto->weaponRating(obj->thisID(),
				                                          actorID,
				                                          _currentTarget->thisID());
			} else
				weaponRating = 0;

			//  a rating of zero means this weapon is useless
			if (weaponRating == 0) continue;

			if (obj == currentWeapon)
				weaponRating += kCurrentWeaponBonus;

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
	_objToGive = objToGiveID != Nothing
	            ?   GameObject::objectAddress(objToGiveID)
	            :   nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 HuntToGiveTask::archiveSize() const {
	return      HuntActorTask::archiveSize()
	            +   sizeof(ObjectID);                //  _objToGive ID
}

void HuntToGiveTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving HuntToGiveTask");

	//  Let base class archive its data
	HuntActorTask::write(out);

	//  Store the ID
	if (_objToGive != nullptr)
		out->writeUint16LE(_objToGive->thisID());
	else
		out->writeUint16LE(Nothing);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 HuntToGiveTask::getType() const {
	return kHuntToGiveTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool HuntToGiveTask::operator == (const Task &t) const {
	if (t.getType() != kHuntToGiveTask) return false;

	const HuntToGiveTask *taskPtr = (const HuntToGiveTask *)&t;

	return      *getTarget() == *taskPtr->getTarget()
	            &&  tracking() ? taskPtr->tracking() : !taskPtr->tracking()
	            &&  _objToGive == taskPtr->_objToGive;
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
	return kTaskNotDone;
}

//----------------------------------------------------------------------

TaskResult HuntToGiveTask::atTargetUpdate() {
	return kTaskNotDone;
}

/* ===================================================================== *
   BandTask member functions
 * ===================================================================== */

//----------------------------------------------------------------------

bool BandTask::BandingRepulsorIterator::first(
    TilePoint   &repulsorVector,
    int16       &repulsorStrength) {
	assert(_a->_leader != nullptr && _a->_leader->_followers != nullptr);

	_band = _a->_leader->_followers;
	_bandIndex = 0;

	while (_bandIndex < _band->size()) {
		Actor       *_bandMember = (*_band)[_bandIndex];

		if (_bandMember != _a) {
			repulsorVector = _bandMember->getLocation() - _a->getLocation();
			repulsorStrength = 1;

			return true;
		}

		_bandIndex++;
	}

	return false;
}

//----------------------------------------------------------------------

bool BandTask::BandingRepulsorIterator::next(
    TilePoint   &repulsorVector,
    int16       &repulsorStrength) {
	assert(_a->_leader != nullptr && _a->_leader->_followers != nullptr);
	assert(_band == _a->_leader->_followers);
	assert(_bandIndex < _band->size());

	_bandIndex++;
	while (_bandIndex < _band->size()) {
		Actor       *_bandMember = (*_band)[_bandIndex];

		if (_bandMember != _a) {
			repulsorVector = _bandMember->getLocation() - _a->getLocation();
			repulsorStrength = 1;

			return true;
		}

		_bandIndex++;
	}

	return false;
}

BandTask::BandTask(Common::InSaveFile *in, TaskID id) : HuntTask(in, id) {
	debugC(3, kDebugSaveload, "... Loading BandTask");

	_attendID = in->readSint16LE();
	_attend = nullptr;

	//  Restore the current target location
	_currentTarget.load(in);

	//  Restore the target evaluation _counter
	_targetEvaluateCtr = in->readByte();
}

//----------------------------------------------------------------------
//	Fixup the subtask pointers

void BandTask::fixup() {
	//	Let the base class fixup its pointers
	HuntTask::fixup();

	//  Convert the TaskID to a Task pointer
	_attend = _attendID != NoTask
	         ? (AttendTask *)getTaskAddress(_attendID)
	         :   nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 BandTask::archiveSize() const {
	return      HuntTask::archiveSize()
	            +   sizeof(TaskID)           //  _attend ID
	            +   sizeof(_currentTarget)
	            +   sizeof(_targetEvaluateCtr);
}

void BandTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving BandTask");

	//  Let the base class archive its data
	HuntTask::write(out);

	//  Store the _attend task ID
	if (_attend != nullptr)
		out->writeSint16LE(getTaskID(_attend));
	else
		out->writeSint16LE(NoTask);

	//  Store the current target location
	_currentTarget.write(out);

	//  Store the target evaluation _counter
	out->writeByte(_targetEvaluateCtr);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 BandTask::getType() const {
	return kBandTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool BandTask::operator == (const Task &t) const {
	return t.getType() == kBandTask;
}

//----------------------------------------------------------------------

void BandTask::evaluateTarget() {
	if (_targetEvaluateCtr == 0) {
		Actor           *leader = _stack->getActor()->_leader;
		TilePoint       actorLoc = _stack->getActor()->getLocation(),
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

		//  Count the leader as two _band members to double his
		//  repulsion
		repulsorVectorArray[0] = leader->getLocation() - actorLoc;
		repulsorStrengthArray[0] = 3;
		repulsorDistArray[0] = repulsorVectorArray[0].quickHDistance();
		repulsorCount = 1;

		//  Iterate through the _band members, adding their locations
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

		_currentTarget = actorLoc + movementVector;
		_currentTarget.z = leader->getLocation().z;

		_targetEvaluateCtr = kTargetEvaluateRate;
	}

	_targetEvaluateCtr--;
}

//----------------------------------------------------------------------

bool BandTask::targetHasChanged(GotoTask *gotoTarget) {
	GotoLocationTask    *gotoLocation = (GotoLocationTask *)gotoTarget;
	TilePoint           actorLoc = _stack->getActor()->getLocation(),
	                    oldTarget = gotoLocation->getTarget();
	int16               slop;

	slop = ((_currentTarget - actorLoc).quickHDistance()
	        +   ABS(_currentTarget.z - actorLoc.z))
	       /   2;

	if ((_currentTarget - oldTarget).quickHDistance()
	        +   ABS(_currentTarget.z - oldTarget.z)
	        >   slop)
		gotoLocation->changeTarget(_currentTarget);

	return false;
}

//----------------------------------------------------------------------

GotoTask *BandTask::setupGoto() {
	return new GotoLocationTask(_stack, _currentTarget, getRunThreshold());
}

//----------------------------------------------------------------------

TilePoint BandTask::currentTargetLoc() {
	return _currentTarget;
}

//----------------------------------------------------------------------

bool BandTask::atTarget() {
	TilePoint       actorLoc = _stack->getActor()->getLocation();

	if ((actorLoc - _currentTarget).quickHDistance() > 6
	        ||  ABS(actorLoc.z - _currentTarget.z) > kMaxStepHeight) {
		if (_attend != nullptr) {
			_attend->abortTask();
			delete _attend;
			_attend = nullptr;
		}

		return false;
	}

	return true;
}

//----------------------------------------------------------------------

void BandTask::atTargetabortTask() {
	if (_attend != nullptr) {
		_attend->abortTask();
		delete _attend;
		_attend = nullptr;
	}
}

//----------------------------------------------------------------------

TaskResult BandTask::atTargetEvaluate() {
	return kTaskNotDone;
}

//----------------------------------------------------------------------

TaskResult BandTask::atTargetUpdate() {
	Actor       *a = _stack->getActor();

	if (_attend != nullptr)
		_attend->update();
	else {
		_attend = new AttendTask(_stack, a->_leader);
		if (_attend != nullptr)
			_attend->update();
	}

	return kTaskNotDone;
}

//----------------------------------------------------------------------

int16 BandTask::getRunThreshold() {
	return kTileUVSize * 3;
}

//----------------------------------------------------------------------

BandTask::RepulsorIterator *BandTask::getNewRepulsorIterator() {
	return new BandingRepulsorIterator(_stack->getActor());
}

/* ===================================================================== *
   BandAndAvoidEnemiesTask member functions
 * ===================================================================== */

//----------------------------------------------------------------------

//bool BandAndAvoidEnemiesTask::BandAndAvoidEnemiesRepulsorIterator::firstEnemyRepulsor(
bool BandTask::BandAndAvoidEnemiesRepulsorIterator::firstEnemyRepulsor(
    TilePoint   &repulsorVector,
    int16       &repulsorStrength) {
	assert(_iteratingThruEnemies);

	int16                   actorDistArray[ARRAYSIZE(_actorArray)];
	TargetActorArray        taa(ARRAYSIZE(_actorArray), _actorArray, actorDistArray);
	ActorPropertyTarget     target(kActorPropIDEnemy);

	_numActors = target.actor(_a->world(), _a->getLocation(), taa);

	assert(_numActors == taa.actors);

	_actorIndex = 0;

	if (_actorIndex < _numActors) {
		repulsorVector =
		    _actorArray[_actorIndex]->getLocation() - _a->getLocation();
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
	assert(_iteratingThruEnemies);

	_actorIndex++;

	if (_actorIndex < _numActors) {
		repulsorVector =
		    _actorArray[_actorIndex]->getLocation() - _a->getLocation();
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
	_iteratingThruEnemies = false;

	if (BandingRepulsorIterator::first(repulsorVector, repulsorStrength))
		return true;

	_iteratingThruEnemies = true;
	return firstEnemyRepulsor(repulsorVector, repulsorStrength);
}

//----------------------------------------------------------------------

//bool BandAndAvoidEnemiesTask::BandAndAvoidEnemiesRepulsorIterator::first(
bool BandTask::BandAndAvoidEnemiesRepulsorIterator::next(
    TilePoint   &repulsorVector,
    int16       &repulsorStrength) {
	if (!_iteratingThruEnemies) {
		if (BandingRepulsorIterator::next(repulsorVector, repulsorStrength))
			return true;

		_iteratingThruEnemies = true;
		return firstEnemyRepulsor(repulsorVector, repulsorStrength);
	}

	return nextEnemyRepulsor(repulsorVector, repulsorStrength);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 BandAndAvoidEnemiesTask::getType() const {
	return kBandAndAvoidEnemiesTask;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool BandAndAvoidEnemiesTask::operator == (const Task &t) const {
	return t.getType() == kBandAndAvoidEnemiesTask;
}

//----------------------------------------------------------------------

int16 BandAndAvoidEnemiesTask::getRunThreshold() {
	return 0;
}

//----------------------------------------------------------------------

BandTask::RepulsorIterator *BandAndAvoidEnemiesTask::getNewRepulsorIterator() {
	return new BandAndAvoidEnemiesRepulsorIterator(_stack->getActor());
}

/* ===================================================================== *
   FollowPatrolRouteTask member functions
 * ===================================================================== */

FollowPatrolRouteTask::FollowPatrolRouteTask(Common::InSaveFile *in, TaskID id) : Task(in, id) {
	debugC(3, kDebugSaveload, "... Loading FollowPatrolRouteTask");

	//  Get the _gotoWayPoint TaskID
	_gotoWayPointID = in->readSint16LE();
	_gotoWayPoint = nullptr;

	//  Restore the patrol route iterator
	_patrolIter.read(in);

	//  Restore the last waypoint number
	_lastWayPointNum = in->readSint16LE();

	//  Restore the _paused flag
	_paused = in->readUint16LE();

	//  Restore the _paused _counter
	_counter = in->readSint16LE();
}

//----------------------------------------------------------------------
//	Fixup the subtask pointers

void FollowPatrolRouteTask::fixup() {
	//	Let the base class fixup its pointers
	Task::fixup();

	//  Convert the TaskID to a Task pointer
	_gotoWayPoint = _gotoWayPointID != NoTask
	               ? (GotoLocationTask *)getTaskAddress(_gotoWayPointID)
	               :   nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 FollowPatrolRouteTask::archiveSize() const {
	return      Task::archiveSize()
	            +   sizeof(TaskID)   //  _gotoWayPoint ID
	            +   sizeof(_patrolIter)
	            +   sizeof(_lastWayPointNum)
	            +   sizeof(_paused)
	            +   sizeof(_counter);
}

void FollowPatrolRouteTask::write(Common::MemoryWriteStreamDynamic *out) const {
	debugC(3, kDebugSaveload, "... Saving FollowPatrolRouteTask");

	//  Let the base class archive its data
	Task::write(out);

	//  Store the _gotoWayPoint ID
	if (_gotoWayPoint != nullptr)
		out->writeSint16LE(getTaskID(_gotoWayPoint));
	else
		out->writeSint16LE(NoTask);

	//  Store the PatrolRouteIterator
	_patrolIter.write(out);

	//  Store the last waypoint number
	out->writeSint16LE(_lastWayPointNum);

	//  Store the _paused flag
	out->writeUint16LE(_paused);

	//  Store the _paused _counter
	out->writeSint16LE(_counter);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 FollowPatrolRouteTask::getType() const {
	return kFollowPatrolRouteTask;
}

//----------------------------------------------------------------------

void FollowPatrolRouteTask::abortTask() {
	//  If there is a subtask, get rid of it
	if (_gotoWayPoint) {
		_gotoWayPoint->abortTask();
		delete _gotoWayPoint;
		_gotoWayPoint = nullptr;
	}
}

//----------------------------------------------------------------------

TaskResult FollowPatrolRouteTask::evaluate() {
	//  Simply check the patrol iterator to determine if there are
	//  any more waypoints
	return *_patrolIter == Nowhere ? kTaskSucceeded : kTaskNotDone;
}

//----------------------------------------------------------------------

TaskResult FollowPatrolRouteTask::update() {
	return !_paused ? handleFollowPatrolRoute() : handlePaused();
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool FollowPatrolRouteTask::operator == (const Task &t) const {
	if (t.getType() != kFollowPatrolRouteTask) return false;

	const FollowPatrolRouteTask *taskPtr = (const FollowPatrolRouteTask *)&t;

	return      _patrolIter == taskPtr->_patrolIter
	            &&  _lastWayPointNum == taskPtr->_lastWayPointNum;
}

//----------------------------------------------------------------------
//	Update function used if this task is not _paused

TaskResult FollowPatrolRouteTask::handleFollowPatrolRoute() {
	TilePoint   currentWayPoint = *_patrolIter,
	            actorLoc = _stack->getActor()->getLocation();

	if (currentWayPoint == Nowhere) return kTaskSucceeded;

	//  Determine if the actor has reached the waypoint tile position
	if ((actorLoc.u >> kTileUVShift)
	        == (currentWayPoint.u >> kTileUVShift)
	        && (actorLoc.v >> kTileUVShift)
	        == (currentWayPoint.v >> kTileUVShift)
	        &&  ABS(actorLoc.z - currentWayPoint.z) <= kMaxStepHeight) {
		//  Delete the _gotoWayPoint task
		if (_gotoWayPoint != nullptr) {
			_gotoWayPoint->abortTask();
			delete _gotoWayPoint;
			_gotoWayPoint = nullptr;
		}

		//  If this way point is the specified last way point,
		//  return success
		if (_lastWayPointNum != -1
		        &&  _patrolIter.wayPointNum() == _lastWayPointNum)
			return kTaskSucceeded;

		//  If there are no more way points in the patrol route, return
		//  success
		if ((currentWayPoint = *++_patrolIter) == Nowhere)
			return kTaskSucceeded;

		//  We are at a way point so randomly determine if we should
		//  pause for a while.
		if (g_vm->_rnd->getRandomNumber(3) == 0) {
			pause();
			return kTaskNotDone;
		}
	}

	//  Setup a _gotoWayPoint task if one doesn't already exist and
	//  update it
	if (_gotoWayPoint != nullptr)
		_gotoWayPoint->update();
	else {
		_gotoWayPoint = new GotoLocationTask(_stack, currentWayPoint);
		if (_gotoWayPoint != nullptr) _gotoWayPoint->update();
	}

	return kTaskNotDone;
}

//----------------------------------------------------------------------
//	Update function used if this task is _paused

TaskResult FollowPatrolRouteTask::handlePaused() {
	TaskResult      result;

	if ((result = evaluate()) == kTaskNotDone) {
		if (_counter == 0)
			followPatrolRoute();
		else
			_counter--;
	}

	return result;
}

//----------------------------------------------------------------------
//	Set this task into the _paused state

void FollowPatrolRouteTask::pause() {
	_paused = true;
	_counter = (g_vm->_rnd->getRandomNumber(63) + g_vm->_rnd->getRandomNumber(63)) / 2;
}

/* ===================================================================== *
   AttendTask member functions
 * ===================================================================== */

AttendTask::AttendTask(Common::InSaveFile *in, TaskID id) : Task(in, id) {
	debugC(3, kDebugSaveload, "... Loading AttendTask");

	//  Get the object ID
	ObjectID objID = in->readUint16LE();

	//  Convert the object ID to a pointer
	_obj = objID != Nothing
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
	if (_obj != nullptr)
		out->writeUint16LE(_obj->thisID());
	else
		out->writeUint16LE(Nothing);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of this task

int16 AttendTask::getType() const {
	return kAttendTask;
}

//----------------------------------------------------------------------

void AttendTask::abortTask() {
	MotionTask  *actorMotion = _stack->getActor()->_moveTask;

	//  Determine if we need to abort the actor motion
	if (actorMotion != nullptr && actorMotion->isTurn())
		actorMotion->finishTurn();
}

//----------------------------------------------------------------------

TaskResult AttendTask::evaluate() {
	//  Attending must be stopped manually
	return kTaskNotDone;
}

//----------------------------------------------------------------------

TaskResult AttendTask::update() {
	Actor       *a = _stack->getActor();
	TilePoint   _attendLoc = _obj->getWorldLocation();

	//  Determine if we are facing the object
	if (a->_currentFacing != (_attendLoc - a->getLocation()).quickDir()) {
		//  If not, turn
		if (!a->_moveTask || !a->_moveTask->isTurn())
			MotionTask::turnTowards(*a, _attendLoc);
	}

	return kTaskNotDone;
}

//----------------------------------------------------------------------
//	Determine if the specified task is equivalent to this task

bool AttendTask::operator == (const Task &t) const {
	if (t.getType() != kAttendTask) return false;

	const AttendTask *taskPtr = (const AttendTask *)&t;

	return _obj == taskPtr->_obj;
}

/* ===================================================================== *
   TaskStack member functions
 * ===================================================================== */

void TaskStack::write(Common::MemoryWriteStreamDynamic *out) {
	//  Store the stack bottom TaskID
	out->writeSint16LE(_stackBottomID);

	//  Store the actor's id
	out->writeUint16LE(_actor->thisID());

	//  Store the _evalCount and _evalRate
	out->writeSint16LE(_evalCount);

	out->writeSint16LE(_evalRate);

	debugC(4, kDebugSaveload, "...... stackBottomID = %d", _stackBottomID);
	debugC(4, kDebugSaveload, "...... actorID = %d", _actor->thisID());
	debugC(4, kDebugSaveload, "...... evalCount = %d", _evalCount);
	debugC(4, kDebugSaveload, "...... evalRate = %d", _evalRate);
}

void TaskStack::read(Common::InSaveFile *in) {
	ObjectID actorID;

	//  Restore the stack bottom pointer
	_stackBottomID = in->readSint16LE();

	//  Restore the actor pointer
	actorID = in->readUint16LE();
	_actor = (Actor *)GameObject::objectAddress(actorID);

	//  Restore the evaluation count
	_evalCount = in->readSint16LE();

	//  Restore the evaluation rate
	_evalRate = in->readSint16LE();

	debugC(4, kDebugSaveload, "...... stackBottomID = %d", _stackBottomID);
	debugC(4, kDebugSaveload, "...... actorID = %d", actorID);
	debugC(4, kDebugSaveload, "...... evalCount = %d", _evalCount);
	debugC(4, kDebugSaveload, "...... evalRate = %d", _evalRate);
}

//----------------------------------------------------------------------
//	Set the bottom task of this task stack

void TaskStack::setTask(Task *t) {
	assert(_stackBottomID == NoTask);

	if (t->_stack == this) {
		TaskID      id = getTaskID(t);

		_stackBottomID = id;
	}
}

//----------------------------------------------------------------------
//  Abort all tasks in stack

void TaskStack::abortTask() {
	if (_stackBottomID != NoTask) {
		Task    *stackBottom = getTaskAddress(_stackBottomID);

		stackBottom->abortTask();
		delete stackBottom;
	}
}

//----------------------------------------------------------------------
//  Re-evaluate tasks in stack

TaskResult TaskStack::evaluate() {
	if (_stackBottomID != -1) {
		Task    *stackBottom = getTaskAddress(_stackBottomID);

		return stackBottom->evaluate();
	} else
		return kTaskNotDone;
}

//----------------------------------------------------------------------
//  Update the state of the tasks in stack

TaskResult TaskStack::update() {
	TaskResult  result;

	//  If the actor is currently uniterruptable then this task is _paused
	if (!_actor->isInterruptable())
		return kTaskNotDone;

	if (_stackBottomID != NoTask) {
		Task    *stackBottom = getTaskAddress(_stackBottomID);

		//  Determine if it is time to reevaluate the tasks
		if (--_evalCount == 0) {
			if ((result = stackBottom->evaluate()) != kTaskNotDone) {
				delete stackBottom;
				_stackBottomID = NoTask;

				return result;
			}
			_evalCount = _evalRate;
		}

		//  Update the tasks
		if ((result = stackBottom->update()) != kTaskNotDone) {
			delete stackBottom;
			_stackBottomID = NoTask;

			return result;
		}
	} else
		return kTaskFailed;

	return kTaskNotDone;
}

} // end of namespace Saga2
