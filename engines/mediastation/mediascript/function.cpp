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

#include "mediastation/mediascript/function.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

namespace MediaStation {
ScriptFunction::ScriptFunction(Chunk &chunk) {
	_contextId = chunk.readTypedUint16();
	// In PROFILE._ST (only present in some titles), the function ID is reported
	// with 19900 added, so function 100 would be reported as 20000. But in
	// bytecode, the zero-based ID is used, so that's what we'll store here.
	_id = chunk.readTypedUint16();
	_code = new CodeChunk(chunk);
}

ScriptFunction::~ScriptFunction() {
	delete _code;
	_code = nullptr;
}

ScriptValue ScriptFunction::execute(Common::Array<ScriptValue> &args) {
	debugC(5, kDebugScript, "\n********** SCRIPT FUNCTION %d **********", _id);
	ScriptValue returnValue = _code->execute(&args);
	debugC(5, kDebugScript, "********** END SCRIPT FUNCTION **********");
	return returnValue;
}

FunctionManager::~FunctionManager() {
	for (auto it = _functions.begin(); it != _functions.end(); ++it) {
		delete it->_value;
	}
	_functions.clear();
}

bool FunctionManager::attemptToReadFromStream(Chunk &chunk, uint sectionType) {
	bool handledParam = true;
	switch (sectionType) {
	case 0x31: {
		ScriptFunction *function = new ScriptFunction(chunk);
		_functions.setVal(function->_id, function);
		break;
	}

	default:
		handledParam = false;
	}

	return handledParam;
}

ScriptValue FunctionManager::call(uint functionId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	// The original had a complex function registration system that I deemed too uselessly complex to
	// reimplement. Here, we get basically the same behaviour by checking for default functions first,
	// then falling through to title-defined functions.
	switch (functionId) {
	case kEffectTransitionFunction:
		g_engine->getDisplayManager()->effectTransition(args);
		break;

	case kEffectTransitionOnSyncFunction:
		g_engine->getDisplayManager()->setTransitionOnSync(args);
		break;

	case kDrawingFunction:
		warning("STUB: %s", builtInFunctionToStr(static_cast<BuiltInFunction>(functionId)));
		break;

	case kUnk1Function:
		warning("%s: Function Unk1 Not implemented", __func__);
		returnValue.setToFloat(1.0);
		break;

	default: {
		// Execute the title-defined function here.
		ScriptFunction *scriptFunction = _functions.getValOrDefault(functionId);
		if (scriptFunction != nullptr) {
			returnValue = scriptFunction->execute(args);
		} else {
			error("%s: Unimplemented function 0x%02x", __func__, functionId);
		}
	}
	}

	return returnValue;
}

void FunctionManager::deleteFunctionsForContext(uint contextId) {
	// Collect function IDs to delete first.
	Common::Array<ScriptFunction *> functionsToDelete;
	for (auto it = _functions.begin(); it != _functions.end(); ++it) {
		ScriptFunction *scriptFunction = it->_value;
		if (scriptFunction->_contextId == contextId) {
			functionsToDelete.push_back(scriptFunction);
		}
	}

	// Now delete them.
	for (ScriptFunction *scriptFunction : functionsToDelete) {
		_functions.erase(scriptFunction->_id);
		delete scriptFunction;
	}
}

} // End of namespace MediaStation
