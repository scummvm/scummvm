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

#include "hopkins/debugger.h"

#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

Debugger::Debugger(HopkinsEngine *vm) : GUI::Debugger() {
	_vm = vm;
	registerCmd("continue", WRAP_METHOD(Debugger, cmdExit));
	registerCmd("rects", WRAP_METHOD(Debugger, cmd_DirtyRects));
	registerCmd("teleport", WRAP_METHOD(Debugger, cmd_Teleport));
	registerCmd("show_room", WRAP_METHOD(Debugger, cmd_ShowCurrentRoom));
	registerCmd("zones", WRAP_METHOD(Debugger, cmd_Zones));
	registerCmd("lines", WRAP_METHOD(Debugger, cmd_Lines));
}

// Turns dirty rects on or off
bool Debugger::cmd_DirtyRects(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("%s: [on | off]\n", argv[0]);
		return true;
	} else {
		_vm->_graphicsMan->_showDirtyRects = !strcmp(argv[1], "on");
		return false;
	}
}

// Change room number
bool Debugger::cmd_Teleport(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("%s: [Room number]\n", argv[0]);
		return true;
	} else {
		_vm->_globals->_exitId = atoi(argv[1]);
		return false;
	}
}

// Display room number
bool Debugger::cmd_ShowCurrentRoom(int argc, const char **argv) {
	debugPrintf("Current room: %d\n", _vm->_globals->_curRoomNum);
	return true;
}

bool Debugger::cmd_Zones(int argc, const char **argv) {
if (argc != 2) {
		debugPrintf("%s: [on | off]\n", argv[0]);
		return true;
	} else {
		_vm->_graphicsMan->_showZones = !strcmp(argv[1], "on");
		return false;
	}
}

bool Debugger::cmd_Lines(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("%s: [on | off]\n", argv[0]);
		return true;
	} else {
		_vm->_graphicsMan->_showLines = !strcmp(argv[1], "on");
		return false;
	}
}


} // End of namespace Hopkins
