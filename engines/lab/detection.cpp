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

 /*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "engines/advancedDetector.h"

#include "lab/lab.h"

static const PlainGameDescriptor lab_setting[] = {
	{ "lab", "Labyrinth of Time" },
	{ 0, 0 }
};

static const ADGameDescription labDescriptions[] = {
	{
		"lab",
		"",
		{
			{ "doors",       0, "d77536010e7e5ae17ee066323ceb9585", 2537 }, // game/doors
			{ "noteold.fon", 0, "6c1d90ad55149556e79d3f7bfddb4bd7", 9252 }, // game/spict/noteold.fon
			{ NULL, 0, NULL, 0 }
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"lab",
		"Lowres",
		{
			{ "doors",   0, "d77536010e7e5ae17ee066323ceb9585", 2537 }, // game/doors
			{ "64b",     0, "3a84d41bcc6a782f22e8e954bce09721", 39916 }, // game/pict/h2/64b
			{ NULL, 0, NULL, 0 }
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		Lab::GF_LOWRES | ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"lab",
		"Rerelease",
		{
			{ "doors",   0, "d77536010e7e5ae17ee066323ceb9585", 2537 }, // game/doors
			{ "noteold.fon", 0, "6c1d90ad55149556e79d3f7bfddb4bd7", 9252 }, // game/spict/noteold.fon
			{ "wyrmkeep",0, "97c7064c54c28b952d37c4ebff6efa50", 52286 }, // game/spict/intro
			{ NULL, 0, NULL, 0 }
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"lab",
		"",
		AD_ENTRY1s("doors", "7bf458df6ec30cc8ef4665e4d7c77f59", 2537), // game/doors
		Common::EN_ANY,
		Common::kPlatformAmiga,
		Lab::GF_LOWRES | ADGF_UNSTABLE,
		GUIO0()
	},
	AD_TABLE_END_MARKER
};

static const char *const directoryGlobs[] = {
		"game",
		"pict",
		"spict",
		"rooms",
		"h2",
		"intro",
		0
};

namespace Lab {

Common::Platform LabEngine::getPlatform() const {
	return _gameDescription->platform;
}

uint32 LabEngine::getFeatures() const {
	return _gameDescription->flags | _extraGameFeatures;
}

} // End of namespace Lab

class LabMetaEngine : public AdvancedMetaEngine {
public:
	LabMetaEngine() : AdvancedMetaEngine(labDescriptions, sizeof(ADGameDescription), lab_setting) {
		_singleId = "lab";

		_maxScanDepth = 4;
		_directoryGlobs = directoryGlobs;
		_flags = kADFlagUseExtraAsHint;
	}

	virtual const char *getName() const {
		return "Labyrinth of Time";
	}

	virtual const char *getOriginalCopyright() const {
		return "Labyrinth of Time (C) 2004 The Wyrmkeep Entertainment Co. and Terra Nova Development";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
		// Instantiate Engine even if the game data is not found.
		*engine = new Lab::LabEngine(syst, desc);
		return true;
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool LabMetaEngine::hasFeature(MetaEngineFeature f) const {
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

bool Lab::LabEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

SaveStateList LabMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Lab::SaveGameHeader header;
	Common::String pattern = target;
	pattern += ".###";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());

	SaveStateList saveList;

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if ((slotNum >= 0) && (slotNum <= 999)) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Lab::readSaveGameHeader(in, header))
					saveList.push_back(SaveStateDescriptor(slotNum, header._descr.getDescription()));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int LabMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void LabMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	saveFileMan->removeSavefile(Common::String::format("%s.%03u", target, slot));
}

SaveStateDescriptor LabMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03u", target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

	if (in) {
		Lab::SaveGameHeader header;

		bool successfulRead = Lab::readSaveGameHeader(in, header, false);
		delete in;

		if (successfulRead) {
			SaveStateDescriptor desc(slot, header._descr.getDescription());
			// Do not allow save slot 0 (used for auto-saving) to be deleted or
			// overwritten.
			//desc.setDeletableFlag(slot != 0);
			//desc.setWriteProtectedFlag(slot == 0);

			return header._descr;
		}
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(LAB)
	REGISTER_PLUGIN_DYNAMIC(LAB, PLUGIN_TYPE_ENGINE, LabMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(LAB, PLUGIN_TYPE_ENGINE, LabMetaEngine);
#endif
