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

#include "base/plugins.h"
#include "common/savefile.h"
#include "common/system.h"
#include "engines/advancedDetector.h"

#include "cruise/cruise.h"
#include "cruise/saveload.h"
#include "cruise/detection.h"

namespace Cruise {

const char *CruiseEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

Common::Language CruiseEngine::getLanguage() const {
	return _gameDescription->desc.language;
}
Common::Platform CruiseEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

} // End of namespace Cruise

class CruiseMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "cruise";
	}

    bool hasFeature(MetaEngineFeature f) const override;
	int getMaximumSaveSlot() const override { return 99; }

	SaveStateList listSaves(const char *target) const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

bool CruiseMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSupportsLoadingDuringStartup);
}

SaveStateList CruiseMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern("cruise.s##");

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);

		if (slotNum >= 0 && slotNum <= 99) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				Cruise::CruiseSavegameHeader header;
				if (Cruise::readSavegameHeader(in, header))
					saveList.push_back(SaveStateDescriptor(slotNum, header.saveName));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

void CruiseMetaEngine::removeSaveState(const char *target, int slot) const {
	g_system->getSavefileManager()->removeSavefile(Cruise::CruiseEngine::getSavegameFile(slot));
}

SaveStateDescriptor CruiseMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(
		Cruise::CruiseEngine::getSavegameFile(slot));

	if (f) {
		Cruise::CruiseSavegameHeader header;
		if (!Cruise::readSavegameHeader(f, header, false)) {
			delete f;
			return SaveStateDescriptor();
		}

		delete f;

		// Create the return descriptor
		SaveStateDescriptor desc(slot, header.saveName);
		desc.setThumbnail(header.thumbnail);

		return desc;
	}

	return SaveStateDescriptor();
}

Common::Error CruiseMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Cruise::CruiseEngine(syst, (const Cruise::CRUISEGameDescription *)desc);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(CRUISE)
	REGISTER_PLUGIN_DYNAMIC(CRUISE, PLUGIN_TYPE_ENGINE, CruiseMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CRUISE, PLUGIN_TYPE_ENGINE, CruiseMetaEngine);
#endif
