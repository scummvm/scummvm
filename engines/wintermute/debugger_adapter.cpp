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

#include "common/str.h"
#include "engines/wintermute/debugger_adapter.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "common/tokenizer.h"
#include <limits.h>
#include <errno.h>
#define SCENGINE _engine->_game->_scEngine
#define DEBUGGER _engine->_debugger
#define DBG_PATH "dbg"

namespace Wintermute {

SourceFile::SourceFile (Common::String filename) {

	_err = 0;

	Common::String dst;
	dst = Common::String(DBG_PATH) + Common::String("\\") + filename;

	Common::SeekableReadStream *file = BaseFileManager::getEngineInstance()->openFile(dst);
	
	if (!file) {
		_err = 1;
	} else {
		if (file->err()) {
			_err = file->err();
		}
		while (!file->eos()) {
			_strings.add(file->readLine());
			if (file->err()) { 
				_err = file->err();
			}
		}
	}
}

DebuggerAdapter::DebuggerAdapter(WintermuteEngine *vm) {
	_engine = vm;
	_lastScript = nullptr;
}

int SourceFile::getLength() {
	if (_err) {
		return 0;
	}
	return _strings.size();
}

Common::String SourceFile::getLine(int n, int *error) {
	if (_err) {
		if (error != nullptr) {
			*error = COULD_NOT_OPEN;
		}
		return 0;
	}	
	if (n < 0 || n > _strings.size()) {
		if (error != nullptr) {
			*error = NO_SUCH_LINE;
		}
		return 0;
	}
	return _strings[n];
}


BaseArray<Common::String> SourceFile::getSurroundingLines(int center, int lines, int *error) {
	return getSurroundingLines(center, lines, lines, error);
}


BaseArray<Common::String> SourceFile::getSurroundingLines(int center, int before, int after, int *error) {
	*error = 0;
	int start = MAX(center - before, 0);
	int finish = MIN(center + after, getLength() - 1);
	BaseArray<Common::String> ret;
	Common::String temp;
	for (int i = start; i <= finish && *error == 0; i++) {
		temp = Common::String::format("%d", i);
		temp += " ";
		temp += getLine(i, error).c_str();
		ret.add(temp);
	}
	return ret;
}

int DebuggerAdapter::addBreakpoint(const char *filename, int line) {
	// TODO: Check if file exists, check if line exists
	assert(SCENGINE);
	SCENGINE->addBreakpoint(filename, line);
	return OK;
}

int DebuggerAdapter::removeBreakpoint(int id) {
	// TODO: Check blah.
	assert(SCENGINE);

	if (SCENGINE->removeBreakpoint(id)) {
		return OK;
	} else {
		return NO_SUCH_BREAKPOINT;
	}
}

int DebuggerAdapter::addWatch(const char *filename, const char *symbol) {
	// TODO: Check if file exists, check if symbol exists
	assert(SCENGINE);
	SCENGINE->addWatch(filename, symbol);
	return OK;
}

bool DebuggerAdapter::triggerBreakpoint(ScScript *script) {
	_lastDepth = script->getCallDepth();
	_lastScript = script;
	_lastLine = script->_currentLine;
	_lastSource = new SourceFile(script->_filename);
	// Todo: leak?
	DEBUGGER->notifyBreakpoint(script->dbgGetFilename(), script->_currentLine);
	return 1;
}

bool DebuggerAdapter::triggerStep(ScScript *script) {
	_lastDepth = script->getCallDepth();
	_lastScript = script; // If script has changed do we still care?
	_lastLine = script->_currentLine;
	_lastSource = new SourceFile(script->_filename);
	// Todo: leak?
	DEBUGGER->notifyStep(script->dbgGetFilename(), script->_currentLine);
	return 1;
}

bool DebuggerAdapter::triggerWatch(ScScript *script, const char *symbol) {
	_lastDepth = script->getCallDepth();
	_lastScript = script; // If script has changed do we still care?
	_lastLine = script->_currentLine;
	_lastSource = new SourceFile(script->_filename);
	DEBUGGER->notifyWatch(script->dbgGetFilename(), symbol, script->resolveName(symbol)->getString());
	return 1;
}

int DebuggerAdapter::stepOver() {
	// TODO: Check if allowed
	assert(_lastScript);
	_lastScript->_step = _lastDepth;
	return OK;
}

int DebuggerAdapter::stepInto() {
	// TODO: Check if allowed
	assert(_lastScript);
	_lastScript->_step = _lastDepth + 1;
	return OK;
}

int DebuggerAdapter::stepContinue() {
	// TODO: Check if allowed
	assert(_lastScript);
	_lastScript->_step = _lastDepth - 1;
	return OK;
}

Common::String DebuggerAdapter::readValue(const char* name, int *error) {
	if (!_lastScript) {
		*error = NOT_ALLOWED;
		return Common::String();
	}
	return _lastScript->getVar(name)->getString();
}

Common::String DebuggerAdapter::readRes(const Common::String &name, int *error) { // Hack
	if (!_lastScript) {
		*error = NOT_ALLOWED;
		return nullptr;
	}

	Common::String strName = Common::String(name);
	strName.trim();
	Common::StringTokenizer st = Common::StringTokenizer(strName.c_str(), ".");
	
	Common::String mainObjectName;
	mainObjectName = st.nextToken(); // First token
	ScValue *result = _lastScript->getVar(mainObjectName.c_str());

	if (!result) {
		*error = NOT_ALLOWED; // TODO: Better one
		return nullptr;
	}
	

	if (!result->isNative()) {
		*error = NOT_ALLOWED; // TODO: Better one
		return nullptr;
	}
	
	BaseScriptable *pos; //  = mainObject->getNative();
	// Now we split tokens like foo(bar)

	pos  = result->getNative();
	Common::String methodName = Common::String("");

	while (!st.empty() && result) {
		// TODO: if result is not native error out
		pos  = result->getNative();

		Common::String callStr = st.nextToken();
		// Okay - now let's see if it's a call
		Common::StringTokenizer callSt = Common::StringTokenizer(callStr.c_str(), "(");
		methodName =  callSt.nextToken();
		Common::String arg;
		if (callSt.empty()) {
			result = pos->scGetProperty(methodName);
			// TODO: Okay, just return that, it's a property
		} else {
			// TODO: support multiple arguments.
			// Let's try to chop parenthesis off it.
			Common::String argList = callSt.nextToken();
			Common::StringTokenizer argSt = Common::StringTokenizer(argList.c_str(), ")");
			arg = argSt.nextToken();
			if (argSt.empty()) {
				// OK
			} else { 
				// WTF? This should not happen.
				assert(false);
			}

			Common::StringTokenizer st3 = Common::StringTokenizer(arg, "\"");
			Common::String dest;
			dest = "";
			while (!st3.empty()) {
				dest += st3.nextToken();
			}
			_lastScript->_stack->pushString(dest.c_str()); // Todo: support ints and other stuff. Remove quotes correctly!
			_lastScript->_stack->pushInt(1);

			if (pos->scCallMethod(_lastScript, _lastScript->_stack, _lastScript->_thisStack, methodName.c_str()) == STATUS_OK) {
				result = _lastScript->_stack->pop();
			} else {
				_lastScript->_stack->pop();
				result = 0;
			}
		}
	}
	if (!result) {
		return Common::String::format("%s has no member %s", pos->getName(), methodName.c_str());
	} else if (result->isNULL()) {
		return "Null";
	} else if (result->isNative()) {
		return result->getNative()->debuggerToString();
	} else if (result->isString()) {
		return result->getString();
	} else if (result->isFloat()) {
		return result->getString();
	} else if (result->isInt()) {
		return result->getString();
	} else {
		return "Not yet implemented"; // TODO: A better way to cop out
	}
}


int DebuggerAdapter::setType(const char* name, int type) {
	// TODO: Less sucky way to pass types

	if (!_lastScript) {
		return NOT_ALLOWED;
	}
	ScValue *var = _lastScript->getVar(name);
	TValType ttype = (TValType)type;
	var->setType(ttype);
	return 0;

}

int DebuggerAdapter::setValue(Common::String name, Common::String value) {
	if (!_lastScript) {
		return NOT_ALLOWED;
	}

	value.trim();

	ScValue *var = _lastScript->getVar(name.c_str());
	if (var->_type == VAL_INT) {
		char *endptr;
		errno = 0;
		int res = strtol(value.c_str(), &endptr, 10); // TODO: Hex too?

		if (
			(errno == ERANGE && (res == LONG_MAX || res == LONG_MIN)) 
			|| 
			(errno != 0 && res == 0)
			) {
			// TODO: Error, bail out
		} else if (endptr == value.c_str()) {
			// TODO: Couldn't parse anything... bail out?
		} else if (endptr == value.c_str() + value.size()) {
			// We've parsed all of it, have we?
			var->setInt(res);
		} else {
			assert (false);
			// Something funny happened here.
		}
	} else if (var->_type == VAL_FLOAT) {
		char *endptr;
		errno = 0;
		float res = (float)strtod(value.c_str(), &endptr);

		if (
			(errno == ERANGE && (res == HUGE_VAL)) 
			|| 
			(errno != 0 && res == 0)
			) {
			// TODO: Error, bail out
		} else if (endptr == value.c_str()) {
			// TODO: Couldn't parse anything... bail out?
		} else if (endptr == value.c_str() + value.size()) {
			// We've parsed all of it, have we?
			var->setFloat(res);
		} else {
			assert (false);
			// Something funny happened here.
		}
	} else if (var->_type == VAL_BOOL) {
		Common::String str = Common::String(value);
		bool valAsBool;
		Common::parseBool(value, valAsBool);
		// TODO: Warn if can't parse.
		var->setBool(valAsBool);
	} else if (var->_type == VAL_STRING) {
		var->setString(value);
	} else {
		// TODO> Not yet implemented
	}
	return 0;
}

bool DebuggerAdapter::showFps(bool show) {
	_engine->_game->setShowFPS(show);
	return true;
}

BaseArray<BreakpointInfo> DebuggerAdapter::getBreakpoints() {
	assert(SCENGINE);
	BaseArray<BreakpointInfo> breakpoints;
	for (int i = 0; i < SCENGINE->_breakpoints.size(); i++) {
		BreakpointInfo bp_info;
		bp_info._filename = SCENGINE->_breakpoints[i]._filename;
		bp_info._line = SCENGINE->_breakpoints[i]._line;
		bp_info._hits = SCENGINE->_breakpoints[i]._hits;
		breakpoints.add(bp_info);
	}
	return breakpoints;
}

BaseArray<WatchInfo> DebuggerAdapter::getWatchlist() {
	BaseArray<WatchInfo> watchlist;
	for (int i = 0; i < SCENGINE->_watchlist.size(); i++) {
		WatchInfo watch_info;
		watch_info._filename = SCENGINE->_watchlist[i]._filename;
		watch_info._symbol = SCENGINE->_watchlist[i]._symbol;
		watch_info._hits = SCENGINE->_breakpoints[i]._hits;
		watchlist.add(watch_info);
	}
	return watchlist;
}

int32 DebuggerAdapter::getLastLine() {
	return _lastLine;
}
} // end of namespace Wintermute
