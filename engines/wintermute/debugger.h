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
#include "engines/wintermute/base/scriptables/script.h"

namespace Wintermute {
class WintermuteEngine;
class Adapter;
class Console : public GUI::Debugger {
public:
	Console(WintermuteEngine *vm);
	virtual ~Console();
	bool Cmd_ShowFps(int argc, const char **argv);
	bool Cmd_DumpFile(int argc, const char **argv);
	bool Cmd_Step(int argc, const char **argv);
	bool Cmd_StepInto(int argc, const char **argv);
	bool Cmd_StepOver(int argc, const char **argv);
	bool Cmd_Continue(int argc, const char **argv);
	bool Cmd_Watch(int argc, const char **argv);
	/**
	 * Add a breakpoint.
	 */
	bool Cmd_AddBreakpoint(int argc, const char **argv);
	bool Cmd_RemoveBreakpoint(int argc, const char **argv);
	/**
	 * List all scripts running ATM.
	 * Also, which line are they on and if they have any breakpoints.
	 */
	bool Cmd_Top(int argc, const char **argv);
	bool Cmd_Info(int argc, const char **argv);
	bool Cmd_List(int argc, const char **argv);
	// For use by the Adapter
	void notifyBreakpoint(const char *filename, int line);
	void notifyStep(const char *filename, int line);
	void notifyWatch(const char *filename, const char *symbol, const char *newValue);

private:
	WintermuteEngine *_engineRef;
};


}

#endif // WINTERMUTE_DEBUGGER_H
