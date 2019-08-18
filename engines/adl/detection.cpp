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
#include "common/file.h"
#include "common/md5.h"
#include "common/debug.h"

#include "graphics/thumbnail.h"

#include "engines/advancedDetector.h"

#include "adl/detection.h"
#include "adl/disk.h"

namespace Adl {

// Mystery House was designed for monochrome display, so we default to
// monochrome mode there. All the other games default to color mode.
#define GAMEOPTION_COLOR_DEFAULT_OFF GUIO_GAMEOPTIONS1
#define GAMEOPTION_SCANLINES         GUIO_GAMEOPTIONS2
#define GAMEOPTION_COLOR_DEFAULT_ON  GUIO_GAMEOPTIONS3
#define GAMEOPTION_NTSC              GUIO_GAMEOPTIONS4
#define GAMEOPTION_MONO_TEXT         GUIO_GAMEOPTIONS5

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_NTSC,
		{
			_s("TV emulation"),
			_s("Emulate composite output to an NTSC TV"),
			"ntsc",
			true
		}
	},

	{
		GAMEOPTION_COLOR_DEFAULT_OFF,
		{
			_s("Color graphics"),
			_s("Use color graphics instead of monochrome"),
			"color",
			false
		}
	},

	{
		GAMEOPTION_COLOR_DEFAULT_ON,
		{
			_s("Color graphics"),
			_s("Use color graphics instead of monochrome"),
			"color",
			true
		}
	},

	{
		GAMEOPTION_SCANLINES,
		{
			_s("Show scanlines"),
			_s("Darken every other scanline to mimic the look of a CRT"),
			"scanlines",
			false
		}
	},

	{
		GAMEOPTION_MONO_TEXT,
		{
			_s("Always use sharp monochrome text"),
			_s("Do not emulate NTSC artifacts for text"),
			"monotext",
			true
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

#define DEFAULT_OPTIONS GUIO4(GAMEOPTION_NTSC, GAMEOPTION_COLOR_DEFAULT_ON, GAMEOPTION_MONO_TEXT, GAMEOPTION_SCANLINES)
#define MH_OPTIONS GUIO4(GAMEOPTION_NTSC, GAMEOPTION_COLOR_DEFAULT_OFF, GAMEOPTION_MONO_TEXT, GAMEOPTION_SCANLINES)

static const PlainGameDescriptor adlGames[] = {
	{ "hires0", "Hi-Res Adventure #0: Mission Asteroid" },
	{ "hires1", "Hi-Res Adventure #1: Mystery House" },
	{ "hires2", "Hi-Res Adventure #2: Wizard and the Princess" },
	{ "hires3", "Hi-Res Adventure #3: Cranston Manor" },
	{ "hires4", "Hi-Res Adventure #4: Ulysses and the Golden Fleece" },
	{ "hires5", "Hi-Res Adventure #5: Time Zone" },
	{ "hires6", "Hi-Res Adventure #6: The Dark Crystal" },
	{ 0, 0 }
};

struct AdlGameDescription {
	ADGameDescription desc;
	GameType gameType;
	GameVersion version;
};

static const AdlGameDescription gameFileDescriptions[] = {
	{ // Hi-Res Adventure #1: Mystery House - Apple II - Contains Simi Valley address
		{
			"hires1", 0,
			{
				{ "ADVENTURE", 0, "22d9e63a11d69fa033ba1738715ad09a", 29952 },
				{ "AUTO LOAD OBJ", 0, "a2ab7be25842e1fa9f1343b0894a8b6f", 4095 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_NO_FLAGS,
			MH_OPTIONS
		},
		GAME_TYPE_HIRES1,
		GAME_VER_HR1_SIMI
	},
	{ // Hi-Res Adventure #1: Mystery House - Apple II - Contains Coarsegold address
		{
			"hires1", 0,
			{
				{ "ADVENTURE", 0, "22d9e63a11d69fa033ba1738715ad09a", 29952 },
				{ "AUTO LOAD OBJ", 0, "f6a6ac60c04c6ba6dff68b92cc279ba2", 12291 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_NO_FLAGS,
			MH_OPTIONS
		},
		GAME_TYPE_HIRES1,
		GAME_VER_HR1_COARSE
	},
	{ // Hi-Res Adventure #1: Mystery House - Apple II - Roberta Williams Anthology
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
			MH_OPTIONS
		},
		GAME_TYPE_HIRES1,
		GAME_VER_HR1_PD
	},
	{ AD_TABLE_END_MARKER, GAME_TYPE_NONE, GAME_VER_NONE }
};

static const AdlGameDescription gameDiskDescriptions[] = {
	{ // Hi-Res Adventure #1: Mystery House - Apple II - Contains Coarsegold address
		{
			"hires1", "",
			{
				{ "mysthous", 0, "8df0b3b3e609a2e40237e2419c1cb767", 116480 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_NO_FLAGS,
			MH_OPTIONS
		},
		GAME_TYPE_HIRES1,
		GAME_VER_HR1_COARSE
	},
	{ // Hi-Res Adventure #1: Mystery House - Apple II - Roberta Williams Anthology
		{
			"hires1", "",
			{
				{ "mysthous", 0, "54d20eb1ef0084ac3c2d16c31c5b7eb7", 143360 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_NO_FLAGS,
			MH_OPTIONS
		},
		GAME_TYPE_HIRES1,
		GAME_VER_HR1_PD
	},
	{ // Hi-Res Adventure #2: Wizard and the Princess - Apple II - Roberta Williams Anthology
		{
			"hires2", "",
			{
				{ "wizard", 0, "72b114bf8f94fafe5672daac2a70c765", 143360 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_NO_FLAGS,
			DEFAULT_OPTIONS
		},
		GAME_TYPE_HIRES2,
		GAME_VER_NONE
	},
	{ // Hi-Res Adventure #0: Mission Asteroid - Apple II - Roberta Williams Anthology
		{
			"hires0", "",
			{
				{ "mission", 0, "6bc53f51a3c8ee65c020af55fb8bd875", 116480 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_NO_FLAGS,
			DEFAULT_OPTIONS
		},
		GAME_TYPE_HIRES0,
		GAME_VER_NONE
	},
	{ // Hi-Res Adventure #3: Cranston Manor - Apple II
		{
			"hires3", "",
			{
				{ "cranston", 0, "e4d35440791a36e55299c7be1ccd2b04", 116480 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_NO_FLAGS,
			DEFAULT_OPTIONS
		},
		GAME_TYPE_HIRES3,
		GAME_VER_NONE
	},
	{ // Hi-Res Adventure #4: Ulysses and the Golden Fleece - Apple II - Load 'N' Go
		{
			"hires4", "",
			{
				{ "ulyssesa", 0, "1eaeb2f1a773ce2d1cb9f16b2ef09049", 143360 },
				{ "ulyssesb", 1, "9fa8552255ae651b252844168b8b6617", 143360 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_NO_FLAGS,
			DEFAULT_OPTIONS
		},
		GAME_TYPE_HIRES4,
		GAME_VER_NONE
	},
	{ // Hi-Res Adventure #4: Ulysses and the Golden Fleece - Atari 8-bit - Re-release
		{
			"hires4", "",
			{
				{ "ulys1b", 0, "bb6aab9a35b41d160b6eefa088165f56", 92160 },
				{ "ulys1a", 0, "c227eeee34d0bacd62b2d6231c409204", 92160 },
				// Load 'N' Go Software release XAG-0646 appears to be missing the second disk
				{ "ulys2c", 0, "8c6a76d1767e4ffa2f0118c9c56c0e90", 92160 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformAtari8Bit,
			ADGF_UNSTABLE,
			GUIO2(GAMEOPTION_COLOR_DEFAULT_ON, GAMEOPTION_SCANLINES)
		},
		GAME_TYPE_HIRES4,
		GAME_VER_NONE
	},
	{ // Hi-Res Adventure #5: Time Zone - Apple II - Version 1.1 - Roberta Williams Anthology
		{
			"hires5", "",
			{
				{ "tzone1a", 2, "731844b1d19c2801e3a5bc61d109af54", 143360 },
				{ "tzone1b", 3, "4eaf8d790e3f93097cca9ddbe863df50", 143360 },
				{ "tzone2c", 4, "e3aa4f56e727339b1ec00978ce9d435b", 143360 },
				{ "tzone2d", 5, "77b8219a380410015c986fa192d4c3bf", 143360 },
				{ "tzone3e", 6, "f7acc03edd8d8aecb90711cd5f9e5593", 143360 },
				{ "tzone3f", 7, "ed74c056976ecea2eab07448c8a72eb8", 143360 },
				{ "tzone4g", 8, "de7bda8a641169fc2dedd8a7b0b7e7de", 143360 },
				{ "tzone4h", 9, "21cf76d97505ff09fff5d5e4711bc47c", 143360 },
				{ "tzone5i", 10, "d665df374e594cd0978b73c3490e5de2", 143360 },
				{ "tzone5j", 11, "5095be23d13201d0897b9169c4e473df", 143360 },
				{ "tzone6k", 12, "bef044503f21af5f0a4088e99aa778b1", 143360 },
				{ "tzone6l", 13, "84801b7c2ab6c09e62a2a0809b94d16a", 143360 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_NO_FLAGS,
			DEFAULT_OPTIONS
		},
		GAME_TYPE_HIRES5,
		GAME_VER_NONE
	},
	{ // Hi-Res Adventure #6: The Dark Crystal - Apple II - Roberta Williams Anthology / SierraVenture
		{
			"hires6", "SierraVenture [version A]",
			{
				{ "dark1a", 0, "9a5968a8f378c84454d88f4cd4e143a9", 143360 },
				{ "dark1b", 3, "1271ff9c3e1bdb4942301dd37dd0ef87", 143360 },
				{ "dark2a", 4, "090e77563add7b4c9ab25f444d727316", 143360 },
				{ "dark2b", 5, "f2db96af0955324900b800505af4d91f", 143360 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_NO_FLAGS,
			DEFAULT_OPTIONS
		},
		GAME_TYPE_HIRES6,
		GAME_VER_NONE
	},
	{ // Hi-Res Adventure #6: The Dark Crystal - Apple II - SierraVenture
		{
			"hires6", "SierraVenture [version B]",
			{
				{ "dark1a", 0, "d0b8e808b02564b6ce58b5ea5cc61ead", 143360 },
				{ "dark1b", 3, "1271ff9c3e1bdb4942301dd37dd0ef87", 143360 },
				{ "dark2a", 4, "090e77563add7b4c9ab25f444d727316", 143360 },
				{ "dark2b", 5, "f2db96af0955324900b800505af4d91f", 143360 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformApple2,
			ADGF_NO_FLAGS,
			DEFAULT_OPTIONS
		},
		GAME_TYPE_HIRES6,
		GAME_VER_NONE
	},
	{ AD_TABLE_END_MARKER, GAME_TYPE_NONE, GAME_VER_NONE }
};

struct DiskImageExt {
	Common::Platform platform;
	const char *extension;
};

const DiskImageExt diskImageExts[] = {
	{ Common::kPlatformApple2, ".woz" },
	{ Common::kPlatformApple2, ".nib" },
	{ Common::kPlatformApple2, ".dsk" },
	{ Common::kPlatformApple2, ".d13" },
	{ Common::kPlatformAtari8Bit, ".xfd" },
	{ Common::kPlatformDOS, ".img" }
};

class AdlMetaEngine : public AdvancedMetaEngine {
public:
	AdlMetaEngine() : AdvancedMetaEngine(gameFileDescriptions, sizeof(AdlGameDescription), adlGames, optionsList) { }

	const char *getName() const override {
		return "ADL";
	}

	const char *getOriginalCopyright() const override {
		return "Copyright (C) Sierra On-Line";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	int getMaximumSaveSlot() const override { return 'O' - 'A'; }
	SaveStateList listSaves(const char *target) const override;
	void removeSaveState(const char *target, int slot) const override;
	ADDetectedGames detectGame(const Common::FSNode &parent, const FileMap &allFiles, Common::Language language, Common::Platform platform, const Common::String &extra) const override;

	bool addFileProps(const FileMap &allFiles, Common::String fname, FilePropertiesMap &filePropsMap) const;

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const override;
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

	Graphics::Surface *thumbnail;
	if (!Graphics::loadThumbnail(*inFile, thumbnail)) {
		delete inFile;
		return SaveStateDescriptor();
	}
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

Common::String getDiskImageName(const AdlGameDescription &adlDesc, byte volume) {
	const ADGameDescription &desc = adlDesc.desc;
	for (uint i = 0; desc.filesDescriptions[i].fileName; ++i) {
		const ADGameFileDescription &fDesc = desc.filesDescriptions[i];

		if (fDesc.fileType == volume) {
			for (uint e = 0; e < ARRAYSIZE(diskImageExts); ++e) {
				if (diskImageExts[e].platform == desc.platform) {
					Common::String testFileName(fDesc.fileName);
					testFileName += diskImageExts[e].extension;
					if (Common::File::exists(testFileName))
						return testFileName;
				}
			}

			error("Failed to find disk image '%s'", fDesc.fileName);
		}
	}

	error("Disk volume %d not found", volume);
}

GameType getGameType(const AdlGameDescription &adlDesc) {
	return adlDesc.gameType;
}

GameVersion getGameVersion(const AdlGameDescription &adlDesc) {
	return adlDesc.version;
}

Common::Platform getPlatform(const AdlGameDescription &adlDesc) {
	return adlDesc.desc.platform;
}

bool AdlMetaEngine::addFileProps(const FileMap &allFiles, Common::String fname, FilePropertiesMap &filePropsMap) const {
	if (filePropsMap.contains(fname))
		return true;

	if (!allFiles.contains(fname))
		return false;

	FileProperties fileProps;
	fileProps.size = computeMD5(allFiles[fname], fileProps.md5, 16384);

	if (fileProps.size != -1) {
		debug(3, "> '%s': '%s'", fname.c_str(), fileProps.md5.c_str());
		filePropsMap[fname] = fileProps;
	}

	return true;
}

// Based on AdvancedMetaEngine::detectGame
ADDetectedGames AdlMetaEngine::detectGame(const Common::FSNode &parent, const FileMap &allFiles, Common::Language language, Common::Platform platform, const Common::String &extra) const {
	// We run the file-based detector first, if it finds a match we do not search for disk images
	ADDetectedGames matched = AdvancedMetaEngine::detectGame(parent, allFiles, language, platform, extra);

	if (!matched.empty())
		return matched;

	debug(3, "Starting disk image detection in dir '%s'", parent.getPath().c_str());

	FilePropertiesMap filesProps;

	for (uint g = 0; gameDiskDescriptions[g].desc.gameId != 0; ++g) {
		ADDetectedGame game(&gameDiskDescriptions[g].desc);

		// Skip games that don't meet the language/platform/extra criteria
		if (language != Common::UNK_LANG && game.desc->language != Common::UNK_LANG) {
			if (game.desc->language != language && !(language == Common::EN_ANY && (game.desc->flags & ADGF_ADDENGLISH)))
				continue;
		}

		if (platform != Common::kPlatformUnknown && game.desc->platform != Common::kPlatformUnknown && game.desc->platform != platform)
			continue;

		if ((_flags & kADFlagUseExtraAsHint) && !extra.empty() && game.desc->extra != extra)
			continue;

		bool allFilesPresent = true;

		for (uint f = 0; game.desc->filesDescriptions[f].fileName; ++f) {
			const ADGameFileDescription &fDesc = game.desc->filesDescriptions[f];
			Common::String fileName;
			bool foundDiskImage = false;

			for (uint e = 0; e < ARRAYSIZE(diskImageExts); ++e) {
				if (diskImageExts[e].platform == game.desc->platform) {
					Common::String testFileName(fDesc.fileName);
					testFileName += diskImageExts[e].extension;

					if (addFileProps(allFiles, testFileName, filesProps)) {
						if (foundDiskImage) {
							warning("Ignoring '%s' (already found '%s')", testFileName.c_str(), fileName.c_str());
							filesProps.erase(testFileName);
						} else {
							foundDiskImage = true;
							fileName = testFileName;
						}
					}
				}
			}

			if (!foundDiskImage) {
				allFilesPresent = false;
				break;
			}

			game.matchedFiles[fileName] = filesProps[fileName];

			if (game.hasUnknownFiles)
				continue;

			if (fDesc.md5 && fDesc.md5 != filesProps[fileName].md5) {
				debug(3, "MD5 Mismatch. Skipping (%s) (%s)", fDesc.md5, filesProps[fileName].md5.c_str());
				game.hasUnknownFiles = true;
				continue;
			}

			if (fDesc.fileSize != -1 && fDesc.fileSize != filesProps[fileName].size) {
				debug(3, "Size Mismatch. Skipping");
				game.hasUnknownFiles = true;
				continue;
			}

			debug(3, "Matched file: %s", fileName.c_str());
		}

		// This assumes that the detection table groups together games that have the same gameId and platform
		if (allFilesPresent) {
			if (!game.hasUnknownFiles) {
				debug(2, "Found game: %s (%s/%s) (%d)", game.desc->gameId, getPlatformDescription(game.desc->platform), getLanguageDescription(game.desc->language), g);
				// If we just added an unknown variant for this game and platform, remove it
				if (!matched.empty() && strcmp(matched.back().desc->gameId, game.desc->gameId) == 0 && matched.back().desc->platform == game.desc->platform)
					matched.pop_back();
				matched.push_back(game);
			} else {
				debug(5, "Skipping game: %s (%s/%s) (%d)", game.desc->gameId, getPlatformDescription(game.desc->platform), getLanguageDescription(game.desc->language), g);
				// If we already added a known or unknown variant for this game and platform, don't add another
				if (matched.empty() || strcmp(matched.back().desc->gameId, game.desc->gameId) != 0 || matched.back().desc->platform != game.desc->platform)
					matched.push_back(game);
			}
		}
	}

	return matched;
}

Engine *HiRes1Engine_create(OSystem *syst, const AdlGameDescription *gd);
Engine *HiRes2Engine_create(OSystem *syst, const AdlGameDescription *gd);
Engine *HiRes0Engine_create(OSystem *syst, const AdlGameDescription *gd);
Engine *HiRes3Engine_create(OSystem *syst, const AdlGameDescription *gd);
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
	case GAME_TYPE_HIRES3:
		*engine = HiRes3Engine_create(syst, adlGd);
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
