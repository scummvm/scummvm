/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#ifndef COMMON_DEBUGGER_H
#define COMMON_DEBUGGER_H

namespace GUI {
	class ConsoleDialog;
}

namespace Common {

// Choose between text console or ScummConsole
#define USE_CONSOLE	1

template <class T>
class Debugger {
public:
	Debugger();
	virtual ~Debugger();

	int DebugPrintf(const char *format, ...);

#ifndef __SYMBIAN32__ // gcc/UIQ doesn't like the debugger code for some reason? Actually get a cc1plus core dump here :)
	virtual void onFrame();

	virtual void attach(const char *entry = 0);
#else
	void onFrame() {}
	void attach(const char *entry = 0) {}
#endif
	bool isAttached() const { return _isAttached; }

protected:
	typedef bool (T::*DebugProc)(int argc, const char **argv);

	enum {
		DVAR_BYTE,
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

	int _frame_countdown, _dvar_count, _dcmd_count;
	DVar _dvars[256];
	DCmd _dcmds[256];
	bool _detach_now;

private:
	bool _isAttached;
	char *_errStr;
	bool _firstTime;
	GUI::ConsoleDialog *_debuggerDialog;

protected:
	void detach();
	void enter();

	virtual void preEnter() = 0;
	virtual void postEnter() = 0;

	bool RunCommand(const char *input);
	bool TabComplete(const char *input, char*& completion);

	void DVar_Register(const char *varname, void *pointer, int type, int optional);
	void DCmd_Register(const char *cmdname, DebugProc pointer);

	bool Cmd_DebugFlagsList(int argc, const char **argv);
	bool Cmd_DebugFlagEnable(int argc, const char **argv);
	bool Cmd_DebugFlagDisable(int argc, const char **argv);

#if USE_CONSOLE
	static bool debuggerInputCallback(GUI::ConsoleDialog *console, const char *input, void *refCon);
	static bool debuggerCompletionCallback(GUI::ConsoleDialog *console, const char *input, char*& completion, void *refCon);
#endif
};

}	// End of namespace Common

#endif
