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
#include "engines/wintermute/debugger/listing_providers/blank_listing_provider.h"
#define SCENGINE _engine->_game->_scEngine
#define DEBUGGER _engine->_debugger

namespace Wintermute {

DebuggerController::~DebuggerController() {
	delete _listingProvider;
}

DebuggerController::DebuggerController(WintermuteEngine *vm) : _engine(vm) {
	_listingProvider = new BlankListingProvider();
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

void DebuggerController::onBreakpoint(const Breakpoint *breakpoint, DebuggableScript *script) {
	_lastScript = script;
	_lastLine = script->_currentLine;
	DEBUGGER->notifyBreakpoint(script->dbgGetFilename().c_str(), script->_currentLine);
}

void DebuggerController::notifyStep(DebuggableScript *script) override {
	_lastScript = script;
	_lastLine = script->_currentLine;
	DEBUGGER->notifyStep(script->dbgGetFilename().c_str(), script->_currentLine);
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
	_lastLine = -1;
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

uint32 DebuggerController::getLastLine() const {
	return _lastLine;
}

Listing* DebuggerController::getListing(Error* &error) {
	delete (error);
	if (_lastScript == nullptr) {
		error = new Error(ERROR, NOT_ALLOWED);
		return nullptr;
	}
	ErrorCode err;
	Listing* res = _listingProvider->getListing(SCENGINE->_currentScript->_filename, err);
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
