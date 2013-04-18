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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "hopkins/debugger.h"

#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

Debugger::Debugger(HopkinsEngine *vm) : GUI::Debugger() {
	_vm = vm;
	DCmd_Register("continue", WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("rects", WRAP_METHOD(Debugger, cmd_DirtyRects));
	DCmd_Register("teleport", WRAP_METHOD(Debugger, cmd_Teleport));
	DCmd_Register("show_room", WRAP_METHOD(Debugger, cmd_ShowCurrentRoom));
}

// Turns dirty rects on or off
bool Debugger::cmd_DirtyRects(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("%s: [on | off]\n", argv[0]);
		return true;
	} else {
		_vm->_graphicsMan->_showDirtyRects = !strcmp(argv[1], "on");
		return false;
	}
}

// Change room number
bool Debugger::cmd_Teleport(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("%s: [Room number]\n", argv[0]);
		return true;
	} else {
		_vm->_globals->_exitId = atoi(argv[1]);
		return false;
	}
}

// Display room number
bool Debugger::cmd_ShowCurrentRoom(int argc, const char **argv) {
	DebugPrintf("Current room: %d\n", _vm->_globals->_curRoomNum);
	return true;
}

} // End of namespace Hopkins
