/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SIMON_DEBUGGER_H
#define SIMON_DEBUGGER_H

#include "common/debugger.h"

namespace Simon {

class SimonEngine;

class Debugger : public Common::Debugger<Debugger> {
public:
	Debugger(SimonEngine *vm);

protected:
	SimonEngine *_vm;

	virtual void preEnter();
	virtual void postEnter();

	bool Cmd_Exit(int argc, const char **argv);
	bool Cmd_Help(int argc, const char **argv);
	bool Cmd_PlayMusic(int argc, const char **argv);
	bool Cmd_PlaySound(int argc, const char **argv);
	bool Cmd_PlayVoice(int argc, const char **argv);
	bool Cmd_SetBit(int argc, const char **argv);
	bool Cmd_SetVar(int argc, const char **argv);
	bool Cmd_StartSubroutine(int argc, const char **argv);
};

} // End of namespace Simon

#endif
