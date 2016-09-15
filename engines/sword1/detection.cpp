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

#include "sword1/sword1.h"
#include "sword1/control.h"

#include "base/plugins.h"
#include "common/fs.h"
#include "common/gui_options.h"
#include "common/savefile.h"
#include "common/system.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "engines/metaengine.h"

/* Broken Sword */
static const PlainGameDescriptor sword1FullSettings =
	{"sword1", "Broken Sword: The Shadow of the Templars"};
static const PlainGameDescriptor sword1DemoSettings =
	{"sword1demo", "Broken Sword: The Shadow of the Templars (Demo)"};
static const PlainGameDescriptor sword1MacFullSettings =
	{"sword1mac", "Broken Sword: The Shadow of the Templars (Mac)"};
static const PlainGameDescriptor sword1MacDemoSettings =
	{"sword1macdemo", "Broken Sword: The Shadow of the Templars (Mac demo)"};
static const PlainGameDescriptor sword1PSXSettings =
	{"sword1psx", "Broken Sword: The Shadow of the Templars (PlayStation)"};
static const PlainGameDescriptor sword1PSXDemoSettings =
	{"sword1psxdemo", "Broken Sword: The Shadow of the Templars (PlayStation demo)"};


// check these subdirectories (if present)
static const char *const g_dirNames[] = { "clusters", "speech", "english", "italian"};

#define NUM_COMMON_FILES_TO_CHECK 1
#define NUM_PC_FILES_TO_CHECK 3
#define NUM_MAC_FILES_TO_CHECK 4
#define NUM_PSX_FILES_TO_CHECK 1
#define NUM_PSX_DEMO_FILES_TO_CHECK 2
#define NUM_DEMO_FILES_TO_CHECK 1
#define NUM_MAC_DEMO_FILES_TO_CHECK 1

#define NUM_FILES_TO_CHECK NUM_COMMON_FILES_TO_CHECK + NUM_PC_FILES_TO_CHECK + NUM_MAC_FILES_TO_CHECK + NUM_PSX_FILES_TO_CHECK + NUM_DEMO_FILES_TO_CHECK + NUM_MAC_DEMO_FILES_TO_CHECK + NUM_PSX_DEMO_FILES_TO_CHECK
static const char *const g_filesToCheck[NUM_FILES_TO_CHECK] = { // these files have to be found
	"swordres.rif", // Mac, PC and PSX version
	"general.clu", // PC and PSX version
	"compacts.clu", // PC and PSX version
	"scripts.clu", // PC and PSX version
	"general.clm", // Mac version only
	"compacts.clm", // Mac version only
	"scripts.clm", // Mac version only
	"paris2.clm", // Mac version (full game only)
	"cows.mad", // this one should only exist in the demo version
	"scripts.clm", // Mac version both demo and full game
	"train.plx", // PSX version only
	"speech.dat", // PSX version only
	"tunes.dat", // PSX version only
	// the engine needs several more files to work, but checking these should be sufficient
};

class SwordMetaEngine : public MetaEngine {
public:
	virtual const char *getEngineId() const {
		return "sword1";
	}

