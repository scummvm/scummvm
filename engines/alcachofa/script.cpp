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

#include "script.h"
#include "stream-helper.h"
#include "alcachofa.h"

#include "common/file.h"

using namespace Common;

namespace Alcachofa {

ScriptInstruction::ScriptInstruction(ReadStream &stream)
	: _op((ScriptOp)stream.readSint32LE())
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

int32 Script::variable(const char *name) const {
	uint32 index;
	if (!_variableNames.tryGetVal(name, index))
		error("Unknown variable: %s", name);
	return _variables[index];
}

int32 &Script::variable(const char *name) {
	uint32 index;
	if (!_variableNames.tryGetVal(name, index))
		error("Unknown variable: %s", name);
	return _variables[index];
}

enum class StackEntryType {
	Number,
	Variable,
	String,
	Instruction
};

struct StackEntry {
	StackEntry(StackEntryType type, int32 number) : _type(type), _number(number) {}
	StackEntry(StackEntryType type, uint32 index) : _type(type), _index(index) {}

	StackEntryType _type;
	union {
		int32 _number;
		uint32 _index;
	};
};

struct ScriptTask : public Task {
	ScriptTask(Process &process, const String &name, uint32 pc)
		: Task(process)
		, _script(g_engine->script())
		, _name(name)
		, _pc(pc) {
		pushInstruction(UINT_MAX);
	}

	ScriptTask(Process &process, const ScriptTask &forkParent)
		: Task(process)
		, _script(g_engine->script())
		, _name(forkParent._name + " FORKED")
		, _pc(forkParent._pc) {
		for (uint i = 0; i < forkParent._stack.size(); i++)
			_stack.push(forkParent._stack[i]);
		pushNumber(1); // this task is the forked one
	}

	virtual TaskReturn run() override {
		if (_returnsFromKernelCall)
			pushNumber(process().returnValue());
		_returnsFromKernelCall = false;

		while (true) {
			if (_pc >= _script._instructions.size())
				error("Script process reached instruction out-of-bounds");
			const auto &instruction = _script._instructions[_pc++];
			switch (instruction._op) {
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
				if (instruction._arg < 0 || (uint)instruction._arg > _stack.size())
					error("Script tried to pop more entries than are available on the stack");
				for (int32 i = 0; i < instruction._arg; i++)
					_stack.pop();
				break;
			case ScriptOp::Store: {
				int32 value = popNumber();
				popVariable() = value;
				pushNumber(value);
			}break;
			case ScriptOp::LoadString:
			case ScriptOp::LoadString2:
				pushString(popNumber());
				break;
			case ScriptOp::ScriptCall:
				pushInstruction(_pc);
				_pc = instruction._arg - 1;
				break;
			case ScriptOp::KernelCall: {
				TaskReturn kernelReturn = kernelCall((ScriptKernelTask)instruction._arg);
				if (kernelReturn.type() == TaskReturnType::Waiting) {
					_returnsFromKernelCall = true;
					return kernelReturn;
				}
				else
					pushNumber(kernelReturn.returnValue());
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
			case ScriptOp::Sub:
				pushNumber(popNumber() - popNumber());
				break;
			case ScriptOp::Less:
				pushNumber(popNumber() < popNumber());
				break;
			case ScriptOp::Greater:
				pushNumber(popNumber() > popNumber());
				break;
			case ScriptOp::LessEquals:
				pushNumber(popNumber() <= popNumber());
				break;
			case ScriptOp::GreaterEquals:
				pushNumber(popNumber() >= popNumber());
				break;
			case ScriptOp::Equals:
				pushNumber(popNumber() == popNumber());
				break;
			case ScriptOp::NotEquals:
				pushNumber(popNumber() != popNumber());
				break;
			case ScriptOp::BitAnd:
				pushNumber(popNumber() & popNumber());
				break;
			case ScriptOp::BitOr:
				pushNumber(popNumber() | popNumber());
				break;
			case ScriptOp::Return: {
				int32 returnValue = popNumber();
				_pc = popInstruction();
				if (_pc == UINT_MAX)
					return TaskReturn::finish(returnValue);
				else
					pushNumber(returnValue);
			}break;
			case ScriptOp::Crash5:
			case ScriptOp::Crash8:
			case ScriptOp::Crash9:
			case ScriptOp::Crash12:
			case ScriptOp::Crash21:
			case ScriptOp::Crash22:
			case ScriptOp::Crash33:
			case ScriptOp::Crash34:
			case ScriptOp::Crash35:
			case ScriptOp::Crash36:
				error("Script reached crash instruction");
			default:
				error("Script reached invalid instruction");
			}
		}
	}

	virtual void debugPrint() {
		g_engine->getDebugger()->debugPrintf("\"%s\" at %u\n", _name.c_str(), _pc);
	}

private:
	void pushNumber(int32 value) {
		_stack.push({ StackEntryType::Number, value });
	}

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
		return _script._strings->data() + entry._index;
	}

