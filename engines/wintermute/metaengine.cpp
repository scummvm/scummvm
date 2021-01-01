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

#include "common/achievements.h"

#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/base/base_persistence_manager.h"

#include "engines/wintermute/keymapper_tables.h"
#include "engines/wintermute/achievements_tables.h"

// Detection related files.
#include "wintermute/detection.h"
#include "wintermute/detection_tables.h"

namespace Wintermute {

/**
 * The fallback game descriptor used by the Wintermute engine's fallbackDetector.
 * Contents of this struct are overwritten by the fallbackDetector. (logic copied partially
 * from the SCI-engine).
 */
static ADGameDescription s_fallbackDesc = {
	"",
	"",
	AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
	Common::UNK_LANG,
	Common::kPlatformWindows,
	ADGF_UNSTABLE,
	GUIO0()
};

static char s_fallbackExtraBuf[256];

class WintermuteMetaEngine : public AdvancedMetaEngine {
public:
    const char *getName() const override {
		return "wintermute";
	}

    Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		const WMEGameDescription *gd = (const WMEGameDescription *)desc;

#ifndef ENABLE_FOXTAIL
		if (gd->targetExecutable >= FOXTAIL_OLDEST_VERSION && gd->targetExecutable <= FOXTAIL_LATEST_VERSION) {
			return Common::Error(Common::kUnsupportedGameidError, _s("FoxTail support is not compiled in"));
		}
#endif

#ifndef ENABLE_HEROCRAFT
		if (gd->targetExecutable == WME_HEROCRAFT) {
			return Common::Error(Common::kUnsupportedGameidError, _s("Hero Craft support is not compiled in"));
		}
#endif

#ifndef ENABLE_WME3D
		if (gd->adDesc.flags & GF_3D) {
			return Common::Error(Common::kUnsupportedGameidError, _s("Wintermute3D support is not compiled in"));
		}
#endif

		*engine = new Wintermute::WintermuteEngine(syst, gd);
		return Common::kNoError;
	}

	bool hasFeature(MetaEngineFeature f) const override {
		switch (f) {
		case MetaEngine::kSupportsListSaves:
			return true;
		case MetaEngine::kSupportsLoadingDuringStartup:
			return true;
		case MetaEngine::kSupportsDeleteSave:
			return true;
		case MetaEngine::kSavesSupportCreationDate:
			return true;
		case MetaEngine::kSavesSupportMetaInfo:
			return true;
		case MetaEngine::kSavesSupportThumbnail:
			return true;
		default:
			return false;
		}
	}

	SaveStateList listSaves(const char *target) const override {
		SaveStateList saves;
		Wintermute::BasePersistenceManager pm(target, true);
		for (int i = 0; i < getMaximumSaveSlot(); i++) {
			if (pm.getSaveExists(i)) {
				SaveStateDescriptor desc;
				pm.getSaveStateDesc(i, desc);
				saves.push_back(desc);
			}
		}
		return saves;
	}

	int getMaximumSaveSlot() const override {
		return 100;
	}

	void removeSaveState(const char *target, int slot) const override {
		Wintermute::BasePersistenceManager pm(target, true);
		pm.deleteSaveSlot(slot);
	}

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override {
		Wintermute::BasePersistenceManager pm(target, true);
		SaveStateDescriptor retVal;
		retVal.setDescription("Invalid savegame");
		pm.getSaveStateDesc(slot, retVal);
		return retVal;
	}

    const Common::AchievementsInfo getAchievementsInfo(const Common::String &target) const override {
		Common::String gameId = ConfMan.get("gameid", target);

		// HACK: "juliauntold" is a DLC of "juliastars", they share the same achievements list
		if (gameId == "juliauntold") {
			gameId = "juliastars";
		}

		Common::AchievementsPlatform platform = Common::STEAM_ACHIEVEMENTS;
		if (ConfMan.get("extra", target).contains("GOG")) {
			platform = Common::GALAXY_ACHIEVEMENTS;
		}

		// "(gameId, platform) -> result" search
		Common::AchievementsInfo result;
		for (const AchievementDescriptionList *i = achievementDescriptionList; i->gameId; i++) {
			if (i->gameId == gameId && i->platform == platform) {
				result.platform = i->platform;
				result.appId = i->appId;
				for (const Common::AchievementDescription *it = i->descriptions; it->id; it++) {
					result.descriptions.push_back(*it);
				}
				break;
			}
		}
		return result;
	}

	Common::KeymapArray initKeymaps(const char *target) const override {
		Common::String gameId = ConfMan.get("gameid", target);
		const char *gameDescr = "Unknown WME game";
		for (const PlainGameDescriptor *it = Wintermute::wintermuteGames; it->gameId ; it++ ) {
			if (gameId == it->gameId) {
				gameDescr = it->description;
			}
		}
		return getWintermuteKeymaps(target, gameId, gameDescr);
	}

	ADDetectedGame fallbackDetectExtern(uint md5Bytes, const FileMap &allFiles, const Common::FSList &fslist) const override {
		// Set some defaults
		s_fallbackDesc.extra = "";
		s_fallbackDesc.language = Common::UNK_LANG;
		s_fallbackDesc.flags = ADGF_UNSTABLE;
		s_fallbackDesc.platform = Common::kPlatformWindows; // default to Windows
		s_fallbackDesc.gameId = "wintermute";
		s_fallbackDesc.guiOptions = GUIO0();

		if (!allFiles.contains("data.dcp")) {
			return ADDetectedGame();
		}

		Common::String name, caption;
		if (!WintermuteEngine::getGameInfo(fslist, name, caption)) {
			return ADDetectedGame();
		}

		Common::String extra = caption;
		if (extra.empty()) {
			extra = name;
		}

		if (!extra.empty()) {
			Common::strlcpy(s_fallbackExtraBuf, extra.c_str(), sizeof(s_fallbackExtraBuf) - 1);
			s_fallbackDesc.extra = s_fallbackExtraBuf;
			s_fallbackDesc.flags |= ADGF_USEEXTRAASTITLE;
			s_fallbackDesc.flags |= ADGF_AUTOGENTARGET;
		}

		ADDetectedGame game(&s_fallbackDesc);

		for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			if (file->isDirectory()) continue;
			if (!file->getName().hasSuffixIgnoreCase(".dcp")) continue;

			FileProperties tmp;
			if (AdvancedMetaEngine::getFilePropertiesExtern(md5Bytes, allFiles, s_fallbackDesc, file->getName(), tmp)) {
				game.hasUnknownFiles = true;
				game.matchedFiles[file->getName()] = tmp;
			}
		}

		return game;
	}
};

} // End of namespace Wintermute

#if PLUGIN_ENABLED_DYNAMIC(WINTERMUTE)
	REGISTER_PLUGIN_DYNAMIC(WINTERMUTE, PLUGIN_TYPE_ENGINE, Wintermute::WintermuteMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(WINTERMUTE, PLUGIN_TYPE_ENGINE, Wintermute::WintermuteMetaEngine);
#endif
