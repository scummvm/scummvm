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

#include "base/plugins.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"

#include "engines/advancedDetector.h"
#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "mohawk/dialogs.h"
#include "mohawk/livingbooks.h"

#ifdef ENABLE_CSTIME
#include "mohawk/cstime.h"
#endif

#ifdef ENABLE_MYST
#include "mohawk/myst.h"
#include "mohawk/myst_state.h"
#endif

#ifdef ENABLE_MYSTME
#ifndef ENABLE_MYST
#error "Myst must be enabled for building Myst ME. Specify --enable-engine=myst,mystme"
#endif
#endif

#ifdef ENABLE_RIVEN
#include "mohawk/riven.h"
#include "mohawk/riven_saveload.h"
#endif

namespace Mohawk {

struct MohawkGameDescription {
	ADGameDescription desc;

	uint8 gameType;
	uint32 features;
	const char *appName;
};

const char* MohawkEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

uint32 MohawkEngine::getFeatures() const {
	return _gameDescription->features;
}

bool MohawkEngine::isGameVariant(MohawkGameFeatures feature) const {
	return (_gameDescription->features & feature) != 0;
}

Common::Platform MohawkEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

const char *MohawkEngine::getAppName() const {
	return _gameDescription->appName;
}

uint8 MohawkEngine::getGameType() const {
	return _gameDescription->gameType;
}

Common::String MohawkEngine_LivingBooks::getBookInfoFileName() const {
	return _gameDescription->desc.filesDescriptions[0].fileName;
}

Common::Language MohawkEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

bool MohawkEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher);
}

#ifdef ENABLE_MYST

bool MohawkEngine_Myst::hasFeature(EngineFeature f) const {
	return
		MohawkEngine::hasFeature(f)
	        || (f == kSupportsLoadingDuringRuntime)
	        || (f == kSupportsSavingDuringRuntime)
	        || (f == kSupportsChangingOptionsDuringRuntime);
}

#endif

#ifdef ENABLE_RIVEN

bool MohawkEngine_Riven::hasFeature(EngineFeature f) const {
	return
		MohawkEngine::hasFeature(f)
	        || (f == kSupportsLoadingDuringRuntime)
	        || (f == kSupportsSavingDuringRuntime)
	        || (f == kSupportsChangingOptionsDuringRuntime);
}

#endif

} // End of Namespace Mohawk

static const PlainGameDescriptor mohawkGames[] = {
	{"myst", "Myst"},
	{"makingofmyst", "The Making of Myst"},
	{"riven", "Riven: The Sequel to Myst"},
	{"cstime", "Where in Time is Carmen Sandiego?"},
	{"carmentq", "Carmen Sandiego's ThinkQuick Challenge"},
	{"carmentqc", "Carmen Sandiego's ThinkQuick Challenge Custom Question Creator"},
	{"maggiesfa", "Maggie's Farmyard Adventure"},
	{"greeneggs", "Green Eggs and Ham"},
	{"seussabc", "Dr Seuss's ABC"},
	{"tortoise", "Aesop's Fables: The Tortoise and the Hare"},
	{"arthur", "Arthur's Teacher Trouble"},
	{"grandma", "Just Grandma and Me"},
	{"ruff", "Ruff's Bone"},
	{"newkid", "The New Kid on the Block"},
	{"arthurrace", "Arthur's Reading Race"},
	{"arthurbday", "Arthur's Birthday"},
	{"lilmonster", "Little Monster at School"},
	{"catinthehat", "The Cat in the Hat"},
	{"rugrats", "Rugrats Adventure Game"},
	{"lbsampler", "Living Books Sampler"},
	{"bearfight", "The Berenstain Bears Get in a Fight"},
	{"beardark", "The Berenstain Bears In The Dark"},
	{"arthurcomp", "Arthur's Computer Adventure"},
	{"harryhh","Harry and the Haunted House"},
	{"stellaluna", "Stellaluna"},
	{"sheila", "Sheila Rae, the Brave"},
	{"rugratsps", "Rugrats Print Shop" },
	{nullptr, nullptr}
};

#include "mohawk/detection_tables.h"

static const char *directoryGlobs[] = {
	"all",
	"assets1",
	"data",
	"program",
	"95instal",
	"Rugrats Adventure Game",
	nullptr
};

class MohawkMetaEngine : public AdvancedMetaEngine {
public:
	MohawkMetaEngine() : AdvancedMetaEngine(Mohawk::gameDescriptions, sizeof(Mohawk::MohawkGameDescription), mohawkGames) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const override {
		return detectGameFilebased(allFiles, fslist, Mohawk::fileBased);
	}

