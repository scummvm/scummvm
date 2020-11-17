/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "engines/advancedDetector.h"
#include "engines/myst3/database.h"
#include "engines/myst3/gfx.h"
#include "engines/myst3/state.h"
#include "engines/myst3/detection.h"

#include "common/config-manager.h"
#include "common/savefile.h"

#include "graphics/scaler.h"

namespace Myst3{

class Myst3MetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "myst3";
	}

	bool hasFeature(MetaEngineFeature f) const override {
		return
			(f == kSupportsListSaves) ||
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
			saveList.push_back(SaveStateDescriptor(i, filenames[i]));

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
		Common::Serializer s = Common::Serializer(saveFile, 0);
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

	void removeSaveState(const char *target, int slot) const override {
		SaveStateDescriptor saveInfos = getSaveDescription(target, slot);
		g_system->getSavefileManager()->removeSavefile(saveInfos.getDescription());
	}

	int getMaximumSaveSlot() const override {
		return 999;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

Common::Error Myst3MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Myst3Engine(syst, (const Myst3GameDescription *)desc);
	return Common::kNoError;
}

Common::Platform Myst3Engine::getPlatform() const {
	return _gameDescription->desc.platform;
}

Common::Language Myst3Engine::getGameLanguage() const {
	return _gameDescription->desc.language;
}

uint32 Myst3Engine::getGameLocalizationType() const {
	return _gameDescription->localizationType;
}

} // End of namespace Myst3

#if PLUGIN_ENABLED_DYNAMIC(MYST3)
	REGISTER_PLUGIN_DYNAMIC(MYST3, PLUGIN_TYPE_ENGINE, Myst3::Myst3MetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MYST3, PLUGIN_TYPE_ENGINE, Myst3::Myst3MetaEngine);
#endif
