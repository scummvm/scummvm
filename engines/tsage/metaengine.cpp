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

#include "common/config-manager.h"
#include "common/system.h"
#include "common/savefile.h"

#include "engines/advancedDetector.h"

#include "base/plugins.h"

#include "tsage/tsage.h"
#include "tsage/detection.h"

namespace TsAGE {

const char *TSageEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

uint32 TSageEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 TSageEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Language TSageEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Path TSageEngine::getPrimaryFilename() const {
	return Common::Path(_gameDescription->desc.filesDescriptions[0].fileName);
}

} // End of namespace TsAGE

enum {
	MAX_SAVES = 100
};

class TSageMetaEngine : public AdvancedMetaEngine<TsAGE::tSageGameDescription> {
public:
	const char *getName() const override {
		return "tsage";
	}

	bool hasFeature(MetaEngineFeature f) const override {
		switch (f) {
		case kSupportsListSaves:
		case kSupportsDeleteSave:
		case kSupportsLoadingDuringStartup:
		case kSavesSupportMetaInfo:
		case kSavesSupportThumbnail:
		case kSavesSupportCreationDate:
		case kSavesSupportPlayTime:
		case kSimpleSavesNames:
			return true;
		default:
			return false;
		}
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const TsAGE::tSageGameDescription *desc) const override {
		*engine = new TsAGE::TSageEngine(syst, desc);
		return Common::kNoError;
	}

	static Common::String generateGameStateFileName(const char *target, int slot) {
		return Common::String::format("%s.%03d", target, slot);
	}

	SaveStateList listSaves(const char *target) const override {
		Common::String pattern = target;
		pattern += ".###";

		Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles(pattern);
		TsAGE::tSageSavegameHeader header;

		SaveStateList saveList;
		for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
			const char *ext = strrchr(file->c_str(), '.');
			int slot = ext ? atoi(ext + 1) : -1;

			if (slot >= 0 && slot < MAX_SAVES) {
				Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

				if (in) {
					if (TsAGE::Saver::readSavegameHeader(in, header)) {
						saveList.push_back(SaveStateDescriptor(this, slot, header._saveName));
					}

					delete in;
				}
			}
		}

		// Sort saves based on slot number.
		Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
		return saveList;
	}

	int getMaximumSaveSlot() const override {
		return MAX_SAVES - 1;
	}

	bool removeSaveState(const char *target, int slot) const override {
		Common::String filename = Common::String::format("%s.%03d", target, slot);
		return g_system->getSavefileManager()->removeSavefile(filename);
	}

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override {
		Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(
			generateGameStateFileName(target, slot));

		if (f) {
			TsAGE::tSageSavegameHeader header;
			if (!TsAGE::Saver::readSavegameHeader(f, header, false)) {
				delete f;
				return SaveStateDescriptor();
			}

			delete f;

			// Create the return descriptor
			SaveStateDescriptor desc(this, slot, header._saveName);
			desc.setThumbnail(header._thumbnail);
			desc.setSaveDate(header._saveYear, header._saveMonth, header._saveDay);
			desc.setSaveTime(header._saveHour, header._saveMinutes);
			desc.setPlayTime(header._totalFrames * GAME_FRAME_TIME);

			return desc;
		}

		return SaveStateDescriptor();
	}

};

#if PLUGIN_ENABLED_DYNAMIC(TSAGE)
	REGISTER_PLUGIN_DYNAMIC(TSAGE, PLUGIN_TYPE_ENGINE, TSageMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TSAGE, PLUGIN_TYPE_ENGINE, TSageMetaEngine);
#endif
