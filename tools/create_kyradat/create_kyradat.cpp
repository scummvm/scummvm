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

enum {
	kKyraDatVersion = 65
};

const ExtractFilename extractFilenames[] = {
	// INTRO / OUTRO sequences
	{ kForestSeq, kTypeRawData, "FOREST.SEQ" },
	{ kKallakWritingSeq, kTypeRawData, "KALLAK-WRITING.SEQ" },
	{ kKyrandiaLogoSeq, kTypeRawData, "KYRANDIA-LOGO.SEQ" },
	{ kKallakMalcolmSeq, kTypeRawData, "KALLAK-MALCOLM.SEQ" },
	{ kMalcolmTreeSeq, kTypeRawData, "MALCOLM-TREE.SEQ" },
	{ kWestwoodLogoSeq, kTypeRawData, "WESTWOOD-LOGO.SEQ" },
	{ kDemo1Seq, kTypeRawData, "DEMO1.SEQ" },
	{ kDemo2Seq, kTypeRawData, "DEMO2.SEQ" },
	{ kDemo3Seq, kTypeRawData, "DEMO3.SEQ" },
	{ kDemo4Seq, kTypeRawData, "DEMO4.SEQ" },
	{ kOutroReunionSeq, kTypeRawData, "REUNION.SEQ" },

	// INTRO / OUTRO strings
	{ kIntroCPSStrings, kTypeStringList, "INTRO-CPS.TXT" },
	{ kIntroCOLStrings, kTypeStringList, "INTRO-COL.TXT" },
	{ kIntroWSAStrings, kTypeStringList, "INTRO-WSA.TXT" },
	{ kIntroStrings, kTypeLanguageList, "INTRO-STRINGS" },
	{ kOutroHomeString, kTypeLanguageList, "HOME" },

	// INGAME strings
	{ kItemNames, kTypeLanguageList, "ITEMLIST" },
	{ kTakenStrings, kTypeLanguageList, "TAKEN" },
	{ kPlacedStrings, kTypeLanguageList, "PLACED" },
	{ kDroppedStrings, kTypeLanguageList, "DROPPED" },
	{ kNoDropStrings, kTypeLanguageList, "NODROP" },
	{ kPutDownString, kTypeLanguageList, "PUTDOWN" },
	{ kWaitAmuletString, kTypeLanguageList, "WAITAMUL" },
	{ kBlackJewelString, kTypeLanguageList, "BLACKJEWEL" },
	{ kPoisonGoneString, kTypeLanguageList, "POISONGONE" },
	{ kHealingTipString, kTypeLanguageList, "HEALINGTIP" },
	{ kThePoisonStrings, kTypeLanguageList, "THEPOISON" },
	{ kFluteStrings, kTypeLanguageList, "FLUTE" },
	{ kWispJewelStrings, kTypeLanguageList, "WISPJEWEL" },
	{ kMagicJewelStrings, kTypeLanguageList, "MAGICJEWEL" },
	{ kFlaskFullString, kTypeLanguageList, "FLASKFULL" },
	{ kFullFlaskString, kTypeLanguageList, "FULLFLASK" },
	{ kVeryCleverString, kTypeLanguageList, "VERYCLEVER" },
	{ kNewGameString, kTypeLanguageList, "NEWGAME" },

	// GUI strings table
	{ kGUIStrings, kTypeLanguageList, "GUISTRINGS" },
	{ kConfigStrings, kTypeLanguageList, "CONFIGSTRINGS" },

	// ROOM table/filenames
	{ kRoomList, kTypeRoomList, "ROOM-TABLE.ROOM" },
	{ kRoomFilenames, kTypeStringList, "ROOM-FILENAMES.TXT" },

	// SHAPE tables
	{ kDefaultShapes, kTypeShapeList, "SHAPES-DEFAULT.SHP" },
	{ kHealing1Shapes, kTypeShapeList, "HEALING.SHP" },
	{ kHealing2Shapes, kTypeShapeList, "HEALING2.SHP" },
	{ kPoisonDeathShapes, kTypeShapeList, "POISONDEATH.SHP" },
	{ kFluteShapes, kTypeShapeList, "FLUTE.SHP" },
	{ kWinter1Shapes, kTypeShapeList, "WINTER1.SHP" },
	{ kWinter2Shapes, kTypeShapeList, "WINTER2.SHP" },
	{ kWinter3Shapes, kTypeShapeList, "WINTER3.SHP" },
	{ kDrinkShapes, kTypeShapeList, "DRINK.SHP" },
	{ kWispShapes, kTypeShapeList, "WISP.SHP" },
	{ kMagicAnimShapes, kTypeShapeList, "MAGICANIM.SHP" },
	{ kBranStoneShapes, kTypeShapeList, "BRANSTONE.SHP" },

	// IMAGE filename table
	{ kCharacterImageFilenames, kTypeStringList, "CHAR-IMAGE.TXT" },

	// AUDIO filename table
	{ kAudioTracks, kTypeStringList, "TRACKS.TXT" },
	{ kAudioTracksIntro, kTypeStringList, "TRACKSINT.TXT" },

	// AMULET anim
	{ kAmuleteAnimSeq, kTypeRawData, "AMULETEANIM.SEQ" },

	// PALETTE table
	{ kPaletteList1, kTypeRawData, "PALTABLE1.PAL" },
	{ kPaletteList2, kTypeRawData, "PALTABLE2.PAL" },
	{ kPaletteList3, kTypeRawData, "PALTABLE3.PAL" },
	{ kPaletteList4, kTypeRawData, "PALTABLE4.PAL" },
	{ kPaletteList5, kTypeRawData, "PALTABLE5.PAL" },
	{ kPaletteList6, kTypeRawData, "PALTABLE6.PAL" },
	{ kPaletteList7, kTypeRawData, "PALTABLE7.PAL" },
	{ kPaletteList8, kTypeRawData, "PALTABLE8.PAL" },
	{ kPaletteList9, kTypeRawData, "PALTABLE9.PAL" },
	{ kPaletteList10, kTypeRawData, "PALTABLE10.PAL" },
	{ kPaletteList11, kTypeRawData, "PALTABLE11.PAL" },
	{ kPaletteList12, kTypeRawData, "PALTABLE12.PAL" },
	{ kPaletteList13, kTypeRawData, "PALTABLE13.PAL" },
	{ kPaletteList14, kTypeRawData, "PALTABLE14.PAL" },
	{ kPaletteList15, kTypeRawData, "PALTABLE15.PAL" },
	{ kPaletteList16, kTypeRawData, "PALTABLE16.PAL" },
	{ kPaletteList17, kTypeRawData, "PALTABLE17.PAL" },
	{ kPaletteList18, kTypeRawData, "PALTABLE18.PAL" },
	{ kPaletteList19, kTypeRawData, "PALTABLE19.PAL" },
	{ kPaletteList20, kTypeRawData, "PALTABLE20.PAL" },
	{ kPaletteList21, kTypeRawData, "PALTABLE21.PAL" },
	{ kPaletteList22, kTypeRawData, "PALTABLE22.PAL" },
	{ kPaletteList23, kTypeRawData, "PALTABLE23.PAL" },
	{ kPaletteList24, kTypeRawData, "PALTABLE24.PAL" },
	{ kPaletteList25, kTypeRawData, "PALTABLE25.PAL" },
	{ kPaletteList26, kTypeRawData, "PALTABLE26.PAL" },
	{ kPaletteList27, kTypeRawData, "PALTABLE27.PAL" },
	{ kPaletteList28, kTypeRawData, "PALTABLE28.PAL" },
	{ kPaletteList29, kTypeRawData, "PALTABLE29.PAL" },
	{ kPaletteList30, kTypeRawData, "PALTABLE30.PAL" },
	{ kPaletteList31, kTypeRawData, "PALTABLE31.PAL" },
	{ kPaletteList32, kTypeRawData, "PALTABLE32.PAL" },
	{ kPaletteList33, kTypeRawData, "PALTABLE33.PAL" },

	// CREDITS (used in FM-TOWNS and AMIGA)
	{ kCreditsStrings, kTypeRawData, "CREDITS" },

	// FM-TOWNS specific
	{ kTOWNSSfxWDTable, kTypeTownsWDSfxTable, "SFXWDTABLE" },
	{ kTOWNSSfxBTTable, kTypeRawData, "SFXBTTABLE" },
	{ kTOWNSCDATable, kTypeRawData, "CDATABLE" },

	// PC98 specific
	{ kPC98StoryStrings, kTypeLanguageList, "INTROSTORY" },
	{ kPC98IntroSfx, kTypeRawData, "INTROSFX" },

	// AMIGA specific
	{ kAmigaIntroSFXTable, kTypeAmigaSfxTable, "SFXINTRO" },
	{ kAmigaGameSFXTable, kTypeAmigaSfxTable, "SFXGAME" },

	// HAND OF FATE

	// Sequence Player
	{ k2SeqplayPakFiles, kTypeStringList, "S_PAKFILES.TXT" },
	{ k2SeqplayCredits, kTypeRawData, "S_CREDITS.TXT" },
	{ k2SeqplayCreditsSpecial, kTypeStringList, "S_CREDITS2.TXT" },
	{ k2SeqplayStrings, kTypeLanguageList, "S_STRINGS" },
	{ k2SeqplaySfxFiles, k2TypeSoundList, "S_SFXFILES.TXT" },
	{ k2SeqplayTlkFiles, k2TypeLangSoundList, "S_TLKFILES" },
	{ k2SeqplaySeqData, k2TypeSeqData, "S_DATA.SEQ" },
	{ k2SeqplayIntroTracks, kTypeStringList, "S_INTRO.TRA" },
	{ k2SeqplayFinaleTracks, kTypeStringList, "S_FINALE.TRA" },
	{ k2SeqplayIntroCDA, kTypeRawData, "S_INTRO.CDA" },
	{ k2SeqplayFinaleCDA, kTypeRawData, "S_FINALE.CDA" },
	{ k2SeqplayShapeAnimData, k2TypeShpDataV1, "S_DEMO.SHP" },

	// Ingame
	{ k2IngamePakFiles, kTypeStringList, "I_PAKFILES.TXT" },
	{ k2IngameSfxFiles, k2TypeSize10StringList, "I_SFXFILES.TRA" },
	{ k2IngameSfxFilesTns, k2TypeSoundList, "I_SFXFILES.TRA" },
	{ k2IngameSfxIndex, kTypeRawData, "I_SFXINDEX.MAP" },
	{ k2IngameTracks, kTypeStringList, "I_TRACKS.TRA" },
	{ k2IngameCDA, kTypeRawData, "I_TRACKS.CDA" },
	{ k2IngameTalkObjIndex, kTypeRawData, "I_TALKOBJECTS.MAP" },
	{ k2IngameTimJpStrings, kTypeStringList, "I_TIMJPSTR.TXT" },
	{ k2IngameItemAnimData, k2TypeShpDataV2, "I_INVANIM.SHP" },
	{ k2IngameTlkDemoStrings, kTypeLanguageList, "I_TLKDEMO.TXT" },


	// MALCOLM'S REVENGE
	{ k3MainMenuStrings, kTypeStringList, "MAINMENU.TXT" },
	{ k3MusicFiles, k2TypeSoundList, "SCORE.TRA" },
	{ k3ScoreTable, kTypeRawData, "SCORE.MAP" },
	{ k3SfxFiles, k2TypeSfxList, "SFXFILES.TRA" },
	{ k3SfxMap, k3TypeRaw16to8, "SFXINDEX.MAP" },
	{ k3ItemAnimData, k3TypeShpData, "INVANIM.SHP" },
	{ k3ItemMagicTable, k3TypeRaw16to8, "ITEMMAGIC.MAP" },
	{ k3ItemStringMap, kTypeRawData, "ITEMSTRINGS.MAP" },

	// LANDS OF LORE

	// Demo Sequence Player
	{ kLolSeqplayIntroTracks, k2TypeSoundList, "S_INTRO.TRA" },

	// Ingame
	{ kLolIngamePakFiles, kTypeStringList, "PAKFILES.TXT" },

	{ kLolCharacterDefs, kTypeRawData, "CHARACTER.DEF" },
	{ kLolIngameSfxFiles, k2TypeSfxList, "SFXFILES.TRA" },
	{ kLolIngameSfxIndex, kTypeRawData, "SFXINDEX.MAP" },
	{ kLolMusicTrackMap, kTypeRawData, "MUSIC.MAP" },
	{ kLolGMSfxIndex, kTypeRawData, "SFX_GM.MAP" },
	{ kLolMT32SfxIndex, kTypeRawData, "SFX_MT32.MAP" },
	{ kLolPcSpkSfxIndex, kTypeRawData, "SFX_PCS.MAP" },
	{ kLolSpellProperties, kTypeRawData, "SPELLS.DEF" },
	{ kLolGameShapeMap, kTypeRawData, "GAMESHP.MAP" },
	{ kLolSceneItemOffs, kTypeRawData, "ITEMOFFS.DEF" },
	{ kLolCharInvIndex, k3TypeRaw16to8, "CHARINV.MAP" },
	{ kLolCharInvDefs, kTypeRawData, "CHARINV.DEF" },
	{ kLolCharDefsMan, kLolTypeRaw16, "CHMAN.DEF" },
	{ kLolCharDefsWoman, kLolTypeRaw16, "CHWOMAN.DEF" },
	{ kLolCharDefsKieran, kLolTypeRaw16, "CHKIERAN.DEF" },
	//{ kLolCharDefsUnk, kLolTypeRaw16, "CHUNK.DEF" },
	{ kLolCharDefsAkshel, kLolTypeRaw16, "CHAKSHEL.DEF" },
	{ kLolExpRequirements, kLolTypeRaw32, "EXPERIENCE.DEF" },
	{ kLolMonsterModifiers, kLolTypeRaw16, "MONSTMOD.DEF" },
	{ kLolMonsterLevelOffsets, kTypeRawData, "MONSTLVL.DEF" },
	{ kLolMonsterDirFlags, kTypeRawData, "MONSTDIR.DEF" },
	{ kLolMonsterScaleY, kTypeRawData, "MONSTZY.DEF" },
	{ kLolMonsterScaleX, kTypeRawData, "MONSTZX.DEF" },
	{ kLolMonsterScaleWH, kLolTypeRaw16, "MONSTSCL.DEF" },
	{ kLolFlyingItemShp, k3TypeRaw16to8, "THRWNSHP.DEF" },
	{ kLolInventoryDesc, kLolTypeRaw16, "INVDESC.DEF" },
	{ kLolLevelShpList, kTypeStringList, "SHPFILES.TXT" },
	{ kLolLevelDatList, kTypeStringList, "DATFILES.TXT" },
	{ kLolCompassDefs, k3TypeRaw16to8, "COMPASS.DEF" },
	{ kLolItemPrices, kLolTypeRaw16, "ITEMCOST.DEF" },
	{ kLolStashSetup, kTypeRawData, "MONEYSTS.DEF" },

	{ kLolDscUnk1, kTypeRawData, "DSCSHPU1.DEF" },
	{ kLolDscShapeIndex1, kTypeRawData, "DSCSHPI1.DEF" },
	{ kLolDscShapeIndex2, kTypeRawData, "DSCSHPI2.DEF" },
	{ kLolDscScaleWidthData, kLolTypeRaw16, "DSCSHPW.DEF" },
	{ kLolDscScaleHeightData, kLolTypeRaw16, "DSCSHPH.DEF" },
	{ kLolDscX, kLolTypeRaw16, "DSCSHPX.DEF" },
	{ kLolDscY, kTypeRawData, "DSCSHPY.DEF" },
	{ kLolDscTileIndex, kTypeRawData, "DSCSHPT.DEF" },
	{ kLolDscUnk2, kTypeRawData, "DSCSHPU2.DEF" },
	{ kLolDscDoorShapeIndex, kTypeRawData, "DSCDOOR.DEF" },
	{ kLolDscDimData1, kTypeRawData, "DSCDIM1.DEF" },
	{ kLolDscDimData2, kTypeRawData, "DSCDIM2.DEF" },
	{ kLolDscBlockMap, kTypeRawData, "DSCBLOCK1.DEF" },
	{ kLolDscDimMap, kTypeRawData, "DSCDIM.DEF" },
	{ kLolDscDoorScale, kLolTypeRaw16, "DSCDOOR3.DEF" },
	{ kLolDscShapeOvlIndex, k3TypeRaw16to8, "DSCBLOCK2.DEF" },
	{ kLolDscBlockIndex, kTypeRawData, "DSCBLOCKX.DEF" },
	{ kLolDscDoor4, kLolTypeRaw16, "DSCDOOR4.DEF" },
	{ kLolDscDoor1, kTypeRawData, "DSCDOOR1.DEF" },
	{ kLolDscDoorX, kLolTypeRaw16, "DSCDOORX.DEF" },
	{ kLolDscDoorY, kLolTypeRaw16, "DSCDOORY.DEF" },

	{ kLolScrollXTop, k3TypeRaw16to8, "SCROLLXT.DEF" },
	{ kLolScrollYTop, k3TypeRaw16to8, "SCROLLYT.DEF" },
	{ kLolScrollXBottom, k3TypeRaw16to8, "SCROLLXB.DEF" },
	{ kLolScrollYBottom, k3TypeRaw16to8, "SCROLLYB.DEF" },

	{ kLolButtonDefs, kLolTypeButtonDef, "BUTTONS.DEF" },
	{ kLolButtonList1, kLolTypeRaw16, "BUTTON1.LST" },
	{ kLolButtonList2, kLolTypeRaw16, "BUTTON2.LST" },
	{ kLolButtonList3, kLolTypeRaw16, "BUTTON3.LST" },
	{ kLolButtonList4, kLolTypeRaw16, "BUTTON4.LST" },
	{ kLolButtonList5, kLolTypeRaw16, "BUTTON5.LST" },
	{ kLolButtonList6, kLolTypeRaw16, "BUTTON6.LST" },
	{ kLolButtonList7, kLolTypeRaw16, "BUTTON7.LST" },
	{ kLolButtonList8, kLolTypeRaw16, "BUTTON84.LST" },

	{ kLolLegendData, kTypeRawData, "MAPLGND.DEF" },
	{ kLolMapCursorOvl, kTypeRawData, "MAPCURSOR.PAL" },
	{ kLolMapStringId, kLolTypeRaw16, "MAPSTRID.LST" },
	//{ kLolMapPal, kTypeRawData, "MAP.PAL" },

	{ kLolSpellbookAnim, k3TypeRaw16to8, "MBOOKA.DEF" },
	{ kLolSpellbookCoords, k3TypeRaw16to8, "MBOOKC.DEF" },
	{ kLolHealShapeFrames, kTypeRawData, "MHEAL.SHP" },
	{ kLolLightningDefs, kTypeRawData, "MLGHTNG.DEF" },
	{ kLolFireballCoords, kLolTypeRaw16, "MFIREBLL.DEF" },

	{ kLolCredits, kTypeRawData, "LOLCREDITS" },

	{ kLolHistory, kTypeRawData, "HISTORY.FLS" },

	{ -1, 0, 0 }
};

