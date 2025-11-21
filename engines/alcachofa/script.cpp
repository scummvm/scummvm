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

#include "alcachofa/script.h"
#include "alcachofa/rooms.h"
#include "alcachofa/global-ui.h"
#include "alcachofa/alcachofa.h"
#include "alcachofa/script-debug.h"

#include "common/file.h"

using namespace Common;
using namespace Math;

namespace Alcachofa {

enum ScriptDebugLevel {
	SCRIPT_DEBUG_LVL_NONE = 0,
	SCRIPT_DEBUG_LVL_TASKS = 1,
	SCRIPT_DEBUG_LVL_KERNELCALLS = 2,
	SCRIPT_DEBUG_LVL_INSTRUCTIONS = 3
};

ScriptInstruction::ScriptInstruction(ReadStream &stream)
	: _op(stream.readSint32LE())
	, _arg(stream.readSint32LE()) {}

Script::Script() {
	File file;
	if (!file.open("script/SCRIPT.COD"))
		error("Could not open script");

	uint32 stringBlobSize = file.readUint32LE();
	uint32 memorySize = file.readUint32LE();
	_strings = SpanOwner<Span<char>>({ new char[stringBlobSize], stringBlobSize });
	if (file.read(&_strings[0], stringBlobSize) != stringBlobSize)
		error("Could not read script string blob");
	if (_strings[stringBlobSize - 1] != 0)
		error("String blob does not end with null terminator");

	if (memorySize % sizeof(int32) != 0)
		error("Unexpected size of script memory");
	_variables.resize(memorySize / sizeof(int32), 0);

	uint32 variableCount = file.readUint32LE();
	for (uint32 i = 0; i < variableCount; i++) {
		String name = readVarString(file);
		uint32 offset = file.readUint32LE();
		if (offset % sizeof(int32) != 0)
			error("Unaligned variable offset");
		_variableNames[name] = offset / 4;
	}

	uint32 procedureCount = file.readUint32LE();
	for (uint32 i = 0; i < procedureCount; i++) {
		String name = readVarString(file);
		uint32 offset = file.readUint32LE();
		file.skip(sizeof(uint32));
		_procedures[name] = offset - 1; // originally one-based, but let's not.
	}

	uint32 behaviorCount = file.readUint32LE();
	for (uint32 i = 0; i < behaviorCount; i++) {
		String behaviorName = readVarString(file) + '/';
		variableCount = file.readUint32LE(); // not used by the original game
		assert(variableCount == 0);
		procedureCount = file.readUint32LE();
		for (uint32 j = 0; j < procedureCount; j++) {
			String name = behaviorName + readVarString(file);
			uint32 offset = file.readUint32LE();
			file.skip(sizeof(uint32));
			_procedures[name] = offset - 1;
		}
	}

	uint32 instructionCount = file.readUint32LE();
	_instructions.reserve(instructionCount);
	for (uint32 i = 0; i < instructionCount; i++)
		_instructions.push_back(ScriptInstruction(file));
}

static void syncAsSint32LE(Serializer &s, int32 &value) {
	s.syncAsSint32LE(value);
}

void Script::syncGame(Serializer &s) {
	s.syncArray(_variables.data(), _variables.size(), syncAsSint32LE);
}

int32 Script::variable(const char *name) const {
	uint32 index;
	if (_variableNames.tryGetVal(name, index))
		return _variables[index];
	g_engine->game().unknownVariable(name);
	return 0;
}

int32 &Script::variable(const char *name) {
	uint32 index;
	if (_variableNames.tryGetVal(name, index))
		return _variables[index];
	g_engine->game().unknownVariable(name);
	static int32 dummy = 0;
	return dummy;
}

bool Script::hasProcedure(const Common::String &behavior, const Common::String &action) const {
	return hasProcedure(behavior + '/' + action);
}

bool Script::hasProcedure(const Common::String &procedure) const {
	return _procedures.contains(procedure);
}

struct ScriptTimerTask final : public Task {
	ScriptTimerTask(Process &process, int32 durationSec)
		: Task(process)
		, _durationSec(durationSec) {}

	ScriptTimerTask(Process &process, Serializer &s)
		: Task(process) {
		ScriptTimerTask::syncGame(s);
	}

	TaskReturn run() override {
		TASK_BEGIN;
		{
			uint32 timeSinceTimer = g_engine->script()._scriptTimer == 0 ? 0
				: (g_engine->getMillis() - g_engine->script()._scriptTimer) / 1000;
			if (_durationSec >= (int32)timeSinceTimer)
				_result = g_engine->script().variable("SeHaPulsadoRaton") ? 0 : 2;
			else
				_result = 1;
			g_engine->player().drawCursor();
		}
		TASK_YIELD(1); // Wait a frame to not produce an endless loop
		TASK_RETURN(_result); //-V779
		TASK_END;
	}

	void debugPrint() override {
		g_engine->getDebugger()->debugPrintf("Check input timer for %dsecs", _durationSec);
	}

