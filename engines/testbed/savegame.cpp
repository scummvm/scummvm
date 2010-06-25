#include "common/savefile.h"

#include "testbed/savegame.h"

namespace Testbed {

/**
 * This test creates a savefile for the given testbed-state and could be reloaded using the saveFile API.
 * It is intended to test saving and loading from savefiles.
 */

bool SaveGametests::testSaveLoadState() {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::OutSaveFile *saveFile = saveFileMan->openForSaving("saveFile.0");

	if (!saveFile) {
		printf("LOG: Can't open saveFile\n");
		return false;
	}

	saveFile->writeString("ScummVM Rocks!");
	saveFile->finalize();
	delete saveFile;

	Common::InSaveFile *loadFile = saveFileMan->openForLoading("saveFile.0");

	
	if (!loadFile) {
		printf("LOG: Can't open save File to load\n");
		return false;
	}

	Common::String lineToRead = loadFile->readLine();
	delete loadFile;

	if (lineToRead.equals("ScummVM Rocks!")) {
		return true;
	}

	return false;
}

SaveGameTestSuite::SaveGameTestSuite() {
	addTest("Opening SaveFile", &SaveGametests::testSaveLoadState);
}

const char *SaveGameTestSuite::getName() const {
	return "SaveGames";
}

} // End of namespace testbed
