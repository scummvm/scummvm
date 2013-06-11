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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "neverhood/console.h"
#include "gui/debugger.h"
#include "neverhood/neverhood.h"
#include "neverhood/gamemodule.h"
#include "neverhood/scene.h"

namespace Neverhood {

Console::Console(NeverhoodEngine *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("room",			WRAP_METHOD(Console, Cmd_Room));
	DCmd_Register("surfaces",		WRAP_METHOD(Console, Cmd_Surfaces));
}

Console::~Console() {
}

bool Console::Cmd_Room(int argc, const char **argv) {
	int currentModule = _vm->_gameModule->getCurrentModuleNum();
	int previousModule = _vm->_gameModule->getPreviousModuleNum();
	int scene = _vm->gameState().sceneNum;

	DebugPrintf("Current module: %d, previous module: %d, scene %d\n", currentModule, previousModule, scene);

	if (argc != 3) {
		DebugPrintf("Use room <module> <scene> to change rooms\n");
		DebugPrintf("Modules are incremental by 100, from 1000 to 3000\n");
	} else {
		int newModule = atoi(argv[1]);
		int newScene  = atoi(argv[2]);

		_vm->gameState().sceneNum = newScene;
		_vm->_gameModule->createModule(newModule, -1);
	}

	return true;
}

bool Console::Cmd_Surfaces(int argc, const char **argv) {
	if (_vm->_gameModule->_childObject) {
		((Scene *)((GameModule *)_vm->_gameModule->_childObject)->_childObject)->printSurfaces(this);
	}
	return true;
}

} // End of namespace Neverhood
