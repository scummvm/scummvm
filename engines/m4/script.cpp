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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "common/textconsole.h"

#include "m4/m4.h"
#include "m4/script.h"
#include "m4/resource.h"

namespace M4 {

enum OpcodeType {
	opRet = 0,
	opCall,
	opCallKernel,
	opPush,
	opPush0,
	opPush1,
	opPushNeg1,
	opPop,
	opMov,
	opAdd,
	opSub,
	opInc,
	opDec,
	opCmp,
	opJmp,
	opJmpByTable,
	opJz,
	opJnz,
	opJe,
	opJne,
	opJl,
	opJle,
	opJg,
	opJge,
	opXor,
	opShl,
	opShr,

	opDebug,

	opInvalid
};

const char *opcodeNames[] = {
	"opRet",
	"opCall",
	"opCallKernel",
	"opPush",
	"opPush0",
	"opPush1",
	"opPushNeg1",
	"opPop",
	"opMov",
	"opAdd",
	"opSub",
	"opInc",
	"opDec",
	"opCmp",
	"opJmp",
	"opJmpByTable",
	"opJz",
	"opJnz",
	"opJe",
	"opJne",
	"opJl",
	"opJle",
	"opJg",
	"opJge",
	"opXor",
	"opShl",
	"opShr",
	"opDebug",
	"opInvalid"
};

StringTable::StringTable() : _stringsData(NULL) {
}

StringTable::~StringTable() {
	delete[] _stringsData;
}

void StringTable::load(Common::File *fd) {
	int stringSize = fd->readUint32LE();
	int stringCount = fd->readUint32LE();
	_stringsData = new char[stringSize];
	fd->read(_stringsData, stringSize);
	char *stringPtr = _stringsData;
	for (int i = 0; i < stringCount; i++) {
		_strings.push_back((const char*)stringPtr);
		stringPtr += strlen(stringPtr) + 1;
	}
}

SeriesStreamBreakList::~SeriesStreamBreakList() {
}

void SeriesStreamBreakList::load(Common::File *fd) {
	uint32 count = fd->readUint32LE();
	debugCN(kDebugScript, "SeriesStreamBreakList::load() count = %d\n", count);
	for (uint32 i = 0; i < count; i++) {
		SeriesStreamBreakItem *item = new SeriesStreamBreakItem();
		item->frameNum = fd->readUint32LE();
		item->digiName = _inter->loadGlobalString(fd);
		item->digiChannel = fd->readUint32LE();
		item->digiVolume = fd->readUint32LE();
		item->trigger = fd->readUint32LE();
		item->flags = fd->readUint32LE();
		item->variable.type = kGameVar;
		item->variable.value = fd->readUint32LE();
		item->value = fd->readUint32LE();
		_items.push_back(item);

		debugCN(kDebugScript, "%02d: frameNum = %d; digiName = %s; digiChannel = %d; digiVolume = %d; trigger = %d; flags = %d; variable = %d; value = %d\n",
			i, item->frameNum, item->digiName,	item->digiChannel, item->digiVolume, item->trigger, item->flags, item->variable.value, item->value);

	}
}

SaidArray::~SaidArray() {
}

void SaidArray::load(Common::File *fd) {
	uint32 count = fd->readUint32LE();
	debugCN(kDebugScript, "SaidArray::load() count = %d\n", count);
	for (uint32 i = 0; i < count; i++) {
		SaidArrayItem *item = new SaidArrayItem();
		item->itemName = _inter->loadGlobalString(fd);
		item->digiNameLook = _inter->loadGlobalString(fd);
		item->digiNameTake = _inter->loadGlobalString(fd);
		item->digiNameGear = _inter->loadGlobalString(fd);
		_items.push_back(item);

		debugCN(kDebugScript, "itemName = %s; digiNameLook = %s; digiNameTake = %s; digiNameGear = %s\n",
			item->itemName, item->digiNameLook, item->digiNameTake, item->digiNameGear);

	}
}

ParserArray::~ParserArray() {
}

void ParserArray::load(Common::File *fd) {
	uint32 count = fd->readUint32LE();
	debugCN(kDebugScript, "ParserArray::load() count = %d\n", count);
	for (uint32 i = 0; i < count; i++) {
		ParserArrayItem *item = new ParserArrayItem();
		item->w0 = _inter->loadGlobalString(fd);
		item->w1 = _inter->loadGlobalString(fd);
		item->trigger = fd->readUint32LE();
		item->testVariable.type = kGameVar;
		item->testVariable.value = fd->readUint32LE();
		item->testValue = fd->readUint32LE();
		item->variable.type = kGameVar;
		item->variable.value = fd->readUint32LE();
		item->value = fd->readUint32LE();
		_items.push_back(item);

		debugCN(kDebugScript, "w0 = %s; w1 = %s; trigger = %d; testVariable = %d; testValue = %d; variable = %d; value = %d\n",
			item->w0, item->w1, item->trigger, item->testVariable.value, item->testValue, item->variable.value, item->value);

	}
}

ScriptFunction::ScriptFunction(ScriptInterpreter *inter) : _inter(inter) {
}

ScriptFunction::~ScriptFunction() {
	delete _code;
}

void ScriptFunction::load(Common::File *fd) {
	debugCN(kDebugScript, "ScriptFunction::load()\n");
	uint32 size = fd->readUint32LE();
	debugCN(kDebugScript, "ScriptFunction::load() size = %d\n", size);
	_code = fd->readStream(size);
}

void ScriptFunction::jumpAbsolute(uint32 ofs) {
	_code->seek(ofs);
}

void ScriptFunction::jumpRelative(int32 ofs) {
	_code->seek(ofs, SEEK_CUR);
}

byte ScriptFunction::readByte() {
	return _code->readByte();
}

uint32 ScriptFunction::readUint32() {
	return _code->readUint32LE();
}


ScriptInterpreter::ScriptInterpreter(MadsM4Engine *vm) : _scriptFile(NULL), _vm(vm) {
	initScriptKernel();
	_dataCache = new ScriptDataCache(this);
	_runningFunction = NULL;
}

ScriptInterpreter::~ScriptInterpreter() {
	close();
	delete _dataCache;
}

void ScriptInterpreter::open(const char *filename) {
	if (_scriptFile)
		close();
	_scriptFile = new Common::File();
	_scriptFile->open(filename);
	if (!_scriptFile->isOpen())
		error("ScriptInterpreter::open() Error opening %s", filename);

	_scriptFile->readUint32LE(); // skip magic for now
	uint32 version = _scriptFile->readUint32LE();
	if (version != kScriptFileVersion) {
		error("ScriptInterpreter::open() DAT file version mismatch; requested %li, got %i", kScriptFileVersion, version);
	}

	int functionCount = _scriptFile->readUint32LE();
	debugCN(kDebugScript, "functionCount = %d\n", functionCount);
	for (int i = 0; i < functionCount; i++) {
		uint32 offset = _scriptFile->readUint32LE();
		debugCN(kDebugScript, "func(%d) offset = %08X\n", i, offset);
		uint32 len = _scriptFile->readUint32LE();
		if (len > 0) {
			char *funcName = new char[len + 1];
			_scriptFile->read(funcName, len);
			funcName[len] = '\0';
			debugCN(kDebugScript, "func(%d) name = %s\n", i, funcName);
			_functionNames[Common::String(funcName)] = _functions.size();
			// DEBUG
			_scriptFunctionNames.push_back(Common::String(funcName));
			delete[] funcName;
		}
		_functions.push_back(new ScriptFunctionEntry(offset));
	}

	int dataCount = _scriptFile->readUint32LE();
	debugCN(kDebugScript, "dataCount = %d\n", dataCount);
	for (int i = 0; i < dataCount; i++) {
		uint32 offset = _scriptFile->readUint32LE();
		ScriptDataType type = (ScriptDataType)_scriptFile->readUint32LE();
		debugCN(kDebugScript, "data(%d) offset = %08X; type = %d\n", i, offset, type);
		_data.push_back(new ScriptDataEntry(offset, type));
	}

	_globalVarCount = _scriptFile->readUint32LE();
	debugCN(kDebugScript, "_globalVarCount = %d\n", _globalVarCount);

	uint32 stringOfs = _scriptFile->readUint32LE();
	_scriptFile->seek(stringOfs);
	_constStrings.load(_scriptFile);

	for (int i = 0; i < ARRAYSIZE(_globalVars); i++) {
		_globalVars[i].type = kInteger;
		_globalVars[i].value = 0;
	}

	memset(_logicGlobals, 0, sizeof(_logicGlobals));

	memset(_registers, 0, sizeof(_registers));
	memset(_stack, 0, sizeof(_stack));
	_stackPtr = 0;

}

void ScriptInterpreter::close() {
	delete _scriptFile;
}

void ScriptInterpreter::initScriptKernel() {

#include "m4/scripttab.h"

	_kernelFunctions = kernelFunctions;
	_kernelFunctionsMax = ARRAYSIZE(kernelFunctions) + 1;

	_kernelVars = kernelVars;
	_kernelVarsMax = ARRAYSIZE(kernelVars) + 1;

}


ScriptFunction *ScriptInterpreter::loadFunction(uint32 index) {
	//GONE WHILE DEBUGGING assert(index < _functions.size());
	if (index >= _functions.size()) return NULL;
	ScriptFunction *scriptFunction;
	scriptFunction = _functions[index]->func;
	if (!scriptFunction) {
		scriptFunction = new ScriptFunction(this);
		_scriptFile->seek(_functions[index]->offset);
		scriptFunction->load(_scriptFile);
		_functions[index]->func = scriptFunction;
	}
	return scriptFunction;
}

ScriptFunction *ScriptInterpreter::loadFunction(const Common::String &name) {
	FunctionNameMap::iterator iter = _functionNames.find(name);
	if (iter == _functionNames.end()) {
		debugCN(kDebugScript, "ScriptInterpreter::loadFunction() Function '%s' not found!\n", name.c_str());
		return NULL;
	}
	uint32 funcIndex = (*iter)._value;
	debugCN(kDebugScript, "ScriptInterpreter::loadFunction() index('%s') = %d\n", name.c_str(), funcIndex);
	return loadFunction(funcIndex);
}

void ScriptInterpreter::unloadFunctions() {
	for (uint32 i = 0; i < _functions.size(); i++) {
		if (_functions[i]->func) {
			delete _functions[i]->func;
			_functions[i]->func = NULL;
		}
	}
}

int ScriptInterpreter::runFunction(ScriptFunction *scriptFunction) {
	bool done = false;

	int oldLocalStackPtr = _localStackPtr;
	ScriptFunction *oldRunningFunction = _runningFunction;

	// TODO: Also initialize _localStackPtr

	_runningFunction = scriptFunction;
	_runningFunction->jumpAbsolute(0);
	while (!done) {
		byte opcode = _runningFunction->readByte();
		done = !execOpcode(opcode);
	}

	_localStackPtr = oldLocalStackPtr;
	_runningFunction = oldRunningFunction;

	return 0;
}

void ScriptInterpreter::push(const ScriptValue &value) {
	if (_stackPtr == ARRAYSIZE(_stack))
		error("ScriptInterpreter::push() Stack overflow");
	_stack[_stackPtr++] = value;
}

void ScriptInterpreter::pop(ScriptValue &value) {
	if (_stackPtr == 0)
		error("ScriptInterpreter::pop() Stack underflow");
	value = _stack[_stackPtr--];
}

void ScriptInterpreter::dumpStack() {
	debugCN(kDebugScript, "ScriptInterpreter::dumpStack()\n");
	for (int i = 0; i < _stackPtr; i++) {
		debugCN(kDebugScript, "%03d. type = %02d; value = %d\n", i, _stack[i].type, _stack[i].value);
	}
}

void ScriptInterpreter::dumpRegisters() {
	debugCN(kDebugScript, "ScriptInterpreter::dumpRegisters()\n");
	for (int i = 0; i < ARRAYSIZE(_registers); i++) {
		debugCN(kDebugScript, "%03d. type = %02d; value = %d\n", i, _registers[i].type, _registers[i].value);
	}
}

void ScriptInterpreter::dumpGlobalVars() {
	debugCN(kDebugScript, "ScriptInterpreter::dumpGlobalVars()\n");
	for (int i = 0; i < ARRAYSIZE(_globalVars); i++) {
		if (_globalVars[i].type != -1)
			debugCN(kDebugScript, "%03d. type = %02d; value = %d\n", i, _globalVars[i].type, _globalVars[i].value);
	}
}

int ScriptInterpreter::toInteger(const ScriptValue &value) {

	switch (value.type) {

	case kInteger:
		return value.value;

	default:
		debugCN(kDebugScript, "ScriptInterpreter::toInteger() Invalid type %d!\n", value.type);
		return 0;

	}

}

const char *ScriptInterpreter::toString(const ScriptValue &value) {

	switch (value.type) {

	case kInteger:
		return NULL;

	case kConstString:
		return _constStrings[value.value];

	default:
		debugCN(kDebugScript, "ScriptInterpreter::toString() Invalid type %d!\n", value.type);
		return NULL;

	}

}

const char *ScriptInterpreter::loadGlobalString(Common::File *fd) {
	uint32 index = fd->readUint32LE();
	if (index != 0xFFFFFFFF)
		return getGlobalString(index);
	else
		return NULL;
}

void ScriptInterpreter::test() {
}

void ScriptInterpreter::loadValue(ScriptValue &value) {

	value.type = (ScriptValueType)_runningFunction->readByte();

	switch (value.type) {

	case kGameVar:
	case kInteger:
	case kConstString:
	case kDataRef:
	case kLogicVar:
	case kLogicVarRef:
	case kKernelVar:
		value.value = _runningFunction->readUint32();
		break;

	case kRegister:
		value.value = _runningFunction->readByte();
		break;

	default:
		debugCN(kDebugScript, "ScriptInterpreter::loadValue() Invalid value type %d!\n", value.type);

	}

}

void ScriptInterpreter::copyValue(ScriptValue &destValue, ScriptValue &sourceValue) {

	if (sourceValue.type == -1) {
		debugCN(kDebugScript, "ScriptInterpreter::copyValue() Trying to read uninitialized value!\n");
	}

	switch (destValue.type) {

	case kGameVar:
		_globalVars[destValue.value] = sourceValue;
		break;

	case kRegister:
		_registers[destValue.value] = sourceValue;
		break;

	case kLogicVar:
		// TODO: Move to own method
		if (sourceValue.type == kInteger) {
			_logicGlobals[destValue.value] = sourceValue.value;
		} else {
			debugCN(kDebugScript, "ScriptInterpreter::copyValue() Invalid source value type %d!\n", sourceValue.type);
		}
		break;

	case kKernelVar:
		setKernelVar(destValue.value, sourceValue);
		break;

	default:
		debugCN(kDebugScript, "ScriptInterpreter::copyValue() Invalid dest value type %d!\n", destValue.type);

	}

}

void ScriptInterpreter::derefValue(ScriptValue &value) {

	switch (value.type) {

	case kGameVar:
		value = _globalVars[value.value];
		break;

	case kInteger:
	case kConstString:
	case kDataRef:
	case kLogicVarRef:
		// These need no dereferencing
		break;

	case kRegister:
		value = _registers[value.value];
		break;

	case kLogicVar:
		// TODO: Move to own method
		value = _logicGlobals[value.value];
		break;

	case kKernelVar:
		getKernelVar(value.value, value);
		break;

	default:
		debugCN(kDebugScript, "ScriptInterpreter::derefValue() Invalid value type %d!\n", value.type);

	}

}

void ScriptInterpreter::callKernelFunction(uint32 index) {

	debugCN(kDebugScript, "ScriptInterpreter::callKernelFunction() index = %d\n", index);

	if (index > _kernelFunctionsMax) {
		debugCN(kDebugScript, "ScriptInterpreter::callKernelFunction() Invalid kernel functionindex (%d)\n", index);
		return;
	}

	debugCN(kDebugScript, "ScriptInterpreter::callKernelFunction() name = %s\n", _kernelFunctions[index].desc);

	int args = (this->*(_kernelFunctions[index].proc))();
	// Now remove values from the stack if the function used any
	if (args > 4)
		_stackPtr -= args - 4;

	debugCN(kDebugScript, "-------------\n");

}

ScriptValue ScriptInterpreter::getArg(uint32 index) {
	if (index < 4) {
		return _registers[index];
	} else {
		index -= 4;
		return _stack[_stackPtr - index - 1];
	}
}

void ScriptInterpreter::dumpArgs(uint32 count) {
	debugCN(kDebugScript, "ScriptInterpreter::dumpArgs() ");
	for (uint32 i = 0; i < count; i++) {
		ScriptValue argValue = getArg(i);
		if (argValue.type == kConstString) {
			debugCN(kDebugScript, "'%s'", toString(argValue));
		} else {
			debugCN(kDebugScript, "%d", argValue.value);
		}
		if (i + 1 < count)
			debugCN(kDebugScript, ", ");
	}
	debugCN(kDebugScript, "\n");
}

void ScriptInterpreter::callFunction(uint32 index) {
	// NOTE: This is a temporary hack for script functions not yet in the m4.dat
	if (index == 0xFFFFFFFF)
		return;
	debugCN(kDebugScript, "ScriptInterpreter::callFunction() index = %d [%s]\n", index, _scriptFunctionNames[index].c_str());
	ScriptFunction *subFunction = loadFunction(index);
	if (!subFunction) {
		// This *should* never happen since the linker checks this
		debugCN(kDebugScript, "ScriptInterpreter::callFunction() Function %d could not be loaded!\n", index);
		return;
	}
	runFunction(subFunction);
}

bool ScriptInterpreter::execOpcode(byte opcode) {

	debugCN(kDebugScript, "opcode = %d (%s)\n", opcode, opcodeNames[opcode]);

	ScriptValue value1, value2, value3;
	uint32 temp;

	/* TODO: Put all opcodes into separate functions and into an array
			 (but only after all needed opcodes are known and frozen)
	*/

	switch (opcode) {

	case opRet:
		return false;

	case opPush:
		loadValue(value1);
		derefValue(value1);
		push(value1);
		return true;

	case opPush0:
		push(ScriptValue(0));
		return true;

	case opPush1:
		push(ScriptValue(1));
		return true;

	case opPushNeg1:
		push(ScriptValue(-1));
		return true;

	case opPop:
		loadValue(value1);
		pop(value2);
		copyValue(value1, value2);
		return true;

	case opMov:
		loadValue(value1);
		loadValue(value2);
		derefValue(value2);
		copyValue(value1, value2);
		return true;

	// Possibly join all jump variants into one opcode

	case opJmp:
		temp = _runningFunction->readUint32();
		debugCN(kDebugScript, "-> ofs = %08X\n", temp);
		_runningFunction->jumpAbsolute(temp);
		return true;

	case opJl:
		temp = _runningFunction->readUint32();
		if (_cmpFlags < 0) {
			debugCN(kDebugScript, "-> ofs = %08X\n", temp);
			_runningFunction->jumpAbsolute(temp);
		}
		return true;

	case opJle:
		temp = _runningFunction->readUint32();
		if (_cmpFlags <= 0) {
			debugCN(kDebugScript, "-> ofs = %08X\n", temp);
			_runningFunction->jumpAbsolute(temp);
		}
		return true;

	case opJg:
		temp = _runningFunction->readUint32();
		if (_cmpFlags > 0) {
			debugCN(kDebugScript, "-> ofs = %08X\n", temp);
			_runningFunction->jumpAbsolute(temp);
		}
		return true;

	case opJge:
		temp = _runningFunction->readUint32();
		if (_cmpFlags >= 0) {
			debugCN(kDebugScript, "-> ofs = %08X\n", temp);
			_runningFunction->jumpAbsolute(temp);
		}
		return true;

	case opJz:
		temp = _runningFunction->readUint32();
		if (_cmpFlags == 0) {
			debugCN(kDebugScript, "-> ofs = %08X\n", temp);
			_runningFunction->jumpAbsolute(temp);
		}
		return true;

	case opJnz:
		temp = _runningFunction->readUint32();
		if (_cmpFlags != 0) {
			debugCN(kDebugScript, "-> ofs = %08X\n", temp);
			_runningFunction->jumpAbsolute(temp);
		}
		return true;

	case opJmpByTable:
		temp = _runningFunction->readUint32();
		debugCN(kDebugScript, "-> index = %d\n", _registers[0].value);
		_runningFunction->jumpRelative(_registers[0].value * 4);
		temp = _runningFunction->readUint32();
		debugCN(kDebugScript, "-> ofs = %08X\n", temp);
		_runningFunction->jumpAbsolute(temp);
		return true;

	case opCmp:
		loadValue(value1);
		loadValue(value2);
		derefValue(value1);
		derefValue(value2);
		if (value1.type != kInteger || value2.type != kInteger)
			warning("ScriptInterpreter::execOpcode() Trying to compare non-integer values (%d, %d, line %d)", value1.type, value2.type, _lineNum);
		_cmpFlags = value1.value - value2.value;
		debugCN(kDebugScript, "-> cmp %d, %d\n", value1.value, value2.value);
		debugCN(kDebugScript, "-> _cmpFlags  = %d\n", _cmpFlags);
		return true;

	case opCall:
		temp = _runningFunction->readUint32();
		callFunction(temp);
		return true;

	case opCallKernel:
		temp = _runningFunction->readUint32();
		callKernelFunction(temp);
		return true;

	case opInc:
		loadValue(value1);
		value2 = value1;
		derefValue(value2);
		value2.value++;
		copyValue(value1, value2);
		return true;

	case opDec:
		loadValue(value1);
		value2 = value1;
		derefValue(value2);
		value2.value--;
		copyValue(value1, value2);
		return true;

	case opAdd:
		loadValue(value1);
		value3 = value1;
		loadValue(value2);
		derefValue(value3);
		derefValue(value2);
		value3.value += value2.value;
		copyValue(value1, value3);
		return true;

	case opSub:
		loadValue(value1);
		value3 = value1;
		loadValue(value2);
		derefValue(value3);
		derefValue(value2);
		value3.value -= value2.value;
		copyValue(value1, value3);
		return true;

	case opDebug:
		_lineNum = (int)_runningFunction->readUint32();
		return true;

	default:
		debugCN(kDebugScript, "Invalid opcode %d!\n", opcode);
		return false;

	}

}

// Kernel functions

#define STRING(arg) (toString(getArg(arg)))
#define INTEGER(arg) (toInteger(getArg(arg)))
#define DATA(arg, T) (toData<T>(getArg(arg)))
#define RETURN(value) (_registers[0] = (value))

int ScriptInterpreter::o1_handleStreamBreak() {
	return 0;
}

int ScriptInterpreter::o1_handlePlayBreak() {
	return 0;
}

int ScriptInterpreter::o1_dispatchTriggerOnSoundState() {
	return 0;
}

int ScriptInterpreter::o1_getTicks() {
	return 0;
}

int ScriptInterpreter::o1_setSoundVolume() {
	return 0;
}

int ScriptInterpreter::o1_getSoundStatus() {
	return 0;
}

int ScriptInterpreter::o1_getSoundDuration() {
	return 0;
}

int ScriptInterpreter::o1_setSeriesFrameRate() {
	return 0;
}

int ScriptInterpreter::o1_terminateMachine() {
	return 0;
}

int ScriptInterpreter::o1_sendWoodScriptMessage() {
	return 0;
}

int ScriptInterpreter::o1_runConversation() {
	return 0;
}

int ScriptInterpreter::o1_exportConversationValue() {
	return 0;
}

int ScriptInterpreter::o1_exportConversationPointer() {
	return 0;
}

int ScriptInterpreter::o1_playBreakSeries() {
	return 0;
}

int ScriptInterpreter::o1_hideWalker() {
	return 0;
}

int ScriptInterpreter::o1_showWalker() {
	return 0;
}

int ScriptInterpreter::o1_walk() {
	return 0;
}

int ScriptInterpreter::o1_overrideCrunchTime() {
	return 0;
}

int ScriptInterpreter::o1_addBlockingRect() {
	return 0;
}

int ScriptInterpreter::o1_setPlayerCommandsAllowed() {
	return 0;
}

int ScriptInterpreter::o1_getPlayerCommandsAllowed() {
	return 0;
}

int ScriptInterpreter::o1_setPlayerFacingAngle() {
	return 0;
}

int ScriptInterpreter::o1_disablePlayerFadeToBlack() {
	return 0;
}

int ScriptInterpreter::o1_enablePlayer() {
	return 0;
}

int ScriptInterpreter::o1_disablePlayer() {
	return 0;
}

int ScriptInterpreter::o1_freshenSentence() {
	return 0;
}

int ScriptInterpreter::o1_playerGiveItem() {
	return 0;
}

int ScriptInterpreter::o1_moveObject() {
	return 0;
}

int ScriptInterpreter::o1_setStopSoundsBetweenRooms() {
	return 0;
}

int ScriptInterpreter::o1_backupPalette() {
	return 0;
}

int ScriptInterpreter::o1_unloadWilburWalker() {
	return 0;
}

int ScriptInterpreter::o1_wilburTalk() {
	return 0;
}

int ScriptInterpreter::o1_wilburFinishedTalking() {
	return 0;
}

int ScriptInterpreter::o1_preloadSound() {
	const char *name = STRING(0);
	int room = INTEGER(1);
	debugCN(kDebugScript, "name = %s; room = %d\n", name, room);
	return 2;
}

int ScriptInterpreter::o1_unloadSound() {
	const char *name = STRING(0);
	int room = INTEGER(1);
	debugCN(kDebugScript, "name = %s; room = %d\n", name, room);
	return 2;
}

int ScriptInterpreter::o1_playSound() {
	const char *name = STRING(0);
	int channel = INTEGER(1);
	int volume = INTEGER(2);
	int trigger = INTEGER(3);
	int room = INTEGER(4);
	debugCN(kDebugScript, "name = %s; channel = %d; volume = %d; trigger = %d; room = %d\n",
		name, channel, volume, trigger, room);

	Common::String soundName = Common::String(name) + ".raw";
	_vm->_sound->playVoice(soundName.c_str(), 100);

	// HACK until fixed
	_vm->_kernel->sendTrigger(trigger);

	return 5;
}

int ScriptInterpreter::o1_playLoopingSound() {
	const char *name = STRING(0);
	int channel = INTEGER(1);
	int volume = INTEGER(2);
	int trigger = INTEGER(3);
	int room = INTEGER(4);
	debugCN(kDebugScript, "name = %s; channel = %d; volume = %d; trigger = %d; room = %d\n",
		name, channel, volume, trigger, room);

	// HACK until fixed
	_vm->_kernel->sendTrigger(trigger);

	return 5;
}

int ScriptInterpreter::o1_stopSound() {
	int channel = INTEGER(0);
	debugCN(kDebugScript, "channel = %d\n", channel);
	return 1;
}

int ScriptInterpreter::o1_fadeSetStart() {
	// skip arg 0: palette ptr
	int percent = INTEGER(1);
	debugCN(kDebugScript, "percent = %d\n", percent);
	return 2;
}

int ScriptInterpreter::o1_fadeInit() {
	// skip arg 0: palette ptr
	int first = INTEGER(1);
	int last = INTEGER(2);
	int percent = INTEGER(3);
	int ticks = INTEGER(4);
	int trigger = INTEGER(5);
	debugCN(kDebugScript, "first = %d; last = %d; percent = %d; ticks = %d; trigger = %d\n",
		first, last, percent, ticks, trigger);

	// HACK until palette fading is implemented
	_vm->_kernel->sendTrigger(trigger);

	return 6;
}

int ScriptInterpreter::o1_fadeToBlack() {
	return 0;
}

int ScriptInterpreter::o1_initPaletteCycle() {
	int first = INTEGER(0);
	int last = INTEGER(1);
	int delay = INTEGER(2);
	int ticks = INTEGER(3);
	int trigger = INTEGER(4);
	debugCN(kDebugScript, "first = %d; last = %d; delay = %d; ticks = %d; trigger = %d\n",
		first, last, delay, ticks, trigger);

	// HACK until palette cycling is implemented
	_vm->_kernel->sendTrigger(trigger);

	return 5;
}

int ScriptInterpreter::o1_stopPaletteCycle() {
	return 0;
}

int ScriptInterpreter::o1_hasPlayerSaid() {
	const char *words[3];
	for (int i = 0; i < 3; i++)
		words[i] = STRING(i);
	debugCN(kDebugScript, "'%s', '%s', '%s'\n", words[0], words[1], words[2]);

	int result = _vm->_player->said(words[0], words[1], words[2]);

	debugCN(kDebugScript, "   -> '%d'\n", result);

	RETURN(result);
	return 3;
}

int ScriptInterpreter::o1_hasPlayerSaidAny() {
	const char *words[10];
	for (int i = 0; i < 10; i++)
		words[i] = STRING(i);

	debugCN(kDebugScript, "'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s'\n",
		words[0], words[1], words[2], words[3], words[4], words[5], words[6], words[7], words[8], words[9]);

	int result = _vm->_player->saidAny(words[0], words[1], words[2], words[3], words[4], words[5], words[6], words[7], words[8], words[9]);
	debugCN(kDebugScript, "   -> '%d'\n", result);

	RETURN(result);
	return 10;
}

int ScriptInterpreter::o1_updatePlayerInfo() {
	// skip arg 0: player info struct
	return 1;
}

int ScriptInterpreter::o1_playerHotspotWalkOverride() {
	int x1 = INTEGER(0);
	int y1 = INTEGER(1);
	int x2 = INTEGER(2);
	int y2 = INTEGER(3);
	debugCN(kDebugScript, "(%d, %d); (%d, %d)\n", x1, y1, x2, y2);
	return 4;
}

int ScriptInterpreter::o1_playerHasItem() {
	const char *name = STRING(0);
	debugCN(kDebugScript, "item = '%s'\n", name);
	// TODO
	RETURN(0);
	return 1;
}

int ScriptInterpreter::o1_setWalkerLocation() {
	// skip arg 0: walker
	int x = INTEGER(1);
	int y = INTEGER(2);
	debugCN(kDebugScript, "x = %d; y = %d\n", x, y);
	return 3;
}

int ScriptInterpreter::o1_setWalkerFacing() {
	// skip arg 0: walker
	int facing = INTEGER(1);
	debugCN(kDebugScript, "facing = %d\n", facing);
	return 2;
}

int ScriptInterpreter::o1_setHotspot() {
	// skip arg 0: hotspot list
	const char *name = STRING(1);
	int value = INTEGER(2);
	debugCN(kDebugScript, "name = '%s' -> %d\n", name, value);

	_vm->_scene->getSceneResources().hotspots->setActive(name, (value != 0));

	return 2;
}

int ScriptInterpreter::o1_loadConversation() {
	const char *name = STRING(0);
	//int trigger = INTEGER(1);
	//int flag = INTEGER(2);

	// TODO; just to show something
	_m4Vm->_converse->startConversation(name);

	return 3;
}

int ScriptInterpreter::o1_playSeries() {
	const char *name = STRING(0);
	int layer = INTEGER(1);
	int flags = INTEGER(2);
	int trigger = INTEGER(3);
	int frameRate = INTEGER(4);
	int loopCount = INTEGER(5);
	int scale = INTEGER(6);
	int x = INTEGER(7);
	int y = INTEGER(8);
	int firstFrame = INTEGER(9);
	int lastFrame = INTEGER(10);

	debugCN(kDebugScript, "name = %s; layer = %04X; flags = %08X; trigger = %d; frameRate = %d; loopCount = %d; scale = %d; x = %d; y = %d: firstFrame = %d; lastFrame = %d\n",
		name, layer, flags, trigger, frameRate, loopCount, scale, x, y, firstFrame, lastFrame);

	// TODO: Return the machine to the script
	_vm->_ws->playSeries(name, layer, flags, trigger, frameRate, loopCount, scale, x, y, firstFrame, lastFrame);

	return 11;
}

int ScriptInterpreter::o1_showSeries() {
	const char *name = STRING(0);
	int layer = INTEGER(1);
	int flags = INTEGER(2);
	int trigger = INTEGER(3);
	int duration = INTEGER(4);
	int frameIndex = INTEGER(5);
	int scale = INTEGER(6);
	int x = INTEGER(7);
	int y = INTEGER(8);

	debugCN(kDebugScript, "name = %s; layer = %04X; flags = %08X; trigger = %d; duration = %d; frameIndex = %d; scale = %d; x = %d; y = %d\n",
		name, layer, flags, trigger, duration, frameIndex, scale, x, y);

	// TODO: Return the machine to the script
	_vm->_ws->showSeries(name, layer, flags, trigger, duration, frameIndex, scale, x, y);

	return 9;
}

int ScriptInterpreter::o1_loadSeries() {
	const char *name = STRING(0);
	int hash = INTEGER(1);
	// skip arg 3: palette ptr

	debugCN(kDebugScript, "name = %s; hash = %d\n", name, hash);

	int result = _vm->_ws->loadSeries(name, hash, NULL);

	RETURN(result);
	return 3;
}

int ScriptInterpreter::o1_unloadSeries() {
	return 0;
}

int ScriptInterpreter::o1_preloadBreakSeries() {
	//const SeriesStreamBreakList& seriesStreamBreakList = DATA(0, SeriesStreamBreakList);
	return 1;
}

int ScriptInterpreter::o1_unloadBreakSeries() {
	//const SeriesStreamBreakList& seriesStreamBreakList = DATA(0, SeriesStreamBreakList);
	return 1;
}

int ScriptInterpreter::o1_startBreakSeries() {
	//const SeriesStreamBreakList& seriesStreamBreakList = DATA(0, SeriesStreamBreakList);
	return 1;
}

int ScriptInterpreter::o1_globalTriggerProc() {
	int value1 = INTEGER(0);
	int value2 = INTEGER(1);
	int value3 = INTEGER(2);
	debugCN(kDebugScript, "%d; %d; %d\n", value1, value2, value3);
	return 3;
}

int ScriptInterpreter::o1_triggerTimerProc() {
	int value1 = INTEGER(0);
	int value2 = INTEGER(1);
	int value3 = INTEGER(2);
	debugCN(kDebugScript, "%d; %d; %d\n", value1, value2, value3);
	return 3;
}

int ScriptInterpreter::o1_dispatchTrigger() {
	int trigger = INTEGER(0);
	debugCN(kDebugScript, "trigger = %d\n", trigger);

	_vm->_kernel->sendTrigger(trigger);
	//g_system->delayMillis(5000);

	return 1;
}

int ScriptInterpreter::o1_getRangedRandomValue() {
	int minValue = INTEGER(0);
	int maxValue = INTEGER(1);
	RETURN(_vm->imath_ranged_rand(minValue, maxValue));
	return 2;
}

int ScriptInterpreter::o1_wilburSaid() {
	const SaidArray& saidArray = DATA(0, SaidArray);

	int result = 0;

	// NOTE: The "Common::String soundName" stuff is just temporary until playVoice is fixed.

	for (int i = 0; i < saidArray.size(); i++) {
		SaidArrayItem *item = saidArray[i];

		if (_vm->_player->said("LOOK AT", item->itemName) && item->digiNameLook) {
			debugCN(kDebugScript, "  -> LOOK AT: '%s'\n", item->digiNameLook);
			Common::String soundName = Common::String(item->digiNameLook) + ".raw";
			_vm->_sound->playVoice(soundName.c_str(), 100);
			result = 1;
			break;
		}

		if (_vm->_player->said("TAKE", item->itemName) && item->digiNameTake) {
			debugCN(kDebugScript, "  -> TAKE: '%s'\n", item->digiNameTake);
			Common::String soundName = Common::String(item->digiNameTake) + ".raw";
			_vm->_sound->playVoice(soundName.c_str(), 100);
			result = 1;
			break;
		}

		if (_vm->_player->said("GEAR", item->itemName) && item->digiNameGear) {
			debugCN(kDebugScript, "  -> GEAR: '%s'\n", item->digiNameGear);
			Common::String soundName = Common::String(item->digiNameGear) + ".raw";
			_vm->_sound->playVoice(soundName.c_str(), 100);
			result = 1;
			break;
		}

		/*
		debugCN(kDebugScript, "##### itemName = '%s'; digiNameLook = %s; digiNameTake = %s; digiNameGear = %s\n",
			item->itemName, item->digiNameLook, item->digiNameTake, item->digiNameGear);
		*/
	}
	debugCN(kDebugScript, "   -> '%d'\n", result);

	RETURN(result);
	return 1;
}

int ScriptInterpreter::o1_wilburParse() {
	//const ParserArray& parserArray = DATA(0, ParserArray);
	RETURN(0);
	return 1;
}

int ScriptInterpreter::o1_wilburSpeech() {
	const char *name = STRING(0);
	int trigger = INTEGER(1);
	int room = INTEGER(2);
	int flag = INTEGER(3);
	int volume = INTEGER(4);
	int slot = INTEGER(5);

	debugCN(kDebugScript, "%s; %d; %d; %d; %d; %d\n", name, trigger, room, flag, volume, slot);
	//g_system->delayMillis(5000);

	KernelTriggerType oldTriggerMode = _vm->_kernel->triggerMode;

	// TODO
	Common::String soundName = Common::String(name) + ".raw";
	_vm->_sound->playVoice(soundName.c_str(), 100);

	_vm->_kernel->triggerMode = oldTriggerMode;

	return 6;
}

// Kernel vars

void ScriptInterpreter::getKernelVar(int index, ScriptValue &value) {

	debugCN(kDebugScript, "ScriptInterpreter::getKernelVar() index = %d\n", index);

	if (index > _kernelVarsMax) {
		debugCN(kDebugScript, "ScriptInterpreter::getKernelVar() Invalid kernel var index %d!\n", index);
		return;
	}

	debugCN(kDebugScript, "ScriptInterpreter::getKernelVar() name = %s\n", _kernelVars[index].desc);

	ScriptKernelVariable var = _kernelVars[index].var;

	switch (var) {

	case kKernelTrigger:
		value = _vm->_kernel->trigger;
		break;

	case kKernelTriggerMode:
		value = (int)_vm->_kernel->triggerMode;
		break;

	case kKernelContinueHandlingTrigger:
		value = (int)_vm->_kernel->daemonTriggerAvailable;
		break;

	case kGameVersion:
		// TODO
		value = 0;
		break;

	case kGameLanguage:
		// TODO
		value = 0;
		break;

	case kGameNewRoom:
		// TODO
		value = 0;
		break;

	case kPlayerCommandReady:
		value = (int)_vm->_player->commandReady;
		break;

	default:
		debugCN(kDebugScript, "ScriptInterpreter::getKernelVar() Invalid kernel var %d!\n", var);
		//g_system->delayMillis(2000);

	}

}

void ScriptInterpreter::setKernelVar(int index, const ScriptValue &value) {

	debugCN(kDebugScript, "ScriptInterpreter::setKernelVar() index = %d\n", index);

	if (index > _kernelVarsMax) {
		debugCN(kDebugScript, "ScriptInterpreter::setKernelVar() Invalid kernel var index %d!\n", index);
		return;
	}

	debugCN(kDebugScript, "ScriptInterpreter::setKernelVar() name = %s\n", _kernelVars[index].desc);

	ScriptKernelVariable var = _kernelVars[index].var;

	switch (var) {

	case kKernelTrigger:
		_vm->_kernel->trigger = toInteger(value);
		debugCN(kDebugScript, "kKernelTrigger -> %d\n", toInteger(value));
		break;

	case kKernelTriggerMode:
		_vm->_kernel->triggerMode = (KernelTriggerType)toInteger(value);
		debugCN(kDebugScript, "kKernelTrigger -> %d\n", toInteger(value));
		break;

	case kKernelContinueHandlingTrigger:
		_vm->_kernel->daemonTriggerAvailable = (toInteger(value) != 0);
		debugCN(kDebugScript, "kKernelContinueHandlingTrigger -> %d\n", toInteger(value));
		break;

	case kGameNewRoom:
		_vm->_kernel->newRoom = toInteger(value);
		debugCN(kDebugScript, "kGameNewRoom -> %d\n", toInteger(value));
		break;

	case kPlayerCommandReady:
		// TODO
		debugCN(kDebugScript, "kPlayerCommandReady -> %d\n", toInteger(value));
		break;

	default:
		debugCN(kDebugScript, "ScriptInterpreter::setKernelVar() Invalid kernel var %d!\n", var);
		//g_system->delayMillis(2000);

	}

}

} // End of namespace M4
