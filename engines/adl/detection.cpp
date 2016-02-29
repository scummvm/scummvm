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

#include "common/system.h"
#include "common/savefile.h"

#include "engines/advancedDetector.h"

#include "adl/adl.h"

namespace Adl {

struct AdlGameDescription {
	ADGameDescription desc;
	GameType gameType;
};

static const PlainGameDescriptor adlGames[] = {
	{"hires1", "Hi-Res Adventure #1: Mystery House"},
	{0, 0}
};

static const AdlGameDescription gameDescriptions[] = {

	{ // MD5 by waltervn
		{
			"hires1", 0,
			{
				{"ADVENTURE", 0, "22d9e63a11d69fa033ba1738715ad09a", 29952},
				{"AUTO LOAD OBJ", 0, "23bfccfe9fcff9b22cf6c41bde9078ac", 12291},
				{"MYSTERY.HELLO", 0, "2289b7fea300b506e902a4c597968369", 836},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2GS, // FIXME
			ADGF_NO_FLAGS,
			GUIO0()
		},
		kGameTypeHires1
	},
	{AD_TABLE_END_MARKER, kGameTypeNone}
};

class AdlMetaEngine : public AdvancedMetaEngine {
public:
	AdlMetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(AdlGameDescription), adlGames) { }

	const char *getName() const {
		return "ADL";
	}

	const char *getOriginalCopyright() const {
		return "Copyright (C) Sierra On-Line";
	}

	bool hasFeature(MetaEngineFeature f) const;
	int getMaximumSaveSlot() const { return 15; }
	SaveStateList listSaves(const char *target) const;

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;
};

bool AdlMetaEngine::hasFeature(MetaEngineFeature f) const {
	switch(f) {
	case kSupportsListSaves:
	case kSupportsLoadingDuringStartup:
		return true;
	default:
		return false;
	}
}

SaveStateList AdlMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray files = saveFileMan->listSavefiles(Common::String(target) + ".s##");

	SaveStateList saveList;
	for (uint i = 0; i < files.size(); ++i) {
		const Common::String &fileName = files[i];
		Common::InSaveFile *inFile = saveFileMan->openForLoading(fileName);
		if (!inFile) {
			warning("Cannot open save file %s", fileName.c_str());
			continue;
		}

		if (inFile->readUint32BE() != MKTAG('A', 'D', 'L', ':')) {
			warning("No header found in '%s'", fileName.c_str());
			delete inFile;
			continue;
		}

		byte saveVersion = inFile->readByte();
		if (saveVersion != SAVEGAME_VERSION) {
			warning("Save game version %i not supported in '%s'", saveVersion, fileName.c_str());
			delete inFile;
			continue;
		}

		char name[SAVEGAME_NAME_LEN] = { };
		inFile->read(name, sizeof(name) - 1);
		delete inFile;

		int slotNum = atoi(fileName.c_str() + fileName.size() - 2);
		SaveStateDescriptor sd(slotNum, name);
		saveList.push_back(sd);
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

bool AdlMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (gd)
		*engine = AdlEngine::create(((const AdlGameDescription *)gd)->gameType, syst, (const AdlGameDescription *)gd);
	return gd != nullptr;
}

} // End of namespace Adl

#if PLUGIN_ENABLED_DYNAMIC(ADL)
	REGISTER_PLUGIN_DYNAMIC(ADL, PLUGIN_TYPE_ENGINE, Adl::AdlMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ADL, PLUGIN_TYPE_ENGINE, Adl::AdlMetaEngine);
#endif
