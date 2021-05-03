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

#include "common/savefile.h"
#include "common/system.h"
#include "common/fs.h"
#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "graphics/thumbnail.h"

#include "tucker/tucker.h"

class TuckerMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "tucker";
	}

	bool hasFeature(MetaEngineFeature f) const override {
		switch (f) {
		case kSupportsListSaves:
		case kSupportsLoadingDuringStartup:
		case kSupportsDeleteSave:
		case kSavesSupportMetaInfo:
		case kSavesSupportThumbnail:
		case kSavesSupportCreationDate:
		case kSavesSupportPlayTime:
			return true;
		default:
			return false;
		}
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		*engine = new Tucker::TuckerEngine(syst, desc->language, desc->flags);
		return Common::kNoError;
	}

	SaveStateList listSaves(const char *target) const override {
		Common::String pattern = Tucker::generateGameStateFileName(target, 0, true);
		Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles(pattern);
		Tucker::TuckerEngine::SavegameHeader header;
		SaveStateList saveList;

		for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
			int slot;
			const char *ext = strrchr(file->c_str(), '.');
			if (ext && (slot = atoi(ext + 1)) >= 0 && slot <= Tucker::kLastSaveSlot) {
				Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);
				if (in) {
					if (Tucker::TuckerEngine::readSavegameHeader(in, header) == Tucker::TuckerEngine::kSavegameNoError) {
						saveList.push_back(SaveStateDescriptor(slot, header.description));
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
		return Tucker::kLastSaveSlot;
	}

	virtual int getAutosaveSlot() const override {
		return Tucker::kAutoSaveSlot;
	}

	void removeSaveState(const char *target, int slot) const override {
		Common::String filename = Tucker::generateGameStateFileName(target, slot);
		g_system->getSavefileManager()->removeSavefile(filename);
	}

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override {
		Common::String fileName = Common::String::format("%s.%d", target, slot);
		Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(fileName);

		if (!file) {
			return SaveStateDescriptor();
		}

		Tucker::TuckerEngine::SavegameHeader header;
		Tucker::TuckerEngine::SavegameError savegameError = Tucker::TuckerEngine::readSavegameHeader(file, header, false);
		if (savegameError) {
			delete file;
			return SaveStateDescriptor();
		}

		SaveStateDescriptor desc(slot, header.description);

		if (slot == Tucker::kAutoSaveSlot) {
			bool autosaveAllowed = Tucker::TuckerEngine::isAutosaveAllowed(target);
			desc.setDeletableFlag(!autosaveAllowed);
			desc.setWriteProtectedFlag(autosaveAllowed);
		}

		if (header.version >= 2) {
			// creation/play time
			if (header.saveDate) {
				int day   = (header.saveDate >> 24) & 0xFF;
				int month = (header.saveDate >> 16) & 0xFF;
				int year  =  header.saveDate        & 0xFFFF;
				desc.setSaveDate(year, month, day);
			}

			if (header.saveTime) {
				int hour    = (header.saveTime >> 16) & 0xFF;
				int minutes = (header.saveTime >>  8) & 0xFF;
				desc.setSaveTime(hour, minutes);
			}

			if (header.playTime) {
				desc.setPlayTime(header.playTime * 1000);
			}

			// thumbnail
			if (header.thumbnail) {
				desc.setThumbnail(header.thumbnail);
			}
		}

		delete file;
		return desc;
	}
};

#if PLUGIN_ENABLED_DYNAMIC(TUCKER)
	REGISTER_PLUGIN_DYNAMIC(TUCKER, PLUGIN_TYPE_ENGINE, TuckerMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TUCKER, PLUGIN_TYPE_ENGINE, TuckerMetaEngine);
#endif
