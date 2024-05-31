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

#include "scheduler.h"

#include "common/system.h"
#include "alcachofa.h"

using namespace Common;

namespace Alcachofa {

struct DelayTask : public Task {
	DelayTask(Process &process, uint32 millis)
		: Task(process)
		, _endTime(millis) {}

	virtual TaskReturn run() override {
		TASK_BEGIN;
		_endTime += g_system->getMillis();
		while (g_system->getMillis() < _endTime)
			TASK_YIELD;
		TASK_END;
	}

	virtual void debugPrint() {
		uint32 remaining = g_system->getMillis() <= _endTime ? _endTime - g_system->getMillis() : 0;
		g_engine->getDebugger()->debugPrintf("Delay for further %ums\n", remaining);
	}

private:
	uint32 _endTime;
};

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

Process::Process(ProcessId pid, MainCharacterKind characterKind)
	: _pid(pid)
	, _character(characterKind)
	, _name("Unnamed process") {
}

Process::~Process() {
	while (!_tasks.empty())
		delete _tasks.pop();
}

TaskReturnType Process::run() {
	while (!_tasks.empty()) {
		TaskReturn ret = _tasks.top()->run();
		switch (ret.type()) {
		case TaskReturnType::Yield: return TaskReturnType::Yield;
		case TaskReturnType::Waiting:
			_tasks.push(ret.taskToWaitFor());
			break;
		case TaskReturnType::Finished:
			_lastReturnValue = ret.returnValue();
			_tasks.pop();
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
	case MainCharacterKind::None: characterName = "    <none>"; break;
	case MainCharacterKind::Filemon: characterName =   " Filemon"; break;
	case MainCharacterKind::Mortadelo: characterName = "Mortadelo"; break;
	default: characterName = "<invalid>"; break;
	}
	debugger->debugPrintf("pid: %3u char: %s ret: %2d \"%s\"\n", _pid, characterName, _lastReturnValue, _name.c_str());

	for (uint i = 0; i < _tasks.size(); i++) {
		debugger->debugPrintf("    %u: ", i);
		_tasks[i]->debugPrint();
	}
}

static void killProcessesForIn(MainCharacterKind characterKind, Array<Process *> &processes, uint firstIndex) {
	assert(firstIndex <= processes.size());
	uint count = processes.size() - firstIndex;
	for (uint i = 0; i < count; i++) {
		Process **process = &processes[processes.size() - 1 - i];
		if ((*process)->character() == characterKind || characterKind == MainCharacterKind::None) {
			delete *process;
			processes.erase(process);
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
	killProcessesForIn(characterKind, processesToRun(), _currentProcessI == UINT_MAX ? 0 : _currentProcessI);
}

static Process **getProcessByName(Array<Process *> &_processes, const String &name) {
	for (auto &process : _processes) {
		if (process->name() == name)
			return &process;
	}
	return nullptr;
}

void Scheduler::killProcessByName(const String &name) {
	assert(processesToRun().empty());
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

}
