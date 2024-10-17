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
#include "common/file.h"

#include "neverhood/dialogs.h"
#include "neverhood/neverhood.h"
#include "neverhood/detection.h"

namespace Neverhood {

const char *NeverhoodEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Platform NeverhoodEngine::getPlatform() const {
	return _gameDescription->platform;
}

Common::Language NeverhoodEngine::getLanguage() const {
	return _gameDescription->language;
}

bool NeverhoodEngine::isDemo() const {
	return _gameDescription->flags & ADGF_DEMO;
}

bool NeverhoodEngine::isBigDemo() const {
	return _gameDescription->flags & GF_BIG_DEMO;
}

bool NeverhoodEngine::applyResourceFixes() const {
	return getLanguage() == Common::RU_RUS;
}

} // End of namespace Neverhood

class NeverhoodMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "neverhood";
	}

	GUI::OptionsContainerWidget *buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override {
		return new Neverhood::NeverhoodOptionsWidget(boss, name, target);
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool NeverhoodMetaEngine::hasFeature(MetaEngineFeature f) const {
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

bool Neverhood::NeverhoodEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error NeverhoodMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Neverhood::NeverhoodEngine(syst, desc);
	return Common::kNoError;
}

SaveStateList NeverhoodMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Neverhood::NeverhoodEngine::SaveHeader header;
	Common::String pattern = target;
	pattern += ".###";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); file++) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);
		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Neverhood::NeverhoodEngine::readSaveHeader(in, header) == Neverhood::NeverhoodEngine::kRSHENoError) {
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

int NeverhoodMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

bool NeverhoodMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String filename = Neverhood::NeverhoodEngine::getSavegameFilename(target, slot);
	return saveFileMan->removeSavefile(filename.c_str());
}

SaveStateDescriptor NeverhoodMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Neverhood::NeverhoodEngine::getSavegameFilename(target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

	if (in) {
		Neverhood::NeverhoodEngine::SaveHeader header;
		Neverhood::NeverhoodEngine::kReadSaveHeaderError error;

		error = Neverhood::NeverhoodEngine::readSaveHeader(in, header, false);
		delete in;

		if (error == Neverhood::NeverhoodEngine::kRSHENoError) {
			SaveStateDescriptor desc(this, slot, header.description);

			desc.setThumbnail(header.thumbnail);
			int day = (header.saveDate >> 24) & 0xFF;
			int month = (header.saveDate >> 16) & 0xFF;
			int year = header.saveDate & 0xFFFF;
			desc.setSaveDate(year, month, day);
			int hour = (header.saveTime >> 16) & 0xFF;
			int minutes = (header.saveTime >> 8) & 0xFF;
			desc.setSaveTime(hour, minutes);
			desc.setPlayTime(header.playTime * 1000);
			return desc;
		}
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(NEVERHOOD)
	REGISTER_PLUGIN_DYNAMIC(NEVERHOOD, PLUGIN_TYPE_ENGINE, NeverhoodMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(NEVERHOOD, PLUGIN_TYPE_ENGINE, NeverhoodMetaEngine);
#endif
