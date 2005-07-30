/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "common/debugger.h"

namespace Scumm {

class ScummEngine;

class ScummDebugger : public Common::Debugger<ScummDebugger> {
public:
	ScummDebugger(ScummEngine *s);
	virtual ~ScummDebugger(); // we need this here for __SYMBIAN32__

protected:
	ScummEngine *_vm;
	bool  _old_soundsPaused;

	virtual void preEnter();
	virtual void postEnter();

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
	bool Cmd_Camera(int argc, const char **argv);
	bool Cmd_Object(int argc, const char **argv);
	bool Cmd_Script(int argc, const char **argv);
	bool Cmd_PrintScript(int argc, const char **argv);
	bool Cmd_ImportRes(int argc, const char **argv);

	bool Cmd_PrintDraft(int argc, const char **argv);

	bool Cmd_Debug(int argc, const char **argv);
	bool Cmd_DebugLevel(int argc, const char **argv);
	bool Cmd_Help(int argc, const char **argv);

	bool Cmd_Show(int argc, const char **argv);
	bool Cmd_Hide(int argc, const char **argv);

	bool Cmd_IMuse (int argc, const char **argv);

	void printBox(int box);
	void drawBox(int box);
};

} // End of namespace Scumm

#endif
