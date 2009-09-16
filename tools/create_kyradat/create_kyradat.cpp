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

#include "tables.h"
#include "providers.h"

#include "md5.h"

enum {
	kKyraDatVersion = 55,
	kIndexSize = 12
};

// tables

#include "misc.h"

bool extractRaw(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch);
bool extractStrings(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch);
bool extractRooms(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch);
bool extractShapes(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch);
bool extractAmigaSfx(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch);
bool extractHofSeqData(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch);
bool extractHofShapeAnimDataV1(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch);
bool extractHofShapeAnimDataV2(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch);

bool extractStringsWoSuffix(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch);
bool extractPaddedStrings(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch);
bool extractRaw16to8(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch);
bool extractMrShapeAnimData(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch);
bool extractRaw16(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch);
bool extractRaw32(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch);
bool extractLolButtonDefs(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch);

int extractHofSeqData_checkString(const void *ptr, uint8 checkSize);
int extractHofSeqData_isSequence(const void *ptr, const Game *g, uint32 maxCheckSize);
int extractHofSeqData_isControl(const void *ptr, uint32 size);

void createFilename(char *dstFilename, const int gid, const int lang, const int special, const char *filename);
void createLangFilename(char *dstFilename, const int gid, const int lang, const int special, const char *filename);

