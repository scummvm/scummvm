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

#include "sludge/debugger.h"
#include "sludge/sludge.h"
#include "sludge/fileset.h"

namespace Sludge {

Debugger::Debugger(SludgeEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("listResources", WRAP_METHOD(Debugger, Cmd_ListResources));
	registerCmd("dumpResource", WRAP_METHOD(Debugger, Cmd_DumpResource));
}

bool Debugger::Cmd_ListResources(int argc, const char **argv) {
	if (argc != 1 && argc != 2) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	for (int i = 0; i < _vm->_resMan->getResourceNameCount(); i++) {
		const Common::String name = _vm->_resMan->resourceNameFromNum(i);
		if (argc == 1 || name.matchString(argv[1]))
			debugPrintf(" - %s\n", name.c_str());
	}
	return true;
}

bool Debugger::Cmd_DumpResource(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	if (_vm->_resMan->dumpFileFromName(argv[1])) {
		debugPrintf("Success\n");
	} else {
		debugPrintf("Failure\n");
	}

	return true;
}

} // End of namespace Sludge
