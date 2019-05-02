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

#include "common/scummsys.h"

#include "base/plugins.h"

#include "engines/advancedDetector.h"

#include "zvision/zvision.h"
#include "zvision/file/save_manager.h"
#include "zvision/scripting/script_manager.h"

#include "common/translation.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/system.h"

namespace ZVision {

struct ZVisionGameDescription {
	ADGameDescription desc;
	ZVisionGameId gameId;
};

ZVisionGameId ZVision::getGameId() const {
	return _gameDescription->gameId;
}
Common::Language ZVision::getLanguage() const {
	return _gameDescription->desc.language;
}
uint32 ZVision::getFeatures() const {
	return _gameDescription->desc.flags;
}

} // End of namespace ZVision

#include "zvision/detection_tables.h"

class ZVisionMetaEngine : public AdvancedMetaEngine {
public:
	ZVisionMetaEngine() : AdvancedMetaEngine(ZVision::gameDescriptions, sizeof(ZVision::ZVisionGameDescription), ZVision::zVisionGames, ZVision::optionsList) {
		_maxScanDepth = 2;
		_directoryGlobs = ZVision::directoryGlobs;
		_singleId = "zvision";
	}

	virtual const char *getName() const {
		return "Z-Vision";
	}

	virtual const char *getOriginalCopyright() const {
		return "Z-Vision (C) 1996 Activision";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool ZVisionMetaEngine::hasFeature(MetaEngineFeature f) const {
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

bool ZVision::ZVision::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error ZVision::ZVision::loadGameState(int slot) {
	return _saveManager->loadGame(slot);
}

Common::Error ZVision::ZVision::saveGameState(int slot, const Common::String &desc) {
	_saveManager->saveGame(slot, desc, false);
	return Common::kNoError;
}

bool ZVision::ZVision::canLoadGameStateCurrently() {
	return !_videoIsPlaying;
}

bool ZVision::ZVision::canSaveGameStateCurrently() {
	Location currentLocation = _scriptManager->getCurrentLocation();
	return !_videoIsPlaying && currentLocation.world != 'g' && !(currentLocation.room == 'j' || currentLocation.room == 'a');
}

bool ZVisionMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const ZVision::ZVisionGameDescription *gd = (const ZVision::ZVisionGameDescription *)desc;
	if (gd) {
		*engine = new ZVision::ZVision(syst, gd);
	}
	return gd != 0;
}

SaveStateList ZVisionMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	ZVision::SaveGameHeader header;
	Common::String pattern = target;
	pattern += ".###";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());

	SaveStateList saveList;
	// We only use readSaveGameHeader() here, which doesn't need an engine callback
	ZVision::SaveManager *zvisionSaveMan = new ZVision::SaveManager(NULL);

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); file++) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (zvisionSaveMan->readSaveGameHeader(in, header)) {
					saveList.push_back(SaveStateDescriptor(slotNum, header.saveName));
				}
				delete in;
			}
		}
	}

	delete zvisionSaveMan;

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int ZVisionMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void ZVisionMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	saveFileMan->removeSavefile(Common::String::format("%s.%03u", target, slot));
}

SaveStateDescriptor ZVisionMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03u", target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

	if (in) {
		ZVision::SaveGameHeader header;

		// We only use readSaveGameHeader() here, which doesn't need an engine callback
		ZVision::SaveManager *zvisionSaveMan = new ZVision::SaveManager(NULL);
		bool successfulRead = zvisionSaveMan->readSaveGameHeader(in, header, false);
		delete zvisionSaveMan;
		delete in;

		if (successfulRead) {
			SaveStateDescriptor desc(slot, header.saveName);

			// Do not allow save slot 0 (used for auto-saving) to be deleted or
			// overwritten.
			desc.setDeletableFlag(slot != 0);
			desc.setWriteProtectedFlag(slot == 0);

			desc.setThumbnail(header.thumbnail);

			if (header.version >= 1) {
				int day = header.saveDay;
				int month = header.saveMonth;
				int year = header.saveYear;

				desc.setSaveDate(year, month, day);

				int hour = header.saveHour;
				int minutes = header.saveMinutes;

				desc.setSaveTime(hour, minutes);
			}

			if (header.version >= 2) {
				desc.setPlayTime(header.playTime * 1000);
			}

			return desc;
		}
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(ZVISION)
	REGISTER_PLUGIN_DYNAMIC(ZVISION, PLUGIN_TYPE_ENGINE, ZVisionMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ZVISION, PLUGIN_TYPE_ENGINE, ZVisionMetaEngine);
#endif
