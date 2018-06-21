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

#include "pink/console.h"
#include "pink/pink.h"

namespace Pink {

Console::Console(PinkEngine *vm)
	: _vm(vm) {
	registerCmd("listModules", WRAP_METHOD(Console, Cmd_ListModules));
	registerCmd("goToModule", WRAP_METHOD(Console, Cmd_GoToModule));

	registerCmd("listGameVars", WRAP_METHOD(Console, Cmd_ListGameVars));
	registerCmd("setGameVar", WRAP_METHOD(Console, Cmd_SetGameVar));
}

bool Console::Cmd_ListModules(int argc, const char **argv) {
	const Array<NamedObject*> modules = _vm->_modules;
	for (uint i = 0; i < modules.size(); ++i) {
		debugPrintf("%d.%s\n", i, modules[i]->getName().c_str());
	}
	return true;
}

bool Console::Cmd_GoToModule(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s moduleName\n", argv[0]);
		debugPrintf("Module may not work properly because of Game vars\n");
		return true;
	}
	const Array<NamedObject*> modules = _vm->_modules;
	for (uint i = 0; i < modules.size(); ++i) {
		if (modules[i]->getName() == argv[1]) {
			_vm->initModule(argv[1], "", nullptr);
			return true;
		}
	}
	debugPrintf("Module %s doesn't exist\n", argv[1]);
	return true;
}

bool Console::Cmd_ListGameVars(int argc, const char **argv) {
	const StringMap &vars = _vm->_variables;
	for (StringMap::const_iterator it = vars.begin(); it != vars.end() ; ++it) {
		debugPrintf("%s %s \n", it->_key.c_str(), it->_value.c_str());
	}
	return true;
}

bool Console::Cmd_SetGameVar(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: %s varName value\n", argv[0]);
		return true;
	}
	_vm->_variables[argv[1]] = argv[2];
	return true;
}

} // End of namespace Pink