const ExtractFilename *getFilenameDesc(const int id) {
	for (const ExtractFilename *i = extractFilenames; i->id != -1; ++i) {
		if (i->id == id)
			return i;
	}
	return 0;
}

// filename processing

bool getFilename(char *dstFilename, const ExtractInformation *info, const int id) {
	const ExtractFilename *i = getFilenameDesc(id);

	if (!i)
		return false;

	const ExtractType *type = findExtractType(i->type);
	type->createFilename(dstFilename, info, i->filename);
	return true;
}

// misc tables

const SpecialExtension specialTable[] = {
	{ kTalkieVersion, "CD" },
	{ kDemoVersion, "DEM" },
	{ kTalkieDemoVersion, "CD.DEM" },

	{ -1, 0 }
};

const Language languageTable[] = {
	{ EN_ANY, "ENG" },
	{ DE_DEU, "GER" },
	{ FR_FRA, "FRE" },
	{ IT_ITA, "ITA" },
	{ ES_ESP, "SPA" },
	{ JA_JPN, "JPN" },
	{ -1, 0 }
};

const PlatformExtension platformTable[] = {
	{ kPlatformAmiga, "AMG" },
	{ kPlatformFMTowns, "TNS" },
	{ kPlatformPC98, "98" },

	{ -1, 0 }
};

