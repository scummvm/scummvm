/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "illusions/console.h"
#include "illusions/illusions.h"
#include "illusions/input.h"

namespace Illusions {

Console::Console(IllusionsEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("cheat", WRAP_METHOD(Console, Cmd_cheat));
}

Console::~Console() {
}

bool Console::Cmd_cheat(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		debugPrintf("Switches on/off the cheat mode\n");
		return true;
	}

	bool active = !_vm->_input->isCheatModeActive();
	_vm->_input->setCheatModeActive(active);
	debugPrintf("Cheat is now %s\n", active ? "ON" : "OFF");
	return true;
}

} // End of namespace Illusions
