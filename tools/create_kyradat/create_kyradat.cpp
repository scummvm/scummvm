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

#include "create_kyradat.h"

#include "md5.h"

enum {
	kKyraDatVersion = 14,
	kIndexSize = 12
};

// tables

#include "misc.h"
#include "eng.h"
#include "esp.h"
#include "fre.h"
#include "ger.h"

bool extractRaw(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename);
bool extractStrings(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename);
bool extractRooms(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename);
bool extractShapes(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename);

void createFilename(char *dstFilename, const int lang, const int special, const char *filename);
void createLangFilename(char *dstFilename, const int lang, const int special, const char *filename);

const ExtractType extractTypeTable[] = {
	{ kTypeLanguageList, extractStrings, createLangFilename },
	{ kTypeStringList, extractStrings, createFilename },
	{ kTypeRoomList, extractRooms, createFilename },
	{ kTypeShapeList, extractShapes, createFilename },
	{ kTypeRawData, extractRaw, createFilename },
	{ -1, 0 }
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
	
	// FM-TOWNS specific
	{ kKyra1TownsSFXTable, kTypeRawData, "SFXTABLE.TSX" },

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
	type->createFilename(dstFilename, g->lang, g->special, i->filename);
	return true;
}

void createFilename(char *dstFilename, const int lang, const int special, const char *filename) {
	strcpy(dstFilename, filename);
	
	for (const SpecialExtension *specialE = specialTable; specialE->special != -1; ++specialE) {
		if (specialE->special == special) {
			strcat(dstFilename, ".");
			strcat(dstFilename, specialE->ext);
			break;
		}
	}
}

void createLangFilename(char *dstFilename, const int lang, const int special, const char *filename) {
	strcpy(dstFilename, filename);
	
	for (const Language *langE = languageTable; langE->lang != -1; ++langE) {
		if (langE->lang == lang) {
			strcat(dstFilename, ".");
			strcat(dstFilename, langE->ext);
			break;
		}
	}
	
	for (const SpecialExtension *specialE = specialTable; specialE->special != -1; ++specialE) {
		if (specialE->special == special) {
			strcat(dstFilename, ".");
			strcat(dstFilename, specialE->ext);
			break;
		}
	}
}

// entry checking

int hashEntries(const int *entries) {
	int hash = 0;
	for (const int *i = entries; *i != -1; ++i) {
		hash += *i;
	}
	return hash;
}

bool hasEntry(const ExtractEntry *entries, const int id) {
	for (const ExtractEntry *i = entries; i->id != -1; ++i) {
		if (i->id == id)
			return true;
	}
	return false;
}

int hashEntries(const Game *game, const GameNeed *need, const PAKFile *file) {
	int hash = 0;
	char filename[128];
	for (const int *i = need->entries; *i != -1; ++i) {
		if (hasEntry(game->entries, *i)) {
			hash += *i;
			continue;
		}
		
		if (file) {
			filename[0] = 0;

			if (!getFilename(filename, game, *i))
				error("couldn't find filename for id %d", *i);
			
			PAKFile::cFileList *list = file->getFileList();
			if (list && list->findEntry(filename) != 0)
				hash += *i;
		}
	}

	return hash;
} 

bool hasNeededEntries(const Game *game, const PAKFile *file) {
	for (const GameNeed *need = gameNeedTable; need->game != -1; ++need) {
		if (need->game == game->game && need->special == game->special) {
			if (hashEntries(need->entries) == hashEntries(game, need, file))
				return true;
		}
	}

	return false;
}

// extraction

bool extractRaw(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename) {
	uint8 *buffer = new uint8[size];
	assert(buffer);
	memcpy(buffer, data, size);
	return out.addFile(filename, buffer, size);
}

bool extractStrings(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename) {
	uint32 entries = 0;
	for (uint32 i = 0; i < size; ++i) {
		if (!data[i])
			++entries;
	}
	
	uint8 *buffer = new uint8[size + 4];
	assert(buffer);
	uint8 *output = buffer;

	WRITE_BE_UINT32(output, entries); output += 4;
	memcpy(output, data, size);

	return out.addFile(filename, buffer, size + 4);
}