// index generation

struct IndexTable {
	int type;
	int value;

	bool operator==(int t) const {
		return (type == t);
	}
};

const IndexTable iGameTable[] = {
	{ kKyra1, 0 },
	{ kKyra2, 1 },
	{ kKyra3, 2 },
	{ kLol, 3 },
	{ -1, -1 }
};

byte getGameID(int game) {
	return std::find(iGameTable, iGameTable + ARRAYSIZE(iGameTable) - 1, game)->value;
}

/*const IndexTable iLanguageTable[] = {
	{ EN_ANY, 0 },
	{ FR_FRA, 1 },
	{ DE_DEU, 2 },
	{ ES_ESP, 3 },
	{ IT_ITA, 4 },
	{ JA_JPN, 5 },
	{ -1, -1 }
};

byte getLanguageID(int lang) {
	return std::find(iLanguageTable, iLanguageTable + ARRAYSIZE(iLanguageTable) - 1, lang)->value;
}*/

const IndexTable iPlatformTable[] = {
	{ kPlatformPC, 0 },
	{ kPlatformAmiga, 1 },
	{ kPlatformFMTowns, 2 },
	{ kPlatformPC98, 3 },
	{ kPlatformMacintosh, 4 },
	{ -1, -1 }
};

byte getPlatformID(int platform) {
	return std::find(iPlatformTable, iPlatformTable + ARRAYSIZE(iPlatformTable) - 1, platform)->value;
}

