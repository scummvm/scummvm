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
#include "common/translation.h"

#include "graphics/thumbnail.h"

#include "engines/advancedDetector.h"

#include "adl/detection.h"

namespace Adl {

// Mystery House was designed for monochrome display, so we default to
// monochrome mode there. All the other games default to color mode.
#define GAMEOPTION_COLOR_DEFAULT_OFF GUIO_GAMEOPTIONS1
#define GAMEOPTION_SCANLINES         GUIO_GAMEOPTIONS2
#define GAMEOPTION_COLOR_DEFAULT_ON  GUIO_GAMEOPTIONS3

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_COLOR_DEFAULT_OFF,
		{
			_s("Color mode"),
			_s("Use color graphics"),
			"color",
			false
		}
	},

	{
		GAMEOPTION_COLOR_DEFAULT_ON,
		{
			_s("Color mode"),
			_s("Use color graphics"),
			"color",
			true
		}
	},

	{
		GAMEOPTION_SCANLINES,
		{
			_s("Scanlines"),
			_s("Show scanlines"),
			"scanlines",
			false
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

static const PlainGameDescriptor adlGames[] = {
	{ "hires0", "Hi-Res Adventure #0: Mission Asteroid" },
	{ "hires1", "Hi-Res Adventure #1: Mystery House" },
	{ "hires2", "Hi-Res Adventure #2: Wizard and the Princess" },
	{ "hires4", "Hi-Res Adventure #4: Ulysses and the Golden Fleece" },
	{ "hires5", "Hi-Res Adventure #5: Time Zone" },
	{ "hires6", "Hi-Res Adventure #6: The Dark Crystal" },
	{ 0, 0 }
};

static const AdlGameDescription gameDescriptions[] = {
	{ // Hi-Res Adventure #1: Mystery House - Apple II - 1987 PD release - Plain files
		{
			"hires1", 0,
			{
				{ "ADVENTURE", 0, "22d9e63a11d69fa033ba1738715ad09a", 29952 },
				{ "AUTO LOAD OBJ", 0, "23bfccfe9fcff9b22cf6c41bde9078ac", 12291 },
				{ "MYSTERY.HELLO", 0, "2289b7fea300b506e902a4c597968369", 836 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_COLOR_DEFAULT_OFF, GAMEOPTION_SCANLINES)
		},
		GAME_TYPE_HIRES1
	},
	{ // Hi-Res Adventure #1: Mystery House - Apple II - 1987 PD release - .DSK format
		{
			"hires1", 0,
			{
				{ "MYSTHOUS.DSK", 0, "34ba05e62bf51404c4475c349ca48921", 143360 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_COLOR_DEFAULT_OFF, GAMEOPTION_SCANLINES)
		},
		GAME_TYPE_HIRES1
	},
	{ // Hi-Res Adventure #2: Wizard and the Princess - Apple II - Roberta Williams Anthology
		{
			"hires2", 0,
			{
				{ "WIZARD.DSK", 0, "816fdfc35e25496213c8db40ecf26569", 143360 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_COLOR_DEFAULT_ON, GAMEOPTION_SCANLINES)
		},
		GAME_TYPE_HIRES2
	},
	{ // Hi-Res Adventure #0: Mission Asteroid - Apple II - Roberta Williams Anthology
		{
			"hires0", 0,
			{
				{ "MISSION.NIB", 0, "b158f6f79681d4edd651e1932f9e01d7", 232960 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_COLOR_DEFAULT_ON, GAMEOPTION_SCANLINES)
		},
		GAME_TYPE_HIRES0
	},
	{ // Hi-Res Adventure #4: Ulysses and the Golden Fleece - Apple II - Load 'N' Go
		{
			"hires4", 0,
			{
				{ "ULYSSESA.DSK", 0, "df21f28ae94440f958dbbcfdfaf0c36e", 143360 },
				{ "ULYSSESB.DSK", 1, "c204e8fe265e9534049f3c0f816cc9fc", 143360 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_TESTING,
			GUIO2(GAMEOPTION_COLOR_DEFAULT_ON, GAMEOPTION_SCANLINES)
		},
		GAME_TYPE_HIRES4
	},
	{ // Hi-Res Adventure #4: Ulysses and the Golden Fleece - Atari 8-bit - Re-release
		{
			"hires4", 0,
			{
				{ "ULYS1A.XFD", 0, "26365d2b06509fd21e7a7919e33f7199", 92160 },
				{ "ULYS1B.XFD", 0, "37919c72a4103e6f897ee7daa8261d1d", 92160 },
				// Load 'N' Go Software release XAG-0646 appears to be missing the second disk
				{ "ULYS2C.XFD", 0, "ff33830246e65dc71c954acb7fd5621a", 92160 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformAtari8Bit,
			ADGF_UNSTABLE,
			GUIO2(GAMEOPTION_COLOR_DEFAULT_ON, GAMEOPTION_SCANLINES)
		},
		GAME_TYPE_HIRES4
	},
	{ // Hi-Res Adventure #5: Time Zone - Apple II - Version 1.1 - Roberta Williams Anthology
		{
			"hires5", 0,
			{
				{ "TZONE1A.NIB", 2, "475dedb7396fdcea81c1a2a4046caebe", 232960 },
				{ "TZONE1B.NIB", 3, "f8aaea094ebbe41cf4354d9fe2c30d9a", 232960 },
				{ "TZONE2C.NIB", 4, "b351a367dc48e776bf08e42a3f50ae74", 232960 },
				{ "TZONE2D.NIB", 5, "9583b287a5c95960f5335878102bb8b1", 232960 },
				{ "TZONE3E.NIB", 6, "502e42a0cb69ffe4a48cd51c1ff210cf", 232960 },
				{ "TZONE3F.NIB", 7, "3d6e0aae15f590b72b6759535b6b7d3c", 232960 },
				{ "TZONE4G.NIB", 8, "ede4113a9c9e17745faf71d099808a18", 232960 },
				{ "TZONE4H.NIB", 9, "f95dae4aae1155a27f7120230464d4e1", 232960 },
				{ "TZONE5I.NIB", 10, "92b3b376877f81a7b7ae426bf1e65456", 232960 },
				{ "TZONE5J.NIB", 11, "c9ef796fa596548dbf8f085901f0bac3", 232960 },
				{ "TZONE6K.NIB", 12, "2e5323be637002efce1d4c813ae20a3f", 232960 },
				{ "TZONE6L.NIB", 13, "7c9268f0ea2d02120c77a46337b3d975", 232960 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_TESTING,
			GUIO2(GAMEOPTION_COLOR_DEFAULT_ON, GAMEOPTION_SCANLINES)
		},
		GAME_TYPE_HIRES5
	},
	{ // Hi-Res Adventure #6: The Dark Crystal - Apple II - Roberta Williams Anthology
		{
			"hires6", 0,
			{
				{ "DARK1A.DSK", 0, "00c2646d6943d1405717332a6f42d493", 143360 },
				{ "DARK1B.NIB", 3, "dbedd736617343ade0e6bead8bf2b10c", 232960 },
				{ "DARK2A.NIB", 4, "271eb92db107e8d5829437f8ba77991e", 232960 },
				{ "DARK2B.NIB", 5, "cb72044a9b391c4285f4752f746bea2e", 232960 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_TESTING,
			GUIO2(GAMEOPTION_COLOR_DEFAULT_ON, GAMEOPTION_SCANLINES)
		},
		GAME_TYPE_HIRES6
	},
	{ AD_TABLE_END_MARKER, GAME_TYPE_NONE }
};

class AdlMetaEngine : public AdvancedMetaEngine {
public:
	AdlMetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(AdlGameDescription), adlGames, optionsList) { }

	const char *getName() const {
		return "ADL";
	}

	const char *getOriginalCopyright() const {
		return "Copyright (C) Sierra On-Line";
	}

	bool hasFeature(MetaEngineFeature f) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	int getMaximumSaveSlot() const { return 'O' - 'A'; }
	SaveStateList listSaves(const char *target) const;
	void removeSaveState(const char *target, int slot) const;

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;
};

bool AdlMetaEngine::hasFeature(MetaEngineFeature f) const {
	switch(f) {
	case kSupportsListSaves:
	case kSupportsLoadingDuringStartup:
	case kSupportsDeleteSave:
	case kSavesSupportMetaInfo:
	case kSavesSupportThumbnail:
	case kSavesSupportCreationDate:
	case kSavesSupportPlayTime:
	case kSimpleSavesNames:
		return true;
	default:
		return false;
	}
}

SaveStateDescriptor AdlMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.s%02d", target, slot);
	Common::InSaveFile *inFile = g_system->getSavefileManager()->openForLoading(fileName);

	if (!inFile)
		return SaveStateDescriptor();

	if (inFile->readUint32BE() != MKTAG('A', 'D', 'L', ':')) {
		delete inFile;
		return SaveStateDescriptor();
	}

	byte saveVersion = inFile->readByte();
	if (saveVersion != SAVEGAME_VERSION) {
		delete inFile;
		return SaveStateDescriptor();
	}

	char name[SAVEGAME_NAME_LEN] = { };
	inFile->read(name, sizeof(name) - 1);
	inFile->readByte();

	if (inFile->eos() || inFile->err()) {
		delete inFile;
		return SaveStateDescriptor();
	}

	SaveStateDescriptor sd(slot, name);

	int year = inFile->readUint16BE();
	int month = inFile->readByte();
	int day = inFile->readByte();
	sd.setSaveDate(year + 1900, month + 1, day);

	int hour = inFile->readByte();
	int minutes = inFile->readByte();
	sd.setSaveTime(hour, minutes);

	uint32 playTime = inFile->readUint32BE();
	sd.setPlayTime(playTime);

	if (inFile->eos() || inFile->err()) {
		delete inFile;
		return SaveStateDescriptor();
	}

	Graphics::Surface *const thumbnail = Graphics::loadThumbnail(*inFile);
	sd.setThumbnail(thumbnail);

	delete inFile;
	return sd;
}

SaveStateList AdlMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray files = saveFileMan->listSavefiles(Common::String(target) + ".s##");

	SaveStateList saveList;

	for (uint i = 0; i < files.size(); ++i) {
		const Common::String &fileName = files[i];
		Common::InSaveFile *inFile = saveFileMan->openForLoading(fileName);
		if (!inFile) {
			warning("Cannot open save file '%s'", fileName.c_str());
			continue;
		}

		if (inFile->readUint32BE() != MKTAG('A', 'D', 'L', ':')) {
			warning("No header found in '%s'", fileName.c_str());
			delete inFile;
			continue;
		}

		byte saveVersion = inFile->readByte();
		if (saveVersion != SAVEGAME_VERSION) {
			warning("Unsupported save game version %i found in '%s'", saveVersion, fileName.c_str());
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

void AdlMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.s%02d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

Engine *HiRes1Engine_create(OSystem *syst, const AdlGameDescription *gd);
Engine *HiRes2Engine_create(OSystem *syst, const AdlGameDescription *gd);
Engine *HiRes0Engine_create(OSystem *syst, const AdlGameDescription *gd);
Engine *HiRes4Engine_create(OSystem *syst, const AdlGameDescription *gd);
Engine *HiRes5Engine_create(OSystem *syst, const AdlGameDescription *gd);
Engine *HiRes6Engine_create(OSystem *syst, const AdlGameDescription *gd);

bool AdlMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (!gd)
		return false;

	const AdlGameDescription *adlGd = (const AdlGameDescription *)gd;

	switch (adlGd->gameType) {
	case GAME_TYPE_HIRES1:
		*engine = HiRes1Engine_create(syst, adlGd);
		break;
	case GAME_TYPE_HIRES2:
		*engine = HiRes2Engine_create(syst, adlGd);
		break;
	case GAME_TYPE_HIRES0:
		*engine = HiRes0Engine_create(syst, adlGd);
		break;
	case GAME_TYPE_HIRES4:
		*engine = HiRes4Engine_create(syst, adlGd);
		break;
	case GAME_TYPE_HIRES5:
		*engine = HiRes5Engine_create(syst, adlGd);
		break;
	case GAME_TYPE_HIRES6:
		*engine = HiRes6Engine_create(syst, adlGd);
		break;
	default:
		error("Unknown GameType");
	}

	return true;
}

} // End of namespace Adl

#if PLUGIN_ENABLED_DYNAMIC(ADL)
	REGISTER_PLUGIN_DYNAMIC(ADL, PLUGIN_TYPE_ENGINE, Adl::AdlMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ADL, PLUGIN_TYPE_ENGINE, Adl::AdlMetaEngine);
#endif