	const char *getEngineId() const override {
		return "mohawk";
	}

	const char *getName() const override {
		return "Mohawk";
	}

	const char *getOriginalCopyright() const override {
		return "Myst and Riven (C) Cyan Worlds\nMohawk OS (C) Ubisoft";
	}

	DetectedGame toDetectedGame(const ADDetectedGame &adGame) const override;

	bool hasFeature(MetaEngineFeature f) const override;
	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateList listSavesForPrefix(const char *prefix, const char *extension) const;
	int getMaximumSaveSlot() const override { return 999; }
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
	void registerDefaultSettings(const Common::String &target) const override;
	GUI::OptionsContainerWidget *buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;
};

DetectedGame MohawkMetaEngine::toDetectedGame(const ADDetectedGame &adGame) const {
	DetectedGame game = AdvancedMetaEngine::toDetectedGame(adGame);

	// The AdvancedDetector model only allows specifying a single supported
	// game language. The 25th anniversary edition Myst games are multilanguage.
	// Here we amend the detected games to set the list of supported languages.
#ifdef ENABLE_MYST
	if (game.gameId == "myst"
			&& Common::checkGameGUIOption(GAMEOPTION_25TH, game.getGUIOptions())
			&& Common::checkGameGUIOption(GAMEOPTION_ME, game.getGUIOptions())) {
		const Mohawk::MystLanguage *languages = Mohawk::MohawkEngine_Myst::listLanguages();
		while (languages->language != Common::UNK_LANG) {
			game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(languages->language));
			languages++;
		}
	}
#endif

#ifdef ENABLE_RIVEN
	if (game.gameId == "riven"
			&& Common::checkGameGUIOption(GAMEOPTION_25TH, game.getGUIOptions())) {
		const Mohawk::RivenLanguage *languages = Mohawk::MohawkEngine_Riven::listLanguages();
		while (languages->language != Common::UNK_LANG) {
			game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(languages->language));
			languages++;
		}
	}
#endif

	return game;
}

bool MohawkMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves)
		|| (f == kSupportsLoadingDuringStartup)
		|| (f == kSupportsDeleteSave)
		|| (f == kSavesSupportMetaInfo)
		|| (f == kSavesSupportThumbnail)
		|| (f == kSavesSupportCreationDate)
		|| (f == kSavesSupportPlayTime);
}

SaveStateList MohawkMetaEngine::listSavesForPrefix(const char *prefix, const char *extension) const {
	Common::String pattern = Common::String::format("%s-###.%s", prefix, extension);
	Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles(pattern);
	size_t prefixLen = strlen(prefix);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
		// Extract the slot number from the filename
		char slot[4];
		slot[0] = (*filename)[prefixLen + 1];
		slot[1] = (*filename)[prefixLen + 2];
		slot[2] = (*filename)[prefixLen + 3];
		slot[3] = '\0';

		int slotNum = atoi(slot);

		saveList.push_back(SaveStateDescriptor(slotNum, ""));
	}

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());

	return saveList;
}

SaveStateList MohawkMetaEngine::listSaves(const char *target) const {
	Common::String gameId = ConfMan.get("gameid", target);
	SaveStateList saveList;

	// Loading games is only supported in Myst/Riven currently.
#ifdef ENABLE_MYST
	if (gameId == "myst") {
		saveList = listSavesForPrefix("myst", "mys");

		for (SaveStateList::iterator save = saveList.begin(); save != saveList.end(); ++save) {
			// Read the description from the save
			int slot = save->getSaveSlot();
			Common::String description = Mohawk::MystGameState::querySaveDescription(slot);
			save->setDescription(description);
		}
	}
#endif
#ifdef ENABLE_RIVEN
	if (gameId == "riven") {
		saveList = listSavesForPrefix("riven", "rvn");

		for (SaveStateList::iterator save = saveList.begin(); save != saveList.end(); ++save) {
			// Read the description from the save
			int slot = save->getSaveSlot();
			Common::String description = Mohawk::RivenSaveLoad::querySaveDescription(slot);
			save->setDescription(description);
		}
	}
#endif

	return saveList;
}

void MohawkMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String gameId = ConfMan.get("gameid", target);

	// Removing saved games is only supported in Myst/Riven currently.