const IndexTable iSpecialTable[] = {
	{ kNoSpecial, 0 },
	{ kTalkieVersion, 1 },
	{ kDemoVersion, 2 },
	{ kTalkieDemoVersion, 3 },
	{ -1, -1 }
};

byte getSpecialID(int special) {
	return std::find(iSpecialTable, iSpecialTable + ARRAYSIZE(iSpecialTable) - 1, special)->value;
}

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
	case kForestSeq:
		return "kForestSeq";
	case kKallakWritingSeq:
		return "kKallakWritingSeq";
	case kKyrandiaLogoSeq:
		return "kKyrandiaLogoSeq";
	case kKallakMalcolmSeq:
		return "kKallakMalcolmSeq";
	case kMalcolmTreeSeq:
		return "kMalcolmTreeSeq";
	case kWestwoodLogoSeq:
		return "kWestwoodLogoSeq";
	case kDemo1Seq:
		return "kDemo1Seq";
	case kDemo2Seq:
		return "kDemo2Seq";
	case kDemo3Seq:
		return "kDemo3Seq";
	case kDemo4Seq:
		return "kDemo4Seq";
	case kAmuleteAnimSeq:
		return "kAmuleteAnimSeq";
	case kOutroReunionSeq:
		return "kOutroReunionSeq";
	case kIntroCPSStrings:
		return "kIntroCPSStrings";
	case kIntroCOLStrings:
		return "kIntroCOLStrings";
	case kIntroWSAStrings:
		return "kIntroWSAStrings";
	case kIntroStrings:
		return "kIntroStrings";
	case kOutroHomeString:
		return "kOutroHomeString";
	case kRoomFilenames:
		return "kRoomFilenames";
	case kRoomList:
		return "kRoomList";
	case kCharacterImageFilenames:
		return "kCharacterImageFilenames";
	case kAudioTracks:
		return "kAudioTracks";
	case kAudioTracksIntro:
		return "kAudioTracksIntro";
	case kItemNames:
		return "kItemNames";
	case kTakenStrings:
		return "kTakenStrings";
	case kPlacedStrings:
		return "kPlacedStrings";
	case kDroppedStrings:
		return "kDroppedStrings";
	case kNoDropStrings:
		return "kNoDropStrings";
	case kPutDownString:
		return "kPutDownString";
	case kWaitAmuletString:
		return "kWaitAmuletString";
	case kBlackJewelString:
		return "kBlackJewelString";
	case kPoisonGoneString:
		return "kPoisonGoneString";
	case kHealingTipString:
		return "kHealingTipString";
	case kWispJewelStrings:
		return "kWispJewelStrings";
	case kMagicJewelStrings:
		return "kMagicJewelStrings";
	case kThePoisonStrings:
		return "kThePoisonStrings";
	case kFluteStrings:
		return "kFluteStrings";
	case kFlaskFullString:
		return "kFlaskFullString";
	case kFullFlaskString:
		return "kFullFlaskString";
	case kVeryCleverString:
		return "kVeryCleverString";
	case kNewGameString:
		return "kNewGameString";
	case kDefaultShapes:
		return "kDefaultShapes";
	case kHealing1Shapes:
		return "kHealing1Shapes";
	case kHealing2Shapes:
		return "kHealing2Shapes";
	case kPoisonDeathShapes:
		return "kPoisonDeathShapes";
	case kFluteShapes:
		return "kFluteShapes";
	case kWinter1Shapes:
		return "kWinter1Shapes";
	case kWinter2Shapes:
		return "kWinter2Shapes";
	case kWinter3Shapes:
		return "kWinter3Shapes";
	case kDrinkShapes:
		return "kDrinkShapes";
	case kWispShapes:
		return "kWispShapes";
	case kMagicAnimShapes:
		return "kMagicAnimShapes";
	case kBranStoneShapes:
		return "kBranStoneShapes";
	case kPaletteList1:
		return "kPaletteList1";
	case kPaletteList2:
		return "kPaletteList2";
	case kPaletteList3:
		return "kPaletteList3";
	case kPaletteList4:
		return "kPaletteList4";
	case kPaletteList5:
		return "kPaletteList5";
	case kPaletteList6:
		return "kPaletteList6";
	case kPaletteList7:
		return "kPaletteList7";
	case kPaletteList8:
		return "kPaletteList8";
	case kPaletteList9:
		return "kPaletteList9";
	case kPaletteList10:
		return "kPaletteList10";
	case kPaletteList11:
		return "kPaletteList11";
	case kPaletteList12:
		return "kPaletteList12";
	case kPaletteList13:
		return "kPaletteList13";
	case kPaletteList14:
		return "kPaletteList14";
	case kPaletteList15:
		return "kPaletteList15";
	case kPaletteList16:
		return "kPaletteList16";
	case kPaletteList17:
		return "kPaletteList17";
	case kPaletteList18:
		return "kPaletteList18";
	case kPaletteList19:
		return "kPaletteList19";
	case kPaletteList20:
		return "kPaletteList20";
	case kPaletteList21:
		return "kPaletteList21";
	case kPaletteList22:
		return "kPaletteList22";
	case kPaletteList23:
		return "kPaletteList23";
	case kPaletteList24:
		return "kPaletteList24";
	case kPaletteList25:
		return "kPaletteList25";
	case kPaletteList26:
		return "kPaletteList26";
	case kPaletteList27:
		return "kPaletteList27";
	case kPaletteList28:
		return "kPaletteList28";
	case kPaletteList29:
		return "kPaletteList29";
	case kPaletteList30:
		return "kPaletteList30";
	case kPaletteList31:
		return "kPaletteList31";
	case kPaletteList32:
		return "kPaletteList32";
	case kPaletteList33:
		return "kPaletteList33";
	case kGUIStrings:
		return "kGUIStrings";
	case kConfigStrings:
		return "kConfigStrings";
	case kTOWNSSfxWDTable:
		return "kTOWNSSfxWDTable";
	case kTOWNSSfxBTTable:
		return "kTOWNSSfxBTTable";
	case kTOWNSCDATable:
		return "kTOWNSCDATable";
	case kPC98StoryStrings:
		return "kPC98StoryStrings";
	case kPC98IntroSfx:
		return "kPC98IntroSfx";
	case kCreditsStrings:
		return "kCreditsStrings";
	case kAmigaIntroSFXTable:
		return "kAmigaIntroSFXTable";
	case kAmigaGameSFXTable:
		return "kAmigaGameSFXTable";
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
	case k2IngameSfxFilesTns:
		return "k2IngameSfxFilesTns";
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
	case k2IngameItemAnimData:
		return "k2IngameItemAnimData";
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
	case kLolSeqplayIntroTracks:
		return "kLolSeqplayIntroTracks";
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
	case kLolGMSfxIndex:
		return "kLolGMSfxIndex";
	case kLolMT32SfxIndex:
		return "kLolMT32SfxIndex";
	case kLolPcSpkSfxIndex:
		return "kLolPcSpkSfxIndex";
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
	case kLolMonsterLevelOffsets:
		return "kLolMonsterLevelOffsets";
	case kLolMonsterDirFlags:
		return "kLolMonsterDirFlags";
	case kLolMonsterScaleY:
		return "kLolMonsterScaleY";
	case kLolMonsterScaleX:
		return "kLolMonsterScaleX";
	case kLolMonsterScaleWH:
		return "kLolMonsterScaleWH";
	case kLolFlyingItemShp:
		return "kLolFlyingItemShp";
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
	case kLolDscShapeIndex1:
		return "kLolDscShapeIndex1";
	case kLolDscShapeIndex2:
		return "kLolDscShapeIndex2";
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
	case kLolDscShapeOvlIndex:
		return "kLolDscShapeOvlIndex";
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

