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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "lab/lab.h"
#include "engines/advancedDetector.h"

namespace Lab {

Common::Platform LabEngine::getPlatform() const {
	return _gameDescription->platform;
}

uint32 LabEngine::getFeatures() const {
	return _gameDescription->flags | _extraGameFeatures;
}

} // End of namespace Lab

class LabMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "lab";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		*engine = new Lab::LabEngine(syst, desc);
		return Common::kNoError;
	}

	bool hasFeature(MetaEngineFeature f) const override;
	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool LabMetaEngine::hasFeature(MetaEngineFeature f) const {
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

bool Lab::LabEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

SaveStateList LabMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Lab::SaveGameHeader header;
	Common::String pattern = target;
	pattern += ".###";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());

	SaveStateList saveList;

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if ((slotNum >= 0) && (slotNum <= 999)) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Lab::readSaveGameHeader(in, header))
					saveList.push_back(SaveStateDescriptor(slotNum, header._descr.getDescription()));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int LabMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void LabMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	saveFileMan->removeSavefile(Common::String::format("%s.%03u", target, slot));
}

SaveStateDescriptor LabMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03u", target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

	if (in) {
		Lab::SaveGameHeader header;

		bool successfulRead = Lab::readSaveGameHeader(in, header, false);
		delete in;

		if (successfulRead) {
			SaveStateDescriptor desc(slot, header._descr.getDescription());
			// Do not allow save slot 0 (used for auto-saving) to be deleted or
			// overwritten.
			//desc.setDeletableFlag(slot != 0);
			//desc.setWriteProtectedFlag(slot == 0);

			return header._descr;
		}
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(LAB)
	REGISTER_PLUGIN_DYNAMIC(LAB, PLUGIN_TYPE_ENGINE, LabMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(LAB, PLUGIN_TYPE_ENGINE, LabMetaEngine);
#endif
