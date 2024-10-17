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


#include "common/savefile.h"
#include "common/system.h"

#include "engines/advancedDetector.h"

#include "graphics/surface.h"

#include "access/access.h"
#include "access/amazon/amazon_game.h"
#include "access/martian/martian_game.h"

#include "access/detection.h"

#define MAX_SAVES 99

namespace Access {

uint32 AccessEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 AccessEngine::getGameFeatures() const {
	return _gameDescription->features;
}

uint32 AccessEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

bool AccessEngine::isCD() const {
	return (bool)(_gameDescription->desc.flags & ADGF_CD);
}

bool AccessEngine::isDemo() const {
	return (bool)(_gameDescription->desc.flags & ADGF_DEMO);
}

Common::Language AccessEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform AccessEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

} // End of namespace Access

class AccessMetaEngine : public AdvancedMetaEngine<Access::AccessGameDescription> {
public:
	const char *getName() const override {
		return "access";
	}

	bool hasFeature(MetaEngineFeature f) const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const Access::AccessGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool AccessMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSimpleSavesNames);
}

bool Access::AccessEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error AccessMetaEngine::createInstance(OSystem *syst, Engine **engine, const Access::AccessGameDescription *gd) const {
	switch (gd->gameID) {
	case Access::GType_Amazon:
		*engine = new Access::Amazon::AmazonEngine(syst, gd);
		break;
	case Access::GType_MartianMemorandum:
		*engine = new Access::Martian::MartianEngine(syst, gd);
		break;
	default:
		return Common::kUnsupportedGameidError;
	}
	return Common::kNoError;
}

SaveStateList AccessMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);
	Access::AccessSavegameHeader header;

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot < MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				if (Access::AccessEngine::readSavegameHeader(in, header))
					saveList.push_back(SaveStateDescriptor(this, slot, header._saveName));

				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int AccessMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

bool AccessMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor AccessMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {
		Access::AccessSavegameHeader header;
		if (!Access::AccessEngine::readSavegameHeader(f, header, false)) {
			delete f;
			return SaveStateDescriptor();
		}

		delete f;

		// Create the return descriptor
		SaveStateDescriptor desc(this, slot, header._saveName);
		desc.setThumbnail(header._thumbnail);
		desc.setSaveDate(header._year, header._month, header._day);
		desc.setSaveTime(header._hour, header._minute);
		desc.setPlayTime(header._totalFrames * GAME_FRAME_TIME);

		return desc;
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(ACCESS)
	REGISTER_PLUGIN_DYNAMIC(ACCESS, PLUGIN_TYPE_ENGINE, AccessMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ACCESS, PLUGIN_TYPE_ENGINE, AccessMetaEngine);
#endif
