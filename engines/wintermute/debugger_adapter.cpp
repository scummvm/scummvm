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
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/scriptables/script.h"
#define SCENGINE _engine->_game->_scEngine
#define DEBUGGER _engine->_debugger

namespace Wintermute {

DebuggerAdapter::DebuggerAdapter(WintermuteEngine *vm) {
	_engine = vm;
	_lastScript = nullptr;
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
	DEBUGGER->notifyBreakpoint(script->dbgGetFilename(), script->_currentLine);
	return 1;
}

bool DebuggerAdapter::triggerStep(ScScript *script) {
	_lastDepth = script->getCallDepth();
	_lastScript = script;
	DEBUGGER->notifyStep(script->dbgGetFilename(), script->_currentLine);
	return 1;
}

bool DebuggerAdapter::triggerWatch(ScScript *script, const char *symbol) {
	_lastDepth = script->getCallDepth();
	_lastScript = script; // If script has changed do we still care?
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

} // end of namespace Wintermute
