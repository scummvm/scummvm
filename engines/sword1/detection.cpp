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
 * $URL$
 * $Id$
 *
 */

#include "sword1/sword1.h"
#include "sword1/control.h"

#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "graphics/thumbnail.h"

#include "engines/metaengine.h"

/* Broken Sword 1 */
static const PlainGameDescriptor sword1FullSettings =
	{"sword1", "Broken Sword 1: The Shadow of the Templars"};
static const PlainGameDescriptor sword1DemoSettings =
	{"sword1demo", "Broken Sword 1: The Shadow of the Templars (Demo)"};
static const PlainGameDescriptor sword1MacFullSettings =
	{"sword1mac", "Broken Sword 1: The Shadow of the Templars (Mac)"};
static const PlainGameDescriptor sword1MacDemoSettings =
	{"sword1macdemo", "Broken Sword 1: The Shadow of the Templars (Mac demo)"};

// check these subdirectories (if present)
static const char *g_dirNames[] = {	"clusters",	"speech" };

#define NUM_COMMON_FILES_TO_CHECK 1
#define NUM_PC_FILES_TO_CHECK 3
#define NUM_MAC_FILES_TO_CHECK 4
#define NUM_DEMO_FILES_TO_CHECK 1
#define NUM_MAC_DEMO_FILES_TO_CHECK 1
#define NUM_FILES_TO_CHECK NUM_COMMON_FILES_TO_CHECK + NUM_PC_FILES_TO_CHECK + NUM_MAC_FILES_TO_CHECK + NUM_DEMO_FILES_TO_CHECK + NUM_MAC_DEMO_FILES_TO_CHECK
static const char *g_filesToCheck[NUM_FILES_TO_CHECK] = { // these files have to be found
	"swordres.rif", // Mac and PC version
	"general.clu", // PC version only
	"compacts.clu", // PC version only
	"scripts.clu", // PC version only
	"general.clm", // Mac version only
	"compacts.clm", // Mac version only
	"scripts.clm", // Mac version only
	"paris2.clm", // Mac version (full game only)
	"cows.mad",	// this one should only exist in the demo version
	"scripts.clm", // Mac version both demo and full game
	// the engine needs several more files to work, but checking these should be sufficient
};

class SwordMetaEngine : public MetaEngine {
public:
	virtual const char *getName() const {
		return "Broken Sword";
	}
	virtual const char *getCopyright() const {
		return "Broken Sword Games (C) Revolution";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual GameList getSupportedGames() const;
	virtual GameDescriptor findGame(const char *gameid) const;
	virtual GameList detectGames(const Common::FSList &fslist) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;

	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const;
};

bool SwordMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate);
}

bool Sword1::SwordEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

GameList SwordMetaEngine::getSupportedGames() const {
	GameList games;
	games.push_back(sword1FullSettings);
	games.push_back(sword1DemoSettings);
	games.push_back(sword1MacFullSettings);
	games.push_back(sword1MacDemoSettings);
	return games;
}

GameDescriptor SwordMetaEngine::findGame(const char *gameid) const {
	if (0 == scumm_stricmp(gameid, sword1FullSettings.gameid))
		return sword1FullSettings;
	if (0 == scumm_stricmp(gameid, sword1DemoSettings.gameid))
		return sword1DemoSettings;
	if (0 == scumm_stricmp(gameid, sword1MacFullSettings.gameid))
		return sword1MacFullSettings;
	if (0 == scumm_stricmp(gameid, sword1MacDemoSettings.gameid))
		return sword1MacDemoSettings;
	return GameDescriptor();
}

void Sword1CheckDirectory(const Common::FSList &fslist, bool *filesFound) {
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (!file->isDirectory()) {
			const char *fileName = file->getName().c_str();
			for (int cnt = 0; cnt < NUM_FILES_TO_CHECK; cnt++)
				if (scumm_stricmp(fileName, g_filesToCheck[cnt]) == 0)
					filesFound[cnt] = true;
		} else {
			for (int cnt = 0; cnt < ARRAYSIZE(g_dirNames); cnt++)
				if (scumm_stricmp(file->getName().c_str(), g_dirNames[cnt]) == 0) {
					Common::FSList fslist2;
					if (file->getChildren(fslist2, Common::FSNode::kListFilesOnly))
						Sword1CheckDirectory(fslist2, filesFound);
				}
		}
	}
}

