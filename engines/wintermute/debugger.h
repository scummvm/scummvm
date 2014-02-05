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
	/*
	 * Step Into - break when you reach the next line in
	 * order of execution (as in II_DBG_LINE) IN the
	 * current block.
	 */
	bool Cmd_StepInto(int argc, const char **argv);
	/*
	 * Step Into - break when you reach the next line in
	 * order of execution, even if it belongs to another script.
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
	bool Cmd_Print(int argc, const char **argv);
	bool Cmd_Set(int argc, const char **argv);
	bool Cmd_SetType(int argc, const char **argv);
	/**
	 * Add a breakpoint.
	 *
	 * Take note that this works in a slightly different fashion
	 * than your usual debugger. We have hooks that allow us to break
	 * when a special instruction in the WME bytecode is executed,
	 * called II_DBG_LINE, that increments a special register in
	 * the execution unit.
	 * That is all we know, along with call depth (in another
	 * register).
	 */
	bool Cmd_AddBreakpoint(int argc, const char **argv);
	bool Cmd_RemoveBreakpoint(int argc, const char **argv);
	bool Cmd_EnableBreakpoint(int argc, const char **argv);
	bool Cmd_DisableBreakpoint(int argc, const char **argv);
	/**
	 * Add a watch.
	 *
	 * The big, fat disclaimer: this works in a slightly
	 * different fashion than your usual gdb-style debugger.
	 *
	 * It monitors the value of some variable x against its
	 * last known state the last time the execution unit
	 * went over this specific script (NOT instance - ANY
	 * instance of the script will do) and it breaks if it
	 * has changed since.
	 *
	 * It is admittedly not awesome and can lead to false
	 * positives, but it's better than nothing.
	 *
	 */
	bool Cmd_Watch(int argc, const char **argv);
	bool Cmd_RemoveWatchpoint(int argc, const char **argv);
	bool Cmd_EnableWatchpoint(int argc, const char **argv);
	bool Cmd_DisableWatchpoint(int argc, const char **argv);
	/**
	 * Print info re:watch and breakpoints.
	 * This differs from gdb in that we have separate lists.
	 */
	bool Cmd_Info(int argc, const char **argv);
	/* 
	 * Print source
	 */
	bool Cmd_List(int argc, const char **argv);
	/**
	 * Dumps info about a specific in-engine resource
	 * referenced by a script variable.
	 * The info is obtained (and formatted)
	 * via the debuggerToString() method.
	 *
	 * You can write your own if you want to dump
	 * something in particular.
	 */
	bool Cmd_DumpRes(int argc, const char **argv);
	/**
	 * Set (DOS-style) source path for debugging.
	 * This is where you will (optionally) put your sources
	 * to enable printing of sources as you step through the
	 * scripts.
	 *
	 * Please note that we have no checksum or anything
	 * to make sure your source files are up to date.
	 *
	 * YOU HAVE to make sure of that.
	 *
	 * You have been warned! :)
	 */
	bool Cmd_SourcePath(int argc, const char **argv);

	Error printSource(int n = DEFAULT_SOURCE_PADDING);

	// Hooks for the Adapter
	/**
	 * To be called by the adapter when a breakpoint
	 * is hit.
	 * Opens a console and prints info and listing if available.
	 */
	void notifyBreakpoint(const char *filename, int line);
	/**
	 * To be called by the adapter when advancing a step.
	 * Opens a console and prints info and listing if available.
	 */
	void notifyStep(const char *filename, int line);
	/**
	 * To be called by the adapter when a watched variable
	 * is changed.
	 * Opens a console and prints info and listing if available.
	 */
	void notifyWatch(const char *filename, const char *symbol, const char *newValue);

private:
	WintermuteEngine *_engineRef;
	void printError(const Common::String &command, Error error);
	void printUsage(const Common::String &command);
};


}

#endif // WINTERMUTE_DEBUGGER_H
