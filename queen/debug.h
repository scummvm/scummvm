/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#ifndef QUEENDEBUG_H
#define QUEENDEBUG_H

#include "common/debugger.h"

namespace Queen {

class QueenEngine;

class Debugger : public Common::Debugger<Debugger> {
public:

	Debugger(QueenEngine *vm);

	bool _drawAreas;

protected:

	virtual void preEnter();
	virtual void postEnter();

	bool Cmd_Exit(int argc, const char **argv);
	bool Cmd_Help(int argc, const char **argv);
	bool Cmd_Areas(int argc, const char **argv);
	bool Cmd_Asm(int argc, const char **argv);
	bool Cmd_GameState(int argc, const char **argv);
	bool Cmd_Info(int argc, const char **argv);
	bool Cmd_Items(int argc, const char **argv);
	bool Cmd_Room(int argc, const char **argv);
	bool Cmd_Song(int argc, const char **argv);
	
private:

	QueenEngine *_vm;
};

} // End of namespace Queen

#endif
