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

#include "zvision/zvision.h"
#include "zvision/detection.h"
#include "zvision/file/save_manager.h"
#include "zvision/scripting/script_manager.h"

#include "common/translation.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/system.h"

namespace ZVision {

uint32 ZVision::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language ZVision::getLanguage() const {
	return _gameDescription->desc.language;
}

} // End of namespace ZVision

static const PlainGameDescriptor zVisionGames[] = {
	{"zvision",  "ZVision Game"},
	{"znemesis", "Zork Nemesis: The Forbidden Lands"},
	{"zgi",      "Zork: Grand Inquisitor"},
	{0, 0}
};

namespace ZVision {

static const ZVisionGameDescription gameDescriptions[] = {

	{
		// Zork Nemesis English version
		{
			"znemesis",
			0,
			AD_ENTRY1s("CSCR.ZFS", "88226e51a205d2e50c67a5237f3bd5f2", 2397741),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		GID_NEMESIS
	},

	{
		// Zork Nemesis English demo version
		{
			"znemesis",
			"Demo",
			AD_ENTRY1s("SCRIPTS.ZFS", "64f1e881394e9462305104f99513c833", 380539),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
		GID_NEMESIS
	},

	{
		// Zork Grand Inquisitor English CD version
		{
			"zgi",
			"CD",
			AD_ENTRY1s("SCRIPTS.ZFS", "81efd40ecc3d22531e211368b779f17f", 8336944),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		GID_GRANDINQUISITOR
	},

	{
		// Zork Grand Inquisitor English demo version
		{
			"zgi",
			"Demo",
			AD_ENTRY1s("SCRIPTS.ZFS", "71a2494fd2fb999347deb13401e9b998", 304239),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
		GID_GRANDINQUISITOR
	},

	{
		// Zork Grand Inquisitor English DVD version
		{
			"zgi",
			"DVD",
			AD_ENTRY1s("SCRIPTS.ZFS", "03157a3399513bfaaf8dc6d5ab798b36", 8433326),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		GID_GRANDINQUISITOR
	},

	{
		AD_TABLE_END_MARKER,
		GID_NONE
	}
};

} // End of namespace ZVision

static const char *directoryGlobs[] = {
	"znemscr",
	0
};

static const ExtraGuiOption ZVisionExtraGuiOption = {
	_s("Use original save/load screens"),
	_s("Use the original save/load screens, instead of the ScummVM ones"),
	"originalsaveload",
	false
};

static const ExtraGuiOption ZVisionExtraGuiOption2 = {
	_s("Double FPS"),
	_s("Halve the update delay"),
	"doublefps",
	false
};

class ZVisionMetaEngine : public AdvancedMetaEngine {
public:
	ZVisionMetaEngine() : AdvancedMetaEngine(ZVision::gameDescriptions, sizeof(ZVision::ZVisionGameDescription), zVisionGames) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
		_singleid = "zvision";
	}

	virtual const char *getName() const {
		return "ZVision";
	}

	virtual const char *getOriginalCopyright() const {
		return "ZVision Activision (C) 1996";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const;
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
	(f == kSavesSupportCreationDate);
	//(f == kSavesSupportPlayTime);
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
	_saveManager->saveGame(slot, desc);
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

const ExtraGuiOptions ZVisionMetaEngine::getExtraGuiOptions(const Common::String &target) const {
	ExtraGuiOptions options;
	options.push_back(ZVisionExtraGuiOption);
	options.push_back(ZVisionExtraGuiOption2);
	return options;
}

SaveStateList ZVisionMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	ZVision::SaveGameHeader header;
	Common::String pattern = target;
	pattern += ".???";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)*/

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

	return saveList;
}

int ZVisionMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void ZVisionMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String filename = Common::String::format("%s.%03u", target, slot);

	saveFileMan->removeSavefile(filename.c_str());

	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".???";
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
	    // Obtain the last 3 digits of the filename, since they correspond to the save slot
	    int slotNum = atoi(file->c_str() + file->size() - 3);

	    // Rename every slot greater than the deleted slot,
	    if (slotNum > slot) {
	        saveFileMan->renameSavefile(file->c_str(), filename.c_str());
	        filename = Common::String::format("%s.%03u", target, ++slot);
	    }
	}
}

SaveStateDescriptor ZVisionMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03u", target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

	if (in) {
	    ZVision::SaveGameHeader header;

		// We only use readSaveGameHeader() here, which doesn't need an engine callback
		ZVision::SaveManager *zvisionSaveMan = new ZVision::SaveManager(NULL);
		bool successfulRead = zvisionSaveMan->readSaveGameHeader(in, header);
		delete zvisionSaveMan;
	    delete in;

	    if (successfulRead) {
	        SaveStateDescriptor desc(slot, header.saveName);

	        desc.setThumbnail(header.thumbnail);

	        if (header.version > 0) {
	            int day = header.saveDay;
	            int month = header.saveMonth;
	            int year = header.saveYear;

	            desc.setSaveDate(year, month, day);

	            int hour = header.saveHour;
	            int minutes = header.saveMinutes;

	            desc.setSaveTime(hour, minutes);

	            //desc.setPlayTime(header.playTime * 1000);
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
