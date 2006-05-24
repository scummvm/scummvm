/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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
 * $URL$
 * $Id$
 *
 */

#ifndef AGI_CONSOLE_H
#define AGI_CONSOLE_H

#include "common/debugger.h"

namespace Agi {

class AgiEngine;

struct agi_debug {
	int enabled;
	int opcodes;
	int logic0;
	int steps;
	int priority;
	int statusline;
	int ignoretriggers;
};

class Console : public Common::Debugger<Console> {
public:
	Console(AgiEngine *vm);
	virtual ~Console(void);

protected:
	virtual void preEnter();
	virtual void postEnter();

private:
	bool Cmd_Exit(int argc, const char **argv);
	bool Cmd_Help(int argc, const char **argv);
	bool Cmd_SetVar(int argc, const char **argv);
	bool Cmd_SetFlag(int argc, const char **argv);
	bool Cmd_SetObj(int argc, const char **argv);
	bool Cmd_RunOpcode(int argc, const char **argv);
	bool Cmd_Crc(int argc, const char **argv);
	bool Cmd_Agiver(int argc, const char **argv);
	bool Cmd_Flags(int argc, const char **argv);
	bool Cmd_Vars(int argc, const char **argv);
	bool Cmd_Objs(int argc, const char **argv);
	bool Cmd_Opcode(int argc, const char **argv);
	bool Cmd_Logic0(int argc, const char **argv);
	bool Cmd_Trigger(int argc, const char **argv);
	bool Cmd_Step(int argc, const char **argv);
	bool Cmd_Debug(int argc, const char **argv);
	bool Cmd_Cont(int argc, const char **argv);

private:
	AgiEngine *_vm;
};

}                             // End of namespace Agi

#endif				/* AGI_CONSOLE_H */