	virtual const char *getName() const {
		return "Broken Sword: The Shadow of the Templars";
	}
	virtual const char *getOriginalCopyright() const {
		return "Broken Sword: The Shadow of the Templars (C) Revolution";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	PlainGameList getSupportedGames() const override;
	PlainGameDescriptor findGame(const char *gameId) const override;
	DetectedGames detectGames(const Common::FSList &fslist) const override;
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
	    (f == kSavesSupportCreationDate) ||
	    (f == kSavesSupportPlayTime);
}

bool Sword1::SwordEngine::hasFeature(EngineFeature f) const {
	return
	    (f == kSupportsRTL) ||
	    (f == kSupportsSavingDuringRuntime) ||
	    (f == kSupportsLoadingDuringRuntime);
}

PlainGameList SwordMetaEngine::getSupportedGames() const {
	PlainGameList games;
	games.push_back(sword1FullSettings);
	games.push_back(sword1DemoSettings);
	games.push_back(sword1MacFullSettings);
	games.push_back(sword1MacDemoSettings);
	games.push_back(sword1PSXSettings);
	games.push_back(sword1PSXDemoSettings);
	return games;
}

PlainGameDescriptor SwordMetaEngine::findGame(const char *gameId) const {
	if (0 == scumm_stricmp(gameId, sword1FullSettings.gameId))
		return sword1FullSettings;
	if (0 == scumm_stricmp(gameId, sword1DemoSettings.gameId))
		return sword1DemoSettings;
	if (0 == scumm_stricmp(gameId, sword1MacFullSettings.gameId))
		return sword1MacFullSettings;
	if (0 == scumm_stricmp(gameId, sword1MacDemoSettings.gameId))
		return sword1MacDemoSettings;
	if (0 == scumm_stricmp(gameId, sword1PSXSettings.gameId))
		return sword1PSXSettings;
	if (0 == scumm_stricmp(gameId, sword1PSXDemoSettings.gameId))
		return sword1PSXDemoSettings;
	return PlainGameDescriptor::empty();
}

void Sword1CheckDirectory(const Common::FSList &fslist, bool *filesFound, bool recursion = false) {
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (!file->isDirectory()) {
			// The required game data files can be located in the game directory, or in
			// a subdirectory called "clusters". In the latter case, we don't want to
			// detect the game in that subdirectory, as this will detect the game twice
			// when mass add is searching inside a directory. In this case, the first
			// result (the game directory) will be correct, but the second result (the
			// clusters subdirectory) will be wrong, as the optional speech, music and
			// video data files will be ignored. Note that this fix will skip the game
			// data files if the user has placed them inside a "clusters" subdirectory,
			// or if he/she points ScummVM directly to the "clusters" directory of the
			// game CD. Fixes bug #3049346.
			Common::String directory = file->getParent().getName();
			directory.toLowercase();
			if (directory.hasPrefix("clusters") && directory.size() <= 9 && !recursion)
				continue;

			for (int cnt = 0; cnt < NUM_FILES_TO_CHECK; cnt++)
				if (scumm_stricmp(file->getName().c_str(), g_filesToCheck[cnt]) == 0)
					filesFound[cnt] = true;
		} else {
			for (int cnt = 0; cnt < ARRAYSIZE(g_dirNames); cnt++)
				if (scumm_stricmp(file->getName().c_str(), g_dirNames[cnt]) == 0) {
					Common::FSList fslist2;
					if (file->getChildren(fslist2, Common::FSNode::kListFilesOnly))
						Sword1CheckDirectory(fslist2, filesFound, true);
				}
		}
	}
}

DetectedGames SwordMetaEngine::detectGames(const Common::FSList &fslist) const {
	int i, j;
	DetectedGames detectedGames;
	bool filesFound[NUM_FILES_TO_CHECK];
	for (i = 0; i < NUM_FILES_TO_CHECK; i++)
		filesFound[i] = false;

	Sword1CheckDirectory(fslist, filesFound);
	bool mainFilesFound = true;
	bool pcFilesFound = true;
	bool macFilesFound = true;
	bool demoFilesFound = true;
	bool macDemoFilesFound = true;
	bool psxFilesFound = true;
	bool psxDemoFilesFound = true;
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
	for (j = 0; j < NUM_PSX_FILES_TO_CHECK; i++, j++)
		if (!filesFound[i])
			psxFilesFound = false;
	for (j = 0; j < NUM_PSX_DEMO_FILES_TO_CHECK; i++, j++)
		if (!filesFound[i] || psxFilesFound)
			psxDemoFilesFound = false;

	DetectedGame game;
	if (mainFilesFound && pcFilesFound && demoFilesFound)
		game = DetectedGame(sword1DemoSettings);
	else if (mainFilesFound && pcFilesFound && psxFilesFound)
		game = DetectedGame(sword1PSXSettings);
	else if (mainFilesFound && pcFilesFound && psxDemoFilesFound)
		game = DetectedGame(sword1PSXDemoSettings);
	else if (mainFilesFound && pcFilesFound && !psxFilesFound)
		game = DetectedGame(sword1FullSettings);
	else if (mainFilesFound && macFilesFound)
		game = DetectedGame(sword1MacFullSettings);
	else if (mainFilesFound && macDemoFilesFound)
		game = DetectedGame(sword1MacDemoSettings);
	else
		return detectedGames;

	game.setGUIOptions(GUIO2(GUIO_NOMIDI, GUIO_NOASPECT));

	game.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(Common::EN_ANY));
	game.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(Common::DE_DEU));
	game.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(Common::FR_FRA));
	game.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(Common::IT_ITA));
	game.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(Common::ES_ESP));
	game.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(Common::PT_BRA));
	game.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(Common::CZ_CZE));

	detectedGames.push_back(game);

	return detectedGames;
}

