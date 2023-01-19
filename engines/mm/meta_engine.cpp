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
#include "backends/keymapper/standard-actions.h"
#include "base/plugins.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "mm/detection.h"
#include "mm/mm1/mm1.h"
#include "mm/xeen/xeen.h"
#include "mm/xeen/meta_engine.h"
#include "mm/xeen/worldofxeen/worldofxeen.h"
#include "mm/xeen/swordsofxeen/swordsofxeen.h"

class MMMetaEngine : public AdvancedMetaEngine {
private:
	/**
	 * Gets the game Id given a target string
	 */
	static Common::String getGameId(const Common::String &target);

	/**
	 * Returns true if the game is a Xeen game
	 */
	static bool isXeenGame(const Common::String &target);
public:
	const char *getName() const override {
		return "mm";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool MMMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSavesUseExtendedFormat) ||
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSimpleSavesNames);
}

bool MM::Xeen::XeenEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error MMMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const MM::MightAndMagicGameDescription *gd = (const MM::MightAndMagicGameDescription *)desc;

	switch (gd->gameID) {
	case MM::GType_MightAndMagic1:
		*engine = new MM::MM1::MM1Engine(syst, gd);
		break;
	case MM::GType_Clouds:
	case MM::GType_DarkSide:
	case MM::GType_WorldOfXeen:
		*engine = new MM::Xeen::WorldOfXeen::WorldOfXeenEngine(syst, gd);
		break;
	case MM::GType_Swords:
		*engine = new MM::Xeen::SwordsOfXeen::SwordsOfXeenEngine(syst, gd);
		break;
	default:
		return Common::kUnsupportedGameidError;
	}

	return Common::kNoError;
}

SaveStateList MMMetaEngine::listSaves(const char *target) const {
	if (isXeenGame(target))
		// Fallback original code for Xeen
		return MM::Xeen::XeenMetaEngine::listSaves(this, target);

	return AdvancedMetaEngine::listSaves(target);
}

SaveStateDescriptor MMMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	if (isXeenGame(target))
		// Fallback original code for Xeen
		return MM::Xeen::XeenMetaEngine::querySaveMetaInfos(this, target, slot);

	return AdvancedMetaEngine::querySaveMetaInfos(target, slot);
}

Common::KeymapArray MMMetaEngine::initKeymaps(const char *target) const {
	const Common::String gameId = getGameId(target);
	if (gameId == "mm1" || gameId == "mm1_enh")
		return MM::MM1::MetaEngine::initKeymaps();

	return Common::KeymapArray();
}

Common::String MMMetaEngine::getGameId(const Common::String &target) {
	// Store a copy of the active domain
	Common::String currDomain = ConfMan.getActiveDomainName();

	// Switch to the given target domain and get it's game Id
	ConfMan.setActiveDomain(target);
	Common::String gameId = ConfMan.get("gameid");

	// Switch back to the original domain and return the game Id
	ConfMan.setActiveDomain(currDomain);
	return gameId;
}

bool MMMetaEngine::isXeenGame(const Common::String &target) {
	Common::String gameId = getGameId(target);
	if (gameId == "mm1" || gameId == "mm1_enh")
		return false;

	return true;
}

#if PLUGIN_ENABLED_DYNAMIC(MM)
	REGISTER_PLUGIN_DYNAMIC(MM, PLUGIN_TYPE_ENGINE, MMMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MM, PLUGIN_TYPE_ENGINE, MMMetaEngine);
#endif
