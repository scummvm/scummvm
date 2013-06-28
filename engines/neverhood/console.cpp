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
#include "neverhood/modules/module1600.h"

namespace Neverhood {

Console::Console(NeverhoodEngine *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("cheat",			WRAP_METHOD(Console, Cmd_Cheat));
	DCmd_Register("dumpvars",		WRAP_METHOD(Console, Cmd_Dumpvars));
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

bool Console::Cmd_Cheat(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Cheats for various puzzles in the game\n");
		DebugPrintf("Use %s <cheatname> to use a cheat.\n", argv[0]);
		DebugPrintf("Cheats:\n-------\n");
		DebugPrintf("  buttons - enables all 3 buttons on the door in the purple building, module 3000, scene 9\n");
		DebugPrintf("  cannon  - sets the correct cannon combination in module 3000, scene 8\n");
		DebugPrintf("  dice    - shows the correct dice combination in the teddy bear puzzle, module 1100, scene 6\n");
		DebugPrintf("  memory  - solves the memory puzzle, module 1400, scene 4\n");
		DebugPrintf("  music   - shows the correct index in the radio music puzzle, module 2800, scene 1\n");
		DebugPrintf("  radio   - enables the radio, module 3000, scene 9 - same as pulling the rightmost cord in the flytrap room\n");
		DebugPrintf("  symbols - solves the symbols puzzle, module 1600, scene 8. Only available in that room\n");
		DebugPrintf("  tubes   - shows the correct test tube combination in module 2800, scenes 7 and 10\n");		
		return true;
	}

	Common::String cheatName = argv[1];
	int moduleNum = _vm->_gameModule->getCurrentModuleNum();
	int sceneNum = _vm->gameState().sceneNum;

	if (cheatName == "buttons") {
		Scene *scene = (Scene *)((GameModule *)_vm->_gameModule->_childObject)->_childObject;

		scene->setSubVar(VA_LOCKS_DISABLED, 0x304008D2, 1);	// kScene3010ButtonNameHashes[0]
		scene->setSubVar(VA_LOCKS_DISABLED, 0x40119852, 1);	// kScene3010ButtonNameHashes[1]
		scene->setSubVar(VA_LOCKS_DISABLED, 0x01180951, 1);	// kScene3010ButtonNameHashes[2]

		DebugPrintf("All 3 door buttons have been enabled\n");
	} else if (cheatName == "cannon") {
		Scene *scene = (Scene *)((GameModule *)_vm->_gameModule->_childObject)->_childObject;

		for (int i = 0; i < 3; i++)
			scene->setSubVar(VA_CURR_CANNON_SYMBOLS, i,	scene->getSubVar(VA_GOOD_CANNON_SYMBOLS_1, i));

		for (int i = 3; i < 6; i++)
			scene->setSubVar(VA_CURR_CANNON_SYMBOLS, i,	scene->getSubVar(VA_GOOD_CANNON_SYMBOLS_2, i - 3));

		DebugPrintf("Puzzle solved\n");
	} else if (cheatName == "dice") {
		Scene *scene = (Scene *)((GameModule *)_vm->_gameModule->_childObject)->_childObject;
		DebugPrintf("Good: (%d %d %d), current: (%d %d %d)\n",
			scene->getSubVar(VA_GOOD_DICE_NUMBERS, 0), scene->getSubVar(VA_GOOD_DICE_NUMBERS, 1), scene->getSubVar(VA_GOOD_DICE_NUMBERS, 2),
			scene->getSubVar(VA_CURR_DICE_NUMBERS, 0), scene->getSubVar(VA_CURR_DICE_NUMBERS, 1), scene->getSubVar(VA_CURR_DICE_NUMBERS, 2)
		);
	} else if (cheatName == "memory") {
		Scene *scene = (Scene *)((GameModule *)_vm->_gameModule->_childObject)->_childObject;

		// Autosolve all tiles and leave only two matching tiles closed
		for (int i = 0; i < 48; i++)
			scene->setSubVar(VA_IS_TILE_MATCH, i, 1);

		// Close the top left tile
		scene->setSubVar(VA_IS_TILE_MATCH, 0, 0);

		// Find and close the pair of the top left tile
		for (int i = 0; i < 48; i++) {
			if (i != 0 && scene->getSubVar(VA_TILE_SYMBOLS, i) == scene->getSubVar(VA_TILE_SYMBOLS, 0)) {
				scene->setSubVar(VA_IS_TILE_MATCH, i, 0);
				break;
			}
		}

		DebugPrintf("Puzzle solved\n");
	} else if (cheatName == "music") {
		Scene *scene = (Scene *)((GameModule *)_vm->_gameModule->_childObject)->_childObject;
		DebugPrintf("Good music index: %d, current radio music index: %d\n", scene->getGlobalVar(V_CURR_RADIO_MUSIC_INDEX), scene->getGlobalVar(V_GOOD_RADIO_MUSIC_INDEX));
	} else if (cheatName == "radio") {
		Scene *scene = (Scene *)((GameModule *)_vm->_gameModule->_childObject)->_childObject;
		scene->setGlobalVar(V_RADIO_ENABLED, 1);

		DebugPrintf("The radio has been enabled\n");
	} else if (cheatName == "symbols") {
		if (moduleNum == 1600 && sceneNum == 8) {
			Scene1609 *scene = ((Scene1609 *)((Module1600 *)_vm->_gameModule->_childObject)->_childObject);

			for (int index = 0; index < 12; index++) {
				scene->_asSymbols[index]->change((int)scene->getSubVar(VA_CODE_SYMBOLS, index) + 12, index == (int)scene->getSubVar(VA_CODE_SYMBOLS, scene->_noisySymbolIndex));
			}

			scene->_changeCurrentSymbol = false;
			scene->_symbolPosition = 11;
			scene->_countdown1 = 36;

			DebugPrintf("Puzzle solved\n");
		} else {
			DebugPrintf("Only available in module 1600, scene 8\n");
		}
	} else if (cheatName == "tubes") {
		Scene *scene = (Scene *)((GameModule *)_vm->_gameModule->_childObject)->_childObject;
		DebugPrintf("Tube set 1: %d %d %d\n", scene->getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 0), scene->getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 1), scene->getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 2));
		DebugPrintf("Tube set 2: %d %d %d\n", scene->getSubVar(VA_GOOD_TEST_TUBES_LEVEL_2, 0), scene->getSubVar(VA_GOOD_TEST_TUBES_LEVEL_2, 1), scene->getSubVar(VA_GOOD_TEST_TUBES_LEVEL_2, 2));
	}

	return true;
}

bool Console::Cmd_Dumpvars(int argc, const char **argv) {
	_vm->_gameVars->dumpVars(this);

	return true;
}

} // End of namespace Neverhood
