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

#ifndef PRINCE_DEBUGGER_H
#define PRINCE_DEBUGGER_H

#include "common/scummsys.h"
#include "gui/debugger.h"

namespace Prince {

class PrinceEngine;
class InterpreterFlags;

class Debugger : public GUI::Debugger {
public:
	Debugger(PrinceEngine *vm, InterpreterFlags *flags);
	~Debugger() override {}	// we need this for __SYMBIAN32__ archaic gcc/UIQ

	uint8 _locationNr;
	uint8 _cursorNr;

private:
	bool Cmd_DebugLevel(int argc, const char **argv);
	bool Cmd_SetFlag(int argc, const char **argv);
	bool Cmd_GetFlag(int argc, const char **argv);
	bool Cmd_ClearFlag(int argc, const char **argv);
	bool Cmd_ViewFlc(int argc, const char **argv);
	bool Cmd_InitRoom(int argc, const char **argv);
	bool Cmd_ChangeCursor(int argc, const char **argv);
	bool Cmd_AddItem(int argc, const char **argv);

	PrinceEngine *_vm;
	InterpreterFlags *_flags;
};

} // End of namespace Prince

#endif
