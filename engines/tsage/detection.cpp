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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/config-manager.h"
#include "common/system.h"
#include "common/savefile.h"

#include "engines/advancedDetector.h"

#include "base/plugins.h"

#include "tsage/tsage.h"

namespace tSage {

struct tSageGameDescription {
	ADGameDescription desc;

	int gameID;
	uint32 features;
};

const char *TSageEngine::getGameId() const {
	return _gameDescription->desc.gameid;
}

uint32 TSageEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 TSageEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::String TSageEngine::getPrimaryFilename() const {
	return Common::String(_gameDescription->desc.filesDescriptions[0].fileName);
}

} // End of namespace tSage

static const PlainGameDescriptor tSageGameTitles[] = {
	{ "tsage", "Unknown Tsunami TSAGE-based Game" },
	{ "ring", "Ringworld: Revenge of the Patriarch" },
	{ "blueforce", "Blue Force" },
	{ 0, 0 }
};

#include "engines/tsage/detection_tables.h"

static const ADParams detectionParams = {
	(const byte *)tSage::gameDescriptions,
	sizeof(tSage::tSageGameDescription),
	0,
	tSageGameTitles,
	0,
	"tsage",
	NULL,
	0,
	Common::GUIO_NONE,
	0,
	NULL
};

#define MAX_SAVES 100

class TSageMetaEngine : public AdvancedMetaEngine {
public:
	TSageMetaEngine() : AdvancedMetaEngine(detectionParams) {
	}

	virtual const char *getName() const {
		return "TsAGE";
	}

	virtual const char *getOriginalCopyright() const {
		return "(c) Tsunami Media";
	}

	virtual bool hasFeature(MetaEngineFeature f) const {
		switch (f) {
		case kSupportsListSaves:
		case kSupportsDeleteSave:
		case kSupportsLoadingDuringStartup:
		case kSavesSupportMetaInfo:
		case kSavesSupportThumbnail:
		case kSavesSupportCreationDate:
		case kSavesSupportPlayTime:
			return true;
		default:
			return false;
		}
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
		if (desc) {
			*engine = new tSage::TSageEngine(syst, (const tSage::tSageGameDescription *)desc);
		}
		return desc != 0;
	}

	static Common::String generateGameStateFileName(const char *target, int slot) {
		return Common::String::format("%s.%03d", target, slot);
	}

	virtual SaveStateList listSaves(const char *target) const {
		Common::String pattern = target;
		pattern += ".???";

		Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles(pattern);
		sort(filenames.begin(), filenames.end());
		tSage::tSageSavegameHeader header;

		SaveStateList saveList;
		for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
			const char *ext = strrchr(file->c_str(), '.');
			int slot = ext ? atoi(ext + 1) : -1;

			if (slot >= 0 && slot < MAX_SAVES) {
				Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

				if (in) {
					if (tSage::Saver::readSavegameHeader(in, header)) {
						saveList.push_back(SaveStateDescriptor(slot, header.saveName));
						delete header.thumbnail;
					}

					delete in;
				}
			}
		}

		return saveList;
	}

	virtual int getMaximumSaveSlot() const {
		return MAX_SAVES - 1;
	}

	virtual void removeSaveState(const char *target, int slot) const {
		Common::String filename = Common::String::format("%s.%03d", target, slot);
		g_system->getSavefileManager()->removeSavefile(filename);
	}

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const {
		Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(
			generateGameStateFileName(target, slot));
		assert(f);

		tSage::tSageSavegameHeader header;
		tSage::Saver::readSavegameHeader(f, header);
		delete f;

		// Create the return descriptor
		SaveStateDescriptor desc(slot, header.saveName);
		desc.setDeletableFlag(true);
		desc.setWriteProtectedFlag(false);
		desc.setThumbnail(header.thumbnail);
		desc.setSaveDate(header.saveYear, header.saveMonth, header.saveDay);
		desc.setSaveTime(header.saveHour, header.saveMinutes);
		desc.setPlayTime(header.totalFrames * GAME_FRAME_TIME);

		return desc;
	}
};

#if PLUGIN_ENABLED_DYNAMIC(TSAGE)
REGISTER_PLUGIN_DYNAMIC(TSAGE, PLUGIN_TYPE_ENGINE, TSageMetaEngine);
#else
REGISTER_PLUGIN_STATIC(TSAGE, PLUGIN_TYPE_ENGINE, TSageMetaEngine);
#endif
