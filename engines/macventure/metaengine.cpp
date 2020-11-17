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

#include "engines/advancedDetector.h"
#include "common/system.h"

#include "macventure/macventure.h"

namespace MacVenture {

const char *MacVentureEngine::getGameFileName() const {
	return _gameDescription->filesDescriptions[0].fileName;
}

} // End of namespace MacVenture


namespace MacVenture {

SaveStateDescriptor loadMetaData(Common::SeekableReadStream *s, int slot, bool skipThumbnail = true);

class MacVentureMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "macventure";
	}

protected:
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override;
	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

};

bool MacVentureMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime);
}

bool MacVentureEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

SaveStateList MacVentureMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		int slotNum = atoi(file->c_str() + file->size() - 3);
		SaveStateDescriptor desc;
		// Do not allow save slot 0 (used for auto-saving) to be deleted or
		// overwritten.
		desc.setDeletableFlag(slotNum != 0);
		desc.setWriteProtectedFlag(slotNum == 0);

		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				desc = loadMetaData(in, slotNum);
				if (desc.getSaveSlot() != slotNum) {
					// invalid
					delete in;
					continue;
				}
				saveList.push_back(desc);
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int MacVentureMetaEngine::getMaximumSaveSlot() const { return 999; }

Common::Error MacVentureMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *game) const {
	*engine = new MacVenture::MacVentureEngine(syst, game);
	return Common::kNoError;
}

void MacVentureMetaEngine::removeSaveState(const char *target, int slot) const {
	g_system->getSavefileManager()->removeSavefile(Common::String::format("%s.%03d", target, slot));
}


SaveStateDescriptor MacVentureMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	SaveStateDescriptor desc;
	Common::String saveFileName;
	Common::String pattern = target;
	pattern += ".###";
	Common::StringArray filenames = saveFileMan->listSavefiles(pattern);
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		int slotNum = atoi(file->c_str() + file->size() - 3);
		if (slotNum == slot) {
			saveFileName = *file;
		}
	}

	Common::InSaveFile *in = saveFileMan->openForLoading(saveFileName);
	if (in) {
		desc = loadMetaData(in, slot, false);
		delete in;
		return desc;
	}
	return SaveStateDescriptor(-1, "");
}

} // End of namespace MacVenture

#if PLUGIN_ENABLED_DYNAMIC(MACVENTURE)
	REGISTER_PLUGIN_DYNAMIC(MACVENTURE, PLUGIN_TYPE_ENGINE, MacVenture::MacVentureMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MACVENTURE, PLUGIN_TYPE_ENGINE, MacVenture::MacVentureMetaEngine);
#endif
