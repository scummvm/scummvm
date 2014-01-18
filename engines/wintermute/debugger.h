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

#ifndef WINTERMUTE_DEBUGGER_H
#define WINTERMUTE_DEBUGGER_H

#include "gui/debugger.h"
#include "engines/wintermute/debugger_adapter.h"
#include "engines/wintermute/base/scriptables/script.h"
#define DEFAULT_SOURCE_PADDING 5

#define STEP_INTO_CMD "step"
#define STEP_OVER_CMD "next"
#define CONTINUE_CMD "next"
#define FINISH_CMD "finish"
#define WATCH_CMD "watch"
#define BREAK_CMD "break"
#define LIST_CMD "list"
#define REMOVE_BREAKPOINT_CMD "del"
#define DISABLE_BREAKPOINT_CMD "disable"
#define ENABLE_BREAKPOINT_CMD "enable"
#define REMOVE_WATCHPOINT_CMD "delw"
#define DISABLE_WATCHPOINT_CMD "disablew"
#define ENABLE_WATCHPOINT_CMD "enablew"
#define INFO_CMD "info"
#define SET_CMD "set"
#define SET_TYPE_CMD "set_type"
#define PRINT_CMD "print"
#define DUMP_RES_CMD "dump_res"
#define SET_PATH_CMD "set_path"

namespace Wintermute {
class WintermuteEngine;
class Adapter;

class Console : public GUI::Debugger {
public:
	Console(WintermuteEngine *vm);
	virtual ~Console();

	/*
	 * Debug commands 
	 */
	bool Cmd_Help(int argc, const char **argv);
	bool Cmd_ShowFps(int argc, const char **argv);
	bool Cmd_DumpFile(int argc, const char **argv);
	bool Cmd_StepInto(int argc, const char **argv);
	/*
	 * gdb-style next
	 */
	bool Cmd_StepOver(int argc, const char **argv);
	/* 
	 * Continue execution
	 */
	bool Cmd_Continue(int argc, const char **argv);
	/*
	 * Don't further break inside this block
	 */
	bool Cmd_Finish(int argc, const char **argv);
	bool Cmd_Watch(int argc, const char **argv);
	bool Cmd_Print(int argc, const char **argv);
	bool Cmd_Set(int argc, const char **argv);
	bool Cmd_SetType(int argc, const char **argv);
	/**
	 * Add a breakpoint.
	 */
	bool Cmd_AddBreakpoint(int argc, const char **argv);
	bool Cmd_RemoveBreakpoint(int argc, const char **argv);
	bool Cmd_EnableBreakpoint(int argc, const char **argv);
	bool Cmd_DisableBreakpoint(int argc, const char **argv);
	bool Cmd_RemoveWatchpoint(int argc, const char **argv);
	bool Cmd_EnableWatchpoint(int argc, const char **argv);
	bool Cmd_DisableWatchpoint(int argc, const char **argv);
	/*
	 * List watch-breakpoints gdb-style
	 */
	bool Cmd_Info(int argc, const char **argv);
	/* 
	 * Print source
	 */
	bool Cmd_List(int argc, const char **argv);
	bool Cmd_DumpRes(int argc, const char **argv);
	/**
	 * Set source path in the Adapter
	 */
	bool Cmd_SourcePath(int argc, const char **argv);

	Error printSource(int n = DEFAULT_SOURCE_PADDING);
	// For use by the Adapter
	void notifyBreakpoint(const char *filename, int line);
	void notifyStep(const char *filename, int line);
	void notifyWatch(const char *filename, const char *symbol, const char *newValue);

private:
	WintermuteEngine *_engineRef;
	void printError(const Common::String &command, Error error);
	void printUsage(const Common::String &command);
};


}

#endif // WINTERMUTE_DEBUGGER_H
