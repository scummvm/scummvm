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

#include "common/savefile.h"
#include "bagel/console.h"
#include "bagel/baglib/var.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/save_game_file.h"
#include "bagel/boflib/app.h"

namespace Bagel {

Console::Console() : GUI::Debugger() {
	registerCmd("var",       WRAP_METHOD(Console, cmdVar));
	registerCmd("vars",      WRAP_METHOD(Console, cmdVars));
	registerCmd("jammer",    WRAP_METHOD(Console, cmdJammer));
	registerCmd("megawave",  WRAP_METHOD(Console, cmdMegawave));
	registerCmd("microwave", WRAP_METHOD(Console, cmdMegawave));
	registerCmd("save",      WRAP_METHOD(Console, cmdSave));
	registerCmd("load",      WRAP_METHOD(Console, cmdLoad));
	registerCmd("video",     WRAP_METHOD(Console, cmdVideo));
	registerCmd("timefreeze", WRAP_METHOD(Console, cmdTimefreeze));
}

Console::~Console() {
}

bool Console::cmdVar(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("var <name> [new value]\n");
		return true;
	}

	Common::String varName = argv[1];
	varName.toUppercase();

	CBagVar *var = g_VarManager->getVariable(varName.c_str());

	if (!var) {
		debugPrintf("Unknown variable.\n");
	} else if (argc == 2) {
		debugPrintf("Current value = %s\n", var->getValue().getBuffer());
	} else {
		var->setValue(argv[2]);
		debugPrintf("Variable set\n");
	}

	return true;
}

bool Console::cmdVars(int argc, const char **argv) {
	for (int i = 0; i < g_VarManager->getNumVars(); i++) {
		CBagVar *pVar = g_VarManager->getVariable(i);
		if (pVar != nullptr) {
			debugPrintf("VAR[%d]: %s = %s\n", i, (const char *)pVar->getName(),
				(const char *)pVar->getValue());
		}
	}

	return true;
}

bool Console::cmdJammer(int argc, const char **argv) {
	CBofString inner = g_VarManager->getVariable("NDJAM_INNERDIAL_DISPLAY")->getValue();
	CBofString outer = g_VarManager->getVariable("NDJAM_OUTERDIAL_DISPLAY")->getValue();
	bool isDone = g_VarManager->getVariable("HFJAM_DONE")->getNumValue() != 0;
	
	debugPrintf("Frequency is %s.%s, jammer is %s\n",
		inner.getBuffer(), outer.getBuffer(),
		isDone ? "correctly set" : "incorrectly set"
	);
	return true;
}

bool Console::cmdMegawave(int argc, const char **argv){
	CBofString wavetime = g_VarManager->getVariable("VEDJWAVETICKS")->getValue();

	debugPrintf("Megawave cooking time is %s\n", wavetime.getBuffer());
	return true;
}

bool Console::cmdLoad(int argc, const char **argv) {
	Common::SeekableReadStream *saveFile =
		g_system->getSavefileManager()->openForLoading("spacebar.sav");

	if (!saveFile) {
		debugPrintf("Could not locate original spacebar.sav in saves folder\n");
		return true;
	}

	delete saveFile;
	CBagSaveGameFile saves(false);
	const int count = saves.getNumSavedGames();

	if (argc == 1) {
		char nameBuffer[MAX_SAVE_TITLE];
		// No slot specified, so just list saves
		for (int i = 0; i < count; ++i) {
			if (saves.findRecord(i) != -1) {
				saves.readTitleOnly(i, nameBuffer);
				if (strlen(nameBuffer) > 0)
					debugPrintf("%2d - %s\n", i, nameBuffer);
			}
		}
	} else {
		// Read in actual savegame
		int slotNum = atoi(argv[1]);

		if (saves.readSavedGame(slotNum) == ERR_NONE)
			return false;

		debugPrintf("Could not read savegame.\n");
	}

	return true;
}

bool Console::cmdSave(int argc, const char **argv) {
	// Remove any existing created saves file
	g_system->getSavefileManager()->removeSavefile("spacebar.sav");

	CBagSaveGameFile saves(true);
	saves.writeSavedGame();

	saves.close();

	debugPrintf("Created new spacebar.sav in saves folder.\n");
	return true;
}

bool Console::cmdVideo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("video <path>\n");
		return true;
	} else if (!CBofApp::getApp()->consolePlayVideo(argv[1])) {
		debugPrintf("Could not locate video.\n");
		return true;
	} else {
		return false;
	}
}

bool Console::cmdTimefreeze(int argc, const char **argv) {
	CBagVar *var = g_VarManager->getVariable("TURNCOUNT");
	var->setFreeze(!var->isFrozen());

	debugPrintf("Time freeze is %s\n", var->isFrozen() ? "on" : "off");
	return true;
}

} // End of namespace Bagel