	void syncGame(Serializer &s) override {
		Task::syncGame(s);
		s.syncAsSint32LE(_durationSec);
		s.syncAsSint32LE(_result);
	}

	const char *taskName() const override;

private:
	int32 _durationSec = 0;
	int32 _result = 1;
};
DECLARE_TASK(ScriptTimerTask)

enum class StackEntryType {
	Number,
	Variable,
	String,
	Instruction
};

struct StackEntry {
	StackEntry(StackEntryType type, int32 number) : _type(type), _number(number) {}
	StackEntry(StackEntryType type, uint32 index) : _type(type), _index(index) {}
	StackEntry(Serializer &s) : _type(), _number(0) { syncGame(s); }

	void syncGame(Serializer &s) {
		syncEnum(s, _type);
		if (_type == StackEntryType::Number)
			s.syncAsSint32LE(_number);
		else
			s.syncAsUint32LE(_index);
	}

	StackEntryType _type;
	union {
		int32 _number;
		uint32 _index;
	};
};

struct ScriptTask final : public Task {
	ScriptTask(Process &process, const String &name, uint32 pc, FakeLock &&lock)
		: Task(process)
		, _script(g_engine->script())
		, _name(name)
		, _pc(pc)
		, _lock(Common::move(lock)) {
		pushInstruction(UINT_MAX);
		debugC(SCRIPT_DEBUG_LVL_TASKS, kDebugScript, "%u: Script start at %u", process.pid(), pc);
	}

	ScriptTask(Process &process, const ScriptTask &forkParent)
		: Task(process)
		, _script(g_engine->script())
		, _name(forkParent._name + " FORKED")
		, _pc(forkParent._pc)
		, _lock(forkParent._lock) {
		for (uint i = 0; i < forkParent._stack.size(); i++)
			_stack.push(forkParent._stack[i]);
		pushNumber(1); // this task is the forked one
		debugC(SCRIPT_DEBUG_LVL_TASKS, kDebugScript, "%u: Script fork from %u at %u", process.pid(), forkParent.process().pid(), _pc);
	}

	ScriptTask(Process &process, Serializer &s)
		: Task(process)
		, _script(g_engine->script()) {
		ScriptTask::syncGame(s);
	}

	TaskReturn run() override {
		if (_isFirstExecution || _returnsFromKernelCall)
			setCharacterVariables();
		if (_returnsFromKernelCall) {
			handleReturnFromKernelCall(process().returnValue());
		}
		_isFirstExecution = _returnsFromKernelCall = false;
		auto opMap = g_engine->game().getScriptOpMap();

		while (true) {
			if (_pc >= _script._instructions.size())
				error("Script process reached instruction out-of-bounds");
			const auto &instruction = _script._instructions[_pc++];
			if (debugChannelSet(SCRIPT_DEBUG_LVL_INSTRUCTIONS, kDebugScript)) {
				debugN("%u: %5u %-12s %8d Stack: ",
					process().pid(), _pc - 1, ScriptOpNames[(int)instruction._op], instruction._arg);
				if (_stack.empty())
					debug("empty");
				else {
					const auto &top = _stack.top();
					switch (top._type) {
					case StackEntryType::Number:
						debug("Number %d", top._number);
						break;
					case StackEntryType::Variable:
						debug("Var %u (%d)", top._index, _script._variables[top._index]);
						break;
					case StackEntryType::Instruction:
						debug("Instr %u", top._index);
						break;
					case StackEntryType::String:
						debug("String %u (\"%s\")", top._index, getStringArg(0));
						break;
					default:
						debug("INVALID");
						break;
					}
				}
			}

			if (instruction._op < 0 || (uint32)instruction._op >= opMap.size()) {
				g_engine->game().unknownInstruction(instruction);
				continue;
			}
			switch (opMap[instruction._op]) {
			case ScriptOp::Nop: break;
			case ScriptOp::Dup:
				if (_stack.empty())
					error("Script tried to duplicate stack top, but stack is empty");
				_stack.push(_stack.top());
				break;
			case ScriptOp::PushAddr:
				pushVariable(instruction._arg);
				break;
			case ScriptOp::PushValue:
				pushNumber(instruction._arg);
				break;
			case ScriptOp::Deref:
				pushNumber(popVariable());
				break;
			case ScriptOp::PopN:
				popN(instruction._arg);
				break;
			case ScriptOp::Store: {
				int32 value = popNumber();
				popVariable() = value;
				pushNumber(value);
			}break;
			case ScriptOp::LoadString:
				pushString(popNumber());
				break;
			case ScriptOp::ScriptCall:
				pushInstruction(_pc);
				_pc = instruction._arg - 1;
				break;
			case ScriptOp::KernelCall: {
				TaskReturn kernelReturn = kernelCall(instruction._arg);
				if (kernelReturn.type() == TaskReturnType::Waiting) {
					_returnsFromKernelCall = true;
					return kernelReturn;
				} else
					handleReturnFromKernelCall(kernelReturn.returnValue());
			}break;
			case ScriptOp::JumpIfFalse:
				if (popNumber() == 0)
					_pc = _pc - 1 + instruction._arg;
				break;
			case ScriptOp::JumpIfTrue:
				if (popNumber() != 0)
					_pc = _pc - 1 + instruction._arg;
				break;
			case ScriptOp::Jump:
				_pc = _pc - 1 + instruction._arg;
				break;
			case ScriptOp::Negate:
				pushNumber(-popNumber());
				break;
			case ScriptOp::BooleanNot:
				pushNumber(popNumber() == 0 ? 1 : 0);
				break;
			case ScriptOp::Mul:
				pushNumber(popNumber() * popNumber());
				break;
			case ScriptOp::Add:
				pushNumber(popNumber() + popNumber());
				break;
			// flipped operators to not use a temporary
			case ScriptOp::Sub:
				pushNumber(-popNumber() + popNumber());
				break;
			case ScriptOp::Less:
				pushNumber(popNumber() > popNumber()); //-V501
				break;
			case ScriptOp::Greater:
				pushNumber(popNumber() < popNumber()); //-V501
				break;
			case ScriptOp::LessEquals:
				pushNumber(popNumber() >= popNumber()); //-V501
				break;
			case ScriptOp::GreaterEquals:
				pushNumber(popNumber() <= popNumber()); //-V501
				break;
			case ScriptOp::Equals:
				pushNumber(popNumber() == popNumber()); //-V501
				break;
			case ScriptOp::NotEquals:
				pushNumber(popNumber() != popNumber()); //-V501
				break;
			case ScriptOp::BitAnd:
				pushNumber(popNumber() & popNumber()); //-V501
				break;
			case ScriptOp::BitOr:
				pushNumber(popNumber() | popNumber()); //-V501
				break;
			case ScriptOp::ReturnValue: {
				int32 returnValue = popNumber();
				_pc = popInstruction();
				if (_pc == UINT_MAX)
					return TaskReturn::finish(returnValue);
				else
					pushNumber(returnValue);
			}break;
			default:
				g_engine->game().unknownInstruction(instruction);
				break;
			}
		}
	}