Common::Error SwordMetaEngine::createInstance(OSystem *syst, Engine **engine) const {
	assert(engine);
	*engine = new Sword1::SwordEngine(syst);
	return Common::kNoError;
}

SaveStateList SwordMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	SaveStateList saveList;
	char saveName[40];

	Common::StringArray filenames = saveFileMan->listSavefiles("sword1.###");

	int slotNum = 0;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				in->readUint32LE(); // header
				in->read(saveName, 40);
				saveList.push_back(SaveStateDescriptor(slotNum, saveName));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int SwordMetaEngine::getMaximumSaveSlot() const { return 999; }

void SwordMetaEngine::removeSaveState(const char *target, int slot) const {
	g_system->getSavefileManager()->removeSavefile(Common::String::format("sword1.%03d", slot));
}

SaveStateDescriptor SwordMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("sword1.%03d", slot);
	char name[40];
	uint32 playTime = 0;
	byte versionSave;

	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fileName);

	if (in) {
		in->skip(4);        // header
		in->read(name, sizeof(name));
		in->read(&versionSave, 1);      // version

		SaveStateDescriptor desc(slot, name);

		if (versionSave < 2) // These older version of the savegames used a flag to signal presence of thumbnail
			in->skip(1);

		if (Graphics::checkThumbnailHeader(*in)) {
			Graphics::Surface *thumbnail;
			if (!Graphics::loadThumbnail(*in, thumbnail)) {
				delete in;
				return SaveStateDescriptor();
			}
			desc.setThumbnail(thumbnail);
		}

		uint32 saveDate = in->readUint32BE();
		uint16 saveTime = in->readUint16BE();
		if (versionSave > 1) // Previous versions did not have playtime data
			playTime = in->readUint32BE();

		int day = (saveDate >> 24) & 0xFF;
		int month = (saveDate >> 16) & 0xFF;
		int year = saveDate & 0xFFFF;

		desc.setSaveDate(year, month, day);

		int hour = (saveTime >> 8) & 0xFF;
		int minutes = saveTime & 0xFF;

		desc.setSaveTime(hour, minutes);

		if (versionSave > 1) {
			desc.setPlayTime(playTime * 1000);
		} else { //We have no playtime data
			desc.setPlayTime(0);
		}

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

Common::Error SwordEngine::loadGameState(int slot) {
	_systemVars.forceRestart = false;
	_systemVars.controlPanelMode = CP_NORMAL;
	_control->restoreGameFromFile(slot);
	reinitialize();
	_control->doRestore();
	reinitRes();
	return Common::kNoError;    // TODO: return success/failure
}

bool SwordEngine::canLoadGameStateCurrently() {
	return (mouseIsActive() && !_control->isPanelShown()); // Disable GMM loading when game panel is shown
}

Common::Error SwordEngine::saveGameState(int slot, const Common::String &desc) {
	_control->setSaveDescription(slot, desc.c_str());
	_control->saveGameToFile(slot);
	return Common::kNoError;    // TODO: return success/failure
}

bool SwordEngine::canSaveGameStateCurrently() {
	return (mouseIsActive() && !_control->isPanelShown());
}

} // End of namespace Sword1
