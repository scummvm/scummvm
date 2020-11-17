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

#include "draci/draci.h"
#include "draci/saveload.h"

#include "base/plugins.h"
#include "common/system.h"
#include "engines/advancedDetector.h"
#include "engines/metaengine.h"

class DraciMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "draci";
	}

    bool hasFeature(MetaEngineFeature f) const override;
	int getMaximumSaveSlot() const override { return 99; }
	SaveStateList listSaves(const char *target) const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

bool DraciMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSupportsLoadingDuringStartup);
}

SaveStateList DraciMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern("draci.s##");

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);

		if (slotNum >= 0 && slotNum <= 99) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				Draci::DraciSavegameHeader header;
				if (Draci::readSavegameHeader(in, header)) {
					saveList.push_back(SaveStateDescriptor(slotNum, header.saveName));
				}
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

void DraciMetaEngine::removeSaveState(const char *target, int slot) const {
	g_system->getSavefileManager()->removeSavefile(Draci::DraciEngine::getSavegameFile(slot));
}

SaveStateDescriptor DraciMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(
		Draci::DraciEngine::getSavegameFile(slot));

	if (f) {
		Draci::DraciSavegameHeader header;
		if (!Draci::readSavegameHeader(f, header, false)) {
			delete f;
			return SaveStateDescriptor();
		}

		delete f;

		// Create the return descriptor
		SaveStateDescriptor desc(slot, header.saveName);
		desc.setThumbnail(header.thumbnail);

		int day = (header.date >> 24) & 0xFF;
		int month = (header.date >> 16) & 0xFF;
		int year = header.date & 0xFFFF;
		desc.setSaveDate(year, month, day);

		int hour = (header.time >> 8) & 0xFF;
		int minutes = header.time & 0xFF;
		desc.setSaveTime(hour, minutes);

		desc.setPlayTime(header.playtime * 1000);

		return desc;
	}

	return SaveStateDescriptor();
}

Common::Error DraciMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Draci::DraciEngine(syst, desc);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(DRACI)
	REGISTER_PLUGIN_DYNAMIC(DRACI, PLUGIN_TYPE_ENGINE, DraciMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DRACI, PLUGIN_TYPE_ENGINE, DraciMetaEngine);
#endif
