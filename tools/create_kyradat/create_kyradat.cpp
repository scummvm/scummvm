/* ScummVM Tools
 * Copyright (C) 2007 The ScummVM project
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

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include "create_kyradat.h"

#include "search.h"
#include "pak.h"

#include "tables.h"
#include "extract.h"

#include "md5.h"

#include <string>
#include <map>
#include <algorithm>
#include <map>

enum {
	kKyraDatVersion = 67
};

const ExtractFilename extractFilenames[] = {
	// INTRO / OUTRO sequences
	{ k1ForestSeq, kTypeRawData, false },
	{ k1KallakWritingSeq, kTypeRawData, false },
	{ k1KyrandiaLogoSeq, kTypeRawData, false },
	{ k1KallakMalcolmSeq, kTypeRawData, false },
	{ k1MalcolmTreeSeq, kTypeRawData, false },
	{ k1WestwoodLogoSeq, kTypeRawData, false },
	{ k1Demo1Seq, kTypeRawData, false },
	{ k1Demo2Seq, kTypeRawData, false },
	{ k1Demo3Seq, kTypeRawData, false },
	{ k1Demo4Seq, kTypeRawData, false },
	{ k1OutroReunionSeq, kTypeRawData, false },

	// INTRO / OUTRO strings
	{ k1IntroCPSStrings, kTypeStringList, false },
	{ k1IntroCOLStrings, kTypeStringList, false },
	{ k1IntroWSAStrings, kTypeStringList, false },
	{ k1IntroStrings, kTypeStringList, true },
	{ k1OutroHomeString, kTypeStringList, true },

	// INGAME strings
	{ k1ItemNames, kTypeStringList, true },
	{ k1TakenStrings, kTypeStringList, true },
	{ k1PlacedStrings, kTypeStringList, true },
	{ k1DroppedStrings, kTypeStringList, true },
	{ k1NoDropStrings, kTypeStringList, true },
	{ k1PutDownString, kTypeStringList, true },
	{ k1WaitAmuletString, kTypeStringList, true },
	{ k1BlackJewelString, kTypeStringList, true },
	{ k1PoisonGoneString, kTypeStringList, true },
	{ k1HealingTipString, kTypeStringList, true },
	{ k1ThePoisonStrings, kTypeStringList, true },
	{ k1FluteStrings, kTypeStringList, true },
	{ k1WispJewelStrings, kTypeStringList, true },
	{ k1MagicJewelStrings, kTypeStringList, true },
	{ k1FlaskFullString, kTypeStringList, true },
	{ k1FullFlaskString, kTypeStringList, true },
	{ k1VeryCleverString, kTypeStringList, true },
	{ k1NewGameString, kTypeStringList, true },

	// GUI strings table
	{ k1GUIStrings, kTypeStringList, true },
	{ k1ConfigStrings, kTypeStringList, true },

	// ROOM table/filenames
	{ k1RoomList, kTypeRoomList, false },
	{ k1RoomFilenames, kTypeStringList, false },

	// SHAPE tables
	{ k1DefaultShapes, kTypeShapeList, false },
	{ k1Healing1Shapes, kTypeShapeList, false },
	{ k1Healing2Shapes, kTypeShapeList, false },
	{ k1PoisonDeathShapes, kTypeShapeList, false },
	{ k1FluteShapes, kTypeShapeList, false },
	{ k1Winter1Shapes, kTypeShapeList, false },
	{ k1Winter2Shapes, kTypeShapeList, false },
	{ k1Winter3Shapes, kTypeShapeList, false },
	{ k1DrinkShapes, kTypeShapeList, false },
	{ k1WispShapes, kTypeShapeList, false },
	{ k1MagicAnimShapes, kTypeShapeList, false },
	{ k1BranStoneShapes, kTypeShapeList, false },

	// IMAGE filename table
	{ k1CharacterImageFilenames, kTypeStringList, false },

	// AUDIO filename table
	{ k1AudioTracks, kTypeStringList, false },
	{ k1AudioTracksIntro, kTypeStringList, false },

	// AMULET anim
	{ k1AmuleteAnimSeq, kTypeRawData, false },

	// PALETTE table
	{ k1SpecialPalette1, kTypeRawData, false },
	{ k1SpecialPalette2, kTypeRawData, false },
	{ k1SpecialPalette3, kTypeRawData, false },
	{ k1SpecialPalette4, kTypeRawData, false },
	{ k1SpecialPalette5, kTypeRawData, false },
	{ k1SpecialPalette6, kTypeRawData, false },
	{ k1SpecialPalette7, kTypeRawData, false },
	{ k1SpecialPalette8, kTypeRawData, false },
	{ k1SpecialPalette9, kTypeRawData, false },
	{ k1SpecialPalette10, kTypeRawData, false },
	{ k1SpecialPalette11, kTypeRawData, false },
	{ k1SpecialPalette12, kTypeRawData, false },
	{ k1SpecialPalette13, kTypeRawData, false },
	{ k1SpecialPalette14, kTypeRawData, false },
	{ k1SpecialPalette15, kTypeRawData, false },
	{ k1SpecialPalette16, kTypeRawData, false },
	{ k1SpecialPalette17, kTypeRawData, false },
	{ k1SpecialPalette18, kTypeRawData, false },
	{ k1SpecialPalette19, kTypeRawData, false },
	{ k1SpecialPalette20, kTypeRawData, false },
	{ k1SpecialPalette21, kTypeRawData, false },
	{ k1SpecialPalette22, kTypeRawData, false },
	{ k1SpecialPalette23, kTypeRawData, false },
	{ k1SpecialPalette24, kTypeRawData, false },
	{ k1SpecialPalette25, kTypeRawData, false },
	{ k1SpecialPalette26, kTypeRawData, false },
	{ k1SpecialPalette27, kTypeRawData, false },
	{ k1SpecialPalette28, kTypeRawData, false },
	{ k1SpecialPalette29, kTypeRawData, false },
	{ k1SpecialPalette30, kTypeRawData, false },
	{ k1SpecialPalette31, kTypeRawData, false },
	{ k1SpecialPalette32, kTypeRawData, false },
	{ k1SpecialPalette33, kTypeRawData, false },

	// CREDITS (used in FM-TOWNS and AMIGA)
	{ k1CreditsStrings, kTypeRawData, true },

	// FM-TOWNS specific
	{ k1TownsSFXwdTable, kTypeTownsWDSfxTable, false },
	{ k1TownsSFXbtTable, kTypeRawData, false },
	{ k1TownsCDATable, kTypeRawData, false },

	// PC98 specific
	{ k1PC98StoryStrings, kTypeStringList, true },
	{ k1PC98IntroSfx, kTypeRawData, false },

	// AMIGA specific
	{ k1AmigaIntroSFXTable, kTypeAmigaSfxTable, false },
	{ k1AmigaGameSFXTable, kTypeAmigaSfxTable, false },

	// HAND OF FATE

	// Sequence Player
	{ k2SeqplayPakFiles, kTypeStringList, false },
	{ k2SeqplayCredits, kTypeRawData, false },
	{ k2SeqplayCreditsSpecial, kTypeStringList, false },
	{ k2SeqplayStrings, kTypeStringList, true },
	{ k2SeqplaySfxFiles, k2TypeSoundList, false },
	{ k2SeqplayTlkFiles, k2TypeLangSoundList, true },
	{ k2SeqplaySeqData, k2TypeSeqData, false },
	{ k2SeqplayIntroTracks, kTypeStringList, false },
	{ k2SeqplayFinaleTracks, kTypeStringList, false },
	{ k2SeqplayIntroCDA, kTypeRawData, false },
	{ k2SeqplayFinaleCDA, kTypeRawData, false },
	{ k2SeqplayShapeAnimData, k2TypeShpDataV1, false },

	// Ingame
	{ k2IngamePakFiles, kTypeStringList, false },
	{ k2IngameSfxFiles, k2TypeSize10StringList, false },
	{ k2IngameSfxIndex, kTypeRawData, false },
	{ k2IngameTracks, kTypeStringList, false },
	{ k2IngameCDA, kTypeRawData, false },
	{ k2IngameTalkObjIndex, kTypeRawData, false },
	{ k2IngameTimJpStrings, kTypeStringList, false },
	{ k2IngameShapeAnimData, k2TypeShpDataV2, false },
	{ k2IngameTlkDemoStrings, kTypeStringList, true },


	// MALCOLM'S REVENGE
	{ k3MainMenuStrings, kTypeStringList, false },
	{ k3MusicFiles, k2TypeSoundList, false },
	{ k3ScoreTable, kTypeRawData, false },
	{ k3SfxFiles, k2TypeSfxList, false },
	{ k3SfxMap, k3TypeRaw16to8, false },
	{ k3ItemAnimData, k3TypeShpData, false },
	{ k3ItemMagicTable, k3TypeRaw16to8, false },
	{ k3ItemStringMap, kTypeRawData, false },

	// LANDS OF LORE

	// Ingame
	{ kLolIngamePakFiles, kTypeStringList, false },

	{ kLolCharacterDefs, kLolTypeCharData, false },
	{ kLolIngameSfxFiles, k2TypeSfxList, false },
	{ kLolIngameSfxIndex, kTypeRawData, false },
	{ kLolMusicTrackMap, kTypeRawData, false },
	{ kLolIngameGMSfxIndex, kTypeRawData, false },
	{ kLolIngameMT32SfxIndex, kTypeRawData, false },
	{ kLolIngamePcSpkSfxIndex, kTypeRawData, false },
	{ kLolSpellProperties, kLolTypeSpellData, false },
	{ kLolGameShapeMap, kTypeRawData, false },
	{ kLolSceneItemOffs, kTypeRawData, false },
	{ kLolCharInvIndex, k3TypeRaw16to8, false },
	{ kLolCharInvDefs, kTypeRawData, false },
	{ kLolCharDefsMan, kLolTypeRaw16, false },
	{ kLolCharDefsWoman, kLolTypeRaw16, false },
	{ kLolCharDefsKieran, kLolTypeRaw16, false },
	{ kLolCharDefsAkshel, kLolTypeRaw16, false },
	{ kLolExpRequirements, kLolTypeRaw32, false },
	{ kLolMonsterModifiers, kLolTypeRaw16, false },
	{ kLolMonsterShiftOffsets, kTypeRawData, false },
	{ kLolMonsterDirFlags, kTypeRawData, false },
	{ kLolMonsterScaleY, kTypeRawData, false },
	{ kLolMonsterScaleX, kTypeRawData, false },
	{ kLolMonsterScaleWH, kLolTypeRaw16, false },
	{ kLolFlyingObjectShp, kLolTypeFlightShpData, false },
	{ kLolInventoryDesc, kLolTypeRaw16, false },
	{ kLolLevelShpList, kTypeStringList, false },
	{ kLolLevelDatList, kTypeStringList, false },
	{ kLolCompassDefs, kLolTypeCompassData, false },
	{ kLolItemPrices, kLolTypeRaw16, false },
	{ kLolStashSetup, kTypeRawData, false },

	{ kLolDscUnk1, kTypeRawData, false },
	{ kLolDscShapeIndex, kTypeRawData, false },
	{ kLolDscOvlMap, kTypeRawData, false },
	{ kLolDscScaleWidthData, kLolTypeRaw16, false },
	{ kLolDscScaleHeightData, kLolTypeRaw16, false },
	{ kLolDscX, kLolTypeRaw16, false },
	{ kLolDscY, kTypeRawData, false },
	{ kLolDscTileIndex, kTypeRawData, false },
	{ kLolDscUnk2, kTypeRawData, false },
	{ kLolDscDoorShapeIndex, kTypeRawData, false },
	{ kLolDscDimData1, kTypeRawData, false },
	{ kLolDscDimData2, kTypeRawData, false },
	{ kLolDscBlockMap, kTypeRawData, false },
	{ kLolDscDimMap, kTypeRawData, false },
	{ kLolDscDoorScale, kLolTypeRaw16, false },
	{ kLolDscOvlIndex, k3TypeRaw16to8, false },
	{ kLolDscBlockIndex, kTypeRawData, false },
	{ kLolDscDoor4, kLolTypeRaw16, false },
	{ kLolDscDoor1, kTypeRawData, false },
	{ kLolDscDoorX, kLolTypeRaw16, false },
	{ kLolDscDoorY, kLolTypeRaw16, false },

	{ kLolScrollXTop, k3TypeRaw16to8, false },
	{ kLolScrollYTop, k3TypeRaw16to8, false },
	{ kLolScrollXBottom, k3TypeRaw16to8, false },
	{ kLolScrollYBottom, k3TypeRaw16to8, false },

	{ kLolButtonDefs, kLolTypeButtonDef, false },
	{ kLolButtonList1, kLolTypeRaw16, false },
	{ kLolButtonList2, kLolTypeRaw16, false },
	{ kLolButtonList3, kLolTypeRaw16, false },
	{ kLolButtonList4, kLolTypeRaw16, false },
	{ kLolButtonList5, kLolTypeRaw16, false },
	{ kLolButtonList6, kLolTypeRaw16, false },
	{ kLolButtonList7, kLolTypeRaw16, false },
	{ kLolButtonList8, kLolTypeRaw16, false },

	{ kLolLegendData, kTypeRawData, false },
	{ kLolMapCursorOvl, kTypeRawData, false },
	{ kLolMapStringId, kLolTypeRaw16, false },

	{ kLolSpellbookAnim, k3TypeRaw16to8, false },
	{ kLolSpellbookCoords, k3TypeRaw16to8, false },
	{ kLolHealShapeFrames, kTypeRawData, false },
	{ kLolLightningDefs, kTypeRawData, false },
	{ kLolFireballCoords, kLolTypeRaw16, false },

	{ kLolCredits, kTypeRawData, false },

	{ kLolHistory, kTypeRawData, false },

	{ -1, 0, 0 }
};

const ExtractFilename *getFilenameDesc(const int id) {
	for (const ExtractFilename *i = extractFilenames; i->id != -1; ++i) {
		if (i->id == id)
			return i;
	}
	return 0;
}

bool isLangSpecific(const int id) {
	const ExtractFilename *desc = getFilenameDesc(id);
	if (!desc)
		return false;
	return desc->langSpecific;
}

// misc tables

const TypeTable gameTable[] = {
	{ kKyra1, 0 },
	{ kKyra2, 1 },
	{ kKyra3, 2 },
	{ kLol, 3 },
	{ -1, -1 }
};

byte getGameID(int game) {
	return std::find(gameTable, gameTable + ARRAYSIZE(gameTable), game)->value;
}

const TypeTable languageTable[] = {
	{ EN_ANY, 0 },
	{ FR_FRA, 1 },
	{ DE_DEU, 2 },
	{ ES_ESP, 3 },
	{ IT_ITA, 4 },
	{ JA_JPN, 5 },
	{ -1, -1 }
};

byte getLanguageID(int lang) {
	return std::find(languageTable, languageTable + ARRAYSIZE(languageTable), lang)->value;
}

const TypeTable platformTable[] = {
	{ kPlatformPC, 0 },
	{ kPlatformAmiga, 1 },
	{ kPlatformFMTowns, 2 },
	{ kPlatformPC98, 3 },
	{ kPlatformMacintosh, 4 },
	{ -1, -1 }
};

byte getPlatformID(int platform) {
	return std::find(platformTable, platformTable + ARRAYSIZE(platformTable), platform)->value;
}

const TypeTable specialTable[] = {
	{ kNoSpecial, 0 },
	{ kTalkieVersion, 1 },
	{ kDemoVersion, 2 },
	{ kTalkieDemoVersion, 3 },
	{ -1, -1 }
};

byte getSpecialID(int special) {
	return std::find(specialTable, specialTable + ARRAYSIZE(specialTable), special)->value;
}

// filename processing

bool getFilename(char *dstFilename, const ExtractInformation *info, const int id) {
	const ExtractFilename *i = getFilenameDesc(id);

	if (!i)
		return false;

	// GAME, PLATFORM, SPECIAL, TYPE, ID[, LANG]
	if (i->langSpecific)
		sprintf(dstFilename, "%01X%01X%01X%02X%03X%01X", getGameID(info->game), getPlatformID(info->platform), getSpecialID(info->special), getTypeID(i->type), id, getLanguageID(info->lang));
	else
		sprintf(dstFilename, "%01X%01X%01X%02X%03X", getGameID(info->game), getPlatformID(info->platform), getSpecialID(info->special), getTypeID(i->type), id);
	return true;
}

// index generation

typedef uint16 GameDef;

GameDef createGameDef(const ExtractInformation *eI) {
	return ((getGameID(eI->game) & 0xF) << 12) |
	       ((getPlatformID(eI->platform) & 0xF) << 8) |
	       ((getSpecialID(eI->special) & 0xF) << 4);
}

struct Index {
	Index() : version(0), includedGames(0), gameList() {}

	uint32 version;
	uint32 includedGames;

	typedef std::list<GameDef> GameList;
	GameList gameList;
};

Index parseIndex(const uint8 *data, uint32 size) {
	Index result;

	if (size < 8)
		return result;

	result.version = READ_BE_UINT32(data); data += 4;
	result.includedGames = READ_BE_UINT32(data); data += 4;

	if (result.includedGames * 2 + 8 != size) {
		result.version = result.includedGames = 0;
		return result;
	}

	for (uint32 i = 0; i < result.includedGames; ++i) {
		GameDef game = READ_BE_UINT16(data); data += 2;
		result.gameList.push_back(game);
	}

	return result;
}

bool updateIndex(PAKFile &out, const ExtractInformation *eI) {
	uint32 size = 0;
	const uint8 *data = out.getFileData("INDEX", &size);

	Index index;
	if (data)
		index = parseIndex(data, size);

	GameDef gameDef = createGameDef(eI);
	if (index.version == kKyraDatVersion) {
		if (std::find(index.gameList.begin(), index.gameList.end(), gameDef) == index.gameList.end()) {
			++index.includedGames;
			index.gameList.push_back(gameDef);
		}
	} else {
		index.version = kKyraDatVersion;
		index.includedGames = 1;
		index.gameList.push_back(gameDef);
	}

	const uint32 indexBufferSize = 8 + index.includedGames * 2;
	uint8 *indexBuffer = new uint8[indexBufferSize];
	assert(indexBuffer);
	uint8 *dst = indexBuffer;
	WRITE_BE_UINT32(dst, index.version); dst += 4;
	WRITE_BE_UINT32(dst, index.includedGames); dst += 4;
	for (Index::GameList::const_iterator i = index.gameList.begin(); i != index.gameList.end(); ++i) {
		WRITE_BE_UINT16(dst, *i); dst += 2;
	}

	out.removeFile("INDEX");
	if (!out.addFile("INDEX", indexBuffer, indexBufferSize)) {
		fprintf(stderr, "ERROR: couldn't update kyra.dat INDEX\n");
		delete[] indexBuffer;
		return false;
	}

	return true;
}

bool checkIndex(PAKFile &file) {
	uint32 size = 0;
	const uint8 *data = file.getFileData("INDEX", &size);
	if (!data)
		return false;

	Index index = parseIndex(data, size);

	if (index.version != kKyraDatVersion)
		return false;
	if (index.includedGames * 2 + 8 != size)
		return false;

	return true;
}

// main processing

void printHelp(const char *f) {
	printf("Usage:\n");
	printf("%s output inputfiles ...", f);
}

bool process(PAKFile &out, const Game *g, const byte *data, const uint32 size);
const Game *findGame(const byte *buffer, const uint32 size);

typedef std::map<std::string, std::string> MD5Map;
MD5Map createMD5Sums(int files, const char * const *filenames);

struct File {
	File() : data(0), size(0) {}
	File(uint8 *d, uint32 s) : data(d), size(s) {}

	uint8 *data;
	uint32 size;
};
typedef std::map<const Game *, File> GameMap;
GameMap createGameMap(const MD5Map &map);

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printHelp(argv[0]);
		return -1;
	}

	// Special case for developer mode of this tool:
	// With "--create filename offset size" the tool will output
	// a search entry for the specifed data in the specified file.
	if (!strcmp(argv[1], "--create")) {
		if (argc < 5) {
			printf("Developer usage: %s --create input_file hex_offset hex_size\n", argv[0]);
			return -1;
		}

		uint32 offset, size;
		sscanf(argv[3], "%x", &offset);
		sscanf(argv[4], "%x", &size);

		FILE *input = fopen(argv[2], "rb");
		if (!input)
			error("Couldn't open file '%s'", argv[2]);

		byte *buffer = new byte[size];
		fseek(input, offset, SEEK_SET);
		if (fread(buffer, 1, size, input) != size) {
			delete[] buffer;
			error("Couldn't read from file '%s'", argv[2]);
		}

		fclose(input);

		SearchData d = SearchCreator::create(buffer, size);
		delete[] buffer;

		printf("{ 0x%.08X, 0x%.08X, { {", d.size, d.byteSum);
		for (int j = 0; j < 16; ++j) {
			printf(" 0x%.2X", d.hash.digest[j]);
			if (j != 15)
				printf(",");
			else
				printf(" } } }\n");
		}

		return 0;
	}

	PAKFile out;
	out.loadFile(argv[1], false);

	// When the output file is no valid kyra.dat file, we will delete
	// all the output.
	if (!checkIndex(out))
		out.clearFile();

	MD5Map inputFiles = createMD5Sums(argc - 2, &argv[2]);

	GameMap games = createGameMap(inputFiles);

	// Check for unused input files
	MD5Map unusedFiles = inputFiles;
	for (GameMap::const_iterator i = games.begin(); i != games.end(); ++i) {
		unusedFiles.erase(i->first->md5[0]);
		if (i->first->md5[1])
			unusedFiles.erase(i->first->md5[1]);
	}

	for (MD5Map::const_iterator i = unusedFiles.begin(); i != unusedFiles.end(); ++i)
		printf("Input file '%s' with md5 sum '%s' is not known.\n", i->second.c_str(), i->first.c_str());

	unusedFiles.clear();

	// Short circuit, in case no games are found.
	if (games.empty()) {
		printf("No games found. Exiting prematurely\n");
		return -1;
	}

	// Process all games found
	for (GameMap::const_iterator i = games.begin(); i != games.end(); ++i) {
		MD5Map::const_iterator f1 = inputFiles.find(i->first->md5[0]);
		MD5Map::const_iterator f2 = inputFiles.end();
		if (i->first->md5[1])
			f2 = inputFiles.find(i->first->md5[1]);

		if (f2 != inputFiles.end())
			printf("Processing files '%s' and '%s'...\n", f1->second.c_str(), f2->second.c_str());
		else
			printf("Processing file '%s'...\n", f1->second.c_str());

		if (!process(out, i->first, i->second.data, i->second.size))
			printf("FAILED\n");
		else
			printf("OK\n");
	}

	// Free up memory
	for (GameMap::iterator i = games.begin(); i != games.end(); ++i)
		delete[] i->second.data;
	games.clear();
	inputFiles.clear();

	if (!out.saveFile(argv[1]))
	error("couldn't save changes to '%s'", argv[1]);

	uint8 digest[16];
	if (!md5_file(argv[1], digest, 0))
		error("couldn't calc. md5 for file '%s'", argv[1]);
	FILE *f = fopen(argv[1], "ab");
	if (!f)
		error("couldn't open file '%s'", argv[1]);
	if (fwrite(digest, 1, 16, f) != 16)
		error("couldn't write md5sum to file '%s'", argv[1]);
	fclose(f);

	return 0;
}

MD5Map createMD5Sums(int files, const char * const *filenames) {
	MD5Map result;

	while (files--) {
		const char *inputFile = *filenames++;
		FILE *input = fopen(inputFile, "rb");

		uint32 size = fileSize(input);
		fseek(input, 0, SEEK_SET);

		byte *buffer = new uint8[size];
		assert(buffer);

		if (fread(buffer, 1, size, input) != size) {
			warning("couldn't read from file '%s', skipping it", inputFile);
			delete[] buffer;
			fclose(input);
			continue;
		}
		fclose(input);

		md5_context ctx;
		uint8 digest[16];
		char md5Str[33];

		md5_starts(&ctx);
		md5_update(&ctx, buffer, size);
		md5_finish(&ctx, digest);

		for (int j = 0; j < 16; ++j)
			sprintf(md5Str + j*2, "%02x", (int)digest[j]);

		delete[] buffer;

		result[md5Str] = inputFile;
	}

	return result;
}

GameMap createGameMap(const MD5Map &map) {
	GameMap result;

	for (const Game * const *g = gameDescs; *g != 0; ++g) {
		for (const Game *sub = *g; sub->game != -1; ++sub) {
			MD5Map::const_iterator file1 = map.find(sub->md5[0]);
			if (file1 == map.end())
				continue;

			MD5Map::const_iterator file2 = map.end();
			if (sub->md5[1] != 0) {
				file2 = map.find(sub->md5[1]);
				if (file2 == map.end())
					continue;
			}

			FILE *f1 = fopen(file1->second.c_str(), "rb");
			FILE *f2 = 0;

			if (file2 != map.end())
				f2 = fopen(file2->second.c_str(), "rb");

			uint32 file1Size = fileSize(f1);
			uint32 file2Size = 0;
			if (f2)
				file2Size = fileSize(f2);

			uint8 *buffer = new uint8[file1Size + file2Size];
			assert(buffer);

			fread(buffer, 1, file1Size, f1);
			if (f2)
				fread(buffer + file1Size, 1, file2Size, f2);

			fclose(f1);
			if (f2)
				fclose(f2);

			result[sub] = File(buffer, file1Size + file2Size);
		}
	}

	return result;
}

const char *getIdString(const int id) {
	switch (id) {
	case k1ForestSeq:
		return "k1ForestSeq";
	case k1KallakWritingSeq:
		return "k1KallakWritingSeq";
	case k1KyrandiaLogoSeq:
		return "k1KyrandiaLogoSeq";
	case k1KallakMalcolmSeq:
		return "k1KallakMalcolmSeq";
	case k1MalcolmTreeSeq:
		return "k1MalcolmTreeSeq";
	case k1WestwoodLogoSeq:
		return "k1WestwoodLogoSeq";
	case k1Demo1Seq:
		return "k1Demo1Seq";
	case k1Demo2Seq:
		return "k1Demo2Seq";
	case k1Demo3Seq:
		return "k1Demo3Seq";
	case k1Demo4Seq:
		return "k1Demo4Seq";
	case k1AmuleteAnimSeq:
		return "k1AmuleteAnimSeq";
	case k1OutroReunionSeq:
		return "k1OutroReunionSeq";
	case k1IntroCPSStrings:
		return "k1IntroCPSStrings";
	case k1IntroCOLStrings:
		return "k1IntroCOLStrings";
	case k1IntroWSAStrings:
		return "k1IntroWSAStrings";
	case k1IntroStrings:
		return "k1IntroStrings";
	case k1OutroHomeString:
		return "k1OutroHomeString";
	case k1RoomFilenames:
		return "k1RoomFilenames";
	case k1RoomList:
		return "k1RoomList";
	case k1CharacterImageFilenames:
		return "k1CharacterImageFilenames";
	case k1AudioTracks:
		return "k1AudioTracks";
	case k1AudioTracksIntro:
		return "k1AudioTracksIntro";
	case k1ItemNames:
		return "k1ItemNames";
	case k1TakenStrings:
		return "k1TakenStrings";
	case k1PlacedStrings:
		return "k1PlacedStrings";
	case k1DroppedStrings:
		return "k1DroppedStrings";
	case k1NoDropStrings:
		return "k1NoDropStrings";
	case k1PutDownString:
		return "k1PutDownString";
	case k1WaitAmuletString:
		return "k1WaitAmuletString";
	case k1BlackJewelString:
		return "k1BlackJewelString";
	case k1PoisonGoneString:
		return "k1PoisonGoneString";
	case k1HealingTipString:
		return "k1HealingTipString";
	case k1WispJewelStrings:
		return "k1WispJewelStrings";
	case k1MagicJewelStrings:
		return "k1MagicJewelStrings";
	case k1ThePoisonStrings:
		return "k1ThePoisonStrings";
	case k1FluteStrings:
		return "k1FluteStrings";
	case k1FlaskFullString:
		return "k1FlaskFullString";
	case k1FullFlaskString:
		return "k1FullFlaskString";
	case k1VeryCleverString:
		return "k1VeryCleverString";
	case k1NewGameString:
		return "k1NewGameString";
	case k1DefaultShapes:
		return "k1DefaultShapes";
	case k1Healing1Shapes:
		return "k1Healing1Shapes";
	case k1Healing2Shapes:
		return "k1Healing2Shapes";
	case k1PoisonDeathShapes:
		return "k1PoisonDeathShapes";
	case k1FluteShapes:
		return "k1FluteShapes";
	case k1Winter1Shapes:
		return "k1Winter1Shapes";
	case k1Winter2Shapes:
		return "k1Winter2Shapes";
	case k1Winter3Shapes:
		return "k1Winter3Shapes";
	case k1DrinkShapes:
		return "k1DrinkShapes";
	case k1WispShapes:
		return "k1WispShapes";
	case k1MagicAnimShapes:
		return "k1MagicAnimShapes";
	case k1BranStoneShapes:
		return "k1BranStoneShapes";
	case k1SpecialPalette1:
		return "k1SpecialPalette1";
	case k1SpecialPalette2:
		return "k1SpecialPalette2";
	case k1SpecialPalette3:
		return "k1SpecialPalette3";
	case k1SpecialPalette4:
		return "k1SpecialPalette4";
	case k1SpecialPalette5:
		return "k1SpecialPalette5";
	case k1SpecialPalette6:
		return "k1SpecialPalette6";
	case k1SpecialPalette7:
		return "k1SpecialPalette7";
	case k1SpecialPalette8:
		return "k1SpecialPalette8";
	case k1SpecialPalette9:
		return "k1SpecialPalette9";
	case k1SpecialPalette10:
		return "k1SpecialPalette10";
	case k1SpecialPalette11:
		return "k1SpecialPalette11";
	case k1SpecialPalette12:
		return "k1SpecialPalette12";
	case k1SpecialPalette13:
		return "k1SpecialPalette13";
	case k1SpecialPalette14:
		return "k1SpecialPalette14";
	case k1SpecialPalette15:
		return "k1SpecialPalette15";
	case k1SpecialPalette16:
		return "k1SpecialPalette16";
	case k1SpecialPalette17:
		return "k1SpecialPalette17";
	case k1SpecialPalette18:
		return "k1SpecialPalette18";
	case k1SpecialPalette19:
		return "k1SpecialPalette19";
	case k1SpecialPalette20:
		return "k1SpecialPalette20";
	case k1SpecialPalette21:
		return "k1SpecialPalette21";
	case k1SpecialPalette22:
		return "k1SpecialPalette22";
	case k1SpecialPalette23:
		return "k1SpecialPalette23";
	case k1SpecialPalette24:
		return "k1SpecialPalette24";
	case k1SpecialPalette25:
		return "k1SpecialPalette25";
	case k1SpecialPalette26:
		return "k1SpecialPalette26";
	case k1SpecialPalette27:
		return "k1SpecialPalette27";
	case k1SpecialPalette28:
		return "k1SpecialPalette28";
	case k1SpecialPalette29:
		return "k1SpecialPalette29";
	case k1SpecialPalette30:
		return "k1SpecialPalette30";
	case k1SpecialPalette31:
		return "k1SpecialPalette31";
	case k1SpecialPalette32:
		return "k1SpecialPalette32";
	case k1SpecialPalette33:
		return "k1SpecialPalette33";
	case k1GUIStrings:
		return "k1GUIStrings";
	case k1ConfigStrings:
		return "k1ConfigStrings";
	case k1TownsSFXwdTable:
		return "k1TownsSFXwdTable";
	case k1TownsSFXbtTable:
		return "k1TownsSFXbtTable";
	case k1TownsCDATable:
		return "k1TownsCDATable";
	case k1PC98StoryStrings:
		return "k1PC98StoryStrings";
	case k1PC98IntroSfx:
		return "k1PC98IntroSfx";
	case k1CreditsStrings:
		return "k1CreditsStrings";
	case k1AmigaIntroSFXTable:
		return "k1AmigaIntroSFXTable";
	case k1AmigaGameSFXTable:
		return "k1AmigaGameSFXTable";
	case k2SeqplayPakFiles:
		return "k2SeqplayPakFiles";
	case k2SeqplayStrings:
		return "k2SeqplayStrings";
	case k2SeqplaySfxFiles:
		return "k2SeqplaySfxFiles";
	case k2SeqplayTlkFiles:
		return "k2SeqplayTlkFiles";
	case k2SeqplaySeqData:
		return "k2SeqplaySeqData";
	case k2SeqplayCredits:
		return "k2SeqplayCredits";
	case k2SeqplayCreditsSpecial:
		return "k2SeqplayCreditsSpecial";
	case k2SeqplayIntroTracks:
		return "k2SeqplayIntroTracks";
	case k2SeqplayFinaleTracks:
		return "k2SeqplayFinaleTracks";
	case k2SeqplayIntroCDA:
		return "k2SeqplayIntroCDA";
	case k2SeqplayFinaleCDA:
		return "k2SeqplayFinaleCDA";
	case k2SeqplayShapeAnimData:
		return "k2SeqplayShapeAnimData";
	case k2IngamePakFiles:
		return "k2IngamePakFiles";
	case k2IngameSfxFiles:
		return "k2IngameSfxFiles";
	case k2IngameSfxIndex:
		return "k2IngameSfxIndex";
	case k2IngameTracks:
		return "k2IngameTracks";
	case k2IngameCDA:
		return "k2IngameCDA";
	case k2IngameTalkObjIndex:
		return "k2IngameTalkObjIndex";
	case k2IngameTimJpStrings:
		return "k2IngameTimJpStrings";
	case k2IngameShapeAnimData:
		return "k2IngameShapeAnimData";
	case k2IngameTlkDemoStrings:
		return "k2IngameTlkDemoStrings";
	case k3MainMenuStrings:
		return "k3MainMenuStrings";
	case k3MusicFiles:
		return "k3MusicFiles";
	case k3ScoreTable:
		return "k3ScoreTable";
	case k3SfxFiles:
		return "k3SfxFiles";
	case k3SfxMap:
		return "k3SfxMap";
	case k3ItemAnimData:
		return "k3ItemAnimData";
	case k3ItemMagicTable:
		return "k3ItemMagicTable";
	case k3ItemStringMap:
		return "k3ItemStringMap";
	case kLolIngamePakFiles:
		return "kLolIngamePakFiles";
	case kLolCharacterDefs:
		return "kLolCharacterDefs";
	case kLolIngameSfxFiles:
		return "kLolIngameSfxFiles";
	case kLolIngameSfxIndex:
		return "kLolIngameSfxIndex";
	case kLolMusicTrackMap:
		return "kLolMusicTrackMap";
	case kLolIngameGMSfxIndex:
		return "kLolIngameGMSfxIndex";
	case kLolIngameMT32SfxIndex:
		return "kLolIngameMT32SfxIndex";
	case kLolIngamePcSpkSfxIndex:
		return "kLolIngamePcSpkSfxIndex";
	case kLolSpellProperties:
		return "kLolSpellProperties";
	case kLolGameShapeMap:
		return "kLolGameShapeMap";
	case kLolSceneItemOffs:
		return "kLolSceneItemOffs";
	case kLolCharInvIndex:
		return "kLolCharInvIndex";
	case kLolCharInvDefs:
		return "kLolCharInvDefs";
	case kLolCharDefsMan:
		return "kLolCharDefsMan";
	case kLolCharDefsWoman:
		return "kLolCharDefsWoman";
	case kLolCharDefsKieran:
		return "kLolCharDefsKieran";
	case kLolCharDefsAkshel:
		return "kLolCharDefsAkshel";
	case kLolExpRequirements:
		return "kLolExpRequirements";
	case kLolMonsterModifiers:
		return "kLolMonsterModifiers";
	case kLolMonsterShiftOffsets:
		return "kLolMonsterShiftOffsets";
	case kLolMonsterDirFlags:
		return "kLolMonsterDirFlags";
	case kLolMonsterScaleY:
		return "kLolMonsterScaleY";
	case kLolMonsterScaleX:
		return "kLolMonsterScaleX";
	case kLolMonsterScaleWH:
		return "kLolMonsterScaleWH";
	case kLolFlyingObjectShp:
		return "kLolFlyingObjectShp";
	case kLolInventoryDesc:
		return "kLolInventoryDesc";
	case kLolLevelShpList:
		return "kLolLevelShpList";
	case kLolLevelDatList:
		return "kLolLevelDatList";
	case kLolCompassDefs:
		return "kLolCompassDefs";
	case kLolItemPrices:
		return "kLolItemPrices";
	case kLolStashSetup:
		return "kLolStashSetup";
	case kLolDscUnk1:
		return "kLolDscUnk1";
	case kLolDscShapeIndex:
		return "kLolDscShapeIndex";
	case kLolDscOvlMap:
		return "kLolDscOvlMap";
	case kLolDscScaleWidthData:
		return "kLolDscScaleWidthData";
	case kLolDscScaleHeightData:
		return "kLolDscScaleHeightData";
	case kLolDscX:
		return "kLolDscX";
	case kLolDscY:
		return "kLolDscY";
	case kLolDscTileIndex:
		return "kLolDscTileIndex";
	case kLolDscUnk2:
		return "kLolDscUnk2";
	case kLolDscDoorShapeIndex:
		return "kLolDscDoorShapeIndex";
	case kLolDscDimData1:
		return "kLolDscDimData1";
	case kLolDscDimData2:
		return "kLolDscDimData2";
	case kLolDscBlockMap:
		return "kLolDscBlockMap";
	case kLolDscDimMap:
		return "kLolDscDimMap";
	case kLolDscOvlIndex:
		return "kLolDscOvlIndex";
	case kLolDscBlockIndex:
		return "kLolDscBlockIndex";
	case kLolDscDoor1:
		return "kLolDscDoor1";
	case kLolDscDoorScale:
		return "kLolDscDoorScale";
	case kLolDscDoor4:
		return "kLolDscDoor4";
	case kLolDscDoorX:
		return "kLolDscDoorX";
	case kLolDscDoorY:
		return "kLolDscDoorY";
	case kLolScrollXTop:
		return "kLolScrollXTop";
	case kLolScrollYTop:
		return "kLolScrollYTop";
	case kLolScrollXBottom:
		return "kLolScrollXBottom";
	case kLolScrollYBottom:
		return "kLolScrollYBottom";
	case kLolButtonDefs:
		return "kLolButtonDefs";
	case kLolButtonList1:
		return "kLolButtonList1";
	case kLolButtonList2:
		return "kLolButtonList2";
	case kLolButtonList3:
		return "kLolButtonList3";
	case kLolButtonList4:
		return "kLolButtonList4";
	case kLolButtonList5:
		return "kLolButtonList5";
	case kLolButtonList6:
		return "kLolButtonList6";
	case kLolButtonList7:
		return "kLolButtonList7";
	case kLolButtonList8:
		return "kLolButtonList8";
	case kLolLegendData:
		return "kLolLegendData";
	case kLolMapCursorOvl:
		return "kLolMapCursorOvl";
	case kLolMapStringId:
		return "kLolMapStringId";
	case kLolSpellbookAnim:
		return "kLolSpellbookAnim";
	case kLolSpellbookCoords:
		return "kLolSpellbookCoords";
	case kLolHealShapeFrames:
		return "kLolHealShapeFrames";
	case kLolLightningDefs:
		return "kLolLightningDefs";
	case kLolFireballCoords:
		return "kLolFireballCoords";
	case kLolHistory:
		return "kLolHistory";
	default:
		return "Unknown";
	}
}

struct ExtractData {
	ExtractData() : desc(), offset() {}
	ExtractData(ExtractEntrySearchData d, uint32 o) : desc(d), offset(o) {}

	ExtractEntrySearchData desc;
	uint32 offset;
};

typedef std::pair<int, ExtractEntrySearchData> SearchMapEntry;
typedef std::multimap<int, ExtractEntrySearchData> SearchMap;

typedef std::pair<int, ExtractData> ExtractMapEntry;
typedef std::multimap<int, ExtractData> ExtractMap;

bool getExtractionData(const Game *g, Search &search, ExtractMap &map);

bool process(PAKFile &out, const Game *g, const byte *data, const uint32 size) {
	char filename[128];

	Search search(data, size);
	ExtractMap ids;

	if (!getExtractionData(g, search, ids))
		return false;

	const int *needList = getNeedList(g);
	if (!needList) {
		fprintf(stderr, "ERROR: No entry need list available\n");
		return false;
	}

	ExtractInformation extractInfo;
	extractInfo.game = g->game;
	extractInfo.platform = g->platform;
	extractInfo.special = g->special;

	for (ExtractMap::const_iterator i = ids.begin(); i != ids.end(); ++i) {
		const int id = i->first;

		const ExtractFilename *fDesc = getFilenameDesc(id);

		if (!fDesc) {
			fprintf(stderr, "ERROR: couldn't find file description for id %d/%s\n", id, getIdString(id));
			return false;
		}

		if (fDesc->langSpecific)
			extractInfo.lang = i->second.desc.lang;
		else
			extractInfo.lang = UNK_LANG;

		filename[0] = 0;
		if (!getFilename(filename, &extractInfo, id)) {
			fprintf(stderr, "ERROR: couldn't get filename for id %d/%s\n", id, getIdString(id));
			return false;
		}

		const ExtractType *tDesc = findExtractType(fDesc->type);

		if (!tDesc) {
			fprintf(stderr, "ERROR: couldn't find type description for id %d/%s (%d)\n", id, getIdString(id), fDesc->type);
			return false;
		}

		PAKFile::cFileList *list = out.getFileList();
		if (list && list->findEntry(filename) != 0)
			continue;

		if (!tDesc->extract(out, &extractInfo, data + i->second.offset, i->second.desc.hint.size, filename, id)) {
			fprintf(stderr, "ERROR: couldn't extract id %d/%s\n", id, getIdString(id));
			return false;
		}
	}

	for (int i = 0; i < 3; ++i) {
		if (g->lang[i] == -1)
			continue;

		extractInfo.lang = g->lang[i];
		if (!updateIndex(out, &extractInfo)) {
			error("couldn't update INDEX file, stop processing of all files");
			return false;
		}
	}

	return true;
}

// Uncomment this to get various debug information about the detection table entries.
//#define DEBUG_EXTRACTION_TABLES

bool setupSearch(const Game *g, const int *needList, Search &search, SearchMap &searchData) {
	for (const int *entry = needList; *entry != -1; ++entry) {
		ExtractEntryList providers = getProvidersForId(*entry);

		if (providers.empty()) {
			fprintf(stderr, "ERROR: No provider for id %d/%s\n", *entry, getIdString(*entry));
			return false;
		} else {
			for (ExtractEntryList::const_iterator i = providers.begin(); i != providers.end(); ++i) {
				// Only add generic or partly matching providers here.
#ifndef DEBUG_EXTRACTION_TABLES
				if ((i->lang == UNK_LANG || i->lang == g->lang[0] || i->lang == g->lang[1] || i->lang == g->lang[2]) &&
				    (i->platform == kPlatformUnknown || (i->platform == g->platform))) {
#endif
					search.addData(i->hint);
					searchData.insert(SearchMapEntry(*entry, *i));
#ifndef DEBUG_EXTRACTION_TABLES
				}
#endif
			}
		}
	}

	return true;
}

typedef std::list<ExtractMap::const_iterator> MatchList;
MatchList filterPlatformMatches(const Game *g, std::pair<ExtractMap::const_iterator, ExtractMap::const_iterator> range) {
	bool hasPlatformMatch = false;
	for (ExtractMap::const_iterator i = range.first; i != range.second; ++i) {
		if (i->second.desc.platform == g->platform) {
			hasPlatformMatch = true;
			break;
		}
	}

	MatchList result;
	if (hasPlatformMatch) {
		for (ExtractMap::const_iterator i = range.first; i != range.second; ++i) {
			if (i->second.desc.platform == g->platform)
				result.push_back(i);
		}
	} else {
		for (ExtractMap::const_iterator i = range.first; i != range.second; ++i)
			result.push_back(i);
	}

	return result;
}

MatchList filterLanguageMatches(const int lang, const MatchList &input) {
	std::list<ExtractMap::const_iterator> result;

	for (MatchList::const_iterator i = input.begin(); i != input.end(); ++i) {
		if ((*i)->second.desc.lang == lang)
			result.push_back(*i);
	}

	return result;
}

MatchList::const_iterator filterOutBestMatch(const MatchList &input) {
	MatchList::const_iterator result = input.begin();

	if (input.size() > 1)
		warning("Multiple entries found for id %d/%s", (*result)->first, getIdString((*result)->first));

	for (MatchList::const_iterator i = input.begin(); i != input.end(); ++i) {
		// Reduce all entries to one single entry.
		//
		// We use the following rules for this (in this order):
		// - Prefer the entry with the higest size
		// - Prefer the entry, which starts at the smallest offest
		//
		// TODO: These rules might not be safe for all games, but hopefully
		// they will work fine. If there are any problems it should be rather
		// easy to identify them, since we print out a warning for multiple
		// entries found.
		if ((*result)->second.desc.hint.size <= (*i)->second.desc.hint.size) {
			if ((*result)->second.offset >= (*i)->second.offset)
				result = i;
		}
	}

	return result;
}

bool getExtractionData(const Game *g, Search &search, ExtractMap &map) {
	SearchMap searchMap;

	const int *needList = getNeedList(g);
	if (!needList) {
		fprintf(stderr, "ERROR: No entry need list available\n");
		return false;
	}

	if (!setupSearch(g, needList, search, searchMap))
		return false;

	// Process the data search
	Search::ResultList results;
	search.search(results);

	if (results.empty()) {
		fprintf(stderr, "ERROR: Couldn't find any required data\n");
		return false;
	}

	ExtractMap temporaryExtractMap;
	for (const int *entry = needList; *entry != -1; ++entry) {
		typedef std::pair<SearchMap::const_iterator, SearchMap::const_iterator> KeyRange;
		KeyRange idRange = searchMap.equal_range(*entry);

		for (Search::ResultList::const_iterator i = results.begin(); i != results.end(); ++i) {
			for (SearchMap::const_iterator j = idRange.first; j != idRange.second; ++j) {
				if (j->second.hint == i->data)
					temporaryExtractMap.insert(ExtractMapEntry(*entry, ExtractData(j->second, i->offset)));
			}
		}
	}

	// Free up some memory
	results.clear();
	searchMap.clear();

	bool result = true;

	for (const int *entry = needList; *entry != -1; ++entry) {
		MatchList possibleMatches = filterPlatformMatches(g, temporaryExtractMap.equal_range(*entry));

		if (possibleMatches.empty()) {
			fprintf(stderr, "ERROR: No entry found for id %d/%s\n", *entry, getIdString(*entry));
			result = false;
			continue;
		}

		if (isLangSpecific(*entry)) {
			for (int i = 0; i < 3; ++i) {
				if (g->lang[i] == -1)
					continue;

				MatchList langMatches = filterLanguageMatches(g->lang[i], possibleMatches);
				MatchList::const_iterator bestMatch = filterOutBestMatch(langMatches);

				if (bestMatch == langMatches.end()) {
					// TODO: Add nice language name to output message.
					fprintf(stderr, "ERROR: No entry found for id %d/%s for language %d\n", *entry, getIdString(*entry), g->lang[i]);
					result = false;
					continue;
				}

#ifdef DEBUG_EXTRACTION_TABLES
				if (((*bestMatch)->second.desc.platform != kPlatformUnknown && (*bestMatch)->second.desc.platform != g->platform))
					printf("%s: %.8X %.8X %d %d\n", getIdString(*entry), (*bestMatch)->second.desc.hint.size, (*bestMatch)->second.desc.hint.byteSum, (*bestMatch)->second.desc.lang, (*bestMatch)->second.desc.platform);
#endif

				map.insert(**bestMatch);
			}
		} else {
			MatchList::const_iterator bestMatch = filterOutBestMatch(possibleMatches);

			if (bestMatch == possibleMatches.end()) {
				fprintf(stderr, "ERROR: No entry found for id %d/%s\n", *entry, getIdString(*entry));
				result = false;
				continue;
			}

#ifdef DEBUG_EXTRACTION_TABLES
			if (((*bestMatch)->second.desc.platform != kPlatformUnknown && (*bestMatch)->second.desc.platform != g->platform))
				printf("%s: %.8X %.8X %d %d\n", getIdString(*entry), (*bestMatch)->second.desc.hint.size, (*bestMatch)->second.desc.hint.byteSum, (*bestMatch)->second.desc.lang, (*bestMatch)->second.desc.platform);
#endif

			map.insert(**bestMatch);
		}
	}

	return result;
}