const ExtractType extractTypeTable[] = {
	{ kTypeLanguageList, extractStrings, createLangFilename },
	{ kTypeStringList, extractStrings, createFilename },
	{ kTypeRoomList, extractRooms, createFilename },
	{ kTypeShapeList, extractShapes, createFilename },
	{ kTypeRawData, extractRaw, createFilename },
	{ kTypeAmigaSfxTable, extractAmigaSfx, createFilename },

	{ k2TypeSeqData, extractHofSeqData, createFilename },
	{ k2TypeShpDataV1, extractHofShapeAnimDataV1, createFilename },
	{ k2TypeShpDataV2, extractHofShapeAnimDataV2, createFilename },
	{ k2TypeSoundList, extractStringsWoSuffix, createFilename },
	{ k2TypeLangSoundList, extractStringsWoSuffix, createLangFilename },
	{ k2TypeSfxList, extractPaddedStrings, createFilename },
	{ k3TypeRaw16to8, extractRaw16to8, createFilename },
	{ k3TypeShpData, extractMrShapeAnimData, createFilename },

	{ kLolTypeRaw16, extractRaw16, createFilename },
	{ kLolTypeRaw32, extractRaw32, createFilename },
	{ kLolTypeButtonDef, extractLolButtonDefs, createFilename },

	{ -1, 0, 0}
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
	{ kCreditsStrings, kTypeRawData, "CREDITS" },

	// FM-TOWNS specific
	{ kKyra1TownsSFXwdTable, kTypeRawData, "SFXWDTABLE" },
	{ kKyra1TownsSFXbtTable, kTypeRawData, "SFXBTTABLE" },
	{ kKyra1TownsCDATable, kTypeRawData, "CDATABLE" },

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
	{ k2IngameSfxFiles, kTypeStringList, "I_SFXFILES.TRA" },
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
	//{ kLolADLSfxIndex, kTypeRawData, "SFX_ADL.MAP" },
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

// type processing

const ExtractType *findExtractType(const int type) {
	for (const ExtractType *i = extractTypeTable; i->type != -1; ++i) {
		if (i->type == type)
			return i;
	}
	return 0;
}

// filename processing

bool getFilename(char *dstFilename, const Game *g, const int id) {
	const ExtractFilename *i = getFilenameDesc(id);

	if (!i)
		return false;

	const ExtractType *type = findExtractType(i->type);
	type->createFilename(dstFilename, g->game, g->lang, g->special, i->filename);
	return true;
}

void createFilename(char *dstFilename, const int gid, const int lang, const int special, const char *filename) {
	strcpy(dstFilename, filename);

	static const char *gidExtensions[] = { "", ".K2", ".K3", 0, ".LOL" };
	strcat(dstFilename, gidExtensions[gid]);

	for (const SpecialExtension *specialE = specialTable; specialE->special != -1; ++specialE) {
		if (specialE->special == special) {
			strcat(dstFilename, ".");
			strcat(dstFilename, specialE->ext);
			break;
		}
	}
}

void createLangFilename(char *dstFilename, const int gid, const int lang, const int special, const char *filename) {
	strcpy(dstFilename, filename);

	for (const Language *langE = languageTable; langE->lang != -1; ++langE) {
		if (langE->lang == lang) {
			strcat(dstFilename, ".");
			strcat(dstFilename, langE->ext);
			break;
		}
	}

	static const char *gidExtensions[] = { "", ".K2", ".K3", 0, ".LOL" };
	strcat(dstFilename, gidExtensions[gid]);

	for (const SpecialExtension *specialE = specialTable; specialE->special != -1; ++specialE) {
		if (specialE->special == special) {
			strcat(dstFilename, ".");
			strcat(dstFilename, specialE->ext);
			break;
		}
	}
}

// extraction

bool extractRaw(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch) {
	uint8 *buffer = 0;

	if (fmtPatch == 2) {
		buffer = new uint8[0x12602];
		assert(buffer);
		memcpy(buffer, data, 0x7EE5);
		memcpy(buffer + 0x7EE5, data + 0x7EE7, 0x7FFF);
		memcpy(buffer + 0xFEE4, data + 0xFEE8, 0x271E);

		return out.addFile(filename, buffer, 0x12602);
	} else {
		buffer = new uint8[size];
		assert(buffer);
		memcpy(buffer, data, size);

		return out.addFile(filename, buffer, size);
	}
}

bool extractStrings(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch) {
	uint32 entries = 0;
	uint32 targetsize = size + 4;
	for (uint32 i = 0; i < size; ++i) {
		if (!data[i]) {
			if (g->special == kAmigaVersion) {
				if (i + 1 >= size)
					++entries;
				else if (!data[i+1] && !(i & 1))
					continue;
				else
					++entries;
			} else {
				++entries;
			}

			if (g->special == kFMTownsVersionE || g->special == kFMTownsVersionJ ||
				g->special == k2TownsFile1E || g->special == k2TownsFile1J ||
				g->special == k2TownsFile2E || g->special == k2TownsFile2J || fmtPatch == 5) {
				// prevents creation of empty entries (which we have mostly between all strings in the FM-TOWNS version)
				while (!data[++i]) {
					if (i == size)
						break;
					targetsize--;
				}
				if (fmtPatch == 1) {
					// Here is the first step of the extra treatment for all FM-TOWNS string arrays that
					// contain more than one string and which the original code
					// addresses via stringname[boolJapanese].
					// We simply skip every other string
					if (i == size)
						continue;
					uint32 len = strlen((const char*) data + i);
					i += len;

					targetsize = targetsize - 1 - len;

					while (!data[++i]) {
						if (i == len)
							break;
						targetsize--;
					}
				}
			}
		}
	}

	if (fmtPatch == 2) {
		if (g->special == kFMTownsVersionE) {
			targetsize--;
			entries += 1;
		} else if (g->special == kFMTownsVersionJ) {
			targetsize += 2;
			entries += 2;
		}
	}

	if (fmtPatch == 3) {
		entries++;
		targetsize++;
	}

	if (fmtPatch == 4) {
		targetsize -= 9;
	}

	uint8 *buffer = new uint8[targetsize];
	assert(buffer);
	memset(buffer, 0, targetsize);
	uint8 *output = buffer;
	const uint8 *input = (const uint8*) data;

	WRITE_BE_UINT32(output, entries); output += 4;
	if (g->special == kFMTownsVersionE || g->special == kFMTownsVersionJ ||
		g->special == k2TownsFile1E || g->special == k2TownsFile1J ||
		g->special == k2TownsFile2E || g->special == k2TownsFile2J || fmtPatch == 5) {
		const byte *c = data + size;
		do {
			if (fmtPatch == 2 && input - data == 0x3C0 && input[0x10] == 0x32) {
				memcpy(output, input, 0x0F);
				input += 0x11; output += 0x0F;
			}

			strcpy((char*) output, (const char*) input);
			uint32 stringsize = strlen((const char*)output) + 1;
			input += stringsize; output += stringsize;
			// skip empty entries
			while (!*input) {
				// Write one empty string into intro strings file
				if (fmtPatch == 2) {
					if ((g->special == kFMTownsVersionE && input - data == 0x260) ||
						(g->special == kFMTownsVersionJ && input - data == 0x2BD) ||
						(g->special == kFMTownsVersionJ && input - data == 0x2BE))
							*output++ = *input;
				}

				// insert one dummy string at hof sequence strings position 59
				if (fmtPatch == 3) {
					if ((g->special == k2TownsFile1E && input - data == 0x695) ||
						(g->special == k2TownsFile1J && input - data == 0x598))
							*output++ = *input;
				}

				if (++input == c)
					break;
			}

			if (fmtPatch == 1) {
				// Here is the extra treatment for all FM-TOWNS string arrays that
				// contain more than one string and which the original code
				// addresses via stringname[boolJapanese].
				// We simply skip every other string
				if (input == c)
					continue;
				input += strlen((const char*)input);
				while (!*input) {
					if (++input == c)
						break;
				}
			}

		} while (input < c);
	} else if (g->special == kAmigaVersion) {
		// we need to strip some aligment zeros out here
		int dstPos = 0;
		for (uint32 i = 0; i < size; ++i) {
			if (!data[i] && !(i & 1)) {
				if (i + 1 > size)
					continue;
				else if (i + 1 < size && !data[i+1])
					continue;
			}

			*output++ = data[i];
			++dstPos;
		}
		targetsize = dstPos + 4;
	} else {
		uint32 copySize = size;
		if (fmtPatch == 4) {
			memcpy(output, data, 44);
			output += 44;
			data += 44;
			for (int t = 1; t != 10; t++) {
				sprintf((char*) output, "COST%d_SH.PAK", t);
				output += 13;
			}
			data += 126;
			copySize -= 170;
		}
		memcpy(output, data, copySize);
	}

	return out.addFile(filename, buffer, targetsize);
}

bool extractRooms(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch) {
	// different entry size for the FM-TOWNS version
	const int roomEntrySize = (g->special == kFMTownsVersionE || g->special == kFMTownsVersionJ) ? (0x69) : ((g->special == kAmigaVersion) ? 0x52 : 0x51);
	const int countRooms = size / roomEntrySize;

	uint8 *buffer = new uint8[countRooms * 9 + 4];
	assert(buffer);
	uint8 *output = buffer;

	WRITE_BE_UINT32(output, countRooms); output += 4;

	const byte *src = data;
	if (g->special == kAmigaVersion) {
		for (int i = 0; i < countRooms; ++i) {
			*output++ = *src++; assert(*src == 0); ++src;
			memcpy(output, src, 8); output += 0x8;
			src += roomEntrySize - 0x2;
		}
	} else {
		for (int i = 0; i < countRooms; ++i) {
			*output++ = *src++;
			WRITE_BE_UINT16(output, READ_LE_UINT16(src)); output += 2; src += 2;
			WRITE_BE_UINT16(output, READ_LE_UINT16(src)); output += 2; src += 2;
			WRITE_BE_UINT16(output, READ_LE_UINT16(src)); output += 2; src += 2;
			WRITE_BE_UINT16(output, READ_LE_UINT16(src)); output += 2; src += 2;
			src += roomEntrySize - 0x9;
		}
	}

	return out.addFile(filename, buffer, countRooms * 9 + 4);
}

bool extractShapes(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch) {
	byte *buffer = new byte[size + 1 * 4];
	assert(buffer);
	byte *output = buffer;

	const int count = size / 0x07;
	WRITE_BE_UINT32(output, count); output += 4;
	memcpy(output, data, size);

	return out.addFile(filename, buffer, size + 1 * 4);
}

bool extractAmigaSfx(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch) {
	const uint32 entries = size / 8;
	byte *buffer = new byte[entries * 6 + 1 * 4];

	byte *output = buffer;
	WRITE_BE_UINT32(output, entries); output += 4;

	for (uint32 i = 0; i < entries; ++i) {
		*output++ = *data++;	// Note
		*output++ = *data++;	// Patch
		data += 2;				// Unused
		WRITE_BE_UINT16(output, READ_BE_UINT16(data)); output += 2; data += 2; // Duration
		*output++ = *data++;	// Volume
		*output++ = *data++;	// Pan
	}

	return out.addFile(filename, buffer, entries * 6 + 1 * 4);
}

bool extractHofSeqData(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch) {
	int numSequences = 0;
	int numNestedSequences = 0;

	uint16 headerSize = 50 * sizeof(uint16);
	uint16 bufferSize = size + headerSize;
	byte *buffer = new byte[bufferSize];
	assert(buffer);
	memset(buffer, 0, bufferSize );
	uint16 *header = (uint16*) buffer;
	byte *output = buffer + headerSize;
	uint16 *hdout = header;

	//debug(1, "\nProcessing Hand of Fate sequence data:\n--------------------------------------\n");
	for (int cycle = 0; cycle < 2; cycle++) {
		const byte *ptr = data;
		hdout++;

		const byte * endOffs = (const byte *)(data + size);

		// detect sequence structs
		while (ptr < endOffs) {
			if (ptr[1]) {
				error("invalid sequence data encountered");
				delete[] buffer;
				return false;
			}

			int v = extractHofSeqData_isSequence(ptr, g, endOffs - ptr);

			if (cycle == 0 && v == 1) {
				if ((g->special == k2FloppyFile1 && *ptr == 5) || (g->special == k2DemoVersion && (ptr - data == 312))) {
					// patch for floppy version: skips invalid ferb sequence
					// patch for demo: skips invalid title sequence
					ptr += 54;
					continue;
				}

				numSequences++;
				uint16 relOffs = (uint16) (output - buffer);
				WRITE_BE_UINT16(hdout, relOffs);
				hdout++;

				WRITE_BE_UINT16(output, READ_LE_UINT16(ptr)); // flags
				ptr += 2;
				output += 2;

				memcpy(output, ptr, 28); // wsa and cps file names
				ptr += 28;
				output += 28;

				if (g->special == k2TownsFile1E) { // startupCommand + finalCommand
					memcpy(output , ptr, 2);
					ptr += 2;
					output += 2;
				} else {
					*output++ = READ_LE_UINT16(ptr) & 0xff;
					ptr += 2;
					*output++ = READ_LE_UINT16(ptr) & 0xff;
					ptr += 2;
				}

				for (int w = 0; w < 7; w++) { //stringIndex1 to yPos
					WRITE_BE_UINT16(output, READ_LE_UINT16(ptr));
					ptr += 2;
					output += 2;
				}

				ptr += 4;
				WRITE_BE_UINT16(output, READ_LE_UINT16(ptr)); // duration
				ptr += 2;
				output+= 2;

			} else if (cycle == 1 && v != 1 && v != -2) {
				uint16 controlOffs = 0;
				uint16 ctrSize = 0;
				if (v) {
					const byte *ctrStart = ptr;
					while (v && v != -2) {
						ptr++;
						v = extractHofSeqData_isSequence(ptr, g, endOffs - ptr);
					}

					if (v == -2)
						break;

					ctrSize = (uint16)(ptr - ctrStart);

					if (g->special != k2DemoVersion &&
						extractHofSeqData_isControl(ctrStart, ctrSize)) {
						controlOffs = (uint16) (output - buffer);
						*output++ = ctrSize >> 2;

						for (int cc = 0; cc < ctrSize; cc += 2)
							WRITE_BE_UINT16(output + cc, READ_LE_UINT16(ctrStart + cc)); // frame control
						output += ctrSize;
					}
				}

				numNestedSequences++;
				uint16 relOffs = (uint16) (output - buffer);
				WRITE_BE_UINT16(hdout, relOffs);
				hdout++;

				WRITE_BE_UINT16(output, READ_LE_UINT16(ptr)); // flags
				ptr += 2;
				output += 2;

				memcpy(output, ptr, 14); // wsa file name
				ptr += 14;
				output += 14;

				// startframe
				WRITE_BE_UINT16(output, READ_LE_UINT16(ptr));
				ptr += 2;
				output += 2;

				// endFrame
				WRITE_BE_UINT16(output, (ctrSize && ((ctrSize >> 2)  < READ_LE_UINT16(ptr))) ? (ctrSize >> 2) : READ_LE_UINT16(ptr));
				ptr += 2;
				output += 2;

				// frameDelay
				WRITE_BE_UINT16(output, READ_LE_UINT16(ptr));
				ptr += 2;
				output += 2;

				ptr += 4;

				for (int w = 0; w < 2; w++) { //x, y
					WRITE_BE_UINT16(output, READ_LE_UINT16(ptr));
					ptr += 2;
					output += 2;
				}

				if (!READ_LE_UINT32(ptr))
					controlOffs = 0;

				WRITE_BE_UINT16(output, controlOffs);
				if (g->special != k2DemoVersion && g->special != k2DemoLol)
					ptr += 4;
				output += 2;

				if (g->special != k2DemoVersion && g->special != k2DemoLol) {
					for (int w = 0; w < 2; w++) { //startupCommand, finalCommand
						WRITE_BE_UINT16(output, READ_LE_UINT16(ptr));
						ptr += 2;
						output += 2;
					}
				} else {
					memset(output, 0, 4);
					output += 4;
				}

				if (g->special == k2TownsFile1E)
					ptr += 2;

			} else if (cycle == 0) {
				while (v != 1 && v != -2) {
					ptr++;
					v = extractHofSeqData_isSequence(ptr, g, endOffs - ptr);
				}

				if (v == -2)
					break;


			} else if (cycle == 1) {
				while (v == 1 && v != -2) {
					ptr++;
					v = extractHofSeqData_isSequence(ptr, g, endOffs - ptr);
				}

				if (v == -2)
					break;
			}
		}
	}

	uint16 finHeaderSize = (2 + numSequences + numNestedSequences) * sizeof(uint16);
	uint16 finBufferSize = ((output - buffer) - headerSize) + finHeaderSize;
	byte *finBuffer = new byte[finBufferSize];
	assert(finBuffer);
	uint16 diff = headerSize - finHeaderSize;
	uint16 *finHeader = (uint16*) finBuffer;

	for (int i = 1; i < finHeaderSize; i++)
		WRITE_BE_UINT16(&finHeader[i], (READ_BE_UINT16(&header[i]) - diff));
	WRITE_BE_UINT16(finHeader, numSequences);
	WRITE_BE_UINT16(&finHeader[numSequences + 1], numNestedSequences);
	memcpy (finBuffer + finHeaderSize, buffer + headerSize, finBufferSize - finHeaderSize);
	delete[] buffer;

	finHeader = (uint16*) (finBuffer + ((numSequences + 2) * sizeof(uint16)));
	for (int i = 0; i < numNestedSequences; i++) {
		uint8 * offs = finBuffer + READ_BE_UINT16(finHeader++) + 26;
		uint16 ctrl = READ_BE_UINT16(offs);
		if (ctrl)
			ctrl -= diff;
		WRITE_BE_UINT16(offs, ctrl);
	}

	return out.addFile(filename, finBuffer, finBufferSize);
}

int extractHofSeqData_checkString(const void *ptr, uint8 checkSize) {
	// return values: 1 = text; 0 = zero string; -1 = other

	int t = 0;
	int c = checkSize;
	const uint8 *s = (const uint8*)ptr;

	// check for character string
	while (c--) {
		if (*s > 31 && *s < 123)
			t++;
		s++;
	}

	if (t == checkSize)
		return 1;

	// check for zero string
	c = checkSize;
	uint32 sum = 0;
	s = (const uint8*)ptr;
	while (c--)
		sum += *s++;

	return (sum) ? -1 : 0;
}

int extractHofSeqData_isSequence(const void *ptr, const Game *g, uint32 maxCheckSize) {
	// return values: 1 = Sequence; 0 = Nested Sequence; -1 = other; -2 = overflow

	if (maxCheckSize < 30)
		return -2;

	const uint8 * s = (const uint8*)ptr;
	int c1 = extractHofSeqData_checkString(s + 2, 6);
	int c2 = extractHofSeqData_checkString(s + 16, 6);
	int c3 = extractHofSeqData_checkString(s + 2, 14);
	int c4 = extractHofSeqData_checkString(s + 16, 14);
	int c0 = s[1];
	int c5 = s[0];

	if (c0 == 0 && c5 && ((c1 + c2) >= 1) && (!(c3 == 0 && c2 != 1)) && (!(c4 == 0 && c1 != 1))) {
		if (maxCheckSize < 41)
			return -2;

		if (g->special == k2TownsFile1E) {
			if (!(s[37] | s[39]) && s[38] > s[36])
				return 1;
		} else {
			if (!(s[39] | s[41]) && s[40] > s[38])
				return 1;
		}
	}

	if (c0 == 0 && c5 == 4 && c3 == 0 && c4 == 0) {
		if (maxCheckSize >= 41 && READ_LE_UINT32(s + 34) && !(s[39] | s[41]) && s[40] > s[38])
			return 1;
	}

	if (c0 == 0 && c5 && c1 == 1 && c4 == -1 && s[20])
		return 0;

	return -1;
}

int extractHofSeqData_isControl(const void *ptr, uint32 size) {
	// return values: 1 = possible frame control data; 0 = definitely not frame control data

	const uint8 *s = (const uint8*)ptr;
	for (uint32 i = 2; i < size; i += 4) {
		if (!s[i])
			return 0;
	}

	for (uint32 i = 1; i < size; i += 2) {
		if (s[i])
			return 0;
	}
	return 1;
}

bool extractHofShapeAnimDataV1(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch) {
	int outsize = 1;
	uint8 *buffer = new uint8[size + 1];
	const uint8 *src = data;
	uint8 *dst = buffer + 1;

	for (int i = 0; i < 4; i++) {
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2;
		dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 4;
		dst += 2;
		outsize += 4;

		for (int j = 0; j < 20; j++) {
			WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
			src += 2;
			dst += 2;
			outsize += 2;
		}

	};

	*buffer = 4; // number of items

	return out.addFile(filename, buffer, outsize);
}

bool extractHofShapeAnimDataV2(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch) {
	int outsize = 1;
	uint8 *buffer = new uint8[size + 1];
	const uint8 *src = data;
	uint8 *dst = buffer + 1;
	const uint8 *fin = data + size;
	int count = 0;

	do {
		if (READ_LE_UINT16(src) == 0xffff)
			break;

		count++;

		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2;
		dst += 2;

		uint8 numFrames = *src;
		*dst++ = numFrames;
		src += 6;
		outsize += 3;

		for (int i = 0; i < (numFrames << 1); i++) {
			WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
			src += 2;
			dst += 2;
			outsize += 2;
		}

		src += (48 - (numFrames << 2));

	} while (src < fin);

	*buffer = count; // number of items

	return out.addFile(filename, buffer, outsize);
}

bool extractStringsWoSuffix(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch) {
	int outsize = size + 4;
	uint8 *buffer = new uint8[outsize];
	const uint8 *src = data;
	uint8 *dst = buffer + 4;
	const uint8 *fin = src + size;
	int entries = 0;

	while (src < fin) {
		while (!*src && src < fin)
			src++;
		while (*src && *src != '.' && src < fin)
			*dst++ = *src++;

		*dst++ = '\0';
		entries++;

		if (*src == '.') {
			while (*src && src < fin)
				src++;
		}
	}

	WRITE_BE_UINT32(buffer, entries);
	outsize = dst - buffer;

	return out.addFile(filename, buffer, outsize);
}

bool extractPaddedStrings(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch) {
	int outsize = size + 4;
	uint8 *buffer = new uint8[outsize];
	const uint8 *src = data;
	uint8 *dst = buffer + 4;
	const uint8 *fin = src + size;
	int entries = 0;

	while (src < fin) {
		while (!*src && src < fin)
			src++;
		while (*src && src < fin)
			*dst++ = *src++;

		*dst++ = '\0';
		entries++;
	}

	WRITE_BE_UINT32(buffer, entries);
	outsize = dst - buffer;

	return out.addFile(filename, buffer, outsize);
}

bool extractRaw16to8(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch) {
	int outsize = size >> 1;
	uint8 *buffer = new uint8[outsize];
	const uint8 *src = data;
	uint8 *dst = buffer;

	for (int i = 0; i < outsize; i++) {
		*dst++ = *src++;
		*src++;
	}

	return out.addFile(filename, buffer, outsize);
}

bool extractRaw16(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch) {
	uint8 *buffer = new uint8[size];
	const uint8 *src = data;
	uint8 *dst = buffer;

	for (uint32 i = 0; i < (size >> 1); i++) {
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2;
		dst += 2;
	}

	return out.addFile(filename, buffer, size);
}

bool extractRaw32(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch) {
	uint8 *buffer = new uint8[size];
	const uint8 *src = data;
	uint8 *dst = buffer;

	for (uint32 i = 0; i < (size >> 2); i++) {
		WRITE_BE_UINT32(dst, READ_LE_UINT32(src));
		src += 4;
		dst += 4;
	}

	return out.addFile(filename, buffer, size);
}

bool extractLolButtonDefs(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch) {
	int num = size / 22;
	uint8 *buffer = new uint8[size];
	uint32 outsize = num * 18;
	const uint8 *src = data;
	uint8 *dst = buffer;

	for (int i = 0; i < num; i++) {
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 6; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
	}

	return out.addFile(filename, buffer, outsize);
}

bool extractMrShapeAnimData(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch) {
	int outsize = 1;
	uint8 *buffer = new uint8[size + 1];
	const uint8 *src2 = data;
	const uint8 *src1 = data + 324;
	uint8 *dst = buffer + 1;
	const uint8 *fin = data + size;
	int count = 0;

	do {
		if (READ_LE_UINT16(src1) == 0xffff)
			break;

		count++;

		WRITE_BE_UINT16(dst, READ_LE_UINT16(src1));
		src1 += 2;
		dst += 2;

		uint8 numFrames = *src1;
		*dst++ = numFrames;
		src1 += 10;
		outsize += 3;

		for (int i = 0; i < (numFrames << 1); i++) {
			WRITE_BE_UINT16(dst, READ_LE_UINT16(src2));
			src2 += 2;
			dst += 2;
			outsize += 2;
		}
	} while (src1 < fin);

	*buffer = count; // number of items

	return out.addFile(filename, buffer, outsize);
}

// index generation

enum {
	GF_FLOPPY	= 1 <<  0,
	GF_TALKIE	= 1 <<  1,
	GF_FMTOWNS	= 1 <<  2,
	GF_DEMO		= 1 <<  3,
	GF_ENGLISH	= 1 <<  4,
	GF_FRENCH	= 1 <<  5,
	GF_GERMAN	= 1 <<  6,
	GF_SPANISH	= 1 <<  7,
	GF_ITALIAN	= 1 <<  8,
	GF_JAPANESE = 1 <<  9,
	// ...
	GF_LNGUNK	= 1 << 16,
	GF_AMIGA	= 1 << 17
};

uint32 getFeatures(const Game *g) {
	uint32 features = 0;

	if (g->special == kTalkieVersion
			|| g->special == k2CDFile1E || g->special == k2CDFile1F || g->special == k2CDFile1G || g->special == k2CDFile1I
			|| g->special == k2CDFile2E || g->special == k2CDFile2F || g->special == k2CDFile2G
			|| g->special == k2CDDemoE || g->special == k2CDDemoF || g->special == k2CDDemoG
			|| g->special == kLolCD1 || g->special == kLolCD2
			|| g->game == kKyra3)
		features |= GF_TALKIE;
	else if (g->special == kDemoVersion || g->special == k2DemoVersion || g->special == k2DemoLol)
		features |= GF_DEMO;
	else if (g->special == kDemoCDVersion)
		features |= (GF_DEMO | GF_TALKIE);
	else if (g->special == kFMTownsVersionE || g->special == kFMTownsVersionJ ||
		g->special == k2TownsFile1E || g->special == k2TownsFile1J ||
		g->special == k2TownsFile2E || g->special == k2TownsFile2J)
		features |= GF_FMTOWNS;
	else if (g->special == kAmigaVersion)
		features |= GF_AMIGA;
	else
		features |= GF_FLOPPY;

	if (g->lang == EN_ANY)
		features |= GF_ENGLISH;
	else if (g->lang == DE_DEU)
		features |= GF_GERMAN;
	else if (g->lang == FR_FRA)
		features |= GF_FRENCH;
	else if (g->lang == ES_ESP)
		features |= GF_SPANISH;
	else if (g->lang == IT_ITA)
		features |= GF_ITALIAN;
	else if (g->lang == JA_JPN)
		features |= GF_JAPANESE;

	return features;
}

bool updateIndex(byte *dst, const int dstSize, const Game *g) {
	if ((size_t)dstSize < kIndexSize)
		return false;

	WRITE_BE_UINT32(dst, kKyraDatVersion); dst += 4;
	WRITE_BE_UINT32(dst, g->game); dst += 4;
	uint32 features = READ_BE_UINT32(dst);
	features |= getFeatures(g);
	WRITE_BE_UINT32(dst, features); dst += 4;

	return true;
}

bool checkIndex(const byte *s, const int srcSize) {
	if ((size_t)srcSize < sizeof(uint32))
		return false;
	uint32 version = READ_BE_UINT32(s);
	return (version == kKyraDatVersion);
}

bool updateIndex(PAKFile &out, const Game *g) {
	char filename[32];
	createFilename(filename, g->game, -1, g->special, "INDEX");

	byte *index = new byte[kIndexSize];
	assert(index);
	memset(index, 0, kIndexSize);

	uint32 size = 0;
	const uint8 *data = out.getFileData(filename, &size);
	if (data)
		memcpy(index, data, size);

	if (!updateIndex(index, kIndexSize, g)) {
		delete[] index;
		return false;
	}

	out.removeFile(filename);
	if (!out.addFile(filename, index, kIndexSize)) {
		fprintf(stderr, "ERROR: couldn't update %s file", filename);
		delete[] index;
		return false;
	}

	return true;
}

bool checkIndex(PAKFile &out, const Game *g) {
	char filename[32];
	createFilename(filename, g->game, -1, g->special, "INDEX");

	uint32 size = 0;
	const uint8 *data = out.getFileData(filename, &size);
	if (!data)
		return true;

	return checkIndex(data, size);
}

// main processing

void printHelp(const char *f) {
	printf("Usage:\n");
	printf("%s output inputfiles ...", f);
}

bool process(PAKFile &out, const Game *g, const byte *data, const uint32 size);
const Game *findGame(const byte *buffer, const uint32 size);

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printHelp(argv[0]);
		return -1;
	}

	PAKFile out;
	out.loadFile(argv[1], false);

	for (int i = 2; i < argc; ++i) {
		FILE *input = fopen(argv[i], "rb");

		if (!input) {
			warning("skipping missing file '%s'", argv[i]);
			continue;
		}

		uint32 size = fileSize(input);
		fseek(input, 0, SEEK_SET);

		byte *buffer = new uint8[size];
		assert(buffer);

		if (fread(buffer, 1, size, input) != size) {
			warning("couldn't read from file '%s', skipping it", argv[i]);
			delete[] buffer;
			fclose(input);
			continue;
		}
		fclose(input);

		const Game *g = findGame(buffer, size);
		if (!g) {
			warning("skipping unknown file '%s'", argv[i]);
			delete[] buffer;
			continue;
		}

		if (!process(out, g, buffer, size))
			fprintf(stderr, "ERROR: couldn't process file '%s'\n", argv[i]);

		if (g->special == kFMTownsVersionE || g->special == k2TownsFile1E || g->special == k2TownsFile2E ||
			g->special == k2CDFile1E || g->special == k2CDFile2E || g->special == k2CDDemoE) {
			// This is for executables which contain support for at least 2 languages
			// The English and non language specific data has now been extracted.
			// We switch to the second language and continue extraction.
			if (!process(out, ++g, buffer, size))
				fprintf(stderr, "ERROR: couldn't process file '%s'\n", argv[i]);
		}

		if (g->special == k2CDFile1F || g->special == k2CDFile2F || g->special == k2CDDemoF) {
			// This is for executables which contain support for 3 languages.
			// We switch to the third language and continue extraction.
			if (!process(out, ++g, buffer, size))
				fprintf(stderr, "ERROR: couldn't process file '%s'\n", argv[i]);
		}

		delete[] buffer;
	}

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