	void debugPrint() override {
		g_engine->getDebugger()->debugPrintf("\"%s\" at %u\n", _name.c_str(), _pc);
	}

	void syncGame(Serializer &s) override {
		assert(s.isSaving() || (_lock.isReleased() && _stack.empty()));

		s.syncString(_name);
		s.syncAsUint32LE(_pc);
		s.syncAsByte(_returnsFromKernelCall);
		s.syncAsByte(_isFirstExecution);

		uint count = _stack.size();
		s.syncAsUint32LE(count);
		if (s.isLoading()) {
			for (uint i = 0; i < count; i++)
				_stack.push(StackEntry(s));
		} else {
			for (uint i = 0; i < count; i++)
				_stack[i].syncGame(s);
		}

		bool hasLock = !_lock.isReleased();
		s.syncAsByte(hasLock);
		if (s.isLoading() && hasLock)
			_lock = FakeLock("script", g_engine->player().semaphoreFor(process().character()));
	}

	const char *taskName() const override;

private:
	void setCharacterVariables() {
		_script.variable("m_o_f") = (int32)process().character();
		_script.variable("m_o_f_real") = (int32)g_engine->player().activeCharacterKind();
	}

	void handleReturnFromKernelCall(int32 returnValue) {
		// this is also original done, every KernelCall is followed by a PopN of the arguments
		// only *after* the PopN the return value is pushed so that the following script can use it
		scumm_assert(
			_pc < _script._instructions.size() &&
			g_engine->game().getScriptOpMap()[_script._instructions[_pc]._op] == ScriptOp::PopN);
		popN(_script._instructions[_pc++]._arg);
		pushNumber(returnValue);
	}

	void pushNumber(int32 value) {
		_stack.push({ StackEntryType::Number, value });
	}

	// For the following methods error recovery is not really viable

	void pushVariable(uint32 offset) {
		uint32 index = offset / sizeof(int32);
		if (offset % sizeof(int32) != 0 || index >= _script._variables.size())
			error("Script tried to push invalid variable offset");
		_stack.push({ StackEntryType::Variable, index });
	}

	void pushString(uint32 offset) {
		if (offset >= _script._strings->size())
			error("Script tried to push invalid string offset");
		_stack.push({ StackEntryType::String, offset });
	}

	void pushInstruction(uint32 pc) {
		_stack.push({ StackEntryType::Instruction, pc });
	}

	StackEntry pop() {
		if (_stack.empty())
			error("Script tried to pop empty stack");
		return _stack.pop();
	}

	int32 popNumber() {
		auto entry = pop();
		if (entry._type != StackEntryType::Number)
			error("Script tried to pop, but top of stack is not a number");
		return entry._number;
	}

