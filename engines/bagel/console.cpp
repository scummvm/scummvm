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
	registerCmd("var",     WRAP_METHOD(Console, cmdVar));
	registerCmd("vars",    WRAP_METHOD(Console, cmdVars));
	registerCmd("fleebix", WRAP_METHOD(Console, cmdFleebix));
	registerCmd("save",    WRAP_METHOD(Console, cmdSave));
	registerCmd("load",    WRAP_METHOD(Console, cmdLoad));
	registerCmd("video",   WRAP_METHOD(Console, cmdVideo));
}

Console::~Console() {
}

bool Console::cmdVar(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("var <name> [new value]\n");
		return true;
	}

	CBagVar *var = VAR_MANAGER->GetVariable(argv[1]);
	assert(var);

	if (argc == 2) {
		debugPrintf("Current value = %s\n", var->GetValue().getBuffer());
	} else {
		var->SetValue(argv[2]);
		debugPrintf("Variable set\n");
	}

	return true;
}

bool Console::cmdVars(int argc, const char **argv) {
	for (int i = 0; i < VAR_MANAGER->GetNumVars(); i++) {
		CBagVar *pVar = VAR_MANAGER->GetVariable(i);
		if (pVar != nullptr) {
			debugPrintf("VAR[%d]: %s = %s\n", i, (const char *)pVar->getName(),
				(const char *)pVar->GetValue());
		}
	}

	return true;
}

bool Console::cmdFleebix(int argc, const char **argv) {
	CBofString inner = VAR_MANAGER->GetVariable("NDJAM_INNERDIAL_DISPLAY")->GetValue();
	CBofString outer = VAR_MANAGER->GetVariable("NDJAM_OUTERDIAL_DISPLAY")->GetValue();

	debugPrintf("Frequency is %s.%s\n", inner.getBuffer(), outer.getBuffer());
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
		char nameBuffer[MAX_SAVETITLE];
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

} // End of namespace Bagel
