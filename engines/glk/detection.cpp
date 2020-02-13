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

#ifdef ENABLE_GLK_ADRIFT
#include "glk/adrift/detection.h"
#include "glk/adrift/adrift.h"
#endif

#ifdef ENABLE_GLK_ADVSYS
#include "glk/advsys/detection.h"
#include "glk/advsys/advsys.h"
#endif

#ifdef ENABLE_GLK_AGT
#include "glk/agt/detection.h"
#include "glk/agt/agt.h"
#endif

#ifdef ENABLE_GLK_ALAN2
#include "glk/alan2/detection.h"
#include "glk/alan2/alan2.h"
#endif

#ifdef ENABLE_GLK_ALAN3
#include "glk/alan3/detection.h"
#include "glk/alan3/alan3.h"
#endif

#ifdef ENABLE_GLK_ARCHETYPE
#include "glk/archetype/archetype.h"
#include "glk/archetype/detection.h"
#endif

#ifdef ENABLE_GLK_FROTZ
#include "glk/frotz/detection.h"
#include "glk/frotz/frotz.h"
#endif

#ifdef ENABLE_GLK_GLULXE
#include "glk/glulxe/detection.h"
#include "glk/glulxe/glulxe.h"
#endif

#ifdef ENABLE_GLK_HUGO
#include "glk/hugo/detection.h"
#include "glk/hugo/hugo.h"
#endif

#ifdef ENABLE_GLK_JACL
#include "glk/jacl/detection.h"
#include "glk/jacl/jacl.h"
#endif

#ifdef ENABLE_GLK_LEVEL9
#include "glk/level9/detection.h"
#include "glk/level9/level9.h"
#endif

#ifdef ENABLE_GLK_MAGNETIC
#include "glk/magnetic/detection.h"
#include "glk/magnetic/magnetic.h"
#endif

#ifdef ENABLE_GLK_QUEST
#include "glk/quest/detection.h"
#include "glk/quest/quest.h"
#endif

#ifdef ENABLE_GLK_SCOTT
#include "glk/scott/detection.h"
#include "glk/scott/scott.h"
#endif

#ifdef ENABLE_GLK_TADS
#include "glk/tads/detection.h"
#include "glk/tads/tads2/tads2.h"
#include "glk/tads/tads3/tads3.h"
#endif

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
		DetectedGame("glk", id, desc, Common::EN_ANY, Common::kPlatformUnknown) {
	setGUIOptions(GUIO3(GUIO_NOSPEECH, GUIO_NOMUSIC, GUIO_NOSUBTITLES));
	addExtraEntry("filename", filename);
}

GlkDetectedGame::GlkDetectedGame(const char *id, const char *desc, const Common::String &filename,
		Common::Language lang) : DetectedGame("glk", id, desc, lang, Common::kPlatformUnknown) {
	setGUIOptions(GUIO3(GUIO_NOSPEECH, GUIO_NOMUSIC, GUIO_NOSUBTITLES));
	addExtraEntry("filename", filename);
}

GlkDetectedGame::GlkDetectedGame(const char *id, const char *desc, const char *xtra,
		const Common::String &filename, Common::Language lang) :
		DetectedGame("glk", id, desc, lang, Common::kPlatformUnknown, xtra) {
	setGUIOptions(GUIO3(GUIO_NOSPEECH, GUIO_NOMUSIC, GUIO_NOSUBTITLES));
	addExtraEntry("filename", filename);
}

