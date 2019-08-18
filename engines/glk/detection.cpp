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
#include "glk/quetzal.h"
#include "glk/advsys/detection.h"
#include "glk/advsys/advsys.h"
#include "glk/alan2/detection.h"
#include "glk/alan2/alan2.h"
#include "glk/alan3/detection.h"
#include "glk/alan3/alan3.h"
#include "glk/frotz/detection.h"
#include "glk/frotz/frotz.h"
#include "glk/glulxe/detection.h"
#include "glk/glulxe/glulxe.h"
#include "glk/hugo/detection.h"
#include "glk/hugo/hugo.h"
#include "glk/magnetic/detection.h"
#include "glk/magnetic/magnetic.h"
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

namespace Glk {

GlkDetectedGame::GlkDetectedGame(const char *id, const char *desc, const Common::String &filename) :
		DetectedGame(id, desc, Common::EN_ANY, Common::kPlatformUnknown) {
	setGUIOptions(GUIO3(GUIO_NOSPEECH, GUIO_NOMUSIC, GUIO_NOSUBTITLES));
	addExtraEntry("filename", filename);
}

GlkDetectedGame::GlkDetectedGame(const char *id, const char *desc, const Common::String &filename,
		Common::Language lang) : DetectedGame(id, desc, lang, Common::kPlatformUnknown) {
	setGUIOptions(GUIO3(GUIO_NOSPEECH, GUIO_NOMUSIC, GUIO_NOSUBTITLES));
	addExtraEntry("filename", filename);
}

GlkDetectedGame::GlkDetectedGame(const char *id, const char *desc, const Common::String &filename,
		const Common::String &md5, size_t filesize) :
		DetectedGame(id, desc, Common::UNK_LANG, Common::kPlatformUnknown) {
	setGUIOptions(GUIO3(GUIO_NOSPEECH, GUIO_NOMUSIC, GUIO_NOSUBTITLES));
	addExtraEntry("filename", filename);

	canBeAdded = true;
	hasUnknownFiles = true;

	FileProperties fp;
	fp.md5 = md5;
	fp.size = filesize;
	matchedFiles[filename] = fp;
}

} // End of namespace Glk

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

template<class META, class ENG>Engine *create(OSystem *syst, Glk::GlkGameDescription &gameDesc) {
	Glk::GameDescriptor gd = META::findGame(gameDesc._gameId.c_str());
	if (gd._description) {
		gameDesc._options = gd._options;
		return new ENG(syst, gameDesc);
	} else {
		return nullptr;
	}
}

