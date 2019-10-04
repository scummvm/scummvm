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

#include "prince/prince.h"
#include "engines/advancedDetector.h"

namespace Prince {

struct PrinceGameDescription {
	ADGameDescription desc;
	PrinceGameType gameType;
};

int PrinceEngine::getGameType() const {
	return _gameDescription->gameType;
}

const char *PrinceEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

uint32 PrinceEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language PrinceEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

} // End of namespace Prince

static const PlainGameDescriptor princeGames[] = {
	{"prince", "The Prince and the Coward"},
	{0, 0}
};

namespace Prince {
static const PrinceGameDescription gameDescriptions[] = {
	{
		{
			"prince",
			"Galador: Der Fluch des Prinzen",
			AD_ENTRY1s("databank.ptc", "5fa03833177331214ec1354761b1d2ee", 3565031),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_USEEXTRAASTITLE,
			GUIO1(GUIO_NONE)
		},
		kPrinceDataDE
	},
	{
		{
			"prince",
			"Ksiaze i Tchorz",
			AD_ENTRY1s("databank.ptc", "48ec9806bda9d152acbea8ce31c93c49", 3435298),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_USEEXTRAASTITLE,
			GUIO1(GUIO_NONE)
		},
		kPrinceDataPL
	},
	{
		{
			"prince",
			"",
			AD_ENTRY1s("talktxt.dat", "02bb2372f19aca3c65896ed81b2cefb3", 125702),
			Common::RU_RUS,
			Common::kPlatformWindows,
			GF_EXTRACTED,
			GUIO1(GUIO_NONE)
		},
		kPrinceDataDE
	},
	{
		{
			"prince",
			"",
			AD_ENTRY1s("databank.ptc", "a67b55730f3d7064921bd2a59e1063a3", 3892982),
			Common::RU_RUS,
			Common::kPlatformWindows,
			GF_NOVOICES,
			GUIO1(GUIO_NONE)
		},
		kPrinceDataDE
	},
	{
		{
			"prince",
			"w/translation",
			{
				{"databank.ptc", 0, "5fa03833177331214ec1354761b1d2ee", 3565031},
				{"prince_translation.dat", 0, 0, -1},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_TRANSLATED,
			GUIO1(GUIO_NONE)
		},
		kPrinceDataDE
	},
	{
		{
			"prince",
			"w/translation",
			{
				{"databank.ptc", 0, "48ec9806bda9d152acbea8ce31c93c49", 3435298},
				{"prince_translation.dat", 0, 0, -1},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_TRANSLATED,
			GUIO1(GUIO_NONE)
		},
		kPrinceDataPL
	},
	{ AD_TABLE_END_MARKER, kPrinceDataUNK }
};

} // End of namespace Prince

const static char *directoryGlobs[] = {
	"all",
	0
};

class PrinceMetaEngine : public AdvancedMetaEngine {
public:
	PrinceMetaEngine() : AdvancedMetaEngine(Prince::gameDescriptions, sizeof(Prince::PrinceGameDescription), princeGames) {
		_singleId = "prince";
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

        virtual const char *getName() const {
                return "The Prince and the Coward";
        }

	virtual const char *getOriginalCopyright() const {
		return "The Prince and the Coward (C) 1996-97 Metropolis";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual int getMaximumSaveSlot() const { return 99; }
	virtual SaveStateList listSaves(const char *target) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	virtual void removeSaveState(const char *target, int slot) const;
};

bool PrinceMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSimpleSavesNames);
}

bool Prince::PrinceEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsRTL);
}

SaveStateList PrinceMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); filename++) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(filename->c_str() + filename->size() - 3);

		if (slotNum >= 0 && slotNum <= 99) {

			Common::InSaveFile *file = saveFileMan->openForLoading(*filename);
			if (file) {
				Prince::SavegameHeader header;

				// Check to see if it's a ScummVM savegame or not
				char buffer[kSavegameStrSize + 1];
				file->read(buffer, kSavegameStrSize + 1);

				if (!strncmp(buffer, kSavegameStr, kSavegameStrSize + 1)) {
					// Valid savegame
					if (Prince::PrinceEngine::readSavegameHeader(file, header)) {
						saveList.push_back(SaveStateDescriptor(slotNum, header.saveName));
					}
				} else {
					// Must be an original format savegame
					saveList.push_back(SaveStateDescriptor(slotNum, "Unknown"));
				}

				delete file;
			}
		}
	}

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor PrinceMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(fileName);

	if (f) {
		Prince::SavegameHeader header;

		// Check to see if it's a ScummVM savegame or not
		char buffer[kSavegameStrSize + 1];
		f->read(buffer, kSavegameStrSize + 1);

		bool hasHeader = !strncmp(buffer, kSavegameStr, kSavegameStrSize + 1) &&
			Prince::PrinceEngine::readSavegameHeader(f, header, false);
		delete f;

		if (!hasHeader) {
			// Original savegame perhaps?
			SaveStateDescriptor desc(slot, "Unknown");
			return desc;
		} else {
			// Create the return descriptor
			SaveStateDescriptor desc(slot, header.saveName);
			desc.setThumbnail(header.thumbnail);
			desc.setSaveDate(header.saveYear, header.saveMonth, header.saveDay);
			desc.setSaveTime(header.saveHour, header.saveMinutes);
			desc.setPlayTime(header.playTime * 1000);

			return desc;
		}
	}

	return SaveStateDescriptor();
}

void PrinceMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

bool PrinceMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	using namespace Prince;
	const PrinceGameDescription *gd = (const PrinceGameDescription *)desc;
	if (gd) {
		*engine = new PrinceEngine(syst, gd);
	}
	return gd != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(PRINCE)
REGISTER_PLUGIN_DYNAMIC(PRINCE, PLUGIN_TYPE_ENGINE, PrinceMetaEngine);
#else
REGISTER_PLUGIN_STATIC(PRINCE, PLUGIN_TYPE_ENGINE, PrinceMetaEngine);
#endif
