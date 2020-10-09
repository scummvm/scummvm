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

#include "kyra/gui/debugger.h"
#include "kyra/engine/kyra_lok.h"
#include "kyra/engine/kyra_hof.h"
#include "kyra/engine/timer.h"
#include "kyra/resource/resource.h"
#include "kyra/engine/lol.h"
#include "kyra/engine/eobcommon.h"

#include "common/system.h"
#include "common/config-manager.h"

namespace Kyra {

Debugger::Debugger(KyraEngine_v1 *vm)
	: ::GUI::Debugger(), _vm(vm) {
}

void Debugger::initialize() {
	registerCmd("continue",           WRAP_METHOD(Debugger, cmdExit));
	registerCmd("screen_debug_mode",  WRAP_METHOD(Debugger, cmdSetScreenDebug));
	registerCmd("load_palette",       WRAP_METHOD(Debugger, cmdLoadPalette));
	registerCmd("facings",            WRAP_METHOD(Debugger, cmdShowFacings));
	registerCmd("gamespeed",          WRAP_METHOD(Debugger, cmdGameSpeed));
	registerCmd("flags",              WRAP_METHOD(Debugger, cmdListFlags));
	registerCmd("toggleflag",         WRAP_METHOD(Debugger, cmdToggleFlag));
	registerCmd("queryflag",          WRAP_METHOD(Debugger, cmdQueryFlag));
	registerCmd("timers",             WRAP_METHOD(Debugger, cmdListTimers));
	registerCmd("settimercountdown",  WRAP_METHOD(Debugger, cmdSetTimerCountdown));
}

bool Debugger::cmdSetScreenDebug(int argc, const char **argv) {
	if (argc > 1) {
		if (scumm_stricmp(argv[1], "enable") == 0)
			_vm->screen()->enableScreenDebug(true);
		else if (scumm_stricmp(argv[1], "disable") == 0)
			_vm->screen()->enableScreenDebug(false);
		else
			debugPrintf("Use screen_debug_mode <enable/disable> to enable or disable it.\n");
	} else {
		debugPrintf("Screen debug mode is %s.\n", (_vm->screen()->queryScreenDebug() ? "enabled" : "disabled"));
		debugPrintf("Use screen_debug_mode <enable/disable> to enable or disable it.\n");
	}
	return true;
}

bool Debugger::cmdLoadPalette(int argc, const char **argv) {
	Palette palette(_vm->screen()->getPalette(0).getNumColors());

	if (argc <= 1) {
		debugPrintf("Use load_palette <file> [start_col] [end_col]\n");
		return true;
	}

	if (_vm->game() != GI_KYRA1 && _vm->resource()->getFileSize(argv[1]) != 768) {
		uint8 *buffer = new uint8[320 * 200 * sizeof(uint8)];
		if (!buffer) {
			debugPrintf("ERROR: Cannot allocate buffer for screen region!\n");
			return true;
		}

		_vm->screen()->copyRegionToBuffer(5, 0, 0, 320, 200, buffer);
		_vm->screen()->loadBitmap(argv[1], 5, 5, 0);
		palette.copy(_vm->screen()->getCPagePtr(5), 0, 256);
		_vm->screen()->copyBlockToPage(5, 0, 0, 320, 200, buffer);

		delete[] buffer;
	} else if (!_vm->screen()->loadPalette(argv[1], palette)) {
		debugPrintf("ERROR: Palette '%s' not found!\n", argv[1]);
		return true;
	}

	int startCol = 0;
	int endCol = palette.getNumColors();
	if (argc > 2)
		startCol = MIN(palette.getNumColors(), MAX(0, atoi(argv[2])));
	if (argc > 3)
		endCol = MIN(palette.getNumColors(), MAX(0, atoi(argv[3])));

	if (startCol > 0)
		palette.copy(_vm->screen()->getPalette(0), 0, startCol);
	if (endCol < palette.getNumColors())
		palette.copy(_vm->screen()->getPalette(0), endCol);

	_vm->screen()->setScreenPalette(palette);
	_vm->screen()->updateScreen();

	return true;
}

bool Debugger::cmdShowFacings(int argc, const char **argv) {
	debugPrintf("Facing directions:\n");
	debugPrintf("7  0  1\n");
	debugPrintf(" \\ | / \n");
	debugPrintf("6--*--2\n");
	debugPrintf(" / | \\\n");
	debugPrintf("5  4  3\n");
	return true;
}

bool Debugger::cmdGameSpeed(int argc, const char **argv) {
	if (argc == 2) {
		int val = atoi(argv[1]);

		if (val < 1 || val > 1000) {
			debugPrintf("speed must lie between 1 and 1000 (default: 60)\n");
			return true;
		}

		_vm->_tickLength = (uint8)(1000.0 / val);
	} else {
		debugPrintf("Syntax: gamespeed <value>\n");
	}

	return true;
}

bool Debugger::cmdListFlags(int argc, const char **argv) {
	for (int i = 0, p = 0; i < (int)sizeof(_vm->_flagsTable) * 8; i++, ++p) {
		debugPrintf("(%-3i): %-2i", i, _vm->queryGameFlag(i));
		if (p == 5) {
			debugPrintf("\n");
			p -= 6;
		}
	}
	debugPrintf("\n");
	return true;
}

bool Debugger::cmdToggleFlag(int argc, const char **argv) {
	if (argc > 1) {
		uint flag = atoi(argv[1]);
		if (_vm->queryGameFlag(flag))
			_vm->resetGameFlag(flag);
		else
			_vm->setGameFlag(flag);
		debugPrintf("Flag %i is now %i\n", flag, _vm->queryGameFlag(flag));
	} else {
		debugPrintf("Syntax: toggleflag <flag>\n");
	}

	return true;
}

bool Debugger::cmdQueryFlag(int argc, const char **argv) {
	if (argc > 1) {
		uint flag = atoi(argv[1]);
		debugPrintf("Flag %i is %i\n", flag, _vm->queryGameFlag(flag));
	} else {
		debugPrintf("Syntax: queryflag <flag>\n");
	}

	return true;
}

bool Debugger::cmdListTimers(int argc, const char **argv) {
	debugPrintf("Current time: %-8u\n", g_system->getMillis());
	for (int i = 0; i < _vm->timer()->count(); i++)
		debugPrintf("Timer %-2i: Active: %-3s Countdown: %-6i %-8u\n", i, _vm->timer()->isEnabled(i) ? "Yes" : "No", _vm->timer()->getDelay(i), _vm->timer()->getNextRun(i));

	return true;
}

bool Debugger::cmdSetTimerCountdown(int argc, const char **argv) {
	if (argc > 2) {
		uint timer = atoi(argv[1]);
		uint countdown = atoi(argv[2]);
		_vm->timer()->setCountdown(timer, countdown);
		debugPrintf("Timer %i now has countdown %i\n", timer, _vm->timer()->getDelay(timer));
	} else {
		debugPrintf("Syntax: settimercountdown <timer> <countdown>\n");
	}

	return true;
}

#pragma mark -

Debugger_LoK::Debugger_LoK(KyraEngine_LoK *vm)
	: Debugger(vm), _vm(vm) {
}

void Debugger_LoK::initialize() {
	registerCmd("enter",              WRAP_METHOD(Debugger_LoK, cmdEnterRoom));
	registerCmd("scenes",             WRAP_METHOD(Debugger_LoK, cmdListScenes));
	registerCmd("give",               WRAP_METHOD(Debugger_LoK, cmdGiveItem));
	registerCmd("birthstones",        WRAP_METHOD(Debugger_LoK, cmdListBirthstones));
	Debugger::initialize();
}

bool Debugger_LoK::cmdEnterRoom(int argc, const char **argv) {
	uint direction = 0;
	if (argc > 1) {
		int room = atoi(argv[1]);

		// game will crash if entering a non-existent room
		if (room >= _vm->_roomTableSize) {
			debugPrintf("room number must be any value between (including) 0 and %d\n", _vm->_roomTableSize - 1);
			return true;
		}

		if (argc > 2) {
			direction = atoi(argv[2]);
		} else {
			if (_vm->_roomTable[room].northExit != 0xFFFF)
				direction = 3;
			else if (_vm->_roomTable[room].eastExit != 0xFFFF)
				direction = 4;
			else if (_vm->_roomTable[room].southExit != 0xFFFF)
				direction = 1;
			else if (_vm->_roomTable[room].westExit != 0xFFFF)
				direction = 2;
		}

		_vm->_system->hideOverlay();
		_vm->_currentCharacter->facing = direction;

		_vm->enterNewScene(room, _vm->_currentCharacter->facing, 0, 0, 1);
		while (!_vm->_screen->isMouseVisible())
			_vm->_screen->showMouse();

		detach();
		return false;
	}

	debugPrintf("Syntax: room <roomnum> <direction>\n");
	return true;
}

bool Debugger_LoK::cmdListScenes(int argc, const char **argv) {
	for (int i = 0; i < _vm->_roomTableSize; i++) {
		debugPrintf("%-3i: %-10s", i, _vm->_roomFilenameTable[_vm->_roomTable[i].nameIndex]);
		if (!(i % 8))
			debugPrintf("\n");
	}
	debugPrintf("\n");
	debugPrintf("Current room: %i\n", _vm->_currentRoom);
	return true;
}

bool Debugger_LoK::cmdGiveItem(int argc, const char **argv) {
	if (argc == 2) {
		int item = atoi(argv[1]);

		// Kyrandia 1 has only 108 items (-1 to 106), otherwise it will crash
		if (item < -1 || item > 106) {
			debugPrintf("'itemid' must be any value between (including) -1 and 106\n");
			return true;
		}

		_vm->setMouseItem(item);
		_vm->_itemInHand = item;
	} else {
		debugPrintf("Syntax: give <itemid>\n");
	}

	return true;
}

bool Debugger_LoK::cmdListBirthstones(int argc, const char **argv) {
	debugPrintf("Needed birthstone gems:\n");
	for (int i = 0; i < ARRAYSIZE(_vm->_birthstoneGemTable); ++i)
		debugPrintf("%-3d '%s'\n", _vm->_birthstoneGemTable[i], _vm->_itemList[_vm->_birthstoneGemTable[i]]);
	return true;
}

#pragma mark -

Debugger_v2::Debugger_v2(KyraEngine_v2 *vm) : Debugger(vm), _vm(vm) {
}

void Debugger_v2::initialize() {
	registerCmd("character_info",     WRAP_METHOD(Debugger_v2, cmdCharacterInfo));
	registerCmd("enter",              WRAP_METHOD(Debugger_v2, cmdEnterScene));
	registerCmd("scenes",             WRAP_METHOD(Debugger_v2, cmdListScenes));
	registerCmd("scene_info",         WRAP_METHOD(Debugger_v2, cmdSceneInfo));
	registerCmd("scene_to_facing",    WRAP_METHOD(Debugger_v2, cmdSceneToFacing));
	registerCmd("give",               WRAP_METHOD(Debugger_v2, cmdGiveItem));
	Debugger::initialize();
}

bool Debugger_v2::cmdEnterScene(int argc, const char **argv) {
	uint direction = 0;
	if (argc > 1) {
		int scene = atoi(argv[1]);

		// game will crash if entering a non-existent scene
		if (scene >= _vm->_sceneListSize) {
			debugPrintf("scene number must be any value between (including) 0 and %d\n", _vm->_sceneListSize - 1);
			return true;
		}

		if (argc > 2) {
			direction = atoi(argv[2]);
		} else {
			if (_vm->_sceneList[scene].exit1 != 0xFFFF)
				direction = 4;
			else if (_vm->_sceneList[scene].exit2 != 0xFFFF)
				direction = 6;
			else if (_vm->_sceneList[scene].exit3 != 0xFFFF)
				direction = 0;
			else if (_vm->_sceneList[scene].exit4 != 0xFFFF)
				direction = 2;
		}

		_vm->_system->hideOverlay();
		_vm->_mainCharacter.facing = direction;

		_vm->enterNewScene(scene, _vm->_mainCharacter.facing, 0, 0, 1);
		while (!_vm->screen_v2()->isMouseVisible())
			_vm->screen_v2()->showMouse();

		detach();
		return false;
	}

	debugPrintf("Syntax: %s <scenenum> <direction>\n", argv[0]);
	return true;
}

bool Debugger_v2::cmdListScenes(int argc, const char **argv) {
	int shown = 1;
	for (int i = 0; i < _vm->_sceneListSize; ++i) {
		if (_vm->_sceneList[i].filename1[0]) {
			debugPrintf("%-2i: %-10s", i, _vm->_sceneList[i].filename1);
			if (!(shown % 5))
				debugPrintf("\n");
			++shown;
		}
	}
	debugPrintf("\n");
	debugPrintf("Current scene: %i\n", _vm->_currentScene);
	return true;
}

bool Debugger_v2::cmdSceneInfo(int argc, const char **argv) {
	debugPrintf("Current scene: %d '%s'\n", _vm->_currentScene, _vm->_sceneList[_vm->_currentScene].filename1);
	debugPrintf("\n");
	debugPrintf("Exit information:\n");
	debugPrintf("Exit1: leads to %d, position %dx%d\n", int16(_vm->_sceneExit1), _vm->_sceneEnterX1, _vm->_sceneEnterY1);
	debugPrintf("Exit2: leads to %d, position %dx%d\n", int16(_vm->_sceneExit2), _vm->_sceneEnterX2, _vm->_sceneEnterY2);
	debugPrintf("Exit3: leads to %d, position %dx%d\n", int16(_vm->_sceneExit3), _vm->_sceneEnterX3, _vm->_sceneEnterY3);
	debugPrintf("Exit4: leads to %d, position %dx%d\n", int16(_vm->_sceneExit4), _vm->_sceneEnterX4, _vm->_sceneEnterY4);
	debugPrintf("Special exit information:\n");
	if (!_vm->_specialExitCount) {
		debugPrintf("No special exits.\n");
	} else {
		debugPrintf("This scene has %d special exits.\n", _vm->_specialExitCount);
		for (int i = 0; i < _vm->_specialExitCount; ++i) {
			debugPrintf("SpecialExit%d: facing %d, position (x1/y1/x2/y2): %d/%d/%d/%d\n", i,
			            _vm->_specialExitTable[20 + i], _vm->_specialExitTable[0 + i], _vm->_specialExitTable[5 + i],
			            _vm->_specialExitTable[10 + i], _vm->_specialExitTable[15 + i]);
		}
	}

	return true;
}

bool Debugger_v2::cmdCharacterInfo(int argc, const char **argv) {
	debugPrintf("Main character is in scene: %d '%s'\n", _vm->_mainCharacter.sceneId, _vm->_sceneList[_vm->_mainCharacter.sceneId].filename1);
	debugPrintf("Position: %dx%d\n", _vm->_mainCharacter.x1, _vm->_mainCharacter.y1);
	debugPrintf("Facing: %d\n", _vm->_mainCharacter.facing);
	debugPrintf("Inventory:\n");
	for (int i = 0; i < 20; ++i) {
		debugPrintf("%-2d ", int8(_vm->_mainCharacter.inventory[i]));
		if (i == 9 || i == 19)
			debugPrintf("\n");
	}
	return true;
}

bool Debugger_v2::cmdSceneToFacing(int argc, const char **argv) {
	if (argc == 2) {
		int facing = atoi(argv[1]);
		int16 exit = -1;

		switch (facing) {
		case 0: case 1: case 7:
			exit = _vm->_sceneList[_vm->_currentScene].exit1;
			break;

		case 6:
			exit = _vm->_sceneList[_vm->_currentScene].exit2;
			break;

		case 3: case 4: case 5:
			exit = _vm->_sceneList[_vm->_currentScene].exit3;
			break;

		case 2:
			exit = _vm->_sceneList[_vm->_currentScene].exit4;
			break;

		default:
			break;
		}

		debugPrintf("Exit to facing %d leads to room %d.\n", facing, exit);
	} else {
		debugPrintf("Usage: %s <facing>\n", argv[0]);
	}

	return true;
}

bool Debugger_v2::cmdGiveItem(int argc, const char **argv) {
	if (argc == 2) {
		int item = atoi(argv[1]);

		if (item < -1 || item > _vm->engineDesc().maxItemId) {
			debugPrintf("itemid must be any value between (including) -1 and %d\n", _vm->engineDesc().maxItemId);
			return true;
		}

		_vm->setHandItem(item);
	} else {
		debugPrintf("Syntax: give <itemid>\n");
	}

	return true;
}

#pragma mark -

Debugger_HoF::Debugger_HoF(KyraEngine_HoF *vm) : Debugger_v2(vm), _vm(vm) {
}

void Debugger_HoF::initialize() {
	registerCmd("pass_codes",         WRAP_METHOD(Debugger_HoF, cmdPasscodes));
	Debugger_v2::initialize();
}

bool Debugger_HoF::cmdPasscodes(int argc, const char **argv) {
	if (argc == 2) {
		int val = atoi(argv[1]);

		if (val < 0 || val > 1) {
			debugPrintf("value must be either 1 (on) or 0 (off)\n");
			return true;
		}

		_vm->_dbgPass = val;
	} else {
		debugPrintf("Syntax: pass_codes <0/1>\n");
	}

	return true;
}

#pragma mark -

#ifdef ENABLE_LOL
Debugger_LoL::Debugger_LoL(LoLEngine *vm) : Debugger(vm), _vm(vm) {
}
#endif // ENABLE_LOL

#ifdef ENABLE_EOB
Debugger_EoB::Debugger_EoB(EoBCoreEngine *vm) : Debugger(vm), _vm(vm) {
}

void Debugger_EoB::initialize() {
	registerCmd("import_savefile", WRAP_METHOD(Debugger_EoB, cmdImportSaveFile));
	registerCmd("save_original", WRAP_METHOD(Debugger_EoB, cmdSaveOriginal));
	registerCmd("list_monsters", WRAP_METHOD(Debugger_EoB, cmdListMonsters));
	registerCmd("show_position", WRAP_METHOD(Debugger_EoB, cmdShowPosition));
	registerCmd("set_position", WRAP_METHOD(Debugger_EoB, cmdSetPosition));
	registerCmd("print_map", WRAP_METHOD(Debugger_EoB, cmdPrintMap));
	registerCmd("open_door", WRAP_METHOD(Debugger_EoB, cmdOpenDoor));
	registerCmd("close_door", WRAP_METHOD(Debugger_EoB, cmdCloseDoor));
	registerCmd("list_flags", WRAP_METHOD(Debugger_EoB, cmdListFlags));
	registerCmd("set_flag", WRAP_METHOD(Debugger_EoB, cmdSetFlag));
	registerCmd("clear_flag", WRAP_METHOD(Debugger_EoB, cmdClearFlag));
}

bool Debugger_EoB::cmdImportSaveFile(int argc, const char **argv) {
	if (!_vm->_allowImport) {
		debugPrintf("This command only works from the main menu.\n");
		return true;
	}

	if (argc == 3) {
		int slot = atoi(argv[1]);
		if (slot < -1 || slot > 989) {
			debugPrintf("slot must be between (including) -1 and 989 \n");
			return true;
		}

		debugPrintf(_vm->importOriginalSaveFile(slot, argv[2]) ? "Success.\n" : "Failure.\n");
		_vm->loadItemDefs();
	} else {
		debugPrintf("Syntax:   import_savefile <dest slot> <source file>\n              (Imports source save game file to dest slot.)\n          import_savefile -1\n              (Imports all original save game files found and puts them into the first available slots.)\n\n");
	}

	return true;
}

bool Debugger_EoB::cmdSaveOriginal(int argc, const char **argv) {
	if (_vm->gameFlags().platform == Common::kPlatformSegaCD) {
		debugPrintf("Command not supported for this version.\n");
		return true;
	}

	if (!_vm->_runFlag) {
		debugPrintf("This command doesn't work during intro or outro sequences,\nfrom the main menu or from the character generation.\n");
		return true;
	}

	Common::String dir = ConfMan.get("savepath");
	if (dir == "None")
		dir.clear();

	Common::FSNode nd(dir);
	if (!nd.isDirectory())
		return false;

	if (_vm->game() == GI_EOB1) {
		if (argc == 1) {
			if (_vm->saveAsOriginalSaveFile()) {
				Common::FSNode nf = nd.getChild(Common::String::format("EOBDATA.SAV"));
				if (nf.isReadable())
					debugPrintf("Saved to file: %s\n\n", nf.getPath().c_str());
				else
					debugPrintf("Failure.\n");
			} else {
				debugPrintf("Failure.\n");
			}
		} else {
			debugPrintf("Syntax:   save_original\n          (Saves game in original file format to a file which can be used with the original game executable.)\n\n");
		}
		return true;

	} else if (argc == 2) {
		int slot = atoi(argv[1]);
		if (slot < 0 || slot > 5) {
			debugPrintf("Slot must be between (including) 0 and 5.\n");
		} else if (_vm->saveAsOriginalSaveFile(slot)) {
			Common::FSNode nf = nd.getChild(Common::String::format("EOBDATA%d.SAV", slot));
			if (nf.isReadable())
				debugPrintf("Saved to file: %s\n\n", nf.getPath().c_str());
			else
				debugPrintf("Failure.\n");
		} else {
			debugPrintf("Failure.\n");
		}
		return true;
	}

	debugPrintf("Syntax:   save_original <slot>\n          (Saves game in original file format to a file which can be used with the original game executable.\n          A save slot between 0 and 5 must be specified.)\n\n");
	return true;
}

bool Debugger_EoB::cmdListMonsters(int, const char **) {
	debugPrintf("\nCurrent level: %d\n----------------------\n\n", _vm->_currentLevel);
	debugPrintf("Id        Type      Unit      Block     Position  Direction Sub Level Mode      Dst.block HP        Flags\n--------------------------------------------------------------------------------------------------------------\n");

	for (int i = 0; i < 30; i++) {
		EoBMonsterInPlay *m = &_vm->_monsters[i];
		debugPrintf("%.02d        %.02d        %.02d        0x%.04x    %d         %d         %d         %.02d        0x%.04x    %.03d/%.03d   0x%.02x\n", i, m->type, m->unit, m->block, m->pos, m->dir, m->sub, m->mode, m->dest, m->hitPointsCur, m->hitPointsMax, m->flags);
	}

	debugPrintf("\n");

	return true;
}

bool Debugger_EoB::cmdShowPosition(int, const char **) {
	debugPrintf("\nCurrent level:      %d\nCurrent Sub Level:  %d\nCurrent block:      %d (0x%.04x)\nNext block:         %d (0x%.04x)\nCurrent direction:  %d\n\n", _vm->_currentLevel, _vm->_currentSub, _vm->_currentBlock, _vm->_currentBlock, _vm->calcNewBlockPosition(_vm->_currentBlock, _vm->_currentDirection), _vm->calcNewBlockPosition(_vm->_currentBlock, _vm->_currentDirection), _vm->_currentDirection);
	return true;
}

bool Debugger_EoB::cmdSetPosition(int argc, const char **argv) {
	if (argc == 4) {
		_vm->_currentBlock = atoi(argv[3]);
		int sub = atoi(argv[2]);
		int level = atoi(argv[1]);

		int maxLevel = (_vm->game() == GI_EOB1) ? 12 : 16;
		if (level < 1 || level > maxLevel) {
			debugPrintf("<level> must be a value from 1 to %d.\n\n", maxLevel);
			return true;
		}

		if (level != _vm->_currentLevel || sub != _vm->_currentSub) {
			_vm->completeDoorOperations();
			_vm->generateTempData();
			_vm->txt()->removePageBreakFlag();
			_vm->screen()->setScreenDim(7);

			_vm->loadLevel(level, sub);

			if (_vm->_dialogueField)
				_vm->restoreAfterDialogueSequence();
		}

		_vm->moveParty(_vm->_currentBlock);

		_vm->_sceneUpdateRequired = true;
		_vm->gui_drawAllCharPortraitsWithStats();
		debugPrintf("Success.\n\n");

	} else {
		debugPrintf("Syntax:   set_position <level>, <sub level>, <block>\n");
		debugPrintf("          (Warning: The sub level and block position parameters will not be checked. Invalid parameters may cause problems.)\n\n");
	}
	return true;
}

bool Debugger_EoB::cmdPrintMap(int, const char **) {
	const uint8 illusion1 = _vm->gameFlags().gameID == GI_EOB1 ? 67 : 46;
	const uint8 illusion2 = _vm->gameFlags().gameID == GI_EOB1 ? 64 : 46;
	const uint8 plate1 = _vm->gameFlags().gameID == GI_EOB1 ? 28 : 35;
	const uint8 plate2 = _vm->gameFlags().gameID == GI_EOB1 ? 28 : 36;
	const uint8 hole = _vm->gameFlags().gameID == GI_EOB1 ? 27 : 38;
	const uint8 stairsUp = 23;
	const uint8 stairsDown = 24;
	const uint8 types[] = { _vm->_teleporterWallId, illusion1, illusion2, stairsUp, stairsDown, hole, plate1, plate2 };
	const uint8 signs[] = { 'T', 'i', 'i', 'U', 'D', 215, 'O', 'O', 'k' };

	for (int i = 0; i < 1024; ++i) {
		if (!(i % 0x20))
			debugPrintf("\n");
		LevelBlockProperty *bl = &_vm->_levelBlockProperties[i];
		uint8 f = _vm->_wllWallFlags[bl->walls[0]] | _vm->_wllWallFlags[bl->walls[1]] | _vm->_wllWallFlags[bl->walls[2]] | _vm->_wllWallFlags[bl->walls[3]];
		uint8 s = _vm->_specialWallTypes[bl->walls[0]] | _vm->_specialWallTypes[bl->walls[1]] | _vm->_specialWallTypes[bl->walls[2]] | _vm->_specialWallTypes[bl->walls[3]];
		uint8 c = ' ';
		if (s == 3 || s == 4)
			c = '/';
		else if (s == 2 || s == 8)
			c = 176;
		else if (f & 8)
			c = 216;
		else if (f & 1)
			c = 35;

		bool key = false;
		for (int t = bl->drawObjects; t; ) {
			EoBItem *itm = &_vm->_items[t];
			if (itm->type == 38)
				key = true;
			t = (itm->next != bl->drawObjects) ? itm->next : 0;
		}

		if (_vm->_currentBlock == i) {
			c = 'X';
		} else if (key) {
			c = signs[8];
		} else {
			for (int ii = 0; ii < ARRAYSIZE(types); ++ii) {
				if (bl->walls[0] == types[ii] || bl->walls[1] == types[ii] || bl->walls[2] == types[ii] || bl->walls[3] == types[ii]) {
					c = signs[ii];
					break;
				}
			}
		}

		debugPrintf("%c", c);
	}
	debugPrintf("\n\nParty Position:   %c  Door:             %c  Stairs Up/Down: %c/%c  Plate:      %c   Hole: %c\nSwitch:           %c  Clickable Object: %c  Illusion Wall:  %c    Teleporter: %c   Key:  %c\n\n", 'X', 216, signs[3], signs[4], signs[6], signs[5], '/', 176, signs[1], signs[0], signs[8]);

	return true;
}

bool Debugger_EoB::cmdOpenDoor(int, const char **) {
	uint16 block = _vm->calcNewBlockPosition(_vm->_currentBlock, _vm->_currentDirection);
	uint8 v = _vm->_wllWallFlags[_vm->_levelBlockProperties[block].walls[0]] | _vm->_wllWallFlags[_vm->_levelBlockProperties[block].walls[1]];
	int flg = (_vm->_flags.gameID == GI_EOB1) ? 1 : 0x10;
	if (!(v & 8)) {
		debugPrintf("Couldn't open any door. Make sure you're facing the door you wish to open and standing right in front of it.\n\n");
	} else if (v & flg) {
		debugPrintf("The door seems to be already open.\n\n");
	} else {
		_vm->openDoor(block);
		debugPrintf("Trying to open door at block %d.\n\n", block);
	}
	return true;
}

bool Debugger_EoB::cmdCloseDoor(int, const char **) {
	uint16 block = _vm->calcNewBlockPosition(_vm->_currentBlock, _vm->_currentDirection);
	uint8 v = _vm->_wllWallFlags[_vm->_levelBlockProperties[block].walls[0]] | _vm->_wllWallFlags[_vm->_levelBlockProperties[block].walls[1]];
	if (!(v & 8)) {
		debugPrintf("Couldn't close any door. Make sure you're facing the door you wish to close and standing right in front of it.\n\n");
	} else if ((_vm->_flags.gameID == GI_EOB1 && !(v & 1)) || (_vm->_flags.gameID == GI_EOB2 && (v & 0x20))) {
		debugPrintf("The door seems to be already closed.\n\n");
	} else {
		_vm->closeDoor(block);
		debugPrintf("Trying to close door at block %d.\n\n", block);
	}
	return true;
}

bool Debugger_EoB::cmdListFlags(int, const char **) {
	debugPrintf("Flag           Status\n----------------------\n\n");
	for (int i = 0; i < 32; i++) {
		uint32 flag = 1 << i;
		debugPrintf("%.2d             %s\n", i, _vm->checkScriptFlags(flag) ? "TRUE" : "FALSE");
	}
	debugPrintf("\n");
	return true;
}

bool Debugger_EoB::cmdSetFlag(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax:   set_flag <flag>\n\n");
		return true;
	}

	int flag = atoi(argv[1]);
	if (flag < 0 || flag > 31) {
		debugPrintf("<flag> must be a value from 0 to 31.\n\n");
	} else {
		_vm->setScriptFlags(1 << flag);
		debugPrintf("Flag '%.2d' has been set.\n\n", flag);
	}

	return true;
}

bool Debugger_EoB::cmdClearFlag(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax:   clear_flag <flag>\n\n");
		return true;
	}

	int flag = atoi(argv[1]);
	if (flag < 0 || flag > 31) {
		debugPrintf("<flag> must be a value from 0 to 31.\n\n");
	} else {
		_vm->clearScriptFlags(1 << flag);
		debugPrintf("Flag '%.2d' has been cleared.\n\n", flag);
	}

	return true;
}

#endif // ENABLE_EOB

} // End of namespace Kyra
