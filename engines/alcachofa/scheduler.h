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

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "common.h"

#include "common/stack.h"
#include "common/str.h"

namespace Alcachofa {

struct Task;
class Process;


enum class TaskReturnType {
	Yield,
	Finished,
	Waiting
};

struct TaskReturn {
	static inline TaskReturn yield() { return {}; }
	static TaskReturn finish(int32 returnValue);
	static TaskReturn waitFor(Task *task);

	inline TaskReturnType type() const { return _type; }
	inline int32 returnValue() const {
		assert(_type == TaskReturnType::Finished);
		return _returnValue;
	}
	inline Task *taskToWaitFor() const {
		assert(_type == TaskReturnType::Waiting);
		return _taskToWaitFor;
	}

private:
	TaskReturn();
	TaskReturnType _type;
	union {
		int32 _returnValue;
		Task *_taskToWaitFor;
	};
};

struct Task {
	Task(Process &process);
	virtual ~Task() = default;
	virtual TaskReturn run() = 0;
	virtual void debugPrint() = 0;

	inline Process &process() const { return _process; }

protected:
	Task *delay(uint32 millis);

	uint32 _line = 0;
private:
	Process &_process;
};

// TODO: This probably should be scummvm common
#if __cplusplus >= 201703L
#define TASK_BREAK_FALLTHROUGH [[fallthrough]];
#else
#define TASK_BREAK_FALLTHROUGH
#endif

#define TASK_BEGIN \
	enum { TASK_COUNTER_BASE = __COUNTER__ }; \
	switch(_line) { \
	case 0:; \

#define TASK_END \
	TASK_RETURN(0); \
	TASK_BREAK_FALLTHROUGH \
	default: assert(false && "Invalid line in task"); \
	} return TaskReturn::finish(0)

#define TASK_INTERNAL_BREAK(ret) \
	do { \
		enum { TASK_COUNTER = __COUNTER__ - TASK_COUNTER_BASE }; \
		_line = TASK_COUNTER; \
		return ret; \
		TASK_BREAK_FALLTHROUGH \
		case TASK_COUNTER:; \
	} while(0)

#define TASK_YIELD TASK_INTERNAL_BREAK(TaskReturn::yield())
#define TASK_WAIT(task) TASK_INTERNAL_BREAK(TaskReturn::waitFor(task))

#define TASK_RETURN(value) \
	do { \
		return TaskReturn::finish(value); \
		_line = UINT_MAX; \
	} while(0)

using ProcessId = uint;
class Process {
public:
	Process(ProcessId pid, MainCharacterKind characterKind);
	~Process();

	inline ProcessId pid() const { return _pid; }
	inline MainCharacterKind character() const { return _character; }
	inline int32 returnValue() const { return _lastReturnValue; }
	inline Common::String &name() { return _name; }
	bool isActiveForPlayer() const; ///< and thus should e.g. draw subtitles or effects

	TaskReturnType run();
	void debugPrint();

private:
	friend class Scheduler;
	ProcessId _pid;
	MainCharacterKind _character;
	Common::Stack<Task *> _tasks;
	Common::String _name;
	int32 _lastReturnValue = 0;
};

class Scheduler {
public:
	~Scheduler();

	void run();
	void backupContext();
	void restoreContext();
	void killAllProcesses();
	void killAllProcessesFor(MainCharacterKind characterKind);
	void killProcessByName(const Common::String &name);
	bool hasProcessWithName(const Common::String &name);
	void debugPrint();

	template<typename TTask, typename... TaskArgs>
	Process *createProcess(MainCharacterKind character, TaskArgs&&... args) {
		Process *process = createProcessInternal(character);
		process->_tasks.push(new TTask(*process, Common::forward<TaskArgs>(args)...));
		return process;
	}

private:
	Process *createProcessInternal(MainCharacterKind character);

	inline Common::Array<Process *> &processesToRun() { return _processArrays[_currentArrayI]; }
	inline Common::Array<Process *> &processesToRunNext() { return _processArrays[!_currentArrayI]; }
	Common::Array<Process *> _processArrays[2];
	Common::Array<Process *> _backupProcesses;
	uint8 _currentArrayI = 0;
	ProcessId _nextPid = 1;
	uint _currentProcessI = UINT_MAX;

};

}

#endif // SCHEDULER_H
