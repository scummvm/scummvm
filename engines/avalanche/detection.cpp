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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#include "engines/advancedDetector.h"
#include "common/system.h"
#include "common/savefile.h"
#include "graphics/thumbnail.h"

#include "avalanche/avalanche.h"

namespace Avalanche {

uint32 AvalancheEngine::getFeatures() const {
	return _gameDescription->flags;
}

const char *AvalancheEngine::getGameId() const {
	return _gameDescription->gameid;
}

static const ADGameDescription gameDescriptions[] = {
	// Avalanche English
	{
		"Avalanche", 0,
		{
			{"avalot.sez", 0, "de10eb353228013da3d3297784f81ff9", 48763},
			{"mainmenu.avd", 0, "89f31211af579a872045b175cc264298", 18880},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformPCEngine,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	AD_TABLE_END_MARKER
};

class AvalancheMetaEngine : public AdvancedMetaEngine {
public:
	AvalancheMetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(ADGameDescription), 0) {
	}

	const char *getName() const {
		return "Avalanche";
	}

	const char *getOriginalCopyright() const {
		return "Avalanche Engine Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.";
	}

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;
	bool hasFeature(MetaEngineFeature f) const;

	int getMaximumSaveSlot() const;
	SaveStateList listSaves(const char *target) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	void removeSaveState(const char *target, int slot) const;
};

bool AvalancheMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (gd) {
		*engine = new AvalancheEngine(syst, (const ADGameDescription *)gd);
		((AvalancheEngine *)*engine)->initGame((const ADGameDescription *)gd);
	}
	return gd != 0;
}

bool AvalancheMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSavesSupportCreationDate);
}

int AvalancheMetaEngine::getMaximumSaveSlot() const { return 99; }

SaveStateList AvalancheMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += "-??.SAV";

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	char slot[3];
	int slotNum = 0;
	for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
		slot[0] = filename->c_str()[filename->size() - 6];
		slot[1] = filename->c_str()[filename->size() - 5];
		slot[2] = '\0';
		// Obtain the last 2 digits of the filename (without extension), since they correspond to the save slot
		slotNum = atoi(slot);
		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::InSaveFile *file = saveFileMan->openForLoading(*filename);
			if (file) {
				int saveVersion = file->readByte();

				if (saveVersion != kSavegameVersion) {
					warning("Savegame of incompatible version");
					delete file;
					continue;
				}

				// read name
				uint16 nameSize = file->readUint16BE();
				if (nameSize >= 255) {
					delete file;
					continue;
				}
				char name[256];
				file->read(name, nameSize);
				name[nameSize] = 0;

				saveList.push_back(SaveStateDescriptor(slotNum, name));
				delete file;
			}
		}
	}

	return saveList;
}

SaveStateDescriptor AvalancheMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s-%02d.SAV", target, slot);
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(fileName);

	if (file) {
		int saveVersion = file->readByte();

		if (saveVersion != kSavegameVersion) {
			warning("Savegame of incompatible version");
			delete file;
			return SaveStateDescriptor();
		}

		uint32 saveNameLength = file->readUint16BE();
		char saveName[256];
		file->read(saveName, saveNameLength);
		saveName[saveNameLength] = 0;

		SaveStateDescriptor desc(slot, saveName);

		Graphics::Surface *const thumbnail = Graphics::loadThumbnail(*file);
		desc.setThumbnail(thumbnail);

		desc.setDeletableFlag(true);
		desc.setWriteProtectedFlag(false);

		uint32 saveDate = file->readUint32BE();
		uint16 saveTime = file->readUint16BE();

		int day = (saveDate >> 24) & 0xFF;
		int month = (saveDate >> 16) & 0xFF;
		int year = saveDate & 0xFFFF;

		desc.setSaveDate(year, month, day);

		int hour = (saveTime >> 8) & 0xFF;
		int minutes = saveTime & 0xFF;

		desc.setSaveTime(hour, minutes);

		// Slot 0 is used for the 'restart game' save in all Avalanche games, thus
		// we prevent it from being deleted.
		desc.setDeletableFlag(slot != 0);
		desc.setWriteProtectedFlag(slot == 0);

		delete file;
		return desc;
	}
	return SaveStateDescriptor();
}

void AvalancheMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s-%02d.SAV", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

} // End of namespace Avalanche

#if PLUGIN_ENABLED_DYNAMIC(AVALANCHE)
REGISTER_PLUGIN_DYNAMIC(AVALANCHE, PLUGIN_TYPE_ENGINE, Avalanche::AvalancheMetaEngine);
#else
REGISTER_PLUGIN_STATIC(AVALANCHE, PLUGIN_TYPE_ENGINE, Avalanche::AvalancheMetaEngine);
#endif

namespace Avalanche {

void AvalancheEngine::initGame(const ADGameDescription *gd) {
	_platform = gd->platform;
}

} // End of namespace Avalanche
