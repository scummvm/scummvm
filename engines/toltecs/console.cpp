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

#include "gui/debugger.h"

#include "toltecs/console.h"
//#include "toltecs/palette.h"
#include "toltecs/resource.h"
//#include "toltecs/sound.h"
#include "toltecs/toltecs.h"

namespace Toltecs {

Console::Console(ToltecsEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("room",			WRAP_METHOD(Console, Cmd_Room));
	registerCmd("dump",			WRAP_METHOD(Console, Cmd_Dump));
}

Console::~Console() {
}

bool Console::Cmd_Room(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Current room number is %d\n", _vm->_sceneResIndex);
#if 0
		debugPrintf("Calling this command with the room number changes the room\n");
		debugPrintf("WARNING: It's a bad idea to warp to rooms with this, as the room object scripts are not loaded\n");
#endif
		return true;
#if 0
	} else {
		int roomNum = atoi(argv[1]);

		// sfClearPaletteFragments
		_vm->_palette->clearFragments();

		// sfLoadScene
		_vm->_sound->stopAll();
		_vm->_res->purgeCache();
		_vm->loadScene(roomNum);
#endif
	}

	return false;
}

bool Console::Cmd_Dump(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: dump <resource number>\n");
		return true;
	}

	int resNum = atoi(argv[1]);
	_vm->_arc->dump(resNum);
	debugPrintf("Resource %d has been dumped to disk\n", resNum);

	return true;
}

} // End of namespace Toltecs
