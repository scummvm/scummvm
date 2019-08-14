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

#include "startrek/console.h"
#include "gui/debugger.h"
#include "startrek/startrek.h"

namespace StarTrek {

Console::Console(StarTrekEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("room",			WRAP_METHOD(Console, Cmd_Room));
}

Console::~Console() {
}

bool Console::Cmd_Room(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Current room: %s%d\n", _vm->_missionToLoad.c_str(), _vm->_roomIndexToLoad);
		debugPrintf("Use room <mission> <room> to teleport\n");
		debugPrintf("Valid missions are: DEMON, TUG, LOVE, MUDD, FEATHER, TRIAL, SINS, VENG");
		return true;
	}

	_vm->_missionToLoad = argv[1];
	_vm->_roomIndexToLoad = atoi(argv[2]);
	_vm->runAwayMission();

	return false;
}

} // End of namespace StarTrek
