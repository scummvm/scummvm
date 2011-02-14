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
 * $URL: https://scummvm-misc.svn.sourceforge.net/svnroot/scummvm-misc/trunk/engines/tsage/detection.cpp $
 * $Id: detection.cpp 209 2011-02-06 00:46:36Z dreammaster $
 *
 */

#include "common/config-manager.h"
#include "common/system.h"
#include "common/savefile.h"

#include "engines/advancedDetector.h"

#include "base/plugins.h"

#include "tsage/tsage.h"

static const PlainGameDescriptor TSAgeGameTitles[] = {
	{ "tsage", "Unknown Tsunami TSAGE-based Game" },
	{ "ring", "Ringworld: Revenge of the Patriarch" },
	{ "blue", "Blue Force" },
	{ 0, 0 }
};

namespace tSage {

static const ADGameDescription TSAgeGameDescriptions[] = {
	// Ringworld English CD version
	{ 
		"ring",
		"CD",
		AD_ENTRY1s("ring.rlb", "466f0e6492d9d0f34d35c5cd088de90f", 37847618),
		Common::EN_ANY,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		Common::GUIO_NONE
	},
	// Ringworld English Floppy version
	{
		"ring",
		"Floppy",
		AD_ENTRY1s("ring.rlb", "61f78f68a56832ae95fe06748c403234", 8438770),
		Common::EN_ANY,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		Common::GUIO_NONE
	},
	// Blue Force
	{
		"blue",
		"",
		AD_ENTRY1s("blue.rlb", "467da43c848cc0e800b547c59d84ccb1", 10032614),
		Common::EN_ANY,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		Common::GUIO_NONE
	},

	AD_TABLE_END_MARKER,
};

const char *TSageEngine::getGameId() const {
	return _gameDescription->gameid;
}

} // End of namespace tSage

static const ADGameDescription TSAgeGameGeneric[] = {
	{"tsage", 0,
		AD_ENTRY1("tsage.rlb", NULL),
		Common::UNK_LANG,
		Common::kPlatformUnknown,
		0,
		Common::GUIO_NONE
	},
	AD_TABLE_END_MARKER
};

static const ADFileBasedFallback TSAgeGameFallback[] = {
	{(const void*)&TSAgeGameGeneric[0], {"ring.rlb", NULL} },
	{(const void*)&TSAgeGameGeneric[0], {"blue.rlb", NULL} },
	{0, {NULL}}
};

static const ADParams detectionParams = {
	(const byte *)tSage::TSAgeGameDescriptions,
	sizeof(ADGameDescription),
	0,
	TSAgeGameTitles,
	0,
	"tsage",
	TSAgeGameFallback,
	kADFlagPrintWarningOnFileBasedFallback,
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
		return "TsAGE Engine";
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
			*engine = new tSage::TSageEngine(syst, desc);
		}
		return desc != 0;
	}

	static Common::String generateGameStateFileName(const char *target, int slot) {
		return Common::String::format("%s.%03d", target, slot);
	}

	virtual SaveStateList listSaves(const char *target) const {
		Common::String pattern = target;
		pattern += ".*";

		Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles(pattern);
		tSage::tSageSavegameHeader header;

		SaveStateList saveList;
		for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
			int slot;
			const char *ext = strrchr(file->c_str(), '.');
			if (ext && (slot = atoi(ext + 1)) >= 0 && slot < MAX_SAVES) {
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
