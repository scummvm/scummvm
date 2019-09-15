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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/algorithm.h"
#include "common/str.h"
#include "common/tokenizer.h"
#include "engines/wintermute/debugger.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/debugger/breakpoint.h"
#include "engines/wintermute/debugger/debugger_controller.h"
#include "engines/wintermute/debugger/watch.h"
#include "engines/wintermute/debugger/listing_providers/blank_listing_provider.h"
#include "engines/wintermute/debugger/listing_providers/cached_source_listing_provider.h"
#include "engines/wintermute/debugger/listing_providers/source_listing.h"
#define SCENGINE _engine->_game->_scEngine
#define DEBUGGER _engine->_debugger

namespace Wintermute {

DebuggerController::~DebuggerController() {
	delete _sourceListingProvider;
}

DebuggerController::DebuggerController(WintermuteEngine *vm) : _engine(vm) {
	_sourceListingProvider = new CachedSourceListingProvider();
	clear();
}

bool DebuggerController::bytecodeExists(const Common::String &filename) {
	uint32 compSize;
	byte *compBuffer = SCENGINE->getCompiledScript(filename.c_str(), &compSize);
	if (!compBuffer) {
		return false;
	} else {
		return true;
	}
}

Error DebuggerController::addBreakpoint(const char *filename, int line) {
	assert(SCENGINE);
	if (bytecodeExists(filename)) {
		SCENGINE->_breakpoints.push_back(new Breakpoint(filename, line, this));
		return Error(SUCCESS, OK);
	} else {
		return Error(ERROR, NO_SUCH_BYTECODE);
	}
}

Error DebuggerController::removeBreakpoint(uint id) {
	assert(SCENGINE);
	if (SCENGINE->_breakpoints.size() > id) {
		SCENGINE->_breakpoints.remove_at(id);
		return Error(SUCCESS, OK);
	} else {
		return Error(ERROR, NO_SUCH_BREAKPOINT, id);
	}
}

Error DebuggerController::disableBreakpoint(uint id) {
	assert(SCENGINE);
	if (SCENGINE->_breakpoints.size() > id) {
		SCENGINE->_breakpoints[id]->disable();
		return Error(SUCCESS, OK);
	} else {
		return Error(ERROR, NO_SUCH_BREAKPOINT, id);
	}
}

Error DebuggerController::enableBreakpoint(uint id) {
	assert(SCENGINE);
	if (SCENGINE->_breakpoints.size() > id) {
		SCENGINE->_breakpoints[id]->enable();
		return Error(SUCCESS, OK);
	} else {
		return Error(ERROR, NO_SUCH_BREAKPOINT, id);
	}
}

Error DebuggerController::removeWatchpoint(uint id) {
	assert(SCENGINE);
	if (SCENGINE->_watches.size() > id) {
		SCENGINE->_watches.remove_at(id);
		return Error(SUCCESS, OK);
	} else {
		return Error(ERROR, NO_SUCH_BREAKPOINT, id);
	}
}


Error DebuggerController::disableWatchpoint(uint id) {
	assert(SCENGINE);
	if (SCENGINE->_watches.size() > id) {
		SCENGINE->_watches[id]->disable();
		return Error(SUCCESS, OK);
	} else {
		return Error(ERROR, NO_SUCH_BREAKPOINT, id);
	}
}

Error DebuggerController::enableWatchpoint(uint id) {
	assert(SCENGINE);
	if (SCENGINE->_watches.size() > id) {
		SCENGINE->_watches[id]->enable();
		return Error(SUCCESS, OK);
	} else {
		return Error(ERROR, NO_SUCH_BREAKPOINT, id);
	}

}

Error DebuggerController::addWatch(const char *filename, const char *symbol) {
	assert(SCENGINE);
	if (!bytecodeExists(filename)) {
		return Error(ERROR, NO_SUCH_BYTECODE, filename);
	}
	SCENGINE->_watches.push_back(new Watch(filename, symbol, this));
	return Error(SUCCESS, OK, "Watchpoint added");
}

void DebuggerController::onBreakpoint(const Breakpoint *breakpoint, DebuggableScript *script) {
	_lastScript = script;
	_lastLine = script->_currentLine;
	DEBUGGER->notifyBreakpoint(script->dbgGetFilename().c_str(), script->_currentLine);
}

void DebuggerController::notifyStep(DebuggableScript *script) {
	_lastScript = script;
	_lastLine = script->_currentLine;
	DEBUGGER->notifyStep(script->dbgGetFilename().c_str(), script->_currentLine);
}

void DebuggerController::onWatch(const Watch *watch, DebuggableScript *script) {
	_lastScript = script; // If script has changed do we still care?
	_lastLine = script->_currentLine;
	Common::String symbol = watch->getSymbol();
	DEBUGGER->notifyWatch(script->dbgGetFilename().c_str(), symbol.c_str(), script->resolveName(symbol)->getString());
}

Error DebuggerController::step() {
	if (!_lastScript) {
		return Error(ERROR, NOT_ALLOWED);
	}
	_lastScript->step();
	clear();
	return Error(SUCCESS, OK);
}

Error DebuggerController::stepContinue() {
	if (!_lastScript) {
		return Error(ERROR, NOT_ALLOWED);
	}
	_lastScript->stepContinue();
	return Error(SUCCESS, OK);
}

Error DebuggerController::stepFinish() {
	if (!_lastScript) {
		return Error(ERROR, NOT_ALLOWED);
	}
	_lastScript->stepFinish();
	clear();
	return Error(SUCCESS, OK);
}

void DebuggerController::clear() {
	_lastScript = nullptr;
	_lastLine = 0xFFFFFFFF; // Invalid
}

Common::String DebuggerController::readValue(const Common::String &name, Error *error) {
	if (!_lastScript) {
		delete error;
		error = new Error(ERROR, NOT_ALLOWED);
		return Common::String();
	}
	char cstr[256]; // TODO not pretty
	Common::strlcpy(cstr, name.c_str(), name.size() + 1);
	cstr[255] = '\0'; // We 0-terminate it just in case it's longer than 255.
	return _lastScript->resolveName(cstr)->getString();
}

Error DebuggerController::setValue(const Common::String &name, const Common::String &value, ScValue *&var) {
	if (!_lastScript) {
		return Error(ERROR, NOT_ALLOWED);
	}

	Common::String trimmed = value;
	trimmed.trim();
	char cstr[256];
	Common::strlcpy(cstr, name.c_str(), name.size() + 1); // TODO not pretty

	var = _lastScript->getVar(cstr);
	if (var->_type == VAL_INT) {
		char *endptr;
		int res = strtol(trimmed.c_str(), &endptr, 10); // TODO: Hex too?
		if (endptr == trimmed.c_str()) {
			return Error(ERROR, PARSE_ERROR);
		} else if (endptr == trimmed.c_str() + trimmed.size()) {
			// We've parsed all of it, have we?
			var->setInt(res);
		} else {
			assert(false);
			return Error(ERROR, PARSE_ERROR);
			// Something funny happened here.
		}
	} else if (var->_type == VAL_FLOAT) {
		char *endptr;
		float res = (float)strtod(trimmed.c_str(), &endptr);
		if (endptr == trimmed.c_str()) {
			return Error(ERROR, PARSE_ERROR);
		} else if (endptr == trimmed.c_str() + trimmed.size()) {
			// We've parsed all of it, have we?
			var->setFloat(res);
		} else {
			return Error(ERROR, PARSE_ERROR);
			assert(false);
			// Something funny happened here.
		}
	} else if (var->_type == VAL_BOOL) {
		//Common::String str = Common::String(trimmed);
		bool valAsBool;
		if (Common::parseBool(trimmed, valAsBool)) {
			var->setBool(valAsBool);
		} else {
			return Error(ERROR, PARSE_ERROR);
		}
	} else if (var->_type == VAL_STRING) {
		var->setString(trimmed);
	} else {
		return Error(ERROR, NOT_YET_IMPLEMENTED);
	}
	return Error(SUCCESS, OK);
}

void DebuggerController::showFps(bool show) {
	_engine->_game->setShowFPS(show);
}

Common::Array<BreakpointInfo> DebuggerController::getBreakpoints() const {
	assert(SCENGINE);
	Common::Array<BreakpointInfo> breakpoints;
	for (uint i = 0; i < SCENGINE->_breakpoints.size(); i++) {
		BreakpointInfo bpInfo;
		bpInfo._filename = SCENGINE->_breakpoints[i]->getFilename();
		bpInfo._line = SCENGINE->_breakpoints[i]->getLine();
		bpInfo._hits = SCENGINE->_breakpoints[i]->getHits();
		bpInfo._enabled = SCENGINE->_breakpoints[i]->isEnabled();
		breakpoints.push_back(bpInfo);
	}
	return breakpoints;
}

Common::Array<WatchInfo> DebuggerController::getWatchlist() const {
	Common::Array<WatchInfo> watchlist;
	for (uint i = 0; i < SCENGINE->_watches.size(); i++) {
		WatchInfo watchInfo;
		watchInfo._filename = SCENGINE->_watches[i]->getFilename();
		watchInfo._symbol = SCENGINE->_watches[i]->getSymbol();
		watchlist.push_back(watchInfo);
	}
	return watchlist;
}

uint32 DebuggerController::getLastLine() const {
	return _lastLine;
}

Common::String DebuggerController::getSourcePath() const {
	return _sourceListingProvider->getPath();
}

Error DebuggerController::setSourcePath(const Common::String &sourcePath) {
	ErrorCode err = _sourceListingProvider->setPath(sourcePath);
	return Error((err == OK ? SUCCESS : ERROR), err);
}

Listing* DebuggerController::getListing(Error* &error) {
	delete (error);
	if (_lastScript == nullptr) {
		error = new Error(ERROR, NOT_ALLOWED);
		return nullptr;
	}
	ErrorCode err;
	Listing* res = _sourceListingProvider->getListing(SCENGINE->_currentScript->_filename, err);
	error = new Error(err == OK ? SUCCESS : ERROR, err);
	return res;
}

Common::Array<TopEntry> DebuggerController::getTop() const {
	Common::Array<TopEntry> res;
	assert(SCENGINE);
	for (uint i = 0; i < SCENGINE->_scripts.size(); i++) {
		TopEntry entry;
		entry.filename = SCENGINE->_scripts[i]->_filename;
		entry.current = (SCENGINE->_scripts[i] == SCENGINE->_currentScript);
		res.push_back(entry);
	}
	return res;
}

} // end of namespace Wintermute
