/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef DEBUG_H
#define DEBUG_H

namespace Scumm {

// Choose between text console or ScummConsole
#define USE_CONSOLE	1

class ScummEngine;
class ScummDebugger;

typedef bool (ScummDebugger::*DebugProc)(int argc, const char **argv);

enum {
	DVAR_INT,
	DVAR_BOOL,
	DVAR_INTARRAY,
	DVAR_STRING
};

struct DVar {
	char name[30];
	void *variable;
	int type, optional;
};

struct DCmd {
	char name[30];
	DebugProc function;
};

class ScummDebugger {
public:
	ScummDebugger(ScummEngine *s);
	
	void onFrame();
	void attach(const char *entry = 0);
	
	bool isAttached() const { return _isAttached; }

protected:
	ScummEngine *_vm;
	int _frame_countdown, _dvar_count, _dcmd_count;
	DVar _dvars[256];
	DCmd _dcmds[256];
	bool _detach_now;
	bool _isAttached;
	char *_errStr;

	void enter();
	void detach();

	void DVar_Register(const char *varname, void *pointer, int type, int optional);
	void DCmd_Register(const char *cmdname, DebugProc pointer);
	bool RunCommand(const char *input);

	// Commands
	bool Cmd_Exit(int argc, const char **argv);
	bool Cmd_Room(int argc, const char **argv);
	bool Cmd_LoadGame(int argc, const char **argv);
	bool Cmd_SaveGame(int argc, const char **argv);
	bool Cmd_Restart(int argc, const char **argv);

	bool Cmd_PrintActor(int argc, const char **argv);
	bool Cmd_PrintBox(int argc, const char **argv);
	bool Cmd_PrintBoxMatrix(int argc, const char **argv);
	bool Cmd_PrintObjects(int argc, const char **argv);
	bool Cmd_Actor(int argc, const char **argv);
	bool Cmd_Object(int argc, const char **argv);
	bool Cmd_Script(int argc, const char **argv);
	bool Cmd_PrintScript(int argc, const char **argv);
	bool Cmd_ImportRes(int argc, const char **argv);

	bool Cmd_PrintDraft(int argc, const char **argv);

	bool Cmd_DebugLevel(int argc, const char **argv);
	bool Cmd_Help(int argc, const char **argv);

	bool Cmd_Show(int argc, const char **argv);
	bool Cmd_Hide(int argc, const char **argv);

	bool Cmd_IMuse (int argc, const char **argv);
	
	void printBox(int box);
	void drawBox(int box);

#ifdef USE_CONSOLE
	static bool debuggerInputCallback(ConsoleDialog *console, const char *input, void *refCon);
	static bool debuggerCompletionCallback(ConsoleDialog *console, const char *input, char*& completion, void *refCon);
#endif

	bool TabComplete(const char *input, char*& completion);

};

} // End of namespace Scumm

#endif
