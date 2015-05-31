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

#include "neverhood/console.h"
#include "gui/debugger.h"
#include "neverhood/neverhood.h"
#include "neverhood/gamemodule.h"
#include "neverhood/navigationscene.h"
#include "neverhood/scene.h"
#include "neverhood/smackerscene.h"
#include "neverhood/sound.h"
#include "neverhood/modules/module1600.h"
#include "neverhood/modules/module3000_sprites.h"

namespace Neverhood {

Console::Console(NeverhoodEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("cheat",			WRAP_METHOD(Console, Cmd_Cheat));
	registerCmd("checkresource",	WRAP_METHOD(Console, Cmd_CheckResource));
	registerCmd("dumpresource",	WRAP_METHOD(Console, Cmd_DumpResource));
	registerCmd("dumpvars",		WRAP_METHOD(Console, Cmd_Dumpvars));
	registerCmd("playsound",		WRAP_METHOD(Console, Cmd_PlaySound));
	registerCmd("scene",			WRAP_METHOD(Console, Cmd_Scene));
	registerCmd("surfaces",		WRAP_METHOD(Console, Cmd_Surfaces));
}

Console::~Console() {
}

bool Console::Cmd_Scene(int argc, const char **argv) {
	if (argc != 3) {
		int currentModule = _vm->_gameModule->getCurrentModuleNum();
		int previousModule = _vm->_gameModule->getPreviousModuleNum();
		int scenenNum = _vm->gameState().sceneNum;
		SceneType sceneType = ((GameModule *)_vm->_gameModule->_childObject)->getSceneType();

		const char *sceneTypes[] = { "normal", "smacker", "navigation" };

		debugPrintf("Current module: %d, previous module: %d, scene %d (%s scene)\n", currentModule, previousModule, scenenNum, sceneTypes[sceneType]);

		if (sceneType == kSceneTypeNormal) {
			Scene *scene = (Scene *)((GameModule *)_vm->_gameModule->_childObject)->_childObject;
			// Normal scenes have a background and a cursor file hash
			debugPrintf("Background hash: 0x%x, cursor hash: 0x%x\n", scene->getBackgroundFileHash(), scene->getCursorFileHash());
		} else if (sceneType == kSceneTypeSmacker) {
			SmackerScene *scene = (SmackerScene *)((GameModule *)_vm->_gameModule->_childObject)->_childObject;
			// Smacker scenes have a file hash, or a list of hashes
			// TODO: Only the first file hash is shown - any additional hashes, found in
			// scenes with a list of hashes (two scenes in module 1100 and the making of
			// video) aren't shown yet
			debugPrintf("File hash: 0x%x\n", scene->getSmackerFileHash());
		} else if (sceneType == kSceneTypeNavigation) {
			NavigationScene *scene = (NavigationScene *)((GameModule *)_vm->_gameModule->_childObject)->_childObject;
			// Navigation scenes have a navigation list and its index
			NavigationList *navigationList = _vm->_staticData->getNavigationList(scene->getNavigationListId());
			int navigationIndex = scene->getGlobalVar(V_NAVIGATION_INDEX);
			NavigationItem curNavigation = (*navigationList)[navigationIndex];
			debugPrintf("Navigation list ID: 0x%x, index: %d\n", scene->getNavigationListId(), navigationIndex);
			debugPrintf("File hash: 0x%x, cursor hash: 0x%x, Smacker hashes: [left: 0x%x, middle: 0x%x, right: 0x%x\n",
				curNavigation.fileHash, curNavigation.mouseCursorFileHash,
				curNavigation.leftSmackerFileHash, curNavigation.middleSmackerFileHash, curNavigation.rightSmackerFileHash);
		}

		debugPrintf("Use %s <module> <scene> to change scenes\n", argv[0]);
		debugPrintf("Modules are incremental by 100, from 1000 to 3000\n");
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
		debugPrintf("Cheats for various puzzles in the game\n");
		debugPrintf("Use %s <cheatname> to use a cheat.\n", argv[0]);
		debugPrintf("Cheats:\n-------\n");
		debugPrintf("  buttons - enables all 3 buttons on the door in the purple building, module 3000, scene 9\n");
		debugPrintf("  cannon  - sets the correct cannon combination in module 3000, scene 8\n");
		debugPrintf("  dice    - shows the correct dice combination in the teddy bear puzzle, module 1100, scene 6\n");
		debugPrintf("  memory  - solves the memory puzzle, module 1400, scene 4\n");
		debugPrintf("  music   - shows the correct index in the radio music puzzle, module 2800, scene 1\n");
		debugPrintf("  radio   - enables the radio, module 3000, scene 9 - same as pulling the rightmost cord in the flytrap room\n");
		debugPrintf("  symbols - solves the symbols puzzle, module 1600, scene 8. Only available in that room\n");
		debugPrintf("  tubes   - shows the correct test tube combination in module 2800, scenes 7 and 10\n");
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

		debugPrintf("All 3 door buttons have been enabled\n");
	} else if (cheatName == "cannon") {
		Scene *scene = (Scene *)((GameModule *)_vm->_gameModule->_childObject)->_childObject;

		for (int i = 0; i < 3; i++)
			scene->setSubVar(VA_CURR_CANNON_SYMBOLS, i,	scene->getSubVar(VA_GOOD_CANNON_SYMBOLS_1, i));

		for (int i = 3; i < 6; i++)
			scene->setSubVar(VA_CURR_CANNON_SYMBOLS, i,	scene->getSubVar(VA_GOOD_CANNON_SYMBOLS_2, i - 3));

		debugPrintf("Puzzle solved\n");
	} else if (cheatName == "dice") {
		Scene *scene = (Scene *)((GameModule *)_vm->_gameModule->_childObject)->_childObject;
		debugPrintf("Good: (%d %d %d), current: (%d %d %d)\n",
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

		debugPrintf("Puzzle solved\n");
	} else if (cheatName == "music") {
		Scene *scene = (Scene *)((GameModule *)_vm->_gameModule->_childObject)->_childObject;
		debugPrintf("Good music index: %d, current radio music index: %d\n", scene->getGlobalVar(V_CURR_RADIO_MUSIC_INDEX), scene->getGlobalVar(V_GOOD_RADIO_MUSIC_INDEX));
	} else if (cheatName == "radio") {
		Scene *scene = (Scene *)((GameModule *)_vm->_gameModule->_childObject)->_childObject;
		scene->setGlobalVar(V_RADIO_ENABLED, 1);

		debugPrintf("The radio has been enabled\n");
	} else if (cheatName == "symbols") {
		if (moduleNum == 1600 && sceneNum == 8) {
			Scene1609 *scene = ((Scene1609 *)((Module1600 *)_vm->_gameModule->_childObject)->_childObject);

			for (int index = 0; index < 12; index++) {
				scene->_asSymbols[index]->change((int)scene->getSubVar(VA_CODE_SYMBOLS, index) + 12, index == (int)scene->getSubVar(VA_CODE_SYMBOLS, scene->_noisySymbolIndex));
			}

			scene->_changeCurrentSymbol = false;
			scene->_symbolPosition = 11;
			scene->_countdown1 = 36;

			debugPrintf("Puzzle solved\n");
		} else {
			debugPrintf("Only available in module 1600, scene 8\n");
		}
	} else if (cheatName == "tubes") {
		Scene *scene = (Scene *)((GameModule *)_vm->_gameModule->_childObject)->_childObject;
		debugPrintf("Tube set 1: %d %d %d\n", scene->getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 0), scene->getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 1), scene->getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 2));
		debugPrintf("Tube set 2: %d %d %d\n", scene->getSubVar(VA_GOOD_TEST_TUBES_LEVEL_2, 0), scene->getSubVar(VA_GOOD_TEST_TUBES_LEVEL_2, 1), scene->getSubVar(VA_GOOD_TEST_TUBES_LEVEL_2, 2));
	}

	return true;
}

