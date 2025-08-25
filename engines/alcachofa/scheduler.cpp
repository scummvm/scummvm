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
 */

#include "alcachofa/scheduler.h"

#include "common/system.h"
#include "alcachofa/alcachofa.h"
#include "alcachofa/menu.h"

using namespace Common;

namespace Alcachofa {

TaskReturn::TaskReturn() {
	_type = TaskReturnType::Yield;
	_returnValue = 0;
	_taskToWaitFor = nullptr;
}

TaskReturn TaskReturn::finish(int32 returnValue) {
	TaskReturn r;
	r._type = TaskReturnType::Finished;
	r._returnValue = returnValue;
	return r;
}

TaskReturn TaskReturn::waitFor(Task *task) {
	assert(task != nullptr);
	TaskReturn r;
	r._type = TaskReturnType::Waiting;
	r._taskToWaitFor = task;
	return r;
}

Task::Task(Process &process) : _process(process) {}

Task *Task::delay(uint32 millis) {
	return new DelayTask(process(), millis);
}

void Task::syncGame(Serializer &s) {
	s.syncAsUint32LE(_stage);
}

void Task::syncObjectAsString(Serializer &s, ObjectBase *&object, bool optional) const {
	String objectName, roomName;
	if (object != nullptr) {
		roomName = object->room()->name();
		objectName = object->name();
	}
	s.syncString(roomName);
	s.syncString(objectName);
	if (s.isSaving())
		return;
	Room *room = g_engine->world().getRoomByName(roomName.c_str());
	object = room == nullptr ? nullptr : room->getObjectByName(objectName.c_str());
	if (object == nullptr) // main characters are not linked by the room they are in
		object = g_engine->world().globalRoom().getObjectByName(objectName.c_str());
	if (object == nullptr && !optional)
		error("Invalid object name \"%s\" in room \"%s\" in savestate for task %s",
			objectName.c_str(), roomName.c_str(), taskName());
}

void Task::errorForUnexpectedObjectType(const ObjectBase *base) const {
	// Implemented as separate function in order to access ObjectBase methods
	error("Unexpected type of object %s in savestate for task %s (got a %s)",
		base->name().c_str(), taskName(), base->typeName());
}

Process::Process(ProcessId pid, MainCharacterKind characterKind)
	: _pid(pid)
	, _character(characterKind)
	, _name("Unnamed process") {}

Process::Process(Serializer &s) {
	syncGame(s);
}

Process::~Process() {
	while (!_tasks.empty())
		delete _tasks.pop();
}

bool Process::isActiveForPlayer() const {
	return _character == MainCharacterKind::None || _character == g_engine->player().activeCharacterKind();
}

TaskReturnType Process::run() {
	while (!_tasks.empty()) {
		TaskReturn ret = _tasks.top()->run();
		switch (ret.type()) {
		case TaskReturnType::Yield:
			return TaskReturnType::Yield;
		case TaskReturnType::Waiting:
			_tasks.push(ret.taskToWaitFor());
			break;
		case TaskReturnType::Finished:
			_lastReturnValue = ret.returnValue();
			delete _tasks.pop();
			break;
		default:
			assert(false && "Invalid task return type");
			return TaskReturnType::Finished;
		}
	}
	return TaskReturnType::Finished;
}

void Process::debugPrint() {
	auto *debugger = g_engine->getDebugger();
	const char *characterName;
	switch (_character) {
	case MainCharacterKind::None:
		characterName = "    <none>";
		break;
	case MainCharacterKind::Filemon:
		characterName = " Filemon";
		break;
	case MainCharacterKind::Mortadelo:
		characterName = "Mortadelo";
		break;
	default:
		characterName = "<invalid>";
		break;
	}
	debugger->debugPrintf("pid: %3u char: %s ret: %2d \"%s\"\n", _pid, characterName, _lastReturnValue, _name.c_str());

	for (uint i = 0; i < _tasks.size(); i++) {
		debugger->debugPrintf("    %u: ", i);
		_tasks[i]->debugPrint();
	}
}

#define DEFINE_TASK(TaskName) \
	extern Task *constructTask_##TaskName(Process &process, Serializer &s);
#include "alcachofa/tasks.h"

static Task *readTask(Process &process, Serializer &s) {
	assert(s.isLoading());
	String taskName;
	s.syncString(taskName);

#define DEFINE_TASK(TaskName) \
	if (taskName == #TaskName) \
		return constructTask_##TaskName(process, s);
#include "alcachofa/tasks.h"

	error("Invalid task type in savestate: %s", taskName.c_str());
}

void Process::syncGame(Serializer &s) {
	s.syncAsUint32LE(_pid);
	syncEnum(s, _character);
	s.syncString(_name);
	s.syncAsSint32LE(_lastReturnValue);

	uint count = _tasks.size();
	s.syncAsUint32LE(count);
	if (s.isLoading()) {
		assert(_tasks.empty());
		for (uint i = 0; i < count; i++)
			_tasks.push(readTask(*this, s));
	} else {
		String taskName;
		for (uint i = 0; i < count; i++) {
			taskName = _tasks[i]->taskName();
			s.syncString(taskName);
			_tasks[i]->syncGame(s);
		}
	}
}

static void killProcessesForIn(MainCharacterKind characterKind, Array<Process *> &processes, uint firstIndex) {
	assert(firstIndex <= processes.size());
	for (uint i = 0; i < processes.size() - firstIndex; i++) {
		Process **process = &processes[processes.size() - 1 - i];
		if ((*process)->character() == characterKind || characterKind == MainCharacterKind::None) {
			delete *process;
			processes.erase(process);
			i--; // underflow is fine here
		}
	}
}

Scheduler::~Scheduler() {
	killAllProcesses();
	killProcessesForIn(MainCharacterKind::None, _backupProcesses, 0);
}

Process *Scheduler::createProcessInternal(MainCharacterKind character) {
	Process *process = new Process(_nextPid++, character);
	processesToRunNext().push_back(process);
	return process;
}

void Scheduler::run() {
	assert(processesToRun().empty()); // otherwise we somehow left normal flow
	_currentArrayI = (_currentArrayI + 1) % 2;
	// processesToRun() can be modified during loop so do not replace with iterators
	for (_currentProcessI = 0; _currentProcessI < processesToRun().size(); _currentProcessI++) {
		Process *process = processesToRun()[_currentProcessI];
		auto ret = process->run();
		if (ret == TaskReturnType::Finished)
			delete process;
		else
			processesToRunNext().push_back(process);
	}
	processesToRun().clear();
	_currentProcessI = UINT_MAX;
}

void Scheduler::backupContext() {
	assert(processesToRun().empty());
	_backupProcesses.push_back(processesToRunNext());
	processesToRunNext().clear();
}

void Scheduler::restoreContext() {
	assert(processesToRun().empty());
	processesToRunNext().push_back(_backupProcesses);
	_backupProcesses.clear();
}

void Scheduler::killAllProcesses() {
	killProcessesForIn(MainCharacterKind::None, _processArrays[0], 0);
	killProcessesForIn(MainCharacterKind::None, _processArrays[1], 0);
}

void Scheduler::killAllProcessesFor(MainCharacterKind characterKind) {
	// this method can be called during run() so be careful
	killProcessesForIn(characterKind, processesToRunNext(), 0);
	killProcessesForIn(characterKind, processesToRun(), _currentProcessI == UINT_MAX ? 0 : _currentProcessI + 1);
}

static Process **getProcessByName(Array<Process *> &_processes, const String &name) {
	for (auto &process : _processes) {
		if (process->name() == name)
			return &process;
	}
	return nullptr;
}

void Scheduler::killProcessByName(const String &name) {
	Process **process = getProcessByName(processesToRunNext(), name);
	if (process != nullptr) {
		delete *process;
		processesToRunNext().erase(process);
	}
}

bool Scheduler::hasProcessWithName(const String &name) {
	assert(processesToRun().empty());
	return getProcessByName(processesToRunNext(), name) != nullptr;
}

void Scheduler::debugPrint() {
	auto &console = g_engine->console();
	bool didPrintSomething = false;

	if (!processesToRun().empty()) {
		console.debugPrintf("Currently running processes:\n");
		for (uint32 i = 0; i < processesToRun().size(); i++) {
			if (_currentProcessI == UINT_MAX || i > _currentProcessI)
				console.debugPrintf("  ");
			else if (i < _currentProcessI)
				console.debugPrintf("# ");
			else
				console.debugPrintf("> ");
			processesToRun()[i]->debugPrint();
		}
		didPrintSomething = true;
	}

	if (!processesToRunNext().empty()) {
		if (didPrintSomething)
			console.debugPrintf("\n");
		console.debugPrintf("Scheduled processes:\n");
		for (auto *process : processesToRunNext()) {
			console.debugPrintf("  ");
			process->debugPrint();
		}
		didPrintSomething = true;
	}

	if (!_backupProcesses.empty()) {
		if (didPrintSomething)
			console.debugPrintf("\n");
		console.debugPrintf("Backed up processes:\n");
		for (auto *process : _backupProcesses) {
			console.debugPrintf("  ");
			process->debugPrint();
		}
		didPrintSomething = true;
	}

	if (!didPrintSomething)
		console.debugPrintf("No processes running or backed up\n");
}

void Scheduler::prepareSyncGame(Serializer &s) {
	if (s.isLoading()) {
		killAllProcesses();
		killProcessesForIn(MainCharacterKind::None, _backupProcesses, 0);
	}
}

void Scheduler::syncGame(Serializer &s) {
	assert(_currentProcessI == UINT_MAX); // let's not sync during ::run
	assert(s.isSaving() || _backupProcesses.empty());

	Common::Array<Process *> *processes = s.isSaving() && g_engine->menu().isOpen()
		? &_backupProcesses
		: &processesToRunNext();

	// we only sync the backupProcesses as these are the ones pertaining to the gameplay
	// the other arrays would be used for the menu

	s.syncAsUint32LE(_nextPid);
	uint32 count = processes->size();
	s.syncAsUint32LE(count);
	if (s.isLoading()) {
		processes->reserve(count);
		for (uint32 i = 0; i < count; i++)
			processes->push_back(new Process(s));
	} else {
		for (Process *process : *processes)
			process->syncGame(s);
	}
}

}
