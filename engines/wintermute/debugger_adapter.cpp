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
#define SCENGINE _engine->getGame()->_scEngine
#define DEBUGGER _engine->getConsole()
#define DBG_PATH "dbg"

namespace Wintermute {

SourceFile::SourceFile(const Common::String &filename) {

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

		_strings.empty();

		while (!file->eos()) {
			_strings.insert_at(_strings.size(), file->readLine());
			if (file->err()) {
				_err = file->err();
			}
		}
	}
}

bool SourceFile::isBlank(int line) {
	Common::String theLine = getLine(line);
	Common::StringTokenizer st(theLine, "//");
	Common::String charsBeforeComment = st.nextToken();
	charsBeforeComment.trim();
	if (charsBeforeComment.size() == 0) {
		// No chars before "//" except for whitespaces... looks like a comment or an empty line
		return true;
	}
	return false;
}

int SourceFile::getLength() {
	if (_err) {
		return 0;
	}
	return _strings.size();
}

Common::String SourceFile::getLine(uint n, int *error) {

	// Line numbers are starting from 1, so...
	n--;

	if (_err) {
		if (error != nullptr) {
			*error = COULD_NOT_OPEN;
		}
		return 0;
	}
	if (n >= _strings.size()) { // also n < 0 if type ever changes
		if (error != nullptr) {
			*error = NO_SUCH_LINE;
		}
		return 0;
	}
	return _strings[n];
}


Common::Array<Common::String> SourceFile::getSurroundingLines(int center, int lines, int *error) {
	return getSurroundingLines(center, lines, lines, error);
}