#ifdef ENABLE_MYST
	if (gameId == "myst") {
		Mohawk::MystGameState::deleteSave(slot);
	}
#endif
#ifdef ENABLE_RIVEN
	if (gameId == "riven") {
		Mohawk::RivenSaveLoad::deleteSave(slot);
	}
#endif
}

SaveStateDescriptor MohawkMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String gameId = ConfMan.get("gameid", target);

#ifdef ENABLE_MYST
	if (gameId == "myst") {
		return Mohawk::MystGameState::querySaveMetaInfos(slot);
	}
#endif
#ifdef ENABLE_RIVEN
	if (gameId == "riven") {
		return Mohawk::RivenSaveLoad::querySaveMetaInfos(slot);
	} else
#endif
	{
		return SaveStateDescriptor();
	}
}

Common::KeymapArray MohawkMetaEngine::initKeymaps(const char *target) const {
	Common::String gameId = ConfMan.get("gameid", target);

#ifdef ENABLE_MYST
	if (gameId == "myst" || gameId == "makingofmyst") {
		return Mohawk::MohawkEngine_Myst::initKeymaps(target);
	}
#endif
#ifdef ENABLE_RIVEN
	if (gameId == "riven") {
		return Mohawk::MohawkEngine_Riven::initKeymaps(target);
	}
#endif

	return AdvancedMetaEngine::initKeymaps(target);
}

void MohawkMetaEngine::registerDefaultSettings(const Common::String &target) const {
	Common::String gameId = ConfMan.get("gameid", target);

#ifdef ENABLE_MYST
	if (gameId == "myst" || gameId == "makingofmyst") {
		return Mohawk::MohawkEngine_Myst::registerDefaultSettings();
	}
#endif
#ifdef ENABLE_RIVEN
	if (gameId == "riven") {
		return Mohawk::MohawkEngine_Riven::registerDefaultSettings();
	}
#endif

	return AdvancedMetaEngine::registerDefaultSettings(target);
}

GUI::OptionsContainerWidget *MohawkMetaEngine::buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	Common::String gameId = ConfMan.get("gameid", target);

#ifdef ENABLE_MYST
	if (gameId == "myst" || gameId == "makingofmyst") {
		return new Mohawk::MystOptionsWidget(boss, name, target);
	}
#endif
#ifdef ENABLE_RIVEN
	if (gameId == "riven") {
		return new Mohawk::RivenOptionsWidget(boss, name, target);
	}
#endif

	return AdvancedMetaEngine::buildEngineOptionsWidget(boss, name, target);
}

bool MohawkMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Mohawk::MohawkGameDescription *gd = (const Mohawk::MohawkGameDescription *)desc;

	if (gd) {
		switch (gd->gameType) {
		case Mohawk::GType_MYST:
		case Mohawk::GType_MAKINGOF:
#ifdef ENABLE_MYST
#ifndef ENABLE_MYSTME
			if (gd->features & Mohawk::GF_ME) {
				warning("Myst ME support not compiled in");
				return false;
			}
#endif
			*engine = new Mohawk::MohawkEngine_Myst(syst, gd);
			break;
#else
			warning("Myst support not compiled in");
			return false;
#endif
		case Mohawk::GType_RIVEN:
#ifdef ENABLE_RIVEN
			*engine = new Mohawk::MohawkEngine_Riven(syst, gd);
			break;
#else
			warning("Riven support not compiled in");
			return false;
#endif
		case Mohawk::GType_LIVINGBOOKSV1:
		case Mohawk::GType_LIVINGBOOKSV2:
		case Mohawk::GType_LIVINGBOOKSV3:
		case Mohawk::GType_LIVINGBOOKSV4:
		case Mohawk::GType_LIVINGBOOKSV5:
			*engine = new Mohawk::MohawkEngine_LivingBooks(syst, gd);
			break;
		case Mohawk::GType_CSTIME:
#ifdef ENABLE_CSTIME
			*engine = new Mohawk::MohawkEngine_CSTime(syst, gd);
			break;
#else
			warning("CSTime support not compiled in");
			return false;
#endif
		default:
			error("Unknown Mohawk Engine");
		}
	}

	return (gd != nullptr);
}

#if PLUGIN_ENABLED_DYNAMIC(MOHAWK)
	REGISTER_PLUGIN_DYNAMIC(MOHAWK, PLUGIN_TYPE_ENGINE, MohawkMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MOHAWK, PLUGIN_TYPE_ENGINE, MohawkMetaEngine);
#endif
