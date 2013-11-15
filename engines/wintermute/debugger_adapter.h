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

class SourceFile {
private:
	Common::Array<Common::String> _strings;
	bool _err;
public:
	enum ErrorCode {
		OK,
		NO_SUCH_SOURCE,
		COULD_NOT_OPEN,
		NO_SUCH_LINE,
		NOT_ALLOWED,
		IS_BLANK, // Is blank or comment-only, actually
		NO_SUCH_SCRIPT
	};
	SourceFile(const Common::String &filename);

	/**
	 * @brief (attempt to) load a source file.
	 * @param filename the WME path (with DOS-style slashes, starting from the DCP root) of the source, e.g. scripts\myscript.script.
	 * @param error will contain an error code in case of failure
	 */
	bool loadFile(Common::String filename, int *error = nullptr);
	/**
	 * @brief get the source file length (in lines)
	 */
	int getLength();
	/**
	 * @brief see if a given line is blank or comment-only
	 */
	bool isBlank(int line);
	/**
	 * @brief shorthand to get a lump of lines instead of calling getLine a number of times
	 */
	Common::Array<Common::String> getSurroundingLines(int centre, int lines, int *error = nullptr);
	Common::Array<Common::String> getSurroundingLines(int cemtre, int before, int after, int *error = nullptr);
	/**
	 * @brief return a specific line from the already-loaded source file
	 * @param n line number
	 * @param error in case of failure this will contain an error code 
	 */
	Common::String getLine(uint n, int *error = nullptr);
};

struct BreakpointInfo {
	// Slightly non-standard. If in doubt make sure you have the way II_DBG_LINE works figured out.
	Common::String _filename;
	int _line;
	int _hits; // How many times has it been hit?
	bool _enabled;
};

struct WatchInfo {
	Common::String _filename;
	Common::String _symbol;
	int _hits;
	bool _enabled;
};

class DebuggerAdapter {

/** 
 * MVA Adapter to be placed between the script engine proper and the Debugger class,
 * hiding one from another.
 */
public:
	enum ErrorCode {
		OK,
		NO_SUCH_SOURCE,
		COULD_NOT_OPEN,
		NO_SUCH_LINE,
		NOT_ALLOWED,
		IS_BLANK, // Is blank or comment-only, actually
		NO_SUCH_SCRIPT,
		DUPLICATE_BREAKPOINT,
		NO_SUCH_BREAKPOINT,
		WRONG_TYPE,
		PARSE_ERROR,
		NOT_YET_IMPLEMENTED
	};

	DebuggerAdapter(WintermuteEngine *vm);
	~DebuggerAdapter();
	// Called by Script (=~Model)
	/**
	 * @brief To be called by the script engine when hitting a breakpoint.
	 */
	bool triggerBreakpoint(ScScript *script);
	/**
	 * @brief To be called by the script engine when stepping
	 */
	bool triggerStep(ScScript *script);
	/**
	 * @brief To be called by the script engine when hitting a watch event.
	 */
	bool triggerWatch(ScScript *script, const char *symbol);
	// Called by Console (~=View)
	int addWatch(const char *filename, const char *symbol);
	int addBreakpoint(const char *filename, int line);
	int isBreakpointLegal(const char *filename, int line);
	int removeWatchpoint(int id);
	int removeBreakpoint(int id);
	int disableBreakpoint(int id);
	int enableBreakpoint(int id);
	int disableWatchpoint(int id);
	int enableWatchpoint(int id);
	Common::Array<BreakpointInfo> getBreakpoints();
	Common::Array<WatchInfo> getWatchlist();
	/** 
	 * @brief step to the next line in the current block
	 */
	int stepOver();
	/**
	 * @brief step at the next line entering any function encountered
	 */
	int stepInto();
	/**
	 * @brief continue and don't step until next breakpoint
	 */
	int stepContinue();
	/**
	 * @brief finish current block, step to the next line of the caller
	 */
	int stepFinish();
	int32 getLastLine();
	/** 
	 * @brief read value for a variable accessible from within the current scope.
	 */
	Common::String readValue(const char *name, int *error);
	/** 
	 * @brief set value for a variable accessible from within the current scope.
	 */
	int setValue(Common::String name, Common::String value, ScValue * &var);
	int setType(const Common::String &name, Common::String &type);
	/** 
	 * @brief Dump *engine* debug info about a variable accessible from within the current scope.
	 * While readValue(somestring) will simply return its value (e.g. "foo") 
	 * this will give you the output of the relevant ScValue or Entity's debuggetToString() 
	 * method, which can include engine information inaccessible from the scripting environment.
	 */
	Common::String readRes(const Common::String &name, int *error);
	void showFps(bool show);
	SourceFile *_lastSource;
private:
	/** 
	 * @brief see if we have compiled bytecode for a given script
	 */
	bool compiledExists(Common::String filename);
	WintermuteEngine *_engine;
	int32 _lastDepth;
	ScScript *_lastScript;
	int32 _lastLine;
	void reset();
};
}

#endif // WINTERMUTE_DEBUGGER_H