Common::Error GlkMetaEngine::createInstance(OSystem *syst, Engine **engine) const {
	Glk::GameDescriptor td = Glk::GameDescriptor::empty();
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
	*engine = nullptr;
	if ((*engine = create<Glk::AdvSys::AdvSysMetaEngine, Glk::AdvSys::AdvSys>(syst, gameDesc)) != nullptr) {}
	else if ((*engine = create<Glk::Alan2::Alan2MetaEngine, Glk::Alan2::Alan2>(syst, gameDesc)) != nullptr) {}
	else if ((*engine = create<Glk::Alan3::Alan3MetaEngine, Glk::Alan3::Alan3>(syst, gameDesc)) != nullptr) {}
	else if ((*engine = create<Glk::Frotz::FrotzMetaEngine, Glk::Frotz::Frotz>(syst, gameDesc)) != nullptr) {}
	else if ((*engine = create<Glk::Glulxe::GlulxeMetaEngine, Glk::Glulxe::Glulxe>(syst, gameDesc)) != nullptr) {}
	else if ((*engine = create<Glk::Hugo::HugoMetaEngine, Glk::Hugo::Hugo>(syst, gameDesc)) != nullptr) {}
	else if ((*engine = create<Glk::Scott::ScottMetaEngine, Glk::Scott::Scott>(syst, gameDesc)) != nullptr) {}
#ifndef RELEASE_BUILD
	else if ((*engine = create<Glk::Magnetic::MagneticMetaEngine, Glk::Magnetic::Magnetic>(syst, gameDesc)) != nullptr) {}
	else if ((td = Glk::TADS::TADSMetaEngine::findGame(gameDesc._gameId.c_str()))._description) {
		if (td._options & Glk::TADS::OPTION_TADS3)
			*engine = new Glk::TADS::TADS3::TADS3(syst, gameDesc);
		else
			*engine = new Glk::TADS::TADS2::TADS2(syst, gameDesc);
	} 
#endif	
	else {
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
	Glk::AdvSys::AdvSysMetaEngine::getSupportedGames(list);
	Glk::Alan2::Alan2MetaEngine::getSupportedGames(list);
	Glk::Alan3::Alan3MetaEngine::getSupportedGames(list);
	Glk::Frotz::FrotzMetaEngine::getSupportedGames(list);
	Glk::Glulxe::GlulxeMetaEngine::getSupportedGames(list);
	Glk::Hugo::HugoMetaEngine::getSupportedGames(list);
	Glk::Scott::ScottMetaEngine::getSupportedGames(list);
#ifndef RELEASE_BUILD
	Glk::Magnetic::MagneticMetaEngine::getSupportedGames(list);
	Glk::TADS::TADSMetaEngine::getSupportedGames(list);
#endif

	return list;
}

PlainGameDescriptor GlkMetaEngine::findGame(const char *gameId) const {
	Glk::GameDescriptor gd = Glk::AdvSys::AdvSysMetaEngine::findGame(gameId);
	if (gd._description) return gd;

	gd = Glk::Alan2::Alan2MetaEngine::findGame(gameId);
	if (gd._description) return gd;

	gd = Glk::Alan3::Alan3MetaEngine::findGame(gameId);
	if (gd._description) return gd;

	gd = Glk::Frotz::FrotzMetaEngine::findGame(gameId);
	if (gd._description) return gd;

	gd = Glk::Glulxe::GlulxeMetaEngine::findGame(gameId);
	if (gd._description) return gd;

	gd = Glk::Hugo::HugoMetaEngine::findGame(gameId);
	if (gd._description) return gd;

	gd = Glk::Scott::ScottMetaEngine::findGame(gameId);
	if (gd._description) return gd;

#ifndef RELEASE_BUILD
	gd = Glk::Magnetic::MagneticMetaEngine::findGame(gameId);
	if (gd._description) return gd;

	gd = Glk::TADS::TADSMetaEngine::findGame(gameId);
	if (gd._description) return gd;
#endif
	return PlainGameDescriptor();
}

DetectedGames GlkMetaEngine::detectGames(const Common::FSList &fslist) const {
	// This is as good a place as any to detect multiple sub-engines using the same Ids
	detectClashes();

	DetectedGames detectedGames;
	Glk::AdvSys::AdvSysMetaEngine::detectGames(fslist, detectedGames);
	Glk::Alan2::Alan2MetaEngine::detectGames(fslist, detectedGames);
	Glk::Alan3::Alan3MetaEngine::detectGames(fslist, detectedGames);
	Glk::Frotz::FrotzMetaEngine::detectGames(fslist, detectedGames);
	Glk::Glulxe::GlulxeMetaEngine::detectGames(fslist, detectedGames);
	Glk::Hugo::HugoMetaEngine::detectGames(fslist, detectedGames);
	Glk::Scott::ScottMetaEngine::detectGames(fslist, detectedGames);

#ifndef RELEASE_BUILD
	Glk::Magnetic::MagneticMetaEngine::detectGames(fslist, detectedGames);
	Glk::TADS::TADSMetaEngine::detectGames(fslist, detectedGames);
#endif

	return detectedGames;
}

void GlkMetaEngine::detectClashes() const {
	Common::StringMap map;
	Glk::AdvSys::AdvSysMetaEngine::detectClashes(map);
	Glk::Alan2::Alan2MetaEngine::detectClashes(map);
	Glk::Alan3::Alan3MetaEngine::detectClashes(map);
	Glk::Frotz::FrotzMetaEngine::detectClashes(map);
	Glk::Glulxe::GlulxeMetaEngine::detectClashes(map);
	Glk::Hugo::HugoMetaEngine::detectClashes(map);
	Glk::Scott::ScottMetaEngine::detectClashes(map);

#ifndef RELEASE_BUILD
	Glk::Magnetic::MagneticMetaEngine::detectClashes(map);
	Glk::TADS::TADSMetaEngine::detectClashes(map);
#endif
}

SaveStateList GlkMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot <= MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				Common::String saveName;
				if (Glk::QuetzalReader::getSavegameDescription(in, saveName))
					saveList.push_back(SaveStateDescriptor(slot, saveName));

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
	SaveStateDescriptor ssd;
	bool result = false;

	if (in) {
		result = Glk::QuetzalReader::getSavegameMetaInfo(in, ssd);
		ssd.setSaveSlot(slot);
		delete in;
	}

	if (result)
		return ssd;

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(GLK)
REGISTER_PLUGIN_DYNAMIC(GLK, PLUGIN_TYPE_ENGINE, GlkMetaEngine);
#else
REGISTER_PLUGIN_STATIC(GLK, PLUGIN_TYPE_ENGINE, GlkMetaEngine);
#endif