Common::Array<Common::String> SourceFile::getSurroundingLines(int center, int before, int after, int *error) {
	*error = 0;
	int start = MAX(center - before, 1);
	int finish = MIN(center + after, getLength()); // Line numbers start from 1
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

DebuggerAdapter::DebuggerAdapter(WintermuteEngine *vm) {
	_engine = vm;
	_lastSource = nullptr;
	reset();
}

DebuggerAdapter::~DebuggerAdapter() {
	delete _lastSource; 
}

bool DebuggerAdapter::compiledExists(Common::String filename) {
	uint32 compSize;
	byte *compBuffer = SCENGINE->getCompiledScript(filename.c_str(), &compSize);
	if (!compBuffer) {
		return false;
	} else {
		return true;
	}
}

int DebuggerAdapter::isBreakpointLegal(const char *filename, int line) {

	// First of all: does the compiled even exist?
	// Otherwise, well... it's very much not legal.

	if (!compiledExists(filename)) {
		return NO_SUCH_SCRIPT;
	}

	SourceFile sf(filename);
	int error = OK;
	sf.getLine(line, &error);

	if (!error) {
		if (sf.isBlank(line)) {
			return IS_BLANK;
		} else {
			return OK;
		}
	} else if (error == SourceFile::NO_SUCH_SOURCE || error == SourceFile::COULD_NOT_OPEN) {
		// Okay, this does not tell us much, except that we don't have the SOURCE file.
		// TODO: Check if the bytecode is there, at least
		return NO_SUCH_SOURCE;
	} else if (error == NO_SUCH_LINE) {
		return NO_SUCH_LINE; // There is apparently no such line in the SOURCE file.
	} else {
		return error;
	}
}

int DebuggerAdapter::addBreakpoint(const char *filename, int line) {
	assert(SCENGINE);
	if (!compiledExists(filename)) {
		return NO_SUCH_SCRIPT;
	}
	int isLegal = isBreakpointLegal(filename, line);
	if (isLegal == OK) {
		SCENGINE->addBreakpoint(filename, line);
		return OK;
	} else if (isLegal == SourceFile::IS_BLANK) {
		// We don't have the SOURCE. A warning will do.
		SCENGINE->addBreakpoint(filename, line);
		return IS_BLANK;
	} else if (isLegal == SourceFile::NO_SUCH_SOURCE) {
		// We don't have the SOURCE. A warning will do.
		SCENGINE->addBreakpoint(filename, line);
		return NO_SUCH_SOURCE;
	} else if (isLegal == SourceFile::NO_SUCH_LINE) {
		// No line in the source A warning will do.
		SCENGINE->addBreakpoint(filename, line);
		return NO_SUCH_LINE;
	} else {
		// Something weird? Don't do anything.
		return isLegal;
	}
}

int DebuggerAdapter::removeBreakpoint(int id) {
	assert(SCENGINE);

	if (SCENGINE->removeBreakpoint(id)) {
		return OK;
	} else {
		return NO_SUCH_BREAKPOINT;
	}
}

int DebuggerAdapter::disableBreakpoint(int id) {
	assert(SCENGINE);

	if (SCENGINE->disableBreakpoint(id)) {
		return OK;
	} else {
		return NO_SUCH_BREAKPOINT;
	}
}

int DebuggerAdapter::enableBreakpoint(int id) {
	assert(SCENGINE);
	if (SCENGINE->enableBreakpoint(id)) {
		return OK;
	} else {
		return NO_SUCH_BREAKPOINT;
	}
}

int DebuggerAdapter::removeWatchpoint(int id) {
	assert(SCENGINE);

	if (SCENGINE->removeWatchpoint(id)) {
		return OK;
	} else {
		return NO_SUCH_BREAKPOINT;
	}
}


int DebuggerAdapter::disableWatchpoint(int id) {
	assert(SCENGINE);

	if (SCENGINE->disableWatchpoint(id)) {
		return OK;
	} else {
		return NO_SUCH_BREAKPOINT;
	}
}

int DebuggerAdapter::enableWatchpoint(int id) {
	assert(SCENGINE);
	if (SCENGINE->enableWatchpoint(id)) {
		return OK;
	} else {
		return NO_SUCH_BREAKPOINT;
	}
}

int DebuggerAdapter::addWatch(const char *filename, const char *symbol) {
	assert(SCENGINE);
	if (!compiledExists(filename)) {
		return NO_SUCH_SCRIPT;
	}	
	
	/* In this case there's not a lot we can check for.
	 * We don't have a symbol table, we can only either hit it
	 * at runtime or... not.
	 *
	 * Basically, because of the way the script engine works,
	 * (no symbol table, only II_DBG_LINE to tell us that where 
	 * we are corresponds to some line in the source)
	 * watching is awkward, unless we parse the whole source.
	 */

	SCENGINE->addWatchpoint(filename, symbol);
	return OK;
}

bool DebuggerAdapter::triggerBreakpoint(ScScript *script) {
	_lastDepth = script->getCallDepth();
	_lastScript = script;
	_lastLine = script->_currentLine;
	delete _lastSource;
	_lastSource = new SourceFile(script->_filename);
	DEBUGGER->notifyBreakpoint(script->dbgGetFilename(), script->_currentLine);
	return 1;
}

bool DebuggerAdapter::triggerStep(ScScript *script) {
	_lastDepth = script->getCallDepth();
	_lastScript = script; // If script has changed do we still care?
	_lastLine = script->_currentLine;
	delete _lastSource;
	_lastSource = new SourceFile(script->_filename);
	DEBUGGER->notifyStep(script->dbgGetFilename(), script->_currentLine);
	return 1;
}

bool DebuggerAdapter::triggerWatch(ScScript *script, const char *symbol) {
	_lastDepth = script->getCallDepth();
	_lastScript = script; // If script has changed do we still care?
	_lastLine = script->_currentLine;
	delete _lastSource;
	_lastSource = new SourceFile(script->_filename);
	DEBUGGER->notifyWatch(script->dbgGetFilename(), symbol, script->resolveName(symbol)->getString());
	return 1;
}

int DebuggerAdapter::stepOver() {
	if (!_lastScript) {
		return NOT_ALLOWED;
	}
	_lastScript->_step = _lastDepth;
	reset();
	return OK;
}

int DebuggerAdapter::stepInto() {
	if (!_lastScript) {
		return NOT_ALLOWED;
	}
	_lastScript->_step = _lastDepth + 1;
	reset();
	return OK;
}

int DebuggerAdapter::stepContinue() {
	if (!_lastScript) {
		return NOT_ALLOWED;
	}
	_lastScript->_step = -2;
	return OK;
}

int DebuggerAdapter::stepFinish() {
	if (!_lastScript) {
		return NOT_ALLOWED;
	}
	_lastScript->_step = _lastDepth - 1;
	// Reset
	reset();
	return OK;
}

void DebuggerAdapter::reset() {
	_lastScript = nullptr;
	_lastLine = -1;
	_lastDepth = kDefaultStep;
}

Common::String DebuggerAdapter::readValue(const char *name, int *error) {
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
		*error = WRONG_TYPE; // TODO: Better one
		return nullptr;
	}

	BaseScriptable *pos;
	// Now we split tokens like foo(bar)

	pos = result->getNative();
	Common::String methodName = Common::String("");

	while (!st.empty() && result) {
		pos = result->getNative();
		if (!result->isNative()) {
			*error = WRONG_TYPE; // TODO: Better one
			return nullptr;
		}

		Common::String callStr = st.nextToken();
		// Okay - now let's see if it's a call
		Common::StringTokenizer callSt = Common::StringTokenizer(callStr.c_str(), "(");
		methodName = callSt.nextToken();
		Common::String args;
		if (callSt.empty()) {
			result = pos->scGetProperty(methodName);
		} else {
			// Let's try to chop parenthesis off it.
			Common::String argList = callSt.nextToken();
			Common::StringTokenizer argSt = Common::StringTokenizer(argList.c_str(), ")");
			args = argSt.nextToken();
			if (argSt.empty()) {
				// OK
			} else {
				// WTF? This should not happen.
				*error = PARSE_ERROR;
				return nullptr;
			}
			// Split args here:

			Common::StringTokenizer commas = Common::StringTokenizer(args, ",");

			int argc = 0;
			while (!commas.empty()) {
				
				Common::String arg = commas.nextToken();
				Common::StringTokenizer st3 = Common::StringTokenizer(arg, "\"");
				Common::String dest;
				dest = "";

				dest += st3.nextToken();
				bool isString = false;

				int tokencount = 0;
				while (!st3.empty()) {
					dest += st3.nextToken();	
					isString = true;
					tokencount++;
				}

				assert (tokencount < 3);
				
				if (isString) {
					dest.trim();
					_lastScript->_stack->pushString(dest.c_str());
				} else {
					// We assume is int.
					_lastScript->_stack->pushInt(atoi(dest.c_str())); 
					// Todo: manually parse floats and stuff
				}
				argc++;
				arg = commas.nextToken();
			}

			_lastScript->_stack->pushInt(argc);

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


int DebuggerAdapter::setType(const Common::String &name, Common::String &type) {

	type.trim();

	if (!_lastScript) {
		return NOT_ALLOWED;
	}

	ScValue *var = _lastScript->getVar(name.c_str());

	if (type == "VAL_NULL") {
		var->setType(VAL_NULL);
		return OK;
	} else if (type == "VAL_STRING") {
		var->setType(VAL_STRING);
		return OK;
	} else if (type == "VAL_INT") {
		var->setType(VAL_INT);
		return OK;
	} else if (type == "VAL_BOOL") {
		var->setType(VAL_BOOL);
		return OK;
	} else if (type == "VAL_FLOAT") {
		var->setType(VAL_FLOAT);
		return OK;
	} else if (type == "VAL_OBJECT") {
		var->setType(VAL_OBJECT);
		return OK;
	} else if (type == "VAL_NATIVE") {
		var->setType(VAL_NATIVE);
		return OK;
	} else if (type == "VAL_VARIABLE_REF") {
		var->setType(VAL_VARIABLE_REF);
		return OK;
	}

	return PARSE_ERROR;
}

int DebuggerAdapter::setValue(Common::String name, Common::String value, ScValue *&var) {
	if (!_lastScript) {
		return NOT_ALLOWED;
	}

	value.trim();

	var = _lastScript->getVar(name.c_str());
	if (var->_type == VAL_INT) {
		char *endptr;
		int res = strtol(value.c_str(), &endptr, 10); // TODO: Hex too?
		if (endptr == value.c_str()) {
			return PARSE_ERROR;
		} else if (endptr == value.c_str() + value.size()) {
			// We've parsed all of it, have we?
			var->setInt(res);
		} else {
			assert(false);
			return PARSE_ERROR;
			// Something funny happened here.
		}
	} else if (var->_type == VAL_FLOAT) {
		char *endptr;
		float res = (float)strtod(value.c_str(), &endptr);
		if (endptr == value.c_str()) {
			return PARSE_ERROR;
		} else if (endptr == value.c_str() + value.size()) {
			// We've parsed all of it, have we?
			var->setFloat(res);
		} else {
			return PARSE_ERROR;
			assert(false);
			// Something funny happened here.
		}
	} else if (var->_type == VAL_BOOL) {
		Common::String str = Common::String(value);
		bool valAsBool;
		if (Common::parseBool(value, valAsBool)) {
			var->setBool(valAsBool);
		} else {
			return PARSE_ERROR;
		}
	} else if (var->_type == VAL_STRING) {
		var->setString(value);
	} else {
		return NOT_YET_IMPLEMENTED;
	}
	return 0;
}

void DebuggerAdapter::showFps(bool show) {
	_engine->getGame()->setShowFPS(show);
}

Common::Array<BreakpointInfo> DebuggerAdapter::getBreakpoints() {
	assert(SCENGINE);
	Common::Array<BreakpointInfo> breakpoints;
	for (uint i = 0; i < SCENGINE->_breakpoints.size(); i++) {
		BreakpointInfo bpInfo;
		bpInfo._filename = SCENGINE->_breakpoints[i]._filename;
		bpInfo._line = SCENGINE->_breakpoints[i]._line;
		bpInfo._hits = SCENGINE->_breakpoints[i]._hits;
		bpInfo._enabled = SCENGINE->_breakpoints[i]._enabled;
		breakpoints.push_back(bpInfo);
	}
	return breakpoints;
}

Common::Array<WatchInfo> DebuggerAdapter::getWatchlist() {
	Common::Array<WatchInfo> watchlist;
	for (uint i = 0; i < SCENGINE->_watchlist.size(); i++) {
		WatchInfo watchInfo;
		watchInfo._filename = SCENGINE->_watchlist[i]._filename;
		watchInfo._symbol = SCENGINE->_watchlist[i]._symbol;
		watchInfo._hits = SCENGINE->_watchlist[i]._hits;
		watchlist.push_back(watchInfo);
	}
	return watchlist;
}

int32 DebuggerAdapter::getLastLine() {
	return _lastLine;
}

int DebuggerAdapter::setSourcePath(Common::String sourcePath) {
	// TODO: check if path is legal
	_sourcePath = sourcePath;
	return OK;
}

Common::String DebuggerAdapter::getSourcePath() {
	return _sourcePath;
}

} // end of namespace Wintermute
