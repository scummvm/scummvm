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

#include "common/coroutines.h"
#include "cryo/debugger.h"
#include "cryo/cryo.h"

namespace Cryo {

Debugger::Debugger(CryoEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("showHotspots", WRAP_METHOD(Debugger, Cmd_ShowHotspots));
	registerCmd("fullInventory", WRAP_METHOD(Debugger, Cmd_FullInventory));
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
} // End of namespace Cryo