	TaskReturn kernelCall(ScriptKernelTask task) {
		switch (task) {
		case ScriptKernelTask::PlayVideo:
			warning("STUB KERNEL CALL: PlayVideo");
			return TaskReturn::finish(0);
		case ScriptKernelTask::PlaySound:
			warning("STUB KERNEL CALL: PlaySound");
			return TaskReturn::finish(0);
		case ScriptKernelTask::PlayMusic:
			warning("STUB KERNEL CALL: PlayMusic");
			return TaskReturn::finish(0);
		case ScriptKernelTask::StopMusic:
			warning("STUB KERNEL CALL: StopMusic");
			return TaskReturn::finish(0);
		case ScriptKernelTask::WaitForMusicToEnd:
			warning("STUB KERNEL CALL: WaitForMusicToEnd");
			return TaskReturn::finish(0);
		case ScriptKernelTask::ShowCenterBottomText:
			warning("STUB KERNEL CALL: ShowCenterBottomText");
			return TaskReturn::finish(0);
		case ScriptKernelTask::StopAndTurn:
			warning("STUB KERNEL CALL: StopAndTurn");
			return TaskReturn::finish(0);
		case ScriptKernelTask::StopAndTurnMe:
			warning("STUB KERNEL CALL: StopAndTurnMe");
			return TaskReturn::finish(0);
		case ScriptKernelTask::ChangeCharacter:
			warning("STUB KERNEL CALL: ChangeCharacter");
			return TaskReturn::finish(0);
		case ScriptKernelTask::SayText:
			warning("STUB KERNEL CALL: SayText");
			return TaskReturn::finish(0);
		case ScriptKernelTask::Go:
			warning("STUB KERNEL CALL: Go");
			return TaskReturn::finish(0);
		case ScriptKernelTask::Put:
			warning("STUB KERNEL CALL: Put");
			return TaskReturn::finish(0);
		case ScriptKernelTask::ChangeCharacterRoom:
			warning("STUB KERNEL CALL: ChangeCharacterRoom");
			return TaskReturn::finish(0);
		case ScriptKernelTask::KillProcesses:
			warning("STUB KERNEL CALL: KillProcesses");
			return TaskReturn::finish(0);
		case ScriptKernelTask::LerpLodBias:
			warning("STUB KERNEL CALL: LerpLodBias");
			return TaskReturn::finish(0);
		case ScriptKernelTask::On:
			warning("STUB KERNEL CALL: On");
			return TaskReturn::finish(0);
		case ScriptKernelTask::Off:
			warning("STUB KERNEL CALL: Off");
			return TaskReturn::finish(0);
		case ScriptKernelTask::Pickup:
			warning("STUB KERNEL CALL: Pickup");
			return TaskReturn::finish(0);
		case ScriptKernelTask::CharacterPickup:
			warning("STUB KERNEL CALL: CharacterPickup");
			return TaskReturn::finish(0);
		case ScriptKernelTask::Drop:
			warning("STUB KERNEL CALL: Drop");
			return TaskReturn::finish(0);
		case ScriptKernelTask::CharacterDrop:
			warning("STUB KERNEL CALL: CharacterDrop");
			return TaskReturn::finish(0);
		case ScriptKernelTask::Delay:
			return getNumberArg(0) <= 0
				? TaskReturn::finish(0)
				: TaskReturn::waitFor(delay((uint32)getNumberArg(0)));
		case ScriptKernelTask::HadNoMousePressFor:
			warning("STUB KERNEL CALL: HadNoMousePressFor");
			return TaskReturn::finish(0);
		case ScriptKernelTask::Fork:
			g_engine->scheduler().createProcess<ScriptTask>(process().character(), *this);
			return TaskReturn::finish(0); // 0 means this is the forking process
		case ScriptKernelTask::Animate:
			warning("STUB KERNEL CALL: Animate");
			return TaskReturn::finish(0);
		case ScriptKernelTask::AnimateCharacter:
			warning("STUB KERNEL CALL: AnimateCharacter");
			return TaskReturn::finish(0);
		case ScriptKernelTask::AnimateTalking:
			warning("STUB KERNEL CALL: AnimateTalking");
			return TaskReturn::finish(0);
		case ScriptKernelTask::ChangeRoom:
			warning("STUB KERNEL CALL: ChangeRoom");
			return TaskReturn::finish(0);
		case ScriptKernelTask::ToggleRoomFloor:
			warning("STUB KERNEL CALL: ToggleRoomFloor");
			return TaskReturn::finish(0);
		case ScriptKernelTask::SetDialogLineReturn:
			warning("STUB KERNEL CALL: SetDialogLineReturn");
			return TaskReturn::finish(0);
		case ScriptKernelTask::DialogMenu:
			warning("STUB KERNEL CALL: DialogMenu");
			return TaskReturn::finish(0);
		case ScriptKernelTask::ClearInventory:
			warning("STUB KERNEL CALL: ClearInventory");
			return TaskReturn::finish(0);
		case ScriptKernelTask::FadeType0:
			warning("STUB KERNEL CALL: FadeType0");
			return TaskReturn::finish(0);
		case ScriptKernelTask::FadeType1:
			warning("STUB KERNEL CALL: FadeType1");
			return TaskReturn::finish(0);
		case ScriptKernelTask::SetLodBias:
			warning("STUB KERNEL CALL: SetLodBias");
			return TaskReturn::finish(0);
		case ScriptKernelTask::FadeType2:
			warning("STUB KERNEL CALL: FadeType2");
			return TaskReturn::finish(0);
		case ScriptKernelTask::SetActiveTextureSet:
			warning("STUB KERNEL CALL: SetActiveTextureSet");
			return TaskReturn::finish(0);
		case ScriptKernelTask::SetMaxCamSpeedFactor:
			warning("STUB KERNEL CALL: SetMaxCamSpeedFactor");
			return TaskReturn::finish(0);
		case ScriptKernelTask::WaitCamStopping:
			warning("STUB KERNEL CALL: WaitCamStopping");
			return TaskReturn::finish(0);
		case ScriptKernelTask::CamFollow:
			warning("STUB KERNEL CALL: CamFollow");
			return TaskReturn::finish(0);
		case ScriptKernelTask::CamShake:
			warning("STUB KERNEL CALL: CamShake");
			return TaskReturn::finish(0);
		case ScriptKernelTask::LerpCamXY:
			warning("STUB KERNEL CALL: LerpCamXY");
			return TaskReturn::finish(0);
		case ScriptKernelTask::LerpCamZ:
			warning("STUB KERNEL CALL: LerpCamZ");
			return TaskReturn::finish(0);
		case ScriptKernelTask::LerpCamScale:
			warning("STUB KERNEL CALL: LerpCamScale");
			return TaskReturn::finish(0);
		case ScriptKernelTask::LerpCamToObjectWithScale:
			warning("STUB KERNEL CALL: LerpCamToObjectWithScale");
			return TaskReturn::finish(0);
		case ScriptKernelTask::LerpCamToObjectResettingZ:
			warning("STUB KERNEL CALL: LerpCamToObjectResettingZ");
			return TaskReturn::finish(0);
		case ScriptKernelTask::LerpCamRotation:
			warning("STUB KERNEL CALL: LerpCamRotation");
			return TaskReturn::finish(0);
		case ScriptKernelTask::FadeIn:
			warning("STUB KERNEL CALL: FadeIn");
			return TaskReturn::finish(0);
		case ScriptKernelTask::FadeOut:
			warning("STUB KERNEL CALL: FadeOut");
			return TaskReturn::finish(0);
		case ScriptKernelTask::FadeIn2:
			warning("STUB KERNEL CALL: FadeIn2");
			return TaskReturn::finish(0);
		case ScriptKernelTask::FadeOut2:
			warning("STUB KERNEL CALL: FadeOut2");
			return TaskReturn::finish(0);
		case ScriptKernelTask::LerpCamXYZ:
			warning("STUB KERNEL CALL: LerpCamXYZ");
			return TaskReturn::finish(0);
		case ScriptKernelTask::LerpCamToObjectKeepingZ:
			warning("STUB KERNEL CALL: LerpCamToObjectKeepingZ");
			return TaskReturn::finish(0);
		case ScriptKernelTask::Nop10:
		case ScriptKernelTask::Nop24:
		case ScriptKernelTask::Nop34:
			return TaskReturn::finish(0);
		default:
			error("Invalid kernel call");
			return TaskReturn::finish(0);
		}
	}

	Script &_script;
	Stack<StackEntry> _stack;
	String _name;
	uint32 _pc;
	bool _returnsFromKernelCall = false;
};

Process *Script::createProcess(MainCharacterKind character, const String &behavior, const String &action, bool allowMissing) {
	return createProcess(character, behavior + '/' + action, allowMissing);
}

Process *Script::createProcess(MainCharacterKind character, const String &procedure, bool allowMissing) {
	uint32 offset;
	if (!_procedures.tryGetVal(procedure, offset)) {
		if (allowMissing)
			return nullptr;
		error("Unknown required procedure: %s", procedure.c_str());
	}
	Process *process = g_engine->scheduler().createProcess<ScriptTask>(character, procedure, offset);
	process->name() = procedure;
	return process;
}

}