struct DataIdEntry {
	DataIdEntry() : extractInfo(), id(-1) {}
	DataIdEntry(ExtractEntrySearchData e, int i) : extractInfo(e), id(i) {}

	ExtractEntrySearchData extractInfo;
	int id;
};

struct ExtractData {
	ExtractData() : extractInfo(), result() {}
	ExtractData(ExtractEntrySearchData e, Search::ResultData r) : extractInfo(e), result(r) {}

	ExtractEntrySearchData extractInfo;
	Search::ResultData result;
};

typedef std::list<DataIdEntry> DataIdList;

typedef std::map<int, Search::ResultData> IdMap;

bool getExtractionData(const Game *g, Search &search, IdMap &map);

bool process(PAKFile &out, const Game *g, const byte *data, const uint32 size) {
	char filename[128];

	ExtractInformation extractInfo;
	extractInfo.game = g->game;
	extractInfo.lang = g->lang;
	extractInfo.platform = g->platform;
	extractInfo.special = g->special;

	Search search(data, size);
	IdMap ids;

	if (!getExtractionData(g, search, ids))
		return false;

	const int *needList = getNeedList(g);
	if (!needList) {
		fprintf(stderr, "ERROR: No entry need list available\n");
		return false;
	}

	bool breakProcess = false;

	// Compare against need list
	for (const int *entry = needList; *entry != -1; ++entry) {
		if (ids.find(*entry) != ids.end())
			continue;

		// Try whether the data is present in the kyra.dat file already
		filename[0] = 0;
		if (!getFilename(filename, &extractInfo, *entry))
			error("couldn't find filename for id %d", *entry);

		PAKFile::cFileList *list = out.getFileList();
		// If the data wasn't found already, we need to break the extraction here
		if (!list || !list->findEntry(filename)) {
			fprintf(stderr, "Couldn't find id %d/%s in executable file\n", *entry, getIdString(*entry));
			breakProcess = true;
		} else {
			warning("Id %d/%s is present in kyra.dat but could not be found in the executable", *entry, getIdString(*entry));
		}
	}

	if (breakProcess)
		return false;

	for (IdMap::const_iterator i = ids.begin(); i != ids.end(); ++i) {
		const int id = i->first;
	
		filename[0] = 0;
		if (!getFilename(filename, &extractInfo, id)) {
			fprintf(stderr, "ERROR: couldn't get filename for id %d\n", id);
			return false;
		}

		const ExtractFilename *fDesc = getFilenameDesc(id);

		if (!fDesc) {
			fprintf(stderr, "ERROR: couldn't find file description for id %d\n", id);
			return false;
		}

		const ExtractType *tDesc = findExtractType(fDesc->type);

		if (!tDesc) {
			fprintf(stderr, "ERROR: couldn't find type description for id %d\n", id);
			return false;
		}

		PAKFile::cFileList *list = out.getFileList();
		if (list && list->findEntry(filename) != 0)
			continue;

		if (!tDesc->extract(out, &extractInfo, data + i->second.offset, i->second.data.size, filename, id)) {
			fprintf(stderr, "ERROR: couldn't extract id %d\n", id);
			return false;
		}
	}

	if (!updateIndex(out, &extractInfo)) {
		error("couldn't update INDEX file, stop processing of all files");
		return false;
	}

	return true;
}

