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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"

#include "graphics/thumbnail.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/savefile.h"

#include "startrek/startrek.h"

#include "startrek/detection.h"

namespace StarTrek {

uint32 StarTrekEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform StarTrekEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint8 StarTrekEngine::getGameType() const {
	return _gameDescription->gameType;
}

Common::Language StarTrekEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

} // End of Namespace StarTrek

class StarTrekMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "startrek";
	}

    bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool StarTrekMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSavesSupportCreationDate) ||
	    (f == kSavesSupportPlayTime) ||
	    (f == kSimpleSavesNames);
}

Common::Error StarTrekMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new StarTrek::StarTrekEngine(syst, (const StarTrek::StarTrekGameDescription *)desc);
	return Common::kNoError;
}

SaveStateList StarTrekMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNr = atoi(file->c_str() + file->size() - 3);

		if (slotNr >= 0 && slotNr <= getMaximumSaveSlot()) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				StarTrek::SavegameMetadata meta;
				StarTrek::saveOrLoadMetadata(in, nullptr, &meta);
				delete in;

				uint16 descriptionPos = 0;

				// Security-check, if saveDescription has a terminating NUL
				while (meta.description[descriptionPos]) {
					descriptionPos++;
					if (descriptionPos >= sizeof(meta.description))
						break;
				}
				if (descriptionPos >= sizeof(meta.description)) {
					strcpy(meta.description, "[broken saved game]");
				}

				saveList.push_back(SaveStateDescriptor(slotNr, meta.description));
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}


int StarTrekMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void StarTrekMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

SaveStateDescriptor StarTrekMetaEngine::querySaveMetaInfos(const char *target, int slotNr) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slotNr);

	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fileName);

	if (in) {
		StarTrek::SavegameMetadata meta;
		StarTrek::saveOrLoadMetadata(in, nullptr, &meta);
		delete in;

		uint16 descriptionPos = 0;

		while (meta.description[descriptionPos]) {
			descriptionPos++;
			if (descriptionPos >= sizeof(meta.description))
				break;
		}
		if (descriptionPos >= sizeof(meta.description)) {
			// broken meta.description, ignore it
			SaveStateDescriptor descriptor(slotNr, "[broken saved game]");
			return descriptor;
		}

		SaveStateDescriptor descriptor(slotNr, meta.description);

		// Do not allow save slot 0 (used for auto-saving) to be deleted or
		// overwritten.
		if (slotNr == 0) {
			descriptor.setWriteProtectedFlag(true);
			descriptor.setDeletableFlag(false);
		} else {
			descriptor.setWriteProtectedFlag(false);
			descriptor.setDeletableFlag(true);
		}

		if (meta.thumbnail == nullptr) {
			return SaveStateDescriptor();
		}

		descriptor.setThumbnail(meta.thumbnail);
		descriptor.setPlayTime(meta.playTime);
		descriptor.setSaveDate(meta.getYear(), meta.getMonth(), meta.getDay());
		descriptor.setSaveTime(meta.getHour(), meta.getMinute());

		return descriptor;

	} else {
		SaveStateDescriptor emptySave;
		// Do not allow save slot 0 (used for auto-saving) to be overwritten.
		if (slotNr == 0) {
			emptySave.setWriteProtectedFlag(true);
		} else {
			emptySave.setWriteProtectedFlag(false);
		}
		return emptySave;
	}
}

#if PLUGIN_ENABLED_DYNAMIC(STARTREK)
    REGISTER_PLUGIN_DYNAMIC(STARTREK, PLUGIN_TYPE_ENGINE, StarTrekMetaEngine);
#else
    REGISTER_PLUGIN_STATIC(STARTREK, PLUGIN_TYPE_ENGINE, StarTrekMetaEngine);
#endif
