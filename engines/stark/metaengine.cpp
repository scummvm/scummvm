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
#include "engines/stark/savemetadata.h"
#include "engines/stark/stark.h"
#include "engines/stark/services/stateprovider.h"

#include "common/savefile.h"
#include "common/system.h"

namespace Stark {

class StarkMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "stark";
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

			saveList.push_back(SaveStateDescriptor(slot, description));
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
};

} // End of namespace Stark

#if PLUGIN_ENABLED_DYNAMIC(STARK)
	REGISTER_PLUGIN_DYNAMIC(STARK, PLUGIN_TYPE_ENGINE, Stark::StarkMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(STARK, PLUGIN_TYPE_ENGINE, Stark::StarkMetaEngine);
#endif
