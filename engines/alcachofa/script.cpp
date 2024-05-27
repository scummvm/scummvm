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

struct ScriptTask : public Task {
	ScriptTask(Process &process, const String &name, uint32 pc)
		: Task(process)
		, _name(name)
		, _pc(pc) {}

	virtual TaskReturn run() override {
		warning("STUB: Script execution at %u", _pc);
		return TaskReturn::finish(0);
	}

	virtual void debugPrint() {
		g_engine->getDebugger()->debugPrintf("\"%s\" at %u\n", _name.c_str(), _pc);
	}

private:
	enum class StackEntryType {
		Numeric,
		Variable,
		String
	};

	struct StackEntry {
		StackEntryType _type;
		union {
			int32 _numeric;
			int32 *_variable;
			const char *_string;
		};
	};

	Stack<StackEntry> _stack;
	String _name;
	uint32 _pc;
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
