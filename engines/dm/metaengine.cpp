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

/*
 * Based on the Reverse Engineering work of Christophe Fontanel,
 * maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
 */

#include "common/config-manager.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/system.h"

#include "engines/advancedDetector.h"

#include "dm/dm.h"

namespace DM {

class DMMetaEngine : public AdvancedMetaEngine<DMADGameDescription> {
public:
	const char *getName() const override {
		return "dm";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const DMADGameDescription *desc) const override {
		*engine = new DM::DMEngine(syst, (const DMADGameDescription*)desc);
		return Common::kNoError;
	}

	bool hasFeature(MetaEngineFeature f) const override {
		return
			(f == kSupportsListSaves) ||
			(f == kSupportsLoadingDuringStartup) ||
			(f == kSavesSupportThumbnail) ||
			(f == kSavesSupportMetaInfo) ||
			(f == kSimpleSavesNames) ||
			(f == kSavesSupportCreationDate);
	}

	int getMaximumSaveSlot() const override { return 99; }

	SaveStateList listSaves(const char *target) const override {
		Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
		SaveGameHeader header;
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
					if (DM::readSaveGameHeader(in, &header))
						saveList.push_back(SaveStateDescriptor(this, slotNum, header._descr.getDescription()));
					delete in;
				}
			}
		}

		// Sort saves based on slot number.
		Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
		return saveList;
	}

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override {
		Common::String filename = Common::String::format("%s.%03u", target, slot);
		Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

		if (in) {
			DM::SaveGameHeader header;

			bool successfulRead = DM::readSaveGameHeader(in, &header);
			delete in;

			if (successfulRead) {
				SaveStateDescriptor desc(this, slot, header._descr.getDescription());

				return header._descr;
			}
		}

		return SaveStateDescriptor();
	}

	bool removeSaveState(const char *target, int slot) const override { return false; }

};

} // End of namespace DM

#if PLUGIN_ENABLED_DYNAMIC(DM)
	REGISTER_PLUGIN_DYNAMIC(DM, PLUGIN_TYPE_ENGINE, DM::DMMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DM, PLUGIN_TYPE_ENGINE, DM::DMMetaEngine);
#endif
