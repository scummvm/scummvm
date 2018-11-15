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

#include "glk/glk.h"
#include "glk/frotz/detection.h"
#include "glk/frotz/frotz.h"
#include "glk/scott/detection.h"
#include "glk/scott/scott.h"

#include "base/plugins.h"
#include "common/md5.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/system.h"
#include "engines/advancedDetector.h"
#include "graphics/colormasks.h"
#include "graphics/surface.h"
#include "common/config-manager.h"
#include "common/file.h"

#define MAX_SAVES 99

class GlkMetaEngine : public MetaEngine {
private:
	Common::String findFileByGameId(const Common::String &gameId) const;
public:
	GlkMetaEngine() : MetaEngine() {}

	virtual const char *getName() const {
		return "ScummGlk Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "ScummGlk Engine (c) 2018";
	}

	virtual bool hasFeature(MetaEngineFeature f) const override;
	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const override;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;

	/**
	 * Returns a list of games supported by this engine.
	 */
	virtual PlainGameList getSupportedGames() const override;

	/**
	 * Runs the engine's game detector on the given list of files, and returns a
	 * (possibly empty) list of games supported by the engine which it was able
	 * to detect amongst the given files.
	 */
	virtual DetectedGames detectGames(const Common::FSList &fslist) const override;

	/**
	 * Query the engine for a PlainGameDescriptor for the specified gameid, if any.
	 */
	virtual PlainGameDescriptor findGame(const char *gameId) const override;
};

bool GlkMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportCreationDate) ||
	    (f == kSavesSupportPlayTime) ||
	    (f == kSimpleSavesNames);
}

bool Glk::GlkEngine::hasFeature(EngineFeature f) const {
	return
	    (f == kSupportsRTL) ||
	    (f == kSupportsLoadingDuringRuntime) ||
	    (f == kSupportsSavingDuringRuntime);
}

Common::Error GlkMetaEngine::createInstance(OSystem *syst, Engine **engine) const {
	assert(engine);

	// Populate the game description
	Glk::GlkGameDescription gameDesc;
	gameDesc._gameId = ConfMan.get("gameid");
	gameDesc._filename = ConfMan.get("filename");

	gameDesc._language = Common::UNK_LANG;
	gameDesc._platform = Common::kPlatformUnknown;
	if (ConfMan.hasKey("language"))
		gameDesc._language = Common::parseLanguage(ConfMan.get("language"));
	if (ConfMan.hasKey("platform"))
		gameDesc._platform = Common::parsePlatform(ConfMan.get("platform"));

	// If the game description has no filename, the engine has been launched directly from
	// the command line. Do a scan for supported games for that Id in the game folder
	if (gameDesc._filename.empty()) {
		gameDesc._filename = findFileByGameId(gameDesc._gameId);
		if (gameDesc._filename.empty())
			return Common::kNoGameDataFoundError;
	}

	// Correct the correct engine
	if (Glk::Frotz::FrotzMetaEngine::findGame(gameDesc._gameId.c_str()).description)
		*engine = new Glk::Frotz::Frotz(syst, gameDesc);
	else if (Glk::Scott::ScottMetaEngine::findGame(gameDesc._gameId.c_str()).description)
		*engine = new Glk::Scott::Scott(syst, gameDesc);
	else
		return Common::kNoGameDataFoundError;

	return Common::kNoError;
}

Common::String GlkMetaEngine::findFileByGameId(const Common::String &gameId) const {
	// Get the list of files in the folder and return detection against them
	Common::FSNode folder = Common::FSNode(ConfMan.get("path"));
	Common::FSList fslist;
	folder.getChildren(fslist, Common::FSNode::kListFilesOnly);
	
	// Iterate over the files
	for (Common::FSList::iterator i = fslist.begin(); i != fslist.end(); ++i) {
		// Run a detection on each file in the folder individually
		Common::FSList singleList;
		singleList.push_back(*i);
		DetectedGames games = detectGames(singleList);

		// If a detection was found with the correct game Id, we have a winner
		if (!games.empty() && games.front().gameId == gameId)
			return (*i).getName();
	}

	// No match found
	return Common::String();
}

PlainGameList GlkMetaEngine::getSupportedGames() const {
	PlainGameList list;
	Glk::Frotz::FrotzMetaEngine::getSupportedGames(list);
	Glk::Scott::ScottMetaEngine::getSupportedGames(list);

	return list;
}

PlainGameDescriptor GlkMetaEngine::findGame(const char *gameId) const {
	PlainGameDescriptor gd;

	gd = Glk::Frotz::FrotzMetaEngine::findGame(gameId);
	if (gd.description) return gd;

	gd = Glk::Scott::ScottMetaEngine::findGame(gameId);
	if (gd.description) return gd;

	return PlainGameDescriptor();
}

DetectedGames GlkMetaEngine::detectGames(const Common::FSList &fslist) const {
	DetectedGames detectedGames;
	Glk::Frotz::FrotzMetaEngine::detectGames(fslist, detectedGames);
	Glk::Scott::ScottMetaEngine::detectGames(fslist, detectedGames);

	return detectedGames;
}

SaveStateList GlkMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);
	Glk::SavegameHeader header;

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot <= MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				if (Glk::FileStream::readSavegameHeader(in, header))
					saveList.push_back(SaveStateDescriptor(slot, header._saveName));

				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int GlkMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

void GlkMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor GlkMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename);

	if (in) {
		Glk::SavegameHeader header;
		if (Glk::FileStream::readSavegameHeader(in, header)) {
			// Create the return descriptor
			SaveStateDescriptor desc(slot, header._saveName);
			desc.setSaveDate(header._year, header._month, header._day);
			desc.setSaveTime(header._hour, header._minute);
			desc.setPlayTime(header._totalFrames * GAME_FRAME_TIME);

			delete in;
			return desc;
		}
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(GLK)
REGISTER_PLUGIN_DYNAMIC(GLK, PLUGIN_TYPE_ENGINE, GlkMetaEngine);
#else
REGISTER_PLUGIN_STATIC(GLK, PLUGIN_TYPE_ENGINE, GlkMetaEngine);
#endif
