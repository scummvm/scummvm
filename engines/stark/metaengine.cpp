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
#include "engines/stark/savemetadata.h"
#include "engines/stark/stark.h"
#include "engines/stark/services/stateprovider.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

namespace Stark {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ASSETS_MOD,
		{
			_s("Load modded assets"),
			_s("Enable loading of external replacement assets."),
			"enable_assets_mod",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_LINEAR_FILTERING,
		{
			_s("Enable linear filtering of the backgrounds images"),
			_s("When linear filtering is enabled the background graphics are smoother in full screen mode, at the cost of some details."),
			"use_linear_filtering",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_FONT_ANTIALIASING,
		{
			_s("Enable font anti-aliasing"),
			_s("When font anti-aliasing is enabled, the text is smoother."),
			"enable_font_antialiasing",
			true,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class StarkMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "stark";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override {
		return
			(f == kSupportsListSaves) ||
			(f == kSupportsLoadingDuringStartup) ||
			(f == kSupportsDeleteSave) ||
			(f == kSavesSupportThumbnail) ||
			(f == kSavesSupportMetaInfo) ||
			(f == kSavesSupportPlayTime) ||
			(f == kSavesSupportCreationDate);
	}

	int getMaximumSaveSlot() const override {
		return 999;
	}

	SaveStateList listSaves(const char *target) const override {
		Common::StringArray filenames = StarkEngine::listSaveNames(target);

		SaveStateList saveList;
		for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
			int slot = StarkEngine::getSaveNameSlot(target, *filename);

			// Read the description from the save
			Common::String description;
			Common::InSaveFile *save = g_system->getSavefileManager()->openForLoading(*filename);
			if (save) {
				StateReadStream stream(save);
				description = stream.readString();
			}

			saveList.push_back(SaveStateDescriptor(this, slot, description));
		}

		Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
		return saveList;
	}

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override {
		Common::String filename = StarkEngine::formatSaveName(target, slot);
		Common::InSaveFile *save = g_system->getSavefileManager()->openForLoading(filename);
		if (!save) {
			return SaveStateDescriptor();
		}

		SaveStateDescriptor descriptor;
		descriptor.setSaveSlot(slot);

		SaveMetadata metadata;
		Common::ErrorCode readError = metadata.read(save, filename);
		if (readError != Common::kNoError) {
			delete save;
			return descriptor;
		}

		descriptor.setDescription(metadata.description);

		if (metadata.version >= 9) {
			Graphics::Surface *thumb = metadata.readGameScreenThumbnail(save);
			descriptor.setThumbnail(thumb);
			descriptor.setPlayTime(metadata.totalPlayTime);
			descriptor.setSaveDate(metadata.saveYear, metadata.saveMonth, metadata.saveDay);
			descriptor.setSaveTime(metadata.saveHour, metadata.saveMinute);
		}

		if (metadata.version >= 13) {
			descriptor.setAutosave(metadata.isAutoSave);
		}

		delete save;

		return descriptor;
	}

	void removeSaveState(const char *target, int slot) const override {
		Common::String filename = StarkEngine::formatSaveName(target, slot);
		g_system->getSavefileManager()->removeSavefile(filename);
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		*engine = new StarkEngine(syst, desc);
		return Common::kNoError;
	}

	Common::String getSavegameFile(int saveGameIdx, const char *target) const override {
		if (!target)
			target = getName();
		if (saveGameIdx == kSavegameFilePattern)
			return Common::String::format("%s-###.tlj", target);
		else
			return StarkEngine::formatSaveName(target, saveGameIdx);
	}
};

} // End of namespace Stark

#if PLUGIN_ENABLED_DYNAMIC(STARK)
	REGISTER_PLUGIN_DYNAMIC(STARK, PLUGIN_TYPE_ENGINE, Stark::StarkMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(STARK, PLUGIN_TYPE_ENGINE, Stark::StarkMetaEngine);
#endif