GlkDetectedGame::GlkDetectedGame(const char *id, const char *desc, const Common::String &filename,
		const Common::String &md5, size_t filesize) :
		DetectedGame("glk", id, desc, Common::UNK_LANG, Common::kPlatformUnknown) {
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
#ifdef ENABLE_GLK_ADRIFT
	if ((*engine = create<Glk::Adrift::AdriftMetaEngine, Glk::Adrift::Adrift>(syst, gameDesc)) != nullptr) {}
	else
#endif
#ifdef ENABLE_GLK_ADVSYS
	if ((*engine = create<Glk::AdvSys::AdvSysMetaEngine, Glk::AdvSys::AdvSys>(syst, gameDesc)) != nullptr) {}
	else
#endif
#ifdef ENABLE_GLK_AGT
		if ((*engine = create<Glk::AGT::AGTMetaEngine, Glk::AGT::AGT>(syst, gameDesc)) != nullptr) {}
		else
#endif
#ifdef ENABLE_GLK_ALAN2
	if ((*engine = create<Glk::Alan2::Alan2MetaEngine, Glk::Alan2::Alan2>(syst, gameDesc)) != nullptr) {}
	else
#endif
#ifdef ENABLE_GLK_ALAN3
	if ((*engine = create<Glk::Alan3::Alan3MetaEngine, Glk::Alan3::Alan3>(syst, gameDesc)) != nullptr) {}
	else
#endif
#ifdef ENABLE_GLK_ARCHETYPE
	if ((*engine = create<Glk::Archetype::ArchetypeMetaEngine, Glk::Archetype::Archetype>(syst, gameDesc)) != nullptr) {}
	else
#endif
#ifdef ENABLE_GLK_FROTZ
	if ((*engine = create<Glk::Frotz::FrotzMetaEngine, Glk::Frotz::Frotz>(syst, gameDesc)) != nullptr) {}
	else
#endif
#ifdef ENABLE_GLK_GLULXE
	if ((*engine = create<Glk::Glulxe::GlulxeMetaEngine, Glk::Glulxe::Glulxe>(syst, gameDesc)) != nullptr) {}
	else
#endif
#ifdef ENABLE_GLK_HUGO
	if ((*engine = create<Glk::Hugo::HugoMetaEngine, Glk::Hugo::Hugo>(syst, gameDesc)) != nullptr) {}
	else
#endif
#ifdef ENABLE_GLK_JACL
	if ((*engine = create<Glk::JACL::JACLMetaEngine, Glk::JACL::JACL>(syst, gameDesc)) != nullptr) {}
	else
#endif
#ifdef ENABLE_GLK_LEVEL9
		if ((*engine = create<Glk::Level9::Level9MetaEngine, Glk::Level9::Level9>(syst, gameDesc)) != nullptr) {}
		else
#endif
#ifdef ENABLE_GLK_MAGNETIC
	if ((*engine = create<Glk::Magnetic::MagneticMetaEngine, Glk::Magnetic::Magnetic>(syst, gameDesc)) != nullptr) {}
	else
#endif
#ifdef ENABLE_GLK_QUEST
	if ((*engine = create<Glk::Quest::QuestMetaEngine, Glk::Quest::Quest>(syst, gameDesc)) != nullptr) {}
	else
#endif
#ifdef ENABLE_GLK_SCOTT
	if ((*engine = create<Glk::Scott::ScottMetaEngine, Glk::Scott::Scott>(syst, gameDesc)) != nullptr) {}
	else
#endif
#ifdef ENABLE_GLK_TADS
	if ((td = Glk::TADS::TADSMetaEngine::findGame(gameDesc._gameId.c_str()))._description) {
		if (td._options & Glk::TADS::OPTION_TADS3)
			*engine = new Glk::TADS::TADS3::TADS3(syst, gameDesc);
		else
			*engine = new Glk::TADS::TADS2::TADS2(syst, gameDesc);
	} else
#endif
	{
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
#ifdef ENABLE_GLK_ADRIFT
	Glk::Adrift::AdriftMetaEngine::getSupportedGames(list);
#endif
#ifdef ENABLE_GLK_ADVSYS
	Glk::AdvSys::AdvSysMetaEngine::getSupportedGames(list);
#endif
#ifdef ENABLE_GLK_AGT
	Glk::AGT::AGTMetaEngine::getSupportedGames(list);
#endif
#ifdef ENABLE_GLK_ALAN2
	Glk::Alan2::Alan2MetaEngine::getSupportedGames(list);
#endif
#ifdef ENABLE_GLK_ALAN3
	Glk::Alan3::Alan3MetaEngine::getSupportedGames(list);
#endif
#ifdef ENABLE_GLK_ARCHETYPE
	Glk::Archetype::ArchetypeMetaEngine::getSupportedGames(list);
#endif
#ifdef ENABLE_GLK_FROTZ
	Glk::Frotz::FrotzMetaEngine::getSupportedGames(list);
#endif
#ifdef ENABLE_GLK_GLULXE
	Glk::Glulxe::GlulxeMetaEngine::getSupportedGames(list);
#endif
#ifdef ENABLE_GLK_HUGO
	Glk::Hugo::HugoMetaEngine::getSupportedGames(list);
#endif
#ifdef ENABLE_GLK_JACL
	Glk::JACL::JACLMetaEngine::getSupportedGames(list);
#endif
#ifdef ENABLE_GLK_LEVEL9
	Glk::Level9::Level9MetaEngine::getSupportedGames(list);
#endif
#ifdef ENABLE_GLK_MAGNETIC
	Glk::Magnetic::MagneticMetaEngine::getSupportedGames(list);
#endif
#ifdef ENABLE_GLK_QUEST
	Glk::Quest::QuestMetaEngine::getSupportedGames(list);
#endif
#ifdef ENABLE_GLK_SCOTT
	Glk::Scott::ScottMetaEngine::getSupportedGames(list);
#endif
#ifdef ENABLE_GLK_TADS
	Glk::TADS::TADSMetaEngine::getSupportedGames(list);
#endif

	return list;
}

#define FIND_GAME(SUBENGINE) \
	Glk::GameDescriptor gd##SUBENGINE = Glk::SUBENGINE::SUBENGINE##MetaEngine::findGame(gameId); \
	if (gd##SUBENGINE._description) return gd##SUBENGINE

PlainGameDescriptor GlkMetaEngine::findGame(const char *gameId) const {
#ifdef ENABLE_GLK_ADRIFT
	FIND_GAME(Adrift);
#endif
#ifdef ENABLE_GLK_ADVSYS
	FIND_GAME(AdvSys);
#endif
#ifdef ENABLE_GLK_ALAN2
	FIND_GAME(Alan2);
#endif
#ifdef ENABLE_GLK_AGT
	FIND_GAME(AGT);
#endif
#ifdef ENABLE_GLK_ALAN3
	FIND_GAME(Alan3);
#endif
#ifdef ENABLE_GLK_ARCHETYPE
	FIND_GAME(Archetype);
#endif
#ifdef ENABLE_GLK_FROTZ
	FIND_GAME(Frotz);
#endif
#ifdef ENABLE_GLK_GLULXE
	FIND_GAME(Glulxe);
#endif
#ifdef ENABLE_GLK_HUGO
	FIND_GAME(Hugo);
#endif
#ifdef ENABLE_GLK_JACL
	FIND_GAME(JACL);
#endif
#ifdef ENABLE_GLK_LEVEL9
	FIND_GAME(Level9);
#endif
#ifdef ENABLE_GLK_MAGNETIC
	FIND_GAME(Magnetic);
#endif
#ifdef ENABLE_GLK_QUEST
	FIND_GAME(Quest);
#endif
#ifdef ENABLE_GLK_SCOTT
	FIND_GAME(Scott);
#endif
#ifdef ENABLE_GLK_TADS
	FIND_GAME(TADS);
#endif

	return PlainGameDescriptor();
}

#undef FIND_GAME

DetectedGames GlkMetaEngine::detectGames(const Common::FSList &fslist) const {
	// This is as good a place as any to detect multiple sub-engines using the same Ids
	detectClashes();

	DetectedGames detectedGames;
#ifdef ENABLE_GLK_ADRIFT
	Glk::Adrift::AdriftMetaEngine::detectGames(fslist, detectedGames);
#endif
#ifdef ENABLE_GLK_ADVSYS
	Glk::AdvSys::AdvSysMetaEngine::detectGames(fslist, detectedGames);
#endif
#ifdef ENABLE_GLK_AGT
	Glk::AGT::AGTMetaEngine::detectGames(fslist, detectedGames);
#endif
#ifdef ENABLE_GLK_ALAN2
	Glk::Alan2::Alan2MetaEngine::detectGames(fslist, detectedGames);
#endif
#ifdef ENABLE_GLK_ALAN3
	Glk::Alan3::Alan3MetaEngine::detectGames(fslist, detectedGames);
#endif
#ifdef ENABLE_GLK_ARCHETYPE
	Glk::Archetype::ArchetypeMetaEngine::detectGames(fslist, detectedGames);
#endif
#ifdef ENABLE_GLK_FROTZ
	Glk::Frotz::FrotzMetaEngine::detectGames(fslist, detectedGames);
#endif
#ifdef ENABLE_GLK_GLULXE
	Glk::Glulxe::GlulxeMetaEngine::detectGames(fslist, detectedGames);
#endif
#ifdef ENABLE_GLK_HUGO
	Glk::Hugo::HugoMetaEngine::detectGames(fslist, detectedGames);
#endif
#ifdef ENABLE_GLK_JACL
	Glk::JACL::JACLMetaEngine::detectGames(fslist, detectedGames);
#endif
#ifdef ENABLE_GLK_LEVEL9
	Glk::Level9::Level9MetaEngine::detectGames(fslist, detectedGames);
#endif
#ifdef ENABLE_GLK_MAGNETIC
	Glk::Magnetic::MagneticMetaEngine::detectGames(fslist, detectedGames);
#endif
#ifdef ENABLE_GLK_QUEST
	Glk::Quest::QuestMetaEngine::detectGames(fslist, detectedGames);
#endif
#ifdef ENABLE_GLK_SCOTT
	Glk::Scott::ScottMetaEngine::detectGames(fslist, detectedGames);
#endif
#ifdef ENABLE_GLK_TADS
	Glk::TADS::TADSMetaEngine::detectGames(fslist, detectedGames);
#endif

	return detectedGames;
}

void GlkMetaEngine::detectClashes() const {
	Common::StringMap map;
#ifdef ENABLE_GLK_ADRIFT
	Glk::Adrift::AdriftMetaEngine::detectClashes(map);
#endif
#ifdef ENABLE_GLK_ADVSYS
	Glk::AdvSys::AdvSysMetaEngine::detectClashes(map);
#endif
#ifdef ENABLE_GLK_AGT
	Glk::AGT::AGTMetaEngine::detectClashes(map);
#endif
#ifdef ENABLE_GLK_ALAN2
	Glk::Alan2::Alan2MetaEngine::detectClashes(map);
#endif
#ifdef ENABLE_GLK_ALAN3
	Glk::Alan3::Alan3MetaEngine::detectClashes(map);
#endif
#ifdef ENABLE_GLK_ARCHETYPE
	Glk::Archetype::ArchetypeMetaEngine::detectClashes(map);
#endif
#ifdef ENABLE_GLK_FROTZ
	Glk::Frotz::FrotzMetaEngine::detectClashes(map);
#endif
#ifdef ENABLE_GLK_GLULXE
	Glk::Glulxe::GlulxeMetaEngine::detectClashes(map);
#endif
#ifdef ENABLE_GLK_HUGO
	Glk::Hugo::HugoMetaEngine::detectClashes(map);
#endif
#ifdef ENABLE_GLK_JACL
	Glk::JACL::JACLMetaEngine::detectClashes(map);
#endif
#ifdef ENABLE_GLK_LEVEL9
	Glk::Level9::Level9MetaEngine::detectClashes(map);
#endif
#ifdef ENABLE_GLK_MAGNETIC
	Glk::Magnetic::MagneticMetaEngine::detectClashes(map);
#endif
#ifdef ENABLE_GLK_QUEST
	Glk::Quest::QuestMetaEngine::detectClashes(map);
#endif
#ifdef ENABLE_GLK_SCOTT
	Glk::Scott::ScottMetaEngine::detectClashes(map);
#endif
#ifdef ENABLE_GLK_TADS
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
