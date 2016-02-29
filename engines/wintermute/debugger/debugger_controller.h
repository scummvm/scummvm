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

#ifndef WINTERMUTE_DEBUGGER_ADAPTER_H
#define WINTERMUTE_DEBUGGER_ADAPTER_H

#include "common/str.h"
#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/debugger/listing_providers/source_listing_provider.h"
#include "script_monitor.h"
#include "error.h"
#include "listing.h"
namespace Wintermute {

class ScScript;
class DebuggableScript;
class ScValue;

struct BreakpointInfo {
	Common::String _filename;
	int _line;
	int _hits;
	bool _enabled;
};

struct TopEntry {
	bool current;
	Common::String filename;
	int watches;
	int breakpointInfo;
};

class DebuggerController : public ScriptMonitor {
	ListingProvider *_listingProvider;
	const WintermuteEngine *_engine;
	DebuggableScript *_lastScript;
	uint32 _lastDepth;
	uint32 _lastLine;
	void clear();
	bool bytecodeExists(const Common::String &filename);
public:
	DebuggerController(WintermuteEngine *vm);
	~DebuggerController();
	Common::Array<TopEntry> getTop() const;
	/**
	 * Get the last line # we've stopped at
	 */
	uint32 getLastLine() const;
	Error addBreakpoint(const char *filename, int line);
	Error removeBreakpoint(uint id);
	Error disableBreakpoint(uint id);
	Error enableBreakpoint(uint id);
	Common::Array<BreakpointInfo> getBreakpoints() const;
	/**
	 * @brief step one instruction
	 */
	Error step();
	/**
	 * @brief continue execution and don't step until next breakpoint
	 */
	Error stepContinue();
	/**
	 * @brief continue execution and don't step until the current activation record is popped
	 */
	Error stepFinish();
	Listing *getListing(Error* &err);
	void showFps(bool show);
	/**
	 * Inherited from ScriptMonitor
	 */
	void onBreakpoint(const Breakpoint *breakpoint, DebuggableScript *script);
	void notifyStep(DebuggableScript *script);
};
}

#endif // WINTERMUTE_DEBUGGER_H
