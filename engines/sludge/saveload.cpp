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

#include "common/system.h"

#include "common/savefile.h"

#include "engines/metaengine.h"

#include "sludge/cursors.h"
#include "sludge/errors.h"
#include "sludge/event.h"
#include "sludge/floor.h"
#include "sludge/fonttext.h"
#include "sludge/function.h"
#include "sludge/graphics.h"
#include "sludge/language.h"
#include "sludge/newfatal.h"
#include "sludge/people.h"
#include "sludge/region.h"
#include "sludge/savedata.h"
#include "sludge/saveload.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/sound.h"
#include "sludge/statusba.h"
#include "sludge/speech.h"
#include "sludge/version.h"

namespace Sludge {

//----------------------------------------------------------------------
// From elsewhere
//----------------------------------------------------------------------

extern LoadedFunction *saverFunc;					// In function.cpp
extern LoadedFunction *allRunningFunctions;         // In sludger.cpp
extern int numGlobals;                              // In sludger.cpp
extern Variable *globalVars;                        // In sludger.cpp
extern FILETIME fileTime;                           // In sludger.cpp
extern bool allowAnyFilename;

//----------------------------------------------------------------------
// Save everything
//----------------------------------------------------------------------

bool handleSaveLoad() {
	if (!g_sludge->loadNow.empty()) {
		if (g_sludge->loadNow[0] == ':') {
			Common::String saveName = g_sludge->loadNow.c_str() + 1;
			uint extensionLength = saveName.size() - saveName.rfind('.');
			saveName = saveName.substr(0, saveName.size() - extensionLength);

			int slot = -1;
			if (g_sludge->_saveNameToSlot.contains(saveName)) {
				slot = g_sludge->_saveNameToSlot[saveName];
			} else {
				// Find next available save slot
				Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
				int maxSaveSlot = g_sludge->getMetaEngine()->getMaximumSaveSlot();
				int autosaveSlot = g_sludge->getMetaEngine()->getAutosaveSlot();
				for (int i = 0; i <= maxSaveSlot; ++i) {
					if (i == autosaveSlot)
						continue;
					const Common::String filename = g_sludge->getMetaEngine()->getSavegameFile(i, g_sludge->getTargetName().c_str());
					if (!saveFileMan->exists(filename)) {
						slot = i;
						break;
					} else {
						// If the game uses only one save for everything (like robinsresque)
						// use that save
						Common::InSaveFile *fp = g_system->getSavefileManager()->openForLoading(filename);
						ExtendedSavegameHeader header;
						if (MetaEngine::readSavegameHeader(fp, &header)) {
							if (saveName == header.description) {
								slot = i;
								g_sludge->_saveNameToSlot[saveName] = slot;
								delete fp;
								break;
							}
						}
						delete fp;
					}
				}
				if (slot == -1) {
					slot = g_sludge->getMetaEngine()->getMaximumSaveSlot();
				}
			}

			g_sludge->saveGameState(slot, saveName, false);
			saverFunc->reg.setVariable(SVT_INT, 1);
		} else {
			if (!loadGame(g_sludge->loadNow))
				return false;
		}
		g_sludge->loadNow.clear();
	}
	return true;
}

bool saveGame(Common::OutSaveFile *saveFile) {
	if (saveFile == NULL)
		return false;

	saveFile->writeString("SLUDSA");
	saveFile->writeByte(0);
	saveFile->writeByte(0);
	saveFile->writeByte(MAJOR_VERSION);
	saveFile->writeByte(MINOR_VERSION);

	if (!g_sludge->_gfxMan->saveThumbnail(saveFile))
		return false;

	saveFile->write(&fileTime, sizeof(FILETIME));

	// DON'T ADD ANYTHING NEW BEFORE THIS POINT!

	saveFile->writeByte(allowAnyFilename);
	saveFile->writeByte(false); // deprecated captureAllKeys
	saveFile->writeByte(true);
	g_sludge->_txtMan->saveFont(saveFile);

	// Save backdrop
	g_sludge->_gfxMan->saveBackdrop(saveFile);

	// Save event handlers
	g_sludge->_evtMan->saveHandlers(saveFile);

	// Save regions
	g_sludge->_regionMan->saveRegions(saveFile);

	g_sludge->_cursorMan->saveCursor(saveFile);

	// Save functions
	LoadedFunction *thisFunction = allRunningFunctions;
	int countFunctions = 0;
	while (thisFunction) {
		countFunctions++;
		thisFunction = thisFunction->next;
	}
	saveFile->writeUint16BE(countFunctions);

	thisFunction = allRunningFunctions;
	while (thisFunction) {
		saveFunction(thisFunction, saveFile);
		thisFunction = thisFunction->next;
	}

	for (int a = 0; a < numGlobals; a++) {
		globalVars[a].save(saveFile);
	}

	g_sludge->_peopleMan->savePeople(saveFile);

	g_sludge->_floorMan->save(saveFile);

	g_sludge->_gfxMan->saveZBuffer(saveFile);
	g_sludge->_gfxMan->saveLightMap(saveFile);

	g_sludge->_speechMan->save(saveFile);
	g_sludge->_statusBar->saveStatusBars(saveFile);
	g_sludge->_soundMan->saveSounds(saveFile);

	saveFile->writeUint16BE(CustomSaveHelper::_saveEncoding);

	g_sludge->_gfxMan->blur_saveSettings(saveFile);

	g_sludge->_gfxMan->saveColors(saveFile);

	g_sludge->_gfxMan->saveParallax(saveFile);
	saveFile->writeByte(0);

	g_sludge->_languageMan->saveLanguageSetting(saveFile);

	g_sludge->_gfxMan->saveSnapshot(saveFile);

	clearStackLib();
	return true;
}

//----------------------------------------------------------------------
// Load everything
//----------------------------------------------------------------------

int ssgVersion;

bool loadGame(const Common::String &fname) {
	Common::InSaveFile *fp = g_system->getSavefileManager()->openForLoading(fname);
	FILETIME savedGameTime;

	while (allRunningFunctions)
		finishFunction(allRunningFunctions);

	if (fp == NULL)
		return false;

	bool headerBad = false;
	if (fp->readByte() != 'S')
		headerBad = true;
	if (fp->readByte() != 'L')
		headerBad = true;
	if (fp->readByte() != 'U')
		headerBad = true;
	if (fp->readByte() != 'D')
		headerBad = true;
	if (fp->readByte() != 'S')
		headerBad = true;
	if (fp->readByte() != 'A')
		headerBad = true;
	if (headerBad) {
		fatal(ERROR_GAME_LOAD_NO, fname);
		return false;
	}
	char c;
	c = fp->readByte();
	while ((c = fp->readByte()))
		;

	int majVersion = fp->readByte();
	int minVersion = fp->readByte();
	ssgVersion = VERSION(majVersion, minVersion);

	if (ssgVersion >= VERSION(1, 4)) {
		if (!g_sludge->_gfxMan->skipThumbnail(fp))
			return fatal(ERROR_GAME_LOAD_CORRUPT, fname);
	}

	uint32 bytes_read = fp->read(&savedGameTime, sizeof(FILETIME));
	if (bytes_read != sizeof(FILETIME) && fp->err()) {
		warning("Reading error in loadGame.");
	}

	if (savedGameTime.dwLowDateTime != fileTime.dwLowDateTime || savedGameTime.dwHighDateTime != fileTime.dwHighDateTime) {
		return fatal(ERROR_GAME_LOAD_WRONG, fname);
	}

	// DON'T ADD ANYTHING NEW BEFORE THIS POINT!

	if (ssgVersion >= VERSION(1, 4)) {
		allowAnyFilename = fp->readByte();
	}
	fp->readByte(); // deprecated captureAllKeys
	fp->readByte(); // updateDisplay (part of movie playing)

	g_sludge->_txtMan->loadFont(ssgVersion, fp);

	g_sludge->_regionMan->kill();

	g_sludge->_gfxMan->loadBackdrop(ssgVersion, fp);

	g_sludge->_evtMan->loadHandlers(fp);

	g_sludge->_regionMan->loadRegions(fp);

	if (!g_sludge->_cursorMan->loadCursor(fp)) {
		return false;
	}

	LoadedFunction *rFunc;
	LoadedFunction **buildList = &allRunningFunctions;

	int countFunctions = fp->readUint16BE();
	while (countFunctions--) {
		rFunc = loadFunction(fp);
		rFunc->next = NULL;
		(*buildList) = rFunc;
		buildList = &(rFunc->next);
	}

	for (int a = 0; a < numGlobals; a++) {
		globalVars[a].unlinkVar();
		globalVars[a].load(fp);
	}

	g_sludge->_peopleMan->loadPeople(fp);

	if (!g_sludge->_floorMan->load(fp)) {
		return false;
	}

	if (!g_sludge->_gfxMan->loadZBuffer(fp))
		return false;

	if (!g_sludge->_gfxMan->loadLightMap(ssgVersion, fp)) {
		return false;
	}

	g_sludge->_speechMan->load(fp);
	g_sludge->_statusBar->loadStatusBars(fp);
	g_sludge->_soundMan->loadSounds(fp);

	CustomSaveHelper::_saveEncoding = fp->readUint16BE();

	if (ssgVersion >= VERSION(1, 6)) {
		if (ssgVersion < VERSION(2, 0)) {
			// aaLoad
			fp->readByte();
			fp->readFloatLE();
			fp->readFloatLE();
		}

		g_sludge->_gfxMan->blur_loadSettings(fp);
	}

	if (ssgVersion >= VERSION(1, 3)) {
		g_sludge->_gfxMan->loadColors(fp);

		// Read parallax layers
		while (fp->readByte()) {
			int im = fp->readUint16BE();
			int fx = fp->readUint16BE();
			int fy = fp->readUint16BE();

			if (!g_sludge->_gfxMan->loadParallax(im, fx, fy))
				return false;
		}

		g_sludge->_languageMan->loadLanguageSetting(fp);
	}

	g_sludge->_gfxMan->nosnapshot();
	if (ssgVersion >= VERSION(1, 4)) {
		if (fp->readByte()) {
			if (!g_sludge->_gfxMan->restoreSnapshot(fp))
				return false;
		}
	}

	ExtendedSavegameHeader header;
	if (MetaEngine::readSavegameHeader(fp, &header))
		g_sludge->setTotalPlayTime(header.playtime);

	delete fp;

	clearStackLib();
	return true;
}

} // End of namespace Sludge
