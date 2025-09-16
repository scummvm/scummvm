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

#ifndef ALCACHOFA_SCHEDULER_H
#define ALCACHOFA_SCHEDULER_H

#include "alcachofa/common.h"

#include "common/stack.h"
#include "common/str.h"
#include "common/type_traits.h"

namespace Alcachofa {

/* Tasks are generally written as coroutines however the common coroutines
 * cannot be used for two reasons:
 * 1. The scheduler is too limited in managing when to run what coroutines
 *    E.g. for the inventory/menu we need to pause a set of coroutines and
 *    continue them later on
 * 2. We need to save and load the state of coroutines
 *    For this we either write the state machine ourselves or we use
 *    the following careful macros where the state ID is explicitly written
 *    This way it is stable and if it has to change we can migrate
 *    savestates upon loading.
 *
 * Tasks are usually private, so in order to load them they:
 *   - need a constructor MyPrivateTask(Process &, Serializer &)
 *   - need call the macro DECLARE_TASK(MyPrivateTask)
 *   - they have to listed in tasks.h
 */

struct Task;
class Process;
class ObjectBase;

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
	virtual ~Task() {}
	virtual TaskReturn run() = 0;
	virtual void debugPrint() = 0;
	virtual void syncGame(Common::Serializer &s);
	virtual const char *taskName() const = 0; // implemented by DECLARE_TASK

	inline Process &process() const { return _process; }

protected:
	Task *delay(uint32 millis);

	void syncObjectAsString(Common::Serializer &s, ObjectBase *&object, bool optional = false) const;
	template<class TObject>
	void syncObjectAsString(Common::Serializer &s, TObject *&object, bool optional = false) const {
		// We could add is_const and therefore true_type, false_type, integral_constant 
		// or we could just use const_cast and promise that we won't modify the object itself
		ObjectBase *base = const_cast<Common::remove_const_t<TObject> *>(object);
		syncObjectAsString(s, base, optional);
		object = dynamic_cast<TObject *>(base);
		if (object == nullptr && base != nullptr)
			errorForUnexpectedObjectType(base);
	}

	uint32 _stage = 0;
private:
	void errorForUnexpectedObjectType(const ObjectBase *base) const;

	Process &_process;
};

// implemented in alcachofa.cpp to prevent a compiler warning when
// the declaration of the construct function comes after the definition
struct DelayTask final : public Task {
	DelayTask(Process &process, uint32 millis);
	DelayTask(Process &process, Common::Serializer &s);
	TaskReturn run() override;
	void debugPrint() override;
	void syncGame(Common::Serializer &s) override;
	const char *taskName() const override;

private:
	uint32 _endTime = 0;
};

#define DECLARE_TASK(TaskName) \
	extern Task *constructTask_##TaskName(Process &process, Serializer &s) { \
		return new TaskName(process, s); \
	} \
	const char *TaskName::taskName() const { \
		return #TaskName; \
	}

#define TASK_BEGIN \
	switch(_stage) { \
	case 0:; \

#define TASK_END \
	TASK_RETURN(0); \
	default: assert(false && "Invalid line in task"); \
	} return TaskReturn::finish(0)

#define TASK_INTERNAL_BREAK(stage, ret) \
	do { \
		_stage = stage; \
		return ret; \
		case stage:; \
	} while(0)

#define TASK_YIELD(stage) TASK_INTERNAL_BREAK((stage), TaskReturn::yield())
#define TASK_WAIT(stage, task) TASK_INTERNAL_BREAK((stage), TaskReturn::waitFor(task))

#define TASK_RETURN(value) \
	do { \
		_stage = UINT_MAX; \
		return TaskReturn::finish(value); \
	} while(0)

using ProcessId = uint32;
class Process {
public:
	Process(ProcessId pid, MainCharacterKind characterKind);
	Process(Common::Serializer &s);
	~Process();

	inline ProcessId pid() const { return _pid; }
	inline MainCharacterKind &character() { return _character; } // is changed in changeCharacter
	inline MainCharacterKind character() const { return _character; }
	inline int32 returnValue() const { return _lastReturnValue; }
	inline Common::String &name() { return _name; }
	bool isActiveForPlayer() const; ///< and thus should e.g. draw subtitles or effects

	TaskReturnType run();
	void debugPrint();
	void syncGame(Common::Serializer &s);

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
	void prepareSyncGame(Common::Serializer &s);
	void syncGame(Common::Serializer &s);

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

#endif // ALCACHOFA_SCHEDULER_H
