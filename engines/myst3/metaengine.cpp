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

#include "engines/advancedDetector.h"
#include "engines/myst3/database.h"
#include "engines/myst3/gfx.h"
#include "engines/myst3/state.h"
#include "engines/myst3/detection.h"

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/translation.h"

#include "graphics/scaler.h"

namespace Myst3{

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_WIDESCREEN_MOD,
		{
			_s("Widescreen mod"),
			_s("Enable widescreen rendering in fullscreen mode."),
			"widescreen_mod",
			false,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class Myst3MetaEngine : public AdvancedMetaEngine<Myst3GameDescription> {
public:
	const char *getName() const override {
		return "myst3";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override {
		return  (f == kSupportsListSaves) ||
		        (f == kSupportsDeleteSave) ||
		        (f == kSupportsLoadingDuringStartup) ||
		        (f == kSavesSupportMetaInfo) ||
		        (f == kSavesSupportThumbnail) ||
		        (f == kSavesSupportCreationDate) ||
		        (f == kSavesSupportPlayTime);
	}

	SaveStateList listSaves(const char *target) const override {
		Common::Platform platform = Common::parsePlatform(ConfMan.get("platform", target));
		Common::StringArray filenames = Saves::list(g_system->getSavefileManager(), platform);

		SaveStateList saveList;
		for (uint32 i = 0; i < filenames.size(); i++)
			// Since slots are ignored when saving, we always return slot 0
			// as an unused slot to optimise the autosave process
			saveList.push_back(SaveStateDescriptor(this, i + 1, filenames[i]));

		return saveList;
	}

	SaveStateDescriptor getSaveDescription(const char *target, int slot) const {
		SaveStateList saves = listSaves(target);

		SaveStateDescriptor description;
		for (uint32 i = 0; i < saves.size(); i++) {
			if (saves[i].getSaveSlot() == slot) {
				description = saves[i];
			}
		}

		return description;
	}

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override {
		SaveStateDescriptor saveInfos = getSaveDescription(target, slot);

		if (saveInfos.getDescription().empty()) {
			// Unused slot
			return SaveStateDescriptor();
		}

		// Open save
		Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(saveInfos.getDescription());
		if (!saveFile) {
			warning("Unable to open file %s for reading, slot %d", saveInfos.getDescription().encode().c_str(), slot);
			return SaveStateDescriptor();
		}

		// Read state data
		Common::Serializer s = Common::Serializer(saveFile, nullptr);
		GameState::StateData data;
		data.syncWithSaveGame(s);

		// Read and resize the thumbnail
		Graphics::Surface *saveThumb = GameState::readThumbnail(saveFile);
		Graphics::Surface *guiThumb = GameState::resizeThumbnail(saveThumb, kThumbnailWidth, kThumbnailHeight1);
		saveThumb->free();
		delete saveThumb;

		// Set metadata
		saveInfos.setThumbnail(guiThumb);
		saveInfos.setPlayTime(data.secondsPlayed * 1000);

		if (data.saveYear != 0) {
			saveInfos.setSaveDate(data.saveYear, data.saveMonth, data.saveDay);
			saveInfos.setSaveTime(data.saveHour, data.saveMinute);
		}

		if (data.saveDescription != "") {
			saveInfos.setDescription(data.saveDescription);
		}

		if (s.getVersion() >= 150) {
			saveInfos.setAutosave(data.isAutosave);
		}

		delete saveFile;

		return saveInfos;
	}

	bool removeSaveState(const char *target, int slot) const override {
		SaveStateDescriptor saveInfos = getSaveDescription(target, slot);
		return g_system->getSavefileManager()->removeSavefile(saveInfos.getDescription());
	}

	int getMaximumSaveSlot() const override {
		return 999;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const Myst3GameDescription *desc) const override;

	// TODO: Add getSavegameFile()
};

Common::Error Myst3MetaEngine::createInstance(OSystem *syst, Engine **engine, const Myst3GameDescription *desc) const {
	*engine = new Myst3Engine(syst,desc);
	return Common::kNoError;
}

Common::Platform Myst3Engine::getPlatform() const {
	return _gameDescription->desc.platform;
}

Common::Language Myst3Engine::getGameLanguage() const {
	return _gameDescription->desc.language;
}

uint32 Myst3Engine::getGameLocalizationType() const {
	return _gameDescription->flags & kGameLocalizationTypeMask;
}

uint32 Myst3Engine::getGameLayoutType() const {
	return _gameDescription->flags & kGameLayoutTypeMask;
}

} // End of namespace Myst3

#if PLUGIN_ENABLED_DYNAMIC(MYST3)
	REGISTER_PLUGIN_DYNAMIC(MYST3, PLUGIN_TYPE_ENGINE, Myst3::Myst3MetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MYST3, PLUGIN_TYPE_ENGINE, Myst3::Myst3MetaEngine);
#endif