bool Console::Cmd_Dumpvars(int argc, const char **argv) {
	_vm->_gameVars->dumpVars(this);

	return true;
}

bool Console::Cmd_PlaySound(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s <sound hash>\n", argv[0]);
	} else {
		uint32 soundHash = strtol(argv[1], NULL, 0);
		AudioResourceManSoundItem *soundItem = new AudioResourceManSoundItem(_vm, soundHash);
		soundItem->setVolume(100);
		soundItem->playSound(false);
		while (soundItem->isPlaying()) {
			_vm->_system->delayMillis(10);
		}
		delete soundItem;
	}

	return true;
}

bool Console::Cmd_CheckResource(int argc, const char **argv) {
	const char *resourceNames[] = { "unknown", "unknown", "bitmap", "palette", "animation", "data", "text", "sound", "music", "unknown", "video" };

	if (argc < 2) {
		debugPrintf("Gets information about a resource\n");
		debugPrintf("Usage: %s <resource hash>\n", argv[0]);
	} else {
		uint32 resourceHash = strtol(argv[1], NULL, 0);
		ResourceHandle handle;

		_vm->_res->queryResource(resourceHash, handle);
		if (!handle.isValid()) {
			debugPrintf("Invalid resource hash\n");
		} else {
			debugPrintf("Resource type: %d (%s). Size: %d bytes\n", handle.type(), resourceNames[handle.type()], handle.size());
		}
	}

	return true;
}

bool Console::Cmd_DumpResource(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Dumps a resource to disk\n");
		debugPrintf("Usage: %s <resource hash> <output file>\n", argv[0]);
	} else {
		uint32 resourceHash = strtol(argv[1], NULL, 0);
		const char *outFileName = argv[2];
		ResourceHandle handle;

		_vm->_res->queryResource(resourceHash, handle);
		if (!handle.isValid()) {
			debugPrintf("Invalid resource hash\n");
		} else {
			_vm->_res->loadResource(handle, _vm->applyResourceFixes());
			Common::DumpFile outFile;
			outFile.open(outFileName);
			outFile.write(handle.data(), handle.size());
			outFile.finalize();
			outFile.close();
			_vm->_res->unloadResource(handle);
		}
	}

	return true;
}

} // End of namespace Neverhood
