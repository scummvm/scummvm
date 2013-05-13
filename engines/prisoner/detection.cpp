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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/file.h"

#include "prisoner/prisoner.h"


namespace Prisoner {

struct PrisonerGameDescription {
	ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

const char *PrisonerEngine::getGameId() const {
	return _gameDescription->desc.gameid;
}

uint32 PrisonerEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform PrisonerEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 PrisonerEngine::getVersion() const {
	return _gameDescription->version;
}

}

static const PlainGameDescriptor prisonerGames[] = {
	{"prisoner", "Prisoner of Ice"},
	{0, 0}
};

namespace Prisoner {

static const PrisonerGameDescription gameDescriptions[] = {

	{
		// Prisoner English version
		{
			"prisoner",
			0,
			AD_ENTRY1s("e_klang.bin", "898cc3ba5b382cf9eed2918bea56ac64", 561),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		0,
		0,
		0,
		0,
	},

	{
		// Prisoner Multilingual Version - English
		{
			"prisoner",
			0,
			AD_ENTRY1s("e_klang.bin", "c8e85c96425a2c5bd535410ef39fd9fe", 561),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		0,
		0,
		0,
		0,
	},

	{
		// Prisoner Multilingual Version - German
		{
			"prisoner",
			0,
			AD_ENTRY1s("d_klang.bin", "2c5e71abf3c7908f80db62fe850a459b", 561),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		0,
		0,
		0,
		0,
	},

	{
		// Prisoner Multilingual Version - French
		{
			"prisoner",
			0,
			AD_ENTRY1s("f_klang.bin", "08866dc389a00b2c3c611b782c68220c", 561),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		0,
		0,
		0,
		0,
	},

	{
		// Prisoner Multilingual Version - Dutch
		{
			"prisoner",
			0,
			AD_ENTRY1s("n_klang.bin", "dd8bcbe13bbf2cc2f5039dd4254b74b7", 561),
			Common::NL_NLD,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		0,
		0,
		0,
		0,
	},

	{
		// Prisoner Multilingual Version - Swedish
		{
			"prisoner",
			0,
			AD_ENTRY1s("w_klang.bin", "68ee7ecba3965975bb857d41026be4b7", 561),
			Common::SE_SWE,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		0,
		0,
		0,
		0,
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0, 0 }
};

/**
 * The fallback game descriptor used by the Prisoner engine's fallbackDetector.
 * Contents of this struct are to be overwritten by the fallbackDetector.
 */
static PrisonerGameDescription g_fallbackDesc = {
	{
		"",
		"",
		AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
		Common::UNK_LANG,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},
	0,
	0,
	0,
	0,
};

} // End of namespace Prisoner

class PrisonerMetaEngine : public AdvancedMetaEngine {
public:
	PrisonerMetaEngine() : AdvancedMetaEngine(Prisoner::gameDescriptions, sizeof(Prisoner::PrisonerGameDescription), prisonerGames) {
		_singleid = "prisoner";
	}

	virtual const char *getName() const {
		return "Prisoner Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Prisoner Engine (C) Infogrames";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;

	SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;

	const ADGameDescription *fallbackDetect(const Common::FSList &fslist) const;

};

bool PrisonerMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
//		(f == kSupportsDeleteSave) ||
	   	(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail);
}

bool Prisoner::PrisonerEngine::hasFeature(EngineFeature f) const {
	return
//		(f == kSupportsRTL) || // TODO: Not yet...
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool PrisonerMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Prisoner::PrisonerGameDescription *gd = (const Prisoner::PrisonerGameDescription *)desc;
	if (gd) {
		*engine = new Prisoner::PrisonerEngine(syst, gd);
	}
	return gd != 0;
}

const ADGameDescription *PrisonerMetaEngine::fallbackDetect(const Common::FSList &fslist) const {
	// Set the default values for the fallback descriptor's ADGameDescription part.
	Prisoner::g_fallbackDesc.desc.language = Common::UNK_LANG;
	Prisoner::g_fallbackDesc.desc.platform = Common::kPlatformDOS;
	Prisoner::g_fallbackDesc.desc.flags = ADGF_NO_FLAGS;

	// Set default values for the fallback descriptor's PrisonerGameDescription part.
	Prisoner::g_fallbackDesc.gameID = 0;
	Prisoner::g_fallbackDesc.features = 0;
	Prisoner::g_fallbackDesc.version = 3;

	return NULL;
}

SaveStateList PrisonerMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Prisoner::PrisonerEngine::SaveHeader header;
	Common::String pattern = target;
	pattern += ".???";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); file++) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Prisoner::PrisonerEngine::readSaveHeader(in, false, header) == Prisoner::PrisonerEngine::kRSHENoError) {
					saveList.push_back(SaveStateDescriptor(slotNum, header.description));
				}
				delete in;
			}
		}
	}

	return saveList;
}

int PrisonerMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void PrisonerMetaEngine::removeSaveState(const char *target, int slot) const {
	// Slot 0 can't be deleted, it's for restarting the game(s)
	if (slot == 0)
		return;

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String filename = Prisoner::PrisonerEngine::getSavegameFilename(target, slot);

	saveFileMan->removeSavefile(filename.c_str());

	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".???";
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		// Rename every slot greater than the deleted slot,
		// Also do not rename quicksaves.
		if (slotNum > slot && slotNum < 990) {
			// FIXME: Our savefile renaming done here is inconsitent with what we do in
			// GUI_v2::deleteMenu. While here we rename every slot with a greater equal
			// number of the deleted slot to deleted slot, deleted slot + 1 etc.,
			// we only rename the following slots in GUI_v2::deleteMenu until a slot
			// is missing.
			saveFileMan->renameSavefile(file->c_str(), filename.c_str());

			filename = Prisoner::PrisonerEngine::getSavegameFilename(target, ++slot);
		}
	}

}

SaveStateDescriptor PrisonerMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Prisoner::PrisonerEngine::getSavegameFilename(target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

	if (in) {
		Prisoner::PrisonerEngine::SaveHeader header;
		Prisoner::PrisonerEngine::kReadSaveHeaderError error;

		error = Prisoner::PrisonerEngine::readSaveHeader(in, true, header);
		delete in;

		if (error == Prisoner::PrisonerEngine::kRSHENoError) {
			SaveStateDescriptor desc(slot, header.description);

			desc.setDeletableFlag(false);
			desc.setWriteProtectedFlag(false);
			desc.setThumbnail(header.thumbnail);

			return desc;
		}
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(PRISONER)
	REGISTER_PLUGIN_DYNAMIC(PRISONER, PLUGIN_TYPE_ENGINE, PrisonerMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(PRISONER, PLUGIN_TYPE_ENGINE, PrisonerMetaEngine);
#endif
