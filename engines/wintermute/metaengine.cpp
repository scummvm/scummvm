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

#include "common/translation.h"

#include "engines/achievements.h"

#include "wintermute/wintermute.h"
#include "wintermute/base/base_persistence_manager.h"

#include "wintermute/keymapper_tables.h"
#include "wintermute/achievements_tables.h"

// Detection related files.
#include "wintermute/detection.h"
#include "wintermute/detection_tables.h"

namespace Wintermute {

static const ADExtraGuiOptionsMap gameGuiOptions[] = {
	{
		GAMEOPTION_SHOW_FPS,
		{
			_s("Show FPS-counter"),
			_s("Show the current number of frames per second in the upper left corner"),
			"show_fps",
			false,
			0,
			0
		},
	},

	{
		GAMEOPTION_BILINEAR,
		{
			_s("Sprite bilinear filtering (SLOW)"),
			_s("Apply bilinear filtering to individual sprites"),
			"bilinear_filtering",
			false,
			0,
			0
		}
	},

#ifdef ENABLE_WME3D
	{
		GAMEOPTION_FORCE_2D_RENDERER,
		{
			_s("Force to use 2D renderer (2D games only)"),
			_s("This setting forces ScummVM to use 2D renderer while running 2D games"),
			"force_2d_renderer",
			false,
			0,
			0
		}
	},
#endif

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

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

class WintermuteMetaEngine : public AdvancedMetaEngine<WMEGameDescription> {
public:
	const char *getName() const override {
		return "wintermute";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return gameGuiOptions;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const WMEGameDescription *gd) const override {
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
		case kSupportsListSaves:
		case kSupportsLoadingDuringStartup:
		case kSupportsDeleteSave:
		case kSavesSupportCreationDate:
		case kSavesSupportMetaInfo:
		case kSavesSupportThumbnail:
		case kSimpleSavesNames:
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

	bool removeSaveState(const char *target, int slot) const override {
		Wintermute::BasePersistenceManager pm(target, true);
		return pm.deleteSaveSlot(slot);
	}

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override {
		Wintermute::BasePersistenceManager pm(target, true);
		SaveStateDescriptor retVal;
		retVal.setDescription("Invalid savegame");
		pm.getSaveStateDesc(slot, retVal);
		return retVal;
	}

	const Common::AchievementDescriptionList* getAchievementDescriptionList() const override {
		return Wintermute::achievementDescriptionList;
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

	ADDetectedGame fallbackDetectExtern(uint md5Bytes, const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extraInfo) const override {
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
			if (AdvancedMetaEngine::getFilePropertiesExtern(md5Bytes, allFiles, kMD5Head, file->getPathInArchive(), tmp)) {
				game.hasUnknownFiles = true;
				game.matchedFiles[file->getPathInArchive()] = tmp;
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
