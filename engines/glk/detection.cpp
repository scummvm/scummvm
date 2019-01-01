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
#include "glk/detection.h"
#include "glk/alan2/detection.h"
#include "glk/alan2/alan2.h"
#include "glk/frotz/detection.h"
#include "glk/frotz/frotz.h"
#include "glk/glulxe/detection.h"
#include "glk/glulxe/glulxe.h"
#include "glk/scott/detection.h"
#include "glk/scott/scott.h"
#include "glk/tads/detection.h"
#include "glk/tads/tads2/tads2.h"
#include "glk/tads/tads3/tads3.h"

#include "base/plugins.h"
#include "common/md5.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/system.h"
#include "graphics/colormasks.h"
#include "graphics/surface.h"
#include "common/config-manager.h"
#include "common/file.h"

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
	Glk::TADS::TADSDescriptor td;
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

	// Get the MD5
	Common::File f;
	if (!f.open(Common::FSNode(ConfMan.get("path")).getChild(gameDesc._filename)))
		return Common::kNoGameDataFoundError;

	gameDesc._md5 = Common::computeStreamMD5AsString(f, 5000);
	f.close();

	// Create the correct engine
	if (Glk::Alan2::Alan2MetaEngine::findGame(gameDesc._gameId.c_str()).description) {
		*engine = new Glk::Alan2::Alan2(syst, gameDesc);
	} else if (Glk::Frotz::FrotzMetaEngine::findGame(gameDesc._gameId.c_str()).description) {
		*engine = new Glk::Frotz::Frotz(syst, gameDesc);
	} else if (Glk::Glulxe::GlulxeMetaEngine::findGame(gameDesc._gameId.c_str()).description) {
		*engine = new Glk::Glulxe::Glulxe(syst, gameDesc);
	} else if (Glk::Scott::ScottMetaEngine::findGame(gameDesc._gameId.c_str()).description) {
		*engine = new Glk::Scott::Scott(syst, gameDesc);
	} else if ((td = Glk::TADS::TADSMetaEngine::findGame(gameDesc._gameId.c_str())).description) {
		if (td.isTADS3)
			*engine = new Glk::TADS::TADS3::TADS3(syst, gameDesc);
		else
			*engine = new Glk::TADS::TADS2::TADS2(syst, gameDesc);
	} else {
		return Common::kNoGameDataFoundError;
	}

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
	Glk::Alan2::Alan2MetaEngine::getSupportedGames(list);
	Glk::Frotz::FrotzMetaEngine::getSupportedGames(list);
	Glk::Glulxe::GlulxeMetaEngine::getSupportedGames(list);
	Glk::Scott::ScottMetaEngine::getSupportedGames(list);
	Glk::TADS::TADSMetaEngine::getSupportedGames(list);

	return list;
}

PlainGameDescriptor GlkMetaEngine::findGame(const char *gameId) const {
	PlainGameDescriptor gd;

	gd = Glk::Alan2::Alan2MetaEngine::findGame(gameId);
	if (gd.description) return gd;

	gd = Glk::Frotz::FrotzMetaEngine::findGame(gameId);
	if (gd.description) return gd;

	gd = Glk::Glulxe::GlulxeMetaEngine::findGame(gameId);
	if (gd.description) return gd;

	gd = Glk::Scott::ScottMetaEngine::findGame(gameId);
	if (gd.description) return gd;

	gd = Glk::TADS::TADSMetaEngine::findGame(gameId);
	if (gd.description) return gd;

	return PlainGameDescriptor();
}

DetectedGames GlkMetaEngine::detectGames(const Common::FSList &fslist) const {
	// This is as good a place as any to detect multiple sub-engines using the same Ids
	detectClashes();

	DetectedGames detectedGames;
	Glk::Alan2::Alan2MetaEngine::detectGames(fslist, detectedGames);
	Glk::Frotz::FrotzMetaEngine::detectGames(fslist, detectedGames);
	Glk::Glulxe::GlulxeMetaEngine::detectGames(fslist, detectedGames);
	Glk::Scott::ScottMetaEngine::detectGames(fslist, detectedGames);
	Glk::TADS::TADSMetaEngine::detectGames(fslist, detectedGames);

	return detectedGames;
}

void GlkMetaEngine::detectClashes() const {
	Common::StringMap map;
	Glk::Alan2::Alan2MetaEngine::detectClashes(map);
	Glk::Frotz::FrotzMetaEngine::detectClashes(map);
	Glk::Glulxe::GlulxeMetaEngine::detectClashes(map);
	Glk::Scott::ScottMetaEngine::detectClashes(map);
	Glk::TADS::TADSMetaEngine::detectClashes(map);
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
				else if (Glk::Frotz::FrotzMetaEngine::readSavegameHeader(in, header))
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