bool extractRooms(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename) {
	const int countRooms = size / 0x51;

	uint8 *buffer = new uint8[countRooms * 9 + 4];
	assert(buffer);
	uint8 *output = buffer;

	WRITE_BE_UINT32(output, countRooms); output += 4;

	const byte *src = data;
	for (int i = 0; i < countRooms; ++i) {
		*output++ = *src++;
		WRITE_BE_UINT16(output, READ_LE_UINT16(src)); output += 2; src += 2;
		WRITE_BE_UINT16(output, READ_LE_UINT16(src)); output += 2; src += 2;
		WRITE_BE_UINT16(output, READ_LE_UINT16(src)); output += 2; src += 2;
		WRITE_BE_UINT16(output, READ_LE_UINT16(src)); output += 2; src += 2;
		src += (0x51 - 9);
	}

	return out.addFile(filename, buffer, countRooms * 9 + 4);
}

bool extractShapes(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename) {
	byte *buffer = new byte[size + 1 * 4];
	assert(buffer);
	byte *output = buffer;

	const int count = size / 0x07;
	WRITE_BE_UINT32(output, count); output += 4;
	memcpy(output, data, size);

	return out.addFile(filename, buffer, size + 1 * 4);
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

	if (g->special == kTalkieVersion)
		features |= GF_TALKIE;
	else if (g->special == kDemoVersion)
		features |= GF_DEMO;
	else if (g->special == kFMTownsVersion)
		features |= GF_FMTOWNS;
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
	createFilename(filename, -1, g->special, "INDEX");
	
	byte *index = new byte[kIndexSize];
	assert(index);
	memset(index, 0, kIndexSize);
	
	uint32 size = 0;
	const uint8 *data = out.getFileData(filename, &size);
	if (data)
		memcpy(index, data, size);
	
	if (!updateIndex(index, kIndexSize, g)) {
		delete [] index;
		return false;
	}
	
	out.removeFile(filename);
	if (!out.addFile(filename, index, kIndexSize)) {
		fprintf(stderr, "ERROR: couldn't update %s file", filename);
		delete [] index;
		return false;
	}
	
	return true;
}

bool checkIndex(PAKFile &out, const Game *g) {
	char filename[32];
	createFilename(filename, -1, g->special, "INDEX");

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
			delete [] buffer;
			fclose(input);
			continue;
		}
		fclose(input);
		
		const Game *g = findGame(buffer, size);
		if (!g) {
			warning("skipping unknown file '%s'", argv[i]);
			delete [] buffer;
			continue;
		}
		
		if (!hasNeededEntries(g, &out)) {
			warning("file '%s' is missing offset entries and thus can't be processed", argv[i]);
			delete [] buffer;
			continue;
		}
		
		if (!process(out, g, buffer, size))
			fprintf(stderr, "ERROR: couldn't process file '%s'", argv[i]);
		
		delete [] buffer;
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

bool process(PAKFile &out, const Game *g, const byte *data, const uint32 size) {
	char filename[128];

	if (!checkIndex(out, g)) {
		fprintf(stderr, "ERROR: corrupted INDEX file\n");
		return false;
	}

	for (const ExtractEntry *i = g->entries; i->id != -1; ++i) {
		if (!getFilename(filename, g, i->id)) {
			fprintf(stderr, "ERROR: couldn't get filename for id %d\n", i->id);
			return false;
		}
		
		const ExtractFilename *fDesc = getFilenameDesc(i->id);

		if (!fDesc) {
			fprintf(stderr, "ERROR: couldn't find file description for id %d\n", i->id);
			return false;
		}

		const ExtractType *tDesc = findExtractType(fDesc->type);
		
		if (!tDesc) {
			fprintf(stderr, "ERROR: couldn't find type description for id %d\n", i->id);
			return false;
		}
		
		PAKFile::cFileList *list = out.getFileList();
		if (list && list->findEntry(filename) != 0)
			continue;

		if (!tDesc->extract(out, g, data + i->startOff, i->endOff - i->startOff, filename)) {
			fprintf(stderr, "ERROR: couldn't extract id %d\n", i->id);
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
	kyra1EngGames,
	kyra1EspGames,
	kyra1FreGames,
	kyra1GerGames,
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
