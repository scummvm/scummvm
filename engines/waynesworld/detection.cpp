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

#include "waynesworld/waynesworld.h"

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "base/plugins.h"
#include "graphics/thumbnail.h"

static const PlainGameDescriptor waynesworldGames[] = {
	{ "waynesworld", "Wayne's World" },
	{ 0, 0 }
};

namespace WaynesWorld {

static const ADGameDescription gameDescriptions[] = {
	{
		"waynesworld",
		0,
		AD_ENTRY1s("e00.txt", "78769d60d48aea314ed6be2f725c8c2f", 1293),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		0
	},

	AD_TABLE_END_MARKER
};

} // End of namespace WaynesWorld

static const char * const directoryGlobs[] = {
	"vnm",
	0
};

class WaynesWorldMetaEngine : public AdvancedMetaEngine {
public:
	WaynesWorldMetaEngine() : AdvancedMetaEngine(WaynesWorld::gameDescriptions, sizeof(ADGameDescription), waynesworldGames) {
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "waynesworld";
	}

	const char *getName() const override {
		return "Wayne's World";
	}

	const char *getOriginalCopyright() const override {
		return "(C) 1993 Capstone Software";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	int getMaximumSaveSlot() const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	void removeSaveState(const char *target, int slot) const override;
};

bool WaynesWorldMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
	/*
	    (f == kSupportsListSaves) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSavesSupportCreationDate) ||
		(f == kSimpleSavesNames);
	*/
}

void WaynesWorldMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

int WaynesWorldMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

SaveStateList WaynesWorldMetaEngine::listSaves(const char *target) const {
	return SaveStateList();
	#if 0
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	WaynesWorld::WaynesWorldEngine::SaveHeader header;
	Common::String pattern = target;
	pattern += ".###";
	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);
		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (WaynesWorld::WaynesWorldEngine::readSaveHeader(in, header) == WaynesWorld::WaynesWorldEngine::kRSHENoError) {
					saveList.push_back(SaveStateDescriptor(slotNum, header.description));
				}
				delete in;
			}
		}
	}
	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
	#endif
}

SaveStateDescriptor WaynesWorldMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	#if 0
	Common::String filename = WaynesWorld::WaynesWorldEngine::getSavegameFilename(target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());
	if (in) {
		WaynesWorld::WaynesWorldEngine::SaveHeader header;
		WaynesWorld::WaynesWorldEngine::kReadSaveHeaderError error;
		error = WaynesWorld::WaynesWorldEngine::readSaveHeader(in, header, false);
		delete in;
		if (error == WaynesWorld::WaynesWorldEngine::kRSHENoError) {
			SaveStateDescriptor desc(slot, header.description);
			// Slot 0 is used for the "Continue" save
			desc.setDeletableFlag(slot != 0);
			desc.setWriteProtectedFlag(slot == 0);
			desc.setThumbnail(header.thumbnail);
			desc.setSaveDate(header.saveDate & 0xFFFF, (header.saveDate >> 16) & 0xFF, (header.saveDate >> 24) & 0xFF);
			desc.setSaveTime((header.saveTime >> 16) & 0xFF, (header.saveTime >> 8) & 0xFF);
			desc.setPlayTime(header.playTime * 1000);
			return desc;
		}
	}
	#endif
	return SaveStateDescriptor();
}

bool WaynesWorldMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new WaynesWorld::WaynesWorldEngine(syst, desc);
	}
	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(WAYNESWORLD)
	REGISTER_PLUGIN_DYNAMIC(WAYNESWORLD, PLUGIN_TYPE_ENGINE, WaynesWorldMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(WAYNESWORLD, PLUGIN_TYPE_ENGINE, WaynesWorldMetaEngine);
#endif
