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
#include "glk/adrift/detection.h"
#include "glk/adrift/adrift.h"
#include "glk/advsys/detection.h"
#include "glk/advsys/advsys.h"
#include "glk/agt/detection.h"
#include "glk/agt/agt.h"
#include "glk/alan2/detection.h"
#include "glk/alan2/alan2.h"
#include "glk/alan3/detection.h"
#include "glk/alan3/alan3.h"
#include "glk/archetype/archetype.h"
#include "glk/archetype/detection.h"
#include "glk/zcode/detection.h"
#include "glk/zcode/zcode.h"
#include "glk/hugo/detection.h"
#include "glk/hugo/hugo.h"
#include "glk/jacl/detection.h"
#include "glk/jacl/jacl.h"
#include "glk/level9/detection.h"
#include "glk/level9/level9.h"
#include "glk/magnetic/detection.h"
#include "glk/magnetic/magnetic.h"
#include "glk/quest/detection.h"
#include "glk/quest/quest.h"
#include "glk/scott/detection.h"
#include "glk/scott/scott.h"

#ifndef RELEASE_BUILD
#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/detection.h"
#include "glk/glulx/detection.h"
#include "glk/glulx/glulx.h"
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
#include "graphics/surface.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/translation.h"

namespace Glk {

Common::String GlkDetectedGame::getGlkGUIOptions() {
#if defined (USE_TTS)
	return GUIO2(GUIO_NOMUSIC, GUIO_NOSUBTITLES);
#else
	return GUIO3(GUIO_NOSPEECH, GUIO_NOMUSIC, GUIO_NOSUBTITLES);
#endif
}

GlkDetectedGame::GlkDetectedGame(const char *id, const char *desc, const Common::String &filename,
		GameSupportLevel supportLevel) :
		DetectedGame("glk", id, desc, Common::EN_ANY, Common::kPlatformUnknown) {
	setGUIOptions(getGlkGUIOptions());
	gameSupportLevel = supportLevel;
	addExtraEntry("filename", filename);
}

GlkDetectedGame::GlkDetectedGame(const char *id, const char *desc, const Common::String &filename,
		Common::Language lang, GameSupportLevel supportLevel) : DetectedGame("glk", id, desc, lang, Common::kPlatformUnknown) {
	setGUIOptions(getGlkGUIOptions());
	gameSupportLevel = supportLevel;
	addExtraEntry("filename", filename);
}

GlkDetectedGame::GlkDetectedGame(const char *id, const char *desc, const char *xtra,
		const Common::String &filename, Common::Language lang,
		GameSupportLevel supportLevel) :
		DetectedGame("glk", id, desc, lang, Common::kPlatformUnknown, xtra) {
	setGUIOptions(getGlkGUIOptions());
	gameSupportLevel = supportLevel;
	addExtraEntry("filename", filename);
}

GlkDetectedGame::GlkDetectedGame(const char *id, const char *desc, const Common::String &filename,
		const Common::String &md5, size_t filesize, GameSupportLevel supportLevel) :
		DetectedGame("glk", id, desc, Common::UNK_LANG, Common::kPlatformUnknown) {
	setGUIOptions(getGlkGUIOptions());
	gameSupportLevel = supportLevel;
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
	    (f == kSupportsReturnToLauncher) ||
	    (f == kSupportsLoadingDuringRuntime) ||
	    (f == kSupportsSavingDuringRuntime);
}

bool isGameAllowed(GameSupportLevel supportLevel) {
	bool showTestingWarning = false;
#ifdef RELEASE_BUILD
	showTestingWarning = true;
#endif

	if (((supportLevel == kUnstableGame
		|| (supportLevel == kTestingGame && showTestingWarning)))
		&& !Engine::warnUserAboutUnsupportedGame())
		return false;

	return true;
}

template<class META, class ENG>bool create(OSystem *syst,
		Glk::GlkGameDescription &gameDesc, Engine *&engine) {

	Glk::GameDescriptor gd = META::findGame(gameDesc._gameId.c_str());
	if (gd._description) {
		if (!isGameAllowed(gd._supportLevel))
			return true;

		gameDesc._options = gd._options;
		engine = new ENG(syst, gameDesc);
		return true;
	} else {
		return false;
	}
}

Common::Error GlkMetaEngine::createInstance(OSystem *syst, Engine **engine) const {
#ifndef RELEASE_BUILD
	Glk::GameDescriptor td = Glk::GameDescriptor::empty();
#endif
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
	if ((create<Glk::Adrift::AdriftMetaEngine, Glk::Adrift::Adrift>(syst, gameDesc, *engine))) {}
	else if ((create<Glk::AdvSys::AdvSysMetaEngine, Glk::AdvSys::AdvSys>(syst, gameDesc, *engine))) {}
	else if ((create<Glk::AGT::AGTMetaEngine, Glk::AGT::AGT>(syst, gameDesc, *engine))) {}
	else if ((create<Glk::Alan2::Alan2MetaEngine, Glk::Alan2::Alan2>(syst, gameDesc, *engine))) {}
	else if ((create<Glk::Alan3::Alan3MetaEngine, Glk::Alan3::Alan3>(syst, gameDesc, *engine))) {}
	else if ((create<Glk::Archetype::ArchetypeMetaEngine, Glk::Archetype::Archetype>(syst, gameDesc, *engine))) {}
	else if ((create<Glk::Hugo::HugoMetaEngine, Glk::Hugo::Hugo>(syst, gameDesc, *engine))) {}
	else if ((create<Glk::JACL::JACLMetaEngine, Glk::JACL::JACL>(syst, gameDesc, *engine))) {}
	else if ((create<Glk::Level9::Level9MetaEngine, Glk::Level9::Level9>(syst, gameDesc, *engine))) {}
	else if ((create<Glk::Magnetic::MagneticMetaEngine, Glk::Magnetic::Magnetic>(syst, gameDesc, *engine))) {}
	else if ((create<Glk::Quest::QuestMetaEngine, Glk::Quest::Quest>(syst, gameDesc, *engine))) {}
	else if ((create<Glk::Scott::ScottMetaEngine, Glk::Scott::Scott>(syst, gameDesc, *engine))) {}
	else if ((create<Glk::ZCode::ZCodeMetaEngine, Glk::ZCode::ZCode>(syst, gameDesc, *engine))) {}
#ifndef RELEASE_BUILD
	else if ((create<Glk::Comprehend::ComprehendMetaEngine, Glk::Comprehend::Comprehend>(syst, gameDesc, *engine))) {}
	else if ((create<Glk::Glulx::GlulxMetaEngine, Glk::Glulx::Glulx>(syst, gameDesc, *engine))) {}
	else if ((td = Glk::TADS::TADSMetaEngine::findGame(gameDesc._gameId.c_str()))._description) {
		if (!isGameAllowed(td._supportLevel))
			return Common::kUserCanceled;
		else if (td._options & Glk::TADS::OPTION_TADS3)
			new Glk::TADS::TADS3::TADS3(syst, gameDesc);
		else
			new Glk::TADS::TADS2::TADS2(syst, gameDesc);
	}
#endif
	else {
		return Common::kNoGameDataFoundError;
	}

	return *engine ? Common::kNoError : Common::kUserCanceled;
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
	Glk::Adrift::AdriftMetaEngine::getSupportedGames(list);
	Glk::AdvSys::AdvSysMetaEngine::getSupportedGames(list);
	Glk::AGT::AGTMetaEngine::getSupportedGames(list);
	Glk::Alan2::Alan2MetaEngine::getSupportedGames(list);
	Glk::Alan3::Alan3MetaEngine::getSupportedGames(list);
	Glk::Archetype::ArchetypeMetaEngine::getSupportedGames(list);
	Glk::Hugo::HugoMetaEngine::getSupportedGames(list);
	Glk::JACL::JACLMetaEngine::getSupportedGames(list);
	Glk::Level9::Level9MetaEngine::getSupportedGames(list);
	Glk::Magnetic::MagneticMetaEngine::getSupportedGames(list);
	Glk::Quest::QuestMetaEngine::getSupportedGames(list);
	Glk::Scott::ScottMetaEngine::getSupportedGames(list);
	Glk::ZCode::ZCodeMetaEngine::getSupportedGames(list);
#ifndef RELEASE_BUILD
	Glk::Comprehend::ComprehendMetaEngine::getSupportedGames(list);
	Glk::Glulx::GlulxMetaEngine::getSupportedGames(list);
	Glk::TADS::TADSMetaEngine::getSupportedGames(list);
#endif

	return list;
}

#define FIND_GAME(SUBENGINE) \
	Glk::GameDescriptor gd##SUBENGINE = Glk::SUBENGINE::SUBENGINE##MetaEngine::findGame(gameId); \
	if (gd##SUBENGINE._description) return gd##SUBENGINE

PlainGameDescriptor GlkMetaEngine::findGame(const char *gameId) const {
	FIND_GAME(Adrift);
	FIND_GAME(AdvSys);
	FIND_GAME(Alan2);
	FIND_GAME(AGT);
	FIND_GAME(Alan3);
	FIND_GAME(Archetype);
	FIND_GAME(Hugo);
	FIND_GAME(JACL);
	FIND_GAME(Level9);
	FIND_GAME(Magnetic);
	FIND_GAME(Quest);
	FIND_GAME(Scott);
	FIND_GAME(ZCode);
#ifndef RELEASE_BUILD
	FIND_GAME(Comprehend);
	FIND_GAME(Glulx);
	FIND_GAME(TADS);
#endif

	return PlainGameDescriptor();
}

#undef FIND_GAME

DetectedGames GlkMetaEngine::detectGames(const Common::FSList &fslist) const {
	// This is as good a place as any to detect multiple sub-engines using the same Ids
	detectClashes();

	DetectedGames detectedGames;
	Glk::Adrift::AdriftMetaEngine::detectGames(fslist, detectedGames);
	Glk::AdvSys::AdvSysMetaEngine::detectGames(fslist, detectedGames);
	Glk::AGT::AGTMetaEngine::detectGames(fslist, detectedGames);
	Glk::Alan2::Alan2MetaEngine::detectGames(fslist, detectedGames);
	Glk::Alan3::Alan3MetaEngine::detectGames(fslist, detectedGames);
	Glk::Archetype::ArchetypeMetaEngine::detectGames(fslist, detectedGames);
	Glk::Hugo::HugoMetaEngine::detectGames(fslist, detectedGames);
	Glk::JACL::JACLMetaEngine::detectGames(fslist, detectedGames);
	Glk::Level9::Level9MetaEngine::detectGames(fslist, detectedGames);
	Glk::Magnetic::MagneticMetaEngine::detectGames(fslist, detectedGames);
	Glk::Quest::QuestMetaEngine::detectGames(fslist, detectedGames);
	Glk::Scott::ScottMetaEngine::detectGames(fslist, detectedGames);
	Glk::ZCode::ZCodeMetaEngine::detectGames(fslist, detectedGames);
#ifndef RELEASE_BUILD
	Glk::Comprehend::ComprehendMetaEngine::detectGames(fslist, detectedGames);
	Glk::Glulx::GlulxMetaEngine::detectGames(fslist, detectedGames);
	Glk::TADS::TADSMetaEngine::detectGames(fslist, detectedGames);
#endif

	return detectedGames;
}

void GlkMetaEngine::detectClashes() const {
	Common::StringMap map;
	Glk::Adrift::AdriftMetaEngine::detectClashes(map);
	Glk::AdvSys::AdvSysMetaEngine::detectClashes(map);
	Glk::AGT::AGTMetaEngine::detectClashes(map);
	Glk::Alan2::Alan2MetaEngine::detectClashes(map);
	Glk::Alan3::Alan3MetaEngine::detectClashes(map);
	Glk::Archetype::ArchetypeMetaEngine::detectClashes(map);
	Glk::Hugo::HugoMetaEngine::detectClashes(map);
	Glk::JACL::JACLMetaEngine::detectClashes(map);
	Glk::Level9::Level9MetaEngine::detectClashes(map);
	Glk::Magnetic::MagneticMetaEngine::detectClashes(map);
	Glk::Quest::QuestMetaEngine::detectClashes(map);
	Glk::Scott::ScottMetaEngine::detectClashes(map);
	Glk::ZCode::ZCodeMetaEngine::detectClashes(map);
#ifndef RELEASE_BUILD
	Glk::Comprehend::ComprehendMetaEngine::detectClashes(map);
	Glk::Glulx::GlulxMetaEngine::detectClashes(map);
	Glk::TADS::TADSMetaEngine::detectClashes(map);
#endif
}

const ExtraGuiOptions GlkMetaEngine::getExtraGuiOptions(const Common::String &) const {
	ExtraGuiOptions  options;
#if defined(USE_TTS)
	static const ExtraGuiOption ttsSpeakOptions = {
		_s("Enable Text to Speech"),
		_s("Use TTS to read the text"),
		"speak",
		false
	};
	static const ExtraGuiOption ttsSpeakInputOptions = {
		_s("Also read input text"),
		_s("Use TTS to read the input text"),
		"speak_input",
		false
	};
	options.push_back(ttsSpeakOptions);
	options.push_back(ttsSpeakInputOptions);
#endif
	return options;
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
