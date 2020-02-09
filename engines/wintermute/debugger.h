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

#ifndef WINTERMUTE_DEBUGGER_H
#define WINTERMUTE_DEBUGGER_H

#define EXTENDED_DEBUGGER_ENABLED 1

#include "gui/debugger.h"

#if EXTENDED_DEBUGGER_ENABLED
#include "engines/wintermute/base/scriptables/debuggable/debuggable_script.h"
#else
#include "engines/wintermute/base/scriptables/script.h"
#endif

#define DEFAULT_SOURCE_PADDING 5


#define STEP_CMD "step"
#define CONTINUE_CMD "continue"
#define FINISH_CMD "finish"
#define WATCH_CMD "watch"
#define BREAK_CMD "break"
#define LIST_CMD "list"
#define REMOVE_BREAKPOINT_CMD "del"
#define DISABLE_BREAKPOINT_CMD "disable"
#define ENABLE_BREAKPOINT_CMD "enable"
#define REMOVE_WATCH_CMD "delw"
#define DISABLE_WATCH_CMD "disablew"
#define ENABLE_WATCH_CMD "enablew"
#define INFO_CMD "info"
#define SET_CMD "set"
#define PRINT_CMD "print"
#define SET_PATH_CMD "set_path"
#define TOP_CMD "top"

namespace Wintermute {
class WintermuteEngine;
class Adapter;
class DebuggerController;
class Error;

class Console : public GUI::Debugger {
public:
	Console(WintermuteEngine *vm);
	~Console() override;
	/*
	 * Debug commands
	 */
	bool Cmd_Help(int argc, const char **argv);
	bool Cmd_ShowFps(int argc, const char **argv);
	bool Cmd_DumpFile(int argc, const char **argv);

#if EXTENDED_DEBUGGER_ENABLED
	/**
	 * Step - break again on next line
	 */
	bool Cmd_Step(int argc, const char **argv);
	/**
	 * Continue execution
	 */
	bool Cmd_Continue(int argc, const char **argv);
	/**
	 * Only break again when the current function is finished
	 * (activation record is popped)
	 */
	bool Cmd_Finish(int argc, const char **argv);
	bool Cmd_Print(int argc, const char **argv);
	bool Cmd_Set(int argc, const char **argv);
	// Breakpoints
	bool Cmd_AddBreakpoint(int argc, const char **argv);
	bool Cmd_RemoveBreakpoint(int argc, const char **argv);
	bool Cmd_EnableBreakpoint(int argc, const char **argv);
	bool Cmd_DisableBreakpoint(int argc, const char **argv);
	/**
	 * Add a watch.
	 *
	 * It monitors the value of some variable x against its
	 * last known state and it breaks if it has changed since.
	 *
	 */
	bool Cmd_Watch(int argc, const char **argv);
	bool Cmd_RemoveWatch(int argc, const char **argv);
	bool Cmd_EnableWatch(int argc, const char **argv);
	bool Cmd_DisableWatch(int argc, const char **argv);
	/**
	 * Print info re:watch and breakpoints.
	 * This differs from e.g. gdb in that we have separate lists.
	 */
	bool Cmd_Info(int argc, const char **argv);
	/**
	 * Print source
	 */
	bool Cmd_List(int argc, const char **argv);
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

	/**
	 * Top
	 */
	bool Cmd_Top(int argc, const char **argv);

	Error printSource(int n = DEFAULT_SOURCE_PADDING);

	/**
	 * Hooks for the controller to open the console
	 */
	void notifyBreakpoint(const char *filename, int line);
	void notifyStep(const char *filename, int line);
	/**
	 * To be called by the adapter when a watched variable
	 * is changed.
	 * Opens a console and prints info and listing if available.
	 */
	void notifyWatch(const char *filename, const char *symbol, const char *newValue);
#endif

private:
	const WintermuteEngine *_engineRef;
	void printError(const Common::String &command, Error error);
	void printUsage(const Common::String &command);
};

}

#endif // WINTERMUTE_DEBUGGER_H
