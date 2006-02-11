/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
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

#ifndef QUEENDEBUG_H
#define QUEENDEBUG_H

#include "common/debugger.h"

namespace Queen {

class QueenEngine;

class Debugger : public Common::Debugger<Debugger> {
public:

	Debugger(QueenEngine *vm);
	virtual ~Debugger(); // we need this here for __SYMBIAN32__ archaic gcc/UIQ

	int flags() const { return _flags; }

	enum {
		DF_DRAW_AREAS = 1 << 0
	};

protected:

	virtual void preEnter();
	virtual void postEnter();

	bool Cmd_Exit(int argc, const char **argv);
	bool Cmd_Help(int argc, const char **argv);
	bool Cmd_Areas(int argc, const char **argv);
	bool Cmd_Asm(int argc, const char **argv);
	bool Cmd_Bob(int argc, const char **argv);
	bool Cmd_GameState(int argc, const char **argv);
	bool Cmd_Info(int argc, const char **argv);
	bool Cmd_Items(int argc, const char **argv);
	bool Cmd_PrintBobs(int argc, const char **argv);
	bool Cmd_Room(int argc, const char **argv);
	bool Cmd_Song(int argc, const char **argv);

private:

	QueenEngine *_vm;
	int _flags;
};

} // End of namespace Queen

#endif
