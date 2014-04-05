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

#include "fullpipe/constants.h"
#include "fullpipe/fullpipe.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/scene.h"

namespace Fullpipe {

Console::Console(FullpipeEngine *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("scene",			WRAP_METHOD(Console, Cmd_Scene));
}

bool Console::Cmd_Scene(int argc, const char **argv) {
	if (argc != 2) {
		int sceneTag = _vm->_currentScene->_sceneId;
		DebugPrintf("Current scene: %d (scene tag: %d)\n", _vm->getSceneFromTag(sceneTag), sceneTag);
		DebugPrintf("Use %s <scene> to change the current scene\n", argv[0]);
		return true;
	} else {
		int scene = _vm->convertScene(atoi(argv[1]));
		_vm->_gameLoader->loadScene(726);
		_vm->_gameLoader->gotoScene(726, TrubaLeft);

		if (scene != 726)
			_vm->_gameLoader->preloadScene(726, _vm->getSceneEntrance(scene));

		return false;
	}
}

} // End of namespace Fullpipe