const int *getNeedList(const Game *g) {
	for (const GameNeed *need = gameNeedTable; need->game != -1; ++need) {
		if (need->game == g->game && need->special == g->special)
			return need->entries;
	}

	return 0;
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
	case kKyra1TownsSFXwdTable:
		return "kKyra1TownsSFXwdTable";
	case kKyra1TownsSFXbtTable:
		return "kKyra1TownsSFXbtTable";
	case kKyra1TownsCDATable:
		return "kKyra1TownsCDATable";
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
	DataIdEntry() : data(), id(-1) {}
	DataIdEntry(SearchData d, int i) : data(d), id(i) {}

	SearchData data;
	int id;
};

typedef std::list<DataIdEntry> DataIdList;

bool process(PAKFile &out, const Game *g, const byte *data, const uint32 size) {
	char filename[128];

	if (!checkIndex(out, g)) {
		fprintf(stderr, "ERROR: corrupted INDEX file\n");
		return false;
	}

	const int *needList = getNeedList(g);
	if (!needList) {
		fprintf(stderr, "ERROR: No entry need list available\n");
		return false;
	}

	DataIdList dataIdList;
	Search search(data, size);

	bool allDataPresentAlready = true;

	for (const int *entry = needList; *entry != -1; ++entry) {
		bool found = false;

		// Try whether the data is present in the kyra.dat file already
		filename[0] = 0;
		if (!getFilename(filename, g, *entry))
			error("couldn't find filename for id %d", *entry);

		// Do not add the entry to the search list, when it's already present
		// in kyra.dat, that will speed up the creation.
		PAKFile::cFileList *list = out.getFileList();
		if (list && list->findEntry(filename) != 0)
			continue;

		allDataPresentAlready = false;

		for (const ExtractEntry *p = extractProviders; p->id != -1; ++p) {
			if (p->id == *entry) {
				// First check for special search ids
				for (const ExtractEntrySearchData *d = p->providers; d->hint.size != 0; ++d) {
					if (d->specialId == g->special) {
						found = true;

						search.addData(d->hint);
						dataIdList.push_back(DataIdEntry(d->hint, *entry));
					}
				}

				// When a special variant was found, we will break.
				if (found)
					break;

				// Add non special variants
				for (const ExtractEntrySearchData *d = p->providers; d->hint.size != 0; ++d) {
					if (d->specialId == -1) {
						found = true;

						search.addData(d->hint);
						dataIdList.push_back(DataIdEntry(d->hint, *entry));
					}
				}

				break;
			}
		}

		if (!found) {
			fprintf(stderr, "ERROR: No provider for id %d/\"%s\"\n", *entry, getIdString(*entry));
			return false;
		}
	}

	if (allDataPresentAlready)
		return true;

	// Process the data search
	Search::ResultList results;
	search.search(results);

	if (results.empty()) {
		fprintf(stderr, "ERROR: Couldn't find any required data\n");
		return false;
	}

	typedef std::map<int, Search::ResultData> IdMap;

	IdMap ids;
	for (const int *entry = needList; *entry != -1; ++entry) {
		Search::ResultList idResults;

		// Fill in all id entries found
		for (Search::ResultList::const_iterator i = results.begin(); i != results.end(); ++i) {
			for (DataIdList::const_iterator j = dataIdList.begin(); j != dataIdList.end(); ++j) {
				if (j->id == *entry && i->data == j->data)
					idResults.push_back(*i);
			}
		}

		if (idResults.size() > 1)
			warning("Multiple entries found for id %d/\"%s\"", *entry, getIdString(*entry));

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
		for (Search::ResultList::iterator i = idResults.begin(); i != idResults.end(); ++i) {
			if (result.data.size <= i->data.size) {
				if (result.offset >= i->offset)
					result = *i;
			}
		}

		ids[*entry] = result;
	}

	// Free up some memory
	results.clear();
	dataIdList.clear();

	// Compare against need list
	for (const int *entry = needList; *entry != -1; ++entry) {
		if (ids.find(*entry) != ids.end())
			continue;

		// Try whether the data is present in the kyra.dat file already
		filename[0] = 0;
		if (!getFilename(filename, g, *entry))
			error("couldn't find filename for id %d", *entry);

		PAKFile::cFileList *list = out.getFileList();
		// If the data wasn't found already, we need to break the extraction here
		if (!list || !list->findEntry(filename)) {
			fprintf(stderr, "Couldn't find id %d/\"%s\" in executable file", *entry, getIdString(*entry));
			return false;
		}
	}

	for (IdMap::const_iterator i = ids.begin(); i != ids.end(); ++i) {
		const int id = i->first;
	
		filename[0] = 0;
		if (!getFilename(filename, g, id)) {
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

		int patch = 0;
		if (g->special == kFMTownsVersionE || g->special == kFMTownsVersionJ) {
			// FM Towns files that need addional patches
			if (id == kTakenStrings || id == kNoDropStrings || id == kPoisonGoneString ||
				id == kThePoisonStrings || id == kFluteStrings || id == kWispJewelStrings)
				patch = 1;
			else if (id == kIntroStrings || id == kKyra1TownsSFXwdTable)
				patch = 2;
		}

		if (g->special == k2TownsFile1E || g->special == k2TownsFile1J) {
			if (id == k2SeqplayStrings)
				patch = 3;
		}

		if (g->special == k2FloppyFile2) {
			if (id == k2IngamePakFiles)
				patch = 4;
		}

		if (g->special == k2FloppyFile2 || g->special == k2CDFile2E) {
			if (id == k2IngameSfxFiles)
				patch = 5;
		}

		if (!tDesc->extract(out, g, data + i->second.offset, i->second.data.size, filename, patch)) {
			fprintf(stderr, "ERROR: couldn't extract id %d\n", id);
			return false;
		}
	}

	if (!updateIndex(out, g)) {
		error("couldn't update INDEX file, stop processing of all files");
		return false;
	}

	return true;
}

// game data detection

const Game *gameDescs[] = {
	kyra1Games,
	kyra2Games,
	kyra3Games,
	lolGames,
	0
};

const Game *findGame(const byte *buffer, const uint32 size) {
	md5_context ctx;
	uint8 digest[16];
	char md5str[33];

	md5_starts(&ctx);
	md5_update(&ctx, buffer, size);
	md5_finish(&ctx, digest);

	for (int j = 0; j < 16; ++j) {
		sprintf(md5str + j*2, "%02x", (int)digest[j]);
	}

	for (const Game **i = gameDescs; *i != 0; ++i) {
		for (const Game *p = *i; p->game != -1; ++p) {
			if (strcmp(md5str, p->md5) == 0)
				return p;
		}
	}

	printf("file is not supported (unknown md5 \"%s\")\n", md5str);
	return 0;
}
