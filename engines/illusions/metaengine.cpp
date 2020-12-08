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

#include "illusions/illusions.h"
#include "illusions/bbdou/illusions_bbdou.h"
#include "illusions/duckman/illusions_duckman.h"

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "base/plugins.h"
#include "graphics/thumbnail.h"

#include "illusions/detection.h"

namespace Illusions {

int IllusionsEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Language IllusionsEngine::getGameLanguage() const {
	return _gameDescription->desc.language;
}

} // End of namespace Illusions

class IllusionsMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "illusions";
	}

    bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	int getMaximumSaveSlot() const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	void removeSaveState(const char *target, int slot) const override;
};

bool IllusionsMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate);
}

void IllusionsMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

int IllusionsMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

SaveStateList IllusionsMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Illusions::IllusionsEngine::SaveHeader header;
	Common::String pattern = target;
	pattern += ".???";
	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);
		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Illusions::IllusionsEngine::readSaveHeader(in, header) == Illusions::IllusionsEngine::kRSHENoError) {
					saveList.push_back(SaveStateDescriptor(slotNum, header.description));
				}
				delete in;
			}
		}
	}
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor IllusionsMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Illusions::IllusionsEngine::getSavegameFilename(target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());
	if (in) {
		Illusions::IllusionsEngine::SaveHeader header;
		Illusions::IllusionsEngine::kReadSaveHeaderError error;
		error = Illusions::IllusionsEngine::readSaveHeader(in, header, false);
		delete in;
		if (error == Illusions::IllusionsEngine::kRSHENoError) {
			SaveStateDescriptor desc(slot, header.description);
			// Slot 0 is used for the "Continue" save
			desc.setDeletableFlag(slot != 0);
			desc.setWriteProtectedFlag(slot == 0);
			desc.setThumbnail(header.thumbnail);
			desc.setSaveDate(header.saveDate & 0xFFFF, (header.saveDate >> 16) & 0xFF, (header.saveDate >> 24) & 0xFF);
			desc.setSaveTime((header.saveTime >> 16) & 0xFF, (header.saveTime >> 8) & 0xFF);
			desc.setPlayTime(header.playTime * 1000);
			return desc;
		}
	}
	return SaveStateDescriptor();
}

Common::Error IllusionsMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Illusions::IllusionsGameDescription *gd = (const Illusions::IllusionsGameDescription *)desc;
	switch (gd->gameId) {
	case Illusions::kGameIdBBDOU:
		*engine = new Illusions::IllusionsEngine_BBDOU(syst, gd);
		break;
	case Illusions::kGameIdDuckman:
		*engine = new Illusions::IllusionsEngine_Duckman(syst, gd);
		break;
	default:
		return Common::kUnsupportedGameidError;
	}
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(ILLUSIONS)
	REGISTER_PLUGIN_DYNAMIC(ILLUSIONS, PLUGIN_TYPE_ENGINE, IllusionsMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ILLUSIONS, PLUGIN_TYPE_ENGINE, IllusionsMetaEngine);
#endif
