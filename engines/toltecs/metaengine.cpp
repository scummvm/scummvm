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

#include "base/plugins.h"

#include "engines/advancedDetector.h"

#include "common/savefile.h"
#include "common/str-array.h"
#include "common/system.h"
#include "common/translation.h"

#include "toltecs/toltecs.h"
#include "toltecs/detection.h"

namespace Toltecs {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

uint32 ToltecsEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language ToltecsEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

} // End of namespace Toltecs

class ToltecsMetaEngine : public AdvancedMetaEngine<Toltecs::ToltecsGameDescription> {
public:
	const char *getName() const override {
		return "toltecs";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Toltecs::optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const Toltecs::ToltecsGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool ToltecsMetaEngine::hasFeature(MetaEngineFeature f) const {
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

bool Toltecs::ToltecsEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error ToltecsMetaEngine::createInstance(OSystem *syst, Engine **engine, const Toltecs::ToltecsGameDescription *desc) const {
	*engine = new Toltecs::ToltecsEngine(syst,desc);
	return Common::kNoError;
}

SaveStateList ToltecsMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Toltecs::ToltecsEngine::SaveHeader header;
	Common::String pattern = target;
	pattern += ".###";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Toltecs::ToltecsEngine::readSaveHeader(in, header) == Toltecs::ToltecsEngine::kRSHENoError) {
					saveList.push_back(SaveStateDescriptor(this, slotNum, header.description));
				}
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int ToltecsMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

bool ToltecsMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String filename = Toltecs::ToltecsEngine::getSavegameFilename(target, slot);

	bool success = saveFileMan->removeSavefile(filename.c_str());

	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		// Rename every slot greater than the deleted slot,
		if (slotNum > slot) {
			saveFileMan->renameSavefile(file->c_str(), filename.c_str());
			filename = Toltecs::ToltecsEngine::getSavegameFilename(target, ++slot);
		}
	}

	return success;
}

SaveStateDescriptor ToltecsMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Toltecs::ToltecsEngine::getSavegameFilename(target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

	if (in) {
		Toltecs::ToltecsEngine::SaveHeader header;
		Toltecs::ToltecsEngine::kReadSaveHeaderError error;

		error = Toltecs::ToltecsEngine::readSaveHeader(in, header, false);
		delete in;

		if (error == Toltecs::ToltecsEngine::kRSHENoError) {
			SaveStateDescriptor desc(this, slot, header.description);

			desc.setThumbnail(header.thumbnail);

			if (header.version > 0) {
				int day = (header.saveDate >> 24) & 0xFF;
				int month = (header.saveDate >> 16) & 0xFF;
				int year = header.saveDate & 0xFFFF;

				desc.setSaveDate(year, month, day);

				int hour = (header.saveTime >> 16) & 0xFF;
				int minutes = (header.saveTime >> 8) & 0xFF;

				desc.setSaveTime(hour, minutes);

				desc.setPlayTime(header.playTime * 1000);
			}

			return desc;
		}
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(TOLTECS)
	REGISTER_PLUGIN_DYNAMIC(TOLTECS, PLUGIN_TYPE_ENGINE, ToltecsMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TOLTECS, PLUGIN_TYPE_ENGINE, ToltecsMetaEngine);
#endif
