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

#include "cryo/debugger.h"
#include "cryo/cryo.h"

namespace Cryo {

Debugger::Debugger(CryoEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("showHotspots", WRAP_METHOD(Debugger, Cmd_ShowHotspots));
	registerCmd("fullInventory", WRAP_METHOD(Debugger, Cmd_FullInventory));
	registerCmd("phase", WRAP_METHOD(Debugger, Cmd_Phase));
}

/**
 * This command enables/disables hotspot display
 */
bool Debugger::Cmd_ShowHotspots(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	_vm->_showHotspots ^= 1;

	return false;
}

bool Debugger::Cmd_FullInventory(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	for (int i = 0; i < MAX_OBJECTS; i++) {
		object_t *object = _vm->_game->getObjectPtr(i);
		object->_flags |= ObjectFlags::ofFlag1;
		object->_count++;
	}

	_vm->_game->showObjects();

	return false;
}

/**
 * Show or set the story phase. Dialog lines are gated on it, so a character
 * with nothing to say is usually a phase that has not been reached yet.
 */
bool Debugger::Cmd_Phase(int argc, const char **argv) {
	global_t *globals = _vm->_game->_globals;

	if (argc == 2)
		globals->_phaseNum = (int16)strtol(argv[1], nullptr, 0);
	else if (argc != 1) {
		debugPrintf("Usage: %s [phase]\n", argv[0]);
		return true;
	}

	debugPrintf("phase %d (0x%X), room %d (0x%X)\n", globals->_phaseNum,
	            globals->_phaseNum, globals->_roomNum, globals->_roomNum);

	return true;
}
} // End of namespace Cryo
