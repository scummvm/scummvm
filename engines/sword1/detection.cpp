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

#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/savefile.h"

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

	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const;
};

bool SwordMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup);
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
	SaveStateList saveList;

	Common::String pattern = "SAVEGAME.???";
	Common::StringList filenames = saveFileMan->listSavefiles(pattern.c_str());
	sort(filenames.begin(), filenames.end());
	Common::StringList::const_iterator file = filenames.begin();

	Common::InSaveFile *in = saveFileMan->openForLoading("SAVEGAME.INF");

	if (in) {
		Common::Array<uint32> offsets;
		uint8 stop = 0;
		int slotsInFile = 0;

		// Find the offset for each savegame name in the file.
		while (stop != 255 && !in->eos()) {
			offsets.push_back(in->pos());
			slotsInFile++;
			stop = 0;
			while (stop != 10 && stop != 255 && !in->eos())
				stop = in->readByte();
		}

		// Match the savegames to the save slot names.
		while (file != filenames.end()) {
			char saveDesc[32];

			if (file->compareToIgnoreCase("SAVEGAME.INF") == 0) {
				file++;
				continue;
			}
			
			// Obtain the last 3 digits of the filename, since they correspond to the save slot
			int slotNum = atoi(file->c_str() + file->size() - 3);

			if (slotNum >= 0 && slotNum < slotsInFile) {
				in->seek(offsets[slotNum]);

				uint pos = 0;
				do {
					stop = in->readByte();
					if (pos < sizeof(saveDesc) - 1) {
						if (stop == 10 || stop == 255 || in->eos())
							saveDesc[pos++] = '\0';
						else if (stop >= 32)
							saveDesc[pos++] = stop;
					}
				} while (stop != 10 && stop != 255 && !in->eos());
			}

			if (saveDesc[0] == 0)
				strcpy(saveDesc, "Unnamed savegame");

			// FIXME: The in-game dialog shows the first save slot as 1, not 0,
			// but if we change the numbering here, the launcher won?t set
			// "save_slot" correctly.
			saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
			file++;
		}
	}

	delete in;

	return saveList;
}

int SwordMetaEngine::getMaximumSaveSlot() const { return 999; }

#if PLUGIN_ENABLED_DYNAMIC(SWORD1)
	REGISTER_PLUGIN_DYNAMIC(SWORD1, PLUGIN_TYPE_ENGINE, SwordMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SWORD1, PLUGIN_TYPE_ENGINE, SwordMetaEngine);
#endif
