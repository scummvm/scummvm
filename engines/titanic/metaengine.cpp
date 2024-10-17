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

#include "titanic/core/project_item.h"
#include "titanic/events.h"
#include "titanic/support/simple_file.h"
#include "titanic/titanic.h"
#include "base/plugins.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/memstream.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "titanic/detection.h"

namespace Titanic {

uint32 TitanicEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language TitanicEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

} // End of namespace Titanic

class TitanicMetaEngine : public AdvancedMetaEngine<Titanic::TitanicGameDescription> {
public:
	const char *getName() const override {
		return "titanic";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const Titanic::TitanicGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool TitanicMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSimpleSavesNames);
}

bool Titanic::TitanicEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error TitanicMetaEngine::createInstance(OSystem *syst, Engine **engine, const Titanic::TitanicGameDescription *desc) const {
	*engine = new Titanic::TitanicEngine(syst,desc);
	return Common::kNoError;
}

SaveStateList TitanicMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);
	Titanic::TitanicSavegameHeader header;

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot <= MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				Titanic::CompressedFile cf;
				cf.open(in);

				if (Titanic::CProjectItem::readSavegameHeader(&cf, header))
					saveList.push_back(SaveStateDescriptor(this, slot, header._saveName));

				cf.close();
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int TitanicMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

bool TitanicMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor TitanicMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {
		Titanic::CompressedFile file;
		file.open(f);

		Titanic::TitanicSavegameHeader header;
		if (!Titanic::CProjectItem::readSavegameHeader(&file, header, false)) {
			file.close();
			return SaveStateDescriptor();
		}

		file.close();

		// Create the return descriptor
		SaveStateDescriptor desc(this, slot, header._saveName);

		if (header._version) {
			desc.setThumbnail(header._thumbnail);
			desc.setSaveDate(header._year, header._month, header._day);
			desc.setSaveTime(header._hour, header._minute);
			desc.setPlayTime(header._totalFrames * GAME_FRAME_TIME);
		}

		return desc;
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(TITANIC)
	REGISTER_PLUGIN_DYNAMIC(TITANIC, PLUGIN_TYPE_ENGINE, TitanicMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TITANIC, PLUGIN_TYPE_ENGINE, TitanicMetaEngine);
#endif
