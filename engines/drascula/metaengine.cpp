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

#include "engines/savestate.h"
#include "engines/advancedDetector.h"

#include "common/translation.h"

#include "graphics/thumbnail.h"

#include "drascula/drascula.h"
#include "drascula/detection.h"

namespace Drascula {

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

uint32 DrasculaEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language DrasculaEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

void DrasculaEngine::loadArchives() {
	const ADGameFileDescription *ag;

	if (getFeatures() & GF_PACKED) {
		for (ag = _gameDescription->desc.filesDescriptions; ag->fileName; ag++) {
			if (!_archives.hasArchive(ag->fileName))
				_archives.registerArchive(ag->fileName, ag->fileType);
		}
	}

	_archives.enableFallback(true);
}

} // End of namespace Drascula


namespace Drascula {

SaveStateDescriptor loadMetaData(Common::ReadStream *s, int slot, bool setPlayTime);

class DrasculaMetaEngine : public AdvancedMetaEngine<Drascula::DrasculaGameDescription> {
public:
	const char *getName() const override {
		return "drascula";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Drascula::optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const Drascula::DrasculaGameDescription *gd) const override;
	bool hasFeature(MetaEngineFeature f) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool DrasculaMetaEngine::hasFeature(MetaEngineFeature f) const {
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

SaveStateList DrasculaMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	int slotNum = 0;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				SaveStateDescriptor desc = loadMetaData(in, slotNum, false);
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

SaveStateDescriptor DrasculaMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);

	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fileName);

	SaveStateDescriptor desc(this, slot, Common::U32String());
	if (in) {
		desc = Drascula::loadMetaData(in, slot, false);
		if (desc.getSaveSlot() != slot) {
			delete in;
			return SaveStateDescriptor();
		}

		Graphics::Surface *thumbnail;
		if (!Graphics::loadThumbnail(*in, thumbnail)) {
			delete in;
			return SaveStateDescriptor();
		}
		desc.setThumbnail(thumbnail);

		delete in;
	}

	return desc;
}

int DrasculaMetaEngine::getMaximumSaveSlot() const { return 999; }

bool DrasculaMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(fileName);
}

Common::Error DrasculaMetaEngine::createInstance(OSystem *syst, Engine **engine, const Drascula::DrasculaGameDescription *desc) const {
	*engine = new Drascula::DrasculaEngine(syst,desc);
	return Common::kNoError;
}

} // End of namespace Drascula

#if PLUGIN_ENABLED_DYNAMIC(DRASCULA)
	REGISTER_PLUGIN_DYNAMIC(DRASCULA, PLUGIN_TYPE_ENGINE, Drascula::DrasculaMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DRASCULA, PLUGIN_TYPE_ENGINE, Drascula::DrasculaMetaEngine);
#endif