	int32 &popVariable() {
		auto entry = pop();
		if (entry._type != StackEntryType::Variable)
			error("Script tried to pop, but top of stack is not a variable");
		return _script._variables[entry._index];
	}

	const char *popString() {
		auto entry = pop();
		if (entry._type != StackEntryType::String)
			error("Script tried to pop, but top of stack is not a string");
		return _script._strings->data() + entry._index;
	}

	uint32 popInstruction() {
		auto entry = pop();
		if (entry._type != StackEntryType::Instruction)
			error("Script tried to pop but top of stack is not an instruction");
		return entry._index;
	}

	void popN(int32 count) {
		if (count < 0 || (uint)count > _stack.size())
			error("Script tried to pop more entries than are available on the stack");
		for (int32 i = 0; i < count; i++)
			_stack.pop();
	}

	StackEntry getArg(uint argI) {
		if (_stack.size() < argI + 1)
			error("Script did not supply enough arguments for kernel call");
		return _stack[_stack.size() - 1 - argI];
	}

	int32 getNumberArg(uint argI) {
		auto entry = getArg(argI);
		if (entry._type != StackEntryType::Number)
			error("Expected number in argument %u for kernel call", argI);
		return entry._number;
	}

	const char *getStringArg(uint argI) {
		auto entry = getArg(argI);
		if (entry._type != StackEntryType::String)
			error("Expected string in argument %u for kernel call", argI);
		return &_script._strings[entry._index];
	}

	int32 getNumberOrStringArg(uint argI) {
		// Original inconsistency: sometimes a string is passed instead of a number
		// as it will be interpreted as a boolean we only care about == 0 / != 0
		auto entry = getArg(argI);
		if (entry._type != StackEntryType::Number && entry._type != StackEntryType::String)
			error("Expected number or string in argument %u for kernel call", argI);
		return entry._number;
	}

	const char *getOptionalStringArg(uint argI) {
		// another special case: a string that may be zero which is passed as number
		auto entry = getArg(argI);
		if (entry._type == StackEntryType::String)
			return &_script._strings[entry._index];
		if (entry._type == StackEntryType::Number && entry._number == 0)
			return nullptr;
		error("Expected optional string in argument %u for kernel call", argI);
	}

	template<class TObject = ObjectBase>
	TObject *getObjectArg(uint argI) {
		const char *const name = getStringArg(argI);
		auto *object = g_engine->world().getObjectByName(process().character(), name);
		return dynamic_cast<TObject *>(object);
	}

	MainCharacter &relatedCharacter() {
		if (process().character() == MainCharacterKind::None)
			error("Script tried to use character from non-character-related process");
		return g_engine->world().getMainCharacterByKind(process().character());
	}

	bool shouldSkipCutscene() {
		return process().character() != MainCharacterKind::None &&
			g_engine->player().activeCharacterKind() != process().character();
	}

