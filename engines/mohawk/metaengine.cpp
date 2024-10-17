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

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"

#include "mohawk/mohawk.h"
#include "mohawk/dialogs.h"
#include "mohawk/livingbooks.h"
#include "mohawk/riven_metaengine.h"
#include "mohawk/myst_metaengine.h"

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

#include "mohawk/detection.h"

namespace Mohawk {

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

Common::Path MohawkEngine_LivingBooks::getBookInfoFileName() const {
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

class MohawkMetaEngine : public AdvancedMetaEngine<Mohawk::MohawkGameDescription> {
public:
	const char *getName() const override {
		return "mohawk";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const Mohawk::MohawkGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	SaveStateList listSavesForPrefix(const char *prefix, const char *extension) const;
	int getMaximumSaveSlot() const override { return 999; }
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;


	void registerDefaultSettings(const Common::String &target) const override;
	GUI::OptionsContainerWidget *buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;
	Common::String getSavegameFile(int saveGameIdx, const char *target) const override {
		if (!target)
			target = getName();
		Common::String gameId = ConfMan.get("gameid", target);
		const char *suffix;
		// Saved games are only supported in Myst/Riven currently.
		if (gameId == "myst")
			suffix = "mys";
		else if (gameId == "riven")
			suffix = "rvn";
		else
			return MetaEngine::getSavegameFile(saveGameIdx, target);

		if (saveGameIdx == kSavegameFilePattern)
			return Common::String::format("%s-###.%s", gameId.c_str(), suffix);
		else
			return Common::String::format("%s-%03d.%s", gameId.c_str(), saveGameIdx, suffix);
	}
};

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

		saveList.push_back(SaveStateDescriptor(this, slotNum, ""));
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

bool MohawkMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String gameId = ConfMan.get("gameid", target);

	// Removing saved games is only supported in Myst/Riven currently.
#ifdef ENABLE_MYST
	if (gameId == "myst") {
		return Mohawk::MystGameState::deleteSave(slot);
	}
#endif
#ifdef ENABLE_RIVEN
	if (gameId == "riven") {
		return Mohawk::RivenSaveLoad::deleteSave(slot);
	}
#endif
	return false;
}

SaveStateDescriptor MohawkMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String gameId = ConfMan.get("gameid", target);

#ifdef ENABLE_MYST
	if (gameId == "myst") {
		return Mohawk::MystGameState::querySaveMetaInfos(this, slot);
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

Common::Error MohawkMetaEngine::createInstance(OSystem *syst, Engine **engine, const Mohawk::MohawkGameDescription *gd) const {
	switch (gd->gameType) {
	case Mohawk::GType_MYST:
	case Mohawk::GType_MAKINGOF:
#ifdef ENABLE_MYST
#ifndef ENABLE_MYSTME
		if (gd->features & Mohawk::GF_ME)
			return Common::Error(Common::kUnsupportedGameidError, _s("Myst ME support not compiled in"));
#endif
		*engine = new Mohawk::MohawkEngine_Myst(syst, gd);
		break;
#else
		return Common::Error(Common::kUnsupportedGameidError, _s("Myst support not compiled in"));
#endif
	case Mohawk::GType_RIVEN:
#ifdef ENABLE_RIVEN
		*engine = new Mohawk::MohawkEngine_Riven(syst, gd);
		break;
#else
		return Common::Error(Common::kUnsupportedGameidError, _s("Riven support not compiled in"));
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
		return Common::Error(Common::kUnsupportedGameidError, _s("CSTime support not compiled in"));
#endif
	default:
		return Common::kUnsupportedGameidError;
	}

	return Common::kNoError;
}

void MohawkMetaEngine::registerDefaultSettings(const Common::String &target) const {
	Common::String gameId = ConfMan.get("gameid", target);

	if (gameId == "myst" || gameId == "makingofmyst") {
		return Mohawk::MohawkMetaEngine_Myst::registerDefaultSettings();
	}

	if (gameId == "riven") {
		return Mohawk::MohawkMetaEngine_Riven::registerDefaultSettings();
	}

	return MetaEngine::registerDefaultSettings(target);
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

	return MetaEngine::buildEngineOptionsWidget(boss, name, target);
}

#if PLUGIN_ENABLED_DYNAMIC(MOHAWK)
	REGISTER_PLUGIN_DYNAMIC(MOHAWK, PLUGIN_TYPE_ENGINE, MohawkMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MOHAWK, PLUGIN_TYPE_ENGINE, MohawkMetaEngine);
#endif