bool setupSearch(const int *needList, Search &search, DataIdList &dataIdList) {
	for (const int *entry = needList; *entry != -1; ++entry) {
		ExtractEntryList providers = getProvidersForId(*entry);

		if (providers.empty()) {
			fprintf(stderr, "ERROR: No provider for id %d/%s\n", *entry, getIdString(*entry));
			return false;
		} else {
			for (ExtractEntryList::const_iterator i = providers.begin(); i != providers.end(); ++i) {
				// We will add *all* providers here, regardless of the language and platform!
				search.addData(i->hint);
				dataIdList.push_back(DataIdEntry(*i, *entry));
			}
		}
	}

	return true;
}

bool getExtractionData(const Game *g, Search &search, IdMap &ids) {
	DataIdList dataIdList;

	const int *needList = getNeedList(g);
	if (!needList) {
		fprintf(stderr, "ERROR: No entry need list available\n");
		return false;
	}

	if (!setupSearch(needList, search, dataIdList))
		return false;

	// Process the data search
	Search::ResultList results;
	search.search(results);

	if (results.empty()) {
		fprintf(stderr, "ERROR: Couldn't find any required data\n");
		return false;
	}

	for (const int *entry = needList; *entry != -1; ++entry) {
		typedef std::list<ExtractData> ExtractList;
		ExtractList idResults;

		// Fill in all id entries found
		for (Search::ResultList::const_iterator i = results.begin(); i != results.end(); ++i) {
			for (DataIdList::const_iterator j = dataIdList.begin(); j != dataIdList.end(); ++j) {
				if (j->id == *entry && i->data == j->extractInfo.hint)
					idResults.push_back(ExtractData(j->extractInfo, *i));
			}
		}

		// Sort out entries with mistmatching language settings
		for (ExtractList::iterator i = idResults.begin(); i != idResults.end();) {
			if (i->extractInfo.lang != UNK_LANG && i->extractInfo.lang != g->lang) {
				i = idResults.erase(i);
				continue;
			}

			++i;
		}

		// Determin whether we have a 100% platform match
		bool hasPlatformMatch = false;
		for (ExtractList::const_iterator i = idResults.begin(); i != idResults.end(); ++i) {
			if (i->extractInfo.platform == g->platform)
				hasPlatformMatch = true;
		}

		for (ExtractList::iterator i = idResults.begin(); i != idResults.end();) {
			// Sort out entries with mistmatching platform settings, when we have a platform match
			if (hasPlatformMatch && i->extractInfo.platform != g->platform) {
				i = idResults.erase(i);
				continue;
			}

			++i;
		}

		if (idResults.empty())
			continue;

		if (idResults.size() > 1)
			warning("Multiple entries found for id %d/%s", *entry, getIdString(*entry));

		Search::ResultData result;
		result.data.size = 0;
		result.offset = 0xFFFFFFFF;

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
		for (ExtractList::iterator i = idResults.begin(); i != idResults.end(); ++i) {
			if (result.data.size <= i->result.data.size) {
				if (result.offset >= i->result.offset)
					result = i->result;
			}
		}

		ids[*entry] = result;
	}

	// Free up some memory
	results.clear();
	dataIdList.clear();

	return true;
}

