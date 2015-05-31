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

#define GAMEOPTION_ORIGINAL_SAVELOAD          GUIO_GAMEOPTIONS1
#define GAMEOPTION_DOUBLE_FPS                 GUIO_GAMEOPTIONS2
#define GAMEOPTION_ENABLE_VENUS               GUIO_GAMEOPTIONS3
#define GAMEOPTION_DISABLE_ANIM_WHILE_TURNING GUIO_GAMEOPTIONS4
#define GAMEOPTION_USE_HIRES_MPEG_MOVIES      GUIO_GAMEOPTIONS5

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
			GUIO4(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_DOUBLE_FPS, GAMEOPTION_ENABLE_VENUS, GAMEOPTION_DISABLE_ANIM_WHILE_TURNING)
		},
		GID_NEMESIS
	},

	{
		// Zork Nemesis French version
		{
			"znemesis",
			0,
			{{"CSCR.ZFS", 0, "f04113357b4748c13efcb58b4629887c", 2577873},
			 {"NEMESIS.STR", 0, "333bcb17bbb7f57cae742fbbe44f56f3", 9219},
			 AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO4(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_DOUBLE_FPS, GAMEOPTION_ENABLE_VENUS, GAMEOPTION_DISABLE_ANIM_WHILE_TURNING)
		},
		GID_NEMESIS
	},

	{
		// Zork Nemesis German version
		{
			"znemesis",
			0,
			{{"CSCR.ZFS", 0, "f04113357b4748c13efcb58b4629887c", 2577873},
			 {"NEMESIS.STR", 0, "3d1a12b907751653866cffc6d4dfb331", 9505},
			 AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO4(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_DOUBLE_FPS, GAMEOPTION_ENABLE_VENUS, GAMEOPTION_DISABLE_ANIM_WHILE_TURNING)
		},
		GID_NEMESIS
	},

	{
		// Zork Nemesis Italian version
		{
			"znemesis",
			0,
			{{"CSCR.ZFS", 0, "f04113357b4748c13efcb58b4629887c", 2577873},
			 {"NEMESIS.STR", 0, "7c568feca8d9f9ae855c47183612c305", 9061},
			 AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO4(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_DOUBLE_FPS, GAMEOPTION_ENABLE_VENUS, GAMEOPTION_DISABLE_ANIM_WHILE_TURNING)
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
			GUIO4(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_DOUBLE_FPS, GAMEOPTION_ENABLE_VENUS, GAMEOPTION_DISABLE_ANIM_WHILE_TURNING)
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
			GUIO3(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_DOUBLE_FPS, GAMEOPTION_DISABLE_ANIM_WHILE_TURNING)
		},
		GID_GRANDINQUISITOR
	},

	{
		// Zork Grand Inquisitor French CD version, reported by ulrichh on IRC
		{
			"zgi",
			"CD",
			AD_ENTRY1s("SCRIPTS.ZFS", "4d1ec4ade7ecc9ee9ec591d43ca3d213", 8338133),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_DOUBLE_FPS, GAMEOPTION_DISABLE_ANIM_WHILE_TURNING)
		},
		GID_GRANDINQUISITOR
	},

	{
		// Zork Grand Inquisitor German CD version, reported by breit in bug #6760
		{
			"zgi",
			"CD",
			AD_ENTRY1s("SCRIPTS.ZFS", "b7ac7e331b9b7f884590b0b325b560c8", 8338133),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_DOUBLE_FPS, GAMEOPTION_DISABLE_ANIM_WHILE_TURNING)
		},
		GID_GRANDINQUISITOR
	},

	{
		// Zork Grand Inquisitor Spanish CD version, reported by dianiu in bug #6764
		{
			"zgi",
			"CD",
			AD_ENTRY1s("SCRIPTS.ZFS", "5cdc4b99c1134053af135aae71326fd1", 8338141),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_DOUBLE_FPS, GAMEOPTION_DISABLE_ANIM_WHILE_TURNING)
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
			GUIO4(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_DOUBLE_FPS, GAMEOPTION_DISABLE_ANIM_WHILE_TURNING, GAMEOPTION_USE_HIRES_MPEG_MOVIES)
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
			GUIO3(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_DOUBLE_FPS, GAMEOPTION_DISABLE_ANIM_WHILE_TURNING)
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

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens, instead of the ScummVM ones"),
			"originalsaveload",
			false
		}
	},

	{
		GAMEOPTION_DOUBLE_FPS,
		{
			_s("Double FPS"),
			_s("Increase game FPS from 30 to 60"),
			"doublefps",
			false
		}
	},

	{
		GAMEOPTION_ENABLE_VENUS,
		{
			_s("Enable Venus"),
			_s("Enable the Venus help system"),
			"venusenabled",
			true
		}
	},

	{
		GAMEOPTION_DISABLE_ANIM_WHILE_TURNING,
		{
			_s("Disable animation while turning"),
			_s("Disable animation while turning in panoramic mode"),
			"noanimwhileturning",
			false
		}
	},

	{
		GAMEOPTION_USE_HIRES_MPEG_MOVIES,
		{
			_s("Use the hires MPEG movies"),
			_s("Use the hires MPEG movies of the DVD version, instead of the lowres AVI ones"),
			"mpegmovies",
			true
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class ZVisionMetaEngine : public AdvancedMetaEngine {
public:
	ZVisionMetaEngine() : AdvancedMetaEngine(ZVision::gameDescriptions, sizeof(ZVision::ZVisionGameDescription), zVisionGames, optionsList) {
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

			// Do not allow save slot 0 (used for auto-saving) to be deleted or
			// overwritten.
			desc.setDeletableFlag(slot != 0);
			desc.setWriteProtectedFlag(slot == 0);

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