GameList SwordMetaEngine::detectGames(const Common::FSList &fslist) const {
	int i, j;
	GameList detectedGames;
	bool filesFound[NUM_FILES_TO_CHECK];
	for (i = 0; i < NUM_FILES_TO_CHECK; i++)
		filesFound[i] = false;

	Sword1CheckDirectory(fslist, filesFound);
	bool mainFilesFound = true;
	bool pcFilesFound = true;
	bool macFilesFound = true;
	bool demoFilesFound = true;
	bool macDemoFilesFound = true;
	for (i = 0; i < NUM_COMMON_FILES_TO_CHECK; i++)
		if (!filesFound[i])
			mainFilesFound = false;
	for (j = 0; j < NUM_PC_FILES_TO_CHECK; i++, j++)
		if (!filesFound[i])
			pcFilesFound = false;
	for (j = 0; j < NUM_MAC_FILES_TO_CHECK; i++, j++)
		if (!filesFound[i])
			macFilesFound = false;
	for (j = 0; j < NUM_DEMO_FILES_TO_CHECK; i++, j++)
		if (!filesFound[i])
			demoFilesFound = false;
	for (j = 0; j < NUM_DEMO_FILES_TO_CHECK; i++, j++)
		if (!filesFound[i])
			macDemoFilesFound = false;

	if (mainFilesFound && pcFilesFound && demoFilesFound)
		detectedGames.push_back(sword1DemoSettings);
	else if (mainFilesFound && pcFilesFound)
		detectedGames.push_back(sword1FullSettings);
	else if (mainFilesFound && macFilesFound)
		detectedGames.push_back(sword1MacFullSettings);
	else if (mainFilesFound && macDemoFilesFound)
		detectedGames.push_back(sword1MacDemoSettings);

	return detectedGames;
}

Common::Error SwordMetaEngine::createInstance(OSystem *syst, Engine **engine) const {
	assert(engine);
	*engine = new Sword1::SwordEngine(syst);
	return Common::kNoError;
}

SaveStateList SwordMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String pattern = target;
	pattern += ".???";
	SaveStateList saveList;
	char saveName[40];

	Common::StringList filenames = saveFileMan->listSavefiles(pattern.c_str());
	sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	int slotNum = 0;
	for (Common::StringList::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 3);
		
		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				in->readUint32LE();	// header
				in->read(saveName, 40);
				saveList.push_back(SaveStateDescriptor(slotNum, saveName));
				delete in;
			}
		}
	}

	return saveList;
}

int SwordMetaEngine::getMaximumSaveSlot() const { return 999; }

void SwordMetaEngine::removeSaveState(const char *target, int slot) const {
	char extension[6];
	snprintf(extension, sizeof(extension), ".%03d", slot);

	Common::String filename = target;
	filename += extension;

	g_system->getSavefileManager()->removeSavefile(filename.c_str());
}

SaveStateDescriptor SwordMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	static char fileName[40];
	sprintf(fileName, "sword1.%03d", slot);
	char name[40];

	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fileName);

	if (in) {
		in->skip(4);		// header
		in->read(name, sizeof(name));
		in->skip(1);		// version

		SaveStateDescriptor desc(slot, name);

		desc.setDeletableFlag(true);
		desc.setWriteProtectedFlag(false);

		bool hasThumbnail = in->readByte();
		if (hasThumbnail) {
			Graphics::Surface *thumbnail = new Graphics::Surface();
			assert(thumbnail);
			if (!Graphics::loadThumbnail(*in, *thumbnail)) {
				delete thumbnail;
				thumbnail = 0;
			}

			desc.setThumbnail(thumbnail);
		}

		uint32 saveDate = in->readUint32BE();
		uint16 saveTime = in->readUint16BE();

		int day = (saveDate >> 24) & 0xFF;
		int month = (saveDate >> 16) & 0xFF;
		int year = saveDate & 0xFFFF;

		desc.setSaveDate(year, month, day);
			
		int hour = (saveTime >> 8) & 0xFF;
		int minutes = saveTime & 0xFF;

		desc.setSaveTime(hour, minutes);

		// TODO: played time

		delete in;

		return desc;
	}
	
	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(SWORD1)
	REGISTER_PLUGIN_DYNAMIC(SWORD1, PLUGIN_TYPE_ENGINE, SwordMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SWORD1, PLUGIN_TYPE_ENGINE, SwordMetaEngine);
#endif

namespace Sword1 {

// FIXME: Loading a game through the GMM crashes the game
#if 0
Common::Error SwordEngine::loadGameState(int slot) {
	_systemVars.forceRestart = false;
	_systemVars.controlPanelMode = CP_NORMAL;
	_control->restoreGameFromFile(slot);
	reinitialize();
	_control->doRestore();
	return Common::kNoError;	// TODO: return success/failure
}

Common::Error SwordEngine::saveGameState(int slot, const char *desc) {
	_control->setSaveDescription(slot, desc);
	_control->saveGameToFile(slot);
	return Common::kNoError;	// TODO: return success/failure
}

bool SwordEngine::canLoadGameStateCurrently() { 
	return mouseIsActive();
}

bool SwordEngine::canSaveGameStateCurrently() { 
	return mouseIsActive();
}
#endif

} // End of namespace Sword1