	TaskReturn kernelCall(int32 taskI) {
		const auto taskMap = g_engine->game().getScriptKernelTaskMap();
		if (taskI < 0 || (uint32)taskI >= taskMap.size()) {
			g_engine->game().unknownKernelTask(taskI);
			return TaskReturn::finish(-1);
		}
		const auto task = taskMap[taskI];

		debugC(SCRIPT_DEBUG_LVL_KERNELCALLS, kDebugScript, "%u: %5u Kernel %-25s",
			process().pid(), _pc - 1, KernelCallNames[(int)task]);
		switch (task) {
		// sound/video
		case ScriptKernelTask::PlayVideo:
			g_engine->playVideo(getNumberArg(0));
			return TaskReturn::finish(0);
		case ScriptKernelTask::PlaySound: {
			auto soundID = g_engine->sounds().playSFX(getStringArg(0));
			g_engine->sounds().setAppropriateVolume(soundID, process().character(), nullptr);
			return getNumberArg(1) == 0
				? TaskReturn::waitFor(new PlaySoundTask(process(), soundID))
				: TaskReturn::finish(1);
		}
		case ScriptKernelTask::PlayMusic:
			if (process().isActiveForPlayer())
				g_engine->sounds().startMusic((int)getNumberArg(0));
			return TaskReturn::finish(0);
		case ScriptKernelTask::StopMusic:
			if (process().isActiveForPlayer())
				g_engine->sounds().fadeMusic();
			return TaskReturn::finish(0);
		case ScriptKernelTask::WaitForMusicToEnd:
			warning("STUB KERNEL CALL: WaitForMusicToEnd");
			return TaskReturn::finish(0);

		// Misc / control flow
		case ScriptKernelTask::ShowCenterBottomText:
			return TaskReturn::waitFor(showCenterBottomText(process(), getNumberArg(0), (uint32)getNumberArg(1)));
		case ScriptKernelTask::Delay:
			return getNumberArg(0) <= 0
				? TaskReturn::finish(0)
				: TaskReturn::waitFor(delay((uint32)getNumberArg(0)));
		case ScriptKernelTask::HadNoMousePressFor:
			return TaskReturn::waitFor(new ScriptTimerTask(process(), getNumberArg(0)));
		case ScriptKernelTask::Fork:
			g_engine->scheduler().createProcess<ScriptTask>(process().character(), *this);
			return TaskReturn::finish(0); // 0 means this is the forking process
		case ScriptKernelTask::KillProcesses:
			killProcessesFor((MainCharacterKind)getNumberArg(0));
			return TaskReturn::finish(1);

		// player/world state changes
		case ScriptKernelTask::ChangeCharacter: {
			MainCharacterKind kind = (MainCharacterKind)getNumberArg(0);
			killProcessesFor(MainCharacterKind::None); // yes, kill for all characters
			auto &camera = g_engine->camera();
			auto &player = g_engine->player();
			camera.resetRotationAndScale();
			camera.backup(0);
			if (kind != MainCharacterKind::None) {
				player.setActiveCharacter(kind);
				player.heldItem() = nullptr;
				camera.setFollow(player.activeCharacter());
				camera.backup(0);
			}
			process().character() = MainCharacterKind::None;
			assert(player.semaphore().isReleased());
			_lock = FakeLock("script", player.semaphore());
			return TaskReturn::finish(1);
		}
		case ScriptKernelTask::ChangeRoom:
			if (scumm_stricmp(getStringArg(0), "SALIR") == 0) {
				g_engine->quitGame();
				g_engine->player().changeRoom("SALIR", true);
			} else if (scumm_stricmp(getStringArg(0), "MENUPRINCIPALINICIO") == 0)
				warning("STUB: change room to MenuPrincipalInicio special case");
			else {
				auto targetRoom = g_engine->world().getRoomByName(getStringArg(0));
				if (targetRoom == nullptr)
					error("Invalid room name: %s\n", getStringArg(0));
				if (process().isActiveForPlayer()) {
					g_engine->player().heldItem() = nullptr;
					bool isTemporaryRoom = false;
					if (g_engine->player().currentRoom() == &g_engine->world().inventory()) {
						isTemporaryRoom = true; // see changeRoom, this fixes a bug on looking at items in the inventory
						// this is also why we do not exit the inventory room here (like when the user closes the inventory)
						g_engine->world().inventory().close();
					}
					if (targetRoom == &g_engine->world().inventory())
						g_engine->world().inventory().open();
					else
						g_engine->player().changeRoom(targetRoom->name(), true, isTemporaryRoom);
					g_engine->sounds().setMusicToRoom(targetRoom->musicID());
				}
				g_engine->script().createProcess(process().character(), "ENTRAR_" + targetRoom->name(), ScriptFlags::AllowMissing);
			}
			return TaskReturn::finish(1);
		case ScriptKernelTask::ToggleRoomFloor:
			if (process().character() == MainCharacterKind::None) {
				if (g_engine->player().currentRoom() != nullptr)
					g_engine->player().currentRoom()->toggleActiveFloor();
			} else
				g_engine->world().getMainCharacterByKind(process().character()).room()->toggleActiveFloor();
			return TaskReturn::finish(1);

			// object control / animation
		case ScriptKernelTask::On:
			g_engine->world().toggleObject(process().character(), getStringArg(0), true);
			return TaskReturn::finish(0);
		case ScriptKernelTask::Off:
			g_engine->world().toggleObject(process().character(), getStringArg(0), false);
			return TaskReturn::finish(0);
		case ScriptKernelTask::Animate: {
			auto graphicObject = getObjectArg<GraphicObject>(0);
			if (graphicObject == nullptr) {
				g_engine->game().unknownAnimateObject(getStringArg(0));
				return TaskReturn::finish(1);
			}
			if (getNumberOrStringArg(1)) {
				graphicObject->toggle(true);
				graphicObject->graphic()->start(false);
				return TaskReturn::finish(1);
			} else
				return TaskReturn::waitFor(graphicObject->animate(process()));
		}

		// character control / animation
		case ScriptKernelTask::StopAndTurn: {
			auto character = getObjectArg<WalkingCharacter>(0);
			if (character == nullptr)
				g_engine->game().unknownScriptCharacter("stop-and-turn", getStringArg(0));
			else
				character->stopWalking((Direction)getNumberArg(1));
			return TaskReturn::finish(1);
		}
		case ScriptKernelTask::StopAndTurnMe: {
			relatedCharacter().stopWalking((Direction)getNumberArg(0));
			return TaskReturn::finish(1);
		}
		case ScriptKernelTask::Go: {
			auto character = getObjectArg<WalkingCharacter>(0);
			if (character == nullptr) {
				g_engine->game().unknownScriptCharacter("go", getStringArg(0));
				return TaskReturn::finish(1);
			}
			auto target = getObjectArg<PointObject>(1);
			if (target == nullptr)
				target = g_engine->game().unknownGoPutTarget(process(), "go", getStringArg(1));
			if (target == nullptr)
				return TaskReturn::finish(0);
			character->walkTo(target->position());

			if (getNumberArg(2) & 2)
				g_engine->camera().setFollow(nullptr);

			return (getNumberArg(2) & 1)
				? TaskReturn::finish(1)
				: TaskReturn::waitFor(character->waitForArrival(process()));
		}
		case ScriptKernelTask::Put: {
			auto character = getObjectArg<WalkingCharacter>(0);
			if (character == nullptr) {
				g_engine->game().unknownScriptCharacter("put", getStringArg(0));
				return TaskReturn::finish(1);
			}
			auto target = getObjectArg<PointObject>(1);
			if (target == nullptr)
				target = g_engine->game().unknownGoPutTarget(process(), "put", getStringArg(1));
			if (target == nullptr)
				return TaskReturn::finish(0);
			character->setPosition(target->position());
			return TaskReturn::finish(1);
		}
		case ScriptKernelTask::ChangeCharacterRoom: {
			auto *character = getObjectArg<Character>(0);
			if (character == nullptr) {
				g_engine->game().unknownScriptCharacter("change character room", getStringArg(0));
				return TaskReturn::finish(1);
			}
			auto *targetRoom = g_engine->world().getRoomByName(getStringArg(1));
			if (targetRoom == nullptr) {
				g_engine->game().unknownChangeCharacterRoom(getStringArg(1));
				return TaskReturn::finish(1);
			}
			character->resetTalking();
			character->room() = targetRoom;
			return TaskReturn::finish(1);
		}
		case ScriptKernelTask::LerpCharacterLodBias: {
			auto *character = getObjectArg<Character>(0);
			if (character == nullptr) {
				g_engine->game().unknownScriptCharacter("lerp character LOD bias", getStringArg(0));
				return TaskReturn::finish(1);
			}
			float targetLodBias = getNumberArg(1) * 0.01f;
			int32 durationMs = getNumberArg(2);
			if (durationMs <= 0) {
				character->lodBias() = targetLodBias;
				return TaskReturn::finish(1);
			} else
				return TaskReturn::waitFor(character->lerpLodBias(process(), targetLodBias, durationMs));
		}
		case ScriptKernelTask::AnimateCharacter: {
			auto *character = getObjectArg<Character>(0);
			if (character == nullptr) {
				g_engine->game().unknownScriptCharacter("animate character", getStringArg(0));
				return TaskReturn::finish(1);
			}
			auto *animObject = getObjectArg(1);
			if (animObject == nullptr) {
				g_engine->game().unknownAnimateCharacterObject(getStringArg(1));
				return TaskReturn::finish(1);
			}
			return TaskReturn::waitFor(character->animate(process(), animObject));
		}
		case ScriptKernelTask::AnimateTalking: {
			auto *character = getObjectArg<Character>(0);
			if (character == nullptr) {
				g_engine->game().unknownScriptCharacter("talk", getStringArg(0));
				return TaskReturn::finish(1);
			}
			ObjectBase *talkObject = getObjectArg(1);
			if (talkObject == nullptr && *getStringArg(1) != '\0') {
				g_engine->game().unknownAnimateTalkingObject(getStringArg(1));
				return TaskReturn::finish(1);
			}
			character->talkUsing(talkObject);
			return TaskReturn::finish(1);
		}
		case ScriptKernelTask::SayText: {
			const char *characterName = getStringArg(0);
			int32 dialogId = getNumberArg(1);
			if (strncmp(characterName, "MENU_", 5) == 0) {
				relatedCharacter().addDialogLine(dialogId);
				return TaskReturn::finish(1);
			}
			Character *_character = strcmp(characterName, "AMBOS") == 0
				? &relatedCharacter()
				: getObjectArg<Character>(0);
			if (_character == nullptr) {
				g_engine->game().unknownSayTextCharacter(characterName, dialogId);
				return TaskReturn::finish(1);
			}
			return TaskReturn::waitFor(_character->sayText(process(), dialogId));
		};
		case ScriptKernelTask::SetDialogLineReturn:
			relatedCharacter().setLastDialogReturnValue(getNumberArg(0));
			return TaskReturn::finish(0);
		case ScriptKernelTask::DialogMenu:
			return TaskReturn::waitFor(relatedCharacter().dialogMenu(process()));

		// Inventory control
		case ScriptKernelTask::Pickup:
			relatedCharacter().pickup(getStringArg(0), !getNumberArg(1));
			return TaskReturn::finish(1);
		case ScriptKernelTask::CharacterPickup: {
			auto &character = g_engine->world().getMainCharacterByKind((MainCharacterKind)getNumberArg(1));
			character.pickup(getStringArg(0), !getNumberArg(2));
			return TaskReturn::finish(1);
		}
		case ScriptKernelTask::Drop:
			relatedCharacter().drop(getStringArg(0));
			return TaskReturn::finish(1);
		case ScriptKernelTask::CharacterDrop: {
			auto &character = g_engine->world().getMainCharacterByKind((MainCharacterKind)getNumberArg(1));
			character.drop(getOptionalStringArg(0));
			return TaskReturn::finish(1);
		}
		case ScriptKernelTask::ClearInventory:
			switch ((MainCharacterKind)getNumberArg(0)) {
			case MainCharacterKind::Mortadelo:
				g_engine->world().mortadelo().clearInventory();
				break;
			case MainCharacterKind::Filemon:
				g_engine->world().filemon().clearInventory();
				break;
			default:
				g_engine->game().unknownClearInventoryTarget(getNumberArg(0));
				break;
			}
			return TaskReturn::finish(1);

		// Camera tasks
		case ScriptKernelTask::WaitCamStopping:
			return TaskReturn::waitFor(g_engine->camera().waitToStop(process()));
		case ScriptKernelTask::CamFollow: {
			WalkingCharacter *target = nullptr;
			if (getNumberArg(0) != 0)
				target = &g_engine->world().getMainCharacterByKind((MainCharacterKind)getNumberArg(0));
			g_engine->camera().setFollow(target, getNumberArg(1) != 0);
			return TaskReturn::finish(1);
		}
		case ScriptKernelTask::CamShake:
			return TaskReturn::waitFor(g_engine->camera().shake(process(),
				Vector2d(getNumberArg(1), getNumberArg(2)),
				Vector2d(getNumberArg(3), getNumberArg(4)),
				getNumberArg(0)));
		case ScriptKernelTask::LerpCamXY:
			return TaskReturn::waitFor(g_engine->camera().lerpPos(process(),
				Vector2d(getNumberArg(0), getNumberArg(1)),
				getNumberArg(2), (EasingType)getNumberArg(3)));
		case ScriptKernelTask::LerpCamXYZ:
			return TaskReturn::waitFor(g_engine->camera().lerpPos(process(),
				Vector3d(getNumberArg(0), getNumberArg(1), getNumberArg(2)),
				getNumberArg(3), (EasingType)getNumberArg(4)));
		case ScriptKernelTask::LerpCamZ:
			return TaskReturn::waitFor(g_engine->camera().lerpPosZ(process(),
				getNumberArg(0),
				getNumberArg(1), (EasingType)getNumberArg(2)));
		case ScriptKernelTask::LerpCamScale:
			return TaskReturn::waitFor(g_engine->camera().lerpScale(process(),
				getNumberArg(0) * 0.01f,
				getNumberArg(1), (EasingType)getNumberArg(2)));
		case ScriptKernelTask::LerpCamRotation:
			return TaskReturn::waitFor(g_engine->camera().lerpRotation(process(),
				getNumberArg(0),
				getNumberArg(1), (EasingType)getNumberArg(2)));
		case ScriptKernelTask::LerpCamToObjectKeepingZ: {
			if (!process().isActiveForPlayer())
				return TaskReturn::finish(0); // contrary to ...ResettingZ this one does not delay if not active
			auto pointObject = getObjectArg<PointObject>(0);
			if (pointObject == nullptr) {
				g_engine->game().unknownCamLerpTarget("LerpCamToObjectKeepingZ", getStringArg(0));
				return TaskReturn::finish(1);
			}
			return TaskReturn::waitFor(g_engine->camera().lerpPos(process(),
				as2D(pointObject->position()),
				getNumberArg(1), EasingType::Linear));
		}
		case ScriptKernelTask::LerpCamToObjectResettingZ: {
			if (!process().isActiveForPlayer())
				return TaskReturn::waitFor(delay(getNumberArg(1)));
			auto pointObject = getObjectArg<PointObject>(0);
			if (pointObject == nullptr) {
				g_engine->game().unknownCamLerpTarget("LerpCamToObjectResettingZ", getStringArg(0));
				return TaskReturn::finish(1);
			}
			return TaskReturn::waitFor(g_engine->camera().lerpPos(process(),
				as3D(pointObject->position()),
				getNumberArg(1), (EasingType)getNumberArg(2)));
		}
		case ScriptKernelTask::LerpCamToObjectWithScale: {
			float targetScale = getNumberArg(1) * 0.01f;
			if (!process().isActiveForPlayer())
				// the scale will wait then snap the scale
				return TaskReturn::waitFor(g_engine->camera().lerpScale(process(), targetScale, getNumberArg(2), EasingType::Linear));
			auto pointObject = getObjectArg<PointObject>(0);
			if (pointObject == nullptr) {
				g_engine->game().unknownCamLerpTarget("LerpCamToObjectWithScale", getStringArg(0));
				return TaskReturn::finish(1);
			}
			return TaskReturn::waitFor(g_engine->camera().lerpPosScale(process(),
				as3D(pointObject->position()), targetScale,
				getNumberArg(2), (EasingType)getNumberArg(3), (EasingType)getNumberArg(4)));
		}

		// Fades
		case ScriptKernelTask::FadeType0:
			return TaskReturn::waitFor(fade(process(), FadeType::ToBlack,
				getNumberArg(0) * 0.01f, getNumberArg(1) * 0.01f,
				getNumberArg(2), (EasingType)getNumberArg(4), getNumberArg(3)));
		case ScriptKernelTask::FadeType1:
			return TaskReturn::waitFor(fade(process(), FadeType::ToWhite,
				getNumberArg(0) * 0.01f, getNumberArg(1) * 0.01f,
				getNumberArg(2), (EasingType)getNumberArg(4), getNumberArg(3)));
		case ScriptKernelTask::FadeIn:
			return TaskReturn::waitFor(fade(process(), FadeType::ToBlack,
				1.0f, 0.0f, getNumberArg(0), EasingType::Out, -5,
				PermanentFadeAction::UnsetFaded));
		case ScriptKernelTask::FadeOut:
			return TaskReturn::waitFor(fade(process(), FadeType::ToBlack,
				0.0f, 1.0f, getNumberArg(0), EasingType::Out, -5,
				PermanentFadeAction::SetFaded));
		case ScriptKernelTask::FadeIn2:
			return TaskReturn::waitFor(fade(process(), FadeType::ToBlack,
				0.0f, 1.0f, getNumberArg(0), (EasingType)getNumberArg(1), -5,
				PermanentFadeAction::UnsetFaded));
		case ScriptKernelTask::FadeOut2:
			return TaskReturn::waitFor(fade(process(), FadeType::ToBlack,
				1.0f, 0.0f, getNumberArg(0), (EasingType)getNumberArg(1), -5,
				PermanentFadeAction::SetFaded));

		// Unused and/or useless
		case ScriptKernelTask::SetMaxCamSpeedFactor:
			warning("STUB KERNEL CALL: SetMaxCamSpeedFactor");
			return TaskReturn::finish(0);
		case ScriptKernelTask::LerpWorldLodBias:
			warning("STUB KERNEL CALL: LerpWorldLodBias");
			return TaskReturn::finish(0);
		case ScriptKernelTask::SetActiveTextureSet:
			// Fortunately this seems to be unused.
			warning("STUB KERNEL CALL: SetActiveTextureSet");
			return TaskReturn::finish(0);
		case ScriptKernelTask::FadeType2:
			warning("STUB KERNEL CALL: FadeType2"); // Crossfade, unused from script
			return TaskReturn::finish(0);
		case ScriptKernelTask::Nop:
			return TaskReturn::finish(0);
		default:
			g_engine->game().unknownKernelTask(taskI);
			return TaskReturn::finish(0);
		}
	}

