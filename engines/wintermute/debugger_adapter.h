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


namespace Wintermute {
class ScScript;
class ScValue;

enum ErrorCode {
	OK,
	NO_SUCH_SOURCE,
	COULD_NOT_OPEN,
	NO_SUCH_LINE,
	DUPLICATE_BREAKPOINT,
	NO_SUCH_BREAKPOINT,
	NOT_ALLOWED,
	IS_BLANK, // Is blank or comment-only, actually
	NO_SUCH_SCRIPT,
	WRONG_TYPE,
	PARSE_ERROR,
	NOT_YET_IMPLEMENTED
};

class SourceFile {
private:
	Common::Array<Common::String> _strings;
	bool _err;
public:
	SourceFile(const Common::String &filename);
	bool loadFile(Common::String filename, int *error = nullptr);
	int getLength();
	bool isBlank(int line);
	Common::Array<Common::String> getSurroundingLines(int centre, int lines, int *error = nullptr);
	Common::Array<Common::String> getSurroundingLines(int cemtre, int before, int after, int *error = nullptr);
	Common::String getLine(uint n, int *error = nullptr);
};

struct BreakpointInfo {
	Common::String _filename;
	int _line;
	int _hits;
	bool _enabled;
};

struct WatchInfo {
	Common::String _filename;
	Common::String _symbol;
	int _hits;
	bool _enabled;
};

class DebuggerAdapter {
	/* Faux MVA Adapter */
public:
	DebuggerAdapter(WintermuteEngine *vm);
	~DebuggerAdapter();
	// Called by Script (=~Model)
	bool triggerBreakpoint(ScScript *script);
	bool triggerStep(ScScript *script);
	bool triggerWatch(ScScript *script, const char *symbol);
	// Called by Console (~=View)
	int addWatch(const char *filename, const char *symbol);
	int addBreakpoint(const char *filename, int line);
	int isBreakpointLegal(const char *filename, int line);
	int removeWatch(int id);
	int removeBreakpoint(int id);
	int disableBreakpoint(int id);
	int enableBreakpoint(int id);
	Common::Array<BreakpointInfo> getBreakpoints();
	Common::Array<WatchInfo> getWatchlist();
	int stepOver();
	int stepInto();
	int stepContinue();
	int stepFinish();
	int32 getLastLine();
	Common::String readValue(const char *name, int *error);
	int setValue(Common::String name, Common::String value, ScValue * &var);
	int setType(const Common::String &name, Common::String &type);
	Common::String readRes(const Common::String &name, int *error);
	void showFps(bool show);
	SourceFile *_lastSource;
private:
	bool compiledExists(Common::String filename);
	WintermuteEngine *_engine;
	int32 _lastDepth;
	ScScript *_lastScript;
	int32 _lastLine;
	void reset();
};
}

#endif // WINTERMUTE_DEBUGGER_H
