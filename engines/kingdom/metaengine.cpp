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

#include "common/savefile.h"
#include "engines/advancedDetector.h"
#include "common/file.h"

#include "kingdom/kingdom.h"

#define MAX_SAVES 99

namespace Kingdom {

const char *KingdomGame::getGameId() const { return _gameDescription->gameId; }
Common::Platform KingdomGame::getPlatform() const { return _gameDescription->platform; }

} // End of namespace Kingdom


class KingdomMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "kingdom";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	int getMaximumSaveSlot() const override;
	SaveStateList listSaves(const char *target) const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool KingdomMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSimpleSavesNames) ||
	    (f == kSavesSupportCreationDate);
}

Common::Error KingdomMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Kingdom::KingdomGame(syst, desc);
	return Common::kNoError;
}

int KingdomMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

SaveStateList KingdomMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);

	filenames = saveFileMan->listSavefiles(pattern);

	Kingdom::KingdomSavegameHeader header;

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot < MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				if (Kingdom::KingdomGame::readSavegameHeader(in, header)) {
					saveList.push_back(SaveStateDescriptor(this, slot, header._saveName));

					header._thumbnail->free();
					delete header._thumbnail;
				}

				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

bool KingdomMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor KingdomMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {
		Kingdom::KingdomSavegameHeader header;
		Kingdom::KingdomGame::readSavegameHeader(f, header);
		delete f;

		// Create the return descriptor
		SaveStateDescriptor desc(this, slot, header._saveName);
		desc.setThumbnail(header._thumbnail);
		desc.setSaveDate(header._year, header._month, header._day);
		desc.setSaveTime(header._hour, header._minute);

		return desc;
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(KINGDOM)
	REGISTER_PLUGIN_DYNAMIC(KINGDOM, PLUGIN_TYPE_ENGINE, KingdomMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(KINGDOM, PLUGIN_TYPE_ENGINE, KingdomMetaEngine);
#endif