	void killProcessesFor(MainCharacterKind kind) {
		if (kind == MainCharacterKind::None) {
			killProcessesFor(MainCharacterKind::Mortadelo);
			killProcessesFor(MainCharacterKind::Filemon);
			g_engine->scheduler().killAllProcessesFor(kind);
			return;
		}
		g_engine->scheduler().killAllProcessesFor(kind);
		g_engine->sounds().fadeOutVoiceAndSFX(200);
		g_engine->player().stopLastDialogCharacters();
		_lock.release(); // yes this seems dangerous, but it is original..
		auto &character = g_engine->world().getMainCharacterByKind(kind);
		character.resetUsingObjectAndDialogMenu();
		assert(character.semaphore().isReleased()); // this process should be the last to hold a lock if at all...
	}

	Script &_script;
	Stack<StackEntry> _stack;
	String _name;
	uint32 _pc = 0;
	bool _returnsFromKernelCall = false;
	bool _isFirstExecution = true;
	FakeLock _lock;
};
DECLARE_TASK(ScriptTask)

Process *Script::createProcess(MainCharacterKind character, const String &behavior, const String &action, ScriptFlags flags) {
	return createProcess(character, behavior + '/' + action, flags);
}

Process *Script::createProcess(MainCharacterKind character, const String &procedure, ScriptFlags flags) {
	uint32 offset;
	if (!_procedures.tryGetVal(procedure, offset)) {
		if (flags & ScriptFlags::AllowMissing)
			return nullptr;
		// it is currently unnecessary but we could return an empty process to avoid returning nullptr here
		g_engine->game().unknownScriptProcedure(procedure);
		return nullptr;
	}
	FakeLock lock;
	if (!(flags & ScriptFlags::IsBackground))
		lock = FakeLock("script", g_engine->player().semaphoreFor(character));
	Process *process = g_engine->scheduler().createProcess<ScriptTask>(character, procedure, offset, Common::move(lock));
	process->name() = procedure;
	return process;
}

void Script::setScriptTimer(bool reset) {
	// Used for the V3 exclusive kernel task HadNoMousePressFor
	if (reset)
		_scriptTimer = 0;
	else if (_scriptTimer == 0)
		_scriptTimer = g_engine->getMillis();
}

}
