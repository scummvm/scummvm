/* Copyright (C) 1994-2003 Revolution Software Ltd
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

#ifndef	C_ONSOLE_H
#define	C_ONSOLE_H

#include "common/debugger.h"
#include "bs2/memory.h"

#define Debug_Printf g_sword2->_debugger->DebugPrintf

namespace Sword2 {

extern bool grabbingSequences;
extern bool wantSfxDebug;	// sfx debug file enabled/disabled from console

class Sword2Engine;

class Debugger : public Common::Debugger<Debugger> {
public:
	Debugger(Sword2Engine *s);

protected:
	Sword2Engine *_vm;

	virtual void preEnter();
	virtual void postEnter();

	// Commands
	bool Cmd_Exit(int argc, const char **argv);
	bool Cmd_Help(int argc, const char **argv);
	bool Cmd_Mem(int argc, const char **argv);
	bool Cmd_Tony(int argc, const char **argv);
	bool Cmd_Res(int argc, const char **argv);
	bool Cmd_Starts(int argc, const char **argv);
	bool Cmd_Start(int argc, const char **argv);
	bool Cmd_Info(int argc, const char **argv);
	bool Cmd_WalkGrid(int argc, const char **argv);
	bool Cmd_Mouse(int argc, const char **argv);
	bool Cmd_Player(int argc, const char **argv);
	bool Cmd_ResLook(int argc, const char **argv);
	bool Cmd_CurrentInfo(int argc, const char **argv);
	bool Cmd_RunList(int argc, const char **argv);
	bool Cmd_Kill(int argc, const char **argv);
	bool Cmd_Nuke(int argc, const char **argv);
	bool Cmd_Var(int argc, const char **argv);
	bool Cmd_Rect(int argc, const char **argv);
	bool Cmd_Clear(int argc, const char **argv);
	bool Cmd_DebugOn(int argc, const char **argv);
	bool Cmd_DebugOff(int argc, const char **argv);
	bool Cmd_SaveRest(int argc, const char **argv);
	bool Cmd_ListSaveGames(int argc, const char **argv);
	bool Cmd_SaveGame(int argc, const char **argv);
	bool Cmd_RestoreGame(int argc, const char **argv);
	bool Cmd_BltFxOn(int argc, const char **argv);
	bool Cmd_BltFxOff(int argc, const char **argv);
	bool Cmd_TimeOn(int argc, const char **argv);
	bool Cmd_TimeOff(int argc, const char **argv);
	bool Cmd_Text(int argc, const char **argv);
	bool Cmd_ShowVar(int argc, const char **argv);
	bool Cmd_HideVar(int argc, const char **argv);
	bool Cmd_Version(int argc, const char **argv);
	bool Cmd_SoftHard(int argc, const char **argv);
	bool Cmd_AnimTest(int argc, const char **argv);
	bool Cmd_TextTest(int argc, const char **argv);
	bool Cmd_LineTest(int argc, const char **argv);
	bool Cmd_Grab(int argc, const char **argv);
	bool Cmd_Events(int argc, const char **argv);
	bool Cmd_Sfx(int argc, const char **argv);
	bool Cmd_English(int argc, const char **argv);
	bool Cmd_Finnish(int argc, const char **argv);
	bool Cmd_Polish(int argc, const char **argv);
};

} // End of namespace Sword2

#endif
