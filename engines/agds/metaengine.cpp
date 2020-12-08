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
#include "agds/agds.h"
#include "agds/object.h"
#include "common/savefile.h"
#include "common/system.h"

namespace AGDS {

class AGDSMetaEngine : public AdvancedMetaEngine {
public:
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override {
		switch (f) {
		case kSimpleSavesNames:
			return true;
		case kSupportsLoadingDuringStartup:
			return false;
		default:
			return AdvancedMetaEngine::hasFeature(f);
		}
	}

	const char *getName() const override {
		return "agds";
	}

	int getMaximumSaveSlot() const override {
		return 24;
	}

	SaveStateList listSaves(const char *target) const {
		Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
		Common::StringArray filenames;
		Common::String pattern(getSavegameFilePattern(target));

		filenames = saveFileMan->listSavefiles(pattern);

		SaveStateList saveList;
		for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
			// Obtain the last 2 digits of the filename, since they correspond to the save slot
			int slotNum = atoi(file->c_str() + file->size() - 2);

			if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
				SaveStateDescriptor desc;

				desc.setSaveSlot(slotNum);
				desc.setDescription(*file);
				if (slotNum == getAutosaveSlot())
					desc.setWriteProtectedFlag(true);

				saveList.push_back(desc);
			}
		}

		// Sort saves based on slot number.
		Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
		return saveList;
	}
};

Common::Error AGDSMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new AGDS::AGDSEngine(syst, desc);
	return Common::Error(Common::kNoError);
}

}

#if PLUGIN_ENABLED_DYNAMIC(AGDS)
	REGISTER_PLUGIN_DYNAMIC(AGDS, PLUGIN_TYPE_ENGINE, AGDS::AGDSMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(AGDS, PLUGIN_TYPE_ENGINE, AGDS::AGDSMetaEngine);
#endif
